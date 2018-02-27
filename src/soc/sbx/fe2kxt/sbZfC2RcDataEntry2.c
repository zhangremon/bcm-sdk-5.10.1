/*
 * $Id: sbZfC2RcDataEntry2.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfC2RcDataEntry2.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2RcDataEntry2_Pack(sbZfC2RcDataEntry2_t *pFrom,
                        uint8_t *pToData,
                        uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2RCDATAENTRY2_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uData39 */
  (pToData)[156] |= ((pFrom)->m_uData39) & 0xFF;
  (pToData)[157] |= ((pFrom)->m_uData39 >> 8) &0xFF;
  (pToData)[158] |= ((pFrom)->m_uData39 >> 16) &0xFF;
  (pToData)[159] |= ((pFrom)->m_uData39 >> 24) &0xFF;

  /* Pack Member: m_uData38 */
  (pToData)[152] |= ((pFrom)->m_uData38) & 0xFF;
  (pToData)[153] |= ((pFrom)->m_uData38 >> 8) &0xFF;
  (pToData)[154] |= ((pFrom)->m_uData38 >> 16) &0xFF;
  (pToData)[155] |= ((pFrom)->m_uData38 >> 24) &0xFF;

  /* Pack Member: m_uData37 */
  (pToData)[148] |= ((pFrom)->m_uData37) & 0xFF;
  (pToData)[149] |= ((pFrom)->m_uData37 >> 8) &0xFF;
  (pToData)[150] |= ((pFrom)->m_uData37 >> 16) &0xFF;
  (pToData)[151] |= ((pFrom)->m_uData37 >> 24) &0xFF;

  /* Pack Member: m_uData36 */
  (pToData)[144] |= ((pFrom)->m_uData36) & 0xFF;
  (pToData)[145] |= ((pFrom)->m_uData36 >> 8) &0xFF;
  (pToData)[146] |= ((pFrom)->m_uData36 >> 16) &0xFF;
  (pToData)[147] |= ((pFrom)->m_uData36 >> 24) &0xFF;

  /* Pack Member: m_uData35 */
  (pToData)[140] |= ((pFrom)->m_uData35) & 0xFF;
  (pToData)[141] |= ((pFrom)->m_uData35 >> 8) &0xFF;
  (pToData)[142] |= ((pFrom)->m_uData35 >> 16) &0xFF;
  (pToData)[143] |= ((pFrom)->m_uData35 >> 24) &0xFF;

  /* Pack Member: m_uData34 */
  (pToData)[136] |= ((pFrom)->m_uData34) & 0xFF;
  (pToData)[137] |= ((pFrom)->m_uData34 >> 8) &0xFF;
  (pToData)[138] |= ((pFrom)->m_uData34 >> 16) &0xFF;
  (pToData)[139] |= ((pFrom)->m_uData34 >> 24) &0xFF;

  /* Pack Member: m_uData33 */
  (pToData)[132] |= ((pFrom)->m_uData33) & 0xFF;
  (pToData)[133] |= ((pFrom)->m_uData33 >> 8) &0xFF;
  (pToData)[134] |= ((pFrom)->m_uData33 >> 16) &0xFF;
  (pToData)[135] |= ((pFrom)->m_uData33 >> 24) &0xFF;

  /* Pack Member: m_uData32 */
  (pToData)[128] |= ((pFrom)->m_uData32) & 0xFF;
  (pToData)[129] |= ((pFrom)->m_uData32 >> 8) &0xFF;
  (pToData)[130] |= ((pFrom)->m_uData32 >> 16) &0xFF;
  (pToData)[131] |= ((pFrom)->m_uData32 >> 24) &0xFF;

  /* Pack Member: m_uData31 */
  (pToData)[124] |= ((pFrom)->m_uData31) & 0xFF;
  (pToData)[125] |= ((pFrom)->m_uData31 >> 8) &0xFF;
  (pToData)[126] |= ((pFrom)->m_uData31 >> 16) &0xFF;
  (pToData)[127] |= ((pFrom)->m_uData31 >> 24) &0xFF;

  /* Pack Member: m_uData30 */
  (pToData)[120] |= ((pFrom)->m_uData30) & 0xFF;
  (pToData)[121] |= ((pFrom)->m_uData30 >> 8) &0xFF;
  (pToData)[122] |= ((pFrom)->m_uData30 >> 16) &0xFF;
  (pToData)[123] |= ((pFrom)->m_uData30 >> 24) &0xFF;

  /* Pack Member: m_uData29 */
  (pToData)[116] |= ((pFrom)->m_uData29) & 0xFF;
  (pToData)[117] |= ((pFrom)->m_uData29 >> 8) &0xFF;
  (pToData)[118] |= ((pFrom)->m_uData29 >> 16) &0xFF;
  (pToData)[119] |= ((pFrom)->m_uData29 >> 24) &0xFF;

  /* Pack Member: m_uData28 */
  (pToData)[112] |= ((pFrom)->m_uData28) & 0xFF;
  (pToData)[113] |= ((pFrom)->m_uData28 >> 8) &0xFF;
  (pToData)[114] |= ((pFrom)->m_uData28 >> 16) &0xFF;
  (pToData)[115] |= ((pFrom)->m_uData28 >> 24) &0xFF;

  /* Pack Member: m_uData27 */
  (pToData)[108] |= ((pFrom)->m_uData27) & 0xFF;
  (pToData)[109] |= ((pFrom)->m_uData27 >> 8) &0xFF;
  (pToData)[110] |= ((pFrom)->m_uData27 >> 16) &0xFF;
  (pToData)[111] |= ((pFrom)->m_uData27 >> 24) &0xFF;

  /* Pack Member: m_uData26 */
  (pToData)[104] |= ((pFrom)->m_uData26) & 0xFF;
  (pToData)[105] |= ((pFrom)->m_uData26 >> 8) &0xFF;
  (pToData)[106] |= ((pFrom)->m_uData26 >> 16) &0xFF;
  (pToData)[107] |= ((pFrom)->m_uData26 >> 24) &0xFF;

  /* Pack Member: m_uData25 */
  (pToData)[100] |= ((pFrom)->m_uData25) & 0xFF;
  (pToData)[101] |= ((pFrom)->m_uData25 >> 8) &0xFF;
  (pToData)[102] |= ((pFrom)->m_uData25 >> 16) &0xFF;
  (pToData)[103] |= ((pFrom)->m_uData25 >> 24) &0xFF;

  /* Pack Member: m_uData24 */
  (pToData)[96] |= ((pFrom)->m_uData24) & 0xFF;
  (pToData)[97] |= ((pFrom)->m_uData24 >> 8) &0xFF;
  (pToData)[98] |= ((pFrom)->m_uData24 >> 16) &0xFF;
  (pToData)[99] |= ((pFrom)->m_uData24 >> 24) &0xFF;

  /* Pack Member: m_uData23 */
  (pToData)[92] |= ((pFrom)->m_uData23) & 0xFF;
  (pToData)[93] |= ((pFrom)->m_uData23 >> 8) &0xFF;
  (pToData)[94] |= ((pFrom)->m_uData23 >> 16) &0xFF;
  (pToData)[95] |= ((pFrom)->m_uData23 >> 24) &0xFF;

  /* Pack Member: m_uData22 */
  (pToData)[88] |= ((pFrom)->m_uData22) & 0xFF;
  (pToData)[89] |= ((pFrom)->m_uData22 >> 8) &0xFF;
  (pToData)[90] |= ((pFrom)->m_uData22 >> 16) &0xFF;
  (pToData)[91] |= ((pFrom)->m_uData22 >> 24) &0xFF;

  /* Pack Member: m_uData21 */
  (pToData)[84] |= ((pFrom)->m_uData21) & 0xFF;
  (pToData)[85] |= ((pFrom)->m_uData21 >> 8) &0xFF;
  (pToData)[86] |= ((pFrom)->m_uData21 >> 16) &0xFF;
  (pToData)[87] |= ((pFrom)->m_uData21 >> 24) &0xFF;

  /* Pack Member: m_uData20 */
  (pToData)[80] |= ((pFrom)->m_uData20) & 0xFF;
  (pToData)[81] |= ((pFrom)->m_uData20 >> 8) &0xFF;
  (pToData)[82] |= ((pFrom)->m_uData20 >> 16) &0xFF;
  (pToData)[83] |= ((pFrom)->m_uData20 >> 24) &0xFF;

  /* Pack Member: m_uData19 */
  (pToData)[76] |= ((pFrom)->m_uData19) & 0xFF;
  (pToData)[77] |= ((pFrom)->m_uData19 >> 8) &0xFF;
  (pToData)[78] |= ((pFrom)->m_uData19 >> 16) &0xFF;
  (pToData)[79] |= ((pFrom)->m_uData19 >> 24) &0xFF;

  /* Pack Member: m_uData18 */
  (pToData)[72] |= ((pFrom)->m_uData18) & 0xFF;
  (pToData)[73] |= ((pFrom)->m_uData18 >> 8) &0xFF;
  (pToData)[74] |= ((pFrom)->m_uData18 >> 16) &0xFF;
  (pToData)[75] |= ((pFrom)->m_uData18 >> 24) &0xFF;

  /* Pack Member: m_uData17 */
  (pToData)[68] |= ((pFrom)->m_uData17) & 0xFF;
  (pToData)[69] |= ((pFrom)->m_uData17 >> 8) &0xFF;
  (pToData)[70] |= ((pFrom)->m_uData17 >> 16) &0xFF;
  (pToData)[71] |= ((pFrom)->m_uData17 >> 24) &0xFF;

  /* Pack Member: m_uData16 */
  (pToData)[64] |= ((pFrom)->m_uData16) & 0xFF;
  (pToData)[65] |= ((pFrom)->m_uData16 >> 8) &0xFF;
  (pToData)[66] |= ((pFrom)->m_uData16 >> 16) &0xFF;
  (pToData)[67] |= ((pFrom)->m_uData16 >> 24) &0xFF;

  /* Pack Member: m_uData15 */
  (pToData)[60] |= ((pFrom)->m_uData15) & 0xFF;
  (pToData)[61] |= ((pFrom)->m_uData15 >> 8) &0xFF;
  (pToData)[62] |= ((pFrom)->m_uData15 >> 16) &0xFF;
  (pToData)[63] |= ((pFrom)->m_uData15 >> 24) &0xFF;

  /* Pack Member: m_uData14 */
  (pToData)[56] |= ((pFrom)->m_uData14) & 0xFF;
  (pToData)[57] |= ((pFrom)->m_uData14 >> 8) &0xFF;
  (pToData)[58] |= ((pFrom)->m_uData14 >> 16) &0xFF;
  (pToData)[59] |= ((pFrom)->m_uData14 >> 24) &0xFF;

  /* Pack Member: m_uData13 */
  (pToData)[52] |= ((pFrom)->m_uData13) & 0xFF;
  (pToData)[53] |= ((pFrom)->m_uData13 >> 8) &0xFF;
  (pToData)[54] |= ((pFrom)->m_uData13 >> 16) &0xFF;
  (pToData)[55] |= ((pFrom)->m_uData13 >> 24) &0xFF;

  /* Pack Member: m_uData12 */
  (pToData)[48] |= ((pFrom)->m_uData12) & 0xFF;
  (pToData)[49] |= ((pFrom)->m_uData12 >> 8) &0xFF;
  (pToData)[50] |= ((pFrom)->m_uData12 >> 16) &0xFF;
  (pToData)[51] |= ((pFrom)->m_uData12 >> 24) &0xFF;

  /* Pack Member: m_uData11 */
  (pToData)[44] |= ((pFrom)->m_uData11) & 0xFF;
  (pToData)[45] |= ((pFrom)->m_uData11 >> 8) &0xFF;
  (pToData)[46] |= ((pFrom)->m_uData11 >> 16) &0xFF;
  (pToData)[47] |= ((pFrom)->m_uData11 >> 24) &0xFF;

  /* Pack Member: m_uData10 */
  (pToData)[40] |= ((pFrom)->m_uData10) & 0xFF;
  (pToData)[41] |= ((pFrom)->m_uData10 >> 8) &0xFF;
  (pToData)[42] |= ((pFrom)->m_uData10 >> 16) &0xFF;
  (pToData)[43] |= ((pFrom)->m_uData10 >> 24) &0xFF;

  /* Pack Member: m_uData9 */
  (pToData)[36] |= ((pFrom)->m_uData9) & 0xFF;
  (pToData)[37] |= ((pFrom)->m_uData9 >> 8) &0xFF;
  (pToData)[38] |= ((pFrom)->m_uData9 >> 16) &0xFF;
  (pToData)[39] |= ((pFrom)->m_uData9 >> 24) &0xFF;

  /* Pack Member: m_uData8 */
  (pToData)[32] |= ((pFrom)->m_uData8) & 0xFF;
  (pToData)[33] |= ((pFrom)->m_uData8 >> 8) &0xFF;
  (pToData)[34] |= ((pFrom)->m_uData8 >> 16) &0xFF;
  (pToData)[35] |= ((pFrom)->m_uData8 >> 24) &0xFF;

  /* Pack Member: m_uSpare7 */
  (pToData)[30] |= ((pFrom)->m_uSpare7 & 0x07) <<5;
  (pToData)[31] |= ((pFrom)->m_uSpare7 >> 3) &0xFF;

  /* Pack Member: m_uData7 */
  (pToData)[28] |= ((pFrom)->m_uData7) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_uData7 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_uData7 >> 16) & 0x1f;

  /* Pack Member: m_uSpare6 */
  (pToData)[26] |= ((pFrom)->m_uSpare6 & 0x07) <<5;
  (pToData)[27] |= ((pFrom)->m_uSpare6 >> 3) &0xFF;

  /* Pack Member: m_uData6 */
  (pToData)[24] |= ((pFrom)->m_uData6) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_uData6 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_uData6 >> 16) & 0x1f;

  /* Pack Member: m_uSpare5 */
  (pToData)[22] |= ((pFrom)->m_uSpare5 & 0x07) <<5;
  (pToData)[23] |= ((pFrom)->m_uSpare5 >> 3) &0xFF;

  /* Pack Member: m_uData5 */
  (pToData)[20] |= ((pFrom)->m_uData5) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_uData5 >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uData5 >> 16) & 0x1f;

  /* Pack Member: m_uSpare4 */
  (pToData)[18] |= ((pFrom)->m_uSpare4 & 0x07) <<5;
  (pToData)[19] |= ((pFrom)->m_uSpare4 >> 3) &0xFF;

  /* Pack Member: m_uData4 */
  (pToData)[16] |= ((pFrom)->m_uData4) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_uData4 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uData4 >> 16) & 0x1f;

  /* Pack Member: m_uSpare3 */
  (pToData)[14] |= ((pFrom)->m_uSpare3 & 0x07) <<5;
  (pToData)[15] |= ((pFrom)->m_uSpare3 >> 3) &0xFF;

  /* Pack Member: m_uData3 */
  (pToData)[12] |= ((pFrom)->m_uData3) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_uData3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uData3 >> 16) & 0x1f;

  /* Pack Member: m_uSpare2 */
  (pToData)[10] |= ((pFrom)->m_uSpare2 & 0x07) <<5;
  (pToData)[11] |= ((pFrom)->m_uSpare2 >> 3) &0xFF;

  /* Pack Member: m_uData2 */
  (pToData)[8] |= ((pFrom)->m_uData2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_uData2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uData2 >> 16) & 0x1f;

  /* Pack Member: m_uSpare1 */
  (pToData)[6] |= ((pFrom)->m_uSpare1 & 0x07) <<5;
  (pToData)[7] |= ((pFrom)->m_uSpare1 >> 3) &0xFF;

  /* Pack Member: m_uData1 */
  (pToData)[4] |= ((pFrom)->m_uData1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uData1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uData1 >> 16) & 0x1f;

  /* Pack Member: m_uSpare0 */
  (pToData)[2] |= ((pFrom)->m_uSpare0 & 0x07) <<5;
  (pToData)[3] |= ((pFrom)->m_uSpare0 >> 3) &0xFF;

  /* Pack Member: m_uData0 */
  (pToData)[0] |= ((pFrom)->m_uData0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uData0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uData0 >> 16) & 0x1f;

  return SB_ZF_C2RCDATAENTRY2_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2RcDataEntry2_Unpack(sbZfC2RcDataEntry2_t *pToStruct,
                          uint8_t *pFromData,
                          uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uData39 */
  (pToStruct)->m_uData39 =  (uint32_t)  (pFromData)[156] ;
  (pToStruct)->m_uData39 |=  (uint32_t)  (pFromData)[157] << 8;
  (pToStruct)->m_uData39 |=  (uint32_t)  (pFromData)[158] << 16;
  (pToStruct)->m_uData39 |=  (uint32_t)  (pFromData)[159] << 24;

  /* Unpack Member: m_uData38 */
  (pToStruct)->m_uData38 =  (uint32_t)  (pFromData)[152] ;
  (pToStruct)->m_uData38 |=  (uint32_t)  (pFromData)[153] << 8;
  (pToStruct)->m_uData38 |=  (uint32_t)  (pFromData)[154] << 16;
  (pToStruct)->m_uData38 |=  (uint32_t)  (pFromData)[155] << 24;

  /* Unpack Member: m_uData37 */
  (pToStruct)->m_uData37 =  (uint32_t)  (pFromData)[148] ;
  (pToStruct)->m_uData37 |=  (uint32_t)  (pFromData)[149] << 8;
  (pToStruct)->m_uData37 |=  (uint32_t)  (pFromData)[150] << 16;
  (pToStruct)->m_uData37 |=  (uint32_t)  (pFromData)[151] << 24;

  /* Unpack Member: m_uData36 */
  (pToStruct)->m_uData36 =  (uint32_t)  (pFromData)[144] ;
  (pToStruct)->m_uData36 |=  (uint32_t)  (pFromData)[145] << 8;
  (pToStruct)->m_uData36 |=  (uint32_t)  (pFromData)[146] << 16;
  (pToStruct)->m_uData36 |=  (uint32_t)  (pFromData)[147] << 24;

  /* Unpack Member: m_uData35 */
  (pToStruct)->m_uData35 =  (uint32_t)  (pFromData)[140] ;
  (pToStruct)->m_uData35 |=  (uint32_t)  (pFromData)[141] << 8;
  (pToStruct)->m_uData35 |=  (uint32_t)  (pFromData)[142] << 16;
  (pToStruct)->m_uData35 |=  (uint32_t)  (pFromData)[143] << 24;

  /* Unpack Member: m_uData34 */
  (pToStruct)->m_uData34 =  (uint32_t)  (pFromData)[136] ;
  (pToStruct)->m_uData34 |=  (uint32_t)  (pFromData)[137] << 8;
  (pToStruct)->m_uData34 |=  (uint32_t)  (pFromData)[138] << 16;
  (pToStruct)->m_uData34 |=  (uint32_t)  (pFromData)[139] << 24;

  /* Unpack Member: m_uData33 */
  (pToStruct)->m_uData33 =  (uint32_t)  (pFromData)[132] ;
  (pToStruct)->m_uData33 |=  (uint32_t)  (pFromData)[133] << 8;
  (pToStruct)->m_uData33 |=  (uint32_t)  (pFromData)[134] << 16;
  (pToStruct)->m_uData33 |=  (uint32_t)  (pFromData)[135] << 24;

  /* Unpack Member: m_uData32 */
  (pToStruct)->m_uData32 =  (uint32_t)  (pFromData)[128] ;
  (pToStruct)->m_uData32 |=  (uint32_t)  (pFromData)[129] << 8;
  (pToStruct)->m_uData32 |=  (uint32_t)  (pFromData)[130] << 16;
  (pToStruct)->m_uData32 |=  (uint32_t)  (pFromData)[131] << 24;

  /* Unpack Member: m_uData31 */
  (pToStruct)->m_uData31 =  (uint32_t)  (pFromData)[124] ;
  (pToStruct)->m_uData31 |=  (uint32_t)  (pFromData)[125] << 8;
  (pToStruct)->m_uData31 |=  (uint32_t)  (pFromData)[126] << 16;
  (pToStruct)->m_uData31 |=  (uint32_t)  (pFromData)[127] << 24;

  /* Unpack Member: m_uData30 */
  (pToStruct)->m_uData30 =  (uint32_t)  (pFromData)[120] ;
  (pToStruct)->m_uData30 |=  (uint32_t)  (pFromData)[121] << 8;
  (pToStruct)->m_uData30 |=  (uint32_t)  (pFromData)[122] << 16;
  (pToStruct)->m_uData30 |=  (uint32_t)  (pFromData)[123] << 24;

  /* Unpack Member: m_uData29 */
  (pToStruct)->m_uData29 =  (uint32_t)  (pFromData)[116] ;
  (pToStruct)->m_uData29 |=  (uint32_t)  (pFromData)[117] << 8;
  (pToStruct)->m_uData29 |=  (uint32_t)  (pFromData)[118] << 16;
  (pToStruct)->m_uData29 |=  (uint32_t)  (pFromData)[119] << 24;

  /* Unpack Member: m_uData28 */
  (pToStruct)->m_uData28 =  (uint32_t)  (pFromData)[112] ;
  (pToStruct)->m_uData28 |=  (uint32_t)  (pFromData)[113] << 8;
  (pToStruct)->m_uData28 |=  (uint32_t)  (pFromData)[114] << 16;
  (pToStruct)->m_uData28 |=  (uint32_t)  (pFromData)[115] << 24;

  /* Unpack Member: m_uData27 */
  (pToStruct)->m_uData27 =  (uint32_t)  (pFromData)[108] ;
  (pToStruct)->m_uData27 |=  (uint32_t)  (pFromData)[109] << 8;
  (pToStruct)->m_uData27 |=  (uint32_t)  (pFromData)[110] << 16;
  (pToStruct)->m_uData27 |=  (uint32_t)  (pFromData)[111] << 24;

  /* Unpack Member: m_uData26 */
  (pToStruct)->m_uData26 =  (uint32_t)  (pFromData)[104] ;
  (pToStruct)->m_uData26 |=  (uint32_t)  (pFromData)[105] << 8;
  (pToStruct)->m_uData26 |=  (uint32_t)  (pFromData)[106] << 16;
  (pToStruct)->m_uData26 |=  (uint32_t)  (pFromData)[107] << 24;

  /* Unpack Member: m_uData25 */
  (pToStruct)->m_uData25 =  (uint32_t)  (pFromData)[100] ;
  (pToStruct)->m_uData25 |=  (uint32_t)  (pFromData)[101] << 8;
  (pToStruct)->m_uData25 |=  (uint32_t)  (pFromData)[102] << 16;
  (pToStruct)->m_uData25 |=  (uint32_t)  (pFromData)[103] << 24;

  /* Unpack Member: m_uData24 */
  (pToStruct)->m_uData24 =  (uint32_t)  (pFromData)[96] ;
  (pToStruct)->m_uData24 |=  (uint32_t)  (pFromData)[97] << 8;
  (pToStruct)->m_uData24 |=  (uint32_t)  (pFromData)[98] << 16;
  (pToStruct)->m_uData24 |=  (uint32_t)  (pFromData)[99] << 24;

  /* Unpack Member: m_uData23 */
  (pToStruct)->m_uData23 =  (uint32_t)  (pFromData)[92] ;
  (pToStruct)->m_uData23 |=  (uint32_t)  (pFromData)[93] << 8;
  (pToStruct)->m_uData23 |=  (uint32_t)  (pFromData)[94] << 16;
  (pToStruct)->m_uData23 |=  (uint32_t)  (pFromData)[95] << 24;

  /* Unpack Member: m_uData22 */
  (pToStruct)->m_uData22 =  (uint32_t)  (pFromData)[88] ;
  (pToStruct)->m_uData22 |=  (uint32_t)  (pFromData)[89] << 8;
  (pToStruct)->m_uData22 |=  (uint32_t)  (pFromData)[90] << 16;
  (pToStruct)->m_uData22 |=  (uint32_t)  (pFromData)[91] << 24;

  /* Unpack Member: m_uData21 */
  (pToStruct)->m_uData21 =  (uint32_t)  (pFromData)[84] ;
  (pToStruct)->m_uData21 |=  (uint32_t)  (pFromData)[85] << 8;
  (pToStruct)->m_uData21 |=  (uint32_t)  (pFromData)[86] << 16;
  (pToStruct)->m_uData21 |=  (uint32_t)  (pFromData)[87] << 24;

  /* Unpack Member: m_uData20 */
  (pToStruct)->m_uData20 =  (uint32_t)  (pFromData)[80] ;
  (pToStruct)->m_uData20 |=  (uint32_t)  (pFromData)[81] << 8;
  (pToStruct)->m_uData20 |=  (uint32_t)  (pFromData)[82] << 16;
  (pToStruct)->m_uData20 |=  (uint32_t)  (pFromData)[83] << 24;

  /* Unpack Member: m_uData19 */
  (pToStruct)->m_uData19 =  (uint32_t)  (pFromData)[76] ;
  (pToStruct)->m_uData19 |=  (uint32_t)  (pFromData)[77] << 8;
  (pToStruct)->m_uData19 |=  (uint32_t)  (pFromData)[78] << 16;
  (pToStruct)->m_uData19 |=  (uint32_t)  (pFromData)[79] << 24;

  /* Unpack Member: m_uData18 */
  (pToStruct)->m_uData18 =  (uint32_t)  (pFromData)[72] ;
  (pToStruct)->m_uData18 |=  (uint32_t)  (pFromData)[73] << 8;
  (pToStruct)->m_uData18 |=  (uint32_t)  (pFromData)[74] << 16;
  (pToStruct)->m_uData18 |=  (uint32_t)  (pFromData)[75] << 24;

  /* Unpack Member: m_uData17 */
  (pToStruct)->m_uData17 =  (uint32_t)  (pFromData)[68] ;
  (pToStruct)->m_uData17 |=  (uint32_t)  (pFromData)[69] << 8;
  (pToStruct)->m_uData17 |=  (uint32_t)  (pFromData)[70] << 16;
  (pToStruct)->m_uData17 |=  (uint32_t)  (pFromData)[71] << 24;

  /* Unpack Member: m_uData16 */
  (pToStruct)->m_uData16 =  (uint32_t)  (pFromData)[64] ;
  (pToStruct)->m_uData16 |=  (uint32_t)  (pFromData)[65] << 8;
  (pToStruct)->m_uData16 |=  (uint32_t)  (pFromData)[66] << 16;
  (pToStruct)->m_uData16 |=  (uint32_t)  (pFromData)[67] << 24;

  /* Unpack Member: m_uData15 */
  (pToStruct)->m_uData15 =  (uint32_t)  (pFromData)[60] ;
  (pToStruct)->m_uData15 |=  (uint32_t)  (pFromData)[61] << 8;
  (pToStruct)->m_uData15 |=  (uint32_t)  (pFromData)[62] << 16;
  (pToStruct)->m_uData15 |=  (uint32_t)  (pFromData)[63] << 24;

  /* Unpack Member: m_uData14 */
  (pToStruct)->m_uData14 =  (uint32_t)  (pFromData)[56] ;
  (pToStruct)->m_uData14 |=  (uint32_t)  (pFromData)[57] << 8;
  (pToStruct)->m_uData14 |=  (uint32_t)  (pFromData)[58] << 16;
  (pToStruct)->m_uData14 |=  (uint32_t)  (pFromData)[59] << 24;

  /* Unpack Member: m_uData13 */
  (pToStruct)->m_uData13 =  (uint32_t)  (pFromData)[52] ;
  (pToStruct)->m_uData13 |=  (uint32_t)  (pFromData)[53] << 8;
  (pToStruct)->m_uData13 |=  (uint32_t)  (pFromData)[54] << 16;
  (pToStruct)->m_uData13 |=  (uint32_t)  (pFromData)[55] << 24;

  /* Unpack Member: m_uData12 */
  (pToStruct)->m_uData12 =  (uint32_t)  (pFromData)[48] ;
  (pToStruct)->m_uData12 |=  (uint32_t)  (pFromData)[49] << 8;
  (pToStruct)->m_uData12 |=  (uint32_t)  (pFromData)[50] << 16;
  (pToStruct)->m_uData12 |=  (uint32_t)  (pFromData)[51] << 24;

  /* Unpack Member: m_uData11 */
  (pToStruct)->m_uData11 =  (uint32_t)  (pFromData)[44] ;
  (pToStruct)->m_uData11 |=  (uint32_t)  (pFromData)[45] << 8;
  (pToStruct)->m_uData11 |=  (uint32_t)  (pFromData)[46] << 16;
  (pToStruct)->m_uData11 |=  (uint32_t)  (pFromData)[47] << 24;

  /* Unpack Member: m_uData10 */
  (pToStruct)->m_uData10 =  (uint32_t)  (pFromData)[40] ;
  (pToStruct)->m_uData10 |=  (uint32_t)  (pFromData)[41] << 8;
  (pToStruct)->m_uData10 |=  (uint32_t)  (pFromData)[42] << 16;
  (pToStruct)->m_uData10 |=  (uint32_t)  (pFromData)[43] << 24;

  /* Unpack Member: m_uData9 */
  (pToStruct)->m_uData9 =  (uint32_t)  (pFromData)[36] ;
  (pToStruct)->m_uData9 |=  (uint32_t)  (pFromData)[37] << 8;
  (pToStruct)->m_uData9 |=  (uint32_t)  (pFromData)[38] << 16;
  (pToStruct)->m_uData9 |=  (uint32_t)  (pFromData)[39] << 24;

  /* Unpack Member: m_uData8 */
  (pToStruct)->m_uData8 =  (uint32_t)  (pFromData)[32] ;
  (pToStruct)->m_uData8 |=  (uint32_t)  (pFromData)[33] << 8;
  (pToStruct)->m_uData8 |=  (uint32_t)  (pFromData)[34] << 16;
  (pToStruct)->m_uData8 |=  (uint32_t)  (pFromData)[35] << 24;

  /* Unpack Member: m_uSpare7 */
  (pToStruct)->m_uSpare7 =  (uint32_t)  ((pFromData)[30] >> 5) & 0x07;
  (pToStruct)->m_uSpare7 |=  (uint32_t)  (pFromData)[31] << 3;

  /* Unpack Member: m_uData7 */
  (pToStruct)->m_uData7 =  (uint32_t)  (pFromData)[28] ;
  (pToStruct)->m_uData7 |=  (uint32_t)  (pFromData)[29] << 8;
  (pToStruct)->m_uData7 |=  (uint32_t)  ((pFromData)[30] & 0x1f) << 16;

  /* Unpack Member: m_uSpare6 */
  (pToStruct)->m_uSpare6 =  (uint32_t)  ((pFromData)[26] >> 5) & 0x07;
  (pToStruct)->m_uSpare6 |=  (uint32_t)  (pFromData)[27] << 3;

  /* Unpack Member: m_uData6 */
  (pToStruct)->m_uData6 =  (uint32_t)  (pFromData)[24] ;
  (pToStruct)->m_uData6 |=  (uint32_t)  (pFromData)[25] << 8;
  (pToStruct)->m_uData6 |=  (uint32_t)  ((pFromData)[26] & 0x1f) << 16;

  /* Unpack Member: m_uSpare5 */
  (pToStruct)->m_uSpare5 =  (uint32_t)  ((pFromData)[22] >> 5) & 0x07;
  (pToStruct)->m_uSpare5 |=  (uint32_t)  (pFromData)[23] << 3;

  /* Unpack Member: m_uData5 */
  (pToStruct)->m_uData5 =  (uint32_t)  (pFromData)[20] ;
  (pToStruct)->m_uData5 |=  (uint32_t)  (pFromData)[21] << 8;
  (pToStruct)->m_uData5 |=  (uint32_t)  ((pFromData)[22] & 0x1f) << 16;

  /* Unpack Member: m_uSpare4 */
  (pToStruct)->m_uSpare4 =  (uint32_t)  ((pFromData)[18] >> 5) & 0x07;
  (pToStruct)->m_uSpare4 |=  (uint32_t)  (pFromData)[19] << 3;

  /* Unpack Member: m_uData4 */
  (pToStruct)->m_uData4 =  (uint32_t)  (pFromData)[16] ;
  (pToStruct)->m_uData4 |=  (uint32_t)  (pFromData)[17] << 8;
  (pToStruct)->m_uData4 |=  (uint32_t)  ((pFromData)[18] & 0x1f) << 16;

  /* Unpack Member: m_uSpare3 */
  (pToStruct)->m_uSpare3 =  (uint32_t)  ((pFromData)[14] >> 5) & 0x07;
  (pToStruct)->m_uSpare3 |=  (uint32_t)  (pFromData)[15] << 3;

  /* Unpack Member: m_uData3 */
  (pToStruct)->m_uData3 =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->m_uData3 |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->m_uData3 |=  (uint32_t)  ((pFromData)[14] & 0x1f) << 16;

  /* Unpack Member: m_uSpare2 */
  (pToStruct)->m_uSpare2 =  (uint32_t)  ((pFromData)[10] >> 5) & 0x07;
  (pToStruct)->m_uSpare2 |=  (uint32_t)  (pFromData)[11] << 3;

  /* Unpack Member: m_uData2 */
  (pToStruct)->m_uData2 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_uData2 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_uData2 |=  (uint32_t)  ((pFromData)[10] & 0x1f) << 16;

  /* Unpack Member: m_uSpare1 */
  (pToStruct)->m_uSpare1 =  (uint32_t)  ((pFromData)[6] >> 5) & 0x07;
  (pToStruct)->m_uSpare1 |=  (uint32_t)  (pFromData)[7] << 3;

  /* Unpack Member: m_uData1 */
  (pToStruct)->m_uData1 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uData1 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uData1 |=  (uint32_t)  ((pFromData)[6] & 0x1f) << 16;

  /* Unpack Member: m_uSpare0 */
  (pToStruct)->m_uSpare0 =  (uint32_t)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_uSpare0 |=  (uint32_t)  (pFromData)[3] << 3;

  /* Unpack Member: m_uData0 */
  (pToStruct)->m_uData0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_uData0 |=  (uint32_t)  ((pFromData)[2] & 0x1f) << 16;

}



/* initialize an instance of this zframe */
void
sbZfC2RcDataEntry2_InitInstance(sbZfC2RcDataEntry2_t *pFrame) {

  pFrame->m_uData39 =  (unsigned int)  0;
  pFrame->m_uData38 =  (unsigned int)  0;
  pFrame->m_uData37 =  (unsigned int)  0;
  pFrame->m_uData36 =  (unsigned int)  0;
  pFrame->m_uData35 =  (unsigned int)  0;
  pFrame->m_uData34 =  (unsigned int)  0;
  pFrame->m_uData33 =  (unsigned int)  0;
  pFrame->m_uData32 =  (unsigned int)  0;
  pFrame->m_uData31 =  (unsigned int)  0;
  pFrame->m_uData30 =  (unsigned int)  0;
  pFrame->m_uData29 =  (unsigned int)  0;
  pFrame->m_uData28 =  (unsigned int)  0;
  pFrame->m_uData27 =  (unsigned int)  0;
  pFrame->m_uData26 =  (unsigned int)  0;
  pFrame->m_uData25 =  (unsigned int)  0;
  pFrame->m_uData24 =  (unsigned int)  0;
  pFrame->m_uData23 =  (unsigned int)  0;
  pFrame->m_uData22 =  (unsigned int)  0;
  pFrame->m_uData21 =  (unsigned int)  0;
  pFrame->m_uData20 =  (unsigned int)  0;
  pFrame->m_uData19 =  (unsigned int)  0;
  pFrame->m_uData18 =  (unsigned int)  0;
  pFrame->m_uData17 =  (unsigned int)  0;
  pFrame->m_uData16 =  (unsigned int)  0;
  pFrame->m_uData15 =  (unsigned int)  0;
  pFrame->m_uData14 =  (unsigned int)  0;
  pFrame->m_uData13 =  (unsigned int)  0;
  pFrame->m_uData12 =  (unsigned int)  0;
  pFrame->m_uData11 =  (unsigned int)  0;
  pFrame->m_uData10 =  (unsigned int)  0;
  pFrame->m_uData9 =  (unsigned int)  0;
  pFrame->m_uData8 =  (unsigned int)  0;
  pFrame->m_uSpare7 =  (unsigned int)  0;
  pFrame->m_uData7 =  (unsigned int)  0;
  pFrame->m_uSpare6 =  (unsigned int)  0;
  pFrame->m_uData6 =  (unsigned int)  0;
  pFrame->m_uSpare5 =  (unsigned int)  0;
  pFrame->m_uData5 =  (unsigned int)  0;
  pFrame->m_uSpare4 =  (unsigned int)  0;
  pFrame->m_uData4 =  (unsigned int)  0;
  pFrame->m_uSpare3 =  (unsigned int)  0;
  pFrame->m_uData3 =  (unsigned int)  0;
  pFrame->m_uSpare2 =  (unsigned int)  0;
  pFrame->m_uData2 =  (unsigned int)  0;
  pFrame->m_uSpare1 =  (unsigned int)  0;
  pFrame->m_uData1 =  (unsigned int)  0;
  pFrame->m_uSpare0 =  (unsigned int)  0;
  pFrame->m_uData0 =  (unsigned int)  0;

}
