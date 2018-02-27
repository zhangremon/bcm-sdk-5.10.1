/*
 * $Id: sbZfKaEpInstructionConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaEpInstructionConsole.hx"



/* Print members in struct */
void
sbZfKaEpInstruction_Print(sbZfKaEpInstruction_t *pFromStruct) {
  SB_LOG("KaEpInstruction:: valid=0x%01x", (unsigned int)  pFromStruct->m_nValid);
  SB_LOG(" opcode=0x%02x", (unsigned int)  pFromStruct->m_nOpcode);
  SB_LOG(" opav=0x%01x", (unsigned int)  pFromStruct->m_nOpAVariable);
  SB_LOG(" opa=0x%02x", (unsigned int)  pFromStruct->m_nOperandA);
  SB_LOG(" opbv=0x%01x", (unsigned int)  pFromStruct->m_nOpBVariable);
  SB_LOG(" opb=0x%02x", (unsigned int)  pFromStruct->m_nOperandB);
  SB_LOG("\n");

  SB_LOG("KaEpInstruction:: opcv=0x%01x", (unsigned int)  pFromStruct->m_nOpCVariable);
  SB_LOG(" opc=0x%03x", (unsigned int)  pFromStruct->m_nOperandC);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaEpInstruction_SPrint(sbZfKaEpInstruction_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpInstruction:: valid=0x%01x", (unsigned int)  pFromStruct->m_nValid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opcode=0x%02x", (unsigned int)  pFromStruct->m_nOpcode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opav=0x%01x", (unsigned int)  pFromStruct->m_nOpAVariable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opa=0x%02x", (unsigned int)  pFromStruct->m_nOperandA);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opbv=0x%01x", (unsigned int)  pFromStruct->m_nOpBVariable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opb=0x%02x", (unsigned int)  pFromStruct->m_nOperandB);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpInstruction:: opcv=0x%01x", (unsigned int)  pFromStruct->m_nOpCVariable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opc=0x%03x", (unsigned int)  pFromStruct->m_nOperandC);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpInstruction_Validate(sbZfKaEpInstruction_t *pZf) {

  if (pZf->m_nValid > 0x1) return 0;
  if (pZf->m_nOpcode > 0x3f) return 0;
  if (pZf->m_nOpAVariable > 0x1) return 0;
  if (pZf->m_nOperandA > 0x3f) return 0;
  if (pZf->m_nOpBVariable > 0x1) return 0;
  if (pZf->m_nOperandB > 0x3f) return 0;
  if (pZf->m_nOpCVariable > 0x1) return 0;
  if (pZf->m_nOperandC > 0x3ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpInstruction_SetField(sbZfKaEpInstruction_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nvalid") == 0) {
    s->m_nValid = value;
  } else if (SB_STRCMP(name, "m_nopcode") == 0) {
    s->m_nOpcode = value;
  } else if (SB_STRCMP(name, "m_nopavariable") == 0) {
    s->m_nOpAVariable = value;
  } else if (SB_STRCMP(name, "m_noperanda") == 0) {
    s->m_nOperandA = value;
  } else if (SB_STRCMP(name, "m_nopbvariable") == 0) {
    s->m_nOpBVariable = value;
  } else if (SB_STRCMP(name, "m_noperandb") == 0) {
    s->m_nOperandB = value;
  } else if (SB_STRCMP(name, "m_nopcvariable") == 0) {
    s->m_nOpCVariable = value;
  } else if (SB_STRCMP(name, "m_noperandc") == 0) {
    s->m_nOperandC = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
