/*
 * $Id: sbZfCaPmSequenceGenStateConsole.c 1.1.48.3 Broadcom SDK $
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
#include "sbZfCaPmSequenceGenStateConsole.hx"



/* Print members in struct */
void
sbZfCaPmSequenceGenState_Print(sbZfCaPmSequenceGenState_t *pFromStruct) {
  SB_LOG("CaPmSequenceGenState:: profile=0x%03x", (unsigned int)  pFromStruct->m_uProfile);
  SB_LOG(" reserve=0x%06x", (unsigned int)  pFromStruct->m_uReserved);
  SB_LOG(" seqnum=0x%08x", (unsigned int)  pFromStruct->m_uSequenceNumber);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaPmSequenceGenState_SPrint(sbZfCaPmSequenceGenState_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmSequenceGenState:: profile=0x%03x", (unsigned int)  pFromStruct->m_uProfile);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserve=0x%06x", (unsigned int)  pFromStruct->m_uReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," seqnum=0x%08x", (unsigned int)  pFromStruct->m_uSequenceNumber);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaPmSequenceGenState_Validate(sbZfCaPmSequenceGenState_t *pZf) {

  if (pZf->m_uProfile > 0x7ff) return 0;
  if (pZf->m_uReserved > 0x1fffff) return 0;
  /* pZf->m_uSequenceNumber implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaPmSequenceGenState_SetField(sbZfCaPmSequenceGenState_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uprofile") == 0) {
    s->m_uProfile = value;
  } else if (SB_STRCMP(name, "m_ureserved") == 0) {
    s->m_uReserved = value;
  } else if (SB_STRCMP(name, "m_usequencenumber") == 0) {
    s->m_uSequenceNumber = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
