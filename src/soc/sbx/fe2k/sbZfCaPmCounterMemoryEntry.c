/*
 * $Id: sbZfCaPmCounterMemoryEntry.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfCaPmCounterMemoryEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPmCounterMemoryEntry_Pack(sbZfCaPmCounterMemoryEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPMCOUNTERMEMORYENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uuByteCntr15 */
  (pToData)[123] |= ((pFrom)->m_uuByteCntr15 & 0x07) <<5;
  (pToData)[124] |= ((pFrom)->m_uuByteCntr15 >> 3) &0xFF;
  (pToData)[125] |= ((pFrom)->m_uuByteCntr15 >> 11) &0xFF;
  (pToData)[126] |= ((pFrom)->m_uuByteCntr15 >> 19) &0xFF;
  (pToData)[127] |= ((pFrom)->m_uuByteCntr15 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr15 */
  (pToData)[120] |= ((pFrom)->m_uPktCntr15) & 0xFF;
  (pToData)[121] |= ((pFrom)->m_uPktCntr15 >> 8) &0xFF;
  (pToData)[122] |= ((pFrom)->m_uPktCntr15 >> 16) &0xFF;
  (pToData)[123] |= ((pFrom)->m_uPktCntr15 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr14 */
  (pToData)[115] |= ((pFrom)->m_uuByteCntr14 & 0x07) <<5;
  (pToData)[116] |= ((pFrom)->m_uuByteCntr14 >> 3) &0xFF;
  (pToData)[117] |= ((pFrom)->m_uuByteCntr14 >> 11) &0xFF;
  (pToData)[118] |= ((pFrom)->m_uuByteCntr14 >> 19) &0xFF;
  (pToData)[119] |= ((pFrom)->m_uuByteCntr14 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr14 */
  (pToData)[112] |= ((pFrom)->m_uPktCntr14) & 0xFF;
  (pToData)[113] |= ((pFrom)->m_uPktCntr14 >> 8) &0xFF;
  (pToData)[114] |= ((pFrom)->m_uPktCntr14 >> 16) &0xFF;
  (pToData)[115] |= ((pFrom)->m_uPktCntr14 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr13 */
  (pToData)[107] |= ((pFrom)->m_uuByteCntr13 & 0x0f) <<4;
  (pToData)[108] |= ((pFrom)->m_uuByteCntr13 >> 4) &0xFF;
  (pToData)[109] |= ((pFrom)->m_uuByteCntr13 >> 12) &0xFF;
  (pToData)[110] |= ((pFrom)->m_uuByteCntr13 >> 20) &0xFF;
  (pToData)[111] |= ((pFrom)->m_uuByteCntr13 >> 28) &0xFF;

  /* Pack Member: m_uPktCntr13 */
  (pToData)[104] |= ((pFrom)->m_uPktCntr13) & 0xFF;
  (pToData)[105] |= ((pFrom)->m_uPktCntr13 >> 8) &0xFF;
  (pToData)[106] |= ((pFrom)->m_uPktCntr13 >> 16) &0xFF;
  (pToData)[107] |= ((pFrom)->m_uPktCntr13 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr12 */
  (pToData)[99] |= ((pFrom)->m_uuByteCntr12 & 0x07) <<5;
  (pToData)[100] |= ((pFrom)->m_uuByteCntr12 >> 3) &0xFF;
  (pToData)[101] |= ((pFrom)->m_uuByteCntr12 >> 11) &0xFF;
  (pToData)[102] |= ((pFrom)->m_uuByteCntr12 >> 19) &0xFF;
  (pToData)[103] |= ((pFrom)->m_uuByteCntr12 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr12 */
  (pToData)[96] |= ((pFrom)->m_uPktCntr12) & 0xFF;
  (pToData)[97] |= ((pFrom)->m_uPktCntr12 >> 8) &0xFF;
  (pToData)[98] |= ((pFrom)->m_uPktCntr12 >> 16) &0xFF;
  (pToData)[99] |= ((pFrom)->m_uPktCntr12 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr11 */
  (pToData)[91] |= ((pFrom)->m_uuByteCntr11 & 0x0f) <<4;
  (pToData)[92] |= ((pFrom)->m_uuByteCntr11 >> 4) &0xFF;
  (pToData)[93] |= ((pFrom)->m_uuByteCntr11 >> 12) &0xFF;
  (pToData)[94] |= ((pFrom)->m_uuByteCntr11 >> 20) &0xFF;
  (pToData)[95] |= ((pFrom)->m_uuByteCntr11 >> 28) &0xFF;

  /* Pack Member: m_uPktCntr11 */
  (pToData)[88] |= ((pFrom)->m_uPktCntr11) & 0xFF;
  (pToData)[89] |= ((pFrom)->m_uPktCntr11 >> 8) &0xFF;
  (pToData)[90] |= ((pFrom)->m_uPktCntr11 >> 16) &0xFF;
  (pToData)[91] |= ((pFrom)->m_uPktCntr11 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr10 */
  (pToData)[83] |= ((pFrom)->m_uuByteCntr10 & 0x07) <<5;
  (pToData)[84] |= ((pFrom)->m_uuByteCntr10 >> 3) &0xFF;
  (pToData)[85] |= ((pFrom)->m_uuByteCntr10 >> 11) &0xFF;
  (pToData)[86] |= ((pFrom)->m_uuByteCntr10 >> 19) &0xFF;
  (pToData)[87] |= ((pFrom)->m_uuByteCntr10 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr10 */
  (pToData)[80] |= ((pFrom)->m_uPktCntr10) & 0xFF;
  (pToData)[81] |= ((pFrom)->m_uPktCntr10 >> 8) &0xFF;
  (pToData)[82] |= ((pFrom)->m_uPktCntr10 >> 16) &0xFF;
  (pToData)[83] |= ((pFrom)->m_uPktCntr10 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr9 */
  (pToData)[75] |= ((pFrom)->m_uuByteCntr9 & 0x07) <<5;
  (pToData)[76] |= ((pFrom)->m_uuByteCntr9 >> 3) &0xFF;
  (pToData)[77] |= ((pFrom)->m_uuByteCntr9 >> 11) &0xFF;
  (pToData)[78] |= ((pFrom)->m_uuByteCntr9 >> 19) &0xFF;
  (pToData)[79] |= ((pFrom)->m_uuByteCntr9 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr9 */
  (pToData)[72] |= ((pFrom)->m_uPktCntr9) & 0xFF;
  (pToData)[73] |= ((pFrom)->m_uPktCntr9 >> 8) &0xFF;
  (pToData)[74] |= ((pFrom)->m_uPktCntr9 >> 16) &0xFF;
  (pToData)[75] |= ((pFrom)->m_uPktCntr9 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr8 */
  (pToData)[67] |= ((pFrom)->m_uuByteCntr8 & 0x0f) <<4;
  (pToData)[68] |= ((pFrom)->m_uuByteCntr8 >> 4) &0xFF;
  (pToData)[69] |= ((pFrom)->m_uuByteCntr8 >> 12) &0xFF;
  (pToData)[70] |= ((pFrom)->m_uuByteCntr8 >> 20) &0xFF;
  (pToData)[71] |= ((pFrom)->m_uuByteCntr8 >> 28) &0xFF;

  /* Pack Member: m_uPktCntr8 */
  (pToData)[64] |= ((pFrom)->m_uPktCntr8) & 0xFF;
  (pToData)[65] |= ((pFrom)->m_uPktCntr8 >> 8) &0xFF;
  (pToData)[66] |= ((pFrom)->m_uPktCntr8 >> 16) &0xFF;
  (pToData)[67] |= ((pFrom)->m_uPktCntr8 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr7 */
  (pToData)[59] |= ((pFrom)->m_uuByteCntr7 & 0x07) <<5;
  (pToData)[60] |= ((pFrom)->m_uuByteCntr7 >> 3) &0xFF;
  (pToData)[61] |= ((pFrom)->m_uuByteCntr7 >> 11) &0xFF;
  (pToData)[62] |= ((pFrom)->m_uuByteCntr7 >> 19) &0xFF;
  (pToData)[63] |= ((pFrom)->m_uuByteCntr7 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr7 */
  (pToData)[56] |= ((pFrom)->m_uPktCntr7) & 0xFF;
  (pToData)[57] |= ((pFrom)->m_uPktCntr7 >> 8) &0xFF;
  (pToData)[58] |= ((pFrom)->m_uPktCntr7 >> 16) &0xFF;
  (pToData)[59] |= ((pFrom)->m_uPktCntr7 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr6 */
  (pToData)[51] |= ((pFrom)->m_uuByteCntr6 & 0x07) <<5;
  (pToData)[52] |= ((pFrom)->m_uuByteCntr6 >> 3) &0xFF;
  (pToData)[53] |= ((pFrom)->m_uuByteCntr6 >> 11) &0xFF;
  (pToData)[54] |= ((pFrom)->m_uuByteCntr6 >> 19) &0xFF;
  (pToData)[55] |= ((pFrom)->m_uuByteCntr6 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr6 */
  (pToData)[48] |= ((pFrom)->m_uPktCntr6) & 0xFF;
  (pToData)[49] |= ((pFrom)->m_uPktCntr6 >> 8) &0xFF;
  (pToData)[50] |= ((pFrom)->m_uPktCntr6 >> 16) &0xFF;
  (pToData)[51] |= ((pFrom)->m_uPktCntr6 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr5 */
  (pToData)[43] |= ((pFrom)->m_uuByteCntr5 & 0x07) <<5;
  (pToData)[44] |= ((pFrom)->m_uuByteCntr5 >> 3) &0xFF;
  (pToData)[45] |= ((pFrom)->m_uuByteCntr5 >> 11) &0xFF;
  (pToData)[46] |= ((pFrom)->m_uuByteCntr5 >> 19) &0xFF;
  (pToData)[47] |= ((pFrom)->m_uuByteCntr5 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr5 */
  (pToData)[40] |= ((pFrom)->m_uPktCntr5) & 0xFF;
  (pToData)[41] |= ((pFrom)->m_uPktCntr5 >> 8) &0xFF;
  (pToData)[42] |= ((pFrom)->m_uPktCntr5 >> 16) &0xFF;
  (pToData)[43] |= ((pFrom)->m_uPktCntr5 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr4 */
  (pToData)[35] |= ((pFrom)->m_uuByteCntr4 & 0x07) <<5;
  (pToData)[36] |= ((pFrom)->m_uuByteCntr4 >> 3) &0xFF;
  (pToData)[37] |= ((pFrom)->m_uuByteCntr4 >> 11) &0xFF;
  (pToData)[38] |= ((pFrom)->m_uuByteCntr4 >> 19) &0xFF;
  (pToData)[39] |= ((pFrom)->m_uuByteCntr4 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr4 */
  (pToData)[32] |= ((pFrom)->m_uPktCntr4) & 0xFF;
  (pToData)[33] |= ((pFrom)->m_uPktCntr4 >> 8) &0xFF;
  (pToData)[34] |= ((pFrom)->m_uPktCntr4 >> 16) &0xFF;
  (pToData)[35] |= ((pFrom)->m_uPktCntr4 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr3 */
  (pToData)[27] |= ((pFrom)->m_uuByteCntr3 & 0x07) <<5;
  (pToData)[28] |= ((pFrom)->m_uuByteCntr3 >> 3) &0xFF;
  (pToData)[29] |= ((pFrom)->m_uuByteCntr3 >> 11) &0xFF;
  (pToData)[30] |= ((pFrom)->m_uuByteCntr3 >> 19) &0xFF;
  (pToData)[31] |= ((pFrom)->m_uuByteCntr3 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr3 */
  (pToData)[24] |= ((pFrom)->m_uPktCntr3) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_uPktCntr3 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_uPktCntr3 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_uPktCntr3 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr2 */
  (pToData)[19] |= ((pFrom)->m_uuByteCntr2 & 0x07) <<5;
  (pToData)[20] |= ((pFrom)->m_uuByteCntr2 >> 3) &0xFF;
  (pToData)[21] |= ((pFrom)->m_uuByteCntr2 >> 11) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uuByteCntr2 >> 19) &0xFF;
  (pToData)[23] |= ((pFrom)->m_uuByteCntr2 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr2 */
  (pToData)[16] |= ((pFrom)->m_uPktCntr2) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_uPktCntr2 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uPktCntr2 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uPktCntr2 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr1 */
  (pToData)[11] |= ((pFrom)->m_uuByteCntr1 & 0x07) <<5;
  (pToData)[12] |= ((pFrom)->m_uuByteCntr1 >> 3) &0xFF;
  (pToData)[13] |= ((pFrom)->m_uuByteCntr1 >> 11) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uuByteCntr1 >> 19) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uuByteCntr1 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr1 */
  (pToData)[8] |= ((pFrom)->m_uPktCntr1) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_uPktCntr1 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uPktCntr1 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uPktCntr1 >> 24) & 0x1f;

  /* Pack Member: m_uuByteCntr0 */
  (pToData)[3] |= ((pFrom)->m_uuByteCntr0 & 0x07) <<5;
  (pToData)[4] |= ((pFrom)->m_uuByteCntr0 >> 3) &0xFF;
  (pToData)[5] |= ((pFrom)->m_uuByteCntr0 >> 11) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uuByteCntr0 >> 19) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uuByteCntr0 >> 27) &0xFF;

  /* Pack Member: m_uPktCntr0 */
  (pToData)[0] |= ((pFrom)->m_uPktCntr0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uPktCntr0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uPktCntr0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uPktCntr0 >> 24) & 0x1f;

  return SB_ZF_CAPMCOUNTERMEMORYENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPmCounterMemoryEntry_Unpack(sbZfCaPmCounterMemoryEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uuByteCntr15 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr15, 0,  (unsigned int) (pFromData)[123]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[124]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[125]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[126]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr15;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[127]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr15 */
  (pToStruct)->m_uPktCntr15 =  (uint32_t)  (pFromData)[120] ;
  (pToStruct)->m_uPktCntr15 |=  (uint32_t)  (pFromData)[121] << 8;
  (pToStruct)->m_uPktCntr15 |=  (uint32_t)  (pFromData)[122] << 16;
  (pToStruct)->m_uPktCntr15 |=  (uint32_t)  ((pFromData)[123] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr14 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr14, 0,  (unsigned int) (pFromData)[115]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[116]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[117]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[118]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr14;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[119]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr14 */
  (pToStruct)->m_uPktCntr14 =  (uint32_t)  (pFromData)[112] ;
  (pToStruct)->m_uPktCntr14 |=  (uint32_t)  (pFromData)[113] << 8;
  (pToStruct)->m_uPktCntr14 |=  (uint32_t)  (pFromData)[114] << 16;
  (pToStruct)->m_uPktCntr14 |=  (uint32_t)  ((pFromData)[115] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr13 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr13, 0,  (unsigned int) (pFromData)[107]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[108]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[109]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[110]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr13;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[111]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr13 */
  (pToStruct)->m_uPktCntr13 =  (uint32_t)  (pFromData)[104] ;
  (pToStruct)->m_uPktCntr13 |=  (uint32_t)  (pFromData)[105] << 8;
  (pToStruct)->m_uPktCntr13 |=  (uint32_t)  (pFromData)[106] << 16;
  (pToStruct)->m_uPktCntr13 |=  (uint32_t)  ((pFromData)[107] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr12 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr12, 0,  (unsigned int) (pFromData)[99]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[100]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[101]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[102]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr12;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[103]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr12 */
  (pToStruct)->m_uPktCntr12 =  (uint32_t)  (pFromData)[96] ;
  (pToStruct)->m_uPktCntr12 |=  (uint32_t)  (pFromData)[97] << 8;
  (pToStruct)->m_uPktCntr12 |=  (uint32_t)  (pFromData)[98] << 16;
  (pToStruct)->m_uPktCntr12 |=  (uint32_t)  ((pFromData)[99] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr11 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr11, 0,  (unsigned int) (pFromData)[91]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[92]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[93]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[94]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr11;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[95]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr11 */
  (pToStruct)->m_uPktCntr11 =  (uint32_t)  (pFromData)[88] ;
  (pToStruct)->m_uPktCntr11 |=  (uint32_t)  (pFromData)[89] << 8;
  (pToStruct)->m_uPktCntr11 |=  (uint32_t)  (pFromData)[90] << 16;
  (pToStruct)->m_uPktCntr11 |=  (uint32_t)  ((pFromData)[91] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr10 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr10, 0,  (unsigned int) (pFromData)[83]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[84]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[85]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[86]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr10;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[87]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr10 */
  (pToStruct)->m_uPktCntr10 =  (uint32_t)  (pFromData)[80] ;
  (pToStruct)->m_uPktCntr10 |=  (uint32_t)  (pFromData)[81] << 8;
  (pToStruct)->m_uPktCntr10 |=  (uint32_t)  (pFromData)[82] << 16;
  (pToStruct)->m_uPktCntr10 |=  (uint32_t)  ((pFromData)[83] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr9 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr9, 0,  (unsigned int) (pFromData)[75]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[76]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[77]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[78]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr9;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[79]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr9 */
  (pToStruct)->m_uPktCntr9 =  (uint32_t)  (pFromData)[72] ;
  (pToStruct)->m_uPktCntr9 |=  (uint32_t)  (pFromData)[73] << 8;
  (pToStruct)->m_uPktCntr9 |=  (uint32_t)  (pFromData)[74] << 16;
  (pToStruct)->m_uPktCntr9 |=  (uint32_t)  ((pFromData)[75] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr8 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr8, 0,  (unsigned int) (pFromData)[67]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[68]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[69]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[70]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr8;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[71]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr8 */
  (pToStruct)->m_uPktCntr8 =  (uint32_t)  (pFromData)[64] ;
  (pToStruct)->m_uPktCntr8 |=  (uint32_t)  (pFromData)[65] << 8;
  (pToStruct)->m_uPktCntr8 |=  (uint32_t)  (pFromData)[66] << 16;
  (pToStruct)->m_uPktCntr8 |=  (uint32_t)  ((pFromData)[67] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr7 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr7, 0,  (unsigned int) (pFromData)[59]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[60]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[61]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[62]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr7;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[63]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr7 */
  (pToStruct)->m_uPktCntr7 =  (uint32_t)  (pFromData)[56] ;
  (pToStruct)->m_uPktCntr7 |=  (uint32_t)  (pFromData)[57] << 8;
  (pToStruct)->m_uPktCntr7 |=  (uint32_t)  (pFromData)[58] << 16;
  (pToStruct)->m_uPktCntr7 |=  (uint32_t)  ((pFromData)[59] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr6 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr6, 0,  (unsigned int) (pFromData)[51]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[52]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[53]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[54]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr6;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[55]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr6 */
  (pToStruct)->m_uPktCntr6 =  (uint32_t)  (pFromData)[48] ;
  (pToStruct)->m_uPktCntr6 |=  (uint32_t)  (pFromData)[49] << 8;
  (pToStruct)->m_uPktCntr6 |=  (uint32_t)  (pFromData)[50] << 16;
  (pToStruct)->m_uPktCntr6 |=  (uint32_t)  ((pFromData)[51] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr5 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr5, 0,  (unsigned int) (pFromData)[43]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[44]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[45]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[46]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr5;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[47]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr5 */
  (pToStruct)->m_uPktCntr5 =  (uint32_t)  (pFromData)[40] ;
  (pToStruct)->m_uPktCntr5 |=  (uint32_t)  (pFromData)[41] << 8;
  (pToStruct)->m_uPktCntr5 |=  (uint32_t)  (pFromData)[42] << 16;
  (pToStruct)->m_uPktCntr5 |=  (uint32_t)  ((pFromData)[43] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr4 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr4, 0,  (unsigned int) (pFromData)[35]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[36]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[37]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[38]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr4;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[39]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr4 */
  (pToStruct)->m_uPktCntr4 =  (uint32_t)  (pFromData)[32] ;
  (pToStruct)->m_uPktCntr4 |=  (uint32_t)  (pFromData)[33] << 8;
  (pToStruct)->m_uPktCntr4 |=  (uint32_t)  (pFromData)[34] << 16;
  (pToStruct)->m_uPktCntr4 |=  (uint32_t)  ((pFromData)[35] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr3 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr3, 0,  (unsigned int) (pFromData)[27]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[28]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[29]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[30]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[31]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr3 */
  (pToStruct)->m_uPktCntr3 =  (uint32_t)  (pFromData)[24] ;
  (pToStruct)->m_uPktCntr3 |=  (uint32_t)  (pFromData)[25] << 8;
  (pToStruct)->m_uPktCntr3 |=  (uint32_t)  (pFromData)[26] << 16;
  (pToStruct)->m_uPktCntr3 |=  (uint32_t)  ((pFromData)[27] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr2 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr2, 0,  (unsigned int) (pFromData)[19]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr2 */
  (pToStruct)->m_uPktCntr2 =  (uint32_t)  (pFromData)[16] ;
  (pToStruct)->m_uPktCntr2 |=  (uint32_t)  (pFromData)[17] << 8;
  (pToStruct)->m_uPktCntr2 |=  (uint32_t)  (pFromData)[18] << 16;
  (pToStruct)->m_uPktCntr2 |=  (uint32_t)  ((pFromData)[19] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr1 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr1, 0,  (unsigned int) (pFromData)[11]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr1 */
  (pToStruct)->m_uPktCntr1 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_uPktCntr1 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_uPktCntr1 |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_uPktCntr1 |=  (uint32_t)  ((pFromData)[11] & 0x1f) << 24;

  /* Unpack Member: m_uuByteCntr0 */
  COMPILER_64_SET((pToStruct)->m_uuByteCntr0, 0,  (unsigned int) (pFromData)[3]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuByteCntr0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uPktCntr0 */
  (pToStruct)->m_uPktCntr0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uPktCntr0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_uPktCntr0 |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_uPktCntr0 |=  (uint32_t)  ((pFromData)[3] & 0x1f) << 24;

}



/* initialize an instance of this zframe */
void
sbZfCaPmCounterMemoryEntry_InitInstance(sbZfCaPmCounterMemoryEntry_t *pFrame) {

  pFrame->m_uuByteCntr15 =  (uint64_t)  0;
  pFrame->m_uPktCntr15 =  (unsigned int)  0;
  pFrame->m_uuByteCntr14 =  (uint64_t)  0;
  pFrame->m_uPktCntr14 =  (unsigned int)  0;
  pFrame->m_uuByteCntr13 =  (uint64_t)  0;
  pFrame->m_uPktCntr13 =  (unsigned int)  0;
  pFrame->m_uuByteCntr12 =  (uint64_t)  0;
  pFrame->m_uPktCntr12 =  (unsigned int)  0;
  pFrame->m_uuByteCntr11 =  (uint64_t)  0;
  pFrame->m_uPktCntr11 =  (unsigned int)  0;
  pFrame->m_uuByteCntr10 =  (uint64_t)  0;
  pFrame->m_uPktCntr10 =  (unsigned int)  0;
  pFrame->m_uuByteCntr9 =  (uint64_t)  0;
  pFrame->m_uPktCntr9 =  (unsigned int)  0;
  pFrame->m_uuByteCntr8 =  (uint64_t)  0;
  pFrame->m_uPktCntr8 =  (unsigned int)  0;
  pFrame->m_uuByteCntr7 =  (uint64_t)  0;
  pFrame->m_uPktCntr7 =  (unsigned int)  0;
  pFrame->m_uuByteCntr6 =  (uint64_t)  0;
  pFrame->m_uPktCntr6 =  (unsigned int)  0;
  pFrame->m_uuByteCntr5 =  (uint64_t)  0;
  pFrame->m_uPktCntr5 =  (unsigned int)  0;
  pFrame->m_uuByteCntr4 =  (uint64_t)  0;
  pFrame->m_uPktCntr4 =  (unsigned int)  0;
  pFrame->m_uuByteCntr3 =  (uint64_t)  0;
  pFrame->m_uPktCntr3 =  (unsigned int)  0;
  pFrame->m_uuByteCntr2 =  (uint64_t)  0;
  pFrame->m_uPktCntr2 =  (unsigned int)  0;
  pFrame->m_uuByteCntr1 =  (uint64_t)  0;
  pFrame->m_uPktCntr1 =  (unsigned int)  0;
  pFrame->m_uuByteCntr0 =  (uint64_t)  0;
  pFrame->m_uPktCntr0 =  (unsigned int)  0;

}
