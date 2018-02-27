/*
 * $Id: port.c 1.5.4.1 Broadcom SDK $
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
 */

#include <soc/defs.h>

#include <assert.h>

#include <sal/core/libc.h>
#if defined(BCM_TRIDENT_SUPPORT)

#include <shared/util.h>
#include <soc/mem.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/trident.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/xgs3.h>
#include <bcm_int/esw_dispatch.h>

STATIC soc_profile_mem_t *_bcm_td_egr_mask_profile[BCM_MAX_NUM_UNITS];
STATIC soc_profile_mem_t *_bcm_td_sys_cfg_profile[BCM_MAX_NUM_UNITS];

int
bcm_td_port_reinit(int unit)
{
    soc_profile_mem_t *profile;
    uint32 entry[SOC_MAX_MEM_WORDS];
    uint32 profile_index;
    bcm_module_t modid;
    bcm_port_t port;
    system_config_table_entry_t *sys_cfg_entry;
    int tpid_enable, tpid_index;
    int is_local;

    /* EGR_MASK profile */
    profile = _bcm_td_egr_mask_profile[unit];
    for (modid = 0; modid <= SOC_MODID_MAX(unit); modid++) {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, EGR_MASK_MODBASEm, MEM_BLOCK_ALL, modid,
                          entry));
        profile_index = soc_mem_field32_get(unit, EGR_MASK_MODBASEm, entry,
                                            BASEf);
        SOC_IF_ERROR_RETURN
            (soc_profile_mem_reference(unit, profile, profile_index,
                                       SOC_PORT_ADDR_MAX(unit) + 1));
    }

    /* SYSTEM_CONFIG_TABLE profile */
    profile = _bcm_td_sys_cfg_profile[unit];
    for (modid = 0; modid <= SOC_MODID_MAX(unit); modid++) {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, SYSTEM_CONFIG_TABLE_MODBASEm, MEM_BLOCK_ALL,
                          modid, entry));
        profile_index = soc_mem_field32_get(unit, SYSTEM_CONFIG_TABLE_MODBASEm,
                                            entry, BASEf);
        SOC_IF_ERROR_RETURN
            (soc_profile_mem_reference(unit, profile, profile_index,
                                       SOC_PORT_ADDR_MAX(unit) + 1));

        BCM_IF_ERROR_RETURN
            (_bcm_esw_modid_is_local(unit, modid, &is_local));

        if (!is_local) {
            /* Increment outer tpid reference count for non-local ports.
             * The local ports' outer tpid reference count is taken care
             * of by _bcm_fb2_outer_tpid_init.
             */
            for (port = 0; port <= SOC_PORT_ADDR_MAX(unit); port++) {
                sys_cfg_entry = SOC_PROFILE_MEM_ENTRY(unit, profile,
                        system_config_table_entry_t *, profile_index + port);
                tpid_enable = soc_SYSTEM_CONFIG_TABLEm_field32_get(unit,
                        sys_cfg_entry, OUTER_TPID_ENABLEf);
                for (tpid_index = 0; tpid_index < 4; tpid_index++) {
                    if (tpid_enable & (1 << tpid_index)) {
                        BCM_IF_ERROR_RETURN
                            (_bcm_fb2_outer_tpid_tab_ref_count_add(unit,
                                                             tpid_index, 1));
                    }
                }
            }
        }
    }

    return BCM_E_NONE;
}

int
bcm_td_port_init(int unit)
{
    soc_mem_t mem;
    int entry_words;
    union {
        egr_mask_entry_t egr_mask[128];
        system_config_table_entry_t sys_cfg[128];
        uint32 w[1];
    } entry;
    void *entries[1];
    int modid, port, index;
    uint32 profile_index;
    uint16 tpid;

    /* Create profile for EGR_MASK table */
    if (_bcm_td_egr_mask_profile[unit] == NULL) {
        _bcm_td_egr_mask_profile[unit] =
            sal_alloc(sizeof(soc_profile_mem_t), "EGR_MASK profile");
        if (_bcm_td_egr_mask_profile[unit] == NULL) {
            return BCM_E_MEMORY;
        }
        soc_profile_mem_t_init(_bcm_td_egr_mask_profile[unit]);
    }
    mem = EGR_MASKm;
    entry_words = sizeof(egr_mask_entry_t) / sizeof(uint32);
    BCM_IF_ERROR_RETURN
        (soc_profile_mem_create(unit, &mem, &entry_words, 1,
                                _bcm_td_egr_mask_profile[unit]));

    /* Create profile for SYSTEM_CONFIG_TABLE table */
    if (_bcm_td_sys_cfg_profile[unit] == NULL) {
        _bcm_td_sys_cfg_profile[unit] =
            sal_alloc(sizeof(soc_profile_mem_t),
                      "SYSTEM_CONFIG_TABLE profile");
        if (_bcm_td_sys_cfg_profile[unit] == NULL) {
            return BCM_E_MEMORY;
        }
        soc_profile_mem_t_init(_bcm_td_sys_cfg_profile[unit]);
    }
    mem = SYSTEM_CONFIG_TABLEm;
    entry_words = sizeof(system_config_table_entry_t) / sizeof(uint32);
    BCM_IF_ERROR_RETURN
        (soc_profile_mem_create(unit, &mem, &entry_words, 1,
                                _bcm_td_sys_cfg_profile[unit]));

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        return bcm_td_port_reinit(unit);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    entries[0] = &entry;

    /* Add default entries for EGR_MASK profile */
    sal_memset(entry.egr_mask, 0, sizeof(entry.egr_mask));
    BCM_IF_ERROR_RETURN
        (soc_profile_mem_add(unit, _bcm_td_egr_mask_profile[unit], entries,
                             SOC_PORT_ADDR_MAX(unit) + 1, &profile_index));
    for (modid = 1; modid <= SOC_MODID_MAX(unit); modid++) {
        SOC_IF_ERROR_RETURN
            (soc_profile_mem_reference(unit, _bcm_td_egr_mask_profile[unit],
                                       profile_index,
                                       SOC_PORT_ADDR_MAX(unit) + 1));
    }
    /* EGR_MASK_MODBASE should be 0 for all modid which should match with the
     * allocated default profile index */

    /* Add default entries for SYSTEM_CONFIG_TABLE profile */
    tpid = _bcm_fb2_outer_tpid_default_get(unit);
    BCM_IF_ERROR_RETURN(_bcm_fb2_outer_tpid_lkup(unit, tpid, &index));

    sal_memset(entry.sys_cfg, 0, sizeof(entry.sys_cfg));
    for (port = 0; port <= SOC_PORT_ADDR_MAX(unit); port++) {
        soc_mem_field32_set(unit, SYSTEM_CONFIG_TABLEm, &entry.sys_cfg[port],
                OUTER_TPID_ENABLEf, 1 << index);
    }

    BCM_IF_ERROR_RETURN
        (soc_profile_mem_add(unit, _bcm_td_sys_cfg_profile[unit], entries,
                             SOC_PORT_ADDR_MAX(unit) + 1, &profile_index));
    for (modid = 1; modid <= SOC_MODID_MAX(unit); modid++) {
        SOC_IF_ERROR_RETURN
            (soc_profile_mem_reference(unit, _bcm_td_sys_cfg_profile[unit],
                                       profile_index,
                                       SOC_PORT_ADDR_MAX(unit) + 1));
    }
    /* SYSTEM_CONFIG_TABLE_MODBASE should be 0 for all modid which should
     * match with the allocated default profile index */

    /* Update the TPID index reference count for all (module, port) except
     * for local (module, port). The TPID index reference count for
     * local (module, port) will be updated during port module init,
     * when default outer TPID will be set for each local port.
     */
    BCM_IF_ERROR_RETURN(_bcm_fb2_outer_tpid_tab_ref_count_add(unit, index,
                (SOC_MODID_MAX(unit) + 1 - NUM_MODID(unit)) *
                (SOC_PORT_ADDR_MAX(unit) + 1)));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td_port_ing_pri_cng_set
 * Description:
 *      Set packet priority and cfi to internal priority and color mapping
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      untagged     - (IN) For untagged packet (ignore pkt_pri, cfi argumnet)
 *      pkt_pri      - (IN) Packet priority (802.1p cos)
 *      cfi          - (IN) Packet CFI
 *      int_pri      - (IN) Internal priority
 *      color        - (IN) Color
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *      When both pkt_pri and cfi are -1, the setting is for untagged packet
 */
int
bcm_td_port_ing_pri_cng_set(int unit, bcm_port_t port, int untagged,
                            int pkt_pri, int cfi,
                            int int_pri, bcm_color_t color)
{
    port_tab_entry_t         pent;
    ing_pri_cng_map_entry_t  map[16];
    ing_untagged_phb_entry_t phb;
    void                     *entries[2];
    uint32                   profile_index, old_profile_index;
    int                      index;
    int                      pkt_pri_cur, pkt_pri_min, pkt_pri_max;
    int                      cfi_cur, cfi_min, cfi_max;

    if (pkt_pri < 0) {
        pkt_pri_min = 0;
        pkt_pri_max = 7;
    } else {
        pkt_pri_min = pkt_pri;
        pkt_pri_max = pkt_pri;
    }

    if (cfi < 0) {
        cfi_min = 0;
        cfi_max = 1;
    } else {
        cfi_min = cfi;
        cfi_max = cfi;
    }

    /* Get profile index from port table. */
    BCM_IF_ERROR_RETURN(soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY,
                                     SOC_PORT_MOD_OFFSET(unit, port), &pent));
    old_profile_index =
        soc_mem_field32_get(unit, PORT_TABm, &pent, TRUST_DOT1P_PTRf) * 16;

    entries[0] = map;
    entries[1] = &phb;

    BCM_IF_ERROR_RETURN
        (_bcm_ing_pri_cng_map_entry_get(unit, old_profile_index, 16, entries));

    if (untagged) {
        if (int_pri >= 0) {
            soc_mem_field32_set(unit, ING_UNTAGGED_PHBm, &phb, PRIf, int_pri);
        }
        if (color >= 0) {
            soc_mem_field32_set(unit, ING_UNTAGGED_PHBm, &phb, CNGf,
                                _BCM_COLOR_ENCODING(unit, color));
        }
    } else {
        for (pkt_pri_cur = pkt_pri_min; pkt_pri_cur <= pkt_pri_max;
             pkt_pri_cur++) {
            for (cfi_cur = cfi_min; cfi_cur <= cfi_max; cfi_cur++) {
                index = (pkt_pri_cur << 1) | cfi_cur;
                if (int_pri >= 0) {
                    soc_mem_field32_set(unit, ING_PRI_CNG_MAPm, &map[index],
                                        PRIf, int_pri);
                }
                if (color >= 0) {
                    soc_mem_field32_set(unit, ING_PRI_CNG_MAPm, &map[index],
                                        CNGf,
                                        _BCM_COLOR_ENCODING(unit, color));
                }
            }
        }
    }

    BCM_IF_ERROR_RETURN
        (_bcm_ing_pri_cng_map_entry_add(unit, entries, 16, &profile_index));
                                        

    if (old_profile_index != 0) {
        BCM_IF_ERROR_RETURN
            (_bcm_ing_pri_cng_map_entry_delete(unit, old_profile_index));
    }

    if (profile_index != old_profile_index) {
        soc_mem_field32_set(unit, PORT_TABm, &pent, TRUST_DOT1P_PTRf,
                            profile_index / 16);
        BCM_IF_ERROR_RETURN
            (soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL,
                           SOC_PORT_MOD_OFFSET(unit, port), &pent));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td_port_ing_pri_cng_get
 * Description:
 *      Get packet priority and cfi to internal priority and color mapping
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      untagged     - (IN) For untagged packet (ignore pkt_pri, cfi argumnet)
 *      pkt_pri      - (IN) Packet priority (802.1p cos)
 *      cfi          - (IN) Packet CFI
 *      int_pri      - (OUT) Internal priority
 *      color        - (OUT) Color
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *      When both pkt_pri and cfi are -1, the setting is for untagged packet
 */
int
bcm_td_port_ing_pri_cng_get(int unit, bcm_port_t port, int untagged,
                            int pkt_pri, int cfi,
                            int *int_pri, bcm_color_t *color)
{
    port_tab_entry_t         pent;
    ing_pri_cng_map_entry_t  map[16];
    ing_untagged_phb_entry_t phb;
    void                     *entries[2];
    uint32                   profile_index;
    int                      index;
    int                      hw_color;

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY,
                                     SOC_PORT_MOD_OFFSET(unit, port), &pent));

    profile_index =
        soc_mem_field32_get(unit, PORT_TABm, &pent, TRUST_DOT1P_PTRf) * 16;

    entries[0] = map;
    entries[1] = &phb;

    BCM_IF_ERROR_RETURN
        (_bcm_ing_pri_cng_map_entry_get(unit, profile_index, 16, entries));
    if (untagged) {
        if (int_pri != NULL) {
            *int_pri = soc_mem_field32_get(unit, ING_UNTAGGED_PHBm, &phb,
                                           PRIf);
        }
        if (color != NULL) {
            hw_color = soc_mem_field32_get(unit, ING_UNTAGGED_PHBm, &phb,
                                           CNGf);
            *color = _BCM_COLOR_DECODING(unit, hw_color);
        }
    } else {
        index = (pkt_pri << 1) | cfi;
        if (int_pri != NULL) {
            *int_pri = soc_mem_field32_get(unit, ING_PRI_CNG_MAPm, &map[index],
                                           PRIf);
        }
        if (color != NULL) {
            hw_color = soc_mem_field32_get(unit, ING_PRI_CNG_MAPm, &map[index],
                                           CNGf);
            *color = _BCM_COLOR_DECODING(unit, hw_color);
        }
    }

    return BCM_E_NONE;
}

int
bcm_td_port_egress_set(int unit, bcm_port_t port, int modid, bcm_pbmp_t pbmp)
{
    soc_profile_mem_t *profile;
    egr_mask_modbase_entry_t base_entry;
    egr_mask_entry_t mask_entries[128];
    void *entries[1];
    uint32 old_index, index;
    bcm_pbmp_t mask_pbmp;
    bcm_module_t modid_min, modid_max, local_modid;
    bcm_port_t port_min, port_max, local_port;
    bcm_trunk_t tid;
    int id, rv;

    profile = _bcm_td_egr_mask_profile[unit];

    BCM_PBMP_NEGATE(mask_pbmp, pbmp);
    BCM_PBMP_AND(mask_pbmp, PBMP_PORT_ALL(unit));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_gport_resolve(unit, port, &modid_min, &port_min, &tid,
                                    &id));
        if (-1 != id || -1 != tid) {
            return BCM_E_PORT;
        }
        modid_max = modid_min;
        port_max = port_min;
    } else {
        if (modid < -1 || modid > SOC_MODID_MAX(unit)) {
            return BCM_E_PARAM;
        } else if (modid == -1) {
            modid_min = 0;
            modid_max = SOC_MODID_MAX(unit);
        } else {
            modid_min = modid;
            modid_max = modid;
        }
        if (port < -1 || port > SOC_PORT_ADDR_MAX(unit)) {
            return BCM_E_PARAM;
        } else if (port == -1) {
            port_min = 0;
            port_max = SOC_PORT_ADDR_MAX(unit);
        } else {
            port_min = port;
            port_max = port;
        }
    }

    entries[0] = &mask_entries;

    soc_mem_lock(unit, EGR_MASK_MODBASEm);

    rv = BCM_E_NONE;
    for (local_modid = modid_min; local_modid <= modid_max; local_modid++) {
        rv = soc_mem_read(unit, EGR_MASK_MODBASEm, MEM_BLOCK_ALL, local_modid,
                          &base_entry);
        if (BCM_FAILURE(rv)) {
            break;
        }
        old_index = soc_mem_field32_get(unit, EGR_MASK_MODBASEm, &base_entry,
                                        BASEf);
        rv = soc_profile_mem_get(unit, profile, old_index,
                                 SOC_PORT_ADDR_MAX(unit) + 1, entries);
        if (BCM_FAILURE(rv)) {
            break;
        }
        for (local_port = port_min; local_port <= port_max; local_port++) {
            soc_mem_pbmp_field_set(unit, EGR_MASKm, &mask_entries[local_port],
                                   EGRESS_MASKf, &mask_pbmp);
        }
        rv = soc_profile_mem_add(unit, profile, entries,
                                 SOC_PORT_ADDR_MAX(unit) + 1, &index);
        if (BCM_FAILURE(rv)) {
            break;
        }
        rv = soc_mem_field32_modify(unit, EGR_MASK_MODBASEm, local_modid,
                                    BASEf, index);
        if (BCM_FAILURE(rv)) {
            break;
        }
        rv = soc_profile_mem_delete(unit, profile, old_index);
        if (BCM_FAILURE(rv)) {
            break;
        }
    }

    soc_mem_unlock(unit, EGR_MASK_MODBASEm);

    return rv;
}

int
bcm_td_port_egress_get(int unit, bcm_port_t port, int modid, bcm_pbmp_t *pbmp)
{
    soc_profile_mem_t *profile;
    egr_mask_modbase_entry_t base_entry;
    egr_mask_entry_t mask_entries[128];
    void *entries[1];
    uint32 index;
    bcm_pbmp_t mask_pbmp;
    bcm_module_t local_modid;
    bcm_port_t local_port;
    bcm_trunk_t tid;
    int id;

    profile = _bcm_td_egr_mask_profile[unit];

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_gport_resolve(unit, port, &local_modid, &local_port,
                                    &tid, &id));
        if (-1 != id || -1 != tid) {
            return BCM_E_PORT;
        }
    } else {
        if (modid < 0 || modid > SOC_MODID_MAX(unit) ||
            port < 0 || port > SOC_PORT_ADDR_MAX(unit)) {
            return BCM_E_PARAM;
        } else {
            local_modid = modid;
            local_port = port;
        }
    }

    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, EGR_MASK_MODBASEm, MEM_BLOCK_ALL, local_modid,
                      &base_entry));
    index = soc_mem_field32_get(unit, EGR_MASK_MODBASEm, &base_entry, BASEf);
    entries[0] = &mask_entries;
    BCM_IF_ERROR_RETURN
        (soc_profile_mem_get(unit, profile, index, SOC_PORT_ADDR_MAX(unit) + 1,
                             entries));
    soc_mem_pbmp_field_get(unit, EGR_MASKm, &mask_entries[local_port],
                           EGRESS_MASKf, &mask_pbmp);

    BCM_PBMP_NEGATE(*pbmp, mask_pbmp);
    BCM_PBMP_AND(*pbmp, PBMP_ALL(unit));
    BCM_PBMP_REMOVE(*pbmp, PBMP_LB(unit));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_mod_port_tpid_enable_write
 * Description:
 *      Write the Tag Protocol ID enables for a (module, port).
 * Parameters:
 *      unit - Device number
 *      mod  - Module ID
 *      port - Port number
 *      tpid_enable - Tag Protocol ID enables
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_td_mod_port_tpid_enable_write(int unit, bcm_module_t mod,
        bcm_port_t port, int tpid_enable)
{
    int rv = BCM_E_NONE;
    system_config_table_modbase_entry_t modbase_entry;
    system_config_table_entry_t *entry_array;
    void *entries;
    uint32 old_profile_index, new_profile_index;
    int i;

    SOC_IF_ERROR_RETURN
        (READ_SYSTEM_CONFIG_TABLE_MODBASEm(unit, MEM_BLOCK_ANY, mod,
                                           &modbase_entry));
    old_profile_index = soc_SYSTEM_CONFIG_TABLE_MODBASEm_field32_get(unit,
            &modbase_entry, BASEf);

    entry_array = sal_alloc(sizeof(system_config_table_entry_t) *
            (SOC_PORT_ADDR_MAX(unit) + 1), "system config table entry array");
    if (entry_array == NULL) {
        return BCM_E_MEMORY;
    }

    for (i = 0; i <= SOC_PORT_ADDR_MAX(unit); i++) {
       sal_memcpy(&entry_array[i],
               SOC_PROFILE_MEM_ENTRY(unit, _bcm_td_sys_cfg_profile[unit],
                   void *, old_profile_index + i),
               sizeof(system_config_table_entry_t)); 
    }

    soc_SYSTEM_CONFIG_TABLEm_field32_set(unit, &entry_array[port],
            OUTER_TPID_ENABLEf, tpid_enable);

    entries = entry_array;
    rv = soc_profile_mem_add(unit, _bcm_td_sys_cfg_profile[unit], &entries,
                             SOC_PORT_ADDR_MAX(unit) + 1, &new_profile_index);
    if (BCM_FAILURE(rv)) {
        sal_free(entry_array);
        return rv;
    }

    soc_SYSTEM_CONFIG_TABLE_MODBASEm_field32_set(unit, &modbase_entry,
            BASEf, new_profile_index);
    rv = WRITE_SYSTEM_CONFIG_TABLE_MODBASEm(unit, MEM_BLOCK_ALL, mod,
                                           &modbase_entry);
    if (BCM_FAILURE(rv)) {
        sal_free(entry_array);
        return rv;
    }

    rv = soc_profile_mem_delete(unit, _bcm_td_sys_cfg_profile[unit],
                                old_profile_index); 
    sal_free(entry_array);
    return rv;
}

/*
 * Function:
 *      _bcm_td_mod_port_tpid_enable_read
 * Description:
 *      Get the Tag Protocol ID enables for a (module, port).
 * Parameters:
 *      unit - Device number
 *      mod  - Module ID
 *      port - Port number
 *      tpid_enable - (OUT) Tag Protocol ID enables
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_td_mod_port_tpid_enable_read(int unit, bcm_module_t mod, bcm_port_t port,
        int *tpid_enable)
{
    system_config_table_modbase_entry_t modbase_entry;
    system_config_table_entry_t sys_cfg_entry;
    int base;

    SOC_IF_ERROR_RETURN
        (READ_SYSTEM_CONFIG_TABLE_MODBASEm(unit, MEM_BLOCK_ANY, mod,
                                           &modbase_entry));
    base = soc_SYSTEM_CONFIG_TABLE_MODBASEm_field32_get(unit, &modbase_entry,
            BASEf);

    SOC_IF_ERROR_RETURN
        (READ_SYSTEM_CONFIG_TABLEm(unit, MEM_BLOCK_ANY, base + port,
                                   &sys_cfg_entry));
    *tpid_enable = soc_SYSTEM_CONFIG_TABLEm_field32_get(unit, &sys_cfg_entry,
            OUTER_TPID_ENABLEf);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_mod_port_tpid_set
 * Description:
 *      Set the Tag Protocol ID for a (module, port).
 * Parameters:
 *      unit - Device number
 *      mod  - Module ID
 *      port - Port number
 *      tpid - Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_td_mod_port_tpid_set(int unit, bcm_module_t mod, bcm_port_t port,
        uint16 tpid)
{
    int rv = BCM_E_NONE;
    int tpid_enable, tpid_index;

    _bcm_fb2_outer_tpid_tab_lock(unit);

    rv = _bcm_td_mod_port_tpid_enable_read(unit, mod, port, &tpid_enable);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    tpid_index = 0;
    while (tpid_enable) {
        if (tpid_enable & 1) {
            rv = _bcm_fb2_outer_tpid_entry_delete(unit, tpid_index);
            if (BCM_FAILURE(rv)) {
                _bcm_fb2_outer_tpid_tab_unlock(unit);
                return rv;
            }
        }
        tpid_enable = tpid_enable >> 1;
        tpid_index++;
    }

    rv = _bcm_fb2_outer_tpid_entry_add(unit, tpid, &tpid_index);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    tpid_enable = 1 << tpid_index;
    rv = _bcm_td_mod_port_tpid_enable_write(unit, mod, port, tpid_enable);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_entry_delete(unit, tpid_index);
    }

    _bcm_fb2_outer_tpid_tab_unlock(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_td_mod_port_tpid_get
 * Description:
 *      Retrieve the Tag Protocol ID for a (module, port).
 * Parameters:
 *      unit - Device number
 *      mod  - Module ID 
 *      port - Port number
 *      tpid - (OUT) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_td_mod_port_tpid_get(int unit, bcm_module_t mod, bcm_port_t port,
        uint16 *tpid)
{
    int tpid_enable, tpid_index;

    BCM_IF_ERROR_RETURN
        (_bcm_td_mod_port_tpid_enable_read(unit, mod, port, &tpid_enable));

    tpid_index = 0;
    while (tpid_enable) {
        if (tpid_enable & 1) {
            return _bcm_fb2_outer_tpid_entry_get(unit, tpid, tpid_index);
        }
        tpid_enable = tpid_enable >> 1;
        tpid_index++;
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_td_mod_port_tpid_add
 * Description:
 *      Add TPID for a (module, port).
 * Parameters:
 *      unit - (IN) Device number
 *      mod  - (IN) Module ID
 *      port - (IN) Port number
 *      tpid - (IN) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_td_mod_port_tpid_add(int unit, bcm_module_t mod, bcm_port_t port, 
                      uint16 tpid)
{
    int rv = BCM_E_NONE;
    int remove_tpid = FALSE;
    int tpid_enable, index;

    _bcm_fb2_outer_tpid_tab_lock(unit);

    rv = _bcm_td_mod_port_tpid_enable_read(unit, mod, port, &tpid_enable);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    rv = _bcm_fb2_outer_tpid_lkup(unit, tpid, &index);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    if (rv == BCM_E_NOT_FOUND || !(tpid_enable & (1 << index))) {
        rv = _bcm_fb2_outer_tpid_entry_add(unit, tpid, &index);
        if (BCM_FAILURE(rv)) {
            _bcm_fb2_outer_tpid_tab_unlock(unit);
            return rv;
        }
        remove_tpid = TRUE;
    }

    tpid_enable |= (1 << index);
    rv = _bcm_td_mod_port_tpid_enable_write(unit, mod, port, tpid_enable);
    if (BCM_FAILURE(rv)) {
        if (remove_tpid) {
            _bcm_fb2_outer_tpid_entry_delete(unit, index);
        }
    }

    _bcm_fb2_outer_tpid_tab_unlock(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_td_mod_port_tpid_delete
 * Description:
 *      Delete TPID for a (module, port).
 * Parameters:
 *      unit - (IN) Device number
 *      mod  - (IN) Module ID
 *      port - (IN) Port number
 *      tpid - (IN) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_td_mod_port_tpid_delete(int unit, bcm_module_t mod, bcm_port_t port,
        uint16 tpid)
{
    int rv = BCM_E_NONE;
    int index, tpid_enable;

    _bcm_fb2_outer_tpid_tab_lock(unit);

    rv = _bcm_fb2_outer_tpid_lkup(unit, tpid, &index);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    } 

    rv = _bcm_td_mod_port_tpid_enable_read(unit, mod, port, &tpid_enable);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    if (tpid_enable & (1 << index)) {
        tpid_enable &= ~(1 << index);
        rv = _bcm_td_mod_port_tpid_enable_write(unit, mod, port, tpid_enable);
    } else {
        rv = BCM_E_NOT_FOUND;
    }
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    rv = _bcm_fb2_outer_tpid_entry_delete(unit, index);
    _bcm_fb2_outer_tpid_tab_unlock(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_td_mod_port_tpid_delete_all
 * Description:
 *      Delete all TPID for a (module, port).
 * Parameters:
 *      unit - (IN) Device number
 *      mod  - (IN) Module ID
 *      port - (IN) Port number
 * Return Value:
 *      BCM_E_XXX
 */
int 
_bcm_td_mod_port_tpid_delete_all(int unit, bcm_module_t mod, bcm_port_t port)
{
    int rv = BCM_E_NONE;
    int tpid_enable, tpid_index;

    _bcm_fb2_outer_tpid_tab_lock(unit);

    rv = _bcm_td_mod_port_tpid_enable_read(unit, mod, port, &tpid_enable);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    rv = _bcm_td_mod_port_tpid_enable_write(unit, mod, port, 0);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    tpid_index = 0;
    while (tpid_enable) {
        if (tpid_enable & 1) {
            rv = _bcm_fb2_outer_tpid_entry_delete(unit, tpid_index);
            if (BCM_FAILURE(rv)) {
                _bcm_fb2_outer_tpid_tab_unlock(unit);
                return rv;
            }
        }
        tpid_enable = tpid_enable >> 1;
        tpid_index++;
    }

    _bcm_fb2_outer_tpid_tab_unlock(unit);
    return rv;
}

#endif /* BCM_TRIDENT_SUPPORT */
