/*
 * $Id: sbZfKaQsLnaNextPriEntryConsole.c 1.2.12.3 Broadcom SDK $
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
#include "sbZfKaQsLnaNextPriEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsLnaNextPriEntry_Print(sbZfKaQsLnaNextPriEntry_t *pFromStruct) {
  SB_LOG("KaQsLnaNextPriEntry:: selport=0x%02x", (unsigned int)  pFromStruct->m_nSelPort);
  SB_LOG(" nextpri24=0x%01x", (unsigned int)  pFromStruct->m_nNextPri24);
  SB_LOG(" nextpri23=0x%01x", (unsigned int)  pFromStruct->m_nNextPri23);
  SB_LOG("\n");

  SB_LOG("KaQsLnaNextPriEntry:: nextpri22=0x%01x", (unsigned int)  pFromStruct->m_nNextPri22);
  SB_LOG(" nextpri21=0x%01x", (unsigned int)  pFromStruct->m_nNextPri21);
  SB_LOG(" nextpri20=0x%01x", (unsigned int)  pFromStruct->m_nNextPri20);
  SB_LOG("\n");

  SB_LOG("KaQsLnaNextPriEntry:: nextpri19=0x%01x", (unsigned int)  pFromStruct->m_nNextPri19);
  SB_LOG(" nextpri18=0x%01x", (unsigned int)  pFromStruct->m_nNextPri18);
  SB_LOG(" nextpri17=0x%01x", (unsigned int)  pFromStruct->m_nNextPri17);
  SB_LOG("\n");

  SB_LOG("KaQsLnaNextPriEntry:: nextpri16=0x%01x", (unsigned int)  pFromStruct->m_nNextPri16);
  SB_LOG(" nextpri15=0x%01x", (unsigned int)  pFromStruct->m_nNextPri15);
  SB_LOG(" nextpri14=0x%01x", (unsigned int)  pFromStruct->m_nNextPri14);
  SB_LOG("\n");

  SB_LOG("KaQsLnaNextPriEntry:: nextpri13=0x%01x", (unsigned int)  pFromStruct->m_nNextPri13);
  SB_LOG(" nextpri12=0x%01x", (unsigned int)  pFromStruct->m_nNextPri12);
  SB_LOG(" nextpri11=0x%01x", (unsigned int)  pFromStruct->m_nNextPri11);
  SB_LOG("\n");

  SB_LOG("KaQsLnaNextPriEntry:: nextpri10=0x%01x", (unsigned int)  pFromStruct->m_nNextPri10);
  SB_LOG(" nextpri9=0x%01x", (unsigned int)  pFromStruct->m_nNextPri9);
  SB_LOG(" nextpri8=0x%01x", (unsigned int)  pFromStruct->m_nNextPri8);
  SB_LOG(" nextpri7=0x%01x", (unsigned int)  pFromStruct->m_nNextPri7);
  SB_LOG("\n");

  SB_LOG("KaQsLnaNextPriEntry:: nextpri6=0x%01x", (unsigned int)  pFromStruct->m_nNextPri6);
  SB_LOG(" nextpri5=0x%01x", (unsigned int)  pFromStruct->m_nNextPri5);
  SB_LOG(" nextpri4=0x%01x", (unsigned int)  pFromStruct->m_nNextPri4);
  SB_LOG(" nextpri3=0x%01x", (unsigned int)  pFromStruct->m_nNextPri3);
  SB_LOG("\n");

  SB_LOG("KaQsLnaNextPriEntry:: nextpri2=0x%01x", (unsigned int)  pFromStruct->m_nNextPri2);
  SB_LOG(" nextpri1=0x%01x", (unsigned int)  pFromStruct->m_nNextPri1);
  SB_LOG(" nextpri0=0x%01x", (unsigned int)  pFromStruct->m_nNextPri0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaQsLnaNextPriEntry_SPrint(sbZfKaQsLnaNextPriEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: selport=0x%02x", (unsigned int)  pFromStruct->m_nSelPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri24=0x%01x", (unsigned int)  pFromStruct->m_nNextPri24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri23=0x%01x", (unsigned int)  pFromStruct->m_nNextPri23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri22=0x%01x", (unsigned int)  pFromStruct->m_nNextPri22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri21=0x%01x", (unsigned int)  pFromStruct->m_nNextPri21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri20=0x%01x", (unsigned int)  pFromStruct->m_nNextPri20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri19=0x%01x", (unsigned int)  pFromStruct->m_nNextPri19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri18=0x%01x", (unsigned int)  pFromStruct->m_nNextPri18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri17=0x%01x", (unsigned int)  pFromStruct->m_nNextPri17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri16=0x%01x", (unsigned int)  pFromStruct->m_nNextPri16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri15=0x%01x", (unsigned int)  pFromStruct->m_nNextPri15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri14=0x%01x", (unsigned int)  pFromStruct->m_nNextPri14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri13=0x%01x", (unsigned int)  pFromStruct->m_nNextPri13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri12=0x%01x", (unsigned int)  pFromStruct->m_nNextPri12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri11=0x%01x", (unsigned int)  pFromStruct->m_nNextPri11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri10=0x%01x", (unsigned int)  pFromStruct->m_nNextPri10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri9=0x%01x", (unsigned int)  pFromStruct->m_nNextPri9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri8=0x%01x", (unsigned int)  pFromStruct->m_nNextPri8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri7=0x%01x", (unsigned int)  pFromStruct->m_nNextPri7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri6=0x%01x", (unsigned int)  pFromStruct->m_nNextPri6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri5=0x%01x", (unsigned int)  pFromStruct->m_nNextPri5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri4=0x%01x", (unsigned int)  pFromStruct->m_nNextPri4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri3=0x%01x", (unsigned int)  pFromStruct->m_nNextPri3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaNextPriEntry:: nextpri2=0x%01x", (unsigned int)  pFromStruct->m_nNextPri2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri1=0x%01x", (unsigned int)  pFromStruct->m_nNextPri1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri0=0x%01x", (unsigned int)  pFromStruct->m_nNextPri0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsLnaNextPriEntry_Validate(sbZfKaQsLnaNextPriEntry_t *pZf) {

  if (pZf->m_nSelPort > 0x3f) return 0;
  if (pZf->m_nNextPri24 > 0xf) return 0;
  if (pZf->m_nNextPri23 > 0xf) return 0;
  if (pZf->m_nNextPri22 > 0xf) return 0;
  if (pZf->m_nNextPri21 > 0xf) return 0;
  if (pZf->m_nNextPri20 > 0xf) return 0;
  if (pZf->m_nNextPri19 > 0xf) return 0;
  if (pZf->m_nNextPri18 > 0xf) return 0;
  if (pZf->m_nNextPri17 > 0xf) return 0;
  if (pZf->m_nNextPri16 > 0xf) return 0;
  if (pZf->m_nNextPri15 > 0xf) return 0;
  if (pZf->m_nNextPri14 > 0xf) return 0;
  if (pZf->m_nNextPri13 > 0xf) return 0;
  if (pZf->m_nNextPri12 > 0xf) return 0;
  if (pZf->m_nNextPri11 > 0xf) return 0;
  if (pZf->m_nNextPri10 > 0xf) return 0;
  if (pZf->m_nNextPri9 > 0xf) return 0;
  if (pZf->m_nNextPri8 > 0xf) return 0;
  if (pZf->m_nNextPri7 > 0xf) return 0;
  if (pZf->m_nNextPri6 > 0xf) return 0;
  if (pZf->m_nNextPri5 > 0xf) return 0;
  if (pZf->m_nNextPri4 > 0xf) return 0;
  if (pZf->m_nNextPri3 > 0xf) return 0;
  if (pZf->m_nNextPri2 > 0xf) return 0;
  if (pZf->m_nNextPri1 > 0xf) return 0;
  if (pZf->m_nNextPri0 > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsLnaNextPriEntry_SetField(sbZfKaQsLnaNextPriEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nselport") == 0) {
    s->m_nSelPort = value;
  } else if (SB_STRCMP(name, "m_nnextpri24") == 0) {
    s->m_nNextPri24 = value;
  } else if (SB_STRCMP(name, "m_nnextpri23") == 0) {
    s->m_nNextPri23 = value;
  } else if (SB_STRCMP(name, "m_nnextpri22") == 0) {
    s->m_nNextPri22 = value;
  } else if (SB_STRCMP(name, "m_nnextpri21") == 0) {
    s->m_nNextPri21 = value;
  } else if (SB_STRCMP(name, "m_nnextpri20") == 0) {
    s->m_nNextPri20 = value;
  } else if (SB_STRCMP(name, "m_nnextpri19") == 0) {
    s->m_nNextPri19 = value;
  } else if (SB_STRCMP(name, "m_nnextpri18") == 0) {
    s->m_nNextPri18 = value;
  } else if (SB_STRCMP(name, "m_nnextpri17") == 0) {
    s->m_nNextPri17 = value;
  } else if (SB_STRCMP(name, "m_nnextpri16") == 0) {
    s->m_nNextPri16 = value;
  } else if (SB_STRCMP(name, "m_nnextpri15") == 0) {
    s->m_nNextPri15 = value;
  } else if (SB_STRCMP(name, "m_nnextpri14") == 0) {
    s->m_nNextPri14 = value;
  } else if (SB_STRCMP(name, "m_nnextpri13") == 0) {
    s->m_nNextPri13 = value;
  } else if (SB_STRCMP(name, "m_nnextpri12") == 0) {
    s->m_nNextPri12 = value;
  } else if (SB_STRCMP(name, "m_nnextpri11") == 0) {
    s->m_nNextPri11 = value;
  } else if (SB_STRCMP(name, "m_nnextpri10") == 0) {
    s->m_nNextPri10 = value;
  } else if (SB_STRCMP(name, "m_nnextpri9") == 0) {
    s->m_nNextPri9 = value;
  } else if (SB_STRCMP(name, "m_nnextpri8") == 0) {
    s->m_nNextPri8 = value;
  } else if (SB_STRCMP(name, "m_nnextpri7") == 0) {
    s->m_nNextPri7 = value;
  } else if (SB_STRCMP(name, "m_nnextpri6") == 0) {
    s->m_nNextPri6 = value;
  } else if (SB_STRCMP(name, "m_nnextpri5") == 0) {
    s->m_nNextPri5 = value;
  } else if (SB_STRCMP(name, "m_nnextpri4") == 0) {
    s->m_nNextPri4 = value;
  } else if (SB_STRCMP(name, "m_nnextpri3") == 0) {
    s->m_nNextPri3 = value;
  } else if (SB_STRCMP(name, "m_nnextpri2") == 0) {
    s->m_nNextPri2 = value;
  } else if (SB_STRCMP(name, "m_nnextpri1") == 0) {
    s->m_nNextPri1 = value;
  } else if (SB_STRCMP(name, "m_nnextpri0") == 0) {
    s->m_nNextPri0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
