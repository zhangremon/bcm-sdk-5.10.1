/*
 * $Id: sbZfFabBm9600LinkFailureInfoConsole.c 1.1.38.3 Broadcom SDK $
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
#include "sbZfFabBm9600LinkFailureInfoConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600LinkFailureInfo_Print(sbZfFabBm9600LinkFailureInfo_t *pFromStruct) {
  SB_LOG("FabBm9600LinkFailureInfo:: qelinkstate=0x%02x", (unsigned int)  pFromStruct->m_bQeLinkState);
  SB_LOG(" expectinglinkerror=0x%01x", (unsigned int)  pFromStruct->m_bExpectingLinkError);
  SB_LOG("\n");

  SB_LOG("FabBm9600LinkFailureInfo:: digestscanstart=0x%016llx", (uint64_t)  pFromStruct->m_uDigestScanStartTime);
  SB_LOG("\n");

  SB_LOG("FabBm9600LinkFailureInfo:: digestscanend=0x%016llx", (uint64_t)  pFromStruct->m_uDigestScanEndTime);
  SB_LOG("\n");

  SB_LOG("FabBm9600LinkFailureInfo:: dutbaseaddress=0x%08x", (unsigned int)  pFromStruct->m_uDUTBaseAddress);
  SB_LOG(" activebmemode=0x%01x", (unsigned int)  pFromStruct->m_uActiveBmeFoMode);
  SB_LOG("\n");

  SB_LOG("FabBm9600LinkFailureInfo:: activebmescilink=0x%01x", (unsigned int)  pFromStruct->m_uActiveBmeSciLink);
  SB_LOG("\n");

  SB_LOG("FabBm9600LinkFailureInfo:: expectedlinkstate=0x%08x", (unsigned int)  pFromStruct->m_uExpectedGlobalLinkState);
  SB_LOG(" inanumber=0x%02x", (unsigned int)  pFromStruct->m_uInaNumber);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfFabBm9600LinkFailureInfo_SPrint(sbZfFabBm9600LinkFailureInfo_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: qelinkstate=0x%02x", (unsigned int)  pFromStruct->m_bQeLinkState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," expectinglinkerror=0x%01x", (unsigned int)  pFromStruct->m_bExpectingLinkError);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: digestscanstart=0x%016llx", (uint64_t)  pFromStruct->m_uDigestScanStartTime);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: digestscanend=0x%016llx", (uint64_t)  pFromStruct->m_uDigestScanEndTime);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: dutbaseaddress=0x%08x", (unsigned int)  pFromStruct->m_uDUTBaseAddress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," activebmemode=0x%01x", (unsigned int)  pFromStruct->m_uActiveBmeFoMode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: activebmescilink=0x%01x", (unsigned int)  pFromStruct->m_uActiveBmeSciLink);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600LinkFailureInfo:: expectedlinkstate=0x%08x", (unsigned int)  pFromStruct->m_uExpectedGlobalLinkState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," inanumber=0x%02x", (unsigned int)  pFromStruct->m_uInaNumber);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600LinkFailureInfo_Validate(sbZfFabBm9600LinkFailureInfo_t *pZf) {

  if (pZf->m_bQeLinkState > 0x1f) return 0;
  if (pZf->m_bExpectingLinkError > 0x1) return 0;
  /* pZf->m_uDigestScanStartTime implicitly masked by data type */
  /* pZf->m_uDigestScanEndTime implicitly masked by data type */
  /* pZf->m_uDUTBaseAddress implicitly masked by data type */
  if (pZf->m_uActiveBmeFoMode > 0x3) return 0;
  if (pZf->m_uActiveBmeSciLink > 0x1) return 0;
  /* pZf->m_uExpectedGlobalLinkState implicitly masked by data type */
  if (pZf->m_uInaNumber > 0x7f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600LinkFailureInfo_SetField(sbZfFabBm9600LinkFailureInfo_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "qelinkstate") == 0) {
    s->m_bQeLinkState = value;
  } else if (SB_STRCMP(name, "expectinglinkerror") == 0) {
    s->m_bExpectingLinkError = value;
  } else if (SB_STRCMP(name, "m_udigestscanstarttime") == 0) {
    s->m_uDigestScanStartTime = value;
  } else if (SB_STRCMP(name, "m_udigestscanendtime") == 0) {
    s->m_uDigestScanEndTime = value;
  } else if (SB_STRCMP(name, "m_udutbaseaddress") == 0) {
    s->m_uDUTBaseAddress = value;
  } else if (SB_STRCMP(name, "m_uactivebmefomode") == 0) {
    s->m_uActiveBmeFoMode = value;
  } else if (SB_STRCMP(name, "m_uactivebmescilink") == 0) {
    s->m_uActiveBmeSciLink = value;
  } else if (SB_STRCMP(name, "m_uexpectedgloballinkstate") == 0) {
    s->m_uExpectedGlobalLinkState = value;
  } else if (SB_STRCMP(name, "m_uinanumber") == 0) {
    s->m_uInaNumber = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
