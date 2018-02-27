/*
 * $Id: cosq.c 1.81.2.12 Broadcom SDK $
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

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>

#include <bcm/error.h>
#include <bcm/cosq.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/scorpion.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/triumph2.h>
#include <bcm_int/esw/trident.h>
#include <bcm_int/esw/hurricane.h>
#include <bcm_int/esw/katana.h>

#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/stat.h>
#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/switch.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#define BCM_COSQ_QUEUE_VALID(unit, numq) \
	((numq) >= 0 && (numq) < NUM_COS(unit))


#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_esw_cosq_sync
 * Purpose:
 *      Record COSq module persisitent info for Level 2 Warm Boot.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_cosq_sync(int unit)
{
    return mbcm_driver[unit]->mbcm_cosq_sync(unit);
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *      _bcm_esw_cosq_config_property_get
 * Purpose:
 *      Get number COSQ from SOC property.
 * Parameters:
 *      unit - Unit number.
 * Returns:
 *      Number COSQ from SOC property.
 */
int
_bcm_esw_cosq_config_property_get(int unit)
{
    int  num_cos;

#if defined (BCM_HAWKEYE_SUPPORT)
    if (SOC_IS_HAWKEYE(unit)) {
        /* Use 2 as the default number of COS queue for HAWKEYE, 
           if bcm_num_cos is not set in config.bcm */
        num_cos = soc_property_get(unit, spn_BCM_NUM_COS, 2);
    } else 
#endif /* BCM_HAWKEYE_SUPPORT */

#if defined (BCM_HURRICANE_SUPPORT)
    if (SOC_IS_HURRICANE(unit)) {
        /* Use 8 as the default number of COS queue for HURRICANE, 
           if bcm_num_cos is not set in config.bcm */
        num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);
    } else 
#endif /* BCM_HURRICANE_SUPPORT */
    {
        num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_DEFAULT);
    }

    if (num_cos < 1) {
        num_cos = 1;
    } else if (num_cos > NUM_COS(unit)) {
        num_cos = NUM_COS(unit);
    }

    return num_cos;
}

/*
 * Function:
 *      bcm_esw_cosq_init
 * Purpose:
 *      Initialize (clear) all COS schedule/mapping state.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_init(int unit)
{

    if (SOC_IS_SHADOW(unit)) {
        return BCM_E_UNAVAIL;
    }

    return mbcm_driver[unit]->mbcm_cosq_init(unit);
}

/*
 * Function:
 *      bcm_esw_cosq_detach
 * Purpose:
 *      Discard all COS schedule/mapping state.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_detach(int unit)
{
    return (mbcm_driver[unit]->mbcm_cosq_detach(unit, 0));
}

int bcm_esw_cosq_deinit(int unit)
{
    return (mbcm_driver[unit]->mbcm_cosq_detach(unit, 1));
}

/*
 * Function:
 *      bcm_esw_cosq_config_set
 * Purpose:
 *      Set the number of COS queues
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      numq - number of COS queues (1, 2, or 4).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_config_set(int unit, bcm_cos_queue_t numq)
{
    if (!BCM_COSQ_QUEUE_VALID(unit, numq-1)) {
	return (BCM_E_PARAM);
    }
    return mbcm_driver[unit]->mbcm_cosq_config_set(unit, numq);
}

/*
 * Function:
 *      bcm_esw_cosq_config_get
 * Purpose:
 *      Get the number of cos queues
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      numq - (Output) number of cosq
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_config_get(int unit, bcm_cos_queue_t *numq)
{
    return (mbcm_driver[unit]->mbcm_cosq_config_get(unit, numq));
}

/*
 * Function:
 *      bcm_esw_cosq_mapping_set
 * Purpose:
 *      Set which cosq a given priority should fall into
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      priority - Priority value to map
 *      cosq - COS queue to map to
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_mapping_set(int unit, bcm_cos_t priority, bcm_cos_queue_t cosq)
{
    if (soc_feature(unit, soc_feature_ets)) {
        return BCM_E_UNAVAIL;
    }

    if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
	return (BCM_E_PARAM);
    }

    return (mbcm_driver[unit]->mbcm_cosq_mapping_set(unit, -1,
						     priority, cosq));
}

int
bcm_esw_cosq_port_mapping_set(int unit, bcm_port_t port,
			      bcm_cos_t priority, bcm_cos_queue_t cosq)
{
    if (!soc_feature(unit, soc_feature_ets)) {
        if (!BCM_COSQ_QUEUE_VALID(unit, cosq)) {
            return (BCM_E_PARAM);
        }
        if (BCM_GPORT_IS_SET(port) && !SOC_IS_KATANA(unit)) {
            BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
        }
    }

    return (mbcm_driver[unit]->mbcm_cosq_mapping_set(unit, port,
						     priority, cosq));
}

/*
 * Function:
 *      bcm_esw_cosq_mapping_get
 * Purpose:
 *      Determine which COS queue a given priority currently maps to.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      priority - Priority value
 *      cosq - (Output) COS queue number
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_mapping_get(int unit, bcm_cos_t priority, bcm_cos_queue_t *cosq)
{
    if (soc_feature(unit, soc_feature_ets)) {
        return BCM_E_UNAVAIL;
    }

    return (mbcm_driver[unit]->mbcm_cosq_mapping_get(unit, -1,
						     priority, cosq));
}

int
bcm_esw_cosq_port_mapping_get(int unit, bcm_port_t port,
			      bcm_cos_t priority, bcm_cos_queue_t *cosq)
{
    if (!soc_feature(unit, soc_feature_ets)) {
        if (BCM_GPORT_IS_SET(port) && !SOC_IS_KATANA(unit)) {
            BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
        }
    }

    return (mbcm_driver[unit]->mbcm_cosq_mapping_get(unit, port,
						     priority, cosq));
}

/*
 * Function:
 *      bcm_esw_cosq_sched_set
 * Purpose:
 *      Set up class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *	weights - Weights for each COS queue
 *		Unused if mode is BCM_COSQ_STRICT.
 *		Indicates number of packets sent before going on to
 *		the next COS queue.
 *	delay - Maximum delay in microseconds before returning the
 *		round-robin to the highest priority COS queue
 *		(Unused if mode other than BCM_COSQ_BOUNDED_DELAY)
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_sched_set(int unit, int mode, const int weights[], int delay)
{
    bcm_pbmp_t pbmp;

    if (soc_feature(unit, soc_feature_ets)) {
        BCM_PBMP_ASSIGN(pbmp, PBMP_CMIC(unit));
    } else {
        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    }

    return (mbcm_driver[unit]->mbcm_cosq_port_sched_set(unit, pbmp, mode,
                                                        weights, delay));
}

int
bcm_esw_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
			int mode, const int weights[], int delay)
{
    bcm_pbmp_t pbm_all = PBMP_ALL(unit);
    BCM_PBMP_AND(pbm_all, pbm);
    if (BCM_PBMP_NEQ(pbm_all, pbm)) {
        return BCM_E_PORT;
    }
    return (mbcm_driver[unit]->mbcm_cosq_port_sched_set(unit,
							pbm,
							mode, weights, delay));
}

/*
 * Function:
 *      bcm_esw_cosq_sched_get
 * Purpose:
 *      Retrieve class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      mode_ptr - (output) Scheduling mode, one of BCM_COSQ_xxx
 *	weights - (output) Weights for each COS queue
 *		Unused if mode is BCM_COSQ_STRICT.
 *	delay - (output) Maximum delay in microseconds before returning
 *		the round-robin to the highest priority COS queue
 *		Unused if mode other than BCM_COSQ_BOUNDED_DELAY.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_sched_get(int unit, int *mode, int weights[], int *delay)
{
    bcm_pbmp_t pbmp;

    if (soc_feature(unit, soc_feature_ets)) {
        BCM_PBMP_ASSIGN(pbmp, PBMP_CMIC(unit));
    } else {
        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    }

    return (mbcm_driver[unit]->mbcm_cosq_port_sched_get(unit, pbmp, mode,
                                                        weights, delay));
}

int
bcm_esw_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
			int *mode, int weights[], int *delay)
{
    bcm_pbmp_t pbm_all = PBMP_ALL(unit);
    BCM_PBMP_AND(pbm_all, pbm);
    if (BCM_PBMP_NEQ(pbm_all, pbm)) {
        return BCM_E_PARAM;
    }
    return (mbcm_driver[unit]->mbcm_cosq_port_sched_get(unit,
							pbm,
							mode, weights, delay));
}

/*
 * Function:
 *      bcm_esw_cosq_sched_weight_max_get
 * Purpose:
 *      Retrieve maximum weights for given COS policy.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *	weight_max - (output) Maximum weight for COS queue.
 *		0 if mode is BCM_COSQ_STRICT.
 *		1 if mode is BCM_COSQ_ROUND_ROBIN.
 *		-1 if not applicable to mode.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_sched_weight_max_get(int unit, int mode, int *weight_max)
{
    return (mbcm_driver[unit]->mbcm_cosq_sched_weight_max_get(unit,
							      mode,
							      weight_max));
}

/*
 * Function:
 *      bcm_esw_cosq_port_bandwidth_set
 * Purpose:
 *      Set bandwidth values for given COS policy.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *	port - port to configure, -1 for all ports.
 *      cosq - COS queue to configure, -1 for all COS queues.
 *      kbits_sec_min - minimum bandwidth, kbits/sec.
 *      kbits_sec_max - maximum bandwidth, kbits/sec.
 *      flags - may include:
 *              BCM_COSQ_BW_EXCESS_PREF
 *              BCM_COSQ_BW_MINIMUM_PREF
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                                bcm_cos_queue_t cosq,
                                uint32 kbits_sec_min,
                                uint32 kbits_sec_max,
                                uint32 flags)
{
    int rv = BCM_E_NONE;
    bcm_pbmp_t pbmp;
    bcm_port_t loc_port;
    bcm_cos_queue_t start_cos, end_cos, loc_cos;
    int num_cosq;

    if (port < 0) {
        if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit) ||
            SOC_IS_SC_CQ(unit) || SOC_IS_ENDURO(unit)  ||
            SOC_IS_KATANA(unit)) {
            BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
            loc_port = SOC_PORT_MIN(unit,port);
        } else {
            BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
            loc_port = SOC_PORT_MIN(unit,all);
        }
        num_cosq = NUM_COS(unit);
    } else {
        if (BCM_GPORT_IS_SET(port)) {
            BCM_IF_ERROR_RETURN(
                bcm_esw_port_local_get(unit, port, &loc_port)); 
        } else {
            loc_port = port;
        }
        if (SOC_PORT_VALID(unit, loc_port)) {
            BCM_PBMP_PORT_SET(pbmp, loc_port);
        } else {
            return BCM_E_PORT;
        }
        num_cosq = IS_CPU_PORT(unit, loc_port) ?
            NUM_CPU_COSQ(unit) : NUM_COS(unit);
    }

    if (cosq < 0) {
        start_cos = 0;
        end_cos = num_cosq - 1;
    } else if (cosq >= num_cosq) {
        return BCM_E_PARAM;
    } else {
        start_cos = end_cos = cosq;
    }

    BCM_PBMP_ITER(pbmp, loc_port) {
        for (loc_cos = start_cos; loc_cos <= end_cos; loc_cos++) {
            if ((rv = mbcm_driver[unit]->mbcm_cosq_port_bandwidth_set(unit,
                           loc_port, loc_cos, kbits_sec_min, kbits_sec_max,
                            kbits_sec_max, flags)) < 0) {
                return rv;
            }
        }
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_cosq_port_bandwidth_get
 * Purpose:
 *      Retrieve bandwidth values for given COS policy.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *	port - port to configure, -1 for any port.
 *      cosq - COS queue to configure, -1 for any COS queue.
 *      kbits_sec_min - (OUT) minimum bandwidth, kbits/sec.
 *      kbits_sec_max - (OUT) maximum bandwidth, kbits/sec.
 *      flags - (OUT) may include:
 *              BCM_COSQ_BW_EXCESS_PREF
 *              BCM_COSQ_BW_MINIMUM_PREF
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_port_bandwidth_get(int unit, bcm_port_t port, 
                                bcm_cos_queue_t cosq,
                                uint32 *kbits_sec_min,
                                uint32 *kbits_sec_max,
                                uint32 *flags)
{
    bcm_port_t loc_port;
    bcm_cos_queue_t loc_cos;
    int num_cosq;
    uint32 kbits_sec_burst;    /* Dummy variable */

    if (port < 0) {
        loc_port = SOC_PORT_MIN(unit,all);
        num_cosq = NUM_COS(unit);
    } else {
        if (BCM_GPORT_IS_SET(port)) {
            BCM_IF_ERROR_RETURN(
                bcm_esw_port_local_get(unit, port, &loc_port)); 
        } else {
            loc_port = port;
        }
        if (!SOC_PORT_VALID(unit, loc_port)) {
            return BCM_E_PORT;
        }
        num_cosq = IS_CPU_PORT(unit, loc_port) ?
            NUM_CPU_COSQ(unit) : NUM_COS(unit);
    }

    if (cosq < 0) {
        loc_cos = 0;
    } else if (cosq >= num_cosq) {
        return BCM_E_PARAM;
    } else {
        loc_cos = cosq;
    }

    return (mbcm_driver[unit]->mbcm_cosq_port_bandwidth_get(unit,
                           loc_port, loc_cos, kbits_sec_min, kbits_sec_max,
                                              &kbits_sec_burst, flags));
}

/*
 * Function:
 *      bcm_esw_cosq_discard_set
 * Purpose:
 *      Set the COS queue WRED parameters
 * Parameters:
 *      unit  - StrataSwitch unit number.
 *      flags - BCM_COSQ_DISCARD_*
 * Returns:
 *      BCM_E_XXX
 */
 
int     
bcm_esw_cosq_discard_set(int unit, uint32 flags)
{
    return (mbcm_driver[unit]->mbcm_cosq_discard_set(unit, flags));
}
 
/*
 * Function:
 *      bcm_esw_cosq_discard_get
 * Purpose:
 *      Get the COS queue WRED parameters
 * Parameters:
 *      unit  - StrataSwitch unit number.
 *      flags - (OUT) BCM_COSQ_DISCARD_*
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_discard_get(int unit, uint32 *flags)
{
    return (mbcm_driver[unit]->mbcm_cosq_discard_get(unit, flags));
}
    
/*
 * Function:
 *      bcm_esw_cosq_discard_port_set
 * Purpose:
 *      Set the COS queue WRED parameters
 * Parameters:
 *      unit  - StrataSwitch unit number.
 *      port  - port to configure (-1 for all ports).
 *      cosq  - COS queue to configure (-1 for all queues).
 *      color - BCM_COSQ_DISCARD_COLOR_*
 *      drop_start -  percentage of queue
 *      drop_slope -  degress 0..90
 *      average_time - in microseconds
 *
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_discard_port_set(int unit, bcm_port_t port,
                          bcm_cos_queue_t cosq,
                          uint32 color,
                          int drop_start,
                          int drop_slope,
                          int average_time)
{
    return mbcm_driver[unit]->mbcm_cosq_discard_port_set
        (unit, port, cosq, color, drop_start, drop_slope, average_time);
}

/*
 * Function:
 *      bcm_esw_cosq_discard_port_get
 * Purpose:
 *      Set the COS queue WRED parameters
 * Parameters:
 *      unit  - StrataSwitch unit number.
 *      port  - port to get (-1 for any).
 *      cosq  - COS queue to get (-1 for any).
 *      color - BCM_COSQ_DISCARD_COLOR_*
 *      drop_start - (OUT) percentage of queue
 *      drop_slope - (OUT) degress 0..90
 *      average_time - (OUT) in microseconds
 *
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_cosq_discard_port_get(int unit, bcm_port_t port,
                          bcm_cos_queue_t cosq,
                          uint32 color,
                          int *drop_start,
                          int *drop_slope,
                          int *average_time)
{
    return mbcm_driver[unit]->mbcm_cosq_discard_port_get
        (unit, port, cosq, color, drop_start, drop_slope, average_time);
}

/*
 * Function:
 *      bcm_cosq_gport_discard_set
 * Purpose:
 *
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) GPORT ID.
 *      cosq    - (IN) COS queue to configure
 *      discard - (IN) Discard settings
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_discard_set(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                               bcm_cosq_gport_discard_t *discard)
{
    if (port == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_discard_set(unit, port, cosq, discard);
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_discard_set(unit, port, cosq, discard);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_discard_set(unit, port, cosq, discard);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit)) {
            return bcm_hu_cosq_gport_discard_set(unit, port, cosq, discard);
        } else 		
#endif /*BCM_HURRICANE_SUPPORT*/
        {
            return bcm_tr_cosq_gport_discard_set(unit, port, cosq, discard);
        }
    }
#endif /* BCM_TRX_SUPPORT */
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_discard_get
 * Purpose:
 *
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) GPORT ID.
 *      cosq    - (IN) COS queue to get
 *      discard - (IN/OUT) Discard settings
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_discard_get(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                               bcm_cosq_gport_discard_t *discard)
{
    if (port == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_discard_get(unit, port, cosq, discard);
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_discard_get(unit, port, cosq, discard);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_discard_get(unit, port, cosq, discard);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit)) {
            return bcm_hu_cosq_gport_discard_get(unit, port, cosq, discard);
        } else 			
#endif
        {
            return bcm_tr_cosq_gport_discard_get(unit, port, cosq, discard);
        }
    }
#endif /* BCM_TRX_SUPPORT */
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_add
 * Purpose:
 *       
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Physical port.
 *      numq - (IN) Number of COS queues.
 *      flags - (IN) Flags.
 *      gport - (IN/OUT) GPORT ID.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_add(int unit, bcm_gport_t port, int numq, 
                       uint32 flags, bcm_gport_t *gport)
{
    if (port == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_add(unit, port, numq, flags, gport);
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_add(unit, port, numq, flags, gport);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_add(unit, port, numq, flags, gport);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && (!(SOC_IS_HURRICANE(unit)))) {
        return bcm_tr_cosq_gport_add(unit, port, numq, flags, gport);
    }
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_delete
 * Purpose:
 *      
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_delete(int unit, bcm_gport_t gport)
{
    if (gport == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_delete(unit, gport);
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_delete(unit, gport);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_delete(unit, gport);
    }
#endif /* BCM_KTANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    /*no stage 1 queues and s1 scheduler to delete for HU*/
    if (SOC_IS_TR_VL(unit) && (!(SOC_IS_HURRICANE(unit)))) {
        return bcm_tr_cosq_gport_delete(unit, gport);
    }
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_traverse
 * Purpose:
 *      Walks through the valid COSQ GPORTs and calls
 *      the user supplied callback function for each entry.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      trav_fn    - (IN) Callback function.
 *      user_data  - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
bcm_esw_cosq_gport_traverse(int unit, bcm_cosq_gport_traverse_cb cb,
                            void *user_data)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_traverse(unit, cb, user_data);
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_traverse(unit, cb, user_data);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_traverse(unit, cb, user_data);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    /*for each 24q ports, this calls a call-back func 
    for scheduler gport. 24q ports and scheduler (S1) do not 
    exist for Hurricane*/
    if (SOC_IS_TR_VL(unit) && (!(SOC_IS_HURRICANE(unit)))) {
        return bcm_tr_cosq_gport_traverse(unit, cb, user_data);
    }
#endif
    return BCM_E_UNAVAIL;
}

int
bcm_esw_cosq_gport_mapping_set(int unit, bcm_port_t ing_port,
                               bcm_cos_t int_pri, uint32 flags,
                               bcm_gport_t gport, bcm_cos_queue_t cosq)
{
    if (!soc_feature(unit, soc_feature_ets)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_mapping_set(unit, ing_port, int_pri, flags,
                                             gport, cosq);
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

int
bcm_esw_cosq_gport_mapping_get(int unit, bcm_port_t ing_port,
                               bcm_cos_t int_pri, uint32 flags,
                               bcm_gport_t *gport, bcm_cos_queue_t *cosq)
{
    if (!soc_feature(unit, soc_feature_ets)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_mapping_get(unit, ing_port, int_pri, flags,
                                             gport, cosq);
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_bandwidth_set
 * Purpose:
 *       
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 *      cosq - (IN) COS queue to configure, -1 for all COS queues.
 *      kbits_sec_min - (IN) minimum bandwidth, kbits/sec.
 *      kbits_sec_max - (IN) maximum bandwidth, kbits/sec.
 *      flags - (IN) BCM_COSQ_BW_*
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_bandwidth_set(int unit, bcm_gport_t gport, 
                                 bcm_cos_queue_t cosq, uint32 kbits_sec_min, 
                                 uint32 kbits_sec_max, uint32 flags)
{
    if (gport == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_bandwidth_set(unit, gport, 
                                               cosq, kbits_sec_min, 
                                               kbits_sec_max, flags);
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_bandwidth_set(unit, gport, cosq,
                                               kbits_sec_min, kbits_sec_max,
                                               flags);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_bandwidth_set(unit, gport, cosq,
                                               kbits_sec_min, kbits_sec_max,
                                               flags);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit)) {
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit)) {
            return bcm_hu_cosq_gport_bandwidth_set(unit, gport, 
                                                   cosq, kbits_sec_min, 
                                                   kbits_sec_max, flags);
        }           
#endif /*SOC_IS_HURRICANE*/
        return bcm_tr_cosq_gport_bandwidth_set(unit, gport, 
                                               cosq, kbits_sec_min, 
                                               kbits_sec_max, flags);
    }
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_bandwidth_get
 * Purpose:
 *       
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 *      cosq - (IN) COS queue to configure, -1 for all COS queues.
 *      kbits_sec_min - (OUT) minimum bandwidth, kbits/sec.
 *      kbits_sec_max - (OUT) maximum bandwidth, kbits/sec.
 *      flags - (OUT) BCM_COSQ_BW_*
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_bandwidth_get(int unit, bcm_gport_t gport, 
                                 bcm_cos_queue_t cosq, uint32 *kbits_sec_min, 
                                 uint32 *kbits_sec_max, uint32 *flags)
{
    if (gport == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_bandwidth_get(unit, gport, 
                                               cosq, kbits_sec_min, 
                                               kbits_sec_max, flags);
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_bandwidth_get(unit, gport, cosq,
                                               kbits_sec_min, kbits_sec_max,
                                               flags);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_bandwidth_get(unit, gport, cosq,
                                               kbits_sec_min, kbits_sec_max,
                                               flags);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit)) {
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit)) {
            return bcm_hu_cosq_gport_bandwidth_get(unit, gport, 
                                                   cosq, kbits_sec_min, 
                                                   kbits_sec_max, flags);
        } else
#endif /* BCM_HURRICANE_SUPPORT*/
        {
            return bcm_tr_cosq_gport_bandwidth_get(unit, gport, 
                                                   cosq, kbits_sec_min, 
                                                   kbits_sec_max, flags);
        }
    }
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_sched_set
 * Purpose:
 *      
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 *      cosq - (IN) COS queue to configure, -1 for all COS queues.
 *      mode - (IN) Scheduling mode, one of BCM_COSQ_xxx
 *	weight - (IN) Weight for the specified COS queue(s)
 *               Unused if mode is BCM_COSQ_STRICT.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_sched_set(int unit, bcm_gport_t gport, 
                             bcm_cos_queue_t cosq, int mode, int weight)
{
    if (gport == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_sched_set(unit, gport, cosq, mode, weight);
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_sched_set(unit, gport, cosq, mode, weight);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_sched_set(unit, gport, cosq, mode, weight);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit)) {
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit)) {
            return bcm_hu_cosq_gport_sched_set(unit, gport, cosq, mode, weight);            
        } else 	
#endif /* BCM_HURRICANE_SUPPORT*/
        {
            return bcm_tr_cosq_gport_sched_set(unit, gport, cosq, mode, weight);
        }
    }
#endif
#ifdef BCM_SCORPION_SUPPORT 
    if (SOC_IS_SC_CQ(unit)) {
        return bcm_sc_cosq_gport_sched_set(unit, gport, cosq, mode, weight);
    }
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_sched_get
 * Purpose:
 *      
 * Parameters:
 *      unit - (IN) Unit number.
 *      gport - (IN) GPORT ID.
 *      cosq - (IN) COS queue
 *      mode - (OUT) Scheduling mode, one of BCM_COSQ_xxx
 *	weight - (OUT) Weight for the specified COS queue(s)
 *               Unused if mode is BCM_COSQ_STRICT.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_sched_get(int unit, bcm_gport_t gport, 
                             bcm_cos_queue_t cosq, int *mode, int *weight)
{
    if (gport == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_sched_get(unit, gport, cosq, mode, weight);
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_sched_get(unit, gport, cosq, mode, weight);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_sched_get(unit, gport, cosq, mode, weight);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit)) {
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit)) {
            return bcm_hu_cosq_gport_sched_get(unit, gport, cosq, mode, weight);           
        } else 	
#endif /* BCM_HURRICANE_SUPPORT*/
        {
            return bcm_tr_cosq_gport_sched_get(unit, gport, cosq, mode, weight);
        }
    }
#endif
#ifdef BCM_SCORPION_SUPPORT 
    if (SOC_IS_SC_CQ(unit)) {
        return bcm_sc_cosq_gport_sched_get(unit, gport, cosq, mode, weight);
    }
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_attach
 * Purpose:
 *      
 * Parameters:
 *      unit       - (IN) Unit number.
 *      sched_port - (IN) Scheduler GPORT ID.
 *      input_port - (IN) GPORT to attach to.
 *      cosq       - (IN) COS queue to attach to.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_cosq_gport_attach(int unit, bcm_gport_t sched_gport, 
                          bcm_gport_t input_gport, bcm_cos_queue_t cosq)
{
    if (sched_gport == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_attach(unit, sched_gport, 
                                        input_gport, cosq);
    }
#endif
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit)){
        return bcm_td_cosq_gport_attach(unit, sched_gport, input_gport, cosq);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_attach(unit, sched_gport, input_gport, cosq);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    /*for HU there is no VLAN cosq and scheduler (S1)*/
    if (SOC_IS_TR_VL(unit) && (!(SOC_IS_HURRICANE(unit)))) {
        return bcm_tr_cosq_gport_attach(unit, sched_gport, 
                                        input_gport, cosq);
    }
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_detach
 * Purpose:
 *
 * Parameters:
 *      unit       - (IN) Unit number.
 *      sched_port - (IN) Scheduler GPORT ID.
 *      input_port - (IN) GPORT to detach from.
 *      cosq       - (IN) COS queue to detach from.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_esw_cosq_gport_detach(int unit, bcm_gport_t sched_gport,
                          bcm_gport_t input_gport, bcm_cos_queue_t cosq)
{
    if (sched_gport == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_detach(unit, sched_gport,
                                        input_gport, cosq);
    }
#endif
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_detach(unit, sched_gport, input_gport, cosq);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_detach(unit, sched_gport, input_gport, cosq);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit)&&(!(SOC_IS_HURRICANE(unit)))) {
        return bcm_tr_cosq_gport_detach(unit, sched_gport,
                                        input_gport, cosq);
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_attach_get
 * Purpose:
 *
 * Parameters:
 *      unit       - (IN) Unit number.
 *      sched_port - (IN) Scheduler GPORT ID.
 *      input_port - (OUT) GPORT attached to.
 *      cosq       - (OUT) COS queue attached to.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_esw_cosq_gport_attach_get(int unit, bcm_gport_t sched_gport,
                              bcm_gport_t *input_gport, bcm_cos_queue_t *cosq)
{
    if (sched_gport == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        return bcm_tr2_cosq_gport_attach_get(unit, sched_gport, 
                                            input_gport, cosq);
    }
#endif
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_attach_get(unit, sched_gport, input_gport,
                                            cosq);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_attach_get(unit, sched_gport, input_gport,
                                            cosq);
    }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && (!(SOC_IS_HURRICANE(unit)))) {
        return bcm_tr_cosq_gport_attach_get(unit, sched_gport, 
                                            input_gport, cosq);
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_gport_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_gport_t *physical_port, 
    int *num_cos_levels, 
    uint32 *flags)
{
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_get(unit, gport, physical_port,
                                     num_cos_levels, flags);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_cosq_gport_get(unit, gport, physical_port,
                                     num_cos_levels, flags);
    }
#endif /* BCM_KATANA_SUPPORT */
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_size_set
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_size_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    uint32 bytes_min, 
    uint32 bytes_max)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_size_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_size_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    uint32 *bytes_min, 
    uint32 *bytes_max)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_enable_set
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_enable_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    int enable)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_enable_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_enable_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    int *enable)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_stat_enable_set
 * Purpose:
 *      ?
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_stat_enable_set(
    int unit, 
    bcm_gport_t gport, 
    int enable)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_stat_enable_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_stat_enable_get(
    int unit, 
    bcm_gport_t gport, 
    int *enable)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_stat_get
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_stat_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_gport_stats_t stat, 
    uint64 *value)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_cosq_gport_stat_set
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_cosq_gport_stat_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_gport_stats_t stat, 
    uint64 value)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_esw_cosq_control_set
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
 */
int
bcm_esw_cosq_control_set(int unit, bcm_gport_t gport, bcm_cos_queue_t cosq,
                         bcm_cosq_control_t type, int arg)
{
    uint32 kbits_sec_min, kbits_sec_max, kbits_sec_burst,
        kbits_burst, flags;
    bcm_pbmp_t pbmp;
    bcm_port_t port;
    bcm_cos_queue_t start_cos, end_cos, loc_cos;
    uint32 cm_val, ocm_val, cmc1_val, ocmc1_val;
    uint64 cmc_val64, ocmc_val64, mask64;
    int num_cos = NUM_COS(unit);

    switch (type) {
    case bcmCosqControlBandwidthBurstMax:
        kbits_burst = arg & 0x7fffffff; /* Convert to uint32  */
#ifdef BCM_TRIUMPH2_SUPPORT
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit)) {
            return bcm_tr2_cosq_gport_bandwidth_burst_set(unit, gport,
                                                       cosq, kbits_burst);
        }
#endif  
#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit)) {
            return bcm_td_cosq_gport_bandwidth_burst_set(unit, gport, cosq,
                                                         kbits_burst);
        }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
        if (SOC_IS_KATANA(unit)) {
            return bcm_kt_cosq_gport_bandwidth_burst_set(unit, gport, cosq,
                                                         kbits_burst);
        }
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
        if (SOC_IS_TR_VL(unit)) {
#ifdef BCM_HURRICANE_SUPPORT
            if(SOC_IS_HURRICANE(unit)) {
                return bcm_hu_cosq_gport_bandwidth_burst_set(unit, gport,
                                                              cosq, kbits_burst);                
            } else	
#endif /* BCM_HURRICANE_SUPPORT*/
            {
                return bcm_tr_cosq_gport_bandwidth_burst_set(unit, gport,
                                                              cosq, kbits_burst);
            }
        }
#endif  
        if (gport < 0) {
            BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        } else {
            /* Must use local ports on legacy devices */
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_local_get(unit, gport, &port));
            if (SOC_PORT_VALID(unit, port)) {
                BCM_PBMP_PORT_SET(pbmp, port);
            } else {
                return BCM_E_PORT;
            }
        }

        if (cosq < 0) {
            start_cos = 0;
            end_cos = NUM_COS(unit) - 1;
        } else {
            if (cosq < NUM_COS(unit)) {
                start_cos = end_cos = cosq;
            } else {
                return BCM_E_PARAM;
            }
        }

        BCM_PBMP_ITER(pbmp, port) {
            for (loc_cos = start_cos; loc_cos <= end_cos; loc_cos++) {
                BCM_IF_ERROR_RETURN
                    (mbcm_driver[unit]->
                     mbcm_cosq_port_bandwidth_get(unit, port, loc_cos,
                                                  &kbits_sec_min,
                                                  &kbits_sec_max,
                                                  &kbits_sec_burst, &flags));
                BCM_IF_ERROR_RETURN
                    (mbcm_driver[unit]->
                     mbcm_cosq_port_bandwidth_set(unit, port, loc_cos,
                                                  kbits_sec_min,
                                                  kbits_sec_max,
                                                  kbits_burst, flags));
            }
        }
        return BCM_E_NONE;

    case bcmCosqControlSchedulable:
        if (!soc_reg_field_valid(unit, COSMASKr, COSMASKf)) {
            return BCM_E_UNAVAIL;
        }

        if (gport < 0) {
            BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        } else {
            /* Must use local ports on legacy devices */
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_local_get(unit, gport, &port));
            if (SOC_PORT_VALID(unit, port)) {
                BCM_PBMP_PORT_SET(pbmp, port);
            } else {
                return BCM_E_PORT;
            }
        }
        
        if (BCM_PBMP_MEMBER(pbmp, CMIC_PORT(unit))) {
            
            if (soc_reg_field_valid(unit, COSMASK_CPU1r, COSMASKf)) {
                if (cosq < 0) {
                    start_cos = 0;
                    end_cos = 31; 
                } else {
                    if (cosq < 32) {
                        start_cos = end_cos = cosq;
                    } else {
                        return BCM_E_PARAM;
                    }
                }
                port = CMIC_PORT(unit);
                BCM_IF_ERROR_RETURN(READ_COSMASKr(unit, port, &cm_val));
                BCM_IF_ERROR_RETURN
                    (READ_COSMASK_CPU1r(unit, &cmc1_val));
                ocm_val = cm_val;
                ocmc1_val = cmc1_val;
                for (loc_cos = start_cos; loc_cos <= end_cos; loc_cos++) {
                    if (loc_cos < 10) {
                        if (arg) {
                            cm_val &= ~(1 << loc_cos);
                        } else {
                            cm_val |= (1 << loc_cos);
                        }
                    } else {
                        if (arg) {
                            cmc1_val &= ~(1 << loc_cos);
                        } else {
                            cmc1_val |= (1 << loc_cos);
                        }
                    }
                }
                if (ocm_val != cm_val) {
                    BCM_IF_ERROR_RETURN(WRITE_COSMASKr(unit, port, cm_val));
                }
                if (ocmc1_val != cmc1_val) {
                    BCM_IF_ERROR_RETURN
                        (WRITE_COSMASK_CPU1r(unit, cmc1_val));
                }
                
                BCM_PBMP_PORT_REMOVE(pbmp, CMIC_PORT(unit));
            } else if (soc_reg_field_valid(unit, COSMASK_CPUr, COSMASKf)) {
                if (cosq < 0) {
                    start_cos = 0;
                    end_cos = 47; 
                } else {
                    if (cosq < 48) {
                        start_cos = end_cos = cosq;
                    } else {
                        return BCM_E_PARAM;
                    }
                }

                BCM_IF_ERROR_RETURN
                    (READ_COSMASK_CPUr(unit, &cmc_val64));
                ocmc_val64 = cmc_val64;

                for (loc_cos = start_cos; loc_cos <= end_cos; loc_cos++) {
                    COMPILER_64_MASK_CREATE(mask64, 1, loc_cos);
                    if (arg) {
                        COMPILER_64_NOT(mask64);
                        COMPILER_64_AND(cmc_val64, mask64);
                    } else {
                        COMPILER_64_OR(cmc_val64, mask64);
                    }
                }

                if (COMPILER_64_NE(ocmc_val64, cmc_val64)) {
                    BCM_IF_ERROR_RETURN
                        (WRITE_COSMASK_CPUr(unit, cmc_val64));
                }

                BCM_PBMP_PORT_REMOVE(pbmp, CMIC_PORT(unit));
            } else {
                /* Handle below */
                num_cos = 10; /* CMIC has more COS queues */
            }
        }

        if (BCM_PBMP_IS_NULL(pbmp)) {
            /* Done */
            return BCM_E_NONE;
        }

        if (cosq < 0) {
            start_cos = 0;
            end_cos = num_cos - 1;
        } else {
            if (cosq < num_cos) {
                start_cos = end_cos = cosq;
            } else {
                return BCM_E_PARAM;
            }
        }

        BCM_PBMP_ITER(pbmp, port) {
            BCM_IF_ERROR_RETURN(READ_COSMASKr(unit, port, &cm_val));
            ocm_val = cm_val;
            for (loc_cos = start_cos; loc_cos <= end_cos; loc_cos++) {
                if (arg) {
                    cm_val &= ~(1 << loc_cos);
                } else {
                    cm_val |= (1 << loc_cos);
                }
            }
            if (ocm_val != cm_val) {
                BCM_IF_ERROR_RETURN(WRITE_COSMASKr(unit, port, cm_val));
            }
        }
        return BCM_E_NONE;

#ifdef BCM_TRIDENT_SUPPORT
    case bcmCosqControlCongestionManagedQueue:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_queue_set(unit, gport, cosq, arg);
        }
        break;
    case bcmCosqControlCongestionFeedbackWeight:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_quantize_set(unit, gport, cosq, arg,
                                                       -1);
        }
        break;
    case bcmCosqControlCongestionSetPoint:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_quantize_set(unit, gport, cosq, -1,
                                                       arg);
        }
        break;
    case bcmCosqControlCongestionSampleBytesMin:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_sample_int_set(unit, gport, cosq,
                                                         arg, -1);
        }
        break;
    case bcmCosqControlCongestionSampleBytesMax:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_sample_int_set(unit, gport, cosq,
                                                         -1, arg);
        }
        break;
    case bcmCosqControlEgressPool:
    case bcmCosqControlEgressPoolLimitBytes:
    case bcmCosqControlEgressPoolYellowLimitBytes:
    case bcmCosqControlEgressPoolRedLimitBytes:
    case bcmCosqControlEgressPoolLimitEnable:
        if (SOC_IS_TD_TT(unit)) {
            return bcm_td_cosq_control_set(unit, gport, cosq, type, arg);
        }
        break;
#endif /* BCM_TRIDENT_SUPPORT */

    default:
        break;
    }

    return BCM_E_UNAVAIL;
}

/* 
 * Function:
 *      bcm_esw_cosq_control_get
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
 */
int
bcm_esw_cosq_control_get(int unit, bcm_gport_t gport, bcm_cos_queue_t cosq,
                         bcm_cosq_control_t type, int *arg)
{
    uint32 kbits_sec_min, kbits_sec_max, kbits_burst, flags;
    bcm_port_t port;
    bcm_cos_queue_t loc_cos;
    uint32 cm_val, cmc1_val;
    uint64 cmc_val64;

    switch (type) {
    case bcmCosqControlBandwidthBurstMax:
#ifdef BCM_TRIUMPH2_SUPPORT
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit)) {
            BCM_IF_ERROR_RETURN
                (bcm_tr2_cosq_gport_bandwidth_burst_get(unit, gport,
                                                        cosq, &kbits_burst));
        } else 
#endif  
#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit)) {
            BCM_IF_ERROR_RETURN
                (bcm_td_cosq_gport_bandwidth_burst_get(unit, gport, cosq,
                                                       &kbits_burst));
        } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
        if (SOC_IS_KATANA(unit)) {
            BCM_IF_ERROR_RETURN
                (bcm_kt_cosq_gport_bandwidth_burst_get(unit, gport, cosq,
                                                       &kbits_burst));
        } else
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
        if (SOC_IS_TR_VL(unit)) {
#ifdef BCM_HURRICANE_SUPPORT
            if(SOC_IS_HURRICANE(unit)) {
                BCM_IF_ERROR_RETURN
                    (bcm_hu_cosq_gport_bandwidth_burst_get(unit, gport,
                                                           cosq, &kbits_burst));                
            } else
#endif /* BCM_HURRICANE_SUPPORT*/
            {
                BCM_IF_ERROR_RETURN
                    (bcm_tr_cosq_gport_bandwidth_burst_get(unit, gport,
                                                           cosq, &kbits_burst));
            }
        } else  /*SOC_IS_TR_VL*/
#endif  
        {
            if (gport < 0) {
                port = SOC_PORT_MIN(unit,all);
            } else {
                /* Must use local ports on legacy devices */
                BCM_IF_ERROR_RETURN
                    (bcm_esw_port_local_get(unit, gport, &port));
                if (!SOC_PORT_VALID(unit, port)) {
                    return BCM_E_PORT;
                }
            }

            if (cosq < 0) {
                loc_cos = 0;
            } else {
                if (cosq < NUM_COS(unit)) {
                    loc_cos = cosq;
                } else {
                    return BCM_E_PARAM;
                }
            }

            BCM_IF_ERROR_RETURN
                (mbcm_driver[unit]->
                 mbcm_cosq_port_bandwidth_get(unit, port, loc_cos,
                                              &kbits_sec_min, &kbits_sec_max,
                                              &kbits_burst, &flags));
        }
        *arg = kbits_burst & 0x7fffffff; /* Convert to int  */
        return BCM_E_NONE;

    case bcmCosqControlSchedulable:
        if (!soc_reg_field_valid(unit, COSMASKr, COSMASKf)) {
            return BCM_E_UNAVAIL;
        }

        if (gport < 0) {
            port = SOC_PORT_MIN(unit,all);
        } else {
            /* Must use local ports on legacy devices */
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_local_get(unit, gport, &port));
            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }
        }

        loc_cos = (cosq < 0) ? 0 : cosq;

        if (port == CMIC_PORT(unit)) {
            if (soc_reg_field_valid(unit, COSMASK_CPUr, COSMASKf)) {
                if (loc_cos > 47) {
                    return BCM_E_PARAM;
                }

                BCM_IF_ERROR_RETURN
                    (READ_COSMASK_CPUr(unit, &cmc_val64));
                *arg = COMPILER_64_BITTEST(cmc_val64, loc_cos) ?
                    FALSE : TRUE; 
            } else if (soc_reg_field_valid(unit, COSMASK_CPU1r, COSMASKf)) {
                if (loc_cos < 10) {
                    BCM_IF_ERROR_RETURN(READ_COSMASKr(unit, port, &cm_val));
                    *arg = (cm_val & (1 << loc_cos)) ? FALSE : TRUE; 
                } else if (loc_cos < 32) {
                    BCM_IF_ERROR_RETURN(READ_COSMASK_CPU1r(unit, &cmc1_val));
                    *arg = (cmc1_val & (1 << loc_cos)) ? FALSE : TRUE;
                } else {
                    return BCM_E_PARAM;
                }
            } else {
                if (loc_cos < 10) {
                    BCM_IF_ERROR_RETURN(READ_COSMASKr(unit, port, &cm_val));
                    *arg = (cm_val & (1 << loc_cos)) ? FALSE : TRUE; 
                } else {
                    return BCM_E_PARAM;
                }
            }
        } else {
            if (cosq < NUM_COS(unit)) {
                BCM_IF_ERROR_RETURN(READ_COSMASKr(unit, port, &cm_val));
                *arg = (cm_val & (1 << loc_cos)) ? FALSE : TRUE; 
            } else {
                return BCM_E_PARAM;
            }
        }

        return BCM_E_NONE;

#ifdef BCM_TRIDENT_SUPPORT
    case bcmCosqControlCongestionManagedQueue:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_queue_get(unit, gport, cosq, arg);
        }
        break;
    case bcmCosqControlCongestionFeedbackWeight:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_quantize_get(unit, gport, cosq, arg,
                                                       NULL);
        }
        break;
    case bcmCosqControlCongestionSetPoint:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_quantize_get(unit, gport, cosq, NULL,
                                                       arg);
        }
        break;
    case bcmCosqControlCongestionSampleBytesMin:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_sample_int_get(unit, gport, cosq,
                                                         arg, NULL);
        }
        break;
    case bcmCosqControlCongestionSampleBytesMax:
        if (soc_feature(unit, soc_feature_qcn)) {
            return bcm_td_cosq_congestion_sample_int_get(unit, gport, cosq,
                                                         NULL, arg);
        }
        break;
    case bcmCosqControlEgressPool:
    case bcmCosqControlEgressPoolLimitBytes:
    case bcmCosqControlEgressPoolYellowLimitBytes:
    case bcmCosqControlEgressPoolRedLimitBytes:
    case bcmCosqControlEgressPoolLimitEnable:
        if (SOC_IS_TD_TT(unit)) {
            return bcm_td_cosq_control_get(unit, gport, cosq, type, arg);
        }
        break;
#endif /* BCM_TRIDENT_SUPPORT */

    default:
        break;
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_cosq_stat_get
 * Purpose:
 *      Get MMU drop statistics on a per port per cosq basis.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      gport - (IN) GPORT ID
 *      cosq  - (IN) COS queue.
 *      stat  - (IN) Statistic to be retrieved.
 *      value - (OUT) Returned statistic value.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      Triumph and Triumph2 have the following MMU drop counters:
 *        - Per cosq packet drop:            DROP_PKT_CNT.
 *        - Per cosq byte drop:              DROP_BYTE_CNT.
 *        - Additional per port packet drop: DROP_PKT_CNT_ING.
 *        - Additional per port byte drop:   DROP_BYTE_CNT_ING.
 *        - Per port yellow packet drop:     DROP_PKT_CNT_YEL.
 *        - Per port red packet drop:        DROP_PKT_CNT_RED.
 *
 *      Scorpion has the following MMU drop counters:
 *        - Per cosq packet drop:            HOLDROP_PKT_CNT.
 *        - Additional per port packet drop: IBP_DROP_PKT_CNT, CFAP_DROP_PKT_CNT.
 *        - Per port yellow packet drop:     YELLOW_CNG_DROP_CNT.
 *        - Per port red packet drop:        RED_CN_DROP_CNT.
 *
 *      Bradley has the following MMU drop counters:
 *        - Per port packet drop:            DROP_PKT_CNT.
 *
 *      Firebolt and Hurricane have the following MMU drop counters:
 *        - Per port packet drop:            EGRDROPPKTCOUNT.
 *        - Per port yellow packet drop:     CNGDROPCOUNT1.
 *        - Per port red packet drop:        CNGDROPCOUNT0.
 */
int 
bcm_esw_cosq_stat_get(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_stat_t stat, 
    uint64 *value)
{
    int max_cosq_per_port;
    bcm_port_t local_port;
    int cosq_allowed=0;

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_stat_get(unit, gport, cosq, stat, value);
    }
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_KATANA_SUPPORT
        if (SOC_IS_KATANA(unit)) {
            return bcm_kt_cosq_stat_get(unit, gport, cosq, stat, value);
        }
#endif /* BCM_KATANA_SUPPORT */

    if (SOC_IS_HURRICANE(unit)) {
        max_cosq_per_port = 8;
    } else if (SOC_IS_TR_VL(unit)) {
        max_cosq_per_port = 48;
    } else if (SOC_IS_SC_CQ(unit)) {
        max_cosq_per_port = 32;
    } else if (SOC_IS_HB_GW(unit)) {
        max_cosq_per_port = 16;
    } else if (SOC_IS_FB_FX_HX(unit)) {
        max_cosq_per_port = 8;
    } else {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, gport, &local_port));
                                                                                
    if (((cosq >= 0) && (cosq < max_cosq_per_port)) || 
        (cosq == BCM_COS_INVALID)) {
        cosq_allowed = 1;
    }

    if (!cosq_allowed) {
        return BCM_E_PARAM;
    }

    if (value == NULL) {
        return BCM_E_PARAM;
    }        

    COMPILER_64_ZERO(*value);

    switch (stat) {
        case bcmCosqStatDroppedPackets:
#ifdef BCM_TRIUMPH_SUPPORT
            if (SOC_IS_TR_VL(unit) && !SOC_IS_HURRICANE(unit)) {
                uint64 val64;
                int i, rv;

                if (cosq == BCM_COS_INVALID) {
                    for (i = 0; i < max_cosq_per_port; i++) {
                        rv = soc_counter_get(unit, local_port, 
                                SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, i, &val64);
                        if (SOC_SUCCESS(rv)) {
                            COMPILER_64_ADD_64(*value, val64);
                        } else {
                            break;
                        }
                    }

                    /* Also add per port DROP_PKT_CNT_ING */
                    SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port,
                        SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING, 0, &val64));
                    COMPILER_64_ADD_64(*value, val64);
                } else {
                    SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port, 
                        SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, cosq, value));
                } 
            } else 
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_SCORPION_SUPPORT
            if (SOC_IS_SC_CQ(unit)) {
                uint64 val64;
                int i, rv;

                if (cosq == BCM_COS_INVALID) {
                    for (i = 0; i < max_cosq_per_port; i++) {
                        rv = soc_counter_get(unit, local_port, 
                                SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, i, &val64);
                        if (SOC_SUCCESS(rv)) {
                            COMPILER_64_ADD_64(*value, val64);
                        } else {
                            break;
                        }
                    }

                    /* Also add per port IBP_DROP_PKT_CNT */
                    SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port,
                        SOC_COUNTER_NON_DMA_PORT_DROP_PKT_IBP, 0, &val64));
                    COMPILER_64_ADD_64(*value, val64);

                    /* Also add per port CFAP_DROP_PKT_CNT */
                    SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port,
                        SOC_COUNTER_NON_DMA_PORT_DROP_PKT_CFAP, 0, &val64));
                    COMPILER_64_ADD_64(*value, val64);
                } else {
                    SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port, 
                        SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, cosq, value));
                } 
            } else 
#endif /* BCM_SCORPION_SUPPORT */
#ifdef BCM_BRADLEY_SUPPORT
            if (SOC_IS_HB_GW(unit)) {
                /* Only the per port DROP_PKT_CNT is available */
                if (cosq != BCM_COS_INVALID) {
                    return BCM_E_UNAVAIL;
                }
                SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port,
                       SOC_COUNTER_NON_DMA_PORT_DROP_PKT, 0, value));
            } else 
#endif /* BCM_BRADLEY_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
            if (SOC_IS_FB_FX_HX(unit) || SOC_IS_HURRICANE(unit)) {
                /* Only the per port EGRDROPPKTCOUNT is available */
                if (cosq != BCM_COS_INVALID) {
                    return BCM_E_UNAVAIL;
                }
                _bcm_stat_counter_extra_get(unit, EGRDROPPKTCOUNTr,
                    local_port, value);
            } else 
#endif /* BCM_FIREBOLT_SUPPORT */
            {
                return BCM_E_UNAVAIL;
            }

            break;

        case bcmCosqStatDroppedBytes:
#ifdef BCM_TRIUMPH_SUPPORT
            if (SOC_IS_TR_VL(unit) && !SOC_IS_HURRICANE(unit)) {
                uint64 val64;
                int i, rv;

                if (cosq == BCM_COS_INVALID) {
                    for (i = 0; i < max_cosq_per_port; i++) {
                        rv = soc_counter_get(unit, local_port, 
                                SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE, i, &val64);
                        if (SOC_SUCCESS(rv)) {
                            COMPILER_64_ADD_64(*value, val64);
                        } else {
                            break;
                        }
                    }

                    /* Also add the per port DROP_BYTE_CNT_ING */
                    SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port,
                        SOC_COUNTER_NON_DMA_PORT_DROP_BYTE_ING, 0, &val64));
                    COMPILER_64_ADD_64(*value, val64);
                } else {
                    SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port, 
                        SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE, cosq, value));
                } 
            } else 
#endif /* BCM_TRIUMPH_SUPPORT */
            {
                return BCM_E_UNAVAIL;
            }
            break;

        case bcmCosqStatYellowCongestionDroppedPackets:
            if (cosq != BCM_COS_INVALID) {
                /* Yellow dropped packet counters are available only on a per
                 * port basis.
                 */
                return BCM_E_UNAVAIL;
            }
            SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port,
                SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW, 0, value));
            break;

        case bcmCosqStatRedCongestionDroppedPackets:
            if (cosq != BCM_COS_INVALID) {
                /* Red dropped packet counters are available only on a per
                 * port basis.
                 */
                return BCM_E_UNAVAIL;
            }
            SOC_IF_ERROR_RETURN(soc_counter_get(unit, local_port,
                SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED, 0, value));
            break;

        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_cosq_stat_get32
 * Purpose:
 *      Get MMU drop statistics on a per port per cosq basis.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      gport - (IN) GPORT ID
 *      cosq  - (IN) COS queue.
 *      stat  - (IN) Statistic to be retrieved.
 *      value - (OUT) Returned statistic value.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_cosq_stat_get32(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_stat_t stat, 
    uint32 *value)
{
    uint64 val64;
    int rv;

    if (NULL == value) {
        return (BCM_E_PARAM);
    }

    rv = bcm_esw_cosq_stat_get(unit, gport, cosq, stat, &val64);

    if (BCM_SUCCESS(rv)) {
        *value = COMPILER_64_LO(val64);
    }

    return rv;
}

/*
 * Function:
 *      bcm_cosq_stat_set
 * Purpose:
 *      Set MMU drop statistics on a per port per cosq basis.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      gport - (IN) GPORT ID
 *      cosq  - (IN) COS queue.
 *      stat  - (IN) Statistic to be set.
 *      value - (IN) Statistic value to be set.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_cosq_stat_set(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_stat_t stat, 
    uint64 value)
{
    int max_cosq_per_port;
    bcm_port_t local_port;
    int cosq_allowed=0;

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_stat_set(unit, gport, cosq, stat, value);
    }
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_KATANA_SUPPORT
        if (SOC_IS_KATANA(unit)) {
            return bcm_kt_cosq_stat_set(unit, gport, cosq, stat, value);
        }
#endif /* BCM_KATANA_SUPPORT */

    if (SOC_IS_HURRICANE(unit)) {
        max_cosq_per_port = 8;
    } else if (SOC_IS_TR_VL(unit)) {
        max_cosq_per_port = 48;
    } else if (SOC_IS_SC_CQ(unit)) {
        max_cosq_per_port = 32;
    } else if (SOC_IS_HB_GW(unit)) {
        max_cosq_per_port = 16;
    } else if (SOC_IS_FB_FX_HX(unit)) {
        max_cosq_per_port = 8;
    } else {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, gport, &local_port));
                                                                                
    if (((cosq >= 0) && (cosq < max_cosq_per_port)) || 
        (cosq == BCM_COS_INVALID)) {
        cosq_allowed = 1;
    }

    if (!cosq_allowed) {
        return BCM_E_PARAM;
    }

    switch (stat) {
        case bcmCosqStatDroppedPackets:
#ifdef BCM_TRIUMPH_SUPPORT
            if (SOC_IS_TR_VL(unit) && !SOC_IS_HURRICANE(unit)) {
                uint64 val64_zero;
                int i, rv;

                COMPILER_64_ZERO(val64_zero);
                if (cosq == BCM_COS_INVALID) {
                    /* Write given value to first COS queue */
                    rv = soc_counter_set(unit, local_port, 
                            SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, 0, value);

                    /* Write zero to all other COS queues */
                    for (i = 1; i < max_cosq_per_port; i++) {
                        rv = soc_counter_set(unit, local_port, 
                                SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, i, val64_zero);
                        if (SOC_FAILURE(rv)) {
                            break;
                        }
                    }

                    /* Also write zero to per port DROP_PKT_CNT_ING */
                    SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port,
                        SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING, 0, val64_zero));
                } else {
                    SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port, 
                        SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, cosq, value));
                } 
            } else 
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_SCORPION_SUPPORT
            if (SOC_IS_SC_CQ(unit)) {
                uint64 val64_zero;
                int i, rv;

                COMPILER_64_ZERO(val64_zero);
                if (cosq == BCM_COS_INVALID) {
                    /* Write given value to first COS queue */
                    rv = soc_counter_set(unit, local_port, 
                            SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, 0, value);

                    /* Write zero to all other COS queues */
                    for (i = 1; i < max_cosq_per_port; i++) {
                        rv = soc_counter_set(unit, local_port, 
                                SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, i, val64_zero);
                        if (SOC_FAILURE(rv)) {
                            break;
                        }
                    }

                    /* Also write zero to per port IBP_DROP_PKT_CNT */
                    SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port,
                        SOC_COUNTER_NON_DMA_PORT_DROP_PKT_IBP, 0, val64_zero));

                    /* Also write zero to per port CFAP_DROP_PKT_CNT */
                    SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port,
                        SOC_COUNTER_NON_DMA_PORT_DROP_PKT_CFAP, 0, val64_zero));
                } else {
                    SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port, 
                        SOC_COUNTER_NON_DMA_COSQ_DROP_PKT, cosq, value));
                } 
            } else 
#endif /* BCM_SCORPION_SUPPORT */
#ifdef BCM_BRADLEY_SUPPORT
            if (SOC_IS_HB_GW(unit)) {
                /* Only the per port DROP_PKT_CNT is available */
                if (cosq != BCM_COS_INVALID) {
                    return BCM_E_UNAVAIL;
                }
                SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port,
                       SOC_COUNTER_NON_DMA_PORT_DROP_PKT, 0, value));
            } else 
#endif /* BCM_BRADLEY_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
            if (SOC_IS_FB_FX_HX(unit) || SOC_IS_HURRICANE(unit)) {
                /* EGRDROPPKTCOUNT is read-only */
                return BCM_E_UNAVAIL;
            } else 
#endif /* BCM_FIREBOLT_SUPPORT */
            {
                return BCM_E_UNAVAIL;
            }

            break;

        case bcmCosqStatDroppedBytes:
#ifdef BCM_TRIUMPH_SUPPORT
            if (SOC_IS_TR_VL(unit) && !SOC_IS_HURRICANE(unit)) {
                uint64 val64_zero;
                int i, rv;

                COMPILER_64_ZERO(val64_zero);
                if (cosq == BCM_COS_INVALID) {
                    /* Write given value to first COS queue */
                    rv = soc_counter_set(unit, local_port, 
                            SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE, 0, value);

                    /* Write zero to all other COS queues */
                    for (i = 1; i < max_cosq_per_port; i++) {
                        rv = soc_counter_set(unit, local_port, 
                                SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE, i, val64_zero);
                        if (SOC_FAILURE(rv)) {
                            break;
                        }
                    }

                    /* Also write zero to per port DROP_BYTE_CNT_ING */
                    SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port,
                        SOC_COUNTER_NON_DMA_PORT_DROP_BYTE_ING, 0, val64_zero));
                } else {
                    SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port, 
                        SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE, cosq, value));
                } 
            } else 
#endif /* BCM_TRIUMPH_SUPPORT */
            {
                return BCM_E_UNAVAIL;
            }
            break;

        case bcmCosqStatYellowCongestionDroppedPackets:
            if (cosq != BCM_COS_INVALID) {
                /* Yellow dropped packet counters are available only on a per
                 * port basis.
                 */
                return BCM_E_UNAVAIL;
            }
            if (SOC_IS_FB_FX_HX(unit) || SOC_IS_HURRICANE(unit)) {
                /* CNGDROPCOUNT1 is read-only */
                return BCM_E_UNAVAIL;
            }
            SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port,
                SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW, 0, value));
            break;

        case bcmCosqStatRedCongestionDroppedPackets:
            if (cosq != BCM_COS_INVALID) {
                /* Red dropped packet counters are available only on a per
                 * port basis.
                 */
                return BCM_E_UNAVAIL;
            }
            if (SOC_IS_FB_FX_HX(unit) || SOC_IS_HURRICANE(unit)) {
                /* CNGDROPCOUNT0 is read-only */
                return BCM_E_UNAVAIL;
            }
            SOC_IF_ERROR_RETURN(soc_counter_set(unit, local_port,
                SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED, 0, value));
            break;

        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_cosq_stat_set32
 * Purpose:
 *      Set MMU drop statistics on a per port per cosq basis.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      gport - (IN) GPORT ID
 *      cosq  - (IN) COS queue.
 *      stat  - (IN) Statistic to be set.
 *      value - (IN) Statistic value to be set.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_cosq_stat_set32(
    int unit, 
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_stat_t stat, 
    uint32 value)
{
    uint64 val64;

    COMPILER_64_SET(val64, 0, value);
    return bcm_esw_cosq_stat_set(unit, gport, cosq, stat, val64);
}

/*
 * Function:
 *      bcm_esw_cosq_gport_destmod_attach
 * Purpose:
 *      Attach gport mapping from ingress port, dest_modid to
 *      fabric egress port.
 * Parameters:
 *      unit            - (IN) Unit number.
 *      gport           - (IN) GPORT ID
 *      ingress_port    - (IN) Ingress port
 *      dest_modid      - (IN) Destination module ID
 *      fabric_egress_port - (IN) Port number on fabric that 
 *                             is connected to dest_modid
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int bcm_esw_cosq_gport_destmod_attach(int unit, bcm_gport_t gport,
                                      bcm_port_t ingress_port, 
                                      bcm_module_t dest_modid, 
                                      int fabric_egress_port)
{
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_destmod_attach(unit, gport, ingress_port,
                                           dest_modid, fabric_egress_port);
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_cosq_gport_destmod_detach
 * Purpose:
 *      Attach gport mapping from ingress port, dest_modid to
 *      fabric egress port.
 * Parameters:
 *      unit            - (IN) Unit number.
 *      gport           - (IN) GPORT ID
 *      ingress_port    - (IN) Ingress port
 *      dest_modid      - (IN) Destination module ID
 *      fabric_egress_port - (IN) Port number on fabric that 
 *                             is connected to dest_modid
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int bcm_esw_cosq_gport_destmod_detach(int unit, bcm_gport_t gport,
                                      bcm_port_t ingress_port, 
                                      bcm_module_t dest_modid, 
                                      int fabric_egress_port)
{
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_gport_destmod_detach(unit, gport, ingress_port,
                                           dest_modid, fabric_egress_port);
    }
#endif
    return BCM_E_UNAVAIL;
}

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_cosq_sw_dump
 * Purpose:
 *     Displays COS Queue information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_cosq_sw_dump(int unit)
{
    mbcm_driver[unit]->mbcm_cosq_sw_dump(unit);
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
