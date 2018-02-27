/*
 * $Id: sbZfKaRbClassHashInputW0Console.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaRbClassHashInputW0Console.hx"



/* Print members in struct */
void
sbZfKaRbClassHashInputW0_Print(sbZfKaRbClassHashInputW0_t *pFromStruct) {
  SB_LOG("KaRbClassHashInputW0:: dmac=0x%012llx", (uint64_t)  pFromStruct->m_nnDmac);
  SB_LOG(" smac=0x%012llx", (uint64_t)  pFromStruct->m_nnSmac);
  SB_LOG(" sp1=0x%04x", (unsigned int)  pFromStruct->m_nSpare1);
  SB_LOG("\n");

  SB_LOG("KaRbClassHashInputW0:: iport=0x%02x", (unsigned int)  pFromStruct->m_nIPort);
  SB_LOG(" vlanid=0x%03x", (unsigned int)  pFromStruct->m_nVlanId);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaRbClassHashInputW0_SPrint(sbZfKaRbClassHashInputW0_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashInputW0:: dmac=0x%012llx", (uint64_t)  pFromStruct->m_nnDmac);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," smac=0x%012llx", (uint64_t)  pFromStruct->m_nnSmac);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sp1=0x%04x", (unsigned int)  pFromStruct->m_nSpare1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashInputW0:: iport=0x%02x", (unsigned int)  pFromStruct->m_nIPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," vlanid=0x%03x", (unsigned int)  pFromStruct->m_nVlanId);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassHashInputW0_Validate(sbZfKaRbClassHashInputW0_t *pZf) {

  if (pZf->m_nnDmac > 0xFFFFFFFFFFFFULL) return 0;
  if (pZf->m_nnSmac > 0xFFFFFFFFFFFFULL) return 0;
  if (pZf->m_nSpare1 > 0x3fff) return 0;
  if (pZf->m_nIPort > 0x3f) return 0;
  if (pZf->m_nVlanId > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassHashInputW0_SetField(sbZfKaRbClassHashInputW0_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nndmac") == 0) {
    s->m_nnDmac = value;
  } else if (SB_STRCMP(name, "m_nnsmac") == 0) {
    s->m_nnSmac = value;
  } else if (SB_STRCMP(name, "m_nspare1") == 0) {
    s->m_nSpare1 = value;
  } else if (SB_STRCMP(name, "m_niport") == 0) {
    s->m_nIPort = value;
  } else if (SB_STRCMP(name, "m_nvlanid") == 0) {
    s->m_nVlanId = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
