/*
 * $Id: sbZfC2PrCcTcamConfigEntry.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfC2PrCcTcamConfigEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2PrCcTcamConfigEntry_Pack(sbZfC2PrCcTcamConfigEntry_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uMask213_192 */
  (pToData)[51] |= ((pFrom)->m_uMask213_192) & 0xFF;
  (pToData)[52] |= ((pFrom)->m_uMask213_192 >> 8) &0xFF;
  (pToData)[53] |= ((pFrom)->m_uMask213_192 >> 16) & 0x3f;

  /* Pack Member: m_uMask191_160 */
  (pToData)[47] |= ((pFrom)->m_uMask191_160) & 0xFF;
  (pToData)[48] |= ((pFrom)->m_uMask191_160 >> 8) &0xFF;
  (pToData)[49] |= ((pFrom)->m_uMask191_160 >> 16) &0xFF;
  (pToData)[50] |= ((pFrom)->m_uMask191_160 >> 24) &0xFF;

  /* Pack Member: m_uMask159_128 */
  (pToData)[43] |= ((pFrom)->m_uMask159_128) & 0xFF;
  (pToData)[44] |= ((pFrom)->m_uMask159_128 >> 8) &0xFF;
  (pToData)[45] |= ((pFrom)->m_uMask159_128 >> 16) &0xFF;
  (pToData)[46] |= ((pFrom)->m_uMask159_128 >> 24) &0xFF;

  /* Pack Member: m_uMask127_96 */
  (pToData)[39] |= ((pFrom)->m_uMask127_96) & 0xFF;
  (pToData)[40] |= ((pFrom)->m_uMask127_96 >> 8) &0xFF;
  (pToData)[41] |= ((pFrom)->m_uMask127_96 >> 16) &0xFF;
  (pToData)[42] |= ((pFrom)->m_uMask127_96 >> 24) &0xFF;

  /* Pack Member: m_uMask95_64 */
  (pToData)[35] |= ((pFrom)->m_uMask95_64) & 0xFF;
  (pToData)[36] |= ((pFrom)->m_uMask95_64 >> 8) &0xFF;
  (pToData)[37] |= ((pFrom)->m_uMask95_64 >> 16) &0xFF;
  (pToData)[38] |= ((pFrom)->m_uMask95_64 >> 24) &0xFF;

  /* Pack Member: m_uMask63_32 */
  (pToData)[31] |= ((pFrom)->m_uMask63_32) & 0xFF;
  (pToData)[32] |= ((pFrom)->m_uMask63_32 >> 8) &0xFF;
  (pToData)[33] |= ((pFrom)->m_uMask63_32 >> 16) &0xFF;
  (pToData)[34] |= ((pFrom)->m_uMask63_32 >> 24) &0xFF;

  /* Pack Member: m_uMask31_0 */
  (pToData)[27] |= ((pFrom)->m_uMask31_0) & 0xFF;
  (pToData)[28] |= ((pFrom)->m_uMask31_0 >> 8) &0xFF;
  (pToData)[29] |= ((pFrom)->m_uMask31_0 >> 16) &0xFF;
  (pToData)[30] |= ((pFrom)->m_uMask31_0 >> 24) &0xFF;

  /* Pack Member: m_uData213_192 */
  (pToData)[24] |= ((pFrom)->m_uData213_192 & 0x3f) <<2;
  (pToData)[25] |= ((pFrom)->m_uData213_192 >> 6) &0xFF;
  (pToData)[26] |= ((pFrom)->m_uData213_192 >> 14) &0xFF;

  /* Pack Member: m_uData191_160 */
  (pToData)[20] |= ((pFrom)->m_uData191_160 & 0x3f) <<2;
  (pToData)[21] |= ((pFrom)->m_uData191_160 >> 6) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uData191_160 >> 14) &0xFF;
  (pToData)[23] |= ((pFrom)->m_uData191_160 >> 22) &0xFF;
  (pToData)[24] |= ((pFrom)->m_uData191_160 >> 30) & 0x03;

  /* Pack Member: m_uData159_128 */
  (pToData)[16] |= ((pFrom)->m_uData159_128 & 0x3f) <<2;
  (pToData)[17] |= ((pFrom)->m_uData159_128 >> 6) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uData159_128 >> 14) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uData159_128 >> 22) &0xFF;
  (pToData)[20] |= ((pFrom)->m_uData159_128 >> 30) & 0x03;

  /* Pack Member: m_uData127_96 */
  (pToData)[12] |= ((pFrom)->m_uData127_96 & 0x3f) <<2;
  (pToData)[13] |= ((pFrom)->m_uData127_96 >> 6) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uData127_96 >> 14) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uData127_96 >> 22) &0xFF;
  (pToData)[16] |= ((pFrom)->m_uData127_96 >> 30) & 0x03;

  /* Pack Member: m_uData95_64 */
  (pToData)[8] |= ((pFrom)->m_uData95_64 & 0x3f) <<2;
  (pToData)[9] |= ((pFrom)->m_uData95_64 >> 6) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uData95_64 >> 14) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uData95_64 >> 22) &0xFF;
  (pToData)[12] |= ((pFrom)->m_uData95_64 >> 30) & 0x03;

  /* Pack Member: m_uData63_32 */
  (pToData)[4] |= ((pFrom)->m_uData63_32 & 0x3f) <<2;
  (pToData)[5] |= ((pFrom)->m_uData63_32 >> 6) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uData63_32 >> 14) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uData63_32 >> 22) &0xFF;
  (pToData)[8] |= ((pFrom)->m_uData63_32 >> 30) & 0x03;

  /* Pack Member: m_uData31_0 */
  (pToData)[0] |= ((pFrom)->m_uData31_0 & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_uData31_0 >> 6) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uData31_0 >> 14) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uData31_0 >> 22) &0xFF;
  (pToData)[4] |= ((pFrom)->m_uData31_0 >> 30) & 0x03;

  /* Pack Member: m_uValid */
  (pToData)[0] |= ((pFrom)->m_uValid & 0x03);

  return SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2PrCcTcamConfigEntry_Unpack(sbZfC2PrCcTcamConfigEntry_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uMask213_192 */
  (pToStruct)->m_uMask213_192 =  (uint32_t)  (pFromData)[51] ;
  (pToStruct)->m_uMask213_192 |=  (uint32_t)  (pFromData)[52] << 8;
  (pToStruct)->m_uMask213_192 |=  (uint32_t)  ((pFromData)[53] & 0x3f) << 16;

  /* Unpack Member: m_uMask191_160 */
  (pToStruct)->m_uMask191_160 =  (uint32_t)  (pFromData)[47] ;
  (pToStruct)->m_uMask191_160 |=  (uint32_t)  (pFromData)[48] << 8;
  (pToStruct)->m_uMask191_160 |=  (uint32_t)  (pFromData)[49] << 16;
  (pToStruct)->m_uMask191_160 |=  (uint32_t)  (pFromData)[50] << 24;

  /* Unpack Member: m_uMask159_128 */
  (pToStruct)->m_uMask159_128 =  (uint32_t)  (pFromData)[43] ;
  (pToStruct)->m_uMask159_128 |=  (uint32_t)  (pFromData)[44] << 8;
  (pToStruct)->m_uMask159_128 |=  (uint32_t)  (pFromData)[45] << 16;
  (pToStruct)->m_uMask159_128 |=  (uint32_t)  (pFromData)[46] << 24;

  /* Unpack Member: m_uMask127_96 */
  (pToStruct)->m_uMask127_96 =  (uint32_t)  (pFromData)[39] ;
  (pToStruct)->m_uMask127_96 |=  (uint32_t)  (pFromData)[40] << 8;
  (pToStruct)->m_uMask127_96 |=  (uint32_t)  (pFromData)[41] << 16;
  (pToStruct)->m_uMask127_96 |=  (uint32_t)  (pFromData)[42] << 24;

  /* Unpack Member: m_uMask95_64 */
  (pToStruct)->m_uMask95_64 =  (uint32_t)  (pFromData)[35] ;
  (pToStruct)->m_uMask95_64 |=  (uint32_t)  (pFromData)[36] << 8;
  (pToStruct)->m_uMask95_64 |=  (uint32_t)  (pFromData)[37] << 16;
  (pToStruct)->m_uMask95_64 |=  (uint32_t)  (pFromData)[38] << 24;

  /* Unpack Member: m_uMask63_32 */
  (pToStruct)->m_uMask63_32 =  (uint32_t)  (pFromData)[31] ;
  (pToStruct)->m_uMask63_32 |=  (uint32_t)  (pFromData)[32] << 8;
  (pToStruct)->m_uMask63_32 |=  (uint32_t)  (pFromData)[33] << 16;
  (pToStruct)->m_uMask63_32 |=  (uint32_t)  (pFromData)[34] << 24;

  /* Unpack Member: m_uMask31_0 */
  (pToStruct)->m_uMask31_0 =  (uint32_t)  (pFromData)[27] ;
  (pToStruct)->m_uMask31_0 |=  (uint32_t)  (pFromData)[28] << 8;
  (pToStruct)->m_uMask31_0 |=  (uint32_t)  (pFromData)[29] << 16;
  (pToStruct)->m_uMask31_0 |=  (uint32_t)  (pFromData)[30] << 24;

  /* Unpack Member: m_uData213_192 */
  (pToStruct)->m_uData213_192 =  (uint32_t)  ((pFromData)[24] >> 2) & 0x3f;
  (pToStruct)->m_uData213_192 |=  (uint32_t)  (pFromData)[25] << 6;
  (pToStruct)->m_uData213_192 |=  (uint32_t)  (pFromData)[26] << 14;

  /* Unpack Member: m_uData191_160 */
  (pToStruct)->m_uData191_160 =  (uint32_t)  ((pFromData)[20] >> 2) & 0x3f;
  (pToStruct)->m_uData191_160 |=  (uint32_t)  (pFromData)[21] << 6;
  (pToStruct)->m_uData191_160 |=  (uint32_t)  (pFromData)[22] << 14;
  (pToStruct)->m_uData191_160 |=  (uint32_t)  (pFromData)[23] << 22;
  (pToStruct)->m_uData191_160 |=  (uint32_t)  ((pFromData)[24] & 0x03) << 30;

  /* Unpack Member: m_uData159_128 */
  (pToStruct)->m_uData159_128 =  (uint32_t)  ((pFromData)[16] >> 2) & 0x3f;
  (pToStruct)->m_uData159_128 |=  (uint32_t)  (pFromData)[17] << 6;
  (pToStruct)->m_uData159_128 |=  (uint32_t)  (pFromData)[18] << 14;
  (pToStruct)->m_uData159_128 |=  (uint32_t)  (pFromData)[19] << 22;
  (pToStruct)->m_uData159_128 |=  (uint32_t)  ((pFromData)[20] & 0x03) << 30;

  /* Unpack Member: m_uData127_96 */
  (pToStruct)->m_uData127_96 =  (uint32_t)  ((pFromData)[12] >> 2) & 0x3f;
  (pToStruct)->m_uData127_96 |=  (uint32_t)  (pFromData)[13] << 6;
  (pToStruct)->m_uData127_96 |=  (uint32_t)  (pFromData)[14] << 14;
  (pToStruct)->m_uData127_96 |=  (uint32_t)  (pFromData)[15] << 22;
  (pToStruct)->m_uData127_96 |=  (uint32_t)  ((pFromData)[16] & 0x03) << 30;

  /* Unpack Member: m_uData95_64 */
  (pToStruct)->m_uData95_64 =  (uint32_t)  ((pFromData)[8] >> 2) & 0x3f;
  (pToStruct)->m_uData95_64 |=  (uint32_t)  (pFromData)[9] << 6;
  (pToStruct)->m_uData95_64 |=  (uint32_t)  (pFromData)[10] << 14;
  (pToStruct)->m_uData95_64 |=  (uint32_t)  (pFromData)[11] << 22;
  (pToStruct)->m_uData95_64 |=  (uint32_t)  ((pFromData)[12] & 0x03) << 30;

  /* Unpack Member: m_uData63_32 */
  (pToStruct)->m_uData63_32 =  (uint32_t)  ((pFromData)[4] >> 2) & 0x3f;
  (pToStruct)->m_uData63_32 |=  (uint32_t)  (pFromData)[5] << 6;
  (pToStruct)->m_uData63_32 |=  (uint32_t)  (pFromData)[6] << 14;
  (pToStruct)->m_uData63_32 |=  (uint32_t)  (pFromData)[7] << 22;
  (pToStruct)->m_uData63_32 |=  (uint32_t)  ((pFromData)[8] & 0x03) << 30;

  /* Unpack Member: m_uData31_0 */
  (pToStruct)->m_uData31_0 =  (uint32_t)  ((pFromData)[0] >> 2) & 0x3f;
  (pToStruct)->m_uData31_0 |=  (uint32_t)  (pFromData)[1] << 6;
  (pToStruct)->m_uData31_0 |=  (uint32_t)  (pFromData)[2] << 14;
  (pToStruct)->m_uData31_0 |=  (uint32_t)  (pFromData)[3] << 22;
  (pToStruct)->m_uData31_0 |=  (uint32_t)  ((pFromData)[4] & 0x03) << 30;

  /* Unpack Member: m_uValid */
  (pToStruct)->m_uValid =  (uint32_t)  ((pFromData)[0] ) & 0x03;

}



/* initialize an instance of this zframe */
void
sbZfC2PrCcTcamConfigEntry_InitInstance(sbZfC2PrCcTcamConfigEntry_t *pFrame) {

  pFrame->m_uMask213_192 =  (unsigned int)  0;
  pFrame->m_uMask191_160 =  (unsigned int)  0;
  pFrame->m_uMask159_128 =  (unsigned int)  0;
  pFrame->m_uMask127_96 =  (unsigned int)  0;
  pFrame->m_uMask95_64 =  (unsigned int)  0;
  pFrame->m_uMask63_32 =  (unsigned int)  0;
  pFrame->m_uMask31_0 =  (unsigned int)  0;
  pFrame->m_uData213_192 =  (unsigned int)  0;
  pFrame->m_uData191_160 =  (unsigned int)  0;
  pFrame->m_uData159_128 =  (unsigned int)  0;
  pFrame->m_uData127_96 =  (unsigned int)  0;
  pFrame->m_uData95_64 =  (unsigned int)  0;
  pFrame->m_uData63_32 =  (unsigned int)  0;
  pFrame->m_uData31_0 =  (unsigned int)  0;
  pFrame->m_uValid =  (unsigned int)  0;

}
