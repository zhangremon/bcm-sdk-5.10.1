/*
 * $Id: switch.c 1.17.42.1 Broadcom SDK $
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
 * File:        switch.c
 * Purpose:     BCM definitions  for bcm_switch_control and
 *              bcm_switch_port_control functions
 */

#include <soc/drv.h>
#include <soc/debug.h>

#include <soc/sbx/sbx_drv.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/port.h>

#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_scoreboard.h>

#include <bcm/switch.h>
#include <bcm/error.h>
#include <soc/sbx/hal_ka_auto.h>

#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx_dispatch.h>

int
bcm_sbx_switch_control_port_set(int unit,
                                bcm_port_t port,
                                bcm_switch_control_t type,
                                int arg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_switch_control_port_get(int unit,
                                bcm_port_t port,
                                bcm_switch_control_t type,
                                int *arg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_switch_state_sync(int unit, int arg)
{
    int rv = BCM_E_UNAVAIL;

#ifdef BCM_WARM_BOOT_SUPPORT
    int sync;

    if (!arg) {
        soc_cm_print("arg=0. Skipping state sync \n");
        return BCM_E_NONE; /* nothing to sync */
    }
    rv = BCM_E_NONE;

    sync = 0; /* do not sync after each individual module */
    /* SOC state */
    BCM_IF_ERROR_RETURN(soc_sbx_wb_sync(unit, sync));
    if (SOC_IS_SBX_QE2000(unit) || SOC_IS_SBX_SIRIUS(unit)) {
        BCM_IF_ERROR_RETURN(soc_sbx_wb_common_state_sync(unit, sync));
    }

    /* BCM State */
    /* PORT state - remove if when SIRIUS support added */
    if (!SOC_IS_SIRIUS(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_wb_port_state_sync(unit, sync));
    }

    BCM_IF_ERROR_RETURN(bcm_sbx_wb_fabric_state_sync(unit, sync));
    BCM_IF_ERROR_RETURN(bcm_sbx_wb_cosq_state_sync(unit, sync));

    /* Now commit the scache to Persistent memory */
    rv = soc_scache_commit(unit);
    if (rv != SOC_E_NONE) {
        soc_cm_print("%s: Error(%s) sync'ing scache to Persistent memory. \n",
                     FUNCTION_NAME(), soc_errmsg(rv));
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}

int
bcm_sbx_switch_control_set(int unit, bcm_switch_control_t type, int arg)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_WARM_BOOT_SUPPORT
    int stable_select;
#endif

    soc_cm_print("bcm_sbx_switch_control_set %d 0x%x\n", type, arg);

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);
    switch (type) {
    case bcmSwitchControlSync:
        rv = bcm_sbx_switch_state_sync(unit, arg);
        break;
    case bcmSwitchControlAutoSync:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = bcm_sbx_switch_control_get(unit, bcmSwitchStableSelect, 
                                        &stable_select);
        if (BCM_SUCCESS(rv)) {
            if (stable_select != BCM_SWITCH_STABLE_NONE) {
                SOC_CONTROL(unit)->autosync = (arg ? 1 : 0);
            } else {
                rv = BCM_E_NOT_FOUND;
            }
        }
#endif
        break;
    case bcmSwitchStableSelect:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_set(unit, arg, 0);
#endif
        break;
    case bcmSwitchStableSize:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_size_set(unit, arg);
#endif
        break;
    case bcmSwitchWarmBoot:
#ifdef BCM_WARM_BOOT_SUPPORT
        /* If true, set the Warm Boot state; clear otherwise */
        if (arg) {
            SOC_WARM_BOOT_START(unit);
        } else {
            SOC_WARM_BOOT_DONE(unit);
        }
        rv = BCM_E_NONE; 
#endif
        break;
    default: 
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_switch_control_set,unit, type, arg);
        break;
    }
    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_switch_control_get(int unit, bcm_switch_control_t type, int *arg)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 flags;
#endif

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    switch (type) {
    case bcmSwitchAutoQueues:
        *arg = SOC_SBX_CFG(unit)->bcm_cosq_init;
        rv = BCM_E_NONE;
        break;
    case bcmSwitchControlAutoSync:
#ifdef BCM_WARM_BOOT_SUPPORT
        *arg = SOC_CONTROL(unit)->autosync;
        rv = BCM_E_NONE;
#endif
        break;
    case bcmSwitchStableSelect:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_get(unit, arg, &flags);
#endif
        break;
    case bcmSwitchStableSize:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_size_get(unit, arg);
#endif
        break;
    case bcmSwitchStableUsed:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_used_get(unit, arg);
#endif
        break;
    case bcmSwitchWarmBoot:
#ifdef BCM_WARM_BOOT_SUPPORT
        *arg = SOC_WARM_BOOT(unit);
        rv = BCM_E_NONE;
#endif
        break;
    default:
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_switch_control_get,unit, type, arg);
        break;
    }
    return rv;
}


/*
 * Function:
 *      bcm_switch_event_register
 * Description:
 *      Registers a call back function for switch critical events
 * Parameters:
 *      unit        - Device unit number
 *  cb          - The desired call back function to register for critical events.
 *  userdata    - Pointer to any user data to carry on.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *      Several call back functions could be registered, they all will be called upon
 *      critical event. If registered callback is called it is adviced to log the 
 *  information and reset the chip. 
 *  Same call back function with different userdata is allowed to be registered. 
 */
int 
bcm_sbx_switch_event_register(int unit, 
			      bcm_switch_event_cb_t cb, 
                              void *userdata)
{
    return MBCM_SBX_DRIVER_CALL(unit, mbcm_switch_event_register, unit, cb, userdata);
}


/*
 * Function:
 *      bcm_switch_event_unregister
 * Description:
 *      Unregisters a call back function for switch critical events
 * Parameters:
 *      unit        - Device unit number
 *  cb          - The desired call back function to unregister for critical events.
 *  userdata    - Pointer to any user data associated with a call back function
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *  If userdata = NULL then all matched call back functions will be unregistered,
 */
int 
bcm_sbx_switch_event_unregister(int unit, 
				bcm_switch_event_cb_t cb, 
                                void *userdata)
{
    return MBCM_SBX_DRIVER_CALL(unit, mbcm_switch_event_unregister, unit, cb, userdata);
}
