/*
 * $Id: sbZfSbQe2000ElibPTConsole.c 1.3.36.3 Broadcom SDK $
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
#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibPTConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibPT_Print(sbZfSbQe2000ElibPT_t *pFromStruct) {
  SB_LOG("SbQe2000ElibPT:: enable15=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb15);
  SB_LOG(" enable14=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb14);
  SB_LOG(" enable13=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb13);
  SB_LOG(" enable12=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb12);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPT:: enable11=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb11);
  SB_LOG(" enable10=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb10);
  SB_LOG(" enable9=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb9);
  SB_LOG(" enable8=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb8);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPT:: enable7=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb7);
  SB_LOG(" enable6=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb6);
  SB_LOG(" enable5=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb5);
  SB_LOG(" enable4=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb4);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPT:: enable3=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb3);
  SB_LOG(" enable2=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb2);
  SB_LOG(" enable1=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb1);
  SB_LOG(" enable0=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb0);
  SB_LOG(" trans=0x%02x", (unsigned int)  pFromStruct->m_nCountTrans);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPT:: resrv1=0x%02x", (unsigned int)  pFromStruct->m_nReserved1);
  SB_LOG(" prepend=0x%01x", (unsigned int)  pFromStruct->m_bPrepend);
  SB_LOG(" valid=0x%01x", (unsigned int)  pFromStruct->m_bInstValid);
  SB_LOG(" inst=0x%08x", (unsigned int)  pFromStruct->m_Instruction);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibPT_SPrint(sbZfSbQe2000ElibPT_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: enable15=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable14=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable13=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable12=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: enable11=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable10=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable9=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable8=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: enable7=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable6=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable5=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable4=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: enable3=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable2=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable1=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable0=0x%01x", (unsigned int)  pFromStruct->m_bClassEnb0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," trans=0x%02x", (unsigned int)  pFromStruct->m_nCountTrans);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPT:: resrv1=0x%02x", (unsigned int)  pFromStruct->m_nReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," prepend=0x%01x", (unsigned int)  pFromStruct->m_bPrepend);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," valid=0x%01x", (unsigned int)  pFromStruct->m_bInstValid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," inst=0x%08x", (unsigned int)  pFromStruct->m_Instruction);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibPT_Validate(sbZfSbQe2000ElibPT_t *pZf) {

  if (pZf->m_bClassEnb15 > 0x1) return 0;
  if (pZf->m_bClassEnb14 > 0x1) return 0;
  if (pZf->m_bClassEnb13 > 0x1) return 0;
  if (pZf->m_bClassEnb12 > 0x1) return 0;
  if (pZf->m_bClassEnb11 > 0x1) return 0;
  if (pZf->m_bClassEnb10 > 0x1) return 0;
  if (pZf->m_bClassEnb9 > 0x1) return 0;
  if (pZf->m_bClassEnb8 > 0x1) return 0;
  if (pZf->m_bClassEnb7 > 0x1) return 0;
  if (pZf->m_bClassEnb6 > 0x1) return 0;
  if (pZf->m_bClassEnb5 > 0x1) return 0;
  if (pZf->m_bClassEnb4 > 0x1) return 0;
  if (pZf->m_bClassEnb3 > 0x1) return 0;
  if (pZf->m_bClassEnb2 > 0x1) return 0;
  if (pZf->m_bClassEnb1 > 0x1) return 0;
  if (pZf->m_bClassEnb0 > 0x1) return 0;
  if (pZf->m_nCountTrans > 0xff) return 0;
  if (pZf->m_nReserved1 > 0x7f) return 0;
  if (pZf->m_bPrepend > 0x1) return 0;
  if (pZf->m_bInstValid > 0x1) return 0;
  if (pZf->m_Instruction > 0x7fffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibPT_SetField(sbZfSbQe2000ElibPT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "classenb15") == 0) {
    s->m_bClassEnb15 = value;
  } else if (SB_STRCMP(name, "classenb14") == 0) {
    s->m_bClassEnb14 = value;
  } else if (SB_STRCMP(name, "classenb13") == 0) {
    s->m_bClassEnb13 = value;
  } else if (SB_STRCMP(name, "classenb12") == 0) {
    s->m_bClassEnb12 = value;
  } else if (SB_STRCMP(name, "classenb11") == 0) {
    s->m_bClassEnb11 = value;
  } else if (SB_STRCMP(name, "classenb10") == 0) {
    s->m_bClassEnb10 = value;
  } else if (SB_STRCMP(name, "classenb9") == 0) {
    s->m_bClassEnb9 = value;
  } else if (SB_STRCMP(name, "classenb8") == 0) {
    s->m_bClassEnb8 = value;
  } else if (SB_STRCMP(name, "classenb7") == 0) {
    s->m_bClassEnb7 = value;
  } else if (SB_STRCMP(name, "classenb6") == 0) {
    s->m_bClassEnb6 = value;
  } else if (SB_STRCMP(name, "classenb5") == 0) {
    s->m_bClassEnb5 = value;
  } else if (SB_STRCMP(name, "classenb4") == 0) {
    s->m_bClassEnb4 = value;
  } else if (SB_STRCMP(name, "classenb3") == 0) {
    s->m_bClassEnb3 = value;
  } else if (SB_STRCMP(name, "classenb2") == 0) {
    s->m_bClassEnb2 = value;
  } else if (SB_STRCMP(name, "classenb1") == 0) {
    s->m_bClassEnb1 = value;
  } else if (SB_STRCMP(name, "classenb0") == 0) {
    s->m_bClassEnb0 = value;
  } else if (SB_STRCMP(name, "m_ncounttrans") == 0) {
    s->m_nCountTrans = value;
  } else if (SB_STRCMP(name, "m_nreserved1") == 0) {
    s->m_nReserved1 = value;
  } else if (SB_STRCMP(name, "prepend") == 0) {
    s->m_bPrepend = value;
  } else if (SB_STRCMP(name, "instvalid") == 0) {
    s->m_bInstValid = value;
  } else if (SB_STRCMP(name, "m_instruction") == 0) {
    s->m_Instruction = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
