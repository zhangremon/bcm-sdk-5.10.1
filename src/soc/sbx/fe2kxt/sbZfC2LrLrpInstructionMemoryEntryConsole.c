/*
 * $Id: sbZfC2LrLrpInstructionMemoryEntryConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2LrLrpInstructionMemoryEntryConsole.hx"



/* Print members in struct */
void
sbZfC2LrLrpInstructionMemoryEntry_Print(sbZfC2LrLrpInstructionMemoryEntry_t *pFromStruct) {
  SB_LOG("C2LrLrpInstructionMemoryEntry:: thread=0x%01x", (unsigned int)  pFromStruct->m_uThread);
  SB_LOG(" opcode=0x%03x", (unsigned int)  pFromStruct->m_uOpcode);
  SB_LOG(" ra=0x%01x", (unsigned int)  pFromStruct->m_uRa);
  SB_LOG(" rb=0x%01x", (unsigned int)  pFromStruct->m_uRb);
  SB_LOG("\n");

  SB_LOG("C2LrLrpInstructionMemoryEntry:: target=0x%05x", (unsigned int)  pFromStruct->m_uTarget);
  SB_LOG(" sourcea=0x%08x", (unsigned int)  pFromStruct->m_uSourceA);
  SB_LOG("\n");

  SB_LOG("C2LrLrpInstructionMemoryEntry:: sourceb=0x%04x", (unsigned int)  pFromStruct->m_uSourceB);
  SB_LOG(" reserved=0x%04x", (unsigned int)  pFromStruct->m_uReserved);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2LrLrpInstructionMemoryEntry_SPrint(sbZfC2LrLrpInstructionMemoryEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2LrLrpInstructionMemoryEntry:: thread=0x%01x", (unsigned int)  pFromStruct->m_uThread);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," opcode=0x%03x", (unsigned int)  pFromStruct->m_uOpcode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ra=0x%01x", (unsigned int)  pFromStruct->m_uRa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rb=0x%01x", (unsigned int)  pFromStruct->m_uRb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2LrLrpInstructionMemoryEntry:: target=0x%05x", (unsigned int)  pFromStruct->m_uTarget);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sourcea=0x%08x", (unsigned int)  pFromStruct->m_uSourceA);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2LrLrpInstructionMemoryEntry:: sourceb=0x%04x", (unsigned int)  pFromStruct->m_uSourceB);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserved=0x%04x", (unsigned int)  pFromStruct->m_uReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2LrLrpInstructionMemoryEntry_Validate(sbZfC2LrLrpInstructionMemoryEntry_t *pZf) {

  if (pZf->m_uThread > 0x7) return 0;
  if (pZf->m_uOpcode > 0x1ff) return 0;
  if (pZf->m_uRa > 0x1) return 0;
  if (pZf->m_uRb > 0x1) return 0;
  if (pZf->m_uTarget > 0x3ffff) return 0;
  /* pZf->m_uSourceA implicitly masked by data type */
  if (pZf->m_uSourceB > 0xffff) return 0;
  if (pZf->m_uReserved > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2LrLrpInstructionMemoryEntry_SetField(sbZfC2LrLrpInstructionMemoryEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uthread") == 0) {
    s->m_uThread = value;
  } else if (SB_STRCMP(name, "m_uopcode") == 0) {
    s->m_uOpcode = value;
  } else if (SB_STRCMP(name, "m_ura") == 0) {
    s->m_uRa = value;
  } else if (SB_STRCMP(name, "m_urb") == 0) {
    s->m_uRb = value;
  } else if (SB_STRCMP(name, "m_utarget") == 0) {
    s->m_uTarget = value;
  } else if (SB_STRCMP(name, "m_usourcea") == 0) {
    s->m_uSourceA = value;
  } else if (SB_STRCMP(name, "m_usourceb") == 0) {
    s->m_uSourceB = value;
  } else if (SB_STRCMP(name, "m_ureserved") == 0) {
    s->m_uReserved = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
