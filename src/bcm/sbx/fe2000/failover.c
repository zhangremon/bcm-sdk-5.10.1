/*
 * $Id: failover.c 1.7.52.1 Broadcom SDK $
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
 * FE2000 failover API
 */

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g2p3/g2p3.h>

#include <bcm/types.h>
#include <bcm/failover.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/fe2000/allocator.h>

#include <bcm/module.h>

typedef struct fe2k_failover_wb_mem_layout_s {
    uint8  fo_v[(SBX_PROTECTION_END + 1) / 8]; /* fo_idx valid   */
    uint8  fo_e[(SBX_PROTECTION_END + 1) / 8]; /* fo_idx enabled */
} fe2k_failover_wb_mem_layout_t;


typedef struct fe2k_failover_state_s {
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 wb_hdl;
    uint32 wb_size;
#endif /* BCM_WARM_BOOT_SUPPORT */
} fe2k_failover_state_t;


#define FO_WB_VERSION_1_0        SOC_SCACHE_VERSION(1,0)
#define FO_WB_CURRENT_VERSION    FO_WB_VERSION_1_0


fe2k_failover_state_t  fo_state[BCM_MAX_NUM_UNITS];

#ifdef BCM_WARM_BOOT_SUPPORT

#define FO_WB_AVAIL(_u)  (fo_state[_u].wb_size > 0)

#define WB_FO_VAR_DEREF(lo__, var__, idx__)  (lo__)->var__[(idx__) / 8]

#define WB_FO_VAR_GET(lo__, var__, idx__) \
    (!!  (WB_FO_VAR_DEREF(lo__, var__, idx__)  & (1 << ((idx__) & 7))))

#define WB_FO_VAR_SET(lo__, var__, idx__, val__) \
    WB_FO_VAR_DEREF(lo__, var__, idx__) =                                  \
        ((WB_FO_VAR_DEREF(lo__, var__, idx__) & ~(1 << ((idx__) & 7)))  |  \
         (!!(val__) << ((idx__) & 7)));

#define WB_FO_ENABLE_SET(lo__, idx__, val__ )  \
    WB_FO_VAR_SET(lo__, fo_e, idx__, val__)
#define WB_FO_ENABLE_GET(lo__, idx__ )  \
    WB_FO_VAR_GET(lo__, fo_e, idx__)

#define WB_FO_VALID_SET(lo__, idx__, val__ )  \
    WB_FO_VAR_SET(lo__, fo_v, idx__, val__)
#define WB_FO_VALID_GET(lo__, idx__ )  \
    WB_FO_VAR_GET(lo__, fo_v, idx__)

#else /* BCM_WARM_BOOT_SUPPORT */

#define FO_WB_AVAIL(_u) 0

#endif /* BCM_WARM_BOOT_SUPPORT */


int bcm_fe2000_failover_create(int unit, uint32 flags, 
                               bcm_failover_t *failover_id);



#ifdef BCM_WARM_BOOT_SUPPORT
static int
fe2k_failover_wb_layout_get(int unit, soc_scache_handle_t hdl,
                            fe2k_failover_wb_mem_layout_t **layout)
{
    uint32 size;
    soc_wb_cache_t *wbc;

    *layout = NULL;
    BCM_IF_ERROR_RETURN(
        soc_scache_ptr_get(unit, hdl, (uint8**)&wbc, &size));

    *layout = (fe2k_failover_wb_mem_layout_t*)wbc->cache;
    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */


int 
bcm_fe2000_failover_init(int unit)
{
    int rv = BCM_E_NONE;

#ifdef BCM_WARM_BOOT_SUPPORT
    int fo_idx, upgrade;
    fe2k_failover_wb_mem_layout_t *layout;

    upgrade = 0;
    sal_memset(&fo_state[unit], 0, sizeof(fo_state[0]));
    
    SOC_SCACHE_HANDLE_SET(fo_state[unit].wb_hdl, unit, BCM_MODULE_FAILOVER, 0);
    fo_state[unit].wb_size = (sizeof(fe2k_failover_wb_mem_layout_t) +
                              SOC_WB_SCACHE_CONTROL_SIZE);
    
    rv = soc_wb_state_alloc_and_check(unit, fo_state[unit].wb_hdl,
                                      &fo_state[unit].wb_size,
                                      FO_WB_CURRENT_VERSION,
                                      &upgrade);
    if (upgrade) {
        return BCM_E_UNAVAIL;
    }
    if (rv == BCM_E_UNAVAIL) {
        rv = BCM_E_NONE;
        fo_state[unit].wb_size = 0;
    }

    /* Nothing more to do for cold boot */
    if (!(SOC_WARM_BOOT(unit)) || 
        !(FO_WB_AVAIL(unit))  ||
        BCM_FAILURE(rv)) {
        return rv;
    }

    /* This is a Warm boot, and there is an scache available to recover */
    rv = fe2k_failover_wb_layout_get(unit,fo_state[unit].wb_hdl, &layout);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    for (fo_idx = SBX_PROTECTION_BASE; 
         fo_idx <= SBX_PROTECTION_END;
         fo_idx ++) {

        if (WB_FO_VALID_GET(layout, fo_idx)) {
            rv = bcm_fe2000_failover_create(unit, 
                                            BCM_FAILOVER_WITH_ID, &fo_idx);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }
    }
    
#endif /* BCM_WARM_BOOT_SUPPORT */
    return rv;
}

/*
 * Set a failover object to enable or disable (note that failover object
 * 0 is reserved.
 */
int 
bcm_fe2000_failover_set(int unit, bcm_failover_t failover_id, int enable)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_rr_t rr;

    rv = _sbx_gu2_resource_test(unit, SBX_GU2K_USR_RES_PROTECTION,
                                failover_id);
    if (rv != BCM_E_EXISTS) {
        return rv;
    }

    rr.backup = enable;
    rv = soc_sbx_g2p3_rr_set(unit, failover_id, &rr);


#ifdef BCM_WARM_BOOT_SUPPORT

    if (FO_WB_AVAIL(unit)) {
        fe2k_failover_wb_mem_layout_t *layout;

        soc_scache_handle_lock(unit, fo_state[unit].wb_hdl);

        rv = fe2k_failover_wb_layout_get(unit,fo_state[unit].wb_hdl, &layout);
        if (BCM_FAILURE(rv)) {
            soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
            return rv;
        }

        WB_FO_ENABLE_SET(layout, failover_id, enable);

        soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}


/* Get the enable status of a failover object. */
int 
bcm_fe2000_failover_get(int unit, bcm_failover_t failover_id, int *enable)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_rr_t rr;

    rv = _sbx_gu2_resource_test(unit, SBX_GU2K_USR_RES_PROTECTION,
                                failover_id);
    if (rv != BCM_E_EXISTS) {
        return rv;
    }

    rv = soc_sbx_g2p3_rr_get(unit, failover_id, &rr);
    *enable = rr.backup;
    
    return rv;
}

int 
bcm_fe2000_failover_create(int unit, uint32 flags, bcm_failover_t *failover_id)
{

    int rv = BCM_E_NONE;
    int alloc_flags = 0;

    if (failover_id == NULL
        || (flags
            && flags != BCM_FAILOVER_WITH_ID
            && flags != BCM_FAILOVER_REPLACE)) {
        return BCM_E_PARAM;
    }

    if (flags & BCM_FAILOVER_WITH_ID) {
        rv = _sbx_gu2_resource_test(unit, SBX_GU2K_USR_RES_PROTECTION,
                                    *failover_id);
        if ((flags & BCM_FAILOVER_REPLACE) && (rv == BCM_E_NOT_FOUND)) {
            return rv;
        }
        if (((flags & BCM_FAILOVER_REPLACE) == 0) && (rv == BCM_E_EXISTS)) {
            return rv;
        }
        rv = BCM_E_NONE;
    }

    if (flags & BCM_FAILOVER_WITH_ID) {
        alloc_flags = _SBX_GU2_RES_FLAGS_RESERVE;
    }
    
    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_PROTECTION,
                                 1, (uint32_t *) failover_id, alloc_flags);

    if (BCM_SUCCESS(rv)) {
        rv = bcm_fe2000_failover_set(unit, *failover_id, 0);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (BCM_SUCCESS(rv) && FO_WB_AVAIL(unit)) {
        fe2k_failover_wb_mem_layout_t *layout;
        soc_scache_handle_lock(unit, fo_state[unit].wb_hdl);

        rv = fe2k_failover_wb_layout_get(unit,fo_state[unit].wb_hdl, &layout);
        if (BCM_FAILURE(rv)) {
            soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
            return rv;
        }

        WB_FO_VALID_SET(layout, *failover_id, 1);
        WB_FO_ENABLE_SET(layout, *failover_id, 0);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}


int 
bcm_fe2000_failover_destroy(int unit, bcm_failover_t failover_id)
{
    int rv = BCM_E_NONE;

    rv = _sbx_gu2_resource_test(unit, SBX_GU2K_USR_RES_PROTECTION,
                                failover_id);
    if (rv == BCM_E_EXISTS) {
        rv = bcm_fe2000_failover_set(unit, failover_id, 0);
        rv = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_PROTECTION, 1,
                                    (uint32_t *)&failover_id, 0);
    }


#ifdef BCM_WARM_BOOT_SUPPORT

    if (FO_WB_AVAIL(unit)) {
        fe2k_failover_wb_mem_layout_t *layout;

        soc_scache_handle_lock(unit, fo_state[unit].wb_hdl);

        rv = fe2k_failover_wb_layout_get(unit,fo_state[unit].wb_hdl, &layout);
        if (BCM_FAILURE(rv)) {
            soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
            return rv;
        }

        WB_FO_VALID_SET(layout, failover_id, 0);

        soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}
