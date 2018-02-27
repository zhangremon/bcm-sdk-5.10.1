#ifndef __SB_FE2000_INIT_STATUS_H__
#define __SB_FE2000_INIT_STATUS_H__
/* -*- Mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
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
 * $Id: sbFe2000InitStatus.h 1.4.156.1 Broadcom SDK $
 *
 * sbFe2000InitStatus.h : Enumerated FE2000 status codes
 *
 *-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * Enumerated Type of Return Codes
 *
 **/
typedef enum sbFe2000InitStatus_es {
  SB_FE2000_STS_INIT_OK_K = 0,
  SB_FE2000_STS_INIT_BAD_ARGS = SB_BUILD_ERR_CODE(SB_VENDOR_ID_SANDBURST, SB_MODULE_ID_FE2000_INIT, 0x1),
  SB_FE2000_STS_INIT_QM_TIMEOUT_ERR_K, /* 17301506 */
  SB_FE2000_STS_INIT_PT_SPI0_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_SPI1_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_AG0_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_AG1_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_XG0_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_XG1_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_PCI_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_PED_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PR_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PP_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_AGM_WRITE_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_AGM_READ_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_XGM_WRITE_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_XGM_READ_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_IIC_READ_TIMEOUT_ERR,
  SB_FE2000_STS_INIT_IIC_WRITE_TIMEOUT_ERR,
  SB_FE2000_STS_INIT_MII_WRITE_TIMEOUT_ERR,
  SB_FE2000_STS_INIT_MII_READ_TIMEOUT_ERR,
  SB_FE2000_STS_INIT_LR_UCODESWAP_TIMEOUT_ERR,
  /* leave as last */
  SB_FE2000_STS_INIT_LAST
} sbFe2000InitStatus_et;

char * sbGetFe2000InitStatusString(sbStatus_t status);
#ifdef __cplusplus
}
#endif

#endif /*__SB_FE2000_INIT_STATUS_H__ */
