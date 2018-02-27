/*
 * $Id: sbZfC2PmCheckerTimerCfgEncodingConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfC2PmCheckerTimerCfgEncodingConsole.hx"



/* Print members in struct */
void
sbZfC2PmCheckerTimerCfgEncoding_Print(sbZfC2PmCheckerTimerCfgEncoding_t *pFromStruct) {
  SB_LOG("C2PmCheckerTimerCfgEncoding:: type=0x%01x", (unsigned int)  pFromStruct->m_uType);
  SB_LOG(" interrupt=0x%01x", (unsigned int)  pFromStruct->m_uInterrupt);
  SB_LOG(" reset=0x%01x", (unsigned int)  pFromStruct->m_uReset);
  SB_LOG(" strict=0x%01x", (unsigned int)  pFromStruct->m_uStrict);
  SB_LOG("\n");

  SB_LOG("C2PmCheckerTimerCfgEncoding:: mode32=0x%01x", (unsigned int)  pFromStruct->m_uMode32);
  SB_LOG(" deadline=0x%05x", (unsigned int)  pFromStruct->m_uDeadline);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PmCheckerTimerCfgEncoding_SPrint(sbZfC2PmCheckerTimerCfgEncoding_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmCheckerTimerCfgEncoding:: type=0x%01x", (unsigned int)  pFromStruct->m_uType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," interrupt=0x%01x", (unsigned int)  pFromStruct->m_uInterrupt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reset=0x%01x", (unsigned int)  pFromStruct->m_uReset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," strict=0x%01x", (unsigned int)  pFromStruct->m_uStrict);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmCheckerTimerCfgEncoding:: mode32=0x%01x", (unsigned int)  pFromStruct->m_uMode32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," deadline=0x%05x", (unsigned int)  pFromStruct->m_uDeadline);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PmCheckerTimerCfgEncoding_Validate(sbZfC2PmCheckerTimerCfgEncoding_t *pZf) {

  if (pZf->m_uType > 0x7) return 0;
  if (pZf->m_uInterrupt > 0x1) return 0;
  if (pZf->m_uReset > 0x1) return 0;
  if (pZf->m_uStrict > 0x1) return 0;
  if (pZf->m_uMode32 > 0x1) return 0;
  if (pZf->m_uDeadline > 0xfffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PmCheckerTimerCfgEncoding_SetField(sbZfC2PmCheckerTimerCfgEncoding_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_utype") == 0) {
    s->m_uType = value;
  } else if (SB_STRCMP(name, "m_uinterrupt") == 0) {
    s->m_uInterrupt = value;
  } else if (SB_STRCMP(name, "m_ureset") == 0) {
    s->m_uReset = value;
  } else if (SB_STRCMP(name, "m_ustrict") == 0) {
    s->m_uStrict = value;
  } else if (SB_STRCMP(name, "m_umode32") == 0) {
    s->m_uMode32 = value;
  } else if (SB_STRCMP(name, "m_udeadline") == 0) {
    s->m_uDeadline = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
