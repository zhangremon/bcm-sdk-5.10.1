/*
 * $Id: sbZfFabBm9600InaEsetPriEntryConsole.c 1.1.38.3 Broadcom SDK $
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
#include "sbZfFabBm9600InaEsetPriEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600InaEsetPriEntry_Print(sbZfFabBm9600InaEsetPriEntry_t *pFromStruct) {
  SB_LOG("FabBm9600InaEsetPriEntry:: pri_15=0x%01x", (unsigned int)  pFromStruct->m_uPri_15);
  SB_LOG(" nextpri_15=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_15);
  SB_LOG(" pri_14=0x%01x", (unsigned int)  pFromStruct->m_uPri_14);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaEsetPriEntry:: nextpri_14=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_14);
  SB_LOG(" pri_13=0x%01x", (unsigned int)  pFromStruct->m_uPri_13);
  SB_LOG(" nextpri_13=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_13);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaEsetPriEntry:: pri_12=0x%01x", (unsigned int)  pFromStruct->m_uPri_12);
  SB_LOG(" nextpri_12=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_12);
  SB_LOG(" pri_11=0x%01x", (unsigned int)  pFromStruct->m_uPri_11);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaEsetPriEntry:: nextpri_11=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_11);
  SB_LOG(" pri_10=0x%01x", (unsigned int)  pFromStruct->m_uPri_10);
  SB_LOG(" nextpri_10=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_10);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaEsetPriEntry:: pri_9=0x%01x", (unsigned int)  pFromStruct->m_uPri_9);
  SB_LOG(" nextpri_9=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_9);
  SB_LOG(" pri_8=0x%01x", (unsigned int)  pFromStruct->m_uPri_8);
  SB_LOG(" nextpri_8=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_8);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaEsetPriEntry:: pri_7=0x%01x", (unsigned int)  pFromStruct->m_uPri_7);
  SB_LOG(" nextpri_7=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_7);
  SB_LOG(" pri_6=0x%01x", (unsigned int)  pFromStruct->m_uPri_6);
  SB_LOG(" nextpri_6=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_6);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaEsetPriEntry:: pri_5=0x%01x", (unsigned int)  pFromStruct->m_uPri_5);
  SB_LOG(" nextpri_5=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_5);
  SB_LOG(" pri_4=0x%01x", (unsigned int)  pFromStruct->m_uPri_4);
  SB_LOG(" nextpri_4=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_4);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaEsetPriEntry:: pri_3=0x%01x", (unsigned int)  pFromStruct->m_uPri_3);
  SB_LOG(" nextpri_3=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_3);
  SB_LOG(" pri_2=0x%01x", (unsigned int)  pFromStruct->m_uPri_2);
  SB_LOG(" nextpri_2=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_2);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaEsetPriEntry:: pri_1=0x%01x", (unsigned int)  pFromStruct->m_uPri_1);
  SB_LOG(" nextpri_1=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_1);
  SB_LOG(" pri_0=0x%01x", (unsigned int)  pFromStruct->m_uPri_0);
  SB_LOG(" nextpri_0=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfFabBm9600InaEsetPriEntry_SPrint(sbZfFabBm9600InaEsetPriEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaEsetPriEntry:: pri_15=0x%01x", (unsigned int)  pFromStruct->m_uPri_15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_15=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_14=0x%01x", (unsigned int)  pFromStruct->m_uPri_14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaEsetPriEntry:: nextpri_14=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_13=0x%01x", (unsigned int)  pFromStruct->m_uPri_13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_13=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaEsetPriEntry:: pri_12=0x%01x", (unsigned int)  pFromStruct->m_uPri_12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_12=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_11=0x%01x", (unsigned int)  pFromStruct->m_uPri_11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaEsetPriEntry:: nextpri_11=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_10=0x%01x", (unsigned int)  pFromStruct->m_uPri_10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_10=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaEsetPriEntry:: pri_9=0x%01x", (unsigned int)  pFromStruct->m_uPri_9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_9=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_8=0x%01x", (unsigned int)  pFromStruct->m_uPri_8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_8=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaEsetPriEntry:: pri_7=0x%01x", (unsigned int)  pFromStruct->m_uPri_7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_7=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_6=0x%01x", (unsigned int)  pFromStruct->m_uPri_6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_6=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaEsetPriEntry:: pri_5=0x%01x", (unsigned int)  pFromStruct->m_uPri_5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_5=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_4=0x%01x", (unsigned int)  pFromStruct->m_uPri_4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_4=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaEsetPriEntry:: pri_3=0x%01x", (unsigned int)  pFromStruct->m_uPri_3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_3=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_2=0x%01x", (unsigned int)  pFromStruct->m_uPri_2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_2=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaEsetPriEntry:: pri_1=0x%01x", (unsigned int)  pFromStruct->m_uPri_1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_1=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri_0=0x%01x", (unsigned int)  pFromStruct->m_uPri_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nextpri_0=0x%01x", (unsigned int)  pFromStruct->m_uNextpri_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600InaEsetPriEntry_Validate(sbZfFabBm9600InaEsetPriEntry_t *pZf) {

  if (pZf->m_uPri_15 > 0xf) return 0;
  if (pZf->m_uNextpri_15 > 0xf) return 0;
  if (pZf->m_uPri_14 > 0xf) return 0;
  if (pZf->m_uNextpri_14 > 0xf) return 0;
  if (pZf->m_uPri_13 > 0xf) return 0;
  if (pZf->m_uNextpri_13 > 0xf) return 0;
  if (pZf->m_uPri_12 > 0xf) return 0;
  if (pZf->m_uNextpri_12 > 0xf) return 0;
  if (pZf->m_uPri_11 > 0xf) return 0;
  if (pZf->m_uNextpri_11 > 0xf) return 0;
  if (pZf->m_uPri_10 > 0xf) return 0;
  if (pZf->m_uNextpri_10 > 0xf) return 0;
  if (pZf->m_uPri_9 > 0xf) return 0;
  if (pZf->m_uNextpri_9 > 0xf) return 0;
  if (pZf->m_uPri_8 > 0xf) return 0;
  if (pZf->m_uNextpri_8 > 0xf) return 0;
  if (pZf->m_uPri_7 > 0xf) return 0;
  if (pZf->m_uNextpri_7 > 0xf) return 0;
  if (pZf->m_uPri_6 > 0xf) return 0;
  if (pZf->m_uNextpri_6 > 0xf) return 0;
  if (pZf->m_uPri_5 > 0xf) return 0;
  if (pZf->m_uNextpri_5 > 0xf) return 0;
  if (pZf->m_uPri_4 > 0xf) return 0;
  if (pZf->m_uNextpri_4 > 0xf) return 0;
  if (pZf->m_uPri_3 > 0xf) return 0;
  if (pZf->m_uNextpri_3 > 0xf) return 0;
  if (pZf->m_uPri_2 > 0xf) return 0;
  if (pZf->m_uNextpri_2 > 0xf) return 0;
  if (pZf->m_uPri_1 > 0xf) return 0;
  if (pZf->m_uNextpri_1 > 0xf) return 0;
  if (pZf->m_uPri_0 > 0xf) return 0;
  if (pZf->m_uNextpri_0 > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600InaEsetPriEntry_SetField(sbZfFabBm9600InaEsetPriEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_upri_15") == 0) {
    s->m_uPri_15 = value;
  } else if (SB_STRCMP(name, "m_unextpri_15") == 0) {
    s->m_uNextpri_15 = value;
  } else if (SB_STRCMP(name, "m_upri_14") == 0) {
    s->m_uPri_14 = value;
  } else if (SB_STRCMP(name, "m_unextpri_14") == 0) {
    s->m_uNextpri_14 = value;
  } else if (SB_STRCMP(name, "m_upri_13") == 0) {
    s->m_uPri_13 = value;
  } else if (SB_STRCMP(name, "m_unextpri_13") == 0) {
    s->m_uNextpri_13 = value;
  } else if (SB_STRCMP(name, "m_upri_12") == 0) {
    s->m_uPri_12 = value;
  } else if (SB_STRCMP(name, "m_unextpri_12") == 0) {
    s->m_uNextpri_12 = value;
  } else if (SB_STRCMP(name, "m_upri_11") == 0) {
    s->m_uPri_11 = value;
  } else if (SB_STRCMP(name, "m_unextpri_11") == 0) {
    s->m_uNextpri_11 = value;
  } else if (SB_STRCMP(name, "m_upri_10") == 0) {
    s->m_uPri_10 = value;
  } else if (SB_STRCMP(name, "m_unextpri_10") == 0) {
    s->m_uNextpri_10 = value;
  } else if (SB_STRCMP(name, "m_upri_9") == 0) {
    s->m_uPri_9 = value;
  } else if (SB_STRCMP(name, "m_unextpri_9") == 0) {
    s->m_uNextpri_9 = value;
  } else if (SB_STRCMP(name, "m_upri_8") == 0) {
    s->m_uPri_8 = value;
  } else if (SB_STRCMP(name, "m_unextpri_8") == 0) {
    s->m_uNextpri_8 = value;
  } else if (SB_STRCMP(name, "m_upri_7") == 0) {
    s->m_uPri_7 = value;
  } else if (SB_STRCMP(name, "m_unextpri_7") == 0) {
    s->m_uNextpri_7 = value;
  } else if (SB_STRCMP(name, "m_upri_6") == 0) {
    s->m_uPri_6 = value;
  } else if (SB_STRCMP(name, "m_unextpri_6") == 0) {
    s->m_uNextpri_6 = value;
  } else if (SB_STRCMP(name, "m_upri_5") == 0) {
    s->m_uPri_5 = value;
  } else if (SB_STRCMP(name, "m_unextpri_5") == 0) {
    s->m_uNextpri_5 = value;
  } else if (SB_STRCMP(name, "m_upri_4") == 0) {
    s->m_uPri_4 = value;
  } else if (SB_STRCMP(name, "m_unextpri_4") == 0) {
    s->m_uNextpri_4 = value;
  } else if (SB_STRCMP(name, "m_upri_3") == 0) {
    s->m_uPri_3 = value;
  } else if (SB_STRCMP(name, "m_unextpri_3") == 0) {
    s->m_uNextpri_3 = value;
  } else if (SB_STRCMP(name, "m_upri_2") == 0) {
    s->m_uPri_2 = value;
  } else if (SB_STRCMP(name, "m_unextpri_2") == 0) {
    s->m_uNextpri_2 = value;
  } else if (SB_STRCMP(name, "m_upri_1") == 0) {
    s->m_uPri_1 = value;
  } else if (SB_STRCMP(name, "m_unextpri_1") == 0) {
    s->m_uNextpri_1 = value;
  } else if (SB_STRCMP(name, "m_upri_0") == 0) {
    s->m_uPri_0 = value;
  } else if (SB_STRCMP(name, "m_unextpri_0") == 0) {
    s->m_uNextpri_0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
