/*
 * $Id: sbZfKaQmFbLine.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaQmFbLine.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaQmFbLine_Pack(sbZfKaQmFbLine_t *pFrom,
                    uint8_t *pToData,
                    uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMFBLINE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nHec1 */
  (pToData)[12] |= ((pFrom)->m_nHec1) & 0xFF;

  /* Pack Member: m_nHec0 */
  (pToData)[13] |= ((pFrom)->m_nHec0) & 0xFF;

  /* Pack Member: m_nSpare */
  (pToData)[15] |= ((pFrom)->m_nSpare & 0x03) <<6;
  (pToData)[14] |= ((pFrom)->m_nSpare >> 2) &0xFF;

  /* Pack Member: m_nPbExtAddr5 */
  (pToData)[9] |= ((pFrom)->m_nPbExtAddr5 & 0x07) <<5;
  (pToData)[8] |= ((pFrom)->m_nPbExtAddr5 >> 3) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nPbExtAddr5 >> 11) & 0x3f;

  /* Pack Member: m_nPbExtAddr4 */
  (pToData)[11] |= ((pFrom)->m_nPbExtAddr4 & 0x0f) <<4;
  (pToData)[10] |= ((pFrom)->m_nPbExtAddr4 >> 4) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nPbExtAddr4 >> 12) & 0x1f;

  /* Pack Member: m_nPbExtAddr3 */
  (pToData)[5] |= ((pFrom)->m_nPbExtAddr3 & 0x1f) <<3;
  (pToData)[4] |= ((pFrom)->m_nPbExtAddr3 >> 5) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nPbExtAddr3 >> 13) & 0x0f;

  /* Pack Member: m_nPbExtAddr2 */
  (pToData)[7] |= ((pFrom)->m_nPbExtAddr2 & 0x3f) <<2;
  (pToData)[6] |= ((pFrom)->m_nPbExtAddr2 >> 6) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nPbExtAddr2 >> 14) & 0x07;

  /* Pack Member: m_nPbExtAddr1 */
  (pToData)[1] |= ((pFrom)->m_nPbExtAddr1 & 0x7f) <<1;
  (pToData)[0] |= ((pFrom)->m_nPbExtAddr1 >> 7) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nPbExtAddr1 >> 15) & 0x03;

  /* Pack Member: m_nPbExtAddr0 */
  (pToData)[3] |= ((pFrom)->m_nPbExtAddr0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nPbExtAddr0 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nPbExtAddr0 >> 16) & 0x01;
#else
  int i;
  int size = SB_ZF_ZFKAQMFBLINE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nHec1 */
  (pToData)[15] |= ((pFrom)->m_nHec1) & 0xFF;

  /* Pack Member: m_nHec0 */
  (pToData)[14] |= ((pFrom)->m_nHec0) & 0xFF;

  /* Pack Member: m_nSpare */
  (pToData)[12] |= ((pFrom)->m_nSpare & 0x03) <<6;
  (pToData)[13] |= ((pFrom)->m_nSpare >> 2) &0xFF;

  /* Pack Member: m_nPbExtAddr5 */
  (pToData)[10] |= ((pFrom)->m_nPbExtAddr5 & 0x07) <<5;
  (pToData)[11] |= ((pFrom)->m_nPbExtAddr5 >> 3) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nPbExtAddr5 >> 11) & 0x3f;

  /* Pack Member: m_nPbExtAddr4 */
  (pToData)[8] |= ((pFrom)->m_nPbExtAddr4 & 0x0f) <<4;
  (pToData)[9] |= ((pFrom)->m_nPbExtAddr4 >> 4) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nPbExtAddr4 >> 12) & 0x1f;

  /* Pack Member: m_nPbExtAddr3 */
  (pToData)[6] |= ((pFrom)->m_nPbExtAddr3 & 0x1f) <<3;
  (pToData)[7] |= ((pFrom)->m_nPbExtAddr3 >> 5) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nPbExtAddr3 >> 13) & 0x0f;

  /* Pack Member: m_nPbExtAddr2 */
  (pToData)[4] |= ((pFrom)->m_nPbExtAddr2 & 0x3f) <<2;
  (pToData)[5] |= ((pFrom)->m_nPbExtAddr2 >> 6) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nPbExtAddr2 >> 14) & 0x07;

  /* Pack Member: m_nPbExtAddr1 */
  (pToData)[2] |= ((pFrom)->m_nPbExtAddr1 & 0x7f) <<1;
  (pToData)[3] |= ((pFrom)->m_nPbExtAddr1 >> 7) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nPbExtAddr1 >> 15) & 0x03;

  /* Pack Member: m_nPbExtAddr0 */
  (pToData)[0] |= ((pFrom)->m_nPbExtAddr0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nPbExtAddr0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nPbExtAddr0 >> 16) & 0x01;
#endif

  return SB_ZF_ZFKAQMFBLINE_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmFbLine_Unpack(sbZfKaQmFbLine_t *pToStruct,
                      uint8_t *pFromData,
                      uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nHec1 */
  (pToStruct)->m_nHec1 =  (uint32_t)  (pFromData)[12] ;

  /* Unpack Member: m_nHec0 */
  (pToStruct)->m_nHec0 =  (uint32_t)  (pFromData)[13] ;

  /* Unpack Member: m_nSpare */
  (pToStruct)->m_nSpare =  (uint32_t)  ((pFromData)[15] >> 6) & 0x03;
  (pToStruct)->m_nSpare |=  (uint32_t)  (pFromData)[14] << 2;

  /* Unpack Member: m_nPbExtAddr5 */
  (pToStruct)->m_nPbExtAddr5 =  (uint32_t)  ((pFromData)[9] >> 5) & 0x07;
  (pToStruct)->m_nPbExtAddr5 |=  (uint32_t)  (pFromData)[8] << 3;
  (pToStruct)->m_nPbExtAddr5 |=  (uint32_t)  ((pFromData)[15] & 0x3f) << 11;

  /* Unpack Member: m_nPbExtAddr4 */
  (pToStruct)->m_nPbExtAddr4 =  (uint32_t)  ((pFromData)[11] >> 4) & 0x0f;
  (pToStruct)->m_nPbExtAddr4 |=  (uint32_t)  (pFromData)[10] << 4;
  (pToStruct)->m_nPbExtAddr4 |=  (uint32_t)  ((pFromData)[9] & 0x1f) << 12;

  /* Unpack Member: m_nPbExtAddr3 */
  (pToStruct)->m_nPbExtAddr3 =  (uint32_t)  ((pFromData)[5] >> 3) & 0x1f;
  (pToStruct)->m_nPbExtAddr3 |=  (uint32_t)  (pFromData)[4] << 5;
  (pToStruct)->m_nPbExtAddr3 |=  (uint32_t)  ((pFromData)[11] & 0x0f) << 13;

  /* Unpack Member: m_nPbExtAddr2 */
  (pToStruct)->m_nPbExtAddr2 =  (uint32_t)  ((pFromData)[7] >> 2) & 0x3f;
  (pToStruct)->m_nPbExtAddr2 |=  (uint32_t)  (pFromData)[6] << 6;
  (pToStruct)->m_nPbExtAddr2 |=  (uint32_t)  ((pFromData)[5] & 0x07) << 14;

  /* Unpack Member: m_nPbExtAddr1 */
  (pToStruct)->m_nPbExtAddr1 =  (uint32_t)  ((pFromData)[1] >> 1) & 0x7f;
  (pToStruct)->m_nPbExtAddr1 |=  (uint32_t)  (pFromData)[0] << 7;
  (pToStruct)->m_nPbExtAddr1 |=  (uint32_t)  ((pFromData)[7] & 0x03) << 15;

  /* Unpack Member: m_nPbExtAddr0 */
  (pToStruct)->m_nPbExtAddr0 =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nPbExtAddr0 |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_nPbExtAddr0 |=  (uint32_t)  ((pFromData)[1] & 0x01) << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nHec1 */
  (pToStruct)->m_nHec1 =  (uint32_t)  (pFromData)[15] ;

  /* Unpack Member: m_nHec0 */
  (pToStruct)->m_nHec0 =  (uint32_t)  (pFromData)[14] ;

  /* Unpack Member: m_nSpare */
  (pToStruct)->m_nSpare =  (uint32_t)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->m_nSpare |=  (uint32_t)  (pFromData)[13] << 2;

  /* Unpack Member: m_nPbExtAddr5 */
  (pToStruct)->m_nPbExtAddr5 =  (uint32_t)  ((pFromData)[10] >> 5) & 0x07;
  (pToStruct)->m_nPbExtAddr5 |=  (uint32_t)  (pFromData)[11] << 3;
  (pToStruct)->m_nPbExtAddr5 |=  (uint32_t)  ((pFromData)[12] & 0x3f) << 11;

  /* Unpack Member: m_nPbExtAddr4 */
  (pToStruct)->m_nPbExtAddr4 =  (uint32_t)  ((pFromData)[8] >> 4) & 0x0f;
  (pToStruct)->m_nPbExtAddr4 |=  (uint32_t)  (pFromData)[9] << 4;
  (pToStruct)->m_nPbExtAddr4 |=  (uint32_t)  ((pFromData)[10] & 0x1f) << 12;

  /* Unpack Member: m_nPbExtAddr3 */
  (pToStruct)->m_nPbExtAddr3 =  (uint32_t)  ((pFromData)[6] >> 3) & 0x1f;
  (pToStruct)->m_nPbExtAddr3 |=  (uint32_t)  (pFromData)[7] << 5;
  (pToStruct)->m_nPbExtAddr3 |=  (uint32_t)  ((pFromData)[8] & 0x0f) << 13;

  /* Unpack Member: m_nPbExtAddr2 */
  (pToStruct)->m_nPbExtAddr2 =  (uint32_t)  ((pFromData)[4] >> 2) & 0x3f;
  (pToStruct)->m_nPbExtAddr2 |=  (uint32_t)  (pFromData)[5] << 6;
  (pToStruct)->m_nPbExtAddr2 |=  (uint32_t)  ((pFromData)[6] & 0x07) << 14;

  /* Unpack Member: m_nPbExtAddr1 */
  (pToStruct)->m_nPbExtAddr1 =  (uint32_t)  ((pFromData)[2] >> 1) & 0x7f;
  (pToStruct)->m_nPbExtAddr1 |=  (uint32_t)  (pFromData)[3] << 7;
  (pToStruct)->m_nPbExtAddr1 |=  (uint32_t)  ((pFromData)[4] & 0x03) << 15;

  /* Unpack Member: m_nPbExtAddr0 */
  (pToStruct)->m_nPbExtAddr0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nPbExtAddr0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_nPbExtAddr0 |=  (uint32_t)  ((pFromData)[2] & 0x01) << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmFbLine_InitInstance(sbZfKaQmFbLine_t *pFrame) {

  pFrame->m_nHec1 =  (unsigned int)  0;
  pFrame->m_nHec0 =  (unsigned int)  0;
  pFrame->m_nSpare =  (unsigned int)  0;
  pFrame->m_nPbExtAddr5 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr4 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr3 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr2 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr1 =  (unsigned int)  0;
  pFrame->m_nPbExtAddr0 =  (unsigned int)  0;

}
