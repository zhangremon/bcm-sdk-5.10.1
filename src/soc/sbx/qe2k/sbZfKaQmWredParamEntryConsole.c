/*
 * $Id: sbZfKaQmWredParamEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaQmWredParamEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmWredParamEntry_Print(sbZfKaQmWredParamEntry_t *pFromStruct) {
  SB_LOG("KaQmWredParamEntry:: tmaxexceeded2=0x%01x", (unsigned int)  pFromStruct->m_nTMaxExceeded2);
  SB_LOG(" ecnexceeded2=0x%01x", (unsigned int)  pFromStruct->m_nEcnExceeded2);
  SB_LOG(" pdrop2=0x%03x", (unsigned int)  pFromStruct->m_nPDrop2);
  SB_LOG("\n");

  SB_LOG("KaQmWredParamEntry:: tmaxexceeded1=0x%01x", (unsigned int)  pFromStruct->m_nTMaxExceeded1);
  SB_LOG(" ecnexceeded1=0x%01x", (unsigned int)  pFromStruct->m_nEcnExceeded1);
  SB_LOG(" pdrop1=0x%03x", (unsigned int)  pFromStruct->m_nPDrop1);
  SB_LOG("\n");

  SB_LOG("KaQmWredParamEntry:: tmaxexceeded0=0x%01x", (unsigned int)  pFromStruct->m_nTMaxExceeded0);
  SB_LOG(" ecnexceeded0=0x%01x", (unsigned int)  pFromStruct->m_nEcnExceeded0);
  SB_LOG(" pdrop0=0x%03x", (unsigned int)  pFromStruct->m_nPDrop0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaQmWredParamEntry_SPrint(sbZfKaQmWredParamEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredParamEntry:: tmaxexceeded2=0x%01x", (unsigned int)  pFromStruct->m_nTMaxExceeded2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecnexceeded2=0x%01x", (unsigned int)  pFromStruct->m_nEcnExceeded2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdrop2=0x%03x", (unsigned int)  pFromStruct->m_nPDrop2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredParamEntry:: tmaxexceeded1=0x%01x", (unsigned int)  pFromStruct->m_nTMaxExceeded1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecnexceeded1=0x%01x", (unsigned int)  pFromStruct->m_nEcnExceeded1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdrop1=0x%03x", (unsigned int)  pFromStruct->m_nPDrop1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredParamEntry:: tmaxexceeded0=0x%01x", (unsigned int)  pFromStruct->m_nTMaxExceeded0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecnexceeded0=0x%01x", (unsigned int)  pFromStruct->m_nEcnExceeded0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdrop0=0x%03x", (unsigned int)  pFromStruct->m_nPDrop0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmWredParamEntry_Validate(sbZfKaQmWredParamEntry_t *pZf) {

  if (pZf->m_nTMaxExceeded2 > 0x1) return 0;
  if (pZf->m_nEcnExceeded2 > 0x1) return 0;
  if (pZf->m_nPDrop2 > 0x3ff) return 0;
  if (pZf->m_nTMaxExceeded1 > 0x1) return 0;
  if (pZf->m_nEcnExceeded1 > 0x1) return 0;
  if (pZf->m_nPDrop1 > 0x3ff) return 0;
  if (pZf->m_nTMaxExceeded0 > 0x1) return 0;
  if (pZf->m_nEcnExceeded0 > 0x1) return 0;
  if (pZf->m_nPDrop0 > 0x3ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmWredParamEntry_SetField(sbZfKaQmWredParamEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntmaxexceeded2") == 0) {
    s->m_nTMaxExceeded2 = value;
  } else if (SB_STRCMP(name, "m_necnexceeded2") == 0) {
    s->m_nEcnExceeded2 = value;
  } else if (SB_STRCMP(name, "m_npdrop2") == 0) {
    s->m_nPDrop2 = value;
  } else if (SB_STRCMP(name, "m_ntmaxexceeded1") == 0) {
    s->m_nTMaxExceeded1 = value;
  } else if (SB_STRCMP(name, "m_necnexceeded1") == 0) {
    s->m_nEcnExceeded1 = value;
  } else if (SB_STRCMP(name, "m_npdrop1") == 0) {
    s->m_nPDrop1 = value;
  } else if (SB_STRCMP(name, "m_ntmaxexceeded0") == 0) {
    s->m_nTMaxExceeded0 = value;
  } else if (SB_STRCMP(name, "m_necnexceeded0") == 0) {
    s->m_nEcnExceeded0 = value;
  } else if (SB_STRCMP(name, "m_npdrop0") == 0) {
    s->m_nPDrop0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
