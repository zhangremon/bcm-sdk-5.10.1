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
 * $Id: sbFe2000InitStatus.c 1.2.220.1 Broadcom SDK $
 *
 * sbFe2000InitStatus.c : FE2000 init status codes
 *
 *-----------------------------------------------------------------------------*/

#include <soc/sbx/sbStatus.h>

char * sbGetFe2000InitStatusString (sbStatus_t status) {
  switch (status) {

  case SB_FE2000_STS_INIT_OK_K: return "ok";
  case SB_FE2000_STS_INIT_BAD_ARGS: return "bad arguments";
  case SB_FE2000_STS_INIT_QM_TIMEOUT_ERR_K: return "Queue Manager timeout";
  case SB_FE2000_STS_INIT_PT_SPI0_TIMEOUT_ERR_K: return "SWS PT(SPI0) timeout";
  case SB_FE2000_STS_INIT_PT_SPI1_TIMEOUT_ERR_K: return "SWS PT(SPI1) timeout";
  case SB_FE2000_STS_INIT_PT_AG0_TIMEOUT_ERR_K: return "SWS PT(AG0) timeout";
  case SB_FE2000_STS_INIT_PT_AG1_TIMEOUT_ERR_K: return "SWS PT(AG1) timeout";
  case SB_FE2000_STS_INIT_PT_XG0_TIMEOUT_ERR_K: return "SWS PT(XG0) timeout";
  case SB_FE2000_STS_INIT_PT_XG1_TIMEOUT_ERR_K: return "SWS PT(XG1) timeout";
  case SB_FE2000_STS_INIT_PT_PCI_TIMEOUT_ERR_K: return "SWS PT(PCI) timeout";
  case SB_FE2000_STS_INIT_PT_PED_TIMEOUT_ERR_K: return "SWS PT(PED) timeout";
  case SB_FE2000_STS_INIT_PR_TIMEOUT_ERR_K: return "SWS Packet Receiver timeout";
  case SB_FE2000_STS_INIT_PP_TIMEOUT_ERR_K: return "Packet Parsing Engine timeout";  
  case SB_FE2000_STS_INIT_AGM_WRITE_TIMEOUT_ERR_K: return "AGM Write timeout";
  case SB_FE2000_STS_INIT_AGM_READ_TIMEOUT_ERR_K: return "AGM Read timeout";
  case SB_FE2000_STS_INIT_XGM_WRITE_TIMEOUT_ERR_K: return "XGM Write timeout";
  case SB_FE2000_STS_INIT_XGM_READ_TIMEOUT_ERR_K: return "XGM Read timeout";
  case SB_FE2000_STS_INIT_IIC_READ_TIMEOUT_ERR:  return "IIC Read timeout";
  case SB_FE2000_STS_INIT_IIC_WRITE_TIMEOUT_ERR: return "IIC Write timeout";
  case SB_FE2000_STS_INIT_MII_WRITE_TIMEOUT_ERR: return "MII Write timeout";
  case SB_FE2000_STS_INIT_MII_READ_TIMEOUT_ERR: return "MII Read timeout";
    /* leave as case last */
  case SB_FE2000_STS_INIT_LAST: return "last status";
  default: return "feInitStatus unknown error code";

  }
}
