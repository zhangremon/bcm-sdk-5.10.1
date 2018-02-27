/*
 * $Id: sbZfCaPtQueueArbParamEntryConsole.c 1.1.48.3 Broadcom SDK $
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
#include "sbZfCaPtQueueArbParamEntryConsole.hx"



/* Print members in struct */
void
sbZfCaPtQueueArbParamEntry_Print(sbZfCaPtQueueArbParamEntry_t *pFromStruct) {
  SB_LOG("CaPtQueueArbParamEntry:: ppeenqueue=0x%01x", (unsigned int)  pFromStruct->m_uPpeEnqueue);
  SB_LOG(" spare0=0x%02x", (unsigned int)  pFromStruct->m_uSpare0);
  SB_LOG(" weight=0x%02x", (unsigned int)  pFromStruct->m_uWeight);
  SB_LOG(" rxport=0x%02x", (unsigned int)  pFromStruct->m_uRxPort);
  SB_LOG("\n");

  SB_LOG("CaPtQueueArbParamEntry:: queue=0x%02x", (unsigned int)  pFromStruct->m_uQueue);
  SB_LOG(" pbselect=0x%01x", (unsigned int)  pFromStruct->m_uPbSelect);
  SB_LOG(" spare1=0x%03x", (unsigned int)  pFromStruct->m_uSpare1);
  SB_LOG("\n");

  SB_LOG("CaPtQueueArbParamEntry:: dstinterface=0x%01x", (unsigned int)  pFromStruct->m_uDstInterface);
  SB_LOG(" space2=0x%01x", (unsigned int)  pFromStruct->m_uSpace2);
  SB_LOG(" dstport=0x%02x", (unsigned int)  pFromStruct->m_uDstPort);
  SB_LOG("\n");

  SB_LOG("CaPtQueueArbParamEntry:: dstqueue=0x%02x", (unsigned int)  pFromStruct->m_uDstQueue);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaPtQueueArbParamEntry_SPrint(sbZfCaPtQueueArbParamEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPtQueueArbParamEntry:: ppeenqueue=0x%01x", (unsigned int)  pFromStruct->m_uPpeEnqueue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare0=0x%02x", (unsigned int)  pFromStruct->m_uSpare0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," weight=0x%02x", (unsigned int)  pFromStruct->m_uWeight);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rxport=0x%02x", (unsigned int)  pFromStruct->m_uRxPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPtQueueArbParamEntry:: queue=0x%02x", (unsigned int)  pFromStruct->m_uQueue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pbselect=0x%01x", (unsigned int)  pFromStruct->m_uPbSelect);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare1=0x%03x", (unsigned int)  pFromStruct->m_uSpare1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPtQueueArbParamEntry:: dstinterface=0x%01x", (unsigned int)  pFromStruct->m_uDstInterface);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," space2=0x%01x", (unsigned int)  pFromStruct->m_uSpace2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dstport=0x%02x", (unsigned int)  pFromStruct->m_uDstPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPtQueueArbParamEntry:: dstqueue=0x%02x", (unsigned int)  pFromStruct->m_uDstQueue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaPtQueueArbParamEntry_Validate(sbZfCaPtQueueArbParamEntry_t *pZf) {

  if (pZf->m_uPpeEnqueue > 0x1) return 0;
  if (pZf->m_uSpare0 > 0xff) return 0;
  if (pZf->m_uWeight > 0x7f) return 0;
  if (pZf->m_uRxPort > 0xff) return 0;
  if (pZf->m_uQueue > 0xff) return 0;
  if (pZf->m_uPbSelect > 0x1) return 0;
  if (pZf->m_uSpare1 > 0x7ff) return 0;
  if (pZf->m_uDstInterface > 0xf) return 0;
  if (pZf->m_uSpace2 > 0x3) return 0;
  if (pZf->m_uDstPort > 0x3f) return 0;
  if (pZf->m_uDstQueue > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaPtQueueArbParamEntry_SetField(sbZfCaPtQueueArbParamEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uppeenqueue") == 0) {
    s->m_uPpeEnqueue = value;
  } else if (SB_STRCMP(name, "m_uspare0") == 0) {
    s->m_uSpare0 = value;
  } else if (SB_STRCMP(name, "m_uweight") == 0) {
    s->m_uWeight = value;
  } else if (SB_STRCMP(name, "m_urxport") == 0) {
    s->m_uRxPort = value;
  } else if (SB_STRCMP(name, "m_uqueue") == 0) {
    s->m_uQueue = value;
  } else if (SB_STRCMP(name, "m_upbselect") == 0) {
    s->m_uPbSelect = value;
  } else if (SB_STRCMP(name, "m_uspare1") == 0) {
    s->m_uSpare1 = value;
  } else if (SB_STRCMP(name, "m_udstinterface") == 0) {
    s->m_uDstInterface = value;
  } else if (SB_STRCMP(name, "m_uspace2") == 0) {
    s->m_uSpace2 = value;
  } else if (SB_STRCMP(name, "m_udstport") == 0) {
    s->m_uDstPort = value;
  } else if (SB_STRCMP(name, "m_udstqueue") == 0) {
    s->m_uDstQueue = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
