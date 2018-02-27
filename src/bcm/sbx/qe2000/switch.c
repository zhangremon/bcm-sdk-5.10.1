/* $Id: switch.c 1.4.60.2 Broadcom SDK $
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
 * Broadcom QE2000 Switch API.
 */

#include <soc/drv.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>

#include <bcm/error.h>
#include <bcm/switch.h>
#include <bcm/debug.h>

#include <bcm/error.h>
#include <soc/sbx/qe2000_scoreboard.h>

#include <bcm_int/sbx/error.h>


int
bcm_qe2000_switch_control_get(int unit,
                 bcm_switch_control_t type,
                 int *val)
{
    if(type == bcmSwitchPktAge ) {
        soc_qe2000_pkt_age_get (unit, val);
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_qe2000_switch_control_set(int unit,
                 bcm_switch_control_t type,
                 int val)
{
     if(type == bcmSwitchPktAge ) {
        soc_qe2000_pkt_age_set (unit, val);
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
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
bcm_qe2000_switch_event_register(int unit, 
			      bcm_switch_event_cb_t cb, 
                              void *userdata)
{
    return soc_event_register(unit, (soc_event_cb_t)cb, userdata);
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
bcm_qe2000_switch_event_unregister(int unit, 
				  bcm_switch_event_cb_t cb, 
				  void *userdata)
{
    return soc_event_unregister(unit, (soc_event_cb_t)cb, userdata);
}
