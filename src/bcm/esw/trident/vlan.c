/*
 * $Id: vlan.c 1.14.4.4 Broadcom SDK $
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
 * File:    vlan.c
 * Purpose: Manages VLAN virtual port creation and deletion.
 *          Also manages addition and removal of virtual
 *          ports from VLAN. The types of virtual ports that
 *          can be added to or remove from VLAN can be VLAN
 *          VPs or NIV VPs.
 */

#include <soc/defs.h>
#include <sal/core/libc.h>

#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3) 

#include <soc/mem.h>
#include <soc/hash.h>

#include <bcm/error.h>

#include <bcm_int/esw_dispatch.h>
#include <bcm_int/api_xlate_port.h>

#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/multicast.h>
#include <bcm_int/esw/virtual.h>
#include <bcm_int/esw/vlan.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/triumph2.h>
#include <bcm_int/esw/trident.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/trunk.h>

/* -------------------------------------------------------
 * Software book keeping for VLAN virtual port information
 * -------------------------------------------------------
 */

typedef struct _bcm_trident_vlan_vp_info_s {
    bcm_vlan_port_match_t criteria;
    uint32 flags;
    bcm_vlan_t match_vlan;
    bcm_vlan_t match_inner_vlan;
    bcm_gport_t port;
} _bcm_trident_vlan_vp_info_t;

typedef struct _bcm_trident_vlan_virtual_bookkeeping_s {
    int vlan_virtual_initialized; /* Flag to check initialized status */
    sal_mutex_t vlan_virtual_mutex; /* VLAN virtual module lock */
    _bcm_trident_vlan_vp_info_t *port_info; /* VP state */
} _bcm_trident_vlan_virtual_bookkeeping_t;

STATIC _bcm_trident_vlan_virtual_bookkeeping_t _bcm_trident_vlan_virtual_bk_info[BCM_MAX_NUM_UNITS];

#define VLAN_VIRTUAL_INFO(unit) (&_bcm_trident_vlan_virtual_bk_info[unit])

#define VLAN_VIRTUAL_INIT(unit)                                   \
    do {                                                          \
        if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {          \
            return BCM_E_UNIT;                                    \
        }                                                         \
        if (!VLAN_VIRTUAL_INFO(unit)->vlan_virtual_initialized) { \
            return BCM_E_INIT;                                    \
        }                                                         \
    } while (0)

#define VLAN_VIRTUAL_LOCK(unit) \
        sal_mutex_take(VLAN_VIRTUAL_INFO(unit)->vlan_virtual_mutex, sal_mutex_FOREVER);

#define VLAN_VIRTUAL_UNLOCK(unit) \
        sal_mutex_give(VLAN_VIRTUAL_INFO(unit)->vlan_virtual_mutex);

#define VLAN_VP_INFO(unit, vp) (&VLAN_VIRTUAL_INFO(unit)->port_info[vp])

/* --------------------------------------------------------
 * Software book keeping for virtual port group information
 * --------------------------------------------------------
 */

typedef struct _bcm_td_vp_group_s {
    int vp_count;            /* Number of VPs that belong to this VP group */
    SHR_BITDCL *vp_bitmap;   /* Bitmap of VPs that belong to this VP group */
    SHR_BITDCL *vlan_bitmap; /* VLANs this VP group belongs to */
} _bcm_td_vp_group_t;

typedef struct _bcm_td_vp_group_bk_s {
    int vp_group_initialized; /* Flag to check initialized status */
    int num_ing_vp_group;     /* Number of ingress VP groups */
    _bcm_td_vp_group_t *ing_vp_group_array; /* Ingress VP group array */
    int num_eg_vp_group;      /* Number of egress VP groups */
    _bcm_td_vp_group_t *eg_vp_group_array; /* Egress VP group array */
} _bcm_td_vp_group_bk_t;

STATIC _bcm_td_vp_group_bk_t _bcm_td_vp_group_bk[BCM_MAX_NUM_UNITS];

#define VP_GROUP_BK(unit) (&_bcm_td_vp_group_bk[unit])

#define VP_GROUP_INIT(unit)                              \
    do {                                                 \
        if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) { \
            return BCM_E_UNIT;                           \
        }                                                \
        if (!VP_GROUP_BK(unit)->vp_group_initialized) {  \
            return BCM_E_INIT;                           \
        }                                                \
    } while (0)

#define ING_VP_GROUP(unit, vp_group) \
    (&VP_GROUP_BK(unit)->ing_vp_group_array[vp_group])

#define EG_VP_GROUP(unit, vp_group) \
    (&VP_GROUP_BK(unit)->eg_vp_group_array[vp_group])

/* ---------------------------------------------------------------------------- 
 *
 * Routines for creation and deletion of VLAN virtual ports
 *
 * ---------------------------------------------------------------------------- 
 */

/*
 * Function:
 *      _bcm_trident_vlan_vp_port_cnt_update
 * Purpose:
 *      Update port's VP count.
 * Parameters:
 *      unit  - (IN) SOC unit number. 
 *      gport - (IN) GPORT ID.
 *      vp    - (IN) Virtual port number.
 *      incr  - (IN) If TRUE, increment VP count, else decrease VP count.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_port_cnt_update(int unit, bcm_gport_t gport,
        int vp, int incr)
{
    int mod_out, port_out, tgid_out, id_out;
    bcm_trunk_add_info_t tinfo;
    int idx;
    int mapped_mod, mapped_port;
    int mod_local;
    _bcm_port_info_t *port_info;

    BCM_IF_ERROR_RETURN
        (_bcm_esw_gport_resolve(unit, gport, &mod_out, 
                                &port_out, &tgid_out, &id_out));
    if (-1 != id_out) {
        return BCM_E_PARAM;
    }

    /* Update the physical port's SW state. If associated with a trunk,
     * update each local physical port's SW state.
     */

    if (BCM_TRUNK_INVALID != tgid_out) {

        BCM_IF_ERROR_RETURN(bcm_esw_trunk_get(unit, tgid_out, &tinfo));

        for (idx = 0; idx < tinfo.num_ports; idx++) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                        tinfo.tm[idx], tinfo.tp[idx],
                                        &mapped_mod, &mapped_port));

            /* Convert system port to physical port */
            if (soc_feature(unit, soc_feature_sysport_remap)) {
                BCM_XLATE_SYSPORT_S2P(unit, &mapped_port);
            }

            BCM_IF_ERROR_RETURN
                (_bcm_esw_modid_is_local(unit, mapped_mod, &mod_local));
            if (mod_local) {
                _bcm_port_info_access(unit, mapped_port, &port_info);
                if (incr) {
                    port_info->vp_count++;
                } else {
                    port_info->vp_count--;
                }
            }
        }
    } else {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_modid_is_local(unit, mod_out, &mod_local));
        if (mod_local) {
            if (soc_feature(unit, soc_feature_sysport_remap)) { 
                BCM_XLATE_SYSPORT_S2P(unit, &port_out); 
            }
            _bcm_port_info_access(unit, port_out, &port_info);
            if (incr) {
                port_info->vp_count++;
            } else {
                port_info->vp_count--;
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trident_vlan_virtual_free_resources
 * Purpose:
 *      Free all allocated tables and memory
 * Parameters:
 *      unit - SOC unit number
 * Returns:
 *      Nothing
 */
STATIC void
_bcm_trident_vlan_virtual_free_resources(int unit)
{
    if (VLAN_VIRTUAL_INFO(unit)->port_info) {
        sal_free(VLAN_VIRTUAL_INFO(unit)->port_info);
        VLAN_VIRTUAL_INFO(unit)->port_info = NULL;
    }
    if (VLAN_VIRTUAL_INFO(unit)->vlan_virtual_mutex) {
        sal_mutex_destroy(VLAN_VIRTUAL_INFO(unit)->vlan_virtual_mutex);
        VLAN_VIRTUAL_INFO(unit)->vlan_virtual_mutex = NULL;
    } 
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      bcm_trident_vlan_virtual_reinit
 * Purpose:
 *      Warm boot recovery for the VLAN virtual software module
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_trident_vlan_virtual_reinit(int unit)
{
    int rv = BCM_E_NONE;
    int stable_size;
    uint8 *vlan_xlate_buf = NULL;
    vlan_xlate_entry_t *vt_entry;
    int i, index_min, index_max;
    uint32 key_type, vp, nh_index, trunk_bit;
    source_vp_entry_t svp_entry;
    ing_dvp_table_entry_t dvp_entry;
    ing_l3_next_hop_entry_t ing_nh_entry;
    wlan_svp_table_entry_t wlan_svp_entry;
    bcm_trunk_t tgid;
    bcm_module_t modid, mod_out;
    bcm_port_t port_num, port_out;
    uint32 profile_idx;
    ing_vlan_tag_action_profile_entry_t ing_profile_entry;

    SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));

    /* Recover VLAN virtual ports from VLAN_XLATE table */

    vlan_xlate_buf = soc_cm_salloc(unit,
            SOC_MEM_TABLE_BYTES(unit, VLAN_XLATEm), "VLAN_XLATE buffer");
    if (NULL == vlan_xlate_buf) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }

    index_min = soc_mem_index_min(unit, VLAN_XLATEm);
    index_max = soc_mem_index_max(unit, VLAN_XLATEm);
    rv = soc_mem_read_range(unit, VLAN_XLATEm, MEM_BLOCK_ANY,
            index_min, index_max, vlan_xlate_buf);
    if (SOC_FAILURE(rv)) {
        goto cleanup;
    }

    for (i = index_min; i <= index_max; i++) {
        vt_entry = soc_mem_table_idx_to_pointer
            (unit, VLAN_XLATEm, vlan_xlate_entry_t *, 
             vlan_xlate_buf, i);

        if (soc_VLAN_XLATEm_field32_get(unit, vt_entry, VALIDf) == 0) {
            continue;
        }

        key_type = soc_VLAN_XLATEm_field32_get(unit, vt_entry, KEY_TYPEf);
        if ((key_type != TR_VLXLT_HASH_KEY_TYPE_OVID) &&
                (key_type != TR_VLXLT_HASH_KEY_TYPE_IVID_OVID) &&
                (key_type != TR_VLXLT_HASH_KEY_TYPE_OTAG)) {
            continue;
        }

        if (soc_VLAN_XLATEm_field32_get(unit, vt_entry, MPLS_ACTIONf) != 1) {
            continue;
        }

        vp = soc_VLAN_XLATEm_field32_get(unit, vt_entry, SOURCE_VPf);
        if ((stable_size == 0) || SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit)) {
            /* Determine if VP is a VLAN VP by process of elimination.
             * Can rule out NIV VP since NIV VPs have different VIF key
             * types in VLAN_XLATEm.
             */

            rv = READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp_entry);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }
            /* Rule out MPLS and MiM VPs, for which ENTRY_TYPE = 1 */
            if (soc_SOURCE_VPm_field32_get(unit, &svp_entry, ENTRY_TYPEf) != 3) {
                continue;
            }

            rv = READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp_entry);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }
            /* Rule out Trill VPs, for which VP_TYPE = 1 */
            if (soc_ING_DVP_TABLEm_field32_get(unit, &dvp_entry, VP_TYPEf) != 0) {
                continue;
            }

            nh_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp_entry,
                    NEXT_HOP_INDEXf);
            rv = READ_ING_L3_NEXT_HOPm(unit, MEM_BLOCK_ANY, nh_index,
                    &ing_nh_entry);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }
            /* Rule out Subport VPs, for which ENTRY_TYPE = 3 */
            if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh_entry,
                        ENTRY_TYPEf) != 2) {
                continue;
            }

            /* Rule out WLAN vp */
            if (SOC_MEM_IS_VALID(unit, WLAN_SVP_TABLEm)) {
                if (vp > soc_mem_index_max(unit, WLAN_SVP_TABLEm)) {
                    continue;
                }
                rv = READ_WLAN_SVP_TABLEm(unit, MEM_BLOCK_ANY, vp,
                    &wlan_svp_entry);
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }
                if (soc_WLAN_SVP_TABLEm_field32_get(unit, &wlan_svp_entry,
                            VALIDf) == 1) { 
                    continue;
                }
            }

            /* At this point, we are sure VP is a VLAN VP. */

            rv = _bcm_vp_used_set(unit, vp, _bcmVpTypeVlan);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }
        } else {
            if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
                /* VP bitmap is recovered by virtual_init */
                continue;
            }
        }

        /* Recover VLAN_VP_INFO(unit, vp)->criteria, match_vlan, and
         * match_inner_vlan.
         */
        switch (key_type) {
            case TR_VLXLT_HASH_KEY_TYPE_OVID:
                VLAN_VP_INFO(unit, vp)->criteria =
                    BCM_VLAN_PORT_MATCH_PORT_VLAN;
                VLAN_VP_INFO(unit, vp)->match_vlan = 
                    soc_VLAN_XLATEm_field32_get(unit, vt_entry, OVIDf);
                break;
            case TR_VLXLT_HASH_KEY_TYPE_IVID_OVID:
                VLAN_VP_INFO(unit, vp)->criteria =
                    BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED;
                VLAN_VP_INFO(unit, vp)->match_vlan = 
                    soc_VLAN_XLATEm_field32_get(unit, vt_entry, OVIDf);
                VLAN_VP_INFO(unit, vp)->match_inner_vlan = 
                    soc_VLAN_XLATEm_field32_get(unit, vt_entry, IVIDf);
                break;
            case TR_VLXLT_HASH_KEY_TYPE_OTAG:
                VLAN_VP_INFO(unit, vp)->criteria =
                    BCM_VLAN_PORT_MATCH_PORT_VLAN16;
                VLAN_VP_INFO(unit, vp)->match_vlan = 
                    soc_VLAN_XLATEm_field32_get(unit, vt_entry, OTAGf);
                break;
            /* coverity[dead_error_begin] */
            default:
                continue;
        }

        /* Recover VLAN_VP_INFO(unit, vp)->port */
        trunk_bit = soc_VLAN_XLATEm_field32_get(unit, vt_entry, Tf);
        if (trunk_bit) {
            tgid = soc_VLAN_XLATEm_field32_get(unit, vt_entry, TGIDf);
            BCM_GPORT_TRUNK_SET(VLAN_VP_INFO(unit, vp)->port, tgid);
        } else {
            modid = soc_VLAN_XLATEm_field32_get(unit, vt_entry, MODULE_IDf);
            port_num = soc_VLAN_XLATEm_field32_get(unit, vt_entry, PORT_NUMf);
            rv = _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                    modid, port_num, &mod_out, &port_out);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }
            BCM_GPORT_MODPORT_SET(VLAN_VP_INFO(unit, vp)->port,
                    mod_out, port_out);
        }

        /* Recover VLAN_VP_INFO(unit, vp)->flags */
        profile_idx = soc_VLAN_XLATEm_field32_get(unit, vt_entry,
                                                  TAG_ACTION_PROFILE_PTRf); 
        rv = READ_ING_VLAN_TAG_ACTION_PROFILEm(unit, MEM_BLOCK_ANY,
                profile_idx, &ing_profile_entry);
        if (BCM_FAILURE(rv)) {
            goto cleanup;
        }
        if ((soc_ING_VLAN_TAG_ACTION_PROFILEm_field32_get(unit,
                    &ing_profile_entry, DT_OTAG_ACTIONf) ==
                    bcmVlanActionReplace) &&
            (soc_ING_VLAN_TAG_ACTION_PROFILEm_field32_get(unit,
                    &ing_profile_entry, DT_ITAG_ACTIONf) ==
                    bcmVlanActionNone)) {
            VLAN_VP_INFO(unit, vp)->flags |= BCM_VLAN_PORT_INNER_VLAN_PRESERVE;
        }
        if (VLAN_VP_INFO(unit, vp)->criteria ==
                BCM_VLAN_PORT_MATCH_PORT_VLAN16) {
            VLAN_VP_INFO(unit, vp)->flags |= BCM_VLAN_PORT_EGRESS_VLAN16;
        }

        if (stable_size == 0) {
            /* In the Port module, a port's VP count is not recovered in 
             * level 1 Warm Boot.
             */
            rv = _bcm_trident_vlan_vp_port_cnt_update(unit,
                    VLAN_VP_INFO(unit, vp)->port, vp, TRUE);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }
        }
    }

cleanup:
    if (vlan_xlate_buf) {
        soc_cm_sfree(unit, vlan_xlate_buf);
    }

    if (BCM_FAILURE(rv)) {
        _bcm_trident_vlan_virtual_free_resources(unit);
    }

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_trident_vlan_vp_sw_dump
 * Purpose:
 *     Displays VLAN VP information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
bcm_trident_vlan_vp_sw_dump(int unit)
{
    int i, num_vp;

    soc_cm_print("\nSW Information VLAN VP - Unit %d\n", unit);

    num_vp = soc_mem_index_count(unit, SOURCE_VPm);

    for (i = 0; i < num_vp; i++) {
        if (VLAN_VP_INFO(unit, i)->port == 0) {
            continue;
        }
        soc_cm_print("\n  VLAN vp = %d\n", i);
        soc_cm_print("  Criteria = 0x%x,", VLAN_VP_INFO(unit, i)->criteria);
        switch (VLAN_VP_INFO(unit, i)->criteria) {
            case BCM_VLAN_PORT_MATCH_PORT_VLAN:
                soc_cm_print(" port plus outer VLAN ID\n");
                break;
            case BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED:
                soc_cm_print(" port plus outer and inner VLAN IDs\n");
                break;
            case BCM_VLAN_PORT_MATCH_PORT_VLAN16:
                soc_cm_print(" port plus outer VLAN tag\n");
                break;
            default:
                soc_cm_print(" \n");
        }
        soc_cm_print("  Flags = 0x%x\n", VLAN_VP_INFO(unit, i)->flags);
        soc_cm_print("  Match VLAN = 0x%x\n", VLAN_VP_INFO(unit, i)->match_vlan);
        soc_cm_print("  Match Inner VLAN = 0x%x\n",
                VLAN_VP_INFO(unit, i)->match_inner_vlan);
        soc_cm_print("  Port = 0x%x\n", VLAN_VP_INFO(unit, i)->port);
    }

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

/*
 * Function:
 *      bcm_trident_vlan_virtual_init
 * Purpose:
 *      Initialize the VLAN virtual port module.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_trident_vlan_virtual_init(int unit)
{
    int num_vp;
    int rv = BCM_E_NONE;

    sal_memset(VLAN_VIRTUAL_INFO(unit), 0,
            sizeof(_bcm_trident_vlan_virtual_bookkeeping_t));

    num_vp = soc_mem_index_count(unit, SOURCE_VPm);
    if (NULL == VLAN_VIRTUAL_INFO(unit)->port_info) {
        VLAN_VIRTUAL_INFO(unit)->port_info =
            sal_alloc(sizeof(_bcm_trident_vlan_vp_info_t) * num_vp, "vlan_vp_info");
        if (NULL == VLAN_VIRTUAL_INFO(unit)->port_info) {
            _bcm_trident_vlan_virtual_free_resources(unit);
            return BCM_E_MEMORY;
        }
    }
    sal_memset(VLAN_VIRTUAL_INFO(unit)->port_info, 0,
            sizeof(_bcm_trident_vlan_vp_info_t) * num_vp);

    if (NULL == VLAN_VIRTUAL_INFO(unit)->vlan_virtual_mutex) {
        VLAN_VIRTUAL_INFO(unit)->vlan_virtual_mutex =
            sal_mutex_create("vlan virtual mutex");
        if (NULL == VLAN_VIRTUAL_INFO(unit)->vlan_virtual_mutex) {
            _bcm_trident_vlan_virtual_free_resources(unit);
            return BCM_E_MEMORY;
        }
    }

    VLAN_VIRTUAL_INFO(unit)->vlan_virtual_initialized = 1;

    /* Warm boot recovery of VLAN_VP_INFO depends on the completion
     * of _bcm_virtual_init, but _bcm_virtual_init is after the 
     * VLAN module in the init sequence. Hence, warm boot recovery
     * of VLAN_VP_INFO is moved to end of _bcm_virtual_init.
     */

    return rv;
}

/*
 * Function:
 *      bcm_trident_vlan_virtual_detach
 * Purpose:
 *      Detach the VLAN virtual port module.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_trident_vlan_virtual_detach(int unit)
{
    _bcm_trident_vlan_virtual_free_resources(unit);

    VLAN_VIRTUAL_INFO(unit)->vlan_virtual_initialized = 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_nh_info_set
 * Purpose:
 *      Get a next hop index and configure next hop tables.
 * Parameters:
 *      unit       - (IN) SOC unit number. 
 *      vlan_vp    - (IN) Pointer to VLAN virtual port structure. 
 *      vp         - (IN) Virtual port number. 
 *      drop       - (IN) Drop indication. 
 *      nh_index   - (IN/OUT) Next hop index. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_nh_info_set(int unit, bcm_vlan_port_t *vlan_vp, int vp,
        int drop, int *nh_index)
{
    int rv;
    uint32 nh_flags;
    bcm_l3_egress_t nh_info;
    egr_l3_next_hop_entry_t egr_nh;
    uint8 egr_nh_entry_type;
    bcm_module_t mod_out;
    bcm_port_t port_out;
    bcm_trunk_t trunk_id;
    int id;
    int ing_nh_port;
    int ing_nh_module;
    int ing_nh_trunk;
    ing_l3_next_hop_entry_t ing_nh;
    initial_ing_l3_next_hop_entry_t initial_ing_nh;

    /* Get a next hop index */

    if (vlan_vp->flags & BCM_VLAN_PORT_REPLACE) {
        if ((*nh_index > soc_mem_index_max(unit, EGR_L3_NEXT_HOPm)) ||
                (*nh_index < soc_mem_index_min(unit, EGR_L3_NEXT_HOPm)))  {
            return BCM_E_PARAM;
        }
    } else {
        /*
         * Allocate a next-hop entry. By calling bcm_xgs3_nh_add()
         * with _BCM_L3_SHR_WRITE_DISABLE flag, a next-hop index is
         * allocated but nothing is written to hardware. The "nh_info"
         * in this case is not used, so just set to all zeros.
         */
         bcm_l3_egress_t_init(&nh_info);
/*        sal_memset(&nh_info, 0, sizeof(bcm_l3_egress_t));*/
        nh_flags = _BCM_L3_SHR_MATCH_DISABLE | _BCM_L3_SHR_WRITE_DISABLE;
        rv = bcm_xgs3_nh_add(unit, nh_flags, &nh_info, nh_index);
        BCM_IF_ERROR_RETURN(rv);
    }

    /* Write EGR_L3_NEXT_HOP entry */

    if (vlan_vp->flags & BCM_VLAN_PORT_REPLACE) {
        /* Read the existing egress next_hop entry */
        rv = soc_mem_read(unit, EGR_L3_NEXT_HOPm, MEM_BLOCK_ANY, 
                *nh_index, &egr_nh);
        BCM_IF_ERROR_RETURN(rv);

        /* Be sure that the existing entry is programmed to SD-tag */
        egr_nh_entry_type = 
            soc_EGR_L3_NEXT_HOPm_field32_get(unit, &egr_nh, ENTRY_TYPEf);
        if (egr_nh_entry_type != 0x2) { /* != SD-tag */
            return BCM_E_PARAM;
        }
    } else {
        egr_nh_entry_type = 0x2; /* SD-tag */
    }

    sal_memset(&egr_nh, 0, sizeof(egr_l3_next_hop_entry_t));
    soc_mem_field32_set(unit, EGR_L3_NEXT_HOPm, &egr_nh,
            ENTRY_TYPEf, egr_nh_entry_type);
    soc_mem_field32_set(unit, EGR_L3_NEXT_HOPm, &egr_nh,
            SD_TAG__DVPf, vp);
    soc_mem_field32_set(unit, EGR_L3_NEXT_HOPm, &egr_nh, 
            SD_TAG__HG_HDR_SELf, 1);
    rv = soc_mem_write(unit, EGR_L3_NEXT_HOPm,
            MEM_BLOCK_ALL, *nh_index, &egr_nh);
    if (rv < 0) {
        goto cleanup;
    }

    /* Resolve gport */

    rv = _bcm_esw_gport_resolve(unit, vlan_vp->port, &mod_out, 
            &port_out, &trunk_id, &id);
    if (rv < 0) {
        goto cleanup;
    }

    ing_nh_port = -1;
    ing_nh_module = -1;
    ing_nh_trunk = -1;

    if (BCM_GPORT_IS_TRUNK(vlan_vp->port)) {
        ing_nh_module = -1;
        ing_nh_port = -1;
        ing_nh_trunk = trunk_id;
    } else {
        ing_nh_module = mod_out;
        ing_nh_port = port_out;
        ing_nh_trunk = -1;
    }

    /* Write ING_L3_NEXT_HOP entry */

    sal_memset(&ing_nh, 0, sizeof(ing_l3_next_hop_entry_t));

    if (ing_nh_trunk == -1) {
        soc_mem_field32_set(unit, ING_L3_NEXT_HOPm,
                &ing_nh, PORT_NUMf, ing_nh_port);
        soc_mem_field32_set(unit, ING_L3_NEXT_HOPm,
                &ing_nh, MODULE_IDf, ing_nh_module);
    } else {    
        soc_mem_field32_set(unit, ING_L3_NEXT_HOPm,
                &ing_nh, Tf, 1);
        soc_mem_field32_set(unit, ING_L3_NEXT_HOPm,
                &ing_nh, TGIDf, ing_nh_trunk);
    }

    soc_mem_field32_set(unit, ING_L3_NEXT_HOPm, &ing_nh, DROPf, drop);
    soc_mem_field32_set(unit, ING_L3_NEXT_HOPm,
            &ing_nh, ENTRY_TYPEf, 0x2); /* L2 DVP */
    soc_mem_field32_set(unit, ING_L3_NEXT_HOPm,
            &ing_nh, MTU_SIZEf, 0x3fff);
    rv = soc_mem_write (unit, ING_L3_NEXT_HOPm,
            MEM_BLOCK_ALL, *nh_index, &ing_nh);
    if (rv < 0) {
        goto cleanup;
    }

    /* Write INITIAL_ING_L3_NEXT_HOP entry */

    sal_memset(&initial_ing_nh, 0, sizeof(initial_ing_l3_next_hop_entry_t));
    if (ing_nh_trunk == -1) {
        soc_mem_field32_set(unit, INITIAL_ING_L3_NEXT_HOPm,
                &initial_ing_nh, PORT_NUMf, ing_nh_port);
        soc_mem_field32_set(unit, INITIAL_ING_L3_NEXT_HOPm,
                &initial_ing_nh, MODULE_IDf, ing_nh_module);
    } else {
        soc_mem_field32_set(unit, INITIAL_ING_L3_NEXT_HOPm,
                &initial_ing_nh, Tf, 1);
        soc_mem_field32_set(unit, INITIAL_ING_L3_NEXT_HOPm,
                &initial_ing_nh, TGIDf, ing_nh_trunk);
    }
    rv = soc_mem_write(unit, INITIAL_ING_L3_NEXT_HOPm,
            MEM_BLOCK_ALL, *nh_index, &initial_ing_nh);
    if (rv < 0) {
        goto cleanup;
    }

    return rv;

cleanup:
    if (!(vlan_vp->flags & BCM_VLAN_PORT_REPLACE)) {
        (void) bcm_xgs3_nh_del(unit, _BCM_L3_SHR_WRITE_DISABLE, *nh_index);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_nh_info_delete
 * Purpose:
 *      Free next hop index and clear next hop tables.
 * Parameters:
 *      unit     - (IN) SOC unit number. 
 *      nh_index - (IN) Next hop index. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_nh_info_delete(int unit, int nh_index)
{
    egr_l3_next_hop_entry_t egr_nh;
    ing_l3_next_hop_entry_t ing_nh;
    initial_ing_l3_next_hop_entry_t initial_ing_nh;

    /* Clear EGR_L3_NEXT_HOP entry */
    sal_memset(&egr_nh, 0, sizeof(egr_l3_next_hop_entry_t));
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, EGR_L3_NEXT_HOPm,
                                   MEM_BLOCK_ALL, nh_index, &egr_nh));

    /* Clear ING_L3_NEXT_HOP entry */
    sal_memset(&ing_nh, 0, sizeof(ing_l3_next_hop_entry_t));
    BCM_IF_ERROR_RETURN(soc_mem_write (unit, ING_L3_NEXT_HOPm,
                                   MEM_BLOCK_ALL, nh_index, &ing_nh));

    /* Clear INITIAL_ING_L3_NEXT_HOP entry */
    sal_memset(&initial_ing_nh, 0, sizeof(initial_ing_l3_next_hop_entry_t));
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, INITIAL_ING_L3_NEXT_HOPm,
                                   MEM_BLOCK_ALL, nh_index, &initial_ing_nh));

    /* Free the next-hop index. */
    BCM_IF_ERROR_RETURN
        (bcm_xgs3_nh_del(unit, _BCM_L3_SHR_WRITE_DISABLE, nh_index));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_match_add
 * Purpose:
 *      Add match criteria for VLAN VP.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vlan_vp - (IN) Pointer to VLAN virtual port structure. 
 *      vp - (IN) Virtual port number.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_match_add(int unit, bcm_vlan_port_t *vlan_vp, int vp)
{
    vlan_xlate_entry_t vent, old_vent;
    int key_type = 0;
    bcm_vlan_action_set_t action;
    uint32 profile_idx;
    int rv;

    if (vlan_vp->criteria == BCM_VLAN_PORT_MATCH_NONE) {
       return BCM_E_NONE;
    }

    if (!((vlan_vp->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN) ||
          (vlan_vp->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED) ||
          (vlan_vp->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN16))) {
        return BCM_E_PARAM;
    }

    if ((vlan_vp->egress_vlan > BCM_VLAN_MAX) ||
        (vlan_vp->egress_inner_vlan > BCM_VLAN_MAX)) {
        return BCM_E_PARAM;
    } 

    sal_memset(&vent, 0, sizeof(vlan_xlate_entry_t));

    soc_VLAN_XLATEm_field32_set(unit, &vent, VALIDf, 1);

    if (vlan_vp->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN) {
        key_type = bcmVlanTranslateKeyPortOuter;
    } else if (vlan_vp->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED) {
        key_type = bcmVlanTranslateKeyPortDouble;
    } else if (vlan_vp->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN16) {
        key_type = bcmVlanTranslateKeyPortOuterTag;
    }
    BCM_IF_ERROR_RETURN
        (_bcm_trx_vlan_translate_entry_assemble(unit, &vent,
                                                vlan_vp->port,
                                                key_type,
                                                vlan_vp->match_inner_vlan,
                                                vlan_vp->match_vlan));

    soc_VLAN_XLATEm_field32_set(unit, &vent, MPLS_ACTIONf, 0x1); /* SVP */
    soc_VLAN_XLATEm_field32_set(unit, &vent, SOURCE_VPf, vp);
    soc_VLAN_XLATEm_field32_set(unit, &vent, NEW_OVIDf,
                                vlan_vp->egress_vlan);
    soc_VLAN_XLATEm_field32_set(unit, &vent, NEW_IVIDf,
                                vlan_vp->egress_inner_vlan);

    bcm_vlan_action_set_t_init(&action);
    if (vlan_vp->flags & BCM_VLAN_PORT_INNER_VLAN_PRESERVE) {
        action.dt_outer = bcmVlanActionReplace;
        action.dt_outer_prio = bcmVlanActionReplace;
        action.dt_inner = bcmVlanActionNone;
        action.dt_inner_prio = bcmVlanActionNone;
    } else {
        action.dt_outer = bcmVlanActionCopy;
        action.dt_outer_prio = bcmVlanActionCopy;
        action.dt_inner = bcmVlanActionDelete;
        action.dt_inner_prio = bcmVlanActionDelete;
    }
    action.ot_outer = bcmVlanActionReplace;
    action.ot_outer_prio = bcmVlanActionReplace;
    action.ot_inner = bcmVlanActionNone;

    BCM_IF_ERROR_RETURN
        (_bcm_trx_vlan_action_profile_entry_add(unit, &action, &profile_idx));

    soc_VLAN_XLATEm_field32_set(unit, &vent, TAG_ACTION_PROFILE_PTRf,
                                profile_idx);

    rv = soc_mem_insert_return_old(unit, VLAN_XLATEm, MEM_BLOCK_ALL,
                                   &vent, &old_vent);
    if (rv == SOC_E_EXISTS) {
        /* Delete the old vlan translate profile entry */
        profile_idx = soc_VLAN_XLATEm_field32_get(unit, &old_vent,
                                                  TAG_ACTION_PROFILE_PTRf);       
        rv = _bcm_trx_vlan_action_profile_entry_delete(unit, profile_idx);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_match_delete
 * Purpose:
 *      Delete match criteria for VLAN VP.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vp - (IN) Virtual port number.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_match_delete(int unit, int vp)
{
    vlan_xlate_entry_t vent, old_vent;
    int key_type = 0;
    uint32 profile_idx;
    int rv;

    if (VLAN_VP_INFO(unit, vp)->criteria == BCM_VLAN_PORT_MATCH_NONE) {
        return BCM_E_NONE;
    }

    if (VLAN_VP_INFO(unit, vp)->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN) {
        key_type = bcmVlanTranslateKeyPortOuter;
    } else if (VLAN_VP_INFO(unit, vp)->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED) {
        key_type = bcmVlanTranslateKeyPortDouble;
    } else if (VLAN_VP_INFO(unit, vp)->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN16) {
        key_type = bcmVlanTranslateKeyPortOuterTag;
    } else {
        return BCM_E_INTERNAL;
    }

    sal_memset(&vent, 0, sizeof(vlan_xlate_entry_t));
    BCM_IF_ERROR_RETURN
        (_bcm_trx_vlan_translate_entry_assemble(unit, &vent,
                                                VLAN_VP_INFO(unit, vp)->port,
                                                key_type,
                                                VLAN_VP_INFO(unit, vp)->match_inner_vlan,
                                                VLAN_VP_INFO(unit, vp)->match_vlan));

    rv = soc_mem_delete_return_old(unit, VLAN_XLATEm, MEM_BLOCK_ALL, &vent, &old_vent);
    if ((rv == SOC_E_NONE) && soc_VLAN_XLATEm_field32_get(unit, &old_vent, VALIDf)) {
        profile_idx = soc_VLAN_XLATEm_field32_get(unit, &old_vent,
                                                  TAG_ACTION_PROFILE_PTRf);       
        /* Delete the old vlan action profile entry */
        rv = _bcm_trx_vlan_action_profile_entry_delete(unit, profile_idx);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_match_get
 * Purpose:
 *      Get match criteria for VLAN VP.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vp - (IN) Virtual port number.
 *      vlan_vp - (OUT) Pointer to VLAN virtual port structure. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_match_get(int unit, int vp, bcm_vlan_port_t *vlan_vp)
{
    vlan_xlate_entry_t vent, vent_out;
    int key_type = 0;
    int idx;

    vlan_vp->criteria = VLAN_VP_INFO(unit, vp)->criteria;
    vlan_vp->match_vlan = VLAN_VP_INFO(unit, vp)->match_vlan;
    vlan_vp->match_inner_vlan = VLAN_VP_INFO(unit, vp)->match_inner_vlan;
    vlan_vp->port = VLAN_VP_INFO(unit, vp)->port;

    if (VLAN_VP_INFO(unit, vp)->criteria == BCM_VLAN_PORT_MATCH_NONE) {
        return BCM_E_NONE;
    }

    if (VLAN_VP_INFO(unit, vp)->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN) {
        key_type = bcmVlanTranslateKeyPortOuter;
    } else if (VLAN_VP_INFO(unit, vp)->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED) {
        key_type = bcmVlanTranslateKeyPortDouble;
    } else if (VLAN_VP_INFO(unit, vp)->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN16) {
        key_type = bcmVlanTranslateKeyPortOuterTag;
    } else {
        return BCM_E_INTERNAL;
    }

    sal_memset(&vent, 0, sizeof(vlan_xlate_entry_t));
    BCM_IF_ERROR_RETURN
        (_bcm_trx_vlan_translate_entry_assemble(unit, &vent,
                                                VLAN_VP_INFO(unit, vp)->port,
                                                key_type,
                                                VLAN_VP_INFO(unit, vp)->match_inner_vlan,
                                                VLAN_VP_INFO(unit, vp)->match_vlan));

    BCM_IF_ERROR_RETURN
        (soc_mem_search(unit, VLAN_XLATEm, MEM_BLOCK_ALL,
                        &idx, &vent, &vent_out, 0));
    vlan_vp->egress_vlan = soc_VLAN_XLATEm_field32_get(unit, &vent_out,
                                                       NEW_OVIDf);
    vlan_vp->egress_inner_vlan = soc_VLAN_XLATEm_field32_get(unit, &vent_out,
                                                       NEW_IVIDf);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_create
 * Purpose:
 *      Create a VLAN virtual port.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vlan_vp - (IN/OUT) Pointer to VLAN virtual port structure. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_create(int unit,
                         bcm_vlan_port_t *vlan_vp)
{
    int mode;
    int vp;
    int rv = BCM_E_NONE;
    int num_vp;
    int nh_index = 0;
    ing_dvp_table_entry_t dvp_entry;
    source_vp_entry_t svp_entry;
    int cml_default_enable=0, cml_default_new=0, cml_default_move=0;

    BCM_IF_ERROR_RETURN(bcm_xgs3_l3_egress_mode_get(unit, &mode));
    if (!mode) {
        soc_cm_debug(DK_L3, "L3 egress mode must be set first\n");
        return BCM_E_DISABLED;
    }

    if (!(vlan_vp->flags & BCM_VLAN_PORT_REPLACE)) { /* Create new VLAN VP */

        if (vlan_vp->flags & BCM_VLAN_PORT_WITH_ID) {
            if (!BCM_GPORT_IS_VLAN_PORT(vlan_vp->vlan_port_id)) {
                return BCM_E_PARAM;
            }
            vp = BCM_GPORT_VLAN_PORT_ID_GET(vlan_vp->vlan_port_id);

            if (_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
                return BCM_E_EXISTS;
            } else {
                rv = _bcm_vp_used_set(unit, vp, _bcmVpTypeVlan);
                if (rv < 0) {
                    return rv;
                }
            }
        } else {
            /* allocate a new VP index */
            num_vp = soc_mem_index_count(unit, SOURCE_VPm);
            rv = _bcm_vp_alloc(unit, 0, (num_vp - 1), 1, SOURCE_VPm,
                    _bcmVpTypeVlan, &vp);
            if (rv < 0) {
                return rv;
            }
        }

        /* Configure next hop tables */
        rv = _bcm_trident_vlan_vp_nh_info_set(unit, vlan_vp, vp, 0,
                                              &nh_index);
        if (rv < 0) {
            goto cleanup;
        }

        /* Configure DVP table */
        sal_memset(&dvp_entry, 0, sizeof(ing_dvp_table_entry_t));
        soc_ING_DVP_TABLEm_field32_set(unit, &dvp_entry, NEXT_HOP_INDEXf,
                                       nh_index);
        rv = WRITE_ING_DVP_TABLEm(unit, MEM_BLOCK_ALL, vp, &dvp_entry);
        if (rv < 0) {
            goto cleanup;
        }

        /* Configure SVP table */
        sal_memset(&svp_entry, 0, sizeof(source_vp_entry_t));
        soc_SOURCE_VPm_field32_set(unit, &svp_entry, ENTRY_TYPEf, 3);


        /* Set the CML */
        rv = _bcm_vp_default_cml_mode_get (unit, 
                           &cml_default_enable, &cml_default_new, 
                           &cml_default_move);
         if (rv < 0) {
             goto cleanup;
         }

        if (cml_default_enable) {
            /* Set the CML to default values */
            soc_SOURCE_VPm_field32_set(unit, &svp_entry, CML_FLAGS_NEWf, cml_default_new);
            soc_SOURCE_VPm_field32_set(unit, &svp_entry, CML_FLAGS_MOVEf, cml_default_move);
        } else {
            /* Set the CML to PVP_CML_SWITCH by default (hw learn and forward) */
            soc_SOURCE_VPm_field32_set(unit, &svp_entry, CML_FLAGS_NEWf, 0x8);
            soc_SOURCE_VPm_field32_set(unit, &svp_entry, CML_FLAGS_MOVEf, 0x8);
        }


        rv = WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp_entry);
        if (rv < 0) {
            goto cleanup;
        }

        /* Configure ingress VLAN translation table */
        rv = _bcm_trident_vlan_vp_match_add(unit, vlan_vp, vp);
        if (rv < 0) {
            goto cleanup;
        }

        /* Increment port's VP count */
        rv = _bcm_trident_vlan_vp_port_cnt_update(unit, vlan_vp->port, vp, TRUE);
        if (rv < 0) {
            goto cleanup;
        }

    } else { /* Replace properties of existing VLAN VP */

        if (!(vlan_vp->flags & BCM_VLAN_PORT_WITH_ID)) {
            return BCM_E_PARAM;
        }
        if (!BCM_GPORT_IS_VLAN_PORT(vlan_vp->vlan_port_id)) {
            return BCM_E_PARAM;
        }
        vp = BCM_GPORT_VLAN_PORT_ID_GET(vlan_vp->vlan_port_id);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
            return BCM_E_PARAM;
        }

        /* For existing vlan vp, NH entry already exists */
        BCM_IF_ERROR_RETURN
            (READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp_entry));

        nh_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp_entry,
                NEXT_HOP_INDEXf);

        /* Update existing next hop entries */
        BCM_IF_ERROR_RETURN
            (_bcm_trident_vlan_vp_nh_info_set(unit, vlan_vp, vp, 0,
                                              &nh_index));

        /* Delete old ingress VLAN translation entry,
         * install new ingress VLAN translation entry
         */
        BCM_IF_ERROR_RETURN
            (_bcm_trident_vlan_vp_match_delete(unit, vp));

        BCM_IF_ERROR_RETURN
            (_bcm_trident_vlan_vp_match_add(unit, vlan_vp, vp));

        /* Decrement old port's VP count, increment new port's VP count */
        BCM_IF_ERROR_RETURN
            (_bcm_trident_vlan_vp_port_cnt_update(unit,
                VLAN_VP_INFO(unit, vp)->port, vp, FALSE));

        BCM_IF_ERROR_RETURN
            (_bcm_trident_vlan_vp_port_cnt_update(unit,
                vlan_vp->port, vp, TRUE));
    }

    /* Set VLAN VP software state */
    VLAN_VP_INFO(unit, vp)->criteria = vlan_vp->criteria;
    VLAN_VP_INFO(unit, vp)->flags = vlan_vp->flags;
    VLAN_VP_INFO(unit, vp)->match_vlan = vlan_vp->match_vlan;
    VLAN_VP_INFO(unit, vp)->match_inner_vlan = vlan_vp->match_inner_vlan;
    VLAN_VP_INFO(unit, vp)->port = vlan_vp->port;

    BCM_GPORT_VLAN_PORT_ID_SET(vlan_vp->vlan_port_id, vp);
    vlan_vp->encap_id = nh_index;

    return rv;

cleanup:
    if (!(vlan_vp->flags & BCM_VLAN_PORT_REPLACE)) {
        (void) _bcm_vp_free(unit, _bcmVpTypeVlan, 1, vp);
        _bcm_trident_vlan_vp_nh_info_delete(unit, nh_index);

        sal_memset(&dvp_entry, 0, sizeof(ing_dvp_table_entry_t));
        (void)WRITE_ING_DVP_TABLEm(unit, MEM_BLOCK_ALL, vp, &dvp_entry);

        sal_memset(&svp_entry, 0, sizeof(source_vp_entry_t));
        (void)WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp_entry);

        (void) _bcm_trident_vlan_vp_match_delete(unit, vp);
    }

    return rv;
}

/*
 * Function:
 *      bcm_trident_vlan_vp_create
 * Purpose:
 *      Create a VLAN virtual port.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vlan_vp - (IN/OUT) Pointer to VLAN virtual port structure. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_trident_vlan_vp_create(int unit,
                         bcm_vlan_port_t *vlan_vp)
{
    int rv;

    VLAN_VIRTUAL_INIT(unit);

    VLAN_VIRTUAL_LOCK(unit);

    rv = _bcm_trident_vlan_vp_create(unit, vlan_vp);

    VLAN_VIRTUAL_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_destroy
 * Purpose:
 *      Destroy a VLAN virtual port.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      gport - (IN) VLAN VP GPORT ID.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_destroy(int unit, bcm_gport_t gport)
{
    int vp;
    source_vp_entry_t svp_entry;
    int nh_index;
    ing_dvp_table_entry_t dvp_entry;

    if (!BCM_GPORT_IS_VLAN_PORT(gport)) {
        return BCM_E_PARAM;
    }

    vp = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
        return BCM_E_NOT_FOUND;
    }

    /* Disable ingress and egress filter modes */
    BCM_IF_ERROR_RETURN
        (bcm_td_vp_vlan_member_set(unit, gport, 0));

    /* Delete ingress VLAN translation entry */
    BCM_IF_ERROR_RETURN
        (_bcm_trident_vlan_vp_match_delete(unit, vp));

    /* Clear SVP entry */
    sal_memset(&svp_entry, 0, sizeof(source_vp_entry_t));
    BCM_IF_ERROR_RETURN
        (WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp_entry));

    /* Clear DVP entry */
    BCM_IF_ERROR_RETURN
        (READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp_entry));
    nh_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp_entry, NEXT_HOP_INDEXf);

    sal_memset(&dvp_entry, 0, sizeof(ing_dvp_table_entry_t));
    BCM_IF_ERROR_RETURN
        (WRITE_ING_DVP_TABLEm(unit, MEM_BLOCK_ALL, vp, &dvp_entry));

    /* Clear next hop entries and free next hop index */
    BCM_IF_ERROR_RETURN
        (_bcm_trident_vlan_vp_nh_info_delete(unit, nh_index));

    /* Decrement port's VP count */
    BCM_IF_ERROR_RETURN
        (_bcm_trident_vlan_vp_port_cnt_update(unit,
                                              VLAN_VP_INFO(unit, vp)->port,
                                              vp, FALSE));
    /* Free VP */
    BCM_IF_ERROR_RETURN
        (_bcm_vp_free(unit, _bcmVpTypeVlan, 1, vp));

    /* Clear VLAN VP software state */
    sal_memset(VLAN_VP_INFO(unit, vp), 0, sizeof(_bcm_trident_vlan_vp_info_t));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_trident_vlan_vp_destroy
 * Purpose:
 *      Destroy a VLAN virtual port.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      gport - (IN) VLAN VP GPORT ID.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_trident_vlan_vp_destroy(int unit, bcm_gport_t gport)
{
    int rv;

    VLAN_VIRTUAL_INIT(unit);

    VLAN_VIRTUAL_LOCK(unit);

    rv = _bcm_trident_vlan_vp_destroy(unit, gport);

    VLAN_VIRTUAL_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_find
 * Purpose:
 *      Get VLAN virtual port info.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vlan_vp - (IN/OUT) Pointer to VLAN virtual port structure. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_find(int unit, bcm_vlan_port_t *vlan_vp)
{
    int vp;
    ing_dvp_table_entry_t dvp_entry;
    int nh_index;

    if (!BCM_GPORT_IS_VLAN_PORT(vlan_vp->vlan_port_id)) {
        return BCM_E_PARAM;
    }

    vp = BCM_GPORT_VLAN_PORT_ID_GET(vlan_vp->vlan_port_id);
    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
        return BCM_E_NOT_FOUND;
    }

    bcm_vlan_port_t_init(vlan_vp);

    BCM_IF_ERROR_RETURN(_bcm_trident_vlan_vp_match_get(unit, vp, vlan_vp));

    vlan_vp->flags = VLAN_VP_INFO(unit, vp)->flags;

    BCM_IF_ERROR_RETURN
        (READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp_entry));
    nh_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp_entry, NEXT_HOP_INDEXf);
    vlan_vp->encap_id = nh_index;

    BCM_GPORT_VLAN_PORT_ID_SET(vlan_vp->vlan_port_id, vp);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_trident_vlan_vp_find
 * Purpose:
 *      Get VLAN virtual port info.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vlan_vp - (IN/OUT) Pointer to VLAN virtual port structure. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_trident_vlan_vp_find(int unit, bcm_vlan_port_t *vlan_vp)
{
    VLAN_VIRTUAL_INIT(unit);
    return _bcm_trident_vlan_vp_find(unit, vlan_vp);
}

/*
 * Function:
 *      _bcm_trident_vlan_port_resolve
 * Purpose:
 *      Get the modid, port, trunk values for a VLAN virtual port
 * Parameters:
 *      unit - (IN) BCM device number
 *      gport - (IN) Global port identifier
 *      modid - (OUT) Module ID
 *      port - (OUT) Port number
 *      trunk_id - (OUT) Trunk ID
 *      id - (OUT) Virtual port ID
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_trident_vlan_port_resolve(int unit, bcm_gport_t vlan_port_id,
                          bcm_module_t *modid, bcm_port_t *port,
                          bcm_trunk_t *trunk_id, int *id)

{
    int rv = BCM_E_NONE, nh_index, vp;
    ing_l3_next_hop_entry_t ing_nh;
    ing_dvp_table_entry_t dvp;

    VLAN_VIRTUAL_INIT(unit);

    if (!BCM_GPORT_IS_VLAN_PORT(vlan_port_id)) {
        return (BCM_E_BADID);
    }

    vp = BCM_GPORT_VLAN_PORT_ID_GET(vlan_port_id);
    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
        return BCM_E_NOT_FOUND;
    }
    BCM_IF_ERROR_RETURN (READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));
    nh_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp,
                                              NEXT_HOP_INDEXf);
    BCM_IF_ERROR_RETURN (soc_mem_read(unit, ING_L3_NEXT_HOPm, MEM_BLOCK_ANY,
                                      nh_index, &ing_nh));

    if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, ENTRY_TYPEf) != 0x2) {
        /* Entry type is not L2 DVP */
        return BCM_E_NOT_FOUND;
    }
    if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, Tf)) {
        *trunk_id = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, TGIDf);
    } else {
        /* Only add this to replication set if destination is local */
        *modid = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, MODULE_IDf);
        *port = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, PORT_NUMf);
    }
    *id = vp;
    return rv;
}

/* ---------------------------------------------------------------------------- 
 *
 * Routines for adding/removing virtual ports to/from VLAN
 *
 * ---------------------------------------------------------------------------- 
 */

/*
 * Function:
 *      bcm_td_vp_control_set
 * Purpose:
 *      Set per virtual port controls.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) VP gport number
 *      type - (IN) Control type
 *      value - (IN) Value to be set
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_td_vp_control_set(int unit, bcm_gport_t gport,
                      bcm_port_control_t type, int value)
{
    int rv;
    int vp;
    source_vp_entry_t svp_entry;
    egr_dvp_attribute_entry_t dvp_entry;

    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        vp = BCM_GPORT_VLAN_PORT_ID_GET(gport);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
            return BCM_E_NOT_FOUND;
        }
    } else {
        return BCM_E_PARAM;
    }

    rv = BCM_E_UNAVAIL;

    switch (type) {
        case bcmPortControlBridge:
            if (SOC_MEM_FIELD_VALID(unit, SOURCE_VPm, DISABLE_VP_PRUNINGf)) {
                SOC_IF_ERROR_RETURN
                    (READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp_entry));
                soc_SOURCE_VPm_field32_set(unit, &svp_entry,
                        DISABLE_VP_PRUNINGf, value ? 1 : 0);
                SOC_IF_ERROR_RETURN
                    (WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp_entry));
                rv = BCM_E_NONE;
            }

            if (SOC_MEM_FIELD_VALID(unit, EGR_DVP_ATTRIBUTEm,
                        DISABLE_VP_PRUNINGf)) {
                SOC_IF_ERROR_RETURN
                    (READ_EGR_DVP_ATTRIBUTEm(unit, MEM_BLOCK_ANY, vp, &dvp_entry));
                soc_EGR_DVP_ATTRIBUTEm_field32_set(unit, &dvp_entry,
                        DISABLE_VP_PRUNINGf, value ? 1 : 0);
                SOC_IF_ERROR_RETURN
                    (WRITE_EGR_DVP_ATTRIBUTEm(unit, MEM_BLOCK_ALL, vp, &dvp_entry));
                rv = BCM_E_NONE;
            } 

            break;
        default:
            break;
    }

    return rv;
}

/*
 * Function:
 *      bcm_td_vp_control_get
 * Purpose:
 *      Get per virtual port controls.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) VP gport number
 *      type - (IN) Control type
 *      value - (OUT) Current value 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_td_vp_control_get(int unit, bcm_gport_t gport,
                       bcm_port_control_t type, int *value)
{
    int rv;
    int vp;
    source_vp_entry_t svp_entry;

    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        vp = BCM_GPORT_VLAN_PORT_ID_GET(gport);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
            return BCM_E_NOT_FOUND;
        }
    } else {
        return BCM_E_PARAM;
    }

    rv = BCM_E_UNAVAIL;

    switch (type) {
        case bcmPortControlBridge:
            if (SOC_MEM_FIELD_VALID(unit, SOURCE_VPm, DISABLE_VP_PRUNINGf)) {
                SOC_IF_ERROR_RETURN
                    (READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp_entry));
                *value = soc_SOURCE_VPm_field32_get(unit, &svp_entry,
                        DISABLE_VP_PRUNINGf);
                rv = BCM_E_NONE;
            } 
            break;
        default:
            break;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_td_vp_group_free_resources
 * Purpose:
 *      Free VP group data structures. 
 * Parameters:
 *      unit - SOC unit number
 * Returns:
 *      Nothing
 */
STATIC void
_bcm_td_vp_group_free_resources(int unit)
{
    int num_vp_groups, i;

    if (VP_GROUP_BK(unit)->ing_vp_group_array) {
        num_vp_groups = soc_mem_field_length(unit, VLAN_TABm, VP_GROUP_BITMAPf);
        for (i = 0; i < num_vp_groups; i++) {
            if (ING_VP_GROUP(unit, i)->vp_bitmap) {
                sal_free(ING_VP_GROUP(unit, i)->vp_bitmap);
                ING_VP_GROUP(unit, i)->vp_bitmap = NULL;
            }
            if (ING_VP_GROUP(unit, i)->vlan_bitmap) {
                sal_free(ING_VP_GROUP(unit, i)->vlan_bitmap);
                ING_VP_GROUP(unit, i)->vlan_bitmap = NULL;
            }
        }
        sal_free(VP_GROUP_BK(unit)->ing_vp_group_array);
        VP_GROUP_BK(unit)->ing_vp_group_array = NULL;
    }

    if (VP_GROUP_BK(unit)->eg_vp_group_array) {
        num_vp_groups = soc_mem_field_length(unit, EGR_VLANm, VP_GROUP_BITMAPf);
        for (i = 0; i < num_vp_groups; i++) {
            if (EG_VP_GROUP(unit, i)->vp_bitmap) {
                sal_free(EG_VP_GROUP(unit, i)->vp_bitmap);
                EG_VP_GROUP(unit, i)->vp_bitmap = NULL;
            }
            if (EG_VP_GROUP(unit, i)->vlan_bitmap) {
                sal_free(EG_VP_GROUP(unit, i)->vlan_bitmap);
                EG_VP_GROUP(unit, i)->vlan_bitmap = NULL;
            }
        }
        sal_free(VP_GROUP_BK(unit)->eg_vp_group_array);
        VP_GROUP_BK(unit)->eg_vp_group_array = NULL;
    }

    return;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_td_vp_group_reinit
 * Purpose:
 *      Recover the virtual port group data structures.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_vp_group_reinit(int unit)
{
    int rv = BCM_E_NONE;
    uint8 *source_vp_buf = NULL;
    uint8 *vlan_buf = NULL;
    uint8 *egr_dvp_buf = NULL;
    uint8 *egr_vlan_buf = NULL;
    int index_min, index_max;
    int i, k;
    source_vp_entry_t *svp_entry;
    int vp_group;
    vlan_tab_entry_t *vlan_entry;
    uint32 fldbuf[2];
    egr_dvp_attribute_entry_t *egr_dvp_entry;
    egr_vlan_entry_t *egr_vlan_entry;

    if (soc_feature(unit, soc_feature_vp_group_ingress_vlan_membership)) {

        /* Recover ingress VP group's virtual ports from SOURCE_VP table */

        source_vp_buf = soc_cm_salloc(unit,
                SOC_MEM_TABLE_BYTES(unit, SOURCE_VPm), "SOURCE_VP buffer");
        if (NULL == source_vp_buf) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        index_min = soc_mem_index_min(unit, SOURCE_VPm);
        index_max = soc_mem_index_max(unit, SOURCE_VPm);
        rv = soc_mem_read_range(unit, SOURCE_VPm, MEM_BLOCK_ANY,
                index_min, index_max, source_vp_buf);
        if (SOC_FAILURE(rv)) {
            goto cleanup;
        }

        for (i = index_min; i <= index_max; i++) {
            svp_entry = soc_mem_table_idx_to_pointer
                (unit, SOURCE_VPm, source_vp_entry_t *, source_vp_buf, i);

            if (soc_SOURCE_VPm_field32_get(unit, svp_entry,
                        ENABLE_IFILTERf) == 0) {
                continue;
            }

            vp_group = soc_SOURCE_VPm_field32_get(unit, svp_entry,
                    VLAN_MEMBERSHIP_PROFILEf);
            SHR_BITSET(ING_VP_GROUP(unit, vp_group)->vp_bitmap, i);
            ING_VP_GROUP(unit, vp_group)->vp_count++;
        }

        soc_cm_sfree(unit, source_vp_buf);
        source_vp_buf= NULL;

        /* Recover ingress VP group's vlans from VLAN table */

        vlan_buf = soc_cm_salloc(unit,
                SOC_MEM_TABLE_BYTES(unit, VLAN_TABm), "VLAN_TAB buffer");
        if (NULL == vlan_buf) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        index_min = soc_mem_index_min(unit, VLAN_TABm);
        index_max = soc_mem_index_max(unit, VLAN_TABm);
        rv = soc_mem_read_range(unit, VLAN_TABm, MEM_BLOCK_ANY,
                index_min, index_max, vlan_buf);
        if (SOC_FAILURE(rv)) {
            goto cleanup;
        }

        for (i = index_min; i <= index_max; i++) {
            vlan_entry = soc_mem_table_idx_to_pointer
                (unit, VLAN_TABm, vlan_tab_entry_t *, vlan_buf, i);

            if (soc_VLAN_TABm_field32_get(unit, vlan_entry, VALIDf) == 0) {
                continue;
            }

            if (soc_VLAN_TABm_field32_get(unit, vlan_entry,
                        VIRTUAL_PORT_ENf) == 0) {
                continue;
            }

            soc_VLAN_TABm_field_get(unit, vlan_entry, VP_GROUP_BITMAPf, fldbuf);
            for (k = 0; k < VP_GROUP_BK(unit)->num_ing_vp_group; k++) {
                if (fldbuf[k / 32] & (1 << (k % 32))) {
                    /* The bit in VP_GROUP_BITMAP that corresponds to
                     * VP group k is set.
                     */
                    SHR_BITSET(ING_VP_GROUP(unit, k)->vlan_bitmap, i);
                }
            }
        }

        soc_cm_sfree(unit, vlan_buf);
        vlan_buf = NULL;
    }

    if (soc_feature(unit, soc_feature_vp_group_egress_vlan_membership)) {

        /* Recover egress VP group's virtual ports from
         * EGR_DVP_ATTRIBUTE table
         */

        egr_dvp_buf = soc_cm_salloc(unit,
                SOC_MEM_TABLE_BYTES(unit, EGR_DVP_ATTRIBUTEm),
                "EGR_DVP buffer");
        if (NULL == egr_dvp_buf) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        index_min = soc_mem_index_min(unit, EGR_DVP_ATTRIBUTEm);
        index_max = soc_mem_index_max(unit, EGR_DVP_ATTRIBUTEm);
        rv = soc_mem_read_range(unit, EGR_DVP_ATTRIBUTEm, MEM_BLOCK_ANY,
                index_min, index_max, egr_dvp_buf);
        if (SOC_FAILURE(rv)) {
            goto cleanup;
        }

        for (i = index_min; i <= index_max; i++) {
            egr_dvp_entry = soc_mem_table_idx_to_pointer
                (unit, EGR_DVP_ATTRIBUTEm, egr_dvp_attribute_entry_t *,
                 egr_dvp_buf, i);

            if (soc_EGR_DVP_ATTRIBUTEm_field32_get(unit, egr_dvp_entry,
                        EN_EFILTERf) == 0) {
                continue;
            }

            vp_group = soc_EGR_DVP_ATTRIBUTEm_field32_get(unit, egr_dvp_entry,
                    VLAN_MEMBERSHIP_PROFILEf);
            SHR_BITSET(EG_VP_GROUP(unit, vp_group)->vp_bitmap, i);
            EG_VP_GROUP(unit, vp_group)->vp_count++;
        }

        soc_cm_sfree(unit, egr_dvp_buf);
        egr_dvp_buf = NULL;

        /* Recover egress VP group's vlans from EGR_VLAN table */

        egr_vlan_buf = soc_cm_salloc(unit,
                SOC_MEM_TABLE_BYTES(unit, EGR_VLANm), "EGR_VLAN buffer");
        if (NULL == egr_vlan_buf) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        index_min = soc_mem_index_min(unit, EGR_VLANm);
        index_max = soc_mem_index_max(unit, EGR_VLANm);
        rv = soc_mem_read_range(unit, EGR_VLANm, MEM_BLOCK_ANY,
                index_min, index_max, egr_vlan_buf);
        if (SOC_FAILURE(rv)) {
            goto cleanup;
        }

        for (i = index_min; i <= index_max; i++) {
            egr_vlan_entry = soc_mem_table_idx_to_pointer
                (unit, EGR_VLANm, egr_vlan_entry_t *, egr_vlan_buf, i);

            if (soc_EGR_VLANm_field32_get(unit, egr_vlan_entry, VALIDf) == 0) {
                continue;
            }

            soc_EGR_VLANm_field_get(unit, egr_vlan_entry, VP_GROUP_BITMAPf,
                    fldbuf);
            for (k = 0; k < VP_GROUP_BK(unit)->num_eg_vp_group; k++) {
                if (fldbuf[k / 32] & (1 << (k % 32))) {
                    /* The bit in VP_GROUP_BITMAP that corresponds to
                     * VP group k is set.
                     */
                    SHR_BITSET(EG_VP_GROUP(unit, k)->vlan_bitmap, i);
                }
            }
        }
        
        soc_cm_sfree(unit, egr_vlan_buf);
        egr_vlan_buf = NULL;
    }

cleanup:
    if (source_vp_buf) {
        soc_cm_sfree(unit, source_vp_buf);
    }
    if (vlan_buf) {
        soc_cm_sfree(unit, vlan_buf);
    }
    if (egr_dvp_buf) {
        soc_cm_sfree(unit, egr_dvp_buf);
    }
    if (egr_vlan_buf) {
        soc_cm_sfree(unit, egr_vlan_buf);
    }

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     bcm_td_vp_group_sw_dump
 * Purpose:
 *     Displays VP group information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
bcm_td_vp_group_sw_dump(int unit)
{
    int i, k;
    int vp_bitmap_bit_size;
    int num_vlan;

    soc_cm_print("\nSW Information Ingress VP Group - Unit %d\n", unit);
    for (i = 0; i < VP_GROUP_BK(unit)->num_ing_vp_group; i++) {
        soc_cm_print("\n  Ingress VP Group = %d\n", i);
        soc_cm_print("    VP Count = %d\n", ING_VP_GROUP(unit, i)->vp_count);

        soc_cm_print("    VP List =");
        vp_bitmap_bit_size = soc_mem_index_count(unit, SOURCE_VPm);
        for (k = 0; k < vp_bitmap_bit_size; k++) {
            if (SHR_BITGET(ING_VP_GROUP(unit, i)->vp_bitmap, k)) {
                soc_cm_print(" %d", k);
            }
        }
        soc_cm_print("\n");

        soc_cm_print("    VLAN List =");
        num_vlan = soc_mem_index_count(unit, VLAN_TABm);
        for (k = 0; k < num_vlan; k++) {
            if (SHR_BITGET(ING_VP_GROUP(unit, i)->vlan_bitmap, k)) {
                soc_cm_print(" %d", k);
            }
        }
        soc_cm_print("\n");
    }

    soc_cm_print("\nSW Information Egress VP Group - Unit %d\n", unit);
    for (i = 0; i < VP_GROUP_BK(unit)->num_eg_vp_group; i++) {
        soc_cm_print("\n  Egress VP Group = %d\n", i);
        soc_cm_print("    VP Count = %d\n", EG_VP_GROUP(unit, i)->vp_count);

        soc_cm_print("    VP List =");
        vp_bitmap_bit_size = soc_mem_index_count(unit, EGR_DVP_ATTRIBUTEm);
        for (k = 0; k < vp_bitmap_bit_size; k++) {
            if (SHR_BITGET(EG_VP_GROUP(unit, i)->vp_bitmap, k)) {
                soc_cm_print(" %d", k);
            }
        }
        soc_cm_print("\n");

        soc_cm_print("    VLAN List =");
        num_vlan = soc_mem_index_count(unit, EGR_VLANm);
        for (k = 0; k < num_vlan; k++) {
            if (SHR_BITGET(EG_VP_GROUP(unit, i)->vlan_bitmap, k)) {
                soc_cm_print(" %d", k);
            }
        }
        soc_cm_print("\n");
    }

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

/*
 * Function:
 *      bcm_td_vp_group_init
 * Purpose:
 *      Initialize the virtual port group data structures.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_td_vp_group_init(int unit)
{
    int num_vp_groups, num_vp, num_vlan, i;
    int rv = BCM_E_NONE;

    sal_memset(VP_GROUP_BK(unit), 0, sizeof(_bcm_td_vp_group_bk_t));

    if (SOC_MEM_FIELD_VALID(unit, VLAN_TABm, VP_GROUP_BITMAPf)) {
        num_vp_groups = soc_mem_field_length(unit, VLAN_TABm, VP_GROUP_BITMAPf);
        VP_GROUP_BK(unit)->num_ing_vp_group = num_vp_groups;

        if (NULL == VP_GROUP_BK(unit)->ing_vp_group_array) {
            VP_GROUP_BK(unit)->ing_vp_group_array = 
                sal_alloc(sizeof(_bcm_td_vp_group_t) * num_vp_groups,
                        "ingress vp group array");
            if (NULL == VP_GROUP_BK(unit)->ing_vp_group_array) {
                _bcm_td_vp_group_free_resources(unit);
                return BCM_E_MEMORY;
            }
        }
        sal_memset(VP_GROUP_BK(unit)->ing_vp_group_array, 0,
                sizeof(_bcm_td_vp_group_t) * num_vp_groups);

        for (i = 0; i < num_vp_groups; i++) {
            num_vp = soc_mem_index_count(unit, SOURCE_VPm);
            if (NULL == ING_VP_GROUP(unit, i)->vp_bitmap) {
                ING_VP_GROUP(unit, i)->vp_bitmap = sal_alloc
                    (SHR_BITALLOCSIZE(num_vp), "ingress vp group vp bitmap");
                if (NULL == ING_VP_GROUP(unit, i)->vp_bitmap) {
                    _bcm_td_vp_group_free_resources(unit);
                    return BCM_E_MEMORY;
                }
            }
            sal_memset(ING_VP_GROUP(unit, i)->vp_bitmap, 0,
                    SHR_BITALLOCSIZE(num_vp));

            num_vlan = soc_mem_index_count(unit, VLAN_TABm);
            if (NULL == ING_VP_GROUP(unit, i)->vlan_bitmap) {
                ING_VP_GROUP(unit, i)->vlan_bitmap = sal_alloc
                    (SHR_BITALLOCSIZE(num_vlan), "ingress vp group vlan bitmap");
                if (NULL == ING_VP_GROUP(unit, i)->vlan_bitmap) {
                    _bcm_td_vp_group_free_resources(unit);
                    return BCM_E_MEMORY;
                }
            }
            sal_memset(ING_VP_GROUP(unit, i)->vlan_bitmap, 0,
                    SHR_BITALLOCSIZE(num_vlan));
        }
    }

    if (SOC_MEM_FIELD_VALID(unit, EGR_VLANm, VP_GROUP_BITMAPf)) {
        num_vp_groups = soc_mem_field_length(unit, EGR_VLANm, VP_GROUP_BITMAPf);
        VP_GROUP_BK(unit)->num_eg_vp_group = num_vp_groups;

        if (NULL == VP_GROUP_BK(unit)->eg_vp_group_array) {
            VP_GROUP_BK(unit)->eg_vp_group_array = 
                sal_alloc(sizeof(_bcm_td_vp_group_t) * num_vp_groups,
                        "egress vp group array");
            if (NULL == VP_GROUP_BK(unit)->eg_vp_group_array) {
                _bcm_td_vp_group_free_resources(unit);
                return BCM_E_MEMORY;
            }
        }
        sal_memset(VP_GROUP_BK(unit)->eg_vp_group_array, 0,
                sizeof(_bcm_td_vp_group_t) * num_vp_groups);

        for (i = 0; i < num_vp_groups; i++) {
            num_vp = soc_mem_index_count(unit, EGR_DVP_ATTRIBUTEm);
            if (NULL == EG_VP_GROUP(unit, i)->vp_bitmap) {
                EG_VP_GROUP(unit, i)->vp_bitmap = sal_alloc
                    (SHR_BITALLOCSIZE(num_vp), "egress vp group vp bitmap");
                if (NULL == EG_VP_GROUP(unit, i)->vp_bitmap) {
                    _bcm_td_vp_group_free_resources(unit);
                    return BCM_E_MEMORY;
                }
            }
            sal_memset(EG_VP_GROUP(unit, i)->vp_bitmap, 0,
                    SHR_BITALLOCSIZE(num_vp));

            num_vlan = soc_mem_index_count(unit, EGR_VLANm);
            if (NULL == EG_VP_GROUP(unit, i)->vlan_bitmap) {
                EG_VP_GROUP(unit, i)->vlan_bitmap = sal_alloc
                    (SHR_BITALLOCSIZE(num_vlan), "egress vp group vlan bitmap");
                if (NULL == EG_VP_GROUP(unit, i)->vlan_bitmap) {
                    _bcm_td_vp_group_free_resources(unit);
                    return BCM_E_MEMORY;
                }
            }
            sal_memset(EG_VP_GROUP(unit, i)->vlan_bitmap, 0,
                    SHR_BITALLOCSIZE(num_vlan));
        }
    }

    VP_GROUP_BK(unit)->vp_group_initialized = 1;

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        rv = _bcm_td_vp_group_reinit(unit);
        if (BCM_FAILURE(rv)) {
            _bcm_td_vp_group_free_resources(unit);
        }
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}

/*
 * Function:
 *      bcm_td_vp_group_detach
 * Purpose:
 *      De-initialize the virtual port group data structures.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_td_vp_group_detach(int unit)
{
    _bcm_td_vp_group_free_resources(unit);

    VP_GROUP_BK(unit)->vp_group_initialized = 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trident_phy_port_trunk_is_local
 * Purpose:
 *      Determine if the given physical port or trunk is local.
 * Parameters:
 *      unit  - (IN) SOC unit number. 
 *      gport - (IN) Physical port or trunk GPORT ID.
 *      is_local - (OUT) Indicates if gport is local.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_phy_port_trunk_is_local(int unit, bcm_gport_t gport, int *is_local)
{
    bcm_trunk_t trunk_id;
    int rv = BCM_E_NONE;
    bcm_trunk_add_info_t tinfo;
    int idx;
    bcm_module_t mod_out;
    bcm_port_t port_out;
    int id;
    int modid_local;

    *is_local = 0;

    if (BCM_GPORT_IS_TRUNK(gport)) {
        trunk_id = BCM_GPORT_TRUNK_GET(gport);
        rv = _bcm_trunk_id_validate(unit, trunk_id);
        if (BCM_FAILURE(rv)) {
            return (BCM_E_PORT);
        }
        rv = bcm_esw_trunk_get(unit, trunk_id, &tinfo);
        if (BCM_FAILURE(rv) || (tinfo.num_ports <= 0)) {
            return (BCM_E_PORT);
        }   

        for (idx=0; idx<tinfo.num_ports; idx++) {
            rv = _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                    tinfo.tm[idx], tinfo.tp[idx], &mod_out, &port_out);
            if (BCM_FAILURE(rv)) {
                return (BCM_E_PORT);
            }

            /* Convert system port to physical port */
            if (soc_feature(unit, soc_feature_sysport_remap)) {
                BCM_XLATE_SYSPORT_S2P(unit, &port_out);
            }
            rv = _bcm_esw_modid_is_local(unit, mod_out, &modid_local);
            if (modid_local) {
                *is_local = 1;
                break;
            }
        }
    } else {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_gport_resolve(unit, gport, &mod_out, &port_out,
                                    &trunk_id, &id)); 
        if ((trunk_id != -1) || (id != -1)) {
            return BCM_E_PARAM;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_esw_modid_is_local(unit, mod_out, &modid_local));

        *is_local = modid_local;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_vp_local_ports_get
 * Purpose:
 *      Get the local ports on which the given VP resides.
 * Parameters:
 *      unit       - (IN) Device Number
 *      vp         - (IN) Virtual port number
 *      local_port_max   - (OUT) Size of local_port_array
 *      local_port_array - (OUT) Array of local ports
 *      local_port_count - (OUT) Number of local ports 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If local_port_max = 0 and local_port_array == NULL,
 *      the number of local ports will still be returned in
 *      local_port_count. 
 */

STATIC int
_bcm_td_vp_local_ports_get(int unit, int vp, int local_port_max,
        bcm_port_t *local_port_array, int *local_port_count)
{
    ing_dvp_table_entry_t dvp_entry;
    ing_l3_next_hop_entry_t ing_nh;
    uint32 nh_index;
    bcm_trunk_t trunk_id;
    bcm_module_t modid;
    bcm_port_t port;
    int modid_local;

    if (local_port_max < 0) {
        return BCM_E_PARAM;
    }

    if ((local_port_max == 0) && (NULL != local_port_array)) {
        return BCM_E_PARAM;
    }

    if ((local_port_max > 0) && (NULL == local_port_array)) {
        return BCM_E_PARAM;
    }

    if (NULL == local_port_count) {
        return BCM_E_PARAM;
    }

    *local_port_count = 0;

    BCM_IF_ERROR_RETURN
        (READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp_entry));
    nh_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp_entry,
            NEXT_HOP_INDEXf);
    BCM_IF_ERROR_RETURN (soc_mem_read(unit, ING_L3_NEXT_HOPm, MEM_BLOCK_ANY,
                nh_index, &ing_nh));

    if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, Tf)) {
        trunk_id = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, TGIDf);

        BCM_IF_ERROR_RETURN
            (_bcm_esw_trunk_local_members_get(unit,
                                             trunk_id, 
                                             local_port_max,
                                             local_port_array, 
                                             local_port_count));
    } else {
        modid = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, MODULE_IDf);
        port = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, PORT_NUMf);

        BCM_IF_ERROR_RETURN
            (_bcm_esw_modid_is_local(unit, modid, &modid_local));
        if (TRUE != modid_local) {
            *local_port_count = 0;
            return BCM_E_NONE;
        }

        *local_port_count = 1;
        if (NULL != local_port_array) {
            local_port_array[0] = port;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_vp_vlan_bitmap_get
 * Purpose:
 *      Get a bitmap of all the VLANs the given VP belongs to.
 * Parameters:
 *      unit        - (IN) BCM device number
 *      vp_gport    - (IN) VP gport ID 
 *      vlan_bitmap - (OUT) Bitmap of VLANs
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_vp_vlan_bitmap_get(int unit, bcm_gport_t vp_gport, SHR_BITDCL *vlan_bitmap)
{
    int rv;
    int vp;
    int mc_type;
    source_vp_entry_t svp_entry;
    int vp_group;
    int num_vlan;
    egr_dvp_attribute_entry_t egr_dvp_entry;
    bcm_port_t local_port;
    int local_port_count;
    uint8 *vlan_tab_buf = NULL;
    int index_min, index_max;
    int if_max, if_count;
    bcm_if_t *if_array = NULL;
    int i, j, k;
    vlan_tab_entry_t *vlan_tab_entry;
    int mc_index_array[3];
    int match_prev_mc_index = FALSE;
    bcm_multicast_t group;
    bcm_gport_t local_gport;
    bcm_if_t encap_id;
    int match = FALSE;

    if (BCM_GPORT_IS_VLAN_PORT(vp_gport)) {
        vp = BCM_GPORT_VLAN_PORT_ID_GET(vp_gport);
        mc_type = _BCM_MULTICAST_TYPE_VLAN;
    } else {
        return BCM_E_PARAM;
    }

    /* Check if VP already belongs to a VP group.
     * If so, just return the VP group's VLAN bitmap.
     */

    if (soc_feature(unit, soc_feature_vp_group_ingress_vlan_membership)) {
        SOC_IF_ERROR_RETURN
            (READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp_entry));
        if (soc_SOURCE_VPm_field32_get(unit, &svp_entry, ENABLE_IFILTERf)) {
            vp_group = soc_SOURCE_VPm_field32_get(unit, &svp_entry,
                    VLAN_MEMBERSHIP_PROFILEf);
            num_vlan = soc_mem_index_count(unit, VLAN_TABm);
            sal_memcpy(vlan_bitmap, ING_VP_GROUP(unit, vp_group)->vlan_bitmap,
                    SHR_BITALLOCSIZE(num_vlan));
            return BCM_E_NONE;
        }
    }

    if (soc_feature(unit, soc_feature_vp_group_egress_vlan_membership)) {
        SOC_IF_ERROR_RETURN
            (READ_EGR_DVP_ATTRIBUTEm(unit, MEM_BLOCK_ANY, vp, &egr_dvp_entry));
        if (soc_EGR_DVP_ATTRIBUTEm_field32_get(unit, &egr_dvp_entry,
                    EN_EFILTERf)) {
            vp_group = soc_EGR_DVP_ATTRIBUTEm_field32_get(unit, &egr_dvp_entry,
                    VLAN_MEMBERSHIP_PROFILEf);
            num_vlan = soc_mem_index_count(unit, EGR_VLANm);
            sal_memcpy(vlan_bitmap, EG_VP_GROUP(unit, vp_group)->vlan_bitmap,
                    SHR_BITALLOCSIZE(num_vlan));
            return BCM_E_NONE;
        }
    }

    /* VP does not belong to any VP group. Need to derive VLAN bitmap by
     * searching through each VLAN table entry's BC_IDX/UMC_IDX/UUC_IDX
     * multicast groups to see if VP belongs to their VP replication lists.
     */ 

    /* Get one local port on which the VP resides.
     * Even if the VP resides on a trunk group, only one trunk
     * member is needed since all members of a trunk group
     * have the same VP replication list.
     */
    BCM_IF_ERROR_RETURN
        (_bcm_td_vp_local_ports_get(unit,
                                    vp, 1, &local_port, &local_port_count));
    if (local_port_count == 0) {
        return BCM_E_PORT; 
    }

    vlan_tab_buf = soc_cm_salloc(unit,
            SOC_MEM_TABLE_BYTES(unit, VLAN_TABm), "VLAN_TAB buffer");
    if (NULL == vlan_tab_buf) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }

    index_min = soc_mem_index_min(unit, VLAN_TABm);
    index_max = soc_mem_index_max(unit, VLAN_TABm);
    rv = soc_mem_read_range(unit, VLAN_TABm, MEM_BLOCK_ANY,
            index_min, index_max, vlan_tab_buf);
    if (SOC_FAILURE(rv)) {
        goto cleanup;
    }

    if_max = soc_mem_index_count(unit, ING_L3_NEXT_HOPm);
    if_array = sal_alloc(sizeof(bcm_if_t) * if_max, "if_array");
    if (NULL == if_array) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }
    sal_memset(if_array, 0, sizeof(bcm_if_t) * if_max);

    SHR_BITCLR_RANGE(vlan_bitmap, 0, soc_mem_index_count(unit, VLAN_TABm));

    for (i = index_min; i <= index_max; i++) {
        vlan_tab_entry = soc_mem_table_idx_to_pointer
            (unit, VLAN_TABm, vlan_tab_entry_t *, vlan_tab_buf, i);

        if (0 == soc_VLAN_TABm_field32_get(unit, vlan_tab_entry,
                    VALIDf)) {
            continue;
        }

        if (0 == soc_VLAN_TABm_field32_get(unit, vlan_tab_entry,
                    VIRTUAL_PORT_ENf)) {
            continue;
        }

        mc_index_array[0] = soc_VLAN_TABm_field32_get(unit, vlan_tab_entry,
                BC_IDXf);
        mc_index_array[1] = soc_VLAN_TABm_field32_get(unit, vlan_tab_entry,
                UMC_IDXf);
        mc_index_array[2] = soc_VLAN_TABm_field32_get(unit, vlan_tab_entry,
                UUC_IDXf);

        for (j = 0; j < 3; j++) {

            /* Check if the same mc_index was already searched */
            match_prev_mc_index = FALSE;
            for (k = j - 1; k >= 0; k--) {
                if (mc_index_array[j] == mc_index_array[k]) {
                    match_prev_mc_index = TRUE;
                    break;
                }
            }
            if (match_prev_mc_index) {
                /* continue to next mc_index */
                continue;
            }

            /* Get VP replication list for (mc_index, local_port) */
            rv = bcm_esw_ipmc_egress_intf_get(unit, mc_index_array[j],
                    local_port, if_max, if_array, &if_count);
            if (SOC_FAILURE(rv)) {
                goto cleanup;
            }

            /* Get VP's encap_id */
            _BCM_MULTICAST_GROUP_SET(group, mc_type, mc_index_array[j]);
            rv = bcm_esw_port_gport_get(unit, local_port, &local_gport);
            if (SOC_FAILURE(rv)) {
                goto cleanup;
            }
            rv = bcm_esw_multicast_vlan_encap_get(unit, group, local_gport, 
                                              vp_gport, &encap_id);
            if (SOC_FAILURE(rv)) {
                goto cleanup;
            }

            /* Search for VP's encap_id in if_array */
            match = FALSE;
            for (k = 0; k < if_count; k++) {
                if (encap_id == if_array[k]) {
                    match = TRUE;
                    break;
                }
            }
            if (match) {
                break;
            }
        }

        if (match) {
            /* VP belongs to this VLAN */
            SHR_BITSET(vlan_bitmap, i);
        }
    }

cleanup:
    if (vlan_tab_buf) {
        soc_cm_sfree(unit, vlan_tab_buf);
    }

    if (if_array) {
        sal_free(if_array);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_td_ing_vp_group_join
 * Purpose:
 *      Assign VP to an ingress VP group.
 * Parameters:
 *      unit        - (IN) BCM device number
 *      vp          - (IN) VP number
 *      vlan_bitmap - (IN) Bitmap of VLANs the VP belongs to.
 *      vp_group    - (OUT) Assigned VP group.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_ing_vp_group_join(int unit, int vp,
        SHR_BITDCL *vlan_bitmap, int *vp_group)
{
    int i;
    int num_vlan, vid;
    vlan_tab_entry_t vlan_entry;
    uint32 fldbuf[2];

    num_vlan = soc_mem_index_count(unit, VLAN_TABm);

    /* First, check if VP's vlan_bitmap matches any existing ingress
     * VP group's vlan_bitmap. If so, assign the VP to the VP group
     * with the matching vlan_bitmap.
     */
    for (i = 0; i < VP_GROUP_BK(unit)->num_ing_vp_group; i++) {
        if ((ING_VP_GROUP(unit, i)->vp_count > 0) &&
            SHR_BITEQ_RANGE(vlan_bitmap, ING_VP_GROUP(unit, i)->vlan_bitmap,
                    0, num_vlan)) {
            *vp_group = i;
            SHR_BITSET(ING_VP_GROUP(unit, i)->vp_bitmap, vp);
            ING_VP_GROUP(unit, i)->vp_count++;
            return BCM_E_NONE;
        }
    }

    /* VP's vlan_bitmap does not match any existing VP group's
     * vlan_bitmap. Allocate a new ingress VP group for the VP.
     */
    for (i = 0; i < VP_GROUP_BK(unit)->num_ing_vp_group; i++) {
        if (0 == ING_VP_GROUP(unit, i)->vp_count) {
            *vp_group = i;
            SHR_BITSET(ING_VP_GROUP(unit, i)->vp_bitmap, vp);
            ING_VP_GROUP(unit, i)->vp_count++;
            sal_memcpy(ING_VP_GROUP(unit, i)->vlan_bitmap, vlan_bitmap,
                    SHR_BITALLOCSIZE(num_vlan));

            /* Update VLAN table's VP_GROUP_BITMAP field */
            for (vid = 0; vid < num_vlan; vid++) {
                if (SHR_BITGET(ING_VP_GROUP(unit, i)->vlan_bitmap, vid)) {
                    SOC_IF_ERROR_RETURN
                        (READ_VLAN_TABm(unit, MEM_BLOCK_ANY, vid, &vlan_entry));
                    soc_VLAN_TABm_field_get(unit, &vlan_entry,
                            VP_GROUP_BITMAPf, fldbuf);
                    fldbuf[i / 32] |= (1 << (i % 32));
                    soc_VLAN_TABm_field_set(unit, &vlan_entry,
                            VP_GROUP_BITMAPf, fldbuf);
                    SOC_IF_ERROR_RETURN
                        (WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, vid, &vlan_entry));
                }
            }

            return BCM_E_NONE;
        }
    }

    /* An empty VP group is not available. */
    return BCM_E_RESOURCE;
}

/*
 * Function:
 *      _bcm_td_eg_vp_group_join
 * Purpose:
 *      Assign VP to an egress VP group.
 * Parameters:
 *      unit        - (IN) BCM device number
 *      vp          - (IN) VP number
 *      vlan_bitmap - (IN) Bitmap of VLANs the VP belongs to.
 *      vp_group    - (OUT) Assigned VP group.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_eg_vp_group_join(int unit, int vp,
        SHR_BITDCL *vlan_bitmap, int *vp_group)
{
    int i;
    int num_vlan, vid;
    egr_vlan_entry_t egr_vlan_entry;
    uint32 fldbuf[2];

    num_vlan = soc_mem_index_count(unit, EGR_VLANm);

    /* First, check if VP's vlan_bitmap matches any existing egress
     * VP group's vlan_bitmap. If so, assign the VP to the VP group
     * with the matching vlan_bitmap.
     */
    for (i = 0; i < VP_GROUP_BK(unit)->num_eg_vp_group; i++) {
        if ((EG_VP_GROUP(unit, i)->vp_count > 0) &&
            SHR_BITEQ_RANGE(vlan_bitmap, EG_VP_GROUP(unit, i)->vlan_bitmap,
                    0, num_vlan)) {
            *vp_group = i;
            SHR_BITSET(EG_VP_GROUP(unit, i)->vp_bitmap, vp);
            EG_VP_GROUP(unit, i)->vp_count++;
            return BCM_E_NONE;
        }
    }

    /* VP's vlan_bitmap does not match any existing VP group's
     * vlan_bitmap. Allocate a new egress VP group for the VP.
     */
    for (i = 0; i < VP_GROUP_BK(unit)->num_eg_vp_group; i++) {
        if (0 == EG_VP_GROUP(unit, i)->vp_count) {
            *vp_group = i;
            SHR_BITSET(EG_VP_GROUP(unit, i)->vp_bitmap, vp);
            EG_VP_GROUP(unit, i)->vp_count++;
            sal_memcpy(EG_VP_GROUP(unit, i)->vlan_bitmap, vlan_bitmap,
                    SHR_BITALLOCSIZE(num_vlan));

            /* Update EGR_VLAN table's VP_GROUP_BITMAP field */
            for (vid = 0; vid < num_vlan; vid++) {
                if (SHR_BITGET(EG_VP_GROUP(unit, i)->vlan_bitmap, vid)) {
                    SOC_IF_ERROR_RETURN
                        (READ_EGR_VLANm(unit, MEM_BLOCK_ANY, vid,
                                        &egr_vlan_entry));
                    soc_EGR_VLANm_field_get(unit, &egr_vlan_entry,
                            VP_GROUP_BITMAPf, fldbuf);
                    fldbuf[i / 32] |= (1 << (i % 32));
                    soc_EGR_VLANm_field_set(unit, &egr_vlan_entry,
                            VP_GROUP_BITMAPf, fldbuf);
                    SOC_IF_ERROR_RETURN
                        (WRITE_EGR_VLANm(unit, MEM_BLOCK_ALL, vid,
                                         &egr_vlan_entry));
                }
            }

            return BCM_E_NONE;
        }
    }

    /* An empty VP group is not available. */
    return BCM_E_RESOURCE;
}

/*
 * Function:
 *      _bcm_td_ing_vp_group_leave
 * Purpose:
 *      Remove VP from an ingress VP group.
 * Parameters:
 *      unit     - (IN) BCM device number
 *      vp       - (IN) VP number
 *      vp_group - (IN) VP group.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_ing_vp_group_leave(int unit, int vp, int vp_group)
{
    int num_vlan, vid;
    vlan_tab_entry_t vlan_entry;
    uint32 fldbuf[2];

    SHR_BITCLR(ING_VP_GROUP(unit, vp_group)->vp_bitmap, vp);
    ING_VP_GROUP(unit, vp_group)->vp_count--;

    if (0 == ING_VP_GROUP(unit, vp_group)->vp_count) {

        /* Clear the bit corresponding to the given VP group in
         * VLAN table's VP_GROUP_BITMAP field
         */
        num_vlan = soc_mem_index_count(unit, VLAN_TABm);
        for (vid = 0; vid < num_vlan; vid++) {
            if (SHR_BITGET(ING_VP_GROUP(unit, vp_group)->vlan_bitmap, vid)) {
                SOC_IF_ERROR_RETURN
                    (READ_VLAN_TABm(unit, MEM_BLOCK_ANY, vid, &vlan_entry));
                soc_VLAN_TABm_field_get(unit, &vlan_entry,
                        VP_GROUP_BITMAPf, fldbuf);
                fldbuf[vp_group / 32] &= ~(1 << (vp_group % 32));
                soc_VLAN_TABm_field_set(unit, &vlan_entry,
                        VP_GROUP_BITMAPf, fldbuf);
                SOC_IF_ERROR_RETURN
                    (WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, vid, &vlan_entry));
            }
        }

        SHR_BITCLR_RANGE(ING_VP_GROUP(unit, vp_group)->vlan_bitmap, 0, num_vlan);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_eg_vp_group_leave
 * Purpose:
 *      Remove VP from an egress VP group.
 * Parameters:
 *      unit     - (IN) BCM device number
 *      vp       - (IN) VP number
 *      vp_group - (IN) VP group.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_eg_vp_group_leave(int unit, int vp, int vp_group)
{
    int num_vlan, vid;
    egr_vlan_entry_t egr_vlan_entry;
    uint32 fldbuf[2];

    SHR_BITCLR(EG_VP_GROUP(unit, vp_group)->vp_bitmap, vp);
    EG_VP_GROUP(unit, vp_group)->vp_count--;

    if (0 == EG_VP_GROUP(unit, vp_group)->vp_count) {

        /* Clear the bit corresponding to the given VP group in
         * EGR_VLAN table's VP_GROUP_BITMAP field
         */
        num_vlan = soc_mem_index_count(unit, EGR_VLANm);
        for (vid = 0; vid < num_vlan; vid++) {
            if (SHR_BITGET(EG_VP_GROUP(unit, vp_group)->vlan_bitmap, vid)) {
                SOC_IF_ERROR_RETURN
                    (READ_EGR_VLANm(unit, MEM_BLOCK_ANY, vid, &egr_vlan_entry));
                soc_EGR_VLANm_field_get(unit, &egr_vlan_entry,
                        VP_GROUP_BITMAPf, fldbuf);
                fldbuf[vp_group / 32] &= ~(1 << (vp_group % 32));
                soc_EGR_VLANm_field_set(unit, &egr_vlan_entry,
                        VP_GROUP_BITMAPf, fldbuf);
                SOC_IF_ERROR_RETURN
                    (WRITE_EGR_VLANm(unit, MEM_BLOCK_ALL, vid, &egr_vlan_entry));
            }
        }

        SHR_BITCLR_RANGE(EG_VP_GROUP(unit, vp_group)->vlan_bitmap, 0, num_vlan);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_ing_vp_group_move
 * Purpose:
 *      Move VP from one ingress VP group to another due to add/remove VP
 *      to/from VLAN.
 * Parameters:
 *      unit - (IN) BCM device number
 *      vp   - (IN) VP number
 *      vlan - (IN) VLAN to/from which VP is added or removed
 *      add  - (IN) If TRUE, VP is added to VLAN, else removed from VLAN.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_ing_vp_group_move(int unit, int vp, bcm_vlan_t vlan, int add)
{
    int rv = BCM_E_NONE;
    source_vp_entry_t svp_entry;
    int ifilter_en;
    int old_vp_group, new_vp_group;
    int num_vlan;
    SHR_BITDCL *new_vlan_bitmap = NULL;


    SOC_IF_ERROR_RETURN
        (READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp_entry));
    ifilter_en = soc_SOURCE_VPm_field32_get(unit, &svp_entry,
            ENABLE_IFILTERf);
    old_vp_group = soc_SOURCE_VPm_field32_get(unit, &svp_entry,
            VLAN_MEMBERSHIP_PROFILEf);

    if (ifilter_en) {

        /* Derive VP's new VLAN bitmap by adding/removing VLAN
         * to/from VP group's VLAN bitmap
         */
        num_vlan = soc_mem_index_count(unit, VLAN_TABm);
        new_vlan_bitmap = sal_alloc(SHR_BITALLOCSIZE(num_vlan),
                "vlan bitmap");
        if (NULL == new_vlan_bitmap) {
            return BCM_E_MEMORY;
        }
        sal_memset(new_vlan_bitmap, 0, SHR_BITALLOCSIZE(num_vlan));

        sal_memcpy(new_vlan_bitmap,
                ING_VP_GROUP(unit, old_vp_group)->vlan_bitmap, 
                SHR_BITALLOCSIZE(num_vlan));
        if (add) {
            SHR_BITSET(new_vlan_bitmap, vlan);
        } else {
            SHR_BITCLR(new_vlan_bitmap, vlan);
        }

        /* If VP's new VLAN bitmap is not the same as the VP group's
         * VLAN bitmap, move the VP to another VP group
         */
        if (!SHR_BITEQ_RANGE(new_vlan_bitmap,
                    ING_VP_GROUP(unit, old_vp_group)->vlan_bitmap,
                    0, num_vlan)) {

            /* Join a new VP group */
            rv = _bcm_td_ing_vp_group_join(unit, vp, new_vlan_bitmap,
                    &new_vp_group);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }

            /* Leave the old VP group */
            rv = _bcm_td_ing_vp_group_leave(unit, vp, old_vp_group);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }

            /* Update vp group field */
            soc_SOURCE_VPm_field32_set(unit, &svp_entry,
                    VLAN_MEMBERSHIP_PROFILEf, new_vp_group);
            rv = WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp_entry);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }
        }
    }

cleanup:
    if (new_vlan_bitmap) {
        sal_free(new_vlan_bitmap);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_td_eg_vp_group_move
 * Purpose:
 *      Move VP from one egress VP group to another due to add/remove VP
 *      to/from VLAN.
 * Parameters:
 *      unit - (IN) BCM device number
 *      vp   - (IN) VP number
 *      vlan - (IN) VLAN to/from which VP is added or removed
 *      add  - (IN) If TRUE, VP is added to VLAN, else removed from VLAN.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_eg_vp_group_move(int unit, int vp, bcm_vlan_t vlan, int add)
{
    int rv = BCM_E_NONE;
    source_vp_entry_t dvp_entry;
    int efilter_en;
    int old_vp_group, new_vp_group;
    int num_vlan;
    SHR_BITDCL *new_vlan_bitmap = NULL;

    SOC_IF_ERROR_RETURN
        (READ_EGR_DVP_ATTRIBUTEm(unit, MEM_BLOCK_ANY, vp, &dvp_entry));
    efilter_en = soc_EGR_DVP_ATTRIBUTEm_field32_get(unit, &dvp_entry,
            EN_EFILTERf);
    old_vp_group = soc_EGR_DVP_ATTRIBUTEm_field32_get(unit, &dvp_entry,
            VLAN_MEMBERSHIP_PROFILEf);

    if (efilter_en) {

        /* Derive VP's new VLAN bitmap by adding/removing VLAN
         * to/from VP group's VLAN bitmap
         */
        num_vlan = soc_mem_index_count(unit, EGR_VLANm);
        new_vlan_bitmap = sal_alloc(SHR_BITALLOCSIZE(num_vlan),
                "vlan bitmap");
        if (NULL == new_vlan_bitmap) {
            return BCM_E_MEMORY;
        }
        sal_memset(new_vlan_bitmap, 0, SHR_BITALLOCSIZE(num_vlan));

        sal_memcpy(new_vlan_bitmap,
                EG_VP_GROUP(unit, old_vp_group)->vlan_bitmap, 
                SHR_BITALLOCSIZE(num_vlan));
        if (add) {
            SHR_BITSET(new_vlan_bitmap, vlan);
        } else {
            SHR_BITCLR(new_vlan_bitmap, vlan);
        }

        /* If VP's new VLAN bitmap is not the same as the VP group's
         * VLAN bitmap, move the VP to another VP group
         */
        if (!SHR_BITEQ_RANGE(new_vlan_bitmap,
                    EG_VP_GROUP(unit, old_vp_group)->vlan_bitmap,
                    0, num_vlan)) {

            /* Join a new VP group */
            rv = _bcm_td_eg_vp_group_join(unit, vp, new_vlan_bitmap,
                    &new_vp_group);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }

            /* Leave the old VP group */
            rv = _bcm_td_eg_vp_group_leave(unit, vp, old_vp_group);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }

            /* Update vp group field */
            soc_EGR_DVP_ATTRIBUTEm_field32_set(unit, &dvp_entry,
                    VLAN_MEMBERSHIP_PROFILEf, new_vp_group);
            rv = WRITE_EGR_DVP_ATTRIBUTEm(unit, MEM_BLOCK_ALL, vp, &dvp_entry);
            if (BCM_FAILURE(rv)) {
                goto cleanup;
            }
        }
    }

cleanup:
    if (new_vlan_bitmap) {
        sal_free(new_vlan_bitmap);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_td_ing_vp_group_vlan_delete_all
 * Purpose:
 *      Delete all VP groups from the given VLAN.
 * Parameters:
 *      unit - (IN) BCM device number
 *      vlan - (IN) VLAN ID 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_ing_vp_group_vlan_delete_all(int unit, bcm_vlan_t vlan)
{
    vlan_tab_entry_t vlan_entry;
    uint32 fldbuf[2];
    int i, k;
    int vp_bitmap_bit_size;

    SOC_IF_ERROR_RETURN
        (READ_VLAN_TABm(unit, MEM_BLOCK_ANY, vlan, &vlan_entry));
    soc_VLAN_TABm_field_get(unit, &vlan_entry, VP_GROUP_BITMAPf, fldbuf);

    for (i = 0; i < VP_GROUP_BK(unit)->num_ing_vp_group; i++) {
        if (fldbuf[i / 32] & (1 << (i % 32))) {
            /* The bit in VP_GROUP_BITMAP that corresponds to
             * VP group i is set. Then remove all VPs in VP group i
             * from VLAN.
             */
            vp_bitmap_bit_size = soc_mem_index_count(unit, SOURCE_VPm);
            for (k = 0; k < vp_bitmap_bit_size; k++) {
                if (SHR_BITGET(ING_VP_GROUP(unit, i)->vp_bitmap, k)) {
                    /* VP k belongs to VP group i */
                    BCM_IF_ERROR_RETURN
                        (_bcm_td_ing_vp_group_move(unit, k, vlan, FALSE));  
                }
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_eg_vp_group_vlan_delete_all
 * Purpose:
 *      Delete all VP groups from the given VLAN.
 * Parameters:
 *      unit - (IN) BCM device number
 *      vlan - (IN) VLAN ID 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_eg_vp_group_vlan_delete_all(int unit, bcm_vlan_t vlan)
{
    egr_vlan_entry_t egr_vlan_entry;
    uint32 fldbuf[2];
    int i, k;
    int vp_bitmap_bit_size;

    SOC_IF_ERROR_RETURN
        (READ_EGR_VLANm(unit, MEM_BLOCK_ANY, vlan, &egr_vlan_entry));
    soc_EGR_VLANm_field_get(unit, &egr_vlan_entry, VP_GROUP_BITMAPf, fldbuf);

    for (i = 0; i < VP_GROUP_BK(unit)->num_eg_vp_group; i++) {
        if (fldbuf[i / 32] & (1 << (i % 32))) {
            /* The bit in VP_GROUP_BITMAP that corresponds to
             * VP group i is set. Then remove all VPs in VP group i
             * from VLAN.
             */
            vp_bitmap_bit_size = soc_mem_index_count(unit, EGR_DVP_ATTRIBUTEm);
            for (k = 0; k < vp_bitmap_bit_size; k++) {
                if (SHR_BITGET(EG_VP_GROUP(unit, i)->vp_bitmap, k)) {
                    /* VP k belongs to VP group i */
                    BCM_IF_ERROR_RETURN
                        (_bcm_td_eg_vp_group_move(unit, k, vlan, FALSE));  
                }
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td_vp_vlan_member_set
 * Purpose:
 *      Set virtual port ingress and egress filter modes.
 * Parameters:
 *      unit  - (IN) BCM device number
 *      gport - (IN) VP gport ID 
 *      flags - (IN) Ingress and egress filter modes.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_td_vp_vlan_member_set(int unit, bcm_gport_t gport, uint32 flags)
{
    int rv = BCM_E_NONE;
    int vp;
    bcm_vlan_port_t vlan_vp;
    bcm_gport_t phy_port_trunk;
    int is_local;
    source_vp_entry_t svp_entry;
    egr_dvp_attribute_entry_t dvp_entry;
    int old_ifilter_en, old_efilter_en;
    int old_vp_group, vp_group;
    int num_vlan;
    SHR_BITDCL *vlan_bitmap = NULL;


    if (flags & BCM_PORT_VLAN_MEMBER_INGRESS) {
        if (!soc_feature(unit, soc_feature_vp_group_ingress_vlan_membership)) {
            return BCM_E_UNAVAIL;
        }
    }

    if (flags & BCM_PORT_VLAN_MEMBER_EGRESS) {
        if (!soc_feature(unit, soc_feature_vp_group_egress_vlan_membership)) {
            return BCM_E_UNAVAIL;
        }
    }

    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        vp = BCM_GPORT_VLAN_PORT_ID_GET(gport);

        /* Get the physical port or trunk the VP resides on */
        bcm_vlan_port_t_init(&vlan_vp);
        BCM_GPORT_VLAN_PORT_ID_SET(vlan_vp.vlan_port_id, vp);
        BCM_IF_ERROR_RETURN(bcm_trident_vlan_vp_find(unit, &vlan_vp));
        phy_port_trunk = vlan_vp.port;
    } else {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_trident_phy_port_trunk_is_local(unit, phy_port_trunk, &is_local));
    if (!is_local) {
        /* Ingress and egress filter modes can be set only on local VPs */
        return BCM_E_PORT;
    }

    /* Set ingress filter mode */

    if (soc_feature(unit, soc_feature_vp_group_ingress_vlan_membership)) {

        SOC_IF_ERROR_RETURN
            (READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp_entry));
        old_ifilter_en = soc_SOURCE_VPm_field32_get(unit, &svp_entry,
                ENABLE_IFILTERf);
        old_vp_group = soc_SOURCE_VPm_field32_get(unit, &svp_entry,
                VLAN_MEMBERSHIP_PROFILEf);

        if (flags & BCM_PORT_VLAN_MEMBER_INGRESS) {
            if (0 == old_ifilter_en) {
                /* Enable ingress filtering */

                /* Get all the VLANs this VP belongs to */
                num_vlan = soc_mem_index_count(unit, VLAN_TABm);
                vlan_bitmap = sal_alloc(SHR_BITALLOCSIZE(num_vlan),
                        "vlan bitmap");
                if (NULL == vlan_bitmap) {
                    return BCM_E_MEMORY;
                }
                sal_memset(vlan_bitmap, 0, SHR_BITALLOCSIZE(num_vlan));
                rv = _bcm_td_vp_vlan_bitmap_get(unit, gport, vlan_bitmap);
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }

                /* Assign VP to an ingress VP group */ 
                rv = _bcm_td_ing_vp_group_join(unit, vp, vlan_bitmap,
                        &vp_group);
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }

                /* Update ingress filter enable and vp group fields */
                soc_SOURCE_VPm_field32_set(unit, &svp_entry,
                        ENABLE_IFILTERf, 1);
                soc_SOURCE_VPm_field32_set(unit, &svp_entry,
                        VLAN_MEMBERSHIP_PROFILEf, vp_group);
                rv = WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp_entry);
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }

                sal_free(vlan_bitmap);
                vlan_bitmap = NULL;
            }
        } else {
            if (1 == old_ifilter_en) {
                /* Disable ingress filtering */

                /* Update ingress filter enable and vp group fields */
                soc_SOURCE_VPm_field32_set(unit, &svp_entry,
                        ENABLE_IFILTERf, 0);
                soc_SOURCE_VPm_field32_set(unit, &svp_entry,
                        VLAN_MEMBERSHIP_PROFILEf, 0);
                BCM_IF_ERROR_RETURN
                    (WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp_entry));

                /* Remove VP from VP group */
                BCM_IF_ERROR_RETURN
                    (_bcm_td_ing_vp_group_leave(unit, vp, old_vp_group));
            }
        }
    }

    /* Set egress filter mode */

    if (soc_feature(unit, soc_feature_vp_group_egress_vlan_membership)) {

        SOC_IF_ERROR_RETURN
            (READ_EGR_DVP_ATTRIBUTEm(unit, MEM_BLOCK_ANY, vp, &dvp_entry));
        old_efilter_en = soc_EGR_DVP_ATTRIBUTEm_field32_get(unit, &dvp_entry,
                EN_EFILTERf);
        old_vp_group = soc_EGR_DVP_ATTRIBUTEm_field32_get(unit, &dvp_entry,
                VLAN_MEMBERSHIP_PROFILEf);

        if (flags & BCM_PORT_VLAN_MEMBER_EGRESS) {
            if (0 == old_efilter_en) {
                /* Enable egress filtering */

                /* Get all the VLANs this VP belongs to */
                num_vlan = soc_mem_index_count(unit, EGR_VLANm);
                vlan_bitmap = sal_alloc(SHR_BITALLOCSIZE(num_vlan),
                        "vlan bitmap");
                if (NULL == vlan_bitmap) {
                    return BCM_E_MEMORY;
                }
                sal_memset(vlan_bitmap, 0, SHR_BITALLOCSIZE(num_vlan));
                rv = _bcm_td_vp_vlan_bitmap_get(unit, gport, vlan_bitmap);
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }

                /* Assign VP to an egress VP group */ 
                rv = _bcm_td_eg_vp_group_join(unit, vp, vlan_bitmap,
                        &vp_group);
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }

                /* Update egress filter enable and vp group fields */
                soc_EGR_DVP_ATTRIBUTEm_field32_set(unit, &dvp_entry,
                        EN_EFILTERf, 1);
                soc_EGR_DVP_ATTRIBUTEm_field32_set(unit, &dvp_entry,
                        VLAN_MEMBERSHIP_PROFILEf, vp_group);
                rv = WRITE_EGR_DVP_ATTRIBUTEm(unit, MEM_BLOCK_ALL, vp, &dvp_entry);
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }

                sal_free(vlan_bitmap);
                vlan_bitmap = NULL;
            }
        } else {
            if (1 == old_efilter_en) {
                /* Disable egress filtering */

                /* Update egress filter enable and vp group fields */
                soc_EGR_DVP_ATTRIBUTEm_field32_set(unit, &dvp_entry,
                        EN_EFILTERf, 0);
                soc_EGR_DVP_ATTRIBUTEm_field32_set(unit, &dvp_entry,
                        VLAN_MEMBERSHIP_PROFILEf, 0);
                BCM_IF_ERROR_RETURN
                    (WRITE_EGR_DVP_ATTRIBUTEm(unit, MEM_BLOCK_ALL, vp, &dvp_entry));

                /* Remove VP from VP group */
                BCM_IF_ERROR_RETURN
                    (_bcm_td_eg_vp_group_leave(unit, vp, old_vp_group));
            }
        }
    }

cleanup:
    if (vlan_bitmap) {
        sal_free(vlan_bitmap);
    }

    return rv;
}

/*
 * Function:
 *      bcm_td_vp_vlan_member_get
 * Purpose:
 *      Get virtual port ingress and egress filter modes.
 * Parameters:
 *      unit  - (IN) BCM device number
 *      gport - (IN) VP gport id
 *      flags - (OUT) Ingress and egress filter modes.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_td_vp_vlan_member_get(int unit, bcm_gport_t gport, uint32 *flags)
{
    int vp;
    source_vp_entry_t svp_entry;
    int ifilter_en;
    egr_dvp_attribute_entry_t dvp_entry;
    int efilter_en;

    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        vp = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    } else {
        return BCM_E_PARAM;
    }

    *flags = 0;

    if (soc_feature(unit, soc_feature_vp_group_ingress_vlan_membership)) {
        SOC_IF_ERROR_RETURN(READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp_entry));
        ifilter_en = soc_SOURCE_VPm_field32_get(unit, &svp_entry, ENABLE_IFILTERf);
        if (ifilter_en) {
            *flags |= BCM_PORT_VLAN_MEMBER_INGRESS;
        }
    }

    if (soc_feature(unit, soc_feature_vp_group_egress_vlan_membership)) {
        SOC_IF_ERROR_RETURN
            (READ_EGR_DVP_ATTRIBUTEm(unit, MEM_BLOCK_ANY, vp, &dvp_entry));
        efilter_en = soc_EGR_DVP_ATTRIBUTEm_field32_get(unit, &dvp_entry,
                EN_EFILTERf);
        if (efilter_en) {
            *flags |= BCM_PORT_VLAN_MEMBER_EGRESS;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_untagged_add
 * Purpose:
 *      Set VLAN VP tagging/untagging status by adding
 *      a (VLAN VP, VLAN) egress VLAN translation entry.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vlan - (IN) VLAN ID. 
 *      vp   - (IN) Virtual port number.
 *      flags - (IN) Untagging indication.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_untagged_add(int unit, bcm_vlan_t vlan, int vp, int flags)
{
    egr_vlan_xlate_entry_t vent, old_vent;
    bcm_vlan_action_set_t action;
    uint32 profile_idx;
    int rv;

    sal_memset(&vent, 0, sizeof(egr_vlan_xlate_entry_t));

    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, VALIDf, 1);

    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, ENTRY_TYPEf, 1);
    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, DVPf, vp);
    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, OVIDf, vlan);

    if (VLAN_VP_INFO(unit, vp)->flags & BCM_VLAN_PORT_EGRESS_VLAN16) {
        soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, NEW_OTAG_VPTAG_SELf, 1);
        soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, NEW_OTAG_VPTAGf,
                                        VLAN_VP_INFO(unit, vp)->match_vlan);
    } else {
        soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, NEW_OTAG_VPTAG_SELf, 0);
        soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, NEW_OVIDf,
                VLAN_VP_INFO(unit, vp)->match_vlan & 0xfff);
    }

    bcm_vlan_action_set_t_init(&action);
    action.dt_outer = bcmVlanActionReplace;
    action.ot_outer = bcmVlanActionReplace;
    if (flags & BCM_VLAN_PORT_UNTAGGED) {
        action.dt_inner = bcmVlanActionNone;
        action.ot_inner = bcmVlanActionNone;
    } else {
        action.dt_inner = bcmVlanActionCopy;
        action.ot_inner = bcmVlanActionCopy;
    }
    BCM_IF_ERROR_RETURN
        (_bcm_trx_egr_vlan_action_profile_entry_add(unit, &action, &profile_idx));

    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, TAG_ACTION_PROFILE_PTRf,
                                profile_idx);

    rv = soc_mem_insert_return_old(unit, EGR_VLAN_XLATEm, MEM_BLOCK_ALL,
                                   &vent, &old_vent);
    if (rv == SOC_E_EXISTS) {
        /* Delete the old vlan translate profile entry */
        profile_idx = soc_EGR_VLAN_XLATEm_field32_get(unit, &old_vent,
                                                  TAG_ACTION_PROFILE_PTRf);       
        rv = _bcm_trx_egr_vlan_action_profile_entry_delete(unit, profile_idx);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_untagged_delete
 * Purpose:
 *      Delete (VLAN VP, VLAN) egress VLAN translation entry.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vlan - (IN) VLAN ID. 
 *      vp   - (IN) Virtual port number.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_untagged_delete(int unit, bcm_vlan_t vlan, int vp)
{
    egr_vlan_xlate_entry_t vent, old_vent;
    uint32 profile_idx;
    int rv;

    sal_memset(&vent, 0, sizeof(egr_vlan_xlate_entry_t));

    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, ENTRY_TYPEf, 1);
    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, DVPf, vp);
    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, OVIDf, vlan);

    rv = soc_mem_delete_return_old(unit, EGR_VLAN_XLATEm, MEM_BLOCK_ALL,
                                   &vent, &old_vent);
    if ((rv == SOC_E_NONE) &&
            soc_EGR_VLAN_XLATEm_field32_get(unit, &old_vent, VALIDf)) {
        /* Delete the old vlan translate profile entry */
        profile_idx = soc_EGR_VLAN_XLATEm_field32_get(unit, &old_vent,
                                                  TAG_ACTION_PROFILE_PTRf);       
        rv = _bcm_trx_egr_vlan_action_profile_entry_delete(unit, profile_idx);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_untagged_delete_all
 * Purpose:
 *      Delete all (VLAN VP, VLAN) egress VLAN translation entries for a
 *      given VLAN.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vlan - (IN) VLAN ID. 
 *      array_size  - (IN) Number of elements in the virtual port array.
 *      gport_array - (IN) Array of VLAN VP GPORT IDs.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_trident_vlan_vp_untagged_delete_all(int unit, bcm_vlan_t vlan,
        int array_size, bcm_gport_t *gport_array)
{
    int i;
    egr_vlan_xlate_entry_t vent, old_vent;
    int vp;
    uint32 profile_idx;
    int rv = BCM_E_NONE;

    for (i = 0; i < array_size; i++) {

        sal_memset(&vent, 0, sizeof(egr_vlan_xlate_entry_t));

        soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, ENTRY_TYPEf, 1);
        vp = BCM_GPORT_VLAN_PORT_ID_GET(gport_array[i]);
        soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, DVPf, vp);
        soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, OVIDf, vlan);

        rv = soc_mem_delete_return_old(unit, EGR_VLAN_XLATEm, MEM_BLOCK_ALL,
                &vent, &old_vent);
        BCM_IF_ERROR_RETURN(rv);

        if (soc_EGR_VLAN_XLATEm_field32_get(unit, &old_vent, VALIDf)) {
            /* Delete the old vlan translate profile entry */
            profile_idx = soc_EGR_VLAN_XLATEm_field32_get(unit, &old_vent,
                    TAG_ACTION_PROFILE_PTRf);       
            rv = _bcm_trx_egr_vlan_action_profile_entry_delete(unit, profile_idx);
        }
    }

    return rv;
}

/*
 * Function:
 *      _bcm_trident_vlan_vp_untagged_get
 * Purpose:
 *      Get tagging/untagging status of a VLAN virtual port by
 *      reading the (VLAN VP, VLAN) egress vlan translation entry.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      vlan - (IN) VLAN to remove virtual port from.
 *      vp   - (IN) Virtual port number.
 *      flags - (OUT) Untagging status of the VLAN virtual port.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_trident_vlan_vp_untagged_get(int unit, bcm_vlan_t vlan, int vp,
                                  int *flags)
{
    egr_vlan_xlate_entry_t vent, res_vent;
    int idx;
    uint32 profile_idx;
    int rv;
    bcm_vlan_action_set_t action;

    sal_memset(&vent, 0, sizeof(egr_vlan_xlate_entry_t));

    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, ENTRY_TYPEf, 1);
    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, DVPf, vp);
    soc_EGR_VLAN_XLATEm_field32_set(unit, &vent, OVIDf, vlan);

    rv = soc_mem_search(unit, EGR_VLAN_XLATEm, MEM_BLOCK_ALL, &idx,
                                   &vent, &res_vent, 0);
    if ((rv == SOC_E_NONE) &&
            soc_EGR_VLAN_XLATEm_field32_get(unit, &res_vent, VALIDf)) {
        profile_idx = soc_EGR_VLAN_XLATEm_field32_get(unit, &res_vent,
                                                  TAG_ACTION_PROFILE_PTRf);       
        _bcm_trx_egr_vlan_action_profile_entry_get(unit, &action, profile_idx);

        if (bcmVlanActionNone == action.ot_inner) {
            *flags = BCM_VLAN_PORT_UNTAGGED;
        } else {
            *flags = 0;
        }
    }

    return rv;
}

/*
 * Function:
 *      bcm_trident_update_vlan_pbmp
 * Purpose:
 *      Update VLAN table's port bitmap.
 * Parameters:
 *      unit  - (IN) SOC unit number. 
 *      vlan  - (IN) VLAN ID.
 *      pbmp  - (IN) VLAN port bitmap. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_trident_update_vlan_pbmp(int unit, bcm_vlan_t vlan,
        bcm_pbmp_t *pbmp)
{
    int rv = BCM_E_NONE;
    vlan_tab_entry_t vtab;
    egr_vlan_entry_t egr_vtab;

    soc_mem_lock(unit, VLAN_TABm);
    
    rv = soc_mem_read(unit, VLAN_TABm, MEM_BLOCK_ANY, vlan, &vtab);
    if (rv < 0) {
        soc_mem_unlock(unit, VLAN_TABm);
        return rv;
    }

    soc_mem_pbmp_field_set(unit, VLAN_TABm, &vtab, PORT_BITMAPf, pbmp);
    rv = soc_mem_write(unit, VLAN_TABm, MEM_BLOCK_ALL, vlan, &vtab);
    if (rv < 0) {
        soc_mem_unlock(unit, VLAN_TABm);
        return rv;
    }

    soc_mem_unlock(unit, VLAN_TABm);

    soc_mem_lock(unit, EGR_VLANm);

    rv = soc_mem_read(unit, EGR_VLANm, MEM_BLOCK_ANY, vlan, &egr_vtab); 
    if (rv < 0) {
        soc_mem_unlock(unit, EGR_VLANm);
        return rv;
    }

    soc_mem_pbmp_field_set(unit, EGR_VLANm, &egr_vtab, PORT_BITMAPf, pbmp);
    rv = soc_mem_write(unit, EGR_VLANm, MEM_BLOCK_ALL, vlan, &egr_vtab);
    if (rv < 0) {
        soc_mem_unlock(unit, EGR_VLANm);
        return rv;
    }

    soc_mem_unlock(unit, EGR_VLANm);

    return rv;
}

/*
 * Function:
 *      bcm_trident_vlan_gport_add
 * Purpose:
 *      Add a VLAN/NIV virtual port to the specified vlan.
 * Parameters:
 *      unit  - (IN) SOC unit number. 
 *      vlan  - (IN) VLAN ID to add virtual port to as a member.
 *      gport - (IN) VLAN or NIV VP Gport ID
 *      flags - (IN) Indicates if packet should egress out of the given
 *                   VLAN virtual port untagged. Not applicable to NIV. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_trident_vlan_gport_add(int unit, bcm_vlan_t vlan, bcm_gport_t gport,
                           int flags)
{
    int rv = BCM_E_NONE;
    int vp;
    _bcm_vp_type_e vp_type;
    int mc_type;
    bcm_vlan_port_t vlan_vp;
    bcm_niv_port_t niv_port;
    bcm_gport_t phy_port_trunk;
    int is_local;
    vlan_tab_entry_t vtab;
    bcm_pbmp_t vlan_pbmp, vlan_ubmp, l2_pbmp, l3_pbmp, l2_l3_pbmp;
    int i, mc_idx;
    bcm_if_t encap_id;
    bcm_multicast_t group;
    int bc_idx, umc_idx, uuc_idx;
    soc_field_t group_type[3] = {BC_IDXf, UMC_IDXf, UUC_IDXf};

    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        if (flags & ~BCM_VLAN_PORT_UNTAGGED) {
            return BCM_E_PARAM;
        }

        vp = BCM_GPORT_VLAN_PORT_ID_GET(gport);
        vp_type = _bcmVpTypeVlan;
        mc_type = _BCM_MULTICAST_TYPE_VLAN;

        /* Get the physical port or trunk the VP resides on */
        bcm_vlan_port_t_init(&vlan_vp);
        vlan_vp.vlan_port_id = gport;
        BCM_IF_ERROR_RETURN(bcm_trident_vlan_vp_find(unit, &vlan_vp));
        phy_port_trunk = vlan_vp.port;
    } else if (BCM_GPORT_IS_NIV_PORT(gport)) {
        if (flags != 0) {
            /* Untagged is not supported for NIV */
            return BCM_E_PARAM;
        }

        vp = BCM_GPORT_NIV_PORT_ID_GET(gport);
        vp_type = _bcmVpTypeNiv;
        mc_type = _BCM_MULTICAST_TYPE_NIV;

        /* Get the physical port or trunk the VP resides on */
        bcm_niv_port_t_init(&niv_port);
        niv_port.niv_port_id = gport;
        BCM_IF_ERROR_RETURN(bcm_trident_niv_port_get(unit, &niv_port));
        phy_port_trunk = niv_port.port;
    } else {
        return BCM_E_PARAM;
    }

    if (!_bcm_vp_used_get(unit, vp, vp_type)) {
        return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_trident_phy_port_trunk_is_local(unit, phy_port_trunk, &is_local));
    if (!is_local) {
        /* VP is added to replication lists only if it resides on a local port */
        return BCM_E_PORT;
    }

    sal_memset(&vtab, 0, sizeof(vlan_tab_entry_t));

    soc_mem_lock(unit, VLAN_TABm);

    rv = soc_mem_read(unit, VLAN_TABm, MEM_BLOCK_ANY, vlan, &vtab); 
    if (rv < 0) {
        soc_mem_unlock(unit, VLAN_TABm);
        return rv;
    }

    if (!soc_mem_field32_get(unit, VLAN_TABm, &vtab, VALIDf)) {
        soc_mem_unlock(unit, VLAN_TABm);
        return BCM_E_NOT_FOUND;
    }

    /* Enable VP switching on the VLAN */
    if (!soc_mem_field32_get(unit, VLAN_TABm, &vtab, VIRTUAL_PORT_ENf)) {
        soc_mem_field32_set(unit, VLAN_TABm, &vtab, VIRTUAL_PORT_ENf, 1);
        rv = soc_mem_write(unit, VLAN_TABm, MEM_BLOCK_ALL, vlan, &vtab);
        if (rv < 0) {
            soc_mem_unlock(unit, VLAN_TABm);
            return rv;
        }
        soc_mem_unlock(unit, VLAN_TABm);

        /* Also need to copy the physical port members to the L2_BITMAP of
         * the IPMC entry for each group once we've gone virtual */
        rv = mbcm_driver[unit]->mbcm_vlan_port_get
            (unit, vlan, &vlan_pbmp, &vlan_ubmp, NULL);
        if (rv < 0) {
            return rv;
        }

        /* Deal with each group */
        for (i = 0; i < 3; i++) {
            mc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, group_type[i]);
            rv = _bcm_esw_multicast_ipmc_read(unit, mc_idx, &l2_pbmp, &l3_pbmp);
            if (rv < 0) {
                return rv;
            }
            rv = _bcm_esw_multicast_ipmc_write(unit, mc_idx, vlan_pbmp,
                    l3_pbmp, TRUE);
            if (rv < 0) {
                return rv;
            }
        }
    } else { 
        soc_mem_unlock(unit, VLAN_TABm);
    }

    /* Add the VP to the BC group */
    bc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, BC_IDXf);
    _BCM_MULTICAST_GROUP_SET(group, mc_type, bc_idx);
    if (mc_type == _BCM_MULTICAST_TYPE_VLAN) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_vlan_encap_get(unit, group, phy_port_trunk,
                                              gport, &encap_id));
    } else {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_niv_encap_get(unit, group, phy_port_trunk,
                                             gport, &encap_id));
    }
    /* coverity[stack_use_overflow : FALSE] */
    BCM_IF_ERROR_RETURN
        (bcm_esw_multicast_egress_add(unit, group, phy_port_trunk, encap_id)); 

    /* Add the VP to the UMC group, if UMC group is different from BC group */
    umc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, UMC_IDXf);
    _BCM_MULTICAST_GROUP_SET(group, mc_type, umc_idx);
    if (mc_type == _BCM_MULTICAST_TYPE_VLAN) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_vlan_encap_get(unit, group, phy_port_trunk,
                                              gport, &encap_id));
    } else {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_niv_encap_get(unit, group, phy_port_trunk,
                                             gport, &encap_id));
    }
    if (umc_idx != bc_idx) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_egress_add(unit, group, phy_port_trunk, encap_id)); 
    }

    /* Add the VP to the UUC group, if UUC group is different from BC/UMC group */
    uuc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, UUC_IDXf);
    _BCM_MULTICAST_GROUP_SET(group, mc_type, uuc_idx);
    if (mc_type == _BCM_MULTICAST_TYPE_VLAN) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_vlan_encap_get(unit, group, phy_port_trunk,
                                              gport, &encap_id));
    } else {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_niv_encap_get(unit, group, phy_port_trunk,
                                             gport, &encap_id));
    }
    if ((uuc_idx != bc_idx) && (uuc_idx != umc_idx)) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_egress_add(unit, group, phy_port_trunk, encap_id)); 
    }

    /* Update the VLAN table's port bitmap to contain the BC/UMC/UUC groups' 
     * L2 and L3 bitmaps, since the VLAN table's port bitmap is used for
     * ingress and egress VLAN membership checks.
     */

    BCM_PBMP_CLEAR(l2_l3_pbmp);
    for (i = 0; i < 3; i++) {
        mc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, group_type[i]);
        BCM_IF_ERROR_RETURN
            (_bcm_esw_multicast_ipmc_read(unit, mc_idx, &l2_pbmp, &l3_pbmp));
        BCM_PBMP_OR(l2_l3_pbmp, l2_pbmp);
        BCM_PBMP_OR(l2_l3_pbmp, l3_pbmp);
    }

    BCM_IF_ERROR_RETURN(bcm_trident_update_vlan_pbmp(unit, vlan, &l2_l3_pbmp));

    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        BCM_IF_ERROR_RETURN
            (_bcm_trident_vlan_vp_untagged_add(unit, vlan, vp, flags));

        /* Due to change in VP's VLAN membership, may need to move VP to
         * another VP group.
         */
        if (soc_feature(unit, soc_feature_vp_group_ingress_vlan_membership)) {
            BCM_IF_ERROR_RETURN
                (_bcm_td_ing_vp_group_move(unit, vp, vlan, TRUE));
        } 
        if (soc_feature(unit, soc_feature_vp_group_egress_vlan_membership)) {
            BCM_IF_ERROR_RETURN
                (_bcm_td_eg_vp_group_move(unit, vp, vlan, TRUE));
        } 
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_trident_vlan_gport_delete
 * Purpose:
 *      Remove a VLAN/NIV virtual port from the specified vlan.
 * Parameters:
 *      unit  - (IN) SOC unit number. 
 *      vlan  - (IN) VLAN to remove virtual port from.
 *      gport - (IN) VLAN or NIV VP Gport ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_trident_vlan_gport_delete(int unit, bcm_vlan_t vlan, bcm_gport_t gport) 
{
    int rv = BCM_E_NONE;
    int vp;
    _bcm_vp_type_e vp_type;
    int mc_type;
    bcm_vlan_port_t vlan_vp;
    bcm_niv_port_t niv_port;
    bcm_gport_t phy_port_trunk;
    int is_local;
    vlan_tab_entry_t vtab;
    int bc_idx, umc_idx, uuc_idx;
    bcm_multicast_t group;
    bcm_if_t encap_id;
    int i, mc_idx;
    soc_field_t group_type[3] = {BC_IDXf, UMC_IDXf, UUC_IDXf};
    bcm_pbmp_t l2_pbmp, l3_pbmp, l2_l3_pbmp;

    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        vp = BCM_GPORT_VLAN_PORT_ID_GET(gport);
        vp_type = _bcmVpTypeVlan;
        mc_type = _BCM_MULTICAST_TYPE_VLAN;

        /* Get the physical port or trunk the VP resides on */
        bcm_vlan_port_t_init(&vlan_vp);
        vlan_vp.vlan_port_id = gport;
        BCM_IF_ERROR_RETURN(bcm_trident_vlan_vp_find(unit, &vlan_vp));
        phy_port_trunk = vlan_vp.port;

    } else if (BCM_GPORT_IS_NIV_PORT(gport)) {
        vp = BCM_GPORT_NIV_PORT_ID_GET(gport);
        vp_type = _bcmVpTypeNiv;
        mc_type = _BCM_MULTICAST_TYPE_NIV;

        /* Get the physical port or trunk the VP resides on */
        bcm_niv_port_t_init(&niv_port);
        niv_port.niv_port_id = gport;
        BCM_IF_ERROR_RETURN(bcm_trident_niv_port_get(unit, &niv_port));
        phy_port_trunk = niv_port.port;

    } else {
        return BCM_E_PARAM;
    }

    if (!_bcm_vp_used_get(unit, vp, vp_type)) {
        return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_trident_phy_port_trunk_is_local(unit, phy_port_trunk, &is_local));
    if (!is_local) {
        /* VP to be deleted must reside on a local port */
        return BCM_E_PORT;
    }

    sal_memset(&vtab, 0, sizeof(vlan_tab_entry_t));

    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, VLAN_TABm, MEM_BLOCK_ANY, vlan, &vtab)); 

    if (!soc_mem_field32_get(unit, VLAN_TABm, &vtab, VALIDf)) {
        return BCM_E_NOT_FOUND;
    }

    if (!soc_mem_field32_get(unit, VLAN_TABm, &vtab, VIRTUAL_PORT_ENf)) {
        /* VP switching is not enabled on the VLAN */
        return BCM_E_PORT;
    }

    /* Delete the VP from the BC group */
    bc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, BC_IDXf);
    _BCM_MULTICAST_GROUP_SET(group, mc_type, bc_idx);
    if (mc_type == _BCM_MULTICAST_TYPE_VLAN) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_vlan_encap_get(unit, group, phy_port_trunk,
                                              gport, &encap_id));
    } else {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_niv_encap_get(unit, group, phy_port_trunk,
                                             gport, &encap_id));
    }
    BCM_IF_ERROR_RETURN
        (bcm_esw_multicast_egress_delete(unit, group, phy_port_trunk, encap_id)); 

    /* Delete the VP from the UMC group, if UMC group is different
     * from BC group.
     */ 
    umc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, UMC_IDXf);
    _BCM_MULTICAST_GROUP_SET(group, mc_type, umc_idx);
    if (mc_type == _BCM_MULTICAST_TYPE_VLAN) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_vlan_encap_get(unit, group, phy_port_trunk,
                                              gport, &encap_id));
    } else {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_niv_encap_get(unit, group, phy_port_trunk,
                                             gport, &encap_id));
    }
    if (umc_idx != bc_idx) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_egress_delete(unit, group, phy_port_trunk, encap_id)); 
    }

    /* Delete the VP from the UUC group, if UUC group is different
     * from BC and UMC groups.
     */
    uuc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, UUC_IDXf);
    _BCM_MULTICAST_GROUP_SET(group, mc_type, uuc_idx);
    if (mc_type == _BCM_MULTICAST_TYPE_VLAN) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_vlan_encap_get(unit, group, phy_port_trunk,
                                              gport, &encap_id));
    } else {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_niv_encap_get(unit, group, phy_port_trunk,
                                             gport, &encap_id));
    }
    if ((uuc_idx != bc_idx) && (uuc_idx != umc_idx)) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_multicast_egress_delete(unit, group, phy_port_trunk, encap_id)); 
    }

    /* Update the VLAN table's port bitmap to contain the BC/UMC/UUC groups' 
     * L2 and L3 bitmaps, since the VLAN table's port bitmap is used for
     * ingress and egress VLAN membership checks.
     */

    BCM_PBMP_CLEAR(l2_l3_pbmp);
    for (i = 0; i < 3; i++) {
        mc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, group_type[i]);
        rv = _bcm_esw_multicast_ipmc_read(unit, mc_idx, &l2_pbmp, &l3_pbmp);
        if (rv < 0) {
            return rv;
        }
        BCM_PBMP_OR(l2_l3_pbmp, l2_pbmp);
        BCM_PBMP_OR(l2_l3_pbmp, l3_pbmp);
    }

    BCM_IF_ERROR_RETURN
        (bcm_trident_update_vlan_pbmp(unit, vlan, &l2_l3_pbmp));

    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        BCM_IF_ERROR_RETURN
            (_bcm_trident_vlan_vp_untagged_delete(unit, vlan, vp));

        /* Due to change in VP's VLAN membership, may need to move VP to
         * another VP group.
         */
        if (soc_feature(unit, soc_feature_vp_group_ingress_vlan_membership)) {
            BCM_IF_ERROR_RETURN
                (_bcm_td_ing_vp_group_move(unit, vp, vlan, FALSE));
        } 
        if (soc_feature(unit, soc_feature_vp_group_egress_vlan_membership)) {
            BCM_IF_ERROR_RETURN
                (_bcm_td_eg_vp_group_move(unit, vp, vlan, FALSE));
        } 

    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_trident_vlan_gport_get
 * Purpose:
 *      Get untagging status of a VLAN virtual port.
 * Parameters:
 *      unit  - (IN) SOC unit number. 
 *      vlan  - (IN) VLAN to remove virtual port from.
 *      gport - (IN) VLAN VP Gport ID
 *      flags - (OUT) Untagging status of the VLAN virtual port.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_trident_vlan_gport_get(int unit, bcm_vlan_t vlan, bcm_gport_t gport,
        int *flags) 
{
    int rv = BCM_E_NONE;
    int vp;
    _bcm_vp_type_e vp_type;
    int mc_type;
    bcm_vlan_port_t vlan_vp;
    bcm_niv_port_t niv_port;
    bcm_gport_t phy_port_trunk;
    int is_local;
    vlan_tab_entry_t vtab;
    bcm_trunk_t trunk_id;
    bcm_trunk_add_info_t tinfo;
    int idx;
    bcm_module_t mod_out;
    bcm_port_t port_out;
    int mod_local;
    bcm_port_t trunk_member_port[BCM_SWITCH_TRUNK_MAX_PORTCNT];
    int trunk_local_ports = 0;
    int match;
    int if_max;
    bcm_if_t *if_array;
    int if_count, if_cur;
    int i, mc_idx;
    soc_field_t group_type[3] = {BC_IDXf, UMC_IDXf, UUC_IDXf};
    bcm_multicast_t group;
    bcm_if_t encap_id;

    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        vp = BCM_GPORT_VLAN_PORT_ID_GET(gport);
        vp_type = _bcmVpTypeVlan;
        mc_type = _BCM_MULTICAST_TYPE_VLAN;

        /* Get the physical port or trunk the VP resides on */
        bcm_vlan_port_t_init(&vlan_vp);
        vlan_vp.vlan_port_id = gport;
        BCM_IF_ERROR_RETURN(bcm_trident_vlan_vp_find(unit, &vlan_vp));
        phy_port_trunk = vlan_vp.port;

    } else if (BCM_GPORT_IS_NIV_PORT(gport)) {
        vp = BCM_GPORT_NIV_PORT_ID_GET(gport);
        vp_type = _bcmVpTypeNiv;
        mc_type = _BCM_MULTICAST_TYPE_NIV;

        /* Get the physical port or trunk the VP resides on */
        bcm_niv_port_t_init(&niv_port);
        niv_port.niv_port_id = gport;
        BCM_IF_ERROR_RETURN(bcm_trident_niv_port_get(unit, &niv_port));
        phy_port_trunk = niv_port.port;

    } else {
        return BCM_E_PARAM;
    }

    if (!_bcm_vp_used_get(unit, vp, vp_type)) {
        return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_trident_phy_port_trunk_is_local(unit, phy_port_trunk, &is_local));
    if (!is_local) {
        return BCM_E_PORT;
    }

    sal_memset(&vtab, 0, sizeof(vlan_tab_entry_t));

    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, VLAN_TABm, MEM_BLOCK_ANY, vlan, &vtab));

    if (!soc_mem_field32_get(unit, VLAN_TABm, &vtab, VALIDf)) {
        return BCM_E_NOT_FOUND;
    }

    if (!soc_mem_field32_get(unit, VLAN_TABm, &vtab, VIRTUAL_PORT_ENf)) {
        /* No virtual port exists for this VLAN */
        return BCM_E_NOT_FOUND;
    } 

    /* Get all the local ports of the trunk */
    if (BCM_GPORT_IS_TRUNK(phy_port_trunk)) {
        trunk_id = BCM_GPORT_TRUNK_GET(phy_port_trunk);
        rv = _bcm_trunk_id_validate(unit, trunk_id);
        if (BCM_FAILURE(rv)) {
            return BCM_E_PORT;
        }
        rv = bcm_esw_trunk_get(unit, trunk_id, &tinfo);
        if (BCM_FAILURE(rv)) {
            return BCM_E_PORT;
        }
        trunk_local_ports = 0;
        for (idx=0; idx<tinfo.num_ports; idx++) {
            rv = _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                    tinfo.tm[idx], tinfo.tp[idx], &mod_out, &port_out);
            if (BCM_FAILURE(rv)) {
                return BCM_E_PORT;
            }
            /* Convert system port to physical port */
            if (soc_feature(unit, soc_feature_sysport_remap)) {
                BCM_XLATE_SYSPORT_S2P(unit, &port_out);
            }
            rv = _bcm_esw_modid_is_local(unit, mod_out, &mod_local);
            if (BCM_FAILURE(rv)) {
                return BCM_E_PORT;
            }
            if (mod_local) {
                trunk_member_port[trunk_local_ports++] = port_out;
            }
        }
    } 

    /* Determine if the given VP belongs to the VLAN */
    match = FALSE;
    if_max = soc_mem_index_count(unit, EGR_L3_NEXT_HOPm);
    if_array = sal_alloc(if_max * sizeof(bcm_if_t),
            "temp repl interface array");
    if (if_array == NULL) {
        return BCM_E_MEMORY;
    }
    for (i = 0; i < 3; i++) {

        mc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, group_type[i]);
        _BCM_MULTICAST_GROUP_SET(group, mc_type, mc_idx);

        if (mc_type == _BCM_MULTICAST_TYPE_VLAN) {
            rv = bcm_esw_multicast_vlan_encap_get(unit, group, phy_port_trunk,
                                                  gport, &encap_id);
        } else {
            rv = bcm_esw_multicast_niv_encap_get(unit, group, phy_port_trunk,
                                                 gport, &encap_id);
        }
        if (BCM_FAILURE(rv)) {
            sal_free(if_array);
            return rv;
        }

        if (BCM_GPORT_IS_TRUNK(phy_port_trunk)) {
            /* Iterate over all local ports in the trunk and search for
             * a match on any local port
             */
            for (idx = 0; idx < trunk_local_ports; idx++) {
                rv = bcm_esw_ipmc_egress_intf_get(unit, mc_idx, 
                        trunk_member_port[idx], 
                        if_max, if_array, &if_count);
                if (BCM_SUCCESS(rv)) {
                    for (if_cur = 0; if_cur < if_count; if_cur++) {
                        if (if_array[if_cur] == encap_id) {
                            match = TRUE;
                            break;
                        }
                    }
                    if (match) {
                        break;
                    }
                }
            }
            if (match) {
                break;
            }
        } else {
            rv = bcm_esw_ipmc_egress_intf_get(unit, mc_idx, phy_port_trunk, if_max,
                    if_array, &if_count);
            if (BCM_SUCCESS(rv)) {
                for (if_cur = 0; if_cur < if_count; if_cur++) {
                    if (if_array[if_cur] == encap_id) {
                        match = TRUE;
                        break;
                    }
                }
                if (match) {
                    break;
                }
            }
        }
    }
    sal_free(if_array);
    if (!match) {
        rv = BCM_E_NOT_FOUND;
    }

    if (BCM_SUCCESS(rv)) {

        if (BCM_GPORT_IS_VLAN_PORT(gport)) {
            BCM_IF_ERROR_RETURN
                (_bcm_trident_vlan_vp_untagged_get(unit, vlan, vp, flags));
        } else {
            /* Untagged is not supported for NIV */
            *flags = 0;
        }
    }

    return rv;
}

/*
 * Function:
 *      _bcm_trident_mc_group_get_all
 * Purpose:
 *      Get all the next hop indices of the given multicast group.
 * Parameters:
 *      unit   - (IN) SOC unit number. 
 *      mc_group - (IN) Multicast group.
 *      nh_bitmap - (IN/OUT) Bitmap of next hop indices.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_trident_mc_group_get_all(int unit, bcm_multicast_t mc_group,
        SHR_BITDCL *nh_bitmap)
{
    int rv;
    int num_encap_id;
    bcm_if_t *encap_id_array = NULL;
    int i;
    int nh_index;

    /* Get the number of encap IDs of the multicast group */
    BCM_IF_ERROR_RETURN
        (bcm_esw_multicast_egress_get(unit, mc_group, 0,
                                      NULL, NULL, &num_encap_id));

    /* Get all the encap IDs of the multicast group */

    encap_id_array = sal_alloc(sizeof(bcm_if_t) * num_encap_id,
            "encap_id_array");
    if (NULL == encap_id_array) {
        return BCM_E_MEMORY;
    }
    sal_memset(encap_id_array, 0, sizeof(bcm_if_t) * num_encap_id);

    rv = bcm_esw_multicast_egress_get(unit, mc_group, num_encap_id,
                                      NULL, encap_id_array, &num_encap_id);
    if (rv < 0) {
        sal_free(encap_id_array);
        return rv;
    }

    /* Convert encap IDs to next hop indices */

    for (i = 0; i < num_encap_id; i++) {
        if (BCM_IF_INVALID != encap_id_array[i]) {
            nh_index = encap_id_array[i] - BCM_XGS3_DVP_EGRESS_IDX_MIN;
            SHR_BITSET(nh_bitmap, nh_index);
        }
    }

    sal_free(encap_id_array);
    return rv;
}

/*
 * Function:
 *      bcm_trident_vlan_gport_get_all
 * Purpose:
 *      Get all virtual ports of the given VLAN.
 * Parameters:
 *      unit      - (IN) SOC unit number. 
 *      vlan      - (IN) VLAN ID.
 *      array_max - (IN) Max number of elements in the array.
 *      gport_array       - (OUT) Array of virtual ports in GPORT format.
 *      flags_array       - (OUT) Array of untagging status.
 *      array_size        - (OUT) Actual number of elements in the array.
 * Returns:
 *      BCM_E_XXX
 * Notes: If array_max == 0 and gport_array == NULL, actual number of
 *        virtual ports in the VLAN will be returned in array_size.
 */
int
bcm_trident_vlan_gport_get_all(int unit, bcm_vlan_t vlan, int array_max,
        bcm_gport_t *gport_array, int *flags_array, int *array_size) 
{
    vlan_tab_entry_t vtab;
    int        nh_tbl_size;
    SHR_BITDCL *nh_bitmap = NULL;
    int        bc_idx, umc_idx, uuc_idx;
    bcm_multicast_t mc_group;
    int        rv;
    int        i;
    egr_l3_next_hop_entry_t egr_nh;
    int        dvp;

    if (array_max < 0) {
        return BCM_E_PARAM;
    }

    if ((array_max > 0) && (NULL == gport_array)) {
        return BCM_E_PARAM;
    }

    if (NULL == array_size) {
        return BCM_E_PARAM;
    }

    *array_size = 0;

    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, VLAN_TABm, MEM_BLOCK_ANY, (int)vlan, &vtab));

    if (!soc_mem_field32_get(unit, VLAN_TABm, &vtab, VIRTUAL_PORT_ENf)) {
        /* This vlan has no virtual port members. */
        return BCM_E_NONE;
    } 

    nh_tbl_size = soc_mem_index_count(unit, ING_L3_NEXT_HOPm);
    nh_bitmap = sal_alloc(SHR_BITALLOCSIZE(nh_tbl_size), "nh_bitmap");
    if (NULL == nh_bitmap) {
        return BCM_E_MEMORY;
    }
    sal_memset(nh_bitmap, 0, SHR_BITALLOCSIZE(nh_tbl_size));

    /* Get VPs from BC group */
    bc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, BC_IDXf);
    rv = _bcm_tr_multicast_ipmc_group_type_get(unit, bc_idx, &mc_group);
    if (BCM_FAILURE(rv)) {
        goto cleanup;
    }
    rv = _bcm_trident_mc_group_get_all(unit, mc_group, nh_bitmap);
    if (BCM_FAILURE(rv)) {
        goto cleanup;
    }

    /* Get VPs from UMC group, if different from BC group */
    umc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, UMC_IDXf);
    rv = _bcm_tr_multicast_ipmc_group_type_get(unit, umc_idx, &mc_group);
    if (BCM_FAILURE(rv)) {
        goto cleanup;
    }
    if (umc_idx != bc_idx) {
        rv = _bcm_trident_mc_group_get_all(unit, mc_group, nh_bitmap);
        if (BCM_FAILURE(rv)) {
            goto cleanup;
        }
    }

    /* Get VPs from UUC group, if different from BC and UMC group */
    uuc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, UUC_IDXf);
    rv = _bcm_tr_multicast_ipmc_group_type_get(unit, uuc_idx, &mc_group);
    if (BCM_FAILURE(rv)) {
        goto cleanup;
    }
    if ((uuc_idx != bc_idx) && (uuc_idx != umc_idx)) {
        rv = _bcm_trident_mc_group_get_all(unit, mc_group, nh_bitmap);
        if (BCM_FAILURE(rv)) {
            goto cleanup;
        }
    }

    /* Convert next hop indices to VLAN VP gports */
    for (i = 0; i < nh_tbl_size; i++) {
        if (SHR_BITGET(nh_bitmap, i)) {
            if (0 == array_max) {
                (*array_size)++;
            } else {
                rv = READ_EGR_L3_NEXT_HOPm(unit, MEM_BLOCK_ANY, i, &egr_nh); 
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }
                dvp = soc_EGR_L3_NEXT_HOPm_field32_get(unit, &egr_nh, SD_TAG__DVPf);
                if (_BCM_MULTICAST_IS_VLAN(mc_group)) {
                    BCM_GPORT_VLAN_PORT_ID_SET(gport_array[*array_size], dvp);
                    if (NULL != flags_array) {
                        rv = _bcm_trident_vlan_vp_untagged_get(unit, vlan, dvp,
                                &flags_array[*array_size]);
                        if (BCM_FAILURE(rv)) {
                            goto cleanup;
                        }
                    }
                } else {
                    BCM_GPORT_NIV_PORT_ID_SET(gport_array[*array_size], dvp);
                    if (NULL != flags_array) {
                        flags_array[*array_size] = 0;
                    }
                }
                (*array_size)++;
                if (*array_size == array_max) {
                    break;
                }
            }
        }
    }

cleanup:
    if (NULL != nh_bitmap) {
        sal_free(nh_bitmap);
    }

    return rv;
}

/*
 * Function:
 *      bcm_trident_vlan_gport_delete_all
 * Purpose:
 *      Remove all VLAN virtual ports from the specified vlan.
 * Parameters:
 *      unit  - (IN) SOC unit number. 
 *      vlan  - (IN) VLAN to remove virtual ports from.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_trident_vlan_gport_delete_all(int unit, bcm_vlan_t vlan) 
{
    vlan_tab_entry_t vtab;
    int num_vp;
    bcm_gport_t *gport_array = NULL;
    int rv = BCM_E_NONE;
    int bc_idx, umc_idx, uuc_idx;
    bcm_multicast_t mc_group;

    sal_memset(&vtab, 0, sizeof(vlan_tab_entry_t));

    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, VLAN_TABm, MEM_BLOCK_ANY, vlan, &vtab)); 

    if (!soc_mem_field32_get(unit, VLAN_TABm, &vtab, VALIDf)) {
        return BCM_E_NOT_FOUND;
    }

    if (soc_mem_field32_get(unit, VLAN_TABm, &vtab, VIRTUAL_PORT_ENf)) {

        /* Get all VPs from the VLAN before deletion */

        BCM_IF_ERROR_RETURN
            (bcm_trident_vlan_gport_get_all(unit, vlan, 0, NULL, NULL, &num_vp));

        gport_array = sal_alloc(sizeof(bcm_gport_t) * num_vp,
                "vlan vp gport array");
        if (NULL == gport_array) {
            return BCM_E_MEMORY;
        }
        sal_memset(gport_array, 0, sizeof(bcm_gport_t) * num_vp);

        rv = bcm_trident_vlan_gport_get_all(unit, vlan, num_vp,
                gport_array, NULL, &num_vp);
        if (rv < 0) {
            goto cleanup;
        }

        /* Delete all VPs from the BC group */

        bc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, BC_IDXf);
        rv = _bcm_tr_multicast_ipmc_group_type_get(unit, bc_idx, &mc_group);
        if (rv < 0) {
            goto cleanup;
        }
        rv = bcm_esw_multicast_egress_delete_all(unit, mc_group); 
        if (rv < 0) {
            goto cleanup;
        }
        rv = bcm_esw_multicast_destroy(unit, mc_group);
        if (rv < 0) {
            goto cleanup;
        }

        /* Delete all VPs from the UMC group, if UMC group is different
         * from BC group.
         */ 

        umc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, UMC_IDXf);
        if (umc_idx != bc_idx) {
            rv = _bcm_tr_multicast_ipmc_group_type_get(unit, umc_idx, &mc_group);
            if (rv < 0) {
                goto cleanup;
            }
            rv = bcm_esw_multicast_egress_delete_all(unit, mc_group); 
            if (rv < 0) {
                goto cleanup;
            }
            rv = bcm_esw_multicast_destroy(unit, mc_group);
            if (rv < 0) {
                goto cleanup;
            }
        }

        /* Delete all VPs from the UUC group, if UUC group is different
         * from BC and UMC groups.
         */
        uuc_idx = soc_mem_field32_get(unit, VLAN_TABm, &vtab, UUC_IDXf);
        if ((uuc_idx != bc_idx) && (uuc_idx != umc_idx)) {
            rv = _bcm_tr_multicast_ipmc_group_type_get(unit, uuc_idx, &mc_group);
            if (rv < 0) {
                goto cleanup;
            }
            rv = bcm_esw_multicast_egress_delete_all(unit, mc_group); 
            if (rv < 0) {
                goto cleanup;
            }
            rv = bcm_esw_multicast_destroy(unit, mc_group);
            if (rv < 0) {
                goto cleanup;
            }
        }

        soc_mem_field32_set(unit, VLAN_TABm, &vtab, VIRTUAL_PORT_ENf, 0);
        soc_mem_field32_set(unit, VLAN_TABm, &vtab, BC_IDXf, 0);
        soc_mem_field32_set(unit, VLAN_TABm, &vtab, UMC_IDXf, 0);
        soc_mem_field32_set(unit, VLAN_TABm, &vtab, UUC_IDXf, 0);
        rv = soc_mem_write(unit, VLAN_TABm, MEM_BLOCK_ALL, (int)vlan, &vtab);
        if (rv < 0) {
            goto cleanup;
        }

        if (_BCM_MULTICAST_IS_VLAN(mc_group)) {
            /* Delete all DVP tagging/untagging entries */
            rv = _bcm_trident_vlan_vp_untagged_delete_all(unit, vlan,
                    num_vp, gport_array);
            if (rv < 0) {
                goto cleanup;
            }

            /* Delete VP groups from VLAN */
            if (soc_feature(unit, soc_feature_vp_group_ingress_vlan_membership)) {
                rv = _bcm_td_ing_vp_group_vlan_delete_all(unit, vlan);
                if (rv < 0) {
                    goto cleanup;
                }
            } 
            if (soc_feature(unit, soc_feature_vp_group_egress_vlan_membership)) {
                rv = _bcm_td_eg_vp_group_vlan_delete_all(unit, vlan);
                if (rv < 0) {
                    goto cleanup;
                }
            } 
        }
    }

cleanup:
    if (NULL != gport_array) {
        sal_free(gport_array);
    }
    return rv;
}

#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */
