/*
 * $Id: ipmc.c 1.76.32.1 Broadcom SDK $
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
 * File:	ipmc.c
 * Purpose:	Tracks and manages IPMC tables.
 */

#ifdef INCLUDE_L3

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/l3x.h>
#include <soc/tucana.h>

#include <bcm/error.h>
#include <bcm/ipmc.h>

#include <bcm_int/esw/ipmc.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/draco.h>
#include <bcm_int/esw/tucana.h>
#include <bcm_int/esw/lynx.h>

#include <bcm_int/esw_dispatch.h>

#define	XGS_IPMC_USED_SET(unit, n)                             \
        {                                                      \
            SHR_BITSET(esw_ipmc_info[unit].ipmc_allocated, n); \
	        esw_ipmc_info[unit].ipmc_count++;              \
        }
#define XGS_IPMC_USED_CLR(unit, n)                             \
        {                                                      \
            SHR_BITCLR(esw_ipmc_info[unit].ipmc_allocated, n); \
	        esw_ipmc_info[unit].ipmc_count--;              \
        }
#define XGS_IPMC_USED_ISSET(unit, n)                           \
        SHR_BITGET(esw_ipmc_info[unit].ipmc_allocated, n)      \

#define XGS_IPMC_USED_ZERO(unit)                                            \
        {                                                                   \
	        sal_memset((char *)(esw_ipmc_info[unit].ipmc_allocated), 0, \
                SHR_BITALLOCSIZE(esw_ipmc_info[unit].ipmc_size));           \
            esw_ipmc_info[unit].ipmc_count = 0;                             \
        }
        
STATIC void _draco15_ipmc_repl_l3port_valid(int unit, int ipmc_id,
                                            bcm_pbmp_t *l3_pbmp);


#if defined(BCM_XGS12_SWITCH_SUPPORT)
/*
 * Function:
 *	_xgs_ipmc_create
 * Description:
 *	Create an ipmc entry.
 */

STATIC int
_xgs_ipmc_create(int unit, int *ipmc_id)
{
    int	i;

    IPMC_LOCK(unit);
    for (i = 0; i < esw_ipmc_info[unit].ipmc_size; i++) {
        if (!XGS_IPMC_USED_ISSET(unit, i)) {
            XGS_IPMC_USED_SET(unit, i);
            *ipmc_id = i;
            IPMC_UNLOCK(unit);
            return BCM_E_NONE;
        }
    }
    IPMC_UNLOCK(unit);

    return (BCM_E_FULL);
}

/*
 * Function:
 *	_xgs_ipmc_id_alloc
 * Description:
 *	allocate the given ipmc entry.
 */

STATIC int
_xgs_ipmc_id_alloc(int unit, int ipmc_id)
{
    IPMC_LOCK(unit);
    if (!XGS_IPMC_USED_ISSET(unit, ipmc_id)) {
        XGS_IPMC_USED_SET(unit, ipmc_id);
        IPMC_UNLOCK(unit);
        return BCM_E_NONE;
    }
    IPMC_UNLOCK(unit);

    return BCM_E_FULL;
}

/*
 * Function:
 *	_xgs_ipmc_id_free
 * Description:
 *	Delete an ipmc entry.
 */

STATIC int
_xgs_ipmc_id_free(int unit, int ipmc_id)
{
    IPMC_INIT(unit);
    IPMC_ID(unit, ipmc_id);

    IPMC_LOCK(unit);
    if (XGS_IPMC_USED_ISSET(unit, ipmc_id)) {
        XGS_IPMC_USED_CLR(unit, ipmc_id);
    }
    IPMC_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *	_xgs_ipmc_info_get
 * Description:
 *	Fill information to bcm_ipmc_addr_t struct.
 */

STATIC int
_xgs_ipmc_info_get(int unit, int l3_index, bcm_ipmc_addr_t *ipmc,
                   ipmc_entry_t *entry)
{
    _bcm_l3_cfg_t	l3cfg;
    int			port_tgid, mod, rv;
    uint32              cfg2;
    int untag, port;

    /* For (S,G,V), VLAN ID is from the L3 table (see below) */
    if (!soc_feature(unit, soc_feature_l3_sgv)) {
	ipmc->vid = soc_L3_IPMCm_field32_get(unit, entry, VLAN_IDf);
    }

    SOC_PBMP_CLEAR(ipmc->l3_pbmp);
    SOC_PBMP_CLEAR(ipmc->l2_pbmp);
    if (SOC_IS_TUCANA(unit)) {
        SOC_PBMP_WORD_SET(ipmc->l3_pbmp, 0,
			  soc_L3_IPMCm_field32_get(unit, entry,
						   L3_BITMAP_M0f));
        SOC_PBMP_WORD_SET(ipmc->l3_pbmp, 1,
			  soc_L3_IPMCm_field32_get(unit, entry,
						   L3_BITMAP_M1f));
        SOC_PBMP_WORD_SET(ipmc->l2_pbmp, 0,
			  soc_L3_IPMCm_field32_get(unit, entry,
						   L2_BITMAP_M0f));
        SOC_PBMP_WORD_SET(ipmc->l2_pbmp, 1,
			  soc_L3_IPMCm_field32_get(unit, entry,
						   L2_BITMAP_M1f));
    } else {
        SOC_PBMP_WORD_SET(ipmc->l3_pbmp, 0,
                          soc_L3_IPMCm_field32_get(unit, entry, L3_BITMAPf));
        SOC_PBMP_WORD_SET(ipmc->l2_pbmp, 0,
                          soc_L3_IPMCm_field32_get(unit, entry, L2_BITMAPf));
    }

    ipmc->cos = soc_L3_IPMCm_field32_get(unit, entry, COS_DSTf);
    ipmc->v = soc_L3_IPMCm_field32_get(unit, entry, VALIDf);
    port_tgid = soc_L3_IPMCm_field32_get(unit, entry, TGID_PORTf);
    mod = soc_L3_IPMCm_field32_get(unit, entry, MODULE_IDf);
    if (port_tgid == 0x3f && (!SOC_IS_DRACO15(unit) || mod == 0x3f)) {
        ipmc->ts = 0;
        ipmc->mod_id = -1;
        ipmc->port_tgid = -1;
        ipmc->flags |= BCM_IPMC_SOURCE_PORT_NOCHECK;
    } else if (port_tgid & BCM_TGID_TRUNK_INDICATOR(unit)) {
        ipmc->ts = 1;
        ipmc->mod_id = 0;
        ipmc->port_tgid = (port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit));
	if (SOC_IS_DRACO15(unit)) {
	    ipmc->port_tgid |= ((mod & 0x3) << 5);
	}
    } else {
        bcm_module_t	mod_in, mod_out;
        bcm_port_t	port_in, port_out;

        mod_in = mod;
        port_in = port_tgid;
        BCM_IF_ERROR_RETURN
	    (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));
        ipmc->ts = 0;
        ipmc->mod_id = mod_out;
        ipmc->port_tgid = port_out;
    }

    if (ipmc->v) {
        sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
        if (ipmc->flags & BCM_IPMC_HIT_CLEAR) {
            l3cfg.l3c_flags |= BCM_L3_HIT_CLEAR;
        }
        rv = bcm_xgs_l3_get_by_index(unit, l3_index, &l3cfg);
        if (rv < 0) {
            return (rv);
        }
        ipmc->s_ip_addr = l3cfg.l3c_src_ip_addr;
        ipmc->mc_ip_addr = l3cfg.l3c_ipmc_group;
        if (soc_feature(unit, soc_feature_l3_sgv)) {  /* (S,G,V) */
            ipmc->vid = l3cfg.l3c_vid;
        }

        if (l3cfg.l3c_flags & BCM_L3_HIT) {
            ipmc->flags |= BCM_IPMC_HIT;
        }
    }

    /* Calculate untagged bitmap */
    BCM_PBMP_CLEAR(ipmc->l2_ubmp);
    BCM_PBMP_ITER(ipmc->l2_pbmp, port) {
        if (!IS_E_PORT(unit, port)) { /* CMIC, HG never untagged */
            continue;
        }
#ifdef BCM_TUCANA_SUPPORT
        if (SOC_IS_TUCANA(unit)) {
            uint64  cfg1;

            COMPILER_64_ZERO(cfg1);
            SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG1r(unit, port, &cfg1));
            untag = soc_reg64_field32_get(unit, EGR_IPMC_CFG1r, cfg1, UNTAGf);
        } else {
#endif
            cfg2 = 0;
            SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &cfg2));
#ifdef BCM_LYNX_SUPPORT
            if (SOC_IS_LYNX(unit)) {
                untag = soc_reg_field_get(unit, EGR_IPMC_CFG2r, cfg2,
					  UNTAG_PKTf);
            } else {
#endif
                untag = soc_reg_field_get(unit, EGR_IPMC_CFG2r, cfg2, UNTAGf);
#ifdef BCM_LYNX_SUPPORT
            }
#endif
#ifdef BCM_TUCANA_SUPPORT
        }
#endif

        if (untag) {
            SOC_PBMP_PORT_ADD(ipmc->l2_ubmp, port);
        }
    }

    return BCM_E_NONE;
}

#endif /* BCM_XGS12_SWITCH_SUPPORT */
/*
 * the corresponding MAC address of an IPMC address
 */
#if 0
STATIC void
_ipmc_ipaddr2mac(bcm_mac_t mac, uint32 ipaddr)
{
    mac[0] = 0x1;
    mac[1] = 0x0;
    mac[2] = 0x5e;
    mac[3] = (ipaddr >> 8*2) & 0xff;
    mac[4] = (ipaddr >> 8*1) & 0xff;
    mac[5] = (ipaddr >> 8*0) & 0xff;
}
#endif

/*
 * Function:
 *	_xgs_ipmc_read
 * Description:
 *	Read an ipmc entry.
 */

STATIC INLINE int
_xgs_ipmc_read(int unit, int ipmc_id, ipmc_entry_t *ipmc_entry)
{
    return soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, ipmc_id, ipmc_entry);
}

/*
 * Function:
 *	_xgs_ipmc_write
 * Description:
 *	Write an ipmc entry from bcm_ipmc_addr_t struct.
 */

STATIC int
_xgs_ipmc_write(int unit, int ipmc_id, bcm_ipmc_addr_t *ipmc)
{
    ipmc_entry_t	entry;

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        soc_L3_IPMCm_field32_set(unit, &entry, L3_BITMAP_M0f,
                                 SOC_PBMP_WORD_GET(ipmc->l3_pbmp, 0));
        soc_L3_IPMCm_field32_set(unit, &entry, L3_BITMAP_M1f,
                                 SOC_PBMP_WORD_GET(ipmc->l3_pbmp, 1));
        soc_L3_IPMCm_field32_set(unit, &entry, L2_BITMAP_M0f,
                                 SOC_PBMP_WORD_GET(ipmc->l2_pbmp, 0));
        soc_L3_IPMCm_field32_set(unit, &entry, L2_BITMAP_M1f,
                                 SOC_PBMP_WORD_GET(ipmc->l2_pbmp, 1));
    } else
#endif
    {
        bcm_pbmp_t l3_pbmp;
        BCM_PBMP_ASSIGN(l3_pbmp, ipmc->l3_pbmp);

        if (SOC_IS_DRACO15(unit)) {
            _draco15_ipmc_repl_l3port_valid(unit, ipmc_id, &l3_pbmp);
        }

        soc_L3_IPMCm_field32_set(unit, &entry, L3_BITMAPf,
                                 SOC_PBMP_WORD_GET(l3_pbmp, 0));
        soc_L3_IPMCm_field32_set(unit, &entry, L2_BITMAPf,
                                 SOC_PBMP_WORD_GET(ipmc->l2_pbmp, 0));
    }

    soc_L3_IPMCm_field32_set(unit, &entry, COS_DSTf, 
                             (ipmc->cos < 0) ? 0 : ipmc->cos);
    soc_L3_IPMCm_field32_set(unit, &entry, VALIDf, ipmc->v);
    if ((ipmc->flags & BCM_IPMC_SOURCE_PORT_NOCHECK) ||
	(ipmc->port_tgid < 0)) {			/* no source port */
	if (SOC_IS_DRACO15(unit)) {
	    soc_L3_IPMCm_field32_set(unit, &entry, MODULE_IDf, 0x3f);
	} else {
	    soc_L3_IPMCm_field32_set(unit, &entry, MODULE_IDf, 0x1f);
	}
	soc_L3_IPMCm_field32_set(unit, &entry, TGID_PORTf, 0x3f);
    } else if (ipmc->ts) {				/* trunk source port */
        if (SOC_IS_DRACO15(unit)){
            soc_L3_IPMCm_field32_set(unit, &entry, TGID_PORTf,
                                     ((ipmc->port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit))
				      | BCM_TGID_TRUNK_INDICATOR(unit)));
            soc_L3_IPMCm_field32_set(unit, &entry, MODULE_IDf,
                                     (ipmc->port_tgid & BCM_TGID_PORT_TRUNK_MASK_HI(unit))
                                     >> 5);
        } else {
            soc_L3_IPMCm_field32_set(unit, &entry, TGID_PORTf,
                                     ipmc->port_tgid | BCM_TGID_TRUNK_INDICATOR(unit));
        }
    } else {						/* source port */
	bcm_module_t	mod_in, mod_out;
	bcm_port_t	port_in, port_out;

	mod_in = ipmc->mod_id;
	port_in = ipmc->port_tgid;
	BCM_IF_ERROR_RETURN
	    (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));
    /* Check parameters, since above is an application callback */
    if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
        return BCM_E_BADID;
    }
    if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
        return BCM_E_PORT;
    }
	soc_L3_IPMCm_field32_set(unit, &entry, MODULE_IDf, mod_out);
	soc_L3_IPMCm_field32_set(unit, &entry, TGID_PORTf, port_out);
    }

    if (!soc_feature(unit, soc_feature_l3_sgv)) {
	soc_L3_IPMCm_field32_set(unit, &entry, VLAN_IDf, ipmc->vid);
    }

    return soc_mem_write(unit, L3_IPMCm, MEM_BLOCK_ALL, ipmc_id, &entry);
}

/*
 * Function:
 *	_xgs_ipmc_table_clear
 * Description:
 *	Clear IPMC table.
 */

STATIC int
_xgs_ipmc_table_clear(int unit)
{
    return soc_mem_clear(unit, L3_IPMCm, MEM_BLOCK_ALL, TRUE);
}

/*
 * Function:
 *	bcm_xgs_ipmc_do_vlan
 * Purpose:
 *	Include/exclude VLAN ID in IPMC hashing (Draco 1.5 only)
 * Parameters:
 *	unit    - Unit number
 *	do_vlan - TRUE to include; FALSE to exclude
 * Returns:
 *	BCM_E_XXX
 */

STATIC int
bcm_xgs_ipmc_do_vlan(int unit, int do_vlan)
{
    int			port;
    uint32		regval, oregval;

    do_vlan = do_vlan ? 1 : 0;

    if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
        if (SOC_IS_LYNX(unit)) {
	    SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &regval));
	    oregval = regval;
            soc_reg_field_set(unit, ARL_CONTROLr, &regval,
                              IPMC_DO_VLANf, do_vlan);
	    if (regval != oregval) {
                SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));
            }
        } else {
            PBMP_E_ITER(unit, port) {
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_port_config_set(unit, port,
                                              _bcmPortIpmcVlanKey, do_vlan));
            }
        }
    } else {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_xgs_ipmc_enable
 * Description:
 *	Enable/disable IPMC support.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	enable - TRUE: enable IPMC support.
 *           FALSE: disable IPMC support.
 * Returns:
 *	BCM_E_XXX
 */

STATIC int
_bcm_xgs_ipmc_enable(int unit, int enable)
{
    uint32		configReg, misc;
    int			blk, port;

    enable = enable ? 1 : 0;

    if (SOC_IS_DRACO15(unit)) {
	PBMP_E_ITER(unit, port) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_config_set(unit, port,
                                          _bcmPortIpmcEnable, enable));
	}
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &misc));
        soc_reg_field_set(unit, MISCCONFIGr, &misc,
                          IPMCREPLICATIONENf, enable);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, misc));
    } else if (SOC_IS_TUCANA(unit)) {
	PBMP_E_ITER(unit, port) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_config_set(unit, port,
                                          _bcmPortIpmcEnable, enable));
	}
    } else {
        SOC_BLOCK_ITER(unit, blk, SOC_BLK_ETHER) {
	    port = SOC_BLOCK_PORT(unit, blk);
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &configReg));
            soc_reg_field_set(unit, CONFIGr, &configReg, IPMC_ENABLEf,
                              enable);
            SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, configReg));
        }
    }

    if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
	/* Set IPMC_DO_VLAN based on soc_property_get() */
	if  (enable && soc_property_get(unit, spn_IPMC_DO_VLAN, 1)) {
	    BCM_IF_ERROR_RETURN(bcm_xgs_ipmc_do_vlan(unit, TRUE));
	}
	else {
	    BCM_IF_ERROR_RETURN(bcm_xgs_ipmc_do_vlan(unit, FALSE));
	}
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_xgs_ipmc_src_port_check
 * Description:
 *	Enable/disable the checking of source port the IP multicast table
 *	against the ingress port the IPMC packet arrived.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	enable - TRUE: enable source port checking.
 *		FALSE: disable source port checking.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	On XGS, this can be selected on a per IPMC entry basis.
 *	Use the BCM_IPMC_SOURCE_PORT_NOCHECK flag.
 */

int
_bcm_xgs_ipmc_src_port_check(int unit, int enable)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *	_bcm_xgs_ipmc_src_ip_search
 * Description:
 *	Define the IPMC searching rule.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	enable - TRUE: IPMC searching key = Source IP + destination IP + VID.
 *           FALSE: IPMC searching key = 0 + destination IP + VID.
 * Returns:
 *	BCM_E_XXX
 */

int
_bcm_xgs_ipmc_src_ip_search(int unit, int enable)
{
    uint32		configReg;
    int			noip, blk, port;

    if (SOC_IS_DRACO15(unit) || SOC_IS_TUCANA(unit)) {
	if (enable) {
	    return BCM_E_NONE;	/* always on */
	} else {
	    return BCM_E_FAIL;	/* cannot be disabled */
	}
    } else {
        /*
         * Note that only port 0 is actually used by HW, but this is
         * logically more straightforward
         */
	noip = enable ? 0 : 1;
        if (SOC_IS_LYNX(unit)) {
            SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &configReg));
            soc_reg_field_set(unit, ARL_CONTROLr, &configReg, SRC_IP_CFGf, noip);
            SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, configReg));
        } else {
            SOC_BLOCK_ITER(unit, blk, SOC_BLK_ETHER) {
                port = SOC_BLOCK_PORT(unit, blk);
                SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &configReg));
                soc_reg_field_set(unit, CONFIGr, &configReg, SRC_IP_CFGf,
                                  noip);
                SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, configReg));
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_lynx_ipmc_sm_dm_search
 * Description:
 *	Define the IPMC (S,G)/(*,G) search rule.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	enable - TRUE: IPMC searching key = Source IP + destination IP + VID,
 *                     then searching key = 0 + destination IP + VID.
 *           FALSE: IPMC searching key = 0 + destination IP + VID.
 * Returns:
 *	BCM_E_XXX
 */

int
_bcm_lynx_ipmc_sm_dm_search(int unit, int enable)
{
    uint32		configReg;
    int			dm_sm_enable;

    dm_sm_enable = enable ? 1 : 0;

    SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &configReg));
    soc_reg_field_set(unit, ARL_CONTROLr, &configReg, PIM_DM_SM_ENABLEf,
                      dm_sm_enable);
    SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, configReg));

    return BCM_E_NONE;
}


/*
 * Function:
 *	bcm_xgs_ipmc_init
 * Description:
 *	Initialize the IPMC module and enable IPMC support. Ths function will
 *	enable the source port checking and the the search rule to include
 *	source ip checking.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	This function has to be called before any other IPMC functions.
 */

int
bcm_xgs_ipmc_init(int unit)
{
    int			    rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    int             i;
    _bcm_esw_ipmc_t	*info = IPMC_INFO(unit);

    bcm_xgs_ipmc_delete_all(unit);
	rv = bcm_xgs_ipmc_detach(unit);
	if (rv >= 0) {
	    rv = _bcm_xgs_ipmc_src_ip_search(unit, TRUE);
	}
#ifdef BCM_LYNX_SUPPORT
	if ((rv >= 0) && SOC_IS_LYNX(unit)) {
	    rv = _bcm_lynx_ipmc_sm_dm_search(unit, TRUE);
	    rv = _bcm_xgs_ipmc_src_ip_search(unit, FALSE);
            /* Must have src IP search off for 5673/74 SM/DM feature */
	}
#endif
	if (rv >= 0) {
	    rv = _bcm_xgs_ipmc_enable(unit, TRUE);
	}
	if (rv < 0) {
	    return (rv);
	}

    info->ipmc_size = soc_mem_index_count(unit, L3_IPMCm);
    info->ipmc_allocated =
        sal_alloc(SHR_BITALLOCSIZE(info->ipmc_size), "IPMC-alloc");
    if (info->ipmc_allocated == NULL) {
	return BCM_E_MEMORY;
    }

    info->ipmc_l3_index =
	sal_alloc(info->ipmc_size * sizeof(int), "IPMC-l3");
    if (info->ipmc_l3_index == NULL) {
	sal_free(info->ipmc_allocated);
	info->ipmc_allocated = NULL;
	return (BCM_E_MEMORY);
    }

    XGS_IPMC_USED_ZERO(unit);
    for (i = 0; i < info->ipmc_size; i++) {
        info->ipmc_l3_index[i] = -1;
    }

    info->ipmc_initialized = TRUE;

    rv = BCM_E_NONE;

#endif /* BCM_XGS12_SWITCH_SUPPORT */

    return rv;
}

/*
 * Function:
 *	bcm_xgs_ipmc_detach
 * Description:
 *	Detach the IPMC module.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_detach(int unit)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    uint32	cfg2;
    int		port;
    _bcm_esw_ipmc_t	*info = IPMC_INFO(unit);

    if (info->ipmc_initialized) {
	BCM_IF_ERROR_RETURN(_bcm_xgs_ipmc_enable(unit, FALSE));

	if (info->ipmc_allocated != NULL) {
	    sal_free(info->ipmc_allocated);
	    info->ipmc_allocated = NULL;
	}

        if (info->ipmc_l3_index != NULL) {
	    sal_free(info->ipmc_l3_index);
	    info->ipmc_l3_index = NULL;
	}

        info->ipmc_initialized = FALSE;
    }

     /*
      * In Draco1.5, Clear IPMC_REPLICATION, DISABLE_TTL_DECREMENT,
      * and DISABLE_SA_REPLACE bits on init
      */    
    if (SOC_IS_DRACO15(unit)) {
	PBMP_E_ITER(unit, port) {
             SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &cfg2));    
             soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,     
                               IPMC_REPLICATIONf, 0);    
             soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,     
                               DISABLE_TTL_DECREMENTf, 0);    
             soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,     
                               DISABLE_SA_REPLACEf, 0);    
             SOC_IF_ERROR_RETURN(WRITE_EGR_IPMC_CFG2r(unit, port, cfg2)); 
        }
    }
    rv = BCM_E_NONE;
#endif /* BCM_XGS12_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *	bcm_xgs_ipmc_get
 * Description:
 *	Get an IPMC entry by index.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	index - The index number.
 *	ipmc - (OUT) IPMC entry information.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_get(int unit, int index, bcm_ipmc_addr_t *ipmc)
{
    int			rv = BCM_E_UNAVAIL;
#if defined (BCM_XGS12_SWITCH_SUPPORT)

    ipmc_entry_t	ipmc_entry;

    IPMC_INIT(unit);
    IPMC_ID(unit, index);

    if (XGS_IPMC_USED_ISSET(unit, index)) {
        rv = soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, index, &ipmc_entry);
        if (rv >= 0) {
	    rv = _xgs_ipmc_info_get(unit,
                      esw_ipmc_info[unit].ipmc_l3_index[index],
                      ipmc, &ipmc_entry);
            ipmc->ipmc_index = index;
        }
    } else {
        rv = BCM_E_NOT_FOUND;
    }

#endif /* BCM_XGS12_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *	bcm_xgs_ipmc_lookup
 * Description:
 *	Look up an IPMC entry by sip, mcip and vid
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	index - (OUT) The index number.
 *	ipmc - (IN, OUT) IPMC entry information.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_lookup(int unit, int *index, bcm_ipmc_addr_t *ipmc)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    ipmc_entry_t	ipmc_entry;
    _bcm_l3_cfg_t	l3cfg;

    IPMC_INIT(unit);

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    l3cfg.l3c_src_ip_addr = ipmc->s_ip_addr;
    l3cfg.l3c_ipmc_group = ipmc->mc_ip_addr;
    l3cfg.l3c_vid = ipmc->vid;
    l3cfg.l3c_flags = BCM_L3_IPMC;

    BCM_IF_ERROR_RETURN(bcm_xgs_l3_get(unit, &l3cfg));

    BCM_IF_ERROR_RETURN
        (_xgs_ipmc_read(unit, l3cfg.l3c_ipmc_ptr, &ipmc_entry));

    BCM_IF_ERROR_RETURN
	(_xgs_ipmc_info_get(unit, l3cfg.l3c_hw_index, ipmc, &ipmc_entry));

    ipmc->ipmc_index = l3cfg.l3c_ipmc_ptr;

    if (index != NULL) {
	*index = l3cfg.l3c_ipmc_ptr;
    }

    rv = BCM_E_NONE;
#endif /* BCM_XGS12_SWITCH_SUPPORT */
    return rv; 
}

/*
 * Function:
 *	bcm_xgs_ipmc_add
 * Description:
 *	Add a new entry to the IPMC table.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	data - IPMC entry information.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_add(int unit, bcm_ipmc_addr_t *ipmc)
{
    int			rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    int			ipmc_id;
    ipmc_entry_t	ipmc_entry;
    _bcm_l3_cfg_t	l3cfg;
    int replace;
    int old_entry_found = FALSE;

    IPMC_INIT(unit);

    /* See if an L3 entry exists */
    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    replace = ipmc->flags & BCM_IPMC_REPLACE;

    if (!(ipmc->flags & BCM_IPMC_IP6)) {
        l3cfg.l3c_src_ip_addr = ipmc->s_ip_addr;
        l3cfg.l3c_ipmc_group = ipmc->mc_ip_addr;
        l3cfg.l3c_vid = ipmc->vid;
        l3cfg.l3c_flags = BCM_L3_IPMC;

        rv = bcm_xgs_l3_get(unit, &l3cfg);
        if (BCM_SUCCESS(rv)) {
            if (!replace) {
                return BCM_E_EXISTS;
            }
            old_entry_found = TRUE;
        } else if (rv != BCM_E_NOT_FOUND) {
            return rv;
        }

    }

    if ((ipmc->flags & BCM_IPMC_USE_IPMC_INDEX) && (!old_entry_found)) {
        /* Upper layer s/w is supplying the ipmc index */
        BCM_IF_ERROR_RETURN(_xgs_ipmc_id_alloc(unit, ipmc->ipmc_index));
        sal_memset(&ipmc_entry, 0, sizeof(ipmc_entry));
        BCM_IF_ERROR_RETURN(_xgs_ipmc_read(unit, ipmc->ipmc_index,
                                           &ipmc_entry));
        if (soc_L3_IPMCm_field32_get(unit, &ipmc_entry, VALIDf)) {
            /* already in use */
            return BCM_E_EXISTS;
        }
        ipmc_id = ipmc->ipmc_index;
    } else if (!old_entry_found) {
        BCM_IF_ERROR_RETURN(_xgs_ipmc_create(unit, &ipmc_id));
    } else {
        ipmc_id = l3cfg.l3c_ipmc_ptr;
    }

    l3cfg.l3c_ipmc_ptr = ipmc_id;
    l3cfg.l3c_flags |= (BCM_L3_IPMC | BCM_L3_HIT);

    if (soc_feature(unit, soc_feature_l3_sgv)) {
        l3cfg.l3c_vid = ipmc->vid;
    }

    if (replace) {
        l3cfg.l3c_flags |= BCM_L3_REPLACE;
    }

    if (ipmc->flags & BCM_IPMC_ADD_DISABLED) {
	ipmc->v = 0;
    } else {
	ipmc->v = 1;
    }
    rv = _xgs_ipmc_write(unit, ipmc_id, ipmc);
    if (rv < 0) {
	_xgs_ipmc_id_free(unit, ipmc_id);
	return rv;
    }

    if (ipmc->flags & BCM_IPMC_PROXY_IP6) {
        /*
         * Do not add entry in L3 table; Adding entry in
         * L3_IPMC table is enough
         */
        return rv;
    }

    rv = bcm_xgs_l3_add(unit, &l3cfg);
    if (rv < 0) {
	(void)soc_mem_write(unit, L3_IPMCm, MEM_BLOCK_ALL, ipmc_id,
			    soc_mem_entry_null(unit, L3_IPMCm));
	_xgs_ipmc_id_free(unit, ipmc_id);
	return rv;
    }

    esw_ipmc_info[unit].ipmc_l3_index[ipmc_id] = l3cfg.l3c_hw_index;

    rv = BCM_E_NONE;
#endif /* BCM_XGS12_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *	bcm_xgs_ipmc_put
 * Description:
 *	Overwrite an entry in the IPMC table.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	index - Table index to overwrite.
 *	data - IPMC entry information.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_put(int unit, int index, bcm_ipmc_addr_t *ipmc)
{
    IPMC_INIT(unit);

    return _xgs_ipmc_write(unit, index, ipmc);
}

/*
 * Function:
 *	bcm_xgs_ipmc_delete
 * Description:
 *	Delete an entry from the IPMC table.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	data - IPMC entry information.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	If BCM_IPMC_KEEP_ENTRY is true, the entry valid bit is cleared
 *	but the entry is not deleted from the table.
 */

int
bcm_xgs_ipmc_delete(int unit, bcm_ipmc_addr_t *ipmc)
{
    int         rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    int			ipmc_id = 0, repl = 0;
    ipmc_entry_t	ipmc_entry;
    _bcm_l3_cfg_t	l3cfg;
    bcm_port_t		port;
    bcm_pbmp_t		repl_pbmp, port_pbmp;

    IPMC_INIT(unit);

    if (!(ipmc->flags & BCM_IPMC_IP6)) {
        l3cfg.l3c_src_ip_addr = ipmc->s_ip_addr;
        l3cfg.l3c_ipmc_group = ipmc->mc_ip_addr;
        l3cfg.l3c_vid = ipmc->vid;
        l3cfg.l3c_flags = BCM_L3_IPMC;

        BCM_IF_ERROR_RETURN(bcm_xgs_l3_get(unit, &l3cfg));

        BCM_IF_ERROR_RETURN
	   (_xgs_ipmc_read(unit, l3cfg.l3c_ipmc_ptr, &ipmc_entry));

        if (soc_L3_IPMCm_field32_get(unit, &ipmc_entry, VALIDf) == 0) {
            /* Invalid entry???  */
	    return BCM_E_NOT_FOUND;
        }

        repl = soc_feature(unit, soc_feature_ip_mcast_repl);

        SOC_PBMP_CLEAR(repl_pbmp);

        if (repl) {
            if (SOC_IS_TUCANA(unit)) {
                SOC_PBMP_WORD_SET(repl_pbmp, 0,
                   soc_L3_IPMCm_field32_get(unit, &ipmc_entry, L3_BITMAP_M0f));
                SOC_PBMP_WORD_SET(repl_pbmp, 1,
                   soc_L3_IPMCm_field32_get(unit, &ipmc_entry, L3_BITMAP_M1f));
            } else {
                SOC_PBMP_WORD_SET(repl_pbmp, 0,
                   soc_L3_IPMCm_field32_get(unit, &ipmc_entry, L3_BITMAPf));
            }

            /* Only Ethernet ports have replication */
            SOC_PBMP_AND(repl_pbmp, PBMP_E_ALL(unit));
        }

        ipmc_id = l3cfg.l3c_ipmc_ptr;
    } else {
        if (ipmc->flags & BCM_IPMC_USE_IPMC_INDEX) {
            IPMC_ID(unit, ipmc->ipmc_index);
            ipmc_id = ipmc->ipmc_index;
        } else {
            /*
             * IPv6 MC delete on 5695 in context of Proxy
             * MUST pass a valid IPMC index
             */
            return BCM_E_PARAM;
        }
    }

    SOC_IF_ERROR_RETURN
	(soc_mem_write(unit, L3_IPMCm, MEM_BLOCK_ALL, ipmc_id,
		       soc_mem_entry_null(unit, L3_IPMCm)));

    if (!(ipmc->flags & BCM_IPMC_KEEP_ENTRY)) {
        _xgs_ipmc_id_free(unit, ipmc_id);

        if (ipmc->flags & BCM_IPMC_PROXY_IP6) {
            /*
             * No entry in L3 table to delete for v6 MC proxy
             */
            return BCM_E_NONE;
        }
        if (ipmc->flags & BCM_IPMC_IP6) {
            /* 
             * IPv6 IPMC entries must have proxy flag set on
             * XGS/XGS2, so we should never get here.
             */
            return BCM_E_PARAM;
        }

        BCM_IF_ERROR_RETURN(bcm_xgs_l3_del(unit, &l3cfg));
        esw_ipmc_info[unit].ipmc_l3_index[ipmc_id] = -1;
        if (repl) {
            BCM_PBMP_ITER(repl_pbmp, port) {
                BCM_PBMP_CLEAR(port_pbmp);
                BCM_PBMP_PORT_SET(port_pbmp, port);
                BCM_IF_ERROR_RETURN(bcm_esw_ipmc_repl_delete_all(unit, ipmc_id,
                                                                 port_pbmp));
            }
        }
    }

    rv = BCM_E_NONE;
#endif /* BCM_XGS12_SWITCH_SUPPORT */

    return rv;
}

/*
 * Function:
 *	bcm_xgs_ipmc_delete_all
 * Description:
 *	Delete all entries from the IPMC table.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_delete_all(int unit)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    _bcm_esw_ipmc_t	*info = IPMC_INFO(unit);
    int i, l3index;

    IPMC_INIT(unit);

    IPMC_LOCK(unit);
    XGS_IPMC_USED_ZERO(unit);
    rv = _xgs_ipmc_table_clear(unit);
    IPMC_UNLOCK(unit);
    SOC_IF_ERROR_RETURN(rv);

    for (i = 0; i < info->ipmc_size; i++) {
        l3index = info->ipmc_l3_index[i];
        BCM_IF_ERROR_RETURN(bcm_xgs_l3_del_by_index(unit, l3index));
    }

    if (soc_feature(unit, soc_feature_ip_mcast_repl)) {
        /* Simplest way to flush all IPMC repl info */
        BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_ipmc_repl_init(unit));
    }

#endif /* BCM_XGS12_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_xgs_ipmc_age
 * Purpose:
 *      Age out the ipmc entry by clearing the HIT bit when appropriate,
 *      the ipmc entry itself is removed if HIT bit is not set.
 * Parameters:
 *      unit       -  (IN) BCM device number.
 *      flags      -  (IN) The criteria used to age out ipmc table.
 *                         IPv6/IPv4
 *      age_cb     -  (IN) Call back routine.
 *      user_data  -  (IN) User provided cookie for callback.
 * Returns:
 *      BCM_E_XXX 
 */
int
bcm_xgs_ipmc_age(int unit, uint32 flags, bcm_ipmc_traverse_cb age_cb, 
                 void *user_data)
{
    int rv = BCM_E_UNAVAIL;    /* Operation return status.    */
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    int idx;                   /* Ipmc table iteration index. */
    _bcm_esw_ipmc_t *info;     /* Sw image pointer.           */
    bcm_ipmc_addr_t entry;     /* Ipmc entry iterator.        */
    

    IPMC_INIT(unit);
    IPMC_LOCK(unit);

    info = IPMC_INFO(unit);

    for (idx = 0; idx < info->ipmc_size; idx++) {
        if (XGS_IPMC_USED_ISSET(unit, idx)) {
            sal_memset(&entry, 0, sizeof(bcm_ipmc_addr_t));
            /* Read entry at specific index . */
            rv = bcm_xgs_ipmc_get(unit, idx, &entry);
            if (BCM_FAILURE(rv)) {
                break;
            }

            if ((flags & BCM_IPMC_IP6) != (entry.flags & BCM_IPMC_IP6)) {
                continue;
            }

            /* Make sure update only ipv4 or ipv6 entries. */
            if (entry.flags & BCM_IPMC_HIT) {
                /* Clear hit bit on used entry. */
                entry.flags |= BCM_IPMC_HIT_CLEAR;
                rv = bcm_xgs_ipmc_get(unit, idx, &entry);
                if (BCM_FAILURE(rv)) {
                    break;
                }
            } else {
                /* Delete & notify caller on unused entry. */
                rv = bcm_xgs_ipmc_delete(unit, &entry);
                if (BCM_FAILURE(rv)) {
                    break;
                }

                /* Invoke user callback. */
                if (NULL != age_cb) {
                    (*age_cb)(unit, &entry, user_data);
                }
            }
        }
    }
    IPMC_UNLOCK(unit);
#endif /* BCM_XGS12_SWITCH_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      bcm_xgs_ipmc_traverse
 * Purpose:
 *      Go through all valid ipmc entries, and call the callback function
 *      at each entry
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      flags     - (IN) The criteria used to age out ipmc table.
 *      cb        - (IN) User supplied callback function.
 *      user_data - (IN) User supplied cookie used in parameter 
 *                       in callback function.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs_ipmc_traverse(int unit, uint32 flags, bcm_ipmc_traverse_cb cb,
                      void *user_data)
{
    int rv = BCM_E_UNAVAIL;    /* Operation return status */
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    int idx;                   /* Ipmc table iteration index. */
    _bcm_esw_ipmc_t *info;     /* Sw image pointer.           */
    bcm_ipmc_addr_t entry;     /* Ipmc entry iterator.        */
    
    IPMC_INIT(unit);
    IPMC_LOCK(unit);

    info = IPMC_INFO(unit);

    for (idx = 0; idx < info->ipmc_size; idx++) {
        if (XGS_IPMC_USED_ISSET(unit, idx)) {
            sal_memset(&entry, 0, sizeof(bcm_ipmc_addr_t));
            /* Read entry at specific index . */
            rv = bcm_xgs_ipmc_get(unit, idx, &entry);
            if (BCM_FAILURE(rv)) {
                break;
            }

            if ((flags & BCM_IPMC_IP6) != (entry.flags & BCM_IPMC_IP6)) {
                continue;
            }

            /* Invoke user callback. */
            rv = (*cb)(unit, &entry, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
            if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                break;
            }
#endif
        }
    }
    IPMC_UNLOCK(unit);
#endif
    return (rv);
}

/*
 * Function:
 *	bcm_xgs_ipmc_enable
 * Purpose:
 *	Enable or disable IPMC chip functions.
 * Parameters:
 *	unit - Unit number
 *	enable - TRUE to enable; FALSE to disable
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_enable(int unit, int enable)
{
    IPMC_INIT(unit);

    return _bcm_xgs_ipmc_enable(unit, enable);
}

/*
 * Function:
 *	bcm_xgs_ipmc_src_port_check
 * Purpose:
 *	Enable or disable Source Port checking in IPMC lookups.
 * Parameters:
 *	unit - Unit number
 *	enable - TRUE to enable; FALSE to disable
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_src_port_check(int unit, int enable)
{
    IPMC_INIT(unit);

    return _bcm_xgs_ipmc_src_port_check(unit, enable);
}

/*
 * Function:
 *	bcm_xgs_ipmc_src_ip_search
 * Purpose:
 *	Enable or disable Source IP significance in IPMC lookups.
 * Parameters:
 *	unit - Unit number
 *	enable - TRUE to enable; FALSE to disable
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_src_ip_search(int unit, int enable)
{
    IPMC_INIT(unit);

    return _bcm_xgs_ipmc_src_ip_search(unit, enable);
}

#ifdef BCM_LYNX_SUPPORT
int _bcm_lynx_ipmc_repl_enabled(int unit, bcm_port_t port);
#endif

#ifdef BCM_DRACO15_SUPPORT
int _bcm_draco15_ipmc_repl_enabled(int unit, bcm_port_t port);
#endif

/*
 * Function:
 *	bcm_xgs_ipmc_egress_port_set
 * Description:
 *	Configure the IP Multicast egress properties
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - port to config.
 *	mac - MAC address.
 *	untag - 1: The IP multicast packet is transmitted as untagged packet.
 *		0: The IP multicast packet is transmitted as tagged packet
 *		with VLAN tag vid.
 *	vid  - VLAN ID.
 *	ttl_thresh - Drop IPMC packets if TTL of IP packet <= ttl_thresh.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_egress_port_set(int unit, bcm_port_t port,
			       const bcm_mac_t mac, int untag,
			       bcm_vlan_t vid, int ttl_thresh)
{
    uint64		cfg0, cfg1;
    uint32		cfg2;
    uint32		sa_byte5_2, sa_byte1_0;

    if (!SOC_PBMP_PORT_VALID(port) || !IS_E_PORT(unit, port)) {
        return BCM_E_BADID;
    }

    COMPILER_64_ZERO(cfg0);
    COMPILER_64_ZERO(cfg1);

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        uint64		sa64;

        COMPILER_64_ZERO(sa64);
        SAL_MAC_ADDR_TO_UINT64(mac, sa64);

        soc_reg64_field32_set(unit, EGR_IPMC_CFG0r, &cfg0,
                          TTL_THRESHf, ttl_thresh);
        soc_reg64_field_set(unit, EGR_IPMC_CFG0r, &cfg0, SAf, sa64);

        soc_reg64_field32_set(unit, EGR_IPMC_CFG1r, &cfg1,
                              UNTAG_VLANf, vid);
        soc_reg64_field32_set(unit, EGR_IPMC_CFG1r, &cfg1,
                              UNTAGf, untag ? 1 : 0);
        /*
         * By default, we set L2SWITCH_SAME_VLAN to 1, to ensure
         * the the replicated IPMC packets are routed only if the
         * replicated VLAN ID != ingress VLAN ID
         */
        soc_reg64_field32_set(unit, EGR_IPMC_CFG1r, &cfg1,
                              L2SWITCH_SAME_VLANf, 1);
   } else {
#endif
        cfg2 = 0;

        sa_byte5_2 = mac[0] << 24 | mac[1] << 16 | mac[2] << 8 | mac[3] << 0;
        sa_byte1_0 = mac[4] << 8 | mac[5] << 0;

        soc_reg64_field32_set(unit, EGR_IPMC_CFG0r, &cfg0,
                              SA_BYTE5_2f, sa_byte5_2);

        soc_reg64_field32_set(unit, EGR_IPMC_CFG1r, &cfg1,
                              SA_BYTE1_0f, sa_byte1_0);

#ifdef BCM_LYNX_SUPPORT
        if (SOC_IS_LYNX(unit)) {
            soc_reg64_field32_set(unit, EGR_IPMC_CFG1r, &cfg1,
                                  UNTAG_VLANf, vid);

            soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                              UNTAG_PKTf, untag ? 1 : 0);
            soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                              TTL_THRESHf, ttl_thresh);
            soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                              EGR_DISABLE_IPMC_REPLICATIONf,
                              _bcm_lynx_ipmc_repl_enabled(unit, port) ? 0 : 1);
        } else {
#endif
            soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                              UNTAGf, untag ? 1 : 0);
            soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                              VLIDf, vid);
            soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                              TTL_THRESHf, ttl_thresh);
#ifdef BCM_DRACO15_SUPPORT
            if (SOC_IS_DRACO15(unit)) {
                soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                                  IPMC_REPLICATIONf,
                        _bcm_draco15_ipmc_repl_enabled(unit, port) ? 1 : 0);
            }
#endif
#ifdef BCM_LYNX_SUPPORT
        }
#endif
        SOC_IF_ERROR_RETURN(WRITE_EGR_IPMC_CFG2r(unit, port, cfg2));
#ifdef BCM_TUCANA_SUPPORT
    }
#endif

    SOC_IF_ERROR_RETURN(WRITE_EGR_IPMC_CFG0r(unit, port, cfg0));
    SOC_IF_ERROR_RETURN(WRITE_EGR_IPMC_CFG1r(unit, port, cfg1));

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_xgs_ipmc_egress_port_get
 * Description:
 *	Return the IP Multicast egress properties
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - port to config.
 *	mac - (OUT) MAC address.
 *	untag - (OUT) 1: The IP multicast packet is transmitted as
 *			 untagged packet.
 *		      0: The IP multicast packet is transmitted as tagged
 *			 packet with VLAN tag vid.
 *	vid - (OUT) VLAN ID.
 *	ttl_thresh - (OUT) Drop IPMC packets if TTL <= ttl_thresh.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_xgs_ipmc_egress_port_get(int unit, bcm_port_t port, sal_mac_addr_t mac,
			       int *untag, bcm_vlan_t *vid, int *ttl_thresh)
{
    uint64		cfg0, cfg1;
    uint32		cfg2;
    uint32		sa_byte5_2, sa_byte1_0;

    if (!SOC_PBMP_PORT_VALID(port) || !IS_E_PORT(unit, port)) {
        return BCM_E_BADID;
    }

    SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG0r(unit, port, &cfg0));
    SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG1r(unit, port, &cfg1));

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        uint64		sa64;
        COMPILER_64_ZERO(sa64);
        sa64 = soc_reg64_field_get(unit, EGR_IPMC_CFG0r, cfg0, SAf);
        SAL_MAC_ADDR_FROM_UINT64(mac, sa64);

        *vid = soc_reg64_field32_get(unit, EGR_IPMC_CFG1r, cfg1, UNTAG_VLANf);
        *untag = soc_reg64_field32_get(unit, EGR_IPMC_CFG1r, cfg1, UNTAGf);
        *ttl_thresh = soc_reg64_field32_get(unit, EGR_IPMC_CFG0r,
                                            cfg0, TTL_THRESHf);
    } else {
#endif
        SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &cfg2));

        sa_byte5_2 = soc_reg64_field32_get(unit, EGR_IPMC_CFG0r,
                                           cfg0, SA_BYTE5_2f);
        sa_byte1_0 = soc_reg64_field32_get(unit, EGR_IPMC_CFG1r,
                                           cfg1, SA_BYTE1_0f);

        mac[0] = (sa_byte5_2 >> 24) & 0xff;
        mac[1] = (sa_byte5_2 >> 16) & 0xff;
        mac[2] = (sa_byte5_2 >>  8) & 0xff;
        mac[3] = (sa_byte5_2 >>  0) & 0xff;
        mac[4] = (sa_byte1_0 >>  8) & 0xff;
        mac[5] = (sa_byte1_0 >>  0) & 0xff;

#ifdef BCM_LYNX_SUPPORT
        if (SOC_IS_LYNX(unit)) {
            *vid = soc_reg64_field32_get(unit, EGR_IPMC_CFG1r,
                                         cfg1, UNTAG_VLANf);
            *untag = soc_reg_field_get(unit, EGR_IPMC_CFG2r, cfg2, UNTAG_PKTf);
        } else {
#endif
            *untag = soc_reg_field_get(unit, EGR_IPMC_CFG2r, cfg2, UNTAGf);
            *vid = soc_reg_field_get(unit, EGR_IPMC_CFG2r, cfg2, VLIDf);
#ifdef BCM_LYNX_SUPPORT
        }
#endif
        *ttl_thresh = soc_reg_field_get(unit, EGR_IPMC_CFG2r,
                                        cfg2, TTL_THRESHf);
#ifdef BCM_TUCANA_SUPPORT
    }
#endif

    return BCM_E_NONE;
}


/*
 * Draco 1.5 IPMC replication code.
 * This should be separated out when the XGS IPMC code is unified.
 */

#define REPL_PORT_CHECK(unit, port) \
    if (!IS_E_PORT(unit, port)) { return BCM_E_PARAM; }

#define DRACO15_IPMC_GROUPS        1024

typedef struct _draco15_repl_port_info_s {
    int enabled;
    int vlan_count[DRACO15_IPMC_GROUPS];  /* # VLANs the port repl to */
    SHR_BITDCLNAME(group_lapsed, DRACO15_IPMC_GROUPS);
    /* (Group, port) deactivated, pending index release (See note below) */
} _draco15_repl_port_info_t;

typedef struct _draco15_repl_info_s {
    int link_callback_set;        /* Started callback to clear lapsed group */
    bcm_pbmp_t lapsed_pbmp;       /* Bitmap of ports with lapsed groups */
    uint16 ipmc_vlan_total;       /* Keep track of total and */
    uint32 *bitmap_entries_used;  /* free entries of IPMC_VLAN table */
    _bcm_repl_list_info_t *repl_list_info;
    _draco15_repl_port_info_t *port_info[SOC_MAX_NUM_PORTS];
} _draco15_repl_info_t;

static _draco15_repl_info_t *_draco15_repl_info[BCM_MAX_NUM_UNITS];

#define IPMC_REPL_LOCK(_u_)                   \
    {                                          \
        soc_mem_lock(_u_, MMU_MEMORIES1_IPMC_GROUP_TBLm);   \
        soc_mem_lock(_u_, MMU_MEMORIES1_IPMC_VLAN_TBLm);   \
    }
#define IPMC_REPL_UNLOCK(_u_)                 \
    {                                          \
        soc_mem_unlock(_u_, MMU_MEMORIES1_IPMC_GROUP_TBLm); \
        soc_mem_unlock(_u_, MMU_MEMORIES1_IPMC_VLAN_TBLm); \
    }
#define IPMC_REPL_LINK_CB_SET(_u_) \
	_draco15_repl_info[_u_]->link_callback_set
#define IPMC_REPL_TOTAL(_u_) \
	_draco15_repl_info[_u_]->ipmc_vlan_total
#define IPMC_REPL_VE_USED_GET(_u_, _i_) \
	SHR_BITGET(_draco15_repl_info[_u_]->bitmap_entries_used, _i_)
#define IPMC_REPL_VE_USED_SET(_u_, _i_) \
	SHR_BITSET(_draco15_repl_info[_u_]->bitmap_entries_used, _i_)
#define IPMC_REPL_VE_USED_CLR(_u_, _i_) \
	SHR_BITCLR(_draco15_repl_info[_u_]->bitmap_entries_used, _i_)
#define IPMC_REPL_LIST_INFO(_u_) \
	_draco15_repl_info[_u_]->repl_list_info

#define IPMC_REPL_PORT_INFO(_u_, _p_) \
	_draco15_repl_info[_u_]->port_info[_p_]
#define IPMC_REPL_PORT_VLAN_COUNT(_u_, _p_, _ipmc_id_) \
	_draco15_repl_info[_u_]->port_info[_p_]->vlan_count[_ipmc_id_]

#define IPMC_REPL_LAPSED_SET(_u_, _p_, _g_) \
	SHR_BITSET(_draco15_repl_info[_u_]->port_info[_p_]->group_lapsed, _g_)
#define IPMC_REPL_LAPSED_GET(_u_, _p_, _g_) \
	SHR_BITGET(_draco15_repl_info[_u_]->port_info[_p_]->group_lapsed, _g_)
#define IPMC_REPL_LAPSED_CLR(_u_, _p_, _g_) \
	SHR_BITCLR(_draco15_repl_info[_u_]->port_info[_p_]->group_lapsed, _g_)

#define IPMC_REPL_LAPSED_PORT_SET(_u_, _p_) \
	BCM_PBMP_PORT_ADD(_draco15_repl_info[_u_]->lapsed_pbmp,_p_)
#define IPMC_REPL_LAPSED_PORT_GET(_u_, _p_) \
	BCM_PBMP_MEMBER(_draco15_repl_info[_u_]->lapsed_pbmp,_p_)
#define IPMC_REPL_LAPSED_PORT_CLR(_u_, _p_) \
	BCM_PBMP_PORT_REMOVE(_draco15_repl_info[_u_]->lapsed_pbmp,_p_)

/* A couple of helper functions to prevent active IPMC configurations
 * from being replicated on 0 VLANS. */

/*
 * Function:
 *	_draco15_ipmc_repl_l3port_valid
 * Description:
 *      Verify replication VLANs exist for IPMC (group, port).
 *      Remove port from L3 bitmap if not.
 */

STATIC void
_draco15_ipmc_repl_l3port_valid(int unit, int ipmc_id, bcm_pbmp_t *l3_pbmp)
{
    bcm_port_t port;
    bcm_pbmp_t res_pbmp;

    BCM_PBMP_CLEAR(res_pbmp);
    BCM_PBMP_ITER(*l3_pbmp, port) {
        if (!IS_E_PORT(unit, port))
            BCM_PBMP_PORT_ADD(res_pbmp, port);
        else if (IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) > 0) {
            /* Based on hw design e port replication count must be > 0 */
            BCM_PBMP_PORT_ADD(res_pbmp, port);
        }
    }

    BCM_PBMP_ASSIGN(*l3_pbmp, res_pbmp);
}

/*
 * Function:
 *	_draco15_ipmc_repl_l3port_set
 * Description:
 *	Set/clear an L3 port from the IPMC entry for the given id.
 */

STATIC int
_draco15_ipmc_repl_l3port_set(int unit, int ipmc_id, bcm_port_t port, int set)
{
    int rv;
    ipmc_entry_t	ipmc_entry;
    bcm_pbmp_t		l3_pbmp, l3_pbmp_new;


    IPMC_INIT(unit);
    IPMC_ID(unit, ipmc_id);

    SOC_PBMP_CLEAR(l3_pbmp);

    IPMC_LOCK(unit);
    if ((rv = soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY,
                           ipmc_id, &ipmc_entry)) < 0) {
        IPMC_UNLOCK(unit);
        return rv;
    }

    SOC_PBMP_WORD_SET(l3_pbmp, 0,
                      soc_L3_IPMCm_field32_get(unit, &ipmc_entry, L3_BITMAPf));
    SOC_PBMP_ASSIGN(l3_pbmp_new, l3_pbmp);
    if (set) {
        SOC_PBMP_PORT_ADD(l3_pbmp_new, port);
    } else {
        SOC_PBMP_PORT_REMOVE(l3_pbmp_new, port);
    }
    if (SOC_PBMP_NEQ(l3_pbmp_new, l3_pbmp)) { /* Changed, update */
        soc_L3_IPMCm_field32_set(unit, &ipmc_entry, L3_BITMAPf,
                                 SOC_PBMP_WORD_GET(l3_pbmp_new, 0));
        rv = soc_mem_write(unit, L3_IPMCm, MEM_BLOCK_ALL,
                           ipmc_id, &ipmc_entry);
    }

    IPMC_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *	bcm_draco15_ipmc_repl_init
 * Purpose:
 *	Initialize IPMC replication.
 * Parameters:
 *	unit     - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_draco15_ipmc_repl_init(int unit)
{
    bcm_port_t		port;
    int			alloc_size;

    if (!SOC_IS_DRACO15(unit)) {
	return BCM_E_UNAVAIL;
    }

    bcm_draco15_ipmc_repl_detach(unit);

    /* Allocate struct for IPMC replication booking keeping */
    alloc_size = sizeof(_draco15_repl_info_t);
    _draco15_repl_info[unit] = sal_alloc(alloc_size, "IPMC repl info");
    if (_draco15_repl_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_draco15_repl_info[unit], 0, alloc_size);

    IPMC_REPL_TOTAL(unit) = 
    	soc_mem_index_count(unit, MMU_MEMORIES1_IPMC_VLAN_TBLm);
    alloc_size = SHR_BITALLOCSIZE(IPMC_REPL_TOTAL(unit));
    _draco15_repl_info[unit]->bitmap_entries_used =
        sal_alloc(alloc_size, "IPMC repl entries used");
    if (_draco15_repl_info[unit]->bitmap_entries_used == NULL) {
        bcm_draco15_ipmc_repl_detach(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(_draco15_repl_info[unit]->bitmap_entries_used, 0, alloc_size);

    /* Always reserve slot 0 */
    IPMC_REPL_VE_USED_SET(unit, 0);

    PBMP_E_ITER(unit, port) {
        alloc_size = sizeof(_draco15_repl_port_info_t);
        IPMC_REPL_PORT_INFO(unit, port) =
            sal_alloc(alloc_size, "IPMC repl port info");
        if (IPMC_REPL_PORT_INFO(unit, port) == NULL) {
	    bcm_draco15_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(IPMC_REPL_PORT_INFO(unit, port), 0, alloc_size);
        IPMC_REPL_PORT_INFO(unit, port)->enabled = 1;
    }

    SOC_IF_ERROR_RETURN
	(soc_mem_clear(unit, MMU_MEMORIES1_IPMC_GROUP_TBLm,
                       MEM_BLOCK_ALL, FALSE));

    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, MMU_MEMORIES1_IPMC_VLAN_TBLm,
                       MEM_BLOCK_ALL, FALSE));

    return BCM_E_NONE;
}

int
_bcm_draco15_ipmc_repl_enabled(int unit, bcm_port_t port)
{
    return IPMC_REPL_PORT_INFO(unit, port)->enabled;
}

/*
 * Function:
 *	bcm_draco15_ipmc_repl_detach
 * Purpose:
 *	Initialize IPMC replication.
 * Parameters:
 *	unit     - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_draco15_ipmc_repl_detach(int unit)
{
    bcm_port_t		port;
    _bcm_repl_list_info_t *rli_current, *rli_free;

    if (!SOC_IS_DRACO15(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (_draco15_repl_info[unit] != NULL) {
	PBMP_E_ITER(unit, port) {
	    if (_draco15_repl_info[unit]->port_info[port] != NULL) {
		sal_free(_draco15_repl_info[unit]->port_info[port]);
	    }
	}

        if (_draco15_repl_info[unit]->bitmap_entries_used != NULL) {
            sal_free(_draco15_repl_info[unit]->bitmap_entries_used);
        }

        if (_draco15_repl_info[unit]->repl_list_info != NULL) {
            rli_current = IPMC_REPL_LIST_INFO(unit);
            while (rli_current != NULL) {
                rli_free = rli_current;
                rli_current = rli_current->next;
                sal_free(rli_free);
            }
        }

	sal_free(_draco15_repl_info[unit]);
	_draco15_repl_info[unit] = NULL;
    }

    return BCM_E_NONE;
}

STATIC soc_field_t
_ipmc_group_field_get(int unit, bcm_port_t port)
{
    if (!SOC_IS_DRACO15(unit)) {
        return INVALIDf;
    }

    switch(port) {
    case 0:
        return PORT0_1STPTRf;
    case 1:
        return PORT1_1STPTRf;
    case 2:
        return PORT2_1STPTRf;
    case 3:
        return PORT3_1STPTRf;
    case 4:
        return PORT4_1STPTRf;
    case 5:
        return PORT5_1STPTRf;
    case 6:
        return PORT6_1STPTRf;
    case 7:
        return PORT7_1STPTRf;
    case 8:
        return PORT8_1STPTRf;
    case 9:
        return PORT9_1STPTRf;
    case 10:
        return PORT10_1STPTRf;
    case 11:
        return PORT11_1STPTRf;
    default:
        return INVALIDf;
    }
}

/*
 * Function:
 *	_bcm_draco15_ipmc_repl_next_free_ptr
 * Purpose:
 *	Internal routine to return an available slot
 */

STATIC int
_bcm_draco15_ipmc_repl_next_free_ptr(int unit)
{
    int			ix, bit;
    SHR_BITDCL		not_ptrs;

    for (ix = 0; ix < _SHR_BITDCLSIZE(IPMC_REPL_TOTAL(unit)); ix++) {
        not_ptrs = ~_draco15_repl_info[unit]->bitmap_entries_used[ix];
        if (not_ptrs) {
            for (bit = 0; bit < SHR_BITWID; bit++) {
                if (not_ptrs & (1 << bit)) {
                    return (ix * SHR_BITWID) + bit;
                }
            }
        }
    }

    return -1;
}

/*
 * Function:
 *	_draco15_ipmc_repl_link_cb
 * Description:
 *	On link down events for E ports, clear the lapsed groups for the port
 *      since all of the cells in the MMU will be drained.
 */

STATIC void
_draco15_ipmc_repl_link_cb(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    int ipmc_id, rv = BCM_E_NONE;
    mmu_memories1_ipmc_group_tbl_entry_t group_entry;
    int	vlan_ptr;

    if (IS_E_PORT(unit, port) && !info->linkstatus &&
        IPMC_REPL_LAPSED_PORT_GET(unit, port)) { /* Link down event */
        IPMC_REPL_LOCK(unit);
        
        for (ipmc_id = 0; ipmc_id < DRACO15_IPMC_GROUPS; ipmc_id++) {
            if (IPMC_REPL_LAPSED_GET(unit, port, ipmc_id)) {
                if ((rv = READ_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit,
                               MEM_BLOCK_ANY, ipmc_id, &group_entry)) < 0) {
                    break;
                }

                vlan_ptr =
                    soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_get(unit,
                        &group_entry, _ipmc_group_field_get(unit, port));

                /* Clear the group table entry now */
                soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_set(unit,
                    &group_entry, _ipmc_group_field_get(unit, port), 0);
                if ((rv = WRITE_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit,
                                MEM_BLOCK_ALL, ipmc_id, &group_entry)) < 0) {
                    break;
                }

                IPMC_REPL_LAPSED_CLR(unit, port, ipmc_id);
                IPMC_REPL_VE_USED_CLR(unit, vlan_ptr);
            }
        }

        if (rv >= 0) {
            /* The loop completed without error */
            IPMC_REPL_LAPSED_PORT_CLR(unit, port);
        }

        IPMC_REPL_UNLOCK(unit);
    }
}

/*
 * Function:
 *	_draco15_ipmc_repl_xqempty_check
 * Description:
 *	Examine the MMU state for a port and determine if any packets are
 *      either pending in the queue or in the process of IPMC replication.
 */

STATIC int
_draco15_ipmc_repl_xqempty_check(int unit, bcm_port_t port, int *empty_ret)
{
    int empty = FALSE;
    uint32 rval, pbm;
    bcm_pbmp_t pbmp;

    SOC_IF_ERROR_RETURN(READ_XQEMPTYr(unit, port, &rval));

    if (rval == 0xff) { /* All COS queues empty */
        SOC_IF_ERROR_RETURN(READ_PPPEMPTYr(unit, &rval));
        pbm = soc_reg_field_get(unit, PPPEMPTYr, rval, PORT_BITMAPf);
        SOC_PBMP_WORD_SET(pbmp, 0, pbm);
        if (BCM_PBMP_MEMBER(pbmp, port)) {
            /* IPMC replication not in progress on this port */
            empty = TRUE;
        }
    }

    if (!empty && !IPMC_REPL_LINK_CB_SET(unit)) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_linkscan_register(unit, _draco15_ipmc_repl_link_cb));
        IPMC_REPL_LINK_CB_SET(unit) = TRUE;
    }
    *empty_ret = empty;
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_draco15_ipmc_repl_get
 * Purpose:
 *	Return set of VLANs selected for port's replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to list.
 *	vlan_vec - (OUT) vector of replicated VLANs common to selected ports.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_draco15_ipmc_repl_get(int unit, int ipmc_id, bcm_port_t port,
                        bcm_vlan_vector_t vlan_vec)
{
    int			rv = BCM_E_NONE;
    uint32		ms_bit;
    uint32		ls_bits[2];
    mmu_memories1_ipmc_group_tbl_entry_t group_entry;
    mmu_memories1_ipmc_vlan_tbl_entry_t vlan_entry;
    int			vlan_ptr, last_vlan_ptr, vlan_count;

    if (!SOC_IS_DRACO15(unit)) {
	return BCM_E_UNAVAIL;
    }

    IPMC_INIT(unit);
    IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    BCM_VLAN_VEC_ZERO(vlan_vec);

    IPMC_REPL_LOCK(unit);

    if (IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        if ((rv = READ_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit, MEM_BLOCK_ANY,
                                          ipmc_id, &group_entry)) < 0) {
            goto get_done;
        }
        vlan_ptr = soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_get(unit,
                           &group_entry, _ipmc_group_field_get(unit, port));

        last_vlan_ptr = -1;
        vlan_count = 0;
        while (vlan_ptr != last_vlan_ptr) {
            if ((rv = READ_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit, MEM_BLOCK_ANY,
                                              vlan_ptr, &vlan_entry)) < 0) {
                goto get_done;
            }
            ms_bit = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                           &vlan_entry, MSB_VLANf);
            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_get(unit, &vlan_entry,
                                                       LSB_VLAN_BMf, ls_bits);
            vlan_vec[2 * ms_bit + 0] = ls_bits[0];
            vlan_vec[2 * ms_bit + 1] = ls_bits[1];
            last_vlan_ptr = vlan_ptr;
            vlan_ptr = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                                   &vlan_entry, NEXTPTRf);

            vlan_count +=
                _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
            if (vlan_count >= IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
                break;
            }
        }
    }

 get_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_draco15_ipmc_repl_add
 * Purpose:
 *	Add VLAN to selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id    - The index number.
 *	port     - port to add.
 *	vlan     - VLAN to replicate.
 * Returns:
 *	BCM_E_XXX
 * Note:
 *      All IPMC groups for all ports share one IPMV_VLAN
 *      table, and contends for entries in this table.
 *      In other words, the total number of VLANs for replication
 *      for all ports and all IPMC groups is constrained by
 *      the max size of IPMC VLAN table.
 */

int
bcm_draco15_ipmc_repl_add(int unit, int ipmc_id, bcm_port_t port,
                          bcm_vlan_t vlan)
{
    int			rv = BCM_E_NONE;
    uint32		ls_bits[2];
    mmu_memories1_ipmc_group_tbl_entry_t group_entry;
    mmu_memories1_ipmc_vlan_tbl_entry_t vlan_entry, prev_vlan_entry;
    int			ms_bit, target_ms_bit, ls_bit;
    int			vlan_ptr, last_vlan_ptr, vlan_count;
    pbmp_t              pbmp, ubmp;

    if (!SOC_IS_DRACO15(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (SOC_IPMCREPLSHR_GET(unit)) {
        return BCM_E_CONFIG;
    }

    IPMC_INIT(unit);
    IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    /* Check if port belongs to this VLAN */
    BCM_IF_ERROR_RETURN(bcm_esw_vlan_port_get(unit, vlan, &pbmp, &ubmp));
    if (!BCM_PBMP_MEMBER(pbmp, port)) {
        return BCM_E_PARAM;
    }

    IPMC_REPL_LOCK(unit);
    if ((rv = READ_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit, MEM_BLOCK_ANY, ipmc_id,
                                                 &group_entry)) < 0) {
        goto add_done;
    }

    vlan_ptr = soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_get(unit,
                       &group_entry, _ipmc_group_field_get(unit, port));

    target_ms_bit = (vlan >> 6) & 0x3f;
    ls_bit = vlan & 0x3f;

    last_vlan_ptr = -1;
    vlan_count = 0;
    ms_bit = -1;

    if (IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        /*
         * Add to existing chain
         */
        while (vlan_ptr != last_vlan_ptr) {
            if ((rv = READ_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit, MEM_BLOCK_ANY,
                                              vlan_ptr, &vlan_entry)) < 0) {
                goto add_done;
            }

            ms_bit = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                           &vlan_entry, MSB_VLANf);
            if (ms_bit == target_ms_bit) {
                /* Add to existing entry */
                break;
            }

            last_vlan_ptr = vlan_ptr;
            vlan_ptr = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                                   &vlan_entry, NEXTPTRf);

            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_get(unit, &vlan_entry,
                                                       LSB_VLAN_BMf, ls_bits);
            vlan_count +=
                _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
            if (vlan_count >= IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
                break;
            }
        }
    }

    if (ms_bit != target_ms_bit) {
        if (IPMC_REPL_LAPSED_GET(unit, port, ipmc_id)) {
            /* Lapsed entry, use the old pointer and overwrite it */
            ms_bit = target_ms_bit; /* Block write to group table below */
        } else {
            /* New entry, find slot */
            vlan_ptr = _bcm_draco15_ipmc_repl_next_free_ptr(unit);
        }
        if (vlan_ptr < 0) {
            /* Out of LS entries */
            rv = BCM_E_RESOURCE;
            goto add_done;
        }

        if (last_vlan_ptr >= 0) {
           /* Save last entry for later write */
            sal_memcpy(&prev_vlan_entry, &vlan_entry, sizeof(vlan_entry));
        }

        sal_memset(&vlan_entry, 0, sizeof(vlan_entry));
        soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit, &vlan_entry,
                                                     MSB_VLANf, target_ms_bit);
        /* Point to self */
        soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit, &vlan_entry,
                                                     NEXTPTRf, vlan_ptr);

        IPMC_REPL_VE_USED_SET(unit, vlan_ptr);
        IPMC_REPL_LAPSED_CLR(unit, port, ipmc_id);
            /* Guarantee old group, port state cleared */
     }

    /* Insert replacement */
    soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_get(unit, &vlan_entry,
                                               LSB_VLAN_BMf, ls_bits);
    if (ls_bits[ls_bit / 32] & (1 << (ls_bit % 32))) {
        /* Already exists! */
        rv = BCM_E_EXISTS;
        goto add_done;
    }
    ls_bits[ls_bit / 32] |= 1 << (ls_bit % 32);
    soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_set(unit, &vlan_entry,
                                               LSB_VLAN_BMf, ls_bits);


    if ((rv = WRITE_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit, MEM_BLOCK_ALL,
                                                 vlan_ptr, &vlan_entry)) < 0) {
        goto add_done;
    }

    if (ms_bit != target_ms_bit) {
        if (last_vlan_ptr < 0) {
            /* Update group table */
            soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_set(unit, &group_entry,
                              _ipmc_group_field_get(unit, port), vlan_ptr);
            if ((rv = WRITE_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit, MEM_BLOCK_ALL,
                                               ipmc_id, &group_entry)) < 0) {
                goto add_done;
            }
        } else {
            /* Update previous last entry */
            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit,
                              &prev_vlan_entry, NEXTPTRf, vlan_ptr);
            if ((rv = WRITE_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit, MEM_BLOCK_ALL,
                                last_vlan_ptr, &prev_vlan_entry)) < 0) {
                goto add_done;
            }
        }
    }

    if (!IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        rv = _draco15_ipmc_repl_l3port_set(unit, ipmc_id, port, TRUE);
    }

    IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)++;

 add_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_draco15_ipmc_repl_delete
 * Purpose:
 *	Remove VLAN from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id    - The index number.
 *	port     - port to remove.
 *	vlan     - VLAN to delete from replication.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_draco15_ipmc_repl_delete(int unit, int ipmc_id, bcm_port_t port,
                                     bcm_vlan_t vlan)
{
    int			rv = BCM_E_NONE;
    uint32		ls_bits[2];
    mmu_memories1_ipmc_group_tbl_entry_t group_entry;
    mmu_memories1_ipmc_vlan_tbl_entry_t vlan_entry, prev_vlan_entry;
    int			ms_bit, target_ms_bit, ls_bit, xqempty;
    int			vlan_ptr, last_vlan_ptr, next_vlan_ptr, vlan_count;

    if (!SOC_IS_DRACO15(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (SOC_IPMCREPLSHR_GET(unit)) {
        return BCM_E_CONFIG;
    }

    IPMC_INIT(unit);
    IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    if (!IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        return BCM_E_NOT_FOUND;
    }

    IPMC_REPL_LOCK(unit);
    if ((rv = READ_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit, MEM_BLOCK_ANY, ipmc_id,
                                                 &group_entry)) < 0) {
        goto del_done;
    }
    vlan_ptr = soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_get(unit,
                       &group_entry, _ipmc_group_field_get(unit, port));

    target_ms_bit = (vlan >> 6) & 0x3f;
    ls_bit = vlan & 0x3f;

    last_vlan_ptr = 0; /* Since ptr 0 should never be used! */
    vlan_count = 0;
    ms_bit = -1;

    while (vlan_ptr != last_vlan_ptr) {
        if ((rv = READ_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit, MEM_BLOCK_ANY,
                                          vlan_ptr, &vlan_entry)) < 0) {
            goto del_done;
        }

        ms_bit = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                             &vlan_entry, MSB_VLANf);
        if (ms_bit == target_ms_bit) {
            /* Delete existing entry */
            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_get(unit, &vlan_entry,
                                                       LSB_VLAN_BMf, ls_bits);
            if (!(ls_bits[ls_bit / 32] & (1 << (ls_bit % 32)))) {
                /* Doesn't exist! */
                rv = BCM_E_NOT_FOUND;
                goto del_done;
            }
            ls_bits[ls_bit / 32] &= ~(1 << (ls_bit % 32));
            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_set(unit, &vlan_entry,
                                                       LSB_VLAN_BMf, ls_bits);

            if ((ls_bits[0] == 0) && (ls_bits[1] == 0)) {
                /* Wiped out the entry */
                /* Record before overwrite */
                next_vlan_ptr =
                    soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                                &vlan_entry, NEXTPTRf);
                soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit,
                                            &vlan_entry, MSB_VLANf, 0);
                soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit,
                                            &vlan_entry, NEXTPTRf, 0);
                if (last_vlan_ptr == 0) {
                    /* First entry for this (group, port) */
                    if (vlan_ptr == next_vlan_ptr) {
                        /* Only link in chain */
                        next_vlan_ptr = 0;
                        /* Need to disable group in L3_IPMC table
                         * before setting port pointer to 0 */
                        if ((rv = _draco15_ipmc_repl_l3port_set(unit,
                                           ipmc_id, port, FALSE)) < 0) {
                            goto del_done;
                        }
                        if ((rv = _draco15_ipmc_repl_xqempty_check(unit,
                                               port, &xqempty)) < 0) {
                            goto del_done;
                        }
                        if (!xqempty) {
                            /* Skip writes, record for later */
                            IPMC_REPL_LAPSED_SET(unit, port, ipmc_id);
                            IPMC_REPL_LAPSED_PORT_SET(unit, port);
                            IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)--;
                            goto del_done;
                        }
                    }

                    soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_set(unit,
                        &group_entry, _ipmc_group_field_get(unit, port),
                        next_vlan_ptr);
                    if ((rv = WRITE_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit,
                                MEM_BLOCK_ALL, ipmc_id, &group_entry)) < 0) {
                        goto del_done;
                    }
                } else {
                    /* Link over with last entry */
                    if (vlan_ptr == next_vlan_ptr) {
                        /* If this is end of chain, end on previous link */
                        next_vlan_ptr = last_vlan_ptr;
                    }
                    soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit,
                            &prev_vlan_entry, NEXTPTRf, next_vlan_ptr);
                    if ((rv = WRITE_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit,
                                    MEM_BLOCK_ALL, last_vlan_ptr,
                                    &prev_vlan_entry)) < 0) {
                        goto del_done;
                    }
                }

                IPMC_REPL_VE_USED_CLR(unit, vlan_ptr);
            }

            if ((rv = WRITE_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit,
                                MEM_BLOCK_ALL, vlan_ptr, &vlan_entry)) < 0) {
                goto del_done;
            }

            IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)--;
            goto del_done;
        }

        last_vlan_ptr = vlan_ptr;
        vlan_ptr = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                               &vlan_entry, NEXTPTRf);
        sal_memcpy(&prev_vlan_entry, &vlan_entry,
                   sizeof(mmu_memories1_ipmc_vlan_tbl_entry_t));

        soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_get(unit, &vlan_entry,
                                                   LSB_VLAN_BMf, ls_bits);
        vlan_count +=
            _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
        if (vlan_count >= IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
            rv = BCM_E_NOT_FOUND;
            goto del_done;
        }
    }

    rv = BCM_E_NOT_FOUND;

 del_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_draco15_ipmc_repl_delete_all
 * Purpose:
 *	Remove all VLANs from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id    - The MC index number.
 *	port     - port from which to remove VLANs.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_draco15_ipmc_repl_delete_all(int unit, int ipmc_id, bcm_port_t port)
{
    int			rv = BCM_E_NONE;
    uint32		ls_bits[2];
    mmu_memories1_ipmc_group_tbl_entry_t group_entry;
    mmu_memories1_ipmc_vlan_tbl_entry_t vlan_entry;
    int			vlan_ptr, last_vlan_ptr, xqempty,
                        ls_bit, leave_one= FALSE;

    if (!SOC_IS_DRACO15(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (SOC_IPMCREPLSHR_GET(unit)) {
        return BCM_E_CONFIG;
    }

    IPMC_INIT(unit);
    IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    if (!IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        /* Nothing to do */
        return BCM_E_NONE;
    }

    IPMC_REPL_LOCK(unit);

    if ((rv = READ_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit, MEM_BLOCK_ANY, ipmc_id,
                                                 &group_entry)) < 0) {
        goto del_all_done;
    }
    vlan_ptr = soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_get(unit,
                       &group_entry, _ipmc_group_field_get(unit, port));

    soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_set(unit,
                      &group_entry, _ipmc_group_field_get(unit, port), 0);
    /* Need to disable group in L3_IPMC table before setting port
     * pointer to 0 */
    if ((rv = _draco15_ipmc_repl_l3port_set(unit, ipmc_id,
                                            port, FALSE)) < 0) {
        goto del_all_done;
    }
    if ((rv = _draco15_ipmc_repl_xqempty_check(unit, port, &xqempty)) < 0) {
        goto del_all_done;
    }
    if (!xqempty) {
        IPMC_REPL_LAPSED_SET(unit, port, ipmc_id);
        IPMC_REPL_LAPSED_PORT_SET(unit, port);
        leave_one = TRUE;
        /* Leave one VLAN behind to hold a place until it is safe */
    }
    if (!leave_one) {
        if ((rv = WRITE_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit,
                        MEM_BLOCK_ALL, ipmc_id, &group_entry)) < 0) {
            goto del_all_done;
        }
    }

    last_vlan_ptr = 0; /* Since ptr 0 should never be used! */

    while (vlan_ptr != last_vlan_ptr) {
        if ((rv = READ_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit, MEM_BLOCK_ANY,
                                          vlan_ptr, &vlan_entry)) < 0) {
            goto del_all_done;
        }

        last_vlan_ptr = vlan_ptr;
        vlan_ptr = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                               &vlan_entry, NEXTPTRf);

        soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_get(unit, &vlan_entry,
                                                   LSB_VLAN_BMf, ls_bits);
        IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) -=
            _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);

        if (leave_one) {
            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit, &vlan_entry,
                                             NEXTPTRf, last_vlan_ptr);
            /* Find first set bit */
            for (ls_bit = 0; ls_bit < 64; ls_bit++) {
                if (ls_bits[ls_bit / 32] & (1 << (ls_bit % 32))) {
                    /* Now make it the only one */
                    ls_bits[ls_bit / 32] = (1 << (ls_bit % 32));
                    if (ls_bit < 32) {
                        ls_bits[1] = 0;
                    }
                    soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_set(unit,
                            &vlan_entry, LSB_VLAN_BMf, ls_bits);
                    break;
                }
            }
            if (ls_bit == 64) {
                /* No bits set, why was this entry in the list here? */
                rv = BCM_E_NOT_FOUND;
                goto del_all_done;
            }
            if ((rv = WRITE_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit,
                        MEM_BLOCK_ALL, last_vlan_ptr, &vlan_entry)) < 0) {
                goto del_all_done;
            }            
            leave_one = FALSE; /* We've done what was needed */
        } else {
            if ((rv = WRITE_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit,
                        MEM_BLOCK_ALL, last_vlan_ptr, soc_mem_entry_null(unit,
                            MMU_MEMORIES1_IPMC_VLAN_TBLm))) < 0) {
                goto del_all_done;
            }

            IPMC_REPL_VE_USED_CLR(unit, last_vlan_ptr);
        }

        if (IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) < 0) {
            IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) = 0;
            goto del_all_done;
        }
    }

    /* Whether we were out of sync or not, we're now at 0 count */
    IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) = 0;

 del_all_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_draco15_ipmc_egress_intf_add
 * Purpose:
 *	Add VLAN to selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	index    - The IPMC index number.
 *	port     - port to add.
 *	l3_intf  - L3 interface containing the VLAN to replicate.      
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_draco15_ipmc_egress_intf_add(int unit, int index, bcm_port_t port,
                                 bcm_l3_intf_t *l3_intf)
{
    int rv = BCM_E_NONE;

    rv = bcm_draco15_ipmc_repl_add(unit, index, port, l3_intf->l3a_vid);
    return rv;
}

/*
 * Function:
 *      bcm_draco15_ipmc_egress_intf_delete
 * Purpose:
 *	Remove VLAN from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	index    - The IPMC index number.
 *	port     - port to add.
 *	l3_intf  - L3 interface containing the replicated  VLAN.      
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_draco15_ipmc_egress_intf_delete(int unit, int index, bcm_port_t port, 
                                    bcm_l3_intf_t *l3_intf)
{
    int rv = BCM_E_NONE;

    rv = bcm_draco15_ipmc_repl_delete(unit, index, port, l3_intf->l3a_vid);
    return rv;
}

/*
 * Function:
 *	_bcm_draco15_repl_list_compare
 * Description:
 *	Compare HW list starting at vlan_index to the VLAN list contained
 *      in vlan_vec.
 */

STATIC int
_bcm_draco15_repl_list_compare(int unit, int vlan_index,
                               bcm_vlan_vector_t vlan_vec)
{
    uint32		ms_bit, hw_ms_bit;
    uint32		ls_bits[2], hw_ls_bits[2];
    mmu_memories1_ipmc_vlan_tbl_entry_t vlan_entry;
    int	                vlan_ptr, last_vlan_ptr;

    last_vlan_ptr = -1;
    vlan_ptr = vlan_index;

    for (ms_bit = 0; ms_bit <= 0x3f; ms_bit++) {
        ls_bits[0] = vlan_vec[2 * ms_bit + 0];
        ls_bits[1] = vlan_vec[2 * ms_bit + 1];
        if ((ls_bits[0] != 0) || (ls_bits[1] != 0)) {
            if (vlan_ptr == last_vlan_ptr) { /* HW list end, not app list */
                return BCM_E_NOT_FOUND;
            }
            SOC_IF_ERROR_RETURN
                (READ_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit, MEM_BLOCK_ANY,
                                                   vlan_ptr, &vlan_entry));
            hw_ms_bit = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                       &vlan_entry, MSB_VLANf);
            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_get(unit, &vlan_entry,
                                                   LSB_VLAN_BMf, hw_ls_bits);
            if ((hw_ms_bit != ms_bit) || (ls_bits[0] != hw_ls_bits[0]) ||
                (ls_bits[1] != hw_ls_bits[1])) {
                return BCM_E_NOT_FOUND;
            }
            last_vlan_ptr = vlan_ptr;
            vlan_ptr = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                          &vlan_entry, NEXTPTRf);
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_draco15_repl_list_free
 * Description:
 *	Release the IPMC_VLAN entries in the HW list starting at start_ptr.
 *      Leave the first entry (at start_ptr) if leave_one is set.
 */

STATIC int
_bcm_draco15_repl_list_free(int unit, int start_ptr, int leave_one)
{
    mmu_memories1_ipmc_vlan_tbl_entry_t vlan_entry;
    int	                vlan_ptr, last_vlan_ptr;

    last_vlan_ptr = -1;
    vlan_ptr = start_ptr;

    while (vlan_ptr != last_vlan_ptr) {
        SOC_IF_ERROR_RETURN
            (READ_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit, MEM_BLOCK_ANY,
                                               vlan_ptr, &vlan_entry));
        last_vlan_ptr = vlan_ptr;
        vlan_ptr = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                          &vlan_entry, NEXTPTRf);
        if (leave_one) {
            /* Do not release the first entry of the list until it is safe */
            leave_one = FALSE;
            /* Terminate the list at the first entry */
            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit,
                        &vlan_entry, NEXTPTRf, last_vlan_ptr);
            SOC_IF_ERROR_RETURN
                (WRITE_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit,
                       MEM_BLOCK_ALL, last_vlan_ptr, &vlan_entry));
        } else {
            IPMC_REPL_VE_USED_CLR(unit, last_vlan_ptr);
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_draco15_repl_list_write
 * Description:
 *	Write the VLAN list contained in vlan_vec into the HW table.
 *      Return the start_index and total VLAN count.
 */

STATIC int
_bcm_draco15_repl_list_write(int unit, int *start_index, int *count,
                               bcm_vlan_vector_t vlan_vec)
{
    uint32		ms_bit;
    uint32		ls_bits[2];
    mmu_memories1_ipmc_vlan_tbl_entry_t vlan_entry;
    int	                vlan_ptr, last_vlan_ptr, vlan_count = 0;

    last_vlan_ptr = -1;

    for (ms_bit = 0; ms_bit <= 0x3f; ms_bit++) {
        ls_bits[0] = vlan_vec[2 * ms_bit + 0];
        ls_bits[1] = vlan_vec[2 * ms_bit + 1];
        if ((ls_bits[0] != 0) || (ls_bits[1] != 0)) {
            vlan_ptr = _bcm_draco15_ipmc_repl_next_free_ptr(unit);
            if (last_vlan_ptr > 0) {
                /* Write previous non-zero entry */
                soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit,
                        &vlan_entry, NEXTPTRf,
                        (vlan_ptr > 0) ? vlan_ptr : last_vlan_ptr);
                /* If vlan_ptr <= 0, terminate so
                 * later cleanup can remove the chain. */
                SOC_IF_ERROR_RETURN
                    (WRITE_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit,
                        MEM_BLOCK_ALL, last_vlan_ptr, &vlan_entry));
                if (vlan_ptr < 0) {
                    _bcm_draco15_repl_list_free(unit, *start_index, FALSE);
                    return BCM_E_RESOURCE;
                }
            } else {
                if (vlan_ptr < 0) {
                    return BCM_E_RESOURCE;
                }
                *start_index = vlan_ptr;
            }
            sal_memset(&vlan_entry, 0, sizeof(vlan_entry));
            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit, &vlan_entry,
                                                         MSB_VLANf, ms_bit);
            soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_set(unit, &vlan_entry,
                                             LSB_VLAN_BMf, ls_bits);
            IPMC_REPL_VE_USED_SET(unit, vlan_ptr);
            vlan_count +=
                _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
            last_vlan_ptr = vlan_ptr;
        }
    }

    if (last_vlan_ptr > 0) {
        /* Write final entry */
        soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_set(unit,
                          &vlan_entry, NEXTPTRf, last_vlan_ptr);
        SOC_IF_ERROR_RETURN
            (WRITE_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit,
                   MEM_BLOCK_ALL, last_vlan_ptr, &vlan_entry));
    }

    *count = vlan_count;
    return BCM_E_NONE;
}


/*
 * Function:
 *	bcm_draco15_ipmc_repl_set
 * Purpose:
 *	Assign set of VLANs provided to port's replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to list.
 *	vlan_vec - (IN) vector of replicated VLANs common to selected ports.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_draco15_ipmc_repl_set(int unit, int ipmc_id, bcm_port_t port,
                          bcm_vlan_vector_t vlan_vec)
{
    int			rv = BCM_E_NONE;
    mmu_memories1_ipmc_group_tbl_entry_t group_entry;
    int			list_start_ptr = 0, prev_start_ptr;
    int                 alloc_size, repl_hash, vlan_count;
    int                 xqempty, leave_one= FALSE;
    int                 vid;
    pbmp_t              pbmp, ubmp;
    _bcm_repl_list_info_t *rli_start, *rli_current, *rli_prev;

    if (!SOC_IS_DRACO15(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (!SOC_IPMCREPLSHR_GET(unit)) {
        return BCM_E_CONFIG;
    }

    IPMC_INIT(unit);
    IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    for (vid = BCM_VLAN_MIN; vid < BCM_VLAN_MAX; vid++) {
        if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_vlan_port_get(unit, vid, &pbmp, &ubmp));
            if (!BCM_PBMP_MEMBER(pbmp, port)) {
                return BCM_E_PARAM;
            }
        }
    }

    IPMC_REPL_LOCK(unit);

    /* Check previous group pointer */
    if ((rv = READ_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit, MEM_BLOCK_ANY,
                                                 ipmc_id,
                                                 &group_entry)) < 0) {
        goto repl_set_done;
    }
          
    prev_start_ptr = soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_get(unit,
                        &group_entry, _ipmc_group_field_get(unit, port));

    /* Search for list already in table */
    rli_start = IPMC_REPL_LIST_INFO(unit);

    repl_hash = _BCM_VLAN_VEC_HASH(vlan_vec);

    for (rli_current = rli_start; rli_current != NULL;
         rli_current = rli_current->next) {
        if (repl_hash == rli_current->hash) {
            rv = _bcm_draco15_repl_list_compare(unit, rli_current->index,
                                                vlan_vec);
            if (rv == BCM_E_NOT_FOUND) {
                continue; /* Not a match */
            } else if (rv < 0) {
                goto repl_set_done; /* Access error */
            } else {
                break; /* Match */
            }
        }
    }

    if (rli_current != NULL) {
        /* Found a match, point to here and increase reference count */
        if (prev_start_ptr == rli_current->index) {
            /* We're already pointing to this list, so done */
            rv = BCM_E_NONE;
            goto repl_set_done;
        } else {
            list_start_ptr = rli_current->index;
            vlan_count = rli_current->list_size;
        }
    } else {
        /* Not a match, make a new chain */
        if ((rv = _bcm_draco15_repl_list_write(unit, &list_start_ptr,
                                         &vlan_count, vlan_vec)) < 0) {
            goto repl_set_done;
        }

        if (vlan_count > 0) {
            /* Update data structures */
            alloc_size = sizeof(_bcm_repl_list_info_t);
            rli_current = sal_alloc(alloc_size, "IPMC repl list info");
            if (rli_current == NULL) {
                /* Release list */
                _bcm_draco15_repl_list_free(unit, list_start_ptr, FALSE);
                rv = BCM_E_MEMORY;
                goto repl_set_done;
            }
            sal_memset(rli_current, 0, alloc_size);
            rli_current->index = list_start_ptr;
            rli_current->hash = repl_hash;
            rli_current->next = rli_start;
            rli_current->list_size = vlan_count;
            IPMC_REPL_LIST_INFO(unit) = rli_current;
            rli_start = rli_current;
        }
    }

    if (vlan_count > 0) {
        soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_set(unit,
                          &group_entry, _ipmc_group_field_get(unit, port),
                                                      list_start_ptr);
        if ((rv = WRITE_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit,
                            MEM_BLOCK_ALL, ipmc_id, &group_entry)) < 0) {
            if (rli_current->refcount == 0) {
                /* This was new */
                _bcm_draco15_repl_list_free(unit, list_start_ptr, FALSE);
                IPMC_REPL_LIST_INFO(unit) = rli_current->next;
                sal_free(rli_current);
            }
            goto repl_set_done;
        }

        (rli_current->refcount)++;
        if (!IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
            rv = _draco15_ipmc_repl_l3port_set(unit, ipmc_id, port, TRUE);
        }
        /* we don't need this rli_current anymore */
   
        if (IPMC_REPL_LAPSED_GET(unit, port, ipmc_id)) {
            IPMC_REPL_LAPSED_CLR(unit, port, ipmc_id);
            IPMC_REPL_VE_USED_CLR(unit, prev_start_ptr);
        }

    } else if (prev_start_ptr != 0) {
        /* Need to disable group in L3_IPMC table before setting port
         * pointer to 0 */
        if ((rv = _draco15_ipmc_repl_l3port_set(unit, ipmc_id,
                                                port, FALSE)) < 0) {
            goto repl_set_done;
        }
        if ((rv = _draco15_ipmc_repl_xqempty_check(unit, port,
                                                   &xqempty)) < 0) {
            goto repl_set_done;
        }
        if (!xqempty) {
            IPMC_REPL_LAPSED_SET(unit, port, ipmc_id);
            IPMC_REPL_LAPSED_PORT_SET(unit, port);
            leave_one = TRUE;
        } else {
            soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_set(unit,
                    &group_entry, _ipmc_group_field_get(unit, port), 0);
            if ((rv = WRITE_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit,
                                MEM_BLOCK_ALL, ipmc_id, &group_entry)) < 0) {
                goto repl_set_done;
            }
            IPMC_REPL_LAPSED_CLR(unit, port, ipmc_id);
        }
    }
    IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) = vlan_count;

    if (prev_start_ptr != 0) {
        rli_prev = NULL;
        for (rli_current = rli_start; rli_current != NULL;
             rli_current = rli_current->next) {
            if (prev_start_ptr == rli_current->index) {
                (rli_current->refcount)--;
                if (rli_current->refcount == 0) {
                    /* Free these linked list entries */
                    rv = _bcm_draco15_repl_list_free(unit, prev_start_ptr,
                                                     leave_one);
                    /* If we have an error, we'll fall out anyway */
                    if (rli_prev == NULL) {
                        IPMC_REPL_LIST_INFO(unit) = rli_current->next;
                    } else {
                        rli_prev->next = rli_current->next;
                    }
                    sal_free(rli_current);
                }
                break;
            }
            rli_prev = rli_current;
        }
    }

 repl_set_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_draco15_ipmc_egress_intf_set
 * Purpose:
 *	Assign set of interfaces provided to port's replication list
 *      for chosen IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	mc_index - The index number.
 *	port     - port to list.
 *      if_count - number of interfaces in list 
 *	if_array - (IN) array of bcm_if_t interfaces, size if_count.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_draco15_ipmc_egress_intf_set(int unit, int mc_index, bcm_port_t port,
                                 int if_count, bcm_if_t *if_array)
{
    int if_num;
    bcm_vlan_vector_t vlan_vec;
    bcm_l3_intf_t l3_intf;

    BCM_VLAN_VEC_ZERO(vlan_vec);
    for (if_num = 0; if_num < if_count; if_num++) {
        l3_intf.l3a_intf_id = if_array[if_num];
        BCM_IF_ERROR_RETURN(bcm_esw_l3_intf_get(unit, &l3_intf));
        BCM_VLAN_VEC_SET(vlan_vec, l3_intf.l3a_vid);        
    }

    return bcm_draco15_ipmc_repl_set(unit, mc_index, port, vlan_vec);
}

/*
 * Function:
 *	bcm_fb_ipmc_egress_intf_get
 * Purpose:
 *	Retreieve set of egress interfaces in port's replication list
 *      for chosen IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to list.
 *	if_max   - maximum number of interfaces in replication list.
 *      if_array - (OUT) List of interface numbers.
 *	if_count - (OUT) number of interfaces returned in replication list.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_draco15_ipmc_egress_intf_get(int unit, int ipmc_id, bcm_port_t port,
                            int if_max, bcm_if_t *if_array, int *if_count)
{
    int intf_num, vid;
    bcm_vlan_vector_t vlan_vec;
    bcm_l3_intf_t l3_intf;

    BCM_IF_ERROR_RETURN
        (bcm_draco15_ipmc_repl_get(unit, ipmc_id, port, vlan_vec));

    intf_num = 0;
    for (vid = BCM_VLAN_MIN; vid < BCM_VLAN_MAX; vid++) {
        if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
            l3_intf.l3a_vid = vid;
            if (bcm_esw_l3_intf_find_vlan(unit, &l3_intf) < 0) {
                
                return BCM_E_NOT_FOUND;
            }
            if_array[intf_num++] = l3_intf.l3a_intf_id;
        }
    }

    *if_count = intf_num;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_draco15_ipmc_repl_reload
 * Purpose:
 *      Re-Initialize IPMC replication software to state consistent with
 *      hardware
 * Parameters:
 *      unit     - SOC unit #
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_draco15_ipmc_repl_reload(int unit)
{
    mmu_memories1_ipmc_group_tbl_entry_t group_entry;
    mmu_memories1_ipmc_vlan_tbl_entry_t vlan_entry;
    uint32 cfg2;
    uint32 ms_bit, ls_bits[2];
    bcm_port_t port;
    int ipmc_id, alloc_size;
    int vlan_ptr, next_vlan_ptr, first_vlan_ptr;
    bcm_vlan_vector_t vlan_vec;
    _bcm_repl_list_info_t *rli_start, *rli_current;
    int rv;
    int list_mode_detected = FALSE;

    if (!SOC_IS_DRACO15(unit)) {
        return BCM_E_UNAVAIL;
    }

    bcm_draco15_ipmc_repl_detach(unit);

    /* Allocate struct for IPMC replication booking keeping */
    alloc_size = sizeof(_draco15_repl_info_t);
    _draco15_repl_info[unit] = sal_alloc(alloc_size, "IPMC repl info");
    if (_draco15_repl_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_draco15_repl_info[unit], 0, alloc_size);

    IPMC_REPL_TOTAL(unit) =
        soc_mem_index_count(unit, MMU_MEMORIES1_IPMC_VLAN_TBLm);
    alloc_size = SHR_BITALLOCSIZE(IPMC_REPL_TOTAL(unit));
    _draco15_repl_info[unit]->bitmap_entries_used =
        sal_alloc(alloc_size, "IPMC repl entries used");
    if (_draco15_repl_info[unit]->bitmap_entries_used == NULL) {
        bcm_draco15_ipmc_repl_detach(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(_draco15_repl_info[unit]->bitmap_entries_used, 0, alloc_size);

    /* Always reserve slot 0 (because 0 means NULL pointer) */
    IPMC_REPL_VE_USED_SET(unit, 0);

    PBMP_E_ITER(unit, port) {
        alloc_size = sizeof(_draco15_repl_port_info_t);
        IPMC_REPL_PORT_INFO(unit, port) =
            sal_alloc(alloc_size, "IPMC repl port info");
        if (IPMC_REPL_PORT_INFO(unit, port) == NULL) {
            bcm_draco15_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(IPMC_REPL_PORT_INFO(unit, port), 0, alloc_size);
    }

    /* IPMC per port enabled ? */
    PBMP_E_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &cfg2));
        IPMC_REPL_PORT_INFO(unit, port)->enabled =
             soc_reg_field_get(unit, EGR_IPMC_CFG2r, cfg2, IPMC_REPLICATIONf);
    }

    rli_start = IPMC_REPL_LIST_INFO(unit);

    /*
     * Read IPMC group and IPMC VLAN tables to build up software state
     */
    for (ipmc_id = soc_mem_index_min(unit, L3_IPMCm);
         ipmc_id <= soc_mem_index_max(unit, L3_IPMCm); ipmc_id++) {
        rv = READ_MMU_MEMORIES1_IPMC_GROUP_TBLm(unit, MEM_BLOCK_ANY,
                                                ipmc_id, &group_entry);

        BCM_IF_ERROR_RETURN(rv);
        PBMP_E_ITER(unit, port) {
            first_vlan_ptr =
                soc_MMU_MEMORIES1_IPMC_GROUP_TBLm_field32_get(unit,
                             &group_entry, _ipmc_group_field_get(unit, port));
            if (first_vlan_ptr == 0) {
                continue;
            }

            if (IPMC_REPL_VE_USED_GET(unit, first_vlan_ptr)) {
                /* We've already traversed this list, just note it */
                for (rli_current = rli_start; rli_current != NULL;
                     rli_current = rli_current->next) {
                    if (rli_current->index == first_vlan_ptr) {
                        (rli_current->refcount)++;
                        IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) = 
                            rli_current->list_size;
                        list_mode_detected = TRUE;
                        break;
                    }
                }
                if (rli_current == NULL) {
                    /* Table out of sync.  Not good. */
                    return BCM_E_INTERNAL;
                } else {
                    continue;
                }
            }

            BCM_VLAN_VEC_ZERO(vlan_vec);
            next_vlan_ptr = first_vlan_ptr;
            vlan_ptr = 0;
            while (vlan_ptr != next_vlan_ptr) {
                vlan_ptr = next_vlan_ptr;
                rv = READ_MMU_MEMORIES1_IPMC_VLAN_TBLm(unit, MEM_BLOCK_ANY,
                                                       vlan_ptr, &vlan_entry);
                BCM_IF_ERROR_RETURN(rv);
                soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field_get(unit, &vlan_entry,
                      LSB_VLAN_BMf, ls_bits);
                ms_bit = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                             &vlan_entry, MSB_VLANf);
                vlan_vec[2 * ms_bit + 0] = ls_bits[0];
                vlan_vec[2 * ms_bit + 1] = ls_bits[1];
                IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) +=
                      _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
                IPMC_REPL_VE_USED_SET(unit, vlan_ptr);
                next_vlan_ptr = soc_MMU_MEMORIES1_IPMC_VLAN_TBLm_field32_get(unit,
                                 &vlan_entry, NEXTPTRf);
            }

            /* Create new list element */
            alloc_size = sizeof(_bcm_repl_list_info_t);
            rli_current = sal_alloc(alloc_size, "IPMC repl list info");
            if (rli_current == NULL) {
                return BCM_E_MEMORY;
            }
            sal_memset(rli_current, 0, alloc_size);
            rli_current->index = first_vlan_ptr;
            rli_current->hash = _BCM_VLAN_VEC_HASH(vlan_vec);
            rli_current->next = rli_start;
            rli_current->list_size =
                IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id);
            IPMC_REPL_LIST_INFO(unit) = rli_current;
            rli_start = rli_current;
            (rli_current->refcount)++;
        }
    }

    if (list_mode_detected) {
        SOC_IPMCREPLSHR_SET(unit, 1);
    }

    return BCM_E_NONE;
}

#else	/* INCLUDE_L3 */

int _bcm_esw_draco_ipmc_not_empty;

#endif	/* INCLUDE_L3 */
