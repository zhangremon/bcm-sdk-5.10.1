/*
 * $Id: l2.c 1.72.2.1 Broadcom SDK $
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
 * File:        l2.c
 * Purpose:     Easyrider L2 function implementations
 */

#include <soc/defs.h>

#ifdef BCM_EASYRIDER_SUPPORT

#include <assert.h>

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/l2x.h>

#include <bcm/l2.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/easyrider.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/common/multicast.h>

#include <bcm_int/esw_dispatch.h>

typedef struct _bcm_mac_block_info_s {
    bcm_pbmp_t mb_pbmp;
    int ref_count;
} _bcm_mac_block_info_t;

static _bcm_mac_block_info_t *_mbi_entries[BCM_MAX_NUM_UNITS];
static int _mbi_num[BCM_MAX_NUM_UNITS];

/*
 * Function:
 *      _bcm_er_l2_from_l2x
 * Purpose:
 *      Convert a Easyrider L2ER entry to a hardware-independent L2 entry
 * Parameters:
 *      unit - Unit number
 *      l2addr - (OUT) Hardware-independent L2 entry
 *      l2er_entry - Easyrider L2ER entry
 */

int
_bcm_er_l2_from_l2x(int unit, bcm_l2_addr_t *l2addr,
                    l2_entry_internal_entry_t *l2er_entry, bcm_pbmp_t *pbm)
{
    int         mb_index;
    uint32      pword;

    sal_memset(l2addr, 0, sizeof (*l2addr));

    /* Valid bit is ignored here; entry is assumed valid */

    soc_L2_ENTRY_INTERNALm_mac_addr_get(unit, l2er_entry,
                                        MAC_ADDRf, l2addr->mac);

    l2addr->vid = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry,
                                                     VLAN_IDf);
    l2addr->cos_dst = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry,
                                                         PRIf);
    l2addr->cos_src = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry,
                                                         PRIf);

    if (soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, CPUf)) {
        l2addr->flags |= BCM_L2_COPY_TO_CPU;
    }

    if (soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, DST_DISCARDf)) {
        l2addr->flags |= BCM_L2_DISCARD_DST;
    }

    if (soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, SRC_DISCARDf)) {
        l2addr->flags |= BCM_L2_DISCARD_SRC;
    }

    if (soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, SCPf)) {
        l2addr->flags |= BCM_L2_COS_SRC_PRI;
    }

    if (soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, RPEf)) {
        l2addr->flags |= BCM_L2_SETPRI;
    }

    if (soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, MH_OPCODE_5f)) {
        l2addr->flags |= BCM_L2_REMOTE_LOOKUP;
    }

    if (BCM_MAC_IS_MCAST(l2addr->mac)) {
        l2addr->flags |= BCM_L2_MCAST;
        if (SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) {
            /* We are storing L2MC_PBM in L2_ENTRY instead of in L2MC table. */
            pword = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, 
                                                       L2MC_PBMf);
            SOC_PBMP_CLEAR(*pbm);
            SOC_PBMP_WORD_SET(*pbm, 0, pword);
        } else {
            l2addr->l2mc_index =
                soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, L2MC_PTRf);
        }
    } else {
        bcm_module_t    modid, mod_out;
        bcm_port_t      tgid_port, port_out;
        _bcm_gport_dest_t       dest;
        int                     isGport, rv;

        _bcm_gport_dest_t_init(&dest);
        tgid_port = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry,
                                                       PORT_TGIDf);
        modid = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry,
                                                   MODULE_IDf);
        /* Trunk group */
        if (tgid_port & BCM_TGID_TRUNK_INDICATOR(unit)) {
    	    l2addr->flags |= BCM_L2_TRUNK_MEMBER;
    	    l2addr->tgid = BCM_MODIDf_TGIDf_TO_TRUNK(unit, modid, tgid_port);
            bcm_esw_trunk_psc_get(unit, l2addr->tgid, &(l2addr->rtag));
            dest.tgid = l2addr->tgid;
            if (soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, REMOTE_TRUNKf)) {
                l2addr->flags |= BCM_L2_REMOTE_TRUNK;
            }
            dest.gport_type = _SHR_GPORT_TYPE_TRUNK;
        } else {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        modid, tgid_port, &mod_out, &port_out));
            l2addr->modid = mod_out;
            l2addr->port = port_out;
            dest.port = l2addr->port;
            dest.modid = l2addr->modid;
            dest.gport_type = _SHR_GPORT_TYPE_MODPORT ;
        }
        rv = bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport);

        if (BCM_SUCCESS(rv) && isGport) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_construct(unit, &dest, &(l2addr->port)));
        }
    }

    if (soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, L3f)) {
        l2addr->flags |= BCM_L2_L3LOOKUP;
    }

    if (SOC_CONTROL(unit)->l2x_group_enable) {
        l2addr->group = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry,
                                                           MAC_BLOCK_INDEXf);
    } else {
        mb_index = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry,
                                                      MAC_BLOCK_INDEXf);
        if (mb_index) {
            BCM_PBMP_ASSIGN(l2addr->block_bitmap,
                            _mbi_entries[unit][mb_index].mb_pbmp);
        }
        l2addr->group = 0;
    }

    if (soc_L2_ENTRY_INTERNALm_field32_get(unit, l2er_entry, STATIC_BITf)) {
        l2addr->flags |= BCM_L2_STATIC;
    }

    

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_er_l2_to_l2x
 * Purpose:
 *      Convert a hardware-independent L2 entry to a Easyrider L2ER entry
 * Parameters:
 *      unit - Unit number
 *      l2er_entry - (OUT) Easyrider L2ER entry
 *      l2addr - Hardware-independent L2 entry
 */

int
_bcm_er_l2_to_l2x(int unit, l2_entry_internal_entry_t *l2er_entry, 
                  bcm_l2_addr_t *l2addr, bcm_pbmp_t pbm)
{
    uint32      pword;
    
    VLAN_CHK_ID(unit, l2addr->vid);
    VLAN_CHK_PRIO(unit, l2addr->cos_dst);
    if (l2addr->flags & BCM_L2_PENDING) {
        return BCM_E_PARAM;
    }

    sal_memset(l2er_entry, 0, sizeof (*l2er_entry));

    soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, VALID_BITf, 1);
    soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, ACTIVEf, 1);
    soc_L2_ENTRY_INTERNALm_mac_addr_set(unit, l2er_entry, MAC_ADDRf,
                                        l2addr->mac);
    soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, VLAN_IDf, l2addr->vid);
    soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, PRIf, l2addr->cos_dst);

    if (l2addr->flags & BCM_L2_COPY_TO_CPU) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, CPUf, 1);
    }

    if (l2addr->flags & BCM_L2_DISCARD_DST) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, DST_DISCARDf, 1);
    }

    if (l2addr->flags & BCM_L2_DISCARD_SRC) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, SRC_DISCARDf, 1);
    }

    if (l2addr->flags & BCM_L2_COS_SRC_PRI) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, SCPf, 1);
    }

    if (l2addr->flags & BCM_L2_REMOTE_TRUNK) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, REMOTE_TRUNKf, 1);
    }

    if (l2addr->flags & BCM_L2_SETPRI) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, RPEf, 1);
    }

    if (l2addr->flags & BCM_L2_REMOTE_LOOKUP) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, MH_OPCODE_5f, 1);
    }

    if (l2addr->flags & BCM_L2_MCAST) { /* if (BCM_MAC_IS_MCAST(l2addr->mac)) { */
        if (SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) {
            /* We are storing L2MC_PBM in L2_ENTRY instead of in L2MC table. */

            pword = SOC_PBMP_WORD_GET(pbm, 0);
            soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, 
                                               L2MC_PBMf, pword);
        } else {
            if (_BCM_MULTICAST_IS_SET(l2addr->l2mc_index)) {
                if (_BCM_MULTICAST_IS_L2(l2addr->l2mc_index)) {
                    soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry,
                       L2MC_PTRf, _BCM_MULTICAST_ID_GET(l2addr->l2mc_index));
                } else {
                    /* No other multicast types in L2 on this device */
                    return BCM_E_PARAM;
                }
            } else {
                soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry,
                             L2MC_PTRf, l2addr->l2mc_index);
            }
        }
    } else {
        bcm_port_t      port = -1;
        bcm_trunk_t     tgid = BCM_TRUNK_INVALID;
        bcm_module_t    modid = -1;

        if (BCM_GPORT_IS_SET(l2addr->port)) {
            _bcm_l2_gport_params_t  g_params;

            BCM_IF_ERROR_RETURN(
                _bcm_esw_l2_gport_parse(unit, l2addr, &g_params));

            switch (g_params.type) {
                case _SHR_GPORT_TYPE_TRUNK: 
                    tgid = g_params.param0;
                    break;
                case  _SHR_GPORT_TYPE_MODPORT:
                    port = g_params.param0;
                    modid = g_params.param1;
                    break;
                case _SHR_GPORT_TYPE_LOCAL_CPU:
                    port = g_params.param0;
                    BCM_IF_ERROR_RETURN(
                        bcm_esw_stk_my_modid_get(unit, &modid));
                    break;
                default:
                    return BCM_E_PORT;
            }
        } else if (l2addr->flags & BCM_L2_TRUNK_MEMBER) {
            tgid = l2addr->tgid;
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                       l2addr->modid, l2addr->port, 
                                       &modid, &port));
            /* Check parameters */
            if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
                return BCM_E_BADID;
            }
            if (!SOC_PORT_ADDRESSABLE(unit, port)) {
                return BCM_E_PORT;
            }
        }

        /* Setting l2x_entry fields according to parameters */
        if ( BCM_TRUNK_INVALID != tgid) {
            TRUNK_CHK_TGID128(unit, tgid);
            soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, MODULE_IDf,
                                               BCM_TRUNK_TO_MODIDf(unit, tgid));
            soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, PORT_TGIDf,
                                               BCM_TRUNK_TO_TGIDf(unit, tgid));
        } else if (-1 != port) {
            soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, MODULE_IDf,
                                               modid);
            soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, PORT_TGIDf,
                                               port);
        }
    }

    if (l2addr->flags & BCM_L2_L3LOOKUP) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, L3f, 1);
    }

    if (SOC_CONTROL(unit)->l2x_group_enable) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, MAC_BLOCK_INDEXf,
                                           l2addr->group);
    } /* else MAC_BLOCK_INDEXf is handled in the add/remove functions below */

    if (l2addr->flags & BCM_L2_STATIC) {
        soc_L2_ENTRY_INTERNALm_field32_set(unit, l2er_entry, STATIC_BITf, 1);
    }

    

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_mac_block_insert
 * Purpose:
 *      Find or create a MAC_BLOCK table entry matching the given bitmap.
 * Parameters:
 *      unit - Unit number
 *      mb_pbmp - egress port bitmap for source MAC blocking
 *      mb_index - (OUT) Index of MAC_BLOCK table with bitmap.
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_RESOURCE          No more MAC_BLOCK entries available
 *      BCM_E_PARAM             Bad bitmap supplied
 */
static int
_bcm_mac_block_insert(int unit, bcm_pbmp_t mb_pbmp, int *mb_index)
{
    int cur_index = 0;
    _bcm_mac_block_info_t *mbi = _mbi_entries[unit];
    uint32      mbe;
    bcm_pbmp_t temp_pbmp;

    /* Check for reasonable pbmp */
    BCM_PBMP_ASSIGN(temp_pbmp, mb_pbmp);
    BCM_PBMP_AND(temp_pbmp, PBMP_ALL(unit));
    if (BCM_PBMP_NEQ(mb_pbmp, temp_pbmp)) {
        return BCM_E_PARAM;
    }

    for (cur_index = 0; cur_index < _mbi_num[unit]; cur_index++) {
        if (BCM_PBMP_EQ(mbi[cur_index].mb_pbmp, mb_pbmp)) {
            mbi[cur_index].ref_count++;
            *mb_index = cur_index;
            return BCM_E_NONE;
        }
    }

    /* Not in table already, see if any space free */
    for (cur_index = 1; cur_index < _mbi_num[unit]; cur_index++) {
        if (mbi[cur_index].ref_count == 0) {
            /* Attempt insert */
            mbe = 0;
            soc_reg_field_set(unit, MAC_BLOCK_TABLEr, &mbe, MAC_BLOCK_MASKf,
                              SOC_PBMP_WORD_GET(mb_pbmp, 0));
            SOC_IF_ERROR_RETURN(WRITE_MAC_BLOCK_TABLEr(unit, cur_index, mbe));
            mbi[cur_index].ref_count++;
            BCM_PBMP_ASSIGN(mbi[cur_index].mb_pbmp, mb_pbmp);
            *mb_index = cur_index;
            return BCM_E_NONE;
        }
    }

    /* Didn't find a free slot, out of table space */
    return BCM_E_RESOURCE;
}

/*
 * Function:
 *      _bcm_mac_block_delete
 * Purpose:
 *      Remove reference to MAC_BLOCK table entry matching the given bitmap.
 * Parameters:
 *      unit - Unit number
 *      mb_index - Index of MAC_BLOCK table with bitmap.
 */
static void
_bcm_mac_block_delete(int unit, int mb_index)
{
    if (_mbi_entries[unit][mb_index].ref_count > 0) {
        _mbi_entries[unit][mb_index].ref_count--;
    } else if (mb_index) {
        
        /* Someone reran init without flushing the L2 table */
    } /* else mb_index = 0, as expected for learning */
}

/*
 * Function:
 *      _bcm_er_l2_addr_add
 * Description:
 *      Add a MAC address to the Switch Address Resolution Logic (ARL)
 *      port with the given VLAN ID and parameters.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 *      pbm - Port bitmap, used if storing L2MC_PBM in L2 entry
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_RESOURCE          No MAC_BLOCK entries available
 * Notes:
 *      Use CMIC_PORT(unit) to associate the entry with the CPU.
 *      Use flag of BCM_L2_COPY_TO_CPU to send a copy to the CPU.
 *      Use flag of BCM_L2_TRUNK_MEMBER to set trunking (TGID must be
 *      passed as well with non-zero trunk group ID)
 *      MAC_BLOCK_INDEX field is used to implement FP filtering on
 *      a group of L2 entries if spn_L2_GROUP_ENABLE is set.
 */

int
_bcm_er_l2_addr_add(int unit, bcm_l2_addr_t *l2addr, bcm_pbmp_t pbm)
{
    l2_entry_internal_entry_t   l2er_entry, l2ent;
    int                         rv;
    l2_entry_internal_entry_t   l2er_lookup;
    int                         l2_index, mb_index = 0;
    soc_mem_t                   l2_mem, er_mem;
    bcm_mac_t                   mac;
    uint8                       key[XGS_HASH_KEY_SIZE];
    uint32                      hash_control, timestamp, ts;
    int                         hash_select, er_bucket_size;
    int                         cf_hit, cf_unhit, bucket, slot;
    int                         er_int_bucket, er_ext_bucket;

    BCM_IF_ERROR_RETURN(
        _bcm_er_l2_to_l2x(unit, &l2er_entry, l2addr, pbm));

    rv = soc_er_l2x_lookup(unit, &l2er_entry, &l2er_lookup,
                           &l2_index, &l2_mem);
    if (rv < 0) {
         if (rv != BCM_E_NOT_FOUND) {
             return rv;
         }
    }

    if (SOC_CONTROL(unit)->l2x_group_enable) {
        /* Using MAC_BLOCK_INDEX field for storing the L2 group */
        soc_L2_ENTRY_INTERNALm_field32_set(unit, &l2er_entry,
                                           MAC_BLOCK_INDEXf, l2addr->group);
    } else {
        if (rv == BCM_E_NONE) {
            /* Only here if L2 entry was found */
            mb_index = soc_L2_ENTRY_INTERNALm_field32_get(unit, &l2er_lookup,
                                                          MAC_BLOCK_INDEXf);
            _bcm_mac_block_delete(unit, mb_index);
        }
        /* Mac blocking, attempt to associate with bitmap entry */
        BCM_IF_ERROR_RETURN(
            _bcm_mac_block_insert(unit, l2addr->block_bitmap, &mb_index));
    
        soc_L2_ENTRY_INTERNALm_field32_set(unit, &l2er_entry,
                                           MAC_BLOCK_INDEXf, mb_index);
    }

    rv = soc_mem_insert(unit, L2_ENTRY_INTERNALm, COPYNO_ALL, &l2er_entry);
    if ((rv == BCM_E_FULL) && (l2addr->flags & BCM_L2_REPLACE_DYNAMIC)) {

        rv = READ_HASH_CONTROLr(unit, &hash_control);
        if (rv < 0 ) {
            goto done;
        }
        hash_select = soc_reg_field_get(unit, HASH_CONTROLr, hash_control,
                                        L2_AND_VLAN_MAC_HASH_SELECTf);

        soc_er_l2x_base_entry_to_key(unit, (uint32 *) &l2er_entry, key);
        er_int_bucket = soc_er_l2_hash(unit, hash_select, key);
        er_ext_bucket = soc_er_l2ext_hash(unit, hash_select, key);
                       
        rv = soc_l2x_freeze(unit);
        if (rv < 0) {
            goto done;
        }
        cf_hit = cf_unhit = -1;
        timestamp = 3;        /* 3 is most used entry, 0 is less used */  
        er_mem = L2_ENTRY_INTERNALm;    /* first check internal memory */
        er_bucket_size = SOC_L2X_BUCKET_SIZE;
        bucket = er_int_bucket;
        while (er_mem != INVALIDm) {
            for (slot = 0; slot < er_bucket_size; slot++) {

                rv = soc_mem_read(unit, er_mem, MEM_BLOCK_ANY, 
                                  bucket * er_bucket_size + slot, 
                                  &l2ent);
                if ((rv < 0)) {
                    (void) soc_l2x_thaw(unit);
                    goto done;
                }

                if (!soc_mem_field32_get(unit, er_mem, &l2ent, VALIDf) ) {
                    /* Found invalid entry - stop the search victim found */
                    cf_unhit = slot; 
                    break;
                } else {

                    soc_mem_mac_addr_get(unit, er_mem, &l2ent, MAC_ADDRf, mac);
                    /* Skip static entries */
                    if ((soc_mem_field32_get(unit, er_mem, &l2ent, 
                                             STATIC_BITf)) ||
                        (BCM_MAC_IS_MCAST(mac)) ||
                        (soc_mem_field32_get(unit, er_mem, &l2ent, L3f))) {
                        continue;
                    }
                    ts = soc_mem_field32_get(unit, er_mem, &l2ent, TIMESTAMPf);
                    if (!ts){
                        /* Found unhit entry - stop search victim found */
                        cf_unhit = slot;
                        break;
                    } else if (timestamp >= ts) {
                        /* If we need to take used entry we'll take less used */
                        cf_hit = slot;  
                        timestamp = ts; 
                    } 
                }
            }

            if (cf_unhit >= 0) {
                slot = cf_unhit;   /* take last unhit dynamic */
                goto done;
            } else if (cf_hit >= 0) {
                slot = cf_hit;     /* or last hit dynamic */
            } else {
                  /* If no entries were found in Internal memory */
                  /* then check the extarnal if exists */
                  if (L2_ENTRY_EXTERNALm == er_mem) {   
                      rv = BCM_E_FULL;     /* no dynamics to delete */
                      (void) soc_l2x_thaw(unit);
                      goto done;
                  }             
             }

            if (L2_ENTRY_EXTERNALm != er_mem && 
                SOC_CONTROL(unit)->l2x_external) {
                er_mem = L2_ENTRY_EXTERNALm;
                er_bucket_size = SOC_L2EXT_BUCKET_SIZE;
                bucket = er_ext_bucket;
            } else {
                er_mem = INVALIDm;
            }
        }
        soc_mem_write(unit, L2_ENTRY_INTERNALm, MEM_BLOCK_ALL,
                      bucket * SOC_L2X_BUCKET_SIZE + slot, &l2er_entry);
        
        rv = soc_l2x_thaw(unit);

    }

done:
    if (rv < 0) {
        _bcm_mac_block_delete(unit, mb_index);
    }

    return rv;
}

/*
 * Function:
 *      bcm_er_l2_addr_add
 * Description:
 *      Add a MAC address to the Switch Address Resolution Logic (ARL)
 *      port with the given VLAN ID and parameters.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_RESOURCE          No MAC_BLOCK entries available
 * Notes:
 *      Use CMIC_PORT(unit) to associate the entry with the CPU.
 *      Use flag of BCM_L2_COPY_TO_CPU to send a copy to the CPU.
 *      Use flag of BCM_L2_TRUNK_MEMBER to set trunking (TGID must be
 *      passed as well with non-zero trunk group ID)
 */

int
bcm_er_l2_addr_add(int unit, bcm_l2_addr_t *l2addr)
{
    bcm_pbmp_t     pbm;

    SOC_PBMP_CLEAR(pbm);
    return _bcm_er_l2_addr_add(unit, l2addr, pbm);
}

/*
 * Function:
 *      bcm_er_l2_addr_delete
 * Description:
 *      Delete an L2 address (MAC+VLAN) from the device
 * Parameters:
 *      unit - device unit
 *      mac  - MAC address to delete
 *      vid  - VLAN id
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    bcm_l2_addr_t  l2addr;
    l2_entry_internal_entry_t    l2er_entry, l2er_lookup;
    int            l2_index, mb_index, rv;
    soc_mem_t      l2_mem;
    bcm_pbmp_t     pbm;
    soc_control_t   *soc = SOC_CONTROL(unit);

    bcm_l2_addr_t_init(&l2addr, mac, vid);

    SOC_PBMP_CLEAR(pbm);
    BCM_IF_ERROR_RETURN(
        _bcm_er_l2_to_l2x(unit, &l2er_entry, &l2addr, pbm));

    soc_mem_lock(unit, L2_ENTRY_INTERNALm);
    soc_mem_lock(unit, L2_ENTRY_OVERFLOWm);
    soc_mem_lock(unit, L2_ENTRY_EXTERNALm);
    rv = soc_er_l2x_lookup(unit, &l2er_entry, &l2er_lookup,
                           &l2_index, &l2_mem);
    if (BCM_E_NONE != rv) {
        soc_mem_unlock(unit, L2_ENTRY_EXTERNALm);
        soc_mem_unlock(unit, L2_ENTRY_OVERFLOWm);
        soc_mem_unlock(unit, L2_ENTRY_INTERNALm);
        return rv;
    }

    if (!SOC_CONTROL(unit)->l2x_group_enable) {
        mb_index = soc_L2_ENTRY_INTERNALm_field32_get(unit, &l2er_lookup,
                                                      MAC_BLOCK_INDEXf);
        _bcm_mac_block_delete(unit, mb_index);
    }

    if (SOC_L2_DEL_SYNC_LOCK(soc) < 0) {
        soc_mem_unlock(unit, L2_ENTRY_EXTERNALm);
        soc_mem_unlock(unit, L2_ENTRY_OVERFLOWm);
        soc_mem_unlock(unit, L2_ENTRY_INTERNALm);
        return BCM_E_RESOURCE;
    }
    if ((rv = soc_mem_delete(unit, L2_ENTRY_INTERNALm, COPYNO_ALL, 
                                                    &l2er_entry)) >= 0) {
        rv = soc_l2x_sync_delete(unit, (uint32 *)&l2er_lookup, l2_index, 0);
    }
    SOC_L2_DEL_SYNC_UNLOCK(soc);
    soc_mem_unlock(unit, L2_ENTRY_EXTERNALm);
    soc_mem_unlock(unit, L2_ENTRY_OVERFLOWm);
    soc_mem_unlock(unit, L2_ENTRY_INTERNALm);

    return(rv);
}

typedef struct {
    int             dyn_only;
    int             by_vlan;
    int             by_port;
    int             by_trunk;
    bcm_vlan_t      vlan;
    bcm_module_t    mod;
    bcm_module_t    new_mod;
    bcm_port_t      port;
    bcm_port_t      new_port;
    bcm_trunk_t     tid;
    bcm_trunk_t     new_tid;
} _bcm_er_l2_dvp_info_t;


STATIC int 
_bcm_er_l2_replace_mod_port(int unit, soc_mem_t tmem, 
                            l2_entry_internal_entry_t *l2ep, 
                            bcm_module_t mod_val, 
                            bcm_port_t port_val, int index)
{
    soc_mem_field32_set(unit, tmem, l2ep, MODULE_IDf, mod_val);
    soc_mem_field32_set(unit, tmem, l2ep, PORT_TGIDf, port_val);
    return soc_mem_write(unit, tmem, MEM_BLOCK_ALL, index, l2ep); 
}


STATIC int
_bcm_er_l2_addr_replace_by_vlan_dest_cb(int unit, uint32 *entry, int index,
                                        soc_mem_t tmem, void *data)
{

    l2_entry_internal_entry_t   *l2ep = (l2_entry_internal_entry_t *)entry;
    _bcm_er_l2_dvp_info_t       *dvp_info = (_bcm_er_l2_dvp_info_t *)data;
    bcm_port_t                  ent_port = 0;
    bcm_trunk_t                 ent_tid = 0;
    bcm_vlan_t                  ent_vlan = 0;
    bcm_module_t                ent_mod = 0;
    uint32                      mod_val = 0, port_val = 0;


    if (dvp_info->dyn_only &&
        soc_L2_ENTRY_INTERNALm_field32_get(unit, l2ep,
                                           STATIC_BITf)) {
        return BCM_E_NONE;
    }
    ent_mod = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2ep, MODULE_IDf);
    ent_vlan = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2ep, VLAN_IDf);
    ent_port = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2ep, PORT_TGIDf);
    if (ent_port & BCM_TGID_TRUNK_INDICATOR(unit)) {
        ent_tid = BCM_MODIDf_TGIDf_TO_TRUNK(unit, ent_mod, ent_port); 
    }

    if (dvp_info->new_tid < 0) {
        mod_val = dvp_info->new_mod;
        port_val = dvp_info->new_port;
    } else {
        mod_val = BCM_TRUNK_TO_MODIDf(unit, dvp_info->new_tid);
        port_val = BCM_TRUNK_TO_TGIDf(unit, dvp_info->new_tid);
    }

    if ((dvp_info->by_trunk) && (dvp_info->by_vlan)){
        if ((dvp_info->vlan == ent_vlan) && 
            (dvp_info->tid == ent_tid)) {
            return _bcm_er_l2_replace_mod_port(unit, tmem, l2ep, 
                                               mod_val, port_val, index);
        }
    } else if ((dvp_info->by_port) && (dvp_info->by_vlan)) {
        if ((dvp_info->port == ent_port) && 
            (dvp_info->mod == ent_mod) && 
            (dvp_info->vlan == ent_vlan)) {
            return _bcm_er_l2_replace_mod_port(unit, tmem, l2ep, 
                                               mod_val, port_val, index);
        }

    } else if (dvp_info->by_port) {
        if ((dvp_info->port == ent_port) && 
            (dvp_info->mod == ent_mod)) {
            return _bcm_er_l2_replace_mod_port(unit, tmem, l2ep, 
                                               mod_val, port_val, index);
        }
    } else if (dvp_info->by_trunk) {
        if (dvp_info->tid == ent_tid) {
            return _bcm_er_l2_replace_mod_port(unit, tmem, l2ep, 
                                               mod_val, port_val, index);
        }
    } else if (dvp_info->by_vlan) {
        if (dvp_info->vlan == ent_vlan) {
            return _bcm_er_l2_replace_mod_port(unit, tmem, l2ep, 
                                               mod_val, port_val, index);
        }
    }

    return BCM_E_NONE;
}


STATIC int
_bcm_er_l2_addr_delete_by_vlan_port_cb(int unit, uint32 *entry, int index,
                                       soc_mem_t tmem, void *data)
{
    bcm_vlan_t  vlan;
    bcm_port_t  port;
    int  tid;
    l2_entry_internal_entry_t *l2ep = (l2_entry_internal_entry_t *)entry;
    _bcm_er_l2_dvp_info_t *dvp_info = (_bcm_er_l2_dvp_info_t *)data;
    bcm_module_t temp_mod;

    COMPILER_REFERENCE(index);

    if (dvp_info->dyn_only &&
        soc_L2_ENTRY_INTERNALm_field32_get(unit, l2ep,
                                           STATIC_BITf)) {
        return BCM_E_NONE;
    }

    if (dvp_info->by_trunk) {
        port = soc_L2_ENTRY_INTERNALm_field32_get(unit,
                                                  l2ep, PORT_TGIDf);
        temp_mod = soc_L2_ENTRY_INTERNALm_field32_get(unit,
                                                  l2ep, MODULE_IDf);
        if (!(port & BCM_TGID_TRUNK_INDICATOR(unit))) {
            return BCM_E_NONE;
        }
        tid = BCM_MODIDf_TGIDf_TO_TRUNK(unit, temp_mod, port);
        if (dvp_info->tid != tid) {
            return BCM_E_NONE;
        }
    } else if (dvp_info->by_port) {
        temp_mod = soc_L2_ENTRY_INTERNALm_field32_get(unit, l2ep, MODULE_IDf);
        if (dvp_info->mod != temp_mod) {
            return BCM_E_NONE;
        }
        port = soc_L2_ENTRY_INTERNALm_field32_get(unit,
                                                  l2ep, PORT_TGIDf);
        if (port & BCM_TGID_TRUNK_INDICATOR(unit)) {
            return BCM_E_NONE;
        }
        if (dvp_info->port != port) {
            return BCM_E_NONE;
        }
    }

    if (dvp_info->by_vlan) {
        vlan = soc_L2_ENTRY_INTERNALm_field32_get(unit,
                                                  l2ep, VLAN_IDf);
        if (dvp_info->vlan != vlan) {
            return BCM_E_NONE;
        }
    }

    return soc_mem_delete(unit, tmem, MEM_BLOCK_ALL, l2ep);
}


/*
 * Function:
 *     _bcm_er_get_op_from_flags
 * Description:
 *     Helper function to _bcm_er_l2_addr_replace_by_vlan_dest to 
 *     decide on a PPA command according to flags
 * Parameters:
 *     flags        flags BCM_L2_REPLACE_* 
 *     op           PPA command to be programmed
 * Return:
 *     BCM_E_NONE   - Success
 *     BCM_E_PARAM  - Failure
 */
STATIC int 
_bcm_er_get_op_from_flags(uint32 flags, uint32 *op)
{
    uint32 cmp_flags = 0, int_op = 0;
    int    rv = BCM_E_NONE;

    if (NULL == op) {
        return BCM_E_PARAM;
    }
    cmp_flags = flags & (BCM_L2_REPLACE_MATCH_VLAN |
                         BCM_L2_REPLACE_MATCH_DEST);
    switch (cmp_flags) {
        case BCM_L2_REPLACE_MATCH_VLAN: 
        {
            int_op = (flags & BCM_L2_REPLACE_DELETE ? SOC_ER_L2_VLAN_DEL : 
                   SOC_ER_L2_VLAN_REPL);
         break;
        }
        case BCM_L2_REPLACE_MATCH_DEST :
        {
            int_op = (flags & BCM_L2_REPLACE_DELETE ? SOC_ER_L2_PORTMOD_DEL :
                   SOC_ER_L2_PORTMOD_REPL);
            break;
        }
        case BCM_L2_REPLACE_MATCH_VLAN | BCM_L2_REPLACE_MATCH_DEST:
        {
            int_op = (flags & BCM_L2_REPLACE_DELETE ? SOC_ER_L2_PORTMOD_VLAN_DEL :
                   SOC_ER_L2_PORTMOD_VLAN_REPL);
            break;
        }
        default:
            rv = BCM_E_PARAM;
            break;
    }

    *op = int_op;
    return rv;
}


/*
 * Function:
 *     _bcm_er_l2_addr_replace_by_vlan_dest
 * Description:
 *     Helper function to _bcm_l2_addr_replace_by_vlan API to replace l2 entries 
 *      by vlan for Firebolt family 
 * Parameters:
 *     unit         device number
 *     flags        flags BCM_L2_REPLACE_* 
 *     rep_st       structure with information of what to replace
 * Return:
 *     BCM_E_XXX
 */
int 
_bcm_er_l2_addr_replace_by_vlan_dest(int unit, uint32 flags, _bcm_l2_replace_t *rep_st)
{
    _bcm_er_l2_dvp_info_t   dvp_info;
    uint32                  op, soc_flags;
    bcm_module_t            mod = 0;
    bcm_vlan_t              vlan = 0;
    bcm_port_t              port_tgid = 0, new_tgid = 0;
    soc_control_t	        *soc = SOC_CONTROL(unit);
    int                     rv = BCM_E_NONE;

    if (NULL == rep_st) {
        return BCM_E_PARAM;
    }

    sal_memset(&dvp_info, 0 , sizeof(_bcm_er_l2_dvp_info_t));
    if (flags & BCM_L2_REPLACE_MATCH_VLAN) {
        VLAN_CHK_ID(unit, rep_st->key_vlan);
        dvp_info.by_vlan = TRUE;
        dvp_info.by_port = FALSE;
        dvp_info.by_trunk = FALSE;
        dvp_info.vlan = rep_st->key_vlan;
        vlan = rep_st->key_vlan;
    }
    if (flags & BCM_L2_REPLACE_MATCH_DEST) {
        dvp_info.by_vlan = FALSE;
        if (rep_st->match_dest.trunk != -1) {
            dvp_info.by_port = FALSE;
            dvp_info.by_trunk = TRUE;
            dvp_info.tid = rep_st->match_dest.trunk;
            port_tgid = BCM_TRUNK_TO_TGIDf(unit, rep_st->match_dest.trunk);
            mod = BCM_TRUNK_TO_MODIDf(unit, rep_st->match_dest.trunk);
            new_tgid = rep_st->new_dest.trunk;
        } else {
            dvp_info.by_port = TRUE;
            dvp_info.by_trunk = FALSE;
            dvp_info.port = rep_st->match_dest.port;
            dvp_info.mod = rep_st->match_dest.module;
            port_tgid = rep_st->match_dest.port;
            mod = rep_st->match_dest.module;
            new_tgid = rep_st->new_dest.port;
        }
    }
    /* In case of VLAN_DEST */
    if (flags & BCM_L2_REPLACE_MATCH_VLAN) {
        dvp_info.by_vlan = TRUE;
    }

    dvp_info.new_mod = rep_st->new_dest.module;
    if (rep_st->new_dest.trunk != -1) {
        dvp_info.new_port = -1;
        dvp_info.new_tid = rep_st->new_dest.trunk;
    } else {
        dvp_info.new_port = rep_st->new_dest.port;
        dvp_info.new_tid = -1;
    }

    dvp_info.dyn_only = !(flags & BCM_L2_REPLACE_MATCH_STATIC);
    BCM_IF_ERROR_RETURN(
        _bcm_er_get_op_from_flags(flags, &op));

    if (SOC_L2_DEL_SYNC_LOCK(soc) < 0) {
        return BCM_E_RESOURCE;
    }

    (void) soc_er_l2x_delete_by(unit, mod, port_tgid, vlan,
                                rep_st->new_dest.module, 
                                new_tgid, op, 
                                ((flags & BCM_L2_REPLACE_MATCH_STATIC) != 0));
    /* Clean up any entries missed by HW PPA command using DMA */

    /* For delete operation */
    if (flags & BCM_L2_REPLACE_DELETE) {
        rv = soc_er_l2_traverse(unit, _bcm_er_l2_addr_delete_by_vlan_port_cb, 
                                &dvp_info);
        if (BCM_SUCCESS(rv)) {
            soc_flags = (flags & BCM_L2_REPLACE_MATCH_STATIC) ? 
                        SOC_L2X_INC_STATIC : 0;
            if (rep_st->match_dest.trunk != -1) {
                rv = soc_l2x_sync_delete_by_trunk(unit, port_tgid, soc_flags);
            } else {
                rv = soc_l2x_sync_delete_by_port(unit, mod, port_tgid, soc_flags);
            }
        }
    } else {  /* For replace operation */
        rv = soc_er_l2_traverse(unit, _bcm_er_l2_addr_replace_by_vlan_dest_cb, 
                                &dvp_info);
    }
    SOC_L2_DEL_SYNC_UNLOCK(soc);

    return(rv);
}

typedef struct {
    int dyn_only;
    int mac_only;
    bcm_mac_t mac;
    bcm_module_t mod;
    bcm_port_t port;
} _bcm_er_l2_dmp_info_t;


/*
 * Function:
 *      _bcm_er_l2_addr_get
 * Description:
 *      Given a MAC address and VLAN ID, check if the entry is present
 *      in the L2 table, and if so, return all associated information.
 * Parameters:
 *      unit - Device unit number
 *      mac - input MAC address to search
 *      vid - input VLAN ID to search
 *      l2addr - Pointer to bcm_l2_addr_t structure to receive results
 *      pbm - Port bitmap if L2MC_PBM are stored in L2 entry
 * Returns:
 *      BCM_E_NONE              Success (l2addr filled in)
 *      BCM_E_PARAM             Illegal parameter (NULL pointer)
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_NOT_FOUND Address not found (l2addr not filled in)
 */

int
_bcm_er_l2_addr_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                   bcm_l2_addr_t *l2addr, bcm_pbmp_t *pbm)
{
    l2_entry_internal_entry_t    l2er_entry, l2er_lookup;
    bcm_l2_addr_t  l2addr_entry;
    int            l2_index, rv;
    soc_mem_t      l2_mem;

    VLAN_CHK_ID(unit, vid);

    bcm_l2_addr_t_init(&l2addr_entry, mac, vid);

    SOC_PBMP_CLEAR(*pbm);
    BCM_IF_ERROR_RETURN(
        _bcm_er_l2_to_l2x(unit, &l2er_entry, &l2addr_entry, *pbm));

    soc_mem_lock(unit, L2_ENTRY_INTERNALm);
    soc_mem_lock(unit, L2_ENTRY_OVERFLOWm);
    soc_mem_lock(unit, L2_ENTRY_EXTERNALm);
    rv = soc_er_l2x_lookup(unit, &l2er_entry, &l2er_lookup,
                           &l2_index, &l2_mem);
    soc_mem_unlock(unit, L2_ENTRY_EXTERNALm);
    soc_mem_unlock(unit, L2_ENTRY_OVERFLOWm);
    soc_mem_unlock(unit, L2_ENTRY_INTERNALm);
 
    BCM_IF_ERROR_RETURN(rv);

    BCM_IF_ERROR_RETURN(_bcm_er_l2_from_l2x(unit, l2addr, &l2er_lookup, pbm));
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_er_l2_addr_get
 * Description:
 *      Given a MAC address and VLAN ID, check if the entry is present
 *      in the L2 table, and if so, return all associated information.
 * Parameters:
 *      unit - Device unit number
 *      mac - input MAC address to search
 *      vid - input VLAN ID to search
 *      l2addr - Pointer to bcm_l2_addr_t structure to receive results
 * Returns:
 *      BCM_E_NONE              Success (l2addr filled in)
 *      BCM_E_PARAM             Illegal parameter (NULL pointer)
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_NOT_FOUND Address not found (l2addr not filled in)
 */

int
bcm_er_l2_addr_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                   bcm_l2_addr_t *l2addr)
{
    bcm_pbmp_t pbm;

    return _bcm_er_l2_addr_get(unit, mac, vid, l2addr, &pbm);
}

#if  defined(BCM_WARM_BOOT_SUPPORT)

/*
 * Function:
 *      _bcm_er_l2_reload_mbi
 * Description:
 *      Load MAC block info from hardware into software data structures.
 * Parameters:
 *      unit - StrataSwitch unit number.
 */

int
_bcm_er_l2_reload_mbi(int unit)
{
    _bcm_mac_block_info_t *mbi = _mbi_entries[unit];
    l2_entry_internal_entry_t         l2er_entry;
    int                 index, mb_index;
    uint32		mbe;

    pbmp_t              mb_pbmp;
    int                 im;
    soc_mem_t		mem;

    /*
     * Refresh MAC Block information from the hardware tables.
     */

    for (mb_index = 0; mb_index < _mbi_num[unit]; mb_index++) {
	SOC_PBMP_CLEAR(mb_pbmp);
	SOC_IF_ERROR_RETURN(READ_MAC_BLOCK_TABLEr(unit, mb_index, &mbe));
        SOC_PBMP_WORD_SET(mb_pbmp, 0,
               soc_reg_field_get(unit, MAC_BLOCK_TABLEr, mbe, 
	                 MAC_BLOCK_MASKf));
        BCM_PBMP_ASSIGN(mbi[mb_index].mb_pbmp, mb_pbmp);
    }
    
    for (im = 0; im < 3; im++) {
        switch (im) {
        case 0:
            mem = L2_ENTRY_EXTERNALm;
            break;
        case 1:
            mem = L2_ENTRY_INTERNALm;
            break;
        case 2:
            mem = L2_ENTRY_OVERFLOWm;
            break;
	default:
	    mem = INVALIDm;
	    assert(0);
        }
        if (!SOC_CONTROL(unit)->l2x_group_enable) {
            for (index = soc_mem_index_min(unit, mem);
                 index <= soc_mem_index_max(unit, mem); index++) {
                SOC_IF_ERROR_RETURN
                    (soc_mem_read(unit, mem, MEM_BLOCK_ANY, index, &l2er_entry));
	    	if (!soc_L2_ENTRY_INTERNALm_field32_get
					(unit, &l2er_entry, VALIDf)) {
  	            continue;
  	    	}		
                mb_index = soc_L2_ENTRY_INTERNALm_field32_get(unit,
                                                  &l2er_entry, MAC_BLOCK_INDEXf);
                mbi[mb_index].ref_count++;
            }
        }
    }

    return SOC_E_NONE;
}

#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *      bcm_er_l2_init
 * Description:
 *      Initialize chip-dependent parts of L2 module
 * Parameters:
 *      unit - StrataSwitch unit number.
 */

int
bcm_er_l2_init(int unit)
{
    int         was_running = FALSE;
    uint32      flags;
    sal_usecs_t interval;
    _bcm_er_l2_dvp_info_t dvp_info;

    if (soc_l2x_running(unit, &flags, &interval)) { 	 
        was_running = TRUE; 	 
        BCM_IF_ERROR_RETURN(soc_l2x_stop(unit)); 	 
    }

    if (!SOC_WARM_BOOT(unit)) {
        if (!SAL_BOOT_SIMULATION || SAL_BOOT_BCMSIM) {
            dvp_info.dyn_only = FALSE;
            dvp_info.by_port = FALSE;
            dvp_info.by_vlan = FALSE;
            dvp_info.by_trunk = FALSE;
        
            BCM_IF_ERROR_RETURN
                (soc_er_l2_traverse(unit, 
                                    _bcm_er_l2_addr_delete_by_vlan_port_cb,
                                    &dvp_info));
        }
    }

    if (_mbi_entries[unit] != NULL) {
        sal_free(_mbi_entries[unit]);
        _mbi_entries[unit] = NULL;
    }

    _mbi_num[unit] = SOC_REG_NUMELS(unit, MAC_BLOCK_TABLEr);
    _mbi_entries[unit] = sal_alloc(_mbi_num[unit] *
                                   sizeof(_bcm_mac_block_info_t),
                                   "BCM L2ER MAC blocking info");
    if (!_mbi_entries[unit]) {
        return BCM_E_MEMORY;
    }

    sal_memset(_mbi_entries[unit], 0,
               _mbi_num[unit] * sizeof(_bcm_mac_block_info_t));

#ifdef BCM_WARM_BOOT_SUPPORT    
    if (SOC_WARM_BOOT(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_er_l2_reload_mbi(unit));
    }
#endif /* BCM_WARM_BOOT_SUPPORT */    

    soc_l2x_register(unit, _bcm_l2_register_callback, NULL);

    if (was_running || SAL_BOOT_BCMSIM) {
        interval = (SAL_BOOT_BCMSIM)? BCMSIM_L2XMSG_INTERVAL : interval;
        soc_l2x_start(unit, flags, interval);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_er_l2_term
 * Description:
 *      Finalize chip-dependent parts of L2 module
 * Parameters:
 *      unit - StrataSwitch unit number.
 */

int
bcm_er_l2_term(int unit)
{
    if (_mbi_entries[unit] != NULL) {
        sal_free(_mbi_entries[unit]);
        _mbi_entries[unit] = NULL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_er_l2_conflict_get
 * Description:
 *      Given an L2 address, return existing addresses which could conflict.
 * Parameters:
 *      unit      - device unit number
 *      addr      - l2 address to search for conflicts
 *      cf_array  - (OUT) list of l2 addresses conflicting with addr
 *      cf_max    - number of entries allocated to cf_array
 *      cf_count  - (OUT) actual number of cf_array entries filled
 */

int
bcm_er_l2_conflict_get(int unit, bcm_l2_addr_t *addr,
                       bcm_l2_addr_t *cf_array, int cf_max,
                       int *cf_count)
{
    l2_entry_internal_entry_t l2ent;
    uint8  key[XGS_HASH_KEY_SIZE];
    int    int_bucket, ext_bucket, slot;
    int    int_hash_select, ext_hash_select;
    uint32 hash_control;
    bcm_pbmp_t pbm;

    *cf_count = 0;

    /* Get L2 hash select */
    SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &hash_control));
    int_hash_select = soc_reg_field_get(unit, HASH_CONTROLr, hash_control,
                                    L2_AND_VLAN_MAC_HASH_SELECTf);
    ext_hash_select = soc_reg_field_get(unit, HASH_CONTROLr, hash_control,
                                    L2_EXT_HASH_SELECTf);

    SOC_PBMP_CLEAR(pbm);
    BCM_IF_ERROR_RETURN(_bcm_er_l2_to_l2x(unit, &l2ent, addr, pbm));
    soc_er_l2x_base_entry_to_key(unit, (uint32 *) &l2ent, key);

    ext_bucket = soc_er_l2ext_hash(unit, ext_hash_select, key);
    int_bucket = soc_er_l2_hash(unit, int_hash_select, key);
    if (soc_mem_index_count(unit, L2_ENTRY_EXTERNALm)) {
        for (slot = 0;
             slot < SOC_L2EXT_BUCKET_SIZE && *cf_count < cf_max;
             slot++) {
            SOC_IF_ERROR_RETURN(soc_mem_read(unit, L2_ENTRY_EXTERNALm, 
                                MEM_BLOCK_ANY,
                                ext_bucket * SOC_L2EXT_BUCKET_SIZE + slot,
                                &l2ent));
            if (soc_L2_ENTRY_EXTERNALm_field32_get(unit, &l2ent, VALID_BITf) &&
                soc_L2_ENTRY_EXTERNALm_field32_get(unit, &l2ent, ACTIVEf)) {
                BCM_IF_ERROR_RETURN(_bcm_er_l2_from_l2x(unit, &cf_array[*cf_count], 
                                    &l2ent, &pbm));
                *cf_count += 1;
            }
        }
    }

    for (slot = 0;
         slot < SOC_L2X_BUCKET_SIZE && *cf_count < cf_max;
         slot++) {
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, L2_ENTRY_INTERNALm, MEM_BLOCK_ANY,
                          int_bucket * SOC_L2X_BUCKET_SIZE + slot,
                          &l2ent));
        if (soc_L2_ENTRY_INTERNALm_field32_get(unit, &l2ent, VALID_BITf) &&
            soc_L2_ENTRY_INTERNALm_field32_get(unit, &l2ent, ACTIVEf)) {
            BCM_IF_ERROR_RETURN(
                _bcm_er_l2_from_l2x(unit, &cf_array[*cf_count], &l2ent, &pbm));
            *cf_count += 1;
        }
    }

    for (slot = 0;
         slot < SOC_L2X_BUCKET_SIZE && *cf_count < cf_max;
         slot++) {
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, L2_ENTRY_OVERFLOWm, MEM_BLOCK_ANY,
                          slot, &l2ent));
        if (soc_L2_ENTRY_OVERFLOWm_field32_get(unit, &l2ent, VALID_BITf) &&
            soc_L2_ENTRY_OVERFLOWm_field32_get(unit, &l2ent, ACTIVEf)) {
            BCM_IF_ERROR_RETURN(
                _bcm_er_l2_from_l2x(unit, &cf_array[*cf_count], &l2ent, &pbm));
            *cf_count += 1;
        }

    }

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_er_l2_sw_dump
 * Purpose:
 *     Displays L2 information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_er_l2_sw_dump(int unit)
{
    _bcm_mac_block_info_t *mbi;
    char                 pfmt[SOC_PBMP_FMT_LEN];
    int                  i;
  	 
    soc_cm_print("\n");
    soc_cm_print("ER L2 MAC Blocking Info -\n");
    soc_cm_print("      Number : %d\n", _mbi_num[unit]);

    mbi = _mbi_entries[unit];
    soc_cm_print("      Entries (index: pbmp-count) :\n");
    if (mbi != NULL) {
        for (i = 0; i < _mbi_num[unit]; i++) {
            SOC_PBMP_FMT(mbi[i].mb_pbmp, pfmt);
            soc_cm_print("          %5d: %s-%d\n", i, pfmt, mbi[i].ref_count);
        }
    }
  	 
    soc_cm_print("\n");
  	 
    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#else  /* BCM_EASYRIDER_SUPPORT */
int _easyrider_l2_not_empty;
#endif  /* BCM_EASYRIDER_SUPPORT */
