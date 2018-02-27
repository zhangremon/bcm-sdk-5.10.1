/*
 * $Id: sbZfFe2000RcSbPattern.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfFe2000RcSbPattern.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000RcSbPattern_Pack(sbZfFe2000RcSbPattern_t *pFrom,
                           uint8_t *pToData,
                           uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE2000RCSBPATTERN_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: uPattern3 */
  (pToData)[12] |= ((pFrom)->uPattern3) & 0xFF;
  (pToData)[13] |= ((pFrom)->uPattern3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->uPattern3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->uPattern3 >> 24) &0xFF;

  /* Pack Member: uPattern2 */
  (pToData)[8] |= ((pFrom)->uPattern2) & 0xFF;
  (pToData)[9] |= ((pFrom)->uPattern2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->uPattern2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->uPattern2 >> 24) &0xFF;

  /* Pack Member: uPattern1 */
  (pToData)[4] |= ((pFrom)->uPattern1) & 0xFF;
  (pToData)[5] |= ((pFrom)->uPattern1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->uPattern1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->uPattern1 >> 24) &0xFF;

  /* Pack Member: uPattern0 */
  (pToData)[0] |= ((pFrom)->uPattern0) & 0xFF;
  (pToData)[1] |= ((pFrom)->uPattern0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->uPattern0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->uPattern0 >> 24) &0xFF;

  return SB_ZF_FE2000RCSBPATTERN_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000RcSbPattern_Unpack(sbZfFe2000RcSbPattern_t *pToStruct,
                             uint8_t *pFromData,
                             uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: uPattern3 */
  (pToStruct)->uPattern3 =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->uPattern3 |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->uPattern3 |=  (uint32_t)  (pFromData)[14] << 16;
  (pToStruct)->uPattern3 |=  (uint32_t)  (pFromData)[15] << 24;

  /* Unpack Member: uPattern2 */
  (pToStruct)->uPattern2 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->uPattern2 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->uPattern2 |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->uPattern2 |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: uPattern1 */
  (pToStruct)->uPattern1 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->uPattern1 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->uPattern1 |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->uPattern1 |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: uPattern0 */
  (pToStruct)->uPattern0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->uPattern0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->uPattern0 |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->uPattern0 |=  (uint32_t)  (pFromData)[3] << 24;

}



/* initialize an instance of this zframe */
void
sbZfFe2000RcSbPattern_InitInstance(sbZfFe2000RcSbPattern_t *pFrame) {

  pFrame->uPattern3 =  (unsigned int)  0;
  pFrame->uPattern2 =  (unsigned int)  0;
  pFrame->uPattern1 =  (unsigned int)  0;
  pFrame->uPattern0 =  (unsigned int)  0;

}



void sbZfFe2000RcSbPatternAlias( sbZfFe2000RcSbPattern_t *pFrame, 
                                 sbZfFe2000RcSbPatternAlias_t *pTo)
{
  pTo->pPattern[0] = &pFrame->uPattern0;
  pTo->pPattern[1] = &pFrame->uPattern1;
  pTo->pPattern[2] = &pFrame->uPattern2;
  pTo->pPattern[3] = &pFrame->uPattern3;
}

void sbZfFe2000RcSbSetRuleBit( sbZfFe2000RcSbPattern_t *pFrame, 
                               uint32_t uRuleId, 
                               uint32_t uValue)
{
  uint32_t uPatternBlock = uRuleId/32;
  uint32_t uBitId        = uRuleId%32;
  uint32_t *pPointer = NULL;

  switch(uPatternBlock) {
  case 0:
    pPointer = &pFrame->uPattern0;
    break;
  case 1:
    pPointer = &pFrame->uPattern1;
    break;
  case 2:
    pPointer = &pFrame->uPattern2;
    break;
  case 3:
    pPointer = &pFrame->uPattern3;
    break;
  default:
    SB_ASSERT(0);
    return;
  }

  if(uValue) {
    *pPointer |= (1 << uBitId);
  } else {
    *pPointer &= ~(1 << uBitId);
  }
}
uint32_t sbZfFe2000RcSbPatternIsNull( sbZfFe2000RcSbPattern_t *pFrameBase,
                                      uint32_t uRuleId,
                                      uint32_t uInstCount)
{
  sbZfFe2000RcSbPattern_t *pFrame;
  uint32_t i;
  uint32_t uPatternBlock = uRuleId/32;
  uint32_t uBitId        = uRuleId%32;
  uint32_t *pPointer = NULL;

  for (i=0; i<uInstCount; i++){
      pFrame = &pFrameBase[i];
      switch(uPatternBlock) {
      case 0:
        pPointer = &pFrame->uPattern0;
        break;
      case 1:
        pPointer = &pFrame->uPattern1;
        break;
      case 2:
        pPointer = &pFrame->uPattern2;
        break;
      case 3:
        pPointer = &pFrame->uPattern3;
        break;
      default:
        SB_ASSERT(0);
        return -1;
      }
      if (*pPointer & (1 << uBitId)) {
          return 0;
      }
  }

  return 1;

}
                                        
void sbZfFe2000RcSbGetRuleBit( sbZfFe2000RcSbPattern_t *pFrame, 
                               uint32_t uRuleId, 
                               uint8_t *uValue)
{
  uint32_t uPatternBlock = uRuleId/32;
  uint32_t uBitId        = uRuleId%32;
  uint32_t *pPointer = NULL;

  switch(uPatternBlock) {
  case 0:
    pPointer = &pFrame->uPattern0;
    break;
  case 1:
    pPointer = &pFrame->uPattern1;
    break;
  case 2:
    pPointer = &pFrame->uPattern2;
    break;
  case 3:
    pPointer = &pFrame->uPattern3;
    break;
  default:
    SB_ASSERT(0);
    return;
  }

  if (*pPointer & (1 << uBitId)) {
      *uValue = 1;
  }else{
      *uValue = 0;
  }
}
