/*
 * $Id: l3.c 1.159.2.1 Broadcom SDK $
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * File:    l3.c
 * Purpose: Functions to manages XGS L3 interfaces, forwarding table,
 *          and LPM routing table.
 *
 * NOTE:
 *                ### L3 TABLE ###
 *
 * Hardware L3 table is a hashing table, for unicast, the key
 * is IP addr, for multicast, the key is either IPMC group address,
 * (IPMC group address + src IP address), or (IPMC group address +
 * src IP address + VID), depending on config settings.
 *
 * In software, software shadow copy of this L3 hash table is kept
 * (_bcm_l3_table_t->l3_hash).  The SW hash table is needed currently
 * to keep extra info about the L3 entries, such as whether it is
 * shared with LPM table, and the reference count, etc.  These extra
 * property is used, for example, in l3_age, LPM entry or IPMC entry
 * is not aged (the hit bit may be cleared for IPMC, but the entry
 * is never deleted).
 *
 *                ### DEFIP TABLE (LPM) ###
 *
 * The DEFIP table in XGS is a LPM table with 4-bit nibble  (except for
 * Draco1.5's first nibble which is 8 bits).  Physically, the LPM table
 * is a linear table with 4K entries for Draco (64K for Draco 1.5, 24K
 * in Lynx/Tucana),
 *
 * Logically, the LPM table is a 16-way radix trie (4-bit nibble),
 * with 4 octets in an IPv4 destination address, the trie has a maximum
 * depth of 8 layers (7 layers for Draco1.5 where the first nibble is 8 bits).
 * This logical view of LPM table is how the hardware logic
 * works for L3 ingress table lookup.  In order to manage LPM according to
 * this logical view, LPM is divided into 16 entry blocks, each block
 * corresponds to a 16-way node in the trie.  Any entry in a block can be in
 * three possible states:
 *
 *   - it points to another block (with NEXT_POINTER)
 *   - it is a leaf entry (with NEXT_POINTER pointing to L3 or ECMP table)
 *   - it is not used (VALID bit is zero)
 *
 * It's common for the LPM table to have many entries left unfilled,
 * yet there is no blocks left, i.e. all LPM table blocks are used in the
 * trie, but there are unused entries in these blocks.  When this happens,
 * it is possible that a particular IP route can _NOT_ be installed
 * into the LPM table (bcm_defip_add() returns BCM_E_FULL).  Because
 * this IP route needs to branch into a new block which is not available
 * (all blocks used already) - this has a consequence that means you may
 * never be able to fully utilize all the entries of a LPM table,
 * unless with some special set of IP routes. And also, you can not know
 * exactly how many routes can be installed in the LPM table.
 *
 * In software, a shadow copy of the whole LPM table is kept
 * (l3_defip_table->l3_lpm).  This is necessary because adding
 * a route involves walking from the root of the LPM trie
 * down to a leaf node.
 *
 * Note that as the hardware LPM table size grows, it may take
 * significant amount of memory for these software tables.
 * For example, in Draco1.5, LPM is 64K, so software copy of the LPM
 * table takes about 64K * 20(or more, see _bcm_lpm_entry_t definition
 * for memory usage per entry) bytes of memory per Draco1.5 chip.
 */

#ifdef INCLUDE_L3

#include <sal/core/libc.h>
#include <sal/core/boot.h>
#include <shared/bitop.h>

#include <soc/drv.h>
#include <soc/l3x.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <soc/hash.h>
#include <soc/tucana.h>

#include <bcm/l2.h>
#include <bcm/l3.h>

#include <bcm_int/esw/draco.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/stack.h>

#include <bcm_int/esw_dispatch.h>


#define	L3_INFO(unit)	(&_bcm_l3_bk_info[unit])
#define INTF_LOCK(unit) \
        soc_mem_lock(unit, L3_INTFm)
#define INTF_UNLOCK(unit) \
        soc_mem_unlock(unit, L3_INTFm)
#define L3_LOCK(unit) \
        soc_mem_lock(unit, L3Xm)
#define L3_UNLOCK(unit) \
        soc_mem_unlock(unit, L3Xm)
#define DEFIP_LOCK(unit) \
        soc_mem_lock(unit, LPM_MEM(unit))
#define DEFIP_UNLOCK(unit) \
        soc_mem_unlock(unit, LPM_MEM(unit))

#define LPM_MEM(unit) \
	l3_defip_table[unit].lpm_mem
#define LPMHIT_MEM(unit) \
	l3_defip_table[unit].lpmhit_mem
#define L3_LPM_ENTRY(_u_, _idx_) \
	&l3_defip_table[_u_].l3_lpm[_idx_]

#define LPM_BITS_PER_BLK  4
#define LPM_BLK_SIZE	  (1 << LPM_BITS_PER_BLK)
#define LPM_BLK_ALL_BITS  ((1 << LPM_BLK_SIZE) - 1)
#define LPM_LONG_BLK_SIZE 256
#define L3_BUCKET_SIZE    8
#define L3_MAX_HASH_BYTES 10
#define L3_BUCKET_OFFSET_BMP 7
#define INVALID_LPM_IDX   0xffff
#define INVALID_L3_IDX    0xffff
#define MIN(a, b)	  ((a) > (b) ? (b) : (a))

/* Prefix length should be 0-32 (0 for default route) */
#define INVALID_PREFIX_LEN 33

/* Draco1.5 per VLAN default route related, means for all VLANs */
#define L3_VLAN_GLOBAL    BCM_VLAN_NONE
/* Draco1.5 per VLAN default route delete, all VLANs + global */
#define L3_VLAN_ALL       BCM_VLAN_ALL
/* L3 index reserved for global default route in Draco1.5 */
#define L3_D15_GLOBAL_DEF_ROUTE_IDX  0
/* L3 index reserved for directly connected route's use */
#define L3_DEFIP_LOCAL_ROUTE_IDX(_u_) \
        (l3_table[_u_].l3_max - 1)

#define L3_SET_ARL_CTRL_EXCL_IPMC_BIT
#define L3_GLOBAL_DEF_ROUTE_USE_IPMC_BIT
#define L3_DEFIP_LOCALROUTE_USE_IPMC_BIT
#define L3_GLOBAL_BLACKHOLE_PORT   (0x1f)
/* #define L3_HOST_TO_LPM_IF_L3_FULL */

/*
 * The first block is 8 bits (not 4 bits) for Draco1.5
 */
#define IS_LPM_LONG_BLOCK(unit, _lpm_idx_) \
	((SOC_IS_DRACO15(unit)) && (_lpm_idx_ == 0))

/*
 * LPM block usage bit mask operations
 */
#define BCM_L3_LPM_BLOCK_GET(_u_, _blk_) \
	SHR_BITGET(l3_defip_table[(_u_)].lpm_blk_used, (_blk_))
#define BCM_L3_LPM_BLOCK_SET(_u_, _blk_) \
	SHR_BITSET(l3_defip_table[(_u_)].lpm_blk_used, (_blk_))
#define BCM_L3_LPM_BLOCK_CLR(_u_, _blk_) \
	SHR_BITCLR(l3_defip_table[(_u_)].lpm_blk_used, (_blk_))

/*
 * L3 interface number handling for 5674
 * Usable generally, but only needed when Lynx might be used
 * (Thus, it is not inserted in code which already checks for
 *  Draco-variants).
 */
#define BCM_L3X_INTF_NUM_GET(_u_, _mem_, _pentry_) \
        (soc_mem_field32_get(_u_, _mem_, _pentry_, L3_INTF_NUMf) | \
         ((soc_feature(unit, soc_feature_lynx_l3_expanded) ? \
           soc_mem_field32_get(_u_, _mem_, _pentry_, L3_INTF_NUM_HIf) : 0) \
          << BCM_LYNX_L3_INTF_NUM_HI_SHIFT))

#define BCM_L3X_INTF_NUM_SET(_u_, _mem_, _pentry_, _l3ifn_) \
{ \
    if (soc_feature(unit, soc_feature_lynx_l3_expanded)) { \
        soc_mem_field32_set(_u_, _mem_, _pentry_, L3_INTF_NUMf, \
                            ((_l3ifn_) & BCM_LYNX_L3_INTF_NUM_L0_MASK)); \
        soc_mem_field32_set(_u_, _mem_, _pentry_, L3_INTF_NUM_HIf, \
                            ((_l3ifn_) >> BCM_LYNX_L3_INTF_NUM_HI_SHIFT)); \
    } else { \
        soc_mem_field32_set(_u_, _mem_, _pentry_, L3_INTF_NUMf, _l3ifn_); \
    } \
}

/*
 * L3 LPM next pointer/L3 index handling for 5674
 * Usable generally, but only needed when Lynx might be used
 * (Thus, it is not inserted in code which already checks for
 *  Draco-variants).
 */
#define BCM_LPM_NEXT_PTR_GET(_u_, _pentry_) \
        (soc_mem_field32_get(_u_, LPM_MEM(unit), _pentry_, NEXT_PTRf) | \
         ((soc_feature(unit, soc_feature_lynx_l3_expanded) ? \
           soc_mem_field32_get(_u_, LPM_MEM(unit), _pentry_, NEXT_PTR_HIf) : 0) \
          << BCM_LYNX_LPM_NEXT_PTR_HI_SHIFT))

#define BCM_LPM_NEXT_PTR_SET(_u_, _pentry_, _next_) \
{ \
    if (soc_feature(unit, soc_feature_lynx_l3_expanded)) { \
        soc_mem_field32_set(_u_, LPM_MEM(unit), _pentry_, NEXT_PTRf, \
                            ((_next_) & BCM_LYNX_LPM_NEXT_PTR_L0_MASK)); \
        soc_mem_field32_set(_u_, LPM_MEM(unit), _pentry_, NEXT_PTR_HIf, \
                            ((_next_) >> BCM_LYNX_LPM_NEXT_PTR_HI_SHIFT)); \
    } else { \
        soc_mem_field32_set(_u_, LPM_MEM(unit), _pentry_, NEXT_PTRf, _next_); \
    } \
}

/*
 * Software book-keeping of L3 tables
 *
 * The hardware L3 table is implemented as hash table.  We have
 * a software shadow copy of this hash table, that keep simpler info.
 */
static _bcm_l3_defip_tbl_t  l3_defip_table[BCM_MAX_NUM_UNITS];
static _bcm_l3_table_t      l3_table[BCM_MAX_NUM_UNITS];

/*
 * Forward declaration
 */
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
STATIC int _xgs_route_lookup(int unit, _bcm_defip_cfg_t *lpm_cfg);
STATIC int _bcm_xgs_host_to_lpm_add(int unit, _bcm_l3_cfg_t *l3cfg);
STATIC int _bcm_xgs_host_to_lpm_del(int unit, _bcm_l3_cfg_t *l3cfg);
#endif
STATIC int _bcm_xgs_lpm_do_hit(int unit, _bcm_defip_cfg_t *lpm_cfg, int action);


#define L3_RI_NETLEN_SHIFT      0
#define L3_RI_ECMP_SHIFT        6
#define L3_RI_L3IDX_SHIFT       7
#define L3_RI_INTF_SHIFT        19
#define L3_RI_ECMPCNT_SHIFT     19
#define L3_RI_ECMPCNT_D15_SHIFT 7
#define L3_RI_ECMPIDX_D15_SHIFT 12

#define L3_RI_NETLEN_MASK       0x3f
#define L3_RI_L3IDX_MASK        0xfff
#define L3_RI_INTF_MASK         0xff
#define L3_RI_ECMP_MASK         0x1
#define L3_RI_ECMPCNT_MASK      0x7
#define L3_RI_ECMPCNT_D15_MASK  0x1f
#define L3_RI_ECMPIDX_D15_MASK  0x7ff

/*
 * Function:
 *	_print_net_addr
 * Purpose:
 *	Debug routine for printing network address
 */
STATIC INLINE void
_print_net_addr(uint32 flags, char *prefix,
                ip_addr_t ip_addr, int netmasklen, char *postfix)
{
    if (soc_cm_debug_check(flags)) {
        soc_cm_print("%s%d.%d.%d.%d/%d%s", prefix,
            (ip_addr >> 24) & 0xff, (ip_addr >> 16) & 0xff,
            (ip_addr >> 8) & 0xff, (ip_addr) & 0xff, netmasklen, postfix);
    }
}

/*
 * Function:
 *      _bcm_l3_from_l3x
 * Purpose:
 *      Internal function to translate from l3x_entry_t to _bcm_l3_cfg_t
 */

int
_bcm_l3_from_l3x(int unit, _bcm_l3_cfg_t *l3cfg, l3x_entry_t *l3x_entry)
{
    sal_memset(l3cfg, 0, sizeof (*l3cfg));

    /* Valid bit is ignored here; entry is assumed valid */

    l3cfg->l3c_ip_addr = soc_L3Xm_field32_get(unit, l3x_entry, IP_ADDRf);
    l3cfg->l3c_intf = BCM_L3X_INTF_NUM_GET(unit, L3Xm, l3x_entry);

    /* Further decode depends on whether this is IPMC or not */
    if (soc_L3Xm_field32_get(unit, l3x_entry, IPMCf)) {
        l3cfg->l3c_flags |= BCM_L3_IPMC;
    }

    if (BCM_IP4_MULTICAST(l3cfg->l3c_ip_addr) ) {
        l3cfg->l3c_src_ip_addr =
            soc_L3Xm_field32_get(unit, l3x_entry, SRC_IP_ADDRf);
        l3cfg->l3c_ipmc_ptr = soc_L3Xm_field32_get(unit, l3x_entry, IPMC_PTRf);

        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            l3cfg->l3c_vid = soc_L3Xm_field32_get(unit, l3x_entry, VLAN_IDf);
        }
    } else {
        uint32          tgid_port;
        tgid_port = soc_L3Xm_field32_get(unit, l3x_entry, TGID_PORTf);
        if (tgid_port & BCM_TGID_TRUNK_INDICATOR(unit)) {
            l3cfg->l3c_flags |= BCM_L3_TGID;
            l3cfg->l3c_port_tgid = tgid_port & BCM_TGID_PORT_TRUNK_MASK(unit);
            if (SOC_IS_DRACO15(unit)) {
		tgid_port = soc_L3Xm_field32_get(unit, l3x_entry, TGID_HIf);
		l3cfg->l3c_port_tgid |= (tgid_port << BCM_TGID_TRUNK_LO_BITS(unit));
            }
        } else {
	    bcm_module_t	mod_in, mod_out;
	    bcm_port_t		port_in, port_out;

	    mod_in = soc_L3Xm_field32_get(unit, l3x_entry, MODULE_IDf);
	    port_in = tgid_port;
	    BCM_IF_ERROR_RETURN
		(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in,
                                        &mod_out, &port_out));
            l3cfg->l3c_modid = mod_out;
            l3cfg->l3c_port_tgid = port_out;
        }
        soc_L3Xm_mac_addr_get(unit, l3x_entry, MAC_ADDRf, l3cfg->l3c_mac_addr);
    }

    if (soc_L3Xm_field32_get(unit, l3x_entry, L3_HITf)) {
        l3cfg->l3c_flags |= BCM_L3_HIT;
    }

    if (soc_L3Xm_field32_get(unit, l3x_entry, UNTAGf)) {
        l3cfg->l3c_flags |= BCM_L3_UNTAG;
    }

    /* Note:  May have to code/decode more fields including ip_mask */
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_l3_to_l3x
 * Purpose:
 *      Internal function to translate from _bcm_l3_cfg_t to l3x_entry_t
 */

int
_bcm_l3_to_l3x(int unit, l3x_entry_t *l3x_entry, _bcm_l3_cfg_t *l3cfg)
{
    sal_memset(l3x_entry, 0, sizeof (*l3x_entry));

    soc_L3Xm_field32_set(unit, l3x_entry, L3_VALIDf, 1);
    soc_L3Xm_field32_set(unit, l3x_entry, IP_ADDRf, l3cfg->l3c_ip_addr);
    BCM_L3X_INTF_NUM_SET(unit, L3Xm, l3x_entry, l3cfg->l3c_intf);

    if (l3cfg->l3c_flags & BCM_L3_IPMC) {
        soc_L3Xm_field32_set(unit, l3x_entry, IPMCf, 1);
    }

    if (BCM_IP4_MULTICAST(l3cfg->l3c_ip_addr)) {
        soc_L3Xm_field32_set(unit, l3x_entry, SRC_IP_ADDRf,
                             l3cfg->l3c_src_ip_addr);
        soc_L3Xm_field32_set(unit, l3x_entry, IPMC_PTRf,
                             l3cfg->l3c_ipmc_ptr);

        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            soc_L3Xm_field32_set(unit, l3x_entry, VLAN_IDf, l3cfg->l3c_vid);
        }
    } else {
        if (l3cfg->l3c_flags & BCM_L3_TGID) {
	    if (SOC_IS_DRACO15(unit)) {
		soc_L3Xm_field32_set(unit, l3x_entry, Tf, 1);
		soc_L3Xm_field32_set(unit, l3x_entry, TGID_LOf,
				     l3cfg->l3c_port_tgid &
				     BCM_TGID_PORT_TRUNK_MASK(unit));
		soc_L3Xm_field32_set(unit, l3x_entry, TGID_HIf,
				     (l3cfg->l3c_port_tgid
				      & BCM_TGID_PORT_TRUNK_MASK_HI(unit)) >> BCM_TGID_TRUNK_LO_BITS(unit));
	    } else {
		soc_L3Xm_field32_set(unit, l3x_entry, TGID_PORTf,
				     l3cfg->l3c_port_tgid |
				     BCM_TGID_TRUNK_INDICATOR(unit));
	    }
        } else {
    	    bcm_module_t	mod_in, mod_out;
    	    bcm_port_t		port_in, port_out;
    
    	    mod_in = l3cfg->l3c_modid;
    	    port_in = l3cfg->l3c_port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit);
            if (port_in != l3cfg->l3c_port_tgid) {
                return BCM_E_PORT;
            }
    	    BCM_IF_ERROR_RETURN
    		(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                        mod_in, port_in,
                                        &mod_out, &port_out));
            if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
                return BCM_E_BADID;
            }
            if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
                return BCM_E_PORT;
            }
    	    soc_L3Xm_field32_set(unit, l3x_entry, MODULE_IDf, mod_out);
    	    soc_L3Xm_field32_set(unit, l3x_entry, TGID_PORTf, port_out);
        }
        soc_L3Xm_mac_addr_set(unit, l3x_entry, MAC_ADDRf, l3cfg->l3c_mac_addr);
    }

    if (l3cfg->l3c_flags & BCM_L3_HIT) {
        soc_L3Xm_field32_set(unit, l3x_entry, L3_HITf, 1);
    }

    if (l3cfg->l3c_flags & BCM_L3_UNTAG) {
        soc_L3Xm_field32_set(unit, l3x_entry, UNTAGf, 1);
    }

    /* Note:  May have to code/decode more fields including ip_mask */
    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_l3_calc_hash_bucket
 * Purpose:
 *	Calculate the L3 hash bucket with SW simulating HW algorithm
 * Parameters:
 *	unit   - unit number.
 *	l3hash - pass in the IP entry info (IP addr for UC, IP addr+SIP for MC)
 *	bucket - (OUT) hash bucket
 *	no_src_ip - (OUT) 1 if SRC IP is used (depends on hw config)
 *	no_vid - (OUT) 1 if VLAN ID is used in hash calculation
 *                     (depends on hw config)
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_bcm_l3_calc_hash_bucket(int unit, _bcm_l3hash_t *l3hash, uint32 *bucket,
                         int *no_src_ip, int *no_vid)
{
    uint8 key[XGS_HASH_KEY_SIZE];
    uint32 hash_control;
    ip_addr_t src_ip;
    uint32 config_reg;
    int hash_sel, ipmc, port;
    int vid = 0, do_vlan = FALSE;

    if (BCM_IP4_MULTICAST(l3hash->l3h_ip_addr)) {
        /* Multicast IP address */
        src_ip = l3hash->l3h_sip_addr;
        ipmc = TRUE;

        if (SOC_IS_DRACO1(unit)) {    /* Draco */
            SOC_IF_ERROR_RETURN
		(soc_reg_read_any_block(unit, CONFIGr, &config_reg));
            if (soc_reg_field_get(unit, CONFIGr,
				  config_reg, SRC_IP_CFGf)) {
                src_ip = 0;
            }
        } else if (SOC_IS_LYNX(unit)) {  /* LYNX */
            SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &config_reg));
            if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
                if (soc_reg_field_get(unit, ARL_CONTROLr, config_reg,
                                      IPMC_DO_VLANf)) {
                    vid = l3hash->l3h_vid;
                }
            }
            if (soc_reg_field_get(unit, ARL_CONTROLr,
                                  config_reg, SRC_IP_CFGf)) {
                src_ip = 0;
            }
        } else if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            /*
             * Depending on IPMC_DO_VLANf, draco 1.5 IPMC key is either
             * (DIP + SIP + VID) or (DIP + SIP + 0), always 76 bits
             */
	    /* WARNING: assumes all ports have IPMC_DO_VLANf set the same */
            PBMP_E_ITER(unit, port) {
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_port_config_get(unit, port,
                                              _bcmPortIpmcVlanKey,
                                              &do_vlan));
                break;
            }
            if (do_vlan) {
                vid = l3hash->l3h_vid;
            } else {
                vid = 0;
            }
        }
    } else {
        /* unicast IP address */
        src_ip = 0;
        ipmc = FALSE;
    }

    if (SOC_IS_LYNX(unit)) {
        /* Must always use src IP value for key comparisions on 5673/74 */
        *no_src_ip = 0;
    } else {
        *no_src_ip = ((src_ip == 0) ? 1 : 0);
    }
    *no_vid = ((vid == 0) ? 1 : 0);

    soc_draco_l3x_param_to_key(l3hash->l3h_ip_addr, src_ip, vid, key);

    SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &hash_control));
    hash_sel = soc_reg_field_get(unit, HASH_CONTROLr, hash_control, HASH_SELECTf);

    *bucket = soc_draco_l3_hash(unit, hash_sel, ipmc, key);

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_l3hash_index_get
 * Purpose:
 *	Get the exact hash table index
 * Parameters:
 *      unit   - unit number.
 *	l3hash - pass in the IP entry info
 *	index - (OUT) hash index in L3 table, if found
 * Returns:
 *	BCM_E_NONE - success
 *	BCM_E_NOT_FOUND - entry not found in hash bucket
 *      BCM_E_XXX - other
 */
STATIC int
_bcm_l3hash_index_get(int unit, _bcm_l3hash_t *l3hash, int *index)
{
    _bcm_l3hash_t *hash_entry;
    uint32 hash_b;
    int no_sip, no_vid;
    int i, hw_index;

    *index = -1;

    BCM_IF_ERROR_RETURN
	(_bcm_l3_calc_hash_bucket(unit, l3hash, &hash_b, &no_sip, &no_vid));

    hw_index = hash_b * L3_BUCKET_SIZE;
    for (i = 0; i < L3_BUCKET_SIZE; i++, hw_index++) {
        hash_entry = &l3_table[unit].l3_hash[hw_index];

        if (BCM_IP4_MULTICAST(l3hash->l3h_ip_addr)) { 
            if (hash_entry->l3h_ip_addr == l3hash->l3h_ip_addr) {
                if (!no_sip && hash_entry->l3h_sip_addr != l3hash->l3h_sip_addr) {
                    continue;
                }
                if (!no_vid && hash_entry->l3h_vid != l3hash->l3h_vid) {
                    continue;
                }

                *index = hw_index;
                return BCM_E_NONE;
            }
        } else {
            if (hash_entry->l3h_ip_addr == l3hash->l3h_ip_addr) {
                *index = hw_index;
                return BCM_E_NONE;
            }
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_l3sw_hash_insert
 * Purpose:
 *      Insert an entry to L3 shadow tables (S/W hash table)
 * Parameters:
 *      unit - SOC unit number.
 *      l3hash - entry info
 *	index - (OUT) index in L3 table, if successful
 * Returns:
 *	BCM_E_NONE - success
 *	BCM_E_FULL - bucket full
 *	BCM_E_XXX - other
 * Notes:
 *	If a duplicate entry already exists, returns success and the
 *	index is valid.
 */
STATIC int
_bcm_l3sw_hash_insert(int unit, int type, _bcm_l3hash_t *l3hash, int *index)
{
    uint32 hash_b;
    int i, hw_index, insert_index = -1;
    int no_sip, no_vid;
    _bcm_l3hash_t *hash_entry;

    *index = -1;

    BCM_IF_ERROR_RETURN
	(_bcm_l3_calc_hash_bucket(unit, l3hash, &hash_b, &no_sip, &no_vid));

    /* Loop through entries in this bucket to see if IP address exists */
    hw_index = hash_b * L3_BUCKET_SIZE;
    for (i = 0; i < L3_BUCKET_SIZE; i++, hw_index++) {
        hash_entry = &l3_table[unit].l3_hash[hw_index];

        if (hash_entry->l3h_ip_addr == 0 && insert_index == -1) {
            insert_index = hw_index;
        }

        /* If the entry already exists */
        if (BCM_IP4_MULTICAST(l3hash->l3h_ip_addr)) {
            if (hash_entry->l3h_ip_addr == l3hash->l3h_ip_addr) {
                if (!no_sip && hash_entry->l3h_sip_addr != l3hash->l3h_sip_addr) {
                    continue;
                }
                if (!no_vid && hash_entry->l3h_vid != l3hash->l3h_vid) {
                    continue;
                }

                l3hash->l3h_use_count = hash_entry->l3h_use_count;
	        *index = hw_index;
                return BCM_E_NONE;
            }
        } else {
            if (hash_entry->l3h_ip_addr == l3hash->l3h_ip_addr) {
	        *index = hw_index;

                if (type == L3H_LPM) {
                    hash_entry->l3h_flags |= L3H_LPM;
                    hash_entry->l3h_use_count++;
                } else {
                    if ((hash_entry->l3h_flags & L3H_HOST) == 0) {
                        hash_entry->l3h_flags |= L3H_HOST;
                        hash_entry->l3h_use_count++;
                    }
                }
                l3hash->l3h_use_count = hash_entry->l3h_use_count;

                return BCM_E_NONE;
            }
        }
    }

    /* Does not exist, look for empty entry */
    if (insert_index != -1) {
        hash_entry = &l3_table[unit].l3_hash[insert_index];

        hash_entry->l3h_ip_addr = l3hash->l3h_ip_addr;
        if (BCM_IP4_MULTICAST(l3hash->l3h_ip_addr)) {
            hash_entry->l3h_sip_addr = l3hash->l3h_sip_addr;
            hash_entry->l3h_vid = l3hash->l3h_vid;
        } else {
            hash_entry->l3h_sip_addr = 0;
            hash_entry->l3h_vid = 0;
        }

        if (type == L3H_LPM) {
            hash_entry->l3h_flags = L3H_LPM;
        } else {
            hash_entry->l3h_flags = L3H_HOST;
        }

        hash_entry->l3h_use_count = l3hash->l3h_use_count = 1;
        l3_table[unit].l3_count++;
        *index = insert_index;

        return BCM_E_NONE;
    }

    return BCM_E_FULL;
}

/*
 * Function:
 *	_bcm_l3sw_hash_del
 * Purpose:
 *      Delete an entry to L3 shadow tables (S/W hash table)
 * Parameters:
 *      unit - SOC unit number.
 *	l3hash - Pointer to memory for which tell what L3 to delete.
 *	index - (OUT) index in L3 table, if successful
 * Returns:
 *      BCM_E_NONE - success
 *	BCM_E_NOT_FOUND - entry not found
 *	BCM_E_XXX - other
 */
STATIC int
_bcm_l3sw_hash_del(int unit, int type, _bcm_l3hash_t *l3hash, int *index)
{
    _bcm_l3hash_t *hash_entry;
    uint32 hash_b;
    int no_sip, no_vid;
    int i, hw_index;

    *index = -1;

    BCM_IF_ERROR_RETURN
	(_bcm_l3_calc_hash_bucket(unit, l3hash, &hash_b, &no_sip, &no_vid));

    /*
     * Loop through all entries in this bucket
     */
    if (BCM_IP4_MULTICAST(l3hash->l3h_ip_addr)) {
        for (hw_index = hash_b * L3_BUCKET_SIZE, i = 0;
             i < L3_BUCKET_SIZE; i++, hw_index++) {
            hash_entry = &l3_table[unit].l3_hash[hw_index];

            if (hash_entry->l3h_ip_addr == l3hash->l3h_ip_addr) {
                if (!no_sip && hash_entry->l3h_sip_addr != l3hash->l3h_sip_addr) {
                    continue;
                }
                if (!no_vid && hash_entry->l3h_vid != l3hash->l3h_vid) {
                    continue;
                }

                l3hash->l3h_use_count = 0;
                hash_entry->l3h_ip_addr = 0;
                l3_table[unit].l3_count--;

	        *index = hw_index;

	        return BCM_E_NONE;
            }
        }
    } else {
        for (hw_index = hash_b * L3_BUCKET_SIZE, i = 0;
             i < L3_BUCKET_SIZE; i++, hw_index++) {
            hash_entry = &l3_table[unit].l3_hash[hw_index];

            if (hash_entry->l3h_ip_addr == l3hash->l3h_ip_addr) {
                assert(hash_entry->l3h_use_count >= 1);

                if (type == L3H_HOST) {
                    if (hash_entry->l3h_flags & L3H_HOST) {
                        hash_entry->l3h_use_count--;
                        hash_entry->l3h_flags &= ~L3H_HOST;
                    }
                } else {
                    if (hash_entry->l3h_flags & L3H_LPM) {
                        hash_entry->l3h_use_count--;
                        if ((hash_entry->l3h_use_count == 1) &&
                            (hash_entry->l3h_flags & L3H_HOST)) {
                            hash_entry->l3h_flags &= ~L3H_LPM;
                        }
                    }
                }

                l3hash->l3h_use_count = hash_entry->l3h_use_count;
                if (l3hash->l3h_use_count == 0) {
                    hash_entry->l3h_ip_addr = 0;
                    l3_table[unit].l3_count--;
                }

	        *index = hw_index;

	        return BCM_E_NONE;
            }
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *	_bcm_l3sw_index_del
 * Purpose:
 *      Delete an entry to L3 shadow tables at specified index
 *      of hash table (S/W hash table)
 * Parameters:
 *      unit - SOC unit number.
 *	index - index
 * Returns:
 *      Usage count
 */
STATIC int
_bcm_l3sw_index_del(int unit, int type, int index, int *l3h_use_count)
{
    _bcm_l3hash_t *hash_entry;

    hash_entry = &l3_table[unit].l3_hash[index];

    if (type == L3H_HOST) {
        if (hash_entry->l3h_flags & L3H_HOST) {
            hash_entry->l3h_use_count--;
            hash_entry->l3h_flags &= ~L3H_HOST;
        }
    } else {
        if (hash_entry->l3h_flags & L3H_LPM) {
            hash_entry->l3h_use_count--;
            if ((hash_entry->l3h_use_count == 1) &&
                (hash_entry->l3h_flags & L3H_HOST)) {
                hash_entry->l3h_flags &= ~L3H_LPM;
            }
        }
    }

    *l3h_use_count = hash_entry->l3h_use_count;

    if (hash_entry->l3h_use_count == 0) {
        hash_entry->l3h_ip_addr = 0;
        l3_table[unit].l3_count--;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_l3sw_add_lpm_entry
 * Purpose:
 *      Insert a pure LPM entry (address LPM_L3_ADDR) at the index
 * Parameters:
 *	unit  - SOC unit
 *	index - index
 * Returns:
 *      Usage count
 * Note:
 *      Used for Tucana style ECMP route add
 */
STATIC void
_bcm_l3sw_add_lpm_entry(int unit, int index, bcm_ip_t nexthop)
{
    l3_table[unit].l3_hash[index].l3h_ip_addr = LPM_L3_ADDR;
    l3_table[unit].l3_hash[index].l3h_sip_addr = nexthop;
    l3_table[unit].l3_hash[index].l3h_use_count = 1;
    l3_table[unit].l3_hash[index].l3h_flags = L3H_LPM;
    l3_table[unit].l3_hash[index].l3h_vid = 0;
    l3_table[unit].l3_count++;
}


/*
 * Function:
 *	_bcm_xgs_get_untag
 * Purpose:
 *      Determine whether the specified port is tagged or untagged
 *      in the vlan corresponding to the specified interface.
 *      
 * Parameters:
 *	unit     - unit number.
 *	intf_num - L3 interface number
 *	modid    - module id
 *	port     - TGID/port number
 *      ut       - pointer to untagged value
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_bcm_xgs_get_untag(int unit, int intf_num, int modid,
                   bcm_port_t port, int *ut)
{
    l3intf_entry_t l3intf;
    pbmp_t pbmp, ubmp;
    int my_modid, isGport, is_local_modid;
    bcm_trunk_t tid;
    bcm_trunk_add_info_t t_info;
    bcm_port_t tport;
    bcm_vlan_t vlan;

    *ut = 0;

    BCM_IF_ERROR_RETURN (bcm_esw_stk_my_modid_get(unit, &my_modid));

    if (port & BCM_TGID_TRUNK_INDICATOR(unit)) {
        /* Determine TID */
        tid = port & BCM_TGID_PORT_TRUNK_MASK(unit);

        /* Map trunk to (non-stack) port */
        BCM_IF_ERROR_RETURN(bcm_esw_trunk_get(unit, tid, &t_info));
        BCM_IF_ERROR_RETURN
            (bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
        if (isGport) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_trunk_gport_resolve(unit, tid, &t_info));
        }

        for (tport = 0; tport < t_info.num_ports; tport++) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_modid_is_local(unit, t_info.tm[tport],
                                         &is_local_modid));
            if (is_local_modid) {
                port = t_info.tp[tport];
                break;
            }
        }

        /* Check if failed to find tport */
        if (port & BCM_TGID_TRUNK_INDICATOR(unit)) {
            /* Failed */
            return BCM_E_NONE;
        }
    } else if (modid != my_modid) {
        /* Other unit will L2 switch */
        return BCM_E_NONE;
    }

    SOC_IF_ERROR_RETURN
        (READ_L3_INTFm(unit, MEM_BLOCK_ANY, intf_num, &l3intf));

    vlan = soc_L3_INTFm_field32_get(unit, &l3intf, VLAN_IDf);
    BCM_IF_ERROR_RETURN(bcm_esw_vlan_port_get(unit, vlan, &pbmp, &ubmp));

    if (!BCM_PBMP_MEMBER(pbmp, port)) {
        /* Port is not a member of the vlan */
        return BCM_E_NONE;
    }

    *ut = BCM_PBMP_MEMBER(ubmp, port) ? 1 : 0;
    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_xgs_l3hw_add_lpm_entry
 * Purpose:
 *      Insert a pure LPM entry (address LPM_L3_ADDR) at the index
 * Parameters:
 *	unit  - unit number.
 *	index - index
 *	lpm_cfg - DEFIP L3 entry related info
 * Returns:
 *	BCM_E_XXX
 * Note:
 *      Used for Tucana style ECMP route add
 */
STATIC int
_bcm_xgs_l3hw_add_lpm_entry(int unit, int index, _bcm_defip_cfg_t *lpm_cfg)
{
    l3x_base_entry_t b_entry;
    l3x_valid_entry_t v_entry;
    uint32 bmval;
    int ut;
    int rv;

    sal_memset(&b_entry, 0, sizeof(l3x_base_entry_t));

    soc_L3Xm_field32_set(unit, &b_entry, IP_ADDRf, LPM_L3_ADDR);
    soc_L3Xm_mac_addr_set(unit, &b_entry, MAC_ADDRf, lpm_cfg->defip_mac_addr);
    BCM_L3X_INTF_NUM_SET(unit, L3Xm, &b_entry, lpm_cfg->defip_intf);

    if (lpm_cfg->defip_flags & BCM_L3_TGID) {
	if (SOC_IS_DRACO15(unit)) {
	    soc_L3Xm_field32_set(unit, &b_entry, Tf, 1);
	    soc_L3Xm_field32_set(unit, &b_entry, TGID_LOf,
				 lpm_cfg->defip_port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit));
	    soc_L3Xm_field32_set(unit, &b_entry, TGID_HIf,
				 (lpm_cfg->defip_port_tgid &
				  BCM_TGID_PORT_TRUNK_MASK_HI(unit)) >> BCM_TGID_TRUNK_LO_BITS(unit));
	} else {
	    soc_L3Xm_field32_set(unit, &b_entry, TGID_PORTf,
				 lpm_cfg->defip_port_tgid | BCM_TGID_TRUNK_INDICATOR(unit));
	}
        _bcm_xgs_get_untag(unit, lpm_cfg->defip_intf, 0, 
                           lpm_cfg->defip_port_tgid | BCM_TGID_TRUNK_INDICATOR(unit), &ut);
    } else {
    	bcm_module_t	mod_in, mod_out;
    	bcm_port_t		port_in, port_out;
    
    	mod_in = lpm_cfg->defip_modid;
    	port_in = lpm_cfg->defip_port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit);
    	BCM_IF_ERROR_RETURN
    	    (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));
        if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
            return BCM_E_BADID;
        }
        if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
            return BCM_E_PORT;
        }
    	soc_L3Xm_field32_set(unit, &b_entry, MODULE_IDf, mod_out);
    	soc_L3Xm_field32_set(unit, &b_entry, TGID_PORTf, port_out);
        _bcm_xgs_get_untag(unit, lpm_cfg->defip_intf, mod_out, port_out, &ut);
    }
    soc_L3Xm_field32_set(unit, &b_entry, UNTAGf, ut);

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L3X_BASEm, MEM_BLOCK_ALL, index, &b_entry));

    /* set valid bit */
    rv  = soc_mem_read(unit, L3X_VALIDm, MEM_BLOCK_ANY, index >> 3, &v_entry);
    BCM_IF_ERROR_RETURN(rv);
    soc_L3X_VALIDm_field_get(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    bmval |= (1 << (index & L3_BUCKET_OFFSET_BMP));
    soc_L3X_VALIDm_field_set(unit, &v_entry, BUCKET_BITMAPf, &bmval);

    rv = soc_mem_write(unit, L3X_VALIDm, MEM_BLOCK_ALL, index >> 3, &v_entry);
    BCM_IF_ERROR_RETURN(rv);

    return BCM_E_NONE;
}

/*
 * The last L3 table entry is reserved for local (directly connected)
 * routes, i.e. all local route's LPM entries point to the last L3 entry.
 * This entry is either set to invalid, or valid with IPMC bit set.
 * either way packet is sent to local CPU.
 */
STATIC int
_bcm_xgs_l3_local_route_setup(int unit)
{
    l3x_base_entry_t b_entry;
    l3x_valid_entry_t v_entry;
    uint32 bmval;
    int local_route_index;

    local_route_index = L3_DEFIP_LOCAL_ROUTE_IDX(unit);

    /* L3 table software */
    l3_table[unit].l3_hash[local_route_index].l3h_ip_addr = LPM_L3_ADDR;
    l3_table[unit].l3_hash[local_route_index].l3h_sip_addr = 0;
    l3_table[unit].l3_hash[local_route_index].l3h_use_count = 1;
    l3_table[unit].l3_hash[local_route_index].l3h_flags = L3H_LPM;
    if (SOC_IS_DRACO1(unit)) {   /* BCM 5690 */
        l3_table[unit].l3_hash[local_route_index].l3h_flags |= L3H_INVALID;
    } else {
#if !defined(L3_DEFIP_LOCALROUTE_USE_IPMC_BIT)
        l3_table[unit].l3_hash[local_route_index].l3h_flags |= L3H_INVALID;
#endif
    }
    l3_table[unit].l3_hash[local_route_index].l3h_vid = 0;
    l3_table[unit].l3_count++;

    /* L3 table hardware */
    sal_memset(&b_entry, 0, sizeof(l3x_base_entry_t));
    soc_L3X_BASEm_field32_set(unit, &b_entry, IP_ADDRf, LPM_L3_ADDR);
    if (!SOC_IS_DRACO1(unit)) {   /* IPMC bit does not work on 5690 */
#ifdef L3_DEFIP_LOCALROUTE_USE_IPMC_BIT
        soc_L3X_BASEm_field32_set(unit, &b_entry, IPMCf, 1);
#endif
    }
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L3X_BASEm, MEM_BLOCK_ALL,
                       local_route_index, &b_entry));

    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, L3X_VALIDm, MEM_BLOCK_ANY,
                      local_route_index >> 3, &v_entry));
    soc_L3X_VALIDm_field_get(unit, &v_entry, BUCKET_BITMAPf, &bmval);

    if (SOC_IS_DRACO1(unit)) {   /* BCM5690 */
        bmval &= ~(1 << (local_route_index & L3_BUCKET_OFFSET_BMP));
    } else {
#ifdef L3_DEFIP_LOCALROUTE_USE_IPMC_BIT
        bmval |= 1 << (local_route_index & L3_BUCKET_OFFSET_BMP);
#else
        bmval &= ~(1 << (local_route_index & L3_BUCKET_OFFSET_BMP));
#endif
    }
    soc_L3X_VALIDm_field_set(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L3X_VALIDm, MEM_BLOCK_ALL,
                       local_route_index >> 3, &v_entry));

    return BCM_E_NONE;
}

STATIC void 
_bcm_draco_l3_route_cb(int unit, soc_switch_event_t  event, uint32 arg1, 
                   uint32 arg2, uint32 arg3, void* userdata)
{

    switch (event) {
        case SOC_SWITCH_EVENT_MODID_CHANGE:
    _bcm_xgs_l3_local_route_setup(unit);
            break;
        default:
            break;
    }

    return;
}

/*
 * Function:
 *      _bcm_draco_l3_dlf_action_set
 * Purpose:
 *      Set L3 miss action to Copy to CPU or blackhole.  
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      action   -  (IN)Action on dlf packets. 
 * Returns:
 *      BCM_X_XXX
 */
int
_bcm_draco_l3_dlf_action_set(int unit, int action)
{
    l3x_base_entry_t b_entry;
    int my_modid;
    int rv; 

    if (!SOC_IS_DRACO15(unit)) {   /* IPMC bit does not work on 5690 */
        return (BCM_E_UNAVAIL);
    }

    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_modid));

    /* Read modify write -> lock the table. */
    MEM_LOCK(unit, L3X_BASEm);

    /* L3 table hardware */
    rv = soc_mem_read(unit, L3X_BASEm, MEM_BLOCK_ANY, 0, &b_entry);
    if (BCM_FAILURE(rv)) {
        MEM_UNLOCK(unit, L3X_BASEm);
        return (rv);
    }

    /* Set destination module id to local modid. */
    soc_L3Xm_field32_set(unit, &b_entry, MODULE_IDf, my_modid);

    /* Blackhole port/ CPU Port. */
    soc_L3X_BASEm_field32_set(unit, &b_entry, TGID_PORTf, (action) ? 0 : \
                              L3_GLOBAL_BLACKHOLE_PORT);
    /* Switch/Do not switch to CPU. */
    soc_L3X_BASEm_field32_set(unit, &b_entry, IPMCf, (action) ? 1 : 0);

    rv = soc_mem_write(unit, L3X_BASEm, MEM_BLOCK_ALL, 0, &b_entry);
    MEM_UNLOCK(unit, L3X_BASEm);

    return (rv);
}

/*
 * Function:
 *      _bcm_draco_l3_dlf_action_get
 * Purpose:
 *      Get L3 miss action Copy to CPU/blackhole.  
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      action   -  (OUT)Action on dlf packets. 
 * Returns:
 *      BCM_X_XXX
 */
int
_bcm_draco_l3_dlf_action_get(int unit, int *action)
{
    l3x_base_entry_t b_entry;

    if (!SOC_IS_DRACO15(unit)) {   /* IPMC bit does not work on 5690 */
        return (BCM_E_UNAVAIL);
    }

    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, L3X_BASEm, MEM_BLOCK_ANY, 0, &b_entry));

    /* Switch/Do not switch to CPU. */
    *action = soc_L3X_BASEm_field32_get(unit, &b_entry, IPMCf);

    return (BCM_E_NONE);
}

/*
 * return the max number of ECMP paths allowed by hardware
 */
STATIC INLINE int
_xgs_l3_hw_ecmp_max(int unit)
{
    switch (SOC_CHIP_GROUP(unit)) {
    case SOC_CHIP_BCM5695:
        return 32;
    case SOC_CHIP_BCM5650:
    case SOC_CHIP_BCM5665:
    case SOC_CHIP_BCM5673:
    case SOC_CHIP_BCM5674:
        return 8;
    default:
        return 0;
    }
}

int
_bcm_xgs_max_ecmp_set(int unit, int max)
{
    if (l3_defip_table[unit].ecmp_inuse) {
        soc_cm_debug(DK_L3, "ECMP already in use, max path can not be reset\n");
        return BCM_E_BUSY;
    }

    if (max < 2 || _xgs_l3_hw_ecmp_max(unit) < max) {
        return BCM_E_PARAM;
    }

    DEFIP_LOCK(unit);
    l3_defip_table[unit].ecmp_max_paths = max;
    DEFIP_UNLOCK(unit);

    return BCM_E_NONE;
}

int
_bcm_xgs_max_ecmp_get(int unit, int *max)
{
    *max = l3_defip_table[unit].ecmp_max_paths;
    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_l3_free_index_get
 * Purpose:
 *	Get the index of a free L3 table entry for DEFIP table add,
 *	Try to return indexes evenly across the table.
 * Parameters:
 *      unit - unit number.
 *      index - (OUT) free index in L3 table
 * Returns:
 *      BCM_E_NONE - success
 *	BCM_E_FULL - table full
 * Note:
 *      Used for Tucana style ECMP route add
 */
STATIC int
_bcm_l3_free_index_get(int unit, int *index)
{
    int total_buckets, used, bucket, free_index, hw_index;
    bcm_ip_t ip_addr;
    int round, i;
    int max_ecmp;

    /* Total number of buckets in the L3 table */
    max_ecmp = l3_defip_table[unit].ecmp_max_paths;
    total_buckets = l3_table[unit].l3_max / L3_BUCKET_SIZE - max_ecmp;

    for (round = 0; round < L3_BUCKET_SIZE; round++) {
        for (bucket = 0; bucket <= total_buckets; bucket += max_ecmp) {
            /*
             * Go through this bucket to see if we have a free entry
             */
            used = 0;
            free_index = INVALID_L3_IDX;
            hw_index = bucket * L3_BUCKET_SIZE;

            for (i = 0; i < L3_BUCKET_SIZE; i++) {
                ip_addr = l3_table[unit].l3_hash[hw_index].l3h_ip_addr;

                if (ip_addr == LPM_L3_ADDR) { /* Entry used by LPM */
                    used++;
                } else if (ip_addr == 0) {   /* Entry free */
                    free_index = hw_index;
                }
                hw_index++;
            }

            if (used > round || free_index == INVALID_L3_IDX) {
                continue;
            } else {
                *index = free_index;
		return BCM_E_NONE;
            }
        }
    }

    return BCM_E_FULL;
}

/*
 * Function:
 *      _bcm_ecmp_free_index_get
 * Purpose:
 *	Get the index of a free ECMP table entry for DEFIP table add
 * Parameters:
 *      unit - unit number.
 *	index - (OUT) index in ECMP table
 * Returns:
 *	BCM_E_NONE - success
 *	BCM_E_FULL - ECMP table full
 * Note:
 *      need improvement
 */
STATIC int
_bcm_ecmp_free_index_get(int unit, int *index)
{
    ecmp_entry_t ecmp_entry;
    int index_min, index_max;
    int max_ecmp, l3_index, i;
    int rv;

    max_ecmp = l3_defip_table[unit].ecmp_max_paths;

    index_min = soc_mem_index_min(unit, L3_ECMPm);
    index_max = soc_mem_index_count(unit, L3_ECMPm) - max_ecmp;

    /*
     * Divide ECMP table into L3_MAX_ECMP (32) entry blocks,
     * return the index at start of block boundary
     */
    for (i = index_min; i <= index_max; i += max_ecmp) {
        rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY, i, &ecmp_entry);
        BCM_IF_ERROR_RETURN(rv);
        l3_index = soc_mem_field32_get(unit, L3_ECMPm, &ecmp_entry, NEXT_HOPf);
        if (l3_index == 0) {
              *index = i;
              return BCM_E_NONE;
        }
    }

    return BCM_E_FULL;
}

/*
 * Function:
 *	_bcm_xgs_l3x_index_write
 * Purpose:
 *	Add entry to L3 hardware table
 * Parameters:
 *	unit - SOC unit number.
 *	hash_index - index
 *	l3x_entry - L3X entry to insert
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_bcm_xgs_l3x_index_write(int unit, int hash_index, l3x_entry_t *l3x_entry)
{
    l3x_base_entry_t b_entry;
    l3x_valid_entry_t v_entry;
    l3x_hit_entry_t h_entry;
    uint32 bmval;

    /*
     * Do L3_BASE table fields
     */
    sal_memcpy(&b_entry, l3x_entry, sizeof(l3x_base_entry_t));

    SOC_IF_ERROR_RETURN
	(soc_mem_write(unit, L3X_BASEm, MEM_BLOCK_ALL,
		       hash_index, &b_entry));

    /* set HIT bit */
    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, L3X_HITm, MEM_BLOCK_ANY, hash_index >> 3, &h_entry));
    soc_L3X_HITm_field_get(unit, &h_entry, BUCKET_BITMAPf, &bmval);
    if (soc_L3Xm_field32_get(unit, l3x_entry, L3_HITf)) {
        bmval |= (1 << (hash_index & L3_BUCKET_OFFSET_BMP));
    } else {
        bmval &= ~(1 << (hash_index & L3_BUCKET_OFFSET_BMP));
    }
    soc_L3X_HITm_field_set(unit, &h_entry, BUCKET_BITMAPf, &bmval);

    SOC_IF_ERROR_RETURN
	(soc_mem_write(unit, L3X_HITm, MEM_BLOCK_ALL,
		       hash_index >> 3, &h_entry));

    /* set valid bit */
    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, L3X_VALIDm, MEM_BLOCK_ANY, hash_index >> 3, &v_entry));
    soc_L3X_VALIDm_field_get(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    if (soc_L3Xm_field32_get(unit, l3x_entry, L3_VALIDf)) {
        bmval |= (1 << (hash_index & L3_BUCKET_OFFSET_BMP));
    } else {
        bmval &= ~(1 << (hash_index & L3_BUCKET_OFFSET_BMP));
    }
    soc_L3X_VALIDm_field_set(unit, &v_entry, BUCKET_BITMAPf, &bmval);

    SOC_IF_ERROR_RETURN
	(soc_mem_write(unit, L3X_VALIDm, MEM_BLOCK_ALL,
		       hash_index >> 3, &v_entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_xgs_l3_add_with_ipmc_set
 * Purpose:
 *	Insert L3 entry to L3 table at some specified index
 * Parameters:
 *	unit     - unit number.
 *	l3cfg    - Pointer to memory for L3 entry related information.
 * Returns:
 *	BCM_E_XXX
 * Note:
 *	This function adds entry for IP UC address, but with IPMC bit set.
 */
STATIC int
_bcm_xgs_l3_add_with_ipmc_set(int unit, _bcm_l3_cfg_t *l3cfg)
{
    l3x_base_entry_t entry;
    l3x_valid_entry_t v_entry;
    uint32 bmval;
    int l3_hw_index;   /* hash table index calculated in SW */

    l3_hw_index = l3cfg->l3c_hw_index;

    sal_memset(&entry, 0, sizeof(l3x_base_entry_t));
    soc_L3X_BASEm_field32_set(unit, &entry, IP_ADDRf, l3cfg->l3c_ip_addr);
    soc_L3X_BASEm_field32_set(unit, &entry, IPMCf, 1);
    SOC_IF_ERROR_RETURN
	(soc_mem_write(unit, L3X_BASEm, MEM_BLOCK_ALL,
		       l3_hw_index, &entry));
    SOC_IF_ERROR_RETURN
	(soc_mem_read(unit, L3X_VALIDm, MEM_BLOCK_ANY,
		      l3_hw_index >> 3, &v_entry));
    soc_L3X_VALIDm_field_get(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    bmval |= 1 << (l3_hw_index & L3_BUCKET_OFFSET_BMP);
    soc_L3X_VALIDm_field_set(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    SOC_IF_ERROR_RETURN
	(soc_mem_write(unit, L3X_VALIDm, MEM_BLOCK_ALL,
		       l3_hw_index >> 3, &v_entry));

    return (BCM_E_NONE);
}

/*
 * Function:
 *	_bcm_xgs_l3hw_insert
 * Purpose:
 *	Add or replace entry in L3 hardware table
 * Parameters:
 *	unit     - unit number.
 *	l3cfg    - Pointer to memory for L3 entry related information.
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_bcm_xgs_l3hw_insert(int unit, _bcm_l3_cfg_t *l3cfg)
{
    l3x_entry_t l3x_entry;
    int ut = 0;

    /*
     * In draco L3 table, if IPMC bit is set for a unicast entry,
     * it means send the original packet to CPU (not L3 switched).
     * BCM_L3_L2TOCPU flag is used to indicate to turn on the
     * IPMC bit for this unicast L3 entry.
     *
     * Note that in this case the hash index is calculated using
     * the 32 bit unicast IP address (as opposed to normal multicast
     * entry where hash index is calculated using 64 bit key).
     * However, if IPMC bit is set, the hardware will calculate
     * hash based on 64 bit key (SIP+DIP), so we have to get
     * table index using 32 bit unicast IP address, then write
     * entry at that index with IPMC bit set, this is what hw_index is for.
     */

    if (!BCM_IP4_MULTICAST(l3cfg->l3c_ip_addr) &&
	l3cfg->l3c_flags & BCM_L3_L2TOCPU) {
        return (_bcm_xgs_l3_add_with_ipmc_set(unit, l3cfg));
    }

    /*
     * Insert a normal unicast/multicast entry to L3 table (hash
     * calculation done in hardware)
     */
    sal_memset(&l3x_entry, 0, sizeof(l3x_entry));
    soc_L3Xm_field32_set(unit, &l3x_entry, IP_ADDRf, l3cfg->l3c_ip_addr);
    if (l3cfg->l3c_flags & BCM_L3_HIT) {
        soc_L3Xm_field32_set(unit, &l3x_entry, L3_HITf, 1);
    }

    if (BCM_IP4_MULTICAST(l3cfg->l3c_ip_addr)) {
        soc_L3Xm_field32_set(unit, &l3x_entry, IPMCf, 1);
        soc_L3Xm_field32_set(unit, &l3x_entry, SRC_IP_ADDRf,
                             l3cfg->l3c_src_ip_addr);
        soc_L3Xm_field32_set(unit, &l3x_entry, IPMC_PTRf, l3cfg->l3c_ipmc_ptr);

        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            soc_L3Xm_field32_set(unit, &l3x_entry, VLAN_IDf, l3cfg->l3c_vid);
        }
    } else {
        soc_L3Xm_mac_addr_set(unit, &l3x_entry, MAC_ADDRf,
			      l3cfg->l3c_mac_addr);
        BCM_L3X_INTF_NUM_SET(unit, L3Xm, &l3x_entry, l3cfg->l3c_intf);

        if (l3cfg->l3c_flags & BCM_L3_TGID) {
	    if (SOC_IS_DRACO15(unit)) {
		soc_L3Xm_field32_set(unit, &l3x_entry, Tf, 1);
		soc_L3Xm_field32_set(unit, &l3x_entry, TGID_LOf,
				     l3cfg->l3c_port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit));
		soc_L3Xm_field32_set(unit, &l3x_entry, TGID_HIf,
				     (l3cfg->l3c_port_tgid &
				      BCM_TGID_PORT_TRUNK_MASK_HI(unit)) >> BCM_TGID_TRUNK_LO_BITS(unit));
	    } else {
		soc_L3Xm_field32_set(unit, &l3x_entry, TGID_PORTf,
				     l3cfg->l3c_port_tgid | BCM_TGID_TRUNK_INDICATOR(unit));
	    }
            _bcm_xgs_get_untag(unit, l3cfg->l3c_intf, 0, 
                               l3cfg->l3c_port_tgid | BCM_TGID_TRUNK_INDICATOR(unit), &ut);
        } else {
	    bcm_module_t	mod_in, mod_out;
	    bcm_port_t		port_in, port_out;

	    mod_in = l3cfg->l3c_modid;
	    port_in = l3cfg->l3c_port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit);
	    BCM_IF_ERROR_RETURN
		(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                        mod_in, port_in,
                                        &mod_out, &port_out));
        if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
            return BCM_E_BADID;
        }
        if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
            return BCM_E_PORT;
        }
	    soc_L3Xm_field32_set(unit, &l3x_entry, MODULE_IDf, mod_out);
	    soc_L3Xm_field32_set(unit, &l3x_entry, TGID_PORTf, port_out);
            _bcm_xgs_get_untag(unit, l3cfg->l3c_intf, mod_out, port_out, &ut);
        }
        soc_L3Xm_field32_set(unit, &l3x_entry, UNTAGf, ut);
    }

    soc_L3Xm_field32_set(unit, &l3x_entry, L3_VALIDf, 1);

#if 0
    
    SOC_IF_ERROR_RETURN
        (soc_l3x_insert(unit, &l3x_entry));
#endif
    BCM_IF_ERROR_RETURN
        (_bcm_xgs_l3x_index_write(unit, l3cfg->l3c_hw_index, &l3x_entry));

    return BCM_E_NONE;
}

/*
 * Add next hop switching information to L3 table
 * with nexthop IP address as the key to the L3 table.
 * Don't LOCK in this routine
 */
STATIC int
_bcm_xgs_l3_lpm_entry_insert(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_l3hash_t hash;
    _bcm_l3_cfg_t l3cfg;
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
    _bcm_defip_cfg_t defip;
    int found;
#endif
    int l3_index;
    int rv;

    if (lpm_cfg->defip_nexthop_ip == 0) {
        soc_cm_debug(DK_L3, "Adding routes requires nexthop IP address\n");
        return BCM_E_PARAM;
    }

#ifdef L3_HOST_TO_LPM_IF_L3_FULL
    /* Is the next hop IP address in the LPM already ? */
    sal_memset(&defip, 0, sizeof(_bcm_defip_cfg_t));
    defip.defip_ip_addr = lpm_cfg->defip_nexthop_ip;
    defip.defip_sub_len = 32;   /* host IP address */
    found = _xgs_route_lookup(unit, &defip);
    if (found) {
        lpm_cfg->defip_l3hw_index = defip.defip_l3hw_index;
        l3_table[unit].l3_hash[defip.defip_l3hw_index].l3h_use_count++;
        return BCM_E_NONE;
    }
#endif

    sal_memset(&hash, 0, sizeof(hash));
    hash.l3h_ip_addr = lpm_cfg->defip_nexthop_ip;
    rv = _bcm_l3sw_hash_insert(unit, L3H_LPM, &hash, &l3_index);

    if (rv == BCM_E_FULL) {
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
        if (lpm_cfg->defip_flags & BCM_L3_HOST_AS_ROUTE) {
            sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
            l3cfg.l3c_ip_addr   = lpm_cfg->defip_nexthop_ip;
            sal_memcpy(&l3cfg.l3c_mac_addr,
                       lpm_cfg->defip_mac_addr, sizeof(sal_mac_addr_t));
            l3cfg.l3c_flags     = lpm_cfg->defip_flags | BCM_L3_HIT;
            l3cfg.l3c_port_tgid = lpm_cfg->defip_port_tgid;
            l3cfg.l3c_modid     = lpm_cfg->defip_modid;
            l3cfg.l3c_intf      = lpm_cfg->defip_intf;

            rv = _bcm_xgs_host_to_lpm_add(unit, &l3cfg);
            if (rv < 0) {
                return rv;
            } else {
                lpm_cfg->defip_l3hw_index = l3cfg.l3c_hw_index;
                return BCM_E_NONE;
            }
        } else {
            return BCM_E_FULL;
        }
#else
        return BCM_E_FULL;
#endif
    } else if (rv < 0) {
        return (rv);
    } else if (hash.l3h_use_count > 1) {
        lpm_cfg->defip_l3hw_index = l3_index;
        return BCM_E_NONE;
    }

    lpm_cfg->defip_l3hw_index = l3_index;

    /* Hardware insert into L3 table */
    l3cfg.l3c_ip_addr   = lpm_cfg->defip_nexthop_ip;
    sal_memcpy(&l3cfg.l3c_mac_addr, lpm_cfg->defip_mac_addr, sizeof(sal_mac_addr_t));
    l3cfg.l3c_flags     = lpm_cfg->defip_flags;
    l3cfg.l3c_port_tgid = lpm_cfg->defip_port_tgid;
    l3cfg.l3c_modid     = lpm_cfg->defip_modid;
    l3cfg.l3c_intf      = lpm_cfg->defip_intf;
    l3cfg.l3c_hw_index  = l3_index;
    rv = _bcm_xgs_l3hw_insert(unit, &l3cfg);
    if (rv < 0) {
        _bcm_l3sw_hash_del(unit, L3H_LPM, &hash, &l3_index);
    }

    return (rv);
}

/*
 * Delete LPM next hop switching information from L3
 * (if the L3 entry count becomes zero)
 */
STATIC int
_bcm_xgs_l3_lpm_entry_del(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    l3x_entry_t entry_l3;
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
    _bcm_l3_cfg_t l3cfg;
#endif
    _bcm_l3hash_t hash;
    int l3_index;
    int rv;

    if (lpm_cfg->defip_nexthop_ip == 0) {
        soc_cm_debug(DK_L3, "Requires nexthop IP address\n");
        return BCM_E_PARAM;
    }

    sal_memset(&hash, 0, sizeof(hash));
    hash.l3h_ip_addr = lpm_cfg->defip_nexthop_ip;

    rv = _bcm_l3sw_hash_del(unit, L3H_LPM, &hash, &l3_index);
    if (rv == BCM_E_NOT_FOUND) {
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
        sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
        l3cfg.l3c_ip_addr   = lpm_cfg->defip_nexthop_ip;
        l3cfg.l3c_intf      = lpm_cfg->defip_intf;
        return (_bcm_xgs_host_to_lpm_del(unit, &l3cfg));
#else
        return BCM_E_NOT_FOUND;
#endif
    } else if (rv < 0) {
        return (rv);
    } else if (hash.l3h_use_count > 0) {
        return BCM_E_NONE;
    }

    /* Hardware L3 delete */
    soc_L3Xm_field32_set(unit, &entry_l3, IP_ADDRf, lpm_cfg->defip_nexthop_ip);
    soc_L3Xm_field32_set(unit, &entry_l3, IPMCf, 0);
    rv = soc_l3x_delete(unit, &entry_l3);

    return (rv);
}

/*
 * Function:
 *	_bcm_xgs_l3hw_index_del
 * Purpose:
 *	Delete L3 entry at index
 * Parameters:
 *	unit - SOC unit number.
 *	index - index
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_bcm_xgs_l3hw_index_del(int unit, int index)
{
    l3x_base_entry_t entry;
    l3x_valid_entry_t v_entry;
    uint32 bmval;

    /* just clear the valid bit */
    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, L3X_VALIDm, MEM_BLOCK_ANY, index >> 3, &v_entry));
    soc_L3X_VALIDm_field_get(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    bmval &= ~(1 << (index & L3_BUCKET_OFFSET_BMP));
    soc_L3X_VALIDm_field_set(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    SOC_IF_ERROR_RETURN
	(soc_mem_write(unit, L3X_VALIDm, MEM_BLOCK_ALL, index >> 3, &v_entry));

    /* Clear table entry.  In fact just clear the Valid bit is enough */
    sal_memset(&entry, 0, sizeof(l3x_base_entry_t));

    SOC_IF_ERROR_RETURN
	(soc_mem_write(unit, L3X_BASEm, MEM_BLOCK_ALL, index, &entry));

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_l3_get_by_index
 * Purpose:
 *	Get L3 entry at index
 * Parameters:
 *	unit - SOC unit number.
 *	index - index
 *	l3cfg - (OUT) decoded L3 entry
 * Returns:
 *	BCM_E_XXX
 * Note:
 *      This function is directly called from ipmc.c
 */
int
bcm_xgs_l3_get_by_index(int unit, int index, _bcm_l3_cfg_t *l3cfg)
{
    _bcm_l3hash_t *hash_entry;
    l3x_entry_t l3x_entry;
    int port_tgid;
    int clear_hit;

    if (index < 0) {
        return (BCM_E_NOT_FOUND);
    }

    hash_entry = &l3_table[unit].l3_hash[index];
    if (hash_entry->l3h_ip_addr == 0) {
        return (BCM_E_PARAM);
    }

    clear_hit = l3cfg->l3c_flags & BCM_L3_HIT_CLEAR;

    sal_memset(&l3x_entry, 0, sizeof(l3x_entry));

    SOC_IF_ERROR_RETURN
        (READ_L3Xm(unit, MEM_BLOCK_ANY, index, &l3x_entry));

    if (!soc_L3Xm_field32_get(unit, &l3x_entry, L3_VALIDf)) {
        return (BCM_E_NOT_FOUND);
    }

    if (soc_L3Xm_field32_get(unit, &l3x_entry, L3_HITf)) {
        l3cfg->l3c_flags |= BCM_L3_HIT;
    }

    l3cfg->l3c_ip_addr = soc_L3Xm_field32_get(unit, &l3x_entry, IP_ADDRf);
    if (BCM_IP4_MULTICAST(l3cfg->l3c_ip_addr)) {
        l3cfg->l3c_src_ip_addr =
	    soc_L3Xm_field32_get(unit, &l3x_entry, SRC_IP_ADDRf);
        l3cfg->l3c_ipmc_ptr =
	    soc_L3Xm_field32_get(unit, &l3x_entry, IPMC_PTRf);

        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            l3cfg->l3c_vid =
                soc_L3Xm_field32_get(unit, &l3x_entry, VLAN_IDf);
        }

        l3cfg->l3c_flags |= BCM_L3_IPMC;
        l3cfg->l3c_intf = 0;
        l3cfg->l3c_port_tgid = 0;
        l3cfg->l3c_modid = 0;
    } else {
        soc_L3Xm_mac_addr_get(unit, &l3x_entry, MAC_ADDRf,
			      l3cfg->l3c_mac_addr);
        l3cfg->l3c_intf = BCM_L3X_INTF_NUM_GET(unit, L3Xm,  &l3x_entry);
        port_tgid = soc_L3Xm_field32_get(unit, &l3x_entry, TGID_PORTf);
        if (port_tgid & BCM_TGID_TRUNK_INDICATOR(unit)) {
            l3cfg->l3c_flags |= BCM_L3_TGID;
            l3cfg->l3c_port_tgid = port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit);
            if (SOC_IS_DRACO15(unit)) {
                port_tgid = soc_L3Xm_field32_get(unit, &l3x_entry, TGID_HIf);
                l3cfg->l3c_port_tgid |= (port_tgid << BCM_TGID_TRUNK_LO_BITS(unit));
            }
        } else {
	    bcm_module_t	mod_in, mod_out;
	    bcm_port_t		port_in, port_out;

	    mod_in = soc_L3Xm_field32_get(unit, &l3x_entry, MODULE_IDf);
	    port_in = port_tgid;
	    BCM_IF_ERROR_RETURN
		(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in,
                                        &mod_out, &port_out));
            l3cfg->l3c_modid = mod_out;
            l3cfg->l3c_port_tgid = port_out;
        }
        if (soc_L3Xm_field32_get(unit, &l3x_entry, IPMCf)) {
            l3cfg->l3c_flags |= BCM_L3_L2TOCPU;
        }
        if (soc_L3Xm_field32_get(unit, &l3x_entry, UNTAGf)) {
            l3cfg->l3c_flags |= BCM_L3_UNTAG;
        }
    }

    /* Clear the HIT bit */
    if (clear_hit && (l3cfg->l3c_flags & BCM_L3_HIT)) {
        soc_L3Xm_field32_set(unit, &l3x_entry, L3_HITf, 0);
        BCM_IF_ERROR_RETURN
            (_bcm_xgs_l3x_index_write(unit, index, &l3x_entry));
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs_l3_del_by_index
 * Purpose:
 *      Delete L3 entry at index for IPMC delete
 * Parameters:
 *      unit - SOC unit number.
 *      index - index
 * Returns:
 *      BCM_E_XXX
 * Note:
 *     This is used by IPMC only
 */
int
bcm_xgs_l3_del_by_index(int unit, int index)
{
    int l3_use_cnt;
    int rv;

    L3_LOCK(unit);
    _bcm_l3sw_index_del(unit, L3H_HOST, index, &l3_use_cnt);
    if (l3_use_cnt == 0) {
        rv = _bcm_xgs_l3hw_index_del(unit, index);
        if (rv < 0) {
            L3_UNLOCK(unit);
            return rv;
        }
    }
    L3_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_xgs_l3_enable
 * Purpose:
 *      Enable/disable L3 function.
 * Parameters:
 *      unit   - unit number
 *      enable - TRUE: enable L3 support; FALSE: disable L3 support.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_enable(int unit, int enable)
{
    bcm_port_t port;

    enable = enable ? 1 : 0;

    PBMP_E_ITER(unit, port) {
        bcm_esw_port_l3_enable_set(unit, port, enable);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_xgs_l3_intf_hw_init
 * Purpose:
 *      Initialize L3 interface table
 * Parameters:
 *      unit - SOC unit number.
 *	entries - (OUT) Number of interface entries.
 * Returns:
 *      BCM_E_NONE - success
 *      BCM_E_XXX - other
 * Note:
 *      L3 interface lock inside the function
 */
STATIC int
_bcm_xgs_l3_intf_hw_init(int unit, int *entries)
{
    int index_min, index_max, index;
    intf_entry_t entry_l3intf;
    bcm_mac_t mac;
    bcm_vlan_t vid;

    /* Clear L3 interface TABLE */
    index_min = soc_mem_index_min(unit, L3_INTFm);
    index_max = soc_mem_index_max(unit, L3_INTFm);

    INTF_LOCK(unit);

    if (L3_INFO(unit)->l3_initialized && L3_INFO(unit)->l3intf_count != 0) {
        for (index = index_min; index <= index_max; index++) {
            if (BCM_L3_INTF_USED_GET(unit, index)) {
                if (BCM_L3_INTF_ARL_GET(unit, index)) {
                    SOC_IF_ERROR_RETURN
                        (READ_L3_INTFm(unit, MEM_BLOCK_ANY, index, &entry_l3intf));
                    soc_L3_INTFm_mac_addr_get(unit, &entry_l3intf,
                                              ROUTER_ADDRf, mac);
                    vid = soc_L3_INTFm_field32_get(unit, &entry_l3intf, VLAN_IDf);
                    (void)bcm_esw_l2_addr_delete(unit, mac, vid);
                    BCM_L3_INTF_ARL_CLR(unit, index);
                }

                sal_memset(&entry_l3intf, 0, sizeof(entry_l3intf));
                SOC_IF_ERROR_RETURN
	            (WRITE_L3_INTFm(unit, MEM_BLOCK_ALL, index, &entry_l3intf));
                BCM_L3_INTF_USED_CLR(unit, index);
            }
        }
    }

    if (!SAL_BOOT_SIMULATION){
        for (index = index_min; index <= index_max; index++) {
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, L3_INTFm, MEM_BLOCK_ALL, index,
                               soc_mem_entry_null(unit, L3_INTFm)));
        }
    }

    INTF_UNLOCK(unit);

    *entries = index_max - index_min + 1;

    return BCM_E_NONE;
}

/*
 * Del global default route (Draco 1.5 only)
 * We used to set index 0 of L3 table to be valid, and
 * with IPMC bit set so packet go to CPU unrouted.  But this
 * has the unwanted effect of causing packets to skip FFP,
 * so right now, we mark index 0 of hardware L3 entry as "not valid",
 * but still reserve this entry in software so it won't be used.
 */
STATIC int
_bcm_xgs_global_def_route_del(int unit)
{
    l3x_base_entry_t b_entry;
    l3x_valid_entry_t v_entry;
    uint32 bmval;
    int rv;

    sal_memset(&b_entry, 0, sizeof(l3x_base_entry_t));
    soc_L3Xm_field32_set(unit, &b_entry, IP_ADDRf, INVALID_IP_ADDR);
#ifdef L3_GLOBAL_DEF_ROUTE_USE_IPMC_BIT
    /* Avoid packet being routed by setting the IPMC bit */
    soc_L3X_BASEm_field32_set(unit, &b_entry, IPMCf, 1);
#endif
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L3X_BASEm, MEM_BLOCK_ALL,
                       L3_D15_GLOBAL_DEF_ROUTE_IDX, &b_entry));

    rv =  soc_mem_read(unit, L3X_VALIDm, MEM_BLOCK_ANY, 0, &v_entry);
    BCM_IF_ERROR_RETURN(rv);
    soc_L3X_VALIDm_field_get(unit, &v_entry, BUCKET_BITMAPf, &bmval);
#ifdef L3_GLOBAL_DEF_ROUTE_USE_IPMC_BIT
    bmval |= 1;
#else
    bmval &= ~1;
#endif
    soc_L3X_VALIDm_field_set(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L3X_VALIDm, MEM_BLOCK_ALL, 0, &v_entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_xgs_l3_hw_table_init
 * Purpose:
 *      Initialize HW L3 tables, and allocate SW shadow table copies.
 * Parameters:
 *      unit - SOC unit number.
 *	entries - (OUT) Number of L3 entries.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs_l3_hw_table_init(int unit, int *entries)
{
    int  index_min, index_max;

    index_min = soc_mem_index_min(unit, L3X_BASEm);
    index_max = soc_mem_index_max(unit, L3X_BASEm);

    /*
     * Clear L3 TABLE, L3 table has been cleared by arl_init bit
     * in arl control register.  Still need to do this because of L3 detach
     */
    if (!SAL_BOOT_SIMULATION){
        SOC_IF_ERROR_RETURN
            (soc_mem_clear(unit, L3Xm, MEM_BLOCK_ALL, TRUE));
    }

    /*
     * Since DEFAULT_ROUTE field in VLAN table is initialized to 0,
     * we must not use the first L3 entry for BCM5695 (per VLAN def route)
     * Write a bogus entry with IP address 0xffffffff to CPU,
     * with IPMC bit set so packet go to CPU unrouted.
     */
    if (SOC_IS_DRACO15(unit)) {  /* Draco 1.5 */
#ifdef L3_SET_ARL_CTRL_EXCL_IPMC_BIT
        /*
         * Set ARL_CONTROL.EXCL_IPMC_BIT so when packet is sent to
         * CPU, so that the OPCODE would be L3_DST_MISS(?)
         */
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, ARL_CONTROLr, REG_PORT_ANY,
                                    EXCL_IPMC_BITf, 1));
#endif
        BCM_IF_ERROR_RETURN(_bcm_xgs_global_def_route_del(unit));
    }

    l3_table[unit].l3_count = 0;
    l3_table[unit].l3_min = index_min;
    l3_table[unit].l3_max = index_max + 1;

    *entries = index_max - index_min + 1;

    return BCM_E_NONE;
}

/*
 * get the number of blocks in LPM table
 */
STATIC INLINE int
_bcm_xgs_lpm_block_count(int unit)
{
    /* first block always used */
    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        /* first block has 256 entries */
        return ((l3_defip_table[unit].lpm_max -
                  l3_defip_table[unit].lpm_min + 1 -
                  LPM_LONG_BLK_SIZE) / (LPM_BLK_SIZE));
    } else {
        return (((l3_defip_table[unit].lpm_max -
                  l3_defip_table[unit].lpm_min + 1) / LPM_BLK_SIZE) - 1);
    }
}

/*
 * the lowest network prefix index for this block
 */
STATIC int
_xgs_lpm_prefix_range(int unit, int blk_index)
{
    _bcm_lpm_entry_t *lpmp;
    int prefix_lo;

    if (blk_index == 0) {
        return 0;
    }

    lpmp = L3_LPM_ENTRY(unit, blk_index);

    prefix_lo = (SOC_IS_DRACO15(unit) ? 5 : 1);
    while (lpmp->parent != INVALID_LPM_IDX) {
        prefix_lo += LPM_BITS_PER_BLK;
        lpmp = L3_LPM_ENTRY(unit, lpmp->parent);
    }

    return prefix_lo;
}

/*
 * Free all memory for backup routes
 */
STATIC void
_bcm_lpm_release_backup(int unit)
{
    _bcm_backup_route_t *prev, *cur, *next;
    _bcm_lpm_entry_t *lpmp;
    int prefix_lo;
    int blk_size, blk, i;

    /*
     * Go through the LPM table once to cut off all non-locally
     * created backup routes
     */
    for (blk = l3_defip_table[unit].lpm_min;
         blk < l3_defip_table[unit].lpm_max; blk += blk_size) {
        lpmp = L3_LPM_ENTRY(unit, blk);

        if (IS_LPM_LONG_BLOCK(unit, blk)) {
            blk_size = LPM_LONG_BLK_SIZE; /* first block in BCM5695 */
        } else {
            blk_size = LPM_BLK_SIZE;
        }

        prefix_lo = _xgs_lpm_prefix_range(unit, blk);

        for (i = 0; i < blk_size; i++) {
            if (lpmp[i].valid) {
                prev = NULL;
                for (cur = lpmp[i].backup_routes; cur; cur = cur->next) {
                    if (cur->subnet_len < prefix_lo) {
                        break;
                    }
                    prev = cur;
                }

                if (prev) {
                    prev->next = NULL;
                } else {
                    lpmp[i].backup_routes = NULL;
                }
            }
        }
    }

    /* Go through LPM once more to free memory for locally created backup */
    for (i = l3_defip_table[unit].lpm_min;
         i < l3_defip_table[unit].lpm_max; i++) {
        lpmp = L3_LPM_ENTRY(unit, i);
        if (lpmp->valid) {
            cur = lpmp->backup_routes;
            while (cur) {
                next = cur->next;
                _print_net_addr(DK_L3, "Free backup ",
                                cur->ip_addr, cur->subnet_len, "\n");
                sal_free(cur);
                cur = next;
            }
        }
    }
}

/*
 * Function:
 *	_bcm_xgs_lpm_sw_table_init
 * Purpose:
 *      Initialize the s/w copy of the LPM table
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *	Nothing
 */
STATIC void
_bcm_xgs_lpm_sw_table_init(int unit, int first_time)
{
    int i;

    if (!first_time) {
        _bcm_lpm_release_backup(unit);
    }

    for (i = l3_defip_table[unit].lpm_min;
         i <= l3_defip_table[unit].lpm_max; i++) {
        l3_defip_table[unit].l3_lpm[i].forward = INVALID_LPM_IDX;
        l3_defip_table[unit].l3_lpm[i].valid = 0;
        l3_defip_table[unit].l3_lpm[i].final = 0;
        l3_defip_table[unit].l3_lpm[i].l3_index = INVALID_L3_IDX;
        l3_defip_table[unit].l3_lpm[i].l3_intf = 0;
        l3_defip_table[unit].l3_lpm[i].ip_addr = INVALID_IP_ADDR;
        l3_defip_table[unit].l3_lpm[i].subnet_len = INVALID_PREFIX_LEN;
        l3_defip_table[unit].l3_lpm[i].dirty = 0;
        l3_defip_table[unit].l3_lpm[i].parent = INVALID_LPM_IDX;
        l3_defip_table[unit].l3_lpm[i].backup_routes = NULL;
        l3_defip_table[unit].l3_lpm[i].ecmp = 0;
        l3_defip_table[unit].l3_lpm[i].ecmp_count = 0;
        l3_defip_table[unit].l3_lpm[i].ecmp_index = -1;
    }

    /*
     * Default route is the last one in LPM table for DRACO/TUCANA/LYNX.
     * On DRACO1.5 there is a default route per VLAN
     */
    l3_defip_table[unit].def_route_index = l3_defip_table[unit].lpm_max;
    l3_defip_table[unit].pv_def_route = NULL;  /* BCM5695 only */
    l3_defip_table[unit].global_def_route_set = 0;
    l3_defip_table[unit].lpm_based_def_route_set = 0;
}

/*
 * Function:
 *	_bcm_xgs_lpm_hw_table_init
 * Purpose:
 *      Initialize HW LPM table, and allocate SW shadow table copies.
 * Parameters:
 *      unit - SOC unit number.
 *	entries - (OUT) Number of DEFIP entries.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs_lpm_hw_table_init(int unit, int *entries)
{
    int min_index, max_index;
    int hit_bucket_len = 0, i;
    defip_hi_entry_t defip_entry;
    defip_hit_hi_entry_t defip_hit_entry;

    if (SOC_IS_DRACO(unit) || SOC_IS_TUCANA(unit)) {
        LPM_MEM(unit) = DEFIP_HIm;
        LPMHIT_MEM(unit) = DEFIP_HIT_HIm;
    } else {                                        /* LYNX */
        LPM_MEM(unit) = DEFIPm;
        LPMHIT_MEM(unit) = DEFIP_HITm;
    }

    min_index = soc_mem_index_min(unit, LPM_MEM(unit));
    max_index = soc_mem_index_max(unit, LPM_MEM(unit));

    /*
     * intialize the H/W LPM tables
     */
    if (!SAL_BOOT_SIMULATION){
        sal_memset(&defip_entry, 0, sizeof(defip_hi_entry_t));
        for (i = min_index; i <= max_index; i++) {
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, LPM_MEM(unit), COPYNO_ALL, i, 
                               &defip_entry));

            if (SOC_IS_DRACO1(unit) || SOC_IS_TUCANA(unit)) {
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, DEFIP_LOm, COPYNO_ALL, i,
                                   &defip_entry));
            }
        }

        /*
         * Clear the hit bits by clearing DEFIP_HIT_HIm/DEFIP_HIT_LOm tables
         */
        hit_bucket_len = soc_mem_field_length(unit, LPMHIT_MEM(unit),
                                              BUCKET_BITMAPf);
        sal_memset(&defip_hit_entry, 0, sizeof(defip_hit_hi_entry_t));

        for (i = min_index; i < max_index/hit_bucket_len; i++) {
            SOC_IF_ERROR_RETURN(soc_mem_write(unit, LPMHIT_MEM(unit),
                                        COPYNO_ALL, i, &defip_hit_entry));
            if (SOC_IS_DRACO(unit) || SOC_IS_TUCANA(unit)) {
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, DEFIP_HIT_LOm, COPYNO_ALL,
                                   i, &defip_hit_entry));
            }
        }
    }

    l3_defip_table[unit].lpm_min = min_index;
    l3_defip_table[unit].lpm_max = max_index;
    l3_defip_table[unit].update_head = NULL;
    l3_defip_table[unit].update_tail = NULL;

    l3_defip_table[unit].lpm_block_max  = _bcm_xgs_lpm_block_count(unit);
    l3_defip_table[unit].lpm_block_used = 0;
    l3_defip_table[unit].lpm_entry_used = 0;

    l3_defip_table[unit].ecmp_max_paths = _xgs_l3_hw_ecmp_max(unit);
    l3_defip_table[unit].ecmp_inuse = 0;

    *entries = max_index - min_index + 1;

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_ecmp_hw_table_init
 * Purpose:
 *      Initialize HW ECMP table
 * Parameters:
 *      unit - SOC unit number.
 *	entries - (OUT) Number of ECMP table entries.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_ecmp_hw_table_init(int unit, int *entries)
{
    int index_min, index_max, index;

    /* Clear L3 interface TABLE */
    index_min = soc_mem_index_min(unit, L3_ECMPm);
    index_max = soc_mem_index_max(unit, L3_ECMPm);

    for (index = index_min; index <= index_max; index++) {
        SOC_IF_ERROR_RETURN
            (soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL, index,
                           soc_mem_entry_null(unit, L3_ECMPm)));
    }

    *entries = index_max;

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_xgs_l3_free_resource
 * Purpose:
 *	Free all allocated tables and memory
 * Parameters:
 *	unit - SOC unit number
 * Returns:
 *	Nothing
 */
STATIC void
_bcm_xgs_l3_free_resource(int unit)
{
    if (L3_INFO(unit)->l3_intf_used) {
        sal_free(L3_INFO(unit)->l3_intf_used);
        L3_INFO(unit)->l3_intf_used = NULL;
    }

    if (L3_INFO(unit)->add_to_arl) {
        sal_free(L3_INFO(unit)->add_to_arl);
        L3_INFO(unit)->add_to_arl = NULL;
    }

    if (l3_table[unit].l3_hash) {
        sal_free(l3_table[unit].l3_hash);
        l3_table[unit].l3_hash = NULL;
    }

    if (l3_defip_table[unit].l3_lpm) {
        /* Free memory for backup routes */
        _bcm_lpm_release_backup(unit);

        sal_free(l3_defip_table[unit].l3_lpm);
        l3_defip_table[unit].l3_lpm = NULL;
    }

    if (l3_defip_table[unit].lpm_blk_used) {
        sal_free(l3_defip_table[unit].lpm_blk_used);
        l3_defip_table[unit].lpm_blk_used = NULL;
    }
}

/*
 * Function:
 *	bcm_xgs_l3_tables_init
 * Purpose:
 *      Initialize all internal L3 tables, and enable L3 on the chip.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_tables_init(unit)
{
    _bcm_l3_bookkeeping_t *l3;
    int num_blk;

    l3 = L3_INFO(unit);

    /*
     * clear all hardware tables
     */
    BCM_IF_ERROR_RETURN
	(_bcm_xgs_l3_intf_hw_init(unit, &l3->l3_intf_table_size));

    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
	BCM_IF_ERROR_RETURN
	    (_bcm_ecmp_hw_table_init(unit, &l3->ecmp_table_size));
    }

    BCM_IF_ERROR_RETURN
        (_bcm_xgs_l3_hw_table_init(unit, &l3->l3_table_size));

    BCM_IF_ERROR_RETURN
        (_bcm_xgs_lpm_hw_table_init(unit, &l3->defip_table_size));

    num_blk = l3->defip_table_size / LPM_BLK_SIZE;

    /*
     * allocate resources
     */
    if (!l3->l3_initialized) {
        /*
         * Allocate SW shadow hashed copy of L3 table
         */
        l3_table[unit].l3_hash =
            sal_alloc(l3->l3_table_size * sizeof(_bcm_l3hash_t), "l3_l3");
        if (l3_table[unit].l3_hash == NULL) {
            _bcm_xgs_l3_free_resource(unit);
            return (BCM_E_MEMORY);
        }
        sal_memset(l3_table[unit].l3_hash, 0,
                   l3->l3_table_size * sizeof(_bcm_l3hash_t));

        /*
         * allocate s/w copy of LPM table
         */
        l3_defip_table[unit].l3_lpm = (_bcm_lpm_entry_t *)
            sal_alloc(l3->defip_table_size * sizeof(_bcm_lpm_entry_t), "l3_lpm");
        if (l3_defip_table[unit].l3_lpm == NULL) {
            _bcm_xgs_l3_free_resource(unit);
            return (BCM_E_MEMORY);
        }
        _bcm_xgs_lpm_sw_table_init(unit, 1);

        l3_defip_table[unit].lpm_blk_used =
            sal_alloc(SHR_BITALLOCSIZE(num_blk), "l3_lpm");
        if (l3_defip_table[unit].lpm_blk_used == NULL) {
            _bcm_xgs_l3_free_resource(unit);
            return (BCM_E_MEMORY);
        }
        sal_memset(l3_defip_table[unit].lpm_blk_used, 0,
                   SHR_BITALLOCSIZE(num_blk));

        /*
         * keep track of L3 interfaces
         */
        l3->l3_intf_used =
            sal_alloc(SHR_BITALLOCSIZE(l3->l3_intf_table_size), "l3_intf");
        l3->add_to_arl =
            sal_alloc(SHR_BITALLOCSIZE(l3->l3_intf_table_size), "l3_intf");
        if (l3->l3_intf_used == NULL || l3->add_to_arl == NULL) {
            _bcm_xgs_l3_free_resource(unit);
            return (BCM_E_MEMORY);
        }
        sal_memset(l3->l3_intf_used, 0,
                   SHR_BITALLOCSIZE(l3->l3_intf_table_size));
        sal_memset(l3->add_to_arl, 0,
                   SHR_BITALLOCSIZE(l3->l3_intf_table_size));
    } else {
        sal_memset(l3_table[unit].l3_hash, 0,
                   l3->l3_table_size * sizeof(_bcm_l3hash_t));
        _bcm_xgs_lpm_sw_table_init(unit, 0);

        sal_memset(l3_defip_table[unit].lpm_blk_used, 0,
                   SHR_BITALLOCSIZE(num_blk));

        sal_memset(l3->l3_intf_used, 0,
                   SHR_BITALLOCSIZE(l3->l3_intf_table_size));
        sal_memset(l3->add_to_arl, 0,
                   SHR_BITALLOCSIZE(l3->l3_intf_table_size));
    }

    /*
     * Since DEFAULT_ROUTE field in VLAN table is initialized to 0,
     * we must not use the first L3 entry for BCM5695 (per VLAN def route)
     */
    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        _bcm_l3sw_add_lpm_entry(unit, L3_D15_GLOBAL_DEF_ROUTE_IDX, 0);
    }

    /*
     * Reserve the last entry in L3 table for use by local routes
     */
    _bcm_xgs_l3_local_route_setup(unit);

    soc_event_register(unit, _bcm_draco_l3_route_cb, NULL);
    
    BCM_IF_ERROR_RETURN
	(bcm_xgs_l3_enable(unit, TRUE));

    l3->l3intf_count = 0;
    l3->l3_initialized = TRUE;

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_l3_tables_cleanup
 * Purpose:
 *      Disable L3 function, cleanup internal L3 tables, free resources.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_xgs_l3_tables_cleanup(int unit)
{
    if (L3_INFO(unit)->l3_initialized) {
	int entries;

        /*
         * clear all hardware tables
         */
        BCM_IF_ERROR_RETURN
	    (_bcm_xgs_l3_intf_hw_init(unit, &entries));

        if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
            _bcm_ecmp_hw_table_init(unit, &entries);
        }

        _bcm_xgs_l3_hw_table_init(unit, &entries);
        _bcm_xgs_lpm_hw_table_init(unit, &entries);
        bcm_xgs_l3_enable(unit, FALSE);

        L3_INFO(unit)->l3intf_count = 0;

        _bcm_xgs_l3_free_resource(unit);

        soc_event_unregister(unit, _bcm_draco_l3_route_cb, NULL);

        L3_INFO(unit)->l3_initialized = FALSE;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs_l3_conflict_get
 * Purpose:
 *      Given a IP address, return conflicts in the L3 table.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipkey    - IP address to test conflict condition
 *      cf_array - (OUT) arrary of conflicting addresses(at most 8)
 *      cf_max   - max number of conflicts wanted
 *      count    - (OUT) actual # of conflicting addresses
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_conflict_get(int unit, bcm_l3_key_t *ipkey, bcm_l3_key_t *cf_array,
                    int cf_max, int *cf_count)
{
    _bcm_l3hash_t *hash_entry;
    int no_sip, no_vid;
    _bcm_l3hash_t hash;
    int hw_index, i;
    uint32 bucket;

    hash.l3h_ip_addr  = ipkey->l3k_ip_addr;
    hash.l3h_sip_addr = ipkey->l3k_sip_addr;
    hash.l3h_vid      = ipkey->l3k_vid;

    _bcm_l3_calc_hash_bucket(unit, &hash, &bucket, &no_sip, &no_vid);

    /*
     * Loop through all L3_BUCKET_SIZE entries in this bucket
     */
    *cf_count = 0;
    hw_index = bucket * L3_BUCKET_SIZE;
    for (i = 0; i < L3_BUCKET_SIZE && *cf_count < cf_max; i++) {
        hash_entry = &l3_table[unit].l3_hash[hw_index];

        if (hash_entry->l3h_ip_addr != 0) {
            cf_array[*cf_count].l3k_ip_addr  = hash_entry->l3h_ip_addr;
            cf_array[*cf_count].l3k_sip_addr = hash_entry->l3h_sip_addr;
            cf_array[*cf_count].l3k_vid      = hash_entry->l3h_vid;
            cf_array[*cf_count].l3k_flags    = 0;
            *cf_count += 1;
        }

        hw_index++;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs_l3_invalidate_entry
 * Purpose:
 *      Given a IP address, invalidate the L3 entry without
 *      clearing the entry information, so that the entry can be
 *      turned back to valid without resetting all the information.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipaddr   - IP address to test conflict condition
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_invalidate_entry(int unit, bcm_ip_t ipaddr)
{
    _bcm_l3hash_t     hash;
    l3x_valid_entry_t v_entry;
    uint32            bmval;
    int               hw_index, rv;

    sal_memset(&hash, 0, sizeof(hash));
    hash.l3h_ip_addr = ipaddr;
    L3_LOCK(unit);
    rv = _bcm_l3hash_index_get(unit, &hash, &hw_index);
    L3_UNLOCK(unit);

    BCM_IF_ERROR_RETURN(rv);

    l3_table[unit].l3_hash[hw_index].l3h_flags |= L3H_INVALID;

    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, L3X_VALIDm, MEM_BLOCK_ANY, hw_index >> 3, &v_entry));
    soc_L3X_VALIDm_field_get(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    bmval &= ~(1 << (hw_index & L3_BUCKET_OFFSET_BMP));
    soc_L3X_VALIDm_field_set(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L3X_VALIDm, MEM_BLOCK_ALL, hw_index >> 3, &v_entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_xgs_l3_validate_entry
 * Purpose:
 *      Given a IP address, validate the L3 entry without
 *      resetting the entry information.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipaddr   - IP address to test conflict condition
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_validate_entry(int unit, bcm_ip_t ipaddr)
{
    _bcm_l3hash_t     hash;
    l3x_valid_entry_t v_entry;
    uint32            bmval;
    int               hw_index, rv;

    sal_memset(&hash, 0, sizeof(hash));
    hash.l3h_ip_addr = ipaddr;
    L3_LOCK(unit);
    rv = _bcm_l3hash_index_get(unit, &hash, &hw_index);
    L3_UNLOCK(unit);

    BCM_IF_ERROR_RETURN(rv);

    l3_table[unit].l3_hash[hw_index].l3h_flags &= ~L3H_INVALID;

    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, L3X_VALIDm, MEM_BLOCK_ANY, hw_index >> 3, &v_entry));
    soc_L3X_VALIDm_field_get(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    bmval |= (1 << (hw_index & L3_BUCKET_OFFSET_BMP));
    soc_L3X_VALIDm_field_set(unit, &v_entry, BUCKET_BITMAPf, &bmval);
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L3X_VALIDm, MEM_BLOCK_ALL, hw_index >> 3, &v_entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_xgs_l3_age
 * Purpose:
 *      Age out the L3 entry based on L3SH, L3DH or both.
 * Parameters:
 *      unit - SOC PCI device unit number (driver internal).
 *   	flags - The criteria used to age out L3 table.
 *      age_out - Call back routine.
 *      user_data - User callback cookie.  
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_xgs_l3_age(int unit, uint32 flags, bcm_l3_host_traverse_cb age_out, 
               void *user_data)
{
    _bcm_l3_cfg_t cfg;
    _bcm_l3hash_t *hash_entry;
    l3x_hit_entry_t h_entry;
    bcm_l3_host_t info;
    uint32 h_val;
    uint32 i;
    int rv = 0;


    for (i = l3_table[unit].l3_min; i < L3_DEFIP_LOCAL_ROUTE_IDX(unit); i++) {
        hash_entry = &l3_table[unit].l3_hash[i];

        if (hash_entry->l3h_ip_addr == 0) {
            continue;
        }

        L3_LOCK(unit);
        sal_memset(&cfg, 0, sizeof(cfg));
        rv = bcm_xgs_l3_get_by_index(unit, i, &cfg);
        if (rv < 0) {
            L3_UNLOCK(unit);
            break;
        }

        assert(cfg.l3c_ip_addr == hash_entry->l3h_ip_addr);

        if ((flags & BCM_L3_HIT) && (cfg.l3c_flags & BCM_L3_HIT)) {
            /* clear HIT bit */
            rv = soc_mem_read(unit, L3X_HITm, MEM_BLOCK_ANY, i >> 3, &h_entry);
            if (rv < 0) {
                L3_UNLOCK(unit);
                return rv;
            }
            soc_L3X_HITm_field_get(unit, &h_entry, BUCKET_BITMAPf, &h_val);
            h_val &= ~(1 << (i & L3_BUCKET_OFFSET_BMP));
            soc_L3X_HITm_field_set(unit, &h_entry, BUCKET_BITMAPf, &h_val);
            rv = soc_mem_write(unit, L3X_HITm, MEM_BLOCK_ALL, i >> 3, &h_entry);
        } else if (!BCM_IP4_MULTICAST(hash_entry->l3h_ip_addr) && /* IPMC */
                   !(hash_entry->l3h_flags & L3H_LPM)) { /* LPM */
            /* age out */
            hash_entry->l3h_ip_addr = 0;
            l3_table[unit].l3_count--;

            rv = _bcm_xgs_l3hw_index_del(unit, i);
	    if (rv < 0) {
	        L3_UNLOCK(unit);
	        break;
	    }

            if (age_out != NULL) {
               bcm_l3_host_t_init(&info); 
               info.l3a_ip_addr = cfg.l3c_ip_addr;
               info.l3a_flags = 0;   /* IPv4 */
               rv = (*age_out)(unit, i, &info, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
               if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                   break;
               }
#endif
            }
        }
        L3_UNLOCK(unit);
    }

    return (rv);
}

/*
 * Function:
 *	bcm_xgs_l3_traverse
 * Purpose:
 *      Go through L3 table and invoke call abck at each valid entry
 * Parameters:
 *      unit  - SOC PCI device unit number (driver internal).
 *      start - starting L3 entry
 *      end   - ending L3 entry
 *	cb    - the call back function pointer
 *      user_data - the cookie to be used in callback function
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_xgs_l3_traverse(int unit, uint32 start, uint32 end,
                      bcm_l3_host_traverse_cb cb, void *user_data)
{
    bcm_l3_host_t info;
    _bcm_l3_cfg_t cfg;
    _bcm_l3hash_t *hash_entry;
    uint32 i, total = 0;
    int rv = 0;

    if (start >= l3_table[unit].l3_max || end > l3_table[unit].l3_max ||
        start > end) {
        return (BCM_E_PARAM);
    }

    for (i = l3_table[unit].l3_min; i < L3_DEFIP_LOCAL_ROUTE_IDX(unit); i++) {
        hash_entry = &l3_table[unit].l3_hash[i];
        if (hash_entry->l3h_ip_addr == 0) {          /* NULL */
            continue;
        }

        total++;
        if (total < start) {
            continue;
        }
 
        if (total > end) {
            return BCM_E_NONE;
        }

        L3_LOCK(unit);
        sal_memset(&cfg, 0, sizeof(cfg));
        rv = bcm_xgs_l3_get_by_index(unit, i, &cfg);
        if (rv < 0) {
            L3_UNLOCK(unit);
            break;
        }

        if (cb != NULL) {
            bcm_l3_host_t_init(&info);
            info.l3a_flags = cfg.l3c_flags;
            if (cfg.l3c_flags & BCM_L3_IPMC) {
                info.l3a_ipmc_ptr = cfg.l3c_ipmc_ptr;
            }
            sal_memcpy(info.l3a_nexthop_mac, cfg.l3c_mac_addr,
                       sizeof(sal_mac_addr_t));
            info.l3a_intf = cfg.l3c_intf;
            info.l3a_port_tgid = cfg.l3c_port_tgid;
            info.l3a_modid = cfg.l3c_modid;
            info.l3a_ip_addr = cfg.l3c_ip_addr;
            rv = (*cb)(unit, total, &info, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
            if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                break;
            }
#endif
        }
        L3_UNLOCK(unit);
    }

    return (rv);
}

/*
 * Function:
 *	bcm_xgs_l3_info
 * Purpose:
 *      Get the status of hardware.
 * Parameters:
 *      unit - SOC PCI device unit number (driver internal).
 *      l3info - (OUT) Point to area which will contain L3 related information.
 * Returns:
 *	BCM_E_XXX.
 */
int
bcm_xgs_l3_info(int unit, bcm_l3_info_t *l3info)
{
    l3info->l3info_max_vrf = 1;
    l3info->l3info_used_vrf = 1;
    l3info->l3info_max_ecmp = l3_defip_table[unit].ecmp_max_paths;

    l3info->l3info_max_intf       = L3_INFO(unit)->l3_intf_table_size;
    l3info->l3info_max_intf_group = 0;
    l3info->l3info_max_host       = L3_INFO(unit)->l3_table_size;
    l3info->l3info_max_route      = L3_INFO(unit)->defip_table_size;
    l3info->l3info_occupied_intf  = L3_INFO(unit)->l3intf_count;
    l3info->l3info_occupied_host  = l3_table[unit].l3_count;

    l3info->l3info_occupied_route = l3_defip_table[unit].lpm_entry_used;
    l3info->l3info_max_lpm_block  = l3_defip_table[unit].lpm_block_max;
    l3info->l3info_used_lpm_block = l3_defip_table[unit].lpm_block_used;

    /* Superseded values for backward compatibility */

    l3info->l3info_max_l3         = l3info->l3info_max_host;
    l3info->l3info_max_defip      = l3info->l3info_max_route;
    l3info->l3info_occupied_l3    = l3info->l3info_occupied_host;
    l3info->l3info_occupied_defip = l3info->l3info_occupied_route;

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_l3_intf_get
 * Purpose:
 *      Get an entry from L3 interface table.
 * Parameters:
 *      unit - SOC unit number.
 *	intf_info - Pointer to memory for interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_intf_get(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    intf_entry_t entry_l3intf;

    if (intf_info->l3i_index >= L3_INFO(unit)->l3_intf_table_size ||
        !BCM_L3_INTF_USED_GET(unit, intf_info->l3i_index)) {
        return (BCM_E_NOT_FOUND);
    }

    SOC_IF_ERROR_RETURN
	(READ_L3_INTFm(unit, MEM_BLOCK_ANY,
		       intf_info->l3i_index, &entry_l3intf));

    soc_L3_INTFm_mac_addr_get(unit, &entry_l3intf, ROUTER_ADDRf,
                              intf_info->l3i_mac_addr);
    intf_info->l3i_vid =
	soc_L3_INTFm_field32_get(unit, &entry_l3intf, VLAN_IDf);

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_l3_intf_get_by_vid
 * Purpose:
 *      Get an entry from L3 interface table.
 * Parameters:
 *      unit - SOC unit number.
 *	intf_info - Pointer to memory for interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_intf_get_by_vid(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    intf_entry_t entry_l3intf;
    int    i;
    bcm_vlan_t vlan_id, vid;

    vid = intf_info->l3i_vid;

    for (i = 0; i < L3_INFO(unit)->l3_intf_table_size; i++) {
        if (0 == BCM_L3_INTF_USED_GET(unit, i)) {
            continue;
        }
        SOC_IF_ERROR_RETURN(READ_L3_INTFm(unit, MEM_BLOCK_ANY, i, &entry_l3intf));

        vlan_id  = soc_L3_INTFm_field32_get(unit, &entry_l3intf, VLAN_IDf);
        if (vlan_id == vid) {
            soc_L3_INTFm_mac_addr_get(unit, &entry_l3intf, ROUTER_ADDRf,
                                      intf_info->l3i_mac_addr);
            intf_info->l3i_index = i;
            return (BCM_E_NONE);
        }
    }
    return (BCM_E_NOT_FOUND);
}

/*
 * Function:
 *	_bcm_xgs_l3_intf_set
 * Purpose:
 *      Set an entry in L3 interface table.
 * Parameters:
 *      unit - SOC unit number.
 *	intf_info - Pointer to memory for interface information.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs_l3_intf_set(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    intf_entry_t entry_l3intf;
    int rv;

    sal_memset(&entry_l3intf, 0, sizeof(entry_l3intf));

    soc_L3_INTFm_mac_addr_set(unit, &entry_l3intf, ROUTER_ADDRf,
                              intf_info->l3i_mac_addr);
    soc_L3_INTFm_field32_set(unit, &entry_l3intf, VLAN_IDf,
                             intf_info->l3i_vid);
    INTF_LOCK(unit);
    rv = WRITE_L3_INTFm(unit, MEM_BLOCK_ALL, intf_info->l3i_index,
                        &entry_l3intf);
    INTF_UNLOCK(unit);

#ifdef BCM_XGS12_SWITCH_SUPPORT
    /* reconcile L3 programming with VLAN tables' state */
    _bcm_xgs_l3_untagged_update(unit, BCM_VLAN_INVALID, BCM_TRUNK_INVALID, 
                                intf_info->l3i_index);
#endif  /* BCM_XGS12_SWITCH_SUPPORT */

    return (rv);
}

/*
 * Create the L3 interface
 */
STATIC int
_xgs_l3_intf_do_create(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    bcm_l2_addr_t l2addr;
    int rv;

    BCM_IF_ERROR_RETURN
        (_bcm_xgs_l3_intf_set(unit, intf_info));

    if (intf_info->l3i_flags & BCM_L3_ADD_TO_ARL) {
        bcm_l2_addr_t_init(&l2addr, intf_info->l3i_mac_addr, intf_info->l3i_vid);
        l2addr.flags = BCM_L2_L3LOOKUP | BCM_L2_STATIC | BCM_L2_REPLACE_DYNAMIC;
        bcm_esw_l2_addr_delete(unit, intf_info->l3i_mac_addr, intf_info->l3i_vid);

        rv = bcm_esw_l2_addr_add(unit, &l2addr);

        if (rv < 0) {
            sal_memcpy(intf_info->l3i_mac_addr,
                       _soc_mac_all_zeroes, sizeof(sal_mac_addr_t));
            intf_info->l3i_vid = 0;
            _bcm_xgs_l3_intf_set(unit, intf_info);
            return (rv);
        }
        BCM_L3_INTF_ARL_SET(unit, intf_info->l3i_index);
    }

    if (!BCM_L3_INTF_USED_GET(unit, intf_info->l3i_index)) {
        BCM_L3_INTF_USED_SET(unit, intf_info->l3i_index);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_l3_intf_create
 * Purpose:
 *      Create a L3 interface
 * Parameters:
 *      unit - SOC unit number.
 *	intf_info - Pointer to memory for interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_intf_create(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    int i, rv;

    if (L3_INFO(unit)->l3intf_count == L3_INFO(unit)->l3_intf_table_size) {
        return BCM_E_FULL;
    }

    for (i = 0; i < L3_INFO(unit)->l3_intf_table_size; i++) {
        if (!BCM_L3_INTF_USED_GET(unit, i)) {
            intf_info->l3i_index = i;
            break;
        }
    }

    rv = _xgs_l3_intf_do_create(unit, intf_info);

    return rv;
}

/*
 * Function:
 *      bcm_xgs_l3_intf_id_create
 * Purpose:
 *      Create L3 interface with specified index
 * Parameters:
 *      unit - SOC unit number.
 *      intf_info - Pointer to memory for interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_intf_id_create(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    int i, rv;

    i = intf_info->l3i_index;
    if (i < 0 || i >= L3_INFO(unit)->l3_intf_table_size) {
        return BCM_E_PARAM;
    }

    rv = _xgs_l3_intf_do_create(unit, intf_info);

    return rv;
}

/*
 * Function:
 *      bcm_xgs_l3_intf_lookup
 * Purpose:
 *      See if an L3 interface exists by VID and MAC.
 * Returns:
 *      BCM_E_NONE if entry exists and is found;
 *      BCM_E_NOT_FOUND if the entry is not found.
 * Parameters:
 *      unit - SOC unit number.
 *      l3i  - Pointer to memory for interface configuration information.
 * Notes:
 *      Returns the interface ID if it exists
 */
int
bcm_xgs_l3_intf_lookup(int unit, _bcm_l3_intf_cfg_t *l3i)
{
    bcm_mac_t entry_mac;
    l3intf_entry_t entry;
    int i;

    for (i = 0; i < L3_INFO(unit)->l3_intf_table_size; i++) {
        SOC_IF_ERROR_RETURN(READ_L3_INTFm(unit, MEM_BLOCK_ANY, i, &entry));
        if (soc_L3_INTFm_field32_get(unit, &entry, VLAN_IDf) == l3i->l3i_vid) {
            soc_L3_INTFm_mac_addr_get(unit, &entry, ROUTER_ADDRf, entry_mac);
            if (sal_memcmp(entry_mac, l3i->l3i_mac_addr, sizeof(bcm_mac_t)) == 0) {
                l3i->l3i_index = i;
                return BCM_E_NONE;
            }
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *	bcm_xgs_l3_intf_del
 * Purpose:
 *      delete L3 interface
 * Parameters:
 *      unit - SOC unit number.
 *	intf_info - Pointer to memory for interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_intf_del(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    intf_entry_t entry_l3intf;

    if (L3_INFO(unit)->l3intf_count == 0) {
        return (BCM_E_NOT_FOUND);
    }

    if (!BCM_L3_INTF_USED_GET(unit, intf_info->l3i_index)) {
        return (BCM_E_NOT_FOUND);
    }

    if (BCM_L3_INTF_ARL_GET(unit, intf_info->l3i_index)) {
        SOC_IF_ERROR_RETURN
	    (READ_L3_INTFm(unit, MEM_BLOCK_ANY,
		       intf_info->l3i_index, &entry_l3intf));
        soc_L3_INTFm_mac_addr_get(unit, &entry_l3intf, ROUTER_ADDRf,
                                  intf_info->l3i_mac_addr);
        intf_info->l3i_vid =
            soc_L3_INTFm_field32_get(unit, &entry_l3intf, VLAN_IDf);
	bcm_esw_l2_addr_delete(unit, intf_info->l3i_mac_addr,
			   intf_info->l3i_vid);
        BCM_L3_INTF_ARL_CLR(unit, intf_info->l3i_index);
    }

    sal_memset(&entry_l3intf, 0, sizeof(entry_l3intf));
    SOC_IF_ERROR_RETURN
	(WRITE_L3_INTFm(unit, MEM_BLOCK_ALL,
		intf_info->l3i_index, &entry_l3intf));

    BCM_L3_INTF_USED_CLR(unit, intf_info->l3i_index);

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_l3_intf_del_all
 * Purpose:
 *      delete all L3 interfaces
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_intf_del_all(int unit)
{
    _bcm_l3_intf_cfg_t intf;
    int i;

    sal_memset(&intf, 0, sizeof(_bcm_l3_intf_cfg_t));

    for (i = 0; i < L3_INFO(unit)->l3_intf_table_size; i++) {
        if (BCM_L3_INTF_USED_GET(unit, i)) {
            intf.l3i_index = i;
            BCM_IF_ERROR_RETURN
		(bcm_xgs_l3_intf_del(unit, &intf));
        }
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_l3_get
 * Purpose:
 *      Get an entry from L3 table.
 * Parameters:
 *      unit - SOC unit number.
 *      l3cfg - Pointer to memory for L3 entry related information.
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This function is directly called from ipmc.c
 */
int
bcm_xgs_l3_get(int unit, _bcm_l3_cfg_t *l3cfg)
{
    _bcm_l3hash_t   hash;
    int rv;
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
    _bcm_defip_cfg_t lpm_cfg;
#endif

    sal_memset(&hash, 0, sizeof(hash));

    if (l3cfg->l3c_flags & BCM_L3_IPMC) {
        hash.l3h_sip_addr = l3cfg->l3c_src_ip_addr;
        hash.l3h_ip_addr = l3cfg->l3c_ipmc_group;
        hash.l3h_vid = l3cfg->l3c_vid;
    } else {
        hash.l3h_ip_addr = l3cfg->l3c_ip_addr;
    }

    rv = _bcm_l3hash_index_get(unit, &hash, &l3cfg->l3c_hw_index);

    if(BCM_E_NOT_FOUND == rv && !(l3cfg->l3c_flags & BCM_L3_IPMC)) {
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
        sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));
        lpm_cfg.defip_ip_addr = l3cfg->l3c_ip_addr;
        lpm_cfg.defip_sub_len = 32;     /* host IP address */
        BCM_IF_ERROR_RETURN(bcm_xgs_defip_cfg_get(unit, &lpm_cfg));

        BCM_IF_ERROR_RETURN
            (bcm_xgs_l3_get_by_index(unit, lpm_cfg.defip_l3hw_index, l3cfg));
        l3cfg->l3c_flags |= BCM_L3_HOST_AS_ROUTE;
        l3cfg->l3c_hw_index = lpm_cfg.defip_l3hw_index;
        return BCM_E_NONE;
#else
        return BCM_E_NOT_FOUND;
#endif
    } else if (rv < 0) {
        return rv;
    }

    return (bcm_xgs_l3_get_by_index(unit, l3cfg->l3c_hw_index, l3cfg));
}

#ifdef L3_HOST_TO_LPM_IF_L3_FULL

/*
 * Host table bucket is full, insert host entry into LPM table
 */
STATIC int
_bcm_xgs_host_to_lpm_add(int unit, _bcm_l3_cfg_t *l3cfg)
{
    _bcm_l3hash_t *hash_entry;
    _bcm_defip_cfg_t lpm_cfg;
    int rv, hw_index;

    rv = _bcm_l3_free_index_get(unit, &hw_index);
    if (rv < 0) {
        return BCM_E_FULL;
    }

    l3cfg->l3c_hw_index = hw_index;
    rv = _bcm_xgs_l3hw_insert(unit, l3cfg);
    if (rv == 0) {
        hash_entry = &l3_table[unit].l3_hash[hw_index];
        hash_entry->l3h_ip_addr = l3cfg->l3c_ip_addr;
        hash_entry->l3h_sip_addr = 0;
        hash_entry->l3h_vid = 0;
        hash_entry->l3h_flags = L3H_HOST | L3H_LPM;
        hash_entry->l3h_use_count = 1;
        l3_table[unit].l3_count++;
    } else {
        return (rv);
    }

    /* Insert to LPM table */
    sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));
    lpm_cfg.defip_ip_addr = l3cfg->l3c_ip_addr;
    lpm_cfg.defip_sub_len = 32;   /* host IP address */
    lpm_cfg.defip_intf    = l3cfg->l3c_intf;
    lpm_cfg.defip_nexthop_ip = l3cfg->l3c_ip_addr;

    lpm_cfg.defip_l3hw_index = hw_index;

    rv = bcm_xgs_lpm_insert(unit, &lpm_cfg);
    if (rv < 0) {
        _bcm_xgs_l3hw_index_del(unit, hw_index);
        l3_table[unit].l3_hash[hw_index].l3h_ip_addr = 0;
        hash_entry->l3h_use_count = 0;
        l3_table[unit].l3_count--;
        return rv;
    }

    return BCM_E_NONE;
}

/*
 * When host table bucket is full, host entry is inserted into LPM table,
 * delete host IP from LPM table.
 */
STATIC int
_bcm_xgs_host_to_lpm_del(int unit, _bcm_l3_cfg_t *l3cfg)
{
    _bcm_l3hash_t *hash_entry;
    _bcm_defip_cfg_t lpm_cfg;
    int rv, hw_index;
    int found;

    sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));
    lpm_cfg.defip_intf    = l3cfg->l3c_intf;
    lpm_cfg.defip_ip_addr = l3cfg->l3c_ip_addr;
    lpm_cfg.defip_sub_len = 32;   /* host IP address */
    found = _xgs_route_lookup(unit, &lpm_cfg);
    if (!found) {
        return BCM_E_NOT_FOUND;
    }

    /* Delete from LPM table */
    BCM_IF_ERROR_RETURN(bcm_xgs_lpm_delete(unit, &lpm_cfg));

    hw_index = lpm_cfg.defip_l3hw_index;
    hash_entry = &l3_table[unit].l3_hash[hw_index];
    if (hash_entry->l3h_use_count > 1) {
        hash_entry->l3h_use_count--;
        return BCM_E_NONE;
    }

    rv = _bcm_xgs_l3hw_index_del(unit, hw_index);
    if (rv == 0) {
        hash_entry = &l3_table[unit].l3_hash[hw_index];
        hash_entry->l3h_ip_addr = 0;
        hash_entry->l3h_flags = 0;
        hash_entry->l3h_use_count = 0;
        l3_table[unit].l3_count--;
    } else {
        return (rv);
    }

    return BCM_E_NONE;
}

/*
 * When host table bucket is full, host entry is inserted into LPM table,
 * update host IP from LPM table.
 */
STATIC int
_bcm_xgs_host_to_lpm_update(int unit, _bcm_l3_cfg_t *l3cfg)
{
    _bcm_defip_cfg_t lpm_cfg;
    int found;
    int rv;

    sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));
    lpm_cfg.defip_ip_addr = l3cfg->l3c_ip_addr;
    lpm_cfg.defip_sub_len = 32;   /* host IP address */
    found = _xgs_route_lookup(unit, &lpm_cfg);
    if (found) {
        l3cfg->l3c_hw_index = lpm_cfg.defip_l3hw_index;
        rv = _bcm_xgs_l3hw_insert(unit, l3cfg);
        return rv;
    } else {
        return BCM_E_NOT_FOUND;
    }

    return BCM_E_NONE;
}
#endif

/*
 * Function:
 *    bcm_xgs_l3_add
 * Purpose:
 *    Add entry to L3 table
 * Parameters:
 *    unit - SOC unit number.
 *    l3cfg - Pointer to memory for L3 entry related information.
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This function is directly called from ipmc.c
 */
int
bcm_xgs_l3_add(int unit, _bcm_l3_cfg_t *l3cfg)
{
    int rv, hw_index;
    _bcm_l3hash_t   hash;
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
    _bcm_defip_cfg_t defip;
    int found;
#endif

    if (l3cfg->l3c_flags & BCM_L3_TGID) {	/* trunk */
        if (SOC_IS_DRACO15(unit)) {
            if (l3cfg->l3c_port_tgid & ~(BCM_TGID_PORT_TRUNK_MASK(unit)
                                     +BCM_TGID_PORT_TRUNK_MASK_HI(unit))) {
                return BCM_E_PARAM;
            }
        } else {
            if (l3cfg->l3c_port_tgid & ~BCM_TGID_PORT_TRUNK_MASK(unit)) {
                return BCM_E_PARAM;
            }
        }
    }

    sal_memset(&hash, 0, sizeof(hash));

    L3_LOCK(unit);

    /* Save entry to SW L3 shadow tables(S/W hash table) */
    if (BCM_IP4_MULTICAST(l3cfg->l3c_ip_addr)) {
        hash.l3h_sip_addr = l3cfg->l3c_src_ip_addr;
        hash.l3h_ip_addr  = l3cfg->l3c_ipmc_group;
        hash.l3h_vid      = l3cfg->l3c_vid;
    } else {
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
        if (l3cfg->l3c_flags & BCM_L3_HOST_AS_ROUTE) {
            /* Is the IP address in LPM table already ? */
            sal_memset(&defip, 0, sizeof(_bcm_defip_cfg_t));
            defip.defip_ip_addr = l3cfg->l3c_ip_addr;
            defip.defip_sub_len = 32;   /* host IP address */
            found = _xgs_route_lookup(unit, &defip);
            if (found) {
                /* Update L3 entry info */
                l3cfg->l3c_hw_index = lpm_cfg.defip_l3hw_index;
                rv = _bcm_xgs_l3hw_insert(unit, l3cfg);
                L3_UNLOCK(unit);
                return BCM_E_NONE;
            }
        }
#endif
        hash.l3h_ip_addr = l3cfg->l3c_ip_addr;
    }

    rv = _bcm_l3sw_hash_insert(unit, L3H_HOST, &hash, &hw_index);

    if (rv == BCM_E_FULL) {
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
        if (l3cfg->l3c_flags & BCM_L3_HOST_AS_ROUTE) {
            rv = _bcm_xgs_host_to_lpm_add(unit, l3cfg);
        }
#endif
        L3_UNLOCK(unit);
        return (rv);
    } else if (rv < 0) {
        L3_UNLOCK(unit);
        return (rv);
    } else if (hash.l3h_use_count > 1) {
        L3_UNLOCK(unit);
        return BCM_E_NONE;
    }

    /* hardware insert for the L3 entry */
    l3cfg->l3c_hw_index = hw_index;
    rv = _bcm_xgs_l3hw_insert(unit, l3cfg);
    if (rv < 0) {
        _bcm_l3sw_hash_del(unit, L3H_HOST, &hash, &hw_index);
    }

    L3_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *    bcm_xgs_l3_del
 * Purpose:
 *    Delete entry to L3 table
 * Parameters:
 *    unit - SOC unit number.
 *    l3cfg - Pointer to memory for L3 entry related information.
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This function is directly called from ipmc.c
 */
int
bcm_xgs_l3_del(int unit, _bcm_l3_cfg_t *l3cfg)
{
    l3x_entry_t l3x_entry;
    int hw_index, rv;
    _bcm_l3hash_t   hash;

    sal_memset(&hash, 0, sizeof(hash));

    L3_LOCK(unit);

    /* Delete from SW L3 shadow tables(S/W hash table) */
    if (BCM_IP4_MULTICAST(l3cfg->l3c_ip_addr)) {
        hash.l3h_sip_addr = l3cfg->l3c_src_ip_addr;
        hash.l3h_ip_addr = l3cfg->l3c_ipmc_group;
        hash.l3h_vid = l3cfg->l3c_vid;
    } else {
        hash.l3h_ip_addr = l3cfg->l3c_ip_addr;
    }

    rv = _bcm_l3sw_hash_del(unit, L3H_HOST, &hash, &hw_index);

    if (rv == BCM_E_NOT_FOUND) {
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
        if (l3cfg->l3c_flags & BCM_L3_HOST_AS_ROUTE) {
            rv = _bcm_xgs_host_to_lpm_del(unit, l3cfg);
        }
#endif
        L3_UNLOCK(unit);
        return (rv);
    } else if (rv < 0) {
        L3_UNLOCK(unit);
        return (rv);
    } else if (hash.l3h_use_count > 0) {
        L3_UNLOCK(unit);
        return BCM_E_NONE;
    }

    /* Hardware deletion from L3 table */

    /* see _bcm_xgs_l3_add_with_ipmc_set() */
    if (!BCM_IP4_MULTICAST(l3cfg->l3c_ip_addr) &&
        l3cfg->l3c_flags & BCM_L3_L2TOCPU) {
        rv = _bcm_xgs_l3hw_index_del(unit, hw_index);
        L3_UNLOCK(unit);
        return rv;
    }

    sal_memset(&l3x_entry, 0, sizeof(l3x_entry));
    soc_L3Xm_field32_set(unit, &l3x_entry, IP_ADDRf, l3cfg->l3c_ip_addr);
    soc_L3Xm_field32_set(unit, &l3x_entry, L3_VALIDf, 0);
    if (BCM_IP4_MULTICAST(l3cfg->l3c_ip_addr)) {
        soc_L3Xm_field32_set(unit, &l3x_entry, SRC_IP_ADDRf,
                             l3cfg->l3c_src_ip_addr);
        soc_L3Xm_field32_set(unit, &l3x_entry, IPMCf, 1);

        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            soc_L3Xm_field32_set(unit, &l3x_entry, VLAN_IDf, l3cfg->l3c_vid);
        }
    }

    rv = soc_l3x_delete(unit, &l3x_entry);

    L3_UNLOCK(unit);

    return(rv);
}

/*
 * Function:
 *    bcm_xgs_l3_del_prefix
 * Purpose:
 *    Delete entries from L3 table that match prefix
 * Parameters:
 *    unit - SOC unit number.
 *    l3cfg - Pointer to memory for L3 entry related information.
 *            (IN) network address
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_del_prefix(int unit, _bcm_l3_cfg_t *l3cfg)
{
    uint32 i;
    int rv = BCM_E_NONE;
    _bcm_l3hash_t   hash;
    _bcm_l3_cfg_t cfg;

    sal_memset(&hash, 0, sizeof(hash));
    sal_memset(&cfg, 0, sizeof(cfg));

    L3_LOCK(unit);
    for (i = l3_table[unit].l3_min; i < L3_DEFIP_LOCAL_ROUTE_IDX(unit); i++) {
        if (l3_table[unit].l3_hash[i].l3h_ip_addr != 0) {
            /* don't touch IPMC or LPM entries */
            if (BCM_IP4_MULTICAST(l3_table[unit].l3_hash[i].l3h_ip_addr) ||
                l3_table[unit].l3_hash[i].l3h_ip_addr == LPM_L3_ADDR) {
                continue;
            }

            cfg.l3c_ip_addr = l3_table[unit].l3_hash[i].l3h_ip_addr;
            cfg.l3c_flags = 0;
            if ((rv = bcm_xgs_l3_get(unit, &cfg)) < 0) {
                break;
            }
            if ((cfg.l3c_ip_addr & l3cfg->l3c_ip_mask) == l3cfg->l3c_ip_addr) {
                if ((rv = bcm_xgs_l3_del(unit, &cfg)) < 0) {
                    break;
                }
            }
        }
    }
    L3_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *    bcm_xgs_l3_del_intf
 * Purpose:
 *    Delete entry from L3 table that matches L3 interface
 * Parameters:
 *    unit - SOC unit number.
 *    l3cfg - Pointer to memory for L3 entry related information.
 *    negate - 0 means interface match; 1 means not match
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_del_intf(int unit, _bcm_l3_cfg_t *l3cfg, int negate)
{
    uint32 i;
    int rv = BCM_E_NONE;
    _bcm_l3_cfg_t cfg;

    if (l3cfg->l3c_intf >= L3_INFO(unit)->l3_intf_table_size) {
        return (BCM_E_PARAM);
    }

    L3_LOCK(unit);
    for (i = l3_table[unit].l3_min; i < L3_DEFIP_LOCAL_ROUTE_IDX(unit); i++) {
        if (i == L3_D15_GLOBAL_DEF_ROUTE_IDX) {
            continue;
        }

        if (l3_table[unit].l3_hash[i].l3h_ip_addr != 0) {
            /* don't touch IPMC or LPM entries */
            if (BCM_IP4_MULTICAST(l3_table[unit].l3_hash[i].l3h_ip_addr) ||
                l3_table[unit].l3_hash[i].l3h_ip_addr == LPM_L3_ADDR) {
                continue;
            }

            sal_memset(&cfg, 0, sizeof(cfg));
            cfg.l3c_ip_addr = l3_table[unit].l3_hash[i].l3h_ip_addr;
            if ((rv = bcm_xgs_l3_get(unit, &cfg)) < 0) {
		break;
	    }

            if (!negate && cfg.l3c_intf == l3cfg->l3c_intf) {
                if ((rv = bcm_xgs_l3_del(unit, &cfg)) < 0) {
                    soc_cm_debug(DK_L3,
                        "bcm_xgs_l3_del_intf failed u=%d intf=%d rv=%s\n",
                         unit, l3cfg->l3c_intf, bcm_errmsg(rv));
		    break;
		}
            } else if (negate && cfg.l3c_intf != l3cfg->l3c_intf) {
                if ((rv = bcm_xgs_l3_del(unit, &cfg)) < 0) {
                    soc_cm_debug(DK_L3,
                        "bcm_xgs_l3_del_intf failed u=%d intf=%d rv=%s\n",
                         unit, l3cfg->l3c_intf, bcm_errmsg(rv));
                    break;
                }
            }
        }
    }
    L3_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *    bcm_xgs_l3_del_all
 * Purpose:
 *    Del all L3 entries
 * Parameters:
 *    unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_del_all(int unit)
{
    uint32 i;
    _bcm_l3hash_t   hash;
    _bcm_l3_cfg_t cfg;
    int rv = 0;

    sal_memset(&hash, 0, sizeof(hash));
    sal_memset(&cfg, 0, sizeof(cfg));

    L3_LOCK(unit);
    for (i = l3_table[unit].l3_min; i < L3_DEFIP_LOCAL_ROUTE_IDX(unit); i++) {
        if (l3_table[unit].l3_hash[i].l3h_ip_addr != 0) {
            /* don't touch IPMC or LPM entries */
            if (BCM_IP4_MULTICAST(l3_table[unit].l3_hash[i].l3h_ip_addr) ||
                l3_table[unit].l3_hash[i].l3h_ip_addr == LPM_L3_ADDR) {
                continue;
            }

            cfg.l3c_ip_addr = l3_table[unit].l3_hash[i].l3h_ip_addr;
            cfg.l3c_flags = 0;
            if ((rv = bcm_xgs_l3_del(unit, &cfg)) < 0) {
		break;
	    }
        }
    }
    L3_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *    bcm_xgs_l3_replace
 * Purpose:
 *    Update entry in L3 table
 * Parameters:
 *    unit - SOC unit number.
 *    l3cfg - Pointer to memory for L3 entry related information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_l3_replace(int unit, _bcm_l3_cfg_t *l3cfg)
{
    _bcm_l3hash_t *hash_entry;
    _bcm_defip_cfg_t lpm_cfg;
    int rv, hw_index;
    uint32 index;
    _bcm_l3hash_t hash;

    if (l3cfg->l3c_flags & BCM_L3_TGID) {       /* trunk */
        if (SOC_IS_DRACO15(unit)) {
            if (l3cfg->l3c_port_tgid & ~(BCM_TGID_PORT_TRUNK_MASK(unit)
                                     +BCM_TGID_PORT_TRUNK_MASK_HI(unit))) {
                return BCM_E_BADID;
            }
        } else {
            if (l3cfg->l3c_port_tgid & ~BCM_TGID_PORT_TRUNK_MASK(unit)) {
                return BCM_E_BADID;
            }
        }
    } else {                            /* port */
        if (!SOC_PBMP_PORT_VALID(l3cfg->l3c_port_tgid)) {
            return BCM_E_BADID;
        }
    }

    sal_memset(&hash, 0, sizeof(hash));

    if (l3cfg->l3c_flags & BCM_L3_IPMC) {
        hash.l3h_sip_addr = l3cfg->l3c_src_ip_addr;
        hash.l3h_ip_addr  = l3cfg->l3c_ipmc_group;
        hash.l3h_vid      = l3cfg->l3c_vid;  /* Draco 1.5 */
    } else {
        hash.l3h_ip_addr = l3cfg->l3c_ip_addr;
    }

    L3_LOCK(unit);

    rv = _bcm_l3hash_index_get(unit, &hash, &hw_index);
    if (rv == BCM_E_NONE) {
        l3cfg->l3c_hw_index = hw_index;
        rv = _bcm_xgs_l3hw_insert(unit, l3cfg);
        if (rv != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return (rv);
        }
    } else if (rv != BCM_E_NOT_FOUND) {
        L3_UNLOCK(unit);
        return (rv);
    }

    /* Key not found in L3 hash bucket (rv == BCM_E_NOT_FOUND) */
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
    if (l3cfg->l3c_flags & BCM_L3_HOST_AS_ROUTE) {
        rv = _bcm_xgs_host_to_lpm_update(unit, l3cfg);
        if (rv != BCM_E_NOT_FOUND) {
            L3_UNLOCK(unit);
            return (rv);
        }
    }
#endif

    /*
     * Key not found using L3 hashing, seach for Tucana/Lynx style
     * ECMP route associated L3 entry
     */
    if (SOC_IS_LYNX(unit) || SOC_IS_TUCANA(unit)) {
        for (index = 0; index < L3_DEFIP_LOCAL_ROUTE_IDX(unit); index++) {
            hash_entry = &l3_table[unit].l3_hash[index];
            if (hash_entry->l3h_ip_addr == LPM_L3_ADDR) {
                /* we use the sip to remember the nexthop IP addr */
                if (hash_entry->l3h_sip_addr == l3cfg->l3c_ip_addr) {
                    sal_memcpy(lpm_cfg.defip_mac_addr, l3cfg->l3c_mac_addr,
                               sizeof(sal_mac_addr_t));
                    lpm_cfg.defip_ip_addr   = l3cfg->l3c_ip_addr;
                    lpm_cfg.defip_flags     = l3cfg->l3c_flags;
                  lpm_cfg.defip_intf      = l3cfg->l3c_intf;
                    lpm_cfg.defip_modid     = l3cfg->l3c_modid;
                    lpm_cfg.defip_port_tgid = l3cfg->l3c_port_tgid;
                    rv = _bcm_xgs_l3hw_add_lpm_entry(unit, index, &lpm_cfg);
                    if (rv != BCM_E_NONE) {
                        L3_UNLOCK(unit);
                        return (rv);
                    }
                }
            }
        }
    }

    L3_UNLOCK(unit);

    return rv;
}

#ifdef  BCM_XGS12_SWITCH_SUPPORT

typedef struct _bcm_intf_cache_s {
    int          intf_num;
    bcm_vlan_t   vlan;
    pbmp_t       ubmp;
} _bcm_intf_cache_t;

#define BCM_INTF_CACHE_COUNT    20

/*
 * Function:
 *      _bcm_xgs_l3_untagged_update
 * Purpose:
 *      Main implementation of bcm_l3_untagged_update.
 * Parameters:
 *      unit - SOC device unit number.
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      The parameters vlan, tid, and intf_num are used as hints to
 *      only update relevant L3x entries, thus saving memory reads.
 */

int
_bcm_xgs_l3_untagged_update(int unit, bcm_vlan_t vlan, bcm_trunk_t tid,
                            int intf_num)
{
    int l3_idx=0;
    bcm_ip_t ip_addr;
    bcm_port_t port, tport;
    bcm_vlan_t temp_vlan=BCM_VLAN_INVALID;
    int temp_intf_num;
    l3intf_entry_t l3intf;
    int cur_ut, new_ut;
    int field_changed;
    int my_modid, module, isGport, is_local_modid;
    pbmp_t pbmp, ubmp;
    bcm_trunk_t temp_tid, tid_hi;
    bcm_trunk_add_info_t t_info;
    _bcm_l3hash_t *hash_entry;
    int i, j, rv=BCM_E_NONE;
    int chunk, nchunks, chunksize=1024;
    l3x_base_entry_t *l3xe, *l3xep;
    _bcm_intf_cache_t cache[BCM_INTF_CACHE_COUNT];

    if (!L3_INFO(unit)->l3_initialized) { 
        return BCM_E_INIT; 
    }

    /* Initialization. */
    sal_memset(cache, 0, (BCM_INTF_CACHE_COUNT * sizeof(_bcm_intf_cache_t)));

    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_modid));

    /* use table dma to walk through l3x table */
    l3xe = soc_cm_salloc(unit, chunksize * sizeof(*l3xe), "l3_chunk");
    if (l3xe == NULL) {
        return BCM_E_MEMORY;
    }
    nchunks = soc_mem_index_count(unit, L3X_BASEm) / chunksize;

    /* Keep a small cache of interfaces */
    for (j=0 ; j < BCM_INTF_CACHE_COUNT; j++) {
        cache[j].intf_num = -1;
    }

    L3_LOCK(unit);
    for (chunk = 0; chunk < nchunks; chunk++) {
        field_changed = 0; /* set to TRUE if something changes in this chunk */
        rv = soc_mem_read_range(unit, L3X_BASEm, MEM_BLOCK_ANY,
                                chunk * chunksize,
                                (chunk + 1) * chunksize - 1,
                                l3xe);
        if (rv < 0) {
            break;
        }

        for (i = 0; i < chunksize; i++) {
            hash_entry = &l3_table[unit].l3_hash[l3_idx++];

            /* First, check SW copy */
            if (hash_entry->l3h_ip_addr == 0) {
                /* Unused */
                continue;
            }
            if (hash_entry->l3h_ip_addr == INVALID_IP_ADDR) {
                /* LPM */
                continue;
            }
            if (BCM_IP4_MULTICAST(hash_entry->l3h_ip_addr)) {
                /* Multicast */
                continue;
            }

            /* Next, check HW copy */
            l3xep = soc_mem_table_idx_to_pointer(unit, L3X_BASEm,
                                                 l3x_base_entry_t *, l3xe, i);


            ip_addr = soc_L3X_BASEm_field32_get(unit, l3xep, IP_ADDRf);
            if (ip_addr != (hash_entry->l3h_ip_addr)) {
                
                continue;
            }

            module = soc_L3X_BASEm_field32_get(unit, l3xep, MODULE_IDf);
            if (module != my_modid) {
                /* Other unit will L2 switch, ignore */
                continue;
            }

            port = soc_L3X_BASEm_field32_get(unit, l3xep, TGID_PORTf);
            if (port & BCM_TGID_TRUNK_INDICATOR(unit)) {
                /* Determine TID */
                temp_tid = port & BCM_TGID_PORT_TRUNK_MASK(unit);
                if (SOC_IS_DRACO15(unit)) {
		    tid_hi = soc_L3X_BASEm_field32_get(unit, l3xep, TGID_HIf);
		    temp_tid |= (tid_hi << BCM_TGID_TRUNK_LO_BITS(unit));
                }

                if ((tid != BCM_TRUNK_INVALID) && (tid != temp_tid)) {
                    /* different from specified tid; continue */
                    continue;
                }

                /* Map trunk to (non-stack) port */
                rv = bcm_esw_trunk_get(unit, temp_tid, &t_info);
                if (rv < 0) {
                    goto done;
                }
                rv = bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport);
                if (BCM_FAILURE(rv)) {
                    goto done;
                }
                if (isGport) {
                    rv = _bcm_esw_trunk_gport_resolve(unit, tid, &t_info);
                    if (BCM_FAILURE(rv)) {
                        goto done;
                    }
                }

                for (tport = 0; tport < t_info.num_ports; tport++) {
                    rv = _bcm_esw_modid_is_local(unit, t_info.tm[tport],
                                                 &is_local_modid);
                    if (BCM_FAILURE(rv)) {
                        goto done;
                    }
                    if (is_local_modid) {
                        port = t_info.tp[tport];
                        break;
                    }
                }

                /* Check if failed to find tport */
                if (port & BCM_TGID_TRUNK_INDICATOR(unit)) {
                    /* Failed; continue */
                    continue;
                }
            }

            cur_ut = soc_L3X_BASEm_field32_get(unit, l3xep, UNTAGf);
            temp_intf_num = soc_L3X_BASEm_field32_get(unit, l3xep, L3_INTF_NUMf);
            if (soc_feature(unit, soc_feature_lynx_l3_expanded)) {
                temp_intf_num |= 
                    soc_L3X_BASEm_field32_get(unit, l3xep, L3_INTF_NUM_HIf) << 
                    BCM_LYNX_L3_INTF_NUM_HI_SHIFT;
            }

            if ((intf_num != -1) && (intf_num != temp_intf_num)) {
                /* different from specified intf_num; continue */
                continue;
            }

            /* Search the small cache of interfaces */
            for (j=0 ; j < BCM_INTF_CACHE_COUNT; j++) {
                if (temp_intf_num == cache[j].intf_num) {
                    temp_vlan = cache[j].vlan;
                    BCM_PBMP_ASSIGN(ubmp, cache[j].ubmp);
                    break;
                }
            }
            if (j < BCM_INTF_CACHE_COUNT) {
                if ((vlan != BCM_VLAN_INVALID) && (vlan != temp_vlan)) {
                    /* different from specified vlan; continue */
                    continue;
                }
            } else {
                /* Intf not found in cache, read from HW */
	            rv = READ_L3_INTFm(unit, MEM_BLOCK_ANY, temp_intf_num, &l3intf);
                if (SOC_E_NONE != rv) {
                    L3_UNLOCK(unit);
                    return rv;
                }

                temp_vlan = soc_L3_INTFm_field32_get(unit, &l3intf, VLAN_IDf);

                rv = bcm_esw_vlan_port_get(unit, temp_vlan, &pbmp, &ubmp);
                if (rv == BCM_E_NOT_FOUND) {
                    continue;
                } else if (rv < 0) {
                    goto done;
                }

                for (j=0 ; j < BCM_INTF_CACHE_COUNT; j++) {
                    if (cache[j].intf_num == -1) {
                        BCM_PBMP_ASSIGN(cache[j].ubmp, ubmp);
                        cache[j].vlan = temp_vlan;
                        cache[j].intf_num = temp_intf_num;
                        break;
                    }
                }

                if ((vlan != BCM_VLAN_INVALID) && (vlan != temp_vlan)) {
                    continue;
                }
            }

            new_ut = BCM_PBMP_MEMBER(ubmp, port) ? 1 : 0;

            if (cur_ut != new_ut) {
                /* Untagged bit has changed, update entry */
                soc_L3X_BASEm_field32_set(unit, l3xep, UNTAGf, new_ut);
                field_changed = 1;
            }
        }
        if (field_changed) {
            /* something changed in this chunk, dma back to HW */
            rv = soc_mem_write_range(unit, L3X_BASEm, MEM_BLOCK_ANY,
                                     chunk * chunksize,
                                     (chunk + 1) * chunksize - 1,
                                     l3xe);
            if (rv < 0) {
                break;
            }
        }
    }

done:
    L3_UNLOCK(unit);
    soc_cm_sfree(unit, l3xe);

    return rv;
}
#endif  /* BCM_XGS12_SWITCH_SUPPORT */

/*
 * Function:
 *	_lpm_getoffset
 * Purpose:
 *	Return a given nybble of an IP address
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *	Nybble value
 */
STATIC int
_lpm_getoffset(int unit, ip_addr_t ipaddr, int nybble)
{
    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        if (nybble == 0) { /* first byte */
            return ((ipaddr >> 24) & 0xff);
        } else {
            assert(nybble < 7); /* 7 offsets; since the first one is a byte */
            return ((ipaddr >> (28 - ((nybble + 1) * LPM_BITS_PER_BLK))) &
                    ((1 << LPM_BITS_PER_BLK) - 1));
        }
    } else {
        return ((ipaddr >> (32 - ((nybble + 1) * LPM_BITS_PER_BLK))) &
                ((1 << LPM_BITS_PER_BLK) - 1));
    }
}

/*
 * Function:
 *	_bcm_xgs_def_route_insert
 * Purpose:
 *	Adds the default IP (0.0.0.0/0) route in the LPM table
 *      (last entry LPM is for default route)
 * Parameters:
 *	unit    - SOC unit number.
 *	lpm_cfg - route info
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *    - For non-bcm5695 chips only (5695 has per VLAN def route)
 *    - Do not lock the LPM table before calling this function as it
 *	is locked inside function.
 */
STATIC int
_bcm_xgs_def_route_insert(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    defip_hi_entry_t defip_entry;
    uint32 def_def_reg = 0;
    _bcm_lpm_entry_t *def_route;
    uint32 next_ptr;
    int def_route_index;
    int rv;

    sal_memset(&defip_entry, 0, sizeof(defip_hi_entry_t));

    def_route_index = l3_defip_table[unit].def_route_index;
    def_route = L3_LPM_ENTRY(unit, def_route_index);

    /*
     * Do the book keeping
     */
    def_route->valid = 1;
    def_route->final = 1;
    def_route->ip_addr = 0;    /* default route */
    def_route->subnet_len = 0;
    def_route->l3_index = lpm_cfg->defip_l3hw_index;
    def_route->l3_intf = lpm_cfg->defip_intf;
    def_route->ecmp = lpm_cfg->defip_ecmp;
    def_route->ecmp_count = lpm_cfg->defip_ecmp_count;
    def_route->ecmp_index = lpm_cfg->defip_ecmp_index;

    /*
     * write to DEFIP_HIm table
     */
    if (SOC_IS_DRACO1(unit)) {
        /*
         * DRACO
         */
        next_ptr = lpm_cfg->defip_l3hw_index;
        soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry, L3_INTF_NUMf,
                            def_route->l3_intf);
    } else if (SOC_IS_TUCANA(unit)) {
        /*
         * TUCANA
         */
        if (def_route->ecmp) {
            assert(lpm_cfg->defip_ecmp_count >= 1);

            /* ecmp_count is # of ECMP routes minus 1 */
            soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry, COUNTf,
                                (lpm_cfg->defip_ecmp_count - 1) & 0x7);
        }
        next_ptr = lpm_cfg->defip_l3hw_index;
    } else if (SOC_IS_LYNX(unit)) {
        /*
         * LYNX
         */
        if (def_route->ecmp) {
            assert(lpm_cfg->defip_ecmp_count >= 1);

            /* ecmp_count is # of ecmp routes minus 1 */
            soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry, ECMP_COUNTf,
                                (lpm_cfg->defip_ecmp_count - 1) & 0x7);
        }
        next_ptr = lpm_cfg->defip_l3hw_index;
    } else {
        return BCM_E_UNIT;
    }

    BCM_LPM_NEXT_PTR_SET(unit, &defip_entry, next_ptr);
    soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry,
			STOPf, 1);
    soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry,
			VALIDf, 1);

    if (soc_feature(unit, soc_feature_lynx_l3_expanded)) {
        /* Perhaps this might be tightened up a bit? */
        soc_reg_field_set(unit, ARL_DEFAULT_DEFAULT_ROUTER_IPr,
                          &def_def_reg, STOPf,
                          soc_mem_field32_get(unit, LPM_MEM(unit),
                                              &defip_entry, STOPf));
        soc_reg_field_set(unit, ARL_DEFAULT_DEFAULT_ROUTER_IPr,
                          &def_def_reg, VALIDf,
                          soc_mem_field32_get(unit, LPM_MEM(unit),
                                              &defip_entry, VALIDf));
        soc_reg_field_set(unit, ARL_DEFAULT_DEFAULT_ROUTER_IPr,
                          &def_def_reg, ECMP_COUNTf,
                          soc_mem_field32_get(unit, LPM_MEM(unit),
                                              &defip_entry, ECMP_COUNTf));
        soc_reg_field_set(unit, ARL_DEFAULT_DEFAULT_ROUTER_IPr,
                          &def_def_reg, L3_INDEXf,
                          soc_mem_field32_get(unit, LPM_MEM(unit),
                                              &defip_entry, L3_INDEXf));
        soc_reg_field_set(unit, ARL_DEFAULT_DEFAULT_ROUTER_IPr,
                          &def_def_reg, L3_INDEX_HIf,
                          soc_mem_field32_get(unit, LPM_MEM(unit),
                                              &defip_entry, L3_INDEX_HIf));
        rv = WRITE_ARL_DEFAULT_DEFAULT_ROUTER_IPr(unit, def_def_reg);
    } else {
        /* last entry LPM is default route */
        rv = soc_mem_write(unit, LPM_MEM(unit), COPYNO_ALL,
                           def_route_index, &defip_entry);
    }

    if (rv < 0) {
        def_route->valid = 0;
        return rv;
    }

    /*
     * if necessary, write to DEFIP_LOm table as well
     */
    if (SOC_IS_DRACO1(unit) || SOC_IS_TUCANA(unit)) {  /* DRACO, TUCANA */
        rv = soc_mem_write(unit, DEFIP_LOm, COPYNO_ALL,
			   def_route_index, &defip_entry);

        if (rv < 0) {
            def_route->valid = 0;

            def_route->final = 0;
            def_route->l3_index = INVALID_L3_IDX;
            def_route->l3_intf = 0;
            def_route->ip_addr = 0;
            def_route->subnet_len = 0;
            def_route->dirty = 0;
            def_route->ecmp = 0;
            def_route->ecmp_count = 0;
            def_route->ecmp_index = -1;

            /* clear the H/W DEFIP_HI default entry */
            sal_memset(&defip_entry, 0, sizeof(defip_hi_entry_t));
            soc_mem_write(unit, LPM_MEM(unit), COPYNO_ALL,
                          def_route_index, &defip_entry);
            return rv;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_xgs_def_route_delete
 * Purpose:
 *	Deletes the default default entry in the LPM table
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	Do not lock the DEFIP before calling this routine as it
 *	is locked in here.
 */
STATIC int
_bcm_xgs_def_route_delete(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    defip_hi_entry_t defip_entry;
    _bcm_lpm_entry_t *def_route;
    int def_route_index;

    def_route_index = l3_defip_table[unit].def_route_index;
    def_route = L3_LPM_ENTRY(unit, def_route_index);

    if (soc_feature(unit, soc_feature_lynx_l3_expanded)) {
        SOC_IF_ERROR_RETURN
            (WRITE_ARL_DEFAULT_DEFAULT_ROUTER_IPr(unit, 0));
    } else {
        /* write to DEFIP table */
        sal_memset(&defip_entry, 0, sizeof(defip_hi_entry_t));
        SOC_IF_ERROR_RETURN
            (soc_mem_write(unit, LPM_MEM(unit), COPYNO_ALL,
                           def_route_index, &defip_entry));

        /* if necessary, write to DEFIP_LOm table */
        if (SOC_IS_DRACO1(unit) || SOC_IS_TUCANA(unit)) {  /* DRACO, TUCANA */
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, DEFIP_LOm, COPYNO_ALL,
                               def_route_index, &defip_entry));
        }
    }

    def_route->valid   = 0;
    def_route->final   = 0;
    def_route->l3_index = 0;
    def_route->l3_intf = 0;
    def_route->ip_addr = INVALID_IP_ADDR;
    def_route->subnet_len = INVALID_PREFIX_LEN;
    def_route->dirty = 0;
    def_route->ecmp = 0;
    def_route->ecmp_count = 0;
    def_route->ecmp_index = 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_xgs_per_vlan_def_route_add
 * Purpose:
 *      Adds the per VLAN default IP (0.0.0.0/0) route
 * Parameters:
 *      unit    - SOC unit number.
 *      lpm_cfg - route info (including VLAN ID)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *    For BCM5695 only (only 5695 supports per VLAN def route)
 */
STATIC int
_bcm_xgs_per_vlan_def_route_add(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_per_vlan_def_route_t *pv_dr;
    vlan_tab_entry_t vt;
    int l3_index;
    int rv;

    /* Check for global default route */
    if (lpm_cfg->defip_vid == L3_VLAN_GLOBAL) {
        if (l3_defip_table[unit].global_def_route_set) {
            return BCM_E_EXISTS;
        }
        /*
         * Since DEFAULT_ROUTEf field in VLAN table is initialized to 0,
         * global default route is set at index 0 of L3 table.
         */
        BCM_IF_ERROR_RETURN(_bcm_xgs_l3hw_add_lpm_entry(
                        unit, L3_D15_GLOBAL_DEF_ROUTE_IDX, lpm_cfg));
        l3_defip_table[unit].global_def_route_set = 1;
        return BCM_E_NONE;
    }

    pv_dr = l3_defip_table[unit].pv_def_route;
    while (pv_dr) {
        if (pv_dr->vid == lpm_cfg->defip_vid) { /* Route exists */
            soc_cm_debug(DK_L3, "Route exists\n");
            return (BCM_E_EXISTS);
        }
        pv_dr = pv_dr->next;
    }

    /* Check if VLAN exists */
    SOC_IF_ERROR_RETURN(
        READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int)lpm_cfg->defip_vid, &vt));
    if (!soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
        soc_cm_debug(DK_L3, "VLAN does not exist\n");
        return BCM_E_PARAM;
    }

    /*
     * Get L3 index for the per VLAN default route for this VID
     */
    L3_LOCK(unit);
    rv = _bcm_xgs_l3_lpm_entry_insert(unit, lpm_cfg);
    if (rv < 0) {
        L3_UNLOCK(unit);
        return rv;
    }
    L3_UNLOCK(unit);

    l3_index = lpm_cfg->defip_l3hw_index;

    /*
     * Set the per VLAN default route in VLAN table
     */
    soc_mem_lock(unit, VLAN_TABm);
    rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int)lpm_cfg->defip_vid, &vt);
    if (rv < 0) {
        soc_mem_unlock(unit, VLAN_TABm);
        return rv;
    }
    soc_VLAN_TABm_field32_set(unit, &vt, DEFAULT_ROUTEf, l3_index);
    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int)lpm_cfg->defip_vid, &vt);
    soc_mem_unlock(unit, VLAN_TABm);

    /* Something is wrong, undo previous actions */
    if (rv < 0) {
        _bcm_xgs_l3_lpm_entry_del(unit, lpm_cfg);
        return rv;
    }

    /* Book keeping */
    pv_dr = sal_alloc(sizeof(_bcm_per_vlan_def_route_t), "l3_lpm");
    if (pv_dr == NULL) {
        return BCM_E_MEMORY;
    }
    pv_dr->l3_index = l3_index;
    pv_dr->vid = lpm_cfg->defip_vid;
    pv_dr->next = l3_defip_table[unit].pv_def_route;
    l3_defip_table[unit].pv_def_route = pv_dr;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_xgs_per_vlan_def_route_del_all
 * Purpose:
 *      Delete all per VLAN default IP (0.0.0.0/0) route
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *    For BCM5695 only (only 5695 supports per VLAN def route)
 */
STATIC int
_bcm_xgs_per_vlan_def_route_del_all(int unit)
{
    _bcm_per_vlan_def_route_t *cur, *next;
    vlan_tab_entry_t vt;
    bcm_vlan_t vid;
    int l3_idx, l3_use_cnt;
    int rv;

    /* Delete global Per VLAN default route */
    if (l3_defip_table[unit].global_def_route_set) {
        BCM_IF_ERROR_RETURN(_bcm_xgs_global_def_route_del(unit));
        l3_defip_table[unit].global_def_route_set = 0;
    }

    for (cur = l3_defip_table[unit].pv_def_route; cur; cur = next) {
        next = cur->next;

        soc_mem_lock(unit, VLAN_TABm);
        vid = cur->vid;
        rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, vid, &vt);
        if (rv < 0) {
            soc_mem_unlock(unit, VLAN_TABm);
            return rv;
        }

        l3_idx = soc_VLAN_TABm_field32_get(unit, &vt, DEFAULT_ROUTEf);
        soc_VLAN_TABm_field32_set(unit, &vt, DEFAULT_ROUTEf, 0);
        rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int)vid, &vt);
        soc_mem_unlock(unit, VLAN_TABm);

        L3_LOCK(unit);
        _bcm_l3sw_index_del(unit, L3H_LPM, l3_idx, &l3_use_cnt);
        if (l3_use_cnt == 0) {
            rv = _bcm_xgs_l3hw_index_del(unit, l3_idx);
            if (rv < 0) {
                L3_UNLOCK(unit);
                return rv;
            }
        }
        L3_UNLOCK(unit);

        sal_free(cur);
    }

    l3_defip_table[unit].pv_def_route = NULL;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_xgs_per_vlan_def_route_del
 * Purpose:
 *      Delete per VLAN default IP (0.0.0.0/0) route
 * Parameters:
 *      unit - SOC unit number.
 *      VID  - VLAN ID
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *    For BCM5695 only (only 5695 supports per VLAN def route)
 */
STATIC int
_bcm_xgs_per_vlan_def_route_del(int unit, bcm_vlan_t vid)
{
    _bcm_per_vlan_def_route_t *pv_dr, *prev;
    vlan_tab_entry_t vt;
    int l3_idx, l3_use_cnt, found = 0;
    int rv;

    /* Delete all per VLAN default routes (including global) */
    if (vid == L3_VLAN_ALL) {
        _bcm_xgs_per_vlan_def_route_del_all(unit);
    }

    /* Check for global default route */
    if (vid == L3_VLAN_GLOBAL) {
        if (!l3_defip_table[unit].global_def_route_set) {
            return BCM_E_NOT_FOUND;
        }
        BCM_IF_ERROR_RETURN(_bcm_xgs_global_def_route_del(unit));
        l3_defip_table[unit].global_def_route_set = 0;
        return BCM_E_NONE;
    }

    prev = NULL;
    pv_dr = l3_defip_table[unit].pv_def_route;
    while (pv_dr) {
        if (pv_dr->vid == vid) {
            found = 1;
            break;
        }
        prev = pv_dr;
        pv_dr = pv_dr->next;
    }

    if (!found) {
        soc_cm_debug(DK_L3, "No Per-VLAN default route set for VLAN %d\n", vid);
        return BCM_E_NOT_FOUND;
    }

    soc_mem_lock(unit, VLAN_TABm);
    rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int)vid, &vt);
    if (rv < 0) {
        soc_mem_unlock(unit, VLAN_TABm);
        return rv;
    }

    l3_idx = soc_VLAN_TABm_field32_get(unit, &vt, DEFAULT_ROUTEf);
    soc_VLAN_TABm_field32_set(unit, &vt, DEFAULT_ROUTEf, 0);
    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int)vid, &vt);
    soc_mem_unlock(unit, VLAN_TABm);

    L3_LOCK(unit);
    _bcm_l3sw_index_del(unit, L3H_LPM, l3_idx, &l3_use_cnt);
    if (l3_use_cnt == 0) {
        rv = _bcm_xgs_l3hw_index_del(unit, l3_idx);
        if (rv < 0) {
            L3_UNLOCK(unit);
            return rv;
        }
    }
    L3_UNLOCK(unit);

    /* book keeping */
    if (prev == NULL) {
        l3_defip_table[unit].pv_def_route = pv_dr->next;
    } else {
        prev->next = pv_dr->next;
    }
    sal_free(pv_dr);

    return BCM_E_NONE;
}

/*
 * Function:
 *	_lpm_add_to_update_list
 * Purpose:
 *	This routine adds the given block to the list of blocks
 *	that need to be written to LPM memory.
 * Parameters:
 *	unit - SOC unit number.
 *	lpm_blk_idx - the LPM block index
 *	offset - offset inside the block
 * Returns:
 *	Nothing
 * Note:
 *	If offset is between (0, LPM_BLK_SIZE), then set one bit
 *	for this offset.
 *	If offset is LPM_BLK_SIZE, then set all the offsets (i.e
 *	write whole block).
 */
STATIC void
_lpm_add_to_update_list(int unit, int lpm_blk_idx, int offset)
{
    _bcm_update_list_t *entry = NULL, *new_update_node = NULL;
    _bcm_lpm_entry_t *lpm_block;
    int found = 0, i;

    if (lpm_blk_idx == INVALID_LPM_IDX) {
        return;
    }

    lpm_block = L3_LPM_ENTRY(unit, lpm_blk_idx);

    /* If not Draco1.5, offset can not be larger than LPM_BLK_SIZE */
    if (!IS_LPM_LONG_BLOCK(unit, lpm_blk_idx)) {
        if (offset < 0 || offset > LPM_BLK_SIZE) {
            return;
        }
    }

    /* If the LPM entry is already in the update list, add to offset bitmap */
    if (l3_defip_table[unit].update_head) {
        entry = l3_defip_table[unit].update_head;
        while (entry) {
            if (entry->block == lpm_blk_idx) {
                found = 1;
                break;
            }
            entry = entry->next;
        }

        if (found) { /* set the offset that needs to be written */
            if (IS_LPM_LONG_BLOCK(unit, lpm_blk_idx)) {  /* 1st blk in BCM5695 */
                lpm_block[offset].dirty = 1;
            } else {
                if (offset < LPM_BLK_SIZE) {
                    entry->offsets |= (0x1<<offset);
                    lpm_block[offset].dirty = 1;
                } else {
                    /* set all the offsets */
                    entry->offsets |= ((0x1<<offset)-1);
                    for (i = 0; i < LPM_BLK_SIZE; i++) {
                        lpm_block[i].dirty = 1;
                    }
                }
            }
        }
    }

    /* add new block to the update list */
    if (!found) {
        new_update_node = (_bcm_update_list_t *)
                    sal_alloc(sizeof(_bcm_update_list_t), "l3_lpm");
        assert(new_update_node != NULL);
        sal_memset(new_update_node, 0, sizeof(_bcm_update_list_t));
        new_update_node->block = lpm_blk_idx;

        if (IS_LPM_LONG_BLOCK(unit, lpm_blk_idx)) { /* first block in BCM5695 */
            lpm_block[offset].dirty = 1;
        } else {
            if (offset < LPM_BLK_SIZE) {
                new_update_node->offsets |= (0x1 << offset);
                lpm_block[offset].dirty = 1;
            } else if (offset == LPM_BLK_SIZE) { /* set all the offsets */
                new_update_node->offsets |= ((0x1 << offset) - 1);
                for (i = 0; i < LPM_BLK_SIZE; i++) {
                    lpm_block[i].dirty = 1;
                }
            }
        }

        if (l3_defip_table[unit].update_head) {
            l3_defip_table[unit].update_tail->next = new_update_node;
            l3_defip_table[unit].update_tail = new_update_node;
        } else {
            l3_defip_table[unit].update_head = new_update_node;
            l3_defip_table[unit].update_tail = new_update_node;
        }
    }
}

/*
 * Function:
 *      _bcm_xgs_defip_do_add
 * Purpose:
 *      Insert route to the LPM table (HW and SW backup tables)
 *      (Single path route or the 1st path of ECMP route)
 * Parameters:
 *     unit  - device #
 *     lpm_cfg - the route to delete
 */
STATIC int
_bcm_xgs_defip_do_add(int unit,
		      _bcm_defip_cfg_t *lpm_cfg)
{
    ecmp_entry_t ecmp_entry;
    int l3_index = 0;
    int rv;

    /*
     * Adding an IP route means adding the route to the LPM table
     * as well as to the L3 table.  The final node in the LPM entry
     * points to a L3 table entry, which is where the next hop MAC addr,
     * port, L3 intf number, ModID are stored for this route.
     * Failure in any table insertion, all previous actions need
     * to be undone.
     *
     * The key to calculate L3 table index is the next hop IP address,
     */
    L3_LOCK(unit);
    if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH) {
        lpm_cfg->defip_ecmp = 1;
        lpm_cfg->defip_ecmp_count = 1;

        if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
	    rv = _bcm_ecmp_free_index_get(unit, &lpm_cfg->defip_ecmp_index);
	    if (rv < 0) {
                L3_UNLOCK(unit);
                return (rv);
            }
        }
    } else {
        lpm_cfg->defip_ecmp = 0;
        lpm_cfg->defip_ecmp_count = 0;
        lpm_cfg->defip_ecmp_index = -1;
    }

    /*
     * Add the route info to the L3 switching table using nexthop IP
     * address as the key, except for Tucana style ECMP route
     */ 
    if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH && !SOC_IS_DRACO15(unit)) {
        rv = _bcm_l3_free_index_get(unit, &l3_index);
        if (rv < 0) {
            L3_UNLOCK(unit);
            return (rv);
        }

        lpm_cfg->defip_l3hw_index = l3_index;

        /* Insert pure LPM entry (IP address LPM_L3_ADDR) */
        rv = _bcm_xgs_l3hw_add_lpm_entry(unit, l3_index, lpm_cfg);
        if (rv < 0) {
            L3_UNLOCK(unit);
            return rv;
        }
        _bcm_l3sw_add_lpm_entry(unit, l3_index, lpm_cfg->defip_nexthop_ip);
    } else {
        if (lpm_cfg->defip_flags & BCM_L3_DEFIP_LOCAL) {
            int local_route_idx;

            local_route_idx = L3_DEFIP_LOCAL_ROUTE_IDX(unit);
            lpm_cfg->defip_l3hw_index = local_route_idx;
            l3_table[unit].l3_hash[local_route_idx].l3h_use_count++;
        } else {
            rv = _bcm_xgs_l3_lpm_entry_insert(unit, lpm_cfg);
            if (rv < 0) {
                L3_UNLOCK(unit);
                return rv;
            }
        }
    }

    soc_cm_debug(DK_L3, "LPM L3 entry index %d\n", lpm_cfg->defip_l3hw_index);

    if (SOC_IS_DRACO15(unit) && lpm_cfg->defip_ecmp) {
        /* Draco1.5 */
        sal_memset(&ecmp_entry, 0, sizeof(ecmp_entry_t));
        soc_mem_field32_set(unit, L3_ECMPm, &ecmp_entry,
                        NEXT_HOPf, lpm_cfg->defip_l3hw_index);
        rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL,
                           lpm_cfg->defip_ecmp_index, &ecmp_entry);
        if (rv < 0) {
            L3_UNLOCK(unit);
            return (rv);
        }
    }

    L3_UNLOCK(unit);

    /*
     * Insert to the HW LPM table (and SW LPM table)
     */
    DEFIP_LOCK(unit);
    if (!SOC_IS_DRACO15(unit) &&
        lpm_cfg->defip_ip_addr == 0 && lpm_cfg->defip_sub_len == 0) {
        rv = _bcm_xgs_def_route_insert(unit, lpm_cfg);
    } else {
        rv = bcm_xgs_lpm_insert(unit, lpm_cfg);
    }
    DEFIP_UNLOCK(unit);

    /* Something is wrong, undo previous actions */
    if (rv < 0) {
        if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH) {
            if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
                sal_memset(&ecmp_entry, 0, sizeof(ecmp_entry_t));
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL,
                              lpm_cfg->defip_ecmp_index, &ecmp_entry));
                _bcm_xgs_l3_lpm_entry_del(unit, lpm_cfg);
            } else {
                _bcm_xgs_l3hw_index_del(unit, l3_index);
                l3_table[unit].l3_hash[l3_index].l3h_ip_addr = 0;
                l3_table[unit].l3_count--;
            }
        } else {
            _bcm_xgs_l3_lpm_entry_del(unit, lpm_cfg);
        }

        return rv;
    }

    if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH &&
        !l3_defip_table[unit].ecmp_inuse) {
        l3_defip_table[unit].ecmp_inuse = 1;
    }

    if (SOC_IS_DRACO15(unit) &&
        lpm_cfg->defip_ip_addr == 0 && lpm_cfg->defip_sub_len == 0) {
        l3_defip_table[unit].lpm_based_def_route_set = 1;
    }

    if (lpm_cfg->defip_flags & BCM_L3_HIT) {
        _bcm_xgs_lpm_do_hit(unit, lpm_cfg, 1);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_xgs_defip_do_del
 * Purpose:
 *	Delete an LPM entry from the LPM table (HW and SW backup tables)
 * Parameters:
 *     unit  - SOC device unit number
 *     lpm_cfg - the route to delete
 */

STATIC int
_bcm_xgs_defip_do_del(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    ecmp_entry_t ecmp_entry;
    int l3_use_cnt, l3_index = 0;
    int i, rv = BCM_E_NONE;

    /*
     * Adding an IP route means adding the route to the LPM table
     * as well as to the L3 table.  The final node in the LPM entry
     * points to a L3 table entry, which is where the next hop MAC address,
     * next hope IP, port, L3 intf number, ModID are stored for this route.
     * So a deletion affacts both LPM and L3 tables.
     *
     * Note that the index into the L3 table should be known,
     * use the index to deletion from the table.
     */
    L3_LOCK(unit);

    if (lpm_cfg->defip_ecmp) {
        for (i = lpm_cfg->defip_ecmp_count - 1; i >= 0; i--) {
            if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
                rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY,
                             lpm_cfg->defip_ecmp_index + i, &ecmp_entry);
                if (BCM_FAILURE(rv)) {
                    L3_UNLOCK(unit);
                    return (rv);
                }

                l3_index = soc_mem_field32_get(unit, L3_ECMPm,
                                               &ecmp_entry, NEXT_HOPf);

                sal_memset(&ecmp_entry, 0, sizeof(ecmp_entry_t));
                rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL,
                             lpm_cfg->defip_ecmp_index + i, &ecmp_entry);
                if (BCM_FAILURE(rv)) {
                    L3_UNLOCK(unit);
                    return (rv);
                }
            } else {
                l3_index = lpm_cfg->defip_l3hw_index + (i * L3_BUCKET_SIZE);
            }

            /* Del from L3 table if necessary (no more other ref to L3 entry) */
            _bcm_l3sw_index_del(unit, L3H_LPM, l3_index, &l3_use_cnt);
            if (l3_use_cnt == 0) {
                rv = _bcm_xgs_l3hw_index_del(unit, l3_index);
                if (rv < 0) {
                    L3_UNLOCK(unit);
                    return rv;
                }
            }
        }
    } else {
        _bcm_l3sw_index_del(unit, L3H_LPM, lpm_cfg->defip_l3hw_index, &l3_use_cnt);
        if (l3_use_cnt == 0) {
            rv = _bcm_xgs_l3hw_index_del(unit, lpm_cfg->defip_l3hw_index);
            if (rv < 0) {
                L3_UNLOCK(unit);
                return rv;
            }
        }
    }

    L3_UNLOCK(unit);

    /* Delete from DEFIP table */
    DEFIP_LOCK(unit);
    if (!SOC_IS_DRACO15(unit) &&
        lpm_cfg->defip_ip_addr == 0 && lpm_cfg->defip_sub_len == 0) {
        rv = _bcm_xgs_def_route_delete(unit, lpm_cfg);
    } else {
        rv = bcm_xgs_lpm_delete(unit, lpm_cfg);
    }
    DEFIP_UNLOCK(unit);

    if (SOC_IS_DRACO15(unit) &&
        lpm_cfg->defip_ip_addr == 0 && lpm_cfg->defip_sub_len == 0) {
        l3_defip_table[unit].lpm_based_def_route_set = 0;
    }

    return rv;
}

/*
 * Function:
 *	_lpm_reverse_update_list
 * Purpose:
 *	This routine reverses the update list. The update list contains
 *	the LPM blocks that need to be updated in H/W.
 * Parameters:
 *	unit - SOC unit number.
 */
STATIC void
_lpm_reverse_update_list(int unit)
{
    _bcm_update_list_t *new_head, *new_tail, *head, *tmp;

    if (l3_defip_table[unit].update_head) {
        new_head = new_tail = head = l3_defip_table[unit].update_head;
        head = head->next;
        new_tail->next = NULL;
        while (head) {
            tmp = head->next;
            head->next = new_head;
            new_head = head;
            head = tmp;
        }

        l3_defip_table[unit].update_head = new_head;
        l3_defip_table[unit].update_tail = new_tail;
    }
}

#ifndef NDEBUG
/*
 * Function:
 *	_lpm_chk_free_blk
 * Purpose:
 *	Check if the LPM block is not used
 * Parameters:
 *	unit - SOC unit number.
 *	index - L3 defip table index.
 */
STATIC int
_lpm_chk_free_blk(int unit, int index)
{
    _bcm_lpm_entry_t *lpmp;
    int i;

    assert (!(index % LPM_BLK_SIZE));

    lpmp = L3_LPM_ENTRY(unit, index);

    for (i = 0; i < LPM_BLK_SIZE; i++) {
	if (lpmp[i].valid == 1) {
	    return 1;
	}
    }

    return 0;
}
#endif

/*
 * Function:
 *	_lpm_hw_write
 * Purpose:
 *	This routine writes the blocks on the update list to H/W LPM table.
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_lpm_hw_write(int unit)
{
    _bcm_update_list_t *entry, *next_entry;
    _bcm_lpm_entry_t  *lpm_block;
    defip_hi_entry_t defip_entry;
    int lpm_blk_idx;
    uint32 next_ptr;
    int i, blk_len;
    int rv = BCM_E_NONE;

    if (l3_defip_table[unit].update_head) {
        /*
         * since h/w table need to be updated in the reverse order,
         * reverse the list first.
         */
        _lpm_reverse_update_list(unit);

        for (entry = l3_defip_table[unit].update_head;
                              entry; entry = entry->next) {

            lpm_blk_idx = entry->block;
            lpm_block = L3_LPM_ENTRY(unit, lpm_blk_idx);
            soc_cm_debug(DK_L3, "Updating Index=%d OffsetMask=0x%x\n",
                         lpm_blk_idx, entry->offsets);

            /* first block in BCM5695 is 256 entries */
            if (IS_LPM_LONG_BLOCK(unit, lpm_blk_idx)) {
                blk_len = LPM_LONG_BLK_SIZE;
            } else {
                blk_len = LPM_BLK_SIZE;
            }

            for (i = 0; i < blk_len; i++) {
                if (!lpm_block[i].dirty) {
                    continue;
                }

                sal_memset(&defip_entry, 0, sizeof(defip_hi_entry_t));
                next_ptr = 0;

                if (lpm_block[i].forward != INVALID_LPM_IDX) {
                    assert(lpm_block[i].final == 0 &&
                           lpm_block[i].valid == 1);
                    assert((lpm_block[i].forward &
                            ((0x1<<LPM_BITS_PER_BLK)-1)) == 0);
                    assert(_lpm_chk_free_blk(unit, lpm_block[i].forward));
                    next_ptr = (lpm_block[i].forward >> LPM_BITS_PER_BLK);
                } else {
                    if (lpm_block[i].final == 1 &&
                        lpm_block[i].valid == 1) {
                        /*
                         * for the entry with final(stop) and valid
                         * bits set, the next pointer should point to
                         * the L3 table index.
                         */
                        assert((lpm_block[i].l3_index >=
                                soc_mem_index_min(unit, L3Xm)) &&
                               (lpm_block[i].l3_index <=
                                soc_mem_index_max(unit, L3Xm)));

                        if (SOC_IS_DRACO1(unit)) { /* DRACO */
                            next_ptr = lpm_block[i].l3_index;
                            soc_mem_field32_set(unit, LPM_MEM(unit),
                                     &defip_entry, L3_INTF_NUMf,
                                     lpm_block[i].l3_intf);

                        } else if (SOC_IS_DRACO15(unit)) {
                            /* DRACO1.5 */
                            if (lpm_block[i].ecmp) {
                                assert(lpm_block[i].ecmp_count >= 1);

                                /* ecmp_count is # of ecmp routes - 1 */
                                next_ptr = lpm_block[i].ecmp_index & 0x7ff;
                                next_ptr |=
                                   (((lpm_block[i].ecmp_count-1) & 0x1f) << 11);
                                soc_mem_field32_set(unit, LPM_MEM(unit),
                                                    &defip_entry, ECMPf, 1);
                            } else {
                                next_ptr = lpm_block[i].l3_index;
                            }

                        } else if (SOC_IS_TUCANA(unit)) {
                            /* TUCANA */
                            if (lpm_block[i].ecmp) {
                                assert(lpm_block[i].ecmp_count >= 1);
                                /* ecmp_count is # of ecmp routes - 1 */
                                soc_mem_field32_set
                                   (unit, LPM_MEM(unit), &defip_entry, COUNTf,
                                   (lpm_block[i].ecmp_count - 1) & 0x7);
                            }
                            next_ptr = lpm_block[i].l3_index;

                        } else if (SOC_IS_LYNX(unit)) {
                            /* LYNX */
                            if (lpm_block[i].ecmp) {
                                assert(lpm_block[i].ecmp_count >= 1);
                                /* ecmp_count is # of ecmp routes - 1 */
                                soc_mem_field32_set
                                    (unit, LPM_MEM(unit),
                                    &defip_entry, ECMP_COUNTf,
                                (lpm_block[i].ecmp_count - 1) & 0x7);
                            }
                            next_ptr = lpm_block[i].l3_index;
                        } else {
                            return BCM_E_UNIT;
                        }
                    } else {
                        assert(lpm_block[i].final == 0 &&
                               lpm_block[i].valid == 0);
                    }
                }

                soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry,
                                    STOPf, lpm_block[i].final);
                soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry,
                                    VALIDf, lpm_block[i].valid);
                BCM_LPM_NEXT_PTR_SET(unit, &defip_entry, next_ptr);

                /* write to DEFIP_HIm table on ALL chips */
                rv = soc_mem_write(unit, LPM_MEM(unit), COPYNO_ALL,
                                   lpm_blk_idx + i, &defip_entry);
                if (rv < 0) {
                    break;
                }

                /* write to DEFIP_LOm table for DRACO and TUCANA */
                if (SOC_IS_DRACO1(unit) || SOC_IS_TUCANA(unit)) {
                    rv = soc_mem_write(unit, DEFIP_LOm, COPYNO_ALL,
                                       lpm_blk_idx + i, &defip_entry);
                    if (rv < 0) {
                        break;
                    }
                }

                /* reset the dirty bit */
                lpm_block[i].dirty = 0;
            }

            if (rv < 0) {
                break;
            }
        }
    } else {
        soc_cm_debug(DK_L3, "Nothing needs to updated\n");
        return rv;
    }

    /* H/W update done, delete update list */
    entry = l3_defip_table[unit].update_head;
    while (entry) {
        next_entry = entry->next;
        sal_free(entry);
        entry = next_entry;
    }

    l3_defip_table[unit].update_head = NULL;
    l3_defip_table[unit].update_tail = NULL;

    return rv;
}

/*
 * Function:
 *      _lpm_sw_ecmp_update
 * Purpose:
 *      Update the ECMP count in the s/w LPM table, add all updated entries
 *      to the update list which will be used to write to h/w LPM table.
 * Parameters:
 *      unit - SOC unit #
 *      increment - 1 to increment 0 to decrement ECMP count
 * Note:
 *	This function is recursive.
 */
STATIC void
_lpm_sw_ecmp_update(int unit, uint16 lpm_blk_idx,
		    _bcm_defip_cfg_t *lpm_cfg,
		    int start_offset, int end_offset,
		    int increment, int *result)
{
    _bcm_lpm_entry_t *lpmp;
    _bcm_backup_route_t *route;
    int prefix_lo, i;
    int found;

    lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);

    /*
     * The prefix len range for this LPM block
     */
    prefix_lo = _xgs_lpm_prefix_range(unit, lpm_blk_idx);

    for (i = start_offset; i < end_offset; i++) {
        if (!lpmp[i].valid) {
            continue;
        }

        if (lpmp[i].final) {
            if (lpmp[i].ip_addr == lpm_cfg->defip_ip_addr &&
                lpmp[i].subnet_len == lpm_cfg->defip_sub_len) {
                /*
                 * route at this offset matches route to be updated
                 */
                if (increment == 1) {
                    lpmp[i].ecmp_count++;
                } else {
                    lpmp[i].ecmp_count--;
                }
                _lpm_add_to_update_list(unit, lpm_blk_idx, i);
                *result = 1;
            } else {
                /*
                 * the default route at this offset doesn't match the route
                 * that needs to be updated, need to check the backup routes
                 * If this path is in the backup list, update it.
                 * LPM hardware need not to be updated.
                 */
                if (lpm_cfg->defip_sub_len >= prefix_lo && lpmp[i].backup_routes) {
                    found = 0;
                    for (route = lpmp[i].backup_routes; route;
			 route = route->next) {
                        if (route->subnet_len == lpm_cfg->defip_sub_len &&
                            route->ip_addr == lpm_cfg->defip_ip_addr) {
                            found = 1;
                            break;
                        }
                    }

                    if (found) {
                        if (increment == 1) {
                            route->ecmp_count++;
                        } else {
                            route->ecmp_count--;
                        }
                        *result = 1;
                    }
                }
            }
        } else { /* not final */
            assert(lpmp[i].forward != INVALID_LPM_IDX);
            _lpm_sw_ecmp_update(unit, lpmp[i].forward, lpm_cfg,
                                0, LPM_BLK_SIZE, increment, result);
        }
    }
}

/*
 * get number of bits given number of blocks
 */
STATIC INLINE int
_lpm_nybble_bits_sum(int unit, int num_blk)
{
    int nybble_sum;
    int i;

    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        nybble_sum = 4;
    } else {
        nybble_sum = 0;
    }

    for (i = 0; i <= num_blk; i++) {
        nybble_sum += LPM_BITS_PER_BLK;
    }

    return nybble_sum;
}

/*
 * get all offsets in the block that need to be updated with the new route info
 */
STATIC INLINE int
_bcm_xgs_offset_range_get(int unit, int lpm_blk_idx, int offset,
       int netlen, int num_blk, int *start_offset, int *end_offset)
{
    int num_bits, sub_mask, nybble_sum;

    nybble_sum = _lpm_nybble_bits_sum(unit, num_blk);

    if (nybble_sum == netlen) {
        *start_offset = offset;
        *end_offset = offset + 1;
    } else {
        num_bits = nybble_sum - netlen;
        if (IS_LPM_LONG_BLOCK(unit, lpm_blk_idx)) { /* first block in BCM5695 */
            assert(num_bits <= 8);
            sub_mask = ((~((0x1 << num_bits) - 1)) & ((1 << 8) - 1));
        } else {
            assert(num_bits < LPM_BITS_PER_BLK);
            sub_mask = ((~((0x1 << num_bits) - 1)) & ((1 << LPM_BITS_PER_BLK)-1));
        }

        *start_offset = offset & sub_mask;
        *end_offset = *start_offset + (1 << num_bits);
    }

    return BCM_E_NONE;
}

/*
 * Calculate the number of nibbles needed
 */
STATIC int
_lpm_num_nybbles(int unit, int subnet_len)
{
    int num_nybbles = 0;

    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        if (subnet_len <= 8) { /* first nybble */
            num_nybbles = 1;
        } else {
            num_nybbles = ((subnet_len + LPM_BITS_PER_BLK - 1) /
			   LPM_BITS_PER_BLK);
            num_nybbles--; /* one less for BCM5695 */
        }
    } else {
        num_nybbles = (subnet_len + LPM_BITS_PER_BLK - 1) / LPM_BITS_PER_BLK;
    }

    return num_nybbles;
}

/*
 * Function:
 *      _bcm_lpm_def_route_ecmp_update
 * Purpose:
 *      Update the ECMP count in the LPM table for default route(0.0.0.0/0)
 * Parameters:
 *      unit - SOC unit #
 *      lpm_cfg - config info
 *      inc_dec - 1 to increment 0 to decrement ECMP count
 * Note:
 *     BCM5665 and BCM5673/74 only
 */
STATIC int
_bcm_lpm_def_route_ecmp_update(int unit, _bcm_defip_cfg_t *lpm_cfg, int inc_dec)
{
    _bcm_lpm_entry_t *def_route;
    uint32 def_def_reg = 0;
    defip_hi_entry_t defip_entry;
    int def_route_index, count;
    int rv = BCM_E_NONE;

    def_route_index = l3_defip_table[unit].def_route_index;
    def_route = L3_LPM_ENTRY(unit, def_route_index);

    if (inc_dec == 1) {
        def_route->ecmp_count++;
    } else {
        def_route->ecmp_count--;
    }

    rv = soc_mem_read(unit, LPM_MEM(unit), MEM_BLOCK_ANY,
                 def_route_index, &defip_entry);
    BCM_IF_ERROR_RETURN(rv);
    if (SOC_IS_TUCANA(unit)) {
        count = soc_mem_field32_get(unit, LPM_MEM(unit), &defip_entry, COUNTf);
        if (inc_dec == 1) {
            soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry, COUNTf,
                                (count + 1) & 0x7);
        } else {
            soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry, COUNTf,
                                (count - 1) & 0x7);
        }
        rv = soc_mem_write(unit, LPM_MEM(unit), COPYNO_ALL,
                           def_route_index, &defip_entry);
    } else if (SOC_IS_LYNX(unit)) {
        if (soc_feature(unit, soc_feature_lynx_l3_expanded)) {
            SOC_IF_ERROR_RETURN
                (READ_ARL_DEFAULT_DEFAULT_ROUTER_IPr(unit, &def_def_reg));
            count = soc_reg_field_get(unit, ARL_DEFAULT_DEFAULT_ROUTER_IPr,
                                      def_def_reg, ECMP_COUNTf);
            if (inc_dec == 1) {
                soc_reg_field_set(unit, ARL_DEFAULT_DEFAULT_ROUTER_IPr,
                                  &def_def_reg, ECMP_COUNTf,
                                  (count + 1) & 0x7);
            } else {
                soc_reg_field_set(unit, ARL_DEFAULT_DEFAULT_ROUTER_IPr,
                                  &def_def_reg, ECMP_COUNTf,
                                  (count - 1) & 0x7);
            }
            rv = WRITE_ARL_DEFAULT_DEFAULT_ROUTER_IPr(unit, def_def_reg);
        } else {
            SOC_IF_ERROR_RETURN
                (soc_mem_read(unit, LPM_MEM(unit), MEM_BLOCK_ANY,
                              def_route_index, &defip_entry));
            count = soc_mem_field32_get(unit, LPM_MEM(unit),&defip_entry,
                                        ECMP_COUNTf);
            if (inc_dec == 1) {
                soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry,
                                    ECMP_COUNTf, (count + 1) & 0x7);
            } else {
                soc_mem_field32_set(unit, LPM_MEM(unit), &defip_entry,
                                    ECMP_COUNTf, (count - 1) & 0x7);
            }
            rv = soc_mem_write(unit, LPM_MEM(unit), COPYNO_ALL,
                               def_route_index, &defip_entry);
        }
    }

    if (rv < 0) {
        def_route->valid = 0;
        return rv;
    }

    /* if necessary, write to DEFIP_LOm table as well */
    if (SOC_IS_DRACO1(unit) || SOC_IS_TUCANA(unit)) {  /* DRACO, TUCANA */
        rv = soc_mem_write(unit, DEFIP_LOm, COPYNO_ALL,
                           def_route_index, &defip_entry);
        if (rv < 0) {
            def_route->valid = 0;

            /* clear the H/W DEFIP_HI default entry */
            sal_memset(&defip_entry, 0, sizeof(defip_hi_entry_t));
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, LPM_MEM(unit), COPYNO_ALL,
                                def_route_index, &defip_entry));
            return rv;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_lpm_ecmp_update
 * Purpose:
 *      Update the ECMP count in the LPM table
 * Parameters:
 *      unit - SOC unit #
 *      inc_dec - 1 to increment 0 to decrement ECMP count
 */
STATIC int
_bcm_lpm_ecmp_update(int unit, _bcm_defip_cfg_t *lpm_cfg, int inc_dec)
{
    _bcm_lpm_entry_t *lpmp;
    _bcm_backup_route_t *backup;
    ip_addr_t ipaddr;
    int offset, start_offset, end_offset, subnet_len;
    int lpm_blk_idx, num_nybbles;
    int is_backup = 0, result = 0;
    int i, rv = BCM_E_NONE;

    ipaddr = lpm_cfg->defip_ip_addr;
    subnet_len = lpm_cfg->defip_sub_len;

    /*
     * Walk through the blocks for the route until the last block
     * that could have the route is reached (LPM).
     */
    num_nybbles = _lpm_num_nybbles(unit, subnet_len);
    lpm_blk_idx = 0;

    for (i = 0; i < num_nybbles - 1 && (lpm_blk_idx != INVALID_LPM_IDX); i++) {
        lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
        offset = _lpm_getoffset(unit, ipaddr, i);

        if (!lpmp[offset].valid || lpmp[offset].final) {
            _print_net_addr(DK_L3, "Invalid offset or premature final for ",
                            ipaddr, subnet_len, " is found\n");
            return BCM_E_NOT_FOUND;
        }

        lpm_blk_idx = lpmp[offset].forward;
    }

    /* The last block of the LPM entry chain is for this subnet */
    if (lpm_blk_idx != INVALID_LPM_IDX) {
        lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
        offset = _lpm_getoffset(unit, ipaddr, i);
        _bcm_xgs_offset_range_get(unit, lpm_blk_idx, offset,
               subnet_len, i, &start_offset, &end_offset);

        /* To be updated route is a backup that is created here */
        for (i = start_offset; i < end_offset; i++) {
            if (lpmp[i].valid && !lpmp[i].final) {
                backup = lpmp[i].backup_routes;
                while (backup != NULL) {
                    if (backup->subnet_len == lpm_cfg->defip_sub_len) {
                        break;
                    }
                    backup = backup->next;
                }
		
		if (backup != NULL) {
		    if (inc_dec == 1) {
			backup->ecmp_count++;
		    } else {
                    	backup->ecmp_count--;
                    }
		}
                is_backup = 1;
            }
        }

        /* now build the update list for the ECMP route */
        _lpm_sw_ecmp_update(unit, lpm_blk_idx, lpm_cfg, start_offset,
                            end_offset, inc_dec, &result);
        if (!result && !is_backup) {    /* route NOT found  */
            assert(l3_defip_table[unit].update_head == NULL);
            _print_net_addr(DK_L3, "Net addr ", ipaddr, subnet_len,
                            " is NOT found\n");
            return BCM_E_NOT_FOUND;
        }
    } else {
        soc_cm_debug(DK_L3, "ERR: _bcm_lpm_ecmp_update Failed\n");
        return BCM_E_FAIL;
    }

    /* Write to LPM hardware table */
    rv = _lpm_hw_write(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_xgs_ecmp_path_add
 * Purpose:
 *      Main routine for adding new ECMP path
 * Parameters:
 *      unit    - SOC unit #
 *      lpm_cfg - the path to delete
 * Note:
 *	In order to add one path for ECMP route, the ECMP count
 *	for all the LPM entries for this subnet has to be incremented.
 *	If this route is a backup, then the backup info has to be updated.
 *	Next, the L3 table and the ECMP table needs to be updated
 *	to add this path.
 */
STATIC int
_bcm_xgs_ecmp_path_add(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    l3x_entry_t l3x_entry;
    ecmp_entry_t ecmp_entry;
    _bcm_l3hash_t *hash_entry;
    int l3_index, hash_index, ecmp_index = 0;
    int i, rv = BCM_E_NONE;

    L3_LOCK(unit);
    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        ecmp_index = lpm_cfg->defip_ecmp_index + lpm_cfg->defip_ecmp_count;

        /* Ideally, can shuffle the ECMP table to get space */
	rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY,
			  ecmp_index, &ecmp_entry);
	if (rv < 0) {
	    L3_UNLOCK(unit);
	    return rv;
	}
        if (soc_mem_field32_get(unit, L3_ECMPm, &ecmp_entry, NEXT_HOPf) != 0) {
            L3_UNLOCK(unit);
            return BCM_E_FULL;
        }

        /*
         * Add the route info to the L3 switching table using
         * nexthop IP address as the key (L3 index is returned)
         */
        rv = _bcm_xgs_l3_lpm_entry_insert(unit, lpm_cfg);
	if (rv < 0) {
	    L3_UNLOCK(unit);
	    return rv;
	}

        /* ECMP table entry points to L3 table */
        l3_index = lpm_cfg->defip_l3hw_index;
        sal_memset(&ecmp_entry, 0, sizeof(ecmp_entry_t));
        soc_mem_field32_set(unit, L3_ECMPm, &ecmp_entry, NEXT_HOPf, l3_index);
        rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL,
			   ecmp_index, &ecmp_entry);
	if (rv < 0) {
	    L3_UNLOCK(unit);
	    return rv;
	}
    } else {
        /* All ECMP paths must use the same hash offset within its bucket */
        l3_index = lpm_cfg->defip_l3hw_index +
                   (lpm_cfg->defip_ecmp_count * L3_BUCKET_SIZE);
        if (l3_index > soc_mem_index_max(unit, LPM_MEM(unit))) {
            L3_UNLOCK(unit);
            return BCM_E_FULL;
        }

        /*
         * If the entry is used, try to swap to an empty entry in the bucket.
         */
        hash_entry = &l3_table[unit].l3_hash[l3_index];
        if (hash_entry->l3h_ip_addr != 0) {
            if (hash_entry->l3h_flags & L3H_LPM) {  /* Can not swap */
                L3_UNLOCK(unit);
                return BCM_E_FULL;
            }

            hash_index = (l3_index >> 3) * L3_BUCKET_SIZE;
            for (i = 0; i < L3_BUCKET_SIZE; i++) {
                hash_entry = &l3_table[unit].l3_hash[hash_index];
                if (hash_entry->l3h_ip_addr == 0) {
                    break;
                }
                hash_index++;
            }

            /* found empty entry in bucket, swap */
            if (i < L3_BUCKET_SIZE) {
		rv = READ_L3Xm(unit, MEM_BLOCK_ANY, l3_index, &l3x_entry);
		if (rv < 0) {
		    L3_UNLOCK(unit);
		    return rv;
		}
                /* WRITE_L3Xm(unit, MEM_BLOCK_ALL, hash_index, &l3x_entry); */
		rv = _bcm_xgs_l3x_index_write(unit, hash_index, &l3x_entry);
		if (rv < 0) {
		    L3_UNLOCK(unit);
		    return rv;
		}

                l3_table[unit].l3_hash[hash_index].l3h_ip_addr =
                            l3_table[unit].l3_hash[l3_index].l3h_ip_addr;
                l3_table[unit].l3_hash[hash_index].l3h_sip_addr =
                            l3_table[unit].l3_hash[l3_index].l3h_sip_addr;
                l3_table[unit].l3_hash[hash_index].l3h_flags =
                            l3_table[unit].l3_hash[l3_index].l3h_flags;
                l3_table[unit].l3_hash[hash_index].l3h_use_count =
                            l3_table[unit].l3_hash[l3_index].l3h_use_count;
                l3_table[unit].l3_hash[hash_index].l3h_vid = 0;
            } else {
                L3_UNLOCK(unit);
                return BCM_E_FULL;
            }
        }

        /* Insert pure LPM entry (IP address LPM_L3_ADDR) */
        rv = _bcm_xgs_l3hw_add_lpm_entry(unit, l3_index, lpm_cfg);
	if (rv < 0) {
	    L3_UNLOCK(unit);
	    return rv;
	}
        _bcm_l3sw_add_lpm_entry(unit, l3_index, lpm_cfg->defip_nexthop_ip);
    }

    L3_UNLOCK(unit);

    /*
     * Walk through the LPM table and update the new ECMP count
     */
    DEFIP_LOCK(unit);
    if (!SOC_IS_DRACO15(unit) &&
        lpm_cfg->defip_ip_addr == 0 && lpm_cfg->defip_sub_len == 0) {
        rv = _bcm_lpm_def_route_ecmp_update(unit, lpm_cfg, 1);
    } else {
        rv = _bcm_lpm_ecmp_update(unit, lpm_cfg, 1);
    }
    DEFIP_UNLOCK(unit);

    /* Something is wrong, undo previous actions */
    if (rv != 0) {
        if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
            _bcm_xgs_l3_lpm_entry_del(unit, lpm_cfg);
        } else {
            _bcm_xgs_l3hw_index_del(unit, l3_index);
            l3_table[unit].l3_hash[l3_index].l3h_ip_addr = 0;
            l3_table[unit].l3_count--;
        }
        return rv;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_xgs_ecmp_path_del
 * Purpose:
 *      Main routine for deleting one of the ECMP paths
 * Parameters:
 *      unit    - SOC unit #
 *      lpm_cfg - the path to delete
 * Returns:
 *	BCM_E_XXX
 * Note:
 *   1) In order to delete one path for ECMP route, the ECMP count
 *	for all the LPM entries for this subnet has to be decremented.
 *	If this route is a backup, then the backup info has to be updated.
 *	Next, the L3 table and the ECMP table needs to be updated
 *	to remove this path.  The last path is moved up to fill up
 *	the removed one in the L3 or ECMP table.  This is done so
 *	that the L3/ECMP pointer from LPM table does not need to be changed.
 *   2) The following parameters are used to decide which ECMP path is deleted:
 *      next hop IP addr(Draco 1.5 only) | (nexthop MAC addr, L3 intf,
 *      port, modid).
 */
STATIC int
_bcm_xgs_ecmp_path_del(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    l3x_entry_t l3x_entry;
    ecmp_entry_t ecmp_entry;
    int l3_use_cnt, found;
    int l3_index, ecmp_index, ecmp_count, l3match;
    int l3_intf, port, modid, port_out, mod_out;
    bcm_mac_t nexthop_mac;
    int i, rv = BCM_E_NONE;
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
    _bcm_defip_cfg_t extarp_lpm_cfg;
#endif

    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        if (lpm_cfg->defip_nexthop_ip != 0) {
            _bcm_l3hash_t l3hash;
            sal_memset(&l3hash, 0, sizeof(l3hash));
            l3hash.l3h_ip_addr = lpm_cfg->defip_nexthop_ip;
            rv = _bcm_l3hash_index_get(unit, &l3hash, &l3match);
            if(rv == BCM_E_NOT_FOUND) {
#ifdef L3_HOST_TO_LPM_IF_L3_FULL
                sal_memset(&extarp_lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));
                extarp_lpm_cfg.defip_ip_addr = lpm_cfg->defip_nexthop_ip;
                extarp_lpm_cfg.defip_sub_len = 32;   /* host IP address */
                found = _xgs_route_lookup(unit, &extarp_lpm_cfg);
                if (!found) {
                    return BCM_E_NOT_FOUND;
                }
                l3match = extarp_lpm_cfg.defip_l3hw_index;
#else
                return BCM_E_NOT_FOUND;
#endif
            }
        }
    }

    /*
     * make sure this is a legit path before walking through LPM table
     */
    found = -1;
    ecmp_index = lpm_cfg->defip_ecmp_index;
    l3_index   = lpm_cfg->defip_l3hw_index;

    for (i = 0; i < lpm_cfg->defip_ecmp_count; i++) {
        if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
            rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY, ecmp_index, &ecmp_entry);
            BCM_IF_ERROR_RETURN(rv);
            l3_index = soc_mem_field32_get(unit, L3_ECMPm, &ecmp_entry, NEXT_HOPf);
        }

        /* Find the ECMP path to be deleted based on nexthop IP */
        if (SOC_IS_DRACO15(unit) && lpm_cfg->defip_nexthop_ip != 0) {
            if (l3match == l3_index) {
                found = i;
                break;
            }
        } else {
            /*
             * Find the ECMP path to be deleted based on
             * (nexthop MAC, L3 intf, port, MODID)
             */
            rv = READ_L3Xm(unit, MEM_BLOCK_ANY, l3_index, &l3x_entry);
            BCM_IF_ERROR_RETURN(rv);
            l3_intf = BCM_L3X_INTF_NUM_GET(unit, L3Xm, &l3x_entry);
            port    = soc_L3Xm_field32_get(unit, &l3x_entry, TGID_PORTf);
            modid   = soc_L3Xm_field32_get(unit, &l3x_entry, MODULE_IDf);
            soc_L3Xm_mac_addr_get(unit, &l3x_entry, MAC_ADDRf, nexthop_mac);

            if ((lpm_cfg->defip_flags & BCM_L3_TGID) == 0) {
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                            modid, port, &mod_out, &port_out));
            }

            if (sal_memcmp(lpm_cfg->defip_mac_addr, nexthop_mac,
                                            sizeof(bcm_mac_t)) == 0 &&
                 (l3_intf  == lpm_cfg->defip_intf) &&
                 ((((lpm_cfg->defip_flags & BCM_L3_TGID) == 0) &&
                      (port_out == lpm_cfg->defip_port_tgid) &&
                      (mod_out  == lpm_cfg->defip_modid)) ||
                 (((lpm_cfg->defip_flags & BCM_L3_TGID) != 0) &&
                  ((port & BCM_TGID_TRUNK_INDICATOR(unit)) != 0) &&
                  ((port & BCM_TGID_PORT_TRUNK_MASK(unit)) ==
                                       lpm_cfg->defip_port_tgid)))) {
                found = i;
                break;
            }
        }

        if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
            ecmp_index++;
        } else {
            l3_index += L3_BUCKET_SIZE;
        }
    }

    if (found == -1) {
        soc_cm_debug(DK_L3, "ecmp_route_delete: path does not exisit\n");
        return BCM_E_NOT_FOUND;
    }

    /*
     * Walk through the LPM table and update the new ECMP count
     */
    DEFIP_LOCK(unit);
    if (!SOC_IS_DRACO15(unit) &&
        lpm_cfg->defip_ip_addr == 0 && lpm_cfg->defip_sub_len == 0) {
        rv = _bcm_lpm_def_route_ecmp_update(unit, lpm_cfg, 0);
    } else {
        rv = _bcm_lpm_ecmp_update(unit, lpm_cfg, 0);
    }
    DEFIP_UNLOCK(unit);
    if (rv != 0) {
        return rv;
    }

    ecmp_index = lpm_cfg->defip_ecmp_index;
    ecmp_count = lpm_cfg->defip_ecmp_count - 1;

    /* Delete ECMP path from L3 table(and ECMP table for Draco 1.5) */
    L3_LOCK(unit);

    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        if (i < ecmp_count) {
            rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY,
                              ecmp_index + ecmp_count, &ecmp_entry);
            if (rv == BCM_E_NONE) {
                rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL,
                              ecmp_index + i, &ecmp_entry);
            }
        }
        if (rv == BCM_E_NONE) {
            sal_memset(&ecmp_entry, 0, sizeof(ecmp_entry_t));
            rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL,
                     ecmp_index + ecmp_count, &ecmp_entry);
        }
        if (rv < 0) {
            L3_UNLOCK(unit);
            return rv;
        }

        /* Del from L3 table if no more other ref to the L3 entry */
        _bcm_l3sw_index_del(unit, L3H_LPM, l3_index, &l3_use_cnt);
        if (l3_use_cnt == 0) {
            rv = _bcm_xgs_l3hw_index_del(unit, l3_index);
            if (rv < 0) {
                L3_UNLOCK(unit);
                return rv;
            }
        }
    } else {
        if (i < ecmp_count) {
            rv = READ_L3Xm(unit, MEM_BLOCK_ANY,
                       lpm_cfg->defip_l3hw_index + L3_BUCKET_SIZE * ecmp_count,
                       &l3x_entry);
            if (rv == BCM_E_NONE) {
                rv = _bcm_xgs_l3x_index_write(unit, l3_index, &l3x_entry);
            }
            if (rv < 0) {
                L3_UNLOCK(unit);
                return rv;
            }
        }

        l3_index = lpm_cfg->defip_l3hw_index + L3_BUCKET_SIZE * ecmp_count;
        _bcm_xgs_l3hw_index_del(unit, l3_index);
        l3_table[unit].l3_hash[l3_index].l3h_ip_addr = 0;
        l3_table[unit].l3_count--;
    }
    L3_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *	_xgs_route_lookup
 * Purpose:
 *	Search the LPM table to see if a route exist
 * Parameters:
 *      unit    - SOC unit number.
 *	lpm_cfg - (IN)specify the net/prefix of route to look for
 *                (OUT)route info
 * Returns:
 *	0 - Success, but route not found 
 *	1 - Success, found route
 */

STATIC int
_xgs_route_lookup(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_lpm_entry_t *lpmp;
    _bcm_backup_route_t *backup;
    int lpm_blk_idx, num_nybbles;
    ip_addr_t ipaddr;
    int subnet_len, offset;
    int i;

    ipaddr = lpm_cfg->defip_ip_addr;
    subnet_len = lpm_cfg->defip_sub_len;

    /* Default route case (0.0.0.0/0) */
    if (ipaddr == 0 && subnet_len == 0) {
        if (!SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
            lpmp = L3_LPM_ENTRY(unit, l3_defip_table[unit].def_route_index);
            if (!lpmp->valid) {
                return 0;
            }

            lpm_cfg->defip_l3hw_index = lpmp->l3_index;
            lpm_cfg->defip_ecmp       = lpmp->ecmp;
            lpm_cfg->defip_ecmp_count = lpmp->ecmp_count;
            lpm_cfg->defip_ecmp_index = lpmp->ecmp_index;
            return TRUE;
        }
    }
 
    /* Walk thru the LPM tree to see if this route exists */
    num_nybbles = _lpm_num_nybbles(unit, subnet_len);
    lpm_blk_idx = 0;

    for (i = 0; i < num_nybbles - 1; i++) {
        lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
        offset = _lpm_getoffset(unit, ipaddr, i);

        if (!lpmp[offset].valid || lpmp[offset].final) {
            return FALSE;
        }

        lpm_blk_idx = lpmp[offset].forward;
        if (lpm_blk_idx == INVALID_LPM_IDX) {
            return FALSE;
        }
    }

    /* The last block where this subnet ends, get the route info here */
    lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
    offset = _lpm_getoffset(unit, ipaddr, i);

    if (!lpmp[offset].valid) {
        soc_cm_debug(DK_L3, "_xgs_route_lookup fails %d\n", lpm_blk_idx + i);
        return FALSE;
    }

    if (lpmp[offset].final) {
        if (lpm_cfg->defip_ip_addr == lpmp[offset].ip_addr &&
            lpm_cfg->defip_sub_len == lpmp[offset].subnet_len) {
            lpm_cfg->defip_l3hw_index = lpmp[offset].l3_index;
            lpm_cfg->defip_ecmp       = lpmp[offset].ecmp;
            lpm_cfg->defip_ecmp_count = lpmp[offset].ecmp_count;
            lpm_cfg->defip_ecmp_index = lpmp[offset].ecmp_index;

            return TRUE;
        }
    }

    /* Check the backup route list */
    backup = lpmp[offset].backup_routes;
    while (backup != NULL) {
        if (backup->subnet_len == lpm_cfg->defip_sub_len &&
               backup->ip_addr == lpm_cfg->defip_ip_addr) {
            lpm_cfg->defip_l3hw_index = backup->l3_index;
            lpm_cfg->defip_ecmp       = backup->ecmp;
            lpm_cfg->defip_ecmp_count = backup->ecmp_count;
            lpm_cfg->defip_ecmp_index = backup->ecmp_index;

            return TRUE;
        }
        backup = backup->next;
    }

    return FALSE;
}

/*
 * Clear the LPM table HIT bit for the entry at index
 */
STATIC int
_lpm_index_hit_set(int unit, int index, int set_value)
{
    defip_hit_hi_entry_t  defip_hit_entry;
    int blk, hit_val;
    int rv;
 
    SOC_MEM_BLOCK_ITER(unit, LPMHIT_MEM(unit), blk) {
        sal_memset(&defip_hit_entry, 0, sizeof(defip_hit_hi_entry_t));
        rv = soc_mem_read(unit, LPMHIT_MEM(unit), blk,
                          index >> 3, &defip_hit_entry);
        BCM_IF_ERROR_RETURN(rv);
        hit_val = soc_mem_field32_get(unit, LPMHIT_MEM(unit),
                            &defip_hit_entry, BUCKET_BITMAPf);
        if (set_value) {
            hit_val |= (1 << (index & L3_BUCKET_OFFSET_BMP));
        } else {
            hit_val &= ~(1 << (index & L3_BUCKET_OFFSET_BMP));
        }
        soc_mem_field32_set(unit, LPMHIT_MEM(unit), &defip_hit_entry,
                            BUCKET_BITMAPf, hit_val);
        rv = soc_mem_write(unit, LPMHIT_MEM(unit), blk,
                           index >> 3, &defip_hit_entry);
        BCM_IF_ERROR_RETURN(rv);
    }

    if (SOC_IS_DRACO(unit) || SOC_IS_TUCANA(unit)) { /* DRACO, TUCANA */
        SOC_MEM_BLOCK_ITER(unit, DEFIP_HIT_LOm, blk) { 
            sal_memset(&defip_hit_entry, 0, sizeof(defip_hit_hi_entry_t));
            rv = soc_mem_read(unit, DEFIP_HIT_LOm, blk, index >> 3, &defip_hit_entry);
            BCM_IF_ERROR_RETURN(rv);
            hit_val = soc_mem_field32_get(unit, DEFIP_HIT_LOm,
                                &defip_hit_entry, BUCKET_BITMAPf);
            if (set_value) {
                hit_val |= (1 << (index & L3_BUCKET_OFFSET_BMP));
            } else {
                hit_val &= ~(1 << (index & L3_BUCKET_OFFSET_BMP));
            }
            soc_mem_field32_set(unit, DEFIP_HIT_LOm, &defip_hit_entry,
                                BUCKET_BITMAPf, hit_val);
            rv = soc_mem_write(unit, DEFIP_HIT_LOm, blk,
                          index >> 3, &defip_hit_entry);
            BCM_IF_ERROR_RETURN(rv);
        }
    }
 
    return BCM_E_NONE;
}
 
/*
 * Get the LPM table HIT bit for the entry at index
 */
STATIC int
_lpm_index_hit_get(int unit, int index, int *hit)
{
    defip_hit_hi_entry_t  defip_hit_entry;
    int blk, hit_val;
    int rv;
 
    *hit = 0;
 
    SOC_MEM_BLOCK_ITER(unit, LPMHIT_MEM(unit), blk) {
        sal_memset(&defip_hit_entry, 0, sizeof(defip_hit_hi_entry_t));
        rv = soc_mem_read(unit, LPMHIT_MEM(unit), blk,
                     index >> 3, &defip_hit_entry);
        BCM_IF_ERROR_RETURN(rv);
        hit_val = soc_mem_field32_get(unit, LPMHIT_MEM(unit),
                            &defip_hit_entry, BUCKET_BITMAPf);
        if (hit_val & (1 << (index & L3_BUCKET_OFFSET_BMP))) {
            *hit = 1;
            return BCM_E_NONE;
        }
    }

    if (SOC_IS_DRACO(unit) || SOC_IS_TUCANA(unit)) { /* DRACO, TUCANA */
        SOC_MEM_BLOCK_ITER(unit, DEFIP_HIT_LOm, blk) { 
            sal_memset(&defip_hit_entry, 0, sizeof(defip_hit_hi_entry_t));
            rv = soc_mem_read(unit, DEFIP_HIT_LOm, blk,
                         index >> 3, &defip_hit_entry);
            BCM_IF_ERROR_RETURN(rv);
            hit_val = soc_mem_field32_get(unit, DEFIP_HIT_LOm,
                                &defip_hit_entry, BUCKET_BITMAPf);
            if (hit_val & (1 << (index & L3_BUCKET_OFFSET_BMP))) {
                *hit = 1;
                return BCM_E_NONE;
            }
        }
    }
 
    return BCM_E_NONE;
}

/*
 * Walk through the LPM entries to get the HIT bit (recursive)
 */
STATIC void
_lpm_hit_get(int unit, uint16 lpm_blk_idx, _bcm_defip_cfg_t *lpm_cfg,
             int start_offset, int end_offset)
{
    _bcm_lpm_entry_t *lpmp;
    int i;
    int hit = 0;

    lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);

    for (i = start_offset; i < end_offset; i++) {
        if (!lpmp[i].valid) {
            continue;
        }

        if (lpmp[i].final) {
            if (lpmp[i].ip_addr == lpm_cfg->defip_ip_addr &&
                lpmp[i].subnet_len == lpm_cfg->defip_sub_len) {
                /*
                 * route at this offset matches network of interest 
                 * read the HIT bit from LPM table (DEFIP_LOm & DEFIP_HIm)
                 */
                _lpm_index_hit_get(unit, lpm_blk_idx + i, &hit);
                if (hit == 1) {
                    lpm_cfg->defip_flags |= BCM_L3_HIT;
                    return;
                }
            }
        } else {  /* not final */
            assert(lpmp[i].forward != INVALID_LPM_IDX);
            _lpm_hit_get(unit, lpmp[i].forward, lpm_cfg, 0, LPM_BLK_SIZE);
        }

        if (lpm_cfg->defip_flags & BCM_L3_HIT) {
            break;
        }
    }
}

/*
 * Walk through the LPM entries to clear the HIT bit (recursive)
 */
STATIC void
_lpm_hit_set(int unit, uint16 lpm_blk_idx, _bcm_defip_cfg_t *lpm_cfg,
             int start_offset, int end_offset, int set_value)
{
    _bcm_lpm_entry_t *lpmp;
    int i;
 
    lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);

    for (i = start_offset; i < end_offset; i++) {
        if (!lpmp[i].valid) {
            continue;
        }

        if (lpmp[i].final) {
            if (lpmp[i].ip_addr == lpm_cfg->defip_ip_addr &&
                lpmp[i].subnet_len == lpm_cfg->defip_sub_len) {
                /*
                 * route at this offset matches network of interest
                 * read the HIT bit from LPM table (DEFIP_HIT_LOm & DEFIP_HIT_HIm)
                 */
                _lpm_index_hit_set(unit, lpm_blk_idx + i, set_value);
            }
        } else {  /* not final */
            assert(lpmp[i].forward != INVALID_LPM_IDX);
            _lpm_hit_set(unit, lpmp[i].forward, lpm_cfg,
                         0, LPM_BLK_SIZE, set_value);
        }
    }
}
 
/*
 * Function:
 *      _bcm_xgs_lpm_do_hit
 * Purpose:
 *      Get/Clear the Hit bit for a network from LPM table
 * Parameters:
 *      unit    - SOC unit number.
 *      lpm_cfg - (IN)network/prefix, (OUT)HIT bit flag
 *      action  - 2 - get, 1 - set, 0 - clear hit
 */
STATIC int
_bcm_xgs_lpm_do_hit(int unit, _bcm_defip_cfg_t *lpm_cfg, int action)
{
    _bcm_lpm_entry_t *lpmp;
    ip_addr_t ipaddr;
    int offset, start_offset, end_offset, subnet_len;
    int lpm_blk_idx, num_nybbles;
    int i;
 
    ipaddr = lpm_cfg->defip_ip_addr;
    subnet_len = lpm_cfg->defip_sub_len;
 
    /* Default route case (0.0.0.0/0), there is no HIT bit for default route */
    if (ipaddr == 0 && subnet_len == 0) {
        return BCM_E_NONE;
    }
 
    /*
     * Walk through the blocks for the route until the last block
     * that could have the route is reached (LPM).
     */
    num_nybbles = _lpm_num_nybbles(unit, subnet_len);
    lpm_blk_idx = 0;
 
    for (i = 0; i < num_nybbles - 1 && (lpm_blk_idx != INVALID_LPM_IDX); i++) {
        lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
        offset = _lpm_getoffset(unit, ipaddr, i);

        if (!lpmp[offset].valid || lpmp[offset].final) {
            _print_net_addr(DK_L3, "Invalid offset or premature final for ",
                            ipaddr, subnet_len, " is found\n");
            return BCM_E_NOT_FOUND;
        }
 
        lpm_blk_idx = lpmp[offset].forward;
    }
 
    /* The last block of the LPM entry chain is for this subnet */
    if (lpm_blk_idx != INVALID_LPM_IDX) {
        lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
        offset = _lpm_getoffset(unit, ipaddr, i);
        _bcm_xgs_offset_range_get(unit, lpm_blk_idx, offset,
                                  subnet_len, i, &start_offset, &end_offset);

        /* Walk through the LPM entries to get/clear the HIT bit (recursive) */
        if (action == 2) {
            _lpm_hit_get(unit, lpm_blk_idx, lpm_cfg, start_offset, end_offset);
        } else if (action == 1) {
            _lpm_hit_set(unit, lpm_blk_idx, lpm_cfg, start_offset, end_offset, 1);
        } else {
            _lpm_hit_set(unit, lpm_blk_idx, lpm_cfg, start_offset, end_offset, 0);
        }
    }
 
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_xgs_defip_cfg_get
 * Purpose:
 *      Get route information from DEFIP table
 * Parameters:
 *      unit - SOC unit number.
 *      lpm_cfg - Pointer to memory for DEFIP table related information.
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      If the route is local, then the BCM_L3_DEFIP_LOCAL flag is
 *      set, and next hop MAC, port/tgid, interface, MODID fields
 *      are not set, and should not be referenced.
 */
int
bcm_xgs_defip_cfg_get(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_l3_cfg_t l3cfg;
    ecmp_entry_t ecmp_entry;
    int clear_hit;
    int rv = 0;

    sal_memset(&l3cfg, 0, sizeof(l3cfg));

    clear_hit = lpm_cfg->defip_flags & BCM_L3_HIT_CLEAR;

    /* Check to see if the net/prefix exists */
    if (SOC_IS_DRACO15(unit) &&  /* Draco 1.5 */
        lpm_cfg->defip_ip_addr == 0 && lpm_cfg->defip_sub_len == 0 &&
        !(lpm_cfg->defip_flags & (BCM_L3_MULTIPATH | BCM_L3_LPM_DEFROUTE))) {
        if (l3_defip_table[unit].global_def_route_set) {
            lpm_cfg->defip_l3hw_index = L3_D15_GLOBAL_DEF_ROUTE_IDX;
            lpm_cfg->defip_ecmp       = 0;
            lpm_cfg->defip_ecmp_count = 0;
            rv = 1;
        }
    } else {
        rv = _xgs_route_lookup(unit, lpm_cfg);
    }

    if (rv == 0) {
        soc_cm_debug(DK_L3, "bcm_xgs_defip_cfg_get route not found\n");
        return (BCM_E_NOT_FOUND);
    }

    /*
     * Get detailed info of the route from the L3 table
     * (for multi-path routes, only the first path)
     */
    if (lpm_cfg->defip_ecmp && SOC_IS_DRACO15(unit)) {
        /* For BCM5695 ECMP route, always depend on ECMP index */
        rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY,
                     lpm_cfg->defip_ecmp_index, &ecmp_entry);
        BCM_IF_ERROR_RETURN(rv);
        l3cfg.l3c_hw_index =
            soc_mem_field32_get(unit, L3_ECMPm, &ecmp_entry, NEXT_HOPf);
    } else {
        l3cfg.l3c_hw_index = lpm_cfg->defip_l3hw_index;
    }

    /* If LPM points to local route L3 entry */
    if ((uint32)l3cfg.l3c_hw_index == L3_DEFIP_LOCAL_ROUTE_IDX(unit)) {
        lpm_cfg->defip_flags = BCM_L3_DEFIP_LOCAL;
        sal_memcpy(lpm_cfg->defip_mac_addr, _soc_mac_all_zeroes,
                   sizeof(sal_mac_addr_t));
        lpm_cfg->defip_port_tgid = 0;
        lpm_cfg->defip_intf = 0;
        lpm_cfg->defip_modid = 0;
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN
        (bcm_xgs_l3_get_by_index(unit, l3cfg.l3c_hw_index, &l3cfg));

    sal_memcpy(lpm_cfg->defip_mac_addr, l3cfg.l3c_mac_addr,
               sizeof(sal_mac_addr_t));
    lpm_cfg->defip_flags = l3cfg.l3c_flags;
    lpm_cfg->defip_port_tgid = l3cfg.l3c_port_tgid;
    lpm_cfg->defip_intf = l3cfg.l3c_intf;
    lpm_cfg->defip_modid = l3cfg.l3c_modid;
    lpm_cfg->defip_nexthop_ip = l3cfg.l3c_ip_addr;
    if (lpm_cfg->defip_ecmp) {  /* ECMP route */
        lpm_cfg->defip_flags = l3cfg.l3c_flags | BCM_L3_MULTIPATH;
    }

    /*
     * Get the Hit bit from LPM table, if the route for this
     * network spreads in many LPM table entries, then if HIT bit
     * is set in any of these entries, we return hit bit set.
     */
    lpm_cfg->defip_flags &= ~BCM_L3_HIT;
    rv = _bcm_xgs_lpm_do_hit(unit, lpm_cfg, 2);
    if (rv < 0) {
        soc_cm_debug(DK_L3, "Error getting LPM HIT bit\n");
        return rv;
    }

    /* Clear the HIT bit */
    if (clear_hit && (lpm_cfg->defip_flags & BCM_L3_HIT)) {
        rv = _bcm_xgs_lpm_do_hit(unit, lpm_cfg, 0);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs_defip_ecmp_get_all
 * Purpose:
 *      Get all paths for a route, useful for ECMP route
 * Parameters:
 *      unit       - (IN) SOC device unit number
 *      the_route  - (IN) route's net/mask
 *      path_array - (OUT) Array of all ECMP paths
 *      max_path   - (IN) Max number of ECMP paths
 *      path_count - (OUT) Actual number of ECMP paths
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_defip_ecmp_get_all(int unit, _bcm_defip_cfg_t *lpm_cfg,
        bcm_l3_route_t *path_array, int max_path, int *path_count)
{
    _bcm_l3_cfg_t l3cfg;
    ecmp_entry_t ecmp_entry;
    l3x_entry_t l3x_entry;
    int ecmp_index, l3_index;
    int rv;
    int i;

    *path_count = 0;
    sal_memset(&l3cfg, 0, sizeof(l3cfg));


    /* Draco1.5 default route mess */
    if (SOC_IS_DRACO15(unit) &&
        lpm_cfg->defip_ip_addr == 0 && lpm_cfg->defip_sub_len == 0 &&
        !(lpm_cfg->defip_flags & (BCM_L3_MULTIPATH | BCM_L3_LPM_DEFROUTE))) {
        if (l3_defip_table[unit].global_def_route_set) {
            BCM_IF_ERROR_RETURN(bcm_xgs_l3_get_by_index(unit,
                                L3_D15_GLOBAL_DEF_ROUTE_IDX, &l3cfg));
            sal_memcpy(path_array[0].l3a_nexthop_mac, l3cfg.l3c_mac_addr,
                       sizeof(sal_mac_addr_t));
            path_array[0].l3a_subnet = 0;
            path_array[0].l3a_ip_mask = 0;
            path_array[0].l3a_nexthop_ip = 0;
            path_array[0].l3a_intf = l3cfg.l3c_intf;
            path_array[0].l3a_port_tgid = l3cfg.l3c_port_tgid;
            path_array[0].l3a_modid = l3cfg.l3c_modid;
            path_array[0].l3a_flags = 0;
            *path_count = 1;
            return BCM_E_NONE;
        }
    }

    /* Check to see if the net/prefix exists */
    if (_xgs_route_lookup(unit, lpm_cfg) == 0) {
        soc_cm_debug(DK_L3, "bcm_xgs_defip_ecmp_get_all route not found\n");
        return (BCM_E_NOT_FOUND);
    }

    /*
     * Get detailed info of the route from the L3 table
     */
    if (lpm_cfg->defip_ecmp) {
        ecmp_index = lpm_cfg->defip_ecmp_index;
        l3_index   = lpm_cfg->defip_l3hw_index;

        for (i = 0; i < lpm_cfg->defip_ecmp_count && i < max_path; i++) {
            if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
                rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY,
                                          ecmp_index, &ecmp_entry);
                BCM_IF_ERROR_RETURN(rv);
                l3_index = soc_mem_field32_get(unit, L3_ECMPm,
                                          &ecmp_entry, NEXT_HOPf);
            }

            rv = READ_L3Xm(unit, MEM_BLOCK_ANY, l3_index, &l3x_entry);
            BCM_IF_ERROR_RETURN(rv);

            path_array[i].l3a_subnet = lpm_cfg->defip_ip_addr;
            path_array[i].l3a_ip_mask =
                        BCM_IP4_MASKLEN_TO_ADDR(lpm_cfg->defip_sub_len);
            soc_L3Xm_mac_addr_get(unit, &l3x_entry, MAC_ADDRf,
                                  path_array[i].l3a_nexthop_mac);
            path_array[i].l3a_nexthop_ip =
                      soc_L3Xm_field32_get(unit, &l3x_entry, IP_ADDRf);
            path_array[i].l3a_intf =
                      BCM_L3X_INTF_NUM_GET(unit, L3Xm, &l3x_entry);
            path_array[i].l3a_port_tgid =
                      soc_L3Xm_field32_get(unit, &l3x_entry, TGID_PORTf);
            path_array[i].l3a_modid =
                      soc_L3Xm_field32_get(unit, &l3x_entry, MODULE_IDf);
            path_array[i].l3a_flags = BCM_L3_MULTIPATH;

            if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
                ecmp_index++;
            } else {
                l3_index += L3_BUCKET_SIZE;
            }
        }
        *path_count = i;
    } else {
        l3cfg.l3c_hw_index = lpm_cfg->defip_l3hw_index;
        BCM_IF_ERROR_RETURN
            (bcm_xgs_l3_get_by_index(unit, l3cfg.l3c_hw_index, &l3cfg));
        sal_memcpy(path_array[0].l3a_nexthop_mac, l3cfg.l3c_mac_addr,
                   sizeof(sal_mac_addr_t));
        path_array[0].l3a_subnet = lpm_cfg->defip_ip_addr;
        path_array[0].l3a_ip_mask = BCM_IP4_MASKLEN_TO_ADDR(lpm_cfg->defip_sub_len);
        path_array[0].l3a_nexthop_ip = 0;
        path_array[0].l3a_intf = l3cfg.l3c_intf;
        path_array[0].l3a_port_tgid = l3cfg.l3c_port_tgid;
        path_array[0].l3a_modid = l3cfg.l3c_modid;
        path_array[0].l3a_flags = 0;
        *path_count = 1;
        return BCM_E_NONE;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs_defip_add
 * Purpose:
 *      Add IP route to LPM table
 * Parameters:
 *      unit - SOC unit number.
 *      lpm_cfg - Pointer to memory for DEFIP table related information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_defip_add(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    int max_ecmp, found;

    /* Only BCM5695 supports per VLAN based default route */
    if (SOC_IS_DRACO15(unit) &&  /* Draco 1.5 */
        lpm_cfg->defip_ip_addr == 0 &&
        lpm_cfg->defip_sub_len == 0 &&
        !(lpm_cfg->defip_flags & (BCM_L3_MULTIPATH | BCM_L3_LPM_DEFROUTE))) {
        return (_bcm_xgs_per_vlan_def_route_add(unit, lpm_cfg));
    } else {
        lpm_cfg->defip_vid = 0;
    }

    if (lpm_cfg->defip_nexthop_ip == 0 &&
        !(lpm_cfg->defip_flags & BCM_L3_DEFIP_LOCAL)) {
        soc_cm_debug(DK_L3, "Adding routes for XGS requires nexthop IP addr\n");
        return BCM_E_PARAM;
    }

    /* Check to see if the net/prefix exists */
    found = _xgs_route_lookup(unit, lpm_cfg);

    max_ecmp = l3_defip_table[unit].ecmp_max_paths;

    /*
     * If adding new paths to ECMP route, add new paths
     * and update ecmp count in DEFIP table.
     */
    if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH) {
        /*
         * ECMP route addition
         */
        if (SOC_IS_DRACO1(unit)) {
            return (BCM_E_UNAVAIL);
	}

        if (lpm_cfg->defip_flags & BCM_L3_DEFIP_LOCAL) {
            soc_cm_debug(DK_L3, "Conflicting flags set\n");
            return BCM_E_PARAM;
        }

	if (found && !lpm_cfg->defip_ecmp) {
            soc_cm_debug(DK_L3, "Previous path not ECMP route\n");
            return (BCM_E_PARAM);
        }

        if (found) {
            /* Add additional ECMP path */
            if (lpm_cfg->defip_ecmp_count >= max_ecmp) {
                soc_cm_debug(DK_L3, "Too many ECMP paths(MAX %d)\n", max_ecmp);
                return (BCM_E_FULL);
            }
            soc_cm_debug(DK_L3, "add more ECMP path\n");
            BCM_IF_ERROR_RETURN
                (_bcm_xgs_ecmp_path_add(unit, lpm_cfg));
        } else {
            /* Add the first ECMP path */
            soc_cm_debug(DK_L3, "add new ECMP route\n");
            BCM_IF_ERROR_RETURN
                (_bcm_xgs_defip_do_add(unit, lpm_cfg));
            L3_INFO(unit)->defip_count++;

            return (BCM_E_NONE);
        }
    } else {
        /*
         * Single route addition
         */
        if (found) {      /* Route exists */
            soc_cm_debug(DK_L3, "Route exists\n");
            return (BCM_E_EXISTS);
        }

        /* Insert route to the LPM tables */
        soc_cm_debug(DK_L3, "DEFIP add non-ecmp route 0x%x/%d\n", 
             lpm_cfg->defip_ip_addr, lpm_cfg->defip_sub_len);
        BCM_IF_ERROR_RETURN
	    (_bcm_xgs_defip_do_add(unit, lpm_cfg));

        L3_INFO(unit)->defip_count++;

        return (BCM_E_NONE);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs_defip_del
 * Purpose:
 *      Del IP route to LPM table
 * Parameters:
 *      unit - SOC unit number.
 *      lpm_cfg - Pointer to memory for DEFIP table related information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_defip_del(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    int rv;

    /* Only BCM5695 supports per VLAN based default route */
    if (SOC_IS_DRACO15(unit) &&  /* Draco 1.5 */
        lpm_cfg->defip_ip_addr == 0 &&
        lpm_cfg->defip_sub_len == 0 &&
        !(lpm_cfg->defip_flags & (BCM_L3_MULTIPATH | BCM_L3_LPM_DEFROUTE))) {
        return (_bcm_xgs_per_vlan_def_route_del(unit, lpm_cfg->defip_vid));
    } else {
        lpm_cfg->defip_vid = 0;
    }

    /*
     * check to see if route exists, do nothing if not
     */
    if (L3_INFO(unit)->defip_count == 0) {
        return (BCM_E_NOT_FOUND);
    }

    /* Check to see if the net/prefix exists */
    rv = _xgs_route_lookup(unit, lpm_cfg);
    if (rv == 0) {
        return (BCM_E_NOT_FOUND);
    }

    if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH) {
        /*
         * ECMP path deletion
         */
        if (SOC_IS_DRACO1(unit) || !lpm_cfg->defip_ecmp) {
            soc_cm_debug(DK_L3, "DEFIP delete error\n");
            return (BCM_E_PARAM);
        }

        if (lpm_cfg->defip_ecmp_count > 1) { /* del one of the multi-paths */
            soc_cm_debug(DK_L3, "DEFIP del one of ECMP paths\n");
            BCM_IF_ERROR_RETURN
		(_bcm_xgs_ecmp_path_del(unit, lpm_cfg));

        } else {  /* delete the last path of multi-path route */
            /* Delete route from the LPM tables */
            soc_cm_debug(DK_L3, "DEFIP del ECMP route\n");
            BCM_IF_ERROR_RETURN
		(_bcm_xgs_defip_do_del(unit, lpm_cfg));

            L3_INFO(unit)->defip_count--;
        }
    } else {
        /*
         * Single route deletion or delete all ECMP paths
         */
        BCM_IF_ERROR_RETURN
            (_bcm_xgs_defip_do_del(unit, lpm_cfg));

        L3_INFO(unit)->defip_count--;
    }

    return (BCM_E_NONE);
}

/*
 * See if the net is in the block between 0 to index
 */
STATIC int
_xgs_route_done(_bcm_lpm_entry_t *lpmp, int index, ip_addr_t ipaddr, int netlen)
{
    _bcm_backup_route_t *cur;
    int i;

    for (i = 0; i < index; i++) {
        if (!lpmp[i].valid) {
                continue;
        }

        if (lpmp[i].final) {
            if (lpmp[i].ip_addr == ipaddr && lpmp[i].subnet_len == netlen) {
                return 1;
            }
        }

        for (cur = lpmp[i].backup_routes; cur; cur = cur->next) {
            if (cur->ip_addr == ipaddr && cur->subnet_len == netlen) {
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Function:
 *      _xgs_route_traverse
 * Purpose:
 *      Traverse all installed routes, and call the call back function
 *      at each installed route once.
 * Parameters:
 *      unit      - SOC unit number.
 *      cb_fn     - the function to be called for each route found
 *      trav_data - data supplied to call back function
 * Note:
 *      This function enumerate all routes that are related to LPM table,
 *      i.e. this does NOT include all the per VLAN default routes
 *      in BCM5695.  Therefore, all the per-VLAN default routes must
 *      be treated as special case following the function call to
 *      _xgs_route_traverse(), whenever this got called.
 */
STATIC int
_xgs_route_traverse(int unit, route_trav_cb_fn cb_fn, void *trav_data)
{
    _bcm_backup_route_t *cur, *next;
    _bcm_lpm_entry_t *lpmp;
    _bcm_lpm_entry_t lpm_entry;
    int blk_size, blk;
    int prefix_lo, i;

    /*
     * Go through all the LPM table blocks to find inserted routes
     */
    for (blk = l3_defip_table[unit].lpm_min;
         blk < l3_defip_table[unit].lpm_max; blk += blk_size) {
        lpmp = L3_LPM_ENTRY(unit, blk);

        /* The prefix len range for this LPM block */
        prefix_lo = _xgs_lpm_prefix_range(unit, blk);

        if (IS_LPM_LONG_BLOCK(unit, blk)) {
            blk_size = LPM_LONG_BLK_SIZE; /* first block in BCM5695 */
        } else {
            blk_size = LPM_BLK_SIZE;
        }

        for (i = 0; i < blk_size; i++) {
            if (!lpmp[i].valid) {
                continue;
            }

            if (lpmp[i].final) {
                if (lpmp[i].subnet_len >= prefix_lo) {
                    /* Have we seen this route in this block ? */
                    if (!_xgs_route_done(lpmp, i,
                                    lpmp[i].ip_addr, lpmp[i].subnet_len)) {
                        lpm_entry.ip_addr    = lpmp[i].ip_addr;
                        lpm_entry.subnet_len = lpmp[i].subnet_len;
                        lpm_entry.l3_index   = lpmp[i].l3_index;
                        lpm_entry.l3_intf    = lpmp[i].l3_intf;
                        lpm_entry.ecmp       = lpmp[i].ecmp;
                        lpm_entry.ecmp_count = lpmp[i].ecmp_count;
                        lpm_entry.ecmp_index = lpmp[i].ecmp_index;
                        if (cb_fn(unit, &lpm_entry, trav_data) < 0) {
                            return BCM_E_NONE;
                        }
                    }
                }
            }

            for (cur = lpmp[i].backup_routes; cur; cur = next) {
                next = cur->next;  /* cur might be del'ed in callback */
                if (cur->subnet_len >= prefix_lo) {
                    if (!_xgs_route_done(lpmp, i,
                                    cur->ip_addr, cur->subnet_len)) {
                        lpm_entry.ip_addr    = cur->ip_addr;
                        lpm_entry.subnet_len = cur->subnet_len;
                        lpm_entry.l3_index   = cur->l3_index;
                        lpm_entry.l3_intf    = cur->l3_intf;
                        lpm_entry.ecmp       = cur->ecmp;
                        lpm_entry.ecmp_count = cur->ecmp_count;
                        lpm_entry.ecmp_index = cur->ecmp_index;
                        if (cb_fn(unit, &lpm_entry, trav_data) < 0) {
                            return BCM_E_NONE;
                        }
                    }
                }
            }
        }
    }

    /* Check default route */
    if (!SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        lpmp = L3_LPM_ENTRY(unit, l3_defip_table[unit].def_route_index);
        if (lpmp->valid) {
            lpm_entry.ip_addr    = 0;
            lpm_entry.subnet_len = 0;
            lpm_entry.l3_index   = lpmp->l3_index;
            lpm_entry.l3_intf    = lpmp->l3_intf;
            lpm_entry.ecmp       = lpmp->ecmp;
            lpm_entry.ecmp_count = lpmp->ecmp_count;
            lpm_entry.ecmp_index = lpmp->ecmp_index;
            if (cb_fn(unit, &lpm_entry, trav_data) < 0) {
                return BCM_E_NONE;
            }
        }
    }

    return (BCM_E_NONE);
}

/*
 * The main function for call abck function in delete by interface
 * negate - 0 means interface match; 1 means not match
 */
STATIC int
_xgs_defip_del_intf_do_cb(int unit, _bcm_lpm_entry_t *lpmp,
                               int intf, int negate)
{
    _bcm_defip_cfg_t lpm_cfg;
    ecmp_entry_t ecmp_entry;
    l3x_entry_t l3x_entry;
    _bcm_defip_cfg_t *ecmp_del_list=NULL;
    int l3_index, del_total, i;
    int l3_use_cnt;
    bcm_if_t l3_intf_num;
    int rv;

    /* Check for ECMP multi-path route */
    if (lpmp->ecmp && lpmp->ecmp_count > 1) {
        /*
         * Must first copy the ECMP paths to be deleted, can not
         * delete in place because delete changes lpmp->ecmp_count.
         */
        del_total = 0;
        ecmp_del_list =
            sal_alloc (sizeof(_bcm_defip_cfg_t) * 32, "local_ecmp_del_list");
        if (!ecmp_del_list) {
            return BCM_E_MEMORY;
        }

        for (i = lpmp->ecmp_count - 1; i >= 0; i--) {
            if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
                rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY,
                             lpmp->ecmp_index + i, &ecmp_entry);
                if(BCM_FAILURE(rv)){
                   sal_free(ecmp_del_list);
                   return rv;
                }
                l3_index = soc_mem_field32_get(unit, L3_ECMPm,
                                               &ecmp_entry, NEXT_HOPf);
            } else {
                l3_index = lpmp->l3_index + (i * L3_BUCKET_SIZE);
            }
            rv = READ_L3Xm(unit, MEM_BLOCK_ANY, l3_index, &l3x_entry);
            if(BCM_FAILURE(rv)){
                sal_free(ecmp_del_list);
                return rv;
            }

            l3_intf_num = BCM_L3X_INTF_NUM_GET(unit, L3Xm, &l3x_entry);
            if ((!negate && l3_intf_num == intf) ||
                (negate && l3_intf_num != intf)) {
                ecmp_del_list[del_total].defip_ip_addr = lpmp->ip_addr;
                ecmp_del_list[del_total].defip_sub_len = lpmp->subnet_len;
                ecmp_del_list[del_total].defip_ecmp_index = lpmp->ecmp_index;
                ecmp_del_list[del_total].defip_l3hw_index = lpmp->l3_index;

                soc_L3Xm_mac_addr_get(unit, &l3x_entry, MAC_ADDRf,
                                      ecmp_del_list[del_total].defip_mac_addr);
                ecmp_del_list[del_total].defip_nexthop_ip = 0;
                ecmp_del_list[del_total].defip_intf      = -1;
                ecmp_del_list[del_total].defip_port_tgid = -1;
                ecmp_del_list[del_total].defip_modid     = -1;
                ecmp_del_list[del_total].defip_ecmp_count =
                                   lpmp->ecmp_count - del_total;
                del_total++;
            }
        }

        /*
         * Do the actual ECMP paths deletion
         */
        if (del_total == 0) {
            return BCM_E_NONE;
        } else if (del_total < lpmp->ecmp_count) {
            for (i = 0; i < del_total; i++) {
                rv = _bcm_xgs_ecmp_path_del(unit, &ecmp_del_list[i]);
                if (BCM_FAILURE(rv)){
                   sal_free(ecmp_del_list);
                   return rv;
                }
            }
            return BCM_E_NONE;
        } else if (del_total == lpmp->ecmp_count) {
            /* deleting all the ECMP paths means deleting it from LPM table */
            for (i = 1; i < del_total; i++) {
                if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
                    rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY,
                                 lpmp->ecmp_index + i, &ecmp_entry);
                    if (BCM_FAILURE(rv)){
                        sal_free(ecmp_del_list);
                        return rv;
                    }

                    l3_index = soc_mem_field32_get(unit, L3_ECMPm,
                                               &ecmp_entry, NEXT_HOPf);
                    sal_memset(&ecmp_entry, 0, sizeof(ecmp_entry_t));
                    rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL,
                                  lpmp->ecmp_index + i, &ecmp_entry);
                    if (BCM_FAILURE(rv)){
                        sal_free(ecmp_del_list);
                        return rv;
                    }

                    /* Del from L3 table if no more other ref to the L3 entry */
                    _bcm_l3sw_index_del(unit, L3H_LPM, l3_index, &l3_use_cnt);
                    if (l3_use_cnt == 0) {
                        _bcm_xgs_l3hw_index_del(unit, l3_index);
                    }
                } else {
                    l3_index = lpmp->l3_index + (i * L3_BUCKET_SIZE);
                    _bcm_xgs_l3hw_index_del(unit, l3_index);
                    l3_table[unit].l3_hash[l3_index].l3h_ip_addr = 0;
                    l3_table[unit].l3_count--;
                }
            }
        }
        if (ecmp_del_list) {
            sal_free(ecmp_del_list);
        }
    } else {
        if (lpmp->ecmp) {
            /* For ECMP, we can not rely on lpmp->intf */
            if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
                rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY,
                             lpmp->ecmp_index, &ecmp_entry);
                BCM_IF_ERROR_RETURN(rv);
                l3_index = soc_mem_field32_get(unit, L3_ECMPm,
                                               &ecmp_entry, NEXT_HOPf);
            } else {
                l3_index = lpmp->l3_index;
            }
            rv = READ_L3Xm(unit, MEM_BLOCK_ANY, l3_index, &l3x_entry);
            BCM_IF_ERROR_RETURN(rv);
            l3_intf_num = BCM_L3X_INTF_NUM_GET(unit, L3Xm, &l3x_entry);
            if ((!negate && (l3_intf_num != intf)) ||
                (negate && (l3_intf_num == intf))) {
                return BCM_E_NONE;
            }
        } else {
            if ((!negate && (lpmp->l3_intf != intf)) ||
                (negate && (lpmp->l3_intf == intf))) {
                return BCM_E_NONE;
            }
        }
    }

    /* delete route from LPM table */
    sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));
    lpm_cfg.defip_ip_addr    = lpmp->ip_addr;
    lpm_cfg.defip_sub_len    = lpmp->subnet_len;
    if (lpmp->ecmp) {
        lpm_cfg.defip_flags  = BCM_L3_MULTIPATH;
    }
    lpm_cfg.defip_l3hw_index = lpmp->l3_index;
    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        lpm_cfg.defip_ecmp_index = lpmp->ecmp_index;
    }

    BCM_IF_ERROR_RETURN(_bcm_xgs_defip_do_del(unit, &lpm_cfg));

    L3_INFO(unit)->defip_count--;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _xgs_defip_del_by_intf_cb
 * Purpose:
 *      DEFIP traversal callback routine to handle del by intf
 * Parameters:
 *      unit      - SOC unit number.
 *      lpmp      - info for the route being traversed
 *      trav_data - User supplied data
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Traverse all routes, and calls this function at each network,
 *      Since we are deleting route based on interface number,
 *      we first check for a interface match, otherwise, do nothing
 */

STATIC int
_xgs_defip_del_by_intf_cb(int unit, _bcm_lpm_entry_t *lpmp, void *trav_data)
{
    int intf;
    int rv;

    _print_net_addr(DK_L3, "_xgs_defip_del_by_intf_cb: ",
               lpmp->ip_addr, lpmp->subnet_len, "\n");

    intf = PTR_TO_INT(trav_data);

    rv = _xgs_defip_del_intf_do_cb(unit, lpmp, intf, FALSE);

    return rv;
}

/*
 * Function:
 *      _xgs_defip_del_not_intf_cb
 * Purpose:
 *      DEFIP traversal callback routine to handle del by not intf
 * Parameters:
 *      unit      - SOC unit number.
 *      lpmp      - info for the route being traversed
 *      trav_data - User supplied data
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Traverse all routes, and calls this function at each network,
 *      Since we are deleting route based on interface number,
 *      we first check for a interface match, otherwise, do nothing
 */

STATIC int
_xgs_defip_del_not_intf_cb(int unit, _bcm_lpm_entry_t *lpmp, void *trav_data)
{
    int intf;
    int rv;

    _print_net_addr(DK_L3, "_xgs_defip_del_not_intf_cb: ",
               lpmp->ip_addr, lpmp->subnet_len, "\n");

    intf = PTR_TO_INT(trav_data);

    rv = _xgs_defip_del_intf_do_cb(unit, lpmp, intf, TRUE);

    return rv;
}

/*
 * The main function for call abck function in delete by interface
 * for per VLAN default route
 * negate - 0 means interface match; 1 means not match
 */
STATIC int
bcm_xgs_pv_def_route_del_intf(int unit, int l3_intf, int negate)
{
    _bcm_per_vlan_def_route_t *pvdr;
    l3x_entry_t l3x_entry;
    bcm_vlan_t *vlan_list;
    int count, i;
    bcm_if_t l3_intf_num;
    int rv;

    count = 0;
    for (pvdr = l3_defip_table[unit].pv_def_route; pvdr; pvdr = pvdr->next) {
        rv = READ_L3Xm(unit, MEM_BLOCK_ANY, pvdr->l3_index, &l3x_entry);
        BCM_IF_ERROR_RETURN(rv);
        l3_intf_num = soc_L3Xm_field32_get(unit, &l3x_entry, L3_INTF_NUMf);
        if ((!negate && (l3_intf_num == l3_intf)) ||
            (negate && (l3_intf_num != l3_intf))) {
            count++;
        }
    }

    vlan_list = sal_alloc((count + 1) * sizeof(bcm_vlan_t), "vlist");
    if (vlan_list == NULL) {
        return BCM_E_MEMORY;
    }

    i = 0;
    if (l3_defip_table[unit].global_def_route_set) {
        rv = READ_L3Xm(unit, MEM_BLOCK_ANY, L3_D15_GLOBAL_DEF_ROUTE_IDX, &l3x_entry);
        if (BCM_FAILURE(rv)) {
            sal_free(vlan_list);
            return (rv);
        }

        l3_intf_num = soc_L3Xm_field32_get(unit, &l3x_entry, L3_INTF_NUMf);
        if ((!negate && (l3_intf_num == l3_intf)) ||
            (negate &&  (l3_intf_num != l3_intf))) {
            vlan_list[i++] = L3_VLAN_GLOBAL;
            count++;
        }
    }

    for (pvdr = l3_defip_table[unit].pv_def_route; pvdr; pvdr = pvdr->next) {
        rv = READ_L3Xm(unit, MEM_BLOCK_ANY, pvdr->l3_index, &l3x_entry);
        if (BCM_FAILURE(rv)) {
            sal_free(vlan_list);
            return (rv);
        }
        l3_intf_num = soc_L3Xm_field32_get(unit, &l3x_entry, L3_INTF_NUMf);
        if ((!negate && (l3_intf_num == l3_intf)) ||
            (negate && (l3_intf_num != l3_intf))) {
            vlan_list[i++] = pvdr->vid;
        }
    }

    /* Do actual deletion */
    for (i = 0; i < count; i++) {
        _bcm_xgs_per_vlan_def_route_del(unit, vlan_list[i]);
    }

    sal_free(vlan_list);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_xgs_defip_del_intf
 * Purpose:
 *      Delete routes from LPM table by interface match
 * Parameters:
 *      unit - SOC unit number.
 *      lpm_cfg - Pointer to memory for DEFIP table related information.
 *      negate - 0 means interface match; 1 means not match
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_defip_del_intf(int unit, _bcm_defip_cfg_t *lpm_cfg, int negate)
{
    if (lpm_cfg->defip_intf >= L3_INFO(unit)->l3_intf_table_size) {
        return (BCM_E_PARAM);
    }

    if (L3_INFO(unit)->defip_count > 0) {
        L3_LOCK(unit);
        if (negate) {
            _xgs_route_traverse(unit, _xgs_defip_del_not_intf_cb,
                                INT_TO_PTR(lpm_cfg->defip_intf));
        } else {
            _xgs_route_traverse(unit, _xgs_defip_del_by_intf_cb,
                                INT_TO_PTR(lpm_cfg->defip_intf));
        }
        L3_UNLOCK(unit);
    }

    /* Draco1.5 per VLAN default routes treatment */
    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        bcm_xgs_pv_def_route_del_intf(unit, lpm_cfg->defip_intf, negate);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_defip_del_all
 * Purpose:
 *      Delete all routes
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_defip_del_all(int unit)
{
    _bcm_per_vlan_def_route_t *pvdr;
    _bcm_l3hash_t *hash_entry;
    vlan_tab_entry_t vt;
    int entries;
    int rv;
    uint32 i;

    if (L3_INFO(unit)->defip_count == 0) {
        return (BCM_E_NONE);
    }

    /* Delete all LPM related L3 entries */
    i = (SOC_IS_DRACO15(unit)) ? 1 : 0;
    for (; i < L3_DEFIP_LOCAL_ROUTE_IDX(unit); i++) {
        hash_entry = &l3_table[unit].l3_hash[i];
        if (hash_entry->l3h_flags & L3H_LPM) {
            if (hash_entry->l3h_flags & L3H_HOST) {
                hash_entry->l3h_use_count = 1;
                hash_entry->l3h_flags &= ~L3H_LPM;
            } else {
                hash_entry->l3h_use_count = 0;
                hash_entry->l3h_ip_addr = 0;
                l3_table[unit].l3_count--;

                /* hardware delete */
                _bcm_xgs_l3hw_index_del(unit, i);
            }
        }
    }

    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        /* Cleanup ECMP table */
        BCM_IF_ERROR_RETURN
            (_bcm_ecmp_hw_table_init(unit, &entries));

        /* Delete all per-VLAN default routes */
        if (l3_defip_table[unit].global_def_route_set) {
            BCM_IF_ERROR_RETURN(_bcm_xgs_global_def_route_del(unit));
            l3_defip_table[unit].global_def_route_set = 0;
        }

        for (pvdr = l3_defip_table[unit].pv_def_route; pvdr; pvdr = pvdr->next) {
            soc_mem_lock(unit, VLAN_TABm);
            rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int)pvdr->vid, &vt);
            if (BCM_FAILURE(rv)) {
                soc_mem_unlock(unit, VLAN_TABm);
                return (rv);
            }
            soc_VLAN_TABm_field32_set(unit, &vt, DEFAULT_ROUTEf, 0);
            rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int)pvdr->vid, &vt);
            soc_mem_unlock(unit, VLAN_TABm);
            BCM_IF_ERROR_RETURN(rv);
        }
    }

    /* Cleanup LPM table */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs_lpm_hw_table_init(unit, &entries));

    _bcm_xgs_lpm_sw_table_init(unit, 0);
    sal_memset(l3_defip_table[unit].lpm_blk_used, 0,
               SHR_BITALLOCSIZE(l3_defip_table[unit].lpm_block_max));

    L3_INFO(unit)->defip_count = 0;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs_lpm_do_age
 * Purpose:
 *      LPM table age function, called from traversal function
 */
STATIC int
_bcm_xgs_lpm_do_age(int unit, _bcm_lpm_entry_t *lpmp, void *trav_data)
{
    bcm_l3_route_t info;
    _bcm_defip_cfg_t defip_cfg;
    _bcm_l3_defip_traverse_t *dt;
    int rv;

    dt = (_bcm_l3_defip_traverse_t *)trav_data;
    dt->trav_index++;

    defip_cfg.defip_ip_addr = lpmp->ip_addr;
    defip_cfg.defip_sub_len = lpmp->subnet_len;
    defip_cfg.defip_flags   = BCM_L3_LPM_DEFROUTE;
    rv = bcm_xgs_defip_cfg_get(unit, &defip_cfg); /* get LPM HIT bit */
    if (rv < 0) {
        soc_cm_debug(DK_L3, "Error getting LPM HIT bit\n");
        return rv;
    }
 
    if (defip_cfg.defip_flags & BCM_L3_HIT) {
        _bcm_xgs_lpm_do_hit(unit, &defip_cfg, 0);    /* clear LPM HIT bit */
    } else { 
        bcm_l3_route_t_init(&info);
        info.l3a_subnet    = defip_cfg.defip_ip_addr;
        info.l3a_ip_mask   = BCM_IP4_MASKLEN_TO_ADDR(defip_cfg.defip_sub_len);
        info.l3a_flags     = defip_cfg.defip_flags;
        info.l3a_port_tgid = defip_cfg.defip_port_tgid;
        info.l3a_intf      = defip_cfg.defip_intf;
        info.l3a_modid     = defip_cfg.defip_modid;
        sal_memcpy(info.l3a_nexthop_mac, defip_cfg.defip_mac_addr,
                   sizeof(sal_mac_addr_t));

        /* Delete unused route. */
        BCM_IF_ERROR_RETURN(bcm_xgs_defip_del(unit, &defip_cfg));

        /* user supplied callback to LPM age */
        if (NULL != dt->trav_cb) {
           (*dt->trav_cb)(unit, dt->trav_index - 1, &info, dt->user_data);
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_xgs_lpm_age
 * Purpose:
 *      Age out the LPM table, clear HIT bits
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      age_out - Call back routine.
 *      user_data - User callback cookie.
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_xgs_lpm_age(int unit, bcm_l3_route_traverse_cb age_out, void *user_data)
{
    _bcm_l3_defip_traverse_t dt;

    sal_memset(&dt, 0, sizeof(_bcm_l3_defip_traverse_t));
    dt.trav_cb = age_out;
    dt.user_data = user_data;

    _xgs_route_traverse(unit, _bcm_xgs_lpm_do_age, (void *)&dt);
 
    return (BCM_E_NONE);
}

/*
 * Function:
 *	_bcm_xgs_defip_trav_cb
 * Purpose:
 *	L3 DEFIP traversal callback routine
 * Parameters:
 *	unit      - SOC unit number.
 *	lpmp      - info for the route being traversed
 *	trav_data - User supplied data
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_bcm_xgs_defip_trav_cb(int unit, _bcm_lpm_entry_t *lpmp, void *trav_data)
{
    _bcm_l3_defip_traverse_t *dt;
    _bcm_defip_cfg_t defip_cfg;
    l3x_entry_t l3x_entry;
    ecmp_entry_t ecmp_entry;
    bcm_l3_route_t info;
    int ecmp_index, l3_index;
    int rv;
    int i;

    dt = (_bcm_l3_defip_traverse_t *)trav_data;

    if (dt->trav_index++ < dt->trav_start) {
        return (BCM_E_NONE);
    }

    /* We call bcm_xgs_defip_cfg_get() here to get the hit bit info */
    defip_cfg.defip_ip_addr = lpmp->ip_addr;
    defip_cfg.defip_sub_len = lpmp->subnet_len;
    defip_cfg.defip_flags   = BCM_L3_LPM_DEFROUTE;
    BCM_IF_ERROR_RETURN(bcm_xgs_defip_cfg_get(unit, &defip_cfg));

    /* Local route */
    bcm_l3_route_t_init(&info);
    if (defip_cfg.defip_flags == BCM_L3_DEFIP_LOCAL) {
        sal_memset(info.l3a_nexthop_mac, 0, sizeof(sal_mac_addr_t));
        info.l3a_port_tgid = -1;
        info.l3a_intf      = 0;
        info.l3a_modid     = 0;
    } else {
        sal_memcpy(info.l3a_nexthop_mac,
                   defip_cfg.defip_mac_addr, sizeof(sal_mac_addr_t));
        info.l3a_port_tgid = defip_cfg.defip_port_tgid;
        info.l3a_intf      = defip_cfg.defip_intf;
        info.l3a_modid     = defip_cfg.defip_modid;
    }
    info.l3a_subnet    = lpmp->ip_addr;
    info.l3a_ip_mask   = BCM_IP4_MASKLEN_TO_ADDR(lpmp->subnet_len);
    info.l3a_flags     = defip_cfg.defip_flags;

    /*
     * call the user supplied function at this route
     */
    if (dt->trav_cb) {
        if (defip_cfg.defip_flags & BCM_L3_MULTIPATH) {
            /*
             * Let's go through all the ECMP paths
             */
            (*dt->trav_cb)(unit, dt->trav_index - 1, &info, dt->user_data);

            for (i = 1; i < lpmp->ecmp_count; i++) {
                if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
                    ecmp_index = lpmp->ecmp_index + i;
                    rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY,
                                 ecmp_index, &ecmp_entry);
                    BCM_IF_ERROR_RETURN(rv);
                    l3_index = soc_mem_field32_get(unit, L3_ECMPm,
                                 &ecmp_entry, NEXT_HOPf);
                } else {
                    l3_index = lpmp->l3_index + (i * L3_BUCKET_SIZE);
                }

                rv = READ_L3Xm(unit, MEM_BLOCK_ANY, l3_index, &l3x_entry);
                BCM_IF_ERROR_RETURN(rv);
                soc_L3Xm_mac_addr_get(unit, &l3x_entry, MAC_ADDRf,
                                      info.l3a_nexthop_mac);
                info.l3a_intf      = BCM_L3X_INTF_NUM_GET(unit, L3Xm, &l3x_entry);
                info.l3a_port_tgid = soc_L3Xm_field32_get(unit, &l3x_entry,
                                                          TGID_PORTf);
                info.l3a_modid     = soc_L3Xm_field32_get(unit, &l3x_entry,
                                                          MODULE_IDf);
                info.l3a_flags     = BCM_L3_MULTIPATH;
                (*dt->trav_cb)(unit, dt->trav_index - 1, &info, dt->user_data);
            }
        } else {
            (*dt->trav_cb)(unit, dt->trav_index - 1, &info, dt->user_data);
        }
    }

    if (dt->trav_index > dt->trav_end) {
        return (-1);  /* stop traversal */
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_defip_traverse
 * Purpose:
 *      Find routes from the DEFIP table by traversing the table
 *      and run the function for each found route
 * Parameters:
 *      unit - SOC PCI device unit number (driver internal).
 *	trav_fn - User callback function, called once per route
 *	start - Starting point of interest.
 *	end   - Ending point of interest.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_defip_traverse(int unit, uint32 start, uint32 end,
                      bcm_l3_route_traverse_cb trav_fn, void *user_data)
{
    _bcm_l3_defip_traverse_t dt;
    int rv = BCM_E_NONE;

    dt.trav_cb = trav_fn;
    dt.user_data = user_data;
    dt.trav_start = start;
    dt.trav_end = end;
    dt.trav_index = 0;

    _xgs_route_traverse(unit, _bcm_xgs_defip_trav_cb, (void *)&dt);

    /* Draco1.5 per VLAN default routes treatment */
    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        _bcm_per_vlan_def_route_t *pvdr;
        l3x_entry_t l3x_entry;
        bcm_l3_route_t info;

        bcm_l3_route_t_init(&info);

        if (l3_defip_table[unit].global_def_route_set) {
            rv = READ_L3Xm(unit, MEM_BLOCK_ANY, L3_D15_GLOBAL_DEF_ROUTE_IDX, &l3x_entry);
            BCM_IF_ERROR_RETURN(rv);
            soc_L3Xm_mac_addr_get(unit, &l3x_entry, MAC_ADDRf, info.l3a_nexthop_mac);
            info.l3a_port_tgid = soc_L3Xm_field32_get(unit, &l3x_entry, TGID_PORTf);
            info.l3a_modid = soc_L3Xm_field32_get(unit, &l3x_entry, MODULE_IDf);
            info.l3a_intf = soc_L3Xm_field32_get(unit, &l3x_entry, L3_INTF_NUMf);
            info.l3a_vid = L3_VLAN_GLOBAL;
            (*trav_fn)(unit, dt.trav_index++, &info, user_data);
        }

        for (pvdr = l3_defip_table[unit].pv_def_route; pvdr; pvdr = pvdr->next) {
            rv = READ_L3Xm(unit, MEM_BLOCK_ANY, pvdr->l3_index, &l3x_entry);
            BCM_IF_ERROR_RETURN(rv);
            soc_L3Xm_mac_addr_get(unit, &l3x_entry, MAC_ADDRf, info.l3a_nexthop_mac);
            info.l3a_port_tgid = soc_L3Xm_field32_get(unit, &l3x_entry, TGID_PORTf);
            info.l3a_modid = soc_L3Xm_field32_get(unit, &l3x_entry, MODULE_IDf);
            info.l3a_intf = soc_L3Xm_field32_get(unit, &l3x_entry, L3_INTF_NUMf);
            info.l3a_vid = pvdr->vid;
            (*trav_fn)(unit, dt.trav_index++, &info, user_data);
        }
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *	_xgs_defip_find_index_cb
 * Purpose:
 *	Callback for DEFIP index search
 * Parameters:
 *	unit      - SOC unit number.
 *	lpmp      - info for the route being traversed
 *	trav_data - User supplied data
 */

STATIC int
_xgs_defip_find_index_cb(int unit, _bcm_lpm_entry_t *lpmp, void *trav_data)
{
    _bcm_l3_defip_traverse_t *dt;
    _bcm_l3_cfg_t l3cfg;
    int rv;

    sal_memset(&l3cfg, 0, sizeof(l3cfg));

    dt = (_bcm_l3_defip_traverse_t *)trav_data;
    if (dt->trav_index < dt->trav_start) {
        dt->trav_index++;
        return (0);
    }

    l3cfg.l3c_hw_index = lpmp->l3_index;
    rv = bcm_xgs_l3_get_by_index(unit, l3cfg.l3c_hw_index, &l3cfg);
    if (rv < 0) {
        return (-1);
    }

    sal_memcpy(dt->route_info->l3a_nexthop_mac,
               l3cfg.l3c_mac_addr, sizeof(sal_mac_addr_t));
    dt->route_info->l3a_flags     = l3cfg.l3c_flags;
    dt->route_info->l3a_port_tgid = l3cfg.l3c_port_tgid;
    dt->route_info->l3a_intf      = l3cfg.l3c_intf;
    dt->route_info->l3a_modid     = l3cfg.l3c_modid;
    dt->route_info->l3a_subnet    = lpmp->ip_addr;
    dt->route_info->l3a_ip_mask   = BCM_IP4_MASKLEN_TO_ADDR(lpmp->subnet_len);

    return (0);
}

/*
 * Function:
 *	bcm_xgs_defip_find_index
 * Purpose:
 *      Find entry from the DEFIP table by traversing the LPM table
 * Parameters:
 *      unit  - SOC PCI device unit number (driver internal).
 *	index - the route number of interest
 *	info  - (out)info of this route
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_defip_find_index(int unit, int index, bcm_l3_route_t *info)
{
    _bcm_l3_defip_traverse_t dt;

    if (index > L3_INFO(unit)->defip_count) {
        return BCM_E_PARAM;
    }

    dt.trav_start = index;
    dt.trav_index = 0;
    dt.route_info = info;
    _xgs_route_traverse(unit, _xgs_defip_find_index_cb, (void *)&dt);

    return (BCM_E_NONE);
}

/*
 * Function:
 *	_lpm_get_free_block
 * Purpose:
 *      Returns the next available block in the LPM table.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *	LPM block index on success, INVALID_LPM_IDX on failure.
 */
STATIC uint16
_lpm_get_free_block(int unit)
{
    int start, i;

    if (l3_defip_table[unit].lpm_block_used ==
        l3_defip_table[unit].lpm_block_max) {
        return (INVALID_LPM_IDX);
    }

    /* first LPM block is always used */
    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        start = l3_defip_table[unit].lpm_min + LPM_LONG_BLK_SIZE;
    } else {
        start = l3_defip_table[unit].lpm_min + LPM_BLK_SIZE;
    }

    for (i = start; i < l3_defip_table[unit].lpm_max; i += LPM_BLK_SIZE) {
        if (!BCM_L3_LPM_BLOCK_GET(unit, i / LPM_BLK_SIZE)) {
            BCM_L3_LPM_BLOCK_SET(unit, i / LPM_BLK_SIZE);
            l3_defip_table[unit].lpm_block_used++;
            return (i);
        }
    }

    /* should not reach here */
    return (INVALID_LPM_IDX);
}

/*
 * Function:
 *	_lpm_release_block
 * Purpose:
 *	This routine "frees" one LPM block (mark the block as unused)
 * Parameters:
 *	unit        - SOC unit number.
 *	lpm_blk_idx - Index of LPM block that needs to be de-allocated
 * Returns:
 *	Nothing
 */
STATIC void
_lpm_release_block(int unit, uint16 lpm_blk_idx)
{
    _bcm_lpm_entry_t *lpmp;
    int i;

    assert(lpm_blk_idx % LPM_BLK_SIZE == 0);

    lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);

    BCM_L3_LPM_BLOCK_CLR(unit, lpm_blk_idx / LPM_BLK_SIZE);
    l3_defip_table[unit].lpm_block_used--;

    for (i = 0; i < LPM_BLK_SIZE; i++) {
        if (lpmp[i].valid) {
            lpmp[i].valid = 0;
            l3_defip_table[unit].lpm_entry_used--;
        }
        lpmp[i].final = 0;
        lpmp[i].forward = INVALID_LPM_IDX;
        lpmp[i].l3_index = INVALID_L3_IDX;
        lpmp[i].l3_intf = 0;
        lpmp[i].ip_addr = INVALID_IP_ADDR;
        lpmp[i].subnet_len = INVALID_PREFIX_LEN;
        lpmp[i].backup_routes = NULL;
        lpmp[i].parent = INVALID_LPM_IDX;
        lpmp[i].ecmp = 0;
        lpmp[i].ecmp_count = 0;
        lpmp[i].ecmp_index = -1;
    }
}

/*
 * Function:
 *	_lpm_compare_backup_routes
 * Purpose:
 *	Check if backup routes are identical for every offset in the block.
 *      Note this function is based on the fact that the back up routes
 *      are always ordered by prefix length
 * Parameters:
 *	unit        - SOC unit number
 *	lpm_blk_idx - LPM block index
 * Returns:
 *	1 - all the backup routes are identical for every offset in lpmp block
 *	0 - otherwise
 */
STATIC int
_lpm_compare_backup_routes(int unit, int lpm_blk_idx)
{
    _bcm_lpm_entry_t *lpmp;
    int i;

    assert(lpm_blk_idx % LPM_BLK_SIZE == 0);

    lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);

    /* The first block is assumed to have different routes */
    if (IS_LPM_LONG_BLOCK(unit, lpm_blk_idx)) {
        return 0;
    }

    /*
     * all entries in the block have backup routes, compare them.
     * Note the back up routes are always ordered by prefix length
     */
    for (i = 1; i < LPM_BLK_SIZE; i++) {
        if (lpmp[0].backup_routes != lpmp[i].backup_routes) {
            return 0;
        }
    }

    return 1;
}

/*
 * Function:
 *	_lpm_del_from_update_list
 * Purpose:
 *	Removes the given block from the update list which
 *	contains all the blocks that need to be written to LPM memory.
 *	This is used during the collapse process which in turn is called
 *	when deleting an LPM entry or when an addition fails.
 * Parameters:
 *	unit - SOC unit number
 *	lpmp - the LPM block to be deleted
 * Returns:
 *	entry - if found
 *	NULL - if not found
 */
STATIC _bcm_update_list_t *
_lpm_del_from_update_list(int unit, uint16 lpm_blk)
{
    _bcm_update_list_t *entry, *prev_entry;
    int found = 0;

    /* check if the lpm entry is in the list */
    entry = prev_entry = NULL;

    if (l3_defip_table[unit].update_head) {
        entry = l3_defip_table[unit].update_head;
        while (entry) {
            if (entry->block == lpm_blk) {
                found = 1;
                break;
            }
            prev_entry = entry;
            entry = entry->next;
        }
    }

    if (found) {
        if (entry == l3_defip_table[unit].update_tail) {
            l3_defip_table[unit].update_tail = prev_entry;
        }

        if (prev_entry) {
            prev_entry->next = entry->next;
        } else {
            l3_defip_table[unit].update_head = entry->next;
        }

        return (entry);
    } else {
        soc_cm_debug(DK_L3, "LPM entry not found in update list\n");
        return NULL;
    }
}

/*
 * Get the blk and offset for a LPM entry
 */
STATIC int
_lpm_block_offset(int unit, uint16 entry_index,
                   int *block_index, int *offset)
{
    if (entry_index == INVALID_LPM_IDX) {
        *block_index = INVALID_LPM_IDX;
        *offset = 0;
        return BCM_E_PARAM;
    }

    if (SOC_IS_DRACO15(unit) &&   /* Draco 1.5 */
        entry_index < LPM_LONG_BLK_SIZE) {
        *block_index = 0;
        *offset = entry_index;
        return BCM_E_NONE;
    }

    *block_index = (entry_index / LPM_BLK_SIZE) * LPM_BLK_SIZE;
    *offset = entry_index % LPM_BLK_SIZE;

    return BCM_E_NONE;
}

/*
 * Function:
 *	_lpm_collapse_nodes
 * Purpose:
 *	This routine looks at all the nodes that can be collapsed when
 *	a route is deleted. This is also called when an addition fails.
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *	None
 */
STATIC void
_lpm_collapse_nodes(int unit)
{
    _bcm_lpm_entry_t *lpmp = NULL, *parent;
    _bcm_update_list_t *entry = NULL;
    int valids, finals;
    int parent_blk, pa_prefix_lo, parent_offset = 0;
    int same_route, delete, backup_mask;
    int i;

    for (entry = l3_defip_table[unit].update_head; entry;
	 entry = entry->next) {
        same_route = 1;
        valids = finals = delete = backup_mask = 0;
        lpmp = L3_LPM_ENTRY(unit, entry->block);

        /*
         * if the parent node is present. then check if it
         * can be collapsed into the parent node
         */
        if (lpmp[0].parent != INVALID_LPM_IDX) {
            parent = L3_LPM_ENTRY(unit, lpmp[0].parent);
            assert(parent->forward == entry->block);
            _lpm_block_offset(unit, lpmp->parent, &parent_blk, &parent_offset);

            /* The lowest prefix length at parent node */
            pa_prefix_lo = _xgs_lpm_prefix_range(unit, parent_blk);

            for (i = 0; i < LPM_BLK_SIZE; i++){
                if (lpmp[i].valid) {
                    valids |= (1 << i);
                }

                if (lpmp[i].final) {
                    finals |= (1 << i);
                }

                if (lpmp[i].backup_routes) {
                    backup_mask |= (1 << i);
                }

                if (lpmp[0].l3_index   != lpmp[i].l3_index ||
                    lpmp[0].l3_intf    != lpmp[i].l3_intf ||
                    lpmp[0].ip_addr    != lpmp[i].ip_addr ||
                    lpmp[0].subnet_len != lpmp[i].subnet_len ||
                    lpmp[0].ecmp       != lpmp[i].ecmp ||
                    lpmp[0].ecmp_count != lpmp[i].ecmp_count ||
                    lpmp[0].ecmp_index != lpmp[i].ecmp_index) {
                    same_route = 0;
                }
            }

            if (valids == 0) {
                /*
                 * all offsets are invalid, i.e. the whole block is invalid
                 * collapse to the parent
                 */
                parent->valid = 0;
                parent->forward = INVALID_LPM_IDX;
                parent->backup_routes = NULL;
                l3_defip_table[unit].lpm_entry_used--;
                soc_cm_debug(DK_L3, "1 collapse node %d to parent %d\n",
                                    entry->block, lpmp[0].parent);

                /* add the parent node to tail of update list */
                _lpm_add_to_update_list(unit, parent_blk, parent_offset);
                delete = 1;
            } else if (valids == LPM_BLK_ALL_BITS && finals == LPM_BLK_ALL_BITS) {
                /*
                 * every entry is valid && final
                 */
                if (backup_mask == 0) {
                    if (same_route) {
                        /*
                         * every entry is valid, no old routes and every
                         * entry has same route. Set the parent node to point
                         * to this route and de-allocate this block
                         */
                        parent->l3_index   = lpmp[0].l3_index;
                        parent->l3_intf    = lpmp[0].l3_intf;
                        parent->subnet_len = lpmp[0].subnet_len;
                        parent->ip_addr    = lpmp[0].ip_addr;
                        parent->ecmp       = lpmp[0].ecmp;
                        parent->ecmp_count = lpmp[0].ecmp_count;
                        parent->ecmp_index = lpmp[0].ecmp_index;
                        parent->forward    = INVALID_LPM_IDX;
                        parent->final      = 1;

                        /*
                         * Delete the local backup route at parent if there
                         * is any. Example 1: add 10.0.0.0/8 10.10.0.0/12, del
                         * 10.10.0.0/12, collapse to 10.0.0.0/8, there is
                         * a backup route at 10.0.0.0/8. Example 2: add 10.0.0.0/8
                         * 10.10.0.0/16, del /16, first collapse to /12 blk,
                         * but there is no backup route at /12, the backup route
                         * is created at /8.
                         * Check if backup is created at parent first.
                         */
                        if (parent->backup_routes &&
                            parent->backup_routes->subnet_len >= pa_prefix_lo) {
                            _print_net_addr(DK_L3, "Free backup 1 ",
                                parent->backup_routes->ip_addr,
                                parent->backup_routes->subnet_len, "\n");
                            sal_free(parent->backup_routes);
                            parent->backup_routes = NULL;
                        }

                        soc_cm_debug(DK_L3, "2 collapse node %d to parent %d\n",
                                    entry->block, lpmp[0].parent);

                        /* add the parent node to the update list */
                        _lpm_add_to_update_list(unit, parent_blk, parent_offset);
                        delete = 1;
                    }
                } else if (backup_mask == LPM_BLK_ALL_BITS) {
                    /*
                     * all entries in the block have old routes
                     */
                    if (same_route) {
                        if (_lpm_compare_backup_routes(unit, entry->block)) {
                            /*
                             * old routes are identical set the parent node to
                             * point to this route and copy the backup routes to
                             * the parent
                             */
                            parent->l3_index   = lpmp[0].l3_index;
                            parent->l3_intf    = lpmp[0].l3_intf;
                            parent->subnet_len = lpmp[0].subnet_len;
                            parent->ip_addr    = lpmp[0].ip_addr;
                            parent->ecmp       = lpmp[0].ecmp;
                            parent->ecmp_count = lpmp[0].ecmp_count;
                            parent->ecmp_index = lpmp[0].ecmp_index;
                            parent->forward    = INVALID_LPM_IDX;
                            parent->final      = 1;

                            /*
                             * Delete the local backup route at parent
                             * Example: add 10.0.0.0/8 10.10.0.0/12, 10.10.0.0/14,
                             * 10.10.0.0/16 10.10.10/20, del 10.10.10/20,
                             * collapse to 10.10.0.0/16
                             * Check if backup is created at parent first.
                             */
                            if (parent->backup_routes &&
                                parent->backup_routes->subnet_len >= pa_prefix_lo) {
                                _print_net_addr(DK_L3, "Free backup 2 ",
                                    parent->backup_routes->ip_addr,
                                    parent->backup_routes->subnet_len, "\n");
                                sal_free(parent->backup_routes);
                            }
                            parent->backup_routes = lpmp[0].backup_routes;

                            soc_cm_debug(DK_L3, "3 collapse node %d to parent %d\n",
                                         entry->block, lpmp[0].parent);

                            _lpm_add_to_update_list(unit, parent_blk,
                                                    parent_offset);
                            delete = 1;
                        }
                    }
                }
            }

            if (delete) {
                /* clear the block and write it to the LPM table */
                for (i = 0; i < LPM_BLK_SIZE; i++) {
                    lpmp[i].backup_routes = NULL;
                }

                _lpm_add_to_update_list(unit, entry->block, LPM_BLK_SIZE);
                _lpm_release_block(unit, entry->block);
            }
        } else {
            /*
             * If it has no parent block, this gotta be the
             * first block in lpm table
             */
            assert(lpmp == L3_LPM_ENTRY(unit, 0));
        }
    }
}

/*
 * Insert a backup route into backup route list
 * (backup route list is in descending order by prefix len)
 */
STATIC void
_l3_lpm_backup_insert(_bcm_backup_route_t **blist, _bcm_backup_route_t *backup)
{
    _bcm_backup_route_t *cur, *prev;

    prev = NULL;

    for (cur = *blist; cur; cur = cur->next) {
        if (cur->subnet_len <= backup->subnet_len) {
            break;
        }
        prev = cur;
    }

    if (cur && (cur->subnet_len == backup->subnet_len)) {
        assert(cur->ip_addr == backup->ip_addr);
        return;
    }

    if (prev) {
        backup->next = prev->next;
        prev->next = backup;
    } else {
        backup->next = *blist;
        *blist = backup;
    }
}

/*
 * Delete the backup route from backup route list
 */
STATIC _bcm_backup_route_t *
_l3_lpm_backup_del(_bcm_backup_route_t **blist, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_backup_route_t *cur, *prev;
    int found = 0;

    prev = NULL;

    for (cur = *blist; cur; cur = cur->next) {
        if (cur->subnet_len == lpm_cfg->defip_sub_len &&
            cur->ip_addr == lpm_cfg->defip_ip_addr) {
            found = 1;
            break;
        }
        prev = cur;
    }

    if (!found) {
        return NULL;
    }

    if (prev) {
        prev->next = cur->next;
    } else {
        *blist = cur->next;
    }

    return cur;
}

/*
 * Function:
 *	_lpm_sw_delete
 * Purpose:
 *	Does the actual deletion of the route from the
 *	s/w LPM table.  Adds all the blocks that need to be written
 *	to LPM memory to the update list which will be used to
 *      update the H/W table later.
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *	Nothing
 * Note:
 *     This function is recursive.
 */
STATIC void
_lpm_sw_delete(int unit, int lpm_blk_idx, _bcm_defip_cfg_t *lpm_cfg,
               int start_offset, int end_offset, int *result)
{
    _bcm_backup_route_t *long_route, *backup;
    _bcm_lpm_entry_t *lpmp;
    int prefix_lo;
    int i;

    lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);

    /*
     * The prefix len range for this LPM block
     */
    prefix_lo = _xgs_lpm_prefix_range(unit, lpm_blk_idx);

    for (i = start_offset; i < end_offset; i++) {
        if (!lpmp[i].valid) {
            continue;
        }

        if (lpmp[i].final) {
            if (lpmp[i].ip_addr == lpm_cfg->defip_ip_addr &&
                lpmp[i].subnet_len == lpm_cfg->defip_sub_len) {
                /*
                 * route at this offset matches the route that needs
                 * to be deleted
                 */
                if (lpmp[i].backup_routes) {
                    /*
                     * Restore longest prefix route from backup route list
                     * Example: add 10.0.0.0/8 10.10.0.0/14, 10.10.0.0/16
                     * del 10.10.0.0/16.
                     */
                    long_route = lpmp[i].backup_routes;
                    lpmp[i].backup_routes = long_route->next;

                    /* update the lpm entry with next longest mask */
                    lpmp[i].l3_index   = long_route->l3_index;
                    lpmp[i].l3_intf    = long_route->l3_intf;
                    lpmp[i].ip_addr    = long_route->ip_addr;
                    lpmp[i].subnet_len = long_route->subnet_len;
                    lpmp[i].ecmp       = long_route->ecmp;
                    lpmp[i].ecmp_count = long_route->ecmp_count;
                    lpmp[i].ecmp_index = long_route->ecmp_index;

                    soc_cm_debug(DK_L3, "Restore backup %d\n", lpm_blk_idx + i);

                    /*
                     * the backup route ends in the same block, free mem
                     */
                    if (long_route->subnet_len >= prefix_lo) {
                        _print_net_addr(DK_L3, "Free backup 3 ",
                            long_route->ip_addr,
                            long_route->subnet_len, "\n");
                        sal_free(long_route);
                    }

                    _lpm_add_to_update_list(unit, lpm_blk_idx, i);

                } else {
                    /*
                     * no old routes, invalidate(remove) the entry
                     */
                    lpmp[i].valid      = 0;
                    lpmp[i].final      = 0;
                    lpmp[i].l3_index   = INVALID_L3_IDX;
                    lpmp[i].l3_intf    = 0;
                    lpmp[i].ip_addr    = INVALID_IP_ADDR;
                    lpmp[i].subnet_len = INVALID_PREFIX_LEN;
                    lpmp[i].ecmp       = 0;
                    lpmp[i].ecmp_count = 0;
                    lpmp[i].ecmp_index = -1;
                    l3_defip_table[unit].lpm_entry_used--;

                    _lpm_add_to_update_list(unit, lpm_blk_idx, i);
                    assert(lpmp[i].forward == INVALID_LPM_IDX);
                }
                *result = 1;

            } else {
                /*
                 * the default route at this offset doesn't match the
                 * route that needs to be deleted, need to check the backup
                 * routes to see if the route to be deleted is in the old routes
                 * list. If it is, then delete from the list. Since the
                 * default route is not deleted, we don't need to
                 * write to the HW LPM memory in this case.
                 */
                if (lpmp[i].backup_routes) {
                    backup = _l3_lpm_backup_del(&lpmp[i].backup_routes, lpm_cfg);
                    if (backup) {
                        *result = 1;

                        /*
                         * the backup route ends in the same block, free mem
                         * Example: add 10.0.0.0/8 10.10.0.0/14, 10.10.0.0/16,
                         * del 10.10.0.0/14.
                         */
                        if (backup->subnet_len >= prefix_lo) {
                            _print_net_addr(DK_L3, "Free backup 4 ",
                                backup->ip_addr, backup->subnet_len, "\n");
                            sal_free(backup);
                        }
                    }
                }
            }
        } else { /* not final */
            assert(lpmp[i].forward != INVALID_LPM_IDX);
            _lpm_sw_delete(unit, lpmp[i].forward,
                           lpm_cfg, 0, LPM_BLK_SIZE, result);
        }
    }
}

/*
 * Function:
 *	_lpm_is_entry_for_route
 * Purpose:
 *	Check if LPM entry(including backup routes) is for a route
 * Parameters:
 *	unit    - SOC unit #
 *	lpmp    - the LPM entry
 *	lpm_cfg - the route to compare to LPM entry
 * Returns:
 *	TRUE - yes
 *	FALSE - no
 */
STATIC int
_lpm_is_entry_for_route(int unit, int lpm_idx, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_lpm_entry_t *lpmp;
    _bcm_backup_route_t *entry = NULL;

    lpmp = L3_LPM_ENTRY(unit, lpm_idx);

    if (lpm_cfg->defip_ip_addr == lpmp->ip_addr &&
        lpm_cfg->defip_sub_len == lpmp->subnet_len) {
        return 1;
    }

    /* Check on the old routes queue */
    if (lpmp->backup_routes) {
        entry = lpmp->backup_routes;
        while (entry) {
            if (lpm_cfg->defip_ip_addr == entry->ip_addr &&
                lpm_cfg->defip_sub_len == entry->subnet_len) {
                return 1;
            }

            entry = entry->next;
        }
    }

    return 0;
}

/*
 * Function:
 *	_lpm_sw_insert
 * Purpose:
 *	This routine does the actual insertion of the route to
 *	the LPM table by updating the block(s) at the specified
 *	offsets. This routine also adds all the blocks, that need
 *	to written to LPM memory, to the update list.
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *	BCM_E_EXISTS - duplicate entry
 *	1 - inserted
 * Note:
 *     This function is recursive
 */
STATIC int
_lpm_sw_insert(int unit, int lpm_blk_idx, _bcm_defip_cfg_t *lpm_cfg,
	    int start_offset, int end_offset)
{
    _bcm_backup_route_t *new_backup = NULL;
    _bcm_lpm_entry_t *lpmp, *temp;
    int prefix_lo, prefix, i;

    lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);

    /*
     * The prefix len range for this LPM block
     */
    prefix_lo = _xgs_lpm_prefix_range(unit, lpm_blk_idx);

    soc_cm_debug(DK_L3,
          "_lpm_sw_insert prefix_lo = %d offset start %d end %d\n",
           prefix_lo, start_offset, end_offset);

    for (i = start_offset; i < end_offset; i++) {
        if (lpmp[i].valid) {
            if (lpmp[i].final) {
                /*
                 * insert only if it is not a duplicate entry
                 */
                if (_lpm_is_entry_for_route(unit, lpm_blk_idx + i, lpm_cfg)) {
                    continue;
                }

                if (lpm_cfg->defip_sub_len > lpmp[i].subnet_len) {
                    /*
                     * to-be-added route has longer subnet mask than the route
                     * at the current offset. Make the new route(more specific)
                     * as the route in hardware, and save the current route to
                     * backup routes list.  The backup route will be used
                     * if this new (and more specific) route is deleted later
                     */

                    if (lpmp[i].subnet_len < prefix_lo) {
                        /*
                         * the backup route is from parent in the tree
                         * example: add 10.0.0.0/8 10.10.0.0/16 at block for 16
                         */
                        temp = &lpmp[i];
                        prefix = prefix_lo;
                        while (temp->parent != INVALID_LPM_IDX) {
                            prefix -= LPM_BITS_PER_BLK;
                            temp = L3_LPM_ENTRY(unit, temp->parent);
                            if (lpmp[i].subnet_len >= prefix) {
                                break;
                            }
                        }

                        for (new_backup = temp->backup_routes; new_backup;
                             new_backup = new_backup->next) {
                            if (new_backup->subnet_len == lpmp[i].subnet_len) {
                                break;
                            }
                        }
                        lpmp[i].backup_routes = new_backup;

                    } else {
                        /*
                         * the backup route is in the same block and is being
                         * overwritten by the new route info, must allocate
                         * new entry to save backup info.
                         * example: add 10.0.0.0/8 10.10.0.0/14 10.10.0.0/16
                         * at block for 14/16
                         */
                        new_backup = (_bcm_backup_route_t *)
                            sal_alloc(sizeof(_bcm_backup_route_t), "l3_backup");
                        if (new_backup == NULL) {
                            return BCM_E_MEMORY;
                        }

                        new_backup->l3_index   = lpmp[i].l3_index;
                        new_backup->ip_addr    = lpmp[i].ip_addr;
                        new_backup->l3_intf    = lpmp[i].l3_intf;
                        new_backup->subnet_len = lpmp[i].subnet_len;
                        new_backup->ecmp       = lpmp[i].ecmp;
                        new_backup->ecmp_count = lpmp[i].ecmp_count;
                        new_backup->ecmp_index = lpmp[i].ecmp_index;
                        new_backup->next       = NULL;
                        _l3_lpm_backup_insert(&lpmp[i].backup_routes, new_backup);

                        soc_cm_debug(DK_L3, "LPM index %d, ", lpm_blk_idx + i);
                        _print_net_addr(DK_L3, "new backup 1 ",
                            new_backup->ip_addr,
                            new_backup->subnet_len, "\n");
                    }

                    lpmp[i].l3_intf    = lpm_cfg->defip_intf;
                    lpmp[i].ip_addr    = lpm_cfg->defip_ip_addr;
                    lpmp[i].subnet_len = lpm_cfg->defip_sub_len;
                    lpmp[i].ecmp       = lpm_cfg->defip_ecmp;
                    lpmp[i].ecmp_count = lpm_cfg->defip_ecmp_count;
                    lpmp[i].ecmp_index = lpm_cfg->defip_ecmp_index;
                    lpmp[i].l3_index   = lpm_cfg->defip_l3hw_index;

                    _lpm_add_to_update_list(unit, lpm_blk_idx, i);

                } else {
                    /*
                     * new route is shorter than the current default route
                     * at this offset, i.e. the current route is more specific.
                     * save the new route on the old routes list
                     */
                    if (lpm_cfg->defip_sub_len < prefix_lo) {
                        /*
                         * new route being added ends in the parent, the backup
                         * is already created there.
                         * example: add 10.0.0.0/8 10.10.10.0/20 10.10.0.0/14
                         * 10.10.0.0/16, net /16 at /20, or (8 20 16 14, 14@20)
                         */
                        temp = &lpmp[i];
                        prefix = prefix_lo;
                        while (temp->parent != INVALID_LPM_IDX) {
                            prefix -= LPM_BITS_PER_BLK;
                            temp = L3_LPM_ENTRY(unit, temp->parent);
                            if (lpm_cfg->defip_sub_len >= prefix) {
                                break;
                            }
                        }

                        for (new_backup = temp->backup_routes; new_backup;
                             new_backup = new_backup->next) {
                            if (new_backup->subnet_len == lpm_cfg->defip_sub_len) {
                                break;
                            }
                        }
                        if (new_backup == NULL) {
                            /* Should never happen */
                            return BCM_E_INTERNAL;
                        }
                        _l3_lpm_backup_insert(&lpmp[i].backup_routes, new_backup);

                    } else {
                        /*
                         * the new route ends in the same block, but is shorter
                         * example (8 16 14, 14@16) (8 16 14 13)
                         */
                        new_backup = (_bcm_backup_route_t *)
                            sal_alloc(sizeof(_bcm_backup_route_t), "l3_backup");
                        if (new_backup == NULL) {
                            return BCM_E_MEMORY;
                        }

                        new_backup->l3_index   = lpm_cfg->defip_l3hw_index;
                        new_backup->l3_intf    = lpm_cfg->defip_intf;
                        new_backup->ip_addr    = lpm_cfg->defip_ip_addr;
                        new_backup->subnet_len = lpm_cfg->defip_sub_len;
                        new_backup->ecmp       = lpm_cfg->defip_ecmp;
                        new_backup->ecmp_count = lpm_cfg->defip_ecmp_count;
                        new_backup->ecmp_index = lpm_cfg->defip_ecmp_index;
                        new_backup->next       = NULL;
                        _l3_lpm_backup_insert(&lpmp[i].backup_routes, new_backup);

                        soc_cm_debug(DK_L3, "LPM index %d, ", lpm_blk_idx + i);
                        _print_net_addr(DK_L3, "new backup 2 ",
                            new_backup->ip_addr,
                            new_backup->subnet_len, "\n");
                    }
                }
            } else {
                /*
                 * Not final, follow the forward block ptr, and update the
                 * block at the forward pointer (recursive call),
                 * start_offset is 0, end_offset is LPM_BLK_SIZE
                 * This is the case when we are inserting a less
                 * specific route, when a subnet of this route already
                 * exists in the LPM table, for example, insert 192.168.0.0/16
                 * when 192.168.10.0/24 already exists.
                 */
                assert(lpmp[i].forward != INVALID_LPM_IDX);
                _lpm_sw_insert(unit, lpmp[i].forward, lpm_cfg, 0, LPM_BLK_SIZE);
            }
        } else {
            /*
             * no route present at this entry in LPM table. So make the
             * new route as the default route at this offset.
             */
            lpmp[i].valid      = 1;
            lpmp[i].final      = 1;
            lpmp[i].l3_index   = lpm_cfg->defip_l3hw_index;
            lpmp[i].l3_intf    = lpm_cfg->defip_intf;
            lpmp[i].ip_addr    = lpm_cfg->defip_ip_addr;
            lpmp[i].subnet_len = lpm_cfg->defip_sub_len;
            lpmp[i].ecmp       = lpm_cfg->defip_ecmp;
            lpmp[i].ecmp_count = lpm_cfg->defip_ecmp_count;
            lpmp[i].ecmp_index = lpm_cfg->defip_ecmp_index;
            lpmp[i].backup_routes = NULL;
            l3_defip_table[unit].lpm_entry_used++;

            _lpm_add_to_update_list(unit, lpm_blk_idx, i);
            assert(lpmp[i].forward == INVALID_LPM_IDX);
        }
    }

    return 1;
}

/*
 * Function:
 *	_lpm_expand_node
 * Purpose:
 *	Expands the offset for the given block by allocating a new block as
 *	a child to the given block at the given offset. Updates the newly
 *	created child block appropriately.
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_lpm_expand_node(int unit, uint16 parent_index)
{
    _bcm_lpm_entry_t *parent, *fwd_blk;
    int i;

    parent = L3_LPM_ENTRY(unit, parent_index);
    assert(parent->forward == INVALID_LPM_IDX);

    /* Get the next free block in LPM table to be used by child block */
    parent->forward = _lpm_get_free_block(unit);
    if (parent->forward == INVALID_LPM_IDX) {
        soc_cm_debug(DK_L3, "Can not get an LPM block\n");
        return (BCM_E_FULL);
    }

    soc_cm_debug(DK_L3, "Expand node to block %d\n", parent->forward);

    fwd_blk = L3_LPM_ENTRY(unit, parent->forward);

    /* add child block pointers to parent */
    for (i = 0; i < LPM_BLK_SIZE; i++) {
        fwd_blk[i].parent = parent_index;
    }

    if (parent->valid && parent->final) {
        /*
         * copy the parent route info to the child block.
         * This is the case when we are inserting a more
         * specific route, when a supernet of this route already
         * exists in the LPM table, for example, insert 192.168.10.0/24
         * when 192.168.0.0/16 already exists.
         */
        for (i = 0; i < LPM_BLK_SIZE; i++) {
            fwd_blk[i].valid = 1;
            fwd_blk[i].final = 1;
            fwd_blk[i].l3_index = parent->l3_index;
            fwd_blk[i].l3_intf = parent->l3_intf;
            fwd_blk[i].ip_addr = parent->ip_addr;
            fwd_blk[i].subnet_len = parent->subnet_len;
            fwd_blk[i].ecmp = parent->ecmp;
            fwd_blk[i].ecmp_count = parent->ecmp_count;
            fwd_blk[i].ecmp_index = parent->ecmp_index;
            l3_defip_table[unit].lpm_entry_used++;

            /* copy the old routes to the child node */
            fwd_blk[i].backup_routes = parent->backup_routes;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_xgs_lpm_compare_table
 * Purpose:
 *	Compare the DEFIP HW and SW tables
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *	SOC_E_XXX
 *	TRUE - tables match
 *	FALSE - tables do not match
 */
int
bcm_xgs_lpm_compare_table(int unit)
{
    int min_index, max_index, index;
    defip_hi_entry_t defip_entry;
    _bcm_lpm_entry_t *lpmp;
    int ret_val = TRUE;
    int final, valid, next_ptr, ecmp, ecmp_count, sw_next_ptr;

    lpmp = L3_LPM_ENTRY(unit, 0);

    min_index = soc_mem_index_min(unit, LPM_MEM(unit));
    max_index = soc_mem_index_max(unit, LPM_MEM(unit));

    for (index = min_index; index < max_index; index++) {
        final = valid = next_ptr = ecmp = ecmp_count = 0;

        sal_memset(&defip_entry, 0, sizeof (defip_hi_entry_t));
        SOC_IF_ERROR_RETURN
	    (soc_mem_read(unit, LPM_MEM(unit), MEM_BLOCK_ANY,
			  index, &defip_entry));

        /*
         * Compare the DEFIP HI entry with s/w copy,
         * Assuming DEFIP HI and LO are in sync
         */
        final = soc_mem_field32_get(unit, LPM_MEM(unit), &defip_entry, STOPf);
        valid = soc_mem_field32_get(unit, LPM_MEM(unit), &defip_entry, VALIDf);

        if (valid) {
            if (!final) {
                next_ptr = BCM_LPM_NEXT_PTR_GET(unit, &defip_entry);
            } else { /* valid & final */
                if (SOC_IS_DRACO1(unit)) {
                    next_ptr = soc_mem_field32_get(unit, LPM_MEM(unit),
                                                   &defip_entry, NEXT_PTRf);
                } else if (SOC_IS_DRACO15(unit)) {
                    ecmp = soc_mem_field32_get(unit, LPM_MEM(unit),
                                               &defip_entry, ECMPf);
                    if (ecmp) {
                        ecmp_count = soc_mem_field32_get(unit, LPM_MEM(unit),
                                            &defip_entry, ECMP_COUNTf);
                        ecmp_count++; /* add 1 back */
                    }
                    next_ptr = soc_mem_field32_get(unit, LPM_MEM(unit),
                                      &defip_entry, ECMP_PTRf);
                } else if (SOC_IS_TUCANA(unit)) {
                    ecmp_count = soc_mem_field32_get(unit, LPM_MEM(unit),
                                        &defip_entry, COUNTf);
                    if (ecmp_count) {
                        ecmp = 1;
                        ecmp_count++; /* add 1 back */
                    }
                    next_ptr = soc_mem_field32_get(unit, LPM_MEM(unit),
                                                   &defip_entry, NEXT_PTRf);
                } else if (SOC_IS_LYNX(unit)) {
                    ecmp_count = soc_mem_field32_get(unit, LPM_MEM(unit),
                                        &defip_entry, ECMP_COUNTf);
                    if (ecmp_count) {
                        ecmp = 1;
                        ecmp_count++; /* add 1 back */
                    }
                    next_ptr = BCM_LPM_NEXT_PTR_GET(unit, &defip_entry);
                }
            }
        }

        if (lpmp[index].forward != INVALID_LPM_IDX) {
            sw_next_ptr = lpmp[index].forward >> LPM_BITS_PER_BLK;
        } else {
            if (lpmp[index].final && lpmp[index].valid) {
                if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
                    if (lpmp[index].ecmp) {
                        sw_next_ptr = lpmp[index].ecmp_index;
                    } else {
                        /* L3 table index */
                        sw_next_ptr = lpmp[index].l3_index;
                    }
                } else { /* DRACO, TUCANA & LYNX */
                    /* L3 table index */
                    sw_next_ptr = lpmp[index].l3_index;
                }
            } else {
                sw_next_ptr = 0;
            }
        }

        if (final != lpmp[index].final ||
            valid != lpmp[index].valid ||
            next_ptr != sw_next_ptr) {
            ret_val = FALSE;
            soc_cm_debug(DK_L3,
			 "LPM entry at index %d doesn't match\n", index);
        }
    }

    return ret_val;
}

/*
 * Function:
 *	_lpm_undo_insert
 * Purpose:
 *	Collapses all the s/w table blocks that were expanded during a
 *	failed insert.
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *	Nothing
 */
STATIC void
_lpm_undo_insert(int unit)
{
    _bcm_lpm_entry_t* lpmp = NULL;
    int blk_index, blk_size, i;

    if (l3_defip_table[unit].update_head != NULL) {
        _lpm_reverse_update_list(unit);
        _lpm_collapse_nodes(unit);

        while (l3_defip_table[unit].update_head) {
            /*
             * clear the dirty bits
             */
            blk_index = l3_defip_table[unit].update_head->block;
            lpmp = L3_LPM_ENTRY(unit, blk_index);
            if (IS_LPM_LONG_BLOCK(unit, blk_index)) {
                blk_size = LPM_LONG_BLK_SIZE; /* first block in BCM5695 */
            } else {
                blk_size = LPM_BLK_SIZE;
            }

            for (i = 0; i < blk_size; i++) {
                lpmp[i].dirty = 0;
            }

            /* free memory */
            sal_free(_lpm_del_from_update_list(unit, blk_index));
        }
    }
}

/*
 * Function:
 *	bcm_xgs_lpm_insert
 * Purpose:
 *	Main routine to insert an LPM entry
 * Parameters:
 *	unit - SOC unit number.
 *	lpm_cfg - Route to insert to the table
 * Returns:
 *	BCM_E_XXX
 * Note:
 *     The new route entry is recorded in the SW LPM table, and
 *     finally write to the HW table if everything is OK.
 *     Adding one IP subnet route results in consuming many
 *     LPM table entries, these entries are linked into
 *     a chain.  The LPM searching algorithm follow
 *     the chain and stops at the longest possible match.
 */
int
bcm_xgs_lpm_insert(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_lpm_entry_t *lpmp;
    _bcm_backup_route_t *backup = NULL;
    int offset, start_offset, end_offset;
    int lpm_blk_idx, num_nybbles;
    int i, rv = BCM_E_NONE;
    int prefix_lo, expanded = 0;
    int new_block_count = 0;
    int new_blocks[8] = {0};

    /*
     * Go through the 8 & 4-bit sub-blocks of the IP subnet to calculate
     * offset in one block of LPM table depending on the chip type.
     */
    lpm_blk_idx = 0;
    num_nybbles = _lpm_num_nybbles(unit, lpm_cfg->defip_sub_len);

    for (i = 0; (i < num_nybbles - 1) && (lpm_blk_idx != INVALID_LPM_IDX); i++) {
        lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
        offset = _lpm_getoffset(unit, lpm_cfg->defip_ip_addr, i);

        if (lpmp[offset].valid) {
            if (lpmp[offset].final) {
                /*
                 * A supernet exists in the LPM table, expand the node
                 * by creating a child block at this offset for the subnet
                 */
                soc_cm_debug(DK_L3,
                    "lpm_expand_node 1 at index %d\n", lpm_blk_idx + offset);
                if ((rv = _lpm_expand_node(unit, lpm_blk_idx + offset)) < 0) {
                    soc_cm_debug(DK_L3, "Node expansion failed\n");
                    break;
                }

                lpmp[offset].final = 0;

                if (expanded) {
                    /* there is no backup route created here */
                    lpmp[offset].ip_addr = INVALID_IP_ADDR;
                    lpmp[offset].subnet_len = INVALID_PREFIX_LEN;
                    lpmp[offset].backup_routes = NULL;
                    soc_cm_debug(DK_L3, "No backup route created here\n");
                } else {
                    expanded = 1;
                    /*
                     * The route that used to end here is being expanded by
                     * subnet route, this will be a backup route, unless
                     * the route that ends was expanded from the parent.
                     */
                    prefix_lo = _xgs_lpm_prefix_range(unit, lpm_blk_idx);
                    soc_cm_debug(DK_L3, "prefix_lo=%d\n", prefix_lo);
                    if (lpmp[offset].subnet_len >= prefix_lo) {
                        backup = (_bcm_backup_route_t *)
                             sal_alloc(sizeof(_bcm_backup_route_t), "l3_backup");
                        if (backup == NULL) {
                            return BCM_E_MEMORY;
                        }

                        backup->l3_index   = lpmp[offset].l3_index;
                        backup->ip_addr    = lpmp[offset].ip_addr;
                        backup->l3_intf    = lpmp[offset].l3_intf;
                        backup->subnet_len = lpmp[offset].subnet_len;
                        backup->ecmp       = lpmp[offset].ecmp;
                        backup->ecmp_count = lpmp[offset].ecmp_count;
                        backup->ecmp_index = lpmp[offset].ecmp_index;
                        backup->next       = NULL;
                        _l3_lpm_backup_insert(&lpmp[offset].backup_routes, backup);

                        soc_cm_debug(DK_L3, "LPM index %d, ", lpm_blk_idx + offset);
                        _print_net_addr(DK_L3, "new backup 3 ",
                            backup->ip_addr,
                            backup->subnet_len, "\n");
                    } else {
                        /* There is no backup route created here */
                        lpmp[offset].ip_addr    = INVALID_IP_ADDR;
                        lpmp[offset].subnet_len = INVALID_PREFIX_LEN;
                        lpmp[offset].backup_routes = NULL;
                    }
                }

                /*
                 * keep track of the blocks to the update list, which
                 * will be written to the hardware LPM table (current
                 * and child node)
		 */
                _lpm_add_to_update_list(unit, lpm_blk_idx, offset);
                _lpm_add_to_update_list(unit, lpmp[offset].forward, LPM_BLK_SIZE);
            }
        } else { /* Blank entry, expand from here */
            soc_cm_debug(DK_L3,
                "lpm_expand_node 2 at index %d\n", lpm_blk_idx + offset);
            if ((rv = _lpm_expand_node(unit, lpm_blk_idx + offset)) < 0) {
                soc_cm_debug(DK_L3, "Node expansion failed\n");
                if (new_block_count) {
                    _lpm_release_block(unit, lpm_blk_idx);
                }
                break;
            }

            /*
             * keep track of blocks that need to have the valid bit reset
             * in case of expand_node failure
             */
            new_blocks[new_block_count++] = lpm_blk_idx + offset;

            /*
             * keep track of the blocks to the update list, which will
             * be written to the hardware LPM table
             */
            _lpm_add_to_update_list(unit, lpm_blk_idx, offset);

            lpmp[offset].valid = 1;
            lpmp[offset].backup_routes = NULL;
            l3_defip_table[unit].lpm_entry_used++;
        }

        lpm_blk_idx = lpmp[offset].forward;
    }

    /*
     * Since the insertion into SW table failed. We don't have
     * to update the h/w table. Just restore the S/W LPM table
     * by collapsing the nodes that were expanded in s/w table,
     * if any, and delete the update list.
     */
    if (rv < 0) {
        for (i = 0; i < new_block_count; i++) {
            lpmp = L3_LPM_ENTRY(unit, new_blocks[i]);
            lpmp->valid = 0;
            lpmp->backup_routes = NULL;
        }
        _lpm_undo_insert(unit);
        for (i = 0; i < new_block_count; i++) {
            lpmp = L3_LPM_ENTRY(unit, new_blocks[i]);
            lpmp->forward = INVALID_LPM_IDX;
        }
        return rv;
    }

    /* The last block of the LPM entry chain is for this subnet */
    if (lpm_blk_idx != INVALID_LPM_IDX) {
        lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
        offset = _lpm_getoffset(unit, lpm_cfg->defip_ip_addr, i);

        /* get all offsets that need to be updated with the new route info */
        _bcm_xgs_offset_range_get(unit, lpm_blk_idx, offset,
              lpm_cfg->defip_sub_len, i, &start_offset, &end_offset);
        soc_cm_debug(DK_L3,
              "Offset start = %d, end = %d\n", start_offset, end_offset);

        for (i = start_offset; i < end_offset; i++) {
            if (lpmp[i].valid && !lpmp[i].final) {
                /*
                 * To-be-added route ends here, but not final, create new backup
                 */
                backup = (_bcm_backup_route_t *)
                     sal_alloc(sizeof(_bcm_backup_route_t), "l3_backup");
                if (backup == NULL) {
                    return BCM_E_MEMORY;
                }

                backup->l3_index   = lpm_cfg->defip_l3hw_index;
                backup->ip_addr    = lpm_cfg->defip_ip_addr;
                backup->l3_intf    = lpm_cfg->defip_intf;
                backup->subnet_len = lpm_cfg->defip_sub_len;
                backup->ecmp       = lpm_cfg->defip_ecmp;
                backup->ecmp_count = lpm_cfg->defip_ecmp_count;
                backup->ecmp_index = lpm_cfg->defip_ecmp_index;
                backup->next       = NULL;
                _l3_lpm_backup_insert(&lpmp[i].backup_routes, backup);

                soc_cm_debug(DK_L3, "LPM index %d, ", lpm_blk_idx + i);
                _print_net_addr(DK_L3, "new backup 4 ",
                    backup->ip_addr, backup->subnet_len, "\n");

                /*
                 * Set subnet_len != INVALID_PREFIX_LEN to indicate
                 * a route(backup) ends here
                 */
                if (lpmp[i].subnet_len == INVALID_PREFIX_LEN ||
                    lpm_cfg->defip_sub_len > lpmp[i].subnet_len) {
                    lpmp[i].ip_addr    = lpm_cfg->defip_ip_addr;
                    lpmp[i].subnet_len = lpm_cfg->defip_sub_len;
                    lpmp[i].l3_intf    = lpm_cfg->defip_intf;
                    lpmp[i].ecmp       = lpm_cfg->defip_ecmp;
                    lpmp[i].ecmp_count = lpm_cfg->defip_ecmp_count;
                    lpmp[i].ecmp_index = lpm_cfg->defip_ecmp_index;
                    lpmp[i].l3_index   = lpm_cfg->defip_l3hw_index;
                }
            }
        }

        rv = _lpm_sw_insert(unit, lpm_blk_idx, lpm_cfg,
                            start_offset, end_offset);
    } else {
        soc_cm_debug(DK_L3, "ERROR: _lpm_sw_insert Failed\n");
        return BCM_E_FAIL;
    }

    if (rv < 0) {
        _lpm_undo_insert(unit);
        return rv;
    }

    /*
     * Fianlly write to the h/w table, if hw write fails, the SW and HW copy
     * will not be in sync
     */
    rv = _lpm_hw_write(unit);

    return rv;
}

/*
 * Function:
 *	bcm_xgs_lpm_delete
 * Purpose:
 *	Main routine for deleting a route
 * Parameters:
 *	unit - SOC unit number.
 *	lpm_cfg - The route to delete
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_xgs_lpm_delete(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_lpm_entry_t *lpmp;
    _bcm_backup_route_t *backup;
    _bcm_backup_route_t **backup_del_list;
    int offset, start_offset, end_offset;
    int lpm_blk_idx, num_nybbles;
    ip_addr_t ipaddr;
    int subnet_len, prefix_lo;
    int result = 0;
    int i, rv = BCM_E_NONE;

    ipaddr = lpm_cfg->defip_ip_addr;
    subnet_len = lpm_cfg->defip_sub_len;
    if (subnet_len <= 4) {
        prefix_lo = 0;
    } else if (SOC_IS_DRACO15(unit) && subnet_len <= 8) {
        prefix_lo = 0;
    } else {
        prefix_lo = ((subnet_len - 1) / LPM_BITS_PER_BLK) * LPM_BITS_PER_BLK + 1;
    }

    /*
     * Walk through the blocks for the route until the last block
     * that could have the route is reached (LPM).
     */
    lpm_blk_idx = 0;
    num_nybbles = _lpm_num_nybbles(unit, subnet_len);

    for (i = 0; (i < num_nybbles - 1) && (lpm_blk_idx != INVALID_LPM_IDX); i++) {
        lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
        offset = _lpm_getoffset(unit, ipaddr, i);

        if (!lpmp[offset].valid || lpmp[offset].final) {
            _print_net_addr(DK_L3,
                  "Invalid offset or premature final for net ",
                   ipaddr, subnet_len, " is found\n");
            return BCM_E_NOT_FOUND;
        }

        lpm_blk_idx = lpmp[offset].forward;
    }

    /* The last block of the LPM entry chain is for this subnet */
    if (lpm_blk_idx != INVALID_LPM_IDX) {
        lpmp = L3_LPM_ENTRY(unit, lpm_blk_idx);
        offset = _lpm_getoffset(unit, ipaddr, i);

        _bcm_xgs_offset_range_get(unit, lpm_blk_idx, offset,
               subnet_len, i, &start_offset, &end_offset);

        soc_cm_debug(DK_L3, "bcm_xgs_lpm_delete offset range %d - %d\n",
                            start_offset, end_offset);

        backup_del_list = (_bcm_backup_route_t **)
                        sal_alloc(sizeof(_bcm_backup_route_t *) *
                                  (end_offset - start_offset + 1), "L3");
        if (backup_del_list == NULL) {
            return BCM_E_MEMORY;
        }

        for (i = start_offset; i < end_offset; i++) {
            if (lpmp[i].valid && !lpmp[i].final) {
                /*
                 * Set subnet_len != INVALID_PREFIX_LEN to indicate
                 * a backup route ends here
                 */
                if (lpm_cfg->defip_sub_len == lpmp[i].subnet_len) {
                    backup = lpmp[i].backup_routes->next;
                    if (backup != NULL && backup->subnet_len >= prefix_lo) {
                        /* There is more backup route created here */
                        lpmp[i].ip_addr    = backup->ip_addr;
                        lpmp[i].subnet_len = backup->subnet_len;
                        lpmp[i].l3_index   = backup->l3_index;
                        lpmp[i].l3_intf    = backup->l3_intf;
                        lpmp[i].ecmp       = backup->ecmp;
                        lpmp[i].ecmp_count = backup->ecmp_count;
                    } else {
                        /* This is the only backup route created here,
                         * do not set lpmp[i].backup_routes = NULL here,
                         * it is set to NULL just above free backup 5 */
                        lpmp[i].ip_addr    = INVALID_IP_ADDR;
                        lpmp[i].subnet_len = INVALID_PREFIX_LEN;
                        lpmp[i].l3_index   = INVALID_L3_IDX;
                        lpmp[i].l3_intf    = 0;
                        lpmp[i].ecmp       = 0;
                        lpmp[i].ecmp_count = 0;
                    }
                }

                /*
                 * To-be-deleted route ends here, but not final, it is
                 * in the backup route list.
                 */
                backup = lpmp[i].backup_routes;
                while (backup != NULL) {
                    if (backup->subnet_len == lpm_cfg->defip_sub_len) {
                        break;
                    }
                    backup = backup->next;
                }
                /* Note, do not set backup->next to NULL here */
                backup_del_list[i - start_offset] = backup;
            }
        }

        /* now delete the specified route */
        _lpm_sw_delete(unit, lpm_blk_idx, lpm_cfg,
		       start_offset, end_offset, &result);
        if (!result) {    /* route NOT found  */
            assert(l3_defip_table[unit].update_head == NULL);
            _print_net_addr(DK_L3,
                 "net addr ", ipaddr, subnet_len, " is NOT found\n");
            sal_free(backup_del_list);
            return BCM_E_NOT_FOUND;
        }

        /* Now the to-be-deleted route is removed from all backup lists,
         * down the trie, free backup route from where it's created
         */
        for (i = start_offset; i < end_offset; i++) {
            if (lpmp[i].valid && !lpmp[i].final) {
                if (backup_del_list[i - start_offset]) {
                    if (lpmp[i].backup_routes ==
                        backup_del_list[i - start_offset]) {
                        backup = lpmp[i].backup_routes->next;
                        if (backup && backup->subnet_len >= prefix_lo) {
                            lpmp[i].backup_routes = backup;
                        } else {
                            lpmp[i].backup_routes = NULL;
                        }
                    } else {
                        
                        _l3_lpm_backup_del(&lpmp[i].backup_routes, lpm_cfg);
                    }

                    _print_net_addr(DK_L3, "Free backup 5 ",
                        backup_del_list[i - start_offset]->ip_addr,
                        backup_del_list[i - start_offset]->subnet_len, "\n");
                    sal_free(backup_del_list[i - start_offset]);
                }
            }
        }
        sal_free(backup_del_list);

    } else {
        soc_cm_debug(DK_L3, "ERROR: LPM Delete Failed\n");
        return BCM_E_FAIL;
    }

    _lpm_reverse_update_list(unit);
    _lpm_collapse_nodes(unit);
    _lpm_reverse_update_list(unit); /* put it back in the original order */

    /*
     * Finally, write to the h/w table, if hw write fails, the SW and HW copy
     * will not be in sync
     */
    rv = _lpm_hw_write(unit);

    return rv;
}

/*
 * Function:
 *	bcm_xgs_lpm_lookup
 * Purpose:
 *	Look up an LPM entry in the SW LPM table
 * Parameters:
 *	unit - SOC unit number.
 *	iproute - The IP route to search for
 *	lpm_cfg - (OUT)IP route info
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_xgs_lpm_lookup(int unit, ip_addr_t iproute, _bcm_defip_cfg_t* lpm_cfg)
{
    _bcm_lpm_entry_t *lpmp;
    int offset, num_nybbles;
    int i;
    int done = 0;

    lpmp = L3_LPM_ENTRY(unit, 0);

    if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */
        num_nybbles = 7;
    } else {
        num_nybbles = 32 / LPM_BITS_PER_BLK;
    }

    for (i = 0; (i < num_nybbles) && !done; i++) {
        offset = _lpm_getoffset(unit, iproute, i);

        if (lpmp[offset].valid) {
            if (lpmp[offset].forward == INVALID_LPM_IDX) {
                if (lpmp[offset].final) {
                    done = 1;
                    lpm_cfg->defip_sub_len = lpmp[offset].subnet_len;
                    lpm_cfg->defip_ip_addr = lpmp[offset].ip_addr;
                    lpm_cfg->defip_ecmp = lpmp[offset].ecmp;
                    lpm_cfg->defip_ecmp_count = lpmp[offset].ecmp_count;
                    lpm_cfg->defip_ecmp_index = lpmp[offset].ecmp_index;
                    if (SOC_IS_DRACO1(unit)) { /* DRACO */
                        lpm_cfg->defip_intf = lpmp[offset].l3_intf;
                    }
                    lpm_cfg->defip_l3hw_index = lpmp[offset].l3_index;
                }
            } else {
                lpmp = L3_LPM_ENTRY(unit, lpmp[offset].forward);
            }
        } else {
            break;
        }
    }

    if (!done) {
        return (BCM_E_NOT_FOUND);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_xgs_lpm_sw_tbl_empty
 * Purpose:
 *	Count entries in DEFIP SW table
 * Parameters:
 *	unit - SOC unit number.
 *      count - (OUT) Number of table entries.
 * Returns:
 *	TRUE - DEFIP SW table is empty.
 *	FALSE - DEFIP SW table is not empty.
 */
int
bcm_xgs_lpm_sw_tbl_empty(int unit, int *count)
{
    _bcm_lpm_entry_t *ent;
    int i, cnt = 0;

    for (i = l3_defip_table[unit].lpm_min;
	 i <= l3_defip_table[unit].lpm_max; i++) {
	ent = L3_LPM_ENTRY(unit, i);
        if (ent->forward != INVALID_LPM_IDX ||
            ent->valid != 0 ||
            ent->final != 0 ||
            ent->backup_routes != NULL) {
            cnt++;
            if (*count && cnt <= *count) {
                soc_cm_debug(DK_L3, "lpmp[%d]: ip ", i);
                _print_net_addr(DK_L3, "", ent->ip_addr, ent->subnet_len, "");
                soc_cm_debug(DK_L3, "e %d e_c %d fwd %d l3hw %d",
                             ent->ecmp,
                             ent->ecmp_count,
                             ent->forward != INVALID_LPM_IDX ? ent->forward : 0,
                             ent->l3_index);
                soc_cm_debug(DK_L3, " e_ind %d st %d val %d old_r %d\n",
                             ent->ecmp_index,
                             ent->final,
                             ent->valid,
                             ent->backup_routes ? 1:0);
            }
        }
    }

    *count = cnt;

    return (cnt == 0);
}

/*
 * Function:
 *	bcm_xgs_lpm_hw_tbl_empty
 * Purpose:
 *	Get the number of entries in HW DEFIP table
 * Parameters:
 *	unit - SOC unit number.
 * Returns:
 *	TRUE - DEFIP HW table is empty.
 *	FALSE - DEFIP HW table is not empty.
 *	BCM_E_XXX except BCM_E_NONE - other error
 * Note:
 *	Assuming DEFIP HI and LO are in sync
 */
int
bcm_xgs_lpm_hw_tbl_empty(int unit, int *count)
{
    int min_index, max_index, index;
    defip_hi_entry_t defip_entry;
    int cnt;
    int valid, ecmp, ecmp_count, l3_ptr, ecmp_index, stop;

    min_index = soc_mem_index_min(unit, LPM_MEM(unit));
    max_index = soc_mem_index_max(unit, LPM_MEM(unit));

    cnt = 0;
    for (index = min_index; index < max_index; index++) {
        sal_memset(&defip_entry, 0, sizeof(defip_hi_entry_t));
        SOC_IF_ERROR_RETURN
	    (soc_mem_read(unit, LPM_MEM(unit), MEM_BLOCK_ANY,
			  index, &defip_entry));

        valid = soc_mem_field32_get(unit, LPM_MEM(unit), &defip_entry, VALIDf);
        if (valid != 0) {
            cnt++;

            if (*count && cnt <= *count) {
                ecmp = ecmp_count = l3_ptr = ecmp_index = stop = 0;

                l3_ptr = BCM_LPM_NEXT_PTR_GET(unit, &defip_entry);
                stop = soc_mem_field32_get(unit, LPM_MEM(unit),
                                           &defip_entry, STOPf);

                if (SOC_IS_DRACO15(unit)) {
                    ecmp = soc_mem_field32_get(unit, LPM_MEM(unit),
                                               &defip_entry, ECMPf);
                    if (ecmp) {
                        ecmp_index = l3_ptr & 0x7ff;
                        ecmp_count = (l3_ptr >> 11) & 0x1f;
                        l3_ptr = 0;
                    }
                } else if (SOC_IS_TUCANA(unit)) {
                    ecmp_count = soc_mem_field32_get(unit, LPM_MEM(unit),
                                                     &defip_entry, COUNTf);
                    if (ecmp_count) {
                        ecmp = 1;
                    }
                } else if (SOC_IS_LYNX(unit)) {
                    ecmp_count = soc_mem_field32_get(unit, LPM_MEM(unit),
                                                     &defip_entry,
						     ECMP_COUNTf);
                    if (ecmp_count) {
                        ecmp = 1;
                        ecmp_count++; /* add 1 back */
                    }
                }

                if (stop == 0) {
                    l3_ptr = l3_ptr << 4; /* get the real DEFIP table index */
                }

                soc_cm_debug(DK_L3, "hw_lpm[%d]: e %d e_c %d l3hw %d",
                             index, ecmp, ecmp_count, l3_ptr);
                soc_cm_debug(DK_L3, " etbl_ind %d stop %d\n",
			     ecmp_index, stop);
            }
        }
    }

    *count = cnt;

    return (cnt == 0);
}


int
bcm_xgs_l3_ip6_get(int unit, _bcm_l3_cfg_t *l3cfg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_xgs_l3_ip6_add(int unit, _bcm_l3_cfg_t *l3cfg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_xgs_l3_ip6_del(int unit, _bcm_l3_cfg_t *l3cfg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_xgs_l3_ip6_del_prefix(int unit, _bcm_l3_cfg_t *l3cfg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_xgs_l3_ip6_replace(int unit, _bcm_l3_cfg_t *l3cfg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_xgs_l3_ip6_traverse(int unit, uint32 start, uint32 end,
                      bcm_l3_host_traverse_cb cb, void *user_data)
{
    return (BCM_E_NONE);
}

int
bcm_xgs_ip6_defip_cfg_get(int unit, _bcm_defip_cfg_t *defip)
{
    return BCM_E_UNAVAIL;
}

int
bcm_xgs_ip6_defip_ecmp_get_all(int unit, _bcm_defip_cfg_t *lpm_cfg,
        bcm_l3_route_t *path_array, int max_path, int *path_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_xgs_ip6_defip_add(int unit, _bcm_defip_cfg_t *defip)
{
    return BCM_E_UNAVAIL;
}

int
bcm_xgs_ip6_defip_del(int unit, _bcm_defip_cfg_t *defip)
{
    return BCM_E_UNAVAIL;
}

int
bcm_xgs_ip6_defip_traverse(int unit, uint32 start, uint32 end,
                      bcm_l3_route_traverse_cb trav_fn, void *user_data)
{
    return BCM_E_UNAVAIL;
}

#endif	/* INCLUDE_L3 */

int _bcm_esw_draco_l3_not_empty;
