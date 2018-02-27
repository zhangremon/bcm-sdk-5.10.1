/*
 * $Id: dev_prop.c 1.14 Broadcom SDK $
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
#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h> 
#include <bcm5348/robo_5348.h>
#include <soc/drv.h> 
/*
 *  Function : drv_bcm5348_dev_prop_get
 *
 *  Purpose :
 *      Get the device property information
 *
 *  Parameters :
 *      unit        :   unit id
 *      prop_type   :   property type
 *      prop_val     :   property value of the property type
 *
 *  Return :
 *      SOC_E_NONE      :   success
 *      SOC_E_PARAM    :   parameter error
 *
 *  Note :
 *      This function is to get the device porperty information.
 *
 */
int 
drv_bcm5348_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32 reg_addr, reg_val, fld_val;
    int retval, reg_len;
    
    switch (prop_type) {
        case DRV_DEV_PROP_MCAST_NUM:
            *prop_val = DRV_MCAST_GROUP_NUM;
            break;
        case DRV_DEV_PROP_AGE_TIMER_MAX_S:
            *prop_val = DRV_AGE_TIMER_MAX;
            break;
        case DRV_DEV_PROP_TRUNK_NUM:
            *prop_val = DRV_TRUNK_GROUP_NUM;
            break;
        case DRV_DEV_PROP_TRUNK_MAX_PORT_NUM:
            *prop_val = DRV_TRUNK_MAX_PORT_NUM;
            break;
        case DRV_DEV_PROP_COSQ_NUM:
            *prop_val = NUM_COS(unit);
            break;
        case DRV_DEV_PROP_MSTP_NUM:
            *prop_val = DRV_MSTP_GROUP_NUM;
            break;
        case DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT:
            *prop_val = DRV_SEC_MAC_NUM_PER_PORT;
            break;
        case DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE:
            *prop_val = DRV_COS_QUEUE_MAX_WEIGHT_VALUE;
            break;
        case DRV_DEV_PROP_AUTH_PBMP:
            if (SOC_IS_ROBO5347(unit)) {
                *prop_val = DRV_AUTH_SUPPORT_PBMP_BCM5347;
                *(prop_val+1) = DRV_AUTH_SUPPORT_PBMP_HI_BCM5347;
            } else {
                *prop_val = DRV_AUTH_SUPPORT_PBMP;
                *(prop_val+1) = DRV_AUTH_SUPPORT_PBMP_HI;
            }
            break;
        case DRV_DEV_PROP_RATE_CONTROL_PBMP:
            if (SOC_IS_ROBO5347(unit)) {
                *prop_val = DRV_RATE_CONTROL_SUPPORT_PBMP_BCM5347;
                *(prop_val+1) = DRV_RATE_CONTROL_SUPPORT_PBMP_HI_BCM5347;
            } else {
                *prop_val = DRV_RATE_CONTROL_SUPPORT_PBMP;
                *(prop_val+1) = DRV_RATE_CONTROL_SUPPORT_PBMP_HI;
            }
            *prop_val = DRV_RATE_CONTROL_SUPPORT_PBMP;
            *(prop_val+1) = DRV_RATE_CONTROL_SUPPORT_PBMP_HI;
            break;
        case DRV_DEV_PROP_VLAN_ENTRY_NUM:
            *prop_val = DRV_VLAN_ENTRY_NUM;
            break;
        case DRV_DEV_PROP_BPDU_NUM:
            *prop_val = DRV_BPDU_NUM;
            break;
        case DRV_DEV_PROP_INTERNAL_MII_PBMP:
            if (SOC_IS_ROBO5347(unit)) {                
                *prop_val = DRV_INTERNAL_MII_PBMP_BCM5347;
                *(prop_val+1) = DRV_INTERNAL_MII_PBMP_HI_BCM5347;
            } else {
                *prop_val = DRV_INTERNAL_MII_PBMP;
                *(prop_val+1) = DRV_INTERNAL_MII_PBMP_HI;
            }
            break;
        case DRV_DEV_PROP_EXTERNAL_MII_PBMP:
            *prop_val = DRV_EXTERNAL_MII_PBMP;
            *(prop_val+1) = DRV_EXTERNAL_MII_PBMP_HI;
            break;
        case DRV_DEV_PROP_CFP_TCAM_SIZE:
            /* Check the CFP size */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, STRAP_STSr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, STRAP_STSr);
            if ((retval = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_val, reg_len)) < 0) {
                    return retval;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, STRAP_STSr, &reg_val, BOND_128CFPf, &fld_val);

            if (fld_val) {
                /* 128 entries */
                *prop_val = DRV_CFP_TCAM_SIZE;
            } else {
                /* 512 entries */
                *prop_val = DRV_CFP_TCAM_SIZE * 4;
            }
            break;
        case DRV_DEV_PROP_CFP_UDFS_NUM:
            *prop_val = DRV_CFP_UDFS_NUM;
            break;
        case DRV_DEV_PROP_CFP_RNG_NUM:
            *prop_val = DRV_CFP_RNG_NUM;
            break;
        case DRV_DEV_PROP_CFP_UDFS_OFFSET_MAX:
            *prop_val = DRV_CFP_UDFS_OFFSET_MAX;
            break;
        case DRV_DEV_PROP_AUTH_SEC_MODE:
            *prop_val = DRV_AUTH_SEC_MODE;
            break;
        case DRV_DEV_PROP_AGE_HIT_VALUE:
            *prop_val = 0x1;
            break;
        case DRV_DEV_PROP_MAX_INGRESS_SFLOW_VALUE:
            /* Max value at INGRESS_RMONr.INGRESS_CFGf */
            *prop_val = 0xF;
            break;
        case DRV_DEV_PROP_MAX_EGRESS_SFLOW_VALUE:
            /* Max value at EGRESS_RMONr.EGRESS_CFGf */
            *prop_val = 0xF;
            break;
        case DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS:
            *prop_val = DRV_LED_FUNC_ALL_MASK & 
                    ~(DRV_LED_FUNC_PHYLED4 | DRV_LED_FUNC_PHYLED3 |
                    DRV_LED_FUNC_EAV_LINK | DRV_LED_FUNC_SP_100_200 | 
                    DRV_LED_FUNC_100_200_ACT | DRV_LED_FUNC_LNK_ACT_SP);
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm5348_dev_prop_set
 *
 *  Purpose :
 *     Set the device property information
 *
 *  Parameters :
 *      unit        :   unit id
 *      prop_type   :   property type
 *      prop_val     :   property value of the property type
 *
 *  Return :
 *      SOC_E_UNAVAIL 
 *
 *  Note :
 *      This function is to set the device porperty information.
 *
 */
int 
drv_bcm5348_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    return SOC_E_UNAVAIL;
}
