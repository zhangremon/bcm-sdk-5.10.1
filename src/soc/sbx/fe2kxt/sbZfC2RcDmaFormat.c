/*
 * $Id: sbZfC2RcDmaFormat.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfC2RcDmaFormat.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2RcDmaFormat_Pack(sbZfC2RcDmaFormat_t *pFrom,
                       uint8_t *pToData,
                       uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2RCDMAFORMAT_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uInstruction0 */
  (pToData)[0] |= ((pFrom)->m_uInstruction0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uInstruction0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uInstruction0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uInstruction0 >> 24) &0xFF;

  /* Pack Member: m_uInstruction1 */
  (pToData)[4] |= ((pFrom)->m_uInstruction1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uInstruction1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uInstruction1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uInstruction1 >> 24) &0xFF;

  /* Pack Member: m_uInstruction2 */
  (pToData)[8] |= ((pFrom)->m_uInstruction2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_uInstruction2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uInstruction2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uInstruction2 >> 24) &0xFF;

  /* Pack Member: m_uInstruction3 */
  (pToData)[12] |= ((pFrom)->m_uInstruction3) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_uInstruction3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uInstruction3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uInstruction3 >> 24) &0xFF;

  /* Pack Member: m_uInstruction4 */
  (pToData)[16] |= ((pFrom)->m_uInstruction4) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_uInstruction4 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uInstruction4 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uInstruction4 >> 24) &0xFF;

  /* Pack Member: m_uInstruction5 */
  (pToData)[20] |= ((pFrom)->m_uInstruction5) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_uInstruction5 >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uInstruction5 >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->m_uInstruction5 >> 24) &0xFF;

  /* Pack Member: m_uInstruction6 */
  (pToData)[24] |= ((pFrom)->m_uInstruction6) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_uInstruction6 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_uInstruction6 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_uInstruction6 >> 24) &0xFF;

  /* Pack Member: m_uInstruction7 */
  (pToData)[28] |= ((pFrom)->m_uInstruction7) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_uInstruction7 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_uInstruction7 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_uInstruction7 >> 24) &0xFF;

  /* Pack Member: m_uResv0 */
  (pToData)[32] |= ((pFrom)->m_uResv0) & 0xFF;
  (pToData)[33] |= ((pFrom)->m_uResv0 >> 8) &0xFF;
  (pToData)[34] |= ((pFrom)->m_uResv0 >> 16) &0xFF;
  (pToData)[35] |= ((pFrom)->m_uResv0 >> 24) &0xFF;

  /* Pack Member: m_uResv1 */
  (pToData)[36] |= ((pFrom)->m_uResv1) & 0xFF;
  (pToData)[37] |= ((pFrom)->m_uResv1 >> 8) &0xFF;
  (pToData)[38] |= ((pFrom)->m_uResv1 >> 16) &0xFF;
  (pToData)[39] |= ((pFrom)->m_uResv1 >> 24) &0xFF;

  /* Pack Member: m_uResv2 */
  (pToData)[40] |= ((pFrom)->m_uResv2) & 0xFF;
  (pToData)[41] |= ((pFrom)->m_uResv2 >> 8) &0xFF;
  (pToData)[42] |= ((pFrom)->m_uResv2 >> 16) &0xFF;
  (pToData)[43] |= ((pFrom)->m_uResv2 >> 24) &0xFF;

  /* Pack Member: m_uResv3 */
  (pToData)[44] |= ((pFrom)->m_uResv3) & 0xFF;
  (pToData)[45] |= ((pFrom)->m_uResv3 >> 8) &0xFF;
  (pToData)[46] |= ((pFrom)->m_uResv3 >> 16) &0xFF;
  (pToData)[47] |= ((pFrom)->m_uResv3 >> 24) &0xFF;

  /* Pack Member: m_uResv4 */
  (pToData)[48] |= ((pFrom)->m_uResv4) & 0xFF;
  (pToData)[49] |= ((pFrom)->m_uResv4 >> 8) &0xFF;
  (pToData)[50] |= ((pFrom)->m_uResv4 >> 16) &0xFF;
  (pToData)[51] |= ((pFrom)->m_uResv4 >> 24) &0xFF;

  /* Pack Member: m_uResv5 */
  (pToData)[52] |= ((pFrom)->m_uResv5) & 0xFF;
  (pToData)[53] |= ((pFrom)->m_uResv5 >> 8) &0xFF;
  (pToData)[54] |= ((pFrom)->m_uResv5 >> 16) &0xFF;
  (pToData)[55] |= ((pFrom)->m_uResv5 >> 24) &0xFF;

  /* Pack Member: m_uResv6 */
  (pToData)[56] |= ((pFrom)->m_uResv6) & 0xFF;
  (pToData)[57] |= ((pFrom)->m_uResv6 >> 8) &0xFF;
  (pToData)[58] |= ((pFrom)->m_uResv6 >> 16) &0xFF;
  (pToData)[59] |= ((pFrom)->m_uResv6 >> 24) &0xFF;

  /* Pack Member: m_uResv7 */
  (pToData)[60] |= ((pFrom)->m_uResv7) & 0xFF;
  (pToData)[61] |= ((pFrom)->m_uResv7 >> 8) &0xFF;
  (pToData)[62] |= ((pFrom)->m_uResv7 >> 16) &0xFF;
  (pToData)[63] |= ((pFrom)->m_uResv7 >> 24) &0xFF;

  /* Pack Member: m_uResv8 */
  (pToData)[64] |= ((pFrom)->m_uResv8) & 0xFF;
  (pToData)[65] |= ((pFrom)->m_uResv8 >> 8) &0xFF;
  (pToData)[66] |= ((pFrom)->m_uResv8 >> 16) &0xFF;
  (pToData)[67] |= ((pFrom)->m_uResv8 >> 24) &0xFF;

  /* Pack Member: m_uResv9 */
  (pToData)[68] |= ((pFrom)->m_uResv9) & 0xFF;
  (pToData)[69] |= ((pFrom)->m_uResv9 >> 8) &0xFF;
  (pToData)[70] |= ((pFrom)->m_uResv9 >> 16) &0xFF;
  (pToData)[71] |= ((pFrom)->m_uResv9 >> 24) &0xFF;

  /* Pack Member: m_uResv10 */
  (pToData)[72] |= ((pFrom)->m_uResv10) & 0xFF;
  (pToData)[73] |= ((pFrom)->m_uResv10 >> 8) &0xFF;
  (pToData)[74] |= ((pFrom)->m_uResv10 >> 16) &0xFF;
  (pToData)[75] |= ((pFrom)->m_uResv10 >> 24) &0xFF;

  /* Pack Member: m_uResv11 */
  (pToData)[76] |= ((pFrom)->m_uResv11) & 0xFF;
  (pToData)[77] |= ((pFrom)->m_uResv11 >> 8) &0xFF;
  (pToData)[78] |= ((pFrom)->m_uResv11 >> 16) &0xFF;
  (pToData)[79] |= ((pFrom)->m_uResv11 >> 24) &0xFF;

  /* Pack Member: m_uResv12 */
  (pToData)[80] |= ((pFrom)->m_uResv12) & 0xFF;
  (pToData)[81] |= ((pFrom)->m_uResv12 >> 8) &0xFF;
  (pToData)[82] |= ((pFrom)->m_uResv12 >> 16) &0xFF;
  (pToData)[83] |= ((pFrom)->m_uResv12 >> 24) &0xFF;

  /* Pack Member: m_uResv13 */
  (pToData)[84] |= ((pFrom)->m_uResv13) & 0xFF;
  (pToData)[85] |= ((pFrom)->m_uResv13 >> 8) &0xFF;
  (pToData)[86] |= ((pFrom)->m_uResv13 >> 16) &0xFF;
  (pToData)[87] |= ((pFrom)->m_uResv13 >> 24) &0xFF;

  /* Pack Member: m_uResv14 */
  (pToData)[88] |= ((pFrom)->m_uResv14) & 0xFF;
  (pToData)[89] |= ((pFrom)->m_uResv14 >> 8) &0xFF;
  (pToData)[90] |= ((pFrom)->m_uResv14 >> 16) &0xFF;
  (pToData)[91] |= ((pFrom)->m_uResv14 >> 24) &0xFF;

  /* Pack Member: m_uResv15 */
  (pToData)[92] |= ((pFrom)->m_uResv15) & 0xFF;
  (pToData)[93] |= ((pFrom)->m_uResv15 >> 8) &0xFF;
  (pToData)[94] |= ((pFrom)->m_uResv15 >> 16) &0xFF;
  (pToData)[95] |= ((pFrom)->m_uResv15 >> 24) &0xFF;

  /* Pack Member: m_uResv16 */
  (pToData)[96] |= ((pFrom)->m_uResv16) & 0xFF;
  (pToData)[97] |= ((pFrom)->m_uResv16 >> 8) &0xFF;
  (pToData)[98] |= ((pFrom)->m_uResv16 >> 16) &0xFF;
  (pToData)[99] |= ((pFrom)->m_uResv16 >> 24) &0xFF;

  /* Pack Member: m_uResv17 */
  (pToData)[100] |= ((pFrom)->m_uResv17) & 0xFF;
  (pToData)[101] |= ((pFrom)->m_uResv17 >> 8) &0xFF;
  (pToData)[102] |= ((pFrom)->m_uResv17 >> 16) &0xFF;
  (pToData)[103] |= ((pFrom)->m_uResv17 >> 24) &0xFF;

  /* Pack Member: m_uResv18 */
  (pToData)[104] |= ((pFrom)->m_uResv18) & 0xFF;
  (pToData)[105] |= ((pFrom)->m_uResv18 >> 8) &0xFF;
  (pToData)[106] |= ((pFrom)->m_uResv18 >> 16) &0xFF;
  (pToData)[107] |= ((pFrom)->m_uResv18 >> 24) &0xFF;

  /* Pack Member: m_uResv19 */
  (pToData)[108] |= ((pFrom)->m_uResv19) & 0xFF;
  (pToData)[109] |= ((pFrom)->m_uResv19 >> 8) &0xFF;
  (pToData)[110] |= ((pFrom)->m_uResv19 >> 16) &0xFF;
  (pToData)[111] |= ((pFrom)->m_uResv19 >> 24) &0xFF;

  /* Pack Member: m_uResv20 */
  (pToData)[112] |= ((pFrom)->m_uResv20) & 0xFF;
  (pToData)[113] |= ((pFrom)->m_uResv20 >> 8) &0xFF;
  (pToData)[114] |= ((pFrom)->m_uResv20 >> 16) &0xFF;
  (pToData)[115] |= ((pFrom)->m_uResv20 >> 24) &0xFF;

  /* Pack Member: m_uResv21 */
  (pToData)[116] |= ((pFrom)->m_uResv21) & 0xFF;
  (pToData)[117] |= ((pFrom)->m_uResv21 >> 8) &0xFF;
  (pToData)[118] |= ((pFrom)->m_uResv21 >> 16) &0xFF;
  (pToData)[119] |= ((pFrom)->m_uResv21 >> 24) &0xFF;

  /* Pack Member: m_uResv22 */
  (pToData)[120] |= ((pFrom)->m_uResv22) & 0xFF;
  (pToData)[121] |= ((pFrom)->m_uResv22 >> 8) &0xFF;
  (pToData)[122] |= ((pFrom)->m_uResv22 >> 16) &0xFF;
  (pToData)[123] |= ((pFrom)->m_uResv22 >> 24) &0xFF;

  /* Pack Member: m_uResv23 */
  (pToData)[124] |= ((pFrom)->m_uResv23) & 0xFF;
  (pToData)[125] |= ((pFrom)->m_uResv23 >> 8) &0xFF;
  (pToData)[126] |= ((pFrom)->m_uResv23 >> 16) &0xFF;
  (pToData)[127] |= ((pFrom)->m_uResv23 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern0_31_0 */
  (pToData)[128] |= ((pFrom)->m_uSb0Pattern0_31_0) & 0xFF;
  (pToData)[129] |= ((pFrom)->m_uSb0Pattern0_31_0 >> 8) &0xFF;
  (pToData)[130] |= ((pFrom)->m_uSb0Pattern0_31_0 >> 16) &0xFF;
  (pToData)[131] |= ((pFrom)->m_uSb0Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern0_63_32 */
  (pToData)[132] |= ((pFrom)->m_uSb0Pattern0_63_32) & 0xFF;
  (pToData)[133] |= ((pFrom)->m_uSb0Pattern0_63_32 >> 8) &0xFF;
  (pToData)[134] |= ((pFrom)->m_uSb0Pattern0_63_32 >> 16) &0xFF;
  (pToData)[135] |= ((pFrom)->m_uSb0Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern0_95_64 */
  (pToData)[136] |= ((pFrom)->m_uSb0Pattern0_95_64) & 0xFF;
  (pToData)[137] |= ((pFrom)->m_uSb0Pattern0_95_64 >> 8) &0xFF;
  (pToData)[138] |= ((pFrom)->m_uSb0Pattern0_95_64 >> 16) &0xFF;
  (pToData)[139] |= ((pFrom)->m_uSb0Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern0_127_96 */
  (pToData)[140] |= ((pFrom)->m_uSb0Pattern0_127_96) & 0xFF;
  (pToData)[141] |= ((pFrom)->m_uSb0Pattern0_127_96 >> 8) &0xFF;
  (pToData)[142] |= ((pFrom)->m_uSb0Pattern0_127_96 >> 16) &0xFF;
  (pToData)[143] |= ((pFrom)->m_uSb0Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern1_31_0 */
  (pToData)[144] |= ((pFrom)->m_uSb0Pattern1_31_0) & 0xFF;
  (pToData)[145] |= ((pFrom)->m_uSb0Pattern1_31_0 >> 8) &0xFF;
  (pToData)[146] |= ((pFrom)->m_uSb0Pattern1_31_0 >> 16) &0xFF;
  (pToData)[147] |= ((pFrom)->m_uSb0Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern1_63_32 */
  (pToData)[148] |= ((pFrom)->m_uSb0Pattern1_63_32) & 0xFF;
  (pToData)[149] |= ((pFrom)->m_uSb0Pattern1_63_32 >> 8) &0xFF;
  (pToData)[150] |= ((pFrom)->m_uSb0Pattern1_63_32 >> 16) &0xFF;
  (pToData)[151] |= ((pFrom)->m_uSb0Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern1_95_64 */
  (pToData)[152] |= ((pFrom)->m_uSb0Pattern1_95_64) & 0xFF;
  (pToData)[153] |= ((pFrom)->m_uSb0Pattern1_95_64 >> 8) &0xFF;
  (pToData)[154] |= ((pFrom)->m_uSb0Pattern1_95_64 >> 16) &0xFF;
  (pToData)[155] |= ((pFrom)->m_uSb0Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern1_127_96 */
  (pToData)[156] |= ((pFrom)->m_uSb0Pattern1_127_96) & 0xFF;
  (pToData)[157] |= ((pFrom)->m_uSb0Pattern1_127_96 >> 8) &0xFF;
  (pToData)[158] |= ((pFrom)->m_uSb0Pattern1_127_96 >> 16) &0xFF;
  (pToData)[159] |= ((pFrom)->m_uSb0Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern2_31_0 */
  (pToData)[160] |= ((pFrom)->m_uSb0Pattern2_31_0) & 0xFF;
  (pToData)[161] |= ((pFrom)->m_uSb0Pattern2_31_0 >> 8) &0xFF;
  (pToData)[162] |= ((pFrom)->m_uSb0Pattern2_31_0 >> 16) &0xFF;
  (pToData)[163] |= ((pFrom)->m_uSb0Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern2_63_32 */
  (pToData)[164] |= ((pFrom)->m_uSb0Pattern2_63_32) & 0xFF;
  (pToData)[165] |= ((pFrom)->m_uSb0Pattern2_63_32 >> 8) &0xFF;
  (pToData)[166] |= ((pFrom)->m_uSb0Pattern2_63_32 >> 16) &0xFF;
  (pToData)[167] |= ((pFrom)->m_uSb0Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern2_95_64 */
  (pToData)[168] |= ((pFrom)->m_uSb0Pattern2_95_64) & 0xFF;
  (pToData)[169] |= ((pFrom)->m_uSb0Pattern2_95_64 >> 8) &0xFF;
  (pToData)[170] |= ((pFrom)->m_uSb0Pattern2_95_64 >> 16) &0xFF;
  (pToData)[171] |= ((pFrom)->m_uSb0Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern2_127_96 */
  (pToData)[172] |= ((pFrom)->m_uSb0Pattern2_127_96) & 0xFF;
  (pToData)[173] |= ((pFrom)->m_uSb0Pattern2_127_96 >> 8) &0xFF;
  (pToData)[174] |= ((pFrom)->m_uSb0Pattern2_127_96 >> 16) &0xFF;
  (pToData)[175] |= ((pFrom)->m_uSb0Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern3_31_0 */
  (pToData)[176] |= ((pFrom)->m_uSb0Pattern3_31_0) & 0xFF;
  (pToData)[177] |= ((pFrom)->m_uSb0Pattern3_31_0 >> 8) &0xFF;
  (pToData)[178] |= ((pFrom)->m_uSb0Pattern3_31_0 >> 16) &0xFF;
  (pToData)[179] |= ((pFrom)->m_uSb0Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern3_63_32 */
  (pToData)[180] |= ((pFrom)->m_uSb0Pattern3_63_32) & 0xFF;
  (pToData)[181] |= ((pFrom)->m_uSb0Pattern3_63_32 >> 8) &0xFF;
  (pToData)[182] |= ((pFrom)->m_uSb0Pattern3_63_32 >> 16) &0xFF;
  (pToData)[183] |= ((pFrom)->m_uSb0Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern3_95_64 */
  (pToData)[184] |= ((pFrom)->m_uSb0Pattern3_95_64) & 0xFF;
  (pToData)[185] |= ((pFrom)->m_uSb0Pattern3_95_64 >> 8) &0xFF;
  (pToData)[186] |= ((pFrom)->m_uSb0Pattern3_95_64 >> 16) &0xFF;
  (pToData)[187] |= ((pFrom)->m_uSb0Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern3_127_96 */
  (pToData)[188] |= ((pFrom)->m_uSb0Pattern3_127_96) & 0xFF;
  (pToData)[189] |= ((pFrom)->m_uSb0Pattern3_127_96 >> 8) &0xFF;
  (pToData)[190] |= ((pFrom)->m_uSb0Pattern3_127_96 >> 16) &0xFF;
  (pToData)[191] |= ((pFrom)->m_uSb0Pattern3_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern4_31_0 */
  (pToData)[192] |= ((pFrom)->m_uSb0Pattern4_31_0) & 0xFF;
  (pToData)[193] |= ((pFrom)->m_uSb0Pattern4_31_0 >> 8) &0xFF;
  (pToData)[194] |= ((pFrom)->m_uSb0Pattern4_31_0 >> 16) &0xFF;
  (pToData)[195] |= ((pFrom)->m_uSb0Pattern4_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern4_63_32 */
  (pToData)[196] |= ((pFrom)->m_uSb0Pattern4_63_32) & 0xFF;
  (pToData)[197] |= ((pFrom)->m_uSb0Pattern4_63_32 >> 8) &0xFF;
  (pToData)[198] |= ((pFrom)->m_uSb0Pattern4_63_32 >> 16) &0xFF;
  (pToData)[199] |= ((pFrom)->m_uSb0Pattern4_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern4_95_64 */
  (pToData)[200] |= ((pFrom)->m_uSb0Pattern4_95_64) & 0xFF;
  (pToData)[201] |= ((pFrom)->m_uSb0Pattern4_95_64 >> 8) &0xFF;
  (pToData)[202] |= ((pFrom)->m_uSb0Pattern4_95_64 >> 16) &0xFF;
  (pToData)[203] |= ((pFrom)->m_uSb0Pattern4_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern4_127_96 */
  (pToData)[204] |= ((pFrom)->m_uSb0Pattern4_127_96) & 0xFF;
  (pToData)[205] |= ((pFrom)->m_uSb0Pattern4_127_96 >> 8) &0xFF;
  (pToData)[206] |= ((pFrom)->m_uSb0Pattern4_127_96 >> 16) &0xFF;
  (pToData)[207] |= ((pFrom)->m_uSb0Pattern4_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern5_31_0 */
  (pToData)[208] |= ((pFrom)->m_uSb0Pattern5_31_0) & 0xFF;
  (pToData)[209] |= ((pFrom)->m_uSb0Pattern5_31_0 >> 8) &0xFF;
  (pToData)[210] |= ((pFrom)->m_uSb0Pattern5_31_0 >> 16) &0xFF;
  (pToData)[211] |= ((pFrom)->m_uSb0Pattern5_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern5_63_32 */
  (pToData)[212] |= ((pFrom)->m_uSb0Pattern5_63_32) & 0xFF;
  (pToData)[213] |= ((pFrom)->m_uSb0Pattern5_63_32 >> 8) &0xFF;
  (pToData)[214] |= ((pFrom)->m_uSb0Pattern5_63_32 >> 16) &0xFF;
  (pToData)[215] |= ((pFrom)->m_uSb0Pattern5_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern5_95_64 */
  (pToData)[216] |= ((pFrom)->m_uSb0Pattern5_95_64) & 0xFF;
  (pToData)[217] |= ((pFrom)->m_uSb0Pattern5_95_64 >> 8) &0xFF;
  (pToData)[218] |= ((pFrom)->m_uSb0Pattern5_95_64 >> 16) &0xFF;
  (pToData)[219] |= ((pFrom)->m_uSb0Pattern5_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern5_127_96 */
  (pToData)[220] |= ((pFrom)->m_uSb0Pattern5_127_96) & 0xFF;
  (pToData)[221] |= ((pFrom)->m_uSb0Pattern5_127_96 >> 8) &0xFF;
  (pToData)[222] |= ((pFrom)->m_uSb0Pattern5_127_96 >> 16) &0xFF;
  (pToData)[223] |= ((pFrom)->m_uSb0Pattern5_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern6_31_0 */
  (pToData)[224] |= ((pFrom)->m_uSb0Pattern6_31_0) & 0xFF;
  (pToData)[225] |= ((pFrom)->m_uSb0Pattern6_31_0 >> 8) &0xFF;
  (pToData)[226] |= ((pFrom)->m_uSb0Pattern6_31_0 >> 16) &0xFF;
  (pToData)[227] |= ((pFrom)->m_uSb0Pattern6_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern6_63_32 */
  (pToData)[228] |= ((pFrom)->m_uSb0Pattern6_63_32) & 0xFF;
  (pToData)[229] |= ((pFrom)->m_uSb0Pattern6_63_32 >> 8) &0xFF;
  (pToData)[230] |= ((pFrom)->m_uSb0Pattern6_63_32 >> 16) &0xFF;
  (pToData)[231] |= ((pFrom)->m_uSb0Pattern6_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern6_95_64 */
  (pToData)[232] |= ((pFrom)->m_uSb0Pattern6_95_64) & 0xFF;
  (pToData)[233] |= ((pFrom)->m_uSb0Pattern6_95_64 >> 8) &0xFF;
  (pToData)[234] |= ((pFrom)->m_uSb0Pattern6_95_64 >> 16) &0xFF;
  (pToData)[235] |= ((pFrom)->m_uSb0Pattern6_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern6_127_96 */
  (pToData)[236] |= ((pFrom)->m_uSb0Pattern6_127_96) & 0xFF;
  (pToData)[237] |= ((pFrom)->m_uSb0Pattern6_127_96 >> 8) &0xFF;
  (pToData)[238] |= ((pFrom)->m_uSb0Pattern6_127_96 >> 16) &0xFF;
  (pToData)[239] |= ((pFrom)->m_uSb0Pattern6_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern7_31_0 */
  (pToData)[240] |= ((pFrom)->m_uSb0Pattern7_31_0) & 0xFF;
  (pToData)[241] |= ((pFrom)->m_uSb0Pattern7_31_0 >> 8) &0xFF;
  (pToData)[242] |= ((pFrom)->m_uSb0Pattern7_31_0 >> 16) &0xFF;
  (pToData)[243] |= ((pFrom)->m_uSb0Pattern7_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern7_63_32 */
  (pToData)[244] |= ((pFrom)->m_uSb0Pattern7_63_32) & 0xFF;
  (pToData)[245] |= ((pFrom)->m_uSb0Pattern7_63_32 >> 8) &0xFF;
  (pToData)[246] |= ((pFrom)->m_uSb0Pattern7_63_32 >> 16) &0xFF;
  (pToData)[247] |= ((pFrom)->m_uSb0Pattern7_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern7_95_64 */
  (pToData)[248] |= ((pFrom)->m_uSb0Pattern7_95_64) & 0xFF;
  (pToData)[249] |= ((pFrom)->m_uSb0Pattern7_95_64 >> 8) &0xFF;
  (pToData)[250] |= ((pFrom)->m_uSb0Pattern7_95_64 >> 16) &0xFF;
  (pToData)[251] |= ((pFrom)->m_uSb0Pattern7_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern7_127_96 */
  (pToData)[252] |= ((pFrom)->m_uSb0Pattern7_127_96) & 0xFF;
  (pToData)[253] |= ((pFrom)->m_uSb0Pattern7_127_96 >> 8) &0xFF;
  (pToData)[254] |= ((pFrom)->m_uSb0Pattern7_127_96 >> 16) &0xFF;
  (pToData)[255] |= ((pFrom)->m_uSb0Pattern7_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern0_31_0 */
  (pToData)[256] |= ((pFrom)->m_uSb1Pattern0_31_0) & 0xFF;
  (pToData)[257] |= ((pFrom)->m_uSb1Pattern0_31_0 >> 8) &0xFF;
  (pToData)[258] |= ((pFrom)->m_uSb1Pattern0_31_0 >> 16) &0xFF;
  (pToData)[259] |= ((pFrom)->m_uSb1Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern0_63_32 */
  (pToData)[260] |= ((pFrom)->m_uSb1Pattern0_63_32) & 0xFF;
  (pToData)[261] |= ((pFrom)->m_uSb1Pattern0_63_32 >> 8) &0xFF;
  (pToData)[262] |= ((pFrom)->m_uSb1Pattern0_63_32 >> 16) &0xFF;
  (pToData)[263] |= ((pFrom)->m_uSb1Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern0_95_64 */
  (pToData)[264] |= ((pFrom)->m_uSb1Pattern0_95_64) & 0xFF;
  (pToData)[265] |= ((pFrom)->m_uSb1Pattern0_95_64 >> 8) &0xFF;
  (pToData)[266] |= ((pFrom)->m_uSb1Pattern0_95_64 >> 16) &0xFF;
  (pToData)[267] |= ((pFrom)->m_uSb1Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern0_127_96 */
  (pToData)[268] |= ((pFrom)->m_uSb1Pattern0_127_96) & 0xFF;
  (pToData)[269] |= ((pFrom)->m_uSb1Pattern0_127_96 >> 8) &0xFF;
  (pToData)[270] |= ((pFrom)->m_uSb1Pattern0_127_96 >> 16) &0xFF;
  (pToData)[271] |= ((pFrom)->m_uSb1Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern1_31_0 */
  (pToData)[272] |= ((pFrom)->m_uSb1Pattern1_31_0) & 0xFF;
  (pToData)[273] |= ((pFrom)->m_uSb1Pattern1_31_0 >> 8) &0xFF;
  (pToData)[274] |= ((pFrom)->m_uSb1Pattern1_31_0 >> 16) &0xFF;
  (pToData)[275] |= ((pFrom)->m_uSb1Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern1_63_32 */
  (pToData)[276] |= ((pFrom)->m_uSb1Pattern1_63_32) & 0xFF;
  (pToData)[277] |= ((pFrom)->m_uSb1Pattern1_63_32 >> 8) &0xFF;
  (pToData)[278] |= ((pFrom)->m_uSb1Pattern1_63_32 >> 16) &0xFF;
  (pToData)[279] |= ((pFrom)->m_uSb1Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern1_95_64 */
  (pToData)[280] |= ((pFrom)->m_uSb1Pattern1_95_64) & 0xFF;
  (pToData)[281] |= ((pFrom)->m_uSb1Pattern1_95_64 >> 8) &0xFF;
  (pToData)[282] |= ((pFrom)->m_uSb1Pattern1_95_64 >> 16) &0xFF;
  (pToData)[283] |= ((pFrom)->m_uSb1Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern1_127_96 */
  (pToData)[284] |= ((pFrom)->m_uSb1Pattern1_127_96) & 0xFF;
  (pToData)[285] |= ((pFrom)->m_uSb1Pattern1_127_96 >> 8) &0xFF;
  (pToData)[286] |= ((pFrom)->m_uSb1Pattern1_127_96 >> 16) &0xFF;
  (pToData)[287] |= ((pFrom)->m_uSb1Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern2_31_0 */
  (pToData)[288] |= ((pFrom)->m_uSb1Pattern2_31_0) & 0xFF;
  (pToData)[289] |= ((pFrom)->m_uSb1Pattern2_31_0 >> 8) &0xFF;
  (pToData)[290] |= ((pFrom)->m_uSb1Pattern2_31_0 >> 16) &0xFF;
  (pToData)[291] |= ((pFrom)->m_uSb1Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern2_63_32 */
  (pToData)[292] |= ((pFrom)->m_uSb1Pattern2_63_32) & 0xFF;
  (pToData)[293] |= ((pFrom)->m_uSb1Pattern2_63_32 >> 8) &0xFF;
  (pToData)[294] |= ((pFrom)->m_uSb1Pattern2_63_32 >> 16) &0xFF;
  (pToData)[295] |= ((pFrom)->m_uSb1Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern2_95_64 */
  (pToData)[296] |= ((pFrom)->m_uSb1Pattern2_95_64) & 0xFF;
  (pToData)[297] |= ((pFrom)->m_uSb1Pattern2_95_64 >> 8) &0xFF;
  (pToData)[298] |= ((pFrom)->m_uSb1Pattern2_95_64 >> 16) &0xFF;
  (pToData)[299] |= ((pFrom)->m_uSb1Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern2_127_96 */
  (pToData)[300] |= ((pFrom)->m_uSb1Pattern2_127_96) & 0xFF;
  (pToData)[301] |= ((pFrom)->m_uSb1Pattern2_127_96 >> 8) &0xFF;
  (pToData)[302] |= ((pFrom)->m_uSb1Pattern2_127_96 >> 16) &0xFF;
  (pToData)[303] |= ((pFrom)->m_uSb1Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern3_31_0 */
  (pToData)[304] |= ((pFrom)->m_uSb1Pattern3_31_0) & 0xFF;
  (pToData)[305] |= ((pFrom)->m_uSb1Pattern3_31_0 >> 8) &0xFF;
  (pToData)[306] |= ((pFrom)->m_uSb1Pattern3_31_0 >> 16) &0xFF;
  (pToData)[307] |= ((pFrom)->m_uSb1Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern3_63_32 */
  (pToData)[308] |= ((pFrom)->m_uSb1Pattern3_63_32) & 0xFF;
  (pToData)[309] |= ((pFrom)->m_uSb1Pattern3_63_32 >> 8) &0xFF;
  (pToData)[310] |= ((pFrom)->m_uSb1Pattern3_63_32 >> 16) &0xFF;
  (pToData)[311] |= ((pFrom)->m_uSb1Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern3_95_64 */
  (pToData)[312] |= ((pFrom)->m_uSb1Pattern3_95_64) & 0xFF;
  (pToData)[313] |= ((pFrom)->m_uSb1Pattern3_95_64 >> 8) &0xFF;
  (pToData)[314] |= ((pFrom)->m_uSb1Pattern3_95_64 >> 16) &0xFF;
  (pToData)[315] |= ((pFrom)->m_uSb1Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern3_127_96 */
  (pToData)[316] |= ((pFrom)->m_uSb1Pattern3_127_96) & 0xFF;
  (pToData)[317] |= ((pFrom)->m_uSb1Pattern3_127_96 >> 8) &0xFF;
  (pToData)[318] |= ((pFrom)->m_uSb1Pattern3_127_96 >> 16) &0xFF;
  (pToData)[319] |= ((pFrom)->m_uSb1Pattern3_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern4_31_0 */
  (pToData)[320] |= ((pFrom)->m_uSb1Pattern4_31_0) & 0xFF;
  (pToData)[321] |= ((pFrom)->m_uSb1Pattern4_31_0 >> 8) &0xFF;
  (pToData)[322] |= ((pFrom)->m_uSb1Pattern4_31_0 >> 16) &0xFF;
  (pToData)[323] |= ((pFrom)->m_uSb1Pattern4_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern4_63_32 */
  (pToData)[324] |= ((pFrom)->m_uSb1Pattern4_63_32) & 0xFF;
  (pToData)[325] |= ((pFrom)->m_uSb1Pattern4_63_32 >> 8) &0xFF;
  (pToData)[326] |= ((pFrom)->m_uSb1Pattern4_63_32 >> 16) &0xFF;
  (pToData)[327] |= ((pFrom)->m_uSb1Pattern4_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern4_95_64 */
  (pToData)[328] |= ((pFrom)->m_uSb1Pattern4_95_64) & 0xFF;
  (pToData)[329] |= ((pFrom)->m_uSb1Pattern4_95_64 >> 8) &0xFF;
  (pToData)[330] |= ((pFrom)->m_uSb1Pattern4_95_64 >> 16) &0xFF;
  (pToData)[331] |= ((pFrom)->m_uSb1Pattern4_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern4_127_96 */
  (pToData)[332] |= ((pFrom)->m_uSb1Pattern4_127_96) & 0xFF;
  (pToData)[333] |= ((pFrom)->m_uSb1Pattern4_127_96 >> 8) &0xFF;
  (pToData)[334] |= ((pFrom)->m_uSb1Pattern4_127_96 >> 16) &0xFF;
  (pToData)[335] |= ((pFrom)->m_uSb1Pattern4_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern5_31_0 */
  (pToData)[336] |= ((pFrom)->m_uSb1Pattern5_31_0) & 0xFF;
  (pToData)[337] |= ((pFrom)->m_uSb1Pattern5_31_0 >> 8) &0xFF;
  (pToData)[338] |= ((pFrom)->m_uSb1Pattern5_31_0 >> 16) &0xFF;
  (pToData)[339] |= ((pFrom)->m_uSb1Pattern5_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern5_63_32 */
  (pToData)[340] |= ((pFrom)->m_uSb1Pattern5_63_32) & 0xFF;
  (pToData)[341] |= ((pFrom)->m_uSb1Pattern5_63_32 >> 8) &0xFF;
  (pToData)[342] |= ((pFrom)->m_uSb1Pattern5_63_32 >> 16) &0xFF;
  (pToData)[343] |= ((pFrom)->m_uSb1Pattern5_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern5_95_64 */
  (pToData)[344] |= ((pFrom)->m_uSb1Pattern5_95_64) & 0xFF;
  (pToData)[345] |= ((pFrom)->m_uSb1Pattern5_95_64 >> 8) &0xFF;
  (pToData)[346] |= ((pFrom)->m_uSb1Pattern5_95_64 >> 16) &0xFF;
  (pToData)[347] |= ((pFrom)->m_uSb1Pattern5_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern5_127_96 */
  (pToData)[348] |= ((pFrom)->m_uSb1Pattern5_127_96) & 0xFF;
  (pToData)[349] |= ((pFrom)->m_uSb1Pattern5_127_96 >> 8) &0xFF;
  (pToData)[350] |= ((pFrom)->m_uSb1Pattern5_127_96 >> 16) &0xFF;
  (pToData)[351] |= ((pFrom)->m_uSb1Pattern5_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern6_31_0 */
  (pToData)[352] |= ((pFrom)->m_uSb1Pattern6_31_0) & 0xFF;
  (pToData)[353] |= ((pFrom)->m_uSb1Pattern6_31_0 >> 8) &0xFF;
  (pToData)[354] |= ((pFrom)->m_uSb1Pattern6_31_0 >> 16) &0xFF;
  (pToData)[355] |= ((pFrom)->m_uSb1Pattern6_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern6_63_32 */
  (pToData)[356] |= ((pFrom)->m_uSb1Pattern6_63_32) & 0xFF;
  (pToData)[357] |= ((pFrom)->m_uSb1Pattern6_63_32 >> 8) &0xFF;
  (pToData)[358] |= ((pFrom)->m_uSb1Pattern6_63_32 >> 16) &0xFF;
  (pToData)[359] |= ((pFrom)->m_uSb1Pattern6_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern6_95_64 */
  (pToData)[360] |= ((pFrom)->m_uSb1Pattern6_95_64) & 0xFF;
  (pToData)[361] |= ((pFrom)->m_uSb1Pattern6_95_64 >> 8) &0xFF;
  (pToData)[362] |= ((pFrom)->m_uSb1Pattern6_95_64 >> 16) &0xFF;
  (pToData)[363] |= ((pFrom)->m_uSb1Pattern6_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern6_127_96 */
  (pToData)[364] |= ((pFrom)->m_uSb1Pattern6_127_96) & 0xFF;
  (pToData)[365] |= ((pFrom)->m_uSb1Pattern6_127_96 >> 8) &0xFF;
  (pToData)[366] |= ((pFrom)->m_uSb1Pattern6_127_96 >> 16) &0xFF;
  (pToData)[367] |= ((pFrom)->m_uSb1Pattern6_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern7_31_0 */
  (pToData)[368] |= ((pFrom)->m_uSb1Pattern7_31_0) & 0xFF;
  (pToData)[369] |= ((pFrom)->m_uSb1Pattern7_31_0 >> 8) &0xFF;
  (pToData)[370] |= ((pFrom)->m_uSb1Pattern7_31_0 >> 16) &0xFF;
  (pToData)[371] |= ((pFrom)->m_uSb1Pattern7_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern7_63_32 */
  (pToData)[372] |= ((pFrom)->m_uSb1Pattern7_63_32) & 0xFF;
  (pToData)[373] |= ((pFrom)->m_uSb1Pattern7_63_32 >> 8) &0xFF;
  (pToData)[374] |= ((pFrom)->m_uSb1Pattern7_63_32 >> 16) &0xFF;
  (pToData)[375] |= ((pFrom)->m_uSb1Pattern7_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern7_95_64 */
  (pToData)[376] |= ((pFrom)->m_uSb1Pattern7_95_64) & 0xFF;
  (pToData)[377] |= ((pFrom)->m_uSb1Pattern7_95_64 >> 8) &0xFF;
  (pToData)[378] |= ((pFrom)->m_uSb1Pattern7_95_64 >> 16) &0xFF;
  (pToData)[379] |= ((pFrom)->m_uSb1Pattern7_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern7_127_96 */
  (pToData)[380] |= ((pFrom)->m_uSb1Pattern7_127_96) & 0xFF;
  (pToData)[381] |= ((pFrom)->m_uSb1Pattern7_127_96 >> 8) &0xFF;
  (pToData)[382] |= ((pFrom)->m_uSb1Pattern7_127_96 >> 16) &0xFF;
  (pToData)[383] |= ((pFrom)->m_uSb1Pattern7_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern0_31_0 */
  (pToData)[384] |= ((pFrom)->m_uSb2Pattern0_31_0) & 0xFF;
  (pToData)[385] |= ((pFrom)->m_uSb2Pattern0_31_0 >> 8) &0xFF;
  (pToData)[386] |= ((pFrom)->m_uSb2Pattern0_31_0 >> 16) &0xFF;
  (pToData)[387] |= ((pFrom)->m_uSb2Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern0_63_32 */
  (pToData)[388] |= ((pFrom)->m_uSb2Pattern0_63_32) & 0xFF;
  (pToData)[389] |= ((pFrom)->m_uSb2Pattern0_63_32 >> 8) &0xFF;
  (pToData)[390] |= ((pFrom)->m_uSb2Pattern0_63_32 >> 16) &0xFF;
  (pToData)[391] |= ((pFrom)->m_uSb2Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern0_95_64 */
  (pToData)[392] |= ((pFrom)->m_uSb2Pattern0_95_64) & 0xFF;
  (pToData)[393] |= ((pFrom)->m_uSb2Pattern0_95_64 >> 8) &0xFF;
  (pToData)[394] |= ((pFrom)->m_uSb2Pattern0_95_64 >> 16) &0xFF;
  (pToData)[395] |= ((pFrom)->m_uSb2Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern0_127_96 */
  (pToData)[396] |= ((pFrom)->m_uSb2Pattern0_127_96) & 0xFF;
  (pToData)[397] |= ((pFrom)->m_uSb2Pattern0_127_96 >> 8) &0xFF;
  (pToData)[398] |= ((pFrom)->m_uSb2Pattern0_127_96 >> 16) &0xFF;
  (pToData)[399] |= ((pFrom)->m_uSb2Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern1_31_0 */
  (pToData)[400] |= ((pFrom)->m_uSb2Pattern1_31_0) & 0xFF;
  (pToData)[401] |= ((pFrom)->m_uSb2Pattern1_31_0 >> 8) &0xFF;
  (pToData)[402] |= ((pFrom)->m_uSb2Pattern1_31_0 >> 16) &0xFF;
  (pToData)[403] |= ((pFrom)->m_uSb2Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern1_63_32 */
  (pToData)[404] |= ((pFrom)->m_uSb2Pattern1_63_32) & 0xFF;
  (pToData)[405] |= ((pFrom)->m_uSb2Pattern1_63_32 >> 8) &0xFF;
  (pToData)[406] |= ((pFrom)->m_uSb2Pattern1_63_32 >> 16) &0xFF;
  (pToData)[407] |= ((pFrom)->m_uSb2Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern1_95_64 */
  (pToData)[408] |= ((pFrom)->m_uSb2Pattern1_95_64) & 0xFF;
  (pToData)[409] |= ((pFrom)->m_uSb2Pattern1_95_64 >> 8) &0xFF;
  (pToData)[410] |= ((pFrom)->m_uSb2Pattern1_95_64 >> 16) &0xFF;
  (pToData)[411] |= ((pFrom)->m_uSb2Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern1_127_96 */
  (pToData)[412] |= ((pFrom)->m_uSb2Pattern1_127_96) & 0xFF;
  (pToData)[413] |= ((pFrom)->m_uSb2Pattern1_127_96 >> 8) &0xFF;
  (pToData)[414] |= ((pFrom)->m_uSb2Pattern1_127_96 >> 16) &0xFF;
  (pToData)[415] |= ((pFrom)->m_uSb2Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern2_31_0 */
  (pToData)[416] |= ((pFrom)->m_uSb2Pattern2_31_0) & 0xFF;
  (pToData)[417] |= ((pFrom)->m_uSb2Pattern2_31_0 >> 8) &0xFF;
  (pToData)[418] |= ((pFrom)->m_uSb2Pattern2_31_0 >> 16) &0xFF;
  (pToData)[419] |= ((pFrom)->m_uSb2Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern2_63_32 */
  (pToData)[420] |= ((pFrom)->m_uSb2Pattern2_63_32) & 0xFF;
  (pToData)[421] |= ((pFrom)->m_uSb2Pattern2_63_32 >> 8) &0xFF;
  (pToData)[422] |= ((pFrom)->m_uSb2Pattern2_63_32 >> 16) &0xFF;
  (pToData)[423] |= ((pFrom)->m_uSb2Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern2_95_64 */
  (pToData)[424] |= ((pFrom)->m_uSb2Pattern2_95_64) & 0xFF;
  (pToData)[425] |= ((pFrom)->m_uSb2Pattern2_95_64 >> 8) &0xFF;
  (pToData)[426] |= ((pFrom)->m_uSb2Pattern2_95_64 >> 16) &0xFF;
  (pToData)[427] |= ((pFrom)->m_uSb2Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern2_127_96 */
  (pToData)[428] |= ((pFrom)->m_uSb2Pattern2_127_96) & 0xFF;
  (pToData)[429] |= ((pFrom)->m_uSb2Pattern2_127_96 >> 8) &0xFF;
  (pToData)[430] |= ((pFrom)->m_uSb2Pattern2_127_96 >> 16) &0xFF;
  (pToData)[431] |= ((pFrom)->m_uSb2Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern3_31_0 */
  (pToData)[432] |= ((pFrom)->m_uSb2Pattern3_31_0) & 0xFF;
  (pToData)[433] |= ((pFrom)->m_uSb2Pattern3_31_0 >> 8) &0xFF;
  (pToData)[434] |= ((pFrom)->m_uSb2Pattern3_31_0 >> 16) &0xFF;
  (pToData)[435] |= ((pFrom)->m_uSb2Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern3_63_32 */
  (pToData)[436] |= ((pFrom)->m_uSb2Pattern3_63_32) & 0xFF;
  (pToData)[437] |= ((pFrom)->m_uSb2Pattern3_63_32 >> 8) &0xFF;
  (pToData)[438] |= ((pFrom)->m_uSb2Pattern3_63_32 >> 16) &0xFF;
  (pToData)[439] |= ((pFrom)->m_uSb2Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern3_95_64 */
  (pToData)[440] |= ((pFrom)->m_uSb2Pattern3_95_64) & 0xFF;
  (pToData)[441] |= ((pFrom)->m_uSb2Pattern3_95_64 >> 8) &0xFF;
  (pToData)[442] |= ((pFrom)->m_uSb2Pattern3_95_64 >> 16) &0xFF;
  (pToData)[443] |= ((pFrom)->m_uSb2Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern3_127_96 */
  (pToData)[444] |= ((pFrom)->m_uSb2Pattern3_127_96) & 0xFF;
  (pToData)[445] |= ((pFrom)->m_uSb2Pattern3_127_96 >> 8) &0xFF;
  (pToData)[446] |= ((pFrom)->m_uSb2Pattern3_127_96 >> 16) &0xFF;
  (pToData)[447] |= ((pFrom)->m_uSb2Pattern3_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern4_31_0 */
  (pToData)[448] |= ((pFrom)->m_uSb2Pattern4_31_0) & 0xFF;
  (pToData)[449] |= ((pFrom)->m_uSb2Pattern4_31_0 >> 8) &0xFF;
  (pToData)[450] |= ((pFrom)->m_uSb2Pattern4_31_0 >> 16) &0xFF;
  (pToData)[451] |= ((pFrom)->m_uSb2Pattern4_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern4_63_32 */
  (pToData)[452] |= ((pFrom)->m_uSb2Pattern4_63_32) & 0xFF;
  (pToData)[453] |= ((pFrom)->m_uSb2Pattern4_63_32 >> 8) &0xFF;
  (pToData)[454] |= ((pFrom)->m_uSb2Pattern4_63_32 >> 16) &0xFF;
  (pToData)[455] |= ((pFrom)->m_uSb2Pattern4_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern4_95_64 */
  (pToData)[456] |= ((pFrom)->m_uSb2Pattern4_95_64) & 0xFF;
  (pToData)[457] |= ((pFrom)->m_uSb2Pattern4_95_64 >> 8) &0xFF;
  (pToData)[458] |= ((pFrom)->m_uSb2Pattern4_95_64 >> 16) &0xFF;
  (pToData)[459] |= ((pFrom)->m_uSb2Pattern4_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern4_127_96 */
  (pToData)[460] |= ((pFrom)->m_uSb2Pattern4_127_96) & 0xFF;
  (pToData)[461] |= ((pFrom)->m_uSb2Pattern4_127_96 >> 8) &0xFF;
  (pToData)[462] |= ((pFrom)->m_uSb2Pattern4_127_96 >> 16) &0xFF;
  (pToData)[463] |= ((pFrom)->m_uSb2Pattern4_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern5_31_0 */
  (pToData)[464] |= ((pFrom)->m_uSb2Pattern5_31_0) & 0xFF;
  (pToData)[465] |= ((pFrom)->m_uSb2Pattern5_31_0 >> 8) &0xFF;
  (pToData)[466] |= ((pFrom)->m_uSb2Pattern5_31_0 >> 16) &0xFF;
  (pToData)[467] |= ((pFrom)->m_uSb2Pattern5_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern5_63_32 */
  (pToData)[468] |= ((pFrom)->m_uSb2Pattern5_63_32) & 0xFF;
  (pToData)[469] |= ((pFrom)->m_uSb2Pattern5_63_32 >> 8) &0xFF;
  (pToData)[470] |= ((pFrom)->m_uSb2Pattern5_63_32 >> 16) &0xFF;
  (pToData)[471] |= ((pFrom)->m_uSb2Pattern5_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern5_95_64 */
  (pToData)[472] |= ((pFrom)->m_uSb2Pattern5_95_64) & 0xFF;
  (pToData)[473] |= ((pFrom)->m_uSb2Pattern5_95_64 >> 8) &0xFF;
  (pToData)[474] |= ((pFrom)->m_uSb2Pattern5_95_64 >> 16) &0xFF;
  (pToData)[475] |= ((pFrom)->m_uSb2Pattern5_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern5_127_96 */
  (pToData)[476] |= ((pFrom)->m_uSb2Pattern5_127_96) & 0xFF;
  (pToData)[477] |= ((pFrom)->m_uSb2Pattern5_127_96 >> 8) &0xFF;
  (pToData)[478] |= ((pFrom)->m_uSb2Pattern5_127_96 >> 16) &0xFF;
  (pToData)[479] |= ((pFrom)->m_uSb2Pattern5_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern6_31_0 */
  (pToData)[480] |= ((pFrom)->m_uSb2Pattern6_31_0) & 0xFF;
  (pToData)[481] |= ((pFrom)->m_uSb2Pattern6_31_0 >> 8) &0xFF;
  (pToData)[482] |= ((pFrom)->m_uSb2Pattern6_31_0 >> 16) &0xFF;
  (pToData)[483] |= ((pFrom)->m_uSb2Pattern6_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern6_63_32 */
  (pToData)[484] |= ((pFrom)->m_uSb2Pattern6_63_32) & 0xFF;
  (pToData)[485] |= ((pFrom)->m_uSb2Pattern6_63_32 >> 8) &0xFF;
  (pToData)[486] |= ((pFrom)->m_uSb2Pattern6_63_32 >> 16) &0xFF;
  (pToData)[487] |= ((pFrom)->m_uSb2Pattern6_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern6_95_64 */
  (pToData)[488] |= ((pFrom)->m_uSb2Pattern6_95_64) & 0xFF;
  (pToData)[489] |= ((pFrom)->m_uSb2Pattern6_95_64 >> 8) &0xFF;
  (pToData)[490] |= ((pFrom)->m_uSb2Pattern6_95_64 >> 16) &0xFF;
  (pToData)[491] |= ((pFrom)->m_uSb2Pattern6_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern6_127_96 */
  (pToData)[492] |= ((pFrom)->m_uSb2Pattern6_127_96) & 0xFF;
  (pToData)[493] |= ((pFrom)->m_uSb2Pattern6_127_96 >> 8) &0xFF;
  (pToData)[494] |= ((pFrom)->m_uSb2Pattern6_127_96 >> 16) &0xFF;
  (pToData)[495] |= ((pFrom)->m_uSb2Pattern6_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern7_31_0 */
  (pToData)[496] |= ((pFrom)->m_uSb2Pattern7_31_0) & 0xFF;
  (pToData)[497] |= ((pFrom)->m_uSb2Pattern7_31_0 >> 8) &0xFF;
  (pToData)[498] |= ((pFrom)->m_uSb2Pattern7_31_0 >> 16) &0xFF;
  (pToData)[499] |= ((pFrom)->m_uSb2Pattern7_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern7_63_32 */
  (pToData)[500] |= ((pFrom)->m_uSb2Pattern7_63_32) & 0xFF;
  (pToData)[501] |= ((pFrom)->m_uSb2Pattern7_63_32 >> 8) &0xFF;
  (pToData)[502] |= ((pFrom)->m_uSb2Pattern7_63_32 >> 16) &0xFF;
  (pToData)[503] |= ((pFrom)->m_uSb2Pattern7_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern7_95_64 */
  (pToData)[504] |= ((pFrom)->m_uSb2Pattern7_95_64) & 0xFF;
  (pToData)[505] |= ((pFrom)->m_uSb2Pattern7_95_64 >> 8) &0xFF;
  (pToData)[506] |= ((pFrom)->m_uSb2Pattern7_95_64 >> 16) &0xFF;
  (pToData)[507] |= ((pFrom)->m_uSb2Pattern7_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb2Pattern7_127_96 */
  (pToData)[508] |= ((pFrom)->m_uSb2Pattern7_127_96) & 0xFF;
  (pToData)[509] |= ((pFrom)->m_uSb2Pattern7_127_96 >> 8) &0xFF;
  (pToData)[510] |= ((pFrom)->m_uSb2Pattern7_127_96 >> 16) &0xFF;
  (pToData)[511] |= ((pFrom)->m_uSb2Pattern7_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern0_31_0 */
  (pToData)[512] |= ((pFrom)->m_uSb3Pattern0_31_0) & 0xFF;
  (pToData)[513] |= ((pFrom)->m_uSb3Pattern0_31_0 >> 8) &0xFF;
  (pToData)[514] |= ((pFrom)->m_uSb3Pattern0_31_0 >> 16) &0xFF;
  (pToData)[515] |= ((pFrom)->m_uSb3Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern0_63_32 */
  (pToData)[516] |= ((pFrom)->m_uSb3Pattern0_63_32) & 0xFF;
  (pToData)[517] |= ((pFrom)->m_uSb3Pattern0_63_32 >> 8) &0xFF;
  (pToData)[518] |= ((pFrom)->m_uSb3Pattern0_63_32 >> 16) &0xFF;
  (pToData)[519] |= ((pFrom)->m_uSb3Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern0_95_64 */
  (pToData)[520] |= ((pFrom)->m_uSb3Pattern0_95_64) & 0xFF;
  (pToData)[521] |= ((pFrom)->m_uSb3Pattern0_95_64 >> 8) &0xFF;
  (pToData)[522] |= ((pFrom)->m_uSb3Pattern0_95_64 >> 16) &0xFF;
  (pToData)[523] |= ((pFrom)->m_uSb3Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern0_127_96 */
  (pToData)[524] |= ((pFrom)->m_uSb3Pattern0_127_96) & 0xFF;
  (pToData)[525] |= ((pFrom)->m_uSb3Pattern0_127_96 >> 8) &0xFF;
  (pToData)[526] |= ((pFrom)->m_uSb3Pattern0_127_96 >> 16) &0xFF;
  (pToData)[527] |= ((pFrom)->m_uSb3Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern1_31_0 */
  (pToData)[528] |= ((pFrom)->m_uSb3Pattern1_31_0) & 0xFF;
  (pToData)[529] |= ((pFrom)->m_uSb3Pattern1_31_0 >> 8) &0xFF;
  (pToData)[530] |= ((pFrom)->m_uSb3Pattern1_31_0 >> 16) &0xFF;
  (pToData)[531] |= ((pFrom)->m_uSb3Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern1_63_32 */
  (pToData)[532] |= ((pFrom)->m_uSb3Pattern1_63_32) & 0xFF;
  (pToData)[533] |= ((pFrom)->m_uSb3Pattern1_63_32 >> 8) &0xFF;
  (pToData)[534] |= ((pFrom)->m_uSb3Pattern1_63_32 >> 16) &0xFF;
  (pToData)[535] |= ((pFrom)->m_uSb3Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern1_95_64 */
  (pToData)[536] |= ((pFrom)->m_uSb3Pattern1_95_64) & 0xFF;
  (pToData)[537] |= ((pFrom)->m_uSb3Pattern1_95_64 >> 8) &0xFF;
  (pToData)[538] |= ((pFrom)->m_uSb3Pattern1_95_64 >> 16) &0xFF;
  (pToData)[539] |= ((pFrom)->m_uSb3Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern1_127_96 */
  (pToData)[540] |= ((pFrom)->m_uSb3Pattern1_127_96) & 0xFF;
  (pToData)[541] |= ((pFrom)->m_uSb3Pattern1_127_96 >> 8) &0xFF;
  (pToData)[542] |= ((pFrom)->m_uSb3Pattern1_127_96 >> 16) &0xFF;
  (pToData)[543] |= ((pFrom)->m_uSb3Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern2_31_0 */
  (pToData)[544] |= ((pFrom)->m_uSb3Pattern2_31_0) & 0xFF;
  (pToData)[545] |= ((pFrom)->m_uSb3Pattern2_31_0 >> 8) &0xFF;
  (pToData)[546] |= ((pFrom)->m_uSb3Pattern2_31_0 >> 16) &0xFF;
  (pToData)[547] |= ((pFrom)->m_uSb3Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern2_63_32 */
  (pToData)[548] |= ((pFrom)->m_uSb3Pattern2_63_32) & 0xFF;
  (pToData)[549] |= ((pFrom)->m_uSb3Pattern2_63_32 >> 8) &0xFF;
  (pToData)[550] |= ((pFrom)->m_uSb3Pattern2_63_32 >> 16) &0xFF;
  (pToData)[551] |= ((pFrom)->m_uSb3Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern2_95_64 */
  (pToData)[552] |= ((pFrom)->m_uSb3Pattern2_95_64) & 0xFF;
  (pToData)[553] |= ((pFrom)->m_uSb3Pattern2_95_64 >> 8) &0xFF;
  (pToData)[554] |= ((pFrom)->m_uSb3Pattern2_95_64 >> 16) &0xFF;
  (pToData)[555] |= ((pFrom)->m_uSb3Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern2_127_96 */
  (pToData)[556] |= ((pFrom)->m_uSb3Pattern2_127_96) & 0xFF;
  (pToData)[557] |= ((pFrom)->m_uSb3Pattern2_127_96 >> 8) &0xFF;
  (pToData)[558] |= ((pFrom)->m_uSb3Pattern2_127_96 >> 16) &0xFF;
  (pToData)[559] |= ((pFrom)->m_uSb3Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern3_31_0 */
  (pToData)[560] |= ((pFrom)->m_uSb3Pattern3_31_0) & 0xFF;
  (pToData)[561] |= ((pFrom)->m_uSb3Pattern3_31_0 >> 8) &0xFF;
  (pToData)[562] |= ((pFrom)->m_uSb3Pattern3_31_0 >> 16) &0xFF;
  (pToData)[563] |= ((pFrom)->m_uSb3Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern3_63_32 */
  (pToData)[564] |= ((pFrom)->m_uSb3Pattern3_63_32) & 0xFF;
  (pToData)[565] |= ((pFrom)->m_uSb3Pattern3_63_32 >> 8) &0xFF;
  (pToData)[566] |= ((pFrom)->m_uSb3Pattern3_63_32 >> 16) &0xFF;
  (pToData)[567] |= ((pFrom)->m_uSb3Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern3_95_64 */
  (pToData)[568] |= ((pFrom)->m_uSb3Pattern3_95_64) & 0xFF;
  (pToData)[569] |= ((pFrom)->m_uSb3Pattern3_95_64 >> 8) &0xFF;
  (pToData)[570] |= ((pFrom)->m_uSb3Pattern3_95_64 >> 16) &0xFF;
  (pToData)[571] |= ((pFrom)->m_uSb3Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern3_127_96 */
  (pToData)[572] |= ((pFrom)->m_uSb3Pattern3_127_96) & 0xFF;
  (pToData)[573] |= ((pFrom)->m_uSb3Pattern3_127_96 >> 8) &0xFF;
  (pToData)[574] |= ((pFrom)->m_uSb3Pattern3_127_96 >> 16) &0xFF;
  (pToData)[575] |= ((pFrom)->m_uSb3Pattern3_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern4_31_0 */
  (pToData)[576] |= ((pFrom)->m_uSb3Pattern4_31_0) & 0xFF;
  (pToData)[577] |= ((pFrom)->m_uSb3Pattern4_31_0 >> 8) &0xFF;
  (pToData)[578] |= ((pFrom)->m_uSb3Pattern4_31_0 >> 16) &0xFF;
  (pToData)[579] |= ((pFrom)->m_uSb3Pattern4_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern4_63_32 */
  (pToData)[580] |= ((pFrom)->m_uSb3Pattern4_63_32) & 0xFF;
  (pToData)[581] |= ((pFrom)->m_uSb3Pattern4_63_32 >> 8) &0xFF;
  (pToData)[582] |= ((pFrom)->m_uSb3Pattern4_63_32 >> 16) &0xFF;
  (pToData)[583] |= ((pFrom)->m_uSb3Pattern4_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern4_95_64 */
  (pToData)[584] |= ((pFrom)->m_uSb3Pattern4_95_64) & 0xFF;
  (pToData)[585] |= ((pFrom)->m_uSb3Pattern4_95_64 >> 8) &0xFF;
  (pToData)[586] |= ((pFrom)->m_uSb3Pattern4_95_64 >> 16) &0xFF;
  (pToData)[587] |= ((pFrom)->m_uSb3Pattern4_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern4_127_96 */
  (pToData)[588] |= ((pFrom)->m_uSb3Pattern4_127_96) & 0xFF;
  (pToData)[589] |= ((pFrom)->m_uSb3Pattern4_127_96 >> 8) &0xFF;
  (pToData)[590] |= ((pFrom)->m_uSb3Pattern4_127_96 >> 16) &0xFF;
  (pToData)[591] |= ((pFrom)->m_uSb3Pattern4_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern5_31_0 */
  (pToData)[592] |= ((pFrom)->m_uSb3Pattern5_31_0) & 0xFF;
  (pToData)[593] |= ((pFrom)->m_uSb3Pattern5_31_0 >> 8) &0xFF;
  (pToData)[594] |= ((pFrom)->m_uSb3Pattern5_31_0 >> 16) &0xFF;
  (pToData)[595] |= ((pFrom)->m_uSb3Pattern5_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern5_63_32 */
  (pToData)[596] |= ((pFrom)->m_uSb3Pattern5_63_32) & 0xFF;
  (pToData)[597] |= ((pFrom)->m_uSb3Pattern5_63_32 >> 8) &0xFF;
  (pToData)[598] |= ((pFrom)->m_uSb3Pattern5_63_32 >> 16) &0xFF;
  (pToData)[599] |= ((pFrom)->m_uSb3Pattern5_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern5_95_64 */
  (pToData)[600] |= ((pFrom)->m_uSb3Pattern5_95_64) & 0xFF;
  (pToData)[601] |= ((pFrom)->m_uSb3Pattern5_95_64 >> 8) &0xFF;
  (pToData)[602] |= ((pFrom)->m_uSb3Pattern5_95_64 >> 16) &0xFF;
  (pToData)[603] |= ((pFrom)->m_uSb3Pattern5_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern5_127_96 */
  (pToData)[604] |= ((pFrom)->m_uSb3Pattern5_127_96) & 0xFF;
  (pToData)[605] |= ((pFrom)->m_uSb3Pattern5_127_96 >> 8) &0xFF;
  (pToData)[606] |= ((pFrom)->m_uSb3Pattern5_127_96 >> 16) &0xFF;
  (pToData)[607] |= ((pFrom)->m_uSb3Pattern5_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern6_31_0 */
  (pToData)[608] |= ((pFrom)->m_uSb3Pattern6_31_0) & 0xFF;
  (pToData)[609] |= ((pFrom)->m_uSb3Pattern6_31_0 >> 8) &0xFF;
  (pToData)[610] |= ((pFrom)->m_uSb3Pattern6_31_0 >> 16) &0xFF;
  (pToData)[611] |= ((pFrom)->m_uSb3Pattern6_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern6_63_32 */
  (pToData)[612] |= ((pFrom)->m_uSb3Pattern6_63_32) & 0xFF;
  (pToData)[613] |= ((pFrom)->m_uSb3Pattern6_63_32 >> 8) &0xFF;
  (pToData)[614] |= ((pFrom)->m_uSb3Pattern6_63_32 >> 16) &0xFF;
  (pToData)[615] |= ((pFrom)->m_uSb3Pattern6_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern6_95_64 */
  (pToData)[616] |= ((pFrom)->m_uSb3Pattern6_95_64) & 0xFF;
  (pToData)[617] |= ((pFrom)->m_uSb3Pattern6_95_64 >> 8) &0xFF;
  (pToData)[618] |= ((pFrom)->m_uSb3Pattern6_95_64 >> 16) &0xFF;
  (pToData)[619] |= ((pFrom)->m_uSb3Pattern6_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern6_127_96 */
  (pToData)[620] |= ((pFrom)->m_uSb3Pattern6_127_96) & 0xFF;
  (pToData)[621] |= ((pFrom)->m_uSb3Pattern6_127_96 >> 8) &0xFF;
  (pToData)[622] |= ((pFrom)->m_uSb3Pattern6_127_96 >> 16) &0xFF;
  (pToData)[623] |= ((pFrom)->m_uSb3Pattern6_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern7_31_0 */
  (pToData)[624] |= ((pFrom)->m_uSb3Pattern7_31_0) & 0xFF;
  (pToData)[625] |= ((pFrom)->m_uSb3Pattern7_31_0 >> 8) &0xFF;
  (pToData)[626] |= ((pFrom)->m_uSb3Pattern7_31_0 >> 16) &0xFF;
  (pToData)[627] |= ((pFrom)->m_uSb3Pattern7_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern7_63_32 */
  (pToData)[628] |= ((pFrom)->m_uSb3Pattern7_63_32) & 0xFF;
  (pToData)[629] |= ((pFrom)->m_uSb3Pattern7_63_32 >> 8) &0xFF;
  (pToData)[630] |= ((pFrom)->m_uSb3Pattern7_63_32 >> 16) &0xFF;
  (pToData)[631] |= ((pFrom)->m_uSb3Pattern7_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern7_95_64 */
  (pToData)[632] |= ((pFrom)->m_uSb3Pattern7_95_64) & 0xFF;
  (pToData)[633] |= ((pFrom)->m_uSb3Pattern7_95_64 >> 8) &0xFF;
  (pToData)[634] |= ((pFrom)->m_uSb3Pattern7_95_64 >> 16) &0xFF;
  (pToData)[635] |= ((pFrom)->m_uSb3Pattern7_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb3Pattern7_127_96 */
  (pToData)[636] |= ((pFrom)->m_uSb3Pattern7_127_96) & 0xFF;
  (pToData)[637] |= ((pFrom)->m_uSb3Pattern7_127_96 >> 8) &0xFF;
  (pToData)[638] |= ((pFrom)->m_uSb3Pattern7_127_96 >> 16) &0xFF;
  (pToData)[639] |= ((pFrom)->m_uSb3Pattern7_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern0_31_0 */
  (pToData)[640] |= ((pFrom)->m_uSb4Pattern0_31_0) & 0xFF;
  (pToData)[641] |= ((pFrom)->m_uSb4Pattern0_31_0 >> 8) &0xFF;
  (pToData)[642] |= ((pFrom)->m_uSb4Pattern0_31_0 >> 16) &0xFF;
  (pToData)[643] |= ((pFrom)->m_uSb4Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern0_63_32 */
  (pToData)[644] |= ((pFrom)->m_uSb4Pattern0_63_32) & 0xFF;
  (pToData)[645] |= ((pFrom)->m_uSb4Pattern0_63_32 >> 8) &0xFF;
  (pToData)[646] |= ((pFrom)->m_uSb4Pattern0_63_32 >> 16) &0xFF;
  (pToData)[647] |= ((pFrom)->m_uSb4Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern0_95_64 */
  (pToData)[648] |= ((pFrom)->m_uSb4Pattern0_95_64) & 0xFF;
  (pToData)[649] |= ((pFrom)->m_uSb4Pattern0_95_64 >> 8) &0xFF;
  (pToData)[650] |= ((pFrom)->m_uSb4Pattern0_95_64 >> 16) &0xFF;
  (pToData)[651] |= ((pFrom)->m_uSb4Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern0_127_96 */
  (pToData)[652] |= ((pFrom)->m_uSb4Pattern0_127_96) & 0xFF;
  (pToData)[653] |= ((pFrom)->m_uSb4Pattern0_127_96 >> 8) &0xFF;
  (pToData)[654] |= ((pFrom)->m_uSb4Pattern0_127_96 >> 16) &0xFF;
  (pToData)[655] |= ((pFrom)->m_uSb4Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern1_31_0 */
  (pToData)[656] |= ((pFrom)->m_uSb4Pattern1_31_0) & 0xFF;
  (pToData)[657] |= ((pFrom)->m_uSb4Pattern1_31_0 >> 8) &0xFF;
  (pToData)[658] |= ((pFrom)->m_uSb4Pattern1_31_0 >> 16) &0xFF;
  (pToData)[659] |= ((pFrom)->m_uSb4Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern1_63_32 */
  (pToData)[660] |= ((pFrom)->m_uSb4Pattern1_63_32) & 0xFF;
  (pToData)[661] |= ((pFrom)->m_uSb4Pattern1_63_32 >> 8) &0xFF;
  (pToData)[662] |= ((pFrom)->m_uSb4Pattern1_63_32 >> 16) &0xFF;
  (pToData)[663] |= ((pFrom)->m_uSb4Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern1_95_64 */
  (pToData)[664] |= ((pFrom)->m_uSb4Pattern1_95_64) & 0xFF;
  (pToData)[665] |= ((pFrom)->m_uSb4Pattern1_95_64 >> 8) &0xFF;
  (pToData)[666] |= ((pFrom)->m_uSb4Pattern1_95_64 >> 16) &0xFF;
  (pToData)[667] |= ((pFrom)->m_uSb4Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern1_127_96 */
  (pToData)[668] |= ((pFrom)->m_uSb4Pattern1_127_96) & 0xFF;
  (pToData)[669] |= ((pFrom)->m_uSb4Pattern1_127_96 >> 8) &0xFF;
  (pToData)[670] |= ((pFrom)->m_uSb4Pattern1_127_96 >> 16) &0xFF;
  (pToData)[671] |= ((pFrom)->m_uSb4Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern2_31_0 */
  (pToData)[672] |= ((pFrom)->m_uSb4Pattern2_31_0) & 0xFF;
  (pToData)[673] |= ((pFrom)->m_uSb4Pattern2_31_0 >> 8) &0xFF;
  (pToData)[674] |= ((pFrom)->m_uSb4Pattern2_31_0 >> 16) &0xFF;
  (pToData)[675] |= ((pFrom)->m_uSb4Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern2_63_32 */
  (pToData)[676] |= ((pFrom)->m_uSb4Pattern2_63_32) & 0xFF;
  (pToData)[677] |= ((pFrom)->m_uSb4Pattern2_63_32 >> 8) &0xFF;
  (pToData)[678] |= ((pFrom)->m_uSb4Pattern2_63_32 >> 16) &0xFF;
  (pToData)[679] |= ((pFrom)->m_uSb4Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern2_95_64 */
  (pToData)[680] |= ((pFrom)->m_uSb4Pattern2_95_64) & 0xFF;
  (pToData)[681] |= ((pFrom)->m_uSb4Pattern2_95_64 >> 8) &0xFF;
  (pToData)[682] |= ((pFrom)->m_uSb4Pattern2_95_64 >> 16) &0xFF;
  (pToData)[683] |= ((pFrom)->m_uSb4Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern2_127_96 */
  (pToData)[684] |= ((pFrom)->m_uSb4Pattern2_127_96) & 0xFF;
  (pToData)[685] |= ((pFrom)->m_uSb4Pattern2_127_96 >> 8) &0xFF;
  (pToData)[686] |= ((pFrom)->m_uSb4Pattern2_127_96 >> 16) &0xFF;
  (pToData)[687] |= ((pFrom)->m_uSb4Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern3_31_0 */
  (pToData)[688] |= ((pFrom)->m_uSb4Pattern3_31_0) & 0xFF;
  (pToData)[689] |= ((pFrom)->m_uSb4Pattern3_31_0 >> 8) &0xFF;
  (pToData)[690] |= ((pFrom)->m_uSb4Pattern3_31_0 >> 16) &0xFF;
  (pToData)[691] |= ((pFrom)->m_uSb4Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern3_63_32 */
  (pToData)[692] |= ((pFrom)->m_uSb4Pattern3_63_32) & 0xFF;
  (pToData)[693] |= ((pFrom)->m_uSb4Pattern3_63_32 >> 8) &0xFF;
  (pToData)[694] |= ((pFrom)->m_uSb4Pattern3_63_32 >> 16) &0xFF;
  (pToData)[695] |= ((pFrom)->m_uSb4Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern3_95_64 */
  (pToData)[696] |= ((pFrom)->m_uSb4Pattern3_95_64) & 0xFF;
  (pToData)[697] |= ((pFrom)->m_uSb4Pattern3_95_64 >> 8) &0xFF;
  (pToData)[698] |= ((pFrom)->m_uSb4Pattern3_95_64 >> 16) &0xFF;
  (pToData)[699] |= ((pFrom)->m_uSb4Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern3_127_96 */
  (pToData)[700] |= ((pFrom)->m_uSb4Pattern3_127_96) & 0xFF;
  (pToData)[701] |= ((pFrom)->m_uSb4Pattern3_127_96 >> 8) &0xFF;
  (pToData)[702] |= ((pFrom)->m_uSb4Pattern3_127_96 >> 16) &0xFF;
  (pToData)[703] |= ((pFrom)->m_uSb4Pattern3_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern4_31_0 */
  (pToData)[704] |= ((pFrom)->m_uSb4Pattern4_31_0) & 0xFF;
  (pToData)[705] |= ((pFrom)->m_uSb4Pattern4_31_0 >> 8) &0xFF;
  (pToData)[706] |= ((pFrom)->m_uSb4Pattern4_31_0 >> 16) &0xFF;
  (pToData)[707] |= ((pFrom)->m_uSb4Pattern4_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern4_63_32 */
  (pToData)[708] |= ((pFrom)->m_uSb4Pattern4_63_32) & 0xFF;
  (pToData)[709] |= ((pFrom)->m_uSb4Pattern4_63_32 >> 8) &0xFF;
  (pToData)[710] |= ((pFrom)->m_uSb4Pattern4_63_32 >> 16) &0xFF;
  (pToData)[711] |= ((pFrom)->m_uSb4Pattern4_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern4_95_64 */
  (pToData)[712] |= ((pFrom)->m_uSb4Pattern4_95_64) & 0xFF;
  (pToData)[713] |= ((pFrom)->m_uSb4Pattern4_95_64 >> 8) &0xFF;
  (pToData)[714] |= ((pFrom)->m_uSb4Pattern4_95_64 >> 16) &0xFF;
  (pToData)[715] |= ((pFrom)->m_uSb4Pattern4_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern4_127_96 */
  (pToData)[716] |= ((pFrom)->m_uSb4Pattern4_127_96) & 0xFF;
  (pToData)[717] |= ((pFrom)->m_uSb4Pattern4_127_96 >> 8) &0xFF;
  (pToData)[718] |= ((pFrom)->m_uSb4Pattern4_127_96 >> 16) &0xFF;
  (pToData)[719] |= ((pFrom)->m_uSb4Pattern4_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern5_31_0 */
  (pToData)[720] |= ((pFrom)->m_uSb4Pattern5_31_0) & 0xFF;
  (pToData)[721] |= ((pFrom)->m_uSb4Pattern5_31_0 >> 8) &0xFF;
  (pToData)[722] |= ((pFrom)->m_uSb4Pattern5_31_0 >> 16) &0xFF;
  (pToData)[723] |= ((pFrom)->m_uSb4Pattern5_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern5_63_32 */
  (pToData)[724] |= ((pFrom)->m_uSb4Pattern5_63_32) & 0xFF;
  (pToData)[725] |= ((pFrom)->m_uSb4Pattern5_63_32 >> 8) &0xFF;
  (pToData)[726] |= ((pFrom)->m_uSb4Pattern5_63_32 >> 16) &0xFF;
  (pToData)[727] |= ((pFrom)->m_uSb4Pattern5_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern5_95_64 */
  (pToData)[728] |= ((pFrom)->m_uSb4Pattern5_95_64) & 0xFF;
  (pToData)[729] |= ((pFrom)->m_uSb4Pattern5_95_64 >> 8) &0xFF;
  (pToData)[730] |= ((pFrom)->m_uSb4Pattern5_95_64 >> 16) &0xFF;
  (pToData)[731] |= ((pFrom)->m_uSb4Pattern5_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern5_127_96 */
  (pToData)[732] |= ((pFrom)->m_uSb4Pattern5_127_96) & 0xFF;
  (pToData)[733] |= ((pFrom)->m_uSb4Pattern5_127_96 >> 8) &0xFF;
  (pToData)[734] |= ((pFrom)->m_uSb4Pattern5_127_96 >> 16) &0xFF;
  (pToData)[735] |= ((pFrom)->m_uSb4Pattern5_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern6_31_0 */
  (pToData)[736] |= ((pFrom)->m_uSb4Pattern6_31_0) & 0xFF;
  (pToData)[737] |= ((pFrom)->m_uSb4Pattern6_31_0 >> 8) &0xFF;
  (pToData)[738] |= ((pFrom)->m_uSb4Pattern6_31_0 >> 16) &0xFF;
  (pToData)[739] |= ((pFrom)->m_uSb4Pattern6_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern6_63_32 */
  (pToData)[740] |= ((pFrom)->m_uSb4Pattern6_63_32) & 0xFF;
  (pToData)[741] |= ((pFrom)->m_uSb4Pattern6_63_32 >> 8) &0xFF;
  (pToData)[742] |= ((pFrom)->m_uSb4Pattern6_63_32 >> 16) &0xFF;
  (pToData)[743] |= ((pFrom)->m_uSb4Pattern6_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern6_95_64 */
  (pToData)[744] |= ((pFrom)->m_uSb4Pattern6_95_64) & 0xFF;
  (pToData)[745] |= ((pFrom)->m_uSb4Pattern6_95_64 >> 8) &0xFF;
  (pToData)[746] |= ((pFrom)->m_uSb4Pattern6_95_64 >> 16) &0xFF;
  (pToData)[747] |= ((pFrom)->m_uSb4Pattern6_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern6_127_96 */
  (pToData)[748] |= ((pFrom)->m_uSb4Pattern6_127_96) & 0xFF;
  (pToData)[749] |= ((pFrom)->m_uSb4Pattern6_127_96 >> 8) &0xFF;
  (pToData)[750] |= ((pFrom)->m_uSb4Pattern6_127_96 >> 16) &0xFF;
  (pToData)[751] |= ((pFrom)->m_uSb4Pattern6_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern7_31_0 */
  (pToData)[752] |= ((pFrom)->m_uSb4Pattern7_31_0) & 0xFF;
  (pToData)[753] |= ((pFrom)->m_uSb4Pattern7_31_0 >> 8) &0xFF;
  (pToData)[754] |= ((pFrom)->m_uSb4Pattern7_31_0 >> 16) &0xFF;
  (pToData)[755] |= ((pFrom)->m_uSb4Pattern7_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern7_63_32 */
  (pToData)[756] |= ((pFrom)->m_uSb4Pattern7_63_32) & 0xFF;
  (pToData)[757] |= ((pFrom)->m_uSb4Pattern7_63_32 >> 8) &0xFF;
  (pToData)[758] |= ((pFrom)->m_uSb4Pattern7_63_32 >> 16) &0xFF;
  (pToData)[759] |= ((pFrom)->m_uSb4Pattern7_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern7_95_64 */
  (pToData)[760] |= ((pFrom)->m_uSb4Pattern7_95_64) & 0xFF;
  (pToData)[761] |= ((pFrom)->m_uSb4Pattern7_95_64 >> 8) &0xFF;
  (pToData)[762] |= ((pFrom)->m_uSb4Pattern7_95_64 >> 16) &0xFF;
  (pToData)[763] |= ((pFrom)->m_uSb4Pattern7_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb4Pattern7_127_96 */
  (pToData)[764] |= ((pFrom)->m_uSb4Pattern7_127_96) & 0xFF;
  (pToData)[765] |= ((pFrom)->m_uSb4Pattern7_127_96 >> 8) &0xFF;
  (pToData)[766] |= ((pFrom)->m_uSb4Pattern7_127_96 >> 16) &0xFF;
  (pToData)[767] |= ((pFrom)->m_uSb4Pattern7_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern0_31_0 */
  (pToData)[768] |= ((pFrom)->m_uSb5Pattern0_31_0) & 0xFF;
  (pToData)[769] |= ((pFrom)->m_uSb5Pattern0_31_0 >> 8) &0xFF;
  (pToData)[770] |= ((pFrom)->m_uSb5Pattern0_31_0 >> 16) &0xFF;
  (pToData)[771] |= ((pFrom)->m_uSb5Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern0_63_32 */
  (pToData)[772] |= ((pFrom)->m_uSb5Pattern0_63_32) & 0xFF;
  (pToData)[773] |= ((pFrom)->m_uSb5Pattern0_63_32 >> 8) &0xFF;
  (pToData)[774] |= ((pFrom)->m_uSb5Pattern0_63_32 >> 16) &0xFF;
  (pToData)[775] |= ((pFrom)->m_uSb5Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern0_95_64 */
  (pToData)[776] |= ((pFrom)->m_uSb5Pattern0_95_64) & 0xFF;
  (pToData)[777] |= ((pFrom)->m_uSb5Pattern0_95_64 >> 8) &0xFF;
  (pToData)[778] |= ((pFrom)->m_uSb5Pattern0_95_64 >> 16) &0xFF;
  (pToData)[779] |= ((pFrom)->m_uSb5Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern0_127_96 */
  (pToData)[780] |= ((pFrom)->m_uSb5Pattern0_127_96) & 0xFF;
  (pToData)[781] |= ((pFrom)->m_uSb5Pattern0_127_96 >> 8) &0xFF;
  (pToData)[782] |= ((pFrom)->m_uSb5Pattern0_127_96 >> 16) &0xFF;
  (pToData)[783] |= ((pFrom)->m_uSb5Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern1_31_0 */
  (pToData)[784] |= ((pFrom)->m_uSb5Pattern1_31_0) & 0xFF;
  (pToData)[785] |= ((pFrom)->m_uSb5Pattern1_31_0 >> 8) &0xFF;
  (pToData)[786] |= ((pFrom)->m_uSb5Pattern1_31_0 >> 16) &0xFF;
  (pToData)[787] |= ((pFrom)->m_uSb5Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern1_63_32 */
  (pToData)[788] |= ((pFrom)->m_uSb5Pattern1_63_32) & 0xFF;
  (pToData)[789] |= ((pFrom)->m_uSb5Pattern1_63_32 >> 8) &0xFF;
  (pToData)[790] |= ((pFrom)->m_uSb5Pattern1_63_32 >> 16) &0xFF;
  (pToData)[791] |= ((pFrom)->m_uSb5Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern1_95_64 */
  (pToData)[792] |= ((pFrom)->m_uSb5Pattern1_95_64) & 0xFF;
  (pToData)[793] |= ((pFrom)->m_uSb5Pattern1_95_64 >> 8) &0xFF;
  (pToData)[794] |= ((pFrom)->m_uSb5Pattern1_95_64 >> 16) &0xFF;
  (pToData)[795] |= ((pFrom)->m_uSb5Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern1_127_96 */
  (pToData)[796] |= ((pFrom)->m_uSb5Pattern1_127_96) & 0xFF;
  (pToData)[797] |= ((pFrom)->m_uSb5Pattern1_127_96 >> 8) &0xFF;
  (pToData)[798] |= ((pFrom)->m_uSb5Pattern1_127_96 >> 16) &0xFF;
  (pToData)[799] |= ((pFrom)->m_uSb5Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern2_31_0 */
  (pToData)[800] |= ((pFrom)->m_uSb5Pattern2_31_0) & 0xFF;
  (pToData)[801] |= ((pFrom)->m_uSb5Pattern2_31_0 >> 8) &0xFF;
  (pToData)[802] |= ((pFrom)->m_uSb5Pattern2_31_0 >> 16) &0xFF;
  (pToData)[803] |= ((pFrom)->m_uSb5Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern2_63_32 */
  (pToData)[804] |= ((pFrom)->m_uSb5Pattern2_63_32) & 0xFF;
  (pToData)[805] |= ((pFrom)->m_uSb5Pattern2_63_32 >> 8) &0xFF;
  (pToData)[806] |= ((pFrom)->m_uSb5Pattern2_63_32 >> 16) &0xFF;
  (pToData)[807] |= ((pFrom)->m_uSb5Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern2_95_64 */
  (pToData)[808] |= ((pFrom)->m_uSb5Pattern2_95_64) & 0xFF;
  (pToData)[809] |= ((pFrom)->m_uSb5Pattern2_95_64 >> 8) &0xFF;
  (pToData)[810] |= ((pFrom)->m_uSb5Pattern2_95_64 >> 16) &0xFF;
  (pToData)[811] |= ((pFrom)->m_uSb5Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern2_127_96 */
  (pToData)[812] |= ((pFrom)->m_uSb5Pattern2_127_96) & 0xFF;
  (pToData)[813] |= ((pFrom)->m_uSb5Pattern2_127_96 >> 8) &0xFF;
  (pToData)[814] |= ((pFrom)->m_uSb5Pattern2_127_96 >> 16) &0xFF;
  (pToData)[815] |= ((pFrom)->m_uSb5Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern3_31_0 */
  (pToData)[816] |= ((pFrom)->m_uSb5Pattern3_31_0) & 0xFF;
  (pToData)[817] |= ((pFrom)->m_uSb5Pattern3_31_0 >> 8) &0xFF;
  (pToData)[818] |= ((pFrom)->m_uSb5Pattern3_31_0 >> 16) &0xFF;
  (pToData)[819] |= ((pFrom)->m_uSb5Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern3_63_32 */
  (pToData)[820] |= ((pFrom)->m_uSb5Pattern3_63_32) & 0xFF;
  (pToData)[821] |= ((pFrom)->m_uSb5Pattern3_63_32 >> 8) &0xFF;
  (pToData)[822] |= ((pFrom)->m_uSb5Pattern3_63_32 >> 16) &0xFF;
  (pToData)[823] |= ((pFrom)->m_uSb5Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern3_95_64 */
  (pToData)[824] |= ((pFrom)->m_uSb5Pattern3_95_64) & 0xFF;
  (pToData)[825] |= ((pFrom)->m_uSb5Pattern3_95_64 >> 8) &0xFF;
  (pToData)[826] |= ((pFrom)->m_uSb5Pattern3_95_64 >> 16) &0xFF;
  (pToData)[827] |= ((pFrom)->m_uSb5Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern3_127_96 */
  (pToData)[828] |= ((pFrom)->m_uSb5Pattern3_127_96) & 0xFF;
  (pToData)[829] |= ((pFrom)->m_uSb5Pattern3_127_96 >> 8) &0xFF;
  (pToData)[830] |= ((pFrom)->m_uSb5Pattern3_127_96 >> 16) &0xFF;
  (pToData)[831] |= ((pFrom)->m_uSb5Pattern3_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern4_31_0 */
  (pToData)[832] |= ((pFrom)->m_uSb5Pattern4_31_0) & 0xFF;
  (pToData)[833] |= ((pFrom)->m_uSb5Pattern4_31_0 >> 8) &0xFF;
  (pToData)[834] |= ((pFrom)->m_uSb5Pattern4_31_0 >> 16) &0xFF;
  (pToData)[835] |= ((pFrom)->m_uSb5Pattern4_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern4_63_32 */
  (pToData)[836] |= ((pFrom)->m_uSb5Pattern4_63_32) & 0xFF;
  (pToData)[837] |= ((pFrom)->m_uSb5Pattern4_63_32 >> 8) &0xFF;
  (pToData)[838] |= ((pFrom)->m_uSb5Pattern4_63_32 >> 16) &0xFF;
  (pToData)[839] |= ((pFrom)->m_uSb5Pattern4_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern4_95_64 */
  (pToData)[840] |= ((pFrom)->m_uSb5Pattern4_95_64) & 0xFF;
  (pToData)[841] |= ((pFrom)->m_uSb5Pattern4_95_64 >> 8) &0xFF;
  (pToData)[842] |= ((pFrom)->m_uSb5Pattern4_95_64 >> 16) &0xFF;
  (pToData)[843] |= ((pFrom)->m_uSb5Pattern4_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern4_127_96 */
  (pToData)[844] |= ((pFrom)->m_uSb5Pattern4_127_96) & 0xFF;
  (pToData)[845] |= ((pFrom)->m_uSb5Pattern4_127_96 >> 8) &0xFF;
  (pToData)[846] |= ((pFrom)->m_uSb5Pattern4_127_96 >> 16) &0xFF;
  (pToData)[847] |= ((pFrom)->m_uSb5Pattern4_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern5_31_0 */
  (pToData)[848] |= ((pFrom)->m_uSb5Pattern5_31_0) & 0xFF;
  (pToData)[849] |= ((pFrom)->m_uSb5Pattern5_31_0 >> 8) &0xFF;
  (pToData)[850] |= ((pFrom)->m_uSb5Pattern5_31_0 >> 16) &0xFF;
  (pToData)[851] |= ((pFrom)->m_uSb5Pattern5_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern5_63_32 */
  (pToData)[852] |= ((pFrom)->m_uSb5Pattern5_63_32) & 0xFF;
  (pToData)[853] |= ((pFrom)->m_uSb5Pattern5_63_32 >> 8) &0xFF;
  (pToData)[854] |= ((pFrom)->m_uSb5Pattern5_63_32 >> 16) &0xFF;
  (pToData)[855] |= ((pFrom)->m_uSb5Pattern5_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern5_95_64 */
  (pToData)[856] |= ((pFrom)->m_uSb5Pattern5_95_64) & 0xFF;
  (pToData)[857] |= ((pFrom)->m_uSb5Pattern5_95_64 >> 8) &0xFF;
  (pToData)[858] |= ((pFrom)->m_uSb5Pattern5_95_64 >> 16) &0xFF;
  (pToData)[859] |= ((pFrom)->m_uSb5Pattern5_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern5_127_96 */
  (pToData)[860] |= ((pFrom)->m_uSb5Pattern5_127_96) & 0xFF;
  (pToData)[861] |= ((pFrom)->m_uSb5Pattern5_127_96 >> 8) &0xFF;
  (pToData)[862] |= ((pFrom)->m_uSb5Pattern5_127_96 >> 16) &0xFF;
  (pToData)[863] |= ((pFrom)->m_uSb5Pattern5_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern6_31_0 */
  (pToData)[864] |= ((pFrom)->m_uSb5Pattern6_31_0) & 0xFF;
  (pToData)[865] |= ((pFrom)->m_uSb5Pattern6_31_0 >> 8) &0xFF;
  (pToData)[866] |= ((pFrom)->m_uSb5Pattern6_31_0 >> 16) &0xFF;
  (pToData)[867] |= ((pFrom)->m_uSb5Pattern6_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern6_63_32 */
  (pToData)[868] |= ((pFrom)->m_uSb5Pattern6_63_32) & 0xFF;
  (pToData)[869] |= ((pFrom)->m_uSb5Pattern6_63_32 >> 8) &0xFF;
  (pToData)[870] |= ((pFrom)->m_uSb5Pattern6_63_32 >> 16) &0xFF;
  (pToData)[871] |= ((pFrom)->m_uSb5Pattern6_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern6_95_64 */
  (pToData)[872] |= ((pFrom)->m_uSb5Pattern6_95_64) & 0xFF;
  (pToData)[873] |= ((pFrom)->m_uSb5Pattern6_95_64 >> 8) &0xFF;
  (pToData)[874] |= ((pFrom)->m_uSb5Pattern6_95_64 >> 16) &0xFF;
  (pToData)[875] |= ((pFrom)->m_uSb5Pattern6_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern6_127_96 */
  (pToData)[876] |= ((pFrom)->m_uSb5Pattern6_127_96) & 0xFF;
  (pToData)[877] |= ((pFrom)->m_uSb5Pattern6_127_96 >> 8) &0xFF;
  (pToData)[878] |= ((pFrom)->m_uSb5Pattern6_127_96 >> 16) &0xFF;
  (pToData)[879] |= ((pFrom)->m_uSb5Pattern6_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern7_31_0 */
  (pToData)[880] |= ((pFrom)->m_uSb5Pattern7_31_0) & 0xFF;
  (pToData)[881] |= ((pFrom)->m_uSb5Pattern7_31_0 >> 8) &0xFF;
  (pToData)[882] |= ((pFrom)->m_uSb5Pattern7_31_0 >> 16) &0xFF;
  (pToData)[883] |= ((pFrom)->m_uSb5Pattern7_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern7_63_32 */
  (pToData)[884] |= ((pFrom)->m_uSb5Pattern7_63_32) & 0xFF;
  (pToData)[885] |= ((pFrom)->m_uSb5Pattern7_63_32 >> 8) &0xFF;
  (pToData)[886] |= ((pFrom)->m_uSb5Pattern7_63_32 >> 16) &0xFF;
  (pToData)[887] |= ((pFrom)->m_uSb5Pattern7_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern7_95_64 */
  (pToData)[888] |= ((pFrom)->m_uSb5Pattern7_95_64) & 0xFF;
  (pToData)[889] |= ((pFrom)->m_uSb5Pattern7_95_64 >> 8) &0xFF;
  (pToData)[890] |= ((pFrom)->m_uSb5Pattern7_95_64 >> 16) &0xFF;
  (pToData)[891] |= ((pFrom)->m_uSb5Pattern7_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb5Pattern7_127_96 */
  (pToData)[892] |= ((pFrom)->m_uSb5Pattern7_127_96) & 0xFF;
  (pToData)[893] |= ((pFrom)->m_uSb5Pattern7_127_96 >> 8) &0xFF;
  (pToData)[894] |= ((pFrom)->m_uSb5Pattern7_127_96 >> 16) &0xFF;
  (pToData)[895] |= ((pFrom)->m_uSb5Pattern7_127_96 >> 24) &0xFF;

  /* Pack Member: m_uResv24 */
  (pToData)[896] |= ((pFrom)->m_uResv24) & 0xFF;
  (pToData)[897] |= ((pFrom)->m_uResv24 >> 8) &0xFF;
  (pToData)[898] |= ((pFrom)->m_uResv24 >> 16) &0xFF;
  (pToData)[899] |= ((pFrom)->m_uResv24 >> 24) &0xFF;

  /* Pack Member: m_uResv25 */
  (pToData)[900] |= ((pFrom)->m_uResv25) & 0xFF;
  (pToData)[901] |= ((pFrom)->m_uResv25 >> 8) &0xFF;
  (pToData)[902] |= ((pFrom)->m_uResv25 >> 16) &0xFF;
  (pToData)[903] |= ((pFrom)->m_uResv25 >> 24) &0xFF;

  /* Pack Member: m_uResv26 */
  (pToData)[904] |= ((pFrom)->m_uResv26) & 0xFF;
  (pToData)[905] |= ((pFrom)->m_uResv26 >> 8) &0xFF;
  (pToData)[906] |= ((pFrom)->m_uResv26 >> 16) &0xFF;
  (pToData)[907] |= ((pFrom)->m_uResv26 >> 24) &0xFF;

  /* Pack Member: m_uResv27 */
  (pToData)[908] |= ((pFrom)->m_uResv27) & 0xFF;
  (pToData)[909] |= ((pFrom)->m_uResv27 >> 8) &0xFF;
  (pToData)[910] |= ((pFrom)->m_uResv27 >> 16) &0xFF;
  (pToData)[911] |= ((pFrom)->m_uResv27 >> 24) &0xFF;

  /* Pack Member: m_uResv28 */
  (pToData)[912] |= ((pFrom)->m_uResv28) & 0xFF;
  (pToData)[913] |= ((pFrom)->m_uResv28 >> 8) &0xFF;
  (pToData)[914] |= ((pFrom)->m_uResv28 >> 16) &0xFF;
  (pToData)[915] |= ((pFrom)->m_uResv28 >> 24) &0xFF;

  /* Pack Member: m_uResv29 */
  (pToData)[916] |= ((pFrom)->m_uResv29) & 0xFF;
  (pToData)[917] |= ((pFrom)->m_uResv29 >> 8) &0xFF;
  (pToData)[918] |= ((pFrom)->m_uResv29 >> 16) &0xFF;
  (pToData)[919] |= ((pFrom)->m_uResv29 >> 24) &0xFF;

  /* Pack Member: m_uResv30 */
  (pToData)[920] |= ((pFrom)->m_uResv30) & 0xFF;
  (pToData)[921] |= ((pFrom)->m_uResv30 >> 8) &0xFF;
  (pToData)[922] |= ((pFrom)->m_uResv30 >> 16) &0xFF;
  (pToData)[923] |= ((pFrom)->m_uResv30 >> 24) &0xFF;

  /* Pack Member: m_uResv31 */
  (pToData)[924] |= ((pFrom)->m_uResv31) & 0xFF;
  (pToData)[925] |= ((pFrom)->m_uResv31 >> 8) &0xFF;
  (pToData)[926] |= ((pFrom)->m_uResv31 >> 16) &0xFF;
  (pToData)[927] |= ((pFrom)->m_uResv31 >> 24) &0xFF;

  /* Pack Member: m_uResv32 */
  (pToData)[928] |= ((pFrom)->m_uResv32) & 0xFF;
  (pToData)[929] |= ((pFrom)->m_uResv32 >> 8) &0xFF;
  (pToData)[930] |= ((pFrom)->m_uResv32 >> 16) &0xFF;
  (pToData)[931] |= ((pFrom)->m_uResv32 >> 24) &0xFF;

  /* Pack Member: m_uResv33 */
  (pToData)[932] |= ((pFrom)->m_uResv33) & 0xFF;
  (pToData)[933] |= ((pFrom)->m_uResv33 >> 8) &0xFF;
  (pToData)[934] |= ((pFrom)->m_uResv33 >> 16) &0xFF;
  (pToData)[935] |= ((pFrom)->m_uResv33 >> 24) &0xFF;

  /* Pack Member: m_uResv34 */
  (pToData)[936] |= ((pFrom)->m_uResv34) & 0xFF;
  (pToData)[937] |= ((pFrom)->m_uResv34 >> 8) &0xFF;
  (pToData)[938] |= ((pFrom)->m_uResv34 >> 16) &0xFF;
  (pToData)[939] |= ((pFrom)->m_uResv34 >> 24) &0xFF;

  /* Pack Member: m_uResv35 */
  (pToData)[940] |= ((pFrom)->m_uResv35) & 0xFF;
  (pToData)[941] |= ((pFrom)->m_uResv35 >> 8) &0xFF;
  (pToData)[942] |= ((pFrom)->m_uResv35 >> 16) &0xFF;
  (pToData)[943] |= ((pFrom)->m_uResv35 >> 24) &0xFF;

  /* Pack Member: m_uResv36 */
  (pToData)[944] |= ((pFrom)->m_uResv36) & 0xFF;
  (pToData)[945] |= ((pFrom)->m_uResv36 >> 8) &0xFF;
  (pToData)[946] |= ((pFrom)->m_uResv36 >> 16) &0xFF;
  (pToData)[947] |= ((pFrom)->m_uResv36 >> 24) &0xFF;

  /* Pack Member: m_uResv37 */
  (pToData)[948] |= ((pFrom)->m_uResv37) & 0xFF;
  (pToData)[949] |= ((pFrom)->m_uResv37 >> 8) &0xFF;
  (pToData)[950] |= ((pFrom)->m_uResv37 >> 16) &0xFF;
  (pToData)[951] |= ((pFrom)->m_uResv37 >> 24) &0xFF;

  /* Pack Member: m_uResv38 */
  (pToData)[952] |= ((pFrom)->m_uResv38) & 0xFF;
  (pToData)[953] |= ((pFrom)->m_uResv38 >> 8) &0xFF;
  (pToData)[954] |= ((pFrom)->m_uResv38 >> 16) &0xFF;
  (pToData)[955] |= ((pFrom)->m_uResv38 >> 24) &0xFF;

  /* Pack Member: m_uResv39 */
  (pToData)[956] |= ((pFrom)->m_uResv39) & 0xFF;
  (pToData)[957] |= ((pFrom)->m_uResv39 >> 8) &0xFF;
  (pToData)[958] |= ((pFrom)->m_uResv39 >> 16) &0xFF;
  (pToData)[959] |= ((pFrom)->m_uResv39 >> 24) &0xFF;

  /* Pack Member: m_uResv40 */
  (pToData)[960] |= ((pFrom)->m_uResv40) & 0xFF;
  (pToData)[961] |= ((pFrom)->m_uResv40 >> 8) &0xFF;
  (pToData)[962] |= ((pFrom)->m_uResv40 >> 16) &0xFF;
  (pToData)[963] |= ((pFrom)->m_uResv40 >> 24) &0xFF;

  /* Pack Member: m_uResv41 */
  (pToData)[964] |= ((pFrom)->m_uResv41) & 0xFF;
  (pToData)[965] |= ((pFrom)->m_uResv41 >> 8) &0xFF;
  (pToData)[966] |= ((pFrom)->m_uResv41 >> 16) &0xFF;
  (pToData)[967] |= ((pFrom)->m_uResv41 >> 24) &0xFF;

  /* Pack Member: m_uResv42 */
  (pToData)[968] |= ((pFrom)->m_uResv42) & 0xFF;
  (pToData)[969] |= ((pFrom)->m_uResv42 >> 8) &0xFF;
  (pToData)[970] |= ((pFrom)->m_uResv42 >> 16) &0xFF;
  (pToData)[971] |= ((pFrom)->m_uResv42 >> 24) &0xFF;

  /* Pack Member: m_uResv43 */
  (pToData)[972] |= ((pFrom)->m_uResv43) & 0xFF;
  (pToData)[973] |= ((pFrom)->m_uResv43 >> 8) &0xFF;
  (pToData)[974] |= ((pFrom)->m_uResv43 >> 16) &0xFF;
  (pToData)[975] |= ((pFrom)->m_uResv43 >> 24) &0xFF;

  /* Pack Member: m_uResv44 */
  (pToData)[976] |= ((pFrom)->m_uResv44) & 0xFF;
  (pToData)[977] |= ((pFrom)->m_uResv44 >> 8) &0xFF;
  (pToData)[978] |= ((pFrom)->m_uResv44 >> 16) &0xFF;
  (pToData)[979] |= ((pFrom)->m_uResv44 >> 24) &0xFF;

  /* Pack Member: m_uResv45 */
  (pToData)[980] |= ((pFrom)->m_uResv45) & 0xFF;
  (pToData)[981] |= ((pFrom)->m_uResv45 >> 8) &0xFF;
  (pToData)[982] |= ((pFrom)->m_uResv45 >> 16) &0xFF;
  (pToData)[983] |= ((pFrom)->m_uResv45 >> 24) &0xFF;

  /* Pack Member: m_uResv46 */
  (pToData)[984] |= ((pFrom)->m_uResv46) & 0xFF;
  (pToData)[985] |= ((pFrom)->m_uResv46 >> 8) &0xFF;
  (pToData)[986] |= ((pFrom)->m_uResv46 >> 16) &0xFF;
  (pToData)[987] |= ((pFrom)->m_uResv46 >> 24) &0xFF;

  /* Pack Member: m_uResv47 */
  (pToData)[988] |= ((pFrom)->m_uResv47) & 0xFF;
  (pToData)[989] |= ((pFrom)->m_uResv47 >> 8) &0xFF;
  (pToData)[990] |= ((pFrom)->m_uResv47 >> 16) &0xFF;
  (pToData)[991] |= ((pFrom)->m_uResv47 >> 24) &0xFF;

  /* Pack Member: m_uResv48 */
  (pToData)[992] |= ((pFrom)->m_uResv48) & 0xFF;
  (pToData)[993] |= ((pFrom)->m_uResv48 >> 8) &0xFF;
  (pToData)[994] |= ((pFrom)->m_uResv48 >> 16) &0xFF;
  (pToData)[995] |= ((pFrom)->m_uResv48 >> 24) &0xFF;

  /* Pack Member: m_uResv49 */
  (pToData)[996] |= ((pFrom)->m_uResv49) & 0xFF;
  (pToData)[997] |= ((pFrom)->m_uResv49 >> 8) &0xFF;
  (pToData)[998] |= ((pFrom)->m_uResv49 >> 16) &0xFF;
  (pToData)[999] |= ((pFrom)->m_uResv49 >> 24) &0xFF;

  /* Pack Member: m_uResv50 */
  (pToData)[1000] |= ((pFrom)->m_uResv50) & 0xFF;
  (pToData)[1001] |= ((pFrom)->m_uResv50 >> 8) &0xFF;
  (pToData)[1002] |= ((pFrom)->m_uResv50 >> 16) &0xFF;
  (pToData)[1003] |= ((pFrom)->m_uResv50 >> 24) &0xFF;

  /* Pack Member: m_uResv51 */
  (pToData)[1004] |= ((pFrom)->m_uResv51) & 0xFF;
  (pToData)[1005] |= ((pFrom)->m_uResv51 >> 8) &0xFF;
  (pToData)[1006] |= ((pFrom)->m_uResv51 >> 16) &0xFF;
  (pToData)[1007] |= ((pFrom)->m_uResv51 >> 24) &0xFF;

  /* Pack Member: m_uResv52 */
  (pToData)[1008] |= ((pFrom)->m_uResv52) & 0xFF;
  (pToData)[1009] |= ((pFrom)->m_uResv52 >> 8) &0xFF;
  (pToData)[1010] |= ((pFrom)->m_uResv52 >> 16) &0xFF;
  (pToData)[1011] |= ((pFrom)->m_uResv52 >> 24) &0xFF;

  /* Pack Member: m_uResv53 */
  (pToData)[1012] |= ((pFrom)->m_uResv53) & 0xFF;
  (pToData)[1013] |= ((pFrom)->m_uResv53 >> 8) &0xFF;
  (pToData)[1014] |= ((pFrom)->m_uResv53 >> 16) &0xFF;
  (pToData)[1015] |= ((pFrom)->m_uResv53 >> 24) &0xFF;

  /* Pack Member: m_uResv54 */
  (pToData)[1016] |= ((pFrom)->m_uResv54) & 0xFF;
  (pToData)[1017] |= ((pFrom)->m_uResv54 >> 8) &0xFF;
  (pToData)[1018] |= ((pFrom)->m_uResv54 >> 16) &0xFF;
  (pToData)[1019] |= ((pFrom)->m_uResv54 >> 24) &0xFF;

  /* Pack Member: m_uResv55 */
  (pToData)[1020] |= ((pFrom)->m_uResv55) & 0xFF;
  (pToData)[1021] |= ((pFrom)->m_uResv55 >> 8) &0xFF;
  (pToData)[1022] |= ((pFrom)->m_uResv55 >> 16) &0xFF;
  (pToData)[1023] |= ((pFrom)->m_uResv55 >> 24) &0xFF;

  return SB_ZF_C2RCDMAFORMAT_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2RcDmaFormat_Unpack(sbZfC2RcDmaFormat_t *pToStruct,
                         uint8_t *pFromData,
                         uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uInstruction0 */
  (pToStruct)->m_uInstruction0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uInstruction0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_uInstruction0 |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_uInstruction0 |=  (uint32_t)  (pFromData)[3] << 24;

  /* Unpack Member: m_uInstruction1 */
  (pToStruct)->m_uInstruction1 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uInstruction1 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uInstruction1 |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_uInstruction1 |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_uInstruction2 */
  (pToStruct)->m_uInstruction2 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_uInstruction2 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_uInstruction2 |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_uInstruction2 |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_uInstruction3 */
  (pToStruct)->m_uInstruction3 =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->m_uInstruction3 |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->m_uInstruction3 |=  (uint32_t)  (pFromData)[14] << 16;
  (pToStruct)->m_uInstruction3 |=  (uint32_t)  (pFromData)[15] << 24;

  /* Unpack Member: m_uInstruction4 */
  (pToStruct)->m_uInstruction4 =  (uint32_t)  (pFromData)[16] ;
  (pToStruct)->m_uInstruction4 |=  (uint32_t)  (pFromData)[17] << 8;
  (pToStruct)->m_uInstruction4 |=  (uint32_t)  (pFromData)[18] << 16;
  (pToStruct)->m_uInstruction4 |=  (uint32_t)  (pFromData)[19] << 24;

  /* Unpack Member: m_uInstruction5 */
  (pToStruct)->m_uInstruction5 =  (uint32_t)  (pFromData)[20] ;
  (pToStruct)->m_uInstruction5 |=  (uint32_t)  (pFromData)[21] << 8;
  (pToStruct)->m_uInstruction5 |=  (uint32_t)  (pFromData)[22] << 16;
  (pToStruct)->m_uInstruction5 |=  (uint32_t)  (pFromData)[23] << 24;

  /* Unpack Member: m_uInstruction6 */
  (pToStruct)->m_uInstruction6 =  (uint32_t)  (pFromData)[24] ;
  (pToStruct)->m_uInstruction6 |=  (uint32_t)  (pFromData)[25] << 8;
  (pToStruct)->m_uInstruction6 |=  (uint32_t)  (pFromData)[26] << 16;
  (pToStruct)->m_uInstruction6 |=  (uint32_t)  (pFromData)[27] << 24;

  /* Unpack Member: m_uInstruction7 */
  (pToStruct)->m_uInstruction7 =  (uint32_t)  (pFromData)[28] ;
  (pToStruct)->m_uInstruction7 |=  (uint32_t)  (pFromData)[29] << 8;
  (pToStruct)->m_uInstruction7 |=  (uint32_t)  (pFromData)[30] << 16;
  (pToStruct)->m_uInstruction7 |=  (uint32_t)  (pFromData)[31] << 24;

  /* Unpack Member: m_uResv0 */
  (pToStruct)->m_uResv0 =  (uint32_t)  (pFromData)[32] ;
  (pToStruct)->m_uResv0 |=  (uint32_t)  (pFromData)[33] << 8;
  (pToStruct)->m_uResv0 |=  (uint32_t)  (pFromData)[34] << 16;
  (pToStruct)->m_uResv0 |=  (uint32_t)  (pFromData)[35] << 24;

  /* Unpack Member: m_uResv1 */
  (pToStruct)->m_uResv1 =  (uint32_t)  (pFromData)[36] ;
  (pToStruct)->m_uResv1 |=  (uint32_t)  (pFromData)[37] << 8;
  (pToStruct)->m_uResv1 |=  (uint32_t)  (pFromData)[38] << 16;
  (pToStruct)->m_uResv1 |=  (uint32_t)  (pFromData)[39] << 24;

  /* Unpack Member: m_uResv2 */
  (pToStruct)->m_uResv2 =  (uint32_t)  (pFromData)[40] ;
  (pToStruct)->m_uResv2 |=  (uint32_t)  (pFromData)[41] << 8;
  (pToStruct)->m_uResv2 |=  (uint32_t)  (pFromData)[42] << 16;
  (pToStruct)->m_uResv2 |=  (uint32_t)  (pFromData)[43] << 24;

  /* Unpack Member: m_uResv3 */
  (pToStruct)->m_uResv3 =  (uint32_t)  (pFromData)[44] ;
  (pToStruct)->m_uResv3 |=  (uint32_t)  (pFromData)[45] << 8;
  (pToStruct)->m_uResv3 |=  (uint32_t)  (pFromData)[46] << 16;
  (pToStruct)->m_uResv3 |=  (uint32_t)  (pFromData)[47] << 24;

  /* Unpack Member: m_uResv4 */
  (pToStruct)->m_uResv4 =  (uint32_t)  (pFromData)[48] ;
  (pToStruct)->m_uResv4 |=  (uint32_t)  (pFromData)[49] << 8;
  (pToStruct)->m_uResv4 |=  (uint32_t)  (pFromData)[50] << 16;
  (pToStruct)->m_uResv4 |=  (uint32_t)  (pFromData)[51] << 24;

  /* Unpack Member: m_uResv5 */
  (pToStruct)->m_uResv5 =  (uint32_t)  (pFromData)[52] ;
  (pToStruct)->m_uResv5 |=  (uint32_t)  (pFromData)[53] << 8;
  (pToStruct)->m_uResv5 |=  (uint32_t)  (pFromData)[54] << 16;
  (pToStruct)->m_uResv5 |=  (uint32_t)  (pFromData)[55] << 24;

  /* Unpack Member: m_uResv6 */
  (pToStruct)->m_uResv6 =  (uint32_t)  (pFromData)[56] ;
  (pToStruct)->m_uResv6 |=  (uint32_t)  (pFromData)[57] << 8;
  (pToStruct)->m_uResv6 |=  (uint32_t)  (pFromData)[58] << 16;
  (pToStruct)->m_uResv6 |=  (uint32_t)  (pFromData)[59] << 24;

  /* Unpack Member: m_uResv7 */
  (pToStruct)->m_uResv7 =  (uint32_t)  (pFromData)[60] ;
  (pToStruct)->m_uResv7 |=  (uint32_t)  (pFromData)[61] << 8;
  (pToStruct)->m_uResv7 |=  (uint32_t)  (pFromData)[62] << 16;
  (pToStruct)->m_uResv7 |=  (uint32_t)  (pFromData)[63] << 24;

  /* Unpack Member: m_uResv8 */
  (pToStruct)->m_uResv8 =  (uint32_t)  (pFromData)[64] ;
  (pToStruct)->m_uResv8 |=  (uint32_t)  (pFromData)[65] << 8;
  (pToStruct)->m_uResv8 |=  (uint32_t)  (pFromData)[66] << 16;
  (pToStruct)->m_uResv8 |=  (uint32_t)  (pFromData)[67] << 24;

  /* Unpack Member: m_uResv9 */
  (pToStruct)->m_uResv9 =  (uint32_t)  (pFromData)[68] ;
  (pToStruct)->m_uResv9 |=  (uint32_t)  (pFromData)[69] << 8;
  (pToStruct)->m_uResv9 |=  (uint32_t)  (pFromData)[70] << 16;
  (pToStruct)->m_uResv9 |=  (uint32_t)  (pFromData)[71] << 24;

  /* Unpack Member: m_uResv10 */
  (pToStruct)->m_uResv10 =  (uint32_t)  (pFromData)[72] ;
  (pToStruct)->m_uResv10 |=  (uint32_t)  (pFromData)[73] << 8;
  (pToStruct)->m_uResv10 |=  (uint32_t)  (pFromData)[74] << 16;
  (pToStruct)->m_uResv10 |=  (uint32_t)  (pFromData)[75] << 24;

  /* Unpack Member: m_uResv11 */
  (pToStruct)->m_uResv11 =  (uint32_t)  (pFromData)[76] ;
  (pToStruct)->m_uResv11 |=  (uint32_t)  (pFromData)[77] << 8;
  (pToStruct)->m_uResv11 |=  (uint32_t)  (pFromData)[78] << 16;
  (pToStruct)->m_uResv11 |=  (uint32_t)  (pFromData)[79] << 24;

  /* Unpack Member: m_uResv12 */
  (pToStruct)->m_uResv12 =  (uint32_t)  (pFromData)[80] ;
  (pToStruct)->m_uResv12 |=  (uint32_t)  (pFromData)[81] << 8;
  (pToStruct)->m_uResv12 |=  (uint32_t)  (pFromData)[82] << 16;
  (pToStruct)->m_uResv12 |=  (uint32_t)  (pFromData)[83] << 24;

  /* Unpack Member: m_uResv13 */
  (pToStruct)->m_uResv13 =  (uint32_t)  (pFromData)[84] ;
  (pToStruct)->m_uResv13 |=  (uint32_t)  (pFromData)[85] << 8;
  (pToStruct)->m_uResv13 |=  (uint32_t)  (pFromData)[86] << 16;
  (pToStruct)->m_uResv13 |=  (uint32_t)  (pFromData)[87] << 24;

  /* Unpack Member: m_uResv14 */
  (pToStruct)->m_uResv14 =  (uint32_t)  (pFromData)[88] ;
  (pToStruct)->m_uResv14 |=  (uint32_t)  (pFromData)[89] << 8;
  (pToStruct)->m_uResv14 |=  (uint32_t)  (pFromData)[90] << 16;
  (pToStruct)->m_uResv14 |=  (uint32_t)  (pFromData)[91] << 24;

  /* Unpack Member: m_uResv15 */
  (pToStruct)->m_uResv15 =  (uint32_t)  (pFromData)[92] ;
  (pToStruct)->m_uResv15 |=  (uint32_t)  (pFromData)[93] << 8;
  (pToStruct)->m_uResv15 |=  (uint32_t)  (pFromData)[94] << 16;
  (pToStruct)->m_uResv15 |=  (uint32_t)  (pFromData)[95] << 24;

  /* Unpack Member: m_uResv16 */
  (pToStruct)->m_uResv16 =  (uint32_t)  (pFromData)[96] ;
  (pToStruct)->m_uResv16 |=  (uint32_t)  (pFromData)[97] << 8;
  (pToStruct)->m_uResv16 |=  (uint32_t)  (pFromData)[98] << 16;
  (pToStruct)->m_uResv16 |=  (uint32_t)  (pFromData)[99] << 24;

  /* Unpack Member: m_uResv17 */
  (pToStruct)->m_uResv17 =  (uint32_t)  (pFromData)[100] ;
  (pToStruct)->m_uResv17 |=  (uint32_t)  (pFromData)[101] << 8;
  (pToStruct)->m_uResv17 |=  (uint32_t)  (pFromData)[102] << 16;
  (pToStruct)->m_uResv17 |=  (uint32_t)  (pFromData)[103] << 24;

  /* Unpack Member: m_uResv18 */
  (pToStruct)->m_uResv18 =  (uint32_t)  (pFromData)[104] ;
  (pToStruct)->m_uResv18 |=  (uint32_t)  (pFromData)[105] << 8;
  (pToStruct)->m_uResv18 |=  (uint32_t)  (pFromData)[106] << 16;
  (pToStruct)->m_uResv18 |=  (uint32_t)  (pFromData)[107] << 24;

  /* Unpack Member: m_uResv19 */
  (pToStruct)->m_uResv19 =  (uint32_t)  (pFromData)[108] ;
  (pToStruct)->m_uResv19 |=  (uint32_t)  (pFromData)[109] << 8;
  (pToStruct)->m_uResv19 |=  (uint32_t)  (pFromData)[110] << 16;
  (pToStruct)->m_uResv19 |=  (uint32_t)  (pFromData)[111] << 24;

  /* Unpack Member: m_uResv20 */
  (pToStruct)->m_uResv20 =  (uint32_t)  (pFromData)[112] ;
  (pToStruct)->m_uResv20 |=  (uint32_t)  (pFromData)[113] << 8;
  (pToStruct)->m_uResv20 |=  (uint32_t)  (pFromData)[114] << 16;
  (pToStruct)->m_uResv20 |=  (uint32_t)  (pFromData)[115] << 24;

  /* Unpack Member: m_uResv21 */
  (pToStruct)->m_uResv21 =  (uint32_t)  (pFromData)[116] ;
  (pToStruct)->m_uResv21 |=  (uint32_t)  (pFromData)[117] << 8;
  (pToStruct)->m_uResv21 |=  (uint32_t)  (pFromData)[118] << 16;
  (pToStruct)->m_uResv21 |=  (uint32_t)  (pFromData)[119] << 24;

  /* Unpack Member: m_uResv22 */
  (pToStruct)->m_uResv22 =  (uint32_t)  (pFromData)[120] ;
  (pToStruct)->m_uResv22 |=  (uint32_t)  (pFromData)[121] << 8;
  (pToStruct)->m_uResv22 |=  (uint32_t)  (pFromData)[122] << 16;
  (pToStruct)->m_uResv22 |=  (uint32_t)  (pFromData)[123] << 24;

  /* Unpack Member: m_uResv23 */
  (pToStruct)->m_uResv23 =  (uint32_t)  (pFromData)[124] ;
  (pToStruct)->m_uResv23 |=  (uint32_t)  (pFromData)[125] << 8;
  (pToStruct)->m_uResv23 |=  (uint32_t)  (pFromData)[126] << 16;
  (pToStruct)->m_uResv23 |=  (uint32_t)  (pFromData)[127] << 24;

  /* Unpack Member: m_uSb0Pattern0_31_0 */
  (pToStruct)->m_uSb0Pattern0_31_0 =  (uint32_t)  (pFromData)[128] ;
  (pToStruct)->m_uSb0Pattern0_31_0 |=  (uint32_t)  (pFromData)[129] << 8;
  (pToStruct)->m_uSb0Pattern0_31_0 |=  (uint32_t)  (pFromData)[130] << 16;
  (pToStruct)->m_uSb0Pattern0_31_0 |=  (uint32_t)  (pFromData)[131] << 24;

  /* Unpack Member: m_uSb0Pattern0_63_32 */
  (pToStruct)->m_uSb0Pattern0_63_32 =  (uint32_t)  (pFromData)[132] ;
  (pToStruct)->m_uSb0Pattern0_63_32 |=  (uint32_t)  (pFromData)[133] << 8;
  (pToStruct)->m_uSb0Pattern0_63_32 |=  (uint32_t)  (pFromData)[134] << 16;
  (pToStruct)->m_uSb0Pattern0_63_32 |=  (uint32_t)  (pFromData)[135] << 24;

  /* Unpack Member: m_uSb0Pattern0_95_64 */
  (pToStruct)->m_uSb0Pattern0_95_64 =  (uint32_t)  (pFromData)[136] ;
  (pToStruct)->m_uSb0Pattern0_95_64 |=  (uint32_t)  (pFromData)[137] << 8;
  (pToStruct)->m_uSb0Pattern0_95_64 |=  (uint32_t)  (pFromData)[138] << 16;
  (pToStruct)->m_uSb0Pattern0_95_64 |=  (uint32_t)  (pFromData)[139] << 24;

  /* Unpack Member: m_uSb0Pattern0_127_96 */
  (pToStruct)->m_uSb0Pattern0_127_96 =  (uint32_t)  (pFromData)[140] ;
  (pToStruct)->m_uSb0Pattern0_127_96 |=  (uint32_t)  (pFromData)[141] << 8;
  (pToStruct)->m_uSb0Pattern0_127_96 |=  (uint32_t)  (pFromData)[142] << 16;
  (pToStruct)->m_uSb0Pattern0_127_96 |=  (uint32_t)  (pFromData)[143] << 24;

  /* Unpack Member: m_uSb0Pattern1_31_0 */
  (pToStruct)->m_uSb0Pattern1_31_0 =  (uint32_t)  (pFromData)[144] ;
  (pToStruct)->m_uSb0Pattern1_31_0 |=  (uint32_t)  (pFromData)[145] << 8;
  (pToStruct)->m_uSb0Pattern1_31_0 |=  (uint32_t)  (pFromData)[146] << 16;
  (pToStruct)->m_uSb0Pattern1_31_0 |=  (uint32_t)  (pFromData)[147] << 24;

  /* Unpack Member: m_uSb0Pattern1_63_32 */
  (pToStruct)->m_uSb0Pattern1_63_32 =  (uint32_t)  (pFromData)[148] ;
  (pToStruct)->m_uSb0Pattern1_63_32 |=  (uint32_t)  (pFromData)[149] << 8;
  (pToStruct)->m_uSb0Pattern1_63_32 |=  (uint32_t)  (pFromData)[150] << 16;
  (pToStruct)->m_uSb0Pattern1_63_32 |=  (uint32_t)  (pFromData)[151] << 24;

  /* Unpack Member: m_uSb0Pattern1_95_64 */
  (pToStruct)->m_uSb0Pattern1_95_64 =  (uint32_t)  (pFromData)[152] ;
  (pToStruct)->m_uSb0Pattern1_95_64 |=  (uint32_t)  (pFromData)[153] << 8;
  (pToStruct)->m_uSb0Pattern1_95_64 |=  (uint32_t)  (pFromData)[154] << 16;
  (pToStruct)->m_uSb0Pattern1_95_64 |=  (uint32_t)  (pFromData)[155] << 24;

  /* Unpack Member: m_uSb0Pattern1_127_96 */
  (pToStruct)->m_uSb0Pattern1_127_96 =  (uint32_t)  (pFromData)[156] ;
  (pToStruct)->m_uSb0Pattern1_127_96 |=  (uint32_t)  (pFromData)[157] << 8;
  (pToStruct)->m_uSb0Pattern1_127_96 |=  (uint32_t)  (pFromData)[158] << 16;
  (pToStruct)->m_uSb0Pattern1_127_96 |=  (uint32_t)  (pFromData)[159] << 24;

  /* Unpack Member: m_uSb0Pattern2_31_0 */
  (pToStruct)->m_uSb0Pattern2_31_0 =  (uint32_t)  (pFromData)[160] ;
  (pToStruct)->m_uSb0Pattern2_31_0 |=  (uint32_t)  (pFromData)[161] << 8;
  (pToStruct)->m_uSb0Pattern2_31_0 |=  (uint32_t)  (pFromData)[162] << 16;
  (pToStruct)->m_uSb0Pattern2_31_0 |=  (uint32_t)  (pFromData)[163] << 24;

  /* Unpack Member: m_uSb0Pattern2_63_32 */
  (pToStruct)->m_uSb0Pattern2_63_32 =  (uint32_t)  (pFromData)[164] ;
  (pToStruct)->m_uSb0Pattern2_63_32 |=  (uint32_t)  (pFromData)[165] << 8;
  (pToStruct)->m_uSb0Pattern2_63_32 |=  (uint32_t)  (pFromData)[166] << 16;
  (pToStruct)->m_uSb0Pattern2_63_32 |=  (uint32_t)  (pFromData)[167] << 24;

  /* Unpack Member: m_uSb0Pattern2_95_64 */
  (pToStruct)->m_uSb0Pattern2_95_64 =  (uint32_t)  (pFromData)[168] ;
  (pToStruct)->m_uSb0Pattern2_95_64 |=  (uint32_t)  (pFromData)[169] << 8;
  (pToStruct)->m_uSb0Pattern2_95_64 |=  (uint32_t)  (pFromData)[170] << 16;
  (pToStruct)->m_uSb0Pattern2_95_64 |=  (uint32_t)  (pFromData)[171] << 24;

  /* Unpack Member: m_uSb0Pattern2_127_96 */
  (pToStruct)->m_uSb0Pattern2_127_96 =  (uint32_t)  (pFromData)[172] ;
  (pToStruct)->m_uSb0Pattern2_127_96 |=  (uint32_t)  (pFromData)[173] << 8;
  (pToStruct)->m_uSb0Pattern2_127_96 |=  (uint32_t)  (pFromData)[174] << 16;
  (pToStruct)->m_uSb0Pattern2_127_96 |=  (uint32_t)  (pFromData)[175] << 24;

  /* Unpack Member: m_uSb0Pattern3_31_0 */
  (pToStruct)->m_uSb0Pattern3_31_0 =  (uint32_t)  (pFromData)[176] ;
  (pToStruct)->m_uSb0Pattern3_31_0 |=  (uint32_t)  (pFromData)[177] << 8;
  (pToStruct)->m_uSb0Pattern3_31_0 |=  (uint32_t)  (pFromData)[178] << 16;
  (pToStruct)->m_uSb0Pattern3_31_0 |=  (uint32_t)  (pFromData)[179] << 24;

  /* Unpack Member: m_uSb0Pattern3_63_32 */
  (pToStruct)->m_uSb0Pattern3_63_32 =  (uint32_t)  (pFromData)[180] ;
  (pToStruct)->m_uSb0Pattern3_63_32 |=  (uint32_t)  (pFromData)[181] << 8;
  (pToStruct)->m_uSb0Pattern3_63_32 |=  (uint32_t)  (pFromData)[182] << 16;
  (pToStruct)->m_uSb0Pattern3_63_32 |=  (uint32_t)  (pFromData)[183] << 24;

  /* Unpack Member: m_uSb0Pattern3_95_64 */
  (pToStruct)->m_uSb0Pattern3_95_64 =  (uint32_t)  (pFromData)[184] ;
  (pToStruct)->m_uSb0Pattern3_95_64 |=  (uint32_t)  (pFromData)[185] << 8;
  (pToStruct)->m_uSb0Pattern3_95_64 |=  (uint32_t)  (pFromData)[186] << 16;
  (pToStruct)->m_uSb0Pattern3_95_64 |=  (uint32_t)  (pFromData)[187] << 24;

  /* Unpack Member: m_uSb0Pattern3_127_96 */
  (pToStruct)->m_uSb0Pattern3_127_96 =  (uint32_t)  (pFromData)[188] ;
  (pToStruct)->m_uSb0Pattern3_127_96 |=  (uint32_t)  (pFromData)[189] << 8;
  (pToStruct)->m_uSb0Pattern3_127_96 |=  (uint32_t)  (pFromData)[190] << 16;
  (pToStruct)->m_uSb0Pattern3_127_96 |=  (uint32_t)  (pFromData)[191] << 24;

  /* Unpack Member: m_uSb0Pattern4_31_0 */
  (pToStruct)->m_uSb0Pattern4_31_0 =  (uint32_t)  (pFromData)[192] ;
  (pToStruct)->m_uSb0Pattern4_31_0 |=  (uint32_t)  (pFromData)[193] << 8;
  (pToStruct)->m_uSb0Pattern4_31_0 |=  (uint32_t)  (pFromData)[194] << 16;
  (pToStruct)->m_uSb0Pattern4_31_0 |=  (uint32_t)  (pFromData)[195] << 24;

  /* Unpack Member: m_uSb0Pattern4_63_32 */
  (pToStruct)->m_uSb0Pattern4_63_32 =  (uint32_t)  (pFromData)[196] ;
  (pToStruct)->m_uSb0Pattern4_63_32 |=  (uint32_t)  (pFromData)[197] << 8;
  (pToStruct)->m_uSb0Pattern4_63_32 |=  (uint32_t)  (pFromData)[198] << 16;
  (pToStruct)->m_uSb0Pattern4_63_32 |=  (uint32_t)  (pFromData)[199] << 24;

  /* Unpack Member: m_uSb0Pattern4_95_64 */
  (pToStruct)->m_uSb0Pattern4_95_64 =  (uint32_t)  (pFromData)[200] ;
  (pToStruct)->m_uSb0Pattern4_95_64 |=  (uint32_t)  (pFromData)[201] << 8;
  (pToStruct)->m_uSb0Pattern4_95_64 |=  (uint32_t)  (pFromData)[202] << 16;
  (pToStruct)->m_uSb0Pattern4_95_64 |=  (uint32_t)  (pFromData)[203] << 24;

  /* Unpack Member: m_uSb0Pattern4_127_96 */
  (pToStruct)->m_uSb0Pattern4_127_96 =  (uint32_t)  (pFromData)[204] ;
  (pToStruct)->m_uSb0Pattern4_127_96 |=  (uint32_t)  (pFromData)[205] << 8;
  (pToStruct)->m_uSb0Pattern4_127_96 |=  (uint32_t)  (pFromData)[206] << 16;
  (pToStruct)->m_uSb0Pattern4_127_96 |=  (uint32_t)  (pFromData)[207] << 24;

  /* Unpack Member: m_uSb0Pattern5_31_0 */
  (pToStruct)->m_uSb0Pattern5_31_0 =  (uint32_t)  (pFromData)[208] ;
  (pToStruct)->m_uSb0Pattern5_31_0 |=  (uint32_t)  (pFromData)[209] << 8;
  (pToStruct)->m_uSb0Pattern5_31_0 |=  (uint32_t)  (pFromData)[210] << 16;
  (pToStruct)->m_uSb0Pattern5_31_0 |=  (uint32_t)  (pFromData)[211] << 24;

  /* Unpack Member: m_uSb0Pattern5_63_32 */
  (pToStruct)->m_uSb0Pattern5_63_32 =  (uint32_t)  (pFromData)[212] ;
  (pToStruct)->m_uSb0Pattern5_63_32 |=  (uint32_t)  (pFromData)[213] << 8;
  (pToStruct)->m_uSb0Pattern5_63_32 |=  (uint32_t)  (pFromData)[214] << 16;
  (pToStruct)->m_uSb0Pattern5_63_32 |=  (uint32_t)  (pFromData)[215] << 24;

  /* Unpack Member: m_uSb0Pattern5_95_64 */
  (pToStruct)->m_uSb0Pattern5_95_64 =  (uint32_t)  (pFromData)[216] ;
  (pToStruct)->m_uSb0Pattern5_95_64 |=  (uint32_t)  (pFromData)[217] << 8;
  (pToStruct)->m_uSb0Pattern5_95_64 |=  (uint32_t)  (pFromData)[218] << 16;
  (pToStruct)->m_uSb0Pattern5_95_64 |=  (uint32_t)  (pFromData)[219] << 24;

  /* Unpack Member: m_uSb0Pattern5_127_96 */
  (pToStruct)->m_uSb0Pattern5_127_96 =  (uint32_t)  (pFromData)[220] ;
  (pToStruct)->m_uSb0Pattern5_127_96 |=  (uint32_t)  (pFromData)[221] << 8;
  (pToStruct)->m_uSb0Pattern5_127_96 |=  (uint32_t)  (pFromData)[222] << 16;
  (pToStruct)->m_uSb0Pattern5_127_96 |=  (uint32_t)  (pFromData)[223] << 24;

  /* Unpack Member: m_uSb0Pattern6_31_0 */
  (pToStruct)->m_uSb0Pattern6_31_0 =  (uint32_t)  (pFromData)[224] ;
  (pToStruct)->m_uSb0Pattern6_31_0 |=  (uint32_t)  (pFromData)[225] << 8;
  (pToStruct)->m_uSb0Pattern6_31_0 |=  (uint32_t)  (pFromData)[226] << 16;
  (pToStruct)->m_uSb0Pattern6_31_0 |=  (uint32_t)  (pFromData)[227] << 24;

  /* Unpack Member: m_uSb0Pattern6_63_32 */
  (pToStruct)->m_uSb0Pattern6_63_32 =  (uint32_t)  (pFromData)[228] ;
  (pToStruct)->m_uSb0Pattern6_63_32 |=  (uint32_t)  (pFromData)[229] << 8;
  (pToStruct)->m_uSb0Pattern6_63_32 |=  (uint32_t)  (pFromData)[230] << 16;
  (pToStruct)->m_uSb0Pattern6_63_32 |=  (uint32_t)  (pFromData)[231] << 24;

  /* Unpack Member: m_uSb0Pattern6_95_64 */
  (pToStruct)->m_uSb0Pattern6_95_64 =  (uint32_t)  (pFromData)[232] ;
  (pToStruct)->m_uSb0Pattern6_95_64 |=  (uint32_t)  (pFromData)[233] << 8;
  (pToStruct)->m_uSb0Pattern6_95_64 |=  (uint32_t)  (pFromData)[234] << 16;
  (pToStruct)->m_uSb0Pattern6_95_64 |=  (uint32_t)  (pFromData)[235] << 24;

  /* Unpack Member: m_uSb0Pattern6_127_96 */
  (pToStruct)->m_uSb0Pattern6_127_96 =  (uint32_t)  (pFromData)[236] ;
  (pToStruct)->m_uSb0Pattern6_127_96 |=  (uint32_t)  (pFromData)[237] << 8;
  (pToStruct)->m_uSb0Pattern6_127_96 |=  (uint32_t)  (pFromData)[238] << 16;
  (pToStruct)->m_uSb0Pattern6_127_96 |=  (uint32_t)  (pFromData)[239] << 24;

  /* Unpack Member: m_uSb0Pattern7_31_0 */
  (pToStruct)->m_uSb0Pattern7_31_0 =  (uint32_t)  (pFromData)[240] ;
  (pToStruct)->m_uSb0Pattern7_31_0 |=  (uint32_t)  (pFromData)[241] << 8;
  (pToStruct)->m_uSb0Pattern7_31_0 |=  (uint32_t)  (pFromData)[242] << 16;
  (pToStruct)->m_uSb0Pattern7_31_0 |=  (uint32_t)  (pFromData)[243] << 24;

  /* Unpack Member: m_uSb0Pattern7_63_32 */
  (pToStruct)->m_uSb0Pattern7_63_32 =  (uint32_t)  (pFromData)[244] ;
  (pToStruct)->m_uSb0Pattern7_63_32 |=  (uint32_t)  (pFromData)[245] << 8;
  (pToStruct)->m_uSb0Pattern7_63_32 |=  (uint32_t)  (pFromData)[246] << 16;
  (pToStruct)->m_uSb0Pattern7_63_32 |=  (uint32_t)  (pFromData)[247] << 24;

  /* Unpack Member: m_uSb0Pattern7_95_64 */
  (pToStruct)->m_uSb0Pattern7_95_64 =  (uint32_t)  (pFromData)[248] ;
  (pToStruct)->m_uSb0Pattern7_95_64 |=  (uint32_t)  (pFromData)[249] << 8;
  (pToStruct)->m_uSb0Pattern7_95_64 |=  (uint32_t)  (pFromData)[250] << 16;
  (pToStruct)->m_uSb0Pattern7_95_64 |=  (uint32_t)  (pFromData)[251] << 24;

  /* Unpack Member: m_uSb0Pattern7_127_96 */
  (pToStruct)->m_uSb0Pattern7_127_96 =  (uint32_t)  (pFromData)[252] ;
  (pToStruct)->m_uSb0Pattern7_127_96 |=  (uint32_t)  (pFromData)[253] << 8;
  (pToStruct)->m_uSb0Pattern7_127_96 |=  (uint32_t)  (pFromData)[254] << 16;
  (pToStruct)->m_uSb0Pattern7_127_96 |=  (uint32_t)  (pFromData)[255] << 24;

  /* Unpack Member: m_uSb1Pattern0_31_0 */
  (pToStruct)->m_uSb1Pattern0_31_0 =  (uint32_t)  (pFromData)[256] ;
  (pToStruct)->m_uSb1Pattern0_31_0 |=  (uint32_t)  (pFromData)[257] << 8;
  (pToStruct)->m_uSb1Pattern0_31_0 |=  (uint32_t)  (pFromData)[258] << 16;
  (pToStruct)->m_uSb1Pattern0_31_0 |=  (uint32_t)  (pFromData)[259] << 24;

  /* Unpack Member: m_uSb1Pattern0_63_32 */
  (pToStruct)->m_uSb1Pattern0_63_32 =  (uint32_t)  (pFromData)[260] ;
  (pToStruct)->m_uSb1Pattern0_63_32 |=  (uint32_t)  (pFromData)[261] << 8;
  (pToStruct)->m_uSb1Pattern0_63_32 |=  (uint32_t)  (pFromData)[262] << 16;
  (pToStruct)->m_uSb1Pattern0_63_32 |=  (uint32_t)  (pFromData)[263] << 24;

  /* Unpack Member: m_uSb1Pattern0_95_64 */
  (pToStruct)->m_uSb1Pattern0_95_64 =  (uint32_t)  (pFromData)[264] ;
  (pToStruct)->m_uSb1Pattern0_95_64 |=  (uint32_t)  (pFromData)[265] << 8;
  (pToStruct)->m_uSb1Pattern0_95_64 |=  (uint32_t)  (pFromData)[266] << 16;
  (pToStruct)->m_uSb1Pattern0_95_64 |=  (uint32_t)  (pFromData)[267] << 24;

  /* Unpack Member: m_uSb1Pattern0_127_96 */
  (pToStruct)->m_uSb1Pattern0_127_96 =  (uint32_t)  (pFromData)[268] ;
  (pToStruct)->m_uSb1Pattern0_127_96 |=  (uint32_t)  (pFromData)[269] << 8;
  (pToStruct)->m_uSb1Pattern0_127_96 |=  (uint32_t)  (pFromData)[270] << 16;
  (pToStruct)->m_uSb1Pattern0_127_96 |=  (uint32_t)  (pFromData)[271] << 24;

  /* Unpack Member: m_uSb1Pattern1_31_0 */
  (pToStruct)->m_uSb1Pattern1_31_0 =  (uint32_t)  (pFromData)[272] ;
  (pToStruct)->m_uSb1Pattern1_31_0 |=  (uint32_t)  (pFromData)[273] << 8;
  (pToStruct)->m_uSb1Pattern1_31_0 |=  (uint32_t)  (pFromData)[274] << 16;
  (pToStruct)->m_uSb1Pattern1_31_0 |=  (uint32_t)  (pFromData)[275] << 24;

  /* Unpack Member: m_uSb1Pattern1_63_32 */
  (pToStruct)->m_uSb1Pattern1_63_32 =  (uint32_t)  (pFromData)[276] ;
  (pToStruct)->m_uSb1Pattern1_63_32 |=  (uint32_t)  (pFromData)[277] << 8;
  (pToStruct)->m_uSb1Pattern1_63_32 |=  (uint32_t)  (pFromData)[278] << 16;
  (pToStruct)->m_uSb1Pattern1_63_32 |=  (uint32_t)  (pFromData)[279] << 24;

  /* Unpack Member: m_uSb1Pattern1_95_64 */
  (pToStruct)->m_uSb1Pattern1_95_64 =  (uint32_t)  (pFromData)[280] ;
  (pToStruct)->m_uSb1Pattern1_95_64 |=  (uint32_t)  (pFromData)[281] << 8;
  (pToStruct)->m_uSb1Pattern1_95_64 |=  (uint32_t)  (pFromData)[282] << 16;
  (pToStruct)->m_uSb1Pattern1_95_64 |=  (uint32_t)  (pFromData)[283] << 24;

  /* Unpack Member: m_uSb1Pattern1_127_96 */
  (pToStruct)->m_uSb1Pattern1_127_96 =  (uint32_t)  (pFromData)[284] ;
  (pToStruct)->m_uSb1Pattern1_127_96 |=  (uint32_t)  (pFromData)[285] << 8;
  (pToStruct)->m_uSb1Pattern1_127_96 |=  (uint32_t)  (pFromData)[286] << 16;
  (pToStruct)->m_uSb1Pattern1_127_96 |=  (uint32_t)  (pFromData)[287] << 24;

  /* Unpack Member: m_uSb1Pattern2_31_0 */
  (pToStruct)->m_uSb1Pattern2_31_0 =  (uint32_t)  (pFromData)[288] ;
  (pToStruct)->m_uSb1Pattern2_31_0 |=  (uint32_t)  (pFromData)[289] << 8;
  (pToStruct)->m_uSb1Pattern2_31_0 |=  (uint32_t)  (pFromData)[290] << 16;
  (pToStruct)->m_uSb1Pattern2_31_0 |=  (uint32_t)  (pFromData)[291] << 24;

  /* Unpack Member: m_uSb1Pattern2_63_32 */
  (pToStruct)->m_uSb1Pattern2_63_32 =  (uint32_t)  (pFromData)[292] ;
  (pToStruct)->m_uSb1Pattern2_63_32 |=  (uint32_t)  (pFromData)[293] << 8;
  (pToStruct)->m_uSb1Pattern2_63_32 |=  (uint32_t)  (pFromData)[294] << 16;
  (pToStruct)->m_uSb1Pattern2_63_32 |=  (uint32_t)  (pFromData)[295] << 24;

  /* Unpack Member: m_uSb1Pattern2_95_64 */
  (pToStruct)->m_uSb1Pattern2_95_64 =  (uint32_t)  (pFromData)[296] ;
  (pToStruct)->m_uSb1Pattern2_95_64 |=  (uint32_t)  (pFromData)[297] << 8;
  (pToStruct)->m_uSb1Pattern2_95_64 |=  (uint32_t)  (pFromData)[298] << 16;
  (pToStruct)->m_uSb1Pattern2_95_64 |=  (uint32_t)  (pFromData)[299] << 24;

  /* Unpack Member: m_uSb1Pattern2_127_96 */
  (pToStruct)->m_uSb1Pattern2_127_96 =  (uint32_t)  (pFromData)[300] ;
  (pToStruct)->m_uSb1Pattern2_127_96 |=  (uint32_t)  (pFromData)[301] << 8;
  (pToStruct)->m_uSb1Pattern2_127_96 |=  (uint32_t)  (pFromData)[302] << 16;
  (pToStruct)->m_uSb1Pattern2_127_96 |=  (uint32_t)  (pFromData)[303] << 24;

  /* Unpack Member: m_uSb1Pattern3_31_0 */
  (pToStruct)->m_uSb1Pattern3_31_0 =  (uint32_t)  (pFromData)[304] ;
  (pToStruct)->m_uSb1Pattern3_31_0 |=  (uint32_t)  (pFromData)[305] << 8;
  (pToStruct)->m_uSb1Pattern3_31_0 |=  (uint32_t)  (pFromData)[306] << 16;
  (pToStruct)->m_uSb1Pattern3_31_0 |=  (uint32_t)  (pFromData)[307] << 24;

  /* Unpack Member: m_uSb1Pattern3_63_32 */
  (pToStruct)->m_uSb1Pattern3_63_32 =  (uint32_t)  (pFromData)[308] ;
  (pToStruct)->m_uSb1Pattern3_63_32 |=  (uint32_t)  (pFromData)[309] << 8;
  (pToStruct)->m_uSb1Pattern3_63_32 |=  (uint32_t)  (pFromData)[310] << 16;
  (pToStruct)->m_uSb1Pattern3_63_32 |=  (uint32_t)  (pFromData)[311] << 24;

  /* Unpack Member: m_uSb1Pattern3_95_64 */
  (pToStruct)->m_uSb1Pattern3_95_64 =  (uint32_t)  (pFromData)[312] ;
  (pToStruct)->m_uSb1Pattern3_95_64 |=  (uint32_t)  (pFromData)[313] << 8;
  (pToStruct)->m_uSb1Pattern3_95_64 |=  (uint32_t)  (pFromData)[314] << 16;
  (pToStruct)->m_uSb1Pattern3_95_64 |=  (uint32_t)  (pFromData)[315] << 24;

  /* Unpack Member: m_uSb1Pattern3_127_96 */
  (pToStruct)->m_uSb1Pattern3_127_96 =  (uint32_t)  (pFromData)[316] ;
  (pToStruct)->m_uSb1Pattern3_127_96 |=  (uint32_t)  (pFromData)[317] << 8;
  (pToStruct)->m_uSb1Pattern3_127_96 |=  (uint32_t)  (pFromData)[318] << 16;
  (pToStruct)->m_uSb1Pattern3_127_96 |=  (uint32_t)  (pFromData)[319] << 24;

  /* Unpack Member: m_uSb1Pattern4_31_0 */
  (pToStruct)->m_uSb1Pattern4_31_0 =  (uint32_t)  (pFromData)[320] ;
  (pToStruct)->m_uSb1Pattern4_31_0 |=  (uint32_t)  (pFromData)[321] << 8;
  (pToStruct)->m_uSb1Pattern4_31_0 |=  (uint32_t)  (pFromData)[322] << 16;
  (pToStruct)->m_uSb1Pattern4_31_0 |=  (uint32_t)  (pFromData)[323] << 24;

  /* Unpack Member: m_uSb1Pattern4_63_32 */
  (pToStruct)->m_uSb1Pattern4_63_32 =  (uint32_t)  (pFromData)[324] ;
  (pToStruct)->m_uSb1Pattern4_63_32 |=  (uint32_t)  (pFromData)[325] << 8;
  (pToStruct)->m_uSb1Pattern4_63_32 |=  (uint32_t)  (pFromData)[326] << 16;
  (pToStruct)->m_uSb1Pattern4_63_32 |=  (uint32_t)  (pFromData)[327] << 24;

  /* Unpack Member: m_uSb1Pattern4_95_64 */
  (pToStruct)->m_uSb1Pattern4_95_64 =  (uint32_t)  (pFromData)[328] ;
  (pToStruct)->m_uSb1Pattern4_95_64 |=  (uint32_t)  (pFromData)[329] << 8;
  (pToStruct)->m_uSb1Pattern4_95_64 |=  (uint32_t)  (pFromData)[330] << 16;
  (pToStruct)->m_uSb1Pattern4_95_64 |=  (uint32_t)  (pFromData)[331] << 24;

  /* Unpack Member: m_uSb1Pattern4_127_96 */
  (pToStruct)->m_uSb1Pattern4_127_96 =  (uint32_t)  (pFromData)[332] ;
  (pToStruct)->m_uSb1Pattern4_127_96 |=  (uint32_t)  (pFromData)[333] << 8;
  (pToStruct)->m_uSb1Pattern4_127_96 |=  (uint32_t)  (pFromData)[334] << 16;
  (pToStruct)->m_uSb1Pattern4_127_96 |=  (uint32_t)  (pFromData)[335] << 24;

  /* Unpack Member: m_uSb1Pattern5_31_0 */
  (pToStruct)->m_uSb1Pattern5_31_0 =  (uint32_t)  (pFromData)[336] ;
  (pToStruct)->m_uSb1Pattern5_31_0 |=  (uint32_t)  (pFromData)[337] << 8;
  (pToStruct)->m_uSb1Pattern5_31_0 |=  (uint32_t)  (pFromData)[338] << 16;
  (pToStruct)->m_uSb1Pattern5_31_0 |=  (uint32_t)  (pFromData)[339] << 24;

  /* Unpack Member: m_uSb1Pattern5_63_32 */
  (pToStruct)->m_uSb1Pattern5_63_32 =  (uint32_t)  (pFromData)[340] ;
  (pToStruct)->m_uSb1Pattern5_63_32 |=  (uint32_t)  (pFromData)[341] << 8;
  (pToStruct)->m_uSb1Pattern5_63_32 |=  (uint32_t)  (pFromData)[342] << 16;
  (pToStruct)->m_uSb1Pattern5_63_32 |=  (uint32_t)  (pFromData)[343] << 24;

  /* Unpack Member: m_uSb1Pattern5_95_64 */
  (pToStruct)->m_uSb1Pattern5_95_64 =  (uint32_t)  (pFromData)[344] ;
  (pToStruct)->m_uSb1Pattern5_95_64 |=  (uint32_t)  (pFromData)[345] << 8;
  (pToStruct)->m_uSb1Pattern5_95_64 |=  (uint32_t)  (pFromData)[346] << 16;
  (pToStruct)->m_uSb1Pattern5_95_64 |=  (uint32_t)  (pFromData)[347] << 24;

  /* Unpack Member: m_uSb1Pattern5_127_96 */
  (pToStruct)->m_uSb1Pattern5_127_96 =  (uint32_t)  (pFromData)[348] ;
  (pToStruct)->m_uSb1Pattern5_127_96 |=  (uint32_t)  (pFromData)[349] << 8;
  (pToStruct)->m_uSb1Pattern5_127_96 |=  (uint32_t)  (pFromData)[350] << 16;
  (pToStruct)->m_uSb1Pattern5_127_96 |=  (uint32_t)  (pFromData)[351] << 24;

  /* Unpack Member: m_uSb1Pattern6_31_0 */
  (pToStruct)->m_uSb1Pattern6_31_0 =  (uint32_t)  (pFromData)[352] ;
  (pToStruct)->m_uSb1Pattern6_31_0 |=  (uint32_t)  (pFromData)[353] << 8;
  (pToStruct)->m_uSb1Pattern6_31_0 |=  (uint32_t)  (pFromData)[354] << 16;
  (pToStruct)->m_uSb1Pattern6_31_0 |=  (uint32_t)  (pFromData)[355] << 24;

  /* Unpack Member: m_uSb1Pattern6_63_32 */
  (pToStruct)->m_uSb1Pattern6_63_32 =  (uint32_t)  (pFromData)[356] ;
  (pToStruct)->m_uSb1Pattern6_63_32 |=  (uint32_t)  (pFromData)[357] << 8;
  (pToStruct)->m_uSb1Pattern6_63_32 |=  (uint32_t)  (pFromData)[358] << 16;
  (pToStruct)->m_uSb1Pattern6_63_32 |=  (uint32_t)  (pFromData)[359] << 24;

  /* Unpack Member: m_uSb1Pattern6_95_64 */
  (pToStruct)->m_uSb1Pattern6_95_64 =  (uint32_t)  (pFromData)[360] ;
  (pToStruct)->m_uSb1Pattern6_95_64 |=  (uint32_t)  (pFromData)[361] << 8;
  (pToStruct)->m_uSb1Pattern6_95_64 |=  (uint32_t)  (pFromData)[362] << 16;
  (pToStruct)->m_uSb1Pattern6_95_64 |=  (uint32_t)  (pFromData)[363] << 24;

  /* Unpack Member: m_uSb1Pattern6_127_96 */
  (pToStruct)->m_uSb1Pattern6_127_96 =  (uint32_t)  (pFromData)[364] ;
  (pToStruct)->m_uSb1Pattern6_127_96 |=  (uint32_t)  (pFromData)[365] << 8;
  (pToStruct)->m_uSb1Pattern6_127_96 |=  (uint32_t)  (pFromData)[366] << 16;
  (pToStruct)->m_uSb1Pattern6_127_96 |=  (uint32_t)  (pFromData)[367] << 24;

  /* Unpack Member: m_uSb1Pattern7_31_0 */
  (pToStruct)->m_uSb1Pattern7_31_0 =  (uint32_t)  (pFromData)[368] ;
  (pToStruct)->m_uSb1Pattern7_31_0 |=  (uint32_t)  (pFromData)[369] << 8;
  (pToStruct)->m_uSb1Pattern7_31_0 |=  (uint32_t)  (pFromData)[370] << 16;
  (pToStruct)->m_uSb1Pattern7_31_0 |=  (uint32_t)  (pFromData)[371] << 24;

  /* Unpack Member: m_uSb1Pattern7_63_32 */
  (pToStruct)->m_uSb1Pattern7_63_32 =  (uint32_t)  (pFromData)[372] ;
  (pToStruct)->m_uSb1Pattern7_63_32 |=  (uint32_t)  (pFromData)[373] << 8;
  (pToStruct)->m_uSb1Pattern7_63_32 |=  (uint32_t)  (pFromData)[374] << 16;
  (pToStruct)->m_uSb1Pattern7_63_32 |=  (uint32_t)  (pFromData)[375] << 24;

  /* Unpack Member: m_uSb1Pattern7_95_64 */
  (pToStruct)->m_uSb1Pattern7_95_64 =  (uint32_t)  (pFromData)[376] ;
  (pToStruct)->m_uSb1Pattern7_95_64 |=  (uint32_t)  (pFromData)[377] << 8;
  (pToStruct)->m_uSb1Pattern7_95_64 |=  (uint32_t)  (pFromData)[378] << 16;
  (pToStruct)->m_uSb1Pattern7_95_64 |=  (uint32_t)  (pFromData)[379] << 24;

  /* Unpack Member: m_uSb1Pattern7_127_96 */
  (pToStruct)->m_uSb1Pattern7_127_96 =  (uint32_t)  (pFromData)[380] ;
  (pToStruct)->m_uSb1Pattern7_127_96 |=  (uint32_t)  (pFromData)[381] << 8;
  (pToStruct)->m_uSb1Pattern7_127_96 |=  (uint32_t)  (pFromData)[382] << 16;
  (pToStruct)->m_uSb1Pattern7_127_96 |=  (uint32_t)  (pFromData)[383] << 24;

  /* Unpack Member: m_uSb2Pattern0_31_0 */
  (pToStruct)->m_uSb2Pattern0_31_0 =  (uint32_t)  (pFromData)[384] ;
  (pToStruct)->m_uSb2Pattern0_31_0 |=  (uint32_t)  (pFromData)[385] << 8;
  (pToStruct)->m_uSb2Pattern0_31_0 |=  (uint32_t)  (pFromData)[386] << 16;
  (pToStruct)->m_uSb2Pattern0_31_0 |=  (uint32_t)  (pFromData)[387] << 24;

  /* Unpack Member: m_uSb2Pattern0_63_32 */
  (pToStruct)->m_uSb2Pattern0_63_32 =  (uint32_t)  (pFromData)[388] ;
  (pToStruct)->m_uSb2Pattern0_63_32 |=  (uint32_t)  (pFromData)[389] << 8;
  (pToStruct)->m_uSb2Pattern0_63_32 |=  (uint32_t)  (pFromData)[390] << 16;
  (pToStruct)->m_uSb2Pattern0_63_32 |=  (uint32_t)  (pFromData)[391] << 24;

  /* Unpack Member: m_uSb2Pattern0_95_64 */
  (pToStruct)->m_uSb2Pattern0_95_64 =  (uint32_t)  (pFromData)[392] ;
  (pToStruct)->m_uSb2Pattern0_95_64 |=  (uint32_t)  (pFromData)[393] << 8;
  (pToStruct)->m_uSb2Pattern0_95_64 |=  (uint32_t)  (pFromData)[394] << 16;
  (pToStruct)->m_uSb2Pattern0_95_64 |=  (uint32_t)  (pFromData)[395] << 24;

  /* Unpack Member: m_uSb2Pattern0_127_96 */
  (pToStruct)->m_uSb2Pattern0_127_96 =  (uint32_t)  (pFromData)[396] ;
  (pToStruct)->m_uSb2Pattern0_127_96 |=  (uint32_t)  (pFromData)[397] << 8;
  (pToStruct)->m_uSb2Pattern0_127_96 |=  (uint32_t)  (pFromData)[398] << 16;
  (pToStruct)->m_uSb2Pattern0_127_96 |=  (uint32_t)  (pFromData)[399] << 24;

  /* Unpack Member: m_uSb2Pattern1_31_0 */
  (pToStruct)->m_uSb2Pattern1_31_0 =  (uint32_t)  (pFromData)[400] ;
  (pToStruct)->m_uSb2Pattern1_31_0 |=  (uint32_t)  (pFromData)[401] << 8;
  (pToStruct)->m_uSb2Pattern1_31_0 |=  (uint32_t)  (pFromData)[402] << 16;
  (pToStruct)->m_uSb2Pattern1_31_0 |=  (uint32_t)  (pFromData)[403] << 24;

  /* Unpack Member: m_uSb2Pattern1_63_32 */
  (pToStruct)->m_uSb2Pattern1_63_32 =  (uint32_t)  (pFromData)[404] ;
  (pToStruct)->m_uSb2Pattern1_63_32 |=  (uint32_t)  (pFromData)[405] << 8;
  (pToStruct)->m_uSb2Pattern1_63_32 |=  (uint32_t)  (pFromData)[406] << 16;
  (pToStruct)->m_uSb2Pattern1_63_32 |=  (uint32_t)  (pFromData)[407] << 24;

  /* Unpack Member: m_uSb2Pattern1_95_64 */
  (pToStruct)->m_uSb2Pattern1_95_64 =  (uint32_t)  (pFromData)[408] ;
  (pToStruct)->m_uSb2Pattern1_95_64 |=  (uint32_t)  (pFromData)[409] << 8;
  (pToStruct)->m_uSb2Pattern1_95_64 |=  (uint32_t)  (pFromData)[410] << 16;
  (pToStruct)->m_uSb2Pattern1_95_64 |=  (uint32_t)  (pFromData)[411] << 24;

  /* Unpack Member: m_uSb2Pattern1_127_96 */
  (pToStruct)->m_uSb2Pattern1_127_96 =  (uint32_t)  (pFromData)[412] ;
  (pToStruct)->m_uSb2Pattern1_127_96 |=  (uint32_t)  (pFromData)[413] << 8;
  (pToStruct)->m_uSb2Pattern1_127_96 |=  (uint32_t)  (pFromData)[414] << 16;
  (pToStruct)->m_uSb2Pattern1_127_96 |=  (uint32_t)  (pFromData)[415] << 24;

  /* Unpack Member: m_uSb2Pattern2_31_0 */
  (pToStruct)->m_uSb2Pattern2_31_0 =  (uint32_t)  (pFromData)[416] ;
  (pToStruct)->m_uSb2Pattern2_31_0 |=  (uint32_t)  (pFromData)[417] << 8;
  (pToStruct)->m_uSb2Pattern2_31_0 |=  (uint32_t)  (pFromData)[418] << 16;
  (pToStruct)->m_uSb2Pattern2_31_0 |=  (uint32_t)  (pFromData)[419] << 24;

  /* Unpack Member: m_uSb2Pattern2_63_32 */
  (pToStruct)->m_uSb2Pattern2_63_32 =  (uint32_t)  (pFromData)[420] ;
  (pToStruct)->m_uSb2Pattern2_63_32 |=  (uint32_t)  (pFromData)[421] << 8;
  (pToStruct)->m_uSb2Pattern2_63_32 |=  (uint32_t)  (pFromData)[422] << 16;
  (pToStruct)->m_uSb2Pattern2_63_32 |=  (uint32_t)  (pFromData)[423] << 24;

  /* Unpack Member: m_uSb2Pattern2_95_64 */
  (pToStruct)->m_uSb2Pattern2_95_64 =  (uint32_t)  (pFromData)[424] ;
  (pToStruct)->m_uSb2Pattern2_95_64 |=  (uint32_t)  (pFromData)[425] << 8;
  (pToStruct)->m_uSb2Pattern2_95_64 |=  (uint32_t)  (pFromData)[426] << 16;
  (pToStruct)->m_uSb2Pattern2_95_64 |=  (uint32_t)  (pFromData)[427] << 24;

  /* Unpack Member: m_uSb2Pattern2_127_96 */
  (pToStruct)->m_uSb2Pattern2_127_96 =  (uint32_t)  (pFromData)[428] ;
  (pToStruct)->m_uSb2Pattern2_127_96 |=  (uint32_t)  (pFromData)[429] << 8;
  (pToStruct)->m_uSb2Pattern2_127_96 |=  (uint32_t)  (pFromData)[430] << 16;
  (pToStruct)->m_uSb2Pattern2_127_96 |=  (uint32_t)  (pFromData)[431] << 24;

  /* Unpack Member: m_uSb2Pattern3_31_0 */
  (pToStruct)->m_uSb2Pattern3_31_0 =  (uint32_t)  (pFromData)[432] ;
  (pToStruct)->m_uSb2Pattern3_31_0 |=  (uint32_t)  (pFromData)[433] << 8;
  (pToStruct)->m_uSb2Pattern3_31_0 |=  (uint32_t)  (pFromData)[434] << 16;
  (pToStruct)->m_uSb2Pattern3_31_0 |=  (uint32_t)  (pFromData)[435] << 24;

  /* Unpack Member: m_uSb2Pattern3_63_32 */
  (pToStruct)->m_uSb2Pattern3_63_32 =  (uint32_t)  (pFromData)[436] ;
  (pToStruct)->m_uSb2Pattern3_63_32 |=  (uint32_t)  (pFromData)[437] << 8;
  (pToStruct)->m_uSb2Pattern3_63_32 |=  (uint32_t)  (pFromData)[438] << 16;
  (pToStruct)->m_uSb2Pattern3_63_32 |=  (uint32_t)  (pFromData)[439] << 24;

  /* Unpack Member: m_uSb2Pattern3_95_64 */
  (pToStruct)->m_uSb2Pattern3_95_64 =  (uint32_t)  (pFromData)[440] ;
  (pToStruct)->m_uSb2Pattern3_95_64 |=  (uint32_t)  (pFromData)[441] << 8;
  (pToStruct)->m_uSb2Pattern3_95_64 |=  (uint32_t)  (pFromData)[442] << 16;
  (pToStruct)->m_uSb2Pattern3_95_64 |=  (uint32_t)  (pFromData)[443] << 24;

  /* Unpack Member: m_uSb2Pattern3_127_96 */
  (pToStruct)->m_uSb2Pattern3_127_96 =  (uint32_t)  (pFromData)[444] ;
  (pToStruct)->m_uSb2Pattern3_127_96 |=  (uint32_t)  (pFromData)[445] << 8;
  (pToStruct)->m_uSb2Pattern3_127_96 |=  (uint32_t)  (pFromData)[446] << 16;
  (pToStruct)->m_uSb2Pattern3_127_96 |=  (uint32_t)  (pFromData)[447] << 24;

  /* Unpack Member: m_uSb2Pattern4_31_0 */
  (pToStruct)->m_uSb2Pattern4_31_0 =  (uint32_t)  (pFromData)[448] ;
  (pToStruct)->m_uSb2Pattern4_31_0 |=  (uint32_t)  (pFromData)[449] << 8;
  (pToStruct)->m_uSb2Pattern4_31_0 |=  (uint32_t)  (pFromData)[450] << 16;
  (pToStruct)->m_uSb2Pattern4_31_0 |=  (uint32_t)  (pFromData)[451] << 24;

  /* Unpack Member: m_uSb2Pattern4_63_32 */
  (pToStruct)->m_uSb2Pattern4_63_32 =  (uint32_t)  (pFromData)[452] ;
  (pToStruct)->m_uSb2Pattern4_63_32 |=  (uint32_t)  (pFromData)[453] << 8;
  (pToStruct)->m_uSb2Pattern4_63_32 |=  (uint32_t)  (pFromData)[454] << 16;
  (pToStruct)->m_uSb2Pattern4_63_32 |=  (uint32_t)  (pFromData)[455] << 24;

  /* Unpack Member: m_uSb2Pattern4_95_64 */
  (pToStruct)->m_uSb2Pattern4_95_64 =  (uint32_t)  (pFromData)[456] ;
  (pToStruct)->m_uSb2Pattern4_95_64 |=  (uint32_t)  (pFromData)[457] << 8;
  (pToStruct)->m_uSb2Pattern4_95_64 |=  (uint32_t)  (pFromData)[458] << 16;
  (pToStruct)->m_uSb2Pattern4_95_64 |=  (uint32_t)  (pFromData)[459] << 24;

  /* Unpack Member: m_uSb2Pattern4_127_96 */
  (pToStruct)->m_uSb2Pattern4_127_96 =  (uint32_t)  (pFromData)[460] ;
  (pToStruct)->m_uSb2Pattern4_127_96 |=  (uint32_t)  (pFromData)[461] << 8;
  (pToStruct)->m_uSb2Pattern4_127_96 |=  (uint32_t)  (pFromData)[462] << 16;
  (pToStruct)->m_uSb2Pattern4_127_96 |=  (uint32_t)  (pFromData)[463] << 24;

  /* Unpack Member: m_uSb2Pattern5_31_0 */
  (pToStruct)->m_uSb2Pattern5_31_0 =  (uint32_t)  (pFromData)[464] ;
  (pToStruct)->m_uSb2Pattern5_31_0 |=  (uint32_t)  (pFromData)[465] << 8;
  (pToStruct)->m_uSb2Pattern5_31_0 |=  (uint32_t)  (pFromData)[466] << 16;
  (pToStruct)->m_uSb2Pattern5_31_0 |=  (uint32_t)  (pFromData)[467] << 24;

  /* Unpack Member: m_uSb2Pattern5_63_32 */
  (pToStruct)->m_uSb2Pattern5_63_32 =  (uint32_t)  (pFromData)[468] ;
  (pToStruct)->m_uSb2Pattern5_63_32 |=  (uint32_t)  (pFromData)[469] << 8;
  (pToStruct)->m_uSb2Pattern5_63_32 |=  (uint32_t)  (pFromData)[470] << 16;
  (pToStruct)->m_uSb2Pattern5_63_32 |=  (uint32_t)  (pFromData)[471] << 24;

  /* Unpack Member: m_uSb2Pattern5_95_64 */
  (pToStruct)->m_uSb2Pattern5_95_64 =  (uint32_t)  (pFromData)[472] ;
  (pToStruct)->m_uSb2Pattern5_95_64 |=  (uint32_t)  (pFromData)[473] << 8;
  (pToStruct)->m_uSb2Pattern5_95_64 |=  (uint32_t)  (pFromData)[474] << 16;
  (pToStruct)->m_uSb2Pattern5_95_64 |=  (uint32_t)  (pFromData)[475] << 24;

  /* Unpack Member: m_uSb2Pattern5_127_96 */
  (pToStruct)->m_uSb2Pattern5_127_96 =  (uint32_t)  (pFromData)[476] ;
  (pToStruct)->m_uSb2Pattern5_127_96 |=  (uint32_t)  (pFromData)[477] << 8;
  (pToStruct)->m_uSb2Pattern5_127_96 |=  (uint32_t)  (pFromData)[478] << 16;
  (pToStruct)->m_uSb2Pattern5_127_96 |=  (uint32_t)  (pFromData)[479] << 24;

  /* Unpack Member: m_uSb2Pattern6_31_0 */
  (pToStruct)->m_uSb2Pattern6_31_0 =  (uint32_t)  (pFromData)[480] ;
  (pToStruct)->m_uSb2Pattern6_31_0 |=  (uint32_t)  (pFromData)[481] << 8;
  (pToStruct)->m_uSb2Pattern6_31_0 |=  (uint32_t)  (pFromData)[482] << 16;
  (pToStruct)->m_uSb2Pattern6_31_0 |=  (uint32_t)  (pFromData)[483] << 24;

  /* Unpack Member: m_uSb2Pattern6_63_32 */
  (pToStruct)->m_uSb2Pattern6_63_32 =  (uint32_t)  (pFromData)[484] ;
  (pToStruct)->m_uSb2Pattern6_63_32 |=  (uint32_t)  (pFromData)[485] << 8;
  (pToStruct)->m_uSb2Pattern6_63_32 |=  (uint32_t)  (pFromData)[486] << 16;
  (pToStruct)->m_uSb2Pattern6_63_32 |=  (uint32_t)  (pFromData)[487] << 24;

  /* Unpack Member: m_uSb2Pattern6_95_64 */
  (pToStruct)->m_uSb2Pattern6_95_64 =  (uint32_t)  (pFromData)[488] ;
  (pToStruct)->m_uSb2Pattern6_95_64 |=  (uint32_t)  (pFromData)[489] << 8;
  (pToStruct)->m_uSb2Pattern6_95_64 |=  (uint32_t)  (pFromData)[490] << 16;
  (pToStruct)->m_uSb2Pattern6_95_64 |=  (uint32_t)  (pFromData)[491] << 24;

  /* Unpack Member: m_uSb2Pattern6_127_96 */
  (pToStruct)->m_uSb2Pattern6_127_96 =  (uint32_t)  (pFromData)[492] ;
  (pToStruct)->m_uSb2Pattern6_127_96 |=  (uint32_t)  (pFromData)[493] << 8;
  (pToStruct)->m_uSb2Pattern6_127_96 |=  (uint32_t)  (pFromData)[494] << 16;
  (pToStruct)->m_uSb2Pattern6_127_96 |=  (uint32_t)  (pFromData)[495] << 24;

  /* Unpack Member: m_uSb2Pattern7_31_0 */
  (pToStruct)->m_uSb2Pattern7_31_0 =  (uint32_t)  (pFromData)[496] ;
  (pToStruct)->m_uSb2Pattern7_31_0 |=  (uint32_t)  (pFromData)[497] << 8;
  (pToStruct)->m_uSb2Pattern7_31_0 |=  (uint32_t)  (pFromData)[498] << 16;
  (pToStruct)->m_uSb2Pattern7_31_0 |=  (uint32_t)  (pFromData)[499] << 24;

  /* Unpack Member: m_uSb2Pattern7_63_32 */
  (pToStruct)->m_uSb2Pattern7_63_32 =  (uint32_t)  (pFromData)[500] ;
  (pToStruct)->m_uSb2Pattern7_63_32 |=  (uint32_t)  (pFromData)[501] << 8;
  (pToStruct)->m_uSb2Pattern7_63_32 |=  (uint32_t)  (pFromData)[502] << 16;
  (pToStruct)->m_uSb2Pattern7_63_32 |=  (uint32_t)  (pFromData)[503] << 24;

  /* Unpack Member: m_uSb2Pattern7_95_64 */
  (pToStruct)->m_uSb2Pattern7_95_64 =  (uint32_t)  (pFromData)[504] ;
  (pToStruct)->m_uSb2Pattern7_95_64 |=  (uint32_t)  (pFromData)[505] << 8;
  (pToStruct)->m_uSb2Pattern7_95_64 |=  (uint32_t)  (pFromData)[506] << 16;
  (pToStruct)->m_uSb2Pattern7_95_64 |=  (uint32_t)  (pFromData)[507] << 24;

  /* Unpack Member: m_uSb2Pattern7_127_96 */
  (pToStruct)->m_uSb2Pattern7_127_96 =  (uint32_t)  (pFromData)[508] ;
  (pToStruct)->m_uSb2Pattern7_127_96 |=  (uint32_t)  (pFromData)[509] << 8;
  (pToStruct)->m_uSb2Pattern7_127_96 |=  (uint32_t)  (pFromData)[510] << 16;
  (pToStruct)->m_uSb2Pattern7_127_96 |=  (uint32_t)  (pFromData)[511] << 24;

  /* Unpack Member: m_uSb3Pattern0_31_0 */
  (pToStruct)->m_uSb3Pattern0_31_0 =  (uint32_t)  (pFromData)[512] ;
  (pToStruct)->m_uSb3Pattern0_31_0 |=  (uint32_t)  (pFromData)[513] << 8;
  (pToStruct)->m_uSb3Pattern0_31_0 |=  (uint32_t)  (pFromData)[514] << 16;
  (pToStruct)->m_uSb3Pattern0_31_0 |=  (uint32_t)  (pFromData)[515] << 24;

  /* Unpack Member: m_uSb3Pattern0_63_32 */
  (pToStruct)->m_uSb3Pattern0_63_32 =  (uint32_t)  (pFromData)[516] ;
  (pToStruct)->m_uSb3Pattern0_63_32 |=  (uint32_t)  (pFromData)[517] << 8;
  (pToStruct)->m_uSb3Pattern0_63_32 |=  (uint32_t)  (pFromData)[518] << 16;
  (pToStruct)->m_uSb3Pattern0_63_32 |=  (uint32_t)  (pFromData)[519] << 24;

  /* Unpack Member: m_uSb3Pattern0_95_64 */
  (pToStruct)->m_uSb3Pattern0_95_64 =  (uint32_t)  (pFromData)[520] ;
  (pToStruct)->m_uSb3Pattern0_95_64 |=  (uint32_t)  (pFromData)[521] << 8;
  (pToStruct)->m_uSb3Pattern0_95_64 |=  (uint32_t)  (pFromData)[522] << 16;
  (pToStruct)->m_uSb3Pattern0_95_64 |=  (uint32_t)  (pFromData)[523] << 24;

  /* Unpack Member: m_uSb3Pattern0_127_96 */
  (pToStruct)->m_uSb3Pattern0_127_96 =  (uint32_t)  (pFromData)[524] ;
  (pToStruct)->m_uSb3Pattern0_127_96 |=  (uint32_t)  (pFromData)[525] << 8;
  (pToStruct)->m_uSb3Pattern0_127_96 |=  (uint32_t)  (pFromData)[526] << 16;
  (pToStruct)->m_uSb3Pattern0_127_96 |=  (uint32_t)  (pFromData)[527] << 24;

  /* Unpack Member: m_uSb3Pattern1_31_0 */
  (pToStruct)->m_uSb3Pattern1_31_0 =  (uint32_t)  (pFromData)[528] ;
  (pToStruct)->m_uSb3Pattern1_31_0 |=  (uint32_t)  (pFromData)[529] << 8;
  (pToStruct)->m_uSb3Pattern1_31_0 |=  (uint32_t)  (pFromData)[530] << 16;
  (pToStruct)->m_uSb3Pattern1_31_0 |=  (uint32_t)  (pFromData)[531] << 24;

  /* Unpack Member: m_uSb3Pattern1_63_32 */
  (pToStruct)->m_uSb3Pattern1_63_32 =  (uint32_t)  (pFromData)[532] ;
  (pToStruct)->m_uSb3Pattern1_63_32 |=  (uint32_t)  (pFromData)[533] << 8;
  (pToStruct)->m_uSb3Pattern1_63_32 |=  (uint32_t)  (pFromData)[534] << 16;
  (pToStruct)->m_uSb3Pattern1_63_32 |=  (uint32_t)  (pFromData)[535] << 24;

  /* Unpack Member: m_uSb3Pattern1_95_64 */
  (pToStruct)->m_uSb3Pattern1_95_64 =  (uint32_t)  (pFromData)[536] ;
  (pToStruct)->m_uSb3Pattern1_95_64 |=  (uint32_t)  (pFromData)[537] << 8;
  (pToStruct)->m_uSb3Pattern1_95_64 |=  (uint32_t)  (pFromData)[538] << 16;
  (pToStruct)->m_uSb3Pattern1_95_64 |=  (uint32_t)  (pFromData)[539] << 24;

  /* Unpack Member: m_uSb3Pattern1_127_96 */
  (pToStruct)->m_uSb3Pattern1_127_96 =  (uint32_t)  (pFromData)[540] ;
  (pToStruct)->m_uSb3Pattern1_127_96 |=  (uint32_t)  (pFromData)[541] << 8;
  (pToStruct)->m_uSb3Pattern1_127_96 |=  (uint32_t)  (pFromData)[542] << 16;
  (pToStruct)->m_uSb3Pattern1_127_96 |=  (uint32_t)  (pFromData)[543] << 24;

  /* Unpack Member: m_uSb3Pattern2_31_0 */
  (pToStruct)->m_uSb3Pattern2_31_0 =  (uint32_t)  (pFromData)[544] ;
  (pToStruct)->m_uSb3Pattern2_31_0 |=  (uint32_t)  (pFromData)[545] << 8;
  (pToStruct)->m_uSb3Pattern2_31_0 |=  (uint32_t)  (pFromData)[546] << 16;
  (pToStruct)->m_uSb3Pattern2_31_0 |=  (uint32_t)  (pFromData)[547] << 24;

  /* Unpack Member: m_uSb3Pattern2_63_32 */
  (pToStruct)->m_uSb3Pattern2_63_32 =  (uint32_t)  (pFromData)[548] ;
  (pToStruct)->m_uSb3Pattern2_63_32 |=  (uint32_t)  (pFromData)[549] << 8;
  (pToStruct)->m_uSb3Pattern2_63_32 |=  (uint32_t)  (pFromData)[550] << 16;
  (pToStruct)->m_uSb3Pattern2_63_32 |=  (uint32_t)  (pFromData)[551] << 24;

  /* Unpack Member: m_uSb3Pattern2_95_64 */
  (pToStruct)->m_uSb3Pattern2_95_64 =  (uint32_t)  (pFromData)[552] ;
  (pToStruct)->m_uSb3Pattern2_95_64 |=  (uint32_t)  (pFromData)[553] << 8;
  (pToStruct)->m_uSb3Pattern2_95_64 |=  (uint32_t)  (pFromData)[554] << 16;
  (pToStruct)->m_uSb3Pattern2_95_64 |=  (uint32_t)  (pFromData)[555] << 24;

  /* Unpack Member: m_uSb3Pattern2_127_96 */
  (pToStruct)->m_uSb3Pattern2_127_96 =  (uint32_t)  (pFromData)[556] ;
  (pToStruct)->m_uSb3Pattern2_127_96 |=  (uint32_t)  (pFromData)[557] << 8;
  (pToStruct)->m_uSb3Pattern2_127_96 |=  (uint32_t)  (pFromData)[558] << 16;
  (pToStruct)->m_uSb3Pattern2_127_96 |=  (uint32_t)  (pFromData)[559] << 24;

  /* Unpack Member: m_uSb3Pattern3_31_0 */
  (pToStruct)->m_uSb3Pattern3_31_0 =  (uint32_t)  (pFromData)[560] ;
  (pToStruct)->m_uSb3Pattern3_31_0 |=  (uint32_t)  (pFromData)[561] << 8;
  (pToStruct)->m_uSb3Pattern3_31_0 |=  (uint32_t)  (pFromData)[562] << 16;
  (pToStruct)->m_uSb3Pattern3_31_0 |=  (uint32_t)  (pFromData)[563] << 24;

  /* Unpack Member: m_uSb3Pattern3_63_32 */
  (pToStruct)->m_uSb3Pattern3_63_32 =  (uint32_t)  (pFromData)[564] ;
  (pToStruct)->m_uSb3Pattern3_63_32 |=  (uint32_t)  (pFromData)[565] << 8;
  (pToStruct)->m_uSb3Pattern3_63_32 |=  (uint32_t)  (pFromData)[566] << 16;
  (pToStruct)->m_uSb3Pattern3_63_32 |=  (uint32_t)  (pFromData)[567] << 24;

  /* Unpack Member: m_uSb3Pattern3_95_64 */
  (pToStruct)->m_uSb3Pattern3_95_64 =  (uint32_t)  (pFromData)[568] ;
  (pToStruct)->m_uSb3Pattern3_95_64 |=  (uint32_t)  (pFromData)[569] << 8;
  (pToStruct)->m_uSb3Pattern3_95_64 |=  (uint32_t)  (pFromData)[570] << 16;
  (pToStruct)->m_uSb3Pattern3_95_64 |=  (uint32_t)  (pFromData)[571] << 24;

  /* Unpack Member: m_uSb3Pattern3_127_96 */
  (pToStruct)->m_uSb3Pattern3_127_96 =  (uint32_t)  (pFromData)[572] ;
  (pToStruct)->m_uSb3Pattern3_127_96 |=  (uint32_t)  (pFromData)[573] << 8;
  (pToStruct)->m_uSb3Pattern3_127_96 |=  (uint32_t)  (pFromData)[574] << 16;
  (pToStruct)->m_uSb3Pattern3_127_96 |=  (uint32_t)  (pFromData)[575] << 24;

  /* Unpack Member: m_uSb3Pattern4_31_0 */
  (pToStruct)->m_uSb3Pattern4_31_0 =  (uint32_t)  (pFromData)[576] ;
  (pToStruct)->m_uSb3Pattern4_31_0 |=  (uint32_t)  (pFromData)[577] << 8;
  (pToStruct)->m_uSb3Pattern4_31_0 |=  (uint32_t)  (pFromData)[578] << 16;
  (pToStruct)->m_uSb3Pattern4_31_0 |=  (uint32_t)  (pFromData)[579] << 24;

  /* Unpack Member: m_uSb3Pattern4_63_32 */
  (pToStruct)->m_uSb3Pattern4_63_32 =  (uint32_t)  (pFromData)[580] ;
  (pToStruct)->m_uSb3Pattern4_63_32 |=  (uint32_t)  (pFromData)[581] << 8;
  (pToStruct)->m_uSb3Pattern4_63_32 |=  (uint32_t)  (pFromData)[582] << 16;
  (pToStruct)->m_uSb3Pattern4_63_32 |=  (uint32_t)  (pFromData)[583] << 24;

  /* Unpack Member: m_uSb3Pattern4_95_64 */
  (pToStruct)->m_uSb3Pattern4_95_64 =  (uint32_t)  (pFromData)[584] ;
  (pToStruct)->m_uSb3Pattern4_95_64 |=  (uint32_t)  (pFromData)[585] << 8;
  (pToStruct)->m_uSb3Pattern4_95_64 |=  (uint32_t)  (pFromData)[586] << 16;
  (pToStruct)->m_uSb3Pattern4_95_64 |=  (uint32_t)  (pFromData)[587] << 24;

  /* Unpack Member: m_uSb3Pattern4_127_96 */
  (pToStruct)->m_uSb3Pattern4_127_96 =  (uint32_t)  (pFromData)[588] ;
  (pToStruct)->m_uSb3Pattern4_127_96 |=  (uint32_t)  (pFromData)[589] << 8;
  (pToStruct)->m_uSb3Pattern4_127_96 |=  (uint32_t)  (pFromData)[590] << 16;
  (pToStruct)->m_uSb3Pattern4_127_96 |=  (uint32_t)  (pFromData)[591] << 24;

  /* Unpack Member: m_uSb3Pattern5_31_0 */
  (pToStruct)->m_uSb3Pattern5_31_0 =  (uint32_t)  (pFromData)[592] ;
  (pToStruct)->m_uSb3Pattern5_31_0 |=  (uint32_t)  (pFromData)[593] << 8;
  (pToStruct)->m_uSb3Pattern5_31_0 |=  (uint32_t)  (pFromData)[594] << 16;
  (pToStruct)->m_uSb3Pattern5_31_0 |=  (uint32_t)  (pFromData)[595] << 24;

  /* Unpack Member: m_uSb3Pattern5_63_32 */
  (pToStruct)->m_uSb3Pattern5_63_32 =  (uint32_t)  (pFromData)[596] ;
  (pToStruct)->m_uSb3Pattern5_63_32 |=  (uint32_t)  (pFromData)[597] << 8;
  (pToStruct)->m_uSb3Pattern5_63_32 |=  (uint32_t)  (pFromData)[598] << 16;
  (pToStruct)->m_uSb3Pattern5_63_32 |=  (uint32_t)  (pFromData)[599] << 24;

  /* Unpack Member: m_uSb3Pattern5_95_64 */
  (pToStruct)->m_uSb3Pattern5_95_64 =  (uint32_t)  (pFromData)[600] ;
  (pToStruct)->m_uSb3Pattern5_95_64 |=  (uint32_t)  (pFromData)[601] << 8;
  (pToStruct)->m_uSb3Pattern5_95_64 |=  (uint32_t)  (pFromData)[602] << 16;
  (pToStruct)->m_uSb3Pattern5_95_64 |=  (uint32_t)  (pFromData)[603] << 24;

  /* Unpack Member: m_uSb3Pattern5_127_96 */
  (pToStruct)->m_uSb3Pattern5_127_96 =  (uint32_t)  (pFromData)[604] ;
  (pToStruct)->m_uSb3Pattern5_127_96 |=  (uint32_t)  (pFromData)[605] << 8;
  (pToStruct)->m_uSb3Pattern5_127_96 |=  (uint32_t)  (pFromData)[606] << 16;
  (pToStruct)->m_uSb3Pattern5_127_96 |=  (uint32_t)  (pFromData)[607] << 24;

  /* Unpack Member: m_uSb3Pattern6_31_0 */
  (pToStruct)->m_uSb3Pattern6_31_0 =  (uint32_t)  (pFromData)[608] ;
  (pToStruct)->m_uSb3Pattern6_31_0 |=  (uint32_t)  (pFromData)[609] << 8;
  (pToStruct)->m_uSb3Pattern6_31_0 |=  (uint32_t)  (pFromData)[610] << 16;
  (pToStruct)->m_uSb3Pattern6_31_0 |=  (uint32_t)  (pFromData)[611] << 24;

  /* Unpack Member: m_uSb3Pattern6_63_32 */
  (pToStruct)->m_uSb3Pattern6_63_32 =  (uint32_t)  (pFromData)[612] ;
  (pToStruct)->m_uSb3Pattern6_63_32 |=  (uint32_t)  (pFromData)[613] << 8;
  (pToStruct)->m_uSb3Pattern6_63_32 |=  (uint32_t)  (pFromData)[614] << 16;
  (pToStruct)->m_uSb3Pattern6_63_32 |=  (uint32_t)  (pFromData)[615] << 24;

  /* Unpack Member: m_uSb3Pattern6_95_64 */
  (pToStruct)->m_uSb3Pattern6_95_64 =  (uint32_t)  (pFromData)[616] ;
  (pToStruct)->m_uSb3Pattern6_95_64 |=  (uint32_t)  (pFromData)[617] << 8;
  (pToStruct)->m_uSb3Pattern6_95_64 |=  (uint32_t)  (pFromData)[618] << 16;
  (pToStruct)->m_uSb3Pattern6_95_64 |=  (uint32_t)  (pFromData)[619] << 24;

  /* Unpack Member: m_uSb3Pattern6_127_96 */
  (pToStruct)->m_uSb3Pattern6_127_96 =  (uint32_t)  (pFromData)[620] ;
  (pToStruct)->m_uSb3Pattern6_127_96 |=  (uint32_t)  (pFromData)[621] << 8;
  (pToStruct)->m_uSb3Pattern6_127_96 |=  (uint32_t)  (pFromData)[622] << 16;
  (pToStruct)->m_uSb3Pattern6_127_96 |=  (uint32_t)  (pFromData)[623] << 24;

  /* Unpack Member: m_uSb3Pattern7_31_0 */
  (pToStruct)->m_uSb3Pattern7_31_0 =  (uint32_t)  (pFromData)[624] ;
  (pToStruct)->m_uSb3Pattern7_31_0 |=  (uint32_t)  (pFromData)[625] << 8;
  (pToStruct)->m_uSb3Pattern7_31_0 |=  (uint32_t)  (pFromData)[626] << 16;
  (pToStruct)->m_uSb3Pattern7_31_0 |=  (uint32_t)  (pFromData)[627] << 24;

  /* Unpack Member: m_uSb3Pattern7_63_32 */
  (pToStruct)->m_uSb3Pattern7_63_32 =  (uint32_t)  (pFromData)[628] ;
  (pToStruct)->m_uSb3Pattern7_63_32 |=  (uint32_t)  (pFromData)[629] << 8;
  (pToStruct)->m_uSb3Pattern7_63_32 |=  (uint32_t)  (pFromData)[630] << 16;
  (pToStruct)->m_uSb3Pattern7_63_32 |=  (uint32_t)  (pFromData)[631] << 24;

  /* Unpack Member: m_uSb3Pattern7_95_64 */
  (pToStruct)->m_uSb3Pattern7_95_64 =  (uint32_t)  (pFromData)[632] ;
  (pToStruct)->m_uSb3Pattern7_95_64 |=  (uint32_t)  (pFromData)[633] << 8;
  (pToStruct)->m_uSb3Pattern7_95_64 |=  (uint32_t)  (pFromData)[634] << 16;
  (pToStruct)->m_uSb3Pattern7_95_64 |=  (uint32_t)  (pFromData)[635] << 24;

  /* Unpack Member: m_uSb3Pattern7_127_96 */
  (pToStruct)->m_uSb3Pattern7_127_96 =  (uint32_t)  (pFromData)[636] ;
  (pToStruct)->m_uSb3Pattern7_127_96 |=  (uint32_t)  (pFromData)[637] << 8;
  (pToStruct)->m_uSb3Pattern7_127_96 |=  (uint32_t)  (pFromData)[638] << 16;
  (pToStruct)->m_uSb3Pattern7_127_96 |=  (uint32_t)  (pFromData)[639] << 24;

  /* Unpack Member: m_uSb4Pattern0_31_0 */
  (pToStruct)->m_uSb4Pattern0_31_0 =  (uint32_t)  (pFromData)[640] ;
  (pToStruct)->m_uSb4Pattern0_31_0 |=  (uint32_t)  (pFromData)[641] << 8;
  (pToStruct)->m_uSb4Pattern0_31_0 |=  (uint32_t)  (pFromData)[642] << 16;
  (pToStruct)->m_uSb4Pattern0_31_0 |=  (uint32_t)  (pFromData)[643] << 24;

  /* Unpack Member: m_uSb4Pattern0_63_32 */
  (pToStruct)->m_uSb4Pattern0_63_32 =  (uint32_t)  (pFromData)[644] ;
  (pToStruct)->m_uSb4Pattern0_63_32 |=  (uint32_t)  (pFromData)[645] << 8;
  (pToStruct)->m_uSb4Pattern0_63_32 |=  (uint32_t)  (pFromData)[646] << 16;
  (pToStruct)->m_uSb4Pattern0_63_32 |=  (uint32_t)  (pFromData)[647] << 24;

  /* Unpack Member: m_uSb4Pattern0_95_64 */
  (pToStruct)->m_uSb4Pattern0_95_64 =  (uint32_t)  (pFromData)[648] ;
  (pToStruct)->m_uSb4Pattern0_95_64 |=  (uint32_t)  (pFromData)[649] << 8;
  (pToStruct)->m_uSb4Pattern0_95_64 |=  (uint32_t)  (pFromData)[650] << 16;
  (pToStruct)->m_uSb4Pattern0_95_64 |=  (uint32_t)  (pFromData)[651] << 24;

  /* Unpack Member: m_uSb4Pattern0_127_96 */
  (pToStruct)->m_uSb4Pattern0_127_96 =  (uint32_t)  (pFromData)[652] ;
  (pToStruct)->m_uSb4Pattern0_127_96 |=  (uint32_t)  (pFromData)[653] << 8;
  (pToStruct)->m_uSb4Pattern0_127_96 |=  (uint32_t)  (pFromData)[654] << 16;
  (pToStruct)->m_uSb4Pattern0_127_96 |=  (uint32_t)  (pFromData)[655] << 24;

  /* Unpack Member: m_uSb4Pattern1_31_0 */
  (pToStruct)->m_uSb4Pattern1_31_0 =  (uint32_t)  (pFromData)[656] ;
  (pToStruct)->m_uSb4Pattern1_31_0 |=  (uint32_t)  (pFromData)[657] << 8;
  (pToStruct)->m_uSb4Pattern1_31_0 |=  (uint32_t)  (pFromData)[658] << 16;
  (pToStruct)->m_uSb4Pattern1_31_0 |=  (uint32_t)  (pFromData)[659] << 24;

  /* Unpack Member: m_uSb4Pattern1_63_32 */
  (pToStruct)->m_uSb4Pattern1_63_32 =  (uint32_t)  (pFromData)[660] ;
  (pToStruct)->m_uSb4Pattern1_63_32 |=  (uint32_t)  (pFromData)[661] << 8;
  (pToStruct)->m_uSb4Pattern1_63_32 |=  (uint32_t)  (pFromData)[662] << 16;
  (pToStruct)->m_uSb4Pattern1_63_32 |=  (uint32_t)  (pFromData)[663] << 24;

  /* Unpack Member: m_uSb4Pattern1_95_64 */
  (pToStruct)->m_uSb4Pattern1_95_64 =  (uint32_t)  (pFromData)[664] ;
  (pToStruct)->m_uSb4Pattern1_95_64 |=  (uint32_t)  (pFromData)[665] << 8;
  (pToStruct)->m_uSb4Pattern1_95_64 |=  (uint32_t)  (pFromData)[666] << 16;
  (pToStruct)->m_uSb4Pattern1_95_64 |=  (uint32_t)  (pFromData)[667] << 24;

  /* Unpack Member: m_uSb4Pattern1_127_96 */
  (pToStruct)->m_uSb4Pattern1_127_96 =  (uint32_t)  (pFromData)[668] ;
  (pToStruct)->m_uSb4Pattern1_127_96 |=  (uint32_t)  (pFromData)[669] << 8;
  (pToStruct)->m_uSb4Pattern1_127_96 |=  (uint32_t)  (pFromData)[670] << 16;
  (pToStruct)->m_uSb4Pattern1_127_96 |=  (uint32_t)  (pFromData)[671] << 24;

  /* Unpack Member: m_uSb4Pattern2_31_0 */
  (pToStruct)->m_uSb4Pattern2_31_0 =  (uint32_t)  (pFromData)[672] ;
  (pToStruct)->m_uSb4Pattern2_31_0 |=  (uint32_t)  (pFromData)[673] << 8;
  (pToStruct)->m_uSb4Pattern2_31_0 |=  (uint32_t)  (pFromData)[674] << 16;
  (pToStruct)->m_uSb4Pattern2_31_0 |=  (uint32_t)  (pFromData)[675] << 24;

  /* Unpack Member: m_uSb4Pattern2_63_32 */
  (pToStruct)->m_uSb4Pattern2_63_32 =  (uint32_t)  (pFromData)[676] ;
  (pToStruct)->m_uSb4Pattern2_63_32 |=  (uint32_t)  (pFromData)[677] << 8;
  (pToStruct)->m_uSb4Pattern2_63_32 |=  (uint32_t)  (pFromData)[678] << 16;
  (pToStruct)->m_uSb4Pattern2_63_32 |=  (uint32_t)  (pFromData)[679] << 24;

  /* Unpack Member: m_uSb4Pattern2_95_64 */
  (pToStruct)->m_uSb4Pattern2_95_64 =  (uint32_t)  (pFromData)[680] ;
  (pToStruct)->m_uSb4Pattern2_95_64 |=  (uint32_t)  (pFromData)[681] << 8;
  (pToStruct)->m_uSb4Pattern2_95_64 |=  (uint32_t)  (pFromData)[682] << 16;
  (pToStruct)->m_uSb4Pattern2_95_64 |=  (uint32_t)  (pFromData)[683] << 24;

  /* Unpack Member: m_uSb4Pattern2_127_96 */
  (pToStruct)->m_uSb4Pattern2_127_96 =  (uint32_t)  (pFromData)[684] ;
  (pToStruct)->m_uSb4Pattern2_127_96 |=  (uint32_t)  (pFromData)[685] << 8;
  (pToStruct)->m_uSb4Pattern2_127_96 |=  (uint32_t)  (pFromData)[686] << 16;
  (pToStruct)->m_uSb4Pattern2_127_96 |=  (uint32_t)  (pFromData)[687] << 24;

  /* Unpack Member: m_uSb4Pattern3_31_0 */
  (pToStruct)->m_uSb4Pattern3_31_0 =  (uint32_t)  (pFromData)[688] ;
  (pToStruct)->m_uSb4Pattern3_31_0 |=  (uint32_t)  (pFromData)[689] << 8;
  (pToStruct)->m_uSb4Pattern3_31_0 |=  (uint32_t)  (pFromData)[690] << 16;
  (pToStruct)->m_uSb4Pattern3_31_0 |=  (uint32_t)  (pFromData)[691] << 24;

  /* Unpack Member: m_uSb4Pattern3_63_32 */
  (pToStruct)->m_uSb4Pattern3_63_32 =  (uint32_t)  (pFromData)[692] ;
  (pToStruct)->m_uSb4Pattern3_63_32 |=  (uint32_t)  (pFromData)[693] << 8;
  (pToStruct)->m_uSb4Pattern3_63_32 |=  (uint32_t)  (pFromData)[694] << 16;
  (pToStruct)->m_uSb4Pattern3_63_32 |=  (uint32_t)  (pFromData)[695] << 24;

  /* Unpack Member: m_uSb4Pattern3_95_64 */
  (pToStruct)->m_uSb4Pattern3_95_64 =  (uint32_t)  (pFromData)[696] ;
  (pToStruct)->m_uSb4Pattern3_95_64 |=  (uint32_t)  (pFromData)[697] << 8;
  (pToStruct)->m_uSb4Pattern3_95_64 |=  (uint32_t)  (pFromData)[698] << 16;
  (pToStruct)->m_uSb4Pattern3_95_64 |=  (uint32_t)  (pFromData)[699] << 24;

  /* Unpack Member: m_uSb4Pattern3_127_96 */
  (pToStruct)->m_uSb4Pattern3_127_96 =  (uint32_t)  (pFromData)[700] ;
  (pToStruct)->m_uSb4Pattern3_127_96 |=  (uint32_t)  (pFromData)[701] << 8;
  (pToStruct)->m_uSb4Pattern3_127_96 |=  (uint32_t)  (pFromData)[702] << 16;
  (pToStruct)->m_uSb4Pattern3_127_96 |=  (uint32_t)  (pFromData)[703] << 24;

  /* Unpack Member: m_uSb4Pattern4_31_0 */
  (pToStruct)->m_uSb4Pattern4_31_0 =  (uint32_t)  (pFromData)[704] ;
  (pToStruct)->m_uSb4Pattern4_31_0 |=  (uint32_t)  (pFromData)[705] << 8;
  (pToStruct)->m_uSb4Pattern4_31_0 |=  (uint32_t)  (pFromData)[706] << 16;
  (pToStruct)->m_uSb4Pattern4_31_0 |=  (uint32_t)  (pFromData)[707] << 24;

  /* Unpack Member: m_uSb4Pattern4_63_32 */
  (pToStruct)->m_uSb4Pattern4_63_32 =  (uint32_t)  (pFromData)[708] ;
  (pToStruct)->m_uSb4Pattern4_63_32 |=  (uint32_t)  (pFromData)[709] << 8;
  (pToStruct)->m_uSb4Pattern4_63_32 |=  (uint32_t)  (pFromData)[710] << 16;
  (pToStruct)->m_uSb4Pattern4_63_32 |=  (uint32_t)  (pFromData)[711] << 24;

  /* Unpack Member: m_uSb4Pattern4_95_64 */
  (pToStruct)->m_uSb4Pattern4_95_64 =  (uint32_t)  (pFromData)[712] ;
  (pToStruct)->m_uSb4Pattern4_95_64 |=  (uint32_t)  (pFromData)[713] << 8;
  (pToStruct)->m_uSb4Pattern4_95_64 |=  (uint32_t)  (pFromData)[714] << 16;
  (pToStruct)->m_uSb4Pattern4_95_64 |=  (uint32_t)  (pFromData)[715] << 24;

  /* Unpack Member: m_uSb4Pattern4_127_96 */
  (pToStruct)->m_uSb4Pattern4_127_96 =  (uint32_t)  (pFromData)[716] ;
  (pToStruct)->m_uSb4Pattern4_127_96 |=  (uint32_t)  (pFromData)[717] << 8;
  (pToStruct)->m_uSb4Pattern4_127_96 |=  (uint32_t)  (pFromData)[718] << 16;
  (pToStruct)->m_uSb4Pattern4_127_96 |=  (uint32_t)  (pFromData)[719] << 24;

  /* Unpack Member: m_uSb4Pattern5_31_0 */
  (pToStruct)->m_uSb4Pattern5_31_0 =  (uint32_t)  (pFromData)[720] ;
  (pToStruct)->m_uSb4Pattern5_31_0 |=  (uint32_t)  (pFromData)[721] << 8;
  (pToStruct)->m_uSb4Pattern5_31_0 |=  (uint32_t)  (pFromData)[722] << 16;
  (pToStruct)->m_uSb4Pattern5_31_0 |=  (uint32_t)  (pFromData)[723] << 24;

  /* Unpack Member: m_uSb4Pattern5_63_32 */
  (pToStruct)->m_uSb4Pattern5_63_32 =  (uint32_t)  (pFromData)[724] ;
  (pToStruct)->m_uSb4Pattern5_63_32 |=  (uint32_t)  (pFromData)[725] << 8;
  (pToStruct)->m_uSb4Pattern5_63_32 |=  (uint32_t)  (pFromData)[726] << 16;
  (pToStruct)->m_uSb4Pattern5_63_32 |=  (uint32_t)  (pFromData)[727] << 24;

  /* Unpack Member: m_uSb4Pattern5_95_64 */
  (pToStruct)->m_uSb4Pattern5_95_64 =  (uint32_t)  (pFromData)[728] ;
  (pToStruct)->m_uSb4Pattern5_95_64 |=  (uint32_t)  (pFromData)[729] << 8;
  (pToStruct)->m_uSb4Pattern5_95_64 |=  (uint32_t)  (pFromData)[730] << 16;
  (pToStruct)->m_uSb4Pattern5_95_64 |=  (uint32_t)  (pFromData)[731] << 24;

  /* Unpack Member: m_uSb4Pattern5_127_96 */
  (pToStruct)->m_uSb4Pattern5_127_96 =  (uint32_t)  (pFromData)[732] ;
  (pToStruct)->m_uSb4Pattern5_127_96 |=  (uint32_t)  (pFromData)[733] << 8;
  (pToStruct)->m_uSb4Pattern5_127_96 |=  (uint32_t)  (pFromData)[734] << 16;
  (pToStruct)->m_uSb4Pattern5_127_96 |=  (uint32_t)  (pFromData)[735] << 24;

  /* Unpack Member: m_uSb4Pattern6_31_0 */
  (pToStruct)->m_uSb4Pattern6_31_0 =  (uint32_t)  (pFromData)[736] ;
  (pToStruct)->m_uSb4Pattern6_31_0 |=  (uint32_t)  (pFromData)[737] << 8;
  (pToStruct)->m_uSb4Pattern6_31_0 |=  (uint32_t)  (pFromData)[738] << 16;
  (pToStruct)->m_uSb4Pattern6_31_0 |=  (uint32_t)  (pFromData)[739] << 24;

  /* Unpack Member: m_uSb4Pattern6_63_32 */
  (pToStruct)->m_uSb4Pattern6_63_32 =  (uint32_t)  (pFromData)[740] ;
  (pToStruct)->m_uSb4Pattern6_63_32 |=  (uint32_t)  (pFromData)[741] << 8;
  (pToStruct)->m_uSb4Pattern6_63_32 |=  (uint32_t)  (pFromData)[742] << 16;
  (pToStruct)->m_uSb4Pattern6_63_32 |=  (uint32_t)  (pFromData)[743] << 24;

  /* Unpack Member: m_uSb4Pattern6_95_64 */
  (pToStruct)->m_uSb4Pattern6_95_64 =  (uint32_t)  (pFromData)[744] ;
  (pToStruct)->m_uSb4Pattern6_95_64 |=  (uint32_t)  (pFromData)[745] << 8;
  (pToStruct)->m_uSb4Pattern6_95_64 |=  (uint32_t)  (pFromData)[746] << 16;
  (pToStruct)->m_uSb4Pattern6_95_64 |=  (uint32_t)  (pFromData)[747] << 24;

  /* Unpack Member: m_uSb4Pattern6_127_96 */
  (pToStruct)->m_uSb4Pattern6_127_96 =  (uint32_t)  (pFromData)[748] ;
  (pToStruct)->m_uSb4Pattern6_127_96 |=  (uint32_t)  (pFromData)[749] << 8;
  (pToStruct)->m_uSb4Pattern6_127_96 |=  (uint32_t)  (pFromData)[750] << 16;
  (pToStruct)->m_uSb4Pattern6_127_96 |=  (uint32_t)  (pFromData)[751] << 24;

  /* Unpack Member: m_uSb4Pattern7_31_0 */
  (pToStruct)->m_uSb4Pattern7_31_0 =  (uint32_t)  (pFromData)[752] ;
  (pToStruct)->m_uSb4Pattern7_31_0 |=  (uint32_t)  (pFromData)[753] << 8;
  (pToStruct)->m_uSb4Pattern7_31_0 |=  (uint32_t)  (pFromData)[754] << 16;
  (pToStruct)->m_uSb4Pattern7_31_0 |=  (uint32_t)  (pFromData)[755] << 24;

  /* Unpack Member: m_uSb4Pattern7_63_32 */
  (pToStruct)->m_uSb4Pattern7_63_32 =  (uint32_t)  (pFromData)[756] ;
  (pToStruct)->m_uSb4Pattern7_63_32 |=  (uint32_t)  (pFromData)[757] << 8;
  (pToStruct)->m_uSb4Pattern7_63_32 |=  (uint32_t)  (pFromData)[758] << 16;
  (pToStruct)->m_uSb4Pattern7_63_32 |=  (uint32_t)  (pFromData)[759] << 24;

  /* Unpack Member: m_uSb4Pattern7_95_64 */
  (pToStruct)->m_uSb4Pattern7_95_64 =  (uint32_t)  (pFromData)[760] ;
  (pToStruct)->m_uSb4Pattern7_95_64 |=  (uint32_t)  (pFromData)[761] << 8;
  (pToStruct)->m_uSb4Pattern7_95_64 |=  (uint32_t)  (pFromData)[762] << 16;
  (pToStruct)->m_uSb4Pattern7_95_64 |=  (uint32_t)  (pFromData)[763] << 24;

  /* Unpack Member: m_uSb4Pattern7_127_96 */
  (pToStruct)->m_uSb4Pattern7_127_96 =  (uint32_t)  (pFromData)[764] ;
  (pToStruct)->m_uSb4Pattern7_127_96 |=  (uint32_t)  (pFromData)[765] << 8;
  (pToStruct)->m_uSb4Pattern7_127_96 |=  (uint32_t)  (pFromData)[766] << 16;
  (pToStruct)->m_uSb4Pattern7_127_96 |=  (uint32_t)  (pFromData)[767] << 24;

  /* Unpack Member: m_uSb5Pattern0_31_0 */
  (pToStruct)->m_uSb5Pattern0_31_0 =  (uint32_t)  (pFromData)[768] ;
  (pToStruct)->m_uSb5Pattern0_31_0 |=  (uint32_t)  (pFromData)[769] << 8;
  (pToStruct)->m_uSb5Pattern0_31_0 |=  (uint32_t)  (pFromData)[770] << 16;
  (pToStruct)->m_uSb5Pattern0_31_0 |=  (uint32_t)  (pFromData)[771] << 24;

  /* Unpack Member: m_uSb5Pattern0_63_32 */
  (pToStruct)->m_uSb5Pattern0_63_32 =  (uint32_t)  (pFromData)[772] ;
  (pToStruct)->m_uSb5Pattern0_63_32 |=  (uint32_t)  (pFromData)[773] << 8;
  (pToStruct)->m_uSb5Pattern0_63_32 |=  (uint32_t)  (pFromData)[774] << 16;
  (pToStruct)->m_uSb5Pattern0_63_32 |=  (uint32_t)  (pFromData)[775] << 24;

  /* Unpack Member: m_uSb5Pattern0_95_64 */
  (pToStruct)->m_uSb5Pattern0_95_64 =  (uint32_t)  (pFromData)[776] ;
  (pToStruct)->m_uSb5Pattern0_95_64 |=  (uint32_t)  (pFromData)[777] << 8;
  (pToStruct)->m_uSb5Pattern0_95_64 |=  (uint32_t)  (pFromData)[778] << 16;
  (pToStruct)->m_uSb5Pattern0_95_64 |=  (uint32_t)  (pFromData)[779] << 24;

  /* Unpack Member: m_uSb5Pattern0_127_96 */
  (pToStruct)->m_uSb5Pattern0_127_96 =  (uint32_t)  (pFromData)[780] ;
  (pToStruct)->m_uSb5Pattern0_127_96 |=  (uint32_t)  (pFromData)[781] << 8;
  (pToStruct)->m_uSb5Pattern0_127_96 |=  (uint32_t)  (pFromData)[782] << 16;
  (pToStruct)->m_uSb5Pattern0_127_96 |=  (uint32_t)  (pFromData)[783] << 24;

  /* Unpack Member: m_uSb5Pattern1_31_0 */
  (pToStruct)->m_uSb5Pattern1_31_0 =  (uint32_t)  (pFromData)[784] ;
  (pToStruct)->m_uSb5Pattern1_31_0 |=  (uint32_t)  (pFromData)[785] << 8;
  (pToStruct)->m_uSb5Pattern1_31_0 |=  (uint32_t)  (pFromData)[786] << 16;
  (pToStruct)->m_uSb5Pattern1_31_0 |=  (uint32_t)  (pFromData)[787] << 24;

  /* Unpack Member: m_uSb5Pattern1_63_32 */
  (pToStruct)->m_uSb5Pattern1_63_32 =  (uint32_t)  (pFromData)[788] ;
  (pToStruct)->m_uSb5Pattern1_63_32 |=  (uint32_t)  (pFromData)[789] << 8;
  (pToStruct)->m_uSb5Pattern1_63_32 |=  (uint32_t)  (pFromData)[790] << 16;
  (pToStruct)->m_uSb5Pattern1_63_32 |=  (uint32_t)  (pFromData)[791] << 24;

  /* Unpack Member: m_uSb5Pattern1_95_64 */
  (pToStruct)->m_uSb5Pattern1_95_64 =  (uint32_t)  (pFromData)[792] ;
  (pToStruct)->m_uSb5Pattern1_95_64 |=  (uint32_t)  (pFromData)[793] << 8;
  (pToStruct)->m_uSb5Pattern1_95_64 |=  (uint32_t)  (pFromData)[794] << 16;
  (pToStruct)->m_uSb5Pattern1_95_64 |=  (uint32_t)  (pFromData)[795] << 24;

  /* Unpack Member: m_uSb5Pattern1_127_96 */
  (pToStruct)->m_uSb5Pattern1_127_96 =  (uint32_t)  (pFromData)[796] ;
  (pToStruct)->m_uSb5Pattern1_127_96 |=  (uint32_t)  (pFromData)[797] << 8;
  (pToStruct)->m_uSb5Pattern1_127_96 |=  (uint32_t)  (pFromData)[798] << 16;
  (pToStruct)->m_uSb5Pattern1_127_96 |=  (uint32_t)  (pFromData)[799] << 24;

  /* Unpack Member: m_uSb5Pattern2_31_0 */
  (pToStruct)->m_uSb5Pattern2_31_0 =  (uint32_t)  (pFromData)[800] ;
  (pToStruct)->m_uSb5Pattern2_31_0 |=  (uint32_t)  (pFromData)[801] << 8;
  (pToStruct)->m_uSb5Pattern2_31_0 |=  (uint32_t)  (pFromData)[802] << 16;
  (pToStruct)->m_uSb5Pattern2_31_0 |=  (uint32_t)  (pFromData)[803] << 24;

  /* Unpack Member: m_uSb5Pattern2_63_32 */
  (pToStruct)->m_uSb5Pattern2_63_32 =  (uint32_t)  (pFromData)[804] ;
  (pToStruct)->m_uSb5Pattern2_63_32 |=  (uint32_t)  (pFromData)[805] << 8;
  (pToStruct)->m_uSb5Pattern2_63_32 |=  (uint32_t)  (pFromData)[806] << 16;
  (pToStruct)->m_uSb5Pattern2_63_32 |=  (uint32_t)  (pFromData)[807] << 24;

  /* Unpack Member: m_uSb5Pattern2_95_64 */
  (pToStruct)->m_uSb5Pattern2_95_64 =  (uint32_t)  (pFromData)[808] ;
  (pToStruct)->m_uSb5Pattern2_95_64 |=  (uint32_t)  (pFromData)[809] << 8;
  (pToStruct)->m_uSb5Pattern2_95_64 |=  (uint32_t)  (pFromData)[810] << 16;
  (pToStruct)->m_uSb5Pattern2_95_64 |=  (uint32_t)  (pFromData)[811] << 24;

  /* Unpack Member: m_uSb5Pattern2_127_96 */
  (pToStruct)->m_uSb5Pattern2_127_96 =  (uint32_t)  (pFromData)[812] ;
  (pToStruct)->m_uSb5Pattern2_127_96 |=  (uint32_t)  (pFromData)[813] << 8;
  (pToStruct)->m_uSb5Pattern2_127_96 |=  (uint32_t)  (pFromData)[814] << 16;
  (pToStruct)->m_uSb5Pattern2_127_96 |=  (uint32_t)  (pFromData)[815] << 24;

  /* Unpack Member: m_uSb5Pattern3_31_0 */
  (pToStruct)->m_uSb5Pattern3_31_0 =  (uint32_t)  (pFromData)[816] ;
  (pToStruct)->m_uSb5Pattern3_31_0 |=  (uint32_t)  (pFromData)[817] << 8;
  (pToStruct)->m_uSb5Pattern3_31_0 |=  (uint32_t)  (pFromData)[818] << 16;
  (pToStruct)->m_uSb5Pattern3_31_0 |=  (uint32_t)  (pFromData)[819] << 24;

  /* Unpack Member: m_uSb5Pattern3_63_32 */
  (pToStruct)->m_uSb5Pattern3_63_32 =  (uint32_t)  (pFromData)[820] ;
  (pToStruct)->m_uSb5Pattern3_63_32 |=  (uint32_t)  (pFromData)[821] << 8;
  (pToStruct)->m_uSb5Pattern3_63_32 |=  (uint32_t)  (pFromData)[822] << 16;
  (pToStruct)->m_uSb5Pattern3_63_32 |=  (uint32_t)  (pFromData)[823] << 24;

  /* Unpack Member: m_uSb5Pattern3_95_64 */
  (pToStruct)->m_uSb5Pattern3_95_64 =  (uint32_t)  (pFromData)[824] ;
  (pToStruct)->m_uSb5Pattern3_95_64 |=  (uint32_t)  (pFromData)[825] << 8;
  (pToStruct)->m_uSb5Pattern3_95_64 |=  (uint32_t)  (pFromData)[826] << 16;
  (pToStruct)->m_uSb5Pattern3_95_64 |=  (uint32_t)  (pFromData)[827] << 24;

  /* Unpack Member: m_uSb5Pattern3_127_96 */
  (pToStruct)->m_uSb5Pattern3_127_96 =  (uint32_t)  (pFromData)[828] ;
  (pToStruct)->m_uSb5Pattern3_127_96 |=  (uint32_t)  (pFromData)[829] << 8;
  (pToStruct)->m_uSb5Pattern3_127_96 |=  (uint32_t)  (pFromData)[830] << 16;
  (pToStruct)->m_uSb5Pattern3_127_96 |=  (uint32_t)  (pFromData)[831] << 24;

  /* Unpack Member: m_uSb5Pattern4_31_0 */
  (pToStruct)->m_uSb5Pattern4_31_0 =  (uint32_t)  (pFromData)[832] ;
  (pToStruct)->m_uSb5Pattern4_31_0 |=  (uint32_t)  (pFromData)[833] << 8;
  (pToStruct)->m_uSb5Pattern4_31_0 |=  (uint32_t)  (pFromData)[834] << 16;
  (pToStruct)->m_uSb5Pattern4_31_0 |=  (uint32_t)  (pFromData)[835] << 24;

  /* Unpack Member: m_uSb5Pattern4_63_32 */
  (pToStruct)->m_uSb5Pattern4_63_32 =  (uint32_t)  (pFromData)[836] ;
  (pToStruct)->m_uSb5Pattern4_63_32 |=  (uint32_t)  (pFromData)[837] << 8;
  (pToStruct)->m_uSb5Pattern4_63_32 |=  (uint32_t)  (pFromData)[838] << 16;
  (pToStruct)->m_uSb5Pattern4_63_32 |=  (uint32_t)  (pFromData)[839] << 24;

  /* Unpack Member: m_uSb5Pattern4_95_64 */
  (pToStruct)->m_uSb5Pattern4_95_64 =  (uint32_t)  (pFromData)[840] ;
  (pToStruct)->m_uSb5Pattern4_95_64 |=  (uint32_t)  (pFromData)[841] << 8;
  (pToStruct)->m_uSb5Pattern4_95_64 |=  (uint32_t)  (pFromData)[842] << 16;
  (pToStruct)->m_uSb5Pattern4_95_64 |=  (uint32_t)  (pFromData)[843] << 24;

  /* Unpack Member: m_uSb5Pattern4_127_96 */
  (pToStruct)->m_uSb5Pattern4_127_96 =  (uint32_t)  (pFromData)[844] ;
  (pToStruct)->m_uSb5Pattern4_127_96 |=  (uint32_t)  (pFromData)[845] << 8;
  (pToStruct)->m_uSb5Pattern4_127_96 |=  (uint32_t)  (pFromData)[846] << 16;
  (pToStruct)->m_uSb5Pattern4_127_96 |=  (uint32_t)  (pFromData)[847] << 24;

  /* Unpack Member: m_uSb5Pattern5_31_0 */
  (pToStruct)->m_uSb5Pattern5_31_0 =  (uint32_t)  (pFromData)[848] ;
  (pToStruct)->m_uSb5Pattern5_31_0 |=  (uint32_t)  (pFromData)[849] << 8;
  (pToStruct)->m_uSb5Pattern5_31_0 |=  (uint32_t)  (pFromData)[850] << 16;
  (pToStruct)->m_uSb5Pattern5_31_0 |=  (uint32_t)  (pFromData)[851] << 24;

  /* Unpack Member: m_uSb5Pattern5_63_32 */
  (pToStruct)->m_uSb5Pattern5_63_32 =  (uint32_t)  (pFromData)[852] ;
  (pToStruct)->m_uSb5Pattern5_63_32 |=  (uint32_t)  (pFromData)[853] << 8;
  (pToStruct)->m_uSb5Pattern5_63_32 |=  (uint32_t)  (pFromData)[854] << 16;
  (pToStruct)->m_uSb5Pattern5_63_32 |=  (uint32_t)  (pFromData)[855] << 24;

  /* Unpack Member: m_uSb5Pattern5_95_64 */
  (pToStruct)->m_uSb5Pattern5_95_64 =  (uint32_t)  (pFromData)[856] ;
  (pToStruct)->m_uSb5Pattern5_95_64 |=  (uint32_t)  (pFromData)[857] << 8;
  (pToStruct)->m_uSb5Pattern5_95_64 |=  (uint32_t)  (pFromData)[858] << 16;
  (pToStruct)->m_uSb5Pattern5_95_64 |=  (uint32_t)  (pFromData)[859] << 24;

  /* Unpack Member: m_uSb5Pattern5_127_96 */
  (pToStruct)->m_uSb5Pattern5_127_96 =  (uint32_t)  (pFromData)[860] ;
  (pToStruct)->m_uSb5Pattern5_127_96 |=  (uint32_t)  (pFromData)[861] << 8;
  (pToStruct)->m_uSb5Pattern5_127_96 |=  (uint32_t)  (pFromData)[862] << 16;
  (pToStruct)->m_uSb5Pattern5_127_96 |=  (uint32_t)  (pFromData)[863] << 24;

  /* Unpack Member: m_uSb5Pattern6_31_0 */
  (pToStruct)->m_uSb5Pattern6_31_0 =  (uint32_t)  (pFromData)[864] ;
  (pToStruct)->m_uSb5Pattern6_31_0 |=  (uint32_t)  (pFromData)[865] << 8;
  (pToStruct)->m_uSb5Pattern6_31_0 |=  (uint32_t)  (pFromData)[866] << 16;
  (pToStruct)->m_uSb5Pattern6_31_0 |=  (uint32_t)  (pFromData)[867] << 24;

  /* Unpack Member: m_uSb5Pattern6_63_32 */
  (pToStruct)->m_uSb5Pattern6_63_32 =  (uint32_t)  (pFromData)[868] ;
  (pToStruct)->m_uSb5Pattern6_63_32 |=  (uint32_t)  (pFromData)[869] << 8;
  (pToStruct)->m_uSb5Pattern6_63_32 |=  (uint32_t)  (pFromData)[870] << 16;
  (pToStruct)->m_uSb5Pattern6_63_32 |=  (uint32_t)  (pFromData)[871] << 24;

  /* Unpack Member: m_uSb5Pattern6_95_64 */
  (pToStruct)->m_uSb5Pattern6_95_64 =  (uint32_t)  (pFromData)[872] ;
  (pToStruct)->m_uSb5Pattern6_95_64 |=  (uint32_t)  (pFromData)[873] << 8;
  (pToStruct)->m_uSb5Pattern6_95_64 |=  (uint32_t)  (pFromData)[874] << 16;
  (pToStruct)->m_uSb5Pattern6_95_64 |=  (uint32_t)  (pFromData)[875] << 24;

  /* Unpack Member: m_uSb5Pattern6_127_96 */
  (pToStruct)->m_uSb5Pattern6_127_96 =  (uint32_t)  (pFromData)[876] ;
  (pToStruct)->m_uSb5Pattern6_127_96 |=  (uint32_t)  (pFromData)[877] << 8;
  (pToStruct)->m_uSb5Pattern6_127_96 |=  (uint32_t)  (pFromData)[878] << 16;
  (pToStruct)->m_uSb5Pattern6_127_96 |=  (uint32_t)  (pFromData)[879] << 24;

  /* Unpack Member: m_uSb5Pattern7_31_0 */
  (pToStruct)->m_uSb5Pattern7_31_0 =  (uint32_t)  (pFromData)[880] ;
  (pToStruct)->m_uSb5Pattern7_31_0 |=  (uint32_t)  (pFromData)[881] << 8;
  (pToStruct)->m_uSb5Pattern7_31_0 |=  (uint32_t)  (pFromData)[882] << 16;
  (pToStruct)->m_uSb5Pattern7_31_0 |=  (uint32_t)  (pFromData)[883] << 24;

  /* Unpack Member: m_uSb5Pattern7_63_32 */
  (pToStruct)->m_uSb5Pattern7_63_32 =  (uint32_t)  (pFromData)[884] ;
  (pToStruct)->m_uSb5Pattern7_63_32 |=  (uint32_t)  (pFromData)[885] << 8;
  (pToStruct)->m_uSb5Pattern7_63_32 |=  (uint32_t)  (pFromData)[886] << 16;
  (pToStruct)->m_uSb5Pattern7_63_32 |=  (uint32_t)  (pFromData)[887] << 24;

  /* Unpack Member: m_uSb5Pattern7_95_64 */
  (pToStruct)->m_uSb5Pattern7_95_64 =  (uint32_t)  (pFromData)[888] ;
  (pToStruct)->m_uSb5Pattern7_95_64 |=  (uint32_t)  (pFromData)[889] << 8;
  (pToStruct)->m_uSb5Pattern7_95_64 |=  (uint32_t)  (pFromData)[890] << 16;
  (pToStruct)->m_uSb5Pattern7_95_64 |=  (uint32_t)  (pFromData)[891] << 24;

  /* Unpack Member: m_uSb5Pattern7_127_96 */
  (pToStruct)->m_uSb5Pattern7_127_96 =  (uint32_t)  (pFromData)[892] ;
  (pToStruct)->m_uSb5Pattern7_127_96 |=  (uint32_t)  (pFromData)[893] << 8;
  (pToStruct)->m_uSb5Pattern7_127_96 |=  (uint32_t)  (pFromData)[894] << 16;
  (pToStruct)->m_uSb5Pattern7_127_96 |=  (uint32_t)  (pFromData)[895] << 24;

  /* Unpack Member: m_uResv24 */
  (pToStruct)->m_uResv24 =  (uint32_t)  (pFromData)[896] ;
  (pToStruct)->m_uResv24 |=  (uint32_t)  (pFromData)[897] << 8;
  (pToStruct)->m_uResv24 |=  (uint32_t)  (pFromData)[898] << 16;
  (pToStruct)->m_uResv24 |=  (uint32_t)  (pFromData)[899] << 24;

  /* Unpack Member: m_uResv25 */
  (pToStruct)->m_uResv25 =  (uint32_t)  (pFromData)[900] ;
  (pToStruct)->m_uResv25 |=  (uint32_t)  (pFromData)[901] << 8;
  (pToStruct)->m_uResv25 |=  (uint32_t)  (pFromData)[902] << 16;
  (pToStruct)->m_uResv25 |=  (uint32_t)  (pFromData)[903] << 24;

  /* Unpack Member: m_uResv26 */
  (pToStruct)->m_uResv26 =  (uint32_t)  (pFromData)[904] ;
  (pToStruct)->m_uResv26 |=  (uint32_t)  (pFromData)[905] << 8;
  (pToStruct)->m_uResv26 |=  (uint32_t)  (pFromData)[906] << 16;
  (pToStruct)->m_uResv26 |=  (uint32_t)  (pFromData)[907] << 24;

  /* Unpack Member: m_uResv27 */
  (pToStruct)->m_uResv27 =  (uint32_t)  (pFromData)[908] ;
  (pToStruct)->m_uResv27 |=  (uint32_t)  (pFromData)[909] << 8;
  (pToStruct)->m_uResv27 |=  (uint32_t)  (pFromData)[910] << 16;
  (pToStruct)->m_uResv27 |=  (uint32_t)  (pFromData)[911] << 24;

  /* Unpack Member: m_uResv28 */
  (pToStruct)->m_uResv28 =  (uint32_t)  (pFromData)[912] ;
  (pToStruct)->m_uResv28 |=  (uint32_t)  (pFromData)[913] << 8;
  (pToStruct)->m_uResv28 |=  (uint32_t)  (pFromData)[914] << 16;
  (pToStruct)->m_uResv28 |=  (uint32_t)  (pFromData)[915] << 24;

  /* Unpack Member: m_uResv29 */
  (pToStruct)->m_uResv29 =  (uint32_t)  (pFromData)[916] ;
  (pToStruct)->m_uResv29 |=  (uint32_t)  (pFromData)[917] << 8;
  (pToStruct)->m_uResv29 |=  (uint32_t)  (pFromData)[918] << 16;
  (pToStruct)->m_uResv29 |=  (uint32_t)  (pFromData)[919] << 24;

  /* Unpack Member: m_uResv30 */
  (pToStruct)->m_uResv30 =  (uint32_t)  (pFromData)[920] ;
  (pToStruct)->m_uResv30 |=  (uint32_t)  (pFromData)[921] << 8;
  (pToStruct)->m_uResv30 |=  (uint32_t)  (pFromData)[922] << 16;
  (pToStruct)->m_uResv30 |=  (uint32_t)  (pFromData)[923] << 24;

  /* Unpack Member: m_uResv31 */
  (pToStruct)->m_uResv31 =  (uint32_t)  (pFromData)[924] ;
  (pToStruct)->m_uResv31 |=  (uint32_t)  (pFromData)[925] << 8;
  (pToStruct)->m_uResv31 |=  (uint32_t)  (pFromData)[926] << 16;
  (pToStruct)->m_uResv31 |=  (uint32_t)  (pFromData)[927] << 24;

  /* Unpack Member: m_uResv32 */
  (pToStruct)->m_uResv32 =  (uint32_t)  (pFromData)[928] ;
  (pToStruct)->m_uResv32 |=  (uint32_t)  (pFromData)[929] << 8;
  (pToStruct)->m_uResv32 |=  (uint32_t)  (pFromData)[930] << 16;
  (pToStruct)->m_uResv32 |=  (uint32_t)  (pFromData)[931] << 24;

  /* Unpack Member: m_uResv33 */
  (pToStruct)->m_uResv33 =  (uint32_t)  (pFromData)[932] ;
  (pToStruct)->m_uResv33 |=  (uint32_t)  (pFromData)[933] << 8;
  (pToStruct)->m_uResv33 |=  (uint32_t)  (pFromData)[934] << 16;
  (pToStruct)->m_uResv33 |=  (uint32_t)  (pFromData)[935] << 24;

  /* Unpack Member: m_uResv34 */
  (pToStruct)->m_uResv34 =  (uint32_t)  (pFromData)[936] ;
  (pToStruct)->m_uResv34 |=  (uint32_t)  (pFromData)[937] << 8;
  (pToStruct)->m_uResv34 |=  (uint32_t)  (pFromData)[938] << 16;
  (pToStruct)->m_uResv34 |=  (uint32_t)  (pFromData)[939] << 24;

  /* Unpack Member: m_uResv35 */
  (pToStruct)->m_uResv35 =  (uint32_t)  (pFromData)[940] ;
  (pToStruct)->m_uResv35 |=  (uint32_t)  (pFromData)[941] << 8;
  (pToStruct)->m_uResv35 |=  (uint32_t)  (pFromData)[942] << 16;
  (pToStruct)->m_uResv35 |=  (uint32_t)  (pFromData)[943] << 24;

  /* Unpack Member: m_uResv36 */
  (pToStruct)->m_uResv36 =  (uint32_t)  (pFromData)[944] ;
  (pToStruct)->m_uResv36 |=  (uint32_t)  (pFromData)[945] << 8;
  (pToStruct)->m_uResv36 |=  (uint32_t)  (pFromData)[946] << 16;
  (pToStruct)->m_uResv36 |=  (uint32_t)  (pFromData)[947] << 24;

  /* Unpack Member: m_uResv37 */
  (pToStruct)->m_uResv37 =  (uint32_t)  (pFromData)[948] ;
  (pToStruct)->m_uResv37 |=  (uint32_t)  (pFromData)[949] << 8;
  (pToStruct)->m_uResv37 |=  (uint32_t)  (pFromData)[950] << 16;
  (pToStruct)->m_uResv37 |=  (uint32_t)  (pFromData)[951] << 24;

  /* Unpack Member: m_uResv38 */
  (pToStruct)->m_uResv38 =  (uint32_t)  (pFromData)[952] ;
  (pToStruct)->m_uResv38 |=  (uint32_t)  (pFromData)[953] << 8;
  (pToStruct)->m_uResv38 |=  (uint32_t)  (pFromData)[954] << 16;
  (pToStruct)->m_uResv38 |=  (uint32_t)  (pFromData)[955] << 24;

  /* Unpack Member: m_uResv39 */
  (pToStruct)->m_uResv39 =  (uint32_t)  (pFromData)[956] ;
  (pToStruct)->m_uResv39 |=  (uint32_t)  (pFromData)[957] << 8;
  (pToStruct)->m_uResv39 |=  (uint32_t)  (pFromData)[958] << 16;
  (pToStruct)->m_uResv39 |=  (uint32_t)  (pFromData)[959] << 24;

  /* Unpack Member: m_uResv40 */
  (pToStruct)->m_uResv40 =  (uint32_t)  (pFromData)[960] ;
  (pToStruct)->m_uResv40 |=  (uint32_t)  (pFromData)[961] << 8;
  (pToStruct)->m_uResv40 |=  (uint32_t)  (pFromData)[962] << 16;
  (pToStruct)->m_uResv40 |=  (uint32_t)  (pFromData)[963] << 24;

  /* Unpack Member: m_uResv41 */
  (pToStruct)->m_uResv41 =  (uint32_t)  (pFromData)[964] ;
  (pToStruct)->m_uResv41 |=  (uint32_t)  (pFromData)[965] << 8;
  (pToStruct)->m_uResv41 |=  (uint32_t)  (pFromData)[966] << 16;
  (pToStruct)->m_uResv41 |=  (uint32_t)  (pFromData)[967] << 24;

  /* Unpack Member: m_uResv42 */
  (pToStruct)->m_uResv42 =  (uint32_t)  (pFromData)[968] ;
  (pToStruct)->m_uResv42 |=  (uint32_t)  (pFromData)[969] << 8;
  (pToStruct)->m_uResv42 |=  (uint32_t)  (pFromData)[970] << 16;
  (pToStruct)->m_uResv42 |=  (uint32_t)  (pFromData)[971] << 24;

  /* Unpack Member: m_uResv43 */
  (pToStruct)->m_uResv43 =  (uint32_t)  (pFromData)[972] ;
  (pToStruct)->m_uResv43 |=  (uint32_t)  (pFromData)[973] << 8;
  (pToStruct)->m_uResv43 |=  (uint32_t)  (pFromData)[974] << 16;
  (pToStruct)->m_uResv43 |=  (uint32_t)  (pFromData)[975] << 24;

  /* Unpack Member: m_uResv44 */
  (pToStruct)->m_uResv44 =  (uint32_t)  (pFromData)[976] ;
  (pToStruct)->m_uResv44 |=  (uint32_t)  (pFromData)[977] << 8;
  (pToStruct)->m_uResv44 |=  (uint32_t)  (pFromData)[978] << 16;
  (pToStruct)->m_uResv44 |=  (uint32_t)  (pFromData)[979] << 24;

  /* Unpack Member: m_uResv45 */
  (pToStruct)->m_uResv45 =  (uint32_t)  (pFromData)[980] ;
  (pToStruct)->m_uResv45 |=  (uint32_t)  (pFromData)[981] << 8;
  (pToStruct)->m_uResv45 |=  (uint32_t)  (pFromData)[982] << 16;
  (pToStruct)->m_uResv45 |=  (uint32_t)  (pFromData)[983] << 24;

  /* Unpack Member: m_uResv46 */
  (pToStruct)->m_uResv46 =  (uint32_t)  (pFromData)[984] ;
  (pToStruct)->m_uResv46 |=  (uint32_t)  (pFromData)[985] << 8;
  (pToStruct)->m_uResv46 |=  (uint32_t)  (pFromData)[986] << 16;
  (pToStruct)->m_uResv46 |=  (uint32_t)  (pFromData)[987] << 24;

  /* Unpack Member: m_uResv47 */
  (pToStruct)->m_uResv47 =  (uint32_t)  (pFromData)[988] ;
  (pToStruct)->m_uResv47 |=  (uint32_t)  (pFromData)[989] << 8;
  (pToStruct)->m_uResv47 |=  (uint32_t)  (pFromData)[990] << 16;
  (pToStruct)->m_uResv47 |=  (uint32_t)  (pFromData)[991] << 24;

  /* Unpack Member: m_uResv48 */
  (pToStruct)->m_uResv48 =  (uint32_t)  (pFromData)[992] ;
  (pToStruct)->m_uResv48 |=  (uint32_t)  (pFromData)[993] << 8;
  (pToStruct)->m_uResv48 |=  (uint32_t)  (pFromData)[994] << 16;
  (pToStruct)->m_uResv48 |=  (uint32_t)  (pFromData)[995] << 24;

  /* Unpack Member: m_uResv49 */
  (pToStruct)->m_uResv49 =  (uint32_t)  (pFromData)[996] ;
  (pToStruct)->m_uResv49 |=  (uint32_t)  (pFromData)[997] << 8;
  (pToStruct)->m_uResv49 |=  (uint32_t)  (pFromData)[998] << 16;
  (pToStruct)->m_uResv49 |=  (uint32_t)  (pFromData)[999] << 24;

  /* Unpack Member: m_uResv50 */
  (pToStruct)->m_uResv50 =  (uint32_t)  (pFromData)[1000] ;
  (pToStruct)->m_uResv50 |=  (uint32_t)  (pFromData)[1001] << 8;
  (pToStruct)->m_uResv50 |=  (uint32_t)  (pFromData)[1002] << 16;
  (pToStruct)->m_uResv50 |=  (uint32_t)  (pFromData)[1003] << 24;

  /* Unpack Member: m_uResv51 */
  (pToStruct)->m_uResv51 =  (uint32_t)  (pFromData)[1004] ;
  (pToStruct)->m_uResv51 |=  (uint32_t)  (pFromData)[1005] << 8;
  (pToStruct)->m_uResv51 |=  (uint32_t)  (pFromData)[1006] << 16;
  (pToStruct)->m_uResv51 |=  (uint32_t)  (pFromData)[1007] << 24;

  /* Unpack Member: m_uResv52 */
  (pToStruct)->m_uResv52 =  (uint32_t)  (pFromData)[1008] ;
  (pToStruct)->m_uResv52 |=  (uint32_t)  (pFromData)[1009] << 8;
  (pToStruct)->m_uResv52 |=  (uint32_t)  (pFromData)[1010] << 16;
  (pToStruct)->m_uResv52 |=  (uint32_t)  (pFromData)[1011] << 24;

  /* Unpack Member: m_uResv53 */
  (pToStruct)->m_uResv53 =  (uint32_t)  (pFromData)[1012] ;
  (pToStruct)->m_uResv53 |=  (uint32_t)  (pFromData)[1013] << 8;
  (pToStruct)->m_uResv53 |=  (uint32_t)  (pFromData)[1014] << 16;
  (pToStruct)->m_uResv53 |=  (uint32_t)  (pFromData)[1015] << 24;

  /* Unpack Member: m_uResv54 */
  (pToStruct)->m_uResv54 =  (uint32_t)  (pFromData)[1016] ;
  (pToStruct)->m_uResv54 |=  (uint32_t)  (pFromData)[1017] << 8;
  (pToStruct)->m_uResv54 |=  (uint32_t)  (pFromData)[1018] << 16;
  (pToStruct)->m_uResv54 |=  (uint32_t)  (pFromData)[1019] << 24;

  /* Unpack Member: m_uResv55 */
  (pToStruct)->m_uResv55 =  (uint32_t)  (pFromData)[1020] ;
  (pToStruct)->m_uResv55 |=  (uint32_t)  (pFromData)[1021] << 8;
  (pToStruct)->m_uResv55 |=  (uint32_t)  (pFromData)[1022] << 16;
  (pToStruct)->m_uResv55 |=  (uint32_t)  (pFromData)[1023] << 24;

}



/* initialize an instance of this zframe */
void
sbZfC2RcDmaFormat_InitInstance(sbZfC2RcDmaFormat_t *pFrame) {

  pFrame->m_uInstruction0 =  (unsigned int)  0;
  pFrame->m_uInstruction1 =  (unsigned int)  0;
  pFrame->m_uInstruction2 =  (unsigned int)  0;
  pFrame->m_uInstruction3 =  (unsigned int)  0;
  pFrame->m_uInstruction4 =  (unsigned int)  0;
  pFrame->m_uInstruction5 =  (unsigned int)  0;
  pFrame->m_uInstruction6 =  (unsigned int)  0;
  pFrame->m_uInstruction7 =  (unsigned int)  0;
  pFrame->m_uResv0 =  (unsigned int)  0;
  pFrame->m_uResv1 =  (unsigned int)  0;
  pFrame->m_uResv2 =  (unsigned int)  0;
  pFrame->m_uResv3 =  (unsigned int)  0;
  pFrame->m_uResv4 =  (unsigned int)  0;
  pFrame->m_uResv5 =  (unsigned int)  0;
  pFrame->m_uResv6 =  (unsigned int)  0;
  pFrame->m_uResv7 =  (unsigned int)  0;
  pFrame->m_uResv8 =  (unsigned int)  0;
  pFrame->m_uResv9 =  (unsigned int)  0;
  pFrame->m_uResv10 =  (unsigned int)  0;
  pFrame->m_uResv11 =  (unsigned int)  0;
  pFrame->m_uResv12 =  (unsigned int)  0;
  pFrame->m_uResv13 =  (unsigned int)  0;
  pFrame->m_uResv14 =  (unsigned int)  0;
  pFrame->m_uResv15 =  (unsigned int)  0;
  pFrame->m_uResv16 =  (unsigned int)  0;
  pFrame->m_uResv17 =  (unsigned int)  0;
  pFrame->m_uResv18 =  (unsigned int)  0;
  pFrame->m_uResv19 =  (unsigned int)  0;
  pFrame->m_uResv20 =  (unsigned int)  0;
  pFrame->m_uResv21 =  (unsigned int)  0;
  pFrame->m_uResv22 =  (unsigned int)  0;
  pFrame->m_uResv23 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern0_31_0 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern0_63_32 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern0_95_64 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern0_127_96 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern1_31_0 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern1_63_32 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern1_95_64 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern1_127_96 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern2_31_0 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern2_63_32 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern2_95_64 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern2_127_96 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern3_31_0 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern3_63_32 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern3_95_64 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern3_127_96 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern4_31_0 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern4_63_32 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern4_95_64 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern4_127_96 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern5_31_0 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern5_63_32 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern5_95_64 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern5_127_96 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern6_31_0 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern6_63_32 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern6_95_64 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern6_127_96 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern7_31_0 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern7_63_32 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern7_95_64 =  (unsigned int)  0;
  pFrame->m_uSb0Pattern7_127_96 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern0_31_0 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern0_63_32 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern0_95_64 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern0_127_96 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern1_31_0 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern1_63_32 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern1_95_64 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern1_127_96 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern2_31_0 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern2_63_32 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern2_95_64 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern2_127_96 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern3_31_0 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern3_63_32 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern3_95_64 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern3_127_96 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern4_31_0 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern4_63_32 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern4_95_64 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern4_127_96 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern5_31_0 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern5_63_32 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern5_95_64 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern5_127_96 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern6_31_0 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern6_63_32 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern6_95_64 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern6_127_96 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern7_31_0 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern7_63_32 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern7_95_64 =  (unsigned int)  0;
  pFrame->m_uSb1Pattern7_127_96 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern0_31_0 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern0_63_32 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern0_95_64 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern0_127_96 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern1_31_0 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern1_63_32 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern1_95_64 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern1_127_96 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern2_31_0 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern2_63_32 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern2_95_64 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern2_127_96 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern3_31_0 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern3_63_32 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern3_95_64 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern3_127_96 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern4_31_0 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern4_63_32 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern4_95_64 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern4_127_96 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern5_31_0 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern5_63_32 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern5_95_64 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern5_127_96 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern6_31_0 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern6_63_32 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern6_95_64 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern6_127_96 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern7_31_0 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern7_63_32 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern7_95_64 =  (unsigned int)  0;
  pFrame->m_uSb2Pattern7_127_96 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern0_31_0 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern0_63_32 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern0_95_64 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern0_127_96 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern1_31_0 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern1_63_32 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern1_95_64 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern1_127_96 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern2_31_0 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern2_63_32 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern2_95_64 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern2_127_96 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern3_31_0 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern3_63_32 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern3_95_64 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern3_127_96 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern4_31_0 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern4_63_32 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern4_95_64 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern4_127_96 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern5_31_0 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern5_63_32 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern5_95_64 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern5_127_96 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern6_31_0 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern6_63_32 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern6_95_64 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern6_127_96 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern7_31_0 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern7_63_32 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern7_95_64 =  (unsigned int)  0;
  pFrame->m_uSb3Pattern7_127_96 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern0_31_0 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern0_63_32 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern0_95_64 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern0_127_96 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern1_31_0 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern1_63_32 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern1_95_64 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern1_127_96 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern2_31_0 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern2_63_32 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern2_95_64 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern2_127_96 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern3_31_0 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern3_63_32 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern3_95_64 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern3_127_96 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern4_31_0 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern4_63_32 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern4_95_64 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern4_127_96 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern5_31_0 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern5_63_32 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern5_95_64 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern5_127_96 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern6_31_0 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern6_63_32 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern6_95_64 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern6_127_96 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern7_31_0 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern7_63_32 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern7_95_64 =  (unsigned int)  0;
  pFrame->m_uSb4Pattern7_127_96 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern0_31_0 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern0_63_32 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern0_95_64 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern0_127_96 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern1_31_0 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern1_63_32 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern1_95_64 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern1_127_96 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern2_31_0 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern2_63_32 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern2_95_64 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern2_127_96 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern3_31_0 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern3_63_32 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern3_95_64 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern3_127_96 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern4_31_0 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern4_63_32 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern4_95_64 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern4_127_96 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern5_31_0 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern5_63_32 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern5_95_64 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern5_127_96 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern6_31_0 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern6_63_32 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern6_95_64 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern6_127_96 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern7_31_0 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern7_63_32 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern7_95_64 =  (unsigned int)  0;
  pFrame->m_uSb5Pattern7_127_96 =  (unsigned int)  0;
  pFrame->m_uResv24 =  (unsigned int)  0;
  pFrame->m_uResv25 =  (unsigned int)  0;
  pFrame->m_uResv26 =  (unsigned int)  0;
  pFrame->m_uResv27 =  (unsigned int)  0;
  pFrame->m_uResv28 =  (unsigned int)  0;
  pFrame->m_uResv29 =  (unsigned int)  0;
  pFrame->m_uResv30 =  (unsigned int)  0;
  pFrame->m_uResv31 =  (unsigned int)  0;
  pFrame->m_uResv32 =  (unsigned int)  0;
  pFrame->m_uResv33 =  (unsigned int)  0;
  pFrame->m_uResv34 =  (unsigned int)  0;
  pFrame->m_uResv35 =  (unsigned int)  0;
  pFrame->m_uResv36 =  (unsigned int)  0;
  pFrame->m_uResv37 =  (unsigned int)  0;
  pFrame->m_uResv38 =  (unsigned int)  0;
  pFrame->m_uResv39 =  (unsigned int)  0;
  pFrame->m_uResv40 =  (unsigned int)  0;
  pFrame->m_uResv41 =  (unsigned int)  0;
  pFrame->m_uResv42 =  (unsigned int)  0;
  pFrame->m_uResv43 =  (unsigned int)  0;
  pFrame->m_uResv44 =  (unsigned int)  0;
  pFrame->m_uResv45 =  (unsigned int)  0;
  pFrame->m_uResv46 =  (unsigned int)  0;
  pFrame->m_uResv47 =  (unsigned int)  0;
  pFrame->m_uResv48 =  (unsigned int)  0;
  pFrame->m_uResv49 =  (unsigned int)  0;
  pFrame->m_uResv50 =  (unsigned int)  0;
  pFrame->m_uResv51 =  (unsigned int)  0;
  pFrame->m_uResv52 =  (unsigned int)  0;
  pFrame->m_uResv53 =  (unsigned int)  0;
  pFrame->m_uResv54 =  (unsigned int)  0;
  pFrame->m_uResv55 =  (unsigned int)  0;

}



void sbZfC2RcDmaFormat_SetPatternWord(uint32_t *pToData, UINT uBlock, UINT uInstruction, UINT uPatternWord, UINT nFromData)
{
  /* pattern data offset + superblock offset + instruction offset + pattern word offset */
  uint32_t uIndex = (32 + 
                   (uBlock * 32) +
                   (uInstruction * 4) +
                   (uPatternWord));

  (pToData)[uIndex] = ((nFromData)) ;
    
}
void sbZfC2RcDmaFormat_GetPatternWord(uint32_t *pToData, UINT uBlock, UINT uInstruction, UINT uPatternWord, UINT *nFromData)
{
  /* pattern data offset + superblock offset + instruction offset + pattern word offset */
  uint32_t uIndex = (32 + 
                   (uBlock * 32) +
                   (uInstruction * 4) +
                   (uPatternWord));

  *(pToData) = ((nFromData)[uIndex]) ;
    
}
