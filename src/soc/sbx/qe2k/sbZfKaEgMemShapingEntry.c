/*
 * $Id: sbZfKaEgMemShapingEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaEgMemShapingEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaEgMemShapingEntry_Pack(sbZfKaEgMemShapingEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEGMEMSHAPINGENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[10] |= ((pFrom)->m_nReserved & 0x07) <<5;

  /* Pack Member: m_nBucketDepth */
  (pToData)[5] |= ((pFrom)->m_nBucketDepth & 0x07) <<5;
  (pToData)[4] |= ((pFrom)->m_nBucketDepth >> 3) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nBucketDepth >> 11) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nBucketDepth >> 19) & 0x1f;

  /* Pack Member: m_nShapeRate */
  (pToData)[0] |= ((pFrom)->m_nShapeRate & 0x07) <<5;
  (pToData)[7] |= ((pFrom)->m_nShapeRate >> 3) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nShapeRate >> 11) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nShapeRate >> 19) & 0x1f;

  /* Pack Member: m_nMaxDepth */
  (pToData)[2] |= ((pFrom)->m_nMaxDepth & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_nMaxDepth >> 2) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nMaxDepth >> 10) & 0x1f;

  /* Pack Member: m_nPort */
  (pToData)[2] |= ((pFrom)->m_nPort & 0x3f);

  /* Pack Member: m_nHiSide */
  (pToData)[3] |= ((pFrom)->m_nHiSide & 0x01) <<7;

  /* Pack Member: m_nShapeSrc */
  (pToData)[3] |= ((pFrom)->m_nShapeSrc & 0x3f) <<1;

  /* Pack Member: m_nEnable */
  (pToData)[3] |= ((pFrom)->m_nEnable & 0x01);
#else
  int i;
  int size = SB_ZF_ZFKAEGMEMSHAPINGENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[9] |= ((pFrom)->m_nReserved & 0x07) <<5;

  /* Pack Member: m_nBucketDepth */
  (pToData)[6] |= ((pFrom)->m_nBucketDepth & 0x07) <<5;
  (pToData)[7] |= ((pFrom)->m_nBucketDepth >> 3) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nBucketDepth >> 11) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nBucketDepth >> 19) & 0x1f;

  /* Pack Member: m_nShapeRate */
  (pToData)[3] |= ((pFrom)->m_nShapeRate & 0x07) <<5;
  (pToData)[4] |= ((pFrom)->m_nShapeRate >> 3) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nShapeRate >> 11) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nShapeRate >> 19) & 0x1f;

  /* Pack Member: m_nMaxDepth */
  (pToData)[1] |= ((pFrom)->m_nMaxDepth & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nMaxDepth >> 2) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nMaxDepth >> 10) & 0x1f;

  /* Pack Member: m_nPort */
  (pToData)[1] |= ((pFrom)->m_nPort & 0x3f);

  /* Pack Member: m_nHiSide */
  (pToData)[0] |= ((pFrom)->m_nHiSide & 0x01) <<7;

  /* Pack Member: m_nShapeSrc */
  (pToData)[0] |= ((pFrom)->m_nShapeSrc & 0x3f) <<1;

  /* Pack Member: m_nEnable */
  (pToData)[0] |= ((pFrom)->m_nEnable & 0x01);
#endif

  return SB_ZF_ZFKAEGMEMSHAPINGENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEgMemShapingEntry_Unpack(sbZfKaEgMemShapingEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[10] >> 5) & 0x07;

  /* Unpack Member: m_nBucketDepth */
  (pToStruct)->m_nBucketDepth =  (uint32_t)  ((pFromData)[5] >> 5) & 0x07;
  (pToStruct)->m_nBucketDepth |=  (uint32_t)  (pFromData)[4] << 3;
  (pToStruct)->m_nBucketDepth |=  (uint32_t)  (pFromData)[11] << 11;
  (pToStruct)->m_nBucketDepth |=  (uint32_t)  ((pFromData)[10] & 0x1f) << 19;

  /* Unpack Member: m_nShapeRate */
  (pToStruct)->m_nShapeRate =  (uint32_t)  ((pFromData)[0] >> 5) & 0x07;
  (pToStruct)->m_nShapeRate |=  (uint32_t)  (pFromData)[7] << 3;
  (pToStruct)->m_nShapeRate |=  (uint32_t)  (pFromData)[6] << 11;
  (pToStruct)->m_nShapeRate |=  (uint32_t)  ((pFromData)[5] & 0x1f) << 19;

  /* Unpack Member: m_nMaxDepth */
  (pToStruct)->m_nMaxDepth =  (uint32_t)  ((pFromData)[2] >> 6) & 0x03;
  (pToStruct)->m_nMaxDepth |=  (uint32_t)  (pFromData)[1] << 2;
  (pToStruct)->m_nMaxDepth |=  (uint32_t)  ((pFromData)[0] & 0x1f) << 10;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32_t)  ((pFromData)[2] ) & 0x3f;

  /* Unpack Member: m_nHiSide */
  (pToStruct)->m_nHiSide =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_nShapeSrc */
  (pToStruct)->m_nShapeSrc =  (uint32_t)  ((pFromData)[3] >> 1) & 0x3f;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint32_t)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[9] >> 5) & 0x07;

  /* Unpack Member: m_nBucketDepth */
  (pToStruct)->m_nBucketDepth =  (uint32_t)  ((pFromData)[6] >> 5) & 0x07;
  (pToStruct)->m_nBucketDepth |=  (uint32_t)  (pFromData)[7] << 3;
  (pToStruct)->m_nBucketDepth |=  (uint32_t)  (pFromData)[8] << 11;
  (pToStruct)->m_nBucketDepth |=  (uint32_t)  ((pFromData)[9] & 0x1f) << 19;

  /* Unpack Member: m_nShapeRate */
  (pToStruct)->m_nShapeRate =  (uint32_t)  ((pFromData)[3] >> 5) & 0x07;
  (pToStruct)->m_nShapeRate |=  (uint32_t)  (pFromData)[4] << 3;
  (pToStruct)->m_nShapeRate |=  (uint32_t)  (pFromData)[5] << 11;
  (pToStruct)->m_nShapeRate |=  (uint32_t)  ((pFromData)[6] & 0x1f) << 19;

  /* Unpack Member: m_nMaxDepth */
  (pToStruct)->m_nMaxDepth =  (uint32_t)  ((pFromData)[1] >> 6) & 0x03;
  (pToStruct)->m_nMaxDepth |=  (uint32_t)  (pFromData)[2] << 2;
  (pToStruct)->m_nMaxDepth |=  (uint32_t)  ((pFromData)[3] & 0x1f) << 10;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32_t)  ((pFromData)[1] ) & 0x3f;

  /* Unpack Member: m_nHiSide */
  (pToStruct)->m_nHiSide =  (uint32_t)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_nShapeSrc */
  (pToStruct)->m_nShapeSrc =  (uint32_t)  ((pFromData)[0] >> 1) & 0x3f;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint32_t)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEgMemShapingEntry_InitInstance(sbZfKaEgMemShapingEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nBucketDepth =  (unsigned int)  0;
  pFrame->m_nShapeRate =  (unsigned int)  0;
  pFrame->m_nMaxDepth =  (unsigned int)  0;
  pFrame->m_nPort =  (unsigned int)  0;
  pFrame->m_nHiSide =  (unsigned int)  0;
  pFrame->m_nShapeSrc =  (unsigned int)  0;
  pFrame->m_nEnable =  (unsigned int)  0;

}
