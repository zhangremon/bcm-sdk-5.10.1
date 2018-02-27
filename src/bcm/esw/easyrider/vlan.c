/*
 * $Id: vlan.c 1.36 Broadcom SDK $
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
 * File:        vlan.c
 * Purpose:     Provide low-level access to XGS3 VLAN resources
 */

#include <soc/defs.h>
#if defined(BCM_EASYRIDER_SUPPORT) 

#include <soc/drv.h>
#include <soc/mem.h>

#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/stg.h>
#include <bcm/port.h>
#include <bcm/trunk.h>
#include <bcm/stack.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/easyrider.h>
#include <bcm_int/esw/vlan.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/xgs3.h>

#include <bcm_int/esw_dispatch.h>

typedef struct _range_profile_s {
    igr_vlan_range_tbl_entry_t   profile_entry;
    int                          ref_count;
} _range_profile_t;

static _range_profile_t (*_range_profile_tab[BCM_MAX_NUM_UNITS])[];

#define RANGE_PROFILE_TAB(unit) _range_profile_tab[unit]

#define RANGE_PROFILE_ENTRY(unit, index) \
    ((*_range_profile_tab[unit])[index].profile_entry)

#define RANGE_PROFILE_REF_COUNT(unit, index) \
    ((*_range_profile_tab[unit])[index].ref_count)

#define RANGE_PROFILE_INIT_CHECK(unit) \
    if (RANGE_PROFILE_TAB(unit) == NULL) { return BCM_E_INIT; }

#define RANGE_PROFILE_DEFAULT  127

/*
 * Function : bcm_er_vlan_range_profile_init
 *
 * Purpose  : to initialize hardware IGR_VLAN_RANGE_TBL and allocate memory
 *            to cache hardware table in RAM.
 *
 * Note:
 *      Allocate memory to cache the profile table and initialize it.
 *      If memory to cache the profile table is already allocated, just
 *      initialize the table.
 */
int
bcm_er_vlan_range_profile_init(int unit)
{
    int i, idx, alloc_size;
    igr_vlan_range_tbl_entry_t profile_entry;
    vlan_range_idx_entry_t index_entry;
    trunk_vlan_range_idx_entry_t trunk_index_entry;
    soc_field_t min_f[] = {VLAN_MIN0f, VLAN_MIN1f, VLAN_MIN2f,
                           VLAN_MIN3f, VLAN_MIN4f, VLAN_MIN5f};
    soc_field_t max_f[] = {VLAN_MAX0f, VLAN_MAX1f, VLAN_MAX2f,
                           VLAN_MAX3f, VLAN_MAX4f, VLAN_MAX5f};

    alloc_size = sizeof(_range_profile_t) *
                     soc_mem_index_count(unit, IGR_VLAN_RANGE_TBLm);
    if (RANGE_PROFILE_TAB(unit) == NULL) {

        /* Allocate memory to cache Range Profile table */
        RANGE_PROFILE_TAB(unit) = sal_alloc(alloc_size,
                                           "Cached VLAN Range Profile Table");

        if (RANGE_PROFILE_TAB(unit) == NULL) {
            return BCM_E_MEMORY;
        }
    }

    if (SOC_WARM_BOOT(unit)) {
        for (i = 0; i < soc_mem_index_count(unit, IGR_VLAN_RANGE_TBLm); i++) {
            BCM_IF_ERROR_RETURN
                (READ_IGR_VLAN_RANGE_TBLm(unit, MEM_BLOCK_ANY, i,
                                          &RANGE_PROFILE_ENTRY(unit, i)));
        }

        /* Increment the ref count for all modid/ports */
        for (i = 0; i < soc_mem_index_count(unit, VLAN_RANGE_IDXm); i++) {
            BCM_IF_ERROR_RETURN
                (READ_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, i, &index_entry));
            idx = soc_mem_field32_get(unit, VLAN_RANGE_IDXm,
                                      &index_entry, VLAN_RANGE_INDEXf);
            RANGE_PROFILE_REF_COUNT(unit, idx)++;
        }

        /* Increment the ref count for all trunk IDs */
        for (i = 0; i < soc_mem_index_count(unit, TRUNK_VLAN_RANGE_IDXm); i++) {
            BCM_IF_ERROR_RETURN
                (READ_TRUNK_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, i,
                                            &trunk_index_entry));
            idx = soc_mem_field32_get(unit, TRUNK_VLAN_RANGE_IDXm,
                                      &trunk_index_entry, VLAN_RANGE_INDEXf);
            RANGE_PROFILE_REF_COUNT(unit, idx)++;
        }
        /* One extra increment to preserve location RANGE_PROFILE_DEFAULT */
        RANGE_PROFILE_REF_COUNT(unit, RANGE_PROFILE_DEFAULT)++;
    } else {

        /* Fill each min/max range as unused. Unused ranges are
         * identified by { min == 1, max == 0 }
         */
        for (i = 0; i < 6; i++) {
            soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                                &profile_entry, min_f[i], 1);
            soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                                &profile_entry, max_f[i], 0);
        }

        sal_memset(RANGE_PROFILE_TAB(unit), 0, alloc_size);

        for (i = 0; i < soc_mem_index_count(unit, IGR_VLAN_RANGE_TBLm); i++) {
            /* init HW entry */
            SOC_IF_ERROR_RETURN
               (WRITE_IGR_VLAN_RANGE_TBLm(unit, MEM_BLOCK_ALL,
                                          i, &profile_entry));

            /* init cache entry */
            sal_memcpy(&RANGE_PROFILE_ENTRY(unit, i),
                       &profile_entry, sizeof(profile_entry));
        }

        /* Point all modid/ports to entry RANGE_PROFILE_DEFAULT. Increment
         * the ref count for location RANGE_PROFILE_DEFAULT for all modid/ports.
         */
        soc_mem_field32_set(unit, VLAN_RANGE_IDXm,
                            &index_entry, VLAN_RANGE_INDEXf, RANGE_PROFILE_DEFAULT);
        for (i = 0; i < soc_mem_index_count(unit, VLAN_RANGE_IDXm); i++) {
            BCM_IF_ERROR_RETURN
                (WRITE_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, i, &index_entry));
            RANGE_PROFILE_REF_COUNT(unit, RANGE_PROFILE_DEFAULT)++;
        }

        /* Point all trunk IDs to entry RANGE_PROFILE_DEFAULT. Increment
         * the ref count for location RANGE_PROFILE_DEFAULT for all trunk IDs.
         */
        soc_mem_field32_set(unit, TRUNK_VLAN_RANGE_IDXm,
                            &trunk_index_entry, VLAN_RANGE_INDEXf,
                            RANGE_PROFILE_DEFAULT);
        for (i = 0; i < soc_mem_index_count(unit, TRUNK_VLAN_RANGE_IDXm); i++) {
            BCM_IF_ERROR_RETURN
                (WRITE_TRUNK_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY,
                                             i, &trunk_index_entry));
            RANGE_PROFILE_REF_COUNT(unit, RANGE_PROFILE_DEFAULT)++; 
        }
        /* One extra increment to preserve location RANGE_PROFILE_DEFAULT */
        RANGE_PROFILE_REF_COUNT(unit, RANGE_PROFILE_DEFAULT)++;
    }

    return BCM_E_NONE;
}

/*
 * Function : bcm_er_vlan_range_profile_entry_add
 *
 * Purpose  : add a new entry to vlan range profile table
 *
 */
STATIC int
bcm_er_vlan_range_profile_entry_add(int unit,
                                    igr_vlan_range_tbl_entry_t *entry,
                                    int *index)
{   
    int free_index = -1;
    int i; 

    /*
     * Search for an existing entry that has the same configuration.
     */
    for (i = 0 ; i < soc_mem_index_count(unit, IGR_VLAN_RANGE_TBLm); i++) {
        if (soc_mem_compare_entry(unit, IGR_VLAN_RANGE_TBLm,
                  &RANGE_PROFILE_ENTRY(unit, i), entry) == 0) {
           RANGE_PROFILE_REF_COUNT(unit, i)++;
           *index = i;
           return BCM_E_NONE;
        }
        if ((free_index == -1) && (RANGE_PROFILE_REF_COUNT(unit, i) == 0)) {
            free_index = i;
        }
    }

    if (free_index >= 0) {
        /* Insert the new configuration into profile table. */
        SOC_IF_ERROR_RETURN
           (WRITE_IGR_VLAN_RANGE_TBLm(unit, MEM_BLOCK_ALL, free_index, entry));
        RANGE_PROFILE_REF_COUNT(unit, free_index)++;
        sal_memcpy(&RANGE_PROFILE_ENTRY(unit, free_index), entry,
                   sizeof(*entry));
        *index = free_index;
        return BCM_E_NONE;
    }

    return BCM_E_RESOURCE;
}

/*
 * Function : bcm_er_vlan_range_profile_index_get
 *
 * Purpose  : get an index to vlan range profile table entry from port
 *
 * Note:
 *
 */
STATIC int
bcm_er_vlan_range_profile_index_get(int unit, bcm_port_t port, int *idx)
{
    bcm_trunk_t                     tid=0, tid_hi=0;
    bcm_module_t                    modid;
    bcm_port_t                      mod_port;
    uint32                          prof_idx;
    vlan_range_idx_entry_t          index_entry; 
    trunk_vlan_range_idx_entry_t    trunk_index_entry;


    /* Get the current profile table pointer for this port/trunk */
    if (port & BCM_TGID_TRUNK_INDICATOR(unit)) {
        tid_hi = port & BCM_TGID_PORT_TRUNK_MASK_HI(unit);
        tid = (port & BCM_TGID_PORT_TRUNK_MASK(unit)) | (tid_hi >> 1);
        SOC_IF_ERROR_RETURN
            (READ_TRUNK_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, tid,
                                        &trunk_index_entry));
        prof_idx = soc_mem_field32_get(unit, TRUNK_VLAN_RANGE_IDXm,
                                      &trunk_index_entry, VLAN_RANGE_INDEXf);
    } else {
        /* The port value that was passed in contains modid + port.
         * Use this to calculate the index to the VLAN_RANGE_IDX table.
         */
        modid = port >> BCM_VLAN_TRANSLATE_PORTMOD_MOD_SHIFT;
        if (modid > SOC_MODID_MAX(unit)) {
            return BCM_E_PARAM;
        }
        mod_port = (modid << 5);
        mod_port |= (port & BCM_VLAN_TRANSLATE_PORTMOD_PORT_MASK);
        SOC_IF_ERROR_RETURN
            (READ_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, mod_port,
                                  &index_entry));
        prof_idx = soc_mem_field32_get(unit, VLAN_RANGE_IDXm,
                                      &index_entry, VLAN_RANGE_INDEXf);
    }

    *idx = prof_idx;
    return BCM_E_NONE;
}

/*
 * Function : bcm_er_vlan_range_profile_entry_get
 *
 * Purpose  : get a copy of cached vlan range profile table entry
 *
 * Note:
 *
 */
STATIC void
bcm_er_vlan_range_profile_entry_get(int unit, 
                                    igr_vlan_range_tbl_entry_t *entry, 
                                    uint32 index)
{

    assert(index >= (uint32)soc_mem_index_min(unit, IGR_VLAN_RANGE_TBLm));
    assert(index <= (uint32)soc_mem_index_max(unit, IGR_VLAN_RANGE_TBLm));
    assert(RANGE_PROFILE_REF_COUNT(unit,index) > 0);
    assert(entry != NULL);

    sal_memcpy(entry, &RANGE_PROFILE_ENTRY(unit, index),
               sizeof(*entry));
}


/*
 * Function : bcm_er_vlan_range_profile_entry_delete
 *
 * Purpose  : remove an entry from vlan range profile table
 *
 * Note:
 *
 */
STATIC void
bcm_er_vlan_range_profile_entry_delete(int unit, int index)
{
    int i;
    igr_vlan_range_tbl_entry_t profile_entry;
    soc_field_t min_f[] = {VLAN_MIN0f, VLAN_MIN1f, VLAN_MIN2f,
                           VLAN_MIN3f, VLAN_MIN4f, VLAN_MIN5f};
    soc_field_t max_f[] = {VLAN_MAX0f, VLAN_MAX1f, VLAN_MAX2f,
                           VLAN_MAX3f, VLAN_MAX4f, VLAN_MAX5f};

    assert(index >= soc_mem_index_min(unit, IGR_VLAN_RANGE_TBLm));
    assert(index <= soc_mem_index_max(unit, IGR_VLAN_RANGE_TBLm));
    assert(RANGE_PROFILE_REF_COUNT(unit, index) > 0);

    if (--RANGE_PROFILE_REF_COUNT(unit, index) == 0) {
        /* Mark the min/max ranges as unused. Unused ranges are
         * identified by { min == 1, max == 0 }
         */
        for (i = 0; i < 6; i++) {
            soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                                &profile_entry, min_f[i], 1);
            soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                                &profile_entry, max_f[i], 0);
        }
        sal_memcpy(&RANGE_PROFILE_ENTRY(unit, index), &profile_entry,
                   sizeof(profile_entry));
        WRITE_IGR_VLAN_RANGE_TBLm(unit, MEM_BLOCK_ALL, index, &profile_entry);
    }
}

/*
 * Function:
 *      bcm_er_vlan_translate_add
 * Purpose:
 *      Add an entry to the specified vlan translate table.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_vlan_translate_add(int unit, int port, bcm_module_t modid, 
                          bcm_trunk_t tgid, bcm_vlan_t old_vid,
                          bcm_vlan_t new_vid, int prio, int xtable)
{
    igr_vlan_xlate_entry_t vtent, result;
    int                    ix, rv;
    soc_mem_t           mem;
    int                 addvid;

    switch (xtable) {
    case BCM_VLAN_XLATE_ING:
        mem = IGR_VLAN_XLATEm;
        addvid = 0;
        break;
    case BCM_VLAN_XLATE_EGR:
        mem = EGR_VLAN_XLATEm;
        addvid = -1;
        break;
    case BCM_VLAN_XLATE_DTAG:
        mem = IGR_VLAN_XLATEm;
        addvid = 1;
        break;
    default:
        return BCM_E_INTERNAL;
    }

    if (tgid == -1) {
        if (!BCM_VLAN_VALID(old_vid)) {
            return BCM_E_PARAM;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
            return BCM_E_PARAM;
        }
    } else {
        TRUNK_CHK_TGID128(unit, tgid);
    }

    if ((prio != -1) && ((prio & ~BCM_PRIO_MASK) != 0)) {
        return BCM_E_PARAM;
    }

    sal_memset(&vtent, 0, sizeof(vtent));

    if (tgid == -1) {
        if (mem == IGR_VLAN_XLATEm) {
            soc_mem_field32_set(unit, mem, &vtent, MOD_IDf, modid);
        }
        soc_mem_field32_set(unit, mem, &vtent, PORT_TGIDf, port);
    } else {
        soc_mem_field32_set(unit, mem, &vtent, MOD_IDf,
                            BCM_TRUNK_TO_MODIDf(unit, tgid));
        soc_mem_field32_set(unit, mem, &vtent, PORT_TGIDf,
                            BCM_TRUNK_TO_TGIDf(unit, tgid));
    }
    soc_mem_field32_set(unit, mem, &vtent, OLD_VLAN_IDf, old_vid);

    soc_mem_lock(unit, mem);

    if (addvid >= 0) {
        rv = soc_mem_search(unit, mem, MEM_BLOCK_ANY, &ix, &vtent, &result, 0);
        if (rv == SOC_E_NONE) {
            int ve_add;

            ve_add = soc_mem_field32_get(unit, mem, &result, ADD_VIDf);
            if (ve_add != addvid) {
                soc_mem_unlock(unit, mem);
                return BCM_E_EXISTS;
            }
        }
    }

    soc_mem_field32_set(unit, mem, &vtent, NEW_VLAN_IDf, new_vid);
    if (prio >= 0) {
        soc_mem_field32_set(unit, mem, &vtent, RPEf, 1);
        soc_mem_field32_set(unit, mem, &vtent, PRIf, prio);
    }
    if (addvid >= 0) {
        soc_mem_field32_set(unit, mem, &vtent, ADD_VIDf, addvid);
    }

    rv = soc_mem_insert(unit, mem, MEM_BLOCK_ALL, &vtent);
    soc_mem_unlock(unit, mem);
    return rv;
}


/*
 * Function:
 *      bcm_er_vlan_translate_get
 * Purpose:
 *      Get an entry from the specified vlan translate table.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_vlan_translate_get(int unit, int port, bcm_module_t modid,
                          bcm_trunk_t tgid, bcm_vlan_t old_vid,
                          bcm_vlan_t *new_vid, int *prio, int xtable)
{

    igr_vlan_xlate_entry_t  vent, res_vent;
    soc_mem_t               mem;
    int                     rv, idx = 0;
    
    switch (xtable) {
    case BCM_VLAN_XLATE_ING:
        mem = IGR_VLAN_XLATEm;
        break;
    case BCM_VLAN_XLATE_EGR:
        mem = EGR_VLAN_XLATEm;
        break;
    case BCM_VLAN_XLATE_DTAG:
        mem = IGR_VLAN_XLATEm;
        break;
    default:
        return BCM_E_INTERNAL;
    }
    
    if (tgid == -1) {
        if (!BCM_VLAN_VALID(old_vid)) {
            return BCM_E_PARAM;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
            return BCM_E_PARAM;
        }
    } else {
        TRUNK_CHK_TGID128(unit, tgid);
    }

    sal_memset(&vent, 0, sizeof(igr_vlan_xlate_entry_t));
    
    if (tgid == -1) {
        if (mem == IGR_VLAN_XLATEm) {
            soc_mem_field32_set(unit, mem, &vent, MOD_IDf, modid);
        }
        soc_mem_field32_set(unit, mem, &vent, PORT_TGIDf, port);
    } else {
        soc_mem_field32_set(unit, mem, &vent, MOD_IDf,
                            BCM_TRUNK_TO_MODIDf(unit, tgid));
        soc_mem_field32_set(unit, mem, &vent, PORT_TGIDf,
                            BCM_TRUNK_TO_TGIDf(unit, tgid));
    }
    soc_mem_field32_set(unit, mem, &vent, OLD_VLAN_IDf, old_vid);

    rv = soc_mem_search(unit, mem, MEM_BLOCK_ANY, &idx, &vent, &res_vent, 0);

    if (BCM_FAILURE(rv)) {
        return rv;
    }

    *new_vid = soc_mem_field32_get(unit, mem, &res_vent, NEW_VLAN_IDf);
    *prio = soc_mem_field32_get(unit, mem, &res_vent, PRIf); 
    return BCM_E_NONE;
}   


/*
 * Function   :
 *      _bcm_fb_vlan_translate_parse
 * Description   :
 *      Helper function for an API to parse a vlan translate 
 *      entry for Firebolt and call given call back.
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      trvs_info       (IN/OUT) Traverse structure that contain all relevant info
 */
int 
_bcm_er_vlan_translate_parse(int unit, soc_mem_t mem, uint32* vent,
                             _bcm_vlan_translate_traverse_t *trvs_info)
{
    bcm_port_t  port;

    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

    port = soc_mem_field32_get(unit,mem , vent,PORT_TGIDf);
    BCM_IF_ERROR_RETURN(
        bcm_esw_port_gport_get(unit, port, &(trvs_info->gport)));
    trvs_info->outer_vlan = soc_mem_field32_get(unit,mem , vent, OLD_VLAN_IDf);
    trvs_info->action->new_outer_vlan = soc_mem_field32_get(unit, mem, vent, 
                                                            NEW_VLAN_IDf);
    trvs_info->action->priority = soc_mem_field32_get(unit,mem , vent, 
                                                            PRIf);
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_er_vlan_translate_delete
 * Purpose:
 *      Delete an entry from the specified vlan translate table.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      No support for wildcarding of port or old_vid
 */
int
bcm_er_vlan_translate_delete(int unit, int port, bcm_module_t modid,
                             bcm_trunk_t tgid, bcm_vlan_t old_vid, int xtable)
{
    /* The entry is declared as igr_vlan_xlate_entry_t, but is used
     * for egr table as well. Is it safe?
     */
    igr_vlan_xlate_entry_t vtent, result;
    int                    ix, rv;
    soc_mem_t           mem;
    int                 addvid;

    switch (xtable) {
    case BCM_VLAN_XLATE_ING:
        mem = IGR_VLAN_XLATEm;
        addvid = 0;
        break;
    case BCM_VLAN_XLATE_EGR:
        mem = EGR_VLAN_XLATEm;
        addvid = -1;
        break;
    case BCM_VLAN_XLATE_DTAG:
        mem = IGR_VLAN_XLATEm;
        addvid = 1;
        break;
    default:
        return BCM_E_INTERNAL;
    }

    if (tgid == -1) {
        if (!BCM_VLAN_VALID(old_vid)) {
            return BCM_E_PARAM;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
            return BCM_E_PARAM;
        }
    } else {
        TRUNK_CHK_TGID128(unit, tgid);
    }

    sal_memset(&vtent, 0, sizeof(vtent));

    if (tgid == -1) {
        if (mem == IGR_VLAN_XLATEm) {
            soc_mem_field32_set(unit, mem, &vtent, MOD_IDf, modid);
        }
        soc_mem_field32_set(unit, mem, &vtent, PORT_TGIDf, port);
    } else {
        soc_mem_field32_set(unit, mem, &vtent, MOD_IDf,
                            BCM_TRUNK_TO_MODIDf(unit, tgid));
        soc_mem_field32_set(unit, mem, &vtent, PORT_TGIDf,
                            BCM_TRUNK_TO_TGIDf(unit, tgid));
    }
    soc_mem_field32_set(unit, mem, &vtent, OLD_VLAN_IDf, old_vid);

    soc_mem_lock(unit, mem);
    
    if (addvid >= 0) {
        rv = soc_mem_search(unit, mem, MEM_BLOCK_ANY, &ix, &vtent, &result, 0);
        if (rv == SOC_E_NONE) {
            int vvid;
            vvid = soc_mem_field32_get(unit, mem, &result, ADD_VIDf);
            if (vvid != addvid) {
                soc_mem_unlock(unit, mem);
                return BCM_E_EXISTS;
            }
        }
    }

    rv = soc_mem_delete(unit, mem, MEM_BLOCK_ALL, &vtent);
    soc_mem_unlock(unit, mem);

    return rv;
}   
    
/*
 * Function:
 *      bcm_er_vlan_translate_delete_all
 * Purpose:
 *      Delete all entries from the specified vlan translate table.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_vlan_translate_delete_all(int unit, int xtable)
{
    vlan_xlate_entry_t  *vtentries, *vtent;
    int                 index_min, index_max, rv, i;
    soc_mem_t           mem;
    int                 addvid, ve_add;

    switch (xtable) {
    case BCM_VLAN_XLATE_ING:
        mem = IGR_VLAN_XLATEm;
        addvid = 0;
        break;
    case BCM_VLAN_XLATE_EGR:
        mem = EGR_VLAN_XLATEm;
        addvid = -1;
        break;
    case BCM_VLAN_XLATE_DTAG:
        mem = IGR_VLAN_XLATEm;
        addvid = 1;
        break;
    default:
        return BCM_E_INTERNAL;
    }

    if (addvid < 0) {
        return soc_mem_clear(unit, mem, MEM_BLOCK_ALL, 0);
    }

    index_min = soc_mem_index_min(unit, mem);

    /* Get the actual no. of valid entries present */
    index_max = soc_mem_entries(unit, mem, MEM_BLOCK_ANY);
    if (index_max == 0) {
        return BCM_E_NONE;
    }

    vtentries = soc_cm_salloc(unit, index_max * sizeof(*vtentries),
                              "vlan xlate");
    if (vtentries == NULL) {
        return BCM_E_MEMORY;
    }

    soc_mem_lock(unit, mem);
    rv = soc_mem_read_range(unit, mem, MEM_BLOCK_ANY,
                            index_min, (index_max - 1), vtentries);
    if (rv >= 0) {
        for (i = 0; i < index_max; i++) {
            vtent = soc_mem_table_idx_to_pointer(unit, mem,
                                                 vlan_xlate_entry_t *,
                                                 vtentries, i);
            ve_add = soc_mem_field32_get(unit, mem, vtent, ADD_VIDf);
            if (ve_add == addvid) {
                rv = soc_mem_delete(unit, mem, MEM_BLOCK_ALL, vtent);
                if (rv < 0) {
                    break;
                }
            }
        }
    }

    soc_mem_unlock(unit, mem);
    soc_cm_sfree(unit, vtentries);
    return rv;
}

/*
 * Function:
 *      bcm_er_vlan_translate_range_add
 * Purpose:
 *      Add a vlan range along with a vlan tranlation entry
 *      in the specified vlan translate table.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_vlan_translate_range_add(int unit, bcm_gport_t gport, 
                                bcm_vlan_t old_vid_low, bcm_vlan_t old_vid_high,
                                bcm_vlan_t new_vid, int int_prio, int xtable)
{
    int rv = BCM_E_UNAVAIL;
    int old_idx, new_idx, min, max, i;
    bcm_trunk_t tid=0;
    int modid, port, id, mod_port=0;
    vlan_range_idx_entry_t index_entry; 
    trunk_vlan_range_idx_entry_t trunk_index_entry;
    igr_vlan_range_tbl_entry_t profile_entry;
    soc_field_t min_f[] = {VLAN_MIN0f, VLAN_MIN1f, VLAN_MIN2f,
                           VLAN_MIN3f, VLAN_MIN4f, VLAN_MIN5f};
    soc_field_t max_f[] = {VLAN_MAX0f, VLAN_MAX1f, VLAN_MAX2f,
                           VLAN_MAX3f, VLAN_MAX4f, VLAN_MAX5f};

    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, gport, &modid, &port, &tid, &id));

    if (-1 != id) {
        return BCM_E_PARAM;
    }
    /* Get the current profile table pointer for this port/trunk */
    if (-1 != tid) {
        SOC_IF_ERROR_RETURN
            (READ_TRUNK_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, tid,
                                        &trunk_index_entry));
        old_idx = soc_mem_field32_get(unit, TRUNK_VLAN_RANGE_IDXm,
                                      &trunk_index_entry, VLAN_RANGE_INDEXf);
    } else {
        /* The port value that was passed in contains modid + port.
         * Use this to calculate the index to the VLAN_RANGE_IDX table.
         */
        mod_port = (modid << 5);
        mod_port |= (port & BCM_VLAN_TRANSLATE_PORTMOD_PORT_MASK);
        SOC_IF_ERROR_RETURN
            (READ_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, mod_port,
                                  &index_entry));
        old_idx = soc_mem_field32_get(unit, VLAN_RANGE_IDXm,
                                      &index_entry, VLAN_RANGE_INDEXf);
    }
    
    /* Get the profile table entry for this port/trunk */
    bcm_er_vlan_range_profile_entry_get(unit, &profile_entry, old_idx);

    /* Find the first unused min/max range. Unused ranges are
     * identified by { min == 1, max == 0 }
     */
    for (i = 0 ; i < 6 ; i++) {
        min = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, min_f[i]);
        max = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, max_f[i]);
        if ((min == 1) && (max == 0)) {
            break;
        } else if (min == old_vid_low) {
            /* Can't have multiple ranges with the same min */
            return BCM_E_EXISTS;
        }
    }
    if (i == 6) {
        /* All ranges are taken */
        return BCM_E_FULL;
    }

    /* Insert the new range into the table entry sorted by min VID */
    for ( ; i > 0 ; i--) {
        min = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, min_f[i - 1]);
        max = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, max_f[i - 1]);
        if (min > old_vid_low) {
            /* Move existing min/max down */
            soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                                &profile_entry, min_f[i], min);
            soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                                &profile_entry, max_f[i], max);
        } else {
            break;
        }
    }
    soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                        &profile_entry, min_f[i], old_vid_low);
    soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                        &profile_entry, max_f[i], old_vid_high);

    /* Try adding the new profile table entry */
    rv = bcm_er_vlan_range_profile_entry_add(unit, &profile_entry,
                                         &new_idx);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    /* Add an entry in the vlan translate table for the low VID */
    rv = bcm_er_vlan_translate_add(unit, port, modid, tid, old_vid_low,
                                    new_vid, int_prio, xtable);
    if (rv != BCM_E_NONE) {
        bcm_er_vlan_range_profile_entry_delete(unit, new_idx);
        return rv;
    }

    /* Update the profile pointer */
    if (port & BCM_TGID_TRUNK_INDICATOR(unit)) {
        soc_mem_field32_set(unit, TRUNK_VLAN_RANGE_IDXm,
                            &trunk_index_entry, VLAN_RANGE_INDEXf, new_idx);
        WRITE_TRUNK_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, tid,
                                    &trunk_index_entry);
    } else {
        soc_mem_field32_set(unit, VLAN_RANGE_IDXm,
                            &index_entry, VLAN_RANGE_INDEXf, new_idx);
        WRITE_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, mod_port, &index_entry);
    }

    /* Delete the old profile entry */
    bcm_er_vlan_range_profile_entry_delete(unit, old_idx);

    return rv;
}

/*
 * Function:
 *      bcm_er_vlan_translate_range_get
 * Purpose:
 *      Add a vlan range along with a vlan tranlation entry
 *      in the specified vlan translate table.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_vlan_translate_range_get(int unit, bcm_gport_t gport, bcm_vlan_t old_vid_low,
                                bcm_vlan_t old_vid_high, bcm_vlan_t *new_vid,
                                int *int_prio, int xtable)
{
    int             old_idx, min, max, i, id;
    bcm_port_t      port;
    bcm_trunk_t     tgid;
    bcm_module_t    modid;

    igr_vlan_range_tbl_entry_t profile_entry;
    soc_field_t min_f[] = {VLAN_MIN0f, VLAN_MIN1f, VLAN_MIN2f,
                           VLAN_MIN3f, VLAN_MIN4f, VLAN_MIN5f};

    soc_field_t max_f[] = {VLAN_MAX0f, VLAN_MAX1f, VLAN_MAX2f,
                           VLAN_MAX3f, VLAN_MAX4f, VLAN_MAX5f};

    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit,gport, &modid, &port, &tgid, &id)); 
    BCM_IF_ERROR_RETURN(
        bcm_er_vlan_range_profile_index_get(unit, port, &old_idx));
    /* Get the profile table entry for this port/trunk */
    bcm_er_vlan_range_profile_entry_get(unit, &profile_entry, old_idx);

    /* Find the first matched min/max range. */
    for (i = 0 ; i < 6 ; i++) {
        min = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, min_f[i]);
        max = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, max_f[i]);
        if ((min == 1) && (max == 0)) {
            break;
        }
    }
    if (i == 6) {
        /* All ranges are taken */
        return BCM_E_NOT_FOUND;
    }

    /* Add a corresponding entry in the vlan translate table for the low VID */
    return bcm_er_vlan_translate_get(unit, port, modid, tgid, old_vid_low,
                                     new_vid, int_prio, xtable);  
}


/*
 * Function:
 *      _bcm_er_vlan_translate_index_to_gport
 * Purpose:
 *      Calculates gport out of vlan range entry index 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_er_vlan_translate_index_to_gport(int unit, int idx, soc_mem_t mem, 
                                      bcm_gport_t *gport)
{
    bcm_module_t    modid;
    bcm_port_t      port;
    bcm_trunk_t     tgid;

    if (NULL == gport) {
        return BCM_E_PARAM;
    }
    if (INVALIDm == mem) {
        return BCM_E_PARAM;
    }

    if (VLAN_RANGE_IDXm == mem) {
        modid = idx / (SOC_PORT_ADDR_MAX(unit) + 1) ; 
        port = idx - modid * (SOC_PORT_ADDR_MAX(unit) + 1);

        if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
            return BCM_E_PARAM;
        }

        if (!SOC_PORT_ADDRESSABLE(unit, port)) {
            return BCM_E_PARAM;
        }

        BCM_GPORT_MODPORT_SET(*gport, modid, port);
    } else {
        tgid = idx;
        BCM_GPORT_TRUNK_SET(*gport, tgid);

    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_er_vlan_translate_range_traverse
 * Purpose:
 *      Traverses over all vlan ranges 
 *      and call given callback function
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_vlan_translate_range_traverse(int unit, 
                                      _bcm_vlan_translate_traverse_t *trvs_st,
                                     int xtable)
{

        /* Indexes to iterate over memories, chunks and entries */
    int                           chnk_idx, ent_idx, chnk_idx_max, mem_idx_max;
    int                             buf_size, chunksize, chnk_end;
    /* Buffer to store chunks of memory table we currently work on */
    uint32                          *tbl_chnk;
    vlan_range_idx_entry_t          *entry;
    soc_mem_t                       mem, mem_array[2] = {VLAN_RANGE_IDXm, TRUNK_VLAN_RANGE_IDXm};
    int                             stop, rv = BCM_E_NONE;
    /* Index to point to table of vlan ranges. */
    int                             range_idx, i, j;
    bcm_gport_t                     gport = 0;
    bcm_port_t                      port;
    bcm_module_t                    modid;
    bcm_trunk_t                     tgid;
    int                             id;
    bcm_vlan_t                      min_vlan, max_vlan;
    igr_vlan_range_tbl_entry_t      profile_entry;
    soc_field_t                     min_f[] = {VLAN_MIN0f, VLAN_MIN1f, 
                                               VLAN_MIN2f, VLAN_MIN3f, 
                                               VLAN_MIN4f, VLAN_MIN5f};
    soc_field_t                     max_f[] = {VLAN_MAX0f, VLAN_MAX1f, 
                                               VLAN_MAX2f, VLAN_MAX3f, 
                                               VLAN_MAX4f, VLAN_MAX5f};

    chunksize = soc_property_get(unit, spn_VLANDELETE_CHUNKS,
                                 VLAN_MEM_CHUNKS_DEFAULT);

    buf_size = 4 * SOC_MAX_MEM_FIELD_WORDS * chunksize;
    tbl_chnk = soc_cm_salloc(unit, buf_size, "vlan translate range traverse");
    if (NULL == tbl_chnk) {
        return BCM_E_MEMORY;
    }

    for (j = 0; j < COUNTOF(mem_array); j++) {
        mem = mem_array[j];
        if (!soc_mem_index_count(unit, mem)) {
            return BCM_E_NONE;
        }

        mem_idx_max = soc_mem_index_max(unit, mem);
        for (chnk_idx = soc_mem_index_min(unit, mem); 
             chnk_idx <= mem_idx_max; 
             chnk_idx += chunksize) {
            sal_memset((void *)tbl_chnk, 0, buf_size);

            chnk_idx_max = 
                ((chnk_idx + chunksize) <= mem_idx_max) ? 
                chnk_idx + chunksize : mem_idx_max;

            rv = soc_mem_read_range(unit, mem, MEM_BLOCK_ANY,
                                    chnk_idx, chnk_idx_max, tbl_chnk);
            if (SOC_FAILURE(rv)) {
                break;
            }
            chnk_end = (chnk_idx_max - chnk_idx);
            for (ent_idx = 0 ; ent_idx < chnk_end; ent_idx ++) {
                entry = 
                    soc_mem_table_idx_to_pointer(unit, mem, 
                                                 vlan_range_idx_entry_t *, 
                                                 tbl_chnk, ent_idx);
                range_idx = soc_mem_field32_get(unit, TRUNK_VLAN_RANGE_IDXm,
                                                entry, VLAN_RANGE_INDEXf);
                bcm_er_vlan_range_profile_entry_get(unit, &profile_entry, 
                                                    range_idx);
                /* Find the first matched min/max range. */
                for (i = 0 ; i < COUNTOF(min_f) ; i++) {
                    min_vlan = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                                   &profile_entry, min_f[i]);
                    max_vlan = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                                   &profile_entry, max_f[i]);
                    if ((min_vlan == 1) && (max_vlan == 0)) {
                        continue;
                    }
                    _bcm_er_vlan_translate_index_to_gport(unit, 
                                            (chnk_idx * chunksize) + ent_idx,
                                                          mem, &gport);
                    rv = _bcm_esw_gport_resolve(unit,gport, &modid, &port, &tgid, &id); 
                    if (BCM_FAILURE(rv)) {
                        break;
                    }
                    rv = bcm_er_vlan_translate_get(unit, port, modid, tgid,
                                                   min_vlan, 
                                                   &(trvs_st->action->new_outer_vlan), 
                                                   &(trvs_st->action->priority), xtable);
                    if (BCM_FAILURE(rv)) {
                        break;
                    }
                    trvs_st->outer_vlan = min_vlan;
                    trvs_st->outer_vlan_high = max_vlan;
                    trvs_st->gport =  gport;
                    rv = trvs_st->int_cb(unit, trvs_st, &stop);
                    if (BCM_FAILURE(rv)) {
                        break;
                    }
                    if (TRUE == stop) {
                        break;
                    }
                }
            }
        }
    }
    soc_cm_sfree(unit, tbl_chnk);
    return rv;        
}

/*
 * Function:
 *      bcm_er_vlan_translate_range_delete
 * Purpose:
 *      Delete a vlan range along with the vlan tranlation entry
 *      in the specified vlan translate table.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_vlan_translate_range_delete(int unit, bcm_gport_t gport,
                                   bcm_vlan_t old_vid_low,
                                   bcm_vlan_t old_vid_high,
                                   int xtable)
{
    int rv = BCM_E_UNAVAIL;
    int old_idx, new_idx, min, max, i;
    vlan_range_idx_entry_t index_entry;
    trunk_vlan_range_idx_entry_t trunk_index_entry;
    int tid, id;
    bcm_port_t  port;
    int modid=0, mod_port=0;
    igr_vlan_range_tbl_entry_t profile_entry;
    soc_field_t min_f[] = {VLAN_MIN0f, VLAN_MIN1f, VLAN_MIN2f,
                           VLAN_MIN3f, VLAN_MIN4f, VLAN_MIN5f};
    soc_field_t max_f[] = {VLAN_MAX0f, VLAN_MAX1f, VLAN_MAX2f, 
                           VLAN_MAX3f, VLAN_MAX4f, VLAN_MAX5f};

    /* Get the current profile table index for this port/trunk */
    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, gport, &modid, &port, &tid, &id));

    if (-1 != id) {
        return BCM_E_PARAM;
    }
    if (-1 != tid) {
        SOC_IF_ERROR_RETURN
            (READ_TRUNK_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, tid,
                                        &trunk_index_entry));
        old_idx = soc_mem_field32_get(unit, TRUNK_VLAN_RANGE_IDXm,
                                      &trunk_index_entry, VLAN_RANGE_INDEXf);
    } else {
        /* The port value that was passed in contains modid + port.
         * Use this to calculate the index to the VLAN_RANGE_IDX table.
         */
        mod_port = (modid << 5);
        mod_port |= (port & BCM_VLAN_TRANSLATE_PORTMOD_PORT_MASK);
        SOC_IF_ERROR_RETURN
            (READ_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, mod_port,
                                  &index_entry));
        old_idx = soc_mem_field32_get(unit, VLAN_RANGE_IDXm,
                                      &index_entry, VLAN_RANGE_INDEXf);
    }

    /* Get the profile table entry for this port/trunk */
    bcm_er_vlan_range_profile_entry_get(unit, &profile_entry, old_idx);
    
    /* Find the min/max range. */
    for (i = 0 ; i < 6 ; i++) {
        min = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, min_f[i]);
        max = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, max_f[i]);
        if ((min == old_vid_low) && (max == old_vid_high)) {
            break;
        }
    }
    if (i == 6) {
        return BCM_E_NOT_FOUND;
    }

    /* Remove the range from the table entry and fill in the gap */
    for ( ; i < 5 ; i++) {
        min = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, min_f[i + 1]);
        max = soc_mem_field32_get(unit, IGR_VLAN_RANGE_TBLm,
                                  &profile_entry, max_f[i + 1]);
 
        /* Move existing min/max UP */
        soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                            &profile_entry, min_f[i], min);
        soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                            &profile_entry, max_f[i], max);
    }
    /* Mark last min/max range as unused. Unused ranges are
     * identified by { min == 1, max == 0 }
     */ 
    soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                        &profile_entry, min_f[i], 1);
    soc_mem_field32_set(unit, IGR_VLAN_RANGE_TBLm,
                        &profile_entry, max_f[i], 0);
    
    /* Try adding the new profile table entry */
    rv = bcm_er_vlan_range_profile_entry_add(unit, &profile_entry,
                                         &new_idx);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    /* Delete the entry from the vlan translate table for the low VID */
    rv = bcm_er_vlan_translate_delete(unit, port, modid, tid, old_vid_low,
                                      xtable);

    /* Update the profile pointer */
    if (port & BCM_TGID_TRUNK_INDICATOR(unit)) {
        soc_mem_field32_set(unit, TRUNK_VLAN_RANGE_IDXm,
                            &trunk_index_entry, VLAN_RANGE_INDEXf, new_idx);
        WRITE_TRUNK_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, tid,
                                    &trunk_index_entry);
    } else {
        soc_mem_field32_set(unit, VLAN_RANGE_IDXm,
                            &index_entry, VLAN_RANGE_INDEXf, new_idx);
        WRITE_VLAN_RANGE_IDXm(unit, MEM_BLOCK_ANY, mod_port, &index_entry);
    }

    /* Delete the old profile entry */
    bcm_er_vlan_range_profile_entry_delete(unit, old_idx);

    return rv;
}

/*
 * Function:
 *      bcm_er_vlan_translate_range_delete_all
 * Purpose:
 *      Delete all vlan ranges along with the vlan tranlation entries
 *      in the specified vlan translate table.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_vlan_translate_range_delete_all(int unit, int xtable)
{
    int rv;

    rv = bcm_er_vlan_range_profile_init(unit);
    if (rv != BCM_E_NONE) {
        rv = bcm_er_vlan_translate_delete_all(unit, xtable);
    }
    return rv;
}

/*
 * Function :
 *    _bcm_er_vlan_control_vlan_set
 * 
 * Purpose  :
 *    Set vlan specific traffic forwarding parameters
 *
 * Parameters:
 *     unit    - (IN) BCM device number.
 *     vid     - (IN) Vlan id. 
 *     control - (OUT) Configuration.
 *
 * Return :
 *    BCM_E_XXX
 */
int
_bcm_er_vlan_control_vlan_set(int unit, bcm_vlan_t vid,
                                bcm_vlan_control_vlan_t *control)
{
    int rv = BCM_E_UNAVAIL;
#if defined(INCLUDE_L3)
    bcm_vlan_control_vlan_t temp;
    uint32                  flags;

    bcm_vlan_control_vlan_t_init(&temp);

    flags = control->flags & BCM_VLAN_MPLS_DISABLE;
    control->flags &= ~BCM_VLAN_MPLS_DISABLE;

    if (0 != sal_memcmp(&temp, control, sizeof(bcm_vlan_control_vlan_t))) {
        return (BCM_E_UNAVAIL);
    }

    if (0 == soc_feature(unit, soc_feature_mpls_per_vlan)) {
        return (BCM_E_UNAVAIL);
    }

    rv = _bcm_mpls_vlan_enable_set(unit, vid, (flags) ? 0 : 1);
#endif /* INCLUDE_L3 */
    return  rv;
}

#else  /* BCM_EASYRIDER_SUPPORT */
int bcm_esw_easyrider_vlan_not_empty;
#endif /* BCM_EASYRIDER_SUPPORT */
