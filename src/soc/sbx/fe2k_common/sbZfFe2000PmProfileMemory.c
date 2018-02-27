/*
 * $Id: sbZfFe2000PmProfileMemory.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfFe2000PmProfileMemory.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000PmProfileMemory_Pack(sbZfFe2000PmProfileMemory_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on big endian */

  /* Pack Member: uResv0 */
  (pToData)[0] |= ((pFrom)->uResv0 & 0x01) <<7;

  /* Pack Member: uProfType */
  (pToData)[0] |= ((pFrom)->uProfType & 0x01) <<6;

  /* Pack Member: uOamProfType */
  (pToData)[0] |= ((pFrom)->uOamProfType & 0x03) <<4;

  /* Pack Member: uResv1 */
  (pToData)[3] |= ((pFrom)->uResv1) & 0xFF;
  (pToData)[2] |= ((pFrom)->uResv1 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->uResv1 >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->uResv1 >> 24) & 0x0f;

  /* Pack Member: uResv2 */
  (pToData)[7] |= ((pFrom)->uResv2) & 0xFF;
  (pToData)[6] |= ((pFrom)->uResv2 >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->uResv2 >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->uResv2 >> 24) &0xFF;

  return SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000PmProfileMemory_Unpack(sbZfFe2000PmProfileMemory_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on big endian */

  /* Unpack Member: uResv0 */
  (pToStruct)->uResv0 =  (uint32_t)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: uProfType */
  (pToStruct)->uProfType =  (uint32_t)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: uOamProfType */
  (pToStruct)->uOamProfType =  (uint32_t)  ((pFromData)[0] >> 4) & 0x03;

  /* Unpack Member: uResv1 */
  (pToStruct)->uResv1 =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->uResv1 |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->uResv1 |=  (uint32_t)  (pFromData)[1] << 16;
  (pToStruct)->uResv1 |=  (uint32_t)  ((pFromData)[0] & 0x0f) << 24;

  /* Unpack Member: uResv2 */
  (pToStruct)->uResv2 =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->uResv2 |=  (uint32_t)  (pFromData)[6] << 8;
  (pToStruct)->uResv2 |=  (uint32_t)  (pFromData)[5] << 16;
  (pToStruct)->uResv2 |=  (uint32_t)  (pFromData)[4] << 24;

}



/* initialize an instance of this zframe */
void
sbZfFe2000PmProfileMemory_InitInstance(sbZfFe2000PmProfileMemory_t *pFrame) {

  pFrame->uResv0 =  (unsigned int)  0;
  pFrame->uProfType =  (unsigned int)  0;
  pFrame->uOamProfType =  (unsigned int)  0;
  pFrame->uResv1 =  (unsigned int)  0;
  pFrame->uResv2 =  (unsigned int)  0;

}



void sbZfFe2000PmProfileMemory_Copy( sbZfFe2000PmProfileMemory_t *pSource,
                                   sbZfFe2000PmProfileMemory_t *pDest)
{
  pDest->uResv0         = pSource->uResv0;
  pDest->uResv1         = pSource->uResv1;
  pDest->uProfType      = pSource->uProfType;
  pDest->uOamProfType   = pSource->uOamProfType;
  pDest->uResv2         = pSource->uResv2;

  return;
}

uint32_t sbZfFe2000PmProfileMemory_Pack32(sbZfFe2000PmProfileMemory_t *pFrom, 
                                    uint32_t *pToData, 
                                    uint32_t nMaxToDataIndex)
{
  uint8_t uBuffer[SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS*4];
  uint32_t uIndex =0;
  uint32_t *pBuffer = NULL;
  uint32_t uNumberOfWords = SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS;
  sbZfFe2000PmProfileMemory_Pack(pFrom , &uBuffer[0], SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_BYTES);
  pBuffer = (uint32_t *) &uBuffer[0];
  
  for(uIndex=0; uIndex < uNumberOfWords; uIndex ++) {
    pToData[uIndex] = pBuffer[uNumberOfWords-uIndex-1];
  }
  return uNumberOfWords;
}

uint32_t sbZfFe2000PmProfileMemory_Unpack32(sbZfFe2000PmProfileMemory_t *pToData,
                                          uint32_t *pFrom,
                                          uint32_t nMaxToDataIndex)
{

  uint32_t uBuffer[SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS*4];
  uint32_t uNumberOfWords = SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS;
  uint32_t uIndex =0;

  for(uIndex=0; uIndex < uNumberOfWords; uIndex ++) {
    uBuffer[uNumberOfWords-uIndex-1] = pFrom[uIndex];
  }

  sbZfFe2000PmProfileMemory_Unpack(pToData , (uint8_t *) &uBuffer[0], SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_BYTES);
  return uNumberOfWords;
}
