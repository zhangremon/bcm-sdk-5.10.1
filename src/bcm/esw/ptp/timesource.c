/*
 * $Id: timesource.c 1.1.2.4 Broadcom SDK $
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


/*
 * Function:
 *      bcm_esw_ptp_tod_input_sources_set
 * Purpose:
 *      Set PTP TOD Input Sources
 * Parameters:
 *      unit        - (IN) Unit number.
 *      ptp_id      - (IN) PTP Stack ID
 *      tod_sources - (IN) PTP TOD sources
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_tod_input_sources_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int num_tod_sources, 
    bcm_ptp_tod_input_t *tod_sources)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_esw_ptp_tod_input_sources_get
 * Purpose:
 *      Get PTP TOD Input Sources
 * Parameters:
 *      unit        - (IN) Unit number.
 *      ptp_id      - (IN) PTP Stack ID
 *      tod_sources - (OUT) PTP TOD sources
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_tod_input_sources_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int num_tod_sources, 
    bcm_ptp_tod_input_t *tod_sources)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_esw_ptp_tod_input_sources_get
 * Purpose:
 *      Get PTP Input Timesource Status
 * Parameters:
 *      unit        - (IN) Unit number.
 *      ptp_id      - (IN) PTP Stack ID
 *      status      - (OUT) PTP timesource status
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_timesource_input_status_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    bcm_ptp_timesource_status_t *status)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_esw_ptp_input_channels_set
 * Purpose:
 *      Set PTP Input Channels
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_input_channels_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int num_channels, 
    bcm_ptp_channel_t *channels)
{
    _bcm_ptp_info_t *ptp_info_p;
    _bcm_ptp_stack_info_t *stack_p;
    bcm_ptp_port_identity_t portid;
    uint8 payload[PTP_MGMTMSG_PAYLOAD_CHANNELS_SIZE_OCTETS] = {0};
    uint8 *curs = &payload[0];
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS; 
    int rv;
    int i;

    SET_PTP_INFO;

    stack_p = &ptp_info_p->stack_info[ptp_id];
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }

    /* XXX should sanity check input channel list */

    if (BCM_FAILURE(rv = bcm_esw_ptp_clock_port_identity_get(unit, ptp_id, 
            clock_num, PTP_IEEE1588_ALL_PORTS, &portid))) {
        return rv;
    }

    soc_htons_store(curs, num_channels);  curs += 2;
    
    for (i = 0; i < num_channels; ++i) {
        soc_htons_store(curs, channels[i].type);          curs += 2;
        soc_htonl_store(curs, channels[i].source);        curs += 4;
        soc_htonl_store(curs, channels[i].frequency);     curs += 4;

        *curs = channels[i].tod_index;      ++curs;

        *curs = channels[i].freq_priority;  ++curs;
        *curs = channels[i].freq_enabled;   ++curs;
        *curs = channels[i].time_prio;      ++curs;
        *curs = channels[i].time_enabled;   ++curs;

        *curs = channels[i].freq_assumed_QL;   ++curs;
        *curs = channels[i].time_assumed_QL;   ++curs;
        *curs = channels[i].assumed_QL_enabled;   ++curs;

        soc_htonl_store(curs, channels[i].resolution);     curs += 4;
    }

    rv = _bcm_ptp_management_message_send(unit, ptp_id, clock_num, &portid, 
                  PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_CHANNELS, payload, curs - payload, resp, &resp_len);

    return rv; 
}


/*
 * Function:
 *      bcm_esw_ptp_input_channels_get
 * Purpose:
 *      Set PTP Input Channels
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 * Returns:
 *      BCM_E_xxx
 * Notes:  not part of external API at this point
 */

int 
_bcm_esw_ptp_input_channels_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int *num_channels, 
    bcm_ptp_channel_t *channels)
{
    _bcm_ptp_info_t *ptp_info_p;
    _bcm_ptp_stack_info_t *stack_p;
    bcm_ptp_port_identity_t portid;
    int max_num_channels = *num_channels;

    uint8 resp[PTP_MGMTMSG_PAYLOAD_CHANNELS_SIZE_OCTETS] = {0};
    uint8 *curs = &resp[0];
    int resp_len = PTP_MGMTMSG_PAYLOAD_CHANNELS_SIZE_OCTETS;
    int rv;
    int i;

    SET_PTP_INFO;

    stack_p = &ptp_info_p->stack_info[ptp_id];
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }

    if (BCM_FAILURE(rv = bcm_esw_ptp_clock_port_identity_get(unit, ptp_id, 
            clock_num, PTP_IEEE1588_ALL_PORTS, &portid))) {
        return rv;
    }

    rv = _bcm_ptp_management_message_send(unit, ptp_id, clock_num, &portid, 
            PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_CHANNELS, 0, 0, resp, &resp_len);

    *num_channels = soc_ntohs_load(curs);  curs += 2;
    
    if (*num_channels > max_num_channels) {
        *num_channels = max_num_channels;
    }
    
    for (i = 0; i < *num_channels; ++i) {
        channels[i].type = soc_ntohs_load(curs);          curs += 2;
        channels[i].source = soc_ntohl_load(curs);        curs += 4;
        channels[i].frequency = soc_ntohl_load(curs);     curs += 4;
        channels[i].tod_index = *curs++;
        channels[i].freq_priority = *curs++;
        channels[i].freq_enabled  = *curs++;
        channels[i].time_prio = *curs++;
        channels[i].time_enabled = *curs++;
        channels[i].freq_assumed_QL = *curs++;
        channels[i].time_assumed_QL = *curs++;
        channels[i].assumed_QL_enabled = *curs++;
        channels[i].resolution = soc_ntohl_load(curs);    curs += 4;
    }

    return rv;
}


/*
 * Function:
 *      bcm_esw_ptp_input_channel_precedence_mode_set
 * Purpose:
 *      Set PTP Input Channels Precedence Mode
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_input_channel_precedence_mode_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int channel_select_mode)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_esw_ptp_input_channel_switching_mode_set
 * Purpose:
 *      Set PTP Input Channels Switching Mode
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_input_channel_switching_mode_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int channel_switching_mode)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_esw_ptp_tod_output_set
 * Purpose:
 *      Set PTP TOD Output
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_tod_output_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int *tod_output_id, 
    bcm_ptp_tod_output_t *output_info)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_esw_ptp_tod_output_get
 * Purpose:
 *      Get PTP TOD Output
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_tod_output_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int *max_tod_output_count, 
    bcm_ptp_tod_output_t *tod_output_id)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_esw_ptp_tod_output_remove
 * Purpose:
 *  
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_tod_output_remove(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int tod_output_id)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_esw_ptp_signal_output_set
 * Purpose:
 *      Set PTP Signal Output
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 *      clock_num - (IN) PTP Clock number
 *      signal_output_id - (OUT) ID of signal
 *      output_info - (IN) Signal information
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_ptp_signal_output_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int *signal_output_id, 
    bcm_ptp_signal_output_t *output)
{
    bcm_ptp_signal_output_t signal;
    bcm_ptp_port_identity_t portid;
    uint8 payload[PTP_MGMTMSG_PAYLOAD_OUTPUT_SIGNALS_SIZE_OCTETS] = {0};
    uint8 *curs = &payload[0];
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS; 
    int rv;
    unsigned idx;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }

    idx = output->pin;
    if (idx >= PTP_MAX_OUTPUT_SIGNALS) {
        return BCM_E_PARAM;
    }

    /* use the pin # as the signal_output_id, since they're unique anyhow  */
    /* this makes the "sparse" problem immediately apparent if the pin# is */
    /* not zero, but the problem would occur anyhow if any but the first   */
    /* signal were removed                                                 */

    *signal_output_id = output->pin; 

    if (BCM_FAILURE(rv = 
           _bcm_ptp_clock_cache_signal_set(unit, ptp_id, clock_num, idx, output))) {
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_esw_ptp_clock_port_identity_get(unit, ptp_id, 
            clock_num, PTP_IEEE1588_ALL_PORTS, &portid))) {
        return rv;
    }

    for (idx = 0; idx < PTP_MAX_OUTPUT_SIGNALS; ++idx) {
        if (BCM_FAILURE(rv = 
                _bcm_ptp_clock_cache_signal_get(unit, ptp_id, clock_num, idx, &signal))) {
            return rv;
        }
        soc_htonl_store(curs, signal.pin);              curs += 4;
        soc_htonl_store(curs, signal.frequency);        curs += 4;
        *curs++ = signal.phase_lock;
        soc_htonl_store(curs, signal.pulse_width_ns);   curs += 4;
        soc_htonl_store(curs, signal.pulse_offset_ns);  curs += 4;
    }

    rv = _bcm_ptp_management_message_send(unit, ptp_id, clock_num, &portid, 
                  PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_OUTPUT_SIGNALS, payload, curs - payload, resp, &resp_len);


    return rv;
}


/*
 * Function:
 *      bcm_esw_ptp_signal_output_get
 * Purpose:
 *      Get PTP Signal Output 
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 *      clock_num - (IN) PTP Clock number
 *      signal_output_count - (IN/OUT) max to return / number returned
 *      signal_output - (OUT) array of outputs
 * Returns:
 *      BCM_E_xxx
 * Notes: This function specification does not provide the ID of each signal,
 *        so signal ID equals the position in the array (there is no mechanism
 *        to make it sparse).  Invalid/inactive outputs are indicated by a
 *        Frequency of 0.       
 */       
int 
bcm_esw_ptp_signal_output_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int *signal_output_count, 
    bcm_ptp_signal_output_t *signal_output)
{
    int rv;
    unsigned i;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }

    if (*signal_output_count >= PTP_MAX_OUTPUT_SIGNALS) {
        *signal_output_count = PTP_MAX_OUTPUT_SIGNALS - 1;
    }
    
    for (i = 0; i < *signal_output_count; ++i) {
        if (BCM_FAILURE(rv = 
               _bcm_ptp_clock_cache_signal_get(unit, ptp_id, clock_num, i, signal_output))) {
            return rv;
        }

        ++signal_output;
    }
    
    return BCM_E_NONE;    
}

/*
 * Function:
 *      bcm_esw_ptp_signal_output_remove
 * Purpose:
 *      Get PTP Signal Output 
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 *      clock_num - (IN) PTP Clock number
 *      signal_output_id - (IN) Signal to remove/invalidate
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
/* Remove PTP Signal Output */
int 
bcm_esw_ptp_signal_output_remove(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int signal_output_id)
{
    bcm_ptp_signal_output_t signal_output = {0};
    signal_output.pin = signal_output_id;

    return bcm_esw_ptp_signal_output_set(unit, ptp_id, clock_num, &signal_output_id, &signal_output);
}

#endif /* defined(INCLUDE_PTP)*/
