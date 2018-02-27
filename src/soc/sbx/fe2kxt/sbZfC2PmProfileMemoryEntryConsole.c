/*
 * $Id: sbZfC2PmProfileMemoryEntryConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2PmProfileMemoryEntryConsole.hx"



/* Print members in struct */
void
sbZfC2PmProfileMemoryEntry_Print(sbZfC2PmProfileMemoryEntry_t *pFromStruct) {
  SB_LOG("C2PmProfileMemoryEntry:: type2=0x%01x", (unsigned int)  pFromStruct->m_uType2);
  SB_LOG(" lengthshift=0x%01x", (unsigned int)  pFromStruct->m_uLengthShift);
  SB_LOG(" couplingflag=0x%01x", (unsigned int)  pFromStruct->m_uCouplingFlag);
  SB_LOG("\n");

  SB_LOG("C2PmProfileMemoryEntry:: rfcmode=0x%01x", (unsigned int)  pFromStruct->m_uRfcMode);
  SB_LOG(" commitedbucketsize=0x%02x", (unsigned int)  pFromStruct->m_uCommitedBucketSize);
  SB_LOG(" colorblind=0x%01x", (unsigned int)  pFromStruct->m_uColorBlind);
  SB_LOG("\n");

  SB_LOG("C2PmProfileMemoryEntry:: excessbucketnodecrement=0x%01x", (unsigned int)  pFromStruct->m_uExcessBucketNoDecrement);
  SB_LOG("\n");

  SB_LOG("C2PmProfileMemoryEntry:: excessinformationrate=0x%05x", (unsigned int)  pFromStruct->m_uExcessInformationRate);
  SB_LOG("\n");

  SB_LOG("C2PmProfileMemoryEntry:: excessburstsize=0x%03x", (unsigned int)  pFromStruct->m_uExcessBurstSize);
  SB_LOG(" droponred=0x%01x", (unsigned int)  pFromStruct->m_uDropOnRed);
  SB_LOG("\n");

  SB_LOG("C2PmProfileMemoryEntry:: committedbucketnodecrement=0x%01x", (unsigned int)  pFromStruct->m_uCommittedBucketNoDecrement);
  SB_LOG("\n");

  SB_LOG("C2PmProfileMemoryEntry:: committedinformationrate=0x%05x", (unsigned int)  pFromStruct->m_uCommittedInformationRate);
  SB_LOG("\n");

  SB_LOG("C2PmProfileMemoryEntry:: committedburstsize=0x%03x", (unsigned int)  pFromStruct->m_uCommittedBurstSize);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PmProfileMemoryEntry_SPrint(sbZfC2PmProfileMemoryEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfileMemoryEntry:: type2=0x%01x", (unsigned int)  pFromStruct->m_uType2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lengthshift=0x%01x", (unsigned int)  pFromStruct->m_uLengthShift);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," couplingflag=0x%01x", (unsigned int)  pFromStruct->m_uCouplingFlag);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfileMemoryEntry:: rfcmode=0x%01x", (unsigned int)  pFromStruct->m_uRfcMode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," commitedbucketsize=0x%02x", (unsigned int)  pFromStruct->m_uCommitedBucketSize);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," colorblind=0x%01x", (unsigned int)  pFromStruct->m_uColorBlind);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfileMemoryEntry:: excessbucketnodecrement=0x%01x", (unsigned int)  pFromStruct->m_uExcessBucketNoDecrement);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfileMemoryEntry:: excessinformationrate=0x%05x", (unsigned int)  pFromStruct->m_uExcessInformationRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfileMemoryEntry:: excessburstsize=0x%03x", (unsigned int)  pFromStruct->m_uExcessBurstSize);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," droponred=0x%01x", (unsigned int)  pFromStruct->m_uDropOnRed);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfileMemoryEntry:: committedbucketnodecrement=0x%01x", (unsigned int)  pFromStruct->m_uCommittedBucketNoDecrement);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfileMemoryEntry:: committedinformationrate=0x%05x", (unsigned int)  pFromStruct->m_uCommittedInformationRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfileMemoryEntry:: committedburstsize=0x%03x", (unsigned int)  pFromStruct->m_uCommittedBurstSize);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PmProfileMemoryEntry_Validate(sbZfC2PmProfileMemoryEntry_t *pZf) {

  if (pZf->m_uType2 > 0x1) return 0;
  if (pZf->m_uLengthShift > 0x7) return 0;
  if (pZf->m_uCouplingFlag > 0x1) return 0;
  if (pZf->m_uRfcMode > 0x1) return 0;
  if (pZf->m_uCommitedBucketSize > 0x1f) return 0;
  if (pZf->m_uColorBlind > 0x1) return 0;
  if (pZf->m_uExcessBucketNoDecrement > 0x1) return 0;
  if (pZf->m_uExcessInformationRate > 0x3ffff) return 0;
  if (pZf->m_uExcessBurstSize > 0xfff) return 0;
  if (pZf->m_uDropOnRed > 0x1) return 0;
  if (pZf->m_uCommittedBucketNoDecrement > 0x1) return 0;
  if (pZf->m_uCommittedInformationRate > 0x3ffff) return 0;
  if (pZf->m_uCommittedBurstSize > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PmProfileMemoryEntry_SetField(sbZfC2PmProfileMemoryEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_utype2") == 0) {
    s->m_uType2 = value;
  } else if (SB_STRCMP(name, "m_ulengthshift") == 0) {
    s->m_uLengthShift = value;
  } else if (SB_STRCMP(name, "m_ucouplingflag") == 0) {
    s->m_uCouplingFlag = value;
  } else if (SB_STRCMP(name, "m_urfcmode") == 0) {
    s->m_uRfcMode = value;
  } else if (SB_STRCMP(name, "m_ucommitedbucketsize") == 0) {
    s->m_uCommitedBucketSize = value;
  } else if (SB_STRCMP(name, "m_ucolorblind") == 0) {
    s->m_uColorBlind = value;
  } else if (SB_STRCMP(name, "m_uexcessbucketnodecrement") == 0) {
    s->m_uExcessBucketNoDecrement = value;
  } else if (SB_STRCMP(name, "m_uexcessinformationrate") == 0) {
    s->m_uExcessInformationRate = value;
  } else if (SB_STRCMP(name, "m_uexcessburstsize") == 0) {
    s->m_uExcessBurstSize = value;
  } else if (SB_STRCMP(name, "m_udroponred") == 0) {
    s->m_uDropOnRed = value;
  } else if (SB_STRCMP(name, "m_ucommittedbucketnodecrement") == 0) {
    s->m_uCommittedBucketNoDecrement = value;
  } else if (SB_STRCMP(name, "m_ucommittedinformationrate") == 0) {
    s->m_uCommittedInformationRate = value;
  } else if (SB_STRCMP(name, "m_ucommittedburstsize") == 0) {
    s->m_uCommittedBurstSize = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
