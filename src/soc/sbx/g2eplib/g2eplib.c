/*
 * $Id: g2eplib.c 1.6 Broadcom SDK $
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
 * FE2000 Ingress library Initialization
 */

#include <sal/types.h>
#include <sal/appl/io.h>
#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/glue.h>
#include <bcm_int/sbx/error.h>
#include "sbG2EplibContext.h"
#include "sbG2Eplib.h"


int
gu2_eplib_init(int unit)
{
    sbhandle pQeHdl;
    sbElibStatus_et nSts;
    sbG2EplibCtxt_pst pEgCtxt;
    sbG2EplibInitParams_pst sEplibPrm;
    uint32_t erhType;
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    assert(sbx != NULL);

    if (!SOC_IS_SBX_QE(unit)) {
        soc_cm_print("\nELib: Init allowed for QE only");
        return SOC_E_INTERNAL;
    }

    if (!SOC_SBX_INIT(unit)) {
        soc_cm_print("\nQE HW has not been initialized");
        return SOC_E_INIT;
    }

    pQeHdl = sbx->sbhdl;
    pEgCtxt = (sbG2EplibCtxt_pst) sal_alloc(sizeof(*pEgCtxt), "EG context");
    if (NULL == pEgCtxt) {
        return SB_ELIB_MEM_ALLOC_FAIL;
    }

    sal_memset(pEgCtxt, 0, sizeof(*pEgCtxt));
    sbx->drv = pEgCtxt;

    sEplibPrm = (sbG2EplibInitParams_st *) sal_alloc(sizeof(sbG2EplibInitParams_st),
                                                     "EPlib params");
    if (NULL == sEplibPrm) {
        return SB_ELIB_MEM_ALLOC_FAIL;
    }

    sbG2EplibDefParamsGet(sEplibPrm);
    sEplibPrm->pHalCtx = pQeHdl;

    erhType = soc_property_get(unit, spn_QE_ERH_TYPE, SOC_SBX_G2P3_ERH_DEFAULT);
    switch (erhType) {
    case SOC_SBX_G2P3_ERH_QESS:
        pEgCtxt->eUcode = G2EPLIB_QESS_UCODE;
        break;
    case SOC_SBX_G2P3_ERH_DEFAULT:
        pEgCtxt->eUcode = G2EPLIB_UCODE;
        break;
    default:
        sal_free(sEplibPrm);
        return SOC_E_INIT;
    }

    nSts = sbG2EplibInit(pEgCtxt, sEplibPrm /*pParams*/);
    if (nSts != SB_OK) {
        soc_cm_print("\nEpLib: Initialization failed\n");
        sal_free(sEplibPrm);
        return nSts;
    }
    sbx->libInit = 1;  /* Set status to Initialized */
    sal_free(sEplibPrm);

    return SB_OK;
}
