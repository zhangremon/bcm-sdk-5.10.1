/*
 * $Id: stack.c 1.295.2.7 Broadcom SDK $
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
 * File:        stack.c
 * Purpose:     BCM level APIs for stacking applications
 */

#include <sal/core/sync.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/profile_mem.h>
#include <soc/draco.h>
#include <soc/firebolt.h>
#include <soc/tucana.h>

#include <bcm/error.h>
#include <bcm/stack.h>
#include <bcm/debug.h>

#include <bcm_int/control.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/vlan.h>
#include <bcm_int/esw/ipmc.h>
#include <bcm_int/esw/mcast.h>
#include <bcm_int/esw/rx.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/trident.h>

#include <bcm_int/esw_dispatch.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/switch.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
#define LOCAL_MODMAP_SUPPORT
STATIC int _bcm_esw_modmap_init(int unit);
#endif /* BCM_HERCULES15_SUPPORT */

#define STK_DEBUG(flags, stuff) BCM_DEBUG(BCM_DBG_STK + (flags), stuff)
#define STK_WARN(stuff) STK_DEBUG(BCM_DBG_WARN, stuff)
#define STK_ERR(stuff) STK_DEBUG(BCM_DBG_ERR, stuff)
#define STK_VERB(stuff) STK_DEBUG(BCM_DBG_VERBOSE, stuff)
#define STK_OUT(stuff) STK_DEBUG(0, stuff)

static sal_mutex_t stack_mutex;

#define ESW_STK_LOCK sal_mutex_take(stack_mutex, sal_mutex_FOREVER)
#define ESW_STK_UNLOCK sal_mutex_give(stack_mutex)

#ifdef _BCM_STK_FLAGS_GET
STATIC int _esw_stk_flags_get(int unit, bcm_port_t port, uint32 *flags);
#endif

STATIC int _esw_stk_sl_mode_enable_set(int unit, int enable);
STATIC int _esw_stk_port_set(int unit, bcm_port_t port, uint32 flags);

STATIC int _bcm_xgs12_stk_init(int unit);
STATIC int _bcm_xgs_fabric_stk_init(int unit);
STATIC int _bcm_xgs3_stk_init(int unit);
STATIC int _bcm_xgs3_ignore_my_modid(int unit);

#if defined(BCM_TRX_SUPPORT)
STATIC int _bcm_stk_modport_profile_init(int unit);
STATIC soc_profile_mem_t *modport_profile[BCM_MAX_NUM_UNITS];
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)

/* In XGS3 devices prior to Trident, tables that contain per (modid, port)
 * information, such as the SOURCE_TRUNK_MAP table, contain
 * (SOC_MODID_MAX + 1) * (SOC_PORT_ADDR_MAX + 1) entries. The index to such
 * tables is formed by modid * (SOC_PORT_ADDR_MAX + 1) + port. In Trident,
 * the index is computed differently. First, the source modid is used to index
 * a table that gives a base index, such as the SOURCE_TRUNK_MAP_MODBASE table.
 * The base index is then added to port ID to derive the final index.
 */

typedef struct src_modid_base_index_bookkeeping_s {
    uint8 *num_ports; /* An array keeping track of number of ports per module */
    SHR_BITDCL *src_mod_port_table_bitmap;
                      /* Keeps track of which entries are used in tables that
                         are indexed by source modid base index and port */
} src_modid_base_index_bookkeeping_t;

STATIC src_modid_base_index_bookkeeping_t *src_modid_base_index_bk[BCM_MAX_NUM_UNITS];

#define _BCM_SRC_MOD_PORT_TABLE_USED_GET(_u_, _idx_) \
    SHR_BITGET(src_modid_base_index_bk[_u_]->src_mod_port_table_bitmap, _idx_)
#define _BCM_SRC_MOD_PORT_TABLE_USED_SET(_u_, _idx_) \
    SHR_BITSET(src_modid_base_index_bk[_u_]->src_mod_port_table_bitmap, _idx_)
#define _BCM_SRC_MOD_PORT_TABLE_USED_CLR(_u_, _idx_) \
    SHR_BITCLR(src_modid_base_index_bk[_u_]->src_mod_port_table_bitmap, _idx_)
#define _BCM_SRC_MOD_PORT_TABLE_USED_SET_RANGE(_u_, _idx_, _count_) \
    SHR_BITSET_RANGE(src_modid_base_index_bk[_u_]->src_mod_port_table_bitmap, _idx_, _count_)
#define _BCM_SRC_MOD_PORT_TABLE_USED_CLR_RANGE(_u_, _idx_, _count_) \
    SHR_BITCLR_RANGE(src_modid_base_index_bk[_u_]->src_mod_port_table_bitmap, _idx_, _count_)
#define _BCM_SRC_MOD_PORT_TABLE_TEST_RANGE(_u_, _idx_, _count_, _result_) \
    SHR_BITTEST_RANGE(src_modid_base_index_bk[_u_]->src_mod_port_table_bitmap, _idx_, _count_, _result_)

STATIC soc_mem_t src_modbase_mems[] = {
    SOURCE_TRUNK_MAP_MODBASEm,
    EGR_GPP_ATTRIBUTES_MODBASEm,
}; 

STATIC soc_mem_t src_mod_port_mems[] = {
    SOURCE_TRUNK_MAP_TABLEm,
    EGR_GPP_ATTRIBUTESm,
};

STATIC int _src_modid_base_index_bk_init(int unit);
STATIC void _src_modid_base_index_bk_detach(int unit);

#endif /* BCM_TRIDENT_SUPPORT */

#if defined(LOCAL_MODMAP_SUPPORT)
#define _NUM_SL_MOD         64
#define _MAX_SPORT          31
#define _XGS3_MODMAP_GROUPS_MAX  1
#define _XGS3_MODMAP_FMOD_MAX   127
#define _XGS4_FABRIC_MODMAP_FMOD_MAX   255
#define FMOD_MAX(unit)  \
        (SOC_IS_SC_CQ(unit) ? _XGS4_FABRIC_MODMAP_FMOD_MAX : \
                              _XGS3_MODMAP_FMOD_MAX)
#define FMOD_RANGE_CHECK(a) \
        if (((a) < 0) || ((a) > FMOD_MAX(unit))) return(BCM_E_PARAM)
#define LMOD_RANGE_CHECK(a) if (((a) < 0) || ((a) > 63))  return(BCM_E_PARAM)
#define SMOD_RANGE_CHECK(a) if (((a) < 0) || ((a) > 63))  return(BCM_E_PARAM)
#define FMOD_LMOD_CHECK(fm, lm) if (fm < lm) return(BCM_E_PARAM)
#define SPORT_RANGE_CHECK(a) if ((a) > _MAX_SPORT)  return(BCM_E_PARAM)
#define BCM_MODMAP_FEATURE_CHECK(unit, f) \
        do { \
            if (!soc_feature(unit, soc_feature_modmap)) { \
                return BCM_E_UNAVAIL; \
            } \
        } while(0)
#define MOD_MAP_DATA_CHECK(unit) \
        if (mod_map_data[unit] == NULL)  return(BCM_E_INIT)
/*
 * One entry for each SOC device containing modmap information
 * for that device.
 */
typedef struct mod_map_ctrl {
    int         l_modid[SOC_MAX_NUM_PORTS];
    int         thresh_d[_NUM_SL_MOD];
    int         fmod0[SOC_MAX_NUM_PORTS];
} mod_map_ctrl_t;

STATIC mod_map_ctrl_t *mod_map_data[BCM_MAX_NUM_UNITS];


STATIC int
_bcm_esw_modmap_init(int unit)
{
    if (NULL == mod_map_data[unit]) {
        mod_map_data[unit] =
            sal_alloc (sizeof (mod_map_ctrl_t), "modmap_state");
    }
    if (NULL == mod_map_data[unit]) {
        return (BCM_E_MEMORY);
    }

    sal_memset(mod_map_data[unit], 0, sizeof (mod_map_ctrl_t));

    return BCM_E_NONE;
}

STATIC int
_bcm_esw_modmap_detach(int unit)
{
    if (NULL == mod_map_data[unit]) {
        return (BCM_E_NONE);
    }
    sal_free(mod_map_data[unit]);

    mod_map_data[unit] = NULL;
    return BCM_E_NONE;
}
#endif  /* MODMAP_SUPPORT */


/*
 * Internal init (alloc the mutex).  Unit is ignored (for now)
 */

STATIC int
_bcm_esw_stk_init(void)
{
    if (stack_mutex == NULL) {
        if ((stack_mutex = sal_mutex_create("bcm_esw_lock")) == NULL) {
            return BCM_E_MEMORY;
        }
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_stk_detach
 * Purpose:
 *      De-initialize device stacking
 * Parameters:
 *      unit  - (IN) BCM device number.
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_esw_stk_detach(int unit)
{
    if (stack_mutex != NULL) {
        sal_mutex_destroy(stack_mutex);
        stack_mutex = NULL;
    }

    BCM_IF_ERROR_RETURN(bcm_esw_stk_modmap_unregister(unit));

#if defined(LOCAL_MODMAP_SUPPORT)
    if (soc_feature(unit, soc_feature_modmap)) {
        SOC_IF_ERROR_RETURN(_bcm_esw_modmap_detach(unit));
    }
#endif /* MODMAP_SUPPORT */

#if defined(BCM_TRX_SUPPORT)
    if (modport_profile[unit]) {
        soc_profile_mem_destroy(unit, modport_profile[unit]);
        sal_free(modport_profile[unit]);
        modport_profile[unit] = NULL;
    }
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {
        _src_modid_base_index_bk_detach(unit);
    }
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        BCM_IF_ERROR_RETURN(bcm_td_modport_map_detach(unit));
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_NONE;
}


#ifdef BCM_WARM_BOOT_SUPPORT

#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_VERSION_1_1                SOC_SCACHE_VERSION(1,1)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_1

/*
 * Function:
 *      _bcm_esw_stk_sync
 * Purpose:
 *      Record STACK module persisitent info for Level 2 Warm Boot
 * Parameters:
 *      unit - Unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_stk_sync(int unit)
{
    soc_scache_handle_t scache_handle;
    uint8               *stk_scache_ptr;

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_STACK, 0);
    BCM_IF_ERROR_RETURN
        (_bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0, &stk_scache_ptr, 
                                 BCM_WB_DEFAULT_VERSION, NULL));
    /* Stack port bitmaps */
    sal_memcpy(stk_scache_ptr, &SOC_PBMP_STACK_CURRENT(unit),
               sizeof(soc_pbmp_t));
    stk_scache_ptr += sizeof(soc_pbmp_t);
    sal_memcpy(stk_scache_ptr, &SOC_PBMP_STACK_INACTIVE(unit),
               sizeof(soc_pbmp_t));
    stk_scache_ptr += sizeof(soc_pbmp_t);
    sal_memcpy(stk_scache_ptr, &SOC_PBMP_STACK_PREVIOUS(unit),
               sizeof(soc_pbmp_t));
    stk_scache_ptr += sizeof(soc_pbmp_t);

#if defined(LOCAL_MODMAP_SUPPORT)
    /* Modmap */
    if (soc_feature(unit, soc_feature_modmap)) {
        int    i;
        int    port;
        int    max_group;
        uint8  val;

        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            val = mod_map_data[unit]->l_modid[port];
            sal_memcpy(stk_scache_ptr, &val, sizeof(val));
            stk_scache_ptr += sizeof(val);
        }

        for (i = 0; i < _NUM_SL_MOD; i++) {
            val = mod_map_data[unit]->thresh_d[i];
            sal_memcpy(stk_scache_ptr, &val, sizeof(val));
            stk_scache_ptr += sizeof(val);
        }

        BCM_IF_ERROR_RETURN
            (bcm_esw_stk_port_modmap_group_max_get(unit, &max_group));
        for (i = 0; i <= max_group; i++) {
            val = mod_map_data[unit]->fmod0[i];
            sal_memcpy(stk_scache_ptr, &val, sizeof(val));
            stk_scache_ptr += sizeof(val);
        }
    }
#endif /* LOCAL_MODMAP_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {
        int i;
        uint8 val;

        for (i = 0; i <= SOC_MODID_MAX(unit); i++) {
            val = src_modid_base_index_bk[unit]->num_ports[i];
            sal_memcpy(stk_scache_ptr, &val, sizeof(val));
            stk_scache_ptr += sizeof(val);
        }
    }

    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        BCM_IF_ERROR_RETURN(bcm_td_modport_map_sync(unit, &stk_scache_ptr));
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_stk_reinit
 * Purpose:
 *      Recover STACK software state.
 * Parameters:
 *      unit - Unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_stk_reinit(int unit)
{
    int                 rv;
    soc_scache_handle_t scache_handle;
    uint8               *stk_scache_ptr;
    uint16              recovered_ver;
 	 
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_STACK, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0, &stk_scache_ptr, 
                                 BCM_WB_DEFAULT_VERSION, &recovered_ver);

    if (rv == BCM_E_NOT_FOUND) {
        stk_scache_ptr = NULL;
    } else if (BCM_FAILURE(rv)) {
        return rv;
    }

    if (stk_scache_ptr != NULL) {
        /* Stack port bitmaps */
        sal_memcpy(&SOC_PBMP_STACK_CURRENT(unit), stk_scache_ptr,
                   sizeof(soc_pbmp_t));
        stk_scache_ptr += sizeof(soc_pbmp_t);
        sal_memcpy(&SOC_PBMP_STACK_INACTIVE(unit), stk_scache_ptr,
                   sizeof(soc_pbmp_t));
        stk_scache_ptr += sizeof(soc_pbmp_t);
        sal_memcpy(&SOC_PBMP_STACK_PREVIOUS(unit), stk_scache_ptr,
                   sizeof(soc_pbmp_t));
        stk_scache_ptr += sizeof(soc_pbmp_t);
    } else {
        BCM_PBMP_CLEAR(SOC_PBMP_STACK_CURRENT(unit));
        BCM_PBMP_CLEAR(SOC_PBMP_STACK_INACTIVE(unit));
        BCM_PBMP_CLEAR(SOC_PBMP_STACK_PREVIOUS(unit));
    }
    
    BCM_IF_ERROR_RETURN(_bcm_esw_stk_init());

#if defined(LOCAL_MODMAP_SUPPORT)
    /* Modmap */
    if (soc_feature(unit, soc_feature_modmap)) {
        int    i;
        int    port;
        int    max_group;
        uint8  val;

        BCM_IF_ERROR_RETURN(_bcm_esw_modmap_init(unit));

        /* Recover l_modid, thresh_d, and fmod0 */
        if (stk_scache_ptr != NULL) {
            for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
                sal_memcpy(&val, stk_scache_ptr, sizeof(val));
                mod_map_data[unit]->l_modid[port] = val;
                stk_scache_ptr += sizeof(val);
            }

            for (i = 0; i < _NUM_SL_MOD; i++) {
                sal_memcpy(&val, stk_scache_ptr, sizeof(val));
                mod_map_data[unit]->thresh_d[i] = val;
                stk_scache_ptr += sizeof(val);
            }

            BCM_IF_ERROR_RETURN
                (bcm_esw_stk_port_modmap_group_max_get(unit, &max_group));
            for (i = 0; i <= max_group; i++) {
                sal_memcpy(&val, stk_scache_ptr, sizeof(val));
                mod_map_data[unit]->fmod0[i] = val;
                stk_scache_ptr += sizeof(val);
            }
        }
    }
#endif /* LOCAL_MODMAP_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {
        int i;
        uint8 val;
        uint8 *stm_modbase_buf = NULL;
        source_trunk_map_modbase_entry_t *stm_modbase_entry;
        int index_min, index_max, base, base_max, num_ports;

        BCM_IF_ERROR_RETURN(_src_modid_base_index_bk_init(unit));

        stm_modbase_buf = soc_cm_salloc(unit,
                SOC_MEM_TABLE_BYTES(unit, SOURCE_TRUNK_MAP_MODBASEm),
                "SOURCE_TRUNK_MAP_MODBASE buffer");
        if (NULL == stm_modbase_buf) {
            return BCM_E_MEMORY;
        }

        index_min = soc_mem_index_min(unit, SOURCE_TRUNK_MAP_MODBASEm);
        index_max = soc_mem_index_max(unit, SOURCE_TRUNK_MAP_MODBASEm);
        rv = soc_mem_read_range(unit, SOURCE_TRUNK_MAP_MODBASEm,
                MEM_BLOCK_ANY, index_min, index_max, stm_modbase_buf);
        if (SOC_FAILURE(rv)) {
            soc_cm_sfree(unit, stm_modbase_buf);
            return rv;
        }

        if (stk_scache_ptr != NULL) {

            /* Recover src_modid_base_index_bk[unit]->num_ports array */
            for (i = 0; i <= SOC_MODID_MAX(unit); i++) {
                sal_memcpy(&val, stk_scache_ptr, sizeof(val));
                src_modid_base_index_bk[unit]->num_ports[i] = val;
                stk_scache_ptr += sizeof(val);
            }

            /* Recover src_modid_base_index_bk[unit]->src_mod_port_table_bitmap */
            for (i = index_min; i <= index_max; i++) {
                stm_modbase_entry = soc_mem_table_idx_to_pointer
                    (unit, SOURCE_TRUNK_MAP_MODBASEm,
                     source_trunk_map_modbase_entry_t *, stm_modbase_buf, i); 

                base = soc_SOURCE_TRUNK_MAP_MODBASEm_field32_get(unit,
                        stm_modbase_entry, BASEf);
                if (0 == base) {
                    continue;
                }

                num_ports = src_modid_base_index_bk[unit]->num_ports[i];
                _BCM_SRC_MOD_PORT_TABLE_USED_SET_RANGE(unit, base, num_ports);

                /* Recover source trunk mapping cache and trunk members
                 * that have EGRESS_DISABLE flags set.
                 */
                rv = _bcm_trident_trunk_mod_port_map_reinit(unit, i, base,
                        num_ports);
                if (SOC_FAILURE(rv)) {
                    soc_cm_sfree(unit, stm_modbase_buf);
                    return rv;
                }
            }
        } else {
            /* In Level 1 recovery of src_modid_base_index_bk[unit]->
             * src_mod_port_table_bitmap, it's not known how many
             * SOURCE_TRUNK_MAP entries each module ID occupies.
             * Hence, we conservatively assume that all
             * SOURCE_TRUNK_MAP entries from index 0 to (highest
             * SOURCE_TRUNK_MAP_MODBASE.BASE + SOC_MAX_NUM_PORTS - 1)
             * are used.
             */
            base_max = 0;
            for (i = index_min; i <= index_max; i++) {
                stm_modbase_entry = soc_mem_table_idx_to_pointer
                    (unit, SOURCE_TRUNK_MAP_MODBASEm,
                     source_trunk_map_modbase_entry_t *, stm_modbase_buf, i); 
                base = soc_SOURCE_TRUNK_MAP_MODBASEm_field32_get(unit,
                        stm_modbase_entry, BASEf);
                if (base > base_max) {
                    base_max = base;
                }
            }
            _BCM_SRC_MOD_PORT_TABLE_USED_SET_RANGE(unit, 0,
                    (base_max + SOC_MAX_NUM_PORTS));
        }

        soc_cm_sfree(unit, stm_modbase_buf);
    }
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_TRX_SUPPORT)
    /* Modport profile */
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        BCM_IF_ERROR_RETURN(bcm_td_modport_map_init(unit));
        if (stk_scache_ptr != NULL) {
            if (recovered_ver >= BCM_WB_VERSION_1_1) { 
                /* Retrieve modport map profile info from scache */
                BCM_IF_ERROR_RETURN(bcm_td_modport_map_reinit(unit,
                            &stk_scache_ptr));
            } else {
                uint8 *null_scache_ptr = NULL;
                int modport_map_scache_size;

                /* Retrieve modport map profile info from hardware */
                BCM_IF_ERROR_RETURN(bcm_td_modport_map_reinit(unit,
                            &null_scache_ptr));
                BCM_IF_ERROR_RETURN(bcm_td_modport_map_scache_size_get(unit,
                            &modport_map_scache_size));
                SOC_IF_ERROR_RETURN(soc_scache_realloc(unit, scache_handle,
                            modport_map_scache_size));
            }
        } else { /* Level 1 warm boot */
            uint8 *null_scache_ptr = NULL;

            /* Retrieve modport map profile info from hardware */
            BCM_IF_ERROR_RETURN(bcm_td_modport_map_reinit(unit,
                        &null_scache_ptr));
        }
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    if (SOC_IS_TR_VL(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_stk_modport_profile_init(unit));
    }
#endif /* BCM_TRX_SUPPORT */

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *      bcm_stk_init
 * Purpose:
 *      Initialize device stacking
 * Parameters:
 *      unit  - SOC unit #
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_stk_init(int unit)
{
    int rv;
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_scache_handle_t scache_handle;
    uint32              stk_scache_size;
    uint8               *stk_scache_ptr;
#endif /* BCM_WARM_BOOT_SUPPORT */

    STK_VERB(("STK %d: Init\n", unit));

    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    if (SOC_IS_SHADOW(unit)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    /* Warm boot level 2 cache size */
    /* Stack ports current, inactive, previous */
    stk_scache_size = sizeof(soc_pbmp_t) * 3;

#if defined(LOCAL_MODMAP_SUPPORT)
    /* Modmap */
    if (soc_feature(unit, soc_feature_modmap)) {
        int max_group;

        BCM_IF_ERROR_RETURN
        (bcm_esw_stk_port_modmap_group_max_get(unit, &max_group));
        
        /* Max values for l_modid, thresh_d, fmod0 fits in uint8 */
        stk_scache_size += (sizeof(uint8) * SOC_MAX_NUM_PORTS);  /* l_modid  */
        stk_scache_size += (sizeof(uint8) * _NUM_SL_MOD);        /* thresh_d */
        stk_scache_size += (sizeof(uint8) * (max_group + 1));    /* fmod0    */
    }
#endif /* LOCAL_MODMAP_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {
        /* Allocate space for src_modid_base_index_bk[unit]->num_ports array */
        stk_scache_size += (sizeof(uint8) * (SOC_MODID_MAX(unit) + 1));
    }
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        int modport_map_scache_size;
        BCM_IF_ERROR_RETURN
            (bcm_td_modport_map_scache_size_get(unit, &modport_map_scache_size));
        stk_scache_size += modport_map_scache_size;
    }
#endif /* BCM_TRIDENT_SUPPORT */

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_STACK, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle,
                                 (0 == SOC_WARM_BOOT(unit)),
                                 stk_scache_size, &stk_scache_ptr, 
                                 BCM_WB_DEFAULT_VERSION, NULL);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    rv = _bcm_esw_stk_detach(unit);
    BCM_IF_ERROR_RETURN(rv);

    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RCPU_ONLY(unit)) {
        if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
            SOC_IF_ERROR_RETURN
                (soc_mem_clear(unit, MODPORT_MAP_SWm, MEM_BLOCK_ALL, 0));
            SOC_IF_ERROR_RETURN
                (soc_mem_clear(unit, MODPORT_MAP_MIRRORm, MEM_BLOCK_ALL, 0));
        } else if (SOC_IS_TR_VL(unit)) {
            SOC_IF_ERROR_RETURN
                (soc_mem_clear(unit, MODPORT_MAPm, MEM_BLOCK_ALL, 0));
        } else {
            BCM_IF_ERROR_RETURN(bcm_esw_stk_modport_clear_all(unit));
        }
    }

    rv = _bcm_esw_stk_init();
    if (BCM_FAILURE(rv)) {
        _bcm_esw_stk_detach(unit);
        return (rv);
    }

    SOC_DEFAULT_DMA_SRCMOD_SET(unit, 0);
    SOC_DEFAULT_DMA_SRCPORT_SET(unit, CMIC_PORT(unit));

    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RCPU_ONLY(unit)) {
        /* Blank slate for stack info */
        BCM_PBMP_CLEAR(SOC_PBMP_STACK_CURRENT(unit));
        BCM_PBMP_CLEAR(SOC_PBMP_STACK_INACTIVE(unit));
        BCM_PBMP_CLEAR(SOC_PBMP_STACK_PREVIOUS(unit));

        switch (BCM_CHIP_FAMILY(unit)) {

        case BCM_FAMILY_DRACO15:
        case BCM_FAMILY_DRACO:
        case BCM_FAMILY_TUCANA:
        case BCM_FAMILY_LYNX:
            rv = _bcm_xgs12_stk_init(unit);
            if (BCM_FAILURE(rv)) {
                break;
            }
            rv = bcm_esw_stk_modport_clear_all(unit);
            break;
        case BCM_FAMILY_HERCULES:
            rv = _bcm_xgs_fabric_stk_init(unit);
            break;
        case BCM_FAMILY_HUMV:
        case BCM_FAMILY_CONQUEROR:
            rv =  _bcm_xgs3_ignore_my_modid(unit);
            if (BCM_FAILURE(rv)) {
                break;
            }
            /* fall through */
        case BCM_FAMILY_BRADLEY:
        case BCM_FAMILY_FIREBOLT:
        case BCM_FAMILY_EASYRIDER:
        case BCM_FAMILY_TRIUMPH:
        case BCM_FAMILY_SCORPION:
        case BCM_FAMILY_TRIUMPH2:
        case BCM_FAMILY_TRIDENT:
            rv = _bcm_xgs3_stk_init(unit);
            break;
        default:
            rv = BCM_E_UNAVAIL;
            break;
            
        }

        if (BCM_FAILURE(rv)) {
            _bcm_esw_stk_detach(unit);
            return (rv);
        }
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        rv = _bcm_stk_reinit(unit);
        if (BCM_FAILURE(rv)) {
            _bcm_esw_stk_detach(unit);
            return (rv);
        }
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}



/****************************************************************
 *
 * Per device stacking initialization
 * Clears stacking configuration
 *
 ****************************************************************/

STATIC int
_bcm_xgs12_stk_init(int unit)
{
    if (SOC_IS_DRACO(unit)) {
#if defined(BCM_DRACO_SUPPORT)
        uint32 ptype, config_reg;
        uint64 iconfig;
        int port;

        /* Gig ports */
        PBMP_E_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config_reg));
            ptype = soc_reg_field_get(unit, CONFIGr, config_reg, PORT_TYPEf);
            /* leave trunk mode (1) alone */
            if (ptype == SOC_TUCANA_PORT_TYPE_STACKING) {
                soc_reg_field_set(unit, CONFIGr, &config_reg, PORT_TYPEf, 0);
            }
            soc_reg_field_set(unit, CONFIGr, &config_reg, ST_SIMPLEXf, 0);
            soc_reg_field_set(unit, CONFIGr, &config_reg, ST_MODULEf, 0);
            soc_reg_field_set(unit, CONFIGr, &config_reg, ST_MCNTf, 0);
            soc_reg_field_set(unit, CONFIGr, &config_reg, MY_MODIDf, 0);
            soc_reg_field_set(unit, CONFIGr, &config_reg, LEARN_LOCALf, 0);
            SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config_reg));
        }

        /* higig ports */
        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_MODULEf, 0);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_COUNTf, 0);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, MY_MODIDf, 0);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, IPIC_CASCADEf, 0);
            SOC_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
            BCM_IF_ERROR_RETURN(bcm_esw_port_stp_set(unit, port,
                                                     BCM_STG_STP_FORWARD));
        }
#endif
    } else if (SOC_IS_LYNX(unit)) {
#if defined(BCM_LYNX_SUPPORT)
        uint32 ptype, config_reg;
        uint64 iconfig;
        int port;

        /* 10Gig ports */
        PBMP_E_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config_reg));
            ptype = soc_reg_field_get(unit, CONFIGr, config_reg, PORT_TYPEf);
            /* leave trunk mode (1) alone */
            if (ptype == SOC_TUCANA_PORT_TYPE_STACKING) {
                soc_reg_field_set(unit, CONFIGr, &config_reg, PORT_TYPEf, 0);
            }
            soc_reg_field_set(unit, CONFIGr, &config_reg, MY_MODIDf, 0);
            SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config_reg));
        }

        /* higig ports */
        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, MY_MODIDf, 0);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, IPIC_CASCADEf, 0);
            SOC_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
            BCM_IF_ERROR_RETURN(bcm_esw_port_stp_set(unit, port,
                                                     BCM_STG_STP_FORWARD));
        }
#endif
    } else if (SOC_IS_TUCANA(unit)) {
#if defined(BCM_TUCANA_SUPPORT)
        uint32 ptype, config_reg;
        uint64 iconfig;
        int port;
        port_tab_entry_t pent;
        int sl_mode;

        sl_mode = SOC_SL_MODE(unit);

        /* GE & FE ports */
        SOC_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &config_reg));
        soc_reg_field_set(unit, CHIP_CONFIGr, &config_reg, ST_MODULEf,
                          sl_mode);
        soc_reg_field_set(unit, CHIP_CONFIGr, &config_reg, SIMPLEX_MCNTf, 0);
        soc_reg_field_set(unit, CHIP_CONFIGr, &config_reg, LEARN_LOCALf, 0);
        soc_reg_field_set(unit, CHIP_CONFIGr, &config_reg, MY_MODID0f, 0);
        soc_reg_field_set(unit, CHIP_CONFIGr, &config_reg, MY_MODID1f, 1);
        SOC_IF_ERROR_RETURN(WRITE_CHIP_CONFIGr(unit, config_reg));

        PBMP_GE_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(
                soc_mem_read(unit, SOC_PORT_MEM_TAB(unit, port), MEM_BLOCK_ANY,
                             SOC_PORT_MOD_OFFSET(unit, port), &pent));
            ptype = soc_PORT_TABm_field32_get(unit, &pent, PORT_TYPEf);
            /* leave trunk mode (1) alone */
            if (ptype == SOC_TUCANA_PORT_TYPE_STACKING) {
                soc_PORT_TABm_field32_set(unit, &pent, PORT_TYPEf, 0);
            }
            soc_PORT_TABm_field32_set(unit, &pent, ST_SIMPLEXf, 0);
            SOC_IF_ERROR_RETURN(
                soc_mem_write(unit, SOC_PORT_MEM_TAB(unit, port), MEM_BLOCK_ALL,
                              SOC_PORT_MOD_OFFSET(unit, port), &pent));
        }

        /* HIGIG port */
        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_MODULEf,
                                  sl_mode);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_COUNTf, 0);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, MY_MODID0f, 0);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, MY_MODID1f, 1);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, IPIC_CASCADEf, 0);
            SOC_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
            BCM_IF_ERROR_RETURN(bcm_esw_port_stp_set(unit, port,
                                                     BCM_STG_STP_FORWARD));
        }
#endif
    } else {
        return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}


STATIC int
_bcm_xgs_fabric_stk_init(int unit)
{
#if defined(BCM_HERCULES_SUPPORT) ||  defined(BCM_BRADLEY_SUPPORT)
    int port;

    PBMP_HG_ITER(unit, port) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_port_stp_set(unit, port, BCM_STG_STP_FORWARD));
    }

#if defined(LOCAL_MODMAP_SUPPORT)
    if (soc_feature(unit, soc_feature_modmap)) {
        BCM_IF_ERROR_RETURN(_bcm_esw_modmap_init(unit));
    }
#endif /* MODMAP_SUPPORT */
#endif

    return BCM_E_NONE;
}


STATIC int
_bcm_xgs3_ignore_my_modid(int unit)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_BRADLEY_SUPPORT)
#if defined(BCM_SCORPION_SUPPORT)
    if (SOC_IS_SC_CQ(unit)) {
        uint64 config, oconfig;

        SOC_IF_ERROR_RETURN(READ_ING_CONFIG_64r(unit, &config));
        oconfig = config;
        soc_reg64_field32_set(unit, ING_CONFIG_64r, &config,
                            IGNORE_MY_MODIDf, 1);
        if (COMPILER_64_NE(config, oconfig)) {
            SOC_IF_ERROR_RETURN(WRITE_ING_CONFIG_64r(unit, config));
        }

        rv = BCM_E_NONE;
    } else
#endif
    {
        uint32 config, oconfig;

        SOC_IF_ERROR_RETURN(READ_ING_CONFIGr(unit, &config));
        oconfig = config;
        soc_reg_field_set(unit, ING_CONFIGr, &config,
                          IGNORE_MY_MODIDf, 1);
        if (config != oconfig) {
            SOC_IF_ERROR_RETURN(WRITE_ING_CONFIGr(unit, config));
        }

        rv = BCM_E_NONE;
    }
#endif
    return rv;
}

STATIC int
_bcm_xgs3_stk_init(int unit)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    int pbm_len;
    bcm_port_t port;
    uint32 cpu_pbm, obm_reg;
    uint64 obm_reg_64;
    soc_pbmp_t pbmp;

#if defined(BCM_TRIDENT_SUPPORT)
    /* src_modid_base_index_bk has to be initialized before setting my_modid,
     * since setting my_modid involves allocation of source modid base index.
     */
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {
        SOC_IF_ERROR_RETURN(_src_modid_base_index_bk_init(unit));
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* Set modid to 0 */
    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RCPU_ONLY(unit)) {
        BCM_IF_ERROR_RETURN(bcm_esw_stk_modid_set(unit, 0));
    }

    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_PORT_SET(pbmp, CMIC_PORT(unit));
    } else if (SOC_IS_TR_VL(unit) && !SOC_IS_TRIUMPH2(unit)
        && !SOC_IS_APOLLO(unit) && !SOC_IS_VALKYRIE2(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_xgs3_port_to_higig_bitmap(unit, CMIC_PORT(unit),
                                           &cpu_pbm));
    } else if (CMIC_PORT(unit)) {
        pbm_len = soc_reg_field_length(unit, ICONTROL_OPCODE_BITMAPr, BITMAPf);
        cpu_pbm = 1 << (pbm_len - 1);
    } else {
        cpu_pbm = 1;
    }

    PBMP_ST_ITER(unit, port) {
        /* HG ports to forwarding */
        BCM_IF_ERROR_RETURN(bcm_esw_port_stp_set(unit, port,
                                                 BCM_STG_STP_FORWARD));

        /* Set HG ingress CPU Opcode map to the CPU */
        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            icontrol_opcode_bitmap_entry_t entry;
            SOC_IF_ERROR_RETURN
                (READ_ICONTROL_OPCODE_BITMAPm(unit, MEM_BLOCK_ANY, port,
                                              &entry));
            soc_mem_pbmp_field_set(unit, ICONTROL_OPCODE_BITMAPm, &entry,
                                   BITMAPf, &pbmp);
            SOC_IF_ERROR_RETURN
                (WRITE_ICONTROL_OPCODE_BITMAPm(unit, MEM_BLOCK_ANY, port,
                                               &entry));
        } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit)) {
            uint64 cpu_pbm_64;
            COMPILER_64_ZERO(cpu_pbm_64);
            COMPILER_64_SET(cpu_pbm_64, 0, cpu_pbm);
            SOC_IF_ERROR_RETURN(READ_ICONTROL_OPCODE_BITMAP_64r(unit, port,
                                                             &obm_reg_64));
            soc_reg64_field_set(unit, ICONTROL_OPCODE_BITMAP_64r, &obm_reg_64,
                              BITMAPf, cpu_pbm_64);
            SOC_IF_ERROR_RETURN(WRITE_ICONTROL_OPCODE_BITMAP_64r(unit, port,
                                                              obm_reg_64));
#ifdef BCM_SHADOW_SUPPORT
        } else if (SOC_IS_SHADOW(unit)) {
            SOC_IF_ERROR_RETURN(READ_ICONTROL_OPCODEr(unit, port, &obm_reg));
            soc_reg_field_set(unit, ICONTROL_OPCODEr, &obm_reg, PORT_NUMf, 0);
            SOC_IF_ERROR_RETURN(WRITE_ICONTROL_OPCODEr(unit, port, obm_reg));
#endif
        } else {
            SOC_IF_ERROR_RETURN(READ_ICONTROL_OPCODE_BITMAPr(unit, port,
                                                             &obm_reg));
            soc_reg_field_set(unit, ICONTROL_OPCODE_BITMAPr, &obm_reg,
                              BITMAPf, cpu_pbm);
            SOC_IF_ERROR_RETURN(WRITE_ICONTROL_OPCODE_BITMAPr(unit, port,
                                                              obm_reg));
        }
    }
#if defined(LOCAL_MODMAP_SUPPORT)
    if (soc_feature(unit, soc_feature_modmap)) {
        SOC_IF_ERROR_RETURN(_bcm_esw_modmap_init(unit));
    }
#endif

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        BCM_IF_ERROR_RETURN(bcm_td_modport_map_init(unit));
    }
#endif

#endif /* BCM_XGS3_SWITCH_SUPPORT */

    return BCM_E_NONE;
}

/****************************************************************
 *
 * Stack port set calls:
 *
 *    Callback into BCM layer for notification
 *    Callback to application for notification
 *
 ****************************************************************/


/* Linked list of callback registrants */
typedef struct _esw_stk_callback_s {
    struct _esw_stk_callback_s *next;
    bcm_stk_cb_f fn;
    void *cookie;
} _esw_stk_callback_t;

/*
 * Function:
 *      bcm_esw_stk_update_callback_register/unregister
 * Purpose:
 *      (Un)register an application function for calbacks from stack port set
 * Parameters:
 *      unit   - ignored
 *      cb     - callback function
 *      cookie - anonymous data passed to callback
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Will only register a given (callback, cookie) pair once.
 *      On unregister, both the callback and cookie must match.
 *      Callbacks are made in the order they are registered.  Enqueue
 *      new registrants at the end of the linked list.
 */

static _esw_stk_callback_t *_esw_stk_cb;
static _esw_stk_callback_t *_esw_stk_cb_tail;

/* Register stack update callback at end of list */

int
bcm_esw_stk_update_callback_register(int unit, bcm_stk_cb_f cb, void *cookie)
{
    _esw_stk_callback_t *node;
    int rv = BCM_E_NONE;

    COMPILER_REFERENCE(unit);

    BCM_IF_ERROR_RETURN(_bcm_esw_stk_init());

    ESW_STK_LOCK;
    /* See if already present */
    node = _esw_stk_cb;
    while (node != NULL) {
        if (node->fn == cb && node->cookie == cookie) {
            break; /* Found it */
        }
        node = node->next;
    }

    if (node == NULL) { /* Not there yet */
        node = sal_alloc(sizeof(*node), "bcm_esw_stk_cb");
        if (node != NULL) {
            node->fn = cb;
            node->cookie = cookie;
            node->next = NULL;

            /* Enqueue at tail */
            if (_esw_stk_cb_tail == NULL) {
                _esw_stk_cb = node;
            } else {
                _esw_stk_cb_tail->next = node;
            }
            _esw_stk_cb_tail = node;
        } else {
            rv = BCM_E_MEMORY;
        }
    }
    ESW_STK_UNLOCK;

    return rv;
}

int
bcm_esw_stk_update_callback_unregister(int unit, bcm_stk_cb_f cb, void *cookie)
{
    _esw_stk_callback_t *node;
    _esw_stk_callback_t *prev;
    int                 rv;

    COMPILER_REFERENCE(unit);

    BCM_IF_ERROR_RETURN(_bcm_esw_stk_init());

    ESW_STK_LOCK;
    node = _esw_stk_cb;
    prev = NULL;

    /* Scan list for match of both function and cookie */
    while (node != NULL) {
        if ((node->fn == cb) && (node->cookie == cookie)) {
            break; /* Found it */
        }
        prev = node;
        node = node->next;
    }

    if (node != NULL) { /* Found */
        if (prev == NULL) { /* First on list */
            _esw_stk_cb = node->next;
        } else { /* Update previous */
            prev->next = node->next;
        }
        if (_esw_stk_cb_tail == node) { /* Was last on list */
            _esw_stk_cb_tail = prev;
        }
        rv = BCM_E_NONE;
    } else {
        rv = BCM_E_NOT_FOUND;
    }
    ESW_STK_UNLOCK;

    if (node != NULL) {
        sal_free(node);
    }

    return (rv);
}

/*
 * Force stack ports into ptable tagged bitmap and out of untagged bitmap
 * Note that if a device has no ethernet ports, this will just return.
 */
STATIC int
_bcm_stk_ptable_update(int unit)
{
    bcm_port_cfg_t pcfg;
    int port;
    bcm_pbmp_t old_pbm, old_upbm;

    PBMP_E_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(
            mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));
        SOC_PBMP_ASSIGN(old_pbm, pcfg.pc_pbm);
        SOC_PBMP_ASSIGN(old_upbm, pcfg.pc_ut_pbm);
        SOC_PBMP_OR(pcfg.pc_pbm, SOC_PBMP_STACK_CURRENT(unit));
        SOC_PBMP_REMOVE(pcfg.pc_ut_pbm, SOC_PBMP_STACK_CURRENT(unit));

        if (SOC_PBMP_NEQ(old_pbm, pcfg.pc_pbm) ||
            SOC_PBMP_NEQ(old_upbm, pcfg.pc_ut_pbm)) {
            SOC_IF_ERROR_RETURN(
                mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg));
        }
    }

    return BCM_E_NONE;
}

STATIC int
_esw_stk_update_callbacks(int unit, bcm_port_t port, uint32 flags)
{
    _esw_stk_callback_t *node;

    /* Internal port-table update (for untagged port setting */
    BCM_IF_ERROR_RETURN(_bcm_stk_ptable_update(unit));

    /* SW module callbacks */
    BCM_IF_ERROR_RETURN(_bcm_esw_vlan_stk_update(unit, flags));
    BCM_IF_ERROR_RETURN(_bcm_esw_mcast_stk_update(unit, flags));
    BCM_IF_ERROR_RETURN(_bcm_esw_ipmc_stk_update(unit, flags));
    /* Make callbacks.  Assumes lock is held by caller. */
    node = _esw_stk_cb;
    while (node != NULL) {
        (node->fn)(unit, port, flags, node->cookie);
        node = node->next;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_stk_port_set
 * Purpose:
 *      Set stacking mode for a port
 * Parameters:
 *      unit     BCM device number
 *      port     BCM port number on device
 *      flags    See bcm/stack.h
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If neither simplex or duplex is specified, port should be
 *      considered "unresolved"; for example, if discovery has not
 *      completed yet.  In general, this should not change a port's
 *      settings, but it may affect the port's setup.
 *
 *      If port < 0, use the unique IPIC_PORT if present
 *
 *      If flags == 0, then the port does not have to be a stack port.
 */

int
bcm_esw_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{
    int rv = BCM_E_NONE;
    int txp, rxp;

    STK_VERB(("STK %d: Port set: p %d. flags 0x%x\n", unit, port, flags));

    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        STK_WARN(("STK: %s unit %d\n",
                  SOC_UNIT_VALID(unit)?"Invalid":"Remote", unit));
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_INVALID == port) {
        port = IPIC_PORT(unit);
    } else if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    if (!IS_PORT(unit, port)) {
        STK_WARN(("STK: invalid port (%d,%d)\n", unit, port));
        return BCM_E_PARAM;
    }

    rv = _bcm_esw_stk_init();
    if (BCM_FAILURE(rv)) {
        STK_WARN(("STK: init failure (%d)\n", rv));
        return rv;
    }

    if (flags & (BCM_STK_ENABLE | BCM_STK_CAPABLE)) {
        if (!IS_ST_PORT(unit, port)) {

            /* Setting a non-HG port to stacking implies SL mode */
            if ((flags & BCM_STK_HG) || !IS_GE_PORT(unit, port)) {
                STK_WARN(("STK: Invalid SL stk cfg. unit %d, port %d\n",
                          unit, port));
                return BCM_E_PARAM;
            }
            flags |= BCM_STK_SL; /* Set courtesy flag for callbacks */

            /* Turn pause off on SL stack ports */
            rv = bcm_esw_port_pause_get(unit, port, &txp, &rxp);
            if (BCM_FAILURE(rv)) {
                STK_WARN(("STK: bcm_port_pause_get failure (%d)\n", rv));
                return rv;
            }

            if ((txp != 0) || (rxp != 0)) {
                rv = bcm_esw_port_pause_set(unit, port, 0, 0);
                if (BCM_FAILURE(rv)) {
                    STK_WARN(("STK: bcm_port_pause_set failure (%d)\n", rv));
                    return rv;
                }
            }
        } else { /* HG port/HG2 over GE port */
            if (flags & BCM_STK_SL) {
                STK_WARN(("STK: Invalid HG stk cfg. unit %d, port %d\n",
                          unit, port));
                return BCM_E_PARAM;
            }
            flags |= BCM_STK_HG; /* Set courtesy flag for callbacks */
        }

        if ((flags & BCM_STK_SIMPLEX) && (flags & BCM_STK_DUPLEX)) {
            STK_WARN(("STK: Dimplex not supported. unit %d, port %d\n",
                      unit, port));
            return BCM_E_PARAM;
        }
    }

    if ((flags & BCM_STK_CUT) && (flags & BCM_STK_SL)) {
        flags |= BCM_STK_INACTIVE;
    }

    ESW_STK_LOCK;
    rv = _esw_stk_port_set(unit, port, flags);
    ESW_STK_UNLOCK;

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return rv;
}

/*
 * Function:
 *      bcm_esw_stk_pbmp_get
 * Purpose:
 *      Expose the current enabled and inactive ports
 * Parameters:
 *
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *
 */

int
bcm_esw_stk_pbmp_get(int unit, bcm_pbmp_t *cur_pbm,
                     bcm_pbmp_t *inactive_pbm)
{
    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    if (cur_pbm != NULL) {
        BCM_PBMP_ASSIGN(*cur_pbm, SOC_PBMP_STACK_CURRENT(unit));
    }

    if (inactive_pbm != NULL) {
        BCM_PBMP_ASSIGN(*inactive_pbm, SOC_PBMP_STACK_INACTIVE(unit));
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_esw_stk_mode_get
 * Purpose:
 *      Returns current stack mode
 * Parameters:
 *      unit    BCM device number
 *      flags   (OUT) Indication of stack mode
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Currently should only return one bit set, but that may
 *      change in the future.  SL has precedence; if that mode is
 *      active on the device, that's what gets returned.  Then, if
 *      the device has any HG ports, HG mode is returned.  Then
 *      "NONE" is returned.  Note that some bit will always be set;
 *      that is, the routine will not return 0.
 */

int
bcm_esw_stk_mode_get(int unit, uint32 *flags)
{
    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    if (SOC_SL_MODE(unit)) {
        *flags = BCM_STK_SL;
    } else if (BCM_PBMP_NOT_NULL(PBMP_ST_ALL(unit))) {
        /* HG/HG2 stacking */
        *flags = BCM_STK_HG;
    } else {
        *flags = BCM_STK_NONE;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_stk_mode_set
 * Purpose:
 *      Set the current stack mode
 * Parameters:
 *      unit    BCM device number
 *      flags   Indication of stack mode
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The main use of this is to put a chip supporting SL
 *      stacking into SL mode before other things are started.
 *
 *      Only BCM_STK_SL, BCM_STK_HG and BCM_STK_NONE are checked.
 *
 *      Currently only turning on SL stacking is supported.
 */

int
bcm_esw_stk_mode_set(int unit, uint32 flags)
{
    STK_VERB(("STK %d: Mode set: flags 0x%x\n", unit, flags));

    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    if (flags & BCM_STK_SL) {
        if (!SOC_SL_MODE(unit)) {
            return _esw_stk_sl_mode_enable_set(unit, TRUE);
        }
        return BCM_E_NONE;
    }

    if (flags & BCM_STK_NONE) {
        if (SOC_SL_MODE(unit)) {
            return BCM_E_UNAVAIL; /* Not currently supported */
        }
        return BCM_E_NONE;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_stk_port_get
 * Purpose:
 *      Get stack port mode information
 * Parameters:
 *      unit      BCM device number
 *      port      BCM port number on device
 *      flags     (OUT) flags related to stacking
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *
 */

int
bcm_esw_stk_port_get(int unit, bcm_port_t port, uint32 *flags)
{
    *flags = 0;

    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_INVALID == port) {     /* BCM_STK_USE_HG_IF */
        port = IPIC_PORT(unit);
    } else if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    if (!IS_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    if (BCM_PBMP_MEMBER(SOC_PBMP_STACK_CURRENT(unit), port)) {
        *flags |= BCM_STK_ENABLE;
        if (BCM_PBMP_MEMBER(SOC_PBMP_STACK_INACTIVE(unit), port)) {
            *flags |= BCM_STK_INACTIVE;
        }
        if (IS_ST_PORT(unit, port)) {
            *flags |= BCM_STK_HG;
        } else {
            *flags |= BCM_STK_SL;
        }
    } else {
        *flags |= BCM_STK_NONE;
    }

    return BCM_E_NONE;
}

/****************************************************************
 * The ugly details of adding stack ports
 ****************************************************************/

#if 0
STATIC int
_tucana_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{
    return BCM_E_NONE;
}
#endif

STATIC int
_xgs_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{
    if (SOC_IS_DRACO(unit)) {
#if defined(BCM_DRACO_SUPPORT)
        uint32      config_reg;

        if (IS_GE_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config_reg));
            if (flags & BCM_STK_ENABLE) {
                soc_reg_field_set(unit, CONFIGr, &config_reg, PORT_TYPEf,
                                  SOC_DRACO_PORT_TYPE_STACKING);
                if (flags & BCM_STK_SIMPLEX) {
                    soc_reg_field_set(unit, CONFIGr, &config_reg, ST_SIMPLEXf, 1);
                } else { /* Assume duplex */
                    soc_reg_field_set(unit, CONFIGr, &config_reg, ST_SIMPLEXf, 0);
                }
                SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config_reg));

                /* Set the stack port to STP forwarding state */
                bcm_esw_port_stp_set(unit, port, BCM_STG_STP_FORWARD);
            } else { /* Port taken out of stacking mode */
                soc_reg_field_set(unit, CONFIGr, &config_reg, PORT_TYPEf, 0);
                soc_reg_field_set(unit, CONFIGr, &config_reg, ST_SIMPLEXf, 0);
                SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config_reg));
            }
        } /* else HG port, no action */
#endif
    } else if (SOC_IS_TUCANA(unit)) {
#if defined(BCM_TUCANA_SUPPORT)
        port_tab_entry_t pent;

        if (IS_GE_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN(soc_mem_read(unit, SOC_PORT_MEM_TAB(unit, port),
                                             MEM_BLOCK_ANY,
                                             SOC_PORT_MOD_OFFSET(unit, port), &pent));
            if (flags & BCM_STK_ENABLE) {
                soc_PORT_TABm_field32_set(unit, &pent, PORT_TYPEf,
                                          SOC_TUCANA_PORT_TYPE_STACKING);
                if (flags & BCM_STK_SIMPLEX) {
                    soc_PORT_TABm_field32_set(unit, &pent, ST_SIMPLEXf, 1);
                } else { /* Assume duplex */
                    soc_PORT_TABm_field32_set(unit, &pent, ST_SIMPLEXf, 0);
                }
                SOC_IF_ERROR_RETURN(
                    soc_mem_write(unit, SOC_PORT_MEM_TAB(unit, port),
                                  MEM_BLOCK_ALL, SOC_PORT_MOD_OFFSET(unit, port),
                                  &pent));

                /* Set the stack port to STP forwarding state */
                bcm_esw_port_stp_set(unit, port, BCM_STG_STP_FORWARD);
            } else { /* Port taken out of stacking mode */
                soc_PORT_TABm_field32_set(unit, &pent, PORT_TYPEf, 0);
                soc_PORT_TABm_field32_set(unit, &pent, ST_SIMPLEXf, 0);
                SOC_IF_ERROR_RETURN(
                    soc_mem_write(unit, SOC_PORT_MEM_TAB(unit, port),
                                  MEM_BLOCK_ALL, SOC_PORT_MOD_OFFSET(unit, port),
                                  &pent));
            }
        } /* else HG port, no action */
#endif
    } else if (SOC_IS_LYNX(unit)) {
        /* HiGig only doesn't require any special attention (yet) */
    } else {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

STATIC int
_xgs3_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{
    if (SOC_IS_XGS3_SWITCH(unit)) {
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (IS_ST_PORT(unit, port) || flags == 0) {
            /* Only stack ports may be set (flags != 0), but any port
               may be unset (flags == 0) */
            return BCM_E_NONE;
        }
#endif
    }

    return BCM_E_UNAVAIL;
}


STATIC int
_esw_soc_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{

    /* SL, HG, simplex and duplex are determined per-device */
    switch (BCM_CHIP_FAMILY(unit)) {
    case BCM_FAMILY_FIREBOLT:
    case BCM_FAMILY_EASYRIDER:
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH2:
    case BCM_FAMILY_TRIDENT:
        return _xgs3_stk_port_set(unit, port, flags);


    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_TUCANA:
    case BCM_FAMILY_LYNX:
        return _xgs_stk_port_set(unit, port, flags);

    default:
        break;
    }

    return BCM_E_NONE;
}


STATIC int
_esw_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{
    int rv = BCM_E_NONE;

    /* Check if SL stacking should be enabled on the device */
    if ((!SOC_SL_MODE(unit)) && (flags & BCM_STK_ENABLE) &&
            (flags & BCM_STK_SL)) {
        BCM_IF_ERROR_RETURN(_esw_stk_sl_mode_enable_set(unit, TRUE));
    }

    /* Program the port appropriately */
    BCM_IF_ERROR_RETURN(_esw_soc_stk_port_set(unit, port, flags));

    /* Record old stack port states; make changes to current */
    BCM_PBMP_ASSIGN(SOC_PBMP_STACK_PREVIOUS(unit), SOC_PBMP_STACK_CURRENT(unit));

    if (flags & BCM_STK_ENABLE) {
        SOC_PBMP_PORT_ADD(SOC_PBMP_STACK_CURRENT(unit), port);
        if (flags & BCM_STK_INACTIVE) {
            SOC_PBMP_PORT_ADD(SOC_PBMP_STACK_INACTIVE(unit), port);
        } else {
            SOC_PBMP_PORT_REMOVE(SOC_PBMP_STACK_INACTIVE(unit), port);
        }
    } else {
        SOC_PBMP_PORT_REMOVE(SOC_PBMP_STACK_CURRENT(unit), port);
        SOC_PBMP_PORT_REMOVE(SOC_PBMP_STACK_INACTIVE(unit), port);
    }

    /* Always make callbacks; callbacks are responsible for determining
     * whether or not anything needs to be done.
     */
    rv = _esw_stk_update_callbacks(unit, port, flags);

    return rv;
}

/*
 * Function:
 *      bcm_stk_sl_simplex_count_set (was bcm_stk_count_set)
 * Purpose:
 *      Set the stack count (hops in simplex loop)
 * Parameters:
 *      unit - SOC unit#
 *      count - number of modules in stack
 * Returns:
 *      BCM_E_xx
 * Note:
 *      The count should be the number of modules in the loop for
 *      simplex case;  duplex stacking does not care about stack count.
 *      In the 48 port dual 5605 case, both chips have to set the same
 *      stack count, even if one chip has only a duplex stack link.
 */

int
bcm_esw_stk_sl_simplex_count_set(int unit, int count)
{
    uint32 config_reg;
    uint64 iconfig;
    int blk, port;

    STK_VERB(("STK %d: SL count set to %d\n", unit, count));

    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    COMPILER_REFERENCE(config_reg);
    COMPILER_REFERENCE(iconfig);
    COMPILER_REFERENCE(port);
    COMPILER_REFERENCE(blk);

    /* Maximum count may be less than SOC_MODID_MAX(unit) */
    if (count < 0 || count > 32) {
        return BCM_E_PARAM;
    }

    if (count > 0) {
        count -= 1;
    }

    switch (BCM_CHIP_FAMILY(unit)) {

#if defined(BCM_DRACO_SUPPORT)
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_DRACO:
        SOC_BLOCK_ITER(unit, blk, SOC_BLK_ETHER) {
            port = SOC_BLOCK_PORT(unit, blk);
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config_reg));
            soc_reg_field_set(unit, CONFIGr, &config_reg, ST_MCNTf, count);
            SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config_reg));
        }

        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_COUNTf, count);
            SOC_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
        }

        return BCM_E_NONE;
#endif

#if defined(BCM_TUCANA_SUPPORT)
    case BCM_FAMILY_TUCANA:
        SOC_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &config_reg));
        soc_reg_field_set(unit, CHIP_CONFIGr, &config_reg, SIMPLEX_MCNTf,
                          count);
        SOC_IF_ERROR_RETURN(WRITE_CHIP_CONFIGr(unit, config_reg));

        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_COUNTf, count);
            SOC_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
        }

        return BCM_E_NONE;
#endif

    case BCM_FAMILY_LYNX:
    case BCM_FAMILY_HERCULES:
    default:
        break;
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_stk_sl_simplex_count_get (was bcm_stk_count_get)
 * Purpose:
 *      Get the stack count
 * Parameters:
 *      unit  - SOC unit#
 *      count - (out)number of modules in stack
 * Returns:
 *      BCM_E_xx
 */

int
bcm_esw_stk_sl_simplex_count_get(int unit, int *count)
{
    uint32      config_reg;

    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    COMPILER_REFERENCE(config_reg);

    switch (BCM_CHIP_FAMILY(unit)) {
#if defined(BCM_DRACO_SUPPORT)
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_DRACO:
        SOC_IF_ERROR_RETURN
            (READ_CONFIGr(unit, SOC_PORT(unit, ge, 0), &config_reg));
        if (soc_reg_field_get(unit, CONFIGr, config_reg, ST_MODULEf) == 0) {
            *count = 0;
        } else {
            *count =
                soc_reg_field_get(unit, CONFIGr, config_reg, ST_MCNTf) + 1;
        }

        return BCM_E_NONE;
#endif

#if defined(BCM_TUCANA_SUPPORT)
    case BCM_FAMILY_TUCANA:
        SOC_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &config_reg));
        if (soc_reg_field_get(unit, CHIP_CONFIGr, config_reg, ST_MODULEf)
                == 0) {
            *count = 0;
        } else {
            *count = soc_reg_field_get(unit, CHIP_CONFIGr,
                                       config_reg, SIMPLEX_MCNTf) + 1;
        }

        return BCM_E_NONE;
#endif

    case BCM_FAMILY_LYNX:
    case BCM_FAMILY_HERCULES:
    default:
        break;
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_stk_modid_set
 * Purpose:
 *      Set the module id of a unit
 * Parameters:
 *      unit    - device number
 *      modid   - module id
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      calls bcm_stk_modmap_map (my_modid_set does not)
 */
int
bcm_esw_stk_modid_set(int unit, int modid)
{
    bcm_module_t        mod_out;
    bcm_port_t          port_out;

    BCM_IF_ERROR_RETURN
        (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                modid, -1,
                                &mod_out, &port_out));

    STK_OUT(("STK %d: modid set to %d; mapped %d\n", unit,
             modid, mod_out));

    return bcm_esw_stk_my_modid_set(unit, mod_out);
}

/*
 * Function:
 *      bcm_stk_modid_get
 * Purpose:
 *      Get the module id of a unit
 * Parameters:
 *      unit    - device number
 *      modid   - module id
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      calls bcm_stk_modmap_map (my_modid_set does not)
 */
int
bcm_esw_stk_modid_get(int unit, int *modid)
{
    bcm_module_t        mod_out;
    bcm_port_t          port_out;

    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, modid));

    BCM_IF_ERROR_RETURN
        (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                *modid, -1,
                                &mod_out, &port_out));
    *modid = mod_out;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_stk_modid_count
 * Purpose:
 *      The number of module identifiers (fabric IDs) needed by the unit
 * Parameters:
 *      unit  - SOC unit#
 *      num_modid - (out)# of MODIDs
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_modid_count(int unit, int *num_modid)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    *num_modid = soc->info.modid_count;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_stk_my_modid_set
 * Purpose:
 *      Set the MY_MODID field
 * Parameters:
 *      unit  - SOC unit#
 *      my_modid - the value to set
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      For Tucana 5655 & 5665, the unit requires 2 MODIDs,
 *      my_modid & my_modid + 1 are set
 */
int
bcm_esw_stk_my_modid_set(int unit, int my_modid)
{
    int blk, port;
    uint32 config;
    uint64 iconfig;
    uint32 config2;
    int modid_count;
    
    COMPILER_REFERENCE(config);
    COMPILER_REFERENCE(config2);
    COMPILER_REFERENCE(iconfig);
    COMPILER_REFERENCE(blk);
    COMPILER_REFERENCE(port);
    COMPILER_REFERENCE(modid_count);

    STK_VERB(("STK %d: Set my modid %d\n", unit, my_modid));

    if ((BCM_MODID_INVALID != my_modid) && !SOC_MODID_ADDRESSABLE(unit, my_modid)) {
        return BCM_E_BADID;
    }

    if (BCM_MODID_INVALID != my_modid) {
    SOC_DEFAULT_DMA_SRCMOD_SET(unit, my_modid);
    }
    
    switch (BCM_CHIP_FAMILY(unit)) {

#if defined(BCM_DRACO_SUPPORT) || defined(BCM_LYNX_SUPPORT)
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_LYNX:
        /* Gig, 10Gig ports */
        if (BCM_MODID_INVALID == my_modid) {
            return BCM_E_BADID;
        }
        PBMP_E_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config));
            soc_reg_field_set(unit, CONFIGr, &config, MY_MODIDf, my_modid);
            SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config));
            if (soc_feature(unit, soc_feature_ipmc_lookup)) {
                /*
                 * Set my_modid in EGR_TRUNK_MAPm
                 */
                mmu_memories2_egr_trunk_map_entry_t etm_entry;
                SOC_IF_ERROR_RETURN(READ_MMU_MEMORIES2_EGR_TRUNK_MAPm(unit,
                                                  MEM_BLOCK_ANY,
                                                  port,
                                                  &etm_entry));

                soc_MMU_MEMORIES2_EGR_TRUNK_MAPm_field32_set(unit,
                                                             &etm_entry,
                                                             MY_MODIDf,
                                                             my_modid);
                SOC_IF_ERROR_RETURN(WRITE_MMU_MEMORIES2_EGR_TRUNK_MAPm(unit,
                                                   MEM_BLOCK_ALL,
                                                   port,
                                                   &etm_entry));
            }
        }

        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, MY_MODIDf, my_modid);
            SOC_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
        }
        break;
#endif

#if defined(BCM_TUCANA_SUPPORT)
    case BCM_FAMILY_TUCANA:
        BCM_IF_ERROR_RETURN
            (bcm_esw_stk_modid_count(unit, &modid_count));
        /* no wrapping back to 0 */
        if (my_modid == 31 && modid_count > 1) {
            return BCM_E_BADID;
        }
        if (BCM_MODID_INVALID == my_modid) {
            return BCM_E_BADID;
        }

        /* GE & FE ports */
        SOC_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &config));
        soc_reg_field_set(unit, CHIP_CONFIGr, &config,
                          MY_MODID0f, my_modid);
        if (modid_count == 2) {
            soc_reg_field_set(unit, CHIP_CONFIGr, &config,
                              MY_MODID1f, my_modid + 1);
        }
        SOC_IF_ERROR_RETURN(WRITE_CHIP_CONFIGr(unit, config));

        /* HIGIG port */
        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig,
                                  MY_MODID0f, my_modid);
            if (modid_count == 2) {
                soc_reg64_field32_set(unit, ICONFIGr, &iconfig,
                                      MY_MODID1f, my_modid + 1);
            }
            SOC_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
        }
        break;
#endif

#if defined(BCM_FIREBOLT_SUPPORT)
    case BCM_FAMILY_FIREBOLT:
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH2:
    case BCM_FAMILY_TRIDENT:
    {
        port_tab_entry_t pte;
        if (BCM_MODID_INVALID == my_modid) {
            return BCM_E_BADID;
        }

#ifdef BCM_TRIUMPH_SUPPORT
        if ((BCM_CHIP_FAMILY(unit) == BCM_FAMILY_TRIUMPH ||
             BCM_CHIP_FAMILY(unit) == BCM_FAMILY_TRIUMPH2) &&
            NUM_MODID(unit) > 1) {
            /* Dual-modid is enabled (32-port module mode). The my_modid
             * value specified here needs to be an even number.
             */
            if (my_modid & 0x1) {
                return BCM_E_BADID;
            }
        }
#endif

        
	PBMP_E_ITER(unit, port) {
	    BCM_IF_ERROR_RETURN
	      (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, port, &pte));
	    soc_PORT_TABm_field32_set(unit, &pte, MY_MODIDf, my_modid);
	    BCM_IF_ERROR_RETURN
	      (soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, port, &pte));
	}
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_wlan) || 
            (soc_feature(unit, soc_feature_mim))) {
            soc_info_t *si;
            int i, min, max;
            lport_tab_entry_t lport_profile;

            si = &SOC_INFO(unit);
            /* Do it for the loopback port */
            port = SOC_IS_ENDURO(unit)? 1 : si->lb_port;
	    BCM_IF_ERROR_RETURN
	      (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, port, &pte));
	    soc_PORT_TABm_field32_set(unit, &pte, MY_MODIDf, my_modid);
	    BCM_IF_ERROR_RETURN
	      (soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, port, &pte));

        /* update si->cpu_hg_index if relevant */
        if (SOC_IS_TD_TT( unit) && (si->cpu_hg_index > 0)) {
            port = si->cpu_hg_index;
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, port, &pte));
            soc_PORT_TABm_field32_set(unit, &pte, MY_MODIDf, my_modid);
            BCM_IF_ERROR_RETURN
	      (soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, port, &pte));
        }

            /* Update modid for LPORT profiles that correspond to the
               loopback port */
            min = soc_mem_index_min(unit, LPORT_TABm);
            max = soc_mem_index_max(unit, LPORT_TABm);
            for (i = min; i <= max; i++) {
                BCM_IF_ERROR_RETURN
                    (READ_LPORT_TABm(unit, MEM_BLOCK_ANY, i, &lport_profile));
                if (soc_mem_field_valid(unit, LPORT_TABm, SRC_SYS_PORT_IDf)) {
                    if (soc_mem_field32_get(unit, LPORT_TABm, &lport_profile,
                                            SRC_SYS_PORT_IDf) != si->lb_port) {
                        continue;
                     }
                } else if (soc_mem_field_valid(unit, LPORT_TABm,
                                               MIM_MC_TERM_ENABLEf)) {
                    if (!soc_mem_field32_get(unit, LPORT_TABm, &lport_profile,
                                             MIM_MC_TERM_ENABLEf)) {
                        continue;
                    }
                }
                soc_LPORT_TABm_field32_set(unit, &lport_profile, MY_MODIDf, 
                                           my_modid);
                BCM_IF_ERROR_RETURN
                    (WRITE_LPORT_TABm(unit, MEM_BLOCK_ALL, i, &lport_profile));
            }
        }
#endif
	PBMP_ST_ITER(unit, port) {
	    BCM_IF_ERROR_RETURN
	      (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, port, &pte));
	    soc_PORT_TABm_field32_set(unit, &pte, MY_MODIDf, my_modid);
	    BCM_IF_ERROR_RETURN
	      (soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, port, &pte));
            if ((IS_HG_PORT(unit, port) && !SOC_IS_TD_TT(unit) &&
                 !SOC_IS_KATANA(unit)) ||
                (IS_HL_PORT(unit, port) && (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)))) {
                
                BCM_IF_ERROR_RETURN(READ_XPORT_CONFIGr(unit, port, &config));
                soc_reg_field_set(unit, XPORT_CONFIGr, &config,
                                  MY_MODIDf, my_modid);
                BCM_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, config));

#if 0
                if (SOC_REG_IS_VALID(unit, XPORT_CONFIGr)) {
                    BCM_IF_ERROR_RETURN
                        (READ_XPORT_CONFIGr(unit, port, &config));
                    soc_reg_field_set(unit, XPORT_CONFIGr, &config,
                                      MY_MODIDf, my_modid);
                    BCM_IF_ERROR_RETURN
                        (WRITE_XPORT_CONFIGr(unit, port, config));
                } else if (SOC_REG_IS_VALID(unit, XLPORT_CONFIGr)) {
                    BCM_IF_ERROR_RETURN
                        (READ_XLPORT_CONFIGr(unit, port, &config));
                    soc_reg_field_set(unit, XLPORT_CONFIGr, &config,
                                      MY_MODIDf, my_modid);
                    BCM_IF_ERROR_RETURN
                        (WRITE_XLPORT_CONFIGr(unit, port, config));
                }
#endif
            }
	}
        port = CMIC_PORT(unit);
        BCM_IF_ERROR_RETURN
          (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, port, &pte));
        soc_PORT_TABm_field32_set(unit, &pte, MY_MODIDf, my_modid);
        BCM_IF_ERROR_RETURN
          (soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, port, &pte));
        if (!SOC_IS_TD_TT(unit) && !SOC_IS_KATANA(unit)) { 
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, IPORT_TABLEm, MEM_BLOCK_ANY, port, &pte));
            soc_IPORT_TABLEm_field32_set(unit, &pte, MY_MODIDf, my_modid);
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, IPORT_TABLEm, MEM_BLOCK_ALL, port, &pte));
        }
        /* Mirroring across stack */
        if (soc_feature(unit, soc_feature_src_modid_blk) &&
            !SOC_IS_SHADOW(unit)) {
            soc_field_t field = MY_MODIDf;
#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                PBMP_ALL_ITER(unit, port) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_fields32_modify(unit, EGR_PORTm, port, 1,
                                                 &field, (uint32*)&my_modid));
                }
                BCM_IF_ERROR_RETURN
                    (soc_mem_fields32_modify(unit, EGR_PORTm,
                                            SOC_INFO(unit).cpu_hg_index, 1,
                                            &field, (uint32*)&my_modid));
            } else
#endif /* BCM_TRIDENT_SUPPORT */
            {
                soc_reg_t egr_port_reg, iegr_port_reg;
                egr_port_reg = SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                    SOC_IS_VALKYRIE2(unit) ? EGR_PORT_64r : EGR_PORTr;
                iegr_port_reg = SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                    SOC_IS_VALKYRIE2(unit) ? IEGR_PORT_64r : IEGR_PORTr;
                PBMP_ALL_ITER(unit, port) {
                    BCM_IF_ERROR_RETURN
                        (soc_reg_fields32_modify(unit, egr_port_reg, port, 1,
                                                 &field, (uint32*)&my_modid));
                }
                port = CMIC_PORT(unit);
                BCM_IF_ERROR_RETURN
                    (soc_reg_fields32_modify(unit, iegr_port_reg, port, 1,
                                             &field, (uint32*)&my_modid));
            }
        }
#if defined(BCM_RAPTOR_SUPPORT)
        if (soc_feature(unit, soc_feature_mac_learn_limit) &&
            SOC_REG_IS_VALID(unit, MAC_LIMIT_CONFIGr)) {
            uint32  limit_config;
            /* Configure the MODID for MAC learn limit */
            BCM_IF_ERROR_RETURN(READ_MAC_LIMIT_CONFIGr(unit, &limit_config));
            soc_reg_field_set(unit, MAC_LIMIT_CONFIGr, &limit_config,
                              MY_MODIDf, my_modid);
            BCM_IF_ERROR_RETURN(WRITE_MAC_LIMIT_CONFIGr(unit, limit_config));
        }
#endif /* BCM_RAPTOR_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(unit, soc_feature_src_modid_base_index)) {
            int base_index;

            /* Allocate a source modid base index for my_modid */
            BCM_IF_ERROR_RETURN(_bcm_esw_src_modid_base_index_get(unit,
                        my_modid, &base_index));
        }
#endif /* BCM_TRIDENT_SUPPORT */
    }
    break;
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_EASYRIDER_SUPPORT)
    case BCM_FAMILY_EASYRIDER:
    {
        if (BCM_MODID_INVALID == my_modid) {
            return BCM_E_BADID;
        }

        /* IGR_PORT.MY_MODID */
        /* IGR_IPORT.MY_MODID */
	PBMP_ALL_ITER(unit, port) {
	    BCM_IF_ERROR_RETURN(READ_IGR_PORTr(unit, port, &config));
	    soc_reg_field_set(unit, IGR_PORTr, &config,
			      MY_MODIDf, my_modid);
	    BCM_IF_ERROR_RETURN(WRITE_IGR_PORTr(unit, port, config));
	    BCM_IF_ERROR_RETURN(READ_IGR_IPORTr(unit, port, &config));
	    soc_reg_field_set(unit, IGR_IPORTr, &config,
			      MY_MODIDf, my_modid);
	    BCM_IF_ERROR_RETURN(WRITE_IGR_IPORTr(unit, port, config));
	}
        /* E2E_MODULE_CONFIG.MY_MODULE_ID */
	    BCM_IF_ERROR_RETURN(READ_E2E_MODULE_CONFIGr(unit, &config));
	    soc_reg_field_set(unit, E2E_MODULE_CONFIGr, &config,
			      MY_MODULE_IDf, my_modid);
	    BCM_IF_ERROR_RETURN(WRITE_E2E_MODULE_CONFIGr(unit, config));
        /* XPORT.MY_MODID */
	PBMP_HG_ITER(unit, port) {
	    BCM_IF_ERROR_RETURN(READ_XPORT_CONFIGr(unit, port, &config));
	    soc_reg_field_set(unit, XPORT_CONFIGr, &config,
			      MY_MODIDf, my_modid);
	    BCM_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, config));
	}
        port = CMIC_PORT(unit);
        BCM_IF_ERROR_RETURN(READ_IGR_PORTr(unit, port, &config));
        soc_reg_field_set(unit, IGR_PORTr, &config,
                          MY_MODIDf, my_modid);
        BCM_IF_ERROR_RETURN(WRITE_IGR_PORTr(unit, port, config));
        BCM_IF_ERROR_RETURN(READ_IGR_IPORTr(unit, port, &config));
        soc_reg_field_set(unit, IGR_IPORTr, &config,
                          MY_MODIDf, my_modid);
        BCM_IF_ERROR_RETURN(WRITE_IGR_IPORTr(unit, port, config));
    }
    break;
#endif /* BCM_EASYRIDER_SUPPORT */

#if defined(BCM_HERCULES_SUPPORT)
    case BCM_FAMILY_HERCULES:
        if (BCM_MODID_INVALID == my_modid) {
            SOC_CONTROL(unit)->info.modid_count = 0;
        } else {
            SOC_CONTROL(unit)->info.modid_count = 1;
        }
        break;
#endif /* BCM_HERCULES_SUPPORT */

    default:
        return BCM_E_UNAVAIL;
    }

    SOC_BASE_MODID_SET(unit, my_modid);
    soc_event_generate(unit, SOC_SWITCH_EVENT_MODID_CHANGE, 0,0,0);

   /* Base module id set/get macro */

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_stk_my_modid_get
 * Purpose:
 *      Get the MY_MODID field
 * Parameters:
 *      unit  - SOC unit#
 *      my_modid - (out)the value
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      For Tucana, the unit has 2 MODIDs, *my_modid return the
 *      lower MODID, higher MODID is (*my_modid + 1)
 */
int
bcm_esw_stk_my_modid_get(int unit, int *my_modid)
{
    *my_modid = SOC_BASE_MODID(unit);

    if (BCM_MODID_INVALID == *my_modid) {
        return BCM_E_UNAVAIL;
    }

    return (BCM_E_NONE);
}

int
bcm_stk_modport_voq_cosq_profile_get(int unit, bcm_port_t ing_port, 
                               bcm_module_t dest_modid, int *profile_id)
{
    if (profile_id == NULL) {
        return BCM_E_PARAM;
    }

#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit)) {
        *profile_id = -1;
        BCM_IF_ERROR_RETURN(
                bcm_td_stk_modport_voq_op(unit, ing_port, dest_modid,
                    profile_id, _BCM_STK_PORT_MODPORT_DMVOQ_GRP_OP_GET));
        return BCM_E_NONE;
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

int
bcm_stk_modport_voq_cosq_profile_set(int unit, bcm_port_t ing_port, 
                               bcm_module_t dest_modid, int profile_id)
{
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit)) {
        BCM_IF_ERROR_RETURN(
                bcm_td_stk_modport_voq_op(unit, ing_port, dest_modid,
                    &profile_id, _BCM_STK_PORT_MODPORT_DMVOQ_GRP_OP_SET));
        return BCM_E_NONE;
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

#if defined(BCM_WARM_BOOT_SUPPORT) 
/*
 * Function:
 *      bcm_esw_reload_stk_my_modid_get
 * Purpose:
 *      Get the MY_MODID field
 * Parameters:
 *      unit  - SOC unit#
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      For Tucana, the unit has 2 MODIDs, *my_modid return the
 *      lower MODID, higher MODID is (*my_modid + 1)
 *
 *      This routine is being called by _bcm_modules_init(), so it is not
 *      not called by the Stack init routine.
 */
int
bcm_esw_reload_stk_my_modid_get(int unit)
{
    uint32 config;
    int blk, port;
    bcm_module_t my_modid = 0;

    COMPILER_REFERENCE(config);
    COMPILER_REFERENCE(blk);
    COMPILER_REFERENCE(port);


    switch (BCM_CHIP_FAMILY(unit)) {

#if defined(BCM_DRACO_SUPPORT) || defined(BCM_LYNX_SUPPORT)
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_LYNX:
        /* Read from GE */
        PBMP_E_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config));
            my_modid = soc_reg_field_get(unit, CONFIGr, config, MY_MODIDf);
            break;
        }
        break;
#endif

#if defined(BCM_TUCANA_SUPPORT)
    case BCM_FAMILY_TUCANA:
        SOC_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &config));
        my_modid = soc_reg_field_get(unit, CHIP_CONFIGr, config, MY_MODID0f);
        break;
#endif

#if defined(BCM_FIREBOLT_SUPPORT)
    case BCM_FAMILY_FIREBOLT:
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH2:
    case BCM_FAMILY_TRIDENT:
	PBMP_PORT_ITER(unit, port) {
	    port_tab_entry_t pte;
	    BCM_IF_ERROR_RETURN
	      (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, port, &pte));
	    my_modid = soc_PORT_TABm_field32_get(unit, &pte, MY_MODIDf);
	    break;
	}
        break;
#endif

#ifdef BCM_EASYRIDER_SUPPORT
    case BCM_FAMILY_EASYRIDER:
        PBMP_E_ITER(unit, port) {
            uint32 igr_port;
            SOC_IF_ERROR_RETURN(READ_IGR_PORTr(unit, port, &igr_port));
            my_modid = soc_reg_field_get(unit, IGR_PORTr, igr_port, MY_MODIDf);
            break;
        }
        break;
#endif /* BCM_EASYRIDER_SUPPORT */

#if defined(BCM_HERCULES_SUPPORT)
    case BCM_FAMILY_HERCULES:
        if (SOC_CONTROL(unit)->info.modid_count > 0) {
            my_modid = SOC_DEFAULT_DMA_SRCMOD_GET(unit);
        } else {
            return BCM_E_UNAVAIL;
        }
        break;
#endif /* BCM_HERCULES_SUPPORT */

    default:
        return BCM_E_UNAVAIL;
    }

    SOC_BASE_MODID_SET(unit, my_modid);    
    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
#if defined(BCM_XGS12_SWITCH_SUPPORT)

/*
 * In XGS SWITCH, the MODPORT table is implemented as 4 or 8 registers,
 * each register holds 8 fields, one for one modid
 */
static soc_reg_t _bcm_stk_xgs_mp_r[] =
{
    MODPORT_7_0r, MODPORT_15_8r, MODPORT_23_16r, MODPORT_31_24r
};

static soc_reg_t _bcm_stk_xgs_imp_r[] =
{
    IMODPORT_7_0r, IMODPORT_15_8r, IMODPORT_23_16r, IMODPORT_31_24r
#if defined(BCM_DRACO15_SUPPORT)
    ,
    IMODPORT_39_32r, IMODPORT_47_40r, IMODPORT_55_48r, IMODPORT_63_56r
#endif /* BCM_DRACO15_SUPPORT */
};

static soc_field_t _bcm_stk_xgs_mp_f[] =
{
     PORT_FOR_MOD0f, PORT_FOR_MOD1f, PORT_FOR_MOD2f,
     PORT_FOR_MOD3f, PORT_FOR_MOD4f, PORT_FOR_MOD5f,
     PORT_FOR_MOD6f, PORT_FOR_MOD7f, PORT_FOR_MOD8f,
     PORT_FOR_MOD9f, PORT_FOR_MOD10f, PORT_FOR_MOD11f,
     PORT_FOR_MOD12f, PORT_FOR_MOD13f, PORT_FOR_MOD14f,
     PORT_FOR_MOD15f, PORT_FOR_MOD16f, PORT_FOR_MOD17f,
     PORT_FOR_MOD18f, PORT_FOR_MOD19f, PORT_FOR_MOD20f,
     PORT_FOR_MOD21f, PORT_FOR_MOD22f, PORT_FOR_MOD23f,
     PORT_FOR_MOD24f, PORT_FOR_MOD25f, PORT_FOR_MOD26f,
     PORT_FOR_MOD27f, PORT_FOR_MOD28f, PORT_FOR_MOD29f,
     PORT_FOR_MOD30f, PORT_FOR_MOD31f
#if defined(BCM_DRACO15_SUPPORT)
     ,
     PORT_FOR_MOD32f, PORT_FOR_MOD33f, PORT_FOR_MOD34f,
     PORT_FOR_MOD35f, PORT_FOR_MOD36f, PORT_FOR_MOD37f,
     PORT_FOR_MOD38f, PORT_FOR_MOD39f, PORT_FOR_MOD40f,
     PORT_FOR_MOD41f, PORT_FOR_MOD42f, PORT_FOR_MOD43f,
     PORT_FOR_MOD44f, PORT_FOR_MOD45f, PORT_FOR_MOD46f,
     PORT_FOR_MOD47f, PORT_FOR_MOD48f, PORT_FOR_MOD49f,
     PORT_FOR_MOD50f, PORT_FOR_MOD51f, PORT_FOR_MOD52f,
     PORT_FOR_MOD53f, PORT_FOR_MOD54f, PORT_FOR_MOD55f,
     PORT_FOR_MOD56f, PORT_FOR_MOD57f, PORT_FOR_MOD58f,
     PORT_FOR_MOD59f, PORT_FOR_MOD60f, PORT_FOR_MOD61f,
     PORT_FOR_MOD62f, PORT_FOR_MOD63f
#endif /* BCM_DRACO15_SUPPORT */
};

#define _bcm_stk_MODPORT_reg(_modid_)   _bcm_stk_xgs_mp_r[_modid_/8]
#define _bcm_stk_IMODPORT_reg(_modid_)  _bcm_stk_xgs_imp_r[_modid_/8]
#define _bcm_stk_MODPORT_f(_modid_)     _bcm_stk_xgs_mp_f[_modid_]

#endif  /* BCM_XGS12_SWITCH_SUPPORT */

#if defined(BCM_TRX_SUPPORT)
STATIC int
_bcm_stk_modport_profile_init(int unit)
{
    soc_mem_t mem;
    int entry_words;
    soc_profile_mem_t *profile;
    void *entry;
    uint32 base_index, index;
    int rv, modid_count, port_count, alloc_size, port;

    if (modport_profile[unit] == NULL) {
        modport_profile[unit] = sal_alloc(sizeof(soc_profile_mem_t),
                                             "ModPort Profile Mem");
        if (modport_profile[unit] == NULL) {
            return BCM_E_MEMORY;
        }
    } else {
        soc_profile_mem_destroy(unit, modport_profile[unit]);
    }
    profile = modport_profile[unit];
    soc_profile_mem_t_init(profile);

    mem = MODPORT_MAPm;
    entry_words = sizeof(modport_map_entry_t) / sizeof(uint32);

    BCM_IF_ERROR_RETURN
        (soc_profile_mem_create(unit, &mem, &entry_words, 1, profile));

    modid_count = SOC_MODID_MAX(unit) + 1;

    if (SOC_WARM_BOOT(unit) || SOC_IS_RCPU_ONLY(unit)) {    /* Warm Boot */
        int     i;
        uint32  val;

        /* Gather reference count for modport map profile table */
        PBMP_ALL_ITER(unit, port) {
            SOC_IF_ERROR_RETURN
                (READ_MODPORT_MAP_SELr(unit, port, &val));
            index = soc_reg_field_get(unit, MODPORT_MAP_SELr, val,
                                      MODPORT_MAP_INDEX_UPPERf);
            index *= modid_count;
            for (i = 0; i < modid_count; i++) {
                SOC_PROFILE_MEM_REFERENCE(unit, modport_profile[unit],
                                          index + i, 1);
                SOC_PROFILE_MEM_ENTRIES_PER_SET(unit,
                                                modport_profile[unit],
                                                index + i,
                                                modid_count);
            }
        }

        return BCM_E_NONE;
    }

    alloc_size = modid_count * entry_words * sizeof(uint32);
    entry = sal_alloc(alloc_size, "modport_map_entry");
    if (entry == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(entry, 0, alloc_size);
    rv = soc_profile_mem_add(unit, profile, &entry, modid_count, &base_index);
    sal_free(entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    port_count = 0;
    PBMP_ALL_ITER(unit, port) {
        BCM_IF_ERROR_RETURN
            (WRITE_MODPORT_MAP_SELr(unit, port, base_index / modid_count));
        port_count++;
    }

    for (index = 0; index < modid_count; index++) {
        SOC_PROFILE_MEM_REFERENCE(unit, profile, base_index + index,
                                  port_count - 1);
    }

    return BCM_E_NONE;
}
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)

STATIC int
_bcm_stk_port_modport_op(int unit, int op, bcm_port_t ing_port,
                         bcm_module_t dest_modid, bcm_port_t *dest_port,
                         int dest_port_count)
{
    int    modid_count, modid_min, modid_max;
    pbmp_t pbmp, hg_trunk_pbmp, hg_pbmp;
    int    index;
    int    tid;
    bcm_trunk_chip_info_t chip_info;
    bcm_trunk_add_info_t t_add_info;
    bcm_port_t local_port;
    modport_map_entry_t entry;
    int    rv;
    uint32 base_index;
    uint32 hg_pbmp32, new_hg_pbmp32;
#if defined(BCM_TRX_SUPPORT)
    pbmp_t new_pbmp;
    soc_profile_mem_t *profile;
    modport_map_entry_t *entry_array;
    void  *entries;
    uint32 rval;
    uint32 new_base_index;
#endif /* BCM_TRX_SUPPORT */

    if (op <= 0 || op >= _BCM_STK_PORT_MODPORT_OP_COUNT) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(ing_port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, ing_port, &ing_port));
    }

    if (!SOC_PORT_VALID(unit, ing_port)) {
        return BCM_E_PORT;
    }

    modid_count = SOC_MODID_MAX(unit) + 1;
    if (dest_modid == -1) {
        modid_min = 0;
        modid_max = modid_count - 1;
    } else {
        if (!SOC_MODID_ADDRESSABLE(unit, dest_modid)) {
            return BCM_E_PARAM;
        }
        modid_min = modid_max = dest_modid;
    }

    if (dest_port_count > SOC_MAX_NUM_PORTS) {
        return BCM_E_INTERNAL;
    }

    BCM_PBMP_CLEAR(pbmp);
    for (index = 0; index < dest_port_count; index++) {

        if (BCM_GPORT_IS_TRUNK(dest_port[index])) {
            tid = SOC_GPORT_TRUNK_GET(dest_port[index]);

            BCM_IF_ERROR_RETURN(bcm_esw_trunk_chip_info_get(unit, &chip_info));
            if (chip_info.trunk_fabric_id_min < 0 || 
                    tid < chip_info.trunk_fabric_id_min || 
                    tid > chip_info.trunk_fabric_id_max) { 
                return BCM_E_PARAM;
            }

            /* Get Higig trunk group port bitmap */

            bcm_trunk_add_info_t_init(&t_add_info);
            BCM_IF_ERROR_RETURN(bcm_esw_trunk_get(unit, tid, &t_add_info));
            if (t_add_info.num_ports == 0) {
                /* No ports have been added to the Higig trunk group */
                return BCM_E_PARAM;
            }

            if (BCM_GPORT_IS_SET(t_add_info.tp[0])) {
                BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit,
                            t_add_info.tp[0], &local_port));
                BCM_PBMP_PORT_SET(hg_trunk_pbmp, local_port);
            } else {
                BCM_PBMP_PORT_SET(hg_trunk_pbmp, t_add_info.tp[0]);
            }

            BCM_IF_ERROR_RETURN
                (bcm_esw_trunk_bitmap_expand(unit, &hg_trunk_pbmp));

            BCM_PBMP_OR(pbmp, hg_trunk_pbmp);

        } else {
            if (!SOC_PORT_VALID(unit, dest_port[index])) {
                return BCM_E_PORT;
            }

            if (dest_port[index] == CMIC_PORT(unit) &&
                    BCM_CHIP_FAMILY(unit) == BCM_FAMILY_TRIUMPH) {
                return BCM_E_PORT;
            }

            BCM_PBMP_PORT_ADD(pbmp, dest_port[index]);
        }
    }

#if defined(BCM_TRX_SUPPORT)

    if (SOC_IS_TR_VL(unit)) {
        /* Profile style MODPORT MAP bitmap table */
        if (modport_profile[unit] == NULL) {
            _bcm_stk_modport_profile_init(unit);
        }
        profile = modport_profile[unit];

        if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit) ||
            SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
            BCM_IF_ERROR_RETURN
                (soc_xgs3_bitmap_to_higig_bitmap(unit, pbmp, &hg_pbmp));
            BCM_PBMP_ASSIGN(pbmp, hg_pbmp);
        }

        entry_array = sal_alloc(sizeof(modport_map_entry_t) * modid_count,
                                "modport_map_entry");
        if (entry_array == NULL) {
            return BCM_E_MEMORY;
        }

        soc_mem_lock(unit, MODPORT_MAPm);
        rv = READ_MODPORT_MAP_SELr(unit, ing_port, &rval);
        if (BCM_FAILURE(rv)) {
            soc_mem_unlock(unit, MODPORT_MAPm);
            sal_free(entry_array);
            return rv;
        }
        base_index = modid_count *
            soc_reg_field_get(unit, MODPORT_MAP_SELr, rval,
                              MODPORT_MAP_INDEX_UPPERf);
        for (index = 0; index < modid_count; index++) {
            sal_memcpy(&entry_array[index],
                       SOC_PROFILE_MEM_ENTRY(unit, profile, void *,
                                             base_index + index),
                       sizeof(modport_map_entry_t));
        }
        for (index = modid_min; index <= modid_max; index++) {
            switch (op) {
            case _BCM_STK_PORT_MODPORT_OP_SET:
                BCM_PBMP_ASSIGN(new_pbmp, pbmp);
                break;
            case _BCM_STK_PORT_MODPORT_OP_ADD:
                soc_mem_pbmp_field_get(unit, MODPORT_MAPm, &entry_array[index],
                                       HIGIG_PORT_BITMAPf, &new_pbmp);
                BCM_PBMP_OR(new_pbmp, pbmp);
                break;
            case _BCM_STK_PORT_MODPORT_OP_DELETE:
                soc_mem_pbmp_field_get(unit, MODPORT_MAPm, &entry_array[index],
                                       HIGIG_PORT_BITMAPf, &new_pbmp);
                BCM_PBMP_REMOVE(new_pbmp, pbmp);
                break;
            default:
                soc_mem_unlock(unit, MODPORT_MAPm);
                sal_free(entry_array);
                return BCM_E_INTERNAL;
            }
            soc_mem_pbmp_field_set(unit, MODPORT_MAPm, &entry_array[index],
                                   HIGIG_PORT_BITMAPf, &new_pbmp);
        }
        entries = entry_array;
        rv = soc_profile_mem_add(unit, profile, &entries, modid_count,
                                 &new_base_index);
        if (BCM_SUCCESS(rv)) {
            soc_reg_field_set(unit, MODPORT_MAP_SELr, &rval,
                              MODPORT_MAP_INDEX_UPPERf,
                              new_base_index / modid_count);
            rv = WRITE_MODPORT_MAP_SELr(unit, ing_port, rval);
        }
        if (BCM_SUCCESS(rv)) {
            rv = soc_profile_mem_delete(unit, profile, base_index);
        }
        soc_mem_unlock(unit, MODPORT_MAPm);
        sal_free(entry_array);
    } else
#endif /* BCM_TRX_SUPPORT */

    if (SOC_IS_HBX(unit)) {
        /* Per port style MODPORT MAP bitmap table */
        BCM_IF_ERROR_RETURN
            (soc_xgs3_bitmap_to_higig_bitmap(unit, pbmp, &hg_pbmp));
        hg_pbmp32 = SOC_PBMP_WORD_GET(hg_pbmp, 0);
        base_index = ing_port * modid_count;

        rv = BCM_E_NONE;
        soc_mem_lock(unit, MODPORT_MAPm);
        for (index = modid_min; index <= modid_max; index++) {
            rv = READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, base_index + index,
                                   &entry);
            if (BCM_FAILURE(rv)) {
                soc_mem_unlock(unit, MODPORT_MAPm);
                return rv;
            }
            switch (op) {
            case _BCM_STK_PORT_MODPORT_OP_SET:
                new_hg_pbmp32 = hg_pbmp32;
                break;
            case _BCM_STK_PORT_MODPORT_OP_ADD:
                new_hg_pbmp32 =
                    soc_mem_field32_get(unit, MODPORT_MAPm, &entry,
                                        HIGIG_PORT_BITMAPf);
                new_hg_pbmp32 |= hg_pbmp32;
                    break;
            case _BCM_STK_PORT_MODPORT_OP_DELETE:
                new_hg_pbmp32 =
                    soc_mem_field32_get(unit, MODPORT_MAPm, &entry,
                                        HIGIG_PORT_BITMAPf);
                new_hg_pbmp32 &= ~hg_pbmp32;
                break;
            default:
                soc_mem_unlock(unit, MODPORT_MAPm);
                return BCM_E_INTERNAL;
            }
            soc_mem_field32_set(unit, MODPORT_MAPm, &entry,
                                HIGIG_PORT_BITMAPf, new_hg_pbmp32);
            rv = WRITE_MODPORT_MAPm(unit, MEM_BLOCK_ANY, base_index + index,
                                    &entry);
        }
        soc_mem_unlock(unit, MODPORT_MAPm);

    } else {
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

STATIC int
_bcm_stk_port_modport_get(int unit, bcm_port_t ing_port,
                          bcm_module_t dest_modid, int dest_port_max,
                          bcm_port_t *dest_port_array,
                          int *dest_port_count)
{
    uint32 hg_pbmp;
    modport_map_entry_t entry;
    int modid_count, base_index;
#if defined(BCM_TRX_SUPPORT)
    soc_profile_mem_t *profile;
    modport_map_entry_t *entry_p;
    pbmp_t pbmp;
    uint32 rval;
    int count, port;
#endif /* BCM_TRX_SUPPORT */

    if (BCM_GPORT_IS_SET(ing_port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, ing_port, &ing_port));
    }
    if (!SOC_PORT_VALID(unit, ing_port)) {
        return BCM_E_PORT;
    }
    if (!SOC_MODID_ADDRESSABLE(unit, dest_modid)) {
        return BCM_E_PARAM;
    }
    if (dest_port_max < 0 || dest_port_array == NULL ||
        dest_port_count == NULL) {
        return BCM_E_PARAM;
    }
    if (dest_port_max == 0) {
        *dest_port_count = 0;
        return BCM_E_NOT_FOUND;
    }

    modid_count = SOC_MODID_MAX(unit) + 1;

#if defined(BCM_TRX_SUPPORT)

    if (SOC_IS_TR_VL(unit)) {
        if (modport_profile[unit] == NULL) {
            *dest_port_count = 0;
            return BCM_E_NOT_FOUND;
        }

        profile = modport_profile[unit];

        BCM_IF_ERROR_RETURN(READ_MODPORT_MAP_SELr(unit, ing_port, &rval));
        base_index = modid_count *
            soc_reg_field_get(unit, MODPORT_MAP_SELr, rval,
                              MODPORT_MAP_INDEX_UPPERf);
        entry_p = SOC_PROFILE_MEM_ENTRY(unit, profile, modport_map_entry_t *,
                                        base_index + dest_modid);
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
            SOC_IS_VALKYRIE2(unit)) {
            soc_mem_pbmp_field_get(unit, MODPORT_MAPm, entry_p,
                                   HIGIG_PORT_BITMAPf, &pbmp);
            count = 0;
            BCM_PBMP_ITER(pbmp, port) {
                dest_port_array[count] = port;
                count++;
                if (count >= dest_port_max) {
                    break;
                }
            }
            *dest_port_count = count;
        } else {
            hg_pbmp = soc_mem_field32_get(unit, MODPORT_MAPm, entry_p,
                                          HIGIG_PORT_BITMAPf);
            BCM_IF_ERROR_RETURN
                (soc_xgs3_higig_bitmap_to_port_all(unit, hg_pbmp,
                                                   dest_port_max,
                                                   dest_port_array,
                                                   dest_port_count));
        }
    } else
#endif /* BCM_TRX_SUPPORT */

    if (SOC_IS_HBX(unit)) {
        base_index = ing_port * modid_count;
        BCM_IF_ERROR_RETURN
            (READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, base_index + dest_modid,
                               &entry));
        hg_pbmp = soc_mem_field32_get(unit, MODPORT_MAPm, &entry,
                                      HIGIG_PORT_BITMAPf);
        BCM_IF_ERROR_RETURN
            (soc_xgs3_higig_bitmap_to_port_all(unit, hg_pbmp, dest_port_max,
                                               dest_port_array,
                                               dest_port_count));
    } else {
        return BCM_E_UNAVAIL;
    }

    return *dest_port_count ? BCM_E_NONE : BCM_E_NOT_FOUND;
}
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

#ifdef BCM_TRIUMPH_SUPPORT
/*
 * Function:
 *      _bcm_esw_tr_trunk_override_ucast_set
 * Purpose:
 *      Use the memory profile mechanish to set/clear
 *      HiGig Trunk Override bit.
 * Parameters:
 *      unit - SOC unit
 *      port - Ingress Port
 *      tgid - Trunk ID
 *      modid - Module ID  
 *      enable - HighGig Trunk Override set/clear flag.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_tr_trunk_override_ucast_set(int unit, bcm_port_t port,
                               bcm_trunk_t hgtid, int modid, int enable)
{
    int i, rv = BCM_E_NONE;
    soc_profile_mem_t *profile;
    modport_map_entry_t *modport_entry;
    uint32 profile_index_old, profile_index_new, rval, hgo_bitmap, hgo_bit;
    int modid_count;

    if (modport_profile[unit] == NULL) {
        _bcm_stk_modport_profile_init(unit);
    }
    profile = modport_profile[unit];

    modid_count = SOC_MODID_MAX(unit) + 1;
    modport_entry = sal_alloc(sizeof(modport_map_entry_t) * modid_count, "modport_map_entry");
    if (modport_entry == NULL) {
        return BCM_E_MEMORY;
    }

    soc_mem_lock(unit, MODPORT_MAPm);

    rv = READ_MODPORT_MAP_SELr(unit, port, &rval);
    if (BCM_FAILURE(rv)) {
        soc_mem_unlock(unit, MODPORT_MAPm);
        sal_free(modport_entry);
        return rv;
    }

    profile_index_old = modid_count * 
                        soc_reg_field_get(unit, MODPORT_MAP_SELr, rval,
                                           MODPORT_MAP_INDEX_UPPERf);
    for (i = 0; i < modid_count; i++) {
        sal_memcpy(&modport_entry[i], SOC_PROFILE_MEM_ENTRY(unit, profile, void *,
                           profile_index_old + i), sizeof(modport_map_entry_t));
    }

    hgo_bitmap = soc_MODPORT_MAPm_field32_get(unit, &modport_entry[modid], HIGIG_TRUNK_OVERRIDEf);
    hgo_bit = 1 << hgtid;
    if (enable) {
        hgo_bitmap |= hgo_bit;
    } else {
        hgo_bitmap &= ~hgo_bit;
    }
    soc_MODPORT_MAPm_field32_set(unit, &modport_entry[modid],
                                 HIGIG_TRUNK_OVERRIDEf,
                                 hgo_bitmap);

    rv = soc_profile_mem_add(unit, profile,
                             (void *)&modport_entry, modid_count,
                             &profile_index_new);
    if (BCM_SUCCESS(rv)) {
        soc_reg_field_set(unit, MODPORT_MAP_SELr, &rval,
                          MODPORT_MAP_INDEX_UPPERf,
                          profile_index_new/modid_count);
        rv = WRITE_MODPORT_MAP_SELr(unit, port, rval);
    }
    if (BCM_SUCCESS(rv)) {
        rv = soc_profile_mem_delete(unit, profile, profile_index_old);
    }

    soc_mem_unlock(unit, MODPORT_MAPm);
    sal_free(modport_entry);

    return rv;
}

/*
 * Function:
 *      _bcm_esw_tr_trunk_override_ucast_get
 * Purpose:
 *      Use the memory profile mechanish to get
 *      HiGig Trunk Override bit.
 * Parameters:
 *      unit - SOC unit
 *      port - Ingress Port
 *      tgid - Trunk ID
 *      modid - Module ID  
 *      enable - HighGig Trunk Override set/clear flag.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_tr_trunk_override_ucast_get(int unit, bcm_port_t port, 
                                 bcm_trunk_t hgtid, int modid, int *enable) 
{
    int rv = BCM_E_NONE;
    soc_profile_mem_t *profile;
    modport_map_entry_t *modport_entry;
    uint32 profile_index, rval, hgo_bitmap, hgo_bit;
    int modid_count;

    if (modport_profile[unit] == NULL) {
        return BCM_E_NOT_FOUND;
    }

    profile = modport_profile[unit];
    modid_count = SOC_MODID_MAX(unit) + 1;
    modport_entry = sal_alloc(sizeof(modport_map_entry_t), "modport_map_entry");
    if (modport_entry == NULL) {
        return BCM_E_MEMORY;
    }

    rv = READ_MODPORT_MAP_SELr(unit, port, &rval);
    if (BCM_FAILURE(rv)) {
        soc_mem_unlock(unit, MODPORT_MAPm);
        sal_free(modport_entry);
        return rv;
    }

    profile_index = modid_count * 
                    soc_reg_field_get(unit, MODPORT_MAP_SELr, rval,
                                           MODPORT_MAP_INDEX_UPPERf);

    sal_memcpy(modport_entry, SOC_PROFILE_MEM_ENTRY(unit, profile, void *,
                       profile_index + modid), sizeof(modport_map_entry_t));

    hgo_bitmap = soc_MODPORT_MAPm_field32_get(unit, modport_entry,
                                        HIGIG_TRUNK_OVERRIDEf);
    hgo_bit = 1 << hgtid;
    *enable = (hgo_bitmap & hgo_bit) ? 1 : 0;

    sal_free(modport_entry);
  
    return rv;
}
#endif /* BCM_TRIUMPH_SUPPORT */


/*
 * Function:
 *      bcm_stk_modport_set
 * Purpose:
 *      Set the port in MODPORT entry for modid
 * Parameters:
 *      unit  - SOC unit#
 *      modid - module id
 *      port - port number
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Supported by all devices except Strata
 */

int
bcm_esw_stk_modport_set(int unit, int modid, bcm_port_t port)
{
    soc_reg_t reg;
    soc_field_t field;
    uint32 addr;
    uint32 modport;
    uint64 modport64;
    int p;

    COMPILER_REFERENCE(reg);
    COMPILER_REFERENCE(field);
    COMPILER_REFERENCE(p);
    COMPILER_REFERENCE(addr);
    COMPILER_REFERENCE(modport);
    COMPILER_REFERENCE(modport64);

    STK_OUT(("STK %d: modport set: modid %d to port %d\n", unit,
             modid, port));

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
        return BCM_E_PARAM;
    }
    if (BCM_GPORT_IS_SET(port) && !BCM_GPORT_IS_TRUNK(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    switch (BCM_CHIP_FAMILY(unit)) {

#if defined(BCM_DRACO_SUPPORT) || defined(BCM_LYNX_SUPPORT)
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_LYNX:

        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }

        field = _bcm_stk_MODPORT_f(modid);

        /* MODPORT tables are identical for all ports */

        if (modid <= 31) {
            /* GE, XE ports */
            reg = _bcm_stk_MODPORT_reg(modid);
            PBMP_E_ITER(unit, p) {
                addr = soc_reg_addr(unit, reg, p, 0);
                BCM_IF_ERROR_RETURN(soc_reg32_get(unit, reg, p, 0, &modport));
                soc_reg_field_set(unit, reg, &modport, field, port);
                BCM_IF_ERROR_RETURN(soc_reg32_set(unit, reg, p, 0, modport));
            }
        }

        /* HIGIG ports */
        reg = _bcm_stk_IMODPORT_reg(modid);
        PBMP_HG_ITER(unit, p) {
            addr = soc_reg_addr(unit, reg, p, 0);
            BCM_IF_ERROR_RETURN(soc_reg32_get(unit, reg, p, 0, &modport));
            soc_reg_field_set(unit, reg, &modport, field, port);
            BCM_IF_ERROR_RETURN(soc_reg32_set(unit, reg, p, 0, modport));
        }
        return BCM_E_NONE;
#endif

#if defined(BCM_TUCANA_SUPPORT)
    case BCM_FAMILY_TUCANA:

        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }

        reg = _bcm_stk_MODPORT_reg(modid);
        field = _bcm_stk_MODPORT_f(modid);

        /* GE, FE ports */
        PBMP_E_ITER(unit, p) {
            addr = soc_reg_addr(unit, reg, p, 0);
            soc_reg64_get(unit, reg, p, 0, &modport64);
            soc_reg64_field32_set(unit, reg, &modport64, field, port);
            soc_reg64_set(unit, reg, p, 0, modport64);
        }

        /* HIGIG ports */
        reg = _bcm_stk_IMODPORT_reg(modid);
        PBMP_HG_ITER(unit, p) {
            addr = soc_reg_addr(unit, reg, p, 0);
            soc_reg64_get(unit, reg, p, 0, &modport64);
            soc_reg64_field32_set(unit, reg, &modport64, field, port);
            soc_reg64_set(unit, reg, p, 0, modport64);
        }
        return BCM_E_NONE;
#endif

#if defined(BCM_FIREBOLT_SUPPORT)
    case BCM_FAMILY_FIREBOLT:
        {
            modport_map_entry_t uc;
            uint32 hg_pbm;
            int rv;

            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }

            /* Convert port to HG bitmap for FBX devices other than Raven */
            /* For Raven, use the absolute bitmap */
            if (SOC_IS_RAVEN(unit)) {
                if (IS_ST_PORT(unit, port) || IS_CPU_PORT(unit, port)) {
                    hg_pbm = 1 << port;
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            } else if (SOC_IS_HAWKEYE(unit)) {
                return BCM_E_UNAVAIL;
            } else {
                rv = soc_xgs3_port_to_higig_bitmap(unit, port, &hg_pbm);
            }
            if (rv != BCM_E_NONE) {
                return BCM_E_PORT;
            }

            soc_mem_lock(unit, MODPORT_MAPm);
            rv = READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, modid, &uc);
            if (rv >= 0) {
                soc_MODPORT_MAPm_field32_set(unit, &uc, HIGIG_PORT_BITMAPf,
                                             hg_pbm);
                rv = WRITE_MODPORT_MAPm(unit, MEM_BLOCK_ALL, modid, &uc);
            }
            soc_mem_unlock(unit, MODPORT_MAPm);
            return rv;
        }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_TRIUMPH2:
        {
            bcm_port_t ing_port;
            PBMP_ALL_ITER(unit, ing_port) {
                BCM_IF_ERROR_RETURN
                    (_bcm_stk_port_modport_op(unit,
                                              _BCM_STK_PORT_MODPORT_OP_SET,
                                              ing_port, modid, &port, 1));
            }
            break;
        }
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
    case BCM_FAMILY_TRIDENT:
        {
            bcm_port_t ing_port;
            PBMP_ALL_ITER(unit, ing_port) {
                BCM_IF_ERROR_RETURN
                    (bcm_td_stk_port_modport_op(unit,
                                                _BCM_STK_PORT_MODPORT_OP_SET,
                                                ing_port, modid, &port, 1));
            }
            break;
        }
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_XGS_FABRIC_SUPPORT)
    case BCM_FAMILY_HERCULES:
    case BCM_FAMILY_HERCULES15:
        {
            int ing_port;
            bcm_pbmp_t pbmp;

            if (!SOC_PORT_VALID(unit, port)) {
                BCM_PBMP_CLEAR(pbmp);
            } else {
                BCM_PBMP_PORT_SET(pbmp, port);
            }

            PBMP_HG_ITER(unit, ing_port) {
                BCM_IF_ERROR_RETURN(bcm_esw_stk_ucbitmap_set(unit, ing_port,
                                                             modid, pbmp));
            }

            return BCM_E_NONE;
        }
#endif

#if defined(BCM_EASYRIDER_SUPPORT)
    case BCM_FAMILY_EASYRIDER:
        {
            int reg_idx, reg_ofs;
            int rv = BCM_E_INTERNAL;
            uint32 data;

            if (port < 0 || port > 16) {
                return BCM_E_PARAM;
            }

            reg_idx = (modid >> 3) & 7;
            reg_ofs =  modid       & 7;

            rv = READ_MODPORT_MAPr(unit, reg_idx, &data);
            if (rv >= 0) {
                data |= (port << (reg_ofs*4));
                rv = WRITE_MODPORT_MAPr(unit, reg_idx, data);
            }
            return rv;
        }
#endif

    default:
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_stk_modport_get_all
 * Purpose:
 *      Get all ports in MODPORT entry for modid
 * Parameters:
 *      unit     - (IN) Device Number
 *      modid      - (IN) module id
 *      port_max   - (IN) Maximum number of ports in array
 *      port_array - (OUT) Array of ports
 *      port_count - (OUT) Number of ports returned in array
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Supported by all devices except Strata and Hawkeye
 */
int
bcm_esw_stk_modport_get_all(int unit,
                            int modid,
                            int port_max,
                            bcm_port_t *port_array,
                            int *port_count)
{
    soc_reg_t   reg;
    soc_field_t field;
    int         p, tmp_port, mymodid;
    uint32      modport;
    uint64      modport64;
    int         count, isGport;
    _bcm_gport_dest_t   gport_st;

    COMPILER_REFERENCE(reg);
    COMPILER_REFERENCE(field);
    COMPILER_REFERENCE(p);
    COMPILER_REFERENCE(modport);
    COMPILER_REFERENCE(modport64);
    COMPILER_REFERENCE(count);

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
        return BCM_E_PARAM;
    }

    if (port_max > 0 && port_array == NULL) {
        return BCM_E_PARAM;
    }

    count = 0;
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
    /* return code intentionaly ignored to support devices where mymodid */
    /* was not assigned */
    (void) bcm_esw_stk_my_modid_get(unit, &mymodid);

    switch (BCM_CHIP_FAMILY(unit)) {

#if defined(BCM_DRACO_SUPPORT) || defined(BCM_LYNX_SUPPORT)
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_LYNX:
        if (SOC_IS_DRACO15(unit)) {
            if (modid > 31) {
                reg = _bcm_stk_IMODPORT_reg(modid);
            }
            else {
                reg = _bcm_stk_MODPORT_reg(modid);
            }
        }
        else {
            reg = _bcm_stk_MODPORT_reg(modid);
        }
        field = _bcm_stk_MODPORT_f(modid);

        /* get info from an appropriate port */
        PBMP_E_ITER(unit, p) {
            if ((modid > 31) && !IS_HG_PORT(unit,p)) {
                continue;
            }
            SOC_IF_ERROR_RETURN
                (soc_reg32_get(unit, reg, p, 0, &modport));
            if (count < port_max) {
                tmp_port = soc_reg_field_get(unit, reg, modport, field);
                if (TRUE == isGport) {
                    if (BCM_MODID_INVALID == mymodid) {
                        gport_st.gport_type = _SHR_GPORT_TYPE_DEVPORT;
                    } else {
                        gport_st.gport_type = _SHR_GPORT_TYPE_MODPORT;
                        gport_st.modid = mymodid;
                    }
                    gport_st.port = tmp_port;
                    BCM_IF_ERROR_RETURN(
                        _bcm_esw_gport_construct(unit, &gport_st, &tmp_port));
                }
                port_array[count++] = tmp_port;
            }
            break;
        }

        if (port_count) {
            *port_count = count;
        }

        return ((count > 0) ? BCM_E_NONE : BCM_E_NOT_FOUND);
#endif

#if defined(BCM_TUCANA_SUPPORT)
    case BCM_FAMILY_TUCANA:
        reg = _bcm_stk_MODPORT_reg(modid);
        field = _bcm_stk_MODPORT_f(modid);

        PBMP_E_ITER(unit, p) {
            SOC_IF_ERROR_RETURN
                (soc_reg64_get(unit, reg, p, 0, &modport64));
            if (count < port_max) {
                tmp_port = soc_reg64_field32_get(unit, reg, modport64, field);
                if (TRUE == isGport) {
                    if (BCM_MODID_INVALID == mymodid) {
                        gport_st.gport_type = _SHR_GPORT_TYPE_DEVPORT;
                    } else {
                        gport_st.gport_type = _SHR_GPORT_TYPE_MODPORT;
                        gport_st.modid = mymodid;
                    }
                    gport_st.port = tmp_port;
                    BCM_IF_ERROR_RETURN(
                        _bcm_esw_gport_construct(unit, &gport_st, &tmp_port));
                }

                port_array[count++] = tmp_port;
                    
            }
            break;
        }

        if (port_count) {
            *port_count = count;
        }

        return ((count > 0) ? BCM_E_NONE : BCM_E_NOT_FOUND);
#endif

#if defined(BCM_FIREBOLT_SUPPORT)
    case BCM_FAMILY_FIREBOLT:
        {
            modport_map_entry_t uc;
            int rv;
            int hg_reg;

            SOC_IF_ERROR_RETURN
                (READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, modid, &uc));
            hg_reg = soc_MODPORT_MAPm_field32_get(unit, &uc,
                                                  HIGIG_PORT_BITMAPf);

            /* For Raven, hg_reg directly corresponds to the physical port */
            if (SOC_IS_RAVEN(unit)) {
                int port;

                port = 0;
                while (hg_reg != 0 && count < port_max) {
                    if (hg_reg & 1) {
                        tmp_port = port; 
                        if (TRUE == isGport) {
                            if (BCM_MODID_INVALID == mymodid) {
                            gport_st.gport_type = _SHR_GPORT_TYPE_DEVPORT;
                            } else {
                                gport_st.gport_type = _SHR_GPORT_TYPE_MODPORT;
                                gport_st.modid = mymodid;
                            }
                            gport_st.port = tmp_port;
                            BCM_IF_ERROR_RETURN(
                                _bcm_esw_gport_construct(unit, &gport_st, 
                                                         &tmp_port));
                        }
                        port_array[count++] = tmp_port;
                    }
                    port++;
                    hg_reg >>= 1;
                }

                if (port_count) {
                    *port_count = count;
                }

                return (count > 0) ? BCM_E_NONE : BCM_E_NOT_FOUND;

            } else if (SOC_IS_HAWKEYE(unit)) {
                rv = BCM_E_UNAVAIL;
            } else {
                rv = soc_xgs3_higig_bitmap_to_port_all(unit,
                                                       hg_reg,
                                                       port_max,
                                                       port_array,
                                                       &count);
                if (BCM_SUCCESS(rv) && TRUE == isGport) {
                    int i;

                    for (i = 0; i < count; i++) {
                        tmp_port = port_array[i];
                        if (BCM_MODID_INVALID == mymodid) {
                            gport_st.gport_type = _SHR_GPORT_TYPE_DEVPORT;
                        } else {
                            gport_st.gport_type = _SHR_GPORT_TYPE_MODPORT;
                            gport_st.modid = mymodid;
                        } 
                        gport_st.port = tmp_port;
                        BCM_IF_ERROR_RETURN(
                            _bcm_esw_gport_construct(unit, &gport_st, 
                                                     &tmp_port));
                        port_array[i] = tmp_port;
                    }
                }
                
                if (port_count) {
                    *port_count = count;
                }
            }

            return rv;
        }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_TRIUMPH2:
    case BCM_FAMILY_TRIDENT:
        {
            bcm_port_t ing_port;
            int count, rv;

            PBMP_ALL_ITER(unit, ing_port) {
#ifdef BCM_TRIDENT_SUPPORT
                if (BCM_CHIP_FAMILY(unit) == BCM_FAMILY_TRIDENT) {
                    rv = bcm_td_stk_port_modport_get(unit, ing_port, modid,
                            port_max, port_array, &count);
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    rv =_bcm_stk_port_modport_get(unit, ing_port, modid,
                            port_max, port_array, &count);
                }
                if (BCM_FAILURE(rv) && rv != BCM_E_NOT_FOUND) {
                    return rv;
                }
                if (port_count) {
                    *port_count = count;
                }
                if (TRUE == isGport) {
                    int i;

                    for (i = 0; i < count; i++) {
                        if (BCM_GPORT_IS_TRUNK(port_array[i])) {
                            continue;
                        }
                        tmp_port = port_array[i];
                        if (BCM_MODID_INVALID == mymodid) {
                            gport_st.gport_type = _SHR_GPORT_TYPE_DEVPORT;
                        } else {
                            gport_st.gport_type = _SHR_GPORT_TYPE_MODPORT;
                            gport_st.modid = mymodid;
                        } 
                        gport_st.port = tmp_port;
                        BCM_IF_ERROR_RETURN(
                            _bcm_esw_gport_construct(unit, &gport_st, 
                                                     &tmp_port));
                        port_array[i] = tmp_port;
                    }
                }

                return rv;
            }
            break;
        }
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

#if defined(BCM_XGS_FABRIC_SUPPORT)
    case BCM_FAMILY_HERCULES:
    case BCM_FAMILY_HERCULES15:
        {
            int ing_port;
            bcm_pbmp_t pbmp;

            /*
             * Iterate through the ingress ports.
             * Find the first ingress port with a non-null bitmap
             * for the modid; return the first port in that bitmap.
             */

            PBMP_HG_ITER(unit, ing_port) {
                BCM_IF_ERROR_RETURN(bcm_esw_stk_ucbitmap_get(unit, ing_port,
                                                             modid, &pbmp));
                if (BCM_PBMP_NOT_NULL(pbmp)) {
                    bcm_port_t port;

                    BCM_PBMP_ITER(pbmp, port) {
                        if (count < port_max) {
                            tmp_port = port; 
                            if (TRUE == isGport) {
                                if (BCM_MODID_INVALID == mymodid) {
                                    gport_st.gport_type = _SHR_GPORT_TYPE_DEVPORT;
                                } else {
                                    gport_st.gport_type = _SHR_GPORT_TYPE_MODPORT;
                                    gport_st.modid = mymodid;
                                }
                                gport_st.port = tmp_port;
                                BCM_IF_ERROR_RETURN(
                                    _bcm_esw_gport_construct(unit, &gport_st, 
                                                             &tmp_port));
                            }
                            port_array[count++] = tmp_port;
                        } else {
                            break;
                        }
                    }
                    break;
                }
            }

            if (port_count) {
                *port_count = count;
            }

            return (count > 0) ? BCM_E_NONE : BCM_E_NOT_FOUND;
        }
#endif

        
#if defined(BCM_EASYRIDER_SUPPORT)
    case BCM_FAMILY_EASYRIDER:
        {
            int reg_idx, reg_ofs;
            int rv = BCM_E_INTERNAL;
            uint32 data;

            reg_idx = (modid >> 3) & 7;
            reg_ofs =  modid       & 7;

            rv = READ_MODPORT_MAPr(unit, reg_idx, &data);
            if (BCM_SUCCESS(rv)) {
                if (count < port_max) {
                    tmp_port = ((data >> (reg_ofs*4)) & 0xf);; 
                    if (TRUE == isGport) {
                        if (BCM_MODID_INVALID == mymodid) {
                            gport_st.gport_type = _SHR_GPORT_TYPE_DEVPORT;
                        } else {
                            gport_st.gport_type = _SHR_GPORT_TYPE_MODPORT;
                            gport_st.modid = mymodid;
                        }
                        gport_st.port = tmp_port;
                        BCM_IF_ERROR_RETURN(
                            _bcm_esw_gport_construct(unit, &gport_st, 
                                                     &tmp_port));
                    }
                    port_array[count++] = tmp_port;
                }
                rv = BCM_E_NONE;
            }

            if (port_count) {
                *port_count = count;
            }

            return (count > 0) ? BCM_E_NONE : BCM_E_NOT_FOUND;
        }
#endif

    default:
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_stk_modport_get
 * Purpose:
 *      Get the port in MODPORT entry for modid
 * Parameters:
 *      unit  - SOC unit#
 *      modid - module id
 *      port  - (OUT) port number
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Supported by all devices except Strata
 */

int
bcm_esw_stk_modport_get(int unit, int modid, bcm_port_t *port)
{
    if (port == NULL) {
        return BCM_E_PARAM;
    }

    *port = -1;
    return bcm_esw_stk_modport_get_all(unit, modid, 1, port, NULL);
}


/*
 * Function:
 *      bcm_stk_modport_clear
 * Purpose:
 *      Clear entry in MODPORT table
 * Parameters:
 *      unit  - SOC unit#
 *      modid - module id
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Supported by all devices except Strata
 */

int
bcm_esw_stk_modport_clear(int unit, int modid)
{
    bcm_pbmp_t pbmp;
    int port;
    int rv;
    int use_modport_set;
    STK_VERB(("STK %d: Clearing mod port info of modid %d.\n", unit, modid));

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
        return BCM_E_PARAM;
    }

    /* First try to clear the modport bitmap, if available */

    rv = BCM_E_INTERNAL;
    use_modport_set = FALSE;
    BCM_PBMP_CLEAR(pbmp);

    /* Go though all ports, because on different devices, some or all
       of their ports may have a modport map. We'll be conservative
       and clear everything; this will catch cases like having stack
       ports with modport map settings that get converted into
       non-stack ports still getting those modport maps cleared. */
    PBMP_ALL_ITER(unit, port) {
        rv = bcm_esw_stk_ucbitmap_set(unit, port, modid, pbmp);
        if (rv == BCM_E_NONE) {
            /* Cleared the modport map so modport_set not needed,
               keep going */
        } else if (rv == BCM_E_UNAVAIL) {
            /* Do need modport_set, stop */
            use_modport_set = TRUE;
            break;
        } else if (rv == BCM_E_PORT) {
            /* no modmap for this port, ignore error and keep going */
        } else {
            /* some other error, stop, do not use modport_set */
            break;
        }
    }

    /* Set the modport to a stack port, if available. This should only
       happen on XGS switch devices that only have one stack port and
       do not have a modport bitmap. */

    if (use_modport_set) {
        port = IPIC_PORT(unit);
        if (port < 0) {
            port = SOC_PORT_MIN(unit, st);
            if (port < 0) {
                /* just clear the modport if a stack port cannot be
                   found */
                port = 0;
            }
        }
        rv = bcm_esw_stk_modport_set(unit, modid, port);
    }

    return rv;
}

/*
 * Function:
 *      bcm_stk_modport_clear_all
 * Purpose:
 *      Clear all entries in MODPORT table
 * Parameters:
 *      unit  - SOC unit#
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Supported by all devices except Strata
 */

int
bcm_esw_stk_modport_clear_all(int unit)
{
    uint32 modport;
    uint64 modport64;
    int i, port;
    int addr, num_regs;
    soc_reg_t reg;

    COMPILER_REFERENCE(modport);
    COMPILER_REFERENCE(modport64);
    COMPILER_REFERENCE(port);
    COMPILER_REFERENCE(reg);
    COMPILER_REFERENCE(addr);
    COMPILER_REFERENCE(i);
    COMPILER_REFERENCE(num_regs);

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    STK_VERB(("STK %d: Clearing all mod port info.\n", unit));

    switch (BCM_CHIP_FAMILY(unit)) {
#if defined(BCM_DRACO_SUPPORT) || defined(BCM_LYNX_SUPPORT)
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_LYNX:
        /* Default for MODPORT table is port HIGIG for all MODIDs */
        port = IPIC_PORT(unit);
        if (port < 0) {
            port = 0;
        }
        modport = port;
        for (i = 1; i < 8; i++) {
            modport = (modport << 4) | port;
        }

        /* GE, XE blocks */
        PBMP_E_ITER(unit, port) {
            for (i = 0; i < COUNTOF(_bcm_stk_xgs_mp_r); i++) {
                reg = _bcm_stk_xgs_mp_r[i];
                BCM_IF_ERROR_RETURN(soc_reg32_set(unit, reg, port, 0, modport));
            }
        }

        num_regs = SOC_IS_DRACO15(unit) ? 8 : 4;
        PBMP_HG_ITER(unit, port) {
            for (i = 0; i < num_regs; i++) {
                reg = _bcm_stk_xgs_imp_r[i];
                BCM_IF_ERROR_RETURN(soc_reg32_set(unit, reg, port, 0, modport));
            }
        }

        return BCM_E_NONE;
#endif

#if defined(BCM_TUCANA_SUPPORT)
    case BCM_FAMILY_TUCANA:
        /* Default for MODPORT table is port HIGIG for all MODIDs */
        port = IPIC_PORT(unit);
        if (port < 0) {
            port = 0;
        }
        COMPILER_64_ZERO(modport64);
        for (i = 0; i < 8; i++) {
            soc_reg64_field32_set(unit, MODPORT_7_0r, &modport64,
                                  _bcm_stk_xgs_mp_f[i], port);
        }

        /* GE & FE blocks */
        PBMP_E_ITER(unit, port) {
            for (i = 0; i < COUNTOF(_bcm_stk_xgs_mp_r); i++) {
                reg = _bcm_stk_xgs_mp_r[i];
                soc_reg64_set(unit, reg, port, 0, modport64);
            }
        }

        /* HIGIG */
        PBMP_HG_ITER(unit, port) {
            for (i = 0; i < 4; i++) {
                reg = _bcm_stk_xgs_imp_r[i];
                soc_reg64_set(unit, reg, port, 0, modport64);
            }
        }

        return BCM_E_NONE;
#endif

#if defined(BCM_FIREBOLT_SUPPORT) || defined(BCM_EASYRIDER_SUPPORT) || \
        defined(BCM_XGS_FABRIC_SUPPORT)
    case BCM_FAMILY_FIREBOLT:
    case BCM_FAMILY_EASYRIDER:
    case BCM_FAMILY_HERCULES:
    case BCM_FAMILY_HERCULES15:
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH2:
    case BCM_FAMILY_TRIDENT:
        for ( i = 0; i <= SOC_MODID_MAX(unit); i++) {
            bcm_esw_stk_modport_clear(unit, i);
        }
        break;
#endif

    default:
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_stk_modport_add
 * Purpose:
 *      Adds a HG port given a destination modid
 * Parameters:
 *      unit   -  SOC unit#
 *      modid  -  module id
 *      port   -  egress HG port
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_modport_add(int unit, int modid, bcm_port_t port)
{
    int rv = BCM_E_UNAVAIL;

    STK_OUT(("STK %d: modport add: modid %d to port %d\n", unit,
             modid, port));

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
        return BCM_E_PARAM;
    }
    if (BCM_GPORT_IS_SET(port) && !BCM_GPORT_IS_TRUNK(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    switch (BCM_CHIP_FAMILY(unit)) {

#if defined(BCM_FIREBOLT_SUPPORT)
    case BCM_FAMILY_FIREBOLT:
        {
            modport_map_entry_t uc;
            uint32 hg_pbm = 0, modport_pbm = 0;

            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }

            /* Convert port to HG bitmap */

            if (SOC_IS_RAVEN(unit)) {
                if (IS_ST_PORT(unit, port) || IS_CPU_PORT(unit, port)) {
                    hg_pbm = 1 << port;
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            } else if (SOC_IS_HAWKEYE(unit)) {
                return BCM_E_UNAVAIL;
            } else {
                rv = soc_xgs3_port_to_higig_bitmap(unit, port, &hg_pbm);
            }
            if (rv != BCM_E_NONE) {
                return BCM_E_PORT;
            }

            soc_mem_lock(unit, MODPORT_MAPm);
            rv = READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, modid, &uc);
            if (rv >= 0) {
                modport_pbm = soc_MODPORT_MAPm_field32_get(unit, &uc,
                                                           HIGIG_PORT_BITMAPf);
                if (modport_pbm & hg_pbm) {
                    soc_mem_unlock(unit, MODPORT_MAPm);
                    return BCM_E_NONE;
                }

                modport_pbm |= hg_pbm;
                soc_MODPORT_MAPm_field32_set(unit, &uc, HIGIG_PORT_BITMAPf,
                                             modport_pbm);
                rv = WRITE_MODPORT_MAPm(unit, MEM_BLOCK_ALL, modid, &uc);
            }
            soc_mem_unlock(unit, MODPORT_MAPm);
            break;
        }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_TRIUMPH2:
        {
            bcm_port_t ing_port;
            PBMP_ALL_ITER(unit, ing_port) {
                BCM_IF_ERROR_RETURN
                    (_bcm_stk_port_modport_op(unit,
                                              _BCM_STK_PORT_MODPORT_OP_ADD,
                                              ing_port, modid, &port, 1));
            }
            rv = BCM_E_NONE;
            break;
        }
#if defined(BCM_TRIDENT_SUPPORT)
    case BCM_FAMILY_TRIDENT:
        {
            bcm_port_t ing_port;
            PBMP_ALL_ITER(unit, ing_port) {
                BCM_IF_ERROR_RETURN
                    (bcm_td_stk_port_modport_op(unit,
                                              _BCM_STK_PORT_MODPORT_OP_ADD,
                                              ing_port, modid, &port, 1));
            }
            rv = BCM_E_NONE;
            break;
        }
#endif /* BCM_TRIDENT_SUPPORT */
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

    default:
        break;
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_stk_modport_delete
 * Purpose:
 *      Removes a HG (egress) port for given a destination modid
 * Parameters:
 *      unit   -  SOC unit#
 *      modid  -  module id
 *      port   -  egress HG port
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_modport_delete(int unit, int modid, bcm_port_t port)
{
    int rv = BCM_E_UNAVAIL;

    STK_OUT(("STK %d: modport delete: modid %d to port %d\n", unit,
             modid, port));

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
        return BCM_E_PARAM;
    }
    if (BCM_GPORT_IS_SET(port) && !BCM_GPORT_IS_TRUNK(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    switch (BCM_CHIP_FAMILY(unit)) {

#if defined(BCM_FIREBOLT_SUPPORT)
    case BCM_FAMILY_FIREBOLT:
        {
            modport_map_entry_t uc;
            uint32 modport_pbm = 0, hg_pbm = 0;

            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }

            /* Convert port to HG bitmap */
            if (SOC_IS_RAVEN(unit)) {
                if (IS_ST_PORT(unit, port) || IS_CPU_PORT(unit, port)) {
                    hg_pbm = 1 << port;
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            } else if (SOC_IS_HAWKEYE(unit)) {
                return BCM_E_UNAVAIL;
            } else {
                rv = soc_xgs3_port_to_higig_bitmap(unit, port, &hg_pbm);
            }
            if (rv != BCM_E_NONE) {
                return BCM_E_PORT;
            }

            soc_mem_lock(unit, MODPORT_MAPm);
            rv = READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, modid, &uc);
            if (rv >= 0) {
                modport_pbm = soc_MODPORT_MAPm_field32_get(unit, &uc,
                                                      HIGIG_PORT_BITMAPf);
                if (!(modport_pbm & hg_pbm)) {
                    soc_mem_unlock(unit, MODPORT_MAPm);
                    return BCM_E_NOT_FOUND;
                }

                modport_pbm &= ~hg_pbm;
                soc_MODPORT_MAPm_field32_set(unit, &uc, HIGIG_PORT_BITMAPf,
                                             modport_pbm);
                rv = WRITE_MODPORT_MAPm(unit, MEM_BLOCK_ALL, modid, &uc);
            }
            soc_mem_unlock(unit, MODPORT_MAPm);
            break;
        }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_TRIUMPH2:
        {
            bcm_port_t ing_port;
            PBMP_ALL_ITER(unit, ing_port) {
                BCM_IF_ERROR_RETURN
                    (_bcm_stk_port_modport_op(unit,
                                              _BCM_STK_PORT_MODPORT_OP_DELETE,
                                              ing_port, modid, &port, 1));
            }
            rv = BCM_E_NONE;
            break;
        }
#if defined(BCM_TRIDENT_SUPPORT)
    case BCM_FAMILY_TRIDENT:
        {
            bcm_port_t ing_port;
            PBMP_ALL_ITER(unit, ing_port) {
                BCM_IF_ERROR_RETURN
                    (bcm_td_stk_port_modport_op(unit,
                                              _BCM_STK_PORT_MODPORT_OP_DELETE,
                                              ing_port, modid, &port, 1));
            }
            rv = BCM_E_NONE;
            break;
        }
#endif /* BCM_TRIDENT_SUPPORT */
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

    default:
        break;
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_stk_port_modport_set
 * Purpose:
 *      Set a destination path for a given destination module of the ingress
 *      port.
 * Parameters:
 *      unit       - (IN) Device number
 *      ing_port   - (IN) Ingress port
 *      dest_modid - (IN) Destination module id
 *      dest_port  - (IN) Destination port
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_port_modport_set(int unit, bcm_port_t ing_port,
                             bcm_module_t dest_modid, bcm_port_t dest_port)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    STK_OUT(("STK %d: port modport set: ing port %d modid %d to port %d\n",
             unit, ing_port, dest_modid, dest_port));

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        return bcm_td_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_SET,
                ing_port, dest_modid, &dest_port, 1);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    {
        return _bcm_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_SET,
                ing_port, dest_modid, &dest_port, 1);
    }
#else /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_stk_port_modport_get
 * Purpose:
 *      Get a destination path for a given destination module of the ingress
 *      port.
 * Parameters:
 *      unit       - (IN) Device number
 *      ing_port   - (IN) Ingress port
 *      dest_modid - (IN) Destination module id
 *      dest_port  - (OUT) Destination port
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_port_modport_get(int unit, bcm_port_t ing_port,
                             bcm_module_t dest_modid, bcm_port_t *dest_port)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    int dest_port_count;

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        return bcm_td_stk_port_modport_get(unit, ing_port, dest_modid, 1, dest_port,
                &dest_port_count);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    {
        return _bcm_stk_port_modport_get(unit, ing_port, dest_modid, 1, dest_port,
                &dest_port_count);
    }
#else /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_stk_port_modport_get_all
 * Purpose:
 *      Get all destination paths for a given destination module of the
 *      ingress port.
 * Parameters:
 *      unit            - (IN) Device number
 *      ing_port        - (IN) Ingress port
 *      dest_modid      - (IN) Destination module id
 *      dest_port_max   - (IN) Maximum number of ports in array
 *      dest_port_array - (OUT) Array of ports
 *      dest_port_count - (OUT) Number of ports returned in array
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_port_modport_get_all(int unit, bcm_port_t ing_port,
                                 bcm_module_t dest_modid, int dest_port_max,
                                 bcm_port_t *dest_port_array,
                                 int *dest_port_count)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        return bcm_td_stk_port_modport_get(unit, ing_port, dest_modid, dest_port_max,
                dest_port_array, dest_port_count);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    {
        return _bcm_stk_port_modport_get(unit, ing_port, dest_modid, dest_port_max,
                dest_port_array, dest_port_count);
    }
#else /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_stk_port_modport_clear
 * Purpose:
 *      Clear all destination paths for a given destination module of the
 *      ingress port.
 * Parameters:
 *      unit       - (IN) Device number
 *      ing_port   - (IN) Ingress port
 *      dest_modid - (IN) Destination module id
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_port_modport_clear(int unit, bcm_port_t ing_port,
                               bcm_module_t dest_modid)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    STK_OUT(("STK %d: port modport clear: ing port %d modid %d\n",
             unit, ing_port, dest_modid));

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        return bcm_td_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_SET,
                ing_port, dest_modid, NULL, 0);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    {
        return _bcm_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_SET,
                ing_port, dest_modid, NULL, 0);
    }
#else /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_stk_port_modport_clear_all
 * Purpose:
 *      Clear destination paths for all destination modules of the ingress
 *      port.
 * Parameters:
 *      unit       - (IN) Device number
 *      ing_port   - (IN) Ingress port
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_port_modport_clear_all(int unit, bcm_port_t ing_port)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    STK_OUT(("STK %d: port modport clear all: ing port %d\n", unit, ing_port));

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        return bcm_td_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_SET,
                ing_port, -1, NULL, 0);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    {
        return _bcm_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_SET,
                ing_port, -1, NULL, 0);
    }
#else /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_stk_port_modport_add
 * Purpose:
 *      Add one path for a given destination module of the ingress port.
 * Parameters:
 *      unit       - (IN) Device number
 *      ing_port   - (IN) Ingress port
 *      dest_modid - (IN) Destination module id
 *      dest_port  - (IN) Destination port
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_port_modport_add(int unit, bcm_port_t ing_port,
                             bcm_module_t dest_modid, bcm_port_t dest_port)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    STK_OUT(("STK %d: port modport add: ing port %d modid %d to port %d\n",
             unit, ing_port, dest_modid, dest_port));

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        return bcm_td_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_ADD,
                ing_port, dest_modid, &dest_port, 1);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    {
        return _bcm_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_ADD,
                ing_port, dest_modid, &dest_port, 1);
    }
#else /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_stk_port_modport_delete
 * Purpose:
 *      Delete one path for a given destination module of the ingress port.
 * Parameters:
 *      unit       - (IN) Device number
 *      ing_port   - (IN) Ingress port
 *      dest_modid - (IN) Destination module id
 *      dest_port  - (IN) Destination port
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_stk_port_modport_delete(int unit, bcm_port_t ing_port,
                                bcm_module_t dest_modid, bcm_port_t dest_port)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    STK_OUT(("STK %d: port modport delete: ing port %d modid %d to port %d\n",
             unit, ing_port, dest_modid, dest_port));

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        return bcm_td_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_DELETE,
                ing_port, dest_modid, &dest_port, 1);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    {
        return _bcm_stk_port_modport_op(unit, _BCM_STK_PORT_MODPORT_OP_DELETE,
                ing_port, dest_modid, &dest_port, 1);
    }
#else /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
}

#ifdef _BCM_STK_FLAGS_GET

/****************************************************************
 *
 * Get port's stack flags, per device type
 *
 ****************************************************************/

/* Local macro to set the simplex or duplex flags */
#define SIM_DUP_SET(flags, sim) \
     (flags) |= ((sim) ? BCM_STK_SIMPLEX : BCM_STK_DUPLEX)

STATIC int
_xgs_stk_flags_get(int unit, bcm_port_t port, uint32 *flags)
{
    if (SOC_IS_DRACO(unit)) {
#if defined(BCM_DRACO_SUPPORT)
        uint32 config;
        uint64 iconfig;

        if (IS_GE_PORT(unit, port)) {
            *flags |= BCM_STK_SL;
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config));
            SIM_DUP_SET(*flags, soc_reg_field_get(unit, CONFIGr, config,
                                                  ST_SIMPLEXf));
        } else if (IS_HG_PORT(unit, port)) {
            *flags |= BCM_STK_HG;
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            SIM_DUP_SET(*flags, soc_reg64_field32_get(unit, ICONFIGr, iconfig,
                                                      IPIC_CASCADEf));
        }
#endif
    } else if (SOC_IS_TUCANA(unit)) {
#if defined(BCM_TUCANA_SUPPORT)
        port_tab_entry_t pent;
        uint64 iconfig;

        if (IS_GE_PORT(unit, port)) {
            *flags |= BCM_STK_SL;
            soc_mem_read(unit, SOC_PORT_MEM_TAB(unit, port), MEM_BLOCK_ANY,
                         SOC_PORT_MOD_OFFSET(unit, port), &pent);
            SIM_DUP_SET(*flags, soc_PORT_TABm_field32_get(unit, &pent,
                                                          ST_SIMPLEXf));
        } else if (IS_HG_PORT(unit, port)) {
            *flags |= BCM_STK_HG;
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            SIM_DUP_SET(*flags, soc_reg64_field32_get(unit, ICONFIGr, iconfig,
                                                      IPIC_CASCADEf));
        }
#endif
    } else if (SOC_IS_LYNX(unit)) {
#if defined(BCM_LYNX_SUPPORT)
        uint64 iconfig;

        if (IS_HG_PORT(unit, port)) {
            *flags |= BCM_STK_HG;
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            SIM_DUP_SET(*flags, soc_reg64_field32_get(unit, ICONFIGr, iconfig,
                                                      IPIC_CASCADEf));
        }
#endif
    } else {
        return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

#undef SIM_DUP_SET

STATIC int
_esw_stk_flags_get(int unit, bcm_port_t port, uint32 *flags)
{
    if (!SOC_IS_STACK_PORT(unit, port)) {
        *flags = BCM_STK_NONE;
        return BCM_E_NONE;
    }
    *flags = BCM_STK_ENABLE;

    if (SOC_IS_INACTIVE_STACK_PORT(unit, port)) {
        *flags |= BCM_STK_INACTIVE;
    }

    /* SL, HG, simplex and duplex are determined per-device */
    switch (BCM_CHIP_FAMILY(unit)) {
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_TUCANA:
    case BCM_FAMILY_LYNX:
        return _xgs_stk_flags_get(unit, port, flags);
        break;

    default:
        break;
    }

    return BCM_E_NONE;
}

#endif /* _BCM_STK_FLAGS_GET */

/****************************************************************
 *
 * Set SL enable, per device type
 *
 ****************************************************************/

STATIC int
_bcm_xgs_sl_mode_enable(int unit)
{
    if (SOC_IS_DRACO(unit)) {
#if defined(BCM_DRACO_SUPPORT)
        uint32 config_reg;
        uint64 iconfig;
        int port;

        /* Gig Ports */
        PBMP_GE_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config_reg));
            soc_reg_field_set(unit, CONFIGr, &config_reg, ST_MODULEf, 1);
            soc_reg_field_set(unit, CONFIGr, &config_reg, ST_MCNTf, 1);
            SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config_reg));
        }

        /* higig ports */
        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_MODULEf, 1);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_COUNTf, 1);
            SOC_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
        }
#endif
    } else if (SOC_IS_TUCANA(unit)) {
#if defined(BCM_TUCANA_SUPPORT)
        uint32 config_reg;
        uint64 iconfig;
        int port;

        /* FE & GE Ports */
        SOC_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &config_reg));
        soc_reg_field_set(unit, CHIP_CONFIGr, &config_reg, ST_MODULEf, 1);
        soc_reg_field_set(unit, CHIP_CONFIGr, &config_reg, SIMPLEX_MCNTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CHIP_CONFIGr(unit, config_reg));

        /* HIGIG ports */
        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_MODULEf, 1);
            soc_reg64_field32_set(unit, ICONFIGr, &iconfig, ST_COUNTf, 1);
            SOC_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
        }
#endif
    } else {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * Enable SL stacking for a chip capable of it.
 * Disabling is not currently supported.
 */

STATIC int
_esw_stk_sl_mode_enable_set(int unit, int enable)
{
    int rv = BCM_E_UNAVAIL;

    STK_VERB(("STK: SL mode set unit %d%s\n", unit,
              enable ? "" : " (disable)"));

    if (enable == FALSE) {
        return BCM_E_UNAVAIL;
    }

    switch (BCM_CHIP_FAMILY(unit)) {
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_TUCANA:
        rv = _bcm_xgs_sl_mode_enable(unit);
        break;
    default:
        break;
    }

    if (rv == BCM_E_NONE) {
        SOC_SL_MODE_SET(unit, enable);
    }

#ifdef BCM_RPC_SUPPORT
    _bcm_rx_sl_mode_update(unit);
#endif

    return rv;
}

#if defined(BCM_XGS12_FABRIC_SUPPORT)
/*
 * Set the unicast forwarding port bitmap
 */
STATIC int
_bcm5670_stk_ucbitmap_set(int unit, bcm_port_t port, int modid, pbmp_t pbmp)
{
    mem_uc_entry_t      uc;
    int                 blk;
    int                 rv;

    if (IS_CPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }
    blk = SOC_PORT_BLOCK(unit, port);
    sal_memset(&uc, 0, sizeof(uc));
    soc_MEM_UCm_field32_set(unit, &uc, UCBITMAPf,
                SOC_PBMP_WORD_GET(pbmp, 0));

    rv = WRITE_MEM_UCm(unit, blk, modid, &uc);

    return rv;
}

STATIC int
_bcm5675_stk_ucbitmap_set(int unit, bcm_port_t port, int modid, pbmp_t pbmp)
{
    mem_uc_entry_t      uc;
    int                 blk;
    int                 rv;

    if (IS_CPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }
    blk = SOC_PORT_BLOCK(unit, port);
    soc_mem_lock(unit, MEM_UCm);
    rv = READ_MEM_UCm(unit, blk, modid, &uc);
    if (rv >= 0) {
        soc_MEM_UCm_field32_set(unit, &uc, UCBITMAPf,
                    SOC_PBMP_WORD_GET(pbmp, 0));

        rv = WRITE_MEM_UCm(unit, blk, modid, &uc);
    }
    soc_mem_unlock(unit, MEM_UCm);
    return rv;
}

STATIC int
_bcm567x_stk_ucbitmap_get(int unit, bcm_port_t port, int modid, pbmp_t *pbmp)
{
    mem_uc_entry_t  uc;
    int             blk;

    if (port < 0) {
        port = SOC_PORT(unit, hg, 0);
    }
    blk = SOC_PORT_BLOCK(unit, port);
    SOC_IF_ERROR_RETURN(READ_MEM_UCm(unit, blk, modid, &uc));
    SOC_PBMP_CLEAR(*pbmp);
    SOC_PBMP_WORD_SET(*pbmp, 0,
                      soc_MEM_UCm_field32_get(unit, &uc, UCBITMAPf));

    return BCM_E_NONE;
}

STATIC int
_bcm567x_stk_ucbitmap_del(int unit, bcm_port_t port, int modid, bcm_pbmp_t pbmp)
{
    mem_uc_entry_t  uc;
    int             blk;
    int             rv;
    uint32          fval;

    /* Clear UCbitmap for port & MH.DST_MODID */
    blk = SOC_PORT_BLOCK(unit, port);
    soc_mem_lock(unit, MEM_UCm);
    rv = READ_MEM_UCm(unit, blk, modid, &uc);
    if (rv >= 0) {
        soc_MEM_UCm_field_get(unit, &uc, UCBITMAPf, &fval);
        fval &= ~SOC_PBMP_WORD_GET(pbmp, 0);
        soc_MEM_UCm_field_set(unit, &uc, UCBITMAPf, &fval);
        rv = WRITE_MEM_UCm(unit, blk, modid, &uc);
    }
    soc_mem_unlock(unit, MEM_UCm);
    return rv;
}

#endif  /* BCM_XGS12_FABRIC_SUPPORT */

#if defined(BCM_FIREBOLT_SUPPORT)
STATIC int
_bcm56504_stk_ucbitmap_set(int unit, int modid, pbmp_t pbmp)
{
    modport_map_entry_t uc;
    int rv;
    uint32 map;
    int offset;
    pbmp_t non_stk_port;

    /* Check that port pbmp has only stack ports */
    SOC_PBMP_ASSIGN(non_stk_port, pbmp);
    SOC_PBMP_REMOVE(non_stk_port, PBMP_ST_ALL(unit));
    if (SOC_PBMP_NOT_NULL(non_stk_port)) {
        return BCM_E_PORT;
    }

    offset = SOC_IS_RAPTOR(unit) ? 1 : SOC_HG_OFFSET(unit);
    map = SOC_PBMP_WORD_GET(pbmp, 0) >> offset;
    soc_mem_lock(unit, MODPORT_MAPm);
    rv = READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, modid, &uc);
    if (rv >= 0) {
        soc_MODPORT_MAPm_field32_set(unit, &uc, HIGIG_PORT_BITMAPf,
                                     map);
        rv = WRITE_MODPORT_MAPm(unit, MEM_BLOCK_ALL, modid, &uc);
    }
    soc_mem_unlock(unit, MODPORT_MAPm);
    return rv;
}

STATIC int
_bcm56504_stk_ucbitmap_get(int unit, int modid, pbmp_t *pbmp)
{
    modport_map_entry_t uc;
    uint32 map;
    int offset;

    offset = SOC_IS_RAPTOR(unit) ? 1 : SOC_HG_OFFSET(unit);
    SOC_IF_ERROR_RETURN
        (READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, modid, &uc));
    map = soc_MODPORT_MAPm_field32_get(unit, &uc, HIGIG_PORT_BITMAPf);
    SOC_PBMP_CLEAR(*pbmp);
    SOC_PBMP_WORD_SET(*pbmp, 0, (map << offset));

    return BCM_E_NONE;
}

STATIC int
_bcm56504_stk_ucbitmap_del(int unit, int modid, bcm_pbmp_t pbmp)
{
    modport_map_entry_t uc;
    int rv;
    uint32 fval;
    uint32 map;
    int offset;
    pbmp_t non_stk_port;

    /* Check that port pbmp has only stack ports */
    SOC_PBMP_ASSIGN(non_stk_port, pbmp);
    SOC_PBMP_REMOVE(non_stk_port, PBMP_ST_ALL(unit));
    if (SOC_PBMP_NOT_NULL(non_stk_port)) {
        return BCM_E_PORT;
    }

    offset = SOC_IS_RAPTOR(unit) ? 1 : SOC_HG_OFFSET(unit);

    /* Convert port to HG bitmap */
    map = SOC_PBMP_WORD_GET(pbmp, 0) >> offset;
    soc_mem_lock(unit, MODPORT_MAPm);
    rv = READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, modid, &uc);
    if (rv >= 0) {
        fval = soc_MODPORT_MAPm_field32_get(unit, &uc, HIGIG_PORT_BITMAPf);
        fval &= ~map;
        soc_MODPORT_MAPm_field32_set(unit, &uc, HIGIG_PORT_BITMAPf, fval);
        rv = WRITE_MODPORT_MAPm(unit, MEM_BLOCK_ALL, modid, &uc);
    }
    soc_mem_unlock(unit, MODPORT_MAPm);
    return rv;
}
#endif /* BCM_FIREBOLT_SUPPORT */

/*
 * Function:
 *      bcm_stk_ucbitmap_set
 * Purpose:
 *      Set the unicast forwarding port bitmap
 * Parameters:
 *      unit  - SOC unit#
 *      port - ingress port number
 *      modid - destination module ID index (from HiGig mod header)
 *      pbmp - forwarding port bitmap
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Fabric only
 */
int
bcm_esw_stk_ucbitmap_set(int unit, bcm_port_t port, int modid, pbmp_t pbmp)
{

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

#ifdef  BCM_XGS12_FABRIC_SUPPORT
    if (SOC_IS_HERCULES1(unit)) {
        return _bcm5670_stk_ucbitmap_set(unit, port, modid, pbmp);
    }
    if (SOC_IS_HERCULES15(unit)) {
        return _bcm5675_stk_ucbitmap_set(unit, port, modid, pbmp);
    }
#endif  /* BCM_XGS12_FABRIC_SUPPORT */

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        bcm_port_t dest_port, dest_port_array[96];
        int count;

        if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
            return BCM_E_PARAM;
        }

        count = 0;
        BCM_PBMP_ITER(pbmp, dest_port) {
            if (count >= 96) {
                return BCM_E_RESOURCE;
            }
            dest_port_array[count] = dest_port;
            count++;
        }
#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
            return bcm_td_stk_port_modport_op(unit,
                    _BCM_STK_PORT_MODPORT_OP_SET,
                    port, modid, dest_port_array, count);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
        {
            return _bcm_stk_port_modport_op(unit,
                    _BCM_STK_PORT_MODPORT_OP_SET,
                    port, modid, dest_port_array, count);
        }
    }
#endif  /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

#ifdef  BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        COMPILER_REFERENCE(port);
        return _bcm56504_stk_ucbitmap_set(unit, modid, pbmp);
    }
#endif  /* BCM_FIREBOLT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_stk_ucbitmap_get
 * Purpose:
 *      Get the unicast forwarding port bitmap
 * Parameters:
 *      unit  - SOC unit#
 *      port - ingress port number
 *      modid - destination module ID index (from HiGig mod header)
 *      pbmp - (out)forwarding port bitmap
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Fabric only
 */
int
bcm_esw_stk_ucbitmap_get(int unit, bcm_port_t port, int modid, pbmp_t *pbmp)
{
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

#ifdef  BCM_XGS12_FABRIC_SUPPORT
    if (SOC_IS_XGS12_FABRIC(unit)) {
        return _bcm567x_stk_ucbitmap_get(unit, port, modid, pbmp);
    }
#endif  /* BCM_XGS12_FABRIC_SUPPORT */

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        bcm_port_t dest_port_array[96];
        int count, index, rv;

        if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
            return BCM_E_PARAM;
        }

#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
            rv = bcm_td_stk_port_modport_get(unit, port, modid, 96,
                    dest_port_array, &count);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
        {
            rv =_bcm_stk_port_modport_get(unit, port, modid, 96,
                    dest_port_array, &count);
        }
        if (rv == BCM_E_NOT_FOUND) {
            BCM_PBMP_CLEAR(*pbmp);
            return BCM_E_NONE;
        } else if (BCM_FAILURE(rv)) {
            return rv;
        }
        BCM_PBMP_CLEAR(*pbmp);
        for (index = 0; index < count; index++) {
            BCM_PBMP_PORT_ADD(*pbmp, dest_port_array[index]);
        }
        return BCM_E_NONE;
    }
#endif  /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

#ifdef  BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        COMPILER_REFERENCE(port);
        return _bcm56504_stk_ucbitmap_get(unit, modid, pbmp);
    }
#endif  /* BCM_FIREBOLT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_stk_ucbitmap_del
 * Purpose:
 *      Remove a group of ports from a Unicast (UC) table entry for
 *      a given ingress port.
 * Parameters:
 *      unit  - SOC unit#
 *      port  - ingress port number
 *      modid - destination module ID index (in HiGig mod header)
 *      pbmp  - forwarding port bitmap
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      Fabric only
 */
int
bcm_esw_stk_ucbitmap_del(int unit, bcm_port_t port, int modid, bcm_pbmp_t pbmp)
{
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

#ifdef  BCM_XGS12_FABRIC_SUPPORT
    if (SOC_IS_XGS12_FABRIC(unit)) {
        return _bcm567x_stk_ucbitmap_del(unit, port, modid, pbmp);
    }
#endif  /* BCM_XGS12_FABRIC_SUPPORT */

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        bcm_port_t dest_port, dest_port_array[96];
        int count;

        if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
            return BCM_E_PARAM;
        }

        count = 0;
        BCM_PBMP_ITER(pbmp, dest_port) {
            if (count >= 96) {
                return BCM_E_RESOURCE;
            }
            dest_port_array[count] = dest_port;
            count++;
        }
#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
            return bcm_td_stk_port_modport_op(unit,
                    _BCM_STK_PORT_MODPORT_OP_DELETE,
                    port, modid, dest_port_array, count);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
        {
            return _bcm_stk_port_modport_op(unit,
                    _BCM_STK_PORT_MODPORT_OP_DELETE,
                    port, modid, dest_port_array, count);
        }
    }
#endif  /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

#ifdef  BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        COMPILER_REFERENCE(port);
        return _bcm56504_stk_ucbitmap_del(unit, modid, pbmp);
    }
#endif  /* BCM_FIREBOLT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_stk_fmod_lmod_mapping_set
 * Purpose:
 *      Set up mapping of (FMODID, FPORT) <--> (LMODID, LPORT) on
 *      a fabric port.  Assigns base FMODID and LMODID to devices
 *      in the legacy domain attached to a fabric port. When fmod is
 *      -1, l_modid state is populated with "lmod" value that is
 *      passed (if it is valid)
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - StrataSwitch port number
 *      fmod - Fabric Module Identifier
 *      lmod - Local Module Identifier
 *      enable - modmap enable = 1, disable = 0
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX - Failure.
 * Notes:
 *      Fabric only (BCM5675).
 *      The fmod assignment will be
 *      Switch-1   LMODID = lmod     FMODID = fmod
 *      Switch-2   LMODID = lmod + 1 FMODID = fmod + 1
 *      Switch-3   LMODID = lmod + 2 FMODID = fmod + 2
 *      Switch-4   LMODID = lmod + 3 FMODID = fmod + 3
 *      and so on.
 */

int
bcm_esw_stk_fmod_lmod_mapping_set(int unit, bcm_port_t port,
                    bcm_module_t fmod, bcm_module_t lmod)
{
#if defined(LOCAL_MODMAP_SUPPORT)
    uint32  reg32, oreg32;
    int     rv;

    BCM_MODMAP_FEATURE_CHECK(unit, bcm_stk_fmod_lmod_mapping_set);
    LMOD_RANGE_CHECK(lmod);
    MOD_MAP_DATA_CHECK(unit);
    if (SOC_WARM_BOOT(unit) || fmod == -1) {
        ESW_STK_LOCK;
        mod_map_data[unit]->l_modid[port] = lmod;
        ESW_STK_UNLOCK;
        return BCM_E_NONE;
    }

    FMOD_RANGE_CHECK(fmod);
    FMOD_LMOD_CHECK(fmod, lmod);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    ESW_STK_LOCK;

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        if ((rv = READ_ING_MODMAP_CTRLr(unit, port, &oreg32)) >= 0) {
            reg32 = oreg32;
            soc_reg_field_set(unit, ING_MODMAP_CTRLr, &reg32,
                              MODULEID_OFFSETf, (fmod - lmod));
            if (reg32 != oreg32) {
                rv = WRITE_ING_MODMAP_CTRLr(unit, port, reg32);
                if (rv >= 0) {
                    /* We must keep the ingress and egress copies in sync */
                    rv = WRITE_EGR_MODMAP_CTRLr(unit, port, reg32);
                }
            }
            mod_map_data[unit]->l_modid[port] = lmod;
        }

        

    } else
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRIUMPH_SUPPORT */
    {
        if ((rv = READ_MODMAP_CTRLr(unit, port, &oreg32)) >= 0) {
            reg32 = oreg32;
            soc_reg_field_set(unit, MODMAP_CTRLr, &reg32,
                              MODULE_ID_OFFSETf, (fmod - lmod));
            if (reg32 != oreg32) {
                rv = WRITE_MODMAP_CTRLr(unit, port, reg32);
            }
            mod_map_data[unit]->l_modid[port] = lmod;
        }
    }
    ESW_STK_UNLOCK;

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return(rv);
#else
    return BCM_E_UNAVAIL;
#endif  /* MODMAP_SUPPORT */
}

/*
 * Function:
 *      bcm_stk_fmod_lmod_mapping_get
 * Purpose:
 *      Get the (FMODID, LMODID) associated with a fabric port.
 *      Gets the base FMODID and LMODID assigned to the devices
 *      in the legacy domain attached to this fabric port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - StrataSwitch port number
 *      fmod - (OUT) Fabric Module Identifier
 *      lmod - (OUT)Local Module Identifier
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX - Failure.
 * Notes:
 *      Fabric only (BCM5675)
 */

int
bcm_esw_stk_fmod_lmod_mapping_get(int unit, bcm_port_t port,
                    bcm_module_t *fmod, bcm_module_t *lmod)
{
#if defined(LOCAL_MODMAP_SUPPORT)
    uint32  oreg32;

    BCM_MODMAP_FEATURE_CHECK(unit, bcm_stk_fmod_lmod_mapping_get);
    MOD_MAP_DATA_CHECK(unit);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        SOC_IF_ERROR_RETURN(READ_ING_MODMAP_CTRLr(unit, port, &oreg32));
        *fmod = soc_reg_field_get(unit, ING_MODMAP_CTRLr, oreg32,
                                  MODULEID_OFFSETf);
    } else
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRIUMPH_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(READ_MODMAP_CTRLr(unit, port, &oreg32));
        *fmod = soc_reg_field_get(unit, MODMAP_CTRLr, oreg32,
                                  MODULE_ID_OFFSETf);
    }

    *lmod = mod_map_data[unit]->l_modid[port];
    *fmod += *lmod;

    return(BCM_E_NONE);
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_MODMAP_SUPPORT */
}

#ifdef BCM_BRADLEY_SUPPORT
int
_bcm_stk_fmod_smod_mapping_group_set(int unit, int group,
                      bcm_module_t fmod, bcm_module_t smod,
                      bcm_port_t sport, uint32 nports)
{
    int rv=BCM_E_NONE;
    ing_mod_map_table_entry_t  immap;
    egr_mod_map_table_entry_t  emmap;
    int         i, found, group_offset_ing, group_offset_egr;
    uint32      fval;
    uint32      thresh;
    soc_field_t tf[] = {THRESH_Af, THRESH_Bf, THRESH_Cf, INVALIDf};
    soc_field_t pf[] = {PORTOFF_Af, PORTOFF_Bf, PORTOFF_Cf, PORTOFF_Df};
    soc_field_t mf[] = {MOD_Af, MOD_Bf, MOD_Cf, MOD_Df, INVALIDf};

    /* Note: Parameter checks were performed before calling this function */
    if (group == 1) {
        group_offset_ing = soc_mem_index_count(unit, ING_MOD_MAP_TABLEm) / 2;
        group_offset_egr = soc_mem_index_count(unit, EGR_MOD_MAP_TABLEm) / 2;
    } else { /* group == 0 */
        group_offset_ing = 0;
        group_offset_egr = 0;
    }

    if (nports != 0) {
        SPORT_RANGE_CHECK((sport + nports - 1));

        sal_memset(&emmap, 0, sizeof(emmap));
        soc_EGR_MOD_MAP_TABLEm_field_set(unit, &emmap,
				      PORT_OFFSETf, (uint32 *)&sport);
        soc_EGR_MOD_MAP_TABLEm_field_set(unit, &emmap,
				      MODIDf, (uint32 *)&smod);
        thresh = sport + nports - 1;

        ESW_STK_LOCK;

        if ((rv = READ_ING_MOD_MAP_TABLEm(unit, SOC_BLOCK_ANY,
                               smod + group_offset_ing, &immap)) >= 0) {
            found = 0;
            for(i = 0; mf[i] != INVALIDf; i++) {
                soc_ING_MOD_MAP_TABLEm_field_get(unit, &immap, mf[i], &fval);
                if (fval == 0) {
                    if (fmod==0) {
                        mod_map_data[unit]->fmod0[group] = smod+1;
                        break;  /* Found a slot for fmod 0 */
                    }
                    if ((mod_map_data[unit]->fmod0[group]) &&
                        ((mod_map_data[unit]->fmod0[group] == (smod+1))) &&
                        !(found)) {
                        found=1; /* This is fmod 0 */
                    } else {
                        break;  /* Found a slot */
                    }
                }
            }

            if (mf[i] != INVALIDf) {
                for(; i > 0; i--) {
                    soc_ING_MOD_MAP_TABLEm_field_get(
                                    unit, &immap, tf[i - 1], &fval);
                    /* THRESH_A < THRESH_B < THRESH_C */
                    if (fval > thresh) {
                        if (tf[i] != INVALIDf) {
                            soc_ING_MOD_MAP_TABLEm_field_set(
                                            unit, &immap, tf[i], &fval);
                        } else {
                            mod_map_data[unit]->thresh_d[smod] = fval;
                        }
                        soc_ING_MOD_MAP_TABLEm_field_get(
                                        unit, &immap, pf[i - 1], &fval);
                        soc_ING_MOD_MAP_TABLEm_field_set(
                                        unit, &immap, pf[i], &fval);
                        soc_ING_MOD_MAP_TABLEm_field_get(
                                        unit, &immap, mf[i - 1], &fval);
                        soc_ING_MOD_MAP_TABLEm_field_set(
                                        unit, &immap, mf[i], &fval);
                    } else {
                        break;
                    }
                }

                if (tf[i] != INVALIDf) {
                    soc_ING_MOD_MAP_TABLEm_field_set(
                                    unit, &immap, tf[i], &thresh);
                } else {
                    mod_map_data[unit]->thresh_d[smod] = thresh;
                }
                soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap,
                                                 pf[i], (uint32 *)&sport);
                soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap,
                                                 mf[i], (uint32 *)&fmod);
                rv = WRITE_EGR_MOD_MAP_TABLEm(unit, SOC_BLOCK_ALL,
                                              fmod + group_offset_egr, &emmap);
                if (rv >= 0) {
                    rv = WRITE_ING_MOD_MAP_TABLEm(unit, SOC_BLOCK_ALL,
                                       smod + group_offset_ing, &immap);
                }
            } else {
                rv = BCM_E_PARAM;
            }
        }
        ESW_STK_UNLOCK;
    } else {  /* This is actually a delete */
        ESW_STK_LOCK;
        if ((rv = READ_EGR_MOD_MAP_TABLEm(unit, SOC_BLOCK_ANY,
                                   fmod + group_offset_egr, &emmap)) >= 0) {
            bcm_port_t o_sport = -1;
            bcm_module_t o_smod = -1;
            int j;

            soc_EGR_MOD_MAP_TABLEm_field_get(unit, &emmap,
                                             PORT_OFFSETf, (uint32 *)&o_sport);
            soc_EGR_MOD_MAP_TABLEm_field_get(unit, &emmap,
                                             MODIDf, (uint32 *)&o_smod);
            sal_memset(&emmap, 0, sizeof(emmap));
            if ((o_sport == sport) && (o_smod == smod)) {
                if ((rv = READ_ING_MOD_MAP_TABLEm(unit, SOC_BLOCK_ANY,
                                   smod + group_offset_ing, &immap)) >= 0) {
                    for(i = 0; mf[i] != INVALIDf; i++) {
                        soc_ING_MOD_MAP_TABLEm_field_get(unit, &immap, mf[i], &fval);
                        soc_ING_MOD_MAP_TABLEm_field_get(unit, &immap, pf[i], &thresh);
                        if ((fval == (uint32)fmod) && (thresh == (uint32)sport)) {
                            if (tf[i] == INVALIDf) {
                                mod_map_data[unit]->thresh_d[smod] = 0;
                                fval = 0;
                                soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap, mf[i], &fval);
                                soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap, pf[i], &fval);
                            } else {
                                for (j = i; mf[j+1] != INVALIDf; j++) {
                                    soc_ING_MOD_MAP_TABLEm_field_get(unit, &immap, pf[j+1], &fval);
                                    soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap, pf[j], &fval);
                                    soc_ING_MOD_MAP_TABLEm_field_get(unit, &immap,mf[j+1], &fval);
                                    soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap, mf[j], &fval);
                                    if (tf[j+1] != INVALIDf) {
                                        soc_ING_MOD_MAP_TABLEm_field_get(unit, &immap, tf[j+1], &fval);
                                        soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap, tf[j], &fval);
                                    }
                                }
                                fval = 0;
                                soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap, pf[j], &fval);
                                soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap, mf[j], &fval);
                                soc_ING_MOD_MAP_TABLEm_field_set(unit, &immap, tf[j-1], &fval);
                            }
                            break;
                        }
                    }
                    if (mf[i] == INVALIDf) {
                        rv = BCM_E_INTERNAL;
                    } else {
                        rv = WRITE_EGR_MOD_MAP_TABLEm(unit, SOC_BLOCK_ALL,
                                           fmod + group_offset_egr, &emmap);
                        if (rv >= 0) {
                            rv = WRITE_ING_MOD_MAP_TABLEm(unit, SOC_BLOCK_ALL,
                                           smod + group_offset_ing, &immap);
                        }

                        if (fmod==0) {
                            mod_map_data[unit]->fmod0[group] = 0;
                        }
                    }
                }
            } else {
                rv = BCM_E_PARAM;
            }
        }
        ESW_STK_UNLOCK;
    }

    return(rv);
}
#endif /* BCM_BRADLEY_SUPPORT */

/*
 * Function:
 *      bcm_stk_fmod_smod_mapping_set
 * Purpose:
 *      Set up mapping of (FMODID, FPORT) <--> (SMODID, SPORT) on fabric
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - StrataSwitch port number (-1 = All ports)
 *      fmod - Fabric Module Identifier
 *      smod - Switch Module Identifier
 *      sport - Switch Module Base Port
 *      nports - Number of ports on switch device with fmod.
 *               (0 to remove previously added entry).
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX - Failure.
 * Notes:
 *      Fabric only (BCM5675)
 */

int
bcm_esw_stk_fmod_smod_mapping_set(int unit, bcm_port_t port,
                      bcm_module_t fmod, bcm_module_t smod,
                      bcm_port_t sport, uint32 nports)
{
#if defined(LOCAL_MODMAP_SUPPORT)
    int rv=BCM_E_NONE;
    mem_ing_modmap_entry_t  immap;
    mem_egr_modmap_entry_t  emmap;
    int         bk=-1, blk;
    int         i, found;
    uint32      fval;
    uint32      thresh;
    soc_field_t tf[] = {THRESH_Af, THRESH_Bf, THRESH_Cf, INVALIDf};
    soc_field_t pf[] = {PORTOFF_Af, PORTOFF_Bf, PORTOFF_Cf, PORTOFF_Df};
    soc_field_t mf[] = {MOD_Af, MOD_Bf, MOD_Cf, MOD_Df, INVALIDf};

    BCM_MODMAP_FEATURE_CHECK(unit, bcm_stk_fmod_smod_mapping_set);

    STK_VERB(("STK unit %d port %d: Mapping fmod %d to smod %d sport %d "
              "for %d ports\n", unit, port, fmod, smod, sport, nports));

    FMOD_RANGE_CHECK(fmod);
    SMOD_RANGE_CHECK(smod);
    MOD_MAP_DATA_CHECK(unit);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    if (port >= 0) {
        if (!IS_HG_PORT(unit, port)) {
            return BCM_E_PARAM;
        }
        bk = SOC_PORT_BLOCK(unit, port);
    }

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        int group;

        /* After range checks, proceed */
        if (port >= 0) {
            /* Determine group offsets for the tables */
            BCM_IF_ERROR_RETURN
                (bcm_esw_stk_port_modmap_group_get(unit, port, &group));
            if ((group < 0) || (group > _XGS3_MODMAP_GROUPS_MAX)) {
                return BCM_E_PARAM;
            }
            return _bcm_stk_fmod_smod_mapping_group_set(unit, group, fmod,
                                                        smod, sport, nports);
        } else {
            for (group = 0; group <= _XGS3_MODMAP_GROUPS_MAX; group++) {
                rv = _bcm_stk_fmod_smod_mapping_group_set(unit, group, fmod,
                                                          smod, sport, nports);
                if (rv < 0) {
                    break;
                }
            }
            return rv;
        }
    }
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRIUMPH_SUPPORT */

    if (nports != 0) {
        SPORT_RANGE_CHECK((sport + nports - 1));

        sal_memset(&emmap, 0, sizeof(emmap));
        soc_MEM_EGR_MODMAPm_field_set(unit, &emmap,
				      PORT_OFFf, (uint32 *)&sport);
        soc_MEM_EGR_MODMAPm_field_set(unit, &emmap,
				      MODf, (uint32 *)&smod);
        thresh = sport + nports - 1;

        ESW_STK_LOCK;
        SOC_MEM_BLOCK_ITER(unit, MEM_ING_MODMAPm, blk) {
            if ((port >= 0) && (bk != blk)) {
                continue;
            }

            if ((rv = READ_MEM_ING_MODMAPm(unit, blk, smod, &immap)) >= 0) {

                found = 0;
                for(i = 0; mf[i] != INVALIDf; i++) {
                    int p;
                    soc_MEM_ING_MODMAPm_field_get(unit, &immap, mf[i], &fval);
                    p = SOC_BLOCK_PORT(unit, blk);
                    if (fval == 0) {
                        if (fmod==0) {
                            mod_map_data[unit]->fmod0[p] = smod+1;
                            break;  /* Found a slot for fmod 0 */
                        }
                        if ((mod_map_data[unit]->fmod0[p]) &&
                                ((mod_map_data[unit]->fmod0[p] == (smod+1))) &&
                                !(found)) {
                            found=1; /* This is fmod 0 */
                        } else {
                            break;  /* Found a slot */
                        }
                    }
                }

                if (mf[i] != INVALIDf) {
                    for(; i > 0; i--) {
                        soc_MEM_ING_MODMAPm_field_get(
                                    unit, &immap, tf[i - 1], &fval);
                        /* THRESH_A < THRESH_B < THRESH_C */
                        if (fval > thresh) {
                            if (tf[i] != INVALIDf) {
                                soc_MEM_ING_MODMAPm_field_set(
                                        unit, &immap, tf[i], &fval);
                            } else {
                                mod_map_data[unit]->thresh_d[smod] = fval;
                            }
                            soc_MEM_ING_MODMAPm_field_get(
                                    unit, &immap, pf[i - 1], &fval);
                            soc_MEM_ING_MODMAPm_field_set(
                                    unit, &immap, pf[i], &fval);
                            soc_MEM_ING_MODMAPm_field_get(
                                    unit, &immap, mf[i - 1], &fval);
                            soc_MEM_ING_MODMAPm_field_set(
                                    unit, &immap, mf[i], &fval);
                        } else {
                            break;
                        }
                    }

                    if (tf[i] != INVALIDf) {
                        soc_MEM_ING_MODMAPm_field_set(unit, &immap, tf[i], &thresh);
                    } else {
                        mod_map_data[unit]->thresh_d[smod] = thresh;
                    }
                    soc_MEM_ING_MODMAPm_field_set(unit, &immap,
						  pf[i], (uint32 *)&sport);
                    soc_MEM_ING_MODMAPm_field_set(unit, &immap,
						  mf[i], (uint32 *)&fmod);
                    if ((rv = WRITE_MEM_EGR_MODMAPm(unit, blk, fmod, &emmap)) < 0) {
                        break;
                    }
                    if ((rv = WRITE_MEM_ING_MODMAPm(unit, blk, smod, &immap)) < 0) {
                        break;
                    }
                } else {
                    rv = BCM_E_PARAM;
                }
            }
        }
        ESW_STK_UNLOCK;
    } else {  /* This is actually a delete */
        ESW_STK_LOCK;
        SOC_MEM_BLOCK_ITER(unit, MEM_ING_MODMAPm, blk) {
            if ((port >= 0) && (bk != blk)) {
                continue;
            }

            if ((rv = READ_MEM_EGR_MODMAPm(unit, blk, fmod, &emmap)) >= 0) {
                bcm_port_t o_sport = -1;
                bcm_module_t o_smod = -1;
                int j, p;

                soc_MEM_EGR_MODMAPm_field_get(unit, &emmap,
					      PORT_OFFf, (uint32 *)&o_sport);
                soc_MEM_EGR_MODMAPm_field_get(unit, &emmap,
					      MODf, (uint32 *)&o_smod);
                sal_memset(&emmap, 0, sizeof(emmap));
                if ((o_sport == sport) && (o_smod == smod)) {
                    if ((rv = READ_MEM_ING_MODMAPm(unit, blk, smod, &immap)) >= 0) {
                        for(i = 0; mf[i] != INVALIDf; i++) {
                            soc_MEM_ING_MODMAPm_field_get(unit, &immap, mf[i], &fval);
                            soc_MEM_ING_MODMAPm_field_get(unit, &immap, pf[i], &thresh);
                            if ((fval == (uint32)fmod) && (thresh == (uint32)sport)) {
                                if (tf[i] == INVALIDf) {
                                  mod_map_data[unit]->thresh_d[smod] = 0;
                                  fval = 0;
                                  soc_MEM_ING_MODMAPm_field_set(unit, &immap, mf[i], &fval);
                                  soc_MEM_ING_MODMAPm_field_set(unit, &immap, pf[i], &fval);
                                } else {
                                  for (j = i; mf[j+1] != INVALIDf; j++) {
                                       soc_MEM_ING_MODMAPm_field_get(unit, &immap, pf[j+1], &fval);
                                       soc_MEM_ING_MODMAPm_field_set(unit, &immap, pf[j], &fval);
                                       soc_MEM_ING_MODMAPm_field_get(unit, &immap,mf[j+1], &fval);
                                       soc_MEM_ING_MODMAPm_field_set(unit, &immap, mf[j], &fval);
                                       if (tf[j+1] != INVALIDf) {
                                           soc_MEM_ING_MODMAPm_field_get(unit, &immap, tf[j+1], &fval);
                                           soc_MEM_ING_MODMAPm_field_set(unit, &immap, tf[j], &fval);
                                       }
                                  }
                                  fval = 0;
                                  soc_MEM_ING_MODMAPm_field_set(unit, &immap, pf[j], &fval);
                                  soc_MEM_ING_MODMAPm_field_set(unit, &immap, mf[j], &fval);
                                  soc_MEM_ING_MODMAPm_field_set(unit, &immap, tf[j-1], &fval);
                                }
                                break;
                            }
                        }
                        if (mf[i] == INVALIDf) {
                            rv = BCM_E_INTERNAL;
                            break;
                        }

                        if ((rv = WRITE_MEM_EGR_MODMAPm(unit, blk, fmod, &emmap)) < 0) {
                             break;
                        }
                        if ((rv = WRITE_MEM_ING_MODMAPm(unit, blk, smod, &immap)) < 0) {
                             break;
                        }

                        if (fmod==0) {
                            p = SOC_BLOCK_PORT(unit, blk);
                            mod_map_data[unit]->fmod0[p] = 0;
                        }
                    }
                } else {
                      rv = BCM_E_PARAM;
                }
            }
        }
        ESW_STK_UNLOCK;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return(rv);
#else
    return BCM_E_UNAVAIL;
#endif  /* MODMAP_SUPPORT */
}

#ifdef BCM_BRADLEY_SUPPORT
int
_bcm_stk_fmod_smod_mapping_group_get(int unit, bcm_port_t port,
                      bcm_module_t fmod, bcm_module_t *smod,
                      bcm_port_t *sport, uint32 *nports)
{
    int rv;
    ing_mod_map_table_entry_t  immap;
    egr_mod_map_table_entry_t  emmap;
    uint32      fval;
    uint32      sp_val;
    int         i, group, group_offset_ing, group_offset_egr;
    soc_field_t tf[] = {THRESH_Af, THRESH_Bf, THRESH_Cf, INVALIDf};
    soc_field_t pf[] = {PORTOFF_Af, PORTOFF_Bf, PORTOFF_Cf, PORTOFF_Df};
    soc_field_t mf[] = {MOD_Af, MOD_Bf, MOD_Cf, MOD_Df, INVALIDf};

    /* Determine group offsets for the tables */
    BCM_IF_ERROR_RETURN
        (bcm_esw_stk_port_modmap_group_get(unit, port, &group));

    if (group != 0) {
        group_offset_ing = soc_mem_index_count(unit, ING_MOD_MAP_TABLEm) / 2;
        group_offset_egr = soc_mem_index_count(unit, EGR_MOD_MAP_TABLEm) / 2;
    } else {
        group_offset_ing = 0;
        group_offset_egr = 0;
    }

    ESW_STK_LOCK;
    if ((rv = READ_EGR_MOD_MAP_TABLEm(unit, SOC_BLOCK_ANY,
                                      fmod + group_offset_egr, &emmap)) >= 0) {
        soc_EGR_MOD_MAP_TABLEm_field_get(unit, &emmap,
                                         PORT_OFFSETf, (uint32 *)sport);
        soc_EGR_MOD_MAP_TABLEm_field_get(unit, &emmap,
                                         MODIDf, (uint32 *)smod);
        if ((rv = READ_ING_MOD_MAP_TABLEm(unit, SOC_BLOCK_ANY,
                               *smod + group_offset_ing, &immap)) >= 0) {
            for(i = 0; mf[i] != INVALIDf; i++) {
                soc_ING_MOD_MAP_TABLEm_field_get(unit, &immap, mf[i], &fval);
                soc_ING_MOD_MAP_TABLEm_field_get(unit, &immap, pf[i], &sp_val);
                if ((fval == (uint32)fmod) && ((uint32)*sport == sp_val)) {
                    if (tf[i] != INVALIDf) {
                        soc_ING_MOD_MAP_TABLEm_field_get(
                                    unit, &immap, tf[i], &fval);
                    } else {
                        fval = mod_map_data[unit]->thresh_d[*smod];
                    }
                    *nports = fval - sp_val + 1;
                    break; /* Found */
                }
            }
            if (mf[i] == INVALIDf ||
               ((fmod==0) && (mod_map_data[unit]->fmod0[group] != (*smod+1)))) {
                rv = BCM_E_NOT_FOUND;
            }
        }
    }
    ESW_STK_UNLOCK;

    return(rv);
}
#endif /* BCM_BRADLEY_SUPPORT */

/*
 * Function:
 *      bcm_stk_fmod_smod_mapping_get
 * Purpose:
 *      Get the (SMODID, BASE_SPORT) and num ports associated with
 *      switch device with fmod
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - StrataSwitch port number
 *      fmod - Fabric Module Identifier
 *      smod - (OUT) Switch Module Identifier
 *      sport - (OUT) Switch Module Base Port
 *      nports - (OUT) Number of ports on switch device with fmod.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX - Failure.
 * Notes:
 *      Fabric only (BCM5675)
 */

int
bcm_esw_stk_fmod_smod_mapping_get(int unit, bcm_port_t port,
                      bcm_module_t fmod, bcm_module_t *smod,
                      bcm_port_t *sport, uint32 *nports)
{
#if defined(LOCAL_MODMAP_SUPPORT)
    int rv;
    mem_ing_modmap_entry_t  immap;
    mem_egr_modmap_entry_t  emmap;
    uint32      fval;
    uint32      sp_val;
    int         blk;
    int         p;
    int         i;
    soc_field_t tf[] = {THRESH_Af, THRESH_Bf, THRESH_Cf, INVALIDf};
    soc_field_t pf[] = {PORTOFF_Af, PORTOFF_Bf, PORTOFF_Cf, PORTOFF_Df};
    soc_field_t mf[] = {MOD_Af, MOD_Bf, MOD_Cf, MOD_Df, INVALIDf};

    BCM_MODMAP_FEATURE_CHECK(unit, bcm_stk_fmod_smod_mapping_get);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    if (!IS_HG_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        /* After range checks, proceed */
        return _bcm_stk_fmod_smod_mapping_group_get(unit, port, fmod, smod,
                                                    sport, nports);
    }
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRIUMPH_SUPPORT */

    blk = SOC_PORT_BLOCK(unit, port);

    ESW_STK_LOCK;
    if ((rv = READ_MEM_EGR_MODMAPm(unit, blk, fmod, &emmap)) >= 0) {
        soc_MEM_EGR_MODMAPm_field_get(unit, &emmap,
				      PORT_OFFf, (uint32 *)sport);
        soc_MEM_EGR_MODMAPm_field_get(unit, &emmap,
				      MODf, (uint32 *)smod);
        if ((rv = READ_MEM_ING_MODMAPm(unit, blk, *smod, &immap)) >= 0) {
            for(i = 0; mf[i] != INVALIDf; i++) {
                soc_MEM_ING_MODMAPm_field_get(unit, &immap, mf[i], &fval);
                soc_MEM_ING_MODMAPm_field_get(unit, &immap, pf[i], &sp_val);
                if ((fval == (uint32)fmod) && ((uint32)*sport == sp_val)) {
                    if (tf[i] != INVALIDf) {
                        soc_MEM_ING_MODMAPm_field_get(
                                    unit, &immap, tf[i], &fval);
                    } else {
                        fval = mod_map_data[unit]->thresh_d[*smod];
                    }
                    *nports = fval - sp_val + 1;
                    break; /* Found */
                }
            }
            p = SOC_BLOCK_PORT(unit, blk);
            if (mf[i] == INVALIDf ||
               ((fmod==0) && (mod_map_data[unit]->fmod0[p] != (*smod+1)))) {
                rv = BCM_E_NOT_FOUND;
            }
        }
    }
    ESW_STK_UNLOCK;

    return(rv);
#else
    return BCM_E_UNAVAIL;
#endif  /* MODMAP_SUPPORT */
}

/*
* Function:
*      bcm_stk_port_modmap_group_set
* Purpose:
*      Assign module mapping group for port to use
* Parameters:
*      unit - StrataSwitch PCI device unit number (driver internal).
*      port - StrataSwitch port number (-1 = All ports)
*      group - Module mapping group
* Returns:
*      BCM_E_NONE - Success.
*      BCM_E_XXX - Failure.
* Notes:
*      Fabric only
*/

int
bcm_esw_stk_port_modmap_group_set(int unit, bcm_port_t port, int group)
{
#if defined(LOCAL_MODMAP_SUPPORT)
    BCM_MODMAP_FEATURE_CHECK(unit, bcm_stk_port_modmap_group_set);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    if (!IS_HG_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        uint32  reg32, oreg32;
        int     rv;

        if ((group < 0) || (group > _XGS3_MODMAP_GROUPS_MAX)) {
            return BCM_E_PARAM;
        }

        ESW_STK_LOCK;

        if ((rv = READ_ING_MODMAP_CTRLr(unit, port, &oreg32)) >= 0) {
            reg32 = oreg32;
            soc_reg_field_set(unit, ING_MODMAP_CTRLr, &reg32,
                              ING_MOD_MAP_IDf, group ? 1 : 0);
            if (reg32 != oreg32) {
                rv = WRITE_ING_MODMAP_CTRLr(unit, port, reg32);
                if (rv >= 0) {
                    /* We must keep the ingress and egress copies in sync */
                    rv = WRITE_EGR_MODMAP_CTRLr(unit, port, reg32);
                }
            }
        }
        ESW_STK_UNLOCK;
    } else
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRIUMPH_SUPPORT */
    {
        /* Only identity mapping allowed */
        if (group != (port - 1)) {
            return BCM_E_PARAM;
        }
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif  /* MODMAP_SUPPORT */
}

/*
* Function:
*      bcm_stk_port_modmap_group_get
* Purpose:
*      Retrieve module mapping group selected for port
* Parameters:
*      unit - StrataSwitch PCI device unit number (driver internal).
*      port - StrataSwitch port number (-1 = All ports)
*      group - (OUT) Module mapping group
* Returns:
*      BCM_E_NONE - Success.
*      BCM_E_XXX - Failure.
* Notes:
*      Fabric only
*/

int
bcm_esw_stk_port_modmap_group_get(int unit, bcm_port_t port, int *group)
{
#if defined(LOCAL_MODMAP_SUPPORT)
    BCM_MODMAP_FEATURE_CHECK(unit, bcm_esw_stk_port_modmap_group_get);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    if (!IS_HG_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        uint32  oreg32;
        SOC_IF_ERROR_RETURN(READ_ING_MODMAP_CTRLr(unit, port, &oreg32));
        *group = soc_reg_field_get(unit, ING_MODMAP_CTRLr, oreg32,
                                   ING_MOD_MAP_IDf);
    } else
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRIUMPH_SUPPORT */
    {
        *group = port - 1;
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif  /* MODMAP_SUPPORT */
}

/*
* Function:
*      bcm_stk_port_modmap_group_max_get
* Purpose:
*      Retrieve maximum module mapping group allowed on device
* Parameters:
*      unit - StrataSwitch PCI device unit number (driver internal).
*      max_group - (OUT) Module mapping group
* Returns:
*      BCM_E_NONE - Success.
*      BCM_E_XXX - Failure.
* Notes:
*      Fabric only
*/

int
bcm_esw_stk_port_modmap_group_max_get(int unit, int *max_group)
{
#if defined(LOCAL_MODMAP_SUPPORT)
    BCM_MODMAP_FEATURE_CHECK(unit, bcm_stk_port_modmap_group_max_get);

    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        *max_group = _XGS3_MODMAP_GROUPS_MAX;
    } else {
        *max_group = SOC_MAX_NUM_PORTS - 1;
    }

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif  /* MODMAP_SUPPORT */
}

/*
 * Function:
 *      bcm_stk_modmap_enable_set
 * Purpose:
 *      Enable/Disable Modid mapping on a given fabric port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - StrataSwitch port number
 *      enable -  Modmap enabled flag. 1 = enabled 0 = disabled
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX - Failure.
 * Notes:
 *      Fabric only (BCM5675)
 */

int
bcm_esw_stk_modmap_enable_set(int unit,
              bcm_port_t port, int enable)
{
#if defined(LOCAL_MODMAP_SUPPORT)
    uint32  reg32, oreg32;
    int     rv;

    BCM_MODMAP_FEATURE_CHECK(unit, bcm_stk_modmap_enable_set);

    STK_VERB(("STK %d: %sabling mod mapping port %d\n",
              unit, enable ? "En" : "Dis", port));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }


    if (!IS_ST_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    ESW_STK_LOCK;
#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        if ((rv = READ_ING_MODMAP_CTRLr(unit, port, &oreg32)) >= 0) {
            reg32 = oreg32;
            soc_reg_field_set(unit, ING_MODMAP_CTRLr, &reg32,
                              ING_MAP_ENf, enable ? 1 : 0);
            if (reg32 != oreg32) {
                rv = WRITE_ING_MODMAP_CTRLr(unit, port, reg32);
                if (rv >= 0) {
                    /* We must keep the ingress and egress copies in sync */
                    rv = WRITE_EGR_MODMAP_CTRLr(unit, port, reg32);
                }
            }
        }
    } else
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRIUMPH_SUPPORT */
    {
        if ((rv = READ_MODMAP_CTRLr(unit, port, &oreg32)) >=0 ) {
            reg32 = oreg32;
            soc_reg_field_set(unit, MODMAP_CTRLr, &reg32,
                              EGR_MAP_ENf, (enable)?1:0);
            soc_reg_field_set(unit, MODMAP_CTRLr, &reg32,
                              ING_MAP_ENf, (enable)?1:0);
            if (reg32 != oreg32) {
                rv = WRITE_MODMAP_CTRLr(unit, port, reg32);
            }
        }
    }
    ESW_STK_UNLOCK;

    return(BCM_E_NONE);
#else
    return BCM_E_UNAVAIL;
#endif  /* MODMAP_SUPPORT */
}

/*
 * Function:
 *      bcm_stk_modmap_enable_get
 * Purpose:
 *      Get Modid mapping Enable/Disable state on a given fabric port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - (OUT) StrataSwitch port number
 *      enable - (OUT) Modmap enabled flag. 1 = enabled 0 = disabled
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX - Failure.
 * Notes:
 *      Fabric only (BCM5675)
 */

int
bcm_esw_stk_modmap_enable_get(int unit,
              bcm_port_t port, int *enable)
{
#if defined(LOCAL_MODMAP_SUPPORT)
    uint32  oreg32;
    uint32  i_enable;
    uint32  e_enable;

    BCM_MODMAP_FEATURE_CHECK(unit, bcm_stk_modmap_enable_get);
    if (!IS_ST_PORT(unit, port)) {
        return BCM_E_PORT;
    }

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_HBX(unit) || SOC_IS_TR_VL(unit)) {
        uint32  oreg32;

        SOC_IF_ERROR_RETURN(READ_ING_MODMAP_CTRLr(unit, port, &oreg32));
        i_enable = soc_reg_field_get(unit, ING_MODMAP_CTRLr, oreg32,
                                     ING_MAP_ENf);
        SOC_IF_ERROR_RETURN(READ_EGR_MODMAP_CTRLr(unit, port, &oreg32));
        e_enable = soc_reg_field_get(unit, EGR_MODMAP_CTRLr, oreg32,
                                     EGR_MAP_ENf);
    } else
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRIUMPH_SUPPORT */
    {
        SOC_IF_ERROR_RETURN(READ_MODMAP_CTRLr(unit, port, &oreg32));
        i_enable = soc_reg_field_get(unit, MODMAP_CTRLr, oreg32, ING_MAP_ENf);
        e_enable = soc_reg_field_get(unit, MODMAP_CTRLr, oreg32, EGR_MAP_ENf);
    }
    *enable = (i_enable && e_enable);
    if ((i_enable || e_enable) != *enable) {
        return(BCM_E_INTERNAL); /* Error if only Ing or Egr map  enabled */
    }

    return(BCM_E_NONE);
#else
    return BCM_E_UNAVAIL;
#endif  /* MODMAP_SUPPORT */
}

/*
 * Module ID Mapping on Switch Chips
 *
 * The programming of module/port destinations that are external to a
 * switch chip must match the mapping that is programmed into the
 * associated fabric chips.  Rather than trying to provide an
 * arbitrarily complex set of APIs to do this for the application, a
 * callback can be registered to map each module/port pair when it is
 * stored into hardware or fetched from hardware.
 *
 * For more information, refer to $SDK/doc/modid-mapping.txt.
 */

bcm_stk_modmap_cb_t     _bcm_stk_modmap_cb[BCM_MAX_NUM_UNITS];

/*
 * Function:
 *      bcm_stk_modmap_register
 * Purpose:
 *      Register module ID mapping/unmapping callback function
 * Parameters:
 *      unit - SOC unit #
 *      func - User callback function for module ID mapping/unmapping
 */
int
bcm_esw_stk_modmap_register(int unit, bcm_stk_modmap_cb_t func)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_IS_XGS_SWITCH(unit)) {
        return BCM_E_PARAM;
    }
    _bcm_stk_modmap_cb[unit] = func;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_stk_modmap_unregister
 * Purpose:
 *      Unregister module ID mapping/unmapping callback function
 * Parameters:
 *      unit - SOC unit #
 */
int
bcm_esw_stk_modmap_unregister(int unit)
{
    _bcm_stk_modmap_cb[unit] = NULL;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_stk_modmap_map
 * Purpose:
 *      Internal function to call module ID mapping/unmapping callback function
 * Parameters:
 *      unit    - (IN) SOC unit #
 *      setget  - (IN) Mapping direction identifier BCM_STK_MODMAP_GET or BCM_STK_MODMAP_SET
 *      mod_in  - (IN) Module Id to map 
 *      port_in  - (IN) Physical port to map no GPORT accepted
 *      mod_out - (OUT) Module Id after mapping
 *      port_out - (OUT) Physical port after mapping not a GPORT
 * Notes:
 *      This function is called from within the BCM API implementations
 *      whenever module ID mapping is needed. 
 */
int
_bcm_esw_stk_modmap_map(int unit, int setget,
                   bcm_module_t mod_in, bcm_port_t port_in,
                   bcm_module_t *mod_out, bcm_port_t *port_out)
{
    int         rv = BCM_E_NONE;
    bcm_module_t mod;
    bcm_port_t  port;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if ((port_in != -1) && (setget == BCM_STK_MODMAP_GET)) {
        /* Before the mapping, if multiple modid mode is enabled, adjust the
         * mod/port such that any port number above modport_max setting is
         * considered to be in the next module.
         * Tucana is expetion among all devices that support multiple modids 
         * and does not need this resolution.
         */
        if (!SOC_IS_TUCANA(unit) && NUM_MODID(unit) > 1 &&
            port_in > SOC_MODPORT_MAX(unit)) {
            bcm_module_t my_modid;
            BCM_IF_ERROR_RETURN (bcm_esw_stk_my_modid_get(unit, &my_modid));
            if ((mod_in == my_modid) && (BCM_MODID_INVALID != mod_in)) {
                mod_in += port_in / (SOC_MODPORT_MAX(unit) + 1);
                port_in %= (SOC_MODPORT_MAX(unit) + 1);
            }
        }
    }

    if ((_bcm_stk_modmap_cb[unit] != NULL) && (BCM_MODID_INVALID != mod_in)) {
        rv = (*_bcm_stk_modmap_cb[unit])(unit, setget,
                                         mod_in, port_in,
                                         &mod, &port);
        if (rv < 0) {
            mod = mod_in;
            port = port_in;
        }
    } else {
        /* default identity mapping */
        mod = mod_in;
        port = port_in;
    }

    if ((rv >= 0) && (port_in != -1) && (setget == BCM_STK_MODMAP_SET)) {
        /* After the mapping, if multiply-modid mode is enabled, adjust the
         * mod/port such that all ports are considered to be in the first
         * module.
         * Tucana is expetion among all devices that support multiple modids
         * and does not need this resolution.
         */
        if (!SOC_IS_TUCANA(unit) && (NUM_MODID(unit) > 1)) {
            bcm_module_t my_modid;
            int          isLocal;
            BCM_IF_ERROR_RETURN (bcm_esw_stk_my_modid_get(unit, &my_modid));
            if (BCM_MODID_INVALID != my_modid){
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_modid_is_local(unit, mod, &isLocal));
                if (isLocal) {
                    port += (mod - my_modid) * (SOC_MODPORT_MAX(unit) + 1);
                    mod = my_modid;
                }
            }
        }
    }
    if (port_out != NULL) {
        *port_out = port;
    }
    if (mod_out != NULL) {
        *mod_out = mod;
    }

    return rv;
}

/*
 * Function:
 *      bcm_stk_modmap_map
 * Purpose:
 *      Call module ID mapping/unmapping callback function
 * Parameters:
 *      unit - SOC unit #
 * Notes:
 *      This function is called from within the BCM API implementations
 *      whenever module ID mapping is needed, but it can also be called
 *      by the user application for informational purposes.
 */
int
bcm_esw_stk_modmap_map(int unit, int setget,
                   bcm_module_t mod_in, bcm_port_t port_in,
                   bcm_module_t *mod_out, bcm_port_t *port_out)
{
    bcm_port_t      tmp_port; 
    bcm_module_t    tmp_modid;
    bcm_trunk_t     tmp_tgid;
    int             tmp_id, isGport;
    _bcm_gport_dest_t gport_st;

    if (BCM_GPORT_IS_SET(port_in)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port_in, &tmp_modid, &tmp_port, 
                                   &tmp_tgid, &tmp_id));
        if ((BCM_TRUNK_INVALID != tmp_tgid) || 
            (-1 != tmp_id)) {
            return BCM_E_PORT;
        }
    } else {
        tmp_port = port_in;
        tmp_modid = mod_in;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_esw_stk_modmap_map(unit, setget, tmp_modid, tmp_port, 
                                mod_out, port_out));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
    if (TRUE == isGport) {
        gport_st.modid = *mod_out;
        gport_st.port = *port_out;
        gport_st.gport_type = _SHR_GPORT_TYPE_MODPORT;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_construct(unit, &gport_st, port_out));
        *mod_out = -1;
    }

    return BCM_E_NONE;
}

/****************************************************************
 *
 * Procedures related to source module ID base indexing
 *
 ****************************************************************/

#if defined(BCM_TRIDENT_SUPPORT)
/*
 * Function:
 *      _src_mod_base_index_bk_init
 * Purpose:
 *      Allocate and initialize data structures related to source modid base
 *      indexing.
 * Parameters:
 *      unit - (IN)SOC unit number. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_src_modid_base_index_bk_init(int unit)
{
    int src_mod_port_tbl_size, i;

    if (src_modid_base_index_bk[unit] == NULL) {
        src_modid_base_index_bk[unit] = 
            sal_alloc(sizeof(src_modid_base_index_bookkeeping_t),
                    "src_modid_base_index_bk");
        if (src_modid_base_index_bk[unit] == NULL) {
            return BCM_E_MEMORY;
        }
    }
    sal_memset(src_modid_base_index_bk[unit], 0,
            sizeof(src_modid_base_index_bookkeeping_t));

    src_modid_base_index_bk[unit]->num_ports =
        sal_alloc(sizeof(uint8) * (SOC_MODID_MAX(unit) + 1),
                "src_moid_base_index_bk num_ports array");
    if (src_modid_base_index_bk[unit]->num_ports == NULL) {
        _src_modid_base_index_bk_detach(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(src_modid_base_index_bk[unit]->num_ports, 0,
            sizeof(uint8) * (SOC_MODID_MAX(unit) + 1));

    src_mod_port_tbl_size = soc_mem_index_count(unit, src_mod_port_mems[0]);
    src_modid_base_index_bk[unit]->src_mod_port_table_bitmap = 
        sal_alloc(SHR_BITALLOCSIZE(src_mod_port_tbl_size),
                "src_mod_port_table_bitmap");
    if (src_modid_base_index_bk[unit]->src_mod_port_table_bitmap == NULL) {
        _src_modid_base_index_bk_detach(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(src_modid_base_index_bk[unit]->src_mod_port_table_bitmap, 0,
            SHR_BITALLOCSIZE(src_mod_port_tbl_size));

    /* Initialize all MODBASE tables such that all module IDs
     * initially share the same region in source mod port tables, from entry 0
     * to entry (SOC_MAX_NUM_PORTS - 1). A module ID will be allocated its own
     * region in source mod port tables as needed.
     */ 

    if(SOC_IS_KATANA(unit)) {
        if(soc_mem_is_valid(unit, SOURCE_TRUNK_MAP_MODBASEm)) {
            SOC_IF_ERROR_RETURN(soc_mem_clear(unit, SOURCE_TRUNK_MAP_MODBASEm,
                                              MEM_BLOCK_ALL, 0));
        }
    } else {

        for (i = 0; i < COUNTOF(src_modbase_mems); i++) {
            if(soc_mem_is_valid(unit, src_modbase_mems[i])) {
                SOC_IF_ERROR_RETURN(soc_mem_clear(unit, src_modbase_mems[i],
                            MEM_BLOCK_ALL, 0));
            }
        }
    }

    _BCM_SRC_MOD_PORT_TABLE_USED_SET_RANGE(unit, 0, SOC_MAX_NUM_PORTS);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _src_modid_base_index_bk_detach
 * Purpose:
 *      Deallocate data structures related to source modid base indexing.
 * Parameters:
 *      unit - (IN)SOC unit number. 
 * Returns:
 *      None
 */
STATIC void
_src_modid_base_index_bk_detach(int unit) 
{
    if (src_modid_base_index_bk[unit]) {
        if (src_modid_base_index_bk[unit]->num_ports) {
            sal_free(src_modid_base_index_bk[unit]->num_ports);
            src_modid_base_index_bk[unit]->num_ports = NULL;
        }
        if (src_modid_base_index_bk[unit]->src_mod_port_table_bitmap) {
            sal_free(src_modid_base_index_bk[unit]->src_mod_port_table_bitmap);
            src_modid_base_index_bk[unit]->src_mod_port_table_bitmap = NULL;
        }
        sal_free(src_modid_base_index_bk[unit]);
        src_modid_base_index_bk[unit] = NULL;

    }
}

/*
 * Function:
 *      _src_modid_base_index_alloc
 * Purpose:
 *      Allocate a source modid base index.
 * Parameters:
 *      unit   - (IN) SOC unit number. 
 *      modid  - (IN) Source Module ID.
 *      nports - (IN) Number of ports on the given module.
 *      base_index - (OUT) Base index.
 * Returns:
 *      BCM_E_xxx
 */
STATIC int
_src_modid_base_index_alloc(
        int unit,
        bcm_module_t modid,
        int nports,
        int *base_index)
{
    source_trunk_map_modbase_entry_t  stm_modbase_entry;
    int        max_base_index;
    int        i, j;
    SHR_BITDCL occupied;

    SOC_IF_ERROR_RETURN
        (READ_SOURCE_TRUNK_MAP_MODBASEm(unit, MEM_BLOCK_ANY, modid,
                                        &stm_modbase_entry));
    *base_index = soc_SOURCE_TRUNK_MAP_MODBASEm_field32_get(unit,
            &stm_modbase_entry, BASEf);
    if (*base_index != 0) {
        return BCM_E_NONE;
    }

    /* Find a contiguous region of source mod port tables that's large
     * enough to hold nports entries.
     */
    max_base_index = soc_mem_index_count(unit, src_mod_port_mems[0]) - nports;
    for (i = 0; i <= max_base_index; i++) {
        /* Check if the contiguous region of source mod port tables from
         * index i to (i + nports - 1) is free. 
         */
        _BCM_SRC_MOD_PORT_TABLE_TEST_RANGE(unit, i, nports, occupied); 
        if (!occupied) {
            break;
        }
    }
    if (i > max_base_index) {
        /* A contiguous region of the desired size could not be found in
         * source mod port tables.
         */
        return BCM_E_RESOURCE;
    }
    *base_index = i;

    if(SOC_IS_KATANA(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_mem_field32_modify(unit, SOURCE_TRUNK_MAP_MODBASEm,
                                    modid, BASEf, *base_index));
    } else {

        for (i = 0; i < COUNTOF(src_modbase_mems); i++) {
            SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit, src_modbase_mems[i],
                        modid, BASEf, *base_index));
        }
    }
    /* Set src_mod_port_table_bitmap */
    _BCM_SRC_MOD_PORT_TABLE_USED_SET_RANGE(unit, *base_index, nports);

    /* Clear the newly allocated region in each src mod port table */
    if(SOC_IS_KATANA(unit)) {
      for (j = *base_index; j < (*base_index + nports); j++) {
                SOC_IF_ERROR_RETURN(soc_mem_write(unit, SOURCE_TRUNK_MAP_TABLEm,
                            MEM_BLOCK_ALL, j,
                            soc_mem_entry_null(unit, SOURCE_TRUNK_MAP_TABLEm)));
      }

    } else {
        for (i = 0; i < COUNTOF(src_mod_port_mems); i++) {
            for (j = *base_index; j < (*base_index + nports); j++) {
                SOC_IF_ERROR_RETURN(soc_mem_write(unit, src_mod_port_mems[i],
                            MEM_BLOCK_ALL, j,
                            soc_mem_entry_null(unit, src_mod_port_mems[i])));
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _src_modid_base_index_free
 * Purpose:
 *      Free a source modid base index.
 * Parameters:
 *      unit   - (IN) SOC unit number. 
 *      modid  - (IN) Source Module ID.
 *      nports - (IN) Number of ports on the given module.
 * Returns:
 *      BCM_E_xxx
 */
STATIC int
_src_modid_base_index_free(
        int unit,
        bcm_module_t modid,
        int nports)
{
    source_trunk_map_modbase_entry_t stm_modbase_entry;
    int base_index;
    int i;

    SOC_IF_ERROR_RETURN
        (READ_SOURCE_TRUNK_MAP_MODBASEm(unit, MEM_BLOCK_ANY, modid,
                                        &stm_modbase_entry));
    base_index = soc_SOURCE_TRUNK_MAP_MODBASEm_field32_get(unit,
            &stm_modbase_entry, BASEf);
    if (base_index == 0) {
        return BCM_E_NONE;
    }

    /* Clear src mod port table bitmap */
    _BCM_SRC_MOD_PORT_TABLE_USED_CLR_RANGE(unit, base_index, nports);
    if(SOC_IS_KATANA(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_mem_field32_modify(unit, SOURCE_TRUNK_MAP_MODBASEm,
                                    modid, BASEf, 0));
    }else {
        for (i = 0; i < COUNTOF(src_modbase_mems); i++) {
            SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit, src_modbase_mems[i],
                        modid, BASEf, 0));
        }
    }
    return BCM_E_NONE;
}

#endif /* BCM_TRIDENT_SUPPORT */

/*
 * Function:
 *      bcm_esw_stk_module_enable
 * Purpose:
 *      Enable or disable a module and specify its number of ports.
 * Parameters:
 *      unit   - (IN) SOC unit number. 
 *      modid  - (IN) Module ID that is being enabled/disabled.
 *      nports - (IN) Number of ports on the given module, 0 for no port
 *                    status change, -1 for default number of ports.
 *      enable - (IN) TRUE to enable, FALSE to disable.
 * Returns:
 *      BCM_E_xxx
 */
int 
bcm_esw_stk_module_enable(
    int unit, 
    bcm_module_t modid, 
    int nports, 
    int enable)
{
#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {

        bcm_module_t        mod_out;
        bcm_port_t          port_out;
        int                 is_local;
        int                 old_nports;
        int                 base_index;

        if (modid > SOC_MODID_MAX(unit)) {
            return BCM_E_PARAM;
        }

        if ((nports > SOC_MAX_NUM_PORTS) || (nports < -1)) {
            return BCM_E_PARAM;
        }

        if (enable && (nports == 0)) {
            return BCM_E_PARAM;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                     modid, -1,
                                     &mod_out, &port_out));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_modid_is_local(unit, mod_out, &is_local));

        if (enable) {
            if (is_local) {
                /* If local modid, use the base modid */
                mod_out = SOC_BASE_MODID(unit);
            }

            old_nports = src_modid_base_index_bk[unit]->num_ports[mod_out];
            if (old_nports > 0) {
                /* Already enabled */
                return BCM_E_EXISTS;
            }

            if (nports > 0) {
                BCM_IF_ERROR_RETURN
                    (_src_modid_base_index_alloc(unit, mod_out, nports,
                                                     &base_index));
                src_modid_base_index_bk[unit]->num_ports[mod_out] = nports;
            } else if (nports == -1) {
                /* Number of ports default to SOC_MAX_NUM_PORTS */
                BCM_IF_ERROR_RETURN
                    (_src_modid_base_index_alloc(unit, mod_out,
                                                     SOC_MAX_NUM_PORTS,
                                                     &base_index));
                src_modid_base_index_bk[unit]->num_ports[mod_out] =
                                                     SOC_MAX_NUM_PORTS;
            }
        } else {
            if (is_local) {
                /* Cannot disable local module */
                return BCM_E_PARAM;
            }

            old_nports = src_modid_base_index_bk[unit]->num_ports[mod_out];
            if (old_nports == 0) {
                /* Not enabled */
                return BCM_E_DISABLED;
            }
            BCM_IF_ERROR_RETURN
                (_src_modid_base_index_free(unit, mod_out, old_nports));
            src_modid_base_index_bk[unit]->num_ports[mod_out] = 0;
        }

        return BCM_E_NONE;
    } 
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _bcm_esw_src_modid_base_index_get
 * Purpose:
 *      Allocate a source modid base index.
 * Parameters:
 *      unit   - (IN) SOC unit number. 
 *      modid  - (IN) Source Module ID.
 *      base_index - (OUT) Source module base index.
 * Returns:
 *      BCM_E_xxx
 */
int
_bcm_esw_src_modid_base_index_get(
        int unit,
        bcm_module_t modid,
        int *base_index)
{
#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {

        source_trunk_map_modbase_entry_t  stm_modbase_entry;
        int nports;

        SOC_IF_ERROR_RETURN
            (READ_SOURCE_TRUNK_MAP_MODBASEm(unit, MEM_BLOCK_ANY, modid,
                                            &stm_modbase_entry));
        *base_index = soc_SOURCE_TRUNK_MAP_MODBASEm_field32_get(unit,
                &stm_modbase_entry, BASEf);
        if (*base_index != 0) {
            /* Source module base index already allocated */
            return BCM_E_NONE;
        }

        if (NULL == src_modid_base_index_bk[unit]) {
            return BCM_E_INIT;
        }

        /* Allocate a source module base index */
        nports = src_modid_base_index_bk[unit]->num_ports[modid];
        if (nports == 0) {
            /* Assume SOC_MAX_NUM_PORTS if nports is not initialized
             * using bcm_esw_stk_module_enable API
             */
            nports = SOC_MAX_NUM_PORTS;
        }
        BCM_IF_ERROR_RETURN
            (_src_modid_base_index_alloc(unit, modid, nports, base_index));
        src_modid_base_index_bk[unit]->num_ports[modid] = nports;

        return BCM_E_NONE;
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _bcm_esw_src_modid_port_get
 * Purpose:
 *      get modid and port number from STM entry index.
 * Parameters:
 *      unit   - (IN) SOC unit number. 
 *      stm_inx  - (IN) Source Trunk Map entry index.
 *      modid  - (OUT) Source Module ID.
 *      port - (OUT) Port Number.
 * Returns:
 *      BCM_E_xxx
 */
int
_bcm_esw_src_modid_port_get(
        int unit,
        int stm_inx,
        bcm_module_t *modid,
        int *port)
{
#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {
        int i;
        int rv;
        int base_inx;
        source_trunk_map_modbase_entry_t  stm_modbase_entry;

        /* The SOURCE_TRUNK_MAP entry range pointed by the base index
         * for each modid is not overlapped.
         */
        for (i = 0; i <= SOC_MODID_MAX(unit) ; i++) {

            /* only read the valid entry */
            if (src_modid_base_index_bk[unit]->num_ports[i]) {
                rv = READ_SOURCE_TRUNK_MAP_MODBASEm(unit, MEM_BLOCK_ANY, i,
                        &stm_modbase_entry);
                if (SOC_FAILURE(rv)) {
                    continue;
                }
                base_inx = soc_SOURCE_TRUNK_MAP_MODBASEm_field32_get(unit,
                              &stm_modbase_entry, BASEf);
                /* check if the stm_inx falls in the range */
                if ((stm_inx >= base_inx) && (stm_inx < (base_inx + 
                        src_modid_base_index_bk[unit]->num_ports[i]))) {
                    /* found the modid */
                    *modid = i;
                    *port = stm_inx - base_inx;
                    return BCM_E_NONE;
                }
            }
        }
        return BCM_E_PARAM; 
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _bcm_esw_src_mod_port_table_index_get
 * Purpose:
 *      Derive index to source mod port tables.
 * Parameters:
 *      unit  - (IN) SOC unit number. 
 *      modid - (IN) Source Module ID.
 *      port  - (IN) Source Port ID.
 *      index - (OUT) Source mod port table index.
 * Returns:
 *      BCM_E_xxx
 */
int
_bcm_esw_src_mod_port_table_index_get(
        int unit,
        bcm_module_t modid,
        bcm_port_t port,
        int *index)
{
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_src_modid_base_index_get(unit, modid, index));
        *index += port;
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
        *index = (modid * 64) + port;
    } else
#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_trunk_group_overlay)) {
        *index = (modid * (SOC_PORT_ADDR_MAX(unit) + 1)) + port;
    } else
#endif
    {
        *index = (modid << SOC_TRUNK_BIT_POS(unit)) + port;
    }

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_stk_sw_dump
 * Purpose:
 *     Displays Stack software structure information.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_stk_sw_dump(int unit)
{
    char  pfmt[SOC_PBMP_FMT_LEN];

    soc_cm_print("\nSW Information Stack - Unit %d\n\n", unit);

    /* Source module id */
    soc_cm_print("  Source Module Id: %d\n\n", SOC_BASE_MODID(unit));

    /* Stack port bitmaps */
    soc_cm_print("  Stack Ports\n");
    soc_cm_print("      Current :  %s\n",
                 SOC_PBMP_FMT(SOC_PBMP_STACK_CURRENT(unit), pfmt));
    soc_cm_print("      Inactive:  %s\n",
                 SOC_PBMP_FMT(SOC_PBMP_STACK_INACTIVE(unit), pfmt));
    soc_cm_print("      Previous:  %s\n",
                 SOC_PBMP_FMT(SOC_PBMP_STACK_PREVIOUS(unit), pfmt));
    soc_cm_print("\n");

#if defined(BCM_TRX_SUPPORT)
    /* Modport profile */
    /* Display those entries with reference count > 0 */
    if (modport_profile[unit] != NULL) {
        int     i;
        int     num_entries;
        int     ref_count;
        int     entries_per_set;
        int     modid_count;
        uint32  index;
        pbmp_t  hg_pbmp;
        modport_map_entry_t    *entry_p;

        num_entries = soc_mem_index_count
            (unit, modport_profile[unit]->tables[0].mem);
        soc_cm_print("  Stack Modport Profile\n");
        soc_cm_print("      Number of entries: %d\n", num_entries);
        soc_cm_print("      Index RefCount EntriesPerSet - "
                     "HIGIG_PORT_BITMAP\n");

        modid_count = SOC_MODID_MAX(unit) + 1;
        for (index = 0; index < num_entries; index += modid_count) {
            if (SOC_FAILURE
                (soc_profile_mem_ref_count_get(unit,
                                               modport_profile[unit],
                                               index, &ref_count))) {
                break;
            }

            if (ref_count <= 0) {
                continue;
            }

            for (i = 0; i < modid_count; i++) {
                entries_per_set =
                    modport_profile[unit]->tables[0].entries[index+i].entries_per_set;
                soc_cm_print("     %5d %8d %13d    ",
                             index + i, ref_count, entries_per_set);

                switch (BCM_CHIP_FAMILY(unit)) {
                case BCM_FAMILY_TRIUMPH:
                case BCM_FAMILY_TRIUMPH2:
                    entry_p = SOC_PROFILE_MEM_ENTRY(unit,
                                                    modport_profile[unit],
                                                    modport_map_entry_t *,
                                                    index + i);
                    soc_mem_pbmp_field_get(unit, MODPORT_MAPm, entry_p,
                                           HIGIG_PORT_BITMAPf, &hg_pbmp);
                    soc_cm_print("%s\n", SOC_PBMP_FMT(hg_pbmp, pfmt));
                    break;
                default:
                    BCM_PBMP_CLEAR(hg_pbmp);
                    soc_cm_print("%s\n", SOC_PBMP_FMT(hg_pbmp, pfmt));
                    break;
                }
            }
        }
    }
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
        bcm_td_modport_map_sw_dump(unit);
    }
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(LOCAL_MODMAP_SUPPORT)
    /* Modmap information */
    if (mod_map_data[unit] != NULL) {
        int  i;
        int  num;
        int  count;

        soc_cm_print("\n  Stack Modmap Data\n");

        count = 0;
        num = COUNTOF(mod_map_data[unit]->l_modid);
        soc_cm_print("      l_modid: ");
        for (i = 0; i < num; i++) {
            if ((count > 0) && (!(count % 10))) {
                soc_cm_print("\n               ");
            }
            soc_cm_print(" %4d", mod_map_data[unit]->l_modid[i]);
            count++;
        }
        soc_cm_print("\n");

        count = 0;
        num = COUNTOF(mod_map_data[unit]->thresh_d);
        soc_cm_print("      thresh_d:");
        for (i = 0; i < num; i++) {
            if ((count > 0) && (!(count % 10))) {
                soc_cm_print("\n               ");
            }
            soc_cm_print(" %4d", mod_map_data[unit]->thresh_d[i]);
            count++;
        }
        soc_cm_print("\n");

        count = 0;
        soc_cm_print("      fmod0:   ");
        num = COUNTOF(mod_map_data[unit]->fmod0);
        for (i = 0; i < num; i++) {
            if ((count > 0) && (!(count % 10))) {
                soc_cm_print("\n               ");
            }
            soc_cm_print(" %4d", mod_map_data[unit]->fmod0[i]);
            count++;
        }
        soc_cm_print("\n");
    }
#endif  /* MODMAP_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_src_modid_base_index)) {
        int i, rv, base_index;
        source_trunk_map_modbase_entry_t stm_modbase_entry;

        soc_cm_print("\n  Stack Source Trunk Map Modbase Data\n");

        for (i = 0; i <= SOC_MODID_MAX(unit) ; i++) {
            rv = READ_SOURCE_TRUNK_MAP_MODBASEm(unit, MEM_BLOCK_ANY, i,
                    &stm_modbase_entry);
            if (SOC_FAILURE(rv)) {
                continue;
            }
            base_index = soc_SOURCE_TRUNK_MAP_MODBASEm_field32_get(unit,
                    &stm_modbase_entry, BASEf);
            if (0 == base_index) {
                continue;
            }
            soc_cm_print("    Modid %3d, ", i);
            soc_cm_print("source trunk map modbase index %4d, ", base_index);
            soc_cm_print("num_ports %2d\n",
                    src_modid_base_index_bk[unit]->num_ports[i]);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
