/*
 * $Id: sbZfFe2000PmPolicerConfigConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFe2000PmPolicerConfigConsole.hx"



/* Print members in struct */
void
sbZfFe2000PmPolicerConfig_Print(sbZfFe2000PmPolicerConfig_t *pFromStruct) {
  SB_LOG("Fe2000PmPolicerConfig:: rfcmode=0x%01x", (unsigned int)  pFromStruct->uRfcMode);
  SB_LOG(" lenshift=0x%01x", (unsigned int)  pFromStruct->uLenShift);
  SB_LOG(" runits=0x%08x", (unsigned int)  pFromStruct->uRate);
  SB_LOG("\n");

  SB_LOG("Fe2000PmPolicerConfig:: cbs=0x%08x", (unsigned int)  pFromStruct->uCBS);
  SB_LOG(" cir=0x%08x", (unsigned int)  pFromStruct->uCIR);
  SB_LOG(" ebs=0x%08x", (unsigned int)  pFromStruct->uEBS);
  SB_LOG("\n");

  SB_LOG("Fe2000PmPolicerConfig:: eir=0x%08x", (unsigned int)  pFromStruct->uEIR);
  SB_LOG(" rsvd=0x%01x", (unsigned int)  pFromStruct->uRsvd);
  SB_LOG(" bmode=0x%01x", (unsigned int)  pFromStruct->bBlindMode);
  SB_LOG(" dred=0x%01x", (unsigned int)  pFromStruct->bDropOnRed);
  SB_LOG("\n");

  SB_LOG("Fe2000PmPolicerConfig:: coupling=0x%01x", (unsigned int)  pFromStruct->bCoupling);
  SB_LOG(" cbsnodcr=0x%01x", (unsigned int)  pFromStruct->bCBSNoDecrement);
  SB_LOG(" ebsnodcr=0x%01x", (unsigned int)  pFromStruct->bEBSNoDecrement);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfFe2000PmPolicerConfig_SPrint(sbZfFe2000PmPolicerConfig_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PmPolicerConfig:: rfcmode=0x%01x", (unsigned int)  pFromStruct->uRfcMode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lenshift=0x%01x", (unsigned int)  pFromStruct->uLenShift);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," runits=0x%08x", (unsigned int)  pFromStruct->uRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PmPolicerConfig:: cbs=0x%08x", (unsigned int)  pFromStruct->uCBS);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cir=0x%08x", (unsigned int)  pFromStruct->uCIR);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ebs=0x%08x", (unsigned int)  pFromStruct->uEBS);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PmPolicerConfig:: eir=0x%08x", (unsigned int)  pFromStruct->uEIR);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd=0x%01x", (unsigned int)  pFromStruct->uRsvd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bmode=0x%01x", (unsigned int)  pFromStruct->bBlindMode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dred=0x%01x", (unsigned int)  pFromStruct->bDropOnRed);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PmPolicerConfig:: coupling=0x%01x", (unsigned int)  pFromStruct->bCoupling);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cbsnodcr=0x%01x", (unsigned int)  pFromStruct->bCBSNoDecrement);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ebsnodcr=0x%01x", (unsigned int)  pFromStruct->bEBSNoDecrement);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFe2000PmPolicerConfig_Validate(sbZfFe2000PmPolicerConfig_t *pZf) {

  if (pZf->uRfcMode > 0x3) return 0;
  if (pZf->uLenShift > 0x7) return 0;
  /* pZf->uRate implicitly masked by data type */
  /* pZf->uCBS implicitly masked by data type */
  /* pZf->uCIR implicitly masked by data type */
  /* pZf->uEBS implicitly masked by data type */
  /* pZf->uEIR implicitly masked by data type */
  if (pZf->uRsvd > 0x1) return 0;
  if (pZf->bBlindMode > 0x1) return 0;
  if (pZf->bDropOnRed > 0x1) return 0;
  if (pZf->bCoupling > 0x1) return 0;
  if (pZf->bCBSNoDecrement > 0x1) return 0;
  if (pZf->bEBSNoDecrement > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFe2000PmPolicerConfig_SetField(sbZfFe2000PmPolicerConfig_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "urfcmode") == 0) {
    s->uRfcMode = value;
  } else if (SB_STRCMP(name, "ulenshift") == 0) {
    s->uLenShift = value;
  } else if (SB_STRCMP(name, "urate") == 0) {
    s->uRate = value;
  } else if (SB_STRCMP(name, "ucbs") == 0) {
    s->uCBS = value;
  } else if (SB_STRCMP(name, "ucir") == 0) {
    s->uCIR = value;
  } else if (SB_STRCMP(name, "uebs") == 0) {
    s->uEBS = value;
  } else if (SB_STRCMP(name, "ueir") == 0) {
    s->uEIR = value;
  } else if (SB_STRCMP(name, "ursvd") == 0) {
    s->uRsvd = value;
  } else if (SB_STRCMP(name, "blindmode") == 0) {
    s->bBlindMode = value;
  } else if (SB_STRCMP(name, "droponred") == 0) {
    s->bDropOnRed = value;
  } else if (SB_STRCMP(name, "coupling") == 0) {
    s->bCoupling = value;
  } else if (SB_STRCMP(name, "cbsnodecrement") == 0) {
    s->bCBSNoDecrement = value;
  } else if (SB_STRCMP(name, "ebsnodecrement") == 0) {
    s->bEBSNoDecrement = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
