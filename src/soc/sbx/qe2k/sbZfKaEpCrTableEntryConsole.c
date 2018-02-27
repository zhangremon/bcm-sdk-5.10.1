/*
 * $Id: sbZfKaEpCrTableEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaEpCrTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEpCrTableEntry_Print(sbZfKaEpCrTableEntry_t *pFromStruct) {
  SB_LOG("KaEpCrTableEntry:: class15=0x%01x", (unsigned int)  pFromStruct->m_nClass15);
  SB_LOG(" class14=0x%01x", (unsigned int)  pFromStruct->m_nClass14);
  SB_LOG(" class13=0x%01x", (unsigned int)  pFromStruct->m_nClass13);
  SB_LOG(" class12=0x%01x", (unsigned int)  pFromStruct->m_nClass12);
  SB_LOG("\n");

  SB_LOG("KaEpCrTableEntry:: class11=0x%01x", (unsigned int)  pFromStruct->m_nClass11);
  SB_LOG(" class10=0x%01x", (unsigned int)  pFromStruct->m_nClass10);
  SB_LOG(" class9=0x%01x", (unsigned int)  pFromStruct->m_nClass9);
  SB_LOG(" class8=0x%01x", (unsigned int)  pFromStruct->m_nClass8);
  SB_LOG(" class7=0x%01x", (unsigned int)  pFromStruct->m_nClass7);
  SB_LOG("\n");

  SB_LOG("KaEpCrTableEntry:: class6=0x%01x", (unsigned int)  pFromStruct->m_nClass6);
  SB_LOG(" class5=0x%01x", (unsigned int)  pFromStruct->m_nClass5);
  SB_LOG(" class4=0x%01x", (unsigned int)  pFromStruct->m_nClass4);
  SB_LOG(" class3=0x%01x", (unsigned int)  pFromStruct->m_nClass3);
  SB_LOG(" class2=0x%01x", (unsigned int)  pFromStruct->m_nClass2);
  SB_LOG("\n");

  SB_LOG("KaEpCrTableEntry:: class1=0x%01x", (unsigned int)  pFromStruct->m_nClass1);
  SB_LOG(" class0=0x%01x", (unsigned int)  pFromStruct->m_nClass0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaEpCrTableEntry_SPrint(sbZfKaEpCrTableEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpCrTableEntry:: class15=0x%01x", (unsigned int)  pFromStruct->m_nClass15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class14=0x%01x", (unsigned int)  pFromStruct->m_nClass14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class13=0x%01x", (unsigned int)  pFromStruct->m_nClass13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class12=0x%01x", (unsigned int)  pFromStruct->m_nClass12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpCrTableEntry:: class11=0x%01x", (unsigned int)  pFromStruct->m_nClass11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class10=0x%01x", (unsigned int)  pFromStruct->m_nClass10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class9=0x%01x", (unsigned int)  pFromStruct->m_nClass9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class8=0x%01x", (unsigned int)  pFromStruct->m_nClass8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class7=0x%01x", (unsigned int)  pFromStruct->m_nClass7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpCrTableEntry:: class6=0x%01x", (unsigned int)  pFromStruct->m_nClass6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class5=0x%01x", (unsigned int)  pFromStruct->m_nClass5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class4=0x%01x", (unsigned int)  pFromStruct->m_nClass4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class3=0x%01x", (unsigned int)  pFromStruct->m_nClass3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class2=0x%01x", (unsigned int)  pFromStruct->m_nClass2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpCrTableEntry:: class1=0x%01x", (unsigned int)  pFromStruct->m_nClass1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class0=0x%01x", (unsigned int)  pFromStruct->m_nClass0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpCrTableEntry_Validate(sbZfKaEpCrTableEntry_t *pZf) {

  if (pZf->m_nClass15 > 0xf) return 0;
  if (pZf->m_nClass14 > 0xf) return 0;
  if (pZf->m_nClass13 > 0xf) return 0;
  if (pZf->m_nClass12 > 0xf) return 0;
  if (pZf->m_nClass11 > 0xf) return 0;
  if (pZf->m_nClass10 > 0xf) return 0;
  if (pZf->m_nClass9 > 0xf) return 0;
  if (pZf->m_nClass8 > 0xf) return 0;
  if (pZf->m_nClass7 > 0xf) return 0;
  if (pZf->m_nClass6 > 0xf) return 0;
  if (pZf->m_nClass5 > 0xf) return 0;
  if (pZf->m_nClass4 > 0xf) return 0;
  if (pZf->m_nClass3 > 0xf) return 0;
  if (pZf->m_nClass2 > 0xf) return 0;
  if (pZf->m_nClass1 > 0xf) return 0;
  if (pZf->m_nClass0 > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpCrTableEntry_SetField(sbZfKaEpCrTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nclass15") == 0) {
    s->m_nClass15 = value;
  } else if (SB_STRCMP(name, "m_nclass14") == 0) {
    s->m_nClass14 = value;
  } else if (SB_STRCMP(name, "m_nclass13") == 0) {
    s->m_nClass13 = value;
  } else if (SB_STRCMP(name, "m_nclass12") == 0) {
    s->m_nClass12 = value;
  } else if (SB_STRCMP(name, "m_nclass11") == 0) {
    s->m_nClass11 = value;
  } else if (SB_STRCMP(name, "m_nclass10") == 0) {
    s->m_nClass10 = value;
  } else if (SB_STRCMP(name, "m_nclass9") == 0) {
    s->m_nClass9 = value;
  } else if (SB_STRCMP(name, "m_nclass8") == 0) {
    s->m_nClass8 = value;
  } else if (SB_STRCMP(name, "m_nclass7") == 0) {
    s->m_nClass7 = value;
  } else if (SB_STRCMP(name, "m_nclass6") == 0) {
    s->m_nClass6 = value;
  } else if (SB_STRCMP(name, "m_nclass5") == 0) {
    s->m_nClass5 = value;
  } else if (SB_STRCMP(name, "m_nclass4") == 0) {
    s->m_nClass4 = value;
  } else if (SB_STRCMP(name, "m_nclass3") == 0) {
    s->m_nClass3 = value;
  } else if (SB_STRCMP(name, "m_nclass2") == 0) {
    s->m_nClass2 = value;
  } else if (SB_STRCMP(name, "m_nclass1") == 0) {
    s->m_nClass1 = value;
  } else if (SB_STRCMP(name, "m_nclass0") == 0) {
    s->m_nClass0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
