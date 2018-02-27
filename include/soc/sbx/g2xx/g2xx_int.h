/*
 * $Id: g2xx_int.h 1.2 Broadcom SDK $
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
 * This file contains aggregated definitions for Guadalupe 2.x microcode
 */

#ifndef _SOC_SBX_G2XX_INT_H
#define _SOC_SBX_G2XX_INT_H

#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/g2xx/g2xx.h>
#include <sal/types.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbWrappers.h>
#include <soc/sbx/g2xx/g2xx_zframes.h>
#include <soc/sbx/glue.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/hal_common.h>
#include <soc/sbx/fe2k_common/sbFeISupport.h>
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h>
#include <soc/sbx/fe2k_common/sbFe2000CmuMgr.h>
#include <soc/sbx/fe2k_common/sbFe2000Pm.h>
#include <soc/sbx/fe2k_common/sbFe2000UcodeLoad.h>
#include <soc/sbx/fe2k/sbFe2000ClsComp.h>
#include <soc/sbx/g2xx/g2xx_pp_internal.h>

typedef struct {
  uint32_t bank;
  uint32_t base;
  uint32_t size;
}sbG2XXFe2000Mem_t;

#define SOC_SBX_G2XX_ERH_LEN(pFe) ( (pFe)->erhlen )

struct soc_sbx_g2xx_state_s {
    void *regSet;
    int unit;
    sbFe2000Queues_t *queues;
    fe2kAsm2IntD ucodestr;
    fe2kAsm2IntD *ucode;
    uint32_t erhlen;
    sbFe2000DmaMgr_t *pDmaMgr;
    sbFe2000CmuMgr_t *pCmuMgr;
    sbFe2000Pm_t *pm;
    soc_sbx_g2xx_pp_rule_mgr_t *pPpMgr;
    soc_sbx_g2xx_pp_mpls_label_space_configuration_t MplsLabelSpace;

    /* Ugh: impure, evict these junkers */
    sbFe2000DmaConfig_t DmaCfg;
    sbCommonConfigParams_t cParams;
    sbIngressConfigParams_t iParams;
    sbIngressUcodeParams_t iuParams;

    void *tableinfo;
};

#endif /* BCM_SBX_SUPPORT */
#endif
