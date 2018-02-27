/*
 * $Id: rx.c 1.1.2.17 Broadcom SDK $
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

#ifdef BCM_HIDE_DISPATCHABLE
#undef BCM_HIDE_DISPATCHABLE
#endif

#include <soc/defs.h>
#include <soc/drv.h>

#include <bcm/pkt.h>
#include <bcm/tx.h>
#include <bcm/rx.h>
#include <bcm/error.h>
#include <bcm/ptp.h>
#include <bcm_int/esw/ptp.h>
#include <soc/uc_msg.h>

#include <bcm_int/esw/rx.h>
#include <bcm_int/esw/tx.h>
#include <bcm_int/esw_dispatch.h>

#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/katana.h>
#endif

#define PTP_SDK_VERSION         0x01000000
#define PTP_UC_MIN_VERSION      0x01000000

#define PTP_RX_PACKET_MIN_SIZE_OCTETS     (14)
#define PTP_RX_TUNNEL_MSG_MIN_SIZE_OCTETS (8)
#define PTP_RX_EVENT_MSG_MIN_SIZE_OCTETS  (2)
#define PTP_RX_MGMT_MIN_SIZE              (0x64)
#define PTP_RX_UDP_PAYLOAD_OFFSET         (46)

/* PTP clock Rx data. */
typedef struct _bcm_ptp_clock_rx_data_s {
    sal_sem_t response_ready;            
    uint8 *response_data;
    int response_len;
} _bcm_ptp_clock_rx_data_t;

/* Stack PTP Rx data arrays. */
typedef struct _bcm_ptp_stack_rx_array_s {
    _bcm_ptp_memstate_t memstate;
    
    bcm_mac_t host_mac;
    bcm_mac_t top_mac;
    int tpid;
    int vlan;

    _bcm_ptp_clock_rx_data_t *clock_data;
} _bcm_ptp_stack_rx_array_t;

/* Unit PTP Rx data arrays. */
typedef struct _bcm_ptp_unit_rx_array_s {
    _bcm_ptp_memstate_t memstate;
       
    bcm_ptp_cb management_cb;
    bcm_ptp_cb event_cb;
    bcm_ptp_cb signal_cb;
    bcm_ptp_cb fault_cb;
    uint8      *management_user_data;
    uint8      *event_user_data;
    uint8      *signal_user_data;
    uint8      *fault_user_data;
            
    _bcm_ptp_stack_rx_array_t *stack_array;
} _bcm_ptp_unit_rx_array_t;

static const _bcm_ptp_clock_rx_data_t rx_default;
static _bcm_ptp_unit_rx_array_t unit_rx_array[BCM_MAX_NUM_UNITS];

#ifdef BCM_KATANA_SUPPORT
static void _bcm_ptp_rx_thread(void *arg);
#endif

#ifdef PTP_KEYSTONE_STACK
static bcm_rx_t _bcm_ptp_rx_callback(
    int unit, 
    bcm_pkt_t *pkt, 
    void *cookie);

#if 0 /* Unused. */
static int _bcm_ptp_rx_message_source_port_get(
    uint8 *message, 
    uint16 *src_port);
#endif /* Unused. */

static int _bcm_ptp_rx_message_destination_port_get(
    uint8 *message, 
    uint16 *dest_port);

#if 0 /* Unused. */
static int _bcm_ptp_rx_message_type_get(
    uint8 *message, 
    _bcmPTPmessageType_t *message_type);
#endif /* Unused. */

static int _bcm_ptp_rx_message_source_clock_identity_get(
    uint8 *message, 
    bcm_ptp_clock_identity_t *clock_identity);

static int _bcm_ptp_rx_message_target_clock_identity_get(
    uint8 *message, 
    bcm_ptp_clock_identity_t *clock_identity);

static int _bcm_ptp_rx_message_length_get(
    uint8 *message,
    uint16 *message_len);

#endif /* PTP_KEYSTONE_STACK */

STATIC int _bcm_ptp_event_handler_default(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint32 flags,
    bcm_ptp_cb_type_t type,
    uint32 length,
    uint8 *data,
    void *user_data);

STATIC int _bcm_ptp_management_handler_default(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint32 flags,
    bcm_ptp_cb_type_t type,
    uint32 length,
    uint8 *data,
    void *user_data);

STATIC int _bcm_ptp_signal_handler_default(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint32 flags,
    bcm_ptp_cb_type_t type,
    uint32 length,
    uint8 *data,
    void *user_data);

STATIC const char* _bcm_ptp_port_state_description(
    _bcm_ptp_port_state_t state);

STATIC const char* _bcm_ptp_ieee1588_warn_reason_description(
    _bcm_ptp_ieee1588_warn_reason_t reason);

STATIC const char* _bcm_ptp_servo_state_description(
    _bcm_ptp_fll_state_t state); 

STATIC const char* _bcm_ptp_pps_in_state_description(
    _bcm_ptp_pps_in_state_t state);

/*
 * Function:
 *      _bcm_ptp_rx_init
 * Purpose:
 *      Initialize the PTP Rx framework and data of a unit.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_rx_init(
    int unit)
{
    int rv = BCM_E_UNAVAIL;
    _bcm_ptp_stack_rx_array_t *stack_p;
     
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
                           
    /* TODO: Fully implement logic to handle repeated initialization. */
    stack_p = sal_alloc(PTP_MAX_STACKS_PER_UNIT*
                        sizeof(_bcm_ptp_stack_rx_array_t),"Unit Rx arrays");

    if (!stack_p) {
        unit_rx_array[unit].memstate = PTP_MEMSTATE_FAILURE;
        return BCM_E_MEMORY;
    }

    unit_rx_array[unit].stack_array = stack_p;
    unit_rx_array[unit].memstate = PTP_MEMSTATE_INITIALIZED;

    unit_rx_array[unit].management_cb = NULL;
    unit_rx_array[unit].event_cb = NULL;
    unit_rx_array[unit].signal_cb = NULL;
    unit_rx_array[unit].fault_cb = NULL;
        
    unit_rx_array[unit].management_user_data = NULL;
    unit_rx_array[unit].event_user_data = NULL;
    unit_rx_array[unit].signal_user_data = NULL;
    unit_rx_array[unit].fault_user_data = NULL;
        
    /* 
     * Add default callback functions for event, tunneled PTP management 
     * messages, and tunneled PTP signaling messages. 
     */
    if (BCM_FAILURE(rv = _bcm_ptp_register_event_callback(unit, 
            _bcm_ptp_event_handler_default, NULL))) {
        PTP_ERROR_FUNC("_bcm_ptp_register_event_callback()");
    }

    if (BCM_FAILURE(rv = _bcm_ptp_register_management_callback(unit, 
            _bcm_ptp_management_handler_default, NULL))) {
        PTP_ERROR_FUNC("_bcm_ptp_register_management_callback()");
    }

    if (BCM_FAILURE(rv = _bcm_ptp_register_signal_callback(unit, 
            _bcm_ptp_signal_handler_default, NULL))) {
        PTP_ERROR_FUNC("_bcm_ptp_register_signal_callback()");
    }

    /* TODO: below code is external-stack only */

#ifdef PTP_KEYSTONE_STACK
    if (!bcm_rx_active(unit)) {
        if (BCM_FAILURE(rv = bcm_rx_cfg_init(unit))) {
            PTP_ERROR_FUNC("bcm_rx_cfg_init()");
            return rv;
        }

        if (BCM_FAILURE(rv = bcm_rx_start(unit, NULL))) {
            PTP_ERROR_FUNC("bcm_rx_start()");
            return rv;
        }
    }

#ifndef CUSTOMER_CALLBACK
    if (BCM_FAILURE(rv = bcm_rx_register(unit, "BCM_PTP_Rx", 
            _bcm_ptp_rx_callback, BCM_RX_PRIO_MAX, NULL, BCM_RCO_F_ALL_COS))) {
        PTP_ERROR_FUNC("bcm_rx_register()");
        return rv;
    }
#endif /* CUSTOMER_CALLBACK */

#endif /* PTP_KEYSTONE_STACK */

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_rx_stack_create
 * Purpose:
 *      Create the PTP Rx data of a PTP stack.
 * Parameters:
 *      unit     - (IN) Unit number.
 *      ptp_id   - (IN) PTP stack ID.
 *      host_mac - (IN) Host MAC address.
 *      top_mac  - (IN) ToP MAC address.
 *      tpid     - (IN) TPID for Host <-> ToP Communication
 *      vlan     - (IN) VLAN for Host <-> ToP Communication
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_rx_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    bcm_mac_t *host_mac,
    bcm_mac_t *top_mac,
    int tpid,
    int vlan)
{
    int rv = BCM_E_UNAVAIL;  
    _bcm_ptp_clock_rx_data_t *data_p;
        
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }
    
    if (unit_rx_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, 
            "_bcm_ptp_rx_stack_create(): memory state"));
        return BCM_E_UNAVAIL;
    }
        
    /* TODO: Fully implement logic to handle repeated creation. */
    data_p = sal_alloc(PTP_MAX_CLOCK_INSTANCES*
                       sizeof(_bcm_ptp_clock_rx_data_t),
                       "PTP stack Rx array");

    if (!data_p) {
        unit_rx_array[unit].stack_array[ptp_id].memstate = 
            PTP_MEMSTATE_FAILURE;
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, 
            "_bcm_ptp_rx_stack_create(): not initialized"));
        return BCM_E_MEMORY;
    }

    unit_rx_array[unit].stack_array[ptp_id].clock_data = data_p;
    unit_rx_array[unit].stack_array[ptp_id].memstate = 
        PTP_MEMSTATE_INITIALIZED;

    /* TODO: this is only for stack on external TOP */
    {
        sal_memcpy(unit_rx_array[unit].stack_array[ptp_id].host_mac, host_mac, 
                   sizeof(bcm_mac_t));
        
        sal_memcpy(unit_rx_array[unit].stack_array[ptp_id].top_mac, top_mac, 
                   sizeof(bcm_mac_t));
        
        unit_rx_array[unit].stack_array[ptp_id].tpid = tpid;
        unit_rx_array[unit].stack_array[ptp_id].vlan = vlan;
    }

    /* TODO: this is only for stack on internal TOP */
#ifdef BCM_KATANA_SUPPORT
	if SOC_IS_KATANA(unit) {
        sal_thread_t rx_tid;
        _bcm_ptp_stack_info_t *stack_p;
        _bcm_ptp_info_t *ptp_info_p;
        int timeout_usec = 1900000;
        int max_num_cores = 2;
        int result;
        int c;
        
        SET_PTP_INFO;
        stack_p = &ptp_info_p->stack_info[ptp_id];

        /* Note: this should probably be in the dispatched transport_init function, but that is not called
           until management init... well after the Rx init that needs to know the core number.  Probably an
           indication that rx and management should be refactored
        */
        rv = BCM_E_UNAVAIL;
        for (c=0; c < max_num_cores; c++) {
            soc_cm_print("Trying Rx on core %d\n", c);
            result = soc_cmic_uc_appl_init(unit, c, MOS_MSG_CLASS_1588, timeout_usec, PTP_SDK_VERSION, PTP_UC_MIN_VERSION);
            if (SOC_E_NONE == result){
                /* uKernel communcations started successfully */

                mos_msg_data_t start_msg;
                start_msg.s.mclass = MOS_MSG_CLASS_1588;
                start_msg.s.subclass = MOS_MSG_SUBCLASS_1588_CONFIG;
                start_msg.s.len = 0;  /* unused */
                start_msg.s.data = bcm_htonl(soc_cm_l2p(unit, (void*)&stack_p->int_state));

                soc_cmic_uc_msg_send(unit, c, &start_msg, timeout_usec);

                soc_cm_print("RECEIVED 1588 TRANSPORT READY on core %d\n", c);


                stack_p->int_state.core_num = c;
                rv = BCM_E_NONE;

                rx_tid = sal_thread_create("PTP Rx", SAL_THREAD_STKSZ,
                                           soc_property_get(unit, spn_UC_MSG_THREAD_PRI, 50) + 1,
                                           _bcm_ptp_rx_thread, stack_p);
                break;
            }
            soc_cm_print("No response on core %d\n", c);
        }

    }
#endif

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_rx_clock_create
 * Purpose:
 *      Create the PTP Rx data of a PTP clock.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_rx_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num)
{
    int rv = BCM_E_UNAVAIL;
             
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
    
    if ((unit_rx_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
        (unit_rx_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }
        
    unit_rx_array[unit].stack_array[ptp_id].clock_data[clock_num].response_data = 0;
    unit_rx_array[unit].stack_array[ptp_id].clock_data[clock_num].response_len = 0;

    unit_rx_array[unit].stack_array[ptp_id].clock_data[clock_num].response_ready = 
        sal_sem_create("BCM_PTP_resp", sal_sem_BINARY, 0);
        
    return rv;
}



/* External version */
int
_bcm_ptp_external_rx_response_free(int unit, int ptp_id, uint8 *resp_data)
{
    return bcm_rx_free(unit, resp_data - PTP_RX_UDP_PAYLOAD_OFFSET);
}

/* Internal version */
int
_bcm_ptp_internal_rx_response_free(int unit, int ptp_id, uint8 *resp_data)
{
    _bcm_ptp_stack_info_t *stack_p;
    _bcm_ptp_info_t *ptp_info_p;
    int i;

    SET_PTP_INFO;
    stack_p = &ptp_info_p->stack_info[ptp_id];
    for (i = 0; i < BCM_PTP_MAX_BUFFERS; ++i) {
        if (stack_p->int_state.mbox->data[i] == resp_data) {
            stack_p->int_state.mbox->status[i] = MBOX_STATUS_EMPTY;
            return BCM_E_NONE;            
        }
    }

    soc_cm_print("Invalid PTP rx response free (%p)\n", (void *)resp_data);

    return BCM_E_NOT_FOUND;
}

                       
/*
 * Function:
 *      _bcm_ptp_rx_response_flush
 * Purpose:
 *      Flush prior Rx response.
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      ptp_id     - (IN)  PTP stack ID.
 *      clock_num  - (IN)  PTP clock number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_rx_response_flush(
    int unit,
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num)
{
    int rv = BCM_E_UNAVAIL;    
    int spl;
    
    uint8 *prior_data;
    _bcm_ptp_stack_info_t *stack_p;
    _bcm_ptp_info_t *ptp_info_p;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num, 
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }
    
    if ((unit_rx_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_rx_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, 
            "_bcm_ptp_rx_response_flush(): memory state"));
        return BCM_E_UNAVAIL;
    }  
    
    if (sal_sem_take(unit_rx_array[unit].stack_array[ptp_id]
            .clock_data[clock_num].response_ready, sal_mutex_NOWAIT) 
            == BCM_E_NONE) {
        /*
         * Flush response.
         * NOTICE: Response already waiting is unexpected.
         */
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, 
            "Flushed unexpected response"));
        
        /* Lock. */
        spl = sal_splhi();
        
        prior_data = unit_rx_array[unit].stack_array[ptp_id]
                                        .clock_data[clock_num].response_data;
        
        unit_rx_array[unit].stack_array[ptp_id]
                           .clock_data[clock_num].response_data = 0;
        
        /* Unlock. */
        sal_spl(spl);

        if (prior_data) {
            
            SET_PTP_INFO;
            stack_p = &ptp_info_p->stack_info[ptp_id];

            stack_p->rx_free(unit, ptp_id, prior_data);
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_rx_response_get
 * Purpose:
 *      Get Rx response data for a PTP clock.
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      ptp_id     - (IN)  PTP stack ID.
 *      clock_num  - (IN)  PTP clock number.
 *      usec       - (IN)  Semaphore timeout (usec).
 *      data       - (OUT) Response data.
 *      data_len   - (OUT) Response data size (octets).
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_rx_response_get(
    int unit,
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int usec, 
    uint8 **data, 
    int *data_len)
{
    int rv = BCM_E_UNAVAIL;   
    int spl;
    sal_usecs_t expiration_time = sal_time_usecs() + usec;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num, 
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
    
    if ((unit_rx_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_rx_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }  

    rv = BCM_E_FAIL;
    while (BCM_FAILURE(rv) && (int32) (sal_time_usecs() - expiration_time) < 0) {
        rv = sal_sem_take(unit_rx_array[unit].stack_array[ptp_id].clock_data[clock_num].response_ready, usec);
    }
    if (BCM_FAILURE(rv)) {
        PTP_ERROR_FUNC("sal_sem_take()");
        return rv;
    }

    /* Lock. */
    spl = sal_splhi(); 
    
    *data = unit_rx_array[unit].stack_array[ptp_id]
               .clock_data[clock_num].response_data;
    
    *data_len = unit_rx_array[unit].stack_array[ptp_id]
                    .clock_data[clock_num].response_len;
    
    unit_rx_array[unit].stack_array[ptp_id]
        .clock_data[clock_num].response_data = 0;
    
    /* Unlock. */
    sal_spl(spl);

    return rv;
}


#ifdef BCM_KATANA_SUPPORT
static void _bcm_ptp_rx_thread(void *arg)
{
    _bcm_ptp_stack_info_t *stack_p = arg;
    int unit = stack_p->unit;
    bcm_ptp_stack_id_t ptp_id = stack_p->stack_id;

    _bcm_ptp_stack_rx_array_t *stack_rx = &unit_rx_array[unit].stack_array[ptp_id];

    int finished = 0;
    int mbox;

    mos_msg_data_t rcv;

    while (!finished) {
        int rv = soc_cmic_uc_msg_receive(stack_p->unit, stack_p->int_state.core_num, MOS_MSG_CLASS_1588, &rcv, sal_sem_FOREVER);
        if (rv) {
            /* got error, so wait */
            sal_usleep(100000);
        }

        if (rcv.s.subclass == MOS_MSG_SUBCLASS_1588_TUNNEL_HOST) {
            unsigned data = bcm_ntohl(rcv.s.data);
            /* unsigned clock_num = ((data >> 8) & 0xff); */
            int cb_flags = 0;                      /* TODO: Set flags for PTP mgmt/signaling message callback. */
            int cb_data_len = rcv.s.len;
            mbox = (data & 0xff);  /* the mailbox with the tunneled message  */

            assert(mbox < BCM_PTP_MAX_BUFFERS);
            if (stack_p->int_state.mbox->status[mbox] == MBOX_STATUS_ATTN_HOST_TUNNEL) {
                int message_type = soc_ntohl_load((uint8 *)stack_p->int_state.mbox->data[mbox]);
                /* int ptp_offset = soc_ntohl_load((uint8 *)stack_p->int_state.mbox->data[mbox] + sizeof(uint32)); */
                uint8 * cb_data = (uint8 *)stack_p->int_state.mbox->data[mbox] + 2 * sizeof(uint32);
                
                switch (message_type) {
                case _bcmPTPmessageType_signaling:
                    if (unit_rx_array[unit].signal_cb) {
                        unit_rx_array[unit].signal_cb(unit, ptp_id, cb_flags, 
                                                      bcmPTPCallbackTypeSignal, 
                                                      cb_data_len, cb_data,
                                                      unit_rx_array[unit].signal_user_data);
                    }
            
                    break;
                    
                case _bcmPTPmessageType_management:
                    if (unit_rx_array[unit].management_cb) {
                        unit_rx_array[unit].management_cb(unit, ptp_id, cb_flags, 
                                                          bcmPTPCallbackTypeManagement, 
                                                          cb_data_len, cb_data,
                                                          unit_rx_array[unit].management_user_data);
                    }
                    
                    break;
                }
            } else {
                /* specified mbox does not hold a message for us */
            }
        } else if (rcv.s.subclass >= MOS_MSG_SUBCLASS_1588_EVENT_BASE) {
            unsigned event = rcv.s.subclass - MOS_MSG_SUBCLASS_1588_EVENT_BASE;
            unsigned data = bcm_ntohl(rcv.s.data);
            unsigned clock_num = ((data >> 8) & 0xff);
            int cb_flags = 0;
            int cb_data_len = rcv.s.len;
            uint8 *cb_data = 0;
            mbox = (data & 0xff);  /* the mailbox with the event data, if any */
            
            if (cb_data_len) {
                assert(mbox > 0);
                assert(mbox < BCM_PTP_MAX_BUFFERS);
                if (stack_p->int_state.mbox->status[mbox] == MBOX_STATUS_ATTN_HOST_EVENT) {
                    cb_data = (uint8 *)stack_p->int_state.mbox->data[mbox];                    
                } else {
                    /* error: indicated response buffer doesn't have the contents flag set correctly */
                    cb_data_len = 0;
                }
            }

            if (unit_rx_array[unit].event_cb) {
                /* construct "event message" buffer with prepended event number */
                uint8 event_data[1600]; /* XXX should use #define based on max message size */
                _bcm_ptp_uint16_write(event_data, event);
                if (cb_data_len) {
                    memcpy(event_data + 2, cb_data, cb_data_len);  /* copy data after event number */
                }
                unit_rx_array[unit].event_cb(unit, clock_num, cb_flags, 
                                             bcmPTPCallbackTypeEvent, 
                                             cb_data_len + 2, event_data,
                                             unit_rx_array[unit].event_user_data);
            }
            
            if (mbox) {
                stack_p->int_state.mbox->status[mbox] = MBOX_STATUS_EMPTY;
            }
        }

        /* First mbox: response to command */
        if (stack_p->int_state.mbox->status[0] == MBOX_STATUS_ATTN_HOST_RESP) {
            int clock_num = stack_p->int_state.mbox->clock_num[0];
            
            stack_rx->clock_data[clock_num].response_data = (uint8 *)stack_p->int_state.mbox->data[0];
            stack_rx->clock_data[clock_num].response_len = stack_p->int_state.mbox->data_len[0];

            sal_sem_give(stack_rx->clock_data[clock_num].response_ready);
                
            /* Really should be done after the response is emptied, but here temporarily: */
            stack_p->int_state.mbox->status[0] = MBOX_STATUS_EMPTY;
        }

        /* loop through mailboxes, looking for incoming messages */
        for (mbox = 1; mbox < BCM_PTP_MAX_BUFFERS; ++mbox) {
            if (stack_p->int_state.mbox->status[mbox] != MBOX_STATUS_EMPTY) {
                soc_cm_print("NOT EMPTY: %d (%d)\n", mbox, stack_p->int_state.mbox->status[mbox]);
            }
        }
    }
}
#endif

#ifdef PTP_KEYSTONE_STACK

bcm_rx_t 
_bcm_ptp_rx_callback(
    int unit, 
    bcm_pkt_t *pkt, 
    void *cookie)
{   
    int rv = BCM_E_UNAVAIL;
    int spl;
    
    uint8 *prior_data;
    uint16 udp_dest_port;
    uint16 message_len;
    
    _bcmPTPmessageType_t message_type;
    bcm_ptp_protocol_t protocol;
    uint16 src_addr_offset;
    uint16 ptp_offset;
    
    _bcm_ptp_stack_info_t *stack_p;
    _bcm_ptp_info_t *ptp_info_p;
    bcm_ptp_clock_identity_t cb_clock_identity;
    int cb_unit;
    bcm_ptp_stack_id_t cb_ptp_id;
    int cb_clock_num;
    uint32 cb_flags;
    int vlan, tpid;

    int i = 0;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");  
        return BCM_RX_NOT_HANDLED;
    }
    
    if (unit_rx_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, 
            "Rx unit data not initialized"));
        return BCM_RX_NOT_HANDLED;
    }  

#ifdef PTP_RX_CALLBACK_DEBUG
    soc_cm_print("_bcm_ptp_rx_callback(%d,%d)\n", pkt->pkt_len, BCM_PKT_IEEE_LEN(pkt));
        _bcm_ptp_dump_hex(BCM_PKT_IEEE(pkt), BCM_PKT_IEEE_LEN(pkt));
#endif
    
    if (pkt->pkt_data[0].len < pkt->pkt_len || 
            pkt->pkt_len < PTP_RX_PACKET_MIN_SIZE_OCTETS) {
        /* 
         * Ignore packet. 
         * NOTICE: inconsistent or incompatible packet length. 
         */
        return BCM_RX_NOT_HANDLED;
    }
    
    if (BCM_FAILURE(rv = _bcm_ptp_rx_message_length_get(BCM_PKT_IEEE(pkt),
            &message_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_rx_message_length_get()");
        return BCM_RX_NOT_HANDLED;
    }
        
    /* 
     * Parse packet data.     
     */

    /** Get destination port from UDP header, which is a proxy for packet
     * type and subsequent handling. 
     */
    if (BCM_FAILURE(rv = _bcm_ptp_rx_message_destination_port_get(
             BCM_PKT_IEEE(pkt), &udp_dest_port))) {
        PTP_ERROR_FUNC("_bcm_ptp_rx_message_destination_port_get()");
        return BCM_RX_NOT_HANDLED;
    }
    
    switch (udp_dest_port) {
    case (0x0140):
        /* Response message. */

        if (pkt->pkt_len < PTP_RX_MGMT_MIN_SIZE) {
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, 
                "Bad response len"));
            return BCM_RX_NOT_HANDLED;
        }
    
        /* 
         * Parse packet data.
         * Lookup the unit number, PTP stack ID, and PTP clock number  
         * association of the packet based on the sender's PTP clock 
         * identity.
         */          
        if (BCM_FAILURE(rv = _bcm_ptp_rx_message_source_clock_identity_get(
                BCM_PKT_IEEE(pkt), &cb_clock_identity))) {
            PTP_ERROR_FUNC("_bcm_ptp_rx_message_source_clock_identity_get()");
            return BCM_RX_NOT_HANDLED;
        }
        if (BCM_FAILURE(rv = _bcm_ptp_clock_lookup(cb_clock_identity, 
                &cb_unit, &cb_ptp_id, &cb_clock_num))) {
            PTP_ERROR_FUNC("_bcm_ptp_clock_lookup()");  
            return BCM_RX_NOT_HANDLED;
        }

        /* 
         * Ensure that Rx framework is initialized and Rx data structures are 
         * created for the requisite unit and PTP stack.
         */
        if ((unit_rx_array[cb_unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
                (unit_rx_array[cb_unit].stack_array[cb_ptp_id].memstate != 
                PTP_MEMSTATE_INITIALIZED)) {
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, 
                "Rx unit/stack data not initialized"));
            return BCM_RX_NOT_HANDLED;
        }  

        /* Sanity / Security check: Validate VLAN & MAC information for this clock*/
        tpid = _bcm_ptp_uint16_read(BCM_PKT_IEEE(pkt) + 12);  /* 12: fixed offset for TPID/VLAN */
        vlan =  _bcm_ptp_uint16_read(BCM_PKT_IEEE(pkt) + 14);

        if (tpid != unit_rx_array[unit].stack_array[cb_ptp_id].tpid ||
            vlan != unit_rx_array[unit].stack_array[cb_ptp_id].vlan) {
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, "Bad response vlan"));
        }

        if (sal_memcmp(BCM_PKT_IEEE(pkt) + sizeof(bcm_mac_t),
                       unit_rx_array[unit].stack_array[cb_ptp_id].top_mac,
                       sizeof(bcm_mac_t)) != 0) {
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, "Bad response smac"));
        }
    
        if (sal_memcmp(BCM_PKT_IEEE(pkt),
                       unit_rx_array[unit].stack_array[cb_ptp_id].host_mac,
                       sizeof(bcm_mac_t)) != 0) {
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, "Bad response dmac"));
        }
    
        /* Lock. */
        spl = sal_splhi(); 
        
        prior_data = unit_rx_array[cb_unit].stack_array[cb_ptp_id]
                                           .clock_data[cb_clock_num]
                                           .response_data;
        
        unit_rx_array[cb_unit].stack_array[cb_ptp_id]
                              .clock_data[cb_clock_num]
                              .response_data = pkt->pkt_data[0].data + PTP_RX_UDP_PAYLOAD_OFFSET;
        
        unit_rx_array[cb_unit].stack_array[cb_ptp_id]
                              .clock_data[cb_clock_num]
                              .response_len =  pkt->pkt_len - PTP_RX_UDP_PAYLOAD_OFFSET;
        
        /* Unlock. */
        sal_spl(spl);
        
        /* 
         * Free unclaimed response.
         * NOTICE: If prior data exists, it must be freed.
         */
        if (prior_data) {
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, "Unclaimed response free'd."));
            
            SET_PTP_INFO;
            stack_p = &ptp_info_p->stack_info[cb_ptp_id];

            stack_p->rx_free(unit, cb_ptp_id, prior_data);
        }
        
        sal_sem_give(unit_rx_array[cb_unit].stack_array[cb_ptp_id]
                     .clock_data[cb_clock_num].response_ready);
        
        return BCM_RX_HANDLED_OWNED;

        break;
        
    case (0x0141):
        /* Forwarded (tunnel) message. */
        if (message_len < PTP_RX_TUNNEL_MSG_MIN_SIZE_OCTETS) {
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "Invalid (too-short) tunnel message received (0x%04x)\n", 
                message_len));
            return BCM_RX_HANDLED;
        }
        
        /* 
         * Parse wrapping header. 
         * Move cursor forward to "remove" wrapping header.
         *
         * NOTICE: Forwarded tunnel message prepends an 8-element header, which
         *         includes PTP message and addressing metadata.
         *         Wrapping Header Octets 0...1: PTP message type.                   
         *         Wrapping Header Octets 2...3: Protocol.
         *         Wrapping Header Octets 4...5: Source address offset.
         *         Wrapping Header Octets 6...7: PTP payload offset.
         */
        i = PTP_PTPHDR_START_IDX;
        
        message_type = _bcm_ptp_uint16_read(BCM_PKT_IEEE(pkt) + i);
        i += sizeof(uint16);
        message_len -= sizeof(uint16);
        
        protocol = _bcm_ptp_uint16_read(BCM_PKT_IEEE(pkt) + i);
        i += sizeof(uint16);
        message_len -= sizeof(uint16);
        
        src_addr_offset = _bcm_ptp_uint16_read(BCM_PKT_IEEE(pkt) + i);
        i += sizeof(uint16);
        message_len -= sizeof(uint16);
        
        ptp_offset = _bcm_ptp_uint16_read(BCM_PKT_IEEE(pkt) + i);
        i += sizeof(uint16);
        message_len -= sizeof(uint16);
                
        /* 
         * TODO: Handle case where tunneled message is non-IPv4. Alternative 
         *       mechanism req'd to ascertain unit, stack, PTP clock routing?
         *       Add this information to the wrapping header?
         *
         * Parse packet data.
         * Lookup the unit number, PTP stack ID, and PTP clock number  
         * association of the packet based on the recipient's PTP clock 
         * identity.
         */          
        if (BCM_FAILURE(rv = _bcm_ptp_rx_message_target_clock_identity_get(
                BCM_PKT_IEEE(pkt) + i, &cb_clock_identity))) {
            PTP_ERROR_FUNC("_bcm_ptp_rx_message_target_clock_identity_get()");
            return BCM_RX_NOT_HANDLED;
        }
        
        if (BCM_FAILURE(rv = _bcm_ptp_clock_lookup(cb_clock_identity, 
                &cb_unit, &cb_ptp_id, &cb_clock_num))) {
            PTP_ERROR_FUNC("_bcm_ptp_clock_lookup()");  
            return BCM_RX_NOT_HANDLED;
        }

        /* 
         * Ensure that Rx framework is initialized and Rx data structures are 
         * created for the requisite unit and PTP stack.
         */
        if ((unit_rx_array[cb_unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
                (unit_rx_array[cb_unit].stack_array[cb_ptp_id].memstate != 
                PTP_MEMSTATE_INITIALIZED)) {
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, "Rx unit/stack data not initialized"));
            return BCM_RX_NOT_HANDLED;
        }          
                
        switch (message_type) {
        case _bcmPTPmessageType_signaling:
            /* TODO: Set flags for PTP signaling message callback. */
            cb_flags = 0;
        
            if (unit_rx_array[unit].signal_cb)
            {
                unit_rx_array[unit].signal_cb(cb_unit, cb_ptp_id, cb_flags, 
                    bcmPTPCallbackTypeSignal, 
                    message_len, BCM_PKT_IEEE(pkt) + i,
                    unit_rx_array[unit].signal_user_data);
            }
            
            break;
            
        case _bcmPTPmessageType_management:
            /* TODO: Set flags for PTP management message callback. */
            cb_flags = 0;
        
            if (unit_rx_array[unit].management_cb) {
                unit_rx_array[unit].management_cb(cb_unit, cb_ptp_id, cb_flags, 
                    bcmPTPCallbackTypeManagement, 
                    message_len, BCM_PKT_IEEE(pkt) + i,
                    unit_rx_array[unit].management_user_data);
            }
            
            break;
        
        default:
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "Invalid tunnel message received: "
                "unknown/unsupported type (0x%02x)\n", message_type));
        }
        
        /* Conditionally tunnel message to ToP. */
        if (rv == PTP_CALLBACK_ACCEPT) {
            if (BCM_FAILURE(rv =_bcm_ptp_tunnel_message_to_top(cb_unit, 
                    cb_ptp_id, message_len, BCM_PKT_IEEE(pkt) + i))) {
                PTP_ERROR_FUNC("_bcm_ptp_tunnel_message_to_top()");
            }
        }
        
        break;
        
    case (0x0142):
        /* Event message. */
        if (message_len < PTP_RX_EVENT_MSG_MIN_SIZE_OCTETS) {
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "Invalid (too-short) event message received (0x%04x)\n", 
                      message_len));
            return BCM_RX_HANDLED;
        }
        
        /* 
         * TODO: Remove hardcoded unit and PTP stack ID.
         *       Additional packet parsing to extract information required 
         *       to route information to unit, PTP stack, and PTP clock.
         *       Prepend metadata to event callback data to facilitate?
         */
        cb_unit = unit;
        cb_ptp_id = 0;        
        cb_flags = 0;
        
        if (unit_rx_array[unit].event_cb) {
            i = PTP_PTPHDR_START_IDX;

            unit_rx_array[unit].event_cb(cb_unit, cb_ptp_id, cb_flags, 
                bcmPTPCallbackTypeEvent, 
                message_len, BCM_PKT_IEEE(pkt) + i,
                unit_rx_array[unit].event_user_data);
        }
           
        break;
        
    default:
#ifdef PTP_RX_CALLBACK_DEBUG
        soc_cm_print("UDP packet dst port 0x%04x not handled\n", udp_dest_port);
#endif
        return BCM_RX_NOT_HANDLED;
    }
    
    return BCM_RX_HANDLED;
}

#if 0 /* Unused. */
/*
 * Function:
 *      _bcm_ptp_rx_message_source_port_get
 * Purpose:
 *      Get source port number in UDP header.
 * Parameters:
 *      message   - (IN)  PTP management message.
 *      src_port  - (OUT) Source port number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_rx_message_source_port_get(
    uint8 *message, 
    uint16 *src_port)
{
    int i = PTP_UDPHDR_START_IDX;
    
    *src_port = _bcm_ptp_uint16_read(message + i);
    return BCM_E_NONE;
}
#endif /* Unused. */

/*
 * Function:
 *      _bcm_ptp_rx_message_destination_port_get
 * Purpose:
 *      Get destination port number in UDP header.
 * Parameters:
 *      message   - (IN)  PTP management message.
 *      dest_port - (OUT) Destination port number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_rx_message_destination_port_get(
    uint8 *message, 
    uint16 *dest_port)
{
    int i = PTP_UDPHDR_START_IDX + PTP_UDPHDR_DESTPORT_OFFSET_OCTETS;

    *dest_port = _bcm_ptp_uint16_read(message + i);
    return BCM_E_NONE;
}

#if 0 /* Unused. */
/*
 * Function:
 *      _bcm_ptp_rx_message_type_get
 * Purpose:
 *      Get PTP messageType attribute in PTP common header.
 * Parameters:
 *      message      - (IN)  PTP management message.
 *      message_type - (OUT) PTP message type (messageType).
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_rx_message_type_get(
    uint8 *message, 
    _bcmPTPmessageType_t *message_type)
{
    *message_type = (_bcmPTPmessageType_t)message[PTP_PTPHDR_START_IDX];
    return BCM_E_NONE;
}
#endif /* Unused. */

/*
 * Function:
 *      _bcm_ptp_rx_message_source_clock_identity_get
 * Purpose:
 *      Get PTP source clock identity in PTP common header.
 * Parameters:
 *      message        - (IN)  PTP management message.
 *      clock_identity - (OUT) PTP source clock identity.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_rx_message_source_clock_identity_get(
    uint8 *message, 
    bcm_ptp_clock_identity_t *clock_identity)
{
    int i = PTP_PTPHDR_START_IDX + PTP_PTPHDR_SRCPORT_OFFSET_OCTETS;
    
    sal_memcpy(clock_identity, message + i, sizeof(bcm_ptp_clock_identity_t));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_rx_message_target_clock_identity_get
 * Purpose:
 *      Get PTP source target identity in PTP management or signaling message.
 * Parameters:
 *      message        - (IN)  PTP management message.
 *      clock_identity - (OUT) PTP source clock identity.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_rx_message_target_clock_identity_get(
    uint8 *message, 
    bcm_ptp_clock_identity_t *clock_identity)
{
    int i = PTP_PTPHDR_START_IDX + PTP_PTPHDR_SIZE_OCTETS;
    
    sal_memcpy(clock_identity, message + i, sizeof(bcm_ptp_clock_identity_t));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_rx_message_length_get
 * Purpose:
 *      Get the length of a message.
 * Parameters:
 *      message     - (IN)  PTP management message.
 *      message_len - (OUT) Message length (octets).
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      Message length is size of Rx packet excluding headers.
 */
static int 
_bcm_ptp_rx_message_length_get(
    uint8 *message,
    uint16 *message_len)
{
    int i = PTP_UDPHDR_START_IDX + PTP_UDPHDR_MSGLEN_OFFSET_OCTETS;
    
    *message_len = _bcm_ptp_uint16_read(message + i) - PTP_UDPHDR_SIZE_OCTETS;
    return BCM_E_NONE;   
}
#endif /* PTP_KEYSTONE_STACK */

/*
 * Function:
 *      _bcm_ptp_register_management_callback
 * Purpose:
 *      Register a management callback function
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb - (IN) A pointer to the callback function to call for the specified PTP events
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      The unit is already locked by the calling function
 */
int
_bcm_ptp_register_management_callback(
    int unit, 
    bcm_ptp_cb cb, 
    void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    unit_rx_array[unit].management_cb = cb;
    unit_rx_array[unit].management_user_data = user_data;

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_register_event_callback
 * Purpose:
 *      Register a event callback function
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb - (IN) A pointer to the callback function to call for the specified PTP events
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      The unit is already locked by the calling function
 */
int
_bcm_ptp_register_event_callback(
    int unit, 
    bcm_ptp_cb cb, 
    void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    unit_rx_array[unit].event_cb = cb;
    unit_rx_array[unit].event_user_data = user_data;

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_register_signal_callback
 * Purpose:
 *      Register a signal callback function
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb - (IN) A pointer to the callback function to call for the specified PTP events
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      The unit is already locked by the calling function
 */
int
_bcm_ptp_register_signal_callback(
    int unit, 
    bcm_ptp_cb cb, 
    void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    unit_rx_array[unit].signal_cb = cb;
    unit_rx_array[unit].signal_user_data = user_data;

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_register_fault_callback
 * Purpose:
 *      Register a fault callback function
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb - (IN) A pointer to the callback function to call for the specified PTP events
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      The unit is already locked by the calling function
 */
int
_bcm_ptp_register_fault_callback(
    int unit, 
    bcm_ptp_cb cb, 
    void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    unit_rx_array[unit].fault_cb = cb;
    unit_rx_array[unit].fault_user_data = user_data;

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_unregister_management_callback
 * Purpose:
 *      Unregister a management callback function
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb - (IN) A pointer to the callback function to call for the specified PTP events
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      The unit is already locked by the calling function
 */
int
_bcm_ptp_unregister_management_callback(
    int unit)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    unit_rx_array[unit].management_cb = NULL;
    unit_rx_array[unit].management_user_data = NULL;

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_unregister_event_callback
 * Purpose:
 *      Unregister a event callback function
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb - (IN) A pointer to the callback function to call for the specified PTP events
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      The unit is already locked by the calling function
 */
int
_bcm_ptp_unregister_event_callback(
    int unit)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    unit_rx_array[unit].event_cb = NULL;
    unit_rx_array[unit].event_user_data = NULL;

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_unregister_signal_callback
 * Purpose:
 *      Unregister a signal callback function
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb - (IN) A pointer to the callback function to call for the specified PTP events
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      The unit is already locked by the calling function
 */
int
_bcm_ptp_unregister_signal_callback(
    int unit)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    unit_rx_array[unit].signal_cb = NULL;
    unit_rx_array[unit].signal_user_data = NULL;

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_unregister_fault_callback
 * Purpose:
 *      Unregister a fault callback function
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb - (IN) A pointer to the callback function to call for the specified PTP events
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      The unit is already locked by the calling function
 */
int
_bcm_ptp_unregister_fault_callback(
    int unit)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    unit_rx_array[unit].fault_cb = NULL;
    unit_rx_array[unit].fault_user_data = NULL;

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_event_handler_default
 * Purpose:
 *      Default event callback handler.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      flags     - (IN) Callback function flags.
 *      type      - (IN) Callback function type.
 *      length    - (IN) Callback data length (octets).
 *      data      - (IN) Callback data.
 *      user_data - (IN) Callback user data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
STATIC int
_bcm_ptp_event_handler_default(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint32 flags,
    bcm_ptp_cb_type_t type,
    uint32 length,
    uint8 *data,
    void *user_data)
{
    int rv = BCM_E_UNAVAIL;  
        
    uint8 verbose = 1; 
    uint16 event_type;
    bcm_ptp_protocol_t ucm_protocol;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }
    
    soc_cm_print("Event callback (Unit = %d, PTP Stack = %d)\n", unit, ptp_id);
    _bcm_ptp_dump_hex(data, length);
   
    /* Extract event type. */
    event_type = _bcm_ptp_uint16_read(data);
    
    /* Move cursor to beginning of event data. */
    data = data + sizeof(uint16);

    switch ((_bcm_ptp_event_t)event_type) {
    case _bcm_ptp_state_change_event:
        if (verbose) {
            soc_cm_print("Event: STATE CHANGE\n");
            /* 
             * Event message data.
             *    Octet 0      : Clock instance.
             *    Octet 1...2  : Port number.
             *    Octet 3...12 : Port identity.
             *    Octet 13     : Port state.
             *    Octet 14     : Prior port state.
             *    Octet 15     : Port state change reason.
             */
            soc_cm_print("   Instance     : %d\n", data[0]);
            soc_cm_print("   Port Number  : %d\n", _bcm_ptp_uint16_read(data+1));
            soc_cm_print("   Port Identity: "
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                       data[3], data[4], data[5], data[6], 
                       data[7], data[8], data[9], data[10],
                       _bcm_ptp_uint16_read(data + 3 + 
                                            sizeof(bcm_ptp_clock_identity_t)));
            soc_cm_print("----------------------------------------"
                       "----------------------------------------\n");
            soc_cm_print("   Old Port State: %d (%s)\n", data[14], 
                       _bcm_ptp_port_state_description(data[14]));
            soc_cm_print("   New Port State: %d (%s)\n", data[13], 
                       _bcm_ptp_port_state_description(data[13]));
            
            switch (data[15]) 
            {
            case _bcm_ptp_state_change_reason_startup:
                soc_cm_print("   Reason        : %u "
                           "(Startup, instance creation).\n", data[15]);
                break;
                
            case _bcm_ptp_state_change_reason_port_init:
                soc_cm_print("   Reason        : %u "
                           "(Port initialization).\n", data[15]);
                break;
                
            case _bcm_ptp_state_change_reason_fault:
                soc_cm_print("   Reason        : %u "
                           "(Fault detected).\n", data[15]);
                break;
                
            case _bcm_ptp_state_change_reason_bmca:
                soc_cm_print("   Reason        : %u "
                           "(BMCA state transition).\n", data[15]);
                break;
                
            case _bcm_ptp_state_change_reason_mnt_enable:
                soc_cm_print("   Reason        : %u "
                           "(Enable port management message).\n", data[15]);
                break;
                
            case _bcm_ptp_state_change_reason_mnt_disable:
                soc_cm_print("   Reason        : %u "
                           "(Disable port management message).\n", data[15]);
                break;
                
            case _bcm_ptp_state_change_reason_netw_reinit:
                soc_cm_print("   Reason        : %u "
                           "(Network interface re-initialization).\n", data[15]);
                break;
                
            case _bcm_ptp_state_change_reason_dt_master_slave:
                soc_cm_print("   Reason        : %u "
                           "(Timestamp difference, master-to-slave).\n", data[15]);
                break;
                
            default:
                soc_cm_print("   Reason        : %u "
                           "(Unknown).\n", data[15]);
            }
        }
        break;

    case _bcm_ptp_master_change_event:
        if (verbose) {
            soc_cm_print("Event: MASTER CHANGE\n");
            /* 
             * Event message data.
             *    Octet 0       : Clock instance.
             *    Octet 1...2   : Port number.
             *    Octet 3...12  : Port identity.
             *    Octet 13...22 : New master port identity.
             *    Octet 23      : New master is-unicast Boolean.
             *    Octet 24      : New master network protocol (unicast master).
             *    Octet 25...26 : New master port address length (unicast master).
             *    Octet 27...42 : New master port address (unicast master).
             *    Octet 43...52 : Old master port identity.
             *    Octet 53      : Old master is-unicast Boolean.
             *    Octet 54      : Old master network protocol (unicast master).
             *    Octet 55...56 : Old master port address length (unicast master).
             *    Octet 57...72 : Old master port address (unicast master).
             */
            soc_cm_print("   Instance     : %d\n", data[0]);
            soc_cm_print("   Port Number  : %d\n", _bcm_ptp_uint16_read(data+1));
            soc_cm_print("   Port Identity: "
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                       data[3], data[4], data[5], data[6], 
                       data[7], data[8], data[9], data[10],
                       _bcm_ptp_uint16_read(data + 3 + 
                                            sizeof(bcm_ptp_clock_identity_t)));
            soc_cm_print("----------------------------------------"
                       "----------------------------------------\n");
            soc_cm_print("   New Master Properties\n");
            soc_cm_print("   Port Identity     : "
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                       data[13], data[14], data[15], data[16], 
                       data[17], data[18], data[19], data[20],
                       _bcm_ptp_uint16_read(data + 13 + 
                                            sizeof(bcm_ptp_clock_identity_t)));
            soc_cm_print("   Unicast Master    : %u\n", data[23]);
            if (data[23]) {
                ucm_protocol = data[24];
               
                soc_cm_print("   UC Master Protocol: %u", ucm_protocol);   
                if (ucm_protocol == bcmPTPIEEE8023) {
                    soc_cm_print(" (Ethernet Layer 2)\n");
                } else if (ucm_protocol == bcmPTPUDPIPv4) {
                    soc_cm_print(" (Ethernet/UDP/IPv4)\n");
                    soc_cm_print("   UC Master Address : %u.%u.%u.%u (IPv4)\n", 
                               data[27], data[28], data[29], data[30]);
                } else if (ucm_protocol == bcmPTPUDPIPv6){
                    soc_cm_print(" (Ethernet/UDP/IPv6)\n");
                    soc_cm_print("   UC Master Address : "
                               "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                               "%02x%02x:%02x%02x:%02x%02x:%02x%02x (IPv6)\n",
                               data[27], data[28], data[29], data[30],
                               data[31], data[32], data[33], data[34],
                               data[35], data[36], data[37], data[38],
                               data[39], data[40], data[41], data[42]);
                } else {
                    soc_cm_print(" (Unknown)\n");
                }
            }
            
            soc_cm_print("\n");
            soc_cm_print("   Old Master Properties\n");
            soc_cm_print("   Port Identity     : "
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                       data[43], data[44], data[45], data[46], 
                       data[47], data[48], data[49], data[50],
                       _bcm_ptp_uint16_read(data + 43 + 
                                            sizeof(bcm_ptp_clock_identity_t)));
            soc_cm_print("   Unicast Master    : %u\n", data[53]);
            if (data[53]) {
                ucm_protocol = data[54];
               
                soc_cm_print("   UC Master Protocol: %u", ucm_protocol);   
                if (ucm_protocol == bcmPTPIEEE8023) {
                    soc_cm_print(" (Ethernet Layer 2)\n");
                } else if (ucm_protocol == bcmPTPUDPIPv4) {
                    soc_cm_print(" (Ethernet/UDP/IPv4)\n");
                    soc_cm_print("   UC Master Address : %u.%u.%u.%u (IPv4)\n", 
                               data[57], data[58], data[59], data[60]);
                } else if (ucm_protocol == bcmPTPUDPIPv6) {
                    soc_cm_print(" (Ethernet/UDP/IPv6)\n");
                    soc_cm_print("   UC Master Address : "
                               "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                               "%02x%02x:%02x%02x:%02x%02x:%02x%02x (IPv6)\n",
                               data[57], data[58], data[59], data[60],
                               data[61], data[62], data[63], data[64],
                               data[65], data[66], data[67], data[68],
                               data[69], data[70], data[71], data[72]);
                } else {
                    soc_cm_print(" (Unknown)\n");
                }
            }
        } 
        break;        

    case _bcm_ptp_master_avail_event:
        if (verbose) {
            soc_cm_print("Event: MASTER AVAILABLE\n");
            /* 
             * Event message data.
             *    Octet 0       : Clock instance.
             *    Octet 1...2   : Port number. 
             *    Octet 3...12  : Port identity.
             *    Octet 13...22 : Foreign master port identity.
             *    Octet 23      : Foreign master is-acceptable Boolean.
             *    Octet 24      : Foreign master is-unicast Boolean.
             *    Octet 25      : Foreign master network protocol (unicast foreign master).
             *    Octet 26...27 : Foreign master port address length (unicast foreign master).
             *    Octet 28...43 : Foreign master port address (unicast foreign master).
             */
            soc_cm_print("   Instance     : %d\n", data[0]);
            soc_cm_print("   Port Number  : %d\n", _bcm_ptp_uint16_read(data+1));
            soc_cm_print("   Port Identity: "
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                       data[3], data[4], data[5], data[6], 
                       data[7], data[8], data[9], data[10],
                       _bcm_ptp_uint16_read(data + 3 + 
                                            sizeof(bcm_ptp_clock_identity_t)));
            soc_cm_print("----------------------------------------"
                       "----------------------------------------\n");
            soc_cm_print("   Foreign Master Properties\n");
            soc_cm_print("   Port Identity     : "
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                       data[13], data[14], data[15], data[16], 
                       data[17], data[18], data[19], data[20],
                       _bcm_ptp_uint16_read(data + 13 + 
                                            sizeof(bcm_ptp_clock_identity_t)));
            soc_cm_print("   Acceptable Master : %u\n", data[23]);
            soc_cm_print("   Unicast Master    : %u\n", data[24]);
            if (data[24]) {
                ucm_protocol = data[25];
               
                soc_cm_print("   UC Master Protocol: %u", ucm_protocol);   
                if (ucm_protocol == bcmPTPIEEE8023) {
                    soc_cm_print(" (Ethernet Layer 2)\n");
                } else if (ucm_protocol == bcmPTPUDPIPv4) {
                    soc_cm_print(" (Ethernet/UDP/IPv4)\n");
                    soc_cm_print("   UC Master Address : %u.%u.%u.%u (IPv4)\n", 
                               data[28], data[29], data[30], data[31]);
                } else if (ucm_protocol== bcmPTPUDPIPv6) {
                    soc_cm_print(" (Ethernet/UDP/IPv6)\n");
                    soc_cm_print("   UC Master Address : "
                               "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                               "%02x%02x:%02x%02x:%02x%02x:%02x%02x (IPv6)\n",
                               data[28], data[29], data[30], data[31],
                               data[32], data[33], data[34], data[35],
                               data[36], data[37], data[38], data[39],
                               data[40], data[41], data[42], data[43]);
                } else {
                    soc_cm_print(" (Unknown)\n");
                }
            }
        }
        break;

    case _bcm_ptp_master_unavail_event:
        if (verbose) {
            soc_cm_print("Event: MASTER UNAVAILABLE\n");
            /*
             * Event message data.
             *    Octet 0     : Clock instance.
             *    Octet 1...2 : Port number.
             *    Octet 3...12: Port identity.
             *    Octet 13...22: Foreign master port identity.
             *    Octet 23     : Foreign master is-acceptable Boolean.
             *    Octet 24     : Foreign master is-unicast Boolean.
             *    Octet 25     : Foreign master network protocol (unicast foreign master).
             *    Octet 26...27: Foreign master port address length (unicast foreign master).
             *    Octet 28...43: Foreign master port address (unicast foreign master).
             */
            soc_cm_print("   Instance     : %d\n", data[0]);
            soc_cm_print("   Port Number  : %d\n", _bcm_ptp_uint16_read(data+1));
            soc_cm_print("   Port Identity: "
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                       data[3], data[4], data[5], data[6], 
                       data[7], data[8], data[9], data[10],
                       _bcm_ptp_uint16_read(data + 3 + 
                                            sizeof(bcm_ptp_clock_identity_t)));
            soc_cm_print("----------------------------------------"
                       "----------------------------------------\n");
            soc_cm_print("   Foreign Master Properties\n");
            soc_cm_print("   Port Identity     : "
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                       data[13], data[14], data[15], data[16], 
                       data[17], data[18], data[19], data[20],
                       _bcm_ptp_uint16_read(data + 13 + 
                                            sizeof(bcm_ptp_clock_identity_t)));
            soc_cm_print("   Acceptable Master : %u\n", data[23]);
            soc_cm_print("   Unicast Master    : %u\n", data[24]);
            if (data[24]) {
                ucm_protocol = data[25];
               
                soc_cm_print("   UC Master Protocol: %u", ucm_protocol);   
                if (ucm_protocol == bcmPTPIEEE8023) {
                    soc_cm_print(" (Ethernet Layer 2)\n");
                } else if (ucm_protocol == bcmPTPUDPIPv4) {
                    soc_cm_print(" (Ethernet/UDP/IPv4)\n");
                    soc_cm_print("   UC Master Address : %u.%u.%u.%u (IPv4)\n", 
                               data[28], data[29], data[30], data[31]);
                } else if (ucm_protocol == bcmPTPUDPIPv6) {
                    soc_cm_print(" (Ethernet/UDP/IPv6)\n");
                    soc_cm_print("   UC Master Address : "
                               "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                               "%02x%02x:%02x%02x:%02x%02x:%02x%02x (IPv6)\n",
                               data[28], data[29], data[30], data[31],
                               data[32], data[33], data[34], data[35],
                               data[36], data[37], data[38], data[39],
                               data[40], data[41], data[42], data[43]);
                } else {
                    soc_cm_print(" (Unknown)\n");
                }
            }
        }
        break;

    case _bcm_ptp_slave_avail_event:
        break;
        
    case _bcm_ptp_slave_unavail_event:
        break;

    case _bcm_ptp_top_oom_event:
        if (verbose) {
            /* 
             * TODO: Requires periodic check of PCI shared data interface in
             *       the ToP OOM manager function, which has previously been 
             *       implemented via piggyback on fault callback function. 
             */
            
            soc_cm_print("Event: ToP OUT-OF-MEMORY\n");
            /*
             * Event message data.
             *    Octet 0...3 : Minimum free memory (bytes).
             *    Octet 4...7 : Free ordinary blocks (bytes).
             */
            soc_cm_print("   Min. Free Memory: %u (bytes)\n", 
                       _bcm_ptp_uint32_read(data));
            soc_cm_print("   Ord. Blocks Free: %u (bytes)\n", 
                       _bcm_ptp_uint32_read(data + sizeof(uint32)));
        }
        break;

    case _bcm_ptp_top_watchdog_event:
        break;

    case _bcm_ptp_top_ready_event:
        if (verbose) {
            soc_cm_print("Event: ToP READY\n");
        }
        break;

    case _bcm_ptp_top_misc_event:
        if (verbose) {
            soc_cm_print("Event: ToP MISC\n");
        }
        break;
        
    case _bcm_ptp_top_tod_avail_event:
        if (verbose) {
            soc_cm_print("Event: ToP ToD Available\n");
        }
        break;
    
    case _bcm_ptp_top_tod_unavail_event:
        if (verbose) {
            soc_cm_print("Event: ToP ToD Unavailable\n");
        }
        break;
        
    case _bcm_ptp_ieee1588_warn_event:
        if (verbose) {
            soc_cm_print("Event: IEEE Std. 1588-2008 WARNING\n");
            /* 
             * Event message data.
             *    Octet 0      : Clock instance.
             *    Octet 1...2  : Port number.
             *    Octet 3...12 : Port identity.
             *    Octet 13     : IEEE Std. 1588-2008 warning reason code.
             *    Octet 14...N : Reason-dependent message data.
             */
            soc_cm_print("   Instance     : %d\n", data[0]);
            soc_cm_print("   Port Number  : %d\n", _bcm_ptp_uint16_read(data+1));
            soc_cm_print("   Port Identity: "
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                       data[3], data[4], data[5], data[6], 
                       data[7], data[8], data[9], data[10],
                       _bcm_ptp_uint16_read(data + 3 + 
                                            sizeof(bcm_ptp_clock_identity_t)));
            soc_cm_print("----------------------------------------"
                       "----------------------------------------\n");
            soc_cm_print("   Warning Reason: %u (%s)\n", data[13], 
                       _bcm_ptp_ieee1588_warn_reason_description(data[13]));
            soc_cm_print("\n");
            
            switch (data[13]) {
            case _bcm_ptp_ieee1588_warn_reason_logAnnounceInterval:
                /*
                 * Non-uniform logAnnounceInterval in a PTP domain.
                 *    Octet 0...13  : IEEE Std. 1588-2008 warning common data.
                 *    Octet 14...23 : Foreign master port identity.
                 *    Octet 24      : Foreign master is-acceptable Boolean.
                 *    Octet 25      : Foreign master is-unicast Boolean.
                 *    Octet 26      : Foreign master network protocol (unicast foreign master).
                 *    Octet 27...28 : Foreign master port address length (unicast foreign master).
                 *    Octet 29...44 : Foreign master port address (unicast foreign master).                
                 *    Octet 45      : Foreign master logAnnounceInterval.
                 */
                soc_cm_print("   Foreign Master Properties\n");
                soc_cm_print("   Port Identity      : "
                           "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%04x\n",
                           data[14], data[15], data[16], data[17], 
                           data[18], data[19], data[20], data[21],
                           _bcm_ptp_uint16_read(data + 14 + 
                                                sizeof(bcm_ptp_clock_identity_t)));
                soc_cm_print("   Acceptable Master  : %u\n", data[24]);
                soc_cm_print("   Unicast Master     : %u\n", data[25]);
                
                if (data[25]) {
                    ucm_protocol = data[26];

                   soc_cm_print("   UC Master Protocol: %u", ucm_protocol);   
                   if (ucm_protocol == bcmPTPIEEE8023) {
                        soc_cm_print(" (Ethernet Layer 2)\n");
                    } else if (ucm_protocol == bcmPTPUDPIPv4) {
                        soc_cm_print(" (Ethernet/UDP/IPv4)\n");
                        soc_cm_print("   UC Master Address  : %u.%u.%u.%u (IPv4)\n", 
                                   data[29], data[30], data[31], data[32]);
                    } else if (ucm_protocol == bcmPTPUDPIPv6) {
                        soc_cm_print(" (Ethernet/UDP/IPv6)\n");
                        soc_cm_print("   UC Master Address  : "
                                   "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                                   "%02x%02x:%02x%02x:%02x%02x:%02x%02x (IPv6)\n",
                                   data[29], data[30], data[31], data[32],
                                   data[33], data[34], data[35], data[36],
                                   data[37], data[38], data[39], data[40],
                                   data[41], data[42], data[43], data[44]);
                    } else {
                        soc_cm_print("   UC Master Protocol : %u (Unknown)\n", ucm_protocol);
                    }
                }
                soc_cm_print("   logAnnounceInterval: %d\n", (int8)data[45]);
                break;
                
            default:       
              ;       
            }
        }
        break;
            
    case _bcm_ptp_servo_state_event:
        soc_cm_print("Event: SERVO STATE\n");
        soc_cm_print("   Instance  : %d\n", data[0]);
        soc_cm_print("   Old State : %d (%s)\n", data[2], 
                   _bcm_ptp_servo_state_description(data[2]));
        soc_cm_print("   New State : %d (%s)\n", data[1], 
                   _bcm_ptp_servo_state_description(data[1]));
        
        break;

    case _bcm_ptp_pps_in_state_event:
        soc_cm_print("Event: PPS-IN STATE\n");
        soc_cm_print("   State : %d (%s)\n", data[0], 
                   _bcm_ptp_pps_in_state_description(data[0]));

        break;

    default:
        soc_cm_print("Unexpected event, type %d\n", event_type);

        break;
    }
    
    /* Rewind cursor. */
    data = data - sizeof(uint16);
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_management_handler_default
 * Purpose:
 *      Default forwarded (tunneled) PTP management message callback handler.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      flags     - (IN) Callback function flags.
 *      type      - (IN) Callback function type.
 *      length    - (IN) Callback data length (octets).
 *      data      - (IN) Callback data.
 *      user_data - (IN) Callback user data.
 * Returns:
 *      BCM_E_XXX (if failure)
 *      PTP_CALLBACK_ACCEPT (if success)
 * Notes:
 */
STATIC int
_bcm_ptp_management_handler_default(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint32 flags,
    bcm_ptp_cb_type_t type,
    uint32 length,
    uint8 *data,
    void *user_data)
{
    int rv = BCM_E_UNAVAIL;  
        
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }
    
    soc_cm_print("Management callback (Unit = %d, PTP Stack = %d)\n", unit, ptp_id);
    _bcm_ptp_dump_hex(data, length);
        
    return PTP_CALLBACK_ACCEPT;
}

/*
 * Function:
 *      _bcm_ptp_signal_handler_default
 * Purpose:
 *      Default forwarded (tunneled) PTP signaling message callback handler.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      flags     - (IN) Callback function flags.
 *      type      - (IN) Callback function type.
 *      length    - (IN) Callback data length (octets).
 *      data      - (IN) Callback data.
 *      user_data - (IN) Callback user data.
 * Returns:
 *      BCM_E_XXX (if failure)
 *      PTP_CALLBACK_ACCEPT (if success)
 * Notes:
 */
STATIC int
_bcm_ptp_signal_handler_default(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint32 flags,
    bcm_ptp_cb_type_t type,
    uint32 length,
    uint8 *data,
    void *user_data)
{
    int rv = BCM_E_UNAVAIL;  
        
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }
    
    soc_cm_print("Signaling callback (Unit = %d, PTP Stack = %d)\n", unit, ptp_id);
    _bcm_ptp_dump_hex(data, length);
        
    return PTP_CALLBACK_ACCEPT;
}

/*
 * Function:
 *      _bcm_ptp_port_state_description
 * Purpose:
 *      Interpret PTP port state values.
 * Parameters:
 *      state - (IN) PTP port state code.
 * Returns:
 *      Port state description
 * Notes:
 *      Ref. IEEE Std. 1588-2008, Chapter 8.2.5.3.1, Table 8.
 */
STATIC const char* 
_bcm_ptp_port_state_description(
    _bcm_ptp_port_state_t state)
{
    switch(state) {
    case _bcm_ptp_state_initializing: 
        return "Init";
        
    case _bcm_ptp_state_faulty: 
        return "Faulty";
        
    case _bcm_ptp_state_disabled: 
        return "Disabled";
        
    case _bcm_ptp_state_listening: 
        return "Listening";
        
    case _bcm_ptp_state_pre_master: 
        return "Pre-Master";
        
    case _bcm_ptp_state_master: 
        return "Master";
        
    case _bcm_ptp_state_passive: 
        return "Passive";
        
    case _bcm_ptp_state_uncalibrated: 
        return "Uncalibrated";
        
    case _bcm_ptp_state_slave: 
        return "Slave";
        
    default: 
        return "<invalid>";
    }
}

/*
 * Function:
 *      _bcm_ptp_ieee1588_warn_reason_description
 * Purpose:
 *      Interpret IEEE Std. 1588-2008 warning values.
 * Parameters:
 *      reason - (IN) IEEE Std. 1588-2008 warning code.
 * Returns:
 *      Warning reason.
 * Notes:
 *      Function provides an implementation specific set of warnings
 *      relevant to the IEEE Std. 1588-2008.
 */
STATIC const char* 
_bcm_ptp_ieee1588_warn_reason_description(
    _bcm_ptp_ieee1588_warn_reason_t reason)
{
   switch(reason) {
   case _bcm_ptp_ieee1588_warn_reason_logAnnounceInterval: 
       return "Non-uniform logAnnounceInterval in PTP domain";
       
   default: 
       return "<invalid>";
   }
}

/*
 * Function:
 *      _bcm_ptp_servo_state_description
 * Purpose:
 *      Interpret servo state values.
 * Parameters:
 *      state - (IN) Servo state code.
 * Returns:
 *      Servo state description
 * Notes:
 */
STATIC const char* 
_bcm_ptp_servo_state_description(
    _bcm_ptp_fll_state_t state) 
{
    switch (state) {
    case _bcm_ptp_fll_state_acquiring:
        return "Acquiring Lock";

    case _bcm_ptp_fll_state_warmup:
        return "Warmup";

    case _bcm_ptp_fll_state_fast:
        return "Fast Loop";

    case _bcm_ptp_fll_state_normal:
        return "Normal Loop";

    case _bcm_ptp_fll_state_bridge:
        return "Bridge";

    case _bcm_ptp_fll_state_holdover:
        return "Holdover";

    default:
        return "<Unknown>";
    }
}

/*
 * Function:
 *      _bcm_ptp_pps_in_state_description
 * Purpose:
 *      Interpret PPS-in state values.
 * Parameters:
 *      state - (IN) PPS-in state code.
 * Returns:
 *      PPS-in state description
 * Notes:
 */
STATIC const char* 
_bcm_ptp_pps_in_state_description(
    _bcm_ptp_pps_in_state_t state) 
{
    switch (state) {
    case _bcm_ptp_pps_in_state_missing:
        return "No PPS IN";

    case _bcm_ptp_pps_in_state_active_missing_tod:
        return "PPS IN, but no valid ToD";

    case _bcm_ptp_pps_in_state_valid:
        return "PPS IN with valid ToD";
        
    default:
        return "<Unknown>";
    }
}
#endif /* defined(INCLUDE_PTP) */
