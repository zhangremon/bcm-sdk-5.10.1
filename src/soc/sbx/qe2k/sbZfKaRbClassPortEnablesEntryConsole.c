/*
 * $Id: sbZfKaRbClassPortEnablesEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaRbClassPortEnablesEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassPortEnablesEntry_Print(sbZfKaRbClassPortEnablesEntry_t *pFromStruct) {
  SB_LOG("KaRbClassPortEnablesEntry:: reserve=0x%03x", (unsigned int)  pFromStruct->m_nReserve);
  SB_LOG(" useuser1=0x%01x", (unsigned int)  pFromStruct->m_nUseUser1);
  SB_LOG(" useuser0=0x%01x", (unsigned int)  pFromStruct->m_nUseUser0);
  SB_LOG("\n");

  SB_LOG("KaRbClassPortEnablesEntry:: usevlanpri=0x%01x", (unsigned int)  pFromStruct->m_nUseVlanPri);
  SB_LOG(" usehiprivlan=0x%01x", (unsigned int)  pFromStruct->m_nUseHiPriVlan);
  SB_LOG("\n");

  SB_LOG("KaRbClassPortEnablesEntry:: usedmacmatch=0x%01x", (unsigned int)  pFromStruct->m_nUseDmacMatch);
  SB_LOG(" uselayer4=0x%01x", (unsigned int)  pFromStruct->m_nUseLayer4);
  SB_LOG(" flowhashenb=0x%01x", (unsigned int)  pFromStruct->m_nFlowHashEnable);
  SB_LOG("\n");

  SB_LOG("KaRbClassPortEnablesEntry:: usehashcos=0x%04x", (unsigned int)  pFromStruct->m_nUseHashCos);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaRbClassPortEnablesEntry_SPrint(sbZfKaRbClassPortEnablesEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassPortEnablesEntry:: reserve=0x%03x", (unsigned int)  pFromStruct->m_nReserve);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," useuser1=0x%01x", (unsigned int)  pFromStruct->m_nUseUser1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," useuser0=0x%01x", (unsigned int)  pFromStruct->m_nUseUser0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassPortEnablesEntry:: usevlanpri=0x%01x", (unsigned int)  pFromStruct->m_nUseVlanPri);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," usehiprivlan=0x%01x", (unsigned int)  pFromStruct->m_nUseHiPriVlan);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassPortEnablesEntry:: usedmacmatch=0x%01x", (unsigned int)  pFromStruct->m_nUseDmacMatch);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," uselayer4=0x%01x", (unsigned int)  pFromStruct->m_nUseLayer4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," flowhashenb=0x%01x", (unsigned int)  pFromStruct->m_nFlowHashEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassPortEnablesEntry:: usehashcos=0x%04x", (unsigned int)  pFromStruct->m_nUseHashCos);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassPortEnablesEntry_Validate(sbZfKaRbClassPortEnablesEntry_t *pZf) {

  if (pZf->m_nReserve > 0x1ff) return 0;
  if (pZf->m_nUseUser1 > 0x1) return 0;
  if (pZf->m_nUseUser0 > 0x1) return 0;
  if (pZf->m_nUseVlanPri > 0x1) return 0;
  if (pZf->m_nUseHiPriVlan > 0x1) return 0;
  if (pZf->m_nUseDmacMatch > 0x1) return 0;
  if (pZf->m_nUseLayer4 > 0x1) return 0;
  if (pZf->m_nFlowHashEnable > 0x1) return 0;
  if (pZf->m_nUseHashCos > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassPortEnablesEntry_SetField(sbZfKaRbClassPortEnablesEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserve") == 0) {
    s->m_nReserve = value;
  } else if (SB_STRCMP(name, "m_nuseuser1") == 0) {
    s->m_nUseUser1 = value;
  } else if (SB_STRCMP(name, "m_nuseuser0") == 0) {
    s->m_nUseUser0 = value;
  } else if (SB_STRCMP(name, "m_nusevlanpri") == 0) {
    s->m_nUseVlanPri = value;
  } else if (SB_STRCMP(name, "m_nusehiprivlan") == 0) {
    s->m_nUseHiPriVlan = value;
  } else if (SB_STRCMP(name, "m_nusedmacmatch") == 0) {
    s->m_nUseDmacMatch = value;
  } else if (SB_STRCMP(name, "m_nuselayer4") == 0) {
    s->m_nUseLayer4 = value;
  } else if (SB_STRCMP(name, "m_nflowhashenable") == 0) {
    s->m_nFlowHashEnable = value;
  } else if (SB_STRCMP(name, "m_nusehashcos") == 0) {
    s->m_nUseHashCos = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
