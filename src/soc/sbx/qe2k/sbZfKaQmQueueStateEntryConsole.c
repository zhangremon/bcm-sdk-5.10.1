/*
 * $Id: sbZfKaQmQueueStateEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaQmQueueStateEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmQueueStateEntry_Print(sbZfKaQmQueueStateEntry_t *pFromStruct) {
  SB_LOG("KaQmQueueStateEntry:: allocatedbuffscnt=0x%05x", (unsigned int)  pFromStruct->m_nAllocatedBuffsCnt);
  SB_LOG(" qtailptr=0x%07x", (unsigned int)  pFromStruct->m_nQTailPtr);
  SB_LOG("\n");

  SB_LOG("KaQmQueueStateEntry:: qheadptr=0x%07x", (unsigned int)  pFromStruct->m_nQHeadPtr);
  SB_LOG(" nobuffsallocated=0x%01x", (unsigned int)  pFromStruct->m_nNoBuffsAllocated);
  SB_LOG(" overflow=0x%01x", (unsigned int)  pFromStruct->m_nOverflow);
  SB_LOG("\n");

  SB_LOG("KaQmQueueStateEntry:: minbuffers=0x%04x", (unsigned int)  pFromStruct->m_nMinBuffers);
  SB_LOG(" maxbuffers=0x%04x", (unsigned int)  pFromStruct->m_nMaxBuffers);
  SB_LOG(" local=0x%01x", (unsigned int)  pFromStruct->m_nLocal);
  SB_LOG("\n");

  SB_LOG("KaQmQueueStateEntry:: queuedepthinline16b=0x%07x", (unsigned int)  pFromStruct->m_nQueueDepthInLine16B);
  SB_LOG(" anemicwatermarksel=0x%01x", (unsigned int)  pFromStruct->m_nAnemicWatermarkSel);
  SB_LOG("\n");

  SB_LOG("KaQmQueueStateEntry:: qetype=0x%01x", (unsigned int)  pFromStruct->m_nQeType);
  SB_LOG(" enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaQmQueueStateEntry_SPrint(sbZfKaQmQueueStateEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: allocatedbuffscnt=0x%05x", (unsigned int)  pFromStruct->m_nAllocatedBuffsCnt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," qtailptr=0x%07x", (unsigned int)  pFromStruct->m_nQTailPtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: qheadptr=0x%07x", (unsigned int)  pFromStruct->m_nQHeadPtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nobuffsallocated=0x%01x", (unsigned int)  pFromStruct->m_nNoBuffsAllocated);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," overflow=0x%01x", (unsigned int)  pFromStruct->m_nOverflow);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: minbuffers=0x%04x", (unsigned int)  pFromStruct->m_nMinBuffers);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," maxbuffers=0x%04x", (unsigned int)  pFromStruct->m_nMaxBuffers);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," local=0x%01x", (unsigned int)  pFromStruct->m_nLocal);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: queuedepthinline16b=0x%07x", (unsigned int)  pFromStruct->m_nQueueDepthInLine16B);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," anemicwatermarksel=0x%01x", (unsigned int)  pFromStruct->m_nAnemicWatermarkSel);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueStateEntry:: qetype=0x%01x", (unsigned int)  pFromStruct->m_nQeType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmQueueStateEntry_Validate(sbZfKaQmQueueStateEntry_t *pZf) {

  if (pZf->m_nAllocatedBuffsCnt > 0x1ffff) return 0;
  if (pZf->m_nQTailPtr > 0x1ffffff) return 0;
  if (pZf->m_nQHeadPtr > 0x1ffffff) return 0;
  if (pZf->m_nNoBuffsAllocated > 0x1) return 0;
  if (pZf->m_nOverflow > 0x1) return 0;
  if (pZf->m_nMinBuffers > 0x3fff) return 0;
  if (pZf->m_nMaxBuffers > 0x3fff) return 0;
  if (pZf->m_nLocal > 0x1) return 0;
  if (pZf->m_nQueueDepthInLine16B > 0x1ffffff) return 0;
  if (pZf->m_nAnemicWatermarkSel > 0x7) return 0;
  if (pZf->m_nQeType > 0x1) return 0;
  if (pZf->m_nEnable > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmQueueStateEntry_SetField(sbZfKaQmQueueStateEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nallocatedbuffscnt") == 0) {
    s->m_nAllocatedBuffsCnt = value;
  } else if (SB_STRCMP(name, "m_nqtailptr") == 0) {
    s->m_nQTailPtr = value;
  } else if (SB_STRCMP(name, "m_nqheadptr") == 0) {
    s->m_nQHeadPtr = value;
  } else if (SB_STRCMP(name, "m_nnobuffsallocated") == 0) {
    s->m_nNoBuffsAllocated = value;
  } else if (SB_STRCMP(name, "m_noverflow") == 0) {
    s->m_nOverflow = value;
  } else if (SB_STRCMP(name, "m_nminbuffers") == 0) {
    s->m_nMinBuffers = value;
  } else if (SB_STRCMP(name, "m_nmaxbuffers") == 0) {
    s->m_nMaxBuffers = value;
  } else if (SB_STRCMP(name, "m_nlocal") == 0) {
    s->m_nLocal = value;
  } else if (SB_STRCMP(name, "m_nqueuedepthinline16b") == 0) {
    s->m_nQueueDepthInLine16B = value;
  } else if (SB_STRCMP(name, "m_nanemicwatermarksel") == 0) {
    s->m_nAnemicWatermarkSel = value;
  } else if (SB_STRCMP(name, "m_nqetype") == 0) {
    s->m_nQeType = value;
  } else if (SB_STRCMP(name, "m_nenable") == 0) {
    s->m_nEnable = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
