/*
 * $Id: sbZfC2PmProfilePolicerMemoryConsole.c 1.1.32.3 Broadcom SDK $
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
#include "sbZfC2PmProfilePolicerMemoryConsole.hx"



/* Print members in struct */
void
sbZfC2PmProfilePolicerMemory_Print(sbZfC2PmProfilePolicerMemory_t *pFromStruct) {
  SB_LOG("C2PmProfilePolicerMemory:: type=0x%01x", (unsigned int)  pFromStruct->uType);
  SB_LOG(" lenshift=0x%01x", (unsigned int)  pFromStruct->uLengthShift);
  SB_LOG(" cflag=0x%01x", (unsigned int)  pFromStruct->bCouplingFlag);
  SB_LOG(" r2698=0x%01x", (unsigned int)  pFromStruct->bRFC2698Mode);
  SB_LOG("\n");

  SB_LOG("C2PmProfilePolicerMemory:: bcsize=0x%02x", (unsigned int)  pFromStruct->uBktCSize);
  SB_LOG(" blind=0x%01x", (unsigned int)  pFromStruct->bBlind);
  SB_LOG(" benodcr=0x%01x", (unsigned int)  pFromStruct->bBktENoDecrement);
  SB_LOG(" eir=0x%05x", (unsigned int)  pFromStruct->uEIR);
  SB_LOG("\n");

  SB_LOG("C2PmProfilePolicerMemory:: ebs=0x%03x", (unsigned int)  pFromStruct->uEBS);
  SB_LOG(" dred=0x%01x", (unsigned int)  pFromStruct->bDropOnRed);
  SB_LOG(" bcnodcr=0x%01x", (unsigned int)  pFromStruct->bBktCNoDecrement);
  SB_LOG(" cir=0x%05x", (unsigned int)  pFromStruct->uCIR);
  SB_LOG("\n");

  SB_LOG("C2PmProfilePolicerMemory:: cbs=0x%03x", (unsigned int)  pFromStruct->uCBS);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PmProfilePolicerMemory_SPrint(sbZfC2PmProfilePolicerMemory_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfilePolicerMemory:: type=0x%01x", (unsigned int)  pFromStruct->uType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lenshift=0x%01x", (unsigned int)  pFromStruct->uLengthShift);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cflag=0x%01x", (unsigned int)  pFromStruct->bCouplingFlag);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," r2698=0x%01x", (unsigned int)  pFromStruct->bRFC2698Mode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfilePolicerMemory:: bcsize=0x%02x", (unsigned int)  pFromStruct->uBktCSize);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," blind=0x%01x", (unsigned int)  pFromStruct->bBlind);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," benodcr=0x%01x", (unsigned int)  pFromStruct->bBktENoDecrement);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," eir=0x%05x", (unsigned int)  pFromStruct->uEIR);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfilePolicerMemory:: ebs=0x%03x", (unsigned int)  pFromStruct->uEBS);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dred=0x%01x", (unsigned int)  pFromStruct->bDropOnRed);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bcnodcr=0x%01x", (unsigned int)  pFromStruct->bBktCNoDecrement);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cir=0x%05x", (unsigned int)  pFromStruct->uCIR);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PmProfilePolicerMemory:: cbs=0x%03x", (unsigned int)  pFromStruct->uCBS);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PmProfilePolicerMemory_Validate(sbZfC2PmProfilePolicerMemory_t *pZf) {

  if (pZf->uType > 0x1) return 0;
  if (pZf->uLengthShift > 0x7) return 0;
  if (pZf->bCouplingFlag > 0x1) return 0;
  if (pZf->bRFC2698Mode > 0x1) return 0;
  if (pZf->uBktCSize > 0x1f) return 0;
  if (pZf->bBlind > 0x1) return 0;
  if (pZf->bBktENoDecrement > 0x1) return 0;
  if (pZf->uEIR > 0x3ffff) return 0;
  if (pZf->uEBS > 0xfff) return 0;
  if (pZf->bDropOnRed > 0x1) return 0;
  if (pZf->bBktCNoDecrement > 0x1) return 0;
  if (pZf->uCIR > 0x3ffff) return 0;
  if (pZf->uCBS > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PmProfilePolicerMemory_SetField(sbZfC2PmProfilePolicerMemory_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "utype") == 0) {
    s->uType = value;
  } else if (SB_STRCMP(name, "ulengthshift") == 0) {
    s->uLengthShift = value;
  } else if (SB_STRCMP(name, "couplingflag") == 0) {
    s->bCouplingFlag = value;
  } else if (SB_STRCMP(name, "rfc2698mode") == 0) {
    s->bRFC2698Mode = value;
  } else if (SB_STRCMP(name, "ubktcsize") == 0) {
    s->uBktCSize = value;
  } else if (SB_STRCMP(name, "blind") == 0) {
    s->bBlind = value;
  } else if (SB_STRCMP(name, "bktenodecrement") == 0) {
    s->bBktENoDecrement = value;
  } else if (SB_STRCMP(name, "ueir") == 0) {
    s->uEIR = value;
  } else if (SB_STRCMP(name, "uebs") == 0) {
    s->uEBS = value;
  } else if (SB_STRCMP(name, "droponred") == 0) {
    s->bDropOnRed = value;
  } else if (SB_STRCMP(name, "bktcnodecrement") == 0) {
    s->bBktCNoDecrement = value;
  } else if (SB_STRCMP(name, "ucir") == 0) {
    s->uCIR = value;
  } else if (SB_STRCMP(name, "ucbs") == 0) {
    s->uCBS = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
