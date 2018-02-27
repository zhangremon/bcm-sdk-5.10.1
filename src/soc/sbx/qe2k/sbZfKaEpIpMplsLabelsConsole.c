/*
 * $Id: sbZfKaEpIpMplsLabelsConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaEpIpMplsLabelsConsole.hx"



/* Print members in struct */
void
sbZfKaEpIpMplsLabels_Print(sbZfKaEpIpMplsLabels_t *pFromStruct) {
  SB_LOG("KaEpIpMplsLabels:: label1=0x%05x", (unsigned int)  pFromStruct->m_nLabel1);
  SB_LOG(" op=0x%01x", (unsigned int)  pFromStruct->m_nOp);
  SB_LOG(" link=0x%01x", (unsigned int)  pFromStruct->m_nLink);
  SB_LOG(" stack1=0x%01x", (unsigned int)  pFromStruct->m_nStack1);
  SB_LOG(" ttl1=0x%02x", (unsigned int)  pFromStruct->m_nTttl1);
  SB_LOG("\n");

  SB_LOG("KaEpIpMplsLabels:: label0=0x%05x", (unsigned int)  pFromStruct->m_nLabel0);
  SB_LOG(" exp=0x%01x", (unsigned int)  pFromStruct->m_nExp);
  SB_LOG(" stack0=0x%01x", (unsigned int)  pFromStruct->m_nStack0);
  SB_LOG(" ttl0=0x%02x", (unsigned int)  pFromStruct->m_nTttl0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaEpIpMplsLabels_SPrint(sbZfKaEpIpMplsLabels_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIpMplsLabels:: label1=0x%05x", (unsigned int)  pFromStruct->m_nLabel1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," op=0x%01x", (unsigned int)  pFromStruct->m_nOp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," link=0x%01x", (unsigned int)  pFromStruct->m_nLink);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," stack1=0x%01x", (unsigned int)  pFromStruct->m_nStack1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ttl1=0x%02x", (unsigned int)  pFromStruct->m_nTttl1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIpMplsLabels:: label0=0x%05x", (unsigned int)  pFromStruct->m_nLabel0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," exp=0x%01x", (unsigned int)  pFromStruct->m_nExp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," stack0=0x%01x", (unsigned int)  pFromStruct->m_nStack0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ttl0=0x%02x", (unsigned int)  pFromStruct->m_nTttl0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIpMplsLabels_Validate(sbZfKaEpIpMplsLabels_t *pZf) {

  if (pZf->m_nLabel1 > 0xfffff) return 0;
  if (pZf->m_nOp > 0x1) return 0;
  if (pZf->m_nLink > 0x3) return 0;
  if (pZf->m_nStack1 > 0x1) return 0;
  if (pZf->m_nTttl1 > 0xff) return 0;
  if (pZf->m_nLabel0 > 0xfffff) return 0;
  if (pZf->m_nExp > 0x7) return 0;
  if (pZf->m_nStack0 > 0x1) return 0;
  if (pZf->m_nTttl0 > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIpMplsLabels_SetField(sbZfKaEpIpMplsLabels_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nlabel1") == 0) {
    s->m_nLabel1 = value;
  } else if (SB_STRCMP(name, "m_nop") == 0) {
    s->m_nOp = value;
  } else if (SB_STRCMP(name, "m_nlink") == 0) {
    s->m_nLink = value;
  } else if (SB_STRCMP(name, "m_nstack1") == 0) {
    s->m_nStack1 = value;
  } else if (SB_STRCMP(name, "m_ntttl1") == 0) {
    s->m_nTttl1 = value;
  } else if (SB_STRCMP(name, "m_nlabel0") == 0) {
    s->m_nLabel0 = value;
  } else if (SB_STRCMP(name, "m_nexp") == 0) {
    s->m_nExp = value;
  } else if (SB_STRCMP(name, "m_nstack0") == 0) {
    s->m_nStack0 = value;
  } else if (SB_STRCMP(name, "m_ntttl0") == 0) {
    s->m_nTttl0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
