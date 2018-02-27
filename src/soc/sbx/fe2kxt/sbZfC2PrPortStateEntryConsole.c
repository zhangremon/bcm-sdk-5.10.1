/*
 * $Id: sbZfC2PrPortStateEntryConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2PrPortStateEntryConsole.hx"



/* Print members in struct */
void
sbZfC2PrPortStateEntry_Print(sbZfC2PrPortStateEntry_t *pFromStruct) {
  SB_LOG("C2PrPortStateEntry:: reserve=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  SB_LOG(" testpkt=0x%01x", (unsigned int)  pFromStruct->m_bTestPkt);
  SB_LOG(" buffer=0x%03x", (unsigned int)  pFromStruct->m_nBuffer);
  SB_LOG(" length=0x%03x", (unsigned int)  pFromStruct->m_nLength);
  SB_LOG("\n");

  SB_LOG("C2PrPortStateEntry:: npages=0x%02x", (unsigned int)  pFromStruct->m_nNumPages);
  SB_LOG(" sop=0x%01x", (unsigned int)  pFromStruct->m_bStartOfPkt);
  SB_LOG(" page=0x%03x", (unsigned int)  pFromStruct->m_nPageAddr);
  SB_LOG(" line=0x%01x", (unsigned int)  pFromStruct->m_nLineAddr);
  SB_LOG(" word=0x%01x", (unsigned int)  pFromStruct->m_nWordNumber);
  SB_LOG("\n");

  SB_LOG("C2PrPortStateEntry:: drop=0x%01x", (unsigned int)  pFromStruct->m_bDropPkt);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PrPortStateEntry_SPrint(sbZfC2PrPortStateEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrPortStateEntry:: reserve=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," testpkt=0x%01x", (unsigned int)  pFromStruct->m_bTestPkt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," buffer=0x%03x", (unsigned int)  pFromStruct->m_nBuffer);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," length=0x%03x", (unsigned int)  pFromStruct->m_nLength);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrPortStateEntry:: npages=0x%02x", (unsigned int)  pFromStruct->m_nNumPages);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sop=0x%01x", (unsigned int)  pFromStruct->m_bStartOfPkt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," page=0x%03x", (unsigned int)  pFromStruct->m_nPageAddr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," line=0x%01x", (unsigned int)  pFromStruct->m_nLineAddr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," word=0x%01x", (unsigned int)  pFromStruct->m_nWordNumber);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrPortStateEntry:: drop=0x%01x", (unsigned int)  pFromStruct->m_bDropPkt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PrPortStateEntry_Validate(sbZfC2PrPortStateEntry_t *pZf) {

  if (pZf->m_nReserved > 0x3f) return 0;
  if (pZf->m_bTestPkt > 0x1) return 0;
  if (pZf->m_nBuffer > 0xfff) return 0;
  if (pZf->m_nLength > 0x7ff) return 0;
  if (pZf->m_nNumPages > 0x7f) return 0;
  if (pZf->m_bStartOfPkt > 0x1) return 0;
  if (pZf->m_nPageAddr > 0xfff) return 0;
  if (pZf->m_nLineAddr > 0x1) return 0;
  if (pZf->m_nWordNumber > 0xf) return 0;
  if (pZf->m_bDropPkt > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PrPortStateEntry_SetField(sbZfC2PrPortStateEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "testpkt") == 0) {
    s->m_bTestPkt = value;
  } else if (SB_STRCMP(name, "m_nbuffer") == 0) {
    s->m_nBuffer = value;
  } else if (SB_STRCMP(name, "m_nlength") == 0) {
    s->m_nLength = value;
  } else if (SB_STRCMP(name, "m_nnumpages") == 0) {
    s->m_nNumPages = value;
  } else if (SB_STRCMP(name, "startofpkt") == 0) {
    s->m_bStartOfPkt = value;
  } else if (SB_STRCMP(name, "m_npageaddr") == 0) {
    s->m_nPageAddr = value;
  } else if (SB_STRCMP(name, "m_nlineaddr") == 0) {
    s->m_nLineAddr = value;
  } else if (SB_STRCMP(name, "m_nwordnumber") == 0) {
    s->m_nWordNumber = value;
  } else if (SB_STRCMP(name, "droppkt") == 0) {
    s->m_bDropPkt = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
