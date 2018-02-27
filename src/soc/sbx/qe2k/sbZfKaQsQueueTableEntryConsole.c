/*
 * $Id: sbZfKaQsQueueTableEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaQsQueueTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsQueueTableEntry_Print(sbZfKaQsQueueTableEntry_t *pFromStruct) {
  SB_LOG("KaQsQueueTableEntry:: credit=0x%07x", (unsigned int)  pFromStruct->m_nCredit);
  SB_LOG(" hplen=0x%01x", (unsigned int)  pFromStruct->m_nHpLen);
  SB_LOG(" depth=0x%01x", (unsigned int)  pFromStruct->m_nDepth);
  SB_LOG(" q2ec=0x%05x", (unsigned int)  pFromStruct->m_nQ2Ec);
  SB_LOG("\n");

  SB_LOG("KaQsQueueTableEntry:: localq=0x%01x", (unsigned int)  pFromStruct->m_nLocalQ);
  SB_LOG(" maxholdts=0x%01x", (unsigned int)  pFromStruct->m_nMaxHoldTs);
  SB_LOG(" queuetype=0x%01x", (unsigned int)  pFromStruct->m_nQueueType);
  SB_LOG("\n");

  SB_LOG("KaQsQueueTableEntry:: shaperatemsb=0x%02x", (unsigned int)  pFromStruct->m_nShapeRateMSB);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaQsQueueTableEntry_SPrint(sbZfKaQsQueueTableEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsQueueTableEntry:: credit=0x%07x", (unsigned int)  pFromStruct->m_nCredit);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hplen=0x%01x", (unsigned int)  pFromStruct->m_nHpLen);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," depth=0x%01x", (unsigned int)  pFromStruct->m_nDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," q2ec=0x%05x", (unsigned int)  pFromStruct->m_nQ2Ec);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsQueueTableEntry:: localq=0x%01x", (unsigned int)  pFromStruct->m_nLocalQ);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," maxholdts=0x%01x", (unsigned int)  pFromStruct->m_nMaxHoldTs);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," queuetype=0x%01x", (unsigned int)  pFromStruct->m_nQueueType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsQueueTableEntry:: shaperatemsb=0x%02x", (unsigned int)  pFromStruct->m_nShapeRateMSB);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsQueueTableEntry_Validate(sbZfKaQsQueueTableEntry_t *pZf) {

  if (pZf->m_nCredit > 0x1ffffff) return 0;
  if (pZf->m_nHpLen > 0x3) return 0;
  if (pZf->m_nDepth > 0xf) return 0;
  if (pZf->m_nQ2Ec > 0x1ffff) return 0;
  if (pZf->m_nLocalQ > 0x1) return 0;
  if (pZf->m_nMaxHoldTs > 0x7) return 0;
  if (pZf->m_nQueueType > 0xf) return 0;
  if (pZf->m_nShapeRateMSB > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsQueueTableEntry_SetField(sbZfKaQsQueueTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ncredit") == 0) {
    s->m_nCredit = value;
  } else if (SB_STRCMP(name, "m_nhplen") == 0) {
    s->m_nHpLen = value;
  } else if (SB_STRCMP(name, "m_ndepth") == 0) {
    s->m_nDepth = value;
  } else if (SB_STRCMP(name, "m_nq2ec") == 0) {
    s->m_nQ2Ec = value;
  } else if (SB_STRCMP(name, "m_nlocalq") == 0) {
    s->m_nLocalQ = value;
  } else if (SB_STRCMP(name, "m_nmaxholdts") == 0) {
    s->m_nMaxHoldTs = value;
  } else if (SB_STRCMP(name, "m_nqueuetype") == 0) {
    s->m_nQueueType = value;
  } else if (SB_STRCMP(name, "m_nshaperatemsb") == 0) {
    s->m_nShapeRateMSB = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
