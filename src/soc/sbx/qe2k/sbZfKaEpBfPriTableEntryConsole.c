/*
 * $Id: sbZfKaEpBfPriTableEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaEpBfPriTableAddr.hx"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpBfPriTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEpBfPriTableEntry_Print(sbZfKaEpBfPriTableEntry_t *pFromStruct) {
  SB_LOG("KaEpBfPriTableEntry:: pri7=0x%01x", (unsigned int)  pFromStruct->m_nPri7);
  SB_LOG(" pri6=0x%01x", (unsigned int)  pFromStruct->m_nPri6);
  SB_LOG(" pri5=0x%01x", (unsigned int)  pFromStruct->m_nPri5);
  SB_LOG(" pri4=0x%01x", (unsigned int)  pFromStruct->m_nPri4);
  SB_LOG(" pri3=0x%01x", (unsigned int)  pFromStruct->m_nPri3);
  SB_LOG(" pri2=0x%01x", (unsigned int)  pFromStruct->m_nPri2);
  SB_LOG("\n");

  SB_LOG("KaEpBfPriTableEntry:: pri1=0x%01x", (unsigned int)  pFromStruct->m_nPri1);
  SB_LOG(" pri0=0x%01x", (unsigned int)  pFromStruct->m_nPri0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaEpBfPriTableEntry_SPrint(sbZfKaEpBfPriTableEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpBfPriTableEntry:: pri7=0x%01x", (unsigned int)  pFromStruct->m_nPri7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri6=0x%01x", (unsigned int)  pFromStruct->m_nPri6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri5=0x%01x", (unsigned int)  pFromStruct->m_nPri5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri4=0x%01x", (unsigned int)  pFromStruct->m_nPri4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri3=0x%01x", (unsigned int)  pFromStruct->m_nPri3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri2=0x%01x", (unsigned int)  pFromStruct->m_nPri2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpBfPriTableEntry:: pri1=0x%01x", (unsigned int)  pFromStruct->m_nPri1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri0=0x%01x", (unsigned int)  pFromStruct->m_nPri0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpBfPriTableEntry_Validate(sbZfKaEpBfPriTableEntry_t *pZf) {

  if (pZf->m_nPri7 > 0x7) return 0;
  if (pZf->m_nPri6 > 0x7) return 0;
  if (pZf->m_nPri5 > 0x7) return 0;
  if (pZf->m_nPri4 > 0x7) return 0;
  if (pZf->m_nPri3 > 0x7) return 0;
  if (pZf->m_nPri2 > 0x7) return 0;
  if (pZf->m_nPri1 > 0x7) return 0;
  if (pZf->m_nPri0 > 0x7) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpBfPriTableEntry_SetField(sbZfKaEpBfPriTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_npri7") == 0) {
    s->m_nPri7 = value;
  } else if (SB_STRCMP(name, "m_npri6") == 0) {
    s->m_nPri6 = value;
  } else if (SB_STRCMP(name, "m_npri5") == 0) {
    s->m_nPri5 = value;
  } else if (SB_STRCMP(name, "m_npri4") == 0) {
    s->m_nPri4 = value;
  } else if (SB_STRCMP(name, "m_npri3") == 0) {
    s->m_nPri3 = value;
  } else if (SB_STRCMP(name, "m_npri2") == 0) {
    s->m_nPri2 = value;
  } else if (SB_STRCMP(name, "m_npri1") == 0) {
    s->m_nPri1 = value;
  } else if (SB_STRCMP(name, "m_npri0") == 0) {
    s->m_nPri0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
