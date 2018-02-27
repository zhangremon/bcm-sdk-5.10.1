/*
 * $Id: igmp.c 1.20.2.1 Broadcom SDK $
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
 * IGMP Snooping
 * Purpose: API to set IGMP Snopping registers.
 *
 */

/*
 * Robo Switch support two IGMP Snooping methods.
 * One is IGMP IP layer snooping.
 * The other is IGMP MAC layer snooping.
 * For BCM5338,BCM5324 it supports two methods.
 * For BCM5380, it only support IGMP MAC layer snooping.
 * 
 * Due to the "IGMP MAC layer snooping" and "Port Ingress Mirroring"
 *  functions are mutually exclusive. The default mode at Ingress 
 *  Mirroring MAC" register is for "IGMP MAC layer snooping".
 */

#include <soc/debug.h>
#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/igmp.h>
#include <bcm/switch.h>


typedef struct igmp_info_s
{
    int    init;   /* TRUE if IGMP module has been inited */
} igmp_info_t;

static igmp_info_t robo_igmp_info[BCM_MAX_NUM_UNITS];

#define IGMP_INIT(unit)                 \
        if (!robo_igmp_info[unit].init)         \
        return BCM_E_INIT

 

/*
 * Function:
 *     bcm_robo_igmp_snooping_init
 * Purpose:
 *     Initialize IGMP Snooping software system.
 * Parameters:
 *  unit - RoboSwitch unit number.
 * Returns:
 *     BCM_E_NONE
 */
int 
bcm_robo_igmp_snooping_init(int unit)
{
    uint32  flag = 0;

    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_igmp_snooping_init()..\n");
    robo_igmp_info[unit].init = 1;

    if (soc_feature(unit, soc_feature_igmp_ip)) {
        flag = DRV_SNOOP_IGMP_DISABLE;
        BCM_IF_ERROR_RETURN(DRV_SNOOP_SET
            (unit, flag));
    
    } else {
        return BCM_E_UNAVAIL;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_igmp_snooping_enable_set
 * Purpose:
 *      Enable/Disable the IGMP Snooping.
 * Parameters:
 *      unit   - RoboSwitch unit number.
 *      enable - TRUE to enable, FALSE to disable.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_igmp_snooping_enable_set(int unit, int enable)
{
    uint32  flag = 0;

    IGMP_INIT(unit);

    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_igmp_snooping_enable_set()..\n");
    if (soc_feature(unit, soc_feature_igmp_ip)) {
        /* This unit support "IGMP IP layer snooping" */
        BCM_IF_ERROR_RETURN(DRV_SNOOP_GET
            (unit, &flag));
        if (enable){
            if (flag & DRV_SNOOP_IGMP){
                return BCM_E_NONE;
            }
            
            /* DRV_SNOOP_IGMP_DISABLE currently is used for bcm53115, bcm53118 only */
            flag &= ~DRV_SNOOP_IGMP_DISABLE;    
            flag |= DRV_SNOOP_IGMP;
        } else {
            if (!(flag & DRV_SNOOP_IGMP)){
                return BCM_E_NONE;
            }
            
            /* bcm53115, bcm53118 support IGMP type seperated snoop/trap setting.
             *  to disable IGMP snooping here will force all types of IGMP
             *  packet been disabled on IGMP snoop/trap. 
             */
            flag |= DRV_SNOOP_IGMP_DISABLE;
            
            flag &= ~DRV_SNOOP_IGMP;

            if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                /* Since IGMP/MLD share the same register field IGMP_MLD_CHK[4:3] for TB. */
                if (flag & DRV_SNOOP_MLD) {
                    flag &= ~DRV_SNOOP_MLD;                    
                }
#endif
            }
        }
        BCM_IF_ERROR_RETURN(DRV_SNOOP_SET
            (unit, flag));
    } else {
        return BCM_E_UNAVAIL;
    }

    return (BCM_E_NONE);
}   

/*
 * Function:
 *      bcm_robo_igmp_snooping_enable_get
 * Purpose:
 *      Return whether IGMP Snooping is enabled on the specified unit.
 * Parameters:
 *      unit   - RoboSwitch unit number.
 *  enable - (OUT) TRUE to enable, FALSE to disable.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_igmp_snooping_enable_get(int unit, int *enable)
{
    uint32  flag = 0;

    IGMP_INIT(unit);

    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_igmp_snooping_enable_get()..\n");
    if (soc_feature(unit, soc_feature_igmp_ip)) {
        /* This unit support "IGMP IP layer snooping" */
        BCM_IF_ERROR_RETURN(DRV_SNOOP_GET
            (unit, &flag));
        /* Note for bcm53115, bcm53118 :
         *  - bcm53115, bcm53118 support IGMP packet type seperated snooping/traping.
         *
         *  if the one or more IGMP type is disabled or at traping mode, 
         *      the return value will be false. That means only all 3 types of 
         *      IGMP packet are working at snooping mode then the function 
         *      return value can be TRUE. 
         */
        *enable = (flag & DRV_SNOOP_IGMP) ? TRUE : FALSE;
    } else {
        return BCM_E_UNAVAIL;
    }

    return (BCM_E_NONE);
}   
 
