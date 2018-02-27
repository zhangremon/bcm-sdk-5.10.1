/*
 * $Id: robo.c 1.2 Broadcom SDK $
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
 * File:        robo.c
 * Purpose:
 * Requires:
 */
#include <shared/switch.h>
#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/mcm/robo/driver.h>
#include <soc/error.h>
#include "robo_53101.h"

#include <soc/debug.h>

#ifdef BCM_ROBO_SUPPORT

/* 
*  Function : _drv_bcm53101_led_funcs_sw_to_hw (Internal usage only)
*
*  Purpose :
*      Transfer the LED function items from SW to HW per register Spec.
*
*  Parameters :
*      unit         :  unit id
*      led_func_sw  :  SW LED funcitons definition
*      led_func_hw  :  HW LED funcitons definition
*
*  Note :
*   1. LED funcitons in most of ROBO chips within the same bit defintions 
*       but bdm53101 has a completly definition thus this routine is designed
*       for such difference.
*   2. This routine is keep internal usage only, due to thers is only 
*       bcm53101 existed such difference.
*/
STATIC int 
_drv_bcm53101_led_funcs_sw_to_hw(int uint, 
                uint32 led_func_sw,  uint32 *led_func_hw)
{
    *led_func_hw = 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_COL) ? 
            _ROBO53101_LED_FUNC_COL : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_SP_10) ? 
            _ROBO53101_LED_FUNC_SP_10 : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_SP_100_200) ? 
            _ROBO53101_LED_FUNC_SP_100_200 : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_SP_1G) ? 
            _ROBO53101_LED_FUNC_SP_1G : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_DPX_COL) ? 
            _ROBO53101_LED_FUNC_DPX_COL : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_DPX) ? 
            _ROBO53101_LED_FUNC_DPX : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_10_ACT) ? 
            _ROBO53101_LED_FUNC_10_ACT : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_100_200_ACT) ? 
            _ROBO53101_LED_FUNC_100_200_ACT : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_1G_ACT) ? 
            _ROBO53101_LED_FUNC_1G_ACT : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_ACT) ? 
            _ROBO53101_LED_FUNC_ACT : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_LNK_ACT_SP) ? 
            _ROBO53101_LED_FUNC_LNK_ACT_SP : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_LINK_ACT) ? 
            _ROBO53101_LED_FUNC_LNK : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_LNK) ? 
            _ROBO53101_LED_FUNC_LNK : 0;
    *led_func_hw |= (led_func_sw & DRV_LED_FUNC_EAV_LINK) ? 
            _ROBO53101_LED_FUNC_AVB_LINK : 0;

    return SOC_E_NONE;    
}

/* 
*  Function : _drv_bcm53101_led_funcs_hw_to_sw (Internal usage only)
*
*  Purpose :
*      Transfer the LED function items from HW 2 SW per register Spec.
*
*  Parameters :
*      unit         :  unit id
*      led_func_hw  :  HW LED funcitons definition
*      led_func_sw  :  SW LED funcitons definition
*
*  Note :
*   1. LED funcitons in most of ROBO chips within the same bit defintions 
*       but bdm53101 has a completly definition thus this routine is designed
*       for such difference.
*   2. This routine is keep internal usage only, due to thers is only 
*       bcm53101 existed such difference.
*/
STATIC int 
_drv_bcm53101_led_funcs_hw_to_sw(int uint, 
                uint32 led_func_hw,  uint32 *led_func_sw)
{
    *led_func_sw = 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_COL) ? 
            DRV_LED_FUNC_COL : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_SP_10) ? 
            DRV_LED_FUNC_SP_10 : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_SP_100_200) ? 
            DRV_LED_FUNC_SP_100_200 : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_SP_1G) ? 
            DRV_LED_FUNC_SP_1G : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_DPX_COL) ? 
            DRV_LED_FUNC_DPX_COL : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_DPX) ? 
            DRV_LED_FUNC_DPX : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_10_ACT) ? 
            DRV_LED_FUNC_10_ACT : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_100_200_ACT) ? 
            DRV_LED_FUNC_100_200_ACT : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_1G_ACT) ? 
            DRV_LED_FUNC_1G_ACT : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_ACT) ? 
            DRV_LED_FUNC_ACT : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_LNK_ACT_SP) ? 
            DRV_LED_FUNC_LNK_ACT_SP : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_LINK_ACT) ? 
            DRV_LED_FUNC_LINK_ACT : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_LNK) ? 
            DRV_LED_FUNC_LNK : 0;
    *led_func_sw |= (led_func_hw & _ROBO53101_LED_FUNC_AVB_LINK) ? 
            DRV_LED_FUNC_EAV_LINK : 0;
    
    return SOC_E_NONE;    
}

/*
 * Function: 
 *	    drv_led_func_get
 * Purpose:
 *	    Get the combined LED functions in the specific LED function group.
 * Parameters:
 *	    led_group     - (IN) led group id
 *          led_functions - (OUT) bitmap format. 
 *                          Each bit indicated a single LED function.
 */
int
drv_bcm53101_led_func_get(int unit,int led_group, uint32 *led_functions)
{
    uint32  led_functions_sw = 0;

    /* not proper for non-53101 chip */
    if (!SOC_IS_ROBO53101(unit)){
        return SOC_E_INTERNAL;
    }
    
    *led_functions = 0;
    if (led_group == DRV_LED_FUNCGRP_0){
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_FUNC0_CTLr(unit, led_functions));        
    } else if (led_group == DRV_LED_FUNCGRP_1){
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_FUNC1_CTLr(unit, led_functions));        
    } else {
        return SOC_E_UNAVAIL;
    }
    
    /* special process for 53101 due to completly changed on register spec. */
    SOC_IF_ERROR_RETURN(_drv_bcm53101_led_funcs_hw_to_sw(unit,
            *led_functions, &led_functions_sw));

    *led_functions = led_functions_sw;

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_func_set
 * Purpose:
 *	    Set the combined LED functions in the specific LED function group.
 * Parameters:
 *	    led_group     - (IN) led group id
 *          led_functions - (OUT) bitmap format. 
 *                          Each bit indicated a single LED function.
 * 
 * Note:
 */
int
drv_bcm53101_led_func_set(int unit,int led_group, uint32 led_functions)
{
    uint32  supported_led_functions = 0;
    uint32  led_functions_hw = 0;

    /* not proper for non-53101 chip */
    if (!SOC_IS_ROBO53101(unit)){
        return SOC_E_INTERNAL;
    }

    /* special process for 53101 due to completly changed on register spec. */
    SOC_IF_ERROR_RETURN(_drv_bcm53101_led_funcs_sw_to_hw(unit, 
            led_functions, &led_functions_hw));

    led_functions = led_functions_hw;

    /* check if the assigning led functions is out of supporting list */
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS, &supported_led_functions));
    if (led_functions & ~(supported_led_functions)){
        return SOC_E_PARAM;
    }
    
    if (led_group == DRV_LED_FUNCGRP_0){
        SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC0_CTLr(unit, &led_functions));
    } else if (led_group == DRV_LED_FUNCGRP_1){
        SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC1_CTLr(unit, &led_functions));
    } else {
        return SOC_E_UNAVAIL;
    }
    
    return SOC_E_NONE;
}

#endif  /* BCM_ROBO_SUPPORT */
