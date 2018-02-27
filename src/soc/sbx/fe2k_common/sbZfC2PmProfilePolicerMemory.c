/*
 * $Id: sbZfC2PmProfilePolicerMemory.c 1.1.32.4 Broadcom SDK $
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
#include "sbZfC2PmProfilePolicerMemory.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2PmProfilePolicerMemory_Pack(sbZfC2PmProfilePolicerMemory_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on big endian */

  /* Pack Member: uType */
  (pToData)[2] |= ((pFrom)->uType & 0x01) <<2;

  /* Pack Member: uLengthShift */
  (pToData)[3] |= ((pFrom)->uLengthShift & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->uLengthShift >> 1) & 0x03;

  /* Pack Member: bCouplingFlag */
  (pToData)[3] |= ((pFrom)->bCouplingFlag & 0x01) <<6;

  /* Pack Member: bRFC2698Mode */
  (pToData)[3] |= ((pFrom)->bRFC2698Mode & 0x01) <<5;

  /* Pack Member: uBktCSize */
  (pToData)[3] |= ((pFrom)->uBktCSize & 0x1f);

  /* Pack Member: bBlind */
  (pToData)[4] |= ((pFrom)->bBlind & 0x01) <<7;

  /* Pack Member: bBktENoDecrement */
  (pToData)[4] |= ((pFrom)->bBktENoDecrement & 0x01) <<6;

  /* Pack Member: uEIR */
  (pToData)[6] |= ((pFrom)->uEIR & 0x0f) <<4;
  (pToData)[5] |= ((pFrom)->uEIR >> 4) &0xFF;
  (pToData)[4] |= ((pFrom)->uEIR >> 12) & 0x3f;

  /* Pack Member: uEBS */
  (pToData)[7] |= ((pFrom)->uEBS) & 0xFF;
  (pToData)[6] |= ((pFrom)->uEBS >> 8) & 0x0f;

  /* Pack Member: bDropOnRed */
  (pToData)[8] |= ((pFrom)->bDropOnRed & 0x01) <<7;

  /* Pack Member: bBktCNoDecrement */
  (pToData)[8] |= ((pFrom)->bBktCNoDecrement & 0x01) <<6;

  /* Pack Member: uCIR */
  (pToData)[10] |= ((pFrom)->uCIR & 0x0f) <<4;
  (pToData)[9] |= ((pFrom)->uCIR >> 4) &0xFF;
  (pToData)[8] |= ((pFrom)->uCIR >> 12) & 0x3f;

  /* Pack Member: uCBS */
  (pToData)[11] |= ((pFrom)->uCBS) & 0xFF;
  (pToData)[10] |= ((pFrom)->uCBS >> 8) & 0x0f;

  return SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2PmProfilePolicerMemory_Unpack(sbZfC2PmProfilePolicerMemory_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on big endian */

  /* Unpack Member: uType */
  (pToStruct)->uType =  (uint32_t)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: uLengthShift */
  (pToStruct)->uLengthShift =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;
  (pToStruct)->uLengthShift |=  (uint32_t)  ((pFromData)[2] & 0x03) << 1;

  /* Unpack Member: bCouplingFlag */
  (pToStruct)->bCouplingFlag =  (uint32_t)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: bRFC2698Mode */
  (pToStruct)->bRFC2698Mode =  (uint32_t)  ((pFromData)[3] >> 5) & 0x01;

  /* Unpack Member: uBktCSize */
  (pToStruct)->uBktCSize =  (uint32_t)  ((pFromData)[3] ) & 0x1f;

  /* Unpack Member: bBlind */
  (pToStruct)->bBlind =  (uint32_t)  ((pFromData)[4] >> 7) & 0x01;

  /* Unpack Member: bBktENoDecrement */
  (pToStruct)->bBktENoDecrement =  (uint32_t)  ((pFromData)[4] >> 6) & 0x01;

  /* Unpack Member: uEIR */
  (pToStruct)->uEIR =  (uint32_t)  ((pFromData)[6] >> 4) & 0x0f;
  (pToStruct)->uEIR |=  (uint32_t)  (pFromData)[5] << 4;
  (pToStruct)->uEIR |=  (uint32_t)  ((pFromData)[4] & 0x3f) << 12;

  /* Unpack Member: uEBS */
  (pToStruct)->uEBS =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->uEBS |=  (uint32_t)  ((pFromData)[6] & 0x0f) << 8;

  /* Unpack Member: bDropOnRed */
  (pToStruct)->bDropOnRed =  (uint32_t)  ((pFromData)[8] >> 7) & 0x01;

  /* Unpack Member: bBktCNoDecrement */
  (pToStruct)->bBktCNoDecrement =  (uint32_t)  ((pFromData)[8] >> 6) & 0x01;

  /* Unpack Member: uCIR */
  (pToStruct)->uCIR =  (uint32_t)  ((pFromData)[10] >> 4) & 0x0f;
  (pToStruct)->uCIR |=  (uint32_t)  (pFromData)[9] << 4;
  (pToStruct)->uCIR |=  (uint32_t)  ((pFromData)[8] & 0x3f) << 12;

  /* Unpack Member: uCBS */
  (pToStruct)->uCBS =  (uint32_t)  (pFromData)[11] ;
  (pToStruct)->uCBS |=  (uint32_t)  ((pFromData)[10] & 0x0f) << 8;

}



/* initialize an instance of this zframe */
void
sbZfC2PmProfilePolicerMemory_InitInstance(sbZfC2PmProfilePolicerMemory_t *pFrame) {

  pFrame->uType =  (unsigned int)  0;
  pFrame->uLengthShift =  (unsigned int)  0;
  pFrame->bCouplingFlag =  (unsigned int)  0;
  pFrame->bRFC2698Mode =  (unsigned int)  0;
  pFrame->uBktCSize =  (unsigned int)  0;
  pFrame->bBlind =  (unsigned int)  0;
  pFrame->bBktENoDecrement =  (unsigned int)  0;
  pFrame->uEIR =  (unsigned int)  0;
  pFrame->uEBS =  (unsigned int)  0;
  pFrame->bDropOnRed =  (unsigned int)  0;
  pFrame->bBktCNoDecrement =  (unsigned int)  0;
  pFrame->uCIR =  (unsigned int)  0;
  pFrame->uCBS =  (unsigned int)  0;

}



uint8_t sbZfC2PmProfilePolicerMemory_Compare( sbZfC2PmProfilePolicerMemory_t *pProfile1,
                                         sbZfC2PmProfilePolicerMemory_t *pProfile2)
{
  if(pProfile1->bCouplingFlag != pProfile2->bCouplingFlag) {
    return 0;
  } 
  if(pProfile1->bRFC2698Mode != pProfile2->bRFC2698Mode) {
    return 0;
  }
  if(pProfile1->bBlind != pProfile2->bBlind) {
    return 0;
  }
  if(pProfile1->uBktCSize != pProfile2->uBktCSize) {
    return 0;
  }
  if(pProfile1->uEBS != pProfile2->uEBS) {
    return 0;
  }
  if(pProfile1->uCBS != pProfile2->uCBS) {
    return 0;
  }
  if(pProfile1->bDropOnRed != pProfile2->bDropOnRed) {
    return 0;
  }
  if(pProfile1->bBktENoDecrement != pProfile2->bBktENoDecrement) {
    return 0;
  }
  if(pProfile1->bBktCNoDecrement != pProfile2->bBktCNoDecrement) {
    return 0;
  }
  if(pProfile1->uLengthShift != pProfile2->uLengthShift) {
    return 0;
  }
  if(pProfile1->uEIR != pProfile2->uEIR) {
    return 0;
  }
  if(pProfile1->uCIR != pProfile2->uCIR) {
    return 0;
  }
  return 1;
}

void sbZfC2PmProfilePolicerMemory_Copy( sbZfC2PmProfilePolicerMemory_t *pSource,
                                          sbZfC2PmProfilePolicerMemory_t *pDest)
{
  pDest->bCouplingFlag = pSource->bCouplingFlag;
  pDest->bRFC2698Mode  = pSource->bRFC2698Mode;
  pDest->bBlind        = pSource->bBlind;
  pDest->uEBS          = pSource->uEBS;
  pDest->uCBS          = pSource->uCBS;
  pDest->uCIR          = pSource->uCIR;
  pDest->uEIR          = pSource->uEIR;
  pDest->uLengthShift    = pSource->uLengthShift;
  pDest->uBktCSize     = pSource->uBktCSize;
  pDest->uType           = pSource->uType;
  pDest->bDropOnRed      = pSource->bDropOnRed;
  pDest->bBktENoDecrement      = pSource->bBktENoDecrement;
  pDest->bBktCNoDecrement      = pSource->bBktCNoDecrement;
  return;
}

uint32_t sbZfC2PmProfilePolicerMemory_Pack32( sbZfC2PmProfilePolicerMemory_t *pFrom, 
                                                uint32_t *pToData, 
                                                uint32_t nMaxToDataIndex)
{
  uint8_t uBuffer[SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_WORDS*4];
  uint32_t uIndex =0;
  uint32_t *pBuffer = NULL;
  uint32_t uNumberOfWords = SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_WORDS;
  sbZfC2PmProfilePolicerMemory_Pack(pFrom , &uBuffer[0], SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES);
  pBuffer = (uint32_t *) &uBuffer[0];
  
  for(uIndex=0; uIndex < uNumberOfWords; uIndex ++) {
    pToData[uIndex] = pBuffer[uIndex];
  }
  return uNumberOfWords;
}

uint32_t sbZfC2PmProfilePolicerMemory_Unpack32(sbZfC2PmProfilePolicerMemory_t *pToData,
                                                 uint32_t *pFrom,
                                                 uint32_t nMaxToDataIndex)
{

  uint32_t uBuffer[SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_WORDS*4];
  uint32_t uNumberOfWords = SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_WORDS;
  uint32_t uIndex =0;

  for(uIndex=0; uIndex < uNumberOfWords; uIndex ++) {
    uBuffer[uIndex] = pFrom[uIndex];
  }

  sbZfC2PmProfilePolicerMemory_Unpack(pToData , (uint8_t *) &uBuffer[0], SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES);
  return uNumberOfWords;
}
