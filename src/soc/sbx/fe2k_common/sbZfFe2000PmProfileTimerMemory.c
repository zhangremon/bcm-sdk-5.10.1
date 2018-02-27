/*
 * $Id: sbZfFe2000PmProfileTimerMemory.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfFe2000PmProfileTimerMemory.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000PmProfileTimerMemory_Pack(sbZfFe2000PmProfileTimerMemory_t *pFrom,
                                    uint8_t *pToData,
                                    uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on big endian */

  /* Pack Member: uResv0 */
  (pToData)[0] |= ((pFrom)->uResv0 & 0x01) <<7;

  /* Pack Member: uType */
  (pToData)[0] |= ((pFrom)->uType & 0x01) <<6;

  /* Pack Member: uProfMode */
  (pToData)[0] |= ((pFrom)->uProfMode & 0x03) <<4;

  /* Pack Member: uResv1 */
  (pToData)[3] |= ((pFrom)->uResv1) & 0xFF;
  (pToData)[2] |= ((pFrom)->uResv1 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->uResv1 >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->uResv1 >> 24) & 0x0f;

  /* Pack Member: uResv2 */
  (pToData)[4] |= ((pFrom)->uResv2) & 0xFF;

  /* Pack Member: bInterrupt */
  (pToData)[5] |= ((pFrom)->bInterrupt & 0x01) <<7;

  /* Pack Member: bReset */
  (pToData)[5] |= ((pFrom)->bReset & 0x01) <<6;

  /* Pack Member: bStrict */
  (pToData)[5] |= ((pFrom)->bStrict & 0x01) <<5;

  /* Pack Member: bStarted */
  (pToData)[5] |= ((pFrom)->bStarted & 0x01) <<4;

  /* Pack Member: uDeadline */
  (pToData)[7] |= ((pFrom)->uDeadline) & 0xFF;
  (pToData)[6] |= ((pFrom)->uDeadline >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->uDeadline >> 16) & 0x0f;

  return SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000PmProfileTimerMemory_Unpack(sbZfFe2000PmProfileTimerMemory_t *pToStruct,
                                      uint8_t *pFromData,
                                      uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on big endian */

  /* Unpack Member: uResv0 */
  (pToStruct)->uResv0 =  (uint32_t)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: uType */
  (pToStruct)->uType =  (uint32_t)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: uProfMode */
  (pToStruct)->uProfMode =  (uint32_t)  ((pFromData)[0] >> 4) & 0x03;

  /* Unpack Member: uResv1 */
  (pToStruct)->uResv1 =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->uResv1 |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->uResv1 |=  (uint32_t)  (pFromData)[1] << 16;
  (pToStruct)->uResv1 |=  (uint32_t)  ((pFromData)[0] & 0x0f) << 24;

  /* Unpack Member: uResv2 */
  (pToStruct)->uResv2 =  (uint32_t)  (pFromData)[4] ;

  /* Unpack Member: bInterrupt */
  (pToStruct)->bInterrupt =  (uint32_t)  ((pFromData)[5] >> 7) & 0x01;

  /* Unpack Member: bReset */
  (pToStruct)->bReset =  (uint32_t)  ((pFromData)[5] >> 6) & 0x01;

  /* Unpack Member: bStrict */
  (pToStruct)->bStrict =  (uint32_t)  ((pFromData)[5] >> 5) & 0x01;

  /* Unpack Member: bStarted */
  (pToStruct)->bStarted =  (uint32_t)  ((pFromData)[5] >> 4) & 0x01;

  /* Unpack Member: uDeadline */
  (pToStruct)->uDeadline =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->uDeadline |=  (uint32_t)  (pFromData)[6] << 8;
  (pToStruct)->uDeadline |=  (uint32_t)  ((pFromData)[5] & 0x0f) << 16;

}



/* initialize an instance of this zframe */
void
sbZfFe2000PmProfileTimerMemory_InitInstance(sbZfFe2000PmProfileTimerMemory_t *pFrame) {

  pFrame->uResv0 =  (unsigned int)  0;
  pFrame->uType =  (unsigned int)  0;
  pFrame->uProfMode =  (unsigned int)  0;
  pFrame->uResv1 =  (unsigned int)  0;
  pFrame->uResv2 =  (unsigned int)  0;
  pFrame->bInterrupt =  (unsigned int)  0;
  pFrame->bReset =  (unsigned int)  0;
  pFrame->bStrict =  (unsigned int)  0;
  pFrame->bStarted =  (unsigned int)  0;
  pFrame->uDeadline =  (unsigned int)  0;

}



uint8_t sbZfFe2000PmProfileTimerMemory_Compare( sbZfFe2000PmProfileTimerMemory_t *pProfile1,
                                              sbZfFe2000PmProfileTimerMemory_t *pProfile2)
{
  if(pProfile1->bInterrupt != pProfile2->bInterrupt) {
    return 0;
  } 
  if(pProfile1->bReset != pProfile2->bReset) {
    return 0;
  }
  if(pProfile1->bStrict != pProfile2->bStrict) {
    return 0;
  }
  if(pProfile1->bStarted != pProfile2->bStarted) {
    return 0;
  }
  if(pProfile1->uDeadline!= pProfile2->uDeadline) {
    return 0;
  }
  if(pProfile1->uType != pProfile2->uType) {
    return 0;
  }
  if(pProfile1->uProfMode != pProfile2->uProfMode) {
    return 0;
  }
  return 1;
}

void sbZfFe2000PmProfileTimerMemory_Copy( sbZfFe2000PmProfileTimerMemory_t *pSource,
                                   sbZfFe2000PmProfileTimerMemory_t *pDest)
{
  pDest->bInterrupt    = pSource->bInterrupt;
  pDest->bReset        = pSource->bReset;
  pDest->bStrict       = pSource->bStrict;
  pDest->bStarted      = pSource->bStarted;
  pDest->uDeadline     = pSource->uDeadline;
  pDest->uType           = pSource->uType;
  pDest->uProfMode       = pSource->uProfMode;
  return;
}

uint32_t sbZfFe2000PmProfileTimerMemory_Pack32(sbZfFe2000PmProfileTimerMemory_t *pFrom, 
                                              uint32_t *pToData, 
                                              uint32_t nMaxToDataIndex)
{
  uint8_t uBuffer[SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_WORDS*4];
  uint32_t uIndex =0;
  uint32_t *pBuffer = NULL;
  uint32_t uNumberOfWords = SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_WORDS;
  sbZfFe2000PmProfileTimerMemory_Pack(pFrom , &uBuffer[0], SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_BYTES);
  pBuffer = (uint32_t *) &uBuffer[0];
  
  for(uIndex=0; uIndex < uNumberOfWords; uIndex ++) {
    pToData[uIndex] = pBuffer[uIndex];
  }
  return uNumberOfWords;
}

uint32_t sbZfFe2000PmProfileTimerMemory_Unpack32(sbZfFe2000PmProfileTimerMemory_t *pToData,
                                               uint32_t *pFrom,
                                               uint32_t nMaxToDataIndex)
{

  uint32_t uBuffer[SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_WORDS*4];
  uint32_t uNumberOfWords = SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_WORDS;
  uint32_t uIndex =0;

  for(uIndex=0; uIndex < uNumberOfWords; uIndex ++) {
    uBuffer[uIndex] = pFrom[uIndex];
  }

  sbZfFe2000PmProfileTimerMemory_Unpack(pToData , (uint8_t *) &uBuffer[0], SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_BYTES);
  return uNumberOfWords;
}
