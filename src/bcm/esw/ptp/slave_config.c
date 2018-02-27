/*
 * $Id: slave_config.c 1.1.2.4 Broadcom SDK $
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

#include <bcm/ptp.h>
#include <bcm/error.h>
#include <bcm_int/esw/ptp.h>

#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/katana.h>
#endif

/* PTP clock unicast slave per-port table. */
typedef struct _bcm_ptp_unicast_slave_table_s {
    uint16 num_entries;
    uint32 clock_identity_counter;
    bcm_ptp_clock_peer_t entry[PTP_MAX_UNICAST_SLAVE_TABLE_ENTRIES];
} _bcm_ptp_unicast_slave_table_t;

/* PTP stack unicast slave table array. */
typedef struct _bcm_ptp_stack_ucs_array_s {
    _bcm_ptp_memstate_t memstate;
    _bcm_ptp_unicast_slave_table_t *table;
} _bcm_ptp_stack_ucs_array_t;

/* Unit unicast slave table array(s). */
typedef struct _bcm_ptp_unit_ucs_array_s {
    _bcm_ptp_memstate_t memstate;
    _bcm_ptp_stack_ucs_array_t *stack_array;
} _bcm_ptp_unit_ucs_array_t;

/* 
 * PTP clock acceptable master clock. 
 * Protocol address and alternate Priority1 for an acceptable master table entry.
 * Ref. IEEE Std. 1588-2008, Chapter 17.6.
 */
typedef struct _bcm_ptp_acceptable_master_s {
    bcm_ptp_clock_peer_address_t address;
    uint8 alt_priority1;
} _bcm_ptp_acceptable_master_t;

/* PTP clock acceptable master table. */
typedef struct _bcm_ptp_acceptable_master_table_s {
    uint16 num_entries;
    _bcm_ptp_acceptable_master_t entry[PTP_MAX_ACCEPTABLE_MASTER_TABLE_ENTRIES];
} _bcm_ptp_acceptable_master_table_t;

/* PTP stack acceptable master table array. */
typedef struct _bcm_ptp_stack_amt_array_s {
    _bcm_ptp_memstate_t memstate;
    _bcm_ptp_acceptable_master_table_t *table;
} _bcm_ptp_stack_amt_array_t;

/* Unit acceptable master table array(s). */
typedef struct _bcm_ptp_unit_amt_array_s {
    _bcm_ptp_memstate_t memstate;
    _bcm_ptp_stack_amt_array_t *stack_array;
} _bcm_ptp_unit_amt_array_t;

static const bcm_ptp_clock_peer_t ucs_entry_default;
static const _bcm_ptp_unicast_slave_table_t ucs_default;
static _bcm_ptp_unit_ucs_array_t unit_ucs_array[BCM_MAX_NUM_UNITS];

static const _bcm_ptp_acceptable_master_t amt_entry_default;
static const _bcm_ptp_acceptable_master_table_t amt_default;
static _bcm_ptp_unit_amt_array_t unit_amt_array[BCM_MAX_NUM_UNITS];

static int _bcm_ptp_add_unicast_slave_announce(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info);

static int _bcm_ptp_add_unicast_slave_sync(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info);

static int _bcm_ptp_add_unicast_slave_delay_response(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info);

static int 
_bcm_ptp_add_unicast_slave_peer_delay_response(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info);

static int _bcm_ptp_remove_unicast_slave_announce(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info);

static int _bcm_ptp_remove_unicast_slave_sync(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info);

static int _bcm_ptp_remove_unicast_slave_delay_response(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info);

static int _bcm_ptp_remove_unicast_slave_peer_delay_response(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info);

static int _bcm_ptp_acceptable_master_table_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num);

/*
 * Function:
 *      _bcm_ptp_unicast_slave_table_init
 * Purpose:
 *      Initialize the unicast slave table arrays of a unit.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_unicast_slave_table_init(
    int unit)
{
    int rv = BCM_E_UNAVAIL;
     
    _bcm_ptp_stack_ucs_array_t *stack_p;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    /* TODO: Fully implement logic to handle repeated initialization. */
    stack_p = sal_alloc(PTP_MAX_STACKS_PER_UNIT*
                        sizeof(_bcm_ptp_stack_ucs_array_t),"Unit UCS arrays");

    if (!stack_p) {
        unit_ucs_array[unit].memstate = PTP_MEMSTATE_FAILURE;
        return BCM_E_MEMORY;
    }

    unit_ucs_array[unit].stack_array = stack_p;
    unit_ucs_array[unit].memstate = PTP_MEMSTATE_INITIALIZED;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_unicast_slave_table_stack_create
 * Purpose:
 *      Create the unicast slave table array of a PTP stack.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      ptp_id  - (IN) PTP stack ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_unicast_slave_table_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id)
{
    int rv = BCM_E_UNAVAIL;
     
    _bcm_ptp_unicast_slave_table_t *table_p;
    int i = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    if (unit_ucs_array[unit].memstate != PTP_MEMSTATE_INITIALIZED)
    {
        return BCM_E_UNAVAIL;
    }

    /* TODO: Fully implement logic to handle repeated creation. */
    table_p = sal_alloc(PTP_MAX_CLOCK_INSTANCES*
                        PTP_MAX_CLOCK_INSTANCE_PORTS*
                        sizeof(_bcm_ptp_unicast_slave_table_t),
                        "PTP stack UCS array");

    if (!table_p) {
        unit_ucs_array[unit].stack_array[ptp_id].memstate = 
            PTP_MEMSTATE_FAILURE;
        return BCM_E_MEMORY;
    }

    unit_ucs_array[unit].stack_array[ptp_id].table = table_p;
    unit_ucs_array[unit].stack_array[ptp_id].memstate = 
        PTP_MEMSTATE_INITIALIZED;

    /* Set number of entries. */
    for (i = 0; i < (PTP_MAX_CLOCK_INSTANCES*PTP_MAX_CLOCK_INSTANCE_PORTS); ++i) {
        unit_ucs_array[unit].stack_array[ptp_id].table[i].num_entries = 0;
    }
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_unicast_slave_table_clock_create
 * Purpose:
 *      Create the unicast slave table array of a PTP clock.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_unicast_slave_table_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num)
{
    int rv = BCM_E_UNAVAIL;
         
    int i = 0;
    int k = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    if ((unit_ucs_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_ucs_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    i = clock_num*PTP_MAX_CLOCK_INSTANCE_PORTS;
    for (k = 0; k < PTP_MAX_CLOCK_INSTANCE_PORTS; ++k) {
        unit_ucs_array[unit].stack_array[ptp_id].table[i+k] = ucs_default;
    }
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_acceptable_master_table_init
 * Purpose:
 *      Initialize the acceptable master table arrays of a unit.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_acceptable_master_table_init(
    int unit)
{
    int rv = BCM_E_UNAVAIL;
     
    _bcm_ptp_stack_amt_array_t *stack_p;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    /* TODO: Fully implement logic to handle repeated initialization. */
    stack_p = sal_alloc(PTP_MAX_STACKS_PER_UNIT*
                        sizeof(_bcm_ptp_stack_amt_array_t),"Unit AMT arrays");

    if (!stack_p) {
        unit_amt_array[unit].memstate = PTP_MEMSTATE_FAILURE;
        return BCM_E_MEMORY;
    }

    unit_amt_array[unit].stack_array = stack_p;
    unit_amt_array[unit].memstate = PTP_MEMSTATE_INITIALIZED;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_acceptable_master_table_stack_create
 * Purpose:
 *      Create the acceptable master table array of a PTP stack.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      ptp_id  - (IN) PTP stack ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_acceptable_master_table_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id)
{
    int rv = BCM_E_UNAVAIL;
     
    _bcm_ptp_acceptable_master_table_t *table_p;
    int i = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
      
    if (unit_amt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED)
    {
        return BCM_E_UNAVAIL;
    }

    /* TODO: Fully implement logic to handle repeated creation. */
    table_p = sal_alloc(PTP_MAX_CLOCK_INSTANCES*
                        sizeof(_bcm_ptp_acceptable_master_table_t),
                        "PTP stack AMT array");

    if (!table_p) {
        unit_amt_array[unit].stack_array[ptp_id].memstate = 
            PTP_MEMSTATE_FAILURE;
        return BCM_E_MEMORY;
    }

    unit_amt_array[unit].stack_array[ptp_id].table = table_p;
    unit_amt_array[unit].stack_array[ptp_id].memstate = 
        PTP_MEMSTATE_INITIALIZED;

    /* Set number of entries. */
    for (i = 0; i < PTP_MAX_CLOCK_INSTANCES; ++i) {
        unit_amt_array[unit].stack_array[ptp_id].table[i].num_entries = 0;
    }        
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_acceptable_master_table_clock_create
 * Purpose:
 *      Create the acceptable master table array of a PTP clock.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_acceptable_master_table_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num)
{
    int rv = BCM_E_UNAVAIL;
             
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    if ((unit_amt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_amt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    unit_amt_array[unit].stack_array[ptp_id].table[clock_num] = amt_default;
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_unicast_slave_add
 * Purpose:
 *      Add a unicast slave to a PTP clock port.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      port_num   - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_unicast_slave_add(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    _bcm_ptp_unicast_slave_table_t *unicast_slave_table;
    int i = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            (uint32)port_num))) {
        return rv;   
    }
  
    if ((unit_ucs_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_ucs_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    i = clock_num*PTP_MAX_CLOCK_INSTANCE_PORTS + (port_num - 1);
    unicast_slave_table = &unit_ucs_array[unit].stack_array[ptp_id].table[i];

    i = 0;
    while (i < unicast_slave_table->num_entries) {
        if (_bcm_ptp_peer_address_compare(&slave_info->peer_address,
                &unicast_slave_table->entry[i].peer_address)) {
            break;
        }
        ++i;
    }

    if (i >= PTP_MAX_UNICAST_SLAVE_TABLE_ENTRIES) {
        return BCM_E_FULL; 
    }

    if (i == unicast_slave_table->num_entries) {
        /* New entry. */
        ++unicast_slave_table->num_entries;
    }
    else
    {
        /* Replacement entry. */

        /* Cancel unicast transmission services to slave. */
        if (BCM_FAILURE(rv = _bcm_ptp_remove_unicast_slave_announce(
                unit, ptp_id, clock_num, (uint32)port_num,
                &unicast_slave_table->entry[i]))) {
            return rv;
        }

        if (BCM_FAILURE(rv = _bcm_ptp_remove_unicast_slave_sync(
                unit, ptp_id, clock_num, (uint32)port_num,
                &unicast_slave_table->entry[i]))) {
            return rv;
        }

        switch (unicast_slave_table->entry[i].delay_mechanism) {
        case (bcmPTPDelayMechanismEnd2End):
            rv = _bcm_ptp_remove_unicast_slave_delay_response(
                    unit, ptp_id, clock_num, (uint32)port_num,
                    &unicast_slave_table->entry[i]);
            break;

        case (bcmPTPDelayMechanismPeer2Peer):
            rv = _bcm_ptp_remove_unicast_slave_peer_delay_response(
                    unit, ptp_id, clock_num, (uint32)port_num,
                    &unicast_slave_table->entry[i]);
            break;

        default:
            return BCM_E_UNAVAIL;
        }
    }

    /* Insert new entry. */
    unicast_slave_table->entry[i] = *slave_info;

    /* Request unicast transmission services. */
    if (BCM_FAILURE(rv = _bcm_ptp_add_unicast_slave_announce(
            unit, ptp_id, clock_num, (uint32)port_num, slave_info))) {
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_add_unicast_slave_sync(
            unit, ptp_id, clock_num, (uint32)port_num, slave_info))) {
        return rv;
    }

    switch (unicast_slave_table->entry[i].delay_mechanism) {
    case (bcmPTPDelayMechanismEnd2End):
        rv = _bcm_ptp_add_unicast_slave_delay_response(
                unit, ptp_id, clock_num, (uint32)port_num,
                slave_info);
        break;

    case (bcmPTPDelayMechanismPeer2Peer):
        rv = _bcm_ptp_add_unicast_slave_peer_delay_response(
                unit, ptp_id, clock_num, (uint32)port_num,
                slave_info);
        break;

    default:
        return BCM_E_UNAVAIL;
    }
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_unicast_slave_list
 * Purpose:
 *      List the unicast slaves of a PTP clock port.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      port_num   - (IN) PTP clock port number.
 *      num_slaves - (OUT) Number of unicast slave table entries.
 *      slave_info - (OUT) Unicast slave table.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_unicast_slave_list(
    int unit,
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num, 
    int *num_slaves, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_NONE;
    
    _bcm_ptp_unicast_slave_table_t *unicast_slave_table;
    int i = 0;
    
    *num_slaves = 0;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            (uint32)port_num))) {
        return rv;   
    }
  
    if ((unit_ucs_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_ucs_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    i = clock_num*PTP_MAX_CLOCK_INSTANCE_PORTS + (port_num - 1);
    unicast_slave_table = &unit_ucs_array[unit].stack_array[ptp_id].table[i];

    if (unicast_slave_table->num_entries == 0) {
        *num_slaves = 0;
        return rv;
    }

    /* 
     * Get PTP clock unicast slave table.
     * NOTICE: Returns a local, SDK copy of unicast slave table. 
     */
    *num_slaves = unicast_slave_table->num_entries;

    for (i = 0; i < (*num_slaves); ++i) {
        slave_info[i] = unicast_slave_table->entry[i];
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_unicast_slave_remove
 * Purpose:
 *      Remove a unicast slave of a PTP clock port.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      port_num   - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_unicast_slave_remove(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    _bcm_ptp_unicast_slave_table_t *unicast_slave_table;
    int i = 0;
    int k = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            (uint32)port_num))) {
        return rv;   
    }
        
    if ((unit_ucs_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_ucs_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    i = clock_num*PTP_MAX_CLOCK_INSTANCE_PORTS + (port_num - 1);
    unicast_slave_table = &unit_ucs_array[unit].stack_array[ptp_id].table[i];

    if (unicast_slave_table->num_entries == 0) {
        return BCM_E_EMPTY;
    }

    i = 0; 
    k = 0;
    while (i < unicast_slave_table->num_entries) {
        if (_bcm_ptp_peer_address_compare(&slave_info->peer_address,
                &unicast_slave_table->entry[i].peer_address)) {
            /* 
             * Remove entry.
             * Cancel unicast transmission services to slave.
             */
            if (BCM_FAILURE(rv = _bcm_ptp_remove_unicast_slave_announce(
                    unit, ptp_id, clock_num, (uint32)port_num, 
                    &unicast_slave_table->entry[i]))) {
                return rv;
            }

            if (BCM_FAILURE(rv = _bcm_ptp_remove_unicast_slave_sync(
                    unit, ptp_id, clock_num, (uint32)port_num,
                    &unicast_slave_table->entry[i]))) {
                return rv;
            }

            switch (unicast_slave_table->entry[i].delay_mechanism) {
            case (bcmPTPDelayMechanismEnd2End):
                rv = _bcm_ptp_remove_unicast_slave_delay_response(
                        unit, ptp_id, clock_num, (uint32)port_num,
                        &unicast_slave_table->entry[i]);
                break;

            case (bcmPTPDelayMechanismPeer2Peer):
                rv = _bcm_ptp_remove_unicast_slave_peer_delay_response(
                        unit, ptp_id, clock_num, (uint32)port_num,
                        &unicast_slave_table->entry[i]);
                break;

            default:
                return BCM_E_UNAVAIL;
            }

            /* 
             * Remove this entry by moving last entry into this slot, and 
             * decrementing number of entries. 
             */
            unicast_slave_table->entry[i] = 
                unicast_slave_table->entry[--unicast_slave_table->num_entries];

            /* Reset unused entry. */
            unicast_slave_table->entry[unicast_slave_table->num_entries] =
                ucs_entry_default;

            ++k;
        } 
        else 
        {
            ++i;
        }
    }

    if (k <= 0) {
        /* No matching peer address(es) in unicast slave table. */
        return BCM_E_NOT_FOUND; 
    }
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_unicast_slave_clear
 * Purpose:
 *      Clear (i.e. remove all) unicast slaves of a PTP clock port.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 *      port_num  - (IN) PTP clock port number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_unicast_slave_clear(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num)
{
    int rv = BCM_E_UNAVAIL;
        
    _bcm_ptp_unicast_slave_table_t *unicast_slave_table;
    int i = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            (uint32)port_num))) {
        return rv;   
    }
        
    if ((unit_ucs_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_ucs_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    i = clock_num*PTP_MAX_CLOCK_INSTANCE_PORTS + (port_num - 1);
    unicast_slave_table = &unit_ucs_array[unit].stack_array[ptp_id].table[i];

    /* 
     * Remove all unicast slaves.
     * NOTICE: Pointer to beginning of unicast slave table causes removal function to
     *         recursively collapse table.
     */
    if (BCM_FAILURE(rv = bcm_esw_ptp_unicast_slave_remove(unit, ptp_id, clock_num, 
            port_num, unicast_slave_table->entry))) {
        return rv;
    }

    i = clock_num*PTP_MAX_CLOCK_INSTANCE_PORTS + (port_num - 1);
    unit_ucs_array[unit].stack_array[ptp_id].table[i] = ucs_default;
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_acceptable_master_table_size_get
 * Purpose:
 *      Get maximum number of acceptable master table entries of a PTP clock.
 * Parameters:
 *      unit              - (IN)  Unit number.
 *      ptp_id            - (IN)  PTP stack ID.
 *      clock_num         - (IN)  PTP clock number.
 *      port_num          - (IN)  PTP clock port number.
 *      max_table_entries - (OUT) Maximum number of acceptable master table entries.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Ref. IEEE Std. 1588-2008, Chapter 17.6.4.
 */
int 
bcm_esw_ptp_acceptable_master_table_size_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num, 
    int *max_table_entries)
{
    int rv = BCM_E_UNAVAIL;
    
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS; 

    bcm_ptp_port_identity_t portid;     
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    if ((unit_amt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_amt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    if (BCM_FAILURE(rv = bcm_esw_ptp_clock_port_identity_get(unit, ptp_id, clock_num, 
            PTP_IEEE1588_ALL_PORTS, &portid))) {
        return rv;
    }

    rv = _bcm_ptp_management_message_send(unit, ptp_id, clock_num, &portid, 
            PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_ACCEPTABLE_MASTER_MAX_TABLE_SIZE, 
            0, 0, resp, &resp_len);

    if (rv == BCM_E_NONE) {
        /* 
         * Parse response.
         * Octet 0...1: Acceptable Master table maximum size.
         */
        *max_table_entries= _bcm_ptp_uint16_read(resp);        
    }
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_acceptable_master_table_add
 * Purpose:
 *      Add an entry to the acceptable master table of a PTP clock.
 * Parameters:
 *      unit          - (IN) Unit number.
 *      ptp_id        - (IN) PTP stack ID.
 *      clock_num     - (IN) PTP clock number.
 *      port_num      - (IN) PTP clock port number.
 *      alt_priority1 - (IN) Alternate priority1.
 *      master_info   - (IN) Acceptable master address.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_acceptable_master_table_add(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num, 
    int alt_priority1,
    bcm_ptp_clock_peer_address_t *master_info)
{
    int rv = BCM_E_UNAVAIL;
    
    _bcm_ptp_acceptable_master_table_t *acceptable_master_table;
    int i = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    if ((unit_amt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_amt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    acceptable_master_table = &unit_amt_array[unit].stack_array[ptp_id]
                                                   .table[clock_num];

    i = 0;
    while (i < acceptable_master_table->num_entries) {
        if (_bcm_ptp_peer_address_compare(master_info,
                &acceptable_master_table->entry[i].address)) {
            break;
        }
        ++i;
    }

    if (i >= PTP_MAX_ACCEPTABLE_MASTER_TABLE_ENTRIES) {
        return BCM_E_FULL; 
    }

    if (i == acceptable_master_table->num_entries) {
        /* New entry. */
        ++acceptable_master_table->num_entries;
    }

    acceptable_master_table->entry[i].address = 
        *master_info;
    acceptable_master_table->entry[i].alt_priority1 = 
        (uint8)alt_priority1;

    /* Update PTP stack firmware acceptable master table. */
    rv = _bcm_ptp_acceptable_master_table_set(unit, ptp_id, clock_num);
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_acceptable_master_table_list
 * Purpose:
 *      List the acceptable master table of a PTP clock.
 * Parameters:
 *      unit        - (IN)  Unit number.
 *      ptp_id      - (IN)  PTP stack ID.
 *      clock_num   - (IN)  PTP clock number.
 *      port_num    - (IN)  PTP clock port number.
 *      num_masters - (OUT) Number of acceptable master table entries.
 *      master_info - (OUT) Acceptable master table.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_acceptable_master_table_list(
    int unit,
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num, 
    int *num_masters, 
    bcm_ptp_clock_peer_address_t *master_info)
{
    int rv = BCM_E_NONE;
    
    _bcm_ptp_acceptable_master_table_t *acceptable_master_table;
    int i = 0;
    
    *num_masters = 0;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    if ((unit_amt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_amt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    acceptable_master_table = &unit_amt_array[unit].stack_array[ptp_id]
                                                   .table[clock_num];

    if (acceptable_master_table->num_entries == 0) {
        *num_masters = 0;
        return rv;
    }

    /* 
     * Get PTP clock acceptable master table.
     * NOTICE: Returns a local, SDK copy of acceptable master table 
     *         rather than query PTP stack firmware.
     */
    *num_masters = acceptable_master_table->num_entries;

    for (i = 0; i < (*num_masters); ++i) {
        master_info[i] = acceptable_master_table->entry[i].address;
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_acceptable_master_table_remove
 * Purpose:
 *      Remove an entry from the acceptable master table of a PTP clock.
 * Parameters:
 *      unit        - (IN) Unit number.
 *      ptp_id      - (IN) PTP stack ID.
 *      clock_num   - (IN) PTP clock number.
 *      port_num    - (IN) PTP clock port number.
 *      master_info - (IN) Acceptable master address.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_acceptable_master_table_remove(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num, 
    bcm_ptp_clock_peer_address_t *master_info)
{
    int rv = BCM_E_UNAVAIL;
    
    _bcm_ptp_acceptable_master_table_t *acceptable_master_table;
    int i = 0;
    int k = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    if ((unit_amt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_amt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    acceptable_master_table = &unit_amt_array[unit].stack_array[ptp_id]
                                                   .table[clock_num];

    if (acceptable_master_table->num_entries == 0) {
        return BCM_E_EMPTY;
    }

    i = 0; 
    k = 0;
    while (i < acceptable_master_table->num_entries) {
        if (_bcm_ptp_peer_address_compare(master_info,
                &acceptable_master_table->entry[i].address)) {
            /* 
             * Remove this entry by moving last entry into this slot, and 
             * decrementing number of entries. 
             */
            acceptable_master_table->entry[i] = 
                acceptable_master_table->entry[--acceptable_master_table->num_entries];

            /* Reset unused entry. */
            acceptable_master_table->entry[acceptable_master_table->num_entries] =
                amt_entry_default;

            ++k;
        } 
        else 
        {
            ++i;
        }
    }

    if (k <= 0) {
        /* No matching peer address(es) in acceptable master table. */
        return BCM_E_NOT_FOUND; 
    }

    /* Update PTP stack firmware acceptable master table. */
    rv = _bcm_ptp_acceptable_master_table_set(unit, ptp_id, clock_num);
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_acceptable_master_table_clear
 * Purpose:
 *      Clear (i.e. remove all entries from) the acceptable master table 
 *      of a PTP clock.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 *      port_num  - (IN) PTP clock port number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_acceptable_master_table_clear(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num)
{
    int rv = BCM_E_UNAVAIL;
        
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    if ((unit_amt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_amt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    unit_amt_array[unit].stack_array[ptp_id].table[clock_num] = amt_default;

    /* Update PTP stack firmware acceptable master table. */
    rv = _bcm_ptp_acceptable_master_table_set(unit, ptp_id, clock_num);
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_acceptable_master_table_enabled_get
 * Purpose:
 *      Get acceptable master table enabled Boolean of a PTP clock port.
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      ptp_id    - (IN)  PTP stack ID.
 *      clock_num - (IN)  PTP clock number.
 *      port_num  - (IN)  PTP clock port number.
 *      enabled   - (OUT) Acceptable master table enabled Boolean.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Ref. IEEE Std. 1588-2008, Chapter 17.6.5. 
 */
int 
bcm_esw_ptp_acceptable_master_table_enabled_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num, 
    uint8 *enabled)
{
    int rv = BCM_E_UNAVAIL;
    
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS; 
    
    bcm_ptp_port_identity_t portid;     
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            (uint32)port_num))) {
        return rv;   
    }

    if (BCM_FAILURE(rv = bcm_esw_ptp_clock_port_identity_get(unit, ptp_id, 
            clock_num, port_num, &portid))) {
        return rv;
    }

    rv = _bcm_ptp_management_message_send(unit, ptp_id, clock_num, &portid, 
            PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_ACCEPTABLE_MASTER_TABLE_ENABLED, 
            0, 0, resp, &resp_len);

    if (rv == BCM_E_NONE) {
        /* 
         * Parse response.
         *    Octet 0 : Acceptable Master table enabled (EN) Boolean. 
         *              |B7|B6|B5|B4|B3|B2|B1|B0| = |0|0|0|0|0|0|0|EN|.
         *    Octet 1 : Reserved.
         */
        *enabled = (0x01 & resp[0]);
    }
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_acceptable_master_table_enabled_set
 * Purpose:
 *      Set acceptable master table enabled Boolean of a PTP clock port.
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      ptp_id    - (IN)  PTP stack ID.
 *      clock_num - (IN)  PTP clock number.
 *      port_num  - (IN)  PTP clock port number.
 *      enabled   - (OUT) Acceptable master table enabled Boolean.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Ref. IEEE Std. 1588-2008, Chapter 17.6.5. 
 */
int 
bcm_esw_ptp_acceptable_master_table_enabled_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int port_num, 
    uint8 enabled)
{
    int rv = BCM_E_UNAVAIL;
    
    uint8 payload[PTP_MGMTMSG_PAYLOAD_MIN_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS; 

    bcm_ptp_port_identity_t portid;     
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            (uint32)port_num))) {
        return rv;   
    }
  
    if (BCM_FAILURE(rv = bcm_esw_ptp_clock_port_identity_get(unit, ptp_id, 
            clock_num, port_num, &portid))) {
        return rv;
    }

    /* 
     * Make payload.
     *    Octet 0 : Acceptable Master table enabled (EN) Boolean. 
     *              |B7|B6|B5|B4|B3|B2|B1|B0| = |0|0|0|0|0|0|0|EN|.
     *    Octet 1 : Reserved.
     */
    payload[0] = (uint8)(1 & enabled);

    rv = _bcm_ptp_management_message_send(unit, ptp_id, clock_num, &portid, 
            PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_ACCEPTABLE_MASTER_TABLE_ENABLED, 
            payload, PTP_MGMTMSG_PAYLOAD_MIN_SIZE_OCTETS, 
            resp, &resp_len);
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_acceptable_master_table_set
 * Purpose:
 *      Set acceptable master table of a PTP clock.
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      ptp_id     - (IN)  PTP stack ID.
 *      clock_num  - (IN)  PTP clock number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_acceptable_master_table_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num)
{
    int rv = BCM_E_UNAVAIL;
    
    uint8 payload[PTP_MGMTMSG_PAYLOAD_MAX_ACCEPTABLE_MASTER_TABLE_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS; 
    
    _bcm_ptp_acceptable_master_table_t *acceptable_master_table;
    bcm_ptp_port_identity_t portid;     
    int i = 0;
    int k = 0;
    int el = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
  
    if ((unit_amt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_amt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }

    if (BCM_FAILURE(rv = bcm_esw_ptp_clock_port_identity_get(unit, ptp_id, 
            clock_num, PTP_IEEE1588_ALL_PORTS, &portid))) {
        return rv;
    }

    acceptable_master_table = &unit_amt_array[unit].stack_array[ptp_id]
                                                   .table[clock_num];
        
    /* 
     * Make payload.
     *    Octet 0...1 : Number of acceptable master table entries.
     *    Octet 2...* : Acceptable master table entries.
     *    CONDITIONAL : PTP management message zero-pad. 
     *                  See IEEE Std. 1588-2008 ACCEPTABLE_MASTER_TABLE 
     *                  management message payload format reqm'ts.
     */
    i = 0;
    _bcm_ptp_uint16_write(payload, acceptable_master_table->num_entries);  
    i += sizeof(uint16);

    for (el = 0; el < acceptable_master_table->num_entries; ++el) {
        _bcm_ptp_uint16_write(&payload[i], 
            acceptable_master_table->entry[el].address.addr_type);  
        i += sizeof(uint16);

        switch (acceptable_master_table->entry[el].address.addr_type) {
        case bcmPTPUDPIPv4:
            /* Ethernet/UDP/IPv4 address type. */
            _bcm_ptp_uint16_write(&payload[i], PTP_IPV4_ADDR_SIZE_BYTES);  
            i += sizeof(uint16);
            _bcm_ptp_uint32_write(&payload[i], 
                acceptable_master_table->entry[el].address.ipv4_addr);  
            i += sizeof(uint32);

            break;

        case bcmPTPUDPIPv6:
            /* Ethernet/UDP/IPv6 address type. */
            _bcm_ptp_uint16_write(&payload[i], PTP_IPV6_ADDR_SIZE_BYTES);  
            i += sizeof(uint16);

            for (k = 0; k < PTP_IPV6_ADDR_SIZE_BYTES; ++k) {
                payload[i++] = acceptable_master_table->entry[el]
                               .address.ipv6_addr[k];
            }

            break;

        default:
            _bcm_ptp_uint16_write(&payload[i], 0);  
            i += sizeof(uint16);
        }            

        payload[i++]= acceptable_master_table->entry[el].alt_priority1;

    }

    /* 
     * Payload padding.
     * NOTICE: IEEE Std. 1588-2008 ACCEPTABLE_MASTER_TABLE PTP management 
     *         message expects even number of octets.
     */
    if (i % 2) {
        payload[i++] = 0x00; 
    }

    rv = _bcm_ptp_management_message_send(unit, ptp_id, clock_num, &portid, 
            PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_ACCEPTABLE_MASTER_TABLE, 
            payload, i, resp, &resp_len);
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_add_unicast_slave_announce
 * Purpose:
 *      Request unicast transmission of PTP announce messages.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_add_unicast_slave_announce(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    rv = _bcm_ptp_unicast_slave_subscribe(unit, ptp_id, clock_num, clock_port,
                slave_info, _bcmPTPmessageType_announce,
                _bcmPTPtlvType_request_unicast_transmission,
                slave_info->log_announce_interval); 
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_add_unicast_slave_sync
 * Purpose:
 *      Request unicast transmission of PTP sync messages.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_add_unicast_slave_sync(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    rv = _bcm_ptp_unicast_slave_subscribe(unit, ptp_id, clock_num, clock_port,
                slave_info, _bcmPTPmessageType_sync,
                _bcmPTPtlvType_request_unicast_transmission,
                slave_info->log_sync_interval); 
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_add_unicast_slave_delay_response
 * Purpose:
 *      Request unicast transmission of PTP delay response messages.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_add_unicast_slave_delay_response(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    rv = _bcm_ptp_unicast_slave_subscribe(unit, ptp_id, clock_num, clock_port,
                slave_info, _bcmPTPmessageType_delay_resp,
                _bcmPTPtlvType_request_unicast_transmission,
                slave_info->log_delay_request_interval); 
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_add_unicast_slave_peer_delay_response
 * Purpose:
 *      Request unicast transmission of PTP peer delay response messages.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_add_unicast_slave_peer_delay_response(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    rv = _bcm_ptp_unicast_slave_subscribe(unit, ptp_id, clock_num, clock_port,
                slave_info, _bcmPTPmessageType_pdelay_resp,
                _bcmPTPtlvType_request_unicast_transmission,
                slave_info->log_peer_delay_request_interval); 
   
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_remove_unicast_slave_announce
 * Purpose:
 *      Cancel unicast transmission of PTP announce messages.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_remove_unicast_slave_announce(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    rv = _bcm_ptp_unicast_slave_subscribe(unit, ptp_id, clock_num, clock_port,
                slave_info, _bcmPTPmessageType_announce,
                _bcmPTPtlvType_cancel_unicast_transmission,
                slave_info->log_announce_interval); 
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_remove_unicast_slave_sync
 * Purpose:
 *      Cancel unicast transmission of PTP sync messages.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_remove_unicast_slave_sync(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    rv = _bcm_ptp_unicast_slave_subscribe(unit, ptp_id, clock_num, clock_port,
                slave_info, _bcmPTPmessageType_sync,
                _bcmPTPtlvType_cancel_unicast_transmission,
                slave_info->log_sync_interval); 
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_remove_unicast_slave_delay_response
 * Purpose:
 *      Cancel unicast transmission of PTP delay response messages.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_remove_unicast_slave_delay_response(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    rv = _bcm_ptp_unicast_slave_subscribe(unit, ptp_id, clock_num, clock_port,
                slave_info, _bcmPTPmessageType_delay_resp,
                _bcmPTPtlvType_cancel_unicast_transmission,
                slave_info->log_delay_request_interval); 
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_remove_unicast_slave_peer_delay_response
 * Purpose:
 *      Cancel unicast transmission of PTP peer delay response messages.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_remove_unicast_slave_peer_delay_response(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info)
{
    int rv = BCM_E_UNAVAIL;
    
    rv = _bcm_ptp_unicast_slave_subscribe(unit, ptp_id, clock_num, clock_port,
                slave_info, _bcmPTPmessageType_pdelay_resp,
                _bcmPTPtlvType_cancel_unicast_transmission,
                slave_info->log_peer_delay_request_interval); 
            
    return rv;
}
#endif /* defined(INCLUDE_PTP)*/
