/*
 * $Id: sbZfFabBm9600NmSysportArrayEntryConsole.c 1.1.38.3 Broadcom SDK $
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
#include "sbZfFabBm9600NmSysportArrayEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600NmSysportArrayEntry_Print(sbZfFabBm9600NmSysportArrayEntry_t *pFromStruct) {
  SB_LOG("FabBm9600NmSysportArrayEntry:: spa_15=0x%03x", (unsigned int)  pFromStruct->m_uSpa_15);
  SB_LOG(" spa_14=0x%03x", (unsigned int)  pFromStruct->m_uSpa_14);
  SB_LOG(" spa_13=0x%03x", (unsigned int)  pFromStruct->m_uSpa_13);
  SB_LOG("\n");

  SB_LOG("FabBm9600NmSysportArrayEntry:: spa_12=0x%03x", (unsigned int)  pFromStruct->m_uSpa_12);
  SB_LOG(" spa_11=0x%03x", (unsigned int)  pFromStruct->m_uSpa_11);
  SB_LOG(" spa_10=0x%03x", (unsigned int)  pFromStruct->m_uSpa_10);
  SB_LOG("\n");

  SB_LOG("FabBm9600NmSysportArrayEntry:: spa_9=0x%03x", (unsigned int)  pFromStruct->m_uSpa_9);
  SB_LOG(" spa_8=0x%03x", (unsigned int)  pFromStruct->m_uSpa_8);
  SB_LOG(" spa_7=0x%03x", (unsigned int)  pFromStruct->m_uSpa_7);
  SB_LOG("\n");

  SB_LOG("FabBm9600NmSysportArrayEntry:: spa_6=0x%03x", (unsigned int)  pFromStruct->m_uSpa_6);
  SB_LOG(" spa_5=0x%03x", (unsigned int)  pFromStruct->m_uSpa_5);
  SB_LOG(" spa_4=0x%03x", (unsigned int)  pFromStruct->m_uSpa_4);
  SB_LOG("\n");

  SB_LOG("FabBm9600NmSysportArrayEntry:: spa_3=0x%03x", (unsigned int)  pFromStruct->m_uSpa_3);
  SB_LOG(" spa_2=0x%03x", (unsigned int)  pFromStruct->m_uSpa_2);
  SB_LOG(" spa_1=0x%03x", (unsigned int)  pFromStruct->m_uSpa_1);
  SB_LOG("\n");

  SB_LOG("FabBm9600NmSysportArrayEntry:: spa_0=0x%03x", (unsigned int)  pFromStruct->m_uSpa_0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfFabBm9600NmSysportArrayEntry_SPrint(sbZfFabBm9600NmSysportArrayEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_15=0x%03x", (unsigned int)  pFromStruct->m_uSpa_15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_14=0x%03x", (unsigned int)  pFromStruct->m_uSpa_14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_13=0x%03x", (unsigned int)  pFromStruct->m_uSpa_13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_12=0x%03x", (unsigned int)  pFromStruct->m_uSpa_12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_11=0x%03x", (unsigned int)  pFromStruct->m_uSpa_11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_10=0x%03x", (unsigned int)  pFromStruct->m_uSpa_10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_9=0x%03x", (unsigned int)  pFromStruct->m_uSpa_9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_8=0x%03x", (unsigned int)  pFromStruct->m_uSpa_8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_7=0x%03x", (unsigned int)  pFromStruct->m_uSpa_7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_6=0x%03x", (unsigned int)  pFromStruct->m_uSpa_6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_5=0x%03x", (unsigned int)  pFromStruct->m_uSpa_5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_4=0x%03x", (unsigned int)  pFromStruct->m_uSpa_4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_3=0x%03x", (unsigned int)  pFromStruct->m_uSpa_3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_2=0x%03x", (unsigned int)  pFromStruct->m_uSpa_2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spa_1=0x%03x", (unsigned int)  pFromStruct->m_uSpa_1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmSysportArrayEntry:: spa_0=0x%03x", (unsigned int)  pFromStruct->m_uSpa_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600NmSysportArrayEntry_Validate(sbZfFabBm9600NmSysportArrayEntry_t *pZf) {

  if (pZf->m_uSpa_15 > 0xfff) return 0;
  if (pZf->m_uSpa_14 > 0xfff) return 0;
  if (pZf->m_uSpa_13 > 0xfff) return 0;
  if (pZf->m_uSpa_12 > 0xfff) return 0;
  if (pZf->m_uSpa_11 > 0xfff) return 0;
  if (pZf->m_uSpa_10 > 0xfff) return 0;
  if (pZf->m_uSpa_9 > 0xfff) return 0;
  if (pZf->m_uSpa_8 > 0xfff) return 0;
  if (pZf->m_uSpa_7 > 0xfff) return 0;
  if (pZf->m_uSpa_6 > 0xfff) return 0;
  if (pZf->m_uSpa_5 > 0xfff) return 0;
  if (pZf->m_uSpa_4 > 0xfff) return 0;
  if (pZf->m_uSpa_3 > 0xfff) return 0;
  if (pZf->m_uSpa_2 > 0xfff) return 0;
  if (pZf->m_uSpa_1 > 0xfff) return 0;
  if (pZf->m_uSpa_0 > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600NmSysportArrayEntry_SetField(sbZfFabBm9600NmSysportArrayEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uspa_15") == 0) {
    s->m_uSpa_15 = value;
  } else if (SB_STRCMP(name, "m_uspa_14") == 0) {
    s->m_uSpa_14 = value;
  } else if (SB_STRCMP(name, "m_uspa_13") == 0) {
    s->m_uSpa_13 = value;
  } else if (SB_STRCMP(name, "m_uspa_12") == 0) {
    s->m_uSpa_12 = value;
  } else if (SB_STRCMP(name, "m_uspa_11") == 0) {
    s->m_uSpa_11 = value;
  } else if (SB_STRCMP(name, "m_uspa_10") == 0) {
    s->m_uSpa_10 = value;
  } else if (SB_STRCMP(name, "m_uspa_9") == 0) {
    s->m_uSpa_9 = value;
  } else if (SB_STRCMP(name, "m_uspa_8") == 0) {
    s->m_uSpa_8 = value;
  } else if (SB_STRCMP(name, "m_uspa_7") == 0) {
    s->m_uSpa_7 = value;
  } else if (SB_STRCMP(name, "m_uspa_6") == 0) {
    s->m_uSpa_6 = value;
  } else if (SB_STRCMP(name, "m_uspa_5") == 0) {
    s->m_uSpa_5 = value;
  } else if (SB_STRCMP(name, "m_uspa_4") == 0) {
    s->m_uSpa_4 = value;
  } else if (SB_STRCMP(name, "m_uspa_3") == 0) {
    s->m_uSpa_3 = value;
  } else if (SB_STRCMP(name, "m_uspa_2") == 0) {
    s->m_uSpa_2 = value;
  } else if (SB_STRCMP(name, "m_uspa_1") == 0) {
    s->m_uSpa_1 = value;
  } else if (SB_STRCMP(name, "m_uspa_0") == 0) {
    s->m_uSpa_0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
