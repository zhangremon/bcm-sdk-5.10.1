/*
 * $Id: sbZfKaSrManualDeskewEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaSrManualDeskewEntryConsole.hx"



/* Print members in struct */
void
sbZfKaSrManualDeskewEntry_Print(sbZfKaSrManualDeskewEntry_t *pFromStruct) {
  SB_LOG("KaSrManualDeskewEntry:: lane16=0x%02x", (unsigned int)  pFromStruct->m_nLane16);
  SB_LOG(" lane15=0x%02x", (unsigned int)  pFromStruct->m_nLane15);
  SB_LOG(" lane14=0x%02x", (unsigned int)  pFromStruct->m_nLane14);
  SB_LOG(" lane13=0x%02x", (unsigned int)  pFromStruct->m_nLane13);
  SB_LOG("\n");

  SB_LOG("KaSrManualDeskewEntry:: lane12=0x%02x", (unsigned int)  pFromStruct->m_nLane12);
  SB_LOG(" lane11=0x%02x", (unsigned int)  pFromStruct->m_nLane11);
  SB_LOG(" lane10=0x%02x", (unsigned int)  pFromStruct->m_nLane10);
  SB_LOG(" lane9=0x%02x", (unsigned int)  pFromStruct->m_nLane9);
  SB_LOG("\n");

  SB_LOG("KaSrManualDeskewEntry:: lane8=0x%02x", (unsigned int)  pFromStruct->m_nLane8);
  SB_LOG(" lane7=0x%02x", (unsigned int)  pFromStruct->m_nLane7);
  SB_LOG(" lane6=0x%02x", (unsigned int)  pFromStruct->m_nLane6);
  SB_LOG(" lane5=0x%02x", (unsigned int)  pFromStruct->m_nLane5);
  SB_LOG("\n");

  SB_LOG("KaSrManualDeskewEntry:: lane4=0x%02x", (unsigned int)  pFromStruct->m_nLane4);
  SB_LOG(" lane3=0x%02x", (unsigned int)  pFromStruct->m_nLane3);
  SB_LOG(" lane2=0x%02x", (unsigned int)  pFromStruct->m_nLane2);
  SB_LOG(" lane1=0x%02x", (unsigned int)  pFromStruct->m_nLane1);
  SB_LOG("\n");

  SB_LOG("KaSrManualDeskewEntry:: lane0=0x%02x", (unsigned int)  pFromStruct->m_nLane0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaSrManualDeskewEntry_SPrint(sbZfKaSrManualDeskewEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane16=0x%02x", (unsigned int)  pFromStruct->m_nLane16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane15=0x%02x", (unsigned int)  pFromStruct->m_nLane15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane14=0x%02x", (unsigned int)  pFromStruct->m_nLane14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane13=0x%02x", (unsigned int)  pFromStruct->m_nLane13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane12=0x%02x", (unsigned int)  pFromStruct->m_nLane12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane11=0x%02x", (unsigned int)  pFromStruct->m_nLane11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane10=0x%02x", (unsigned int)  pFromStruct->m_nLane10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane9=0x%02x", (unsigned int)  pFromStruct->m_nLane9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane8=0x%02x", (unsigned int)  pFromStruct->m_nLane8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane7=0x%02x", (unsigned int)  pFromStruct->m_nLane7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane6=0x%02x", (unsigned int)  pFromStruct->m_nLane6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane5=0x%02x", (unsigned int)  pFromStruct->m_nLane5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane4=0x%02x", (unsigned int)  pFromStruct->m_nLane4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane3=0x%02x", (unsigned int)  pFromStruct->m_nLane3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane2=0x%02x", (unsigned int)  pFromStruct->m_nLane2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lane1=0x%02x", (unsigned int)  pFromStruct->m_nLane1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaSrManualDeskewEntry:: lane0=0x%02x", (unsigned int)  pFromStruct->m_nLane0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaSrManualDeskewEntry_Validate(sbZfKaSrManualDeskewEntry_t *pZf) {

  if (pZf->m_nLane16 > 0x1f) return 0;
  if (pZf->m_nLane15 > 0x1f) return 0;
  if (pZf->m_nLane14 > 0x1f) return 0;
  if (pZf->m_nLane13 > 0x1f) return 0;
  if (pZf->m_nLane12 > 0x1f) return 0;
  if (pZf->m_nLane11 > 0x1f) return 0;
  if (pZf->m_nLane10 > 0x1f) return 0;
  if (pZf->m_nLane9 > 0x1f) return 0;
  if (pZf->m_nLane8 > 0x1f) return 0;
  if (pZf->m_nLane7 > 0x1f) return 0;
  if (pZf->m_nLane6 > 0x1f) return 0;
  if (pZf->m_nLane5 > 0x1f) return 0;
  if (pZf->m_nLane4 > 0x1f) return 0;
  if (pZf->m_nLane3 > 0x1f) return 0;
  if (pZf->m_nLane2 > 0x1f) return 0;
  if (pZf->m_nLane1 > 0x1f) return 0;
  if (pZf->m_nLane0 > 0x1f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaSrManualDeskewEntry_SetField(sbZfKaSrManualDeskewEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nlane16") == 0) {
    s->m_nLane16 = value;
  } else if (SB_STRCMP(name, "m_nlane15") == 0) {
    s->m_nLane15 = value;
  } else if (SB_STRCMP(name, "m_nlane14") == 0) {
    s->m_nLane14 = value;
  } else if (SB_STRCMP(name, "m_nlane13") == 0) {
    s->m_nLane13 = value;
  } else if (SB_STRCMP(name, "m_nlane12") == 0) {
    s->m_nLane12 = value;
  } else if (SB_STRCMP(name, "m_nlane11") == 0) {
    s->m_nLane11 = value;
  } else if (SB_STRCMP(name, "m_nlane10") == 0) {
    s->m_nLane10 = value;
  } else if (SB_STRCMP(name, "m_nlane9") == 0) {
    s->m_nLane9 = value;
  } else if (SB_STRCMP(name, "m_nlane8") == 0) {
    s->m_nLane8 = value;
  } else if (SB_STRCMP(name, "m_nlane7") == 0) {
    s->m_nLane7 = value;
  } else if (SB_STRCMP(name, "m_nlane6") == 0) {
    s->m_nLane6 = value;
  } else if (SB_STRCMP(name, "m_nlane5") == 0) {
    s->m_nLane5 = value;
  } else if (SB_STRCMP(name, "m_nlane4") == 0) {
    s->m_nLane4 = value;
  } else if (SB_STRCMP(name, "m_nlane3") == 0) {
    s->m_nLane3 = value;
  } else if (SB_STRCMP(name, "m_nlane2") == 0) {
    s->m_nLane2 = value;
  } else if (SB_STRCMP(name, "m_nlane1") == 0) {
    s->m_nLane1 = value;
  } else if (SB_STRCMP(name, "m_nlane0") == 0) {
    s->m_nLane0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
