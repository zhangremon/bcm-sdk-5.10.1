/*
 * $Id: sbZfCaRcePatternConsole.c 1.1.48.3 Broadcom SDK $
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
#include "sbZfCaRcePatternConsole.hx"



/* Print members in struct */
void
sbZfCaRcePattern_Print(sbZfCaRcePattern_t *pFromStruct) {
  SB_LOG("CaRcePattern:: patd=0x%08x", (unsigned int)  pFromStruct->m_uPatternD);
  SB_LOG(" patc=0x%08x", (unsigned int)  pFromStruct->m_uPatternC);
  SB_LOG(" patb=0x%08x", (unsigned int)  pFromStruct->m_uPatternB);
  SB_LOG("\n");

  SB_LOG("CaRcePattern:: pata=0x%08x", (unsigned int)  pFromStruct->m_uPatternA);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaRcePattern_SPrint(sbZfCaRcePattern_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcePattern:: patd=0x%08x", (unsigned int)  pFromStruct->m_uPatternD);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," patc=0x%08x", (unsigned int)  pFromStruct->m_uPatternC);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," patb=0x%08x", (unsigned int)  pFromStruct->m_uPatternB);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcePattern:: pata=0x%08x", (unsigned int)  pFromStruct->m_uPatternA);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaRcePattern_Validate(sbZfCaRcePattern_t *pZf) {

  /* pZf->m_uPatternD implicitly masked by data type */
  /* pZf->m_uPatternC implicitly masked by data type */
  /* pZf->m_uPatternB implicitly masked by data type */
  /* pZf->m_uPatternA implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaRcePattern_SetField(sbZfCaRcePattern_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_upatternd") == 0) {
    s->m_uPatternD = value;
  } else if (SB_STRCMP(name, "m_upatternc") == 0) {
    s->m_uPatternC = value;
  } else if (SB_STRCMP(name, "m_upatternb") == 0) {
    s->m_uPatternB = value;
  } else if (SB_STRCMP(name, "m_upatterna") == 0) {
    s->m_uPatternA = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
