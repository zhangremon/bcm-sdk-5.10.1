/*
 * $Id: ptp_init.c 1.1.2.16 Broadcom SDK $
 * 
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
#if defined(INCLUDE_PTP)

#include <soc/defs.h>
#include <soc/drv.h>

#include <sal/core/dpc.h>

#include <bcm/ptp.h>
#include <bcm_int/esw/ptp.h>
#include <bcm/error.h>

#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/katana.h>
#endif


static const _bcm_ptp_clock_cache_t cache_default;
static int next_stack_id = 0;  /* ID of next stack */
_bcm_ptp_unit_cache_t _bcm_esw_ptp_unit_array[BCM_MAX_NUM_UNITS] = {{0}};
_bcm_ptp_info_t _bcm_esw_ptp_info[BCM_MAX_NUM_UNITS];

/* Static functions */
static int _bcm_ptp_system_debug_configure(int unit, bcm_ptp_stack_id_t ptp_id, uint32 flags);

/*
 * Function:
 *      bcm_esw_ptp_lock
 * Purpose:
 *      Locks the unit mutex
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INIT
 * Notes:
 */
int bcm_esw_ptp_lock(int unit)
{
    if (_bcm_esw_ptp_info[unit].mutex == NULL) {
        return BCM_E_INIT;
    }

    sal_mutex_take(_bcm_esw_ptp_info[unit].mutex, sal_mutex_FOREVER);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_ptp_unlock
 * Purpose:
 *      Unlocks the unit mutex
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 * Notes:
 */
int bcm_esw_ptp_unlock(int unit)
{
    if (sal_mutex_give(_bcm_esw_ptp_info[unit].mutex) != 0) {
        return BCM_E_INTERNAL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_ptp_init
 * Purpose:
 *      Initialize the PTP subsystem
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_init(
    int unit)
{
    int rv = BCM_E_NONE;
    _bcm_ptp_info_t *ptp_info_p;

    if (soc_feature(unit, soc_feature_ptp)) {
        if (_bcm_esw_ptp_info[unit].mutex == NULL) {
            _bcm_esw_ptp_info[unit].mutex = sal_mutex_create("ptp.mutex");
            if (_bcm_esw_ptp_info[unit].mutex == NULL) {
                rv = BCM_E_MEMORY;
            }
        }
#if defined(BCM_KATANA_SUPPORT)
        if (SOC_IS_KATANA(unit)) {
            rv = bcm_kt_ptp_init(unit);
        }
#endif

#if defined BCM_ENDURO_SUPPORT
/* TODO: correct define to use ?*/
        if (SOC_IS_ENDURO(unit)) {
            SET_PTP_INFO;

            if (ptp_info_p->initialized) {
                /* Reset everything */
                /* TODO: add later */
            }

            ptp_info_p->initialized = 1;
        }
#endif
        if (BCM_FAILURE(rv)) {
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_rx_init(unit))) {
            PTP_ERROR_FUNC("_bcm_ptp_rx_init()");
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_clock_cache_init(unit))) {
            PTP_ERROR_FUNC("_bcm_ptp_clock_cache_init()");
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_management_init(unit))) {
            PTP_ERROR_FUNC("_bcm_ptp_management_init()");
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_unicast_master_table_init(unit))) {
            PTP_ERROR_FUNC("_bcm_ptp_unicast_master_table_init()");
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_unicast_slave_table_init(unit))) {
            PTP_ERROR_FUNC("_bcm_ptp_unicast_slave_table_init()");
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_acceptable_master_table_init(unit))) {
            PTP_ERROR_FUNC("_bcm_ptp_acceptable_master_table_init()");
            goto done;
        }
    }
done:    
    if (BCM_FAILURE(rv)) {
       sal_mutex_destroy(_bcm_esw_ptp_info[unit].mutex);
       _bcm_esw_ptp_info[unit].mutex = NULL;
    }
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_detach
 * Purpose:
 *      Shut down the PTP subsystem
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_detach(
    int unit)
{
    int result = BCM_E_UNAVAIL;


    if (soc_feature(unit, soc_feature_ptp)) {
        if (_bcm_esw_ptp_info[unit].mutex == NULL) {
            return BCM_E_NONE;
        }

        bcm_esw_ptp_lock(unit);

#if defined(BCM_KATANA_SUPPORT)
        if (SOC_IS_KATANA(unit)) {
            result = bcm_kt_ptp_detach(unit);
        } 
#endif
        
        BCM_IF_ERROR_RETURN(bcm_esw_ptp_unlock(unit));

        sal_mutex_destroy(_bcm_esw_ptp_info[unit].mutex);

        _bcm_esw_ptp_info[unit].mutex = NULL;
    }

    return result;
}

/*
 * Function:
 *      bcm_ptp_stack_create
 * Purpose:
 *      Create a PTP stack instance
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_info - (IN/OUT) Pointer to an PTP Stack Info structure
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_stack_create(
    int unit,
    bcm_ptp_stack_info_t *info)
{
    int rv = BCM_E_NONE;
    _bcm_ptp_stack_info_t *stack_p;
    _bcm_ptp_info_t *ptp_info_p;
    pbmp_t pbmp = {{0x03008000}};
    int stack_initialized = 0;
    int i;

    SET_PTP_INFO;
    if (soc_feature(unit, soc_feature_ptp)) {
        bcm_esw_ptp_lock(unit);
        
        if (info->flags & BCM_PTP_STACK_WITH_ID) {
            if (info->id > PTP_MAX_STACKS_PER_UNIT) {
                rv = BCM_E_PARAM;
                goto done;
            }
        } else {
            info->id = (bcm_ptp_stack_id_t) next_stack_id++;
        }
        /* Check to see it is in_use */
        stack_p = &ptp_info_p->stack_info[info->id];
        if (stack_p->in_use) {
            rv = BCM_E_EXISTS;
            goto done;
        }
        stack_p->stack_id = info->id;
        stack_p->unit = unit;
        stack_p->in_use = 1;

        /* Check if the stack supports Keystone */

        if (info->flags & BCM_PTP_STACK_EXTERNAL_TOP) {
            if (BCM_FAILURE(rv = _bcm_ptp_external_stack_create(unit, info, stack_p->stack_id))) {
                PTP_ERROR_FUNC("_bcm_ptp_external_stack_create()");
                goto done;
            }
            stack_initialized = 1;
        } else if (SOC_IS_KATANA(unit)) {
            if (BCM_FAILURE(rv = _bcm_ptp_internal_stack_create(unit, info, stack_p->stack_id))) {
                goto done;
            }
            stack_initialized = 1;
        }

        if (!stack_initialized) {
            soc_cm_print("Found no supported PTP platforms");
            goto done;
        }

        soc_cm_print("initializing rx\n"); sal_sleep(1);
        if (BCM_FAILURE(rv = _bcm_ptp_rx_stack_create(                  /* call with blank MACs for Katana */
                unit, stack_p->stack_id, &stack_p->ext_info.host_mac, 
                &stack_p->ext_info.top_mac, stack_p->ext_info.tpid,
                (stack_p->ext_info.vlan | (stack_p->ext_info.vlan_pri << 5))))) {
            PTP_ERROR_FUNC("_bcm_ptp_rx_stack_create()");
            goto done;
        }

        for (i = 0; i < PTP_MAX_CLOCKS_PER_STACK; ++i) {
            if (BCM_FAILURE(rv = _bcm_ptp_rx_clock_create(
                    unit, stack_p->stack_id, i))) {
                PTP_ERROR_FUNC("_bcm_ptp_rx_clock_create()");
                goto done;
            }   
        }
        
        if (BCM_FAILURE(rv = _bcm_ptp_clock_cache_stack_create(
            unit,stack_p->stack_id))) {
            PTP_ERROR_FUNC("_bcm_ptp_clock_cache_stack_create()");
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_management_stack_create(
                unit, stack_p->stack_id, &stack_p->ext_info.host_mac, 
                &stack_p->ext_info.top_mac, stack_p->ext_info.host_ip_addr, 
                stack_p->ext_info.top_ip_addr, stack_p->ext_info.vlan, 
                stack_p->ext_info.vlan_pri, pbmp))) {
            PTP_ERROR_FUNC("_bcm_ptp_management_stack_create()");
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_unicast_master_table_stack_create(
               unit, stack_p->stack_id))) {
            PTP_ERROR_FUNC("_bcm_ptp_unicast_master_table_stack_create()");
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_unicast_slave_table_stack_create(
                unit, stack_p->stack_id))) {
            PTP_ERROR_FUNC("_bcm_ptp_unicast_slave_table_stack_create()");
            goto done;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_acceptable_master_table_stack_create(
                unit, stack_p->stack_id))) {
            PTP_ERROR_FUNC("_bcm_ptp_acceptable_master_table_stack_create()");
            goto done;
        }

        /* TODO: set proper debug configuration */
        _bcm_ptp_system_debug_configure(unit, stack_p->stack_id, 0xffffffff);

done:
        BCM_IF_ERROR_RETURN(bcm_esw_ptp_unlock(unit));
        soc_cm_print("PTP stack created\n");
    } else {
        rv = BCM_E_UNAVAIL;
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "PTP not supported on this unit"));
    }

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_system_debug_configure
 * Purpose:
 *      Configure the type of debug output produced by the ToP firmware.
 * Parameters:
 *      unit      - (IN) PTP unit
 *      flags     - (IN) debug flags
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
_bcm_ptp_system_debug_configure(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    uint32 flags)
{
    _bcm_ptp_info_t *ptp_info_p;
    _bcm_ptp_stack_info_t *stack_p;

    SET_PTP_INFO;
    if (!ptp_info_p->initialized) {
        return BCM_E_UNAVAIL;
    }
    stack_p = &ptp_info_p->stack_info[ptp_id];
   
#if defined(PTP_KEYSTONE_STACK)
    esw_set_ext_stack_config_uint32(stack_p, CONFIG_RCPU_SIZE + CONFIG_VLAN_SIZE + CONFIG_MPLS_SIZE + CONFIG_HOST_SIZE, flags);
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_ptp_time_format_set
 * Purpose:
 *      Set Time Of Day format for PTP stack instance.
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_info - (IN) PTP Stack ID
 *      format - (IN) Time of Day format
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_time_format_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    bcm_ptp_time_type_t type)
{
    int result = BCM_E_UNAVAIL;

    if (soc_feature(unit, soc_feature_ptp)) {
        bcm_esw_ptp_lock(unit);

#if defined(BCM_KATANA_SUPPORT)
        if (SOC_IS_KATANA(unit)) {
            result = bcm_kt_ptp_time_format_set(unit, ptp_id, type);
        }
#endif

        BCM_IF_ERROR_RETURN(bcm_esw_ptp_unlock(unit));
    }

    return result;
}

/*
 * Function:
 *      bcm_esw_ptp_cb_register
 * Purpose:
 *      Register a callback for handling PTP events
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb_types - (IN) The set of PTP callbacks types for which the specified callback should be called
 *      cb - (IN) A pointer to the callback function to call for the specified PTP events
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_cb_register(
    int unit, 
    bcm_ptp_cb_types_t cb_types, 
    bcm_ptp_cb cb, 
    void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    if (soc_feature(unit, soc_feature_ptp)) {
        bcm_esw_ptp_lock(unit);

#if defined(BCM_KATANA_SUPPORT)
        if (SOC_IS_KATANA(unit)) {
            rv = bcm_kt_ptp_cb_register(unit, cb_types, cb, user_data);
        }
#endif
        if (cb_types.w[bcmPTPCallbackTypeManagement]) {
            rv = _bcm_ptp_register_management_callback(unit, cb, user_data);
        }

        if (cb_types.w[bcmPTPCallbackTypeEvent]) {
            rv = _bcm_ptp_register_event_callback(unit, cb, user_data);
        }

        if (cb_types.w[bcmPTPCallbackTypeFault]) {
            rv = _bcm_ptp_register_fault_callback(unit, cb, user_data);
        }

        if (cb_types.w[bcmPTPCallbackTypeSignal]) {
            rv = _bcm_ptp_register_signal_callback(unit, cb, user_data);
        }

        BCM_IF_ERROR_RETURN(bcm_esw_ptp_unlock(unit));
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_cb_unregister
 * Purpose:
 *      Unregister a callback for handling PTP events
 * Parameters:
 *      unit - (IN) Unit number.
 *      event_types - (IN) The set of PTP events to unregister for the specified callback
 *      cb - (IN) A pointer to the callback function to unregister from the specified PTP events
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_cb_unregister(
    int unit, 
    bcm_ptp_cb_types_t cb_types, 
    bcm_ptp_cb cb)
{
    int rv = BCM_E_UNAVAIL;

    if (soc_feature(unit, soc_feature_ptp)) {
        bcm_esw_ptp_lock(unit);

#if defined(BCM_KATANA_SUPPORT)
        if (SOC_IS_KATANA(unit)) {
            rv = bcm_kt_ptp_cb_unregister(unit, cb_types, cb);
        }
#endif

        if (cb_types.w[bcmPTPCallbackTypeManagement]) {
            rv = _bcm_ptp_unregister_management_callback(unit);
        }

        if (cb_types.w[bcmPTPCallbackTypeEvent]) {
            rv = _bcm_ptp_unregister_event_callback(unit);
        }

        if (cb_types.w[bcmPTPCallbackTypeFault]) {
            rv = _bcm_ptp_unregister_fault_callback(unit);
        }

        if (cb_types.w[bcmPTPCallbackTypeSignal]) {
            rv = _bcm_ptp_unregister_signal_callback(unit);
        }

        BCM_IF_ERROR_RETURN(bcm_esw_ptp_unlock(unit));
    }

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_clock_cache_init
 * Purpose:
 *      Initialize the PTP clock information caches a unit.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_clock_cache_init(
    int unit)
{
    int rv = BCM_E_UNAVAIL;
    
    _bcm_ptp_stack_cache_t *stack_p;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");  
        return rv;
    }
        
    /* TODO: Fully implement logic to handle repeated initialization. */
    stack_p = sal_alloc(PTP_MAX_STACKS_PER_UNIT*
                        sizeof(_bcm_ptp_stack_cache_t),"Unit PTP Clock Information Caches");

    if (!stack_p) {
        _bcm_esw_ptp_unit_array[unit].memstate = PTP_MEMSTATE_FAILURE;
        return BCM_E_MEMORY;
    }

    _bcm_esw_ptp_unit_array[unit].stack_array = stack_p;
    _bcm_esw_ptp_unit_array[unit].memstate = PTP_MEMSTATE_INITIALIZED;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_clock_cache_stack_create
 * Purpose:
 *      Create the PTP clock information caches of a PTP stack.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      ptp_id  - (IN) PTP stack ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_clock_cache_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id)
{
    int rv = BCM_E_UNAVAIL;
    
    _bcm_ptp_clock_cache_t *clock_p;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");  
        return rv;
    }
        
    if (_bcm_esw_ptp_unit_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) {
        return BCM_E_UNAVAIL;
    }

    /* TODO: Fully implement logic to handle repeated creation. */
    clock_p = sal_alloc(PTP_MAX_CLOCK_INSTANCES*sizeof(_bcm_ptp_clock_cache_t),
                        "Stack PTP Information Caches");

    if (!clock_p) {
        _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].memstate = 
            PTP_MEMSTATE_FAILURE;
        return BCM_E_MEMORY;
    }

    _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].clock_array = clock_p;
    _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].memstate = 
        PTP_MEMSTATE_INITIALIZED;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_clock_cache_info_create
 * Purpose:
 *      Create the information cache of a PTP clock.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_clock_cache_info_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num)
{
    int rv = BCM_E_UNAVAIL;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");  
        return rv;
    }
        
    if ((_bcm_esw_ptp_unit_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
        (_bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    /* 
     * TODO: Assign application-relevant default PTP clock and port 
     *       information. 
     */
    _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].clock_array[clock_num] = 
        cache_default;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_clock_cache_info_get
 * Purpose:
 *      Get the clock information cache of a PTP clock.
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      ptp_id     - (IN)  PTP stack ID.
 *      clock_num  - (IN)  PTP clock number.
 *      clock_info - (OUT) PTP clock information cache.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int _bcm_ptp_clock_cache_info_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    bcm_ptp_clock_info_t *clock_info)
{
    int rv = BCM_E_UNAVAIL;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");  
        return rv;
    }
        
    if ((_bcm_esw_ptp_unit_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
        (_bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }        

    *clock_info = _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id]
                                  .clock_array[clock_num].clock_info;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_clock_cache_info_set
 * Purpose:
 *      Set the clock information cache of a PTP clock.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_info - (IN) PTP clock information cache.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int _bcm_ptp_clock_cache_info_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    const bcm_ptp_clock_info_t clock_info)
{
    int rv = BCM_E_UNAVAIL;
         
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");  
        return rv;
    }
        
    if ((_bcm_esw_ptp_unit_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
        (_bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }        

    _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id]
                    .clock_array[clock_num].clock_info = clock_info;
    
    return rv;
}

int _bcm_ptp_clock_cache_signal_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    int signal_num,
    bcm_ptp_signal_output_t *signal)
{
    int rv = BCM_E_UNAVAIL;
    
    if (signal_num >= PTP_MAX_OUTPUT_SIGNALS || signal_num < 0) {
        return BCM_E_PARAM;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");  
        return rv;
    }
        
    if ((_bcm_esw_ptp_unit_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
        (_bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }        

    *signal = _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].clock_array[clock_num].signal_output[signal_num];
    
    return rv;
}


int _bcm_ptp_clock_cache_signal_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    int signal_num,
    const bcm_ptp_signal_output_t *signal)
{
    int rv = BCM_E_UNAVAIL;
    
    if (signal_num >= PTP_MAX_OUTPUT_SIGNALS || signal_num < 0) {
        return BCM_E_PARAM;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");  
        return rv;
    }
        
    if ((_bcm_esw_ptp_unit_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
        (_bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }        

    _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].clock_array[clock_num].signal_output[signal_num] = *signal;
    
    return rv;
}

#endif /* defined(INCLUDE_PTP)*/
