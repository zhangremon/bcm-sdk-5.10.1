/*
 * $Id: l2.c 1.7.20.1 Broadcom SDK $
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
 * Purpose:     Trident L2 function implementations
 */
#include <soc/defs.h>
	 
#if defined(BCM_TRIDENT_SUPPORT)
	 
#include <assert.h>
	 
#include <sal/core/libc.h>
	 
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/hash.h>
#include <soc/l2x.h>
#include <soc/triumph.h>
	 
#include <bcm/l2.h>
#include <bcm/error.h>
	 
#include <bcm_int/esw/l2.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/trill.h>
#include <bcm_int/esw/trident.h>

#define DEFAULT_L2DELETE_CHUNKS		64	/* 16k entries / 64 = 256 */

/*
 * Purpose:
 *      Add TRILL Multicast entry
 * Parameters:
 *      unit            - Device unit number
 *      key             - defines entry type
 *      vid             - Customer Vlan ID
 *      mac             - Customer MAC
 *      trill_tree_id   - specify Multicast Tree ID
 *      group           - Multicast group
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int 
bcm_td_l2_trill_multicast_entry_add(int unit, int key, 
                                    bcm_vlan_t vid, bcm_mac_t mac, 
                                    uint8 trill_tree_id, bcm_multicast_t group)
{
    l2x_entry_t  l2x_entry, l2x_lookup;
    int rv;
    int mc_index,l2_index;

    sal_memset(&l2x_entry, 0, sizeof (l2x_entry));

    if (_BCM_MULTICAST_IS_SET(group)) {

        mc_index = _BCM_MULTICAST_ID_GET(group);

        switch (key) {
        case TR_L2_HASH_KEY_TYPE_BRIDGE:
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 L2__TRILL_NETWORK_RECEIVERS_PRESENTf, 1);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 KEY_TYPEf, TR_L2_HASH_KEY_TYPE_BRIDGE);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 L2__DEST_TYPEf, 0);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 L2__DESTINATIONf, mc_index);
            soc_L2Xm_field32_set(unit, &l2x_entry,
                                 L2__VLAN_IDf, vid);
            soc_L2Xm_mac_addr_set(unit, &l2x_entry, 
                                  L2__MAC_ADDRf, mac);
            break;

        case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS:
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_ACCESS__DEST_TYPEf, 0x3);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 KEY_TYPEf,
                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS);
            soc_L2Xm_field32_set(unit, &l2x_entry,
                                 TRILL_NONUC_ACCESS__L3MC_PTRf, mc_index);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_ACCESS__VLAN_IDf, vid);
            soc_L2Xm_mac_addr_set(unit, &l2x_entry, 
                                  TRILL_NONUC_ACCESS__MAC_ADDRf, mac);
            break;

        case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT:
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__TREE_IDf, trill_tree_id);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__TRILL_ACCESS_RECEIVERS_PRESENTf, 1);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__DEST_TYPEf, 0x3);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 KEY_TYPEf, 
                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__L3MC_INDEXf, mc_index);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__VLAN_IDf, vid);
            break;

        case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG:
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__TREE_IDf, trill_tree_id);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__TRILL_ACCESS_RECEIVERS_PRESENTf, 1);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__DEST_TYPEf, 0x3);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 KEY_TYPEf, 
                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__L3MC_INDEXf, mc_index);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__VLAN_IDf, vid);
            soc_L2Xm_mac_addr_set(unit, &l2x_entry, 
                                  TRILL_NONUC_NETWORK_LONG__MAC_ADDRESSf, mac);
            break;
                        
        default:
            break;
        }
    }
    soc_L2Xm_field32_set(unit, &l2x_entry, VALIDf, 1);

    /* See if the entry already exists */
    rv = soc_mem_generic_lookup(unit, L2Xm, MEM_BLOCK_ANY, 0, 
                                (void *)&l2x_entry, (void *)&l2x_lookup, &l2_index);

    if (rv == SOC_E_NONE) {
        if (key == TR_L2_HASH_KEY_TYPE_BRIDGE) {
            soc_L2Xm_field32_set(unit, &l2x_lookup,
                                 L2__TRILL_NETWORK_RECEIVERS_PRESENTf, 1);
            rv = soc_mem_write(unit, L2Xm,
                               MEM_BLOCK_ALL, l2_index,
                               &l2x_lookup);
        }
    } else if ((rv < 0) && (rv != SOC_E_NOT_FOUND)) {
        return rv;
    } else {
        /* Add Entry */
        rv = soc_mem_insert(unit, L2Xm, MEM_BLOCK_ANY, (void *)&l2x_entry);
    }
    return rv;
}

/*
 * Purpose:
 *      Delete TRILL Multicast entry
 * Parameters:
 *      unit            - Device unit number
 *      key             - defines entry type
 *      vid             - Customer Vlan ID
 *      mac             - Customer MAC
 *      trill_tree_id   - specify Multicast Tree ID
 *      group           - Multicast group
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int 
bcm_td_l2_trill_multicast_entry_delete(int unit, int key, bcm_vlan_t vid, 
                                       bcm_mac_t mac, uint8 trill_tree_id,
                                       bcm_multicast_t group)
{
    l2x_entry_t  l2x_entry, l2x_lookup;
    int rv;
    int mc_index, l2_index;

    sal_memset(&l2x_entry, 0, sizeof (l2x_entry));

    if (_BCM_MULTICAST_IS_SET(group)) {

        mc_index = _BCM_MULTICAST_ID_GET(group);

        switch (key) {
        case TR_L2_HASH_KEY_TYPE_BRIDGE:
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NETWORK_RECEIVERS_PRESENTf, 1);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 KEY_TYPEf, TR_L2_HASH_KEY_TYPE_BRIDGE);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 DEST_TYPEf, 0);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 DESTINATIONf, mc_index);
            soc_L2Xm_field32_set(unit, &l2x_entry,
                                 VLAN_IDf, vid);
            soc_L2Xm_mac_addr_set(unit, &l2x_entry, 
                                  MAC_ADDRf, mac);
            break;

        case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS:
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_ACCESS__DEST_TYPEf, 0x3);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_ACCESS__KEYf,
                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS);
            soc_L2Xm_field32_set(unit, &l2x_entry,
                                 TRILL_NONUC_ACCESS__L3MC_PTRf, mc_index);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_ACCESS__VLAN_IDf, vid);
            soc_L2Xm_mac_addr_set(unit, &l2x_entry, 
                                  TRILL_NONUC_ACCESS__MAC_ADDRf, mac);
            break;

        case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT:
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__TREE_IDf, trill_tree_id);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__TRILL_ACCESS_RECEIVERS_PRESENTf, 1);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__DEST_TYPEf, 0x3);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__KEYf, 
                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__L3MC_INDEXf, mc_index);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_SHORT__VLAN_IDf, vid);
            break;

        case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG:
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__TREE_IDf, trill_tree_id);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__TRILL_ACCESS_RECEIVERS_PRESENTf, 1);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__DEST_TYPEf, 0x3);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__KEYf, 
                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__L3MC_INDEXf, mc_index);
            soc_L2Xm_field32_set(unit, &l2x_entry, 
                                 TRILL_NONUC_NETWORK_LONG__VLAN_IDf, vid);
            soc_L2Xm_mac_addr_set(unit, &l2x_entry, 
                                  TRILL_NONUC_NETWORK_LONG__MAC_ADDRESSf, mac);
            break;
                        
        default:
            break;
        }
    }
    soc_L2Xm_field32_set(unit, &l2x_entry, VALIDf, 1);


    rv = soc_mem_search(unit, L2Xm, MEM_BLOCK_ANY, &l2_index, 
                        (void *)&l2x_entry, (void *)&l2x_lookup, 0);
                 
    if ((rv < 0) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } 

    rv = soc_mem_delete(unit, L2Xm, MEM_BLOCK_ANY, (void *)&l2x_entry);
    return rv;
}


/*
 * Description:
 *      Delete all TRILL tree-sepecific Multicast entries
 * Parameters:
 *     unit             - device number
 *     tree_id          - Trill Tree ID
 * Return:
 *     BCM_E_XXX
 */
int
bcm_td_l2_trill_multicast_entry_delete_all (int unit, uint8 tree_id)
{
    int           ix, key_type;
    int           chunksize, nchunks, chunk;
    l2x_entry_t   *l2xe, *l2xep;
    soc_control_t *soc = SOC_CONTROL(unit);
    int           rv = BCM_E_UNAVAIL;
    chunksize = soc_property_get(unit, spn_L2DELETE_CHUNKS,
                                 DEFAULT_L2DELETE_CHUNKS);

    l2xe = soc_cm_salloc(unit, chunksize * sizeof(*l2xe), "l2entrydel_chunk");
    if (l2xe == NULL) {
        return BCM_E_MEMORY;
    }

    nchunks = soc_mem_index_count(unit, L2Xm) / chunksize;
    rv = BCM_E_NONE;

    soc_mem_lock(unit, L2Xm);
    for (chunk = 0; chunk < nchunks; chunk++) {
        rv = soc_mem_read_range(unit, L2Xm, MEM_BLOCK_ANY,
                                chunk * chunksize,
                                (chunk + 1) * chunksize - 1,
                                l2xe);
        if (rv < 0) {
            break;
        }
        for (ix = 0; ix < chunksize; ix++) {
            l2xep = soc_mem_table_idx_to_pointer(unit, L2Xm,
                                                 l2x_entry_t *, l2xe, ix);
            if (!soc_L2Xm_field32_get(unit, l2xep, VALIDf)) {
                continue;
            }
            if (soc_L2Xm_field32_get(unit, l2xep, STATIC_BITf)) {
                continue;
            }
            if (!soc_mem_field32_get(unit, L2Xm, l2xep, PENDINGf)) {
                continue;
            }
            
            /* Match the Key Type to TRILL */
            key_type = soc_L2Xm_field32_get(unit, l2xep, KEY_TYPEf);
            if (key_type != TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS) {
                if (key_type != TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT) {
                    if (key_type != TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG) {
                        continue;
                    }
                }
            }

            if ((tree_id != soc_L2Xm_field32_get (unit, l2xep,
                                                  TRILL_NONUC_NETWORK_LONG__TREE_IDf)) ||
                (tree_id != soc_L2Xm_field32_get(unit, l2xep,
                                                 TRILL_NONUC_NETWORK_SHORT__TREE_IDf))){
                continue;
            }
            
            if (SOC_L2_DEL_SYNC_LOCK(soc) < 0) {
                rv = BCM_E_RESOURCE;
                break;
            }
            if (((rv = soc_mem_delete(unit, L2Xm, MEM_BLOCK_ALL, l2xep)) < 0) ||
                ((rv = soc_l2x_sync_delete(unit, (uint32 *) l2xep,
                                           ((chunk * chunksize) + ix), 0)) < 0)) {
                SOC_L2_DEL_SYNC_UNLOCK(soc);
                break;
            }
            SOC_L2_DEL_SYNC_UNLOCK(soc);
        }
        if (rv < 0) {
            break;
        }
    }
    soc_mem_unlock(unit, L2Xm);
    soc_cm_sfree(unit, l2xe);

    return rv;
}


#endif /* BCM_TRIDENT_SUPORT */
