/*
 * $Id: sbZfFabBm9600NmFullStatusEntryConsole.c 1.1.16.3 Broadcom SDK $
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
#include "sbZfFabBm9600NmFullStatusEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600NmFullStatusEntry_Print(sbZfFabBm9600NmFullStatusEntry_t *pFromStruct) {
  SB_LOG("FabBm9600NmFullStatusEntry:: fs8=0x%03x", (unsigned int)  pFromStruct->m_FullStatus8);
  SB_LOG(" fs7=0x%08x", (unsigned int)  pFromStruct->m_FullStatus7);
  SB_LOG(" fs6=0x%08x", (unsigned int)  pFromStruct->m_FullStatus6);
  SB_LOG("\n");

  SB_LOG("FabBm9600NmFullStatusEntry:: fs5=0x%08x", (unsigned int)  pFromStruct->m_FullStatus5);
  SB_LOG(" fs4=0x%08x", (unsigned int)  pFromStruct->m_FullStatus4);
  SB_LOG(" fs3=0x%08x", (unsigned int)  pFromStruct->m_FullStatus3);
  SB_LOG("\n");

  SB_LOG("FabBm9600NmFullStatusEntry:: fs2=0x%08x", (unsigned int)  pFromStruct->m_FullStatus2);
  SB_LOG(" fs1=0x%08x", (unsigned int)  pFromStruct->m_FullStatus1);
  SB_LOG(" fs0=0x%08x", (unsigned int)  pFromStruct->m_FullStatus0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfFabBm9600NmFullStatusEntry_SPrint(sbZfFabBm9600NmFullStatusEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmFullStatusEntry:: fs8=0x%03x", (unsigned int)  pFromStruct->m_FullStatus8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs7=0x%08x", (unsigned int)  pFromStruct->m_FullStatus7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs6=0x%08x", (unsigned int)  pFromStruct->m_FullStatus6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmFullStatusEntry:: fs5=0x%08x", (unsigned int)  pFromStruct->m_FullStatus5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs4=0x%08x", (unsigned int)  pFromStruct->m_FullStatus4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs3=0x%08x", (unsigned int)  pFromStruct->m_FullStatus3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmFullStatusEntry:: fs2=0x%08x", (unsigned int)  pFromStruct->m_FullStatus2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs1=0x%08x", (unsigned int)  pFromStruct->m_FullStatus1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fs0=0x%08x", (unsigned int)  pFromStruct->m_FullStatus0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600NmFullStatusEntry_Validate(sbZfFabBm9600NmFullStatusEntry_t *pZf) {

  if (pZf->m_FullStatus8 > 0x3ff) return 0;
  /* pZf->m_FullStatus7 implicitly masked by data type */
  /* pZf->m_FullStatus6 implicitly masked by data type */
  /* pZf->m_FullStatus5 implicitly masked by data type */
  /* pZf->m_FullStatus4 implicitly masked by data type */
  /* pZf->m_FullStatus3 implicitly masked by data type */
  /* pZf->m_FullStatus2 implicitly masked by data type */
  /* pZf->m_FullStatus1 implicitly masked by data type */
  /* pZf->m_FullStatus0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600NmFullStatusEntry_SetField(sbZfFabBm9600NmFullStatusEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_fullstatus8") == 0) {
    s->m_FullStatus8 = value;
  } else if (SB_STRCMP(name, "m_fullstatus7") == 0) {
    s->m_FullStatus7 = value;
  } else if (SB_STRCMP(name, "m_fullstatus6") == 0) {
    s->m_FullStatus6 = value;
  } else if (SB_STRCMP(name, "m_fullstatus5") == 0) {
    s->m_FullStatus5 = value;
  } else if (SB_STRCMP(name, "m_fullstatus4") == 0) {
    s->m_FullStatus4 = value;
  } else if (SB_STRCMP(name, "m_fullstatus3") == 0) {
    s->m_FullStatus3 = value;
  } else if (SB_STRCMP(name, "m_fullstatus2") == 0) {
    s->m_FullStatus2 = value;
  } else if (SB_STRCMP(name, "m_fullstatus1") == 0) {
    s->m_FullStatus1 = value;
  } else if (SB_STRCMP(name, "m_fullstatus0") == 0) {
    s->m_FullStatus0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
