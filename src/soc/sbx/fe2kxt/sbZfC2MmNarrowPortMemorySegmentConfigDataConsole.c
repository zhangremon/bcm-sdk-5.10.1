/*
 * $Id: sbZfC2MmNarrowPortMemorySegmentConfigDataConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2MmNarrowPortMemorySegmentConfigDataConsole.hx"



/* Print members in struct */
void
sbZfC2MmNarrowPortMemorySegmentConfigData_Print(sbZfC2MmNarrowPortMemorySegmentConfigData_t *pFromStruct) {
  SB_LOG("C2MmNarrowPortMemorySegmentConfigData:: resv0=0x%02x", (unsigned int)  pFromStruct->m_uResv0);
  SB_LOG(" wacc=0x%01x", (unsigned int)  pFromStruct->m_uWriteAccess);
  SB_LOG(" racc=0x%01x", (unsigned int)  pFromStruct->m_uReadAccess);
  SB_LOG("\n");

  SB_LOG("C2MmNarrowPortMemorySegmentConfigData:: ding=0x%01x", (unsigned int)  pFromStruct->m_uDataIntegrity);
  SB_LOG(" trap=0x%01x", (unsigned int)  pFromStruct->m_uTrapNullPointer);
  SB_LOG(" wrap=0x%01x", (unsigned int)  pFromStruct->m_uWrap);
  SB_LOG("\n");

  SB_LOG("C2MmNarrowPortMemorySegmentConfigData:: lmask=0x%06x", (unsigned int)  pFromStruct->m_uLimitMask);
  SB_LOG(" baddr=0x%06x", (unsigned int)  pFromStruct->m_uBaseAddress);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2MmNarrowPortMemorySegmentConfigData_SPrint(sbZfC2MmNarrowPortMemorySegmentConfigData_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2MmNarrowPortMemorySegmentConfigData:: resv0=0x%02x", (unsigned int)  pFromStruct->m_uResv0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," wacc=0x%01x", (unsigned int)  pFromStruct->m_uWriteAccess);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," racc=0x%01x", (unsigned int)  pFromStruct->m_uReadAccess);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2MmNarrowPortMemorySegmentConfigData:: ding=0x%01x", (unsigned int)  pFromStruct->m_uDataIntegrity);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," trap=0x%01x", (unsigned int)  pFromStruct->m_uTrapNullPointer);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," wrap=0x%01x", (unsigned int)  pFromStruct->m_uWrap);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2MmNarrowPortMemorySegmentConfigData:: lmask=0x%06x", (unsigned int)  pFromStruct->m_uLimitMask);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," baddr=0x%06x", (unsigned int)  pFromStruct->m_uBaseAddress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2MmNarrowPortMemorySegmentConfigData_Validate(sbZfC2MmNarrowPortMemorySegmentConfigData_t *pZf) {

  if (pZf->m_uResv0 > 0x7f) return 0;
  if (pZf->m_uWriteAccess > 0x7) return 0;
  if (pZf->m_uReadAccess > 0x7) return 0;
  if (pZf->m_uDataIntegrity > 0x7) return 0;
  if (pZf->m_uTrapNullPointer > 0x1) return 0;
  if (pZf->m_uWrap > 0x1) return 0;
  if (pZf->m_uLimitMask > 0x7fffff) return 0;
  if (pZf->m_uBaseAddress > 0x7fffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2MmNarrowPortMemorySegmentConfigData_SetField(sbZfC2MmNarrowPortMemorySegmentConfigData_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uresv0") == 0) {
    s->m_uResv0 = value;
  } else if (SB_STRCMP(name, "m_uwriteaccess") == 0) {
    s->m_uWriteAccess = value;
  } else if (SB_STRCMP(name, "m_ureadaccess") == 0) {
    s->m_uReadAccess = value;
  } else if (SB_STRCMP(name, "m_udataintegrity") == 0) {
    s->m_uDataIntegrity = value;
  } else if (SB_STRCMP(name, "m_utrapnullpointer") == 0) {
    s->m_uTrapNullPointer = value;
  } else if (SB_STRCMP(name, "m_uwrap") == 0) {
    s->m_uWrap = value;
  } else if (SB_STRCMP(name, "m_ulimitmask") == 0) {
    s->m_uLimitMask = value;
  } else if (SB_STRCMP(name, "m_ubaseaddress") == 0) {
    s->m_uBaseAddress = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
