/*
 * $Id: sbZfKaEgMemShapingEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaEgMemShapingEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEgMemShapingEntry_Print(sbZfKaEgMemShapingEntry_t *pFromStruct) {
  SB_LOG("KaEgMemShapingEntry:: reserved=0x%01x", (unsigned int)  pFromStruct->m_nReserved);
  SB_LOG(" bucketdepth=0x%06x", (unsigned int)  pFromStruct->m_nBucketDepth);
  SB_LOG(" shaperate=0x%06x", (unsigned int)  pFromStruct->m_nShapeRate);
  SB_LOG("\n");

  SB_LOG("KaEgMemShapingEntry:: maxdepth=0x%04x", (unsigned int)  pFromStruct->m_nMaxDepth);
  SB_LOG(" port=0x%02x", (unsigned int)  pFromStruct->m_nPort);
  SB_LOG(" hiside=0x%01x", (unsigned int)  pFromStruct->m_nHiSide);
  SB_LOG(" shapesrc=0x%02x", (unsigned int)  pFromStruct->m_nShapeSrc);
  SB_LOG("\n");

  SB_LOG("KaEgMemShapingEntry:: enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaEgMemShapingEntry_SPrint(sbZfKaEgMemShapingEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemShapingEntry:: reserved=0x%01x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bucketdepth=0x%06x", (unsigned int)  pFromStruct->m_nBucketDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shaperate=0x%06x", (unsigned int)  pFromStruct->m_nShapeRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemShapingEntry:: maxdepth=0x%04x", (unsigned int)  pFromStruct->m_nMaxDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port=0x%02x", (unsigned int)  pFromStruct->m_nPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hiside=0x%01x", (unsigned int)  pFromStruct->m_nHiSide);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shapesrc=0x%02x", (unsigned int)  pFromStruct->m_nShapeSrc);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemShapingEntry:: enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEgMemShapingEntry_Validate(sbZfKaEgMemShapingEntry_t *pZf) {

  if (pZf->m_nReserved > 0x7) return 0;
  if (pZf->m_nBucketDepth > 0xffffff) return 0;
  if (pZf->m_nShapeRate > 0xffffff) return 0;
  if (pZf->m_nMaxDepth > 0x7fff) return 0;
  if (pZf->m_nPort > 0x3f) return 0;
  if (pZf->m_nHiSide > 0x1) return 0;
  if (pZf->m_nShapeSrc > 0x3f) return 0;
  if (pZf->m_nEnable > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEgMemShapingEntry_SetField(sbZfKaEgMemShapingEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nbucketdepth") == 0) {
    s->m_nBucketDepth = value;
  } else if (SB_STRCMP(name, "m_nshaperate") == 0) {
    s->m_nShapeRate = value;
  } else if (SB_STRCMP(name, "m_nmaxdepth") == 0) {
    s->m_nMaxDepth = value;
  } else if (SB_STRCMP(name, "m_nport") == 0) {
    s->m_nPort = value;
  } else if (SB_STRCMP(name, "m_nhiside") == 0) {
    s->m_nHiSide = value;
  } else if (SB_STRCMP(name, "m_nshapesrc") == 0) {
    s->m_nShapeSrc = value;
  } else if (SB_STRCMP(name, "m_nenable") == 0) {
    s->m_nEnable = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
