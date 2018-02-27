/*
 * $Id: device_wred.c 1.4 Broadcom SDK $
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
 * File:        device_wred.c
 * Purpose:     Implement WRED configuration parameter algorithm. This could be
 *              shared across devices.
 */


#include <soc/sbx/sbTypes.h>
#include <soc/sbx/fabric/sbZfFabWredParameters.hx>
#include <soc/cm.h>
#include <bcm/cosq.h>
#include <bcm_int/sbx/device_wred.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <soc/sbx/sbx_drv.h>

int
_bcm_sbx_device_wred_calc_config(int unit,
                                 int mtu_size,
                                 int max_queue_depth,
                                 bcm_cosq_gport_discard_t *discard,
                                 sbZfFabWredParameters_t *chip_params)
{
    int                     rc = BCM_E_NONE;
    int                     i;
    uint8_t                 msb_queue_depth = 0, scaler;
    uint32_t                min_threshold, max_threshold, ecn_threshold;
    uint32_t                delta, pb2snd, pb_scale_slope, slope;


    BCM_VERB(("WRED Parameters, MtuSz(0x%x) nMaxQueueDepth(0x%x) nMinThresholdInBytes(0x%x) nMaxThresholdInBytes(0x%x) DpAtMaxThrehold(0x%x) nEcnThreshold(0x%x)\n",
       mtu_size, max_queue_depth, discard->min_thresh, discard->max_thresh,
       discard->drop_probability, discard->ecn_thresh));

    if (soc_feature(unit, soc_feature_ingress_size_templates)) {   
	min_threshold = discard->min_thresh;
	max_threshold = discard->max_thresh;
	ecn_threshold = discard->ecn_thresh;
    } else {
	/* convert thresholds to units of 16 Bytes. Queue length messages are in units of 16 Bytes */
	max_queue_depth= max_queue_depth / BCM_SBX_DEVICE_WRED_THRESHOLD_UNIT_SZ;
	min_threshold = discard->min_thresh / BCM_SBX_DEVICE_WRED_THRESHOLD_UNIT_SZ;
	max_threshold = discard->max_thresh / BCM_SBX_DEVICE_WRED_THRESHOLD_UNIT_SZ;
	ecn_threshold = discard->ecn_thresh / BCM_SBX_DEVICE_WRED_THRESHOLD_UNIT_SZ;
    }

    for (i = 0, msb_queue_depth = 0; i < 32; i++) {
        if ((max_queue_depth & (1 << i)) != 0) {
            msb_queue_depth = i;
        }
    }
    BCM_VERB(("Max Bit Set in Queue Depth(0x%x)\n", msb_queue_depth));

    /* calculate scaler value */
    scaler = (msb_queue_depth > 15) ? (msb_queue_depth - 15) : 0;
    BCM_VERB(("Scaler(0x%x)\n", scaler));

    /*
     * denominator calculation
     */
    delta = (max_threshold >> scaler) - (min_threshold >> scaler);
    BCM_VERB(("nDelta(0x%x)\n", delta));

    if (!delta) {
        BCM_ERR(("ERROR: %s: Invalid config (delta = 0)\n", FUNCTION_NAME()));
        return BCM_E_CONFIG;
    }

    /*
     * numerator calculation
     */
    /* The factor "100" accounts for the drop probabilty representation as a percentage */
    pb2snd = (discard->drop_probability * BCM_SBX_DEVICE_WRED_MAX_RANGE_RANDOM_NUMBER) / (mtu_size * 100);
    BCM_VERB(("nPb2Snd(0x%x)\n", pb2snd));

    /* Accounts for BM weightage */
    pb_scale_slope = pb2snd << BCM_SBX_DEVICE_WRED_BM_WEIGHT_SHIFT;

    /*
     * slope calculation
     */
    slope = pb_scale_slope / delta;

    BCM_VERB(("nPb2Snd(0x%x), nPbScaleSlope(0x%x), nSlope(0x%x)\n", pb2snd, pb_scale_slope , slope));

    if (slope > BCM_SBX_DEVICE_WRED_MAX_SLOPE_VALUE) {
        BCM_VERB(("WRED Slope(0x%x) Calculated to be greater then Maximum(0x%x)\n",
                                                   slope, BCM_SBX_DEVICE_WRED_MAX_SLOPE_VALUE));
        slope = BCM_SBX_DEVICE_WRED_MAX_SLOPE_VALUE;
    }

    chip_params->m_nSlope = slope;
    chip_params->m_nScale = scaler;
    chip_params->m_nTmax = (max_threshold >> scaler);
    chip_params->m_nTmin = (min_threshold >> scaler);
    chip_params->m_nTecn = (ecn_threshold >> scaler);

    BCM_VERB(("WRED Parameters, Slope(0x%x), Scale(0x%x), Tmax(0x%x) Tmin(0x%x) Tecn(0x%x)\n",
             chip_params->m_nSlope, chip_params->m_nScale,
             chip_params->m_nTmax, chip_params->m_nTmin, chip_params->m_nTecn));

    return(rc);
}
