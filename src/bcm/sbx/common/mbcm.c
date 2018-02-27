/*
 * $Id: mbcm.c 1.5 Broadcom SDK $
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
 * File:        mbcm.c
 * Purpose:     Implementation of bcm multiplexing
 */


#include <soc/defs.h>
#include <soc/sbx/sbx_drv.h>
#include <bcm/error.h>

#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/mbcm.h>

mbcm_sbx_functions_t    *mbcm_sbx_driver[BCM_MAX_NUM_UNITS]={0};
soc_sbx_chip_family_t    mbcm_sbx_family[BCM_MAX_NUM_UNITS];

/****************************************************************
 *
 * Function:        mbcm_sbx_init
 * Parameters:      unit   --   unit to setup
 * 
 * Initialize the mbcm driver for the indicated unit.
 *
 ****************************************************************/
int
mbcm_sbx_init(int unit)
{
#ifdef  BCM_QE2000_SUPPORT
    if (SOC_IS_SBX_QE2000(unit)) {
        mbcm_sbx_driver[unit] = &mbcm_qe2000_driver;
        mbcm_sbx_family[unit] = BCM_FAMILY_QE2000;
        return BCM_E_NONE;
    }
#endif  /* BCM_QE2000_SUPPORT */
#ifdef  BCM_BME3200_SUPPORT
    if (SOC_IS_SBX_BME3200(unit)) {
        mbcm_sbx_driver[unit] = &mbcm_bm3200_driver;
        mbcm_sbx_family[unit] = BCM_FAMILY_BM3200;
        return BCM_E_NONE;
    }
#endif  /* BCM_BME3200_SUPPORT */

#ifdef  BCM_BM9600_SUPPORT
    if (SOC_IS_SBX_BM9600(unit)) {
        mbcm_sbx_driver[unit] = &mbcm_bm9600_driver;
        mbcm_sbx_family[unit] = BCM_FAMILY_BM9600;
        return BCM_E_NONE;
    }
#endif  /* BCM_BM9600_SUPPORT */
#ifdef  BCM_SIRIUS_SUPPORT
    if (SOC_IS_SBX_SIRIUS(unit)) {
        mbcm_sbx_driver[unit] = &mbcm_sirius_driver;
        mbcm_sbx_family[unit] = BCM_FAMILY_SIRIUS;
        return BCM_E_NONE;
    }
#endif  /* BCM_SIRIUS_SUPPORT */

    soc_cm_print("ERROR: mbcm_sbx_init unit %d: unsupported chip type\n", unit);
    return BCM_E_INTERNAL;
}
