/*
 * $Id: sbZfKaRbClassIPv6ClassEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaRbClassIPv6ClassEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassIPv6ClassEntry_Print(sbZfKaRbClassIPv6ClassEntry_t *pFromStruct) {
  SB_LOG("KaRbClassIPv6ClassEntry:: res1=0x%01x", (unsigned int)  pFromStruct->m_nReserve1);
  SB_LOG(" class1dp=0x%01x", (unsigned int)  pFromStruct->m_nClass1Dp);
  SB_LOG(" class1lsb=0x%01x", (unsigned int)  pFromStruct->m_nClass1Lsb);
  SB_LOG(" res0=0x%01x", (unsigned int)  pFromStruct->m_nReserve0);
  SB_LOG("\n");

  SB_LOG("KaRbClassIPv6ClassEntry:: class0dp=0x%01x", (unsigned int)  pFromStruct->m_nClass0Dp);
  SB_LOG(" class0lsb=0x%01x", (unsigned int)  pFromStruct->m_nClass0Lsb);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaRbClassIPv6ClassEntry_SPrint(sbZfKaRbClassIPv6ClassEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassIPv6ClassEntry:: res1=0x%01x", (unsigned int)  pFromStruct->m_nReserve1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class1dp=0x%01x", (unsigned int)  pFromStruct->m_nClass1Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class1lsb=0x%01x", (unsigned int)  pFromStruct->m_nClass1Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," res0=0x%01x", (unsigned int)  pFromStruct->m_nReserve0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassIPv6ClassEntry:: class0dp=0x%01x", (unsigned int)  pFromStruct->m_nClass0Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class0lsb=0x%01x", (unsigned int)  pFromStruct->m_nClass0Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassIPv6ClassEntry_Validate(sbZfKaRbClassIPv6ClassEntry_t *pZf) {

  if (pZf->m_nReserve1 > 0x3) return 0;
  if (pZf->m_nClass1Dp > 0x3) return 0;
  if (pZf->m_nClass1Lsb > 0xf) return 0;
  if (pZf->m_nReserve0 > 0x3) return 0;
  if (pZf->m_nClass0Dp > 0x3) return 0;
  if (pZf->m_nClass0Lsb > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassIPv6ClassEntry_SetField(sbZfKaRbClassIPv6ClassEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserve1") == 0) {
    s->m_nReserve1 = value;
  } else if (SB_STRCMP(name, "m_nclass1dp") == 0) {
    s->m_nClass1Dp = value;
  } else if (SB_STRCMP(name, "m_nclass1lsb") == 0) {
    s->m_nClass1Lsb = value;
  } else if (SB_STRCMP(name, "m_nreserve0") == 0) {
    s->m_nReserve0 = value;
  } else if (SB_STRCMP(name, "m_nclass0dp") == 0) {
    s->m_nClass0Dp = value;
  } else if (SB_STRCMP(name, "m_nclass0lsb") == 0) {
    s->m_nClass0Lsb = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
