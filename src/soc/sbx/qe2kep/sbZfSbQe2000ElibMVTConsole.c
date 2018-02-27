/*
 * $Id: sbZfSbQe2000ElibMVTConsole.c 1.3.36.3 Broadcom SDK $
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
#include "sbZfSbQe2000ElibMVTConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibMVT_Print(sbZfSbQe2000ElibMVT_t *pFromStruct) {
  SB_LOG("SbQe2000ElibMVT:: reserved=0x%01x", (unsigned int)  pFromStruct->m_nReserved);
  SB_LOG(" port_map=0x%013llx", (uint64_t)  pFromStruct->m_nnPortMap);
  SB_LOG(" mvtd_a=0x%04x", (unsigned int)  pFromStruct->m_nMvtda);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibMVT:: mvtd_b=0x%01x", (unsigned int)  pFromStruct->m_nMvtdb);
  SB_LOG(" next=0x%04x", (unsigned int)  pFromStruct->m_nNext);
  SB_LOG(" knockout=0x%01x", (unsigned int)  pFromStruct->m_nKnockout);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibMVT_SPrint(sbZfSbQe2000ElibMVT_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibMVT:: reserved=0x%01x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_map=0x%013llx", (uint64_t)  pFromStruct->m_nnPortMap);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mvtd_a=0x%04x", (unsigned int)  pFromStruct->m_nMvtda);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibMVT:: mvtd_b=0x%01x", (unsigned int)  pFromStruct->m_nMvtdb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," next=0x%04x", (unsigned int)  pFromStruct->m_nNext);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," knockout=0x%01x", (unsigned int)  pFromStruct->m_nKnockout);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibMVT_Validate(sbZfSbQe2000ElibMVT_t *pZf) {

  if (pZf->m_nReserved > 0x7) return 0;
  if (pZf->m_nnPortMap > 0x3FFFFFFFFFFFFULL) return 0;
  if (pZf->m_nMvtda > 0x3fff) return 0;
  if (pZf->m_nMvtdb > 0xf) return 0;
  if (pZf->m_nNext > 0xffff) return 0;
  if (pZf->m_nKnockout > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibMVT_SetField(sbZfSbQe2000ElibMVT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nnportmap") == 0) {
    s->m_nnPortMap = value;
  } else if (SB_STRCMP(name, "m_nmvtda") == 0) {
    s->m_nMvtda = value;
  } else if (SB_STRCMP(name, "m_nmvtdb") == 0) {
    s->m_nMvtdb = value;
  } else if (SB_STRCMP(name, "m_nnext") == 0) {
    s->m_nNext = value;
  } else if (SB_STRCMP(name, "m_nknockout") == 0) {
    s->m_nKnockout = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
