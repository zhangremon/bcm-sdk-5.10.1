/*
 * $Id: sbZfFe2000RcDmaFormat.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfFe2000RcDmaFormat.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000RcDmaFormat_Pack(sbZfFe2000RcDmaFormat_t *pFrom,
                           uint8_t *pToData,
                           uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE2000RCDMAFORMAT_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: uInstruction0 */
  (pToData)[0] |= ((pFrom)->uInstruction0) & 0xFF;
  (pToData)[1] |= ((pFrom)->uInstruction0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->uInstruction0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->uInstruction0 >> 24) &0xFF;

  /* Pack Member: uInstruction1 */
  (pToData)[4] |= ((pFrom)->uInstruction1) & 0xFF;
  (pToData)[5] |= ((pFrom)->uInstruction1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->uInstruction1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->uInstruction1 >> 24) &0xFF;

  /* Pack Member: uInstruction2 */
  (pToData)[8] |= ((pFrom)->uInstruction2) & 0xFF;
  (pToData)[9] |= ((pFrom)->uInstruction2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->uInstruction2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->uInstruction2 >> 24) &0xFF;

  /* Pack Member: uInstruction3 */
  (pToData)[12] |= ((pFrom)->uInstruction3) & 0xFF;
  (pToData)[13] |= ((pFrom)->uInstruction3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->uInstruction3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->uInstruction3 >> 24) &0xFF;

  /* Pack Member: uResv0 */
  (pToData)[16] |= ((pFrom)->uResv0) & 0xFF;
  (pToData)[17] |= ((pFrom)->uResv0 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->uResv0 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->uResv0 >> 24) &0xFF;

  /* Pack Member: uResv1 */
  (pToData)[20] |= ((pFrom)->uResv1) & 0xFF;
  (pToData)[21] |= ((pFrom)->uResv1 >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->uResv1 >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->uResv1 >> 24) &0xFF;

  /* Pack Member: uResv2 */
  (pToData)[24] |= ((pFrom)->uResv2) & 0xFF;
  (pToData)[25] |= ((pFrom)->uResv2 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->uResv2 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->uResv2 >> 24) &0xFF;

  /* Pack Member: uResv3 */
  (pToData)[28] |= ((pFrom)->uResv3) & 0xFF;
  (pToData)[29] |= ((pFrom)->uResv3 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->uResv3 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->uResv3 >> 24) &0xFF;

  /* Pack Member: uResv4 */
  (pToData)[32] |= ((pFrom)->uResv4) & 0xFF;
  (pToData)[33] |= ((pFrom)->uResv4 >> 8) &0xFF;
  (pToData)[34] |= ((pFrom)->uResv4 >> 16) &0xFF;
  (pToData)[35] |= ((pFrom)->uResv4 >> 24) &0xFF;

  /* Pack Member: uResv5 */
  (pToData)[36] |= ((pFrom)->uResv5) & 0xFF;
  (pToData)[37] |= ((pFrom)->uResv5 >> 8) &0xFF;
  (pToData)[38] |= ((pFrom)->uResv5 >> 16) &0xFF;
  (pToData)[39] |= ((pFrom)->uResv5 >> 24) &0xFF;

  /* Pack Member: uResv6 */
  (pToData)[40] |= ((pFrom)->uResv6) & 0xFF;
  (pToData)[41] |= ((pFrom)->uResv6 >> 8) &0xFF;
  (pToData)[42] |= ((pFrom)->uResv6 >> 16) &0xFF;
  (pToData)[43] |= ((pFrom)->uResv6 >> 24) &0xFF;

  /* Pack Member: uResv7 */
  (pToData)[44] |= ((pFrom)->uResv7) & 0xFF;
  (pToData)[45] |= ((pFrom)->uResv7 >> 8) &0xFF;
  (pToData)[46] |= ((pFrom)->uResv7 >> 16) &0xFF;
  (pToData)[47] |= ((pFrom)->uResv7 >> 24) &0xFF;

  /* Pack Member: uResv8 */
  (pToData)[48] |= ((pFrom)->uResv8) & 0xFF;
  (pToData)[49] |= ((pFrom)->uResv8 >> 8) &0xFF;
  (pToData)[50] |= ((pFrom)->uResv8 >> 16) &0xFF;
  (pToData)[51] |= ((pFrom)->uResv8 >> 24) &0xFF;

  /* Pack Member: uResv9 */
  (pToData)[52] |= ((pFrom)->uResv9) & 0xFF;
  (pToData)[53] |= ((pFrom)->uResv9 >> 8) &0xFF;
  (pToData)[54] |= ((pFrom)->uResv9 >> 16) &0xFF;
  (pToData)[55] |= ((pFrom)->uResv9 >> 24) &0xFF;

  /* Pack Member: uResv10 */
  (pToData)[56] |= ((pFrom)->uResv10) & 0xFF;
  (pToData)[57] |= ((pFrom)->uResv10 >> 8) &0xFF;
  (pToData)[58] |= ((pFrom)->uResv10 >> 16) &0xFF;
  (pToData)[59] |= ((pFrom)->uResv10 >> 24) &0xFF;

  /* Pack Member: uResv11 */
  (pToData)[60] |= ((pFrom)->uResv11) & 0xFF;
  (pToData)[61] |= ((pFrom)->uResv11 >> 8) &0xFF;
  (pToData)[62] |= ((pFrom)->uResv11 >> 16) &0xFF;
  (pToData)[63] |= ((pFrom)->uResv11 >> 24) &0xFF;

  /* Pack Member: uResv12 */
  (pToData)[64] |= ((pFrom)->uResv12) & 0xFF;
  (pToData)[65] |= ((pFrom)->uResv12 >> 8) &0xFF;
  (pToData)[66] |= ((pFrom)->uResv12 >> 16) &0xFF;
  (pToData)[67] |= ((pFrom)->uResv12 >> 24) &0xFF;

  /* Pack Member: uResv13 */
  (pToData)[68] |= ((pFrom)->uResv13) & 0xFF;
  (pToData)[69] |= ((pFrom)->uResv13 >> 8) &0xFF;
  (pToData)[70] |= ((pFrom)->uResv13 >> 16) &0xFF;
  (pToData)[71] |= ((pFrom)->uResv13 >> 24) &0xFF;

  /* Pack Member: uResv14 */
  (pToData)[72] |= ((pFrom)->uResv14) & 0xFF;
  (pToData)[73] |= ((pFrom)->uResv14 >> 8) &0xFF;
  (pToData)[74] |= ((pFrom)->uResv14 >> 16) &0xFF;
  (pToData)[75] |= ((pFrom)->uResv14 >> 24) &0xFF;

  /* Pack Member: uResv15 */
  (pToData)[76] |= ((pFrom)->uResv15) & 0xFF;
  (pToData)[77] |= ((pFrom)->uResv15 >> 8) &0xFF;
  (pToData)[78] |= ((pFrom)->uResv15 >> 16) &0xFF;
  (pToData)[79] |= ((pFrom)->uResv15 >> 24) &0xFF;

  /* Pack Member: uResv16 */
  (pToData)[80] |= ((pFrom)->uResv16) & 0xFF;
  (pToData)[81] |= ((pFrom)->uResv16 >> 8) &0xFF;
  (pToData)[82] |= ((pFrom)->uResv16 >> 16) &0xFF;
  (pToData)[83] |= ((pFrom)->uResv16 >> 24) &0xFF;

  /* Pack Member: uResv17 */
  (pToData)[84] |= ((pFrom)->uResv17) & 0xFF;
  (pToData)[85] |= ((pFrom)->uResv17 >> 8) &0xFF;
  (pToData)[86] |= ((pFrom)->uResv17 >> 16) &0xFF;
  (pToData)[87] |= ((pFrom)->uResv17 >> 24) &0xFF;

  /* Pack Member: uResv18 */
  (pToData)[88] |= ((pFrom)->uResv18) & 0xFF;
  (pToData)[89] |= ((pFrom)->uResv18 >> 8) &0xFF;
  (pToData)[90] |= ((pFrom)->uResv18 >> 16) &0xFF;
  (pToData)[91] |= ((pFrom)->uResv18 >> 24) &0xFF;

  /* Pack Member: uResv19 */
  (pToData)[92] |= ((pFrom)->uResv19) & 0xFF;
  (pToData)[93] |= ((pFrom)->uResv19 >> 8) &0xFF;
  (pToData)[94] |= ((pFrom)->uResv19 >> 16) &0xFF;
  (pToData)[95] |= ((pFrom)->uResv19 >> 24) &0xFF;

  /* Pack Member: uResv20 */
  (pToData)[96] |= ((pFrom)->uResv20) & 0xFF;
  (pToData)[97] |= ((pFrom)->uResv20 >> 8) &0xFF;
  (pToData)[98] |= ((pFrom)->uResv20 >> 16) &0xFF;
  (pToData)[99] |= ((pFrom)->uResv20 >> 24) &0xFF;

  /* Pack Member: uResv21 */
  (pToData)[100] |= ((pFrom)->uResv21) & 0xFF;
  (pToData)[101] |= ((pFrom)->uResv21 >> 8) &0xFF;
  (pToData)[102] |= ((pFrom)->uResv21 >> 16) &0xFF;
  (pToData)[103] |= ((pFrom)->uResv21 >> 24) &0xFF;

  /* Pack Member: uResv22 */
  (pToData)[104] |= ((pFrom)->uResv22) & 0xFF;
  (pToData)[105] |= ((pFrom)->uResv22 >> 8) &0xFF;
  (pToData)[106] |= ((pFrom)->uResv22 >> 16) &0xFF;
  (pToData)[107] |= ((pFrom)->uResv22 >> 24) &0xFF;

  /* Pack Member: uResv23 */
  (pToData)[108] |= ((pFrom)->uResv23) & 0xFF;
  (pToData)[109] |= ((pFrom)->uResv23 >> 8) &0xFF;
  (pToData)[110] |= ((pFrom)->uResv23 >> 16) &0xFF;
  (pToData)[111] |= ((pFrom)->uResv23 >> 24) &0xFF;

  /* Pack Member: uResv24 */
  (pToData)[112] |= ((pFrom)->uResv24) & 0xFF;
  (pToData)[113] |= ((pFrom)->uResv24 >> 8) &0xFF;
  (pToData)[114] |= ((pFrom)->uResv24 >> 16) &0xFF;
  (pToData)[115] |= ((pFrom)->uResv24 >> 24) &0xFF;

  /* Pack Member: uResv25 */
  (pToData)[116] |= ((pFrom)->uResv25) & 0xFF;
  (pToData)[117] |= ((pFrom)->uResv25 >> 8) &0xFF;
  (pToData)[118] |= ((pFrom)->uResv25 >> 16) &0xFF;
  (pToData)[119] |= ((pFrom)->uResv25 >> 24) &0xFF;

  /* Pack Member: uResv26 */
  (pToData)[120] |= ((pFrom)->uResv26) & 0xFF;
  (pToData)[121] |= ((pFrom)->uResv26 >> 8) &0xFF;
  (pToData)[122] |= ((pFrom)->uResv26 >> 16) &0xFF;
  (pToData)[123] |= ((pFrom)->uResv26 >> 24) &0xFF;

  /* Pack Member: uResv27 */
  (pToData)[124] |= ((pFrom)->uResv27) & 0xFF;
  (pToData)[125] |= ((pFrom)->uResv27 >> 8) &0xFF;
  (pToData)[126] |= ((pFrom)->uResv27 >> 16) &0xFF;
  (pToData)[127] |= ((pFrom)->uResv27 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern0_31_0 */
  (pToData)[128] |= ((pFrom)->uSb0Pattern0_31_0) & 0xFF;
  (pToData)[129] |= ((pFrom)->uSb0Pattern0_31_0 >> 8) &0xFF;
  (pToData)[130] |= ((pFrom)->uSb0Pattern0_31_0 >> 16) &0xFF;
  (pToData)[131] |= ((pFrom)->uSb0Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern0_63_32 */
  (pToData)[132] |= ((pFrom)->uSb0Pattern0_63_32) & 0xFF;
  (pToData)[133] |= ((pFrom)->uSb0Pattern0_63_32 >> 8) &0xFF;
  (pToData)[134] |= ((pFrom)->uSb0Pattern0_63_32 >> 16) &0xFF;
  (pToData)[135] |= ((pFrom)->uSb0Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern0_95_64 */
  (pToData)[136] |= ((pFrom)->uSb0Pattern0_95_64) & 0xFF;
  (pToData)[137] |= ((pFrom)->uSb0Pattern0_95_64 >> 8) &0xFF;
  (pToData)[138] |= ((pFrom)->uSb0Pattern0_95_64 >> 16) &0xFF;
  (pToData)[139] |= ((pFrom)->uSb0Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern0_127_96 */
  (pToData)[140] |= ((pFrom)->uSb0Pattern0_127_96) & 0xFF;
  (pToData)[141] |= ((pFrom)->uSb0Pattern0_127_96 >> 8) &0xFF;
  (pToData)[142] |= ((pFrom)->uSb0Pattern0_127_96 >> 16) &0xFF;
  (pToData)[143] |= ((pFrom)->uSb0Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern1_31_0 */
  (pToData)[144] |= ((pFrom)->uSb0Pattern1_31_0) & 0xFF;
  (pToData)[145] |= ((pFrom)->uSb0Pattern1_31_0 >> 8) &0xFF;
  (pToData)[146] |= ((pFrom)->uSb0Pattern1_31_0 >> 16) &0xFF;
  (pToData)[147] |= ((pFrom)->uSb0Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern1_63_32 */
  (pToData)[148] |= ((pFrom)->uSb0Pattern1_63_32) & 0xFF;
  (pToData)[149] |= ((pFrom)->uSb0Pattern1_63_32 >> 8) &0xFF;
  (pToData)[150] |= ((pFrom)->uSb0Pattern1_63_32 >> 16) &0xFF;
  (pToData)[151] |= ((pFrom)->uSb0Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern1_95_64 */
  (pToData)[152] |= ((pFrom)->uSb0Pattern1_95_64) & 0xFF;
  (pToData)[153] |= ((pFrom)->uSb0Pattern1_95_64 >> 8) &0xFF;
  (pToData)[154] |= ((pFrom)->uSb0Pattern1_95_64 >> 16) &0xFF;
  (pToData)[155] |= ((pFrom)->uSb0Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern1_127_96 */
  (pToData)[156] |= ((pFrom)->uSb0Pattern1_127_96) & 0xFF;
  (pToData)[157] |= ((pFrom)->uSb0Pattern1_127_96 >> 8) &0xFF;
  (pToData)[158] |= ((pFrom)->uSb0Pattern1_127_96 >> 16) &0xFF;
  (pToData)[159] |= ((pFrom)->uSb0Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern2_31_0 */
  (pToData)[160] |= ((pFrom)->uSb0Pattern2_31_0) & 0xFF;
  (pToData)[161] |= ((pFrom)->uSb0Pattern2_31_0 >> 8) &0xFF;
  (pToData)[162] |= ((pFrom)->uSb0Pattern2_31_0 >> 16) &0xFF;
  (pToData)[163] |= ((pFrom)->uSb0Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern2_63_32 */
  (pToData)[164] |= ((pFrom)->uSb0Pattern2_63_32) & 0xFF;
  (pToData)[165] |= ((pFrom)->uSb0Pattern2_63_32 >> 8) &0xFF;
  (pToData)[166] |= ((pFrom)->uSb0Pattern2_63_32 >> 16) &0xFF;
  (pToData)[167] |= ((pFrom)->uSb0Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern2_95_64 */
  (pToData)[168] |= ((pFrom)->uSb0Pattern2_95_64) & 0xFF;
  (pToData)[169] |= ((pFrom)->uSb0Pattern2_95_64 >> 8) &0xFF;
  (pToData)[170] |= ((pFrom)->uSb0Pattern2_95_64 >> 16) &0xFF;
  (pToData)[171] |= ((pFrom)->uSb0Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern2_127_96 */
  (pToData)[172] |= ((pFrom)->uSb0Pattern2_127_96) & 0xFF;
  (pToData)[173] |= ((pFrom)->uSb0Pattern2_127_96 >> 8) &0xFF;
  (pToData)[174] |= ((pFrom)->uSb0Pattern2_127_96 >> 16) &0xFF;
  (pToData)[175] |= ((pFrom)->uSb0Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern3_31_0 */
  (pToData)[176] |= ((pFrom)->uSb0Pattern3_31_0) & 0xFF;
  (pToData)[177] |= ((pFrom)->uSb0Pattern3_31_0 >> 8) &0xFF;
  (pToData)[178] |= ((pFrom)->uSb0Pattern3_31_0 >> 16) &0xFF;
  (pToData)[179] |= ((pFrom)->uSb0Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern3_63_32 */
  (pToData)[180] |= ((pFrom)->uSb0Pattern3_63_32) & 0xFF;
  (pToData)[181] |= ((pFrom)->uSb0Pattern3_63_32 >> 8) &0xFF;
  (pToData)[182] |= ((pFrom)->uSb0Pattern3_63_32 >> 16) &0xFF;
  (pToData)[183] |= ((pFrom)->uSb0Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern3_95_64 */
  (pToData)[184] |= ((pFrom)->uSb0Pattern3_95_64) & 0xFF;
  (pToData)[185] |= ((pFrom)->uSb0Pattern3_95_64 >> 8) &0xFF;
  (pToData)[186] |= ((pFrom)->uSb0Pattern3_95_64 >> 16) &0xFF;
  (pToData)[187] |= ((pFrom)->uSb0Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: uSb0Pattern3_127_96 */
  (pToData)[188] |= ((pFrom)->uSb0Pattern3_127_96) & 0xFF;
  (pToData)[189] |= ((pFrom)->uSb0Pattern3_127_96 >> 8) &0xFF;
  (pToData)[190] |= ((pFrom)->uSb0Pattern3_127_96 >> 16) &0xFF;
  (pToData)[191] |= ((pFrom)->uSb0Pattern3_127_96 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern0_31_0 */
  (pToData)[192] |= ((pFrom)->uSb1Pattern0_31_0) & 0xFF;
  (pToData)[193] |= ((pFrom)->uSb1Pattern0_31_0 >> 8) &0xFF;
  (pToData)[194] |= ((pFrom)->uSb1Pattern0_31_0 >> 16) &0xFF;
  (pToData)[195] |= ((pFrom)->uSb1Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern0_63_32 */
  (pToData)[196] |= ((pFrom)->uSb1Pattern0_63_32) & 0xFF;
  (pToData)[197] |= ((pFrom)->uSb1Pattern0_63_32 >> 8) &0xFF;
  (pToData)[198] |= ((pFrom)->uSb1Pattern0_63_32 >> 16) &0xFF;
  (pToData)[199] |= ((pFrom)->uSb1Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern0_95_64 */
  (pToData)[200] |= ((pFrom)->uSb1Pattern0_95_64) & 0xFF;
  (pToData)[201] |= ((pFrom)->uSb1Pattern0_95_64 >> 8) &0xFF;
  (pToData)[202] |= ((pFrom)->uSb1Pattern0_95_64 >> 16) &0xFF;
  (pToData)[203] |= ((pFrom)->uSb1Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern0_127_96 */
  (pToData)[204] |= ((pFrom)->uSb1Pattern0_127_96) & 0xFF;
  (pToData)[205] |= ((pFrom)->uSb1Pattern0_127_96 >> 8) &0xFF;
  (pToData)[206] |= ((pFrom)->uSb1Pattern0_127_96 >> 16) &0xFF;
  (pToData)[207] |= ((pFrom)->uSb1Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern1_31_0 */
  (pToData)[208] |= ((pFrom)->uSb1Pattern1_31_0) & 0xFF;
  (pToData)[209] |= ((pFrom)->uSb1Pattern1_31_0 >> 8) &0xFF;
  (pToData)[210] |= ((pFrom)->uSb1Pattern1_31_0 >> 16) &0xFF;
  (pToData)[211] |= ((pFrom)->uSb1Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern1_63_32 */
  (pToData)[212] |= ((pFrom)->uSb1Pattern1_63_32) & 0xFF;
  (pToData)[213] |= ((pFrom)->uSb1Pattern1_63_32 >> 8) &0xFF;
  (pToData)[214] |= ((pFrom)->uSb1Pattern1_63_32 >> 16) &0xFF;
  (pToData)[215] |= ((pFrom)->uSb1Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern1_95_64 */
  (pToData)[216] |= ((pFrom)->uSb1Pattern1_95_64) & 0xFF;
  (pToData)[217] |= ((pFrom)->uSb1Pattern1_95_64 >> 8) &0xFF;
  (pToData)[218] |= ((pFrom)->uSb1Pattern1_95_64 >> 16) &0xFF;
  (pToData)[219] |= ((pFrom)->uSb1Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern1_127_96 */
  (pToData)[220] |= ((pFrom)->uSb1Pattern1_127_96) & 0xFF;
  (pToData)[221] |= ((pFrom)->uSb1Pattern1_127_96 >> 8) &0xFF;
  (pToData)[222] |= ((pFrom)->uSb1Pattern1_127_96 >> 16) &0xFF;
  (pToData)[223] |= ((pFrom)->uSb1Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern2_31_0 */
  (pToData)[224] |= ((pFrom)->uSb1Pattern2_31_0) & 0xFF;
  (pToData)[225] |= ((pFrom)->uSb1Pattern2_31_0 >> 8) &0xFF;
  (pToData)[226] |= ((pFrom)->uSb1Pattern2_31_0 >> 16) &0xFF;
  (pToData)[227] |= ((pFrom)->uSb1Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern2_63_32 */
  (pToData)[228] |= ((pFrom)->uSb1Pattern2_63_32) & 0xFF;
  (pToData)[229] |= ((pFrom)->uSb1Pattern2_63_32 >> 8) &0xFF;
  (pToData)[230] |= ((pFrom)->uSb1Pattern2_63_32 >> 16) &0xFF;
  (pToData)[231] |= ((pFrom)->uSb1Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern2_95_64 */
  (pToData)[232] |= ((pFrom)->uSb1Pattern2_95_64) & 0xFF;
  (pToData)[233] |= ((pFrom)->uSb1Pattern2_95_64 >> 8) &0xFF;
  (pToData)[234] |= ((pFrom)->uSb1Pattern2_95_64 >> 16) &0xFF;
  (pToData)[235] |= ((pFrom)->uSb1Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern2_127_96 */
  (pToData)[236] |= ((pFrom)->uSb1Pattern2_127_96) & 0xFF;
  (pToData)[237] |= ((pFrom)->uSb1Pattern2_127_96 >> 8) &0xFF;
  (pToData)[238] |= ((pFrom)->uSb1Pattern2_127_96 >> 16) &0xFF;
  (pToData)[239] |= ((pFrom)->uSb1Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern3_31_0 */
  (pToData)[240] |= ((pFrom)->uSb1Pattern3_31_0) & 0xFF;
  (pToData)[241] |= ((pFrom)->uSb1Pattern3_31_0 >> 8) &0xFF;
  (pToData)[242] |= ((pFrom)->uSb1Pattern3_31_0 >> 16) &0xFF;
  (pToData)[243] |= ((pFrom)->uSb1Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern3_63_32 */
  (pToData)[244] |= ((pFrom)->uSb1Pattern3_63_32) & 0xFF;
  (pToData)[245] |= ((pFrom)->uSb1Pattern3_63_32 >> 8) &0xFF;
  (pToData)[246] |= ((pFrom)->uSb1Pattern3_63_32 >> 16) &0xFF;
  (pToData)[247] |= ((pFrom)->uSb1Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern3_95_64 */
  (pToData)[248] |= ((pFrom)->uSb1Pattern3_95_64) & 0xFF;
  (pToData)[249] |= ((pFrom)->uSb1Pattern3_95_64 >> 8) &0xFF;
  (pToData)[250] |= ((pFrom)->uSb1Pattern3_95_64 >> 16) &0xFF;
  (pToData)[251] |= ((pFrom)->uSb1Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: uSb1Pattern3_127_96 */
  (pToData)[252] |= ((pFrom)->uSb1Pattern3_127_96) & 0xFF;
  (pToData)[253] |= ((pFrom)->uSb1Pattern3_127_96 >> 8) &0xFF;
  (pToData)[254] |= ((pFrom)->uSb1Pattern3_127_96 >> 16) &0xFF;
  (pToData)[255] |= ((pFrom)->uSb1Pattern3_127_96 >> 24) &0xFF;

  return SB_ZF_FE2000RCDMAFORMAT_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000RcDmaFormat_Unpack(sbZfFe2000RcDmaFormat_t *pToStruct,
                             uint8_t *pFromData,
                             uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: uInstruction0 */
  (pToStruct)->uInstruction0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->uInstruction0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->uInstruction0 |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->uInstruction0 |=  (uint32_t)  (pFromData)[3] << 24;

  /* Unpack Member: uInstruction1 */
  (pToStruct)->uInstruction1 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->uInstruction1 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->uInstruction1 |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->uInstruction1 |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: uInstruction2 */
  (pToStruct)->uInstruction2 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->uInstruction2 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->uInstruction2 |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->uInstruction2 |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: uInstruction3 */
  (pToStruct)->uInstruction3 =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->uInstruction3 |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->uInstruction3 |=  (uint32_t)  (pFromData)[14] << 16;
  (pToStruct)->uInstruction3 |=  (uint32_t)  (pFromData)[15] << 24;

  /* Unpack Member: uResv0 */
  (pToStruct)->uResv0 =  (uint32_t)  (pFromData)[16] ;
  (pToStruct)->uResv0 |=  (uint32_t)  (pFromData)[17] << 8;
  (pToStruct)->uResv0 |=  (uint32_t)  (pFromData)[18] << 16;
  (pToStruct)->uResv0 |=  (uint32_t)  (pFromData)[19] << 24;

  /* Unpack Member: uResv1 */
  (pToStruct)->uResv1 =  (uint32_t)  (pFromData)[20] ;
  (pToStruct)->uResv1 |=  (uint32_t)  (pFromData)[21] << 8;
  (pToStruct)->uResv1 |=  (uint32_t)  (pFromData)[22] << 16;
  (pToStruct)->uResv1 |=  (uint32_t)  (pFromData)[23] << 24;

  /* Unpack Member: uResv2 */
  (pToStruct)->uResv2 =  (uint32_t)  (pFromData)[24] ;
  (pToStruct)->uResv2 |=  (uint32_t)  (pFromData)[25] << 8;
  (pToStruct)->uResv2 |=  (uint32_t)  (pFromData)[26] << 16;
  (pToStruct)->uResv2 |=  (uint32_t)  (pFromData)[27] << 24;

  /* Unpack Member: uResv3 */
  (pToStruct)->uResv3 =  (uint32_t)  (pFromData)[28] ;
  (pToStruct)->uResv3 |=  (uint32_t)  (pFromData)[29] << 8;
  (pToStruct)->uResv3 |=  (uint32_t)  (pFromData)[30] << 16;
  (pToStruct)->uResv3 |=  (uint32_t)  (pFromData)[31] << 24;

  /* Unpack Member: uResv4 */
  (pToStruct)->uResv4 =  (uint32_t)  (pFromData)[32] ;
  (pToStruct)->uResv4 |=  (uint32_t)  (pFromData)[33] << 8;
  (pToStruct)->uResv4 |=  (uint32_t)  (pFromData)[34] << 16;
  (pToStruct)->uResv4 |=  (uint32_t)  (pFromData)[35] << 24;

  /* Unpack Member: uResv5 */
  (pToStruct)->uResv5 =  (uint32_t)  (pFromData)[36] ;
  (pToStruct)->uResv5 |=  (uint32_t)  (pFromData)[37] << 8;
  (pToStruct)->uResv5 |=  (uint32_t)  (pFromData)[38] << 16;
  (pToStruct)->uResv5 |=  (uint32_t)  (pFromData)[39] << 24;

  /* Unpack Member: uResv6 */
  (pToStruct)->uResv6 =  (uint32_t)  (pFromData)[40] ;
  (pToStruct)->uResv6 |=  (uint32_t)  (pFromData)[41] << 8;
  (pToStruct)->uResv6 |=  (uint32_t)  (pFromData)[42] << 16;
  (pToStruct)->uResv6 |=  (uint32_t)  (pFromData)[43] << 24;

  /* Unpack Member: uResv7 */
  (pToStruct)->uResv7 =  (uint32_t)  (pFromData)[44] ;
  (pToStruct)->uResv7 |=  (uint32_t)  (pFromData)[45] << 8;
  (pToStruct)->uResv7 |=  (uint32_t)  (pFromData)[46] << 16;
  (pToStruct)->uResv7 |=  (uint32_t)  (pFromData)[47] << 24;

  /* Unpack Member: uResv8 */
  (pToStruct)->uResv8 =  (uint32_t)  (pFromData)[48] ;
  (pToStruct)->uResv8 |=  (uint32_t)  (pFromData)[49] << 8;
  (pToStruct)->uResv8 |=  (uint32_t)  (pFromData)[50] << 16;
  (pToStruct)->uResv8 |=  (uint32_t)  (pFromData)[51] << 24;

  /* Unpack Member: uResv9 */
  (pToStruct)->uResv9 =  (uint32_t)  (pFromData)[52] ;
  (pToStruct)->uResv9 |=  (uint32_t)  (pFromData)[53] << 8;
  (pToStruct)->uResv9 |=  (uint32_t)  (pFromData)[54] << 16;
  (pToStruct)->uResv9 |=  (uint32_t)  (pFromData)[55] << 24;

  /* Unpack Member: uResv10 */
  (pToStruct)->uResv10 =  (uint32_t)  (pFromData)[56] ;
  (pToStruct)->uResv10 |=  (uint32_t)  (pFromData)[57] << 8;
  (pToStruct)->uResv10 |=  (uint32_t)  (pFromData)[58] << 16;
  (pToStruct)->uResv10 |=  (uint32_t)  (pFromData)[59] << 24;

  /* Unpack Member: uResv11 */
  (pToStruct)->uResv11 =  (uint32_t)  (pFromData)[60] ;
  (pToStruct)->uResv11 |=  (uint32_t)  (pFromData)[61] << 8;
  (pToStruct)->uResv11 |=  (uint32_t)  (pFromData)[62] << 16;
  (pToStruct)->uResv11 |=  (uint32_t)  (pFromData)[63] << 24;

  /* Unpack Member: uResv12 */
  (pToStruct)->uResv12 =  (uint32_t)  (pFromData)[64] ;
  (pToStruct)->uResv12 |=  (uint32_t)  (pFromData)[65] << 8;
  (pToStruct)->uResv12 |=  (uint32_t)  (pFromData)[66] << 16;
  (pToStruct)->uResv12 |=  (uint32_t)  (pFromData)[67] << 24;

  /* Unpack Member: uResv13 */
  (pToStruct)->uResv13 =  (uint32_t)  (pFromData)[68] ;
  (pToStruct)->uResv13 |=  (uint32_t)  (pFromData)[69] << 8;
  (pToStruct)->uResv13 |=  (uint32_t)  (pFromData)[70] << 16;
  (pToStruct)->uResv13 |=  (uint32_t)  (pFromData)[71] << 24;

  /* Unpack Member: uResv14 */
  (pToStruct)->uResv14 =  (uint32_t)  (pFromData)[72] ;
  (pToStruct)->uResv14 |=  (uint32_t)  (pFromData)[73] << 8;
  (pToStruct)->uResv14 |=  (uint32_t)  (pFromData)[74] << 16;
  (pToStruct)->uResv14 |=  (uint32_t)  (pFromData)[75] << 24;

  /* Unpack Member: uResv15 */
  (pToStruct)->uResv15 =  (uint32_t)  (pFromData)[76] ;
  (pToStruct)->uResv15 |=  (uint32_t)  (pFromData)[77] << 8;
  (pToStruct)->uResv15 |=  (uint32_t)  (pFromData)[78] << 16;
  (pToStruct)->uResv15 |=  (uint32_t)  (pFromData)[79] << 24;

  /* Unpack Member: uResv16 */
  (pToStruct)->uResv16 =  (uint32_t)  (pFromData)[80] ;
  (pToStruct)->uResv16 |=  (uint32_t)  (pFromData)[81] << 8;
  (pToStruct)->uResv16 |=  (uint32_t)  (pFromData)[82] << 16;
  (pToStruct)->uResv16 |=  (uint32_t)  (pFromData)[83] << 24;

  /* Unpack Member: uResv17 */
  (pToStruct)->uResv17 =  (uint32_t)  (pFromData)[84] ;
  (pToStruct)->uResv17 |=  (uint32_t)  (pFromData)[85] << 8;
  (pToStruct)->uResv17 |=  (uint32_t)  (pFromData)[86] << 16;
  (pToStruct)->uResv17 |=  (uint32_t)  (pFromData)[87] << 24;

  /* Unpack Member: uResv18 */
  (pToStruct)->uResv18 =  (uint32_t)  (pFromData)[88] ;
  (pToStruct)->uResv18 |=  (uint32_t)  (pFromData)[89] << 8;
  (pToStruct)->uResv18 |=  (uint32_t)  (pFromData)[90] << 16;
  (pToStruct)->uResv18 |=  (uint32_t)  (pFromData)[91] << 24;

  /* Unpack Member: uResv19 */
  (pToStruct)->uResv19 =  (uint32_t)  (pFromData)[92] ;
  (pToStruct)->uResv19 |=  (uint32_t)  (pFromData)[93] << 8;
  (pToStruct)->uResv19 |=  (uint32_t)  (pFromData)[94] << 16;
  (pToStruct)->uResv19 |=  (uint32_t)  (pFromData)[95] << 24;

  /* Unpack Member: uResv20 */
  (pToStruct)->uResv20 =  (uint32_t)  (pFromData)[96] ;
  (pToStruct)->uResv20 |=  (uint32_t)  (pFromData)[97] << 8;
  (pToStruct)->uResv20 |=  (uint32_t)  (pFromData)[98] << 16;
  (pToStruct)->uResv20 |=  (uint32_t)  (pFromData)[99] << 24;

  /* Unpack Member: uResv21 */
  (pToStruct)->uResv21 =  (uint32_t)  (pFromData)[100] ;
  (pToStruct)->uResv21 |=  (uint32_t)  (pFromData)[101] << 8;
  (pToStruct)->uResv21 |=  (uint32_t)  (pFromData)[102] << 16;
  (pToStruct)->uResv21 |=  (uint32_t)  (pFromData)[103] << 24;

  /* Unpack Member: uResv22 */
  (pToStruct)->uResv22 =  (uint32_t)  (pFromData)[104] ;
  (pToStruct)->uResv22 |=  (uint32_t)  (pFromData)[105] << 8;
  (pToStruct)->uResv22 |=  (uint32_t)  (pFromData)[106] << 16;
  (pToStruct)->uResv22 |=  (uint32_t)  (pFromData)[107] << 24;

  /* Unpack Member: uResv23 */
  (pToStruct)->uResv23 =  (uint32_t)  (pFromData)[108] ;
  (pToStruct)->uResv23 |=  (uint32_t)  (pFromData)[109] << 8;
  (pToStruct)->uResv23 |=  (uint32_t)  (pFromData)[110] << 16;
  (pToStruct)->uResv23 |=  (uint32_t)  (pFromData)[111] << 24;

  /* Unpack Member: uResv24 */
  (pToStruct)->uResv24 =  (uint32_t)  (pFromData)[112] ;
  (pToStruct)->uResv24 |=  (uint32_t)  (pFromData)[113] << 8;
  (pToStruct)->uResv24 |=  (uint32_t)  (pFromData)[114] << 16;
  (pToStruct)->uResv24 |=  (uint32_t)  (pFromData)[115] << 24;

  /* Unpack Member: uResv25 */
  (pToStruct)->uResv25 =  (uint32_t)  (pFromData)[116] ;
  (pToStruct)->uResv25 |=  (uint32_t)  (pFromData)[117] << 8;
  (pToStruct)->uResv25 |=  (uint32_t)  (pFromData)[118] << 16;
  (pToStruct)->uResv25 |=  (uint32_t)  (pFromData)[119] << 24;

  /* Unpack Member: uResv26 */
  (pToStruct)->uResv26 =  (uint32_t)  (pFromData)[120] ;
  (pToStruct)->uResv26 |=  (uint32_t)  (pFromData)[121] << 8;
  (pToStruct)->uResv26 |=  (uint32_t)  (pFromData)[122] << 16;
  (pToStruct)->uResv26 |=  (uint32_t)  (pFromData)[123] << 24;

  /* Unpack Member: uResv27 */
  (pToStruct)->uResv27 =  (uint32_t)  (pFromData)[124] ;
  (pToStruct)->uResv27 |=  (uint32_t)  (pFromData)[125] << 8;
  (pToStruct)->uResv27 |=  (uint32_t)  (pFromData)[126] << 16;
  (pToStruct)->uResv27 |=  (uint32_t)  (pFromData)[127] << 24;

  /* Unpack Member: uSb0Pattern0_31_0 */
  (pToStruct)->uSb0Pattern0_31_0 =  (uint32_t)  (pFromData)[128] ;
  (pToStruct)->uSb0Pattern0_31_0 |=  (uint32_t)  (pFromData)[129] << 8;
  (pToStruct)->uSb0Pattern0_31_0 |=  (uint32_t)  (pFromData)[130] << 16;
  (pToStruct)->uSb0Pattern0_31_0 |=  (uint32_t)  (pFromData)[131] << 24;

  /* Unpack Member: uSb0Pattern0_63_32 */
  (pToStruct)->uSb0Pattern0_63_32 =  (uint32_t)  (pFromData)[132] ;
  (pToStruct)->uSb0Pattern0_63_32 |=  (uint32_t)  (pFromData)[133] << 8;
  (pToStruct)->uSb0Pattern0_63_32 |=  (uint32_t)  (pFromData)[134] << 16;
  (pToStruct)->uSb0Pattern0_63_32 |=  (uint32_t)  (pFromData)[135] << 24;

  /* Unpack Member: uSb0Pattern0_95_64 */
  (pToStruct)->uSb0Pattern0_95_64 =  (uint32_t)  (pFromData)[136] ;
  (pToStruct)->uSb0Pattern0_95_64 |=  (uint32_t)  (pFromData)[137] << 8;
  (pToStruct)->uSb0Pattern0_95_64 |=  (uint32_t)  (pFromData)[138] << 16;
  (pToStruct)->uSb0Pattern0_95_64 |=  (uint32_t)  (pFromData)[139] << 24;

  /* Unpack Member: uSb0Pattern0_127_96 */
  (pToStruct)->uSb0Pattern0_127_96 =  (uint32_t)  (pFromData)[140] ;
  (pToStruct)->uSb0Pattern0_127_96 |=  (uint32_t)  (pFromData)[141] << 8;
  (pToStruct)->uSb0Pattern0_127_96 |=  (uint32_t)  (pFromData)[142] << 16;
  (pToStruct)->uSb0Pattern0_127_96 |=  (uint32_t)  (pFromData)[143] << 24;

  /* Unpack Member: uSb0Pattern1_31_0 */
  (pToStruct)->uSb0Pattern1_31_0 =  (uint32_t)  (pFromData)[144] ;
  (pToStruct)->uSb0Pattern1_31_0 |=  (uint32_t)  (pFromData)[145] << 8;
  (pToStruct)->uSb0Pattern1_31_0 |=  (uint32_t)  (pFromData)[146] << 16;
  (pToStruct)->uSb0Pattern1_31_0 |=  (uint32_t)  (pFromData)[147] << 24;

  /* Unpack Member: uSb0Pattern1_63_32 */
  (pToStruct)->uSb0Pattern1_63_32 =  (uint32_t)  (pFromData)[148] ;
  (pToStruct)->uSb0Pattern1_63_32 |=  (uint32_t)  (pFromData)[149] << 8;
  (pToStruct)->uSb0Pattern1_63_32 |=  (uint32_t)  (pFromData)[150] << 16;
  (pToStruct)->uSb0Pattern1_63_32 |=  (uint32_t)  (pFromData)[151] << 24;

  /* Unpack Member: uSb0Pattern1_95_64 */
  (pToStruct)->uSb0Pattern1_95_64 =  (uint32_t)  (pFromData)[152] ;
  (pToStruct)->uSb0Pattern1_95_64 |=  (uint32_t)  (pFromData)[153] << 8;
  (pToStruct)->uSb0Pattern1_95_64 |=  (uint32_t)  (pFromData)[154] << 16;
  (pToStruct)->uSb0Pattern1_95_64 |=  (uint32_t)  (pFromData)[155] << 24;

  /* Unpack Member: uSb0Pattern1_127_96 */
  (pToStruct)->uSb0Pattern1_127_96 =  (uint32_t)  (pFromData)[156] ;
  (pToStruct)->uSb0Pattern1_127_96 |=  (uint32_t)  (pFromData)[157] << 8;
  (pToStruct)->uSb0Pattern1_127_96 |=  (uint32_t)  (pFromData)[158] << 16;
  (pToStruct)->uSb0Pattern1_127_96 |=  (uint32_t)  (pFromData)[159] << 24;

  /* Unpack Member: uSb0Pattern2_31_0 */
  (pToStruct)->uSb0Pattern2_31_0 =  (uint32_t)  (pFromData)[160] ;
  (pToStruct)->uSb0Pattern2_31_0 |=  (uint32_t)  (pFromData)[161] << 8;
  (pToStruct)->uSb0Pattern2_31_0 |=  (uint32_t)  (pFromData)[162] << 16;
  (pToStruct)->uSb0Pattern2_31_0 |=  (uint32_t)  (pFromData)[163] << 24;

  /* Unpack Member: uSb0Pattern2_63_32 */
  (pToStruct)->uSb0Pattern2_63_32 =  (uint32_t)  (pFromData)[164] ;
  (pToStruct)->uSb0Pattern2_63_32 |=  (uint32_t)  (pFromData)[165] << 8;
  (pToStruct)->uSb0Pattern2_63_32 |=  (uint32_t)  (pFromData)[166] << 16;
  (pToStruct)->uSb0Pattern2_63_32 |=  (uint32_t)  (pFromData)[167] << 24;

  /* Unpack Member: uSb0Pattern2_95_64 */
  (pToStruct)->uSb0Pattern2_95_64 =  (uint32_t)  (pFromData)[168] ;
  (pToStruct)->uSb0Pattern2_95_64 |=  (uint32_t)  (pFromData)[169] << 8;
  (pToStruct)->uSb0Pattern2_95_64 |=  (uint32_t)  (pFromData)[170] << 16;
  (pToStruct)->uSb0Pattern2_95_64 |=  (uint32_t)  (pFromData)[171] << 24;

  /* Unpack Member: uSb0Pattern2_127_96 */
  (pToStruct)->uSb0Pattern2_127_96 =  (uint32_t)  (pFromData)[172] ;
  (pToStruct)->uSb0Pattern2_127_96 |=  (uint32_t)  (pFromData)[173] << 8;
  (pToStruct)->uSb0Pattern2_127_96 |=  (uint32_t)  (pFromData)[174] << 16;
  (pToStruct)->uSb0Pattern2_127_96 |=  (uint32_t)  (pFromData)[175] << 24;

  /* Unpack Member: uSb0Pattern3_31_0 */
  (pToStruct)->uSb0Pattern3_31_0 =  (uint32_t)  (pFromData)[176] ;
  (pToStruct)->uSb0Pattern3_31_0 |=  (uint32_t)  (pFromData)[177] << 8;
  (pToStruct)->uSb0Pattern3_31_0 |=  (uint32_t)  (pFromData)[178] << 16;
  (pToStruct)->uSb0Pattern3_31_0 |=  (uint32_t)  (pFromData)[179] << 24;

  /* Unpack Member: uSb0Pattern3_63_32 */
  (pToStruct)->uSb0Pattern3_63_32 =  (uint32_t)  (pFromData)[180] ;
  (pToStruct)->uSb0Pattern3_63_32 |=  (uint32_t)  (pFromData)[181] << 8;
  (pToStruct)->uSb0Pattern3_63_32 |=  (uint32_t)  (pFromData)[182] << 16;
  (pToStruct)->uSb0Pattern3_63_32 |=  (uint32_t)  (pFromData)[183] << 24;

  /* Unpack Member: uSb0Pattern3_95_64 */
  (pToStruct)->uSb0Pattern3_95_64 =  (uint32_t)  (pFromData)[184] ;
  (pToStruct)->uSb0Pattern3_95_64 |=  (uint32_t)  (pFromData)[185] << 8;
  (pToStruct)->uSb0Pattern3_95_64 |=  (uint32_t)  (pFromData)[186] << 16;
  (pToStruct)->uSb0Pattern3_95_64 |=  (uint32_t)  (pFromData)[187] << 24;

  /* Unpack Member: uSb0Pattern3_127_96 */
  (pToStruct)->uSb0Pattern3_127_96 =  (uint32_t)  (pFromData)[188] ;
  (pToStruct)->uSb0Pattern3_127_96 |=  (uint32_t)  (pFromData)[189] << 8;
  (pToStruct)->uSb0Pattern3_127_96 |=  (uint32_t)  (pFromData)[190] << 16;
  (pToStruct)->uSb0Pattern3_127_96 |=  (uint32_t)  (pFromData)[191] << 24;

  /* Unpack Member: uSb1Pattern0_31_0 */
  (pToStruct)->uSb1Pattern0_31_0 =  (uint32_t)  (pFromData)[192] ;
  (pToStruct)->uSb1Pattern0_31_0 |=  (uint32_t)  (pFromData)[193] << 8;
  (pToStruct)->uSb1Pattern0_31_0 |=  (uint32_t)  (pFromData)[194] << 16;
  (pToStruct)->uSb1Pattern0_31_0 |=  (uint32_t)  (pFromData)[195] << 24;

  /* Unpack Member: uSb1Pattern0_63_32 */
  (pToStruct)->uSb1Pattern0_63_32 =  (uint32_t)  (pFromData)[196] ;
  (pToStruct)->uSb1Pattern0_63_32 |=  (uint32_t)  (pFromData)[197] << 8;
  (pToStruct)->uSb1Pattern0_63_32 |=  (uint32_t)  (pFromData)[198] << 16;
  (pToStruct)->uSb1Pattern0_63_32 |=  (uint32_t)  (pFromData)[199] << 24;

  /* Unpack Member: uSb1Pattern0_95_64 */
  (pToStruct)->uSb1Pattern0_95_64 =  (uint32_t)  (pFromData)[200] ;
  (pToStruct)->uSb1Pattern0_95_64 |=  (uint32_t)  (pFromData)[201] << 8;
  (pToStruct)->uSb1Pattern0_95_64 |=  (uint32_t)  (pFromData)[202] << 16;
  (pToStruct)->uSb1Pattern0_95_64 |=  (uint32_t)  (pFromData)[203] << 24;

  /* Unpack Member: uSb1Pattern0_127_96 */
  (pToStruct)->uSb1Pattern0_127_96 =  (uint32_t)  (pFromData)[204] ;
  (pToStruct)->uSb1Pattern0_127_96 |=  (uint32_t)  (pFromData)[205] << 8;
  (pToStruct)->uSb1Pattern0_127_96 |=  (uint32_t)  (pFromData)[206] << 16;
  (pToStruct)->uSb1Pattern0_127_96 |=  (uint32_t)  (pFromData)[207] << 24;

  /* Unpack Member: uSb1Pattern1_31_0 */
  (pToStruct)->uSb1Pattern1_31_0 =  (uint32_t)  (pFromData)[208] ;
  (pToStruct)->uSb1Pattern1_31_0 |=  (uint32_t)  (pFromData)[209] << 8;
  (pToStruct)->uSb1Pattern1_31_0 |=  (uint32_t)  (pFromData)[210] << 16;
  (pToStruct)->uSb1Pattern1_31_0 |=  (uint32_t)  (pFromData)[211] << 24;

  /* Unpack Member: uSb1Pattern1_63_32 */
  (pToStruct)->uSb1Pattern1_63_32 =  (uint32_t)  (pFromData)[212] ;
  (pToStruct)->uSb1Pattern1_63_32 |=  (uint32_t)  (pFromData)[213] << 8;
  (pToStruct)->uSb1Pattern1_63_32 |=  (uint32_t)  (pFromData)[214] << 16;
  (pToStruct)->uSb1Pattern1_63_32 |=  (uint32_t)  (pFromData)[215] << 24;

  /* Unpack Member: uSb1Pattern1_95_64 */
  (pToStruct)->uSb1Pattern1_95_64 =  (uint32_t)  (pFromData)[216] ;
  (pToStruct)->uSb1Pattern1_95_64 |=  (uint32_t)  (pFromData)[217] << 8;
  (pToStruct)->uSb1Pattern1_95_64 |=  (uint32_t)  (pFromData)[218] << 16;
  (pToStruct)->uSb1Pattern1_95_64 |=  (uint32_t)  (pFromData)[219] << 24;

  /* Unpack Member: uSb1Pattern1_127_96 */
  (pToStruct)->uSb1Pattern1_127_96 =  (uint32_t)  (pFromData)[220] ;
  (pToStruct)->uSb1Pattern1_127_96 |=  (uint32_t)  (pFromData)[221] << 8;
  (pToStruct)->uSb1Pattern1_127_96 |=  (uint32_t)  (pFromData)[222] << 16;
  (pToStruct)->uSb1Pattern1_127_96 |=  (uint32_t)  (pFromData)[223] << 24;

  /* Unpack Member: uSb1Pattern2_31_0 */
  (pToStruct)->uSb1Pattern2_31_0 =  (uint32_t)  (pFromData)[224] ;
  (pToStruct)->uSb1Pattern2_31_0 |=  (uint32_t)  (pFromData)[225] << 8;
  (pToStruct)->uSb1Pattern2_31_0 |=  (uint32_t)  (pFromData)[226] << 16;
  (pToStruct)->uSb1Pattern2_31_0 |=  (uint32_t)  (pFromData)[227] << 24;

  /* Unpack Member: uSb1Pattern2_63_32 */
  (pToStruct)->uSb1Pattern2_63_32 =  (uint32_t)  (pFromData)[228] ;
  (pToStruct)->uSb1Pattern2_63_32 |=  (uint32_t)  (pFromData)[229] << 8;
  (pToStruct)->uSb1Pattern2_63_32 |=  (uint32_t)  (pFromData)[230] << 16;
  (pToStruct)->uSb1Pattern2_63_32 |=  (uint32_t)  (pFromData)[231] << 24;

  /* Unpack Member: uSb1Pattern2_95_64 */
  (pToStruct)->uSb1Pattern2_95_64 =  (uint32_t)  (pFromData)[232] ;
  (pToStruct)->uSb1Pattern2_95_64 |=  (uint32_t)  (pFromData)[233] << 8;
  (pToStruct)->uSb1Pattern2_95_64 |=  (uint32_t)  (pFromData)[234] << 16;
  (pToStruct)->uSb1Pattern2_95_64 |=  (uint32_t)  (pFromData)[235] << 24;

  /* Unpack Member: uSb1Pattern2_127_96 */
  (pToStruct)->uSb1Pattern2_127_96 =  (uint32_t)  (pFromData)[236] ;
  (pToStruct)->uSb1Pattern2_127_96 |=  (uint32_t)  (pFromData)[237] << 8;
  (pToStruct)->uSb1Pattern2_127_96 |=  (uint32_t)  (pFromData)[238] << 16;
  (pToStruct)->uSb1Pattern2_127_96 |=  (uint32_t)  (pFromData)[239] << 24;

  /* Unpack Member: uSb1Pattern3_31_0 */
  (pToStruct)->uSb1Pattern3_31_0 =  (uint32_t)  (pFromData)[240] ;
  (pToStruct)->uSb1Pattern3_31_0 |=  (uint32_t)  (pFromData)[241] << 8;
  (pToStruct)->uSb1Pattern3_31_0 |=  (uint32_t)  (pFromData)[242] << 16;
  (pToStruct)->uSb1Pattern3_31_0 |=  (uint32_t)  (pFromData)[243] << 24;

  /* Unpack Member: uSb1Pattern3_63_32 */
  (pToStruct)->uSb1Pattern3_63_32 =  (uint32_t)  (pFromData)[244] ;
  (pToStruct)->uSb1Pattern3_63_32 |=  (uint32_t)  (pFromData)[245] << 8;
  (pToStruct)->uSb1Pattern3_63_32 |=  (uint32_t)  (pFromData)[246] << 16;
  (pToStruct)->uSb1Pattern3_63_32 |=  (uint32_t)  (pFromData)[247] << 24;

  /* Unpack Member: uSb1Pattern3_95_64 */
  (pToStruct)->uSb1Pattern3_95_64 =  (uint32_t)  (pFromData)[248] ;
  (pToStruct)->uSb1Pattern3_95_64 |=  (uint32_t)  (pFromData)[249] << 8;
  (pToStruct)->uSb1Pattern3_95_64 |=  (uint32_t)  (pFromData)[250] << 16;
  (pToStruct)->uSb1Pattern3_95_64 |=  (uint32_t)  (pFromData)[251] << 24;

  /* Unpack Member: uSb1Pattern3_127_96 */
  (pToStruct)->uSb1Pattern3_127_96 =  (uint32_t)  (pFromData)[252] ;
  (pToStruct)->uSb1Pattern3_127_96 |=  (uint32_t)  (pFromData)[253] << 8;
  (pToStruct)->uSb1Pattern3_127_96 |=  (uint32_t)  (pFromData)[254] << 16;
  (pToStruct)->uSb1Pattern3_127_96 |=  (uint32_t)  (pFromData)[255] << 24;

}



/* initialize an instance of this zframe */
void
sbZfFe2000RcDmaFormat_InitInstance(sbZfFe2000RcDmaFormat_t *pFrame) {

  pFrame->uInstruction0 =  (unsigned int)  0;
  pFrame->uInstruction1 =  (unsigned int)  0;
  pFrame->uInstruction2 =  (unsigned int)  0;
  pFrame->uInstruction3 =  (unsigned int)  0;
  pFrame->uResv0 =  (unsigned int)  0;
  pFrame->uResv1 =  (unsigned int)  0;
  pFrame->uResv2 =  (unsigned int)  0;
  pFrame->uResv3 =  (unsigned int)  0;
  pFrame->uResv4 =  (unsigned int)  0;
  pFrame->uResv5 =  (unsigned int)  0;
  pFrame->uResv6 =  (unsigned int)  0;
  pFrame->uResv7 =  (unsigned int)  0;
  pFrame->uResv8 =  (unsigned int)  0;
  pFrame->uResv9 =  (unsigned int)  0;
  pFrame->uResv10 =  (unsigned int)  0;
  pFrame->uResv11 =  (unsigned int)  0;
  pFrame->uResv12 =  (unsigned int)  0;
  pFrame->uResv13 =  (unsigned int)  0;
  pFrame->uResv14 =  (unsigned int)  0;
  pFrame->uResv15 =  (unsigned int)  0;
  pFrame->uResv16 =  (unsigned int)  0;
  pFrame->uResv17 =  (unsigned int)  0;
  pFrame->uResv18 =  (unsigned int)  0;
  pFrame->uResv19 =  (unsigned int)  0;
  pFrame->uResv20 =  (unsigned int)  0;
  pFrame->uResv21 =  (unsigned int)  0;
  pFrame->uResv22 =  (unsigned int)  0;
  pFrame->uResv23 =  (unsigned int)  0;
  pFrame->uResv24 =  (unsigned int)  0;
  pFrame->uResv25 =  (unsigned int)  0;
  pFrame->uResv26 =  (unsigned int)  0;
  pFrame->uResv27 =  (unsigned int)  0;
  pFrame->uSb0Pattern0_31_0 =  (unsigned int)  0;
  pFrame->uSb0Pattern0_63_32 =  (unsigned int)  0;
  pFrame->uSb0Pattern0_95_64 =  (unsigned int)  0;
  pFrame->uSb0Pattern0_127_96 =  (unsigned int)  0;
  pFrame->uSb0Pattern1_31_0 =  (unsigned int)  0;
  pFrame->uSb0Pattern1_63_32 =  (unsigned int)  0;
  pFrame->uSb0Pattern1_95_64 =  (unsigned int)  0;
  pFrame->uSb0Pattern1_127_96 =  (unsigned int)  0;
  pFrame->uSb0Pattern2_31_0 =  (unsigned int)  0;
  pFrame->uSb0Pattern2_63_32 =  (unsigned int)  0;
  pFrame->uSb0Pattern2_95_64 =  (unsigned int)  0;
  pFrame->uSb0Pattern2_127_96 =  (unsigned int)  0;
  pFrame->uSb0Pattern3_31_0 =  (unsigned int)  0;
  pFrame->uSb0Pattern3_63_32 =  (unsigned int)  0;
  pFrame->uSb0Pattern3_95_64 =  (unsigned int)  0;
  pFrame->uSb0Pattern3_127_96 =  (unsigned int)  0;
  pFrame->uSb1Pattern0_31_0 =  (unsigned int)  0;
  pFrame->uSb1Pattern0_63_32 =  (unsigned int)  0;
  pFrame->uSb1Pattern0_95_64 =  (unsigned int)  0;
  pFrame->uSb1Pattern0_127_96 =  (unsigned int)  0;
  pFrame->uSb1Pattern1_31_0 =  (unsigned int)  0;
  pFrame->uSb1Pattern1_63_32 =  (unsigned int)  0;
  pFrame->uSb1Pattern1_95_64 =  (unsigned int)  0;
  pFrame->uSb1Pattern1_127_96 =  (unsigned int)  0;
  pFrame->uSb1Pattern2_31_0 =  (unsigned int)  0;
  pFrame->uSb1Pattern2_63_32 =  (unsigned int)  0;
  pFrame->uSb1Pattern2_95_64 =  (unsigned int)  0;
  pFrame->uSb1Pattern2_127_96 =  (unsigned int)  0;
  pFrame->uSb1Pattern3_31_0 =  (unsigned int)  0;
  pFrame->uSb1Pattern3_63_32 =  (unsigned int)  0;
  pFrame->uSb1Pattern3_95_64 =  (unsigned int)  0;
  pFrame->uSb1Pattern3_127_96 =  (unsigned int)  0;

}



void sbZfFe2000RcDmaFormatAlias(sbZfFe2000RcDmaFormat_t *pFrom,
                                sbFe2000RcDmaFormatAlias_t *pTo)
{
  pTo->pInstruction[0] = &pFrom->uInstruction0;
  pTo->pInstruction[1] = &pFrom->uInstruction1;
  pTo->pInstruction[2] = &pFrom->uInstruction2;
  pTo->pInstruction[3] = &pFrom->uInstruction3;

  /* sb0 instruction 0 */
  pTo->pPattern[0] = &pFrom->uSb0Pattern0_31_0;
  pTo->pPattern[1] = &pFrom->uSb0Pattern0_63_32;
  pTo->pPattern[2] = &pFrom->uSb0Pattern0_95_64;
  pTo->pPattern[3] = &pFrom->uSb0Pattern0_127_96;

  /* sb0 instruction 1 */
  pTo->pPattern[4] = &pFrom->uSb0Pattern1_31_0;
  pTo->pPattern[5] = &pFrom->uSb0Pattern1_63_32;
  pTo->pPattern[6] = &pFrom->uSb0Pattern1_95_64;
  pTo->pPattern[7] = &pFrom->uSb0Pattern1_127_96;

  /* sb0 instruction 2 */
  pTo->pPattern[8]  = &pFrom->uSb0Pattern2_31_0;
  pTo->pPattern[9]  = &pFrom->uSb0Pattern2_63_32;
  pTo->pPattern[10] = &pFrom->uSb0Pattern2_95_64;
  pTo->pPattern[11] = &pFrom->uSb0Pattern2_127_96;

  /* sb0 instruction 3 */
  pTo->pPattern[12]   = &pFrom->uSb0Pattern3_31_0;
  pTo->pPattern[13]   = &pFrom->uSb0Pattern3_63_32;
  pTo->pPattern[14]   = &pFrom->uSb0Pattern3_95_64;
  pTo->pPattern[15]   = &pFrom->uSb0Pattern3_127_96;

  /* sb1 instruction 0 */
  pTo->pPattern[16] = &pFrom->uSb1Pattern0_31_0;
  pTo->pPattern[17] = &pFrom->uSb1Pattern0_63_32;
  pTo->pPattern[18] = &pFrom->uSb1Pattern0_95_64;
  pTo->pPattern[19] = &pFrom->uSb1Pattern0_127_96;

  /* sb1 instruction 1 */
  pTo->pPattern[20] = &pFrom->uSb1Pattern1_31_0;
  pTo->pPattern[21] = &pFrom->uSb1Pattern1_63_32;
  pTo->pPattern[22] = &pFrom->uSb1Pattern1_95_64;
  pTo->pPattern[23] = &pFrom->uSb1Pattern1_127_96;

  /* sb1 instruction 2 */
  pTo->pPattern[24]  = &pFrom->uSb1Pattern2_31_0;
  pTo->pPattern[25]  = &pFrom->uSb1Pattern2_63_32;
  pTo->pPattern[26]  = &pFrom->uSb1Pattern2_95_64;
  pTo->pPattern[27]  = &pFrom->uSb1Pattern2_127_96;

  /* sb1 instruction 3 */
  pTo->pPattern[28]   = &pFrom->uSb1Pattern3_31_0;
  pTo->pPattern[29]   = &pFrom->uSb1Pattern3_63_32;
  pTo->pPattern[30]   = &pFrom->uSb1Pattern3_95_64;
  pTo->pPattern[31]   = &pFrom->uSb1Pattern3_127_96;
}

void sbZfFe2000RcDmaBlock2DmaFormat(sbFe2000RcDmaBlock_t *pFrom,
                                    sbZfFe2000RcDmaFormat_t *pToData)
{
  pToData->uInstruction0 = pFrom->uInstruction[0];
  pToData->uInstruction1 = pFrom->uInstruction[1];
  pToData->uInstruction2 = pFrom->uInstruction[2];
  pToData->uInstruction3 = pFrom->uInstruction[3];
  

  /* sb0 pattern 0 */
  pToData->uSb0Pattern0_31_0   = pFrom->uPattern[0];
  pToData->uSb0Pattern0_63_32  = pFrom->uPattern[1];
  pToData->uSb0Pattern0_95_64  = pFrom->uPattern[2];
  pToData->uSb0Pattern0_127_96 = pFrom->uPattern[3];


  /* sb0 pattern 1 */
  pToData->uSb0Pattern1_31_0   = pFrom->uPattern[4];
  pToData->uSb0Pattern1_63_32  = pFrom->uPattern[5];
  pToData->uSb0Pattern1_95_64  = pFrom->uPattern[6];
  pToData->uSb0Pattern1_127_96 = pFrom->uPattern[7];

  /* sb0 pattern 2 */
  pToData->uSb0Pattern2_31_0   = pFrom->uPattern[8];
  pToData->uSb0Pattern2_63_32  = pFrom->uPattern[9];
  pToData->uSb0Pattern2_95_64  = pFrom->uPattern[10];
  pToData->uSb0Pattern2_127_96 = pFrom->uPattern[11];

  /* sb0 pattern 3 */
  pToData->uSb0Pattern3_31_0   = pFrom->uPattern[12];
  pToData->uSb0Pattern3_63_32  = pFrom->uPattern[13];
  pToData->uSb0Pattern3_95_64  = pFrom->uPattern[14];
  pToData->uSb0Pattern3_127_96 = pFrom->uPattern[15];

  /* sb1 pattern 0 */
  pToData->uSb0Pattern0_31_0   = pFrom->uPattern[16];
  pToData->uSb0Pattern0_63_32  = pFrom->uPattern[17];
  pToData->uSb0Pattern0_95_64  = pFrom->uPattern[18];
  pToData->uSb0Pattern0_127_96 = pFrom->uPattern[19];

  /* sb1 pattern 1 */
  pToData->uSb0Pattern1_31_0   = pFrom->uPattern[20];
  pToData->uSb0Pattern1_63_32  = pFrom->uPattern[21];
  pToData->uSb0Pattern1_95_64  = pFrom->uPattern[22];
  pToData->uSb0Pattern1_127_96 = pFrom->uPattern[23];

  /* sb1 pattern 2 */
  pToData->uSb0Pattern2_31_0   = pFrom->uPattern[24];
  pToData->uSb0Pattern2_63_32  = pFrom->uPattern[25];
  pToData->uSb0Pattern2_95_64  = pFrom->uPattern[26];
  pToData->uSb0Pattern2_127_96 = pFrom->uPattern[27];

  /* sb1 pattern 3 */
  pToData->uSb0Pattern3_31_0   = pFrom->uPattern[28];
  pToData->uSb0Pattern3_63_32  = pFrom->uPattern[29];
  pToData->uSb0Pattern3_95_64  = pFrom->uPattern[30];
  pToData->uSb0Pattern3_127_96 = pFrom->uPattern[31];
}
