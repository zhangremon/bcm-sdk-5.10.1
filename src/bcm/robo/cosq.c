/*
 * $Id: cosq.c 1.70.2.1 Broadcom SDK $
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
 * COS Queue Management
 * Purpose: API to set different cosq, priorities, and scheduler registers.
 */

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/cosq.h>
#include <bcm/eav.h>

#include <sal/types.h>
#include <sal/appl/io.h>

#include <shared/pbmp.h>
#include <shared/types.h>

#include <bcm_int/robo/cosq.h>
#include <bcm_int/robo/port.h>

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>

/*
 * Function:
 *      bcm_robo_cosq_detach
 * Purpose:
 *      Discard all COS schedule/mapping state.
 * Parameters:
 *      unit - RoboSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This API is designed mainly for Tucana Chip and not suitable
 *      for Robo Chip.
 */
int 
bcm_robo_cosq_detach(int unit)
{
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_detach()..\n");
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                (unit, DRV_QOS_CTL_SW_SHADOW, FALSE));
    }
#endif
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_init
 * Purpose:
 *      Initialize (clear) all COS schedule/mapping state.
 * Parameters:
 *      unit - RoboSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_init(int unit)
{  
    bcm_cos_t   prio;
    int         num_cos;
    int         map_queue = 0;
#ifdef BCM_TB_SUPPORT
    bcm_port_t port = 0;
    int dp = 0;
    int srccp = 0;
    pcp2dptc_entry_t  entry_pcp2dptc;
    uint32  field_val32 = 0, count = 0;
    int  index = 0;
#endif
    pbmp_t      t_pbm;

    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_init()..\n");
    /* 
     *  Setting default queue number 
     */
    num_cos = soc_property_get(unit, spn_BCM_NUM_COS, _BCM_COS_DEFAULT(unit));

    if (num_cos < 1) {
        num_cos = 1;
    } else if (num_cos > NUM_COS(unit)) {
        num_cos = NUM_COS(unit);
    }

    BCM_IF_ERROR_RETURN(bcm_cosq_config_set(unit, num_cos));

    /* Qos 1P Enable */
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_ASSIGN(t_pbm, PBMP_ALL(unit));
    BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_SET
                    (unit, t_pbm, 
                    DRV_QUEUE_MAP_PRIO, TRUE));
    /* 
     *  Setting default priority mapping 
     */
    for (prio = 0; prio <= _BCM_PRIO_MAX(unit); prio++) {
        map_queue = prio / ((int)((_BCM_PRIO_MAX(unit)+1)/num_cos));
        BCM_IF_ERROR_RETURN(bcm_cosq_mapping_set(unit, prio, map_queue));
    }

    /* 
     * Setting default Priority Threshold and the queuing mode
     */
    /* this section is not implemented in Robo5324/5338 */
    
    /* Setting default mapping of reason code and cosq, this feature so far 
     * is supported on bcm5395/53115 only
     */
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        /* default mapping on reason_code will be :
         *  1. Mirroring, Default: 0
         *  2. SA Learning, Default: 0
         *  3. Switching /Flooding, Default: 1
         *  4. Protocol Termination, Default: 3
         *  5. Protocol Snooping, Default: 2
         *  6. Exception Processing, Default: 2 
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_MIRRORING, 0));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_SA_LEARNING, 0));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_SWITCHING, 1));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_MIRRORING, 3));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_PROTO_SNOOP, 2));
        BCM_IF_ERROR_RETURN(DRV_QUEUE_RX_REASON_SET
                (unit, DRV_RX_REASON_EXCEPTION, 2));
    }
    
#ifdef BCM_TB_SUPPORT
    sal_memset(&entry_pcp2dptc, 0, sizeof (entry_pcp2dptc));
    if (SOC_IS_TBX(unit)) {
        /* Initialize DP = DP1 in 1P2TCDP and DSCP2TCDP mapping */
        BCM_PBMP_CLEAR(t_pbm); 
        BCM_PBMP_ASSIGN(t_pbm, PBMP_ALL(unit));

        dp = _BCM_COLOR_ENCODING(unit, bcmColorGreen);
        prio = BCM_PRIO_GREEN;

        /* Initialize Port n Default Port QoS Configuration for DP = DP1 */
        BCM_PBMP_ITER(t_pbm, port) {
            BCM_IF_ERROR_RETURN(bcm_port_control_set
                (unit, port, bcmPortControlDropPrecedence, dp));
        }

        /* Initialize DP = DP1 in 1P2TCDP mapping */
        index = 0;
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
            (unit, DRV_MEM_1P_TO_TCDP, (uint32)index, 1, (uint32 *)&entry_pcp2dptc));

        field_val32 = (uint32)dp;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_1P_TO_TCDP, DRV_MEM_FIELD_DP,
            (uint32 *)&entry_pcp2dptc, (uint32 *)&field_val32));

        BCM_IF_ERROR_RETURN(DRV_MEM_LENGTH_GET
            (unit, DRV_MEM_1P_TO_TCDP, (uint32 *)&count));

        BCM_IF_ERROR_RETURN(DRV_MEM_FILL
            (unit, DRV_MEM_1P_TO_TCDP, 
            (uint32)index, count, (uint32 *)&entry_pcp2dptc));

        /* Initialize DP = DP1 in DSCP2TCDP mapping (Global) */
        port = 0;
        for (srccp = 0 ; srccp <= 63 ; srccp++) {
            BCM_IF_ERROR_RETURN(bcm_port_dscp_map_set
                (unit, port, srccp, srccp, prio));
        }

        /* Allocate and update sw shadow */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                (unit, DRV_QOS_CTL_SW_SHADOW, TRUE));
    }
#endif
    
    return BCM_E_NONE;

}

/*
 * Function:
 *      bcm_robo_cosq_config_set
 * Purpose:
 *      Set the number of COS queues
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      numq - number of COS queues (2, 3, or 4).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_config_set(int unit, bcm_cos_queue_t numq)
{   
    uint8   drv_value;
    
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_config_set()..\n");

    if (!BCM_COSQ_QUEUE_VALID(unit, numq - 1)) {
        return (BCM_E_PARAM);
    }
    
    drv_value = numq;
    BCM_IF_ERROR_RETURN(DRV_QUEUE_COUNT_SET
                    (unit, 0, drv_value));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_config_get
 * Purpose:
 *      Get the number of cos queues
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      numq - (Output) number of cosq
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_config_get(int unit, bcm_cos_queue_t *numq)
{

    uint8   drv_value;
    
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_config_get()..\n");
    BCM_IF_ERROR_RETURN(DRV_QUEUE_COUNT_GET
                    (unit, 0, &drv_value));
                    
    *numq = (bcm_cos_queue_t)drv_value;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_mapping_set
 * Purpose:
 *      Set which cosq a given priority should fall into
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      priority - Priority value to map
 *      cosq - COS queue to map to
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_mapping_set(int unit, bcm_cos_t priority, bcm_cos_queue_t cosq)
{
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_mapping_set()..\n");
    if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
        return (BCM_E_PARAM);
    }
    
    if (!_BCM_COSQ_PRIO_VALID(unit, priority)) {
        return (BCM_E_PARAM);
    }
    
    BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_SET
                (unit, -1, priority, cosq));

    return (BCM_E_NONE);
}
                
/*
 * Function:
 *      bcm_robo_cosq_mapping_get
 * Purpose:
 *      Determine which COS queue a given priority currently maps to.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      priority - Priority value
 *      cosq - (Output) COS queue number
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_cosq_mapping_get(int unit, bcm_cos_t priority, bcm_cos_queue_t *cosq)
{
    uint8       t_cosq = 0;
    
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_mapping_get()..\n");

    if (!_BCM_COSQ_PRIO_VALID(unit, priority)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_GET
                (unit, -1, priority, &t_cosq));

    *cosq = t_cosq;
    return (BCM_E_NONE);
}

int
bcm_robo_cosq_port_mapping_set(int unit, bcm_port_t port,
                   bcm_cos_t priority, bcm_cos_queue_t cosq)
{
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_port_mapping_set()..\n");
    if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
        return (BCM_E_PARAM);
    }
    
    if (!_BCM_COSQ_PRIO_VALID(unit, priority)) {
        return (BCM_E_PARAM);
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    } else {
        if (port != -1 && !SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    }
    
    BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_SET
                (unit, port, priority, cosq));

    return (BCM_E_NONE);
}
                
int
bcm_robo_cosq_port_mapping_get(int unit, bcm_port_t port,
                   bcm_cos_t priority, bcm_cos_queue_t *cosq)
{
    uint8       t_cosq = 0;
    
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_port_mapping_get()..\n");

    if (!_BCM_COSQ_PRIO_VALID(unit, priority)) {
        return (BCM_E_PARAM);
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    } else {
        if (port != -1 && !SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    }

    BCM_IF_ERROR_RETURN(DRV_QUEUE_PRIO_GET
                (unit, port, priority, &t_cosq));

    *cosq = t_cosq;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_cosq_port_bandwidth_get
 * Purpose:
 *      Retrieve bandwidth values for given COS policy.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *  port - port to configure, -1 for any port.
 *      cosq - COS queue to configure, -1 for any COS queue.
 *      kbits_sec_min - (OUT) minimum bandwidth, kbits/sec.
 *      kbits_sec_max - (OUT) maximum bandwidth, kbits/sec.
 *      flags - (OUT) may include:
 *              BCM_COSQ_BW_EXCESS_PREF
 *              BCM_COSQ_BW_MINIMUM_PREF
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This API is designed mainly for Tucana Chip and not suitable
 *      for Robo Chip.
 *
 *      Per queue egress rate control is implemented in this API
 *      for Robo chips. 
 *      Egress rate control need 2 parameters, rate limit and 
 *      bucket size. Although the meaning is not exactly the same,
 *      the kbits_sec_min and kbits_sec_max parameters of this API 
 *      are used to represent the rate limit and bucket size, respectively.
 *
 *      For TB Chip, the kbits_sec_min and kbits_sec_max parameters
 *      are used to represent the minimum and maximum rate limit, respectively.
 *      And the bucket size is the same as max rate limit in this API.
 */
int 
bcm_robo_cosq_port_bandwidth_get(int unit, bcm_port_t port,
                                   bcm_cos_queue_t cosq,
                                   uint32 *kbits_sec_min,
                                   uint32 *kbits_sec_max,
                                   uint32 *flags)
{
    uint32 temp, temp2;
    bcm_port_t loc_port;
    bcm_cos_queue_t loc_cos;
    uint32 kbits_sec_burst;    /* Dummy variable */
    
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_port_bandwidth_get()\n");

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (port < 0) {
        loc_port = SOC_PORT_MIN(unit,all);
    } else {
        if (BCM_GPORT_IS_SET(port)) {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
        } else {
            loc_port = port;
        }

        if (!SOC_PORT_VALID(unit, loc_port)) {
            return BCM_E_PORT; 
        }
    }

    if (cosq < 0) {
        loc_cos = 0;
    } else {
        loc_cos = cosq;
    }

    switch (loc_cos) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
        case 7:
            BCM_IF_ERROR_RETURN(DRV_RATE_GET
                                (unit, loc_port, loc_cos,
                                DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                kbits_sec_min, kbits_sec_max, &kbits_sec_burst));
            break;
        case 4:
            if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                BCM_IF_ERROR_RETURN(DRV_RATE_GET
                                    (unit, loc_port, loc_cos,
                                    DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                    kbits_sec_min, kbits_sec_max, &kbits_sec_burst));
#endif
            } else if (soc_feature(unit, soc_feature_eav_support)) {
                /* Get current Macro Slot time */
                BCM_IF_ERROR_RETURN(DRV_EAV_TIME_SYNC_GET
                    (unit, DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD, &temp, &temp2));
                /*
                 * kbit/sec =(bytes/slot time) * 8 * 1000 / (macro slot time * 1024)
                 */
                BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
                    (unit, loc_port, DRV_EAV_QUEUE_Q4_BANDWIDTH, &temp2));
                *kbits_sec_min = (temp2 * 8 * 1000) / (temp * 1024); 

                    /* unused parameter kbits_sec_max : return (-1) */
                    *kbits_sec_max = -1;
                    *flags = 0;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case 5:
            if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                BCM_IF_ERROR_RETURN(DRV_RATE_GET
                                    (unit, loc_port, loc_cos,
                                    DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                    kbits_sec_min, kbits_sec_max, &kbits_sec_burst));
#endif
            } else if (soc_feature(unit, soc_feature_eav_support)) {
                /*
                 * Class 5 slot time is 125 us.
                 * kbits/sec = (bytes/slot * 8 * 8000) / 1024
                 */
                BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
                    (unit, loc_port, DRV_EAV_QUEUE_Q5_BANDWIDTH, &temp));
                *kbits_sec_min = (temp * 8 * 8000) / 1024;

                /* unused parameter kbits_sec_max : return (-1) */
                *kbits_sec_max = -1;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_port_bandwidth_set
 * Purpose:
 *      Set bandwidth values for given COS policy.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *  port - port to configure, -1 for all ports.
 *      cosq - COS queue to configure, -1 for all COS queues.
 *      kbits_sec_min - minimum bandwidth, kbits/sec.
 *      kbits_sec_max - maximum bandwidth, kbits/sec.
 *      flags - may include:
 *              BCM_COSQ_BW_EXCESS_PREF
 *              BCM_COSQ_BW_MINIMUM_PREF
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This API is designed mainly for Tucana Chip and not suitable
 *      for Robo Chip.
 *
 *      Per queue egress rate control is implemented in this API
 *      for Robo chips. 
 *      Egress rate control need 2 parameters, rate limit and 
 *      bucket size. Although the meaning is not exactly the same,
 *      the kbits_sec_min and kbits_sec_max parameters of this API 
 *      are used to represent the rate limit and bucket size, respectively.
 *
 *      For TB Chip, the kbits_sec_min and kbits_sec_max parameters
 *      are used to represent the minimum and maximum rate limit, respectively.
 *      And the bucket size is the same as max rate limit in this API.
 */
int 
bcm_robo_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                                   bcm_cos_queue_t cosq,
                                   uint32 kbits_sec_min,
                                   uint32 kbits_sec_max,
                                   uint32 flags)
{
    uint32 temp, temp2;
    bcm_pbmp_t pbmp;
    bcm_port_t loc_port;
    int i;
    
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_port_bandwidth_set()\n");

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (port < 0) {
        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
        if (BCM_GPORT_IS_SET(port)) {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
        }
        if (SOC_PORT_VALID(unit, port)) {
            BCM_PBMP_PORT_SET(pbmp, port);
        } else {
            return BCM_E_PORT; 
        }
    }

    if (cosq < 0) {
        for (i = 0; i < NUM_COS(unit) - 1; i++) {
            BCM_IF_ERROR_RETURN(DRV_RATE_SET
                                (unit, pbmp, i,
                                DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                kbits_sec_min, kbits_sec_max, kbits_sec_max));
        }
        return BCM_E_NONE;
    }

    switch (cosq) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
        case 7:
            BCM_IF_ERROR_RETURN(DRV_RATE_SET
                                (unit, pbmp, cosq,
                                DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                kbits_sec_min, kbits_sec_max, kbits_sec_max));
            break;
        case 4:
            if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                BCM_IF_ERROR_RETURN(DRV_RATE_SET
                                    (unit, pbmp, cosq,
                                    DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                    kbits_sec_min, kbits_sec_max, kbits_sec_max));
#endif
            } else if (soc_feature(unit, soc_feature_eav_support)) {
                
                 /* Check the maximum valid bandwidth value for EAV Class 4 */
                BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
                    (unit, port, DRV_EAV_QUEUE_Q4_BANDWIDTH_MAX_VALUE, &temp));

                if (kbits_sec_min > temp) {
                    soc_cm_debug(DK_ERR, 
                        "bcm_robo_cosq_port_bandwidth_set : BW value unsupported. \n");
                    return  SOC_E_PARAM;
                }
                /* Get current Macro Slot time */
                BCM_IF_ERROR_RETURN(DRV_EAV_TIME_SYNC_GET
                    (unit, DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD, &temp, &temp2));
                /*
                 * bytes/slot = kbits/sec * 1024 / (8 * macro slot time * 1000)
                 */
                temp2 = (kbits_sec_min * 1024 * temp) / (8 * 1000);
                BCM_PBMP_ITER(pbmp, loc_port) {
                    BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_SET
                        (unit, loc_port, DRV_EAV_QUEUE_Q4_BANDWIDTH, temp2));
                }
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case 5:
            if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                BCM_IF_ERROR_RETURN(DRV_RATE_SET
                                    (unit, pbmp, cosq,
                                    DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE,
                                    kbits_sec_min, kbits_sec_max, kbits_sec_max));
#endif
             } else if (soc_feature(unit, soc_feature_eav_support)) {

                 /* Check the maximum valid bandwidth value for EAV Class 5 */
                BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
                    (unit, port, DRV_EAV_QUEUE_Q5_BANDWIDTH_MAX_VALUE, &temp));

                if (kbits_sec_min > temp) {
                    soc_cm_debug(DK_ERR, 
                        "bcm_robo_cosq_port_bandwidth_set : BW value unsupported. \n");
                    return  SOC_E_PARAM;
                }
                /*
                 * Class 5 slot time is 125 us.
                 * bytes/125us = kbit/sec * 1024 /(8 * 8000) 
                 */
                temp = (kbits_sec_min * 1024) / (8 * 8000); 
                BCM_PBMP_ITER(pbmp, loc_port) {
                    BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_SET
                        (unit, loc_port, DRV_EAV_QUEUE_Q5_BANDWIDTH, temp));
                }
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_sched_weight_max_get
 * Purpose:
 *      Retrieve maximum weights for given COS policy.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *  weight_max - (output) Maximum weight for COS queue.
 *      0 if mode is BCM_COSQ_STRICT.
 *      1 if mode is BCM_COSQ_ROUND_ROBIN.
 *      -1 if not applicable to mode.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_cosq_sched_weight_max_get(int unit, int mode,
                     int *weight_max)
{
    uint32 prop_val = 0;
    
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_sched_weight_max_get()..\n");
    switch (mode) {
    case BCM_COSQ_STRICT:
        *weight_max = BCM_COSQ_WEIGHT_STRICT;
        break;
    case BCM_COSQ_DEFICIT_ROUND_ROBIN:
    case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                        (unit, DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE, &prop_val));
        *weight_max = (int)prop_val;
        break;
    default:
        *weight_max = BCM_COSQ_WEIGHT_UNLIMITED;
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

int
bcm_robo_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
            int mode, const int weights[], int delay)
{
    uint32      drv_value, weight_value;
    int         i, sp_num;
    bcm_pbmp_t pbm_all = PBMP_ALL(unit);
    
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_port_sched_set()..\n");

    if (BCM_PBMP_IS_NULL(pbm)) {
        return BCM_E_PORT;
    }

    BCM_PBMP_AND(pbm_all, pbm);
    if (BCM_PBMP_NEQ(pbm_all, pbm)) {
        return BCM_E_PORT;
    }
    
    /* for Robo Chip, we support Strict, WRR and WDRR mode */
    drv_value = (mode == BCM_COSQ_STRICT) ? DRV_QUEUE_MODE_STRICT : 
                (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN) ? DRV_QUEUE_MODE_WRR :
                (mode == BCM_COSQ_DEFICIT_ROUND_ROBIN) ? 
                DRV_QUEUE_MODE_WDRR : 0;
    
    /* the COSQ mode for Robo allowed Strict & WRR mode only */
    if (drv_value == 0){
        return BCM_E_PARAM;
    }

    if (mode == BCM_COSQ_STRICT) {
        BCM_IF_ERROR_RETURN(DRV_QUEUE_MODE_SET
                        (unit, pbm, drv_value));
        return BCM_E_NONE;
    }

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        /* Now only TB support WDRR mode */
        /* set the weight if mode is WDRR */
        sp_num = 0;
        for (i = 0; i < NUM_COS(unit); i++) {
            weight_value = weights[i];
            if (weight_value == BCM_COSQ_WEIGHT_STRICT) {
                sp_num ++;
            }
        }

        /* Check the scheduler configuration selections for combination of SP and WDRR */
        if (sp_num >= 5) {
            return BCM_E_PARAM;
        }

        /* Set scheduling combination mode if ROBO chip support it */
        if (sp_num == 1) { 
             /* 1STRICT/7WDRR : COSQ7 = STRICT */
            if (weights[NUM_COS(unit) - 1] == BCM_COSQ_WEIGHT_STRICT) {
                drv_value = DRV_QUEUE_MODE_1STRICT_7WDRR;
            } else {
                return BCM_E_PARAM;
            }
        } else if ((sp_num == 2)) { 
            /* 2STRICT/6WDRR : COSQ7 = COSQ6 = STRICT */
            if ((weights[NUM_COS(unit) - 1] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 2] == BCM_COSQ_WEIGHT_STRICT)) {
                drv_value = DRV_QUEUE_MODE_2STRICT_6WDRR;
            } else {
                return BCM_E_PARAM;
            }
        } else if ((sp_num == 3)) { 
            /* 3STRICT/5WDRR : COSQ7 = COSQ6 = COSQ5 = STRICT */
            if ((weights[NUM_COS(unit) - 1] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 2] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 3] == BCM_COSQ_WEIGHT_STRICT)) {
                drv_value = DRV_QUEUE_MODE_3STRICT_5WDRR;
            } else {
                return BCM_E_PARAM;
            }
        } else if ((sp_num == 4)) { 
            /* 4STRICT/4WDRR : COSQ7 = COSQ6 = COSQ5 = COSQ4 = STRICT */
            if ((weights[NUM_COS(unit) - 1] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 2] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 3] == BCM_COSQ_WEIGHT_STRICT) &&
                 (weights[NUM_COS(unit) - 4] == BCM_COSQ_WEIGHT_STRICT)) {
                drv_value = DRV_QUEUE_MODE_4STRICT_4WDRR;
            } else {
                return BCM_E_PARAM;
            }
        }

        for (i = 0; i < NUM_COS(unit); i++) {
            weight_value = weights[i];
            if (weight_value != BCM_COSQ_WEIGHT_STRICT) {
                BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_SET
                    (unit, 0, i, weight_value));
            }
        }
#endif
    } else {
        if (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN) {
            /* set the weight if mode is WRR */
            sp_num = 0;
            for (i = 0; i < NUM_COS(unit); i++){
                weight_value = weights[i];
                /* COSQ3 or COSQ2 is Strict priority if ROBO chip support STRICT/WRR scheduler */
                if (((i == NUM_COS(unit) - 1) || (i == NUM_COS(unit) - 2)) && 
                    (weight_value == BCM_COSQ_WEIGHT_STRICT)) {
                    sp_num ++;
                } else {
                        BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_SET
                                    (unit, 0, i, weight_value));
                }
            }
        
            /* Set scheduling combination mode if ROBO chip support it */
            if (sp_num == 1) { /* 1STRICT/3WRR : COSQ3>COS2/COS1/COS0 */
               if (weights[NUM_COS(unit) - 1] == BCM_COSQ_WEIGHT_STRICT) {
                   drv_value = DRV_QUEUE_MODE_1STRICT_3WRR;
               } else {
                   return BCM_E_PARAM;
               }
            } else if ((sp_num == 2)) { /* 2STRICT/2WRR : COSQ3>COS2>COS1/COS0 */
                drv_value = DRV_QUEUE_MODE_2STRICT_2WRR;
            }
        } else {
            return BCM_E_UNAVAIL;
        }
    }

    BCM_IF_ERROR_RETURN(DRV_QUEUE_MODE_SET
                    (unit, pbm,drv_value));    
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_sched_set
 * Purpose:
 *      Set up class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *  weights - Weights for each COS queue
 *      Unused if mode is BCM_COSQ_STRICT.
 *      Indicates number of packets sent before going on to
 *      the next COS queue.
 *  delay - Maximum delay in microseconds before returning the
 *      round-robin to the highest priority COS queue
 *      (Unused if mode other than BCM_COSQ_BOUNDED_DELAY)
 * Returns:
 *      BCM_E_XXX
 * Note :
 *      1. Not recommend user to set queue threshold at 
 *          Robo5338/5324/5380/5388.
 *      2. the Strict Mode in RobSwitch actually strict at the higest 
 *          Queue only. the other lower queues will still working at 
 *          WRR mode if set BCM_COSQ_STRICT.
 */

int
bcm_robo_cosq_sched_set(int unit, int mode, const int weights[], int delay)
{
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_sched_set()..\n");
    /* no CPU port been assigned in this API */
    return (bcm_cosq_port_sched_set(unit,
                            PBMP_ALL(unit),
                            mode, weights, delay));
}


int
bcm_robo_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
                int *mode, int weights[], int *delay)
{
    uint32      drv_value = 0;
    int         i, port;
    bcm_pbmp_t pbm_all = PBMP_ALL(unit);
    
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_port_sched_get()..\n");

    if (BCM_PBMP_IS_NULL(pbm)) {
        return BCM_E_PORT;
    }

    BCM_PBMP_AND(pbm_all, pbm);
    if (BCM_PBMP_NEQ(pbm_all, pbm)) {
        return BCM_E_PORT;
    }
    
    /* get the cosq schedule at the first port only in pbm */
    BCM_PBMP_ITER(pbm, port){
        BCM_IF_ERROR_RETURN(DRV_QUEUE_MODE_GET
                        (unit, port, &drv_value));
        break;
    }
            
    /* Robo Chip shuld have strict, WRR or WDRR mode only */
    *mode = (drv_value == DRV_QUEUE_MODE_STRICT) ? BCM_COSQ_STRICT : 
            (drv_value == DRV_QUEUE_MODE_1STRICT_3WRR) ? BCM_COSQ_WEIGHTED_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_2STRICT_2WRR) ? BCM_COSQ_WEIGHTED_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_1STRICT_7WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_2STRICT_6WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_3STRICT_5WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_4STRICT_4WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_WDRR) ? BCM_COSQ_DEFICIT_ROUND_ROBIN :
            (drv_value == DRV_QUEUE_MODE_WRR) ? BCM_COSQ_WEIGHTED_ROUND_ROBIN : -1;

    if (*mode == -1) {
        return BCM_E_INTERNAL;
    }
            
    /* get the weight if mode is WRR or WDRR */
    for (i = 0; i < NUM_COS(unit); i++){
        BCM_IF_ERROR_RETURN(DRV_QUEUE_WRR_WEIGHT_GET
                        (unit, 0, i, &drv_value));
                        
        weights[i] = drv_value;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_cosq_sched_get
 * Purpose:
 *      Retrieve class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      mode_ptr - (output) Scheduling mode, one of BCM_COSQ_xxx
 *  weights - (output) Weights for each COS queue
 *      Unused if mode is BCM_COSQ_STRICT.
 *  delay - (output) Maximum delay in microseconds before returning
 *      the round-robin to the highest priority COS queue
 *      Unused if mode other than BCM_COSQ_BOUNDED_DELAY.
 * Returns:
 *      BCM_E_XXX
 * Note :
 *      1. Not recommend user to set queue threshold at 
 *          Robo5338/5324/5380/5388.
 *      2. the Strict Mode in RobSwitch actually strict at the higest 
 *          Queue only. the other lower queues will still working at 
 *          WRR mode if set BCM_COSQ_STRICT.
 */

int
bcm_robo_cosq_sched_get(int unit, int *mode, int weights[], int *delay)
{   
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_sched_get()..\n");
    /* no CPU port been assigned in this API */
    return (bcm_cosq_port_sched_get(unit,
                            PBMP_ALL(unit),
                            mode, weights, delay));
}

int
bcm_robo_cosq_discard_set(int unit, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_cosq_discard_get(int unit, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_cosq_discard_port_set(int unit, bcm_port_t port,
                                 bcm_cos_queue_t cosq,
                                 uint32 color,
                                 int drop_start,
                                 int drop_slope,
                                 int average_time)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_cosq_discard_port_get(int unit, bcm_port_t port,
                                 bcm_cos_queue_t cosq,
                                 uint32 color,
                                 int *drop_start,
                                 int *drop_slope,
                                 int *average_time)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_cosq_control_set
 * Purpose:
 *      Set specified feature configuration
 *
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID.
 *      cosq - (IN) COS queue.
 *      type - (IN) feature
 *      arg  - (IN) feature value
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      1. This API only supported for TB on ROBO chips now.
 *      2. The port and cosq parameters are not used for cosq control types as system based:
 *          - bcmCosqControlCopyPktToCpuUseTC
 *          - bcmCosqControlWdrrGranularity
 *          - bcmCosqControlDpValueDlf
 *          - bcmCosqControlDpChangeDlf
 *          - bcmCosqControlDpChangeXoff
 */
int
bcm_robo_cosq_control_set(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                                           bcm_cosq_control_t type, int arg)
{
    bcm_pbmp_t pbmp;
    uint32  temp = 0, reg_val = 0;

    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_control_set()..\n");

    BCM_PBMP_CLEAR(pbmp);

    if ((type == bcmCosqControlPortTxqBackpressure) ||
         (type == bcmCosqControlEgressRateBurstAccumulateControl) ||
         (type == bcmCosqControlEgressRateType) ||
         (type == bcmCosqControlEavClassAWindow)) {
        /* Per-port based cosq control */
        BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port));
    } else if ((type == bcmCosqControlCopyPktToCpuUseTC) ||
        (type == bcmCosqControlWdrrGranularity) ||
        (type == bcmCosqControlDpValueDlf) ||
        (type == bcmCosqControlDpChangeDlf) ||
        (type == bcmCosqControlDpChangeXoff)) {
        /* System based cosq control */
        if ((port != -1) || (cosq != -1)) {
            return BCM_E_PARAM;
        }
    } else if (type == bcmCosqControlEEETxQCongestionThreshold) {
        /* Per-queue based cosq control */
        if (port != -1) {
            return BCM_E_PARAM;
        }
    }

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        switch (type) {
            case bcmCosqControlPortTxqBackpressure:
                /* check port is IMP port or GE ports */
                if ((!IS_GE_PORT(unit, port)) && (!IS_CPU_PORT(unit, port))) {
                    return BCM_E_PARAM;
                }
            
                if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
                    return (BCM_E_PARAM);
                }
        
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_PORT_TXQ_PAUSE_SET
                    (unit, port, cosq, (uint8)temp));
                break;
            case bcmCosqControlCopyPktToCpuUseTC:
                /* this type value definition is :
                 *  1: Use generic TC based COS mapping for copying packets to CPU.
                 *  0: Use Reason basd COS mapping  for copying packets to CPU.
                 */
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                    (unit, DRV_QOS_CTL_USE_TC, temp));
                break;
            case bcmCosqControlWdrrGranularity:
                /* this type value definition is :
                 *  1: number of packet.
                 *  0: number of 64-bytes (64-byte: TBD).
                 */
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                    (unit, DRV_QOS_CTL_WDRR_GRANULARTTY, temp));
                break;
            case bcmCosqControlDpValueDlf:
                /* this type value definition is :
                 *  DP_CTRL[3:2] value of the unknown unicast/multicast packet (0~ 3)
                 */
                temp = (uint32)arg;
                BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                    (unit, DRV_QOS_CTL_DP_VALUE_DLF, _BCM_COLOR_ENCODING(unit, temp)));
                break;
            case bcmCosqControlDpChangeDlf:
                /* this type value definition is :
                 *  1: DP=DLF_DP (DP_CTRL[3:2]) if the packet is a DLF packet.
                 *  0: DP is depended on the setting of default port DP.
                 */
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                    (unit, DRV_QOS_CTL_DP_CHANGE_DLF, temp));
                break;
            case bcmCosqControlDpChangeXoff:
                /* this type value definition is :
                 *  1: DP=DP0, if the port is flow-controllable port.
                 *  0: DP is depended on the setting of DLF DP or the default port DP.
                 */
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_SET
                    (unit, DRV_QOS_CTL_DP_CHANGE_XOFF, temp));
                break;
            case bcmCosqControlEgressRateBurstAccumulateControl:
                /* this type value definition is :
                 *  1: Enable accumulation.
                 *  0: To indicate the value of the Burst Tolerance Size should be reset to zero 
                 *       when there is no packet in the queue waiting to be transmitted.
                 */
                if (cosq != -1) {
                    return BCM_E_PARAM;
                }
        
                BCM_PBMP_PORT_SET(pbmp, port);
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
                    (unit, pbmp, DRV_RATE_CONFIG_RATE_BAC, temp));
                break;
            case bcmCosqControlEgressRateType:
                /* this type value definition is :
                 *  1: pps
                 *  0: kbps
                 */
                if (cosq != -1) {
                    return BCM_E_PARAM;
                }
        
                BCM_PBMP_PORT_SET(pbmp, port);
                if (arg) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                BCM_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET
                    (unit, pbmp, DRV_RATE_CONFIG_RATE_TYPE, temp));
                break;
            default:
                return BCM_E_UNAVAIL;;
        }

        return BCM_E_NONE;
#endif /* BCM_TB_SUPPORT */    
    }

    if (type == bcmCosqControlEavClassAWindow) {
        /* 
         * 1: Enable the jitter control of EAV class A traffic 
         * 0: Disable the jitter control of EAV class A traffic
         */
        if (arg) {
            temp = 1;
        } else {
            temp = 0;
        }
        BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_SET
            (unit, port, DRV_EAV_QUEUE_Q5_WINDOW, temp));

        return BCM_E_NONE;
    }

    if ((type == bcmCosqControlEEETxQCongestionThreshold) &&
        soc_feature (unit, soc_feature_eee)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_EEE_TXQ_CONG_THr(unit, cosq, &reg_val));
        temp = arg;
        soc_EEE_TXQ_CONG_THr_field_set(unit, &reg_val, 
            TXQ_CONG_THf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_EEE_TXQ_CONG_THr(unit, cosq, &reg_val));
    }

    return BCM_E_UNAVAIL;;
}

/*
 * Function:
 *      bcm_robo_cosq_control_get
 * Purpose:
 *      Get specified feature configuration
 *
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID.
 *      cosq - (IN) COS queue.
 *      type - (IN) feature
 *      arg  - (OUT) feature value
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      1. This API only supported for TB on ROBO chips now.
 *      2. The port and cosq parameters are not used for cosq control types as system based:
 *          - bcmCosqControlCopyPktToCpuUseTC
 *          - bcmCosqControlWdrrGranularity
 *          - bcmCosqControlDpValueDlf
 *          - bcmCosqControlDpChangeDlf
 *          - bcmCosqControlDpChangeXoff
 */
int
bcm_robo_cosq_control_get(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                                          bcm_cosq_control_t type, int *arg)
{
    uint32  temp = 0, reg_val = 0;

    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_cosq_control_get()..\n");

    if ((type == bcmCosqControlPortTxqBackpressure) ||
         (type == bcmCosqControlEgressRateBurstAccumulateControl) ||
         (type == bcmCosqControlEgressRateType) ||
         (type == bcmCosqControlEavClassAWindow)) {
        /* Per-port based cosq control */
        BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port));
    } else if ((type == bcmCosqControlCopyPktToCpuUseTC) ||
        (type == bcmCosqControlWdrrGranularity) ||
        (type == bcmCosqControlDpValueDlf) ||
        (type == bcmCosqControlDpChangeDlf) ||
        (type == bcmCosqControlDpChangeXoff)) {
        /* System based cosq control */        
        if ((port != -1) || (cosq != -1)) {
            return BCM_E_PARAM;
        }
    } else if (type == bcmCosqControlEEETxQCongestionThreshold) {
        /* Per-queue based cosq control */
        if (port != -1) {
            return BCM_E_PARAM;
        }
    }

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT        
    switch (type) {
    case bcmCosqControlPortTxqBackpressure:
        /* check port is IMP port or GE ports */
        if ((!IS_GE_PORT(unit, port)) && (!IS_CPU_PORT(unit, port))) {
            return BCM_E_PARAM;
        }
    
        if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
            return (BCM_E_PARAM);
        }
        BCM_IF_ERROR_RETURN(DRV_QUEUE_PORT_TXQ_PAUSE_GET
            (unit, port, cosq, (uint8 *)&temp));
        break;
    case bcmCosqControlCopyPktToCpuUseTC:
        /* this type value definition is :
         *  1: Use generic TC based COS mapping for copying packets to CPU.
         *  0: Use Reason basd COS mapping  for copying packets to CPU.
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
            (unit, DRV_QOS_CTL_USE_TC, &temp));
        break;
    case bcmCosqControlWdrrGranularity:
        /* this type value definition is :
         *  1: number of packet.
         *  0: number of 64-bytes (64-byte: TBD).
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
            (unit, DRV_QOS_CTL_WDRR_GRANULARTTY, &temp));
        break;
    case bcmCosqControlDpValueDlf:
        /* this type value definition is :
         *  DP_CTRL[3:2] value of the unknown unicast/multicast packet (0~ 3)
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
            (unit, DRV_QOS_CTL_DP_VALUE_DLF, &temp));
        break;
    case bcmCosqControlDpChangeDlf:
        /* this type value definition is :
         *  1: DP=DLF_DP (DP_CTRL[3:2]) if the packet is a DLF packet.
         *  0: DP is depended on the setting of default port DP.
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
            (unit, DRV_QOS_CTL_DP_CHANGE_DLF, &temp));
        break;
    case bcmCosqControlDpChangeXoff:
        /* this type value definition is :
         *  1: DP=DP0, if the port is flow-controllable port.
         *  0: DP is depended on the setting of DLF DP or the default port DP.
         */
        BCM_IF_ERROR_RETURN(DRV_QUEUE_QOS_CONTROL_GET
            (unit, DRV_QOS_CTL_DP_CHANGE_XOFF, &temp));
        break;
    case bcmCosqControlEgressRateBurstAccumulateControl:
        /* this type value definition is :
         *  1: Enable accumulation.
         *  0: To indicate the value of the Burst Tolerance Size should be reset to zero 
         *       when there is no packet in the queue waiting to be transmitted.
         */
        if (cosq != -1) {
            return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
            (unit, port, DRV_RATE_CONFIG_RATE_BAC, &temp));
        break;
    case bcmCosqControlEgressRateType:
        /* this type value definition is :
         *  1: pkt / sec (100 pkts/sec).
         *  0: bit / sec (64 * 1000 bits/sec).
         */
        if (cosq != -1) {
            return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN(DRV_RATE_CONFIG_GET
            (unit, port, DRV_RATE_CONFIG_RATE_TYPE, &temp));
        break;
    default:
        return BCM_E_UNAVAIL;;
    }

    *arg = temp;

    return BCM_E_NONE;
#endif /* BCM_TB_SUPPORT */    
    }

    if (type == bcmCosqControlEavClassAWindow) {
        /* 
         * 1: Enable the jitter control of EAV class A traffic 
         * 0: Disable the jitter control of EAV class A traffic
         */
        BCM_IF_ERROR_RETURN(DRV_EAV_QUEUE_CONTROL_GET
            (unit, port, DRV_EAV_QUEUE_Q5_WINDOW, &temp));

        if (temp) {
            *arg = 1;
        } else {
            *arg = 0;
        }

        return BCM_E_NONE;
    }

    if ((type == bcmCosqControlEEETxQCongestionThreshold) &&
        soc_feature (unit, soc_feature_eee)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_EEE_TXQ_CONG_THr(unit, cosq, &reg_val));
        soc_EEE_TXQ_CONG_THr_field_get(unit, &reg_val, 
            TXQ_CONG_THf, &temp);
        *arg = temp;
    }

    return BCM_E_UNAVAIL;;
}
