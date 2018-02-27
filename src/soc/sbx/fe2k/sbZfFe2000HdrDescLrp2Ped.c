/*
 * $Id: sbZfFe2000HdrDescLrp2Ped.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfFe2000HdrDescLrp2Ped.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000HdrDescLrp2Ped_Pack(sbZfFe2000HdrDescLrp2Ped_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE2000HDRDESCLRP2PED_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: ulContinueByte */
  (pToData)[3] |= ((pFrom)->ulContinueByte) & 0xFF;

  /* Pack Member: ulHdr0 */
  (pToData)[2] |= ((pFrom)->ulHdr0 & 0x0f) <<4;

  /* Pack Member: ulHdr1 */
  (pToData)[2] |= ((pFrom)->ulHdr1 & 0x0f);

  /* Pack Member: ulHdr2 */
  (pToData)[1] |= ((pFrom)->ulHdr2 & 0x0f) <<4;

  /* Pack Member: ulHdr3 */
  (pToData)[1] |= ((pFrom)->ulHdr3 & 0x0f);

  /* Pack Member: ulHdr4 */
  (pToData)[0] |= ((pFrom)->ulHdr4 & 0x0f) <<4;

  /* Pack Member: ulHdr5 */
  (pToData)[0] |= ((pFrom)->ulHdr5 & 0x0f);

  /* Pack Member: ulHdr6 */
  (pToData)[7] |= ((pFrom)->ulHdr6 & 0x0f) <<4;

  /* Pack Member: ulHdr7 */
  (pToData)[7] |= ((pFrom)->ulHdr7 & 0x0f);

  /* Pack Member: ulHdr8 */
  (pToData)[6] |= ((pFrom)->ulHdr8 & 0x0f) <<4;

  /* Pack Member: ulHdr9 */
  (pToData)[6] |= ((pFrom)->ulHdr9 & 0x0f);

  /* Pack Member: ulLocn0 */
  (pToData)[5] |= ((pFrom)->ulLocn0) & 0xFF;

  /* Pack Member: ulLocn1 */
  (pToData)[4] |= ((pFrom)->ulLocn1) & 0xFF;

  /* Pack Member: ulLocn2 */
  (pToData)[11] |= ((pFrom)->ulLocn2) & 0xFF;

  /* Pack Member: ulLocn3 */
  (pToData)[10] |= ((pFrom)->ulLocn3) & 0xFF;

  /* Pack Member: ulLocn4 */
  (pToData)[9] |= ((pFrom)->ulLocn4) & 0xFF;

  /* Pack Member: ulLocn5 */
  (pToData)[8] |= ((pFrom)->ulLocn5) & 0xFF;

  /* Pack Member: ulLocn6 */
  (pToData)[15] |= ((pFrom)->ulLocn6) & 0xFF;

  /* Pack Member: ulLocn7 */
  (pToData)[14] |= ((pFrom)->ulLocn7) & 0xFF;

  /* Pack Member: ulLocn8 */
  (pToData)[13] |= ((pFrom)->ulLocn8) & 0xFF;

  /* Pack Member: ulLocn9 */
  (pToData)[12] |= ((pFrom)->ulLocn9) & 0xFF;

  /* Pack Member: ulStr */
  (pToData)[19] |= ((pFrom)->ulStr & 0x07) <<5;

  /* Pack Member: ulBuffer */
  (pToData)[18] |= ((pFrom)->ulBuffer) & 0xFF;
  (pToData)[19] |= ((pFrom)->ulBuffer >> 8) & 0x1f;

  /* Pack Member: ulExceptIndex */
  (pToData)[17] |= ((pFrom)->ulExceptIndex) & 0xFF;

  /* Pack Member: ulSQueue */
  (pToData)[16] |= ((pFrom)->ulSQueue) & 0xFF;

  /* Pack Member: ulTc */
  (pToData)[23] |= ((pFrom)->ulTc & 0x07) <<5;

  /* Pack Member: ulHc */
  (pToData)[23] |= ((pFrom)->ulHc & 0x01) <<4;

  /* Pack Member: ulMirrorIndex */
  (pToData)[23] |= ((pFrom)->ulMirrorIndex & 0x0f);

  /* Pack Member: ulCopyCnt */
  (pToData)[21] |= ((pFrom)->ulCopyCnt & 0x0f) <<4;
  (pToData)[22] |= ((pFrom)->ulCopyCnt >> 4) &0xFF;

  /* Pack Member: ulCM */
  (pToData)[21] |= ((pFrom)->ulCM & 0x03) <<2;

  /* Pack Member: ulD */
  (pToData)[21] |= ((pFrom)->ulD & 0x01) <<1;

  /* Pack Member: ulEnq */
  (pToData)[21] |= ((pFrom)->ulEnq & 0x01);

  /* Pack Member: ulDQueue */
  (pToData)[20] |= ((pFrom)->ulDQueue) & 0xFF;

  /* Pack Member: ulXferLen */
  (pToData)[26] |= ((pFrom)->ulXferLen & 0x3f) <<2;
  (pToData)[27] |= ((pFrom)->ulXferLen >> 6) &0xFF;

  /* Pack Member: ulPm */
  (pToData)[26] |= ((pFrom)->ulPm & 0x03);

  /* Pack Member: ulPadding0 */
  (pToData)[25] |= ((pFrom)->ulPadding0 & 0x03) <<6;

  /* Pack Member: ulFrameLen */
  (pToData)[24] |= ((pFrom)->ulFrameLen) & 0xFF;
  (pToData)[25] |= ((pFrom)->ulFrameLen >> 8) & 0x3f;

  /* Pack Member: ulPadding1 */
  (pToData)[28] |= ((pFrom)->ulPadding1) & 0xFF;
  (pToData)[29] |= ((pFrom)->ulPadding1 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->ulPadding1 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->ulPadding1 >> 24) &0xFF;

  /* Pack Member: ulPadding2 */
  (pToData)[32] |= ((pFrom)->ulPadding2) & 0xFF;
  (pToData)[33] |= ((pFrom)->ulPadding2 >> 8) &0xFF;
  (pToData)[34] |= ((pFrom)->ulPadding2 >> 16) &0xFF;
  (pToData)[35] |= ((pFrom)->ulPadding2 >> 24) &0xFF;

  return SB_ZF_FE2000HDRDESCLRP2PED_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000HdrDescLrp2Ped_Unpack(sbZfFe2000HdrDescLrp2Ped_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: ulContinueByte */
  (pToStruct)->ulContinueByte =  (uint32_t)  (pFromData)[3] ;

  /* Unpack Member: ulHdr0 */
  (pToStruct)->ulHdr0 =  (uint32_t)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: ulHdr1 */
  (pToStruct)->ulHdr1 =  (uint32_t)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: ulHdr2 */
  (pToStruct)->ulHdr2 =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: ulHdr3 */
  (pToStruct)->ulHdr3 =  (uint32_t)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: ulHdr4 */
  (pToStruct)->ulHdr4 =  (uint32_t)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: ulHdr5 */
  (pToStruct)->ulHdr5 =  (uint32_t)  ((pFromData)[0] ) & 0x0f;

  /* Unpack Member: ulHdr6 */
  (pToStruct)->ulHdr6 =  (uint32_t)  ((pFromData)[7] >> 4) & 0x0f;

  /* Unpack Member: ulHdr7 */
  (pToStruct)->ulHdr7 =  (uint32_t)  ((pFromData)[7] ) & 0x0f;

  /* Unpack Member: ulHdr8 */
  (pToStruct)->ulHdr8 =  (uint32_t)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: ulHdr9 */
  (pToStruct)->ulHdr9 =  (uint32_t)  ((pFromData)[6] ) & 0x0f;

  /* Unpack Member: ulLocn0 */
  (pToStruct)->ulLocn0 =  (uint32_t)  (pFromData)[5] ;

  /* Unpack Member: ulLocn1 */
  (pToStruct)->ulLocn1 =  (uint32_t)  (pFromData)[4] ;

  /* Unpack Member: ulLocn2 */
  (pToStruct)->ulLocn2 =  (uint32_t)  (pFromData)[11] ;

  /* Unpack Member: ulLocn3 */
  (pToStruct)->ulLocn3 =  (uint32_t)  (pFromData)[10] ;

  /* Unpack Member: ulLocn4 */
  (pToStruct)->ulLocn4 =  (uint32_t)  (pFromData)[9] ;

  /* Unpack Member: ulLocn5 */
  (pToStruct)->ulLocn5 =  (uint32_t)  (pFromData)[8] ;

  /* Unpack Member: ulLocn6 */
  (pToStruct)->ulLocn6 =  (uint32_t)  (pFromData)[15] ;

  /* Unpack Member: ulLocn7 */
  (pToStruct)->ulLocn7 =  (uint32_t)  (pFromData)[14] ;

  /* Unpack Member: ulLocn8 */
  (pToStruct)->ulLocn8 =  (uint32_t)  (pFromData)[13] ;

  /* Unpack Member: ulLocn9 */
  (pToStruct)->ulLocn9 =  (uint32_t)  (pFromData)[12] ;

  /* Unpack Member: ulStr */
  (pToStruct)->ulStr =  (uint32_t)  ((pFromData)[19] >> 5) & 0x07;

  /* Unpack Member: ulBuffer */
  (pToStruct)->ulBuffer =  (uint32_t)  (pFromData)[18] ;
  (pToStruct)->ulBuffer |=  (uint32_t)  ((pFromData)[19] & 0x1f) << 8;

  /* Unpack Member: ulExceptIndex */
  (pToStruct)->ulExceptIndex =  (uint32_t)  (pFromData)[17] ;

  /* Unpack Member: ulSQueue */
  (pToStruct)->ulSQueue =  (uint32_t)  (pFromData)[16] ;

  /* Unpack Member: ulTc */
  (pToStruct)->ulTc =  (uint32_t)  ((pFromData)[23] >> 5) & 0x07;

  /* Unpack Member: ulHc */
  (pToStruct)->ulHc =  (uint32_t)  ((pFromData)[23] >> 4) & 0x01;

  /* Unpack Member: ulMirrorIndex */
  (pToStruct)->ulMirrorIndex =  (uint32_t)  ((pFromData)[23] ) & 0x0f;

  /* Unpack Member: ulCopyCnt */
  (pToStruct)->ulCopyCnt =  (uint32_t)  ((pFromData)[21] >> 4) & 0x0f;
  (pToStruct)->ulCopyCnt |=  (uint32_t)  (pFromData)[22] << 4;

  /* Unpack Member: ulCM */
  (pToStruct)->ulCM =  (uint32_t)  ((pFromData)[21] >> 2) & 0x03;

  /* Unpack Member: ulD */
  (pToStruct)->ulD =  (uint32_t)  ((pFromData)[21] >> 1) & 0x01;

  /* Unpack Member: ulEnq */
  (pToStruct)->ulEnq =  (uint32_t)  ((pFromData)[21] ) & 0x01;

  /* Unpack Member: ulDQueue */
  (pToStruct)->ulDQueue =  (uint32_t)  (pFromData)[20] ;

  /* Unpack Member: ulXferLen */
  (pToStruct)->ulXferLen =  (uint32_t)  ((pFromData)[26] >> 2) & 0x3f;
  (pToStruct)->ulXferLen |=  (uint32_t)  (pFromData)[27] << 6;

  /* Unpack Member: ulPm */
  (pToStruct)->ulPm =  (uint32_t)  ((pFromData)[26] ) & 0x03;

  /* Unpack Member: ulPadding0 */
  (pToStruct)->ulPadding0 =  (uint32_t)  ((pFromData)[25] >> 6) & 0x03;

  /* Unpack Member: ulFrameLen */
  (pToStruct)->ulFrameLen =  (uint32_t)  (pFromData)[24] ;
  (pToStruct)->ulFrameLen |=  (uint32_t)  ((pFromData)[25] & 0x3f) << 8;

  /* Unpack Member: ulPadding1 */
  (pToStruct)->ulPadding1 =  (uint32_t)  (pFromData)[28] ;
  (pToStruct)->ulPadding1 |=  (uint32_t)  (pFromData)[29] << 8;
  (pToStruct)->ulPadding1 |=  (uint32_t)  (pFromData)[30] << 16;
  (pToStruct)->ulPadding1 |=  (uint32_t)  (pFromData)[31] << 24;

  /* Unpack Member: ulPadding2 */
  (pToStruct)->ulPadding2 =  (uint32_t)  (pFromData)[32] ;
  (pToStruct)->ulPadding2 |=  (uint32_t)  (pFromData)[33] << 8;
  (pToStruct)->ulPadding2 |=  (uint32_t)  (pFromData)[34] << 16;
  (pToStruct)->ulPadding2 |=  (uint32_t)  (pFromData)[35] << 24;

}



/* initialize an instance of this zframe */
void
sbZfFe2000HdrDescLrp2Ped_InitInstance(sbZfFe2000HdrDescLrp2Ped_t *pFrame) {

  pFrame->ulContinueByte =  (unsigned int)  0;
  pFrame->ulHdr0 =  (unsigned int)  0;
  pFrame->ulHdr1 =  (unsigned int)  0;
  pFrame->ulHdr2 =  (unsigned int)  0;
  pFrame->ulHdr3 =  (unsigned int)  0;
  pFrame->ulHdr4 =  (unsigned int)  0;
  pFrame->ulHdr5 =  (unsigned int)  0;
  pFrame->ulHdr6 =  (unsigned int)  0;
  pFrame->ulHdr7 =  (unsigned int)  0;
  pFrame->ulHdr8 =  (unsigned int)  0;
  pFrame->ulHdr9 =  (unsigned int)  0;
  pFrame->ulLocn0 =  (unsigned int)  0;
  pFrame->ulLocn1 =  (unsigned int)  0;
  pFrame->ulLocn2 =  (unsigned int)  0;
  pFrame->ulLocn3 =  (unsigned int)  0;
  pFrame->ulLocn4 =  (unsigned int)  0;
  pFrame->ulLocn5 =  (unsigned int)  0;
  pFrame->ulLocn6 =  (unsigned int)  0;
  pFrame->ulLocn7 =  (unsigned int)  0;
  pFrame->ulLocn8 =  (unsigned int)  0;
  pFrame->ulLocn9 =  (unsigned int)  0;
  pFrame->ulStr =  (unsigned int)  0;
  pFrame->ulBuffer =  (unsigned int)  0;
  pFrame->ulExceptIndex =  (unsigned int)  0;
  pFrame->ulSQueue =  (unsigned int)  0;
  pFrame->ulTc =  (unsigned int)  0;
  pFrame->ulHc =  (unsigned int)  0;
  pFrame->ulMirrorIndex =  (unsigned int)  0;
  pFrame->ulCopyCnt =  (unsigned int)  0;
  pFrame->ulCM =  (unsigned int)  0;
  pFrame->ulD =  (unsigned int)  0;
  pFrame->ulEnq =  (unsigned int)  0;
  pFrame->ulDQueue =  (unsigned int)  0;
  pFrame->ulXferLen =  (unsigned int)  0;
  pFrame->ulPm =  (unsigned int)  0;
  pFrame->ulPadding0 =  (unsigned int)  0;
  pFrame->ulFrameLen =  (unsigned int)  0;
  pFrame->ulPadding1 =  (unsigned int)  0;
  pFrame->ulPadding2 =  (unsigned int)  0;

}
