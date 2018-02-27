/*
 * $Id: sbZfC2PpRxPortDataPpDescriptorFormatConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2PpRxPortDataPpDescriptorFormatConsole.hx"



/* Print members in struct */
void
sbZfC2PpRxPortDataPpDescriptorFormat_Print(sbZfC2PpRxPortDataPpDescriptorFormat_t *pFromStruct) {
  SB_LOG("C2PpRxPortDataPpDescriptorFormat:: rxportdata3=0x%02x", (unsigned int)  pFromStruct->m_uRxPortMask_57_50);
  SB_LOG(" rxportdata1=0x%05x", (unsigned int)  pFromStruct->m_uRxPortMask_49_32);
  SB_LOG("\n");

  SB_LOG("C2PpRxPortDataPpDescriptorFormat:: rxportdata0=0x%08x", (unsigned int)  pFromStruct->m_uRxPortMask_31_0);
  SB_LOG(" rxportpad=0x%02x", (unsigned int)  pFromStruct->m_uPad);
  SB_LOG("\n");

  SB_LOG("C2PpRxPortDataPpDescriptorFormat:: rxportmask3=0x%02x", (unsigned int)  pFromStruct->m_uRxPortData_57_50);
  SB_LOG(" rxportmask1=0x%05x", (unsigned int)  pFromStruct->m_uRxPortData_49_32);
  SB_LOG("\n");

  SB_LOG("C2PpRxPortDataPpDescriptorFormat:: rxportmask0=0x%08x", (unsigned int)  pFromStruct->m_uRxPortData_31_0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PpRxPortDataPpDescriptorFormat_SPrint(sbZfC2PpRxPortDataPpDescriptorFormat_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpRxPortDataPpDescriptorFormat:: rxportdata3=0x%02x", (unsigned int)  pFromStruct->m_uRxPortMask_57_50);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rxportdata1=0x%05x", (unsigned int)  pFromStruct->m_uRxPortMask_49_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpRxPortDataPpDescriptorFormat:: rxportdata0=0x%08x", (unsigned int)  pFromStruct->m_uRxPortMask_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rxportpad=0x%02x", (unsigned int)  pFromStruct->m_uPad);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpRxPortDataPpDescriptorFormat:: rxportmask3=0x%02x", (unsigned int)  pFromStruct->m_uRxPortData_57_50);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rxportmask1=0x%05x", (unsigned int)  pFromStruct->m_uRxPortData_49_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpRxPortDataPpDescriptorFormat:: rxportmask0=0x%08x", (unsigned int)  pFromStruct->m_uRxPortData_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PpRxPortDataPpDescriptorFormat_Validate(sbZfC2PpRxPortDataPpDescriptorFormat_t *pZf) {

  if (pZf->m_uRxPortMask_57_50 > 0xff) return 0;
  if (pZf->m_uRxPortMask_49_32 > 0x3ffff) return 0;
  /* pZf->m_uRxPortMask_31_0 implicitly masked by data type */
  if (pZf->m_uPad > 0x3f) return 0;
  if (pZf->m_uRxPortData_57_50 > 0xff) return 0;
  if (pZf->m_uRxPortData_49_32 > 0x3ffff) return 0;
  /* pZf->m_uRxPortData_31_0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PpRxPortDataPpDescriptorFormat_SetField(sbZfC2PpRxPortDataPpDescriptorFormat_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_urxportmask_57_50") == 0) {
    s->m_uRxPortMask_57_50 = value;
  } else if (SB_STRCMP(name, "m_urxportmask_49_32") == 0) {
    s->m_uRxPortMask_49_32 = value;
  } else if (SB_STRCMP(name, "m_urxportmask_31_0") == 0) {
    s->m_uRxPortMask_31_0 = value;
  } else if (SB_STRCMP(name, "m_upad") == 0) {
    s->m_uPad = value;
  } else if (SB_STRCMP(name, "m_urxportdata_57_50") == 0) {
    s->m_uRxPortData_57_50 = value;
  } else if (SB_STRCMP(name, "m_urxportdata_49_32") == 0) {
    s->m_uRxPortData_49_32 = value;
  } else if (SB_STRCMP(name, "m_urxportdata_31_0") == 0) {
    s->m_uRxPortData_31_0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
