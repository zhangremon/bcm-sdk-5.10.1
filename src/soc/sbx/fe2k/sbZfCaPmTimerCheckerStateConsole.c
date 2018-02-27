/*
 * $Id: sbZfCaPmTimerCheckerStateConsole.c 1.1.48.3 Broadcom SDK $
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
#include "sbZfCaPmTimerCheckerStateConsole.hx"



/* Print members in struct */
void
sbZfCaPmTimerCheckerState_Print(sbZfCaPmTimerCheckerState_t *pFromStruct) {
  SB_LOG("CaPmTimerCheckerState:: profile=0x%03x", (unsigned int)  pFromStruct->m_uProfile);
  SB_LOG(" timerstart=0x%01x", (unsigned int)  pFromStruct->m_bTimerStarted);
  SB_LOG(" timer=0x%05x", (unsigned int)  pFromStruct->m_uTimer);
  SB_LOG("\n");

  SB_LOG("CaPmTimerCheckerState:: seqnum=0x%08x", (unsigned int)  pFromStruct->m_uSequenceNumber);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaPmTimerCheckerState_SPrint(sbZfCaPmTimerCheckerState_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmTimerCheckerState:: profile=0x%03x", (unsigned int)  pFromStruct->m_uProfile);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," timerstart=0x%01x", (unsigned int)  pFromStruct->m_bTimerStarted);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," timer=0x%05x", (unsigned int)  pFromStruct->m_uTimer);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmTimerCheckerState:: seqnum=0x%08x", (unsigned int)  pFromStruct->m_uSequenceNumber);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaPmTimerCheckerState_Validate(sbZfCaPmTimerCheckerState_t *pZf) {

  if (pZf->m_uProfile > 0x7ff) return 0;
  if (pZf->m_bTimerStarted > 0x1) return 0;
  if (pZf->m_uTimer > 0xfffff) return 0;
  /* pZf->m_uSequenceNumber implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaPmTimerCheckerState_SetField(sbZfCaPmTimerCheckerState_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uprofile") == 0) {
    s->m_uProfile = value;
  } else if (SB_STRCMP(name, "timerstarted") == 0) {
    s->m_bTimerStarted = value;
  } else if (SB_STRCMP(name, "m_utimer") == 0) {
    s->m_uTimer = value;
  } else if (SB_STRCMP(name, "m_usequencenumber") == 0) {
    s->m_uSequenceNumber = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
