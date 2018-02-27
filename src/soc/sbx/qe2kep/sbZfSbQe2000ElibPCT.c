/*
 * $Id: sbZfSbQe2000ElibPCT.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfSbQe2000ElibPCT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfSbQe2000ElibPCT_Pack(sbZfSbQe2000ElibPCT_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_PktClass15 */
  (pToData)[124] |= ((pFrom)->m_PktClass15 & 0x1f) <<3;
  (pToData)[125] |= ((pFrom)->m_PktClass15 >> 5) &0xFF;
  (pToData)[126] |= ((pFrom)->m_PktClass15 >> 13) &0xFF;
  (pToData)[127] |= ((pFrom)->m_PktClass15 >> 21) &0xFF;

  /* Pack Member: m_ByteClass15 */
  (pToData)[120] |= ((pFrom)->m_ByteClass15) & 0xFF;
  (pToData)[121] |= ((pFrom)->m_ByteClass15 >> 8) &0xFF;
  (pToData)[122] |= ((pFrom)->m_ByteClass15 >> 16) &0xFF;
  (pToData)[123] |= ((pFrom)->m_ByteClass15 >> 24) &0xFF;
  (pToData)[124] |= ((pFrom)->m_ByteClass15 >> 32) & 0x07;

  /* Pack Member: m_PktClass14 */
  (pToData)[116] |= ((pFrom)->m_PktClass14 & 0x1f) <<3;
  (pToData)[117] |= ((pFrom)->m_PktClass14 >> 5) &0xFF;
  (pToData)[118] |= ((pFrom)->m_PktClass14 >> 13) &0xFF;
  (pToData)[119] |= ((pFrom)->m_PktClass14 >> 21) &0xFF;

  /* Pack Member: m_ByteClass14 */
  (pToData)[112] |= ((pFrom)->m_ByteClass14) & 0xFF;
  (pToData)[113] |= ((pFrom)->m_ByteClass14 >> 8) &0xFF;
  (pToData)[114] |= ((pFrom)->m_ByteClass14 >> 16) &0xFF;
  (pToData)[115] |= ((pFrom)->m_ByteClass14 >> 24) &0xFF;
  (pToData)[116] |= ((pFrom)->m_ByteClass14 >> 32) & 0x07;

  /* Pack Member: m_PktClass13 */
  (pToData)[108] |= ((pFrom)->m_PktClass13 & 0x1f) <<3;
  (pToData)[109] |= ((pFrom)->m_PktClass13 >> 5) &0xFF;
  (pToData)[110] |= ((pFrom)->m_PktClass13 >> 13) &0xFF;
  (pToData)[111] |= ((pFrom)->m_PktClass13 >> 21) &0xFF;

  /* Pack Member: m_ByteClass13 */
  (pToData)[104] |= ((pFrom)->m_ByteClass13) & 0xFF;
  (pToData)[105] |= ((pFrom)->m_ByteClass13 >> 8) &0xFF;
  (pToData)[106] |= ((pFrom)->m_ByteClass13 >> 16) &0xFF;
  (pToData)[107] |= ((pFrom)->m_ByteClass13 >> 24) &0xFF;
  (pToData)[108] |= ((pFrom)->m_ByteClass13 >> 32) & 0x07;

  /* Pack Member: m_PktClass12 */
  (pToData)[100] |= ((pFrom)->m_PktClass12 & 0x1f) <<3;
  (pToData)[101] |= ((pFrom)->m_PktClass12 >> 5) &0xFF;
  (pToData)[102] |= ((pFrom)->m_PktClass12 >> 13) &0xFF;
  (pToData)[103] |= ((pFrom)->m_PktClass12 >> 21) &0xFF;

  /* Pack Member: m_ByteClass12 */
  (pToData)[96] |= ((pFrom)->m_ByteClass12) & 0xFF;
  (pToData)[97] |= ((pFrom)->m_ByteClass12 >> 8) &0xFF;
  (pToData)[98] |= ((pFrom)->m_ByteClass12 >> 16) &0xFF;
  (pToData)[99] |= ((pFrom)->m_ByteClass12 >> 24) &0xFF;
  (pToData)[100] |= ((pFrom)->m_ByteClass12 >> 32) & 0x07;

  /* Pack Member: m_PktClass11 */
  (pToData)[92] |= ((pFrom)->m_PktClass11 & 0x1f) <<3;
  (pToData)[93] |= ((pFrom)->m_PktClass11 >> 5) &0xFF;
  (pToData)[94] |= ((pFrom)->m_PktClass11 >> 13) &0xFF;
  (pToData)[95] |= ((pFrom)->m_PktClass11 >> 21) &0xFF;

  /* Pack Member: m_ByteClass11 */
  (pToData)[88] |= ((pFrom)->m_ByteClass11) & 0xFF;
  (pToData)[89] |= ((pFrom)->m_ByteClass11 >> 8) &0xFF;
  (pToData)[90] |= ((pFrom)->m_ByteClass11 >> 16) &0xFF;
  (pToData)[91] |= ((pFrom)->m_ByteClass11 >> 24) &0xFF;
  (pToData)[92] |= ((pFrom)->m_ByteClass11 >> 32) & 0x07;

  /* Pack Member: m_PktClass10 */
  (pToData)[84] |= ((pFrom)->m_PktClass10 & 0x1f) <<3;
  (pToData)[85] |= ((pFrom)->m_PktClass10 >> 5) &0xFF;
  (pToData)[86] |= ((pFrom)->m_PktClass10 >> 13) &0xFF;
  (pToData)[87] |= ((pFrom)->m_PktClass10 >> 21) &0xFF;

  /* Pack Member: m_ByteClass10 */
  (pToData)[80] |= ((pFrom)->m_ByteClass10) & 0xFF;
  (pToData)[81] |= ((pFrom)->m_ByteClass10 >> 8) &0xFF;
  (pToData)[82] |= ((pFrom)->m_ByteClass10 >> 16) &0xFF;
  (pToData)[83] |= ((pFrom)->m_ByteClass10 >> 24) &0xFF;
  (pToData)[84] |= ((pFrom)->m_ByteClass10 >> 32) & 0x07;

  /* Pack Member: m_PktClass9 */
  (pToData)[76] |= ((pFrom)->m_PktClass9 & 0x1f) <<3;
  (pToData)[77] |= ((pFrom)->m_PktClass9 >> 5) &0xFF;
  (pToData)[78] |= ((pFrom)->m_PktClass9 >> 13) &0xFF;
  (pToData)[79] |= ((pFrom)->m_PktClass9 >> 21) &0xFF;

  /* Pack Member: m_ByteClass9 */
  (pToData)[72] |= ((pFrom)->m_ByteClass9) & 0xFF;
  (pToData)[73] |= ((pFrom)->m_ByteClass9 >> 8) &0xFF;
  (pToData)[74] |= ((pFrom)->m_ByteClass9 >> 16) &0xFF;
  (pToData)[75] |= ((pFrom)->m_ByteClass9 >> 24) &0xFF;
  (pToData)[76] |= ((pFrom)->m_ByteClass9 >> 32) & 0x07;

  /* Pack Member: m_PktClass8 */
  (pToData)[68] |= ((pFrom)->m_PktClass8 & 0x1f) <<3;
  (pToData)[69] |= ((pFrom)->m_PktClass8 >> 5) &0xFF;
  (pToData)[70] |= ((pFrom)->m_PktClass8 >> 13) &0xFF;
  (pToData)[71] |= ((pFrom)->m_PktClass8 >> 21) &0xFF;

  /* Pack Member: m_ByteClass8 */
  (pToData)[64] |= ((pFrom)->m_ByteClass8) & 0xFF;
  (pToData)[65] |= ((pFrom)->m_ByteClass8 >> 8) &0xFF;
  (pToData)[66] |= ((pFrom)->m_ByteClass8 >> 16) &0xFF;
  (pToData)[67] |= ((pFrom)->m_ByteClass8 >> 24) &0xFF;
  (pToData)[68] |= ((pFrom)->m_ByteClass8 >> 32) & 0x07;

  /* Pack Member: m_PktClass7 */
  (pToData)[60] |= ((pFrom)->m_PktClass7 & 0x1f) <<3;
  (pToData)[61] |= ((pFrom)->m_PktClass7 >> 5) &0xFF;
  (pToData)[62] |= ((pFrom)->m_PktClass7 >> 13) &0xFF;
  (pToData)[63] |= ((pFrom)->m_PktClass7 >> 21) &0xFF;

  /* Pack Member: m_ByteClass7 */
  (pToData)[56] |= ((pFrom)->m_ByteClass7) & 0xFF;
  (pToData)[57] |= ((pFrom)->m_ByteClass7 >> 8) &0xFF;
  (pToData)[58] |= ((pFrom)->m_ByteClass7 >> 16) &0xFF;
  (pToData)[59] |= ((pFrom)->m_ByteClass7 >> 24) &0xFF;
  (pToData)[60] |= ((pFrom)->m_ByteClass7 >> 32) & 0x07;

  /* Pack Member: m_PktClass6 */
  (pToData)[52] |= ((pFrom)->m_PktClass6 & 0x1f) <<3;
  (pToData)[53] |= ((pFrom)->m_PktClass6 >> 5) &0xFF;
  (pToData)[54] |= ((pFrom)->m_PktClass6 >> 13) &0xFF;
  (pToData)[55] |= ((pFrom)->m_PktClass6 >> 21) &0xFF;

  /* Pack Member: m_ByteClass6 */
  (pToData)[48] |= ((pFrom)->m_ByteClass6) & 0xFF;
  (pToData)[49] |= ((pFrom)->m_ByteClass6 >> 8) &0xFF;
  (pToData)[50] |= ((pFrom)->m_ByteClass6 >> 16) &0xFF;
  (pToData)[51] |= ((pFrom)->m_ByteClass6 >> 24) &0xFF;
  (pToData)[52] |= ((pFrom)->m_ByteClass6 >> 32) & 0x07;

  /* Pack Member: m_PktClass5 */
  (pToData)[44] |= ((pFrom)->m_PktClass5 & 0x1f) <<3;
  (pToData)[45] |= ((pFrom)->m_PktClass5 >> 5) &0xFF;
  (pToData)[46] |= ((pFrom)->m_PktClass5 >> 13) &0xFF;
  (pToData)[47] |= ((pFrom)->m_PktClass5 >> 21) &0xFF;

  /* Pack Member: m_ByteClass5 */
  (pToData)[40] |= ((pFrom)->m_ByteClass5) & 0xFF;
  (pToData)[41] |= ((pFrom)->m_ByteClass5 >> 8) &0xFF;
  (pToData)[42] |= ((pFrom)->m_ByteClass5 >> 16) &0xFF;
  (pToData)[43] |= ((pFrom)->m_ByteClass5 >> 24) &0xFF;
  (pToData)[44] |= ((pFrom)->m_ByteClass5 >> 32) & 0x07;

  /* Pack Member: m_PktClass4 */
  (pToData)[36] |= ((pFrom)->m_PktClass4 & 0x1f) <<3;
  (pToData)[37] |= ((pFrom)->m_PktClass4 >> 5) &0xFF;
  (pToData)[38] |= ((pFrom)->m_PktClass4 >> 13) &0xFF;
  (pToData)[39] |= ((pFrom)->m_PktClass4 >> 21) &0xFF;

  /* Pack Member: m_ByteClass4 */
  (pToData)[32] |= ((pFrom)->m_ByteClass4) & 0xFF;
  (pToData)[33] |= ((pFrom)->m_ByteClass4 >> 8) &0xFF;
  (pToData)[34] |= ((pFrom)->m_ByteClass4 >> 16) &0xFF;
  (pToData)[35] |= ((pFrom)->m_ByteClass4 >> 24) &0xFF;
  (pToData)[36] |= ((pFrom)->m_ByteClass4 >> 32) & 0x07;

  /* Pack Member: m_PktClass3 */
  (pToData)[28] |= ((pFrom)->m_PktClass3 & 0x1f) <<3;
  (pToData)[29] |= ((pFrom)->m_PktClass3 >> 5) &0xFF;
  (pToData)[30] |= ((pFrom)->m_PktClass3 >> 13) &0xFF;
  (pToData)[31] |= ((pFrom)->m_PktClass3 >> 21) &0xFF;

  /* Pack Member: m_ByteClass3 */
  (pToData)[24] |= ((pFrom)->m_ByteClass3) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_ByteClass3 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_ByteClass3 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_ByteClass3 >> 24) &0xFF;
  (pToData)[28] |= ((pFrom)->m_ByteClass3 >> 32) & 0x07;

  /* Pack Member: m_PktClass2 */
  (pToData)[20] |= ((pFrom)->m_PktClass2 & 0x1f) <<3;
  (pToData)[21] |= ((pFrom)->m_PktClass2 >> 5) &0xFF;
  (pToData)[22] |= ((pFrom)->m_PktClass2 >> 13) &0xFF;
  (pToData)[23] |= ((pFrom)->m_PktClass2 >> 21) &0xFF;

  /* Pack Member: m_ByteClass2 */
  (pToData)[16] |= ((pFrom)->m_ByteClass2) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_ByteClass2 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_ByteClass2 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_ByteClass2 >> 24) &0xFF;
  (pToData)[20] |= ((pFrom)->m_ByteClass2 >> 32) & 0x07;

  /* Pack Member: m_PktClass1 */
  (pToData)[12] |= ((pFrom)->m_PktClass1 & 0x1f) <<3;
  (pToData)[13] |= ((pFrom)->m_PktClass1 >> 5) &0xFF;
  (pToData)[14] |= ((pFrom)->m_PktClass1 >> 13) &0xFF;
  (pToData)[15] |= ((pFrom)->m_PktClass1 >> 21) &0xFF;

  /* Pack Member: m_ByteClass1 */
  (pToData)[8] |= ((pFrom)->m_ByteClass1) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_ByteClass1 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_ByteClass1 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_ByteClass1 >> 24) &0xFF;
  (pToData)[12] |= ((pFrom)->m_ByteClass1 >> 32) & 0x07;

  /* Pack Member: m_PktClass0 */
  (pToData)[4] |= ((pFrom)->m_PktClass0 & 0x1f) <<3;
  (pToData)[5] |= ((pFrom)->m_PktClass0 >> 5) &0xFF;
  (pToData)[6] |= ((pFrom)->m_PktClass0 >> 13) &0xFF;
  (pToData)[7] |= ((pFrom)->m_PktClass0 >> 21) &0xFF;

  /* Pack Member: m_ByteClass0 */
  (pToData)[0] |= ((pFrom)->m_ByteClass0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_ByteClass0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_ByteClass0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_ByteClass0 >> 24) &0xFF;
  (pToData)[4] |= ((pFrom)->m_ByteClass0 >> 32) & 0x07;

  return SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibPCT_Unpack(sbZfSbQe2000ElibPCT_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_PktClass15 */
  COMPILER_64_SET((pToStruct)->m_PktClass15, 0,  (unsigned int) (pFromData)[124]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[125]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[126]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[127]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass15 */
  COMPILER_64_SET((pToStruct)->m_ByteClass15, 0,  (unsigned int) (pFromData)[120]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[121]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[122]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[123]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[124]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass14 */
  COMPILER_64_SET((pToStruct)->m_PktClass14, 0,  (unsigned int) (pFromData)[116]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[117]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[118]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[119]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass14 */
  COMPILER_64_SET((pToStruct)->m_ByteClass14, 0,  (unsigned int) (pFromData)[112]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[113]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[114]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[115]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[116]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass13 */
  COMPILER_64_SET((pToStruct)->m_PktClass13, 0,  (unsigned int) (pFromData)[108]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[109]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[110]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[111]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass13 */
  COMPILER_64_SET((pToStruct)->m_ByteClass13, 0,  (unsigned int) (pFromData)[104]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[105]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[106]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[107]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[108]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass12 */
  COMPILER_64_SET((pToStruct)->m_PktClass12, 0,  (unsigned int) (pFromData)[100]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[101]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[102]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[103]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass12 */
  COMPILER_64_SET((pToStruct)->m_ByteClass12, 0,  (unsigned int) (pFromData)[96]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[97]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[98]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[99]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[100]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass11 */
  COMPILER_64_SET((pToStruct)->m_PktClass11, 0,  (unsigned int) (pFromData)[92]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[93]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[94]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[95]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass11 */
  COMPILER_64_SET((pToStruct)->m_ByteClass11, 0,  (unsigned int) (pFromData)[88]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[89]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[90]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[91]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[92]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass10 */
  COMPILER_64_SET((pToStruct)->m_PktClass10, 0,  (unsigned int) (pFromData)[84]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[85]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[86]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[87]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass10 */
  COMPILER_64_SET((pToStruct)->m_ByteClass10, 0,  (unsigned int) (pFromData)[80]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[81]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[82]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[83]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[84]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass9 */
  COMPILER_64_SET((pToStruct)->m_PktClass9, 0,  (unsigned int) (pFromData)[76]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[77]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[78]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[79]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass9 */
  COMPILER_64_SET((pToStruct)->m_ByteClass9, 0,  (unsigned int) (pFromData)[72]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[73]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[74]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[75]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[76]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass8 */
  COMPILER_64_SET((pToStruct)->m_PktClass8, 0,  (unsigned int) (pFromData)[68]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[69]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[70]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[71]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass8 */
  COMPILER_64_SET((pToStruct)->m_ByteClass8, 0,  (unsigned int) (pFromData)[64]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[65]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[66]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[67]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[68]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass7 */
  COMPILER_64_SET((pToStruct)->m_PktClass7, 0,  (unsigned int) (pFromData)[60]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[61]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[62]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[63]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass7 */
  COMPILER_64_SET((pToStruct)->m_ByteClass7, 0,  (unsigned int) (pFromData)[56]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[57]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[58]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[59]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[60]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass6 */
  COMPILER_64_SET((pToStruct)->m_PktClass6, 0,  (unsigned int) (pFromData)[52]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[53]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[54]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[55]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass6 */
  COMPILER_64_SET((pToStruct)->m_ByteClass6, 0,  (unsigned int) (pFromData)[48]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[49]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[50]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[51]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[52]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass5 */
  COMPILER_64_SET((pToStruct)->m_PktClass5, 0,  (unsigned int) (pFromData)[44]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[45]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[46]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[47]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass5 */
  COMPILER_64_SET((pToStruct)->m_ByteClass5, 0,  (unsigned int) (pFromData)[40]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[41]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[42]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[43]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[44]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass4 */
  COMPILER_64_SET((pToStruct)->m_PktClass4, 0,  (unsigned int) (pFromData)[36]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[37]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[38]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[39]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass4 */
  COMPILER_64_SET((pToStruct)->m_ByteClass4, 0,  (unsigned int) (pFromData)[32]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[33]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[34]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[35]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[36]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass3 */
  COMPILER_64_SET((pToStruct)->m_PktClass3, 0,  (unsigned int) (pFromData)[28]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[29]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[30]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[31]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass3 */
  COMPILER_64_SET((pToStruct)->m_ByteClass3, 0,  (unsigned int) (pFromData)[24]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[25]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[26]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[27]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[28]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass2 */
  COMPILER_64_SET((pToStruct)->m_PktClass2, 0,  (unsigned int) (pFromData)[20]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass2 */
  COMPILER_64_SET((pToStruct)->m_ByteClass2, 0,  (unsigned int) (pFromData)[16]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass1 */
  COMPILER_64_SET((pToStruct)->m_PktClass1, 0,  (unsigned int) (pFromData)[12]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass1 */
  COMPILER_64_SET((pToStruct)->m_ByteClass1, 0,  (unsigned int) (pFromData)[8]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_PktClass0 */
  COMPILER_64_SET((pToStruct)->m_PktClass0, 0,  (unsigned int) (pFromData)[4]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_PktClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_ByteClass0 */
  COMPILER_64_SET((pToStruct)->m_ByteClass0, 0,  (unsigned int) (pFromData)[0]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_ByteClass0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibPCT_InitInstance(sbZfSbQe2000ElibPCT_t *pFrame) {

  pFrame->m_PktClass15 =  (unsigned int)  0;
  pFrame->m_ByteClass15 =  (uint64_t)  0;
  pFrame->m_PktClass14 =  (unsigned int)  0;
  pFrame->m_ByteClass14 =  (uint64_t)  0;
  pFrame->m_PktClass13 =  (unsigned int)  0;
  pFrame->m_ByteClass13 =  (uint64_t)  0;
  pFrame->m_PktClass12 =  (unsigned int)  0;
  pFrame->m_ByteClass12 =  (uint64_t)  0;
  pFrame->m_PktClass11 =  (unsigned int)  0;
  pFrame->m_ByteClass11 =  (uint64_t)  0;
  pFrame->m_PktClass10 =  (unsigned int)  0;
  pFrame->m_ByteClass10 =  (uint64_t)  0;
  pFrame->m_PktClass9 =  (unsigned int)  0;
  pFrame->m_ByteClass9 =  (uint64_t)  0;
  pFrame->m_PktClass8 =  (unsigned int)  0;
  pFrame->m_ByteClass8 =  (uint64_t)  0;
  pFrame->m_PktClass7 =  (unsigned int)  0;
  pFrame->m_ByteClass7 =  (uint64_t)  0;
  pFrame->m_PktClass6 =  (unsigned int)  0;
  pFrame->m_ByteClass6 =  (uint64_t)  0;
  pFrame->m_PktClass5 =  (unsigned int)  0;
  pFrame->m_ByteClass5 =  (uint64_t)  0;
  pFrame->m_PktClass4 =  (unsigned int)  0;
  pFrame->m_ByteClass4 =  (uint64_t)  0;
  pFrame->m_PktClass3 =  (unsigned int)  0;
  pFrame->m_ByteClass3 =  (uint64_t)  0;
  pFrame->m_PktClass2 =  (unsigned int)  0;
  pFrame->m_ByteClass2 =  (uint64_t)  0;
  pFrame->m_PktClass1 =  (unsigned int)  0;
  pFrame->m_ByteClass1 =  (uint64_t)  0;
  pFrame->m_PktClass0 =  (unsigned int)  0;
  pFrame->m_ByteClass0 =  (uint64_t)  0;

}
