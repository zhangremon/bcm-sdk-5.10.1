/*
 * $Id: sbZfFe2000ClsInstructionConsole.c 1.3.36.3 Broadcom SDK $
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
#include "sbZfFe2000ClsInstructionConsole.hx"



/* Print members in struct */
void
sbZfFe2000ClsInstruction_Print(sbZfFe2000ClsInstruction_t *pFromStruct) {
  SB_LOG("Fe2000ClsInstruction:: opmsb=0x%01x", (unsigned int)  pFromStruct->uOpCodeMsb);
  SB_LOG(" oplsb=0x%01x", (unsigned int)  pFromStruct->uOpCodeLsb);
  SB_LOG(" sfld=0x%01x", (unsigned int)  pFromStruct->uSField);
  SB_LOG(" mfld=0x%01x", (unsigned int)  pFromStruct->uMField);
  SB_LOG(" afld=0x%02x", (unsigned int)  pFromStruct->uAField);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfFe2000ClsInstruction_SPrint(sbZfFe2000ClsInstruction_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000ClsInstruction:: opmsb=0x%01x", (unsigned int)  pFromStruct->uOpCodeMsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," oplsb=0x%01x", (unsigned int)  pFromStruct->uOpCodeLsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sfld=0x%01x", (unsigned int)  pFromStruct->uSField);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mfld=0x%01x", (unsigned int)  pFromStruct->uMField);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," afld=0x%02x", (unsigned int)  pFromStruct->uAField);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFe2000ClsInstruction_Validate(sbZfFe2000ClsInstruction_t *pZf) {

  if (pZf->uOpCodeMsb > 0x3) return 0;
  if (pZf->uOpCodeLsb > 0x3) return 0;
  if (pZf->uSField > 0xf) return 0;
  if (pZf->uMField > 0xf) return 0;
  if (pZf->uAField > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFe2000ClsInstruction_SetField(sbZfFe2000ClsInstruction_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "uopcodemsb") == 0) {
    s->uOpCodeMsb = value;
  } else if (SB_STRCMP(name, "uopcodelsb") == 0) {
    s->uOpCodeLsb = value;
  } else if (SB_STRCMP(name, "usfield") == 0) {
    s->uSField = value;
  } else if (SB_STRCMP(name, "umfield") == 0) {
    s->uMField = value;
  } else if (SB_STRCMP(name, "uafield") == 0) {
    s->uAField = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
