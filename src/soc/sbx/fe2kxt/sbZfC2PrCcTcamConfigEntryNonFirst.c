/*
 * $Id: sbZfC2PrCcTcamConfigEntryNonFirst.c 1.1.22.4 Broadcom SDK $
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
#include "sbZfC2PrCcTcamConfigEntryNonFirst.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2PrCcTcamConfigEntryNonFirst_Pack(sbZfC2PrCcTcamConfigEntryNonFirst_t *pFrom,
                                       uint8_t *pToData,
                                       uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uMask0_31 */
  (pToData)[49] |= ((pFrom)->m_uMask0_31 & 0x03) <<6;
  (pToData)[50] |= ((pFrom)->m_uMask0_31 >> 2) &0xFF;
  (pToData)[51] |= ((pFrom)->m_uMask0_31 >> 10) &0xFF;
  (pToData)[52] |= ((pFrom)->m_uMask0_31 >> 18) &0xFF;
  (pToData)[53] |= ((pFrom)->m_uMask0_31 >> 26) & 0x3f;

  /* Pack Member: m_uMask32_63 */
  (pToData)[45] |= ((pFrom)->m_uMask32_63 & 0x03) <<6;
  (pToData)[46] |= ((pFrom)->m_uMask32_63 >> 2) &0xFF;
  (pToData)[47] |= ((pFrom)->m_uMask32_63 >> 10) &0xFF;
  (pToData)[48] |= ((pFrom)->m_uMask32_63 >> 18) &0xFF;
  (pToData)[49] |= ((pFrom)->m_uMask32_63 >> 26) & 0x3f;

  /* Pack Member: m_uMask64_95 */
  (pToData)[41] |= ((pFrom)->m_uMask64_95 & 0x03) <<6;
  (pToData)[42] |= ((pFrom)->m_uMask64_95 >> 2) &0xFF;
  (pToData)[43] |= ((pFrom)->m_uMask64_95 >> 10) &0xFF;
  (pToData)[44] |= ((pFrom)->m_uMask64_95 >> 18) &0xFF;
  (pToData)[45] |= ((pFrom)->m_uMask64_95 >> 26) & 0x3f;

  /* Pack Member: m_uMask96_127 */
  (pToData)[37] |= ((pFrom)->m_uMask96_127 & 0x03) <<6;
  (pToData)[38] |= ((pFrom)->m_uMask96_127 >> 2) &0xFF;
  (pToData)[39] |= ((pFrom)->m_uMask96_127 >> 10) &0xFF;
  (pToData)[40] |= ((pFrom)->m_uMask96_127 >> 18) &0xFF;
  (pToData)[41] |= ((pFrom)->m_uMask96_127 >> 26) & 0x3f;

  /* Pack Member: m_uMask128_159 */
  (pToData)[33] |= ((pFrom)->m_uMask128_159 & 0x03) <<6;
  (pToData)[34] |= ((pFrom)->m_uMask128_159 >> 2) &0xFF;
  (pToData)[35] |= ((pFrom)->m_uMask128_159 >> 10) &0xFF;
  (pToData)[36] |= ((pFrom)->m_uMask128_159 >> 18) &0xFF;
  (pToData)[37] |= ((pFrom)->m_uMask128_159 >> 26) & 0x3f;

  /* Pack Member: m_uMask160_191 */
  (pToData)[29] |= ((pFrom)->m_uMask160_191 & 0x03) <<6;
  (pToData)[30] |= ((pFrom)->m_uMask160_191 >> 2) &0xFF;
  (pToData)[31] |= ((pFrom)->m_uMask160_191 >> 10) &0xFF;
  (pToData)[32] |= ((pFrom)->m_uMask160_191 >> 18) &0xFF;
  (pToData)[33] |= ((pFrom)->m_uMask160_191 >> 26) & 0x3f;

  /* Pack Member: m_uMask192_199 */
  (pToData)[28] |= ((pFrom)->m_uMask192_199 & 0x03) <<6;
  (pToData)[29] |= ((pFrom)->m_uMask192_199 >> 2) & 0x3f;

  /* Pack Member: m_uMaskState */
  (pToData)[27] |= ((pFrom)->m_uMaskState) & 0xFF;
  (pToData)[28] |= ((pFrom)->m_uMaskState >> 8) & 0x3f;

  /* Pack Member: m_uPacket0_31 */
  (pToData)[23] |= ((pFrom)->m_uPacket0_31) & 0xFF;
  (pToData)[24] |= ((pFrom)->m_uPacket0_31 >> 8) &0xFF;
  (pToData)[25] |= ((pFrom)->m_uPacket0_31 >> 16) &0xFF;
  (pToData)[26] |= ((pFrom)->m_uPacket0_31 >> 24) &0xFF;

  /* Pack Member: m_uPacket32_63 */
  (pToData)[19] |= ((pFrom)->m_uPacket32_63) & 0xFF;
  (pToData)[20] |= ((pFrom)->m_uPacket32_63 >> 8) &0xFF;
  (pToData)[21] |= ((pFrom)->m_uPacket32_63 >> 16) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uPacket32_63 >> 24) &0xFF;

  /* Pack Member: m_uPacket64_95 */
  (pToData)[15] |= ((pFrom)->m_uPacket64_95) & 0xFF;
  (pToData)[16] |= ((pFrom)->m_uPacket64_95 >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_uPacket64_95 >> 16) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uPacket64_95 >> 24) &0xFF;

  /* Pack Member: m_uPacket96_127 */
  (pToData)[11] |= ((pFrom)->m_uPacket96_127) & 0xFF;
  (pToData)[12] |= ((pFrom)->m_uPacket96_127 >> 8) &0xFF;
  (pToData)[13] |= ((pFrom)->m_uPacket96_127 >> 16) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uPacket96_127 >> 24) &0xFF;

  /* Pack Member: m_uPacket128_159 */
  (pToData)[7] |= ((pFrom)->m_uPacket128_159) & 0xFF;
  (pToData)[8] |= ((pFrom)->m_uPacket128_159 >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_uPacket128_159 >> 16) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uPacket128_159 >> 24) &0xFF;

  /* Pack Member: m_uPacket160_191 */
  (pToData)[3] |= ((pFrom)->m_uPacket160_191) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_uPacket160_191 >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_uPacket160_191 >> 16) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uPacket160_191 >> 24) &0xFF;

  /* Pack Member: m_uPacket192_199 */
  (pToData)[2] |= ((pFrom)->m_uPacket192_199) & 0xFF;

  /* Pack Member: m_uState */
  (pToData)[0] |= ((pFrom)->m_uState & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_uState >> 6) &0xFF;

  /* Pack Member: m_uValid */
  (pToData)[0] |= ((pFrom)->m_uValid & 0x03);

  return SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2PrCcTcamConfigEntryNonFirst_Unpack(sbZfC2PrCcTcamConfigEntryNonFirst_t *pToStruct,
                                         uint8_t *pFromData,
                                         uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uMask0_31 */
  (pToStruct)->m_uMask0_31 =  (uint32_t)  ((pFromData)[49] >> 6) & 0x03;
  (pToStruct)->m_uMask0_31 |=  (uint32_t)  (pFromData)[50] << 2;
  (pToStruct)->m_uMask0_31 |=  (uint32_t)  (pFromData)[51] << 10;
  (pToStruct)->m_uMask0_31 |=  (uint32_t)  (pFromData)[52] << 18;
  (pToStruct)->m_uMask0_31 |=  (uint32_t)  ((pFromData)[53] & 0x3f) << 26;

  /* Unpack Member: m_uMask32_63 */
  (pToStruct)->m_uMask32_63 =  (uint32_t)  ((pFromData)[45] >> 6) & 0x03;
  (pToStruct)->m_uMask32_63 |=  (uint32_t)  (pFromData)[46] << 2;
  (pToStruct)->m_uMask32_63 |=  (uint32_t)  (pFromData)[47] << 10;
  (pToStruct)->m_uMask32_63 |=  (uint32_t)  (pFromData)[48] << 18;
  (pToStruct)->m_uMask32_63 |=  (uint32_t)  ((pFromData)[49] & 0x3f) << 26;

  /* Unpack Member: m_uMask64_95 */
  (pToStruct)->m_uMask64_95 =  (uint32_t)  ((pFromData)[41] >> 6) & 0x03;
  (pToStruct)->m_uMask64_95 |=  (uint32_t)  (pFromData)[42] << 2;
  (pToStruct)->m_uMask64_95 |=  (uint32_t)  (pFromData)[43] << 10;
  (pToStruct)->m_uMask64_95 |=  (uint32_t)  (pFromData)[44] << 18;
  (pToStruct)->m_uMask64_95 |=  (uint32_t)  ((pFromData)[45] & 0x3f) << 26;

  /* Unpack Member: m_uMask96_127 */
  (pToStruct)->m_uMask96_127 =  (uint32_t)  ((pFromData)[37] >> 6) & 0x03;
  (pToStruct)->m_uMask96_127 |=  (uint32_t)  (pFromData)[38] << 2;
  (pToStruct)->m_uMask96_127 |=  (uint32_t)  (pFromData)[39] << 10;
  (pToStruct)->m_uMask96_127 |=  (uint32_t)  (pFromData)[40] << 18;
  (pToStruct)->m_uMask96_127 |=  (uint32_t)  ((pFromData)[41] & 0x3f) << 26;

  /* Unpack Member: m_uMask128_159 */
  (pToStruct)->m_uMask128_159 =  (uint32_t)  ((pFromData)[33] >> 6) & 0x03;
  (pToStruct)->m_uMask128_159 |=  (uint32_t)  (pFromData)[34] << 2;
  (pToStruct)->m_uMask128_159 |=  (uint32_t)  (pFromData)[35] << 10;
  (pToStruct)->m_uMask128_159 |=  (uint32_t)  (pFromData)[36] << 18;
  (pToStruct)->m_uMask128_159 |=  (uint32_t)  ((pFromData)[37] & 0x3f) << 26;

  /* Unpack Member: m_uMask160_191 */
  (pToStruct)->m_uMask160_191 =  (uint32_t)  ((pFromData)[29] >> 6) & 0x03;
  (pToStruct)->m_uMask160_191 |=  (uint32_t)  (pFromData)[30] << 2;
  (pToStruct)->m_uMask160_191 |=  (uint32_t)  (pFromData)[31] << 10;
  (pToStruct)->m_uMask160_191 |=  (uint32_t)  (pFromData)[32] << 18;
  (pToStruct)->m_uMask160_191 |=  (uint32_t)  ((pFromData)[33] & 0x3f) << 26;

  /* Unpack Member: m_uMask192_199 */
  (pToStruct)->m_uMask192_199 =  (uint32_t)  ((pFromData)[28] >> 6) & 0x03;
  (pToStruct)->m_uMask192_199 |=  (uint32_t)  ((pFromData)[29] & 0x3f) << 2;

  /* Unpack Member: m_uMaskState */
  (pToStruct)->m_uMaskState =  (uint32_t)  (pFromData)[27] ;
  (pToStruct)->m_uMaskState |=  (uint32_t)  ((pFromData)[28] & 0x3f) << 8;

  /* Unpack Member: m_uPacket0_31 */
  (pToStruct)->m_uPacket0_31 =  (uint32_t)  (pFromData)[23] ;
  (pToStruct)->m_uPacket0_31 |=  (uint32_t)  (pFromData)[24] << 8;
  (pToStruct)->m_uPacket0_31 |=  (uint32_t)  (pFromData)[25] << 16;
  (pToStruct)->m_uPacket0_31 |=  (uint32_t)  (pFromData)[26] << 24;

  /* Unpack Member: m_uPacket32_63 */
  (pToStruct)->m_uPacket32_63 =  (uint32_t)  (pFromData)[19] ;
  (pToStruct)->m_uPacket32_63 |=  (uint32_t)  (pFromData)[20] << 8;
  (pToStruct)->m_uPacket32_63 |=  (uint32_t)  (pFromData)[21] << 16;
  (pToStruct)->m_uPacket32_63 |=  (uint32_t)  (pFromData)[22] << 24;

  /* Unpack Member: m_uPacket64_95 */
  (pToStruct)->m_uPacket64_95 =  (uint32_t)  (pFromData)[15] ;
  (pToStruct)->m_uPacket64_95 |=  (uint32_t)  (pFromData)[16] << 8;
  (pToStruct)->m_uPacket64_95 |=  (uint32_t)  (pFromData)[17] << 16;
  (pToStruct)->m_uPacket64_95 |=  (uint32_t)  (pFromData)[18] << 24;

  /* Unpack Member: m_uPacket96_127 */
  (pToStruct)->m_uPacket96_127 =  (uint32_t)  (pFromData)[11] ;
  (pToStruct)->m_uPacket96_127 |=  (uint32_t)  (pFromData)[12] << 8;
  (pToStruct)->m_uPacket96_127 |=  (uint32_t)  (pFromData)[13] << 16;
  (pToStruct)->m_uPacket96_127 |=  (uint32_t)  (pFromData)[14] << 24;

  /* Unpack Member: m_uPacket128_159 */
  (pToStruct)->m_uPacket128_159 =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_uPacket128_159 |=  (uint32_t)  (pFromData)[8] << 8;
  (pToStruct)->m_uPacket128_159 |=  (uint32_t)  (pFromData)[9] << 16;
  (pToStruct)->m_uPacket128_159 |=  (uint32_t)  (pFromData)[10] << 24;

  /* Unpack Member: m_uPacket160_191 */
  (pToStruct)->m_uPacket160_191 =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_uPacket160_191 |=  (uint32_t)  (pFromData)[4] << 8;
  (pToStruct)->m_uPacket160_191 |=  (uint32_t)  (pFromData)[5] << 16;
  (pToStruct)->m_uPacket160_191 |=  (uint32_t)  (pFromData)[6] << 24;

  /* Unpack Member: m_uPacket192_199 */
  (pToStruct)->m_uPacket192_199 =  (uint32_t)  (pFromData)[2] ;

  /* Unpack Member: m_uState */
  (pToStruct)->m_uState =  (uint32_t)  ((pFromData)[0] >> 2) & 0x3f;
  (pToStruct)->m_uState |=  (uint32_t)  (pFromData)[1] << 6;

  /* Unpack Member: m_uValid */
  (pToStruct)->m_uValid =  (uint32_t)  ((pFromData)[0] ) & 0x03;

}



/* initialize an instance of this zframe */
void
sbZfC2PrCcTcamConfigEntryNonFirst_InitInstance(sbZfC2PrCcTcamConfigEntryNonFirst_t *pFrame) {

  pFrame->m_uMask0_31 =  (unsigned int)  0;
  pFrame->m_uMask32_63 =  (unsigned int)  0;
  pFrame->m_uMask64_95 =  (unsigned int)  0;
  pFrame->m_uMask96_127 =  (unsigned int)  0;
  pFrame->m_uMask128_159 =  (unsigned int)  0;
  pFrame->m_uMask160_191 =  (unsigned int)  0;
  pFrame->m_uMask192_199 =  (unsigned int)  0;
  pFrame->m_uMaskState =  (unsigned int)  0;
  pFrame->m_uPacket0_31 =  (unsigned int)  0;
  pFrame->m_uPacket32_63 =  (unsigned int)  0;
  pFrame->m_uPacket64_95 =  (unsigned int)  0;
  pFrame->m_uPacket96_127 =  (unsigned int)  0;
  pFrame->m_uPacket128_159 =  (unsigned int)  0;
  pFrame->m_uPacket160_191 =  (unsigned int)  0;
  pFrame->m_uPacket192_199 =  (unsigned int)  0;
  pFrame->m_uState =  (unsigned int)  0;
  pFrame->m_uValid =  (unsigned int)  0;

}
