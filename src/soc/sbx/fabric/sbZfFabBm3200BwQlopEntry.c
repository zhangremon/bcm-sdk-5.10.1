/*
 * $Id: sbZfFabBm3200BwQlopEntry.c 1.2.28.4 Broadcom SDK $
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
#include "sbZfFabBm3200BwQlopEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm3200BwQlopEntry_Pack(sbZfFabBm3200BwQlopEntry_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_QLOP_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nAlpha */
  (pToData)[1] |= ((pFrom)->m_nAlpha & 0x01) <<7;
  (pToData)[0] |= ((pFrom)->m_nAlpha >> 1) & 0x03;

  /* Pack Member: m_nBeta */
  (pToData)[1] |= ((pFrom)->m_nBeta & 0x07) <<4;

  /* Pack Member: m_nEpsilon */
  (pToData)[2] |= ((pFrom)->m_nEpsilon & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_nEpsilon >> 6) & 0x0f;

  /* Pack Member: m_nRateDeltaMax */
  (pToData)[3] |= ((pFrom)->m_nRateDeltaMax) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nRateDeltaMax >> 8) & 0x03;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_QLOP_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nAlpha */
  (pToData)[2] |= ((pFrom)->m_nAlpha & 0x01) <<7;
  (pToData)[3] |= ((pFrom)->m_nAlpha >> 1) & 0x03;

  /* Pack Member: m_nBeta */
  (pToData)[2] |= ((pFrom)->m_nBeta & 0x07) <<4;

  /* Pack Member: m_nEpsilon */
  (pToData)[1] |= ((pFrom)->m_nEpsilon & 0x3f) <<2;
  (pToData)[2] |= ((pFrom)->m_nEpsilon >> 6) & 0x0f;

  /* Pack Member: m_nRateDeltaMax */
  (pToData)[0] |= ((pFrom)->m_nRateDeltaMax) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nRateDeltaMax >> 8) & 0x03;
#endif

  return SB_ZF_FAB_BM3200_QLOP_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwQlopEntry_Unpack(sbZfFabBm3200BwQlopEntry_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nAlpha */
  (pToStruct)->m_nAlpha =  (uint32_t)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nAlpha |=  (uint32_t)  ((pFromData)[0] & 0x03) << 1;

  /* Unpack Member: m_nBeta */
  (pToStruct)->m_nBeta =  (uint32_t)  ((pFromData)[1] >> 4) & 0x07;

  /* Unpack Member: m_nEpsilon */
  (pToStruct)->m_nEpsilon =  (uint32_t)  ((pFromData)[2] >> 2) & 0x3f;
  (pToStruct)->m_nEpsilon |=  (uint32_t)  ((pFromData)[1] & 0x0f) << 6;

  /* Unpack Member: m_nRateDeltaMax */
  (pToStruct)->m_nRateDeltaMax =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nRateDeltaMax |=  (uint32_t)  ((pFromData)[2] & 0x03) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nAlpha */
  (pToStruct)->m_nAlpha =  (uint32_t)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nAlpha |=  (uint32_t)  ((pFromData)[3] & 0x03) << 1;

  /* Unpack Member: m_nBeta */
  (pToStruct)->m_nBeta =  (uint32_t)  ((pFromData)[2] >> 4) & 0x07;

  /* Unpack Member: m_nEpsilon */
  (pToStruct)->m_nEpsilon =  (uint32_t)  ((pFromData)[1] >> 2) & 0x3f;
  (pToStruct)->m_nEpsilon |=  (uint32_t)  ((pFromData)[2] & 0x0f) << 6;

  /* Unpack Member: m_nRateDeltaMax */
  (pToStruct)->m_nRateDeltaMax =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nRateDeltaMax |=  (uint32_t)  ((pFromData)[1] & 0x03) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwQlopEntry_InitInstance(sbZfFabBm3200BwQlopEntry_t *pFrame) {

  pFrame->m_nAlpha =  (unsigned int)  0;
  pFrame->m_nBeta =  (unsigned int)  0;
  pFrame->m_nEpsilon =  (unsigned int)  0;
  pFrame->m_nRateDeltaMax =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwQlopEntry.c,v 1.2.28.4 2011/05/22 05:39:14 iakramov Exp $
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
#include "sbZfFabBm3200BwQlopEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwQlopEntry_Print(sbZfFabBm3200BwQlopEntry_t *pFromStruct) {
  SB_LOG("FabBm3200BwQlopEntry:: alpha=0x%01x", (unsigned int)  pFromStruct->m_nAlpha);
  SB_LOG(" beta=0x%01x", (unsigned int)  pFromStruct->m_nBeta);
  SB_LOG(" epsilon=0x%03x", (unsigned int)  pFromStruct->m_nEpsilon);
  SB_LOG(" ratedeltamax=0x%03x", (unsigned int)  pFromStruct->m_nRateDeltaMax);
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabBm3200BwQlopEntry_SPrint(sbZfFabBm3200BwQlopEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwQlopEntry:: alpha=0x%01x", (unsigned int)  pFromStruct->m_nAlpha);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," beta=0x%01x", (unsigned int)  pFromStruct->m_nBeta);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," epsilon=0x%03x", (unsigned int)  pFromStruct->m_nEpsilon);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ratedeltamax=0x%03x", (unsigned int)  pFromStruct->m_nRateDeltaMax);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwQlopEntry_Validate(sbZfFabBm3200BwQlopEntry_t *pZf) {

  if (pZf->m_nAlpha > 0x7) return 0;
  if (pZf->m_nBeta > 0x7) return 0;
  if (pZf->m_nEpsilon > 0x3ff) return 0;
  if (pZf->m_nRateDeltaMax > 0x3ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwQlopEntry_SetField(sbZfFabBm3200BwQlopEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nalpha") == 0) {
    s->m_nAlpha = value;
  } else if (SB_STRCMP(name, "m_nbeta") == 0) {
    s->m_nBeta = value;
  } else if (SB_STRCMP(name, "m_nepsilon") == 0) {
    s->m_nEpsilon = value;
  } else if (SB_STRCMP(name, "m_nratedeltamax") == 0) {
    s->m_nRateDeltaMax = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
