/*
 * $Id: sbZfKaRbClassDefaultQEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaRbClassDefaultQEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassDefaultQEntry_Print(sbZfKaRbClassDefaultQEntry_t *pFromStruct) {
  SB_LOG("KaRbClassDefaultQEntry:: spare1=0x%04x", (unsigned int)  pFromStruct->m_nSpare1);
  SB_LOG(" defaultdp=0x%01x", (unsigned int)  pFromStruct->m_nDefaultDp);
  SB_LOG(" spare0=0x%01x", (unsigned int)  pFromStruct->m_nSpare0);
  SB_LOG("\n");

  SB_LOG("KaRbClassDefaultQEntry:: defaultq=0x%04x", (unsigned int)  pFromStruct->m_nDefaultQ);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaRbClassDefaultQEntry_SPrint(sbZfKaRbClassDefaultQEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDefaultQEntry:: spare1=0x%04x", (unsigned int)  pFromStruct->m_nSpare1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," defaultdp=0x%01x", (unsigned int)  pFromStruct->m_nDefaultDp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare0=0x%01x", (unsigned int)  pFromStruct->m_nSpare0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDefaultQEntry:: defaultq=0x%04x", (unsigned int)  pFromStruct->m_nDefaultQ);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassDefaultQEntry_Validate(sbZfKaRbClassDefaultQEntry_t *pZf) {

  if (pZf->m_nSpare1 > 0x3fff) return 0;
  if (pZf->m_nDefaultDp > 0x3) return 0;
  if (pZf->m_nSpare0 > 0x3) return 0;
  if (pZf->m_nDefaultQ > 0x3fff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassDefaultQEntry_SetField(sbZfKaRbClassDefaultQEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nspare1") == 0) {
    s->m_nSpare1 = value;
  } else if (SB_STRCMP(name, "m_ndefaultdp") == 0) {
    s->m_nDefaultDp = value;
  } else if (SB_STRCMP(name, "m_nspare0") == 0) {
    s->m_nSpare0 = value;
  } else if (SB_STRCMP(name, "m_ndefaultq") == 0) {
    s->m_nDefaultQ = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
