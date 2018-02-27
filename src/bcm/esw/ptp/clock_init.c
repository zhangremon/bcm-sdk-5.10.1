/*
 * $Id: clock_init.c 1.1.2.6 Broadcom SDK $
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
#include <bcm_int/esw/ptp.h>
#include <bcm/error.h>

#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/katana.h>
#endif

/*
 * Function:
 *      bcm_esw_ptp_clock_port_identity_get
 * Purpose:
 *      Get port identity of a PTP clock port.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      identity   - (OUT) PTP port identity.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_clock_port_identity_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_port_identity_t *identity)
{
    int rv = BCM_E_UNAVAIL;
    
    bcm_ptp_clock_info_t ci;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            clock_port))) {
        return rv;   
    }
        
    if (BCM_FAILURE(rv = 
            _bcm_ptp_clock_cache_info_get(unit, ptp_id, clock_num, &ci))) {
        return rv;
    }

    sal_memcpy(identity->clock_identity, 
               ci.clock_identity, 
               sizeof(bcm_ptp_clock_identity_t));
    
    identity->port_number = (uint16)clock_port; 
    
    return rv;
}

/*
 * Function:
 *      bcm_esw_ptp_clock_create
 * Purpose:
 *      Create a PTP clock.
 * Parameters:
 *      unit       - (IN)     Unit number.
 *      ptp_id     - (IN)     PTP stack ID.
 *      clock_info - (IN/OUT) PTP clock information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_ptp_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    bcm_ptp_clock_info_t *clock_info)
{
    int rv = BCM_E_UNAVAIL;
    
    uint8 payload[PTP_MGMTMSG_PAYLOAD_CLOCK_INSTANCE_SIZE_OCTETS]= {0};
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len= PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;
    int i= 0;
   
    _bcm_ptp_info_t *ptp_info_p;
    bcm_ptp_port_identity_t portid;
    
sal_sleep(1);

    if (soc_feature(unit, soc_feature_ptp)) {
        rv = BCM_E_NONE;

        bcm_esw_ptp_lock(unit);

        SET_PTP_INFO;

        if (!ptp_info_p->initialized) {
            return BCM_E_PARAM;
        }

        /* 
         * TODO: multiple clock support.  Create bitmap to represent clock_num usage 
         * TODO: Add flags to bcm_ptp_clock_info_t 
         */
#if 0
        if (clock_info->flags & BCM_PTP_CLOCK_WITH_ID) {
            if (clock_info->clock_num  > PTP_MAX_CLOCKS_PER_STACK) {
                return BCM_E_PARAM;
            }
            /* TODO: check in-use */
        }else {
            if (clock_info->clock_num  >= PTP_MAX_CLOCKS_PER_STACK) {
                return BCM_E_PARAM;
            }
        }
#else
        clock_info->clock_num = 0;
#endif
        
        /* Clock type and port number checks. */
        switch (clock_info->type) {
        case bcmPTPClockTypeOrdinary:
            if (clock_info->num_ports != 1) {
                /* Too few / too many ports. */
                return BCM_E_PARAM; 
            }
            break;
            
        case bcmPTPClockTypeBoundary:
            /* Fall through. */
        case bcmPTPClockTypeTransparent:
            if ((clock_info->num_ports < 1) ||
                    (clock_info->num_ports > PTP_MAX_CLOCK_INSTANCE_PORTS)) {
                /* Too few / too many ports. */
                return BCM_E_PARAM; 
            }
            break;
            
        default:
            /* Invalid/unknown clock type. */
            return BCM_E_UNAVAIL; 
        }
    
        /* Parameter extrema checking. */
        if ((clock_info->announce_receipt_timeout_minimum < 
                PTP_CLOCK_PRESETS_ANNOUNCE_RECEIPT_TIMEOUT_MINIMUM) ||
                (clock_info->announce_receipt_timeout_maximum > 
                PTP_CLOCK_PRESETS_ANNOUNCE_RECEIPT_TIMEOUT_MAXIMUM)) {
            /* Caller provided (min,max) exceed allowable value(s). */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->log_announce_interval_minimum < 
                PTP_CLOCK_PRESETS_LOG_ANNOUNCE_INTERVAL_MINIMUM) ||
                (clock_info->log_announce_interval_maximum > 
                PTP_CLOCK_PRESETS_LOG_ANNOUNCE_INTERVAL_MAXIMUM)) {
            /* Caller provided (min,max) exceed allowable value(s). */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->log_sync_interval_minimum < 
                PTP_CLOCK_PRESETS_LOG_SYNC_INTERVAL_MINIMUM) ||
                (clock_info->log_sync_interval_maximum > 
                PTP_CLOCK_PRESETS_LOG_SYNC_INTERVAL_MAXIMUM)) {
            /* Caller provided (min,max) exceed allowable value(s). */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->log_min_delay_req_interval_minimum < 
                 PTP_CLOCK_PRESETS_LOG_MIN_DELAY_REQ_INTERVAL_MINIMUM) ||
                 (clock_info->log_min_delay_req_interval_maximum > 
                 PTP_CLOCK_PRESETS_LOG_MIN_DELAY_REQ_INTERVAL_MAXIMUM)) {
            /* Caller provided (min,max) exceed allowable value(s). */
            return BCM_E_PARAM; 
        }

        /* DEFAULT clock configuration PTP attribute checking. */
        if ((clock_info->announce_receipt_timeout_default < 
                 clock_info->announce_receipt_timeout_minimum) ||
                (clock_info->announce_receipt_timeout_default> 
                 clock_info->announce_receipt_timeout_maximum)) {
             /* Caller provided default value is not in (min,max) range. */
            return BCM_E_PARAM;
        }
        
        if ((clock_info->log_announce_interval_default < 
                 clock_info->log_announce_interval_minimum) ||
                (clock_info->log_announce_interval_default > 
                 clock_info->log_announce_interval_maximum)) {
            /* Caller provided default value is not in (min,max) range. */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->log_sync_interval_default < 
                 clock_info->log_sync_interval_minimum) ||
                (clock_info->log_sync_interval_default > 
                 clock_info->log_sync_interval_maximum)) {
            /* Caller provided default value is not in (min,max) range. */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->log_min_delay_req_interval_default < 
                 clock_info->log_min_delay_req_interval_minimum) ||
                (clock_info->log_min_delay_req_interval_default > 
                 clock_info->log_min_delay_req_interval_maximum)) {
            /* Caller provided default value is not in (min,max) range. */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->domain_number_default < 
                 clock_info->domain_number_minimum) ||
                (clock_info->domain_number_default > 
                 clock_info->domain_number_maximum)) 
        {
            /* Caller provided default value is not in (min,max) range. */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->priority1_default < clock_info->priority1_minimum) ||
                (clock_info->priority1_default > clock_info->priority1_maximum)) {
            /* Caller provided default value is not in (min,max) range. */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->priority2_default < clock_info->priority2_minimum) ||
                (clock_info->priority2_default > clock_info->priority2_maximum)) {
            /* Caller provided default value is not in (min,max) range. */
            return BCM_E_PARAM; 
        }
        
        /* CURRENT clock configuration PTP attribute checking. */
        if ((clock_info->domain_number < clock_info->domain_number_minimum) ||
                (clock_info->domain_number > clock_info->domain_number_maximum)) {
            /* Caller provided current value is not in (min,max) range. */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->priority1< clock_info->priority1_minimum) ||
                (clock_info->priority1> clock_info->priority1_maximum)) {
            /* Caller provided current value is not in (min,max) range. */
            return BCM_E_PARAM; 
        }
        
        if ((clock_info->priority2< clock_info->priority2_minimum) ||
                (clock_info->priority2> clock_info->priority2_maximum)) {
            /* Caller provided current value is not in (min,max) range. */
            return BCM_E_PARAM; 
        }    

        /* Assign PTP clock information to cache. */
        if (BCM_FAILURE(rv = _bcm_ptp_clock_cache_info_create(
                unit, ptp_id, clock_info->clock_num))) {
            return rv;
        }

        _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id]
                                .clock_array[clock_info->clock_num]
                                .clock_info = *clock_info;

        if (BCM_FAILURE(rv = bcm_esw_ptp_clock_port_identity_get(unit, ptp_id, 
                clock_info->clock_num, PTP_IEEE1588_ALL_PORTS, &portid))) {
            return rv;
        }

        
        /* Create the PTP management data for the PTP clock. */
        if (BCM_FAILURE(rv = _bcm_ptp_management_clock_create(unit,
                ptp_id, clock_info->clock_num))) {
            return rv;
        }

        /*
         * Make payload.
         *    Octet 0...2   : Custom management message key/identifier.
         *                   Octet 0= 'B'; Octet 1= 'C'; Octet 2= 'M'.
         *    Octet 3...5   : Reserved.
         *    Octet 6       : Instance.
         *    Octet 7       : Clock type.
         *    Octet 8...9   : Number of ports.
         *    Octet 10      : Clock class.
         *    Octet 11      : PTP domain.
         *    Octet 12...13 : Scaled log variance.
         *    Octet 14      : Priority1.
         *    Octet 15      : Priority2.
         *    Octet 16      : Slave-Only (SO) Boolean.
         *    Octet 17      : Transparent clock primary domain.
         *    Octet 18      : Transparent clock delay mechanism.
         *    Octet 19      : Log announce interval (MINIMUM).
         *    Octet 20      : Log announce interval (DEFAULT).
         *    Octet 21      : Log announce interval (MAXIMUM).
         *    Octet 22      : Announce receipt timeout (MINIMUM).
         *    Octet 23      : Announce receipt timeout (DEFAULT).
         *    Octet 24      : Announce receipt timeout (MAXIMUM).
         *    Octet 25      : Log sync interval (MINIMUM).
         *    Octet 26      : Log sync interval (DEFAULT).
         *    Octet 27      : Log sync interval (MAXIMUM).
         *    Octet 28      : Log min PDelay request interval (MININUM) or
         *                    log min delay request interval (MINIMUM).
         *    Octet 29      : Log min PDelay request interval (DEFAULT) or
         *                    log min delay request interval (DEFAULT).
         *    Octet 30      : Log min PDelay request interval (MAXIMUM) or
         *                    log min delay request interval (MAXIMUM).
         *    Octet 31      : PTP domain number (MINIMUM).
         *    Octet 32      : PTP domain number (DEFAULT).
         *    Octet 33      : PTP domain number (MAXIMUM).
         *    Octet 34      : Priority1 (MINIMUM).
         *    Octet 35      : Priority1 (DEFAULT).
         *    Octet 36      : Priority1 (MAXIMUM).
         *    Octet 37      : Priority2 (MINIMUM).
         *    Octet 38      : Priority2 (DEFAULT).
         *    Octet 39      : Priority2 (MAXIMUM).
         *    Octet 40      : Number of virtual interfaces.
         */
        i = 0;
        payload[i++] = 'B';
        payload[i++] = 'C';
        payload[i++] = 'M';
        i += 3;
       
        payload[i++] = clock_info->clock_num;
        payload[i++] = clock_info->type;
       
        _bcm_ptp_uint16_write(payload+i, clock_info->num_ports);
        i += sizeof(uint16);
       
        payload[i++] = clock_info->class;
        payload[i++] = clock_info->domain_number;
       
        _bcm_ptp_uint16_write(payload+i, clock_info->scaled_log_variance);
        i += sizeof(uint16);
       
        payload[i++] = clock_info->priority1;
        payload[i++] = clock_info->priority2;
        payload[i++] = clock_info->slaveonly & 1;
       
        payload[i++] = clock_info->tc_primary_domain;
        payload[i++] = clock_info->tc_delay_mechanism;
       
        payload[i++] = clock_info->log_announce_interval_minimum;
        payload[i++] = clock_info->log_announce_interval_default;
        payload[i++] = clock_info->log_announce_interval_maximum;
       
        payload[i++] = clock_info->announce_receipt_timeout_minimum;
        payload[i++] = clock_info->announce_receipt_timeout_default;
        payload[i++] = clock_info->announce_receipt_timeout_maximum;
       
        payload[i++] = clock_info->log_sync_interval_minimum;
        payload[i++] = clock_info->log_sync_interval_default;
        payload[i++] = clock_info->log_sync_interval_maximum;
       
        payload[i++] = clock_info->log_min_delay_req_interval_minimum;
        payload[i++] = clock_info->log_min_delay_req_interval_default;
        payload[i++] = clock_info->log_min_delay_req_interval_maximum;
       
        payload[i++] = clock_info->domain_number_minimum;
        payload[i++] = clock_info->domain_number_default;
        payload[i++] = clock_info->domain_number_maximum;
        
        payload[i++] = clock_info->priority1_minimum;
        payload[i++] = clock_info->priority1_default;
        payload[i++] = clock_info->priority1_maximum;
       
        payload[i++] = clock_info->priority2_minimum;
        payload[i++] = clock_info->priority2_default;
        payload[i++] = clock_info->priority2_maximum;
       
        payload[i] = clock_info->number_virtual_interfaces;

        if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, ptp_id, 
                clock_info->clock_num, &portid,
                PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_CREATE_CLOCK_INSTANCE,
                payload, PTP_MGMTMSG_PAYLOAD_CLOCK_INSTANCE_SIZE_OCTETS, 
                resp, &resp_len))) {
            return rv;    
        }
        
        /*
         * Register domain with PTP management framework.
         * Ensure subsequent management messages are configured with proper 
         * domain.
         */
        if (BCM_FAILURE(rv = _bcm_ptp_management_domain_set(unit, ptp_id, 
                clock_info->clock_num, clock_info->domain_number))) {
            return rv;
        }
        
        /* Create a unicast master table for the PTP clock. */
        if (BCM_FAILURE(rv = _bcm_ptp_unicast_master_table_clock_create(unit,
                ptp_id, clock_info->clock_num))) {
            return rv;
        }
        
        /* Create unicast slave tables for the PTP clock ports. */
        if (BCM_FAILURE(rv = _bcm_ptp_unicast_slave_table_clock_create(unit,
                ptp_id, clock_info->clock_num))) {
            return rv;
        }
        
        /* Create an acceptable master table for the PTP clock. */
        if (BCM_FAILURE(rv = _bcm_ptp_acceptable_master_table_clock_create(unit,
                ptp_id, clock_info->clock_num))) {
            return rv;
        }
        
        BCM_IF_ERROR_RETURN(bcm_esw_ptp_unlock(unit));
    }

    return rv;
}
#endif /* defined(INCLUDE_PTP)*/
