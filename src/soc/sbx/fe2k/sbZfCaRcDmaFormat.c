/*
 * $Id: sbZfCaRcDmaFormat.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaRcDmaFormat.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaRcDmaFormat_Pack(sbZfCaRcDmaFormat_t *pFrom,
                       uint8_t *pToData,
                       uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CARCDMAFORMAT_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uInstruction0 */
  (pToData)[252] |= ((pFrom)->m_uInstruction0) & 0xFF;
  (pToData)[253] |= ((pFrom)->m_uInstruction0 >> 8) &0xFF;
  (pToData)[254] |= ((pFrom)->m_uInstruction0 >> 16) &0xFF;
  (pToData)[255] |= ((pFrom)->m_uInstruction0 >> 24) &0xFF;

  /* Pack Member: m_uInstruction1 */
  (pToData)[248] |= ((pFrom)->m_uInstruction1) & 0xFF;
  (pToData)[249] |= ((pFrom)->m_uInstruction1 >> 8) &0xFF;
  (pToData)[250] |= ((pFrom)->m_uInstruction1 >> 16) &0xFF;
  (pToData)[251] |= ((pFrom)->m_uInstruction1 >> 24) &0xFF;

  /* Pack Member: m_uInstruction2 */
  (pToData)[244] |= ((pFrom)->m_uInstruction2) & 0xFF;
  (pToData)[245] |= ((pFrom)->m_uInstruction2 >> 8) &0xFF;
  (pToData)[246] |= ((pFrom)->m_uInstruction2 >> 16) &0xFF;
  (pToData)[247] |= ((pFrom)->m_uInstruction2 >> 24) &0xFF;

  /* Pack Member: m_uInstruction3 */
  (pToData)[240] |= ((pFrom)->m_uInstruction3) & 0xFF;
  (pToData)[241] |= ((pFrom)->m_uInstruction3 >> 8) &0xFF;
  (pToData)[242] |= ((pFrom)->m_uInstruction3 >> 16) &0xFF;
  (pToData)[243] |= ((pFrom)->m_uInstruction3 >> 24) &0xFF;

  /* Pack Member: m_uResv0 */
  (pToData)[236] |= ((pFrom)->m_uResv0) & 0xFF;
  (pToData)[237] |= ((pFrom)->m_uResv0 >> 8) &0xFF;
  (pToData)[238] |= ((pFrom)->m_uResv0 >> 16) &0xFF;
  (pToData)[239] |= ((pFrom)->m_uResv0 >> 24) &0xFF;

  /* Pack Member: m_uResv1 */
  (pToData)[232] |= ((pFrom)->m_uResv1) & 0xFF;
  (pToData)[233] |= ((pFrom)->m_uResv1 >> 8) &0xFF;
  (pToData)[234] |= ((pFrom)->m_uResv1 >> 16) &0xFF;
  (pToData)[235] |= ((pFrom)->m_uResv1 >> 24) &0xFF;

  /* Pack Member: m_uResv2 */
  (pToData)[228] |= ((pFrom)->m_uResv2) & 0xFF;
  (pToData)[229] |= ((pFrom)->m_uResv2 >> 8) &0xFF;
  (pToData)[230] |= ((pFrom)->m_uResv2 >> 16) &0xFF;
  (pToData)[231] |= ((pFrom)->m_uResv2 >> 24) &0xFF;

  /* Pack Member: m_uResv3 */
  (pToData)[224] |= ((pFrom)->m_uResv3) & 0xFF;
  (pToData)[225] |= ((pFrom)->m_uResv3 >> 8) &0xFF;
  (pToData)[226] |= ((pFrom)->m_uResv3 >> 16) &0xFF;
  (pToData)[227] |= ((pFrom)->m_uResv3 >> 24) &0xFF;

  /* Pack Member: m_uResv4 */
  (pToData)[220] |= ((pFrom)->m_uResv4) & 0xFF;
  (pToData)[221] |= ((pFrom)->m_uResv4 >> 8) &0xFF;
  (pToData)[222] |= ((pFrom)->m_uResv4 >> 16) &0xFF;
  (pToData)[223] |= ((pFrom)->m_uResv4 >> 24) &0xFF;

  /* Pack Member: m_uResv5 */
  (pToData)[216] |= ((pFrom)->m_uResv5) & 0xFF;
  (pToData)[217] |= ((pFrom)->m_uResv5 >> 8) &0xFF;
  (pToData)[218] |= ((pFrom)->m_uResv5 >> 16) &0xFF;
  (pToData)[219] |= ((pFrom)->m_uResv5 >> 24) &0xFF;

  /* Pack Member: m_uResv6 */
  (pToData)[212] |= ((pFrom)->m_uResv6) & 0xFF;
  (pToData)[213] |= ((pFrom)->m_uResv6 >> 8) &0xFF;
  (pToData)[214] |= ((pFrom)->m_uResv6 >> 16) &0xFF;
  (pToData)[215] |= ((pFrom)->m_uResv6 >> 24) &0xFF;

  /* Pack Member: m_uResv7 */
  (pToData)[208] |= ((pFrom)->m_uResv7) & 0xFF;
  (pToData)[209] |= ((pFrom)->m_uResv7 >> 8) &0xFF;
  (pToData)[210] |= ((pFrom)->m_uResv7 >> 16) &0xFF;
  (pToData)[211] |= ((pFrom)->m_uResv7 >> 24) &0xFF;

  /* Pack Member: m_uResv8 */
  (pToData)[204] |= ((pFrom)->m_uResv8) & 0xFF;
  (pToData)[205] |= ((pFrom)->m_uResv8 >> 8) &0xFF;
  (pToData)[206] |= ((pFrom)->m_uResv8 >> 16) &0xFF;
  (pToData)[207] |= ((pFrom)->m_uResv8 >> 24) &0xFF;

  /* Pack Member: m_uResv9 */
  (pToData)[200] |= ((pFrom)->m_uResv9) & 0xFF;
  (pToData)[201] |= ((pFrom)->m_uResv9 >> 8) &0xFF;
  (pToData)[202] |= ((pFrom)->m_uResv9 >> 16) &0xFF;
  (pToData)[203] |= ((pFrom)->m_uResv9 >> 24) &0xFF;

  /* Pack Member: m_uResv10 */
  (pToData)[196] |= ((pFrom)->m_uResv10) & 0xFF;
  (pToData)[197] |= ((pFrom)->m_uResv10 >> 8) &0xFF;
  (pToData)[198] |= ((pFrom)->m_uResv10 >> 16) &0xFF;
  (pToData)[199] |= ((pFrom)->m_uResv10 >> 24) &0xFF;

  /* Pack Member: m_uResv11 */
  (pToData)[192] |= ((pFrom)->m_uResv11) & 0xFF;
  (pToData)[193] |= ((pFrom)->m_uResv11 >> 8) &0xFF;
  (pToData)[194] |= ((pFrom)->m_uResv11 >> 16) &0xFF;
  (pToData)[195] |= ((pFrom)->m_uResv11 >> 24) &0xFF;

  /* Pack Member: m_uResv12 */
  (pToData)[188] |= ((pFrom)->m_uResv12) & 0xFF;
  (pToData)[189] |= ((pFrom)->m_uResv12 >> 8) &0xFF;
  (pToData)[190] |= ((pFrom)->m_uResv12 >> 16) &0xFF;
  (pToData)[191] |= ((pFrom)->m_uResv12 >> 24) &0xFF;

  /* Pack Member: m_uResv13 */
  (pToData)[184] |= ((pFrom)->m_uResv13) & 0xFF;
  (pToData)[185] |= ((pFrom)->m_uResv13 >> 8) &0xFF;
  (pToData)[186] |= ((pFrom)->m_uResv13 >> 16) &0xFF;
  (pToData)[187] |= ((pFrom)->m_uResv13 >> 24) &0xFF;

  /* Pack Member: m_uResv14 */
  (pToData)[180] |= ((pFrom)->m_uResv14) & 0xFF;
  (pToData)[181] |= ((pFrom)->m_uResv14 >> 8) &0xFF;
  (pToData)[182] |= ((pFrom)->m_uResv14 >> 16) &0xFF;
  (pToData)[183] |= ((pFrom)->m_uResv14 >> 24) &0xFF;

  /* Pack Member: m_uResv15 */
  (pToData)[176] |= ((pFrom)->m_uResv15) & 0xFF;
  (pToData)[177] |= ((pFrom)->m_uResv15 >> 8) &0xFF;
  (pToData)[178] |= ((pFrom)->m_uResv15 >> 16) &0xFF;
  (pToData)[179] |= ((pFrom)->m_uResv15 >> 24) &0xFF;

  /* Pack Member: m_uResv16 */
  (pToData)[172] |= ((pFrom)->m_uResv16) & 0xFF;
  (pToData)[173] |= ((pFrom)->m_uResv16 >> 8) &0xFF;
  (pToData)[174] |= ((pFrom)->m_uResv16 >> 16) &0xFF;
  (pToData)[175] |= ((pFrom)->m_uResv16 >> 24) &0xFF;

  /* Pack Member: m_uResv17 */
  (pToData)[168] |= ((pFrom)->m_uResv17) & 0xFF;
  (pToData)[169] |= ((pFrom)->m_uResv17 >> 8) &0xFF;
  (pToData)[170] |= ((pFrom)->m_uResv17 >> 16) &0xFF;
  (pToData)[171] |= ((pFrom)->m_uResv17 >> 24) &0xFF;

  /* Pack Member: m_uResv18 */
  (pToData)[164] |= ((pFrom)->m_uResv18) & 0xFF;
  (pToData)[165] |= ((pFrom)->m_uResv18 >> 8) &0xFF;
  (pToData)[166] |= ((pFrom)->m_uResv18 >> 16) &0xFF;
  (pToData)[167] |= ((pFrom)->m_uResv18 >> 24) &0xFF;

  /* Pack Member: m_uResv19 */
  (pToData)[160] |= ((pFrom)->m_uResv19) & 0xFF;
  (pToData)[161] |= ((pFrom)->m_uResv19 >> 8) &0xFF;
  (pToData)[162] |= ((pFrom)->m_uResv19 >> 16) &0xFF;
  (pToData)[163] |= ((pFrom)->m_uResv19 >> 24) &0xFF;

  /* Pack Member: m_uResv20 */
  (pToData)[156] |= ((pFrom)->m_uResv20) & 0xFF;
  (pToData)[157] |= ((pFrom)->m_uResv20 >> 8) &0xFF;
  (pToData)[158] |= ((pFrom)->m_uResv20 >> 16) &0xFF;
  (pToData)[159] |= ((pFrom)->m_uResv20 >> 24) &0xFF;

  /* Pack Member: m_uResv21 */
  (pToData)[152] |= ((pFrom)->m_uResv21) & 0xFF;
  (pToData)[153] |= ((pFrom)->m_uResv21 >> 8) &0xFF;
  (pToData)[154] |= ((pFrom)->m_uResv21 >> 16) &0xFF;
  (pToData)[155] |= ((pFrom)->m_uResv21 >> 24) &0xFF;

  /* Pack Member: m_uResv22 */
  (pToData)[148] |= ((pFrom)->m_uResv22) & 0xFF;
  (pToData)[149] |= ((pFrom)->m_uResv22 >> 8) &0xFF;
  (pToData)[150] |= ((pFrom)->m_uResv22 >> 16) &0xFF;
  (pToData)[151] |= ((pFrom)->m_uResv22 >> 24) &0xFF;

  /* Pack Member: m_uResv23 */
  (pToData)[144] |= ((pFrom)->m_uResv23) & 0xFF;
  (pToData)[145] |= ((pFrom)->m_uResv23 >> 8) &0xFF;
  (pToData)[146] |= ((pFrom)->m_uResv23 >> 16) &0xFF;
  (pToData)[147] |= ((pFrom)->m_uResv23 >> 24) &0xFF;

  /* Pack Member: m_uResv24 */
  (pToData)[140] |= ((pFrom)->m_uResv24) & 0xFF;
  (pToData)[141] |= ((pFrom)->m_uResv24 >> 8) &0xFF;
  (pToData)[142] |= ((pFrom)->m_uResv24 >> 16) &0xFF;
  (pToData)[143] |= ((pFrom)->m_uResv24 >> 24) &0xFF;

  /* Pack Member: m_uResv25 */
  (pToData)[136] |= ((pFrom)->m_uResv25) & 0xFF;
  (pToData)[137] |= ((pFrom)->m_uResv25 >> 8) &0xFF;
  (pToData)[138] |= ((pFrom)->m_uResv25 >> 16) &0xFF;
  (pToData)[139] |= ((pFrom)->m_uResv25 >> 24) &0xFF;

  /* Pack Member: m_uResv26 */
  (pToData)[132] |= ((pFrom)->m_uResv26) & 0xFF;
  (pToData)[133] |= ((pFrom)->m_uResv26 >> 8) &0xFF;
  (pToData)[134] |= ((pFrom)->m_uResv26 >> 16) &0xFF;
  (pToData)[135] |= ((pFrom)->m_uResv26 >> 24) &0xFF;

  /* Pack Member: m_uResv27 */
  (pToData)[128] |= ((pFrom)->m_uResv27) & 0xFF;
  (pToData)[129] |= ((pFrom)->m_uResv27 >> 8) &0xFF;
  (pToData)[130] |= ((pFrom)->m_uResv27 >> 16) &0xFF;
  (pToData)[131] |= ((pFrom)->m_uResv27 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern0_31_0 */
  (pToData)[124] |= ((pFrom)->m_uSb0Pattern0_31_0) & 0xFF;
  (pToData)[125] |= ((pFrom)->m_uSb0Pattern0_31_0 >> 8) &0xFF;
  (pToData)[126] |= ((pFrom)->m_uSb0Pattern0_31_0 >> 16) &0xFF;
  (pToData)[127] |= ((pFrom)->m_uSb0Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern0_63_32 */
  (pToData)[120] |= ((pFrom)->m_uSb0Pattern0_63_32) & 0xFF;
  (pToData)[121] |= ((pFrom)->m_uSb0Pattern0_63_32 >> 8) &0xFF;
  (pToData)[122] |= ((pFrom)->m_uSb0Pattern0_63_32 >> 16) &0xFF;
  (pToData)[123] |= ((pFrom)->m_uSb0Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern0_95_64 */
  (pToData)[116] |= ((pFrom)->m_uSb0Pattern0_95_64) & 0xFF;
  (pToData)[117] |= ((pFrom)->m_uSb0Pattern0_95_64 >> 8) &0xFF;
  (pToData)[118] |= ((pFrom)->m_uSb0Pattern0_95_64 >> 16) &0xFF;
  (pToData)[119] |= ((pFrom)->m_uSb0Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern0_127_96 */
  (pToData)[112] |= ((pFrom)->m_uSb0Pattern0_127_96) & 0xFF;
  (pToData)[113] |= ((pFrom)->m_uSb0Pattern0_127_96 >> 8) &0xFF;
  (pToData)[114] |= ((pFrom)->m_uSb0Pattern0_127_96 >> 16) &0xFF;
  (pToData)[115] |= ((pFrom)->m_uSb0Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern1_31_0 */
  (pToData)[108] |= ((pFrom)->m_uSb0Pattern1_31_0) & 0xFF;
  (pToData)[109] |= ((pFrom)->m_uSb0Pattern1_31_0 >> 8) &0xFF;
  (pToData)[110] |= ((pFrom)->m_uSb0Pattern1_31_0 >> 16) &0xFF;
  (pToData)[111] |= ((pFrom)->m_uSb0Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern1_63_32 */
  (pToData)[104] |= ((pFrom)->m_uSb0Pattern1_63_32) & 0xFF;
  (pToData)[105] |= ((pFrom)->m_uSb0Pattern1_63_32 >> 8) &0xFF;
  (pToData)[106] |= ((pFrom)->m_uSb0Pattern1_63_32 >> 16) &0xFF;
  (pToData)[107] |= ((pFrom)->m_uSb0Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern1_95_64 */
  (pToData)[100] |= ((pFrom)->m_uSb0Pattern1_95_64) & 0xFF;
  (pToData)[101] |= ((pFrom)->m_uSb0Pattern1_95_64 >> 8) &0xFF;
  (pToData)[102] |= ((pFrom)->m_uSb0Pattern1_95_64 >> 16) &0xFF;
  (pToData)[103] |= ((pFrom)->m_uSb0Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern1_127_96 */
  (pToData)[96] |= ((pFrom)->m_uSb0Pattern1_127_96) & 0xFF;
  (pToData)[97] |= ((pFrom)->m_uSb0Pattern1_127_96 >> 8) &0xFF;
  (pToData)[98] |= ((pFrom)->m_uSb0Pattern1_127_96 >> 16) &0xFF;
  (pToData)[99] |= ((pFrom)->m_uSb0Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern2_31_0 */
  (pToData)[92] |= ((pFrom)->m_uSb0Pattern2_31_0) & 0xFF;
  (pToData)[93] |= ((pFrom)->m_uSb0Pattern2_31_0 >> 8) &0xFF;
  (pToData)[94] |= ((pFrom)->m_uSb0Pattern2_31_0 >> 16) &0xFF;
  (pToData)[95] |= ((pFrom)->m_uSb0Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern2_63_32 */
  (pToData)[88] |= ((pFrom)->m_uSb0Pattern2_63_32) & 0xFF;
  (pToData)[89] |= ((pFrom)->m_uSb0Pattern2_63_32 >> 8) &0xFF;
  (pToData)[90] |= ((pFrom)->m_uSb0Pattern2_63_32 >> 16) &0xFF;
  (pToData)[91] |= ((pFrom)->m_uSb0Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern2_95_64 */
  (pToData)[84] |= ((pFrom)->m_uSb0Pattern2_95_64) & 0xFF;
  (pToData)[85] |= ((pFrom)->m_uSb0Pattern2_95_64 >> 8) &0xFF;
  (pToData)[86] |= ((pFrom)->m_uSb0Pattern2_95_64 >> 16) &0xFF;
  (pToData)[87] |= ((pFrom)->m_uSb0Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern2_127_96 */
  (pToData)[80] |= ((pFrom)->m_uSb0Pattern2_127_96) & 0xFF;
  (pToData)[81] |= ((pFrom)->m_uSb0Pattern2_127_96 >> 8) &0xFF;
  (pToData)[82] |= ((pFrom)->m_uSb0Pattern2_127_96 >> 16) &0xFF;
  (pToData)[83] |= ((pFrom)->m_uSb0Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern3_31_0 */
  (pToData)[76] |= ((pFrom)->m_uSb0Pattern3_31_0) & 0xFF;
  (pToData)[77] |= ((pFrom)->m_uSb0Pattern3_31_0 >> 8) &0xFF;
  (pToData)[78] |= ((pFrom)->m_uSb0Pattern3_31_0 >> 16) &0xFF;
  (pToData)[79] |= ((pFrom)->m_uSb0Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern3_63_32 */
  (pToData)[72] |= ((pFrom)->m_uSb0Pattern3_63_32) & 0xFF;
  (pToData)[73] |= ((pFrom)->m_uSb0Pattern3_63_32 >> 8) &0xFF;
  (pToData)[74] |= ((pFrom)->m_uSb0Pattern3_63_32 >> 16) &0xFF;
  (pToData)[75] |= ((pFrom)->m_uSb0Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern3_95_64 */
  (pToData)[68] |= ((pFrom)->m_uSb0Pattern3_95_64) & 0xFF;
  (pToData)[69] |= ((pFrom)->m_uSb0Pattern3_95_64 >> 8) &0xFF;
  (pToData)[70] |= ((pFrom)->m_uSb0Pattern3_95_64 >> 16) &0xFF;
  (pToData)[71] |= ((pFrom)->m_uSb0Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb0Pattern3_127_96 */
  (pToData)[64] |= ((pFrom)->m_uSb0Pattern3_127_96) & 0xFF;
  (pToData)[65] |= ((pFrom)->m_uSb0Pattern3_127_96 >> 8) &0xFF;
  (pToData)[66] |= ((pFrom)->m_uSb0Pattern3_127_96 >> 16) &0xFF;
  (pToData)[67] |= ((pFrom)->m_uSb0Pattern3_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern0_31_0 */
  (pToData)[60] |= ((pFrom)->m_uSb1Pattern0_31_0) & 0xFF;
  (pToData)[61] |= ((pFrom)->m_uSb1Pattern0_31_0 >> 8) &0xFF;
  (pToData)[62] |= ((pFrom)->m_uSb1Pattern0_31_0 >> 16) &0xFF;
  (pToData)[63] |= ((pFrom)->m_uSb1Pattern0_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern0_63_32 */
  (pToData)[56] |= ((pFrom)->m_uSb1Pattern0_63_32) & 0xFF;
  (pToData)[57] |= ((pFrom)->m_uSb1Pattern0_63_32 >> 8) &0xFF;
  (pToData)[58] |= ((pFrom)->m_uSb1Pattern0_63_32 >> 16) &0xFF;
  (pToData)[59] |= ((pFrom)->m_uSb1Pattern0_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern0_95_64 */
  (pToData)[52] |= ((pFrom)->m_uSb1Pattern0_95_64) & 0xFF;
  (pToData)[53] |= ((pFrom)->m_uSb1Pattern0_95_64 >> 8) &0xFF;
  (pToData)[54] |= ((pFrom)->m_uSb1Pattern0_95_64 >> 16) &0xFF;
  (pToData)[55] |= ((pFrom)->m_uSb1Pattern0_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern0_127_96 */
  (pToData)[48] |= ((pFrom)->m_uSb1Pattern0_127_96) & 0xFF;
  (pToData)[49] |= ((pFrom)->m_uSb1Pattern0_127_96 >> 8) &0xFF;
  (pToData)[50] |= ((pFrom)->m_uSb1Pattern0_127_96 >> 16) &0xFF;
  (pToData)[51] |= ((pFrom)->m_uSb1Pattern0_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern1_31_0 */
  (pToData)[44] |= ((pFrom)->m_uSb1Pattern1_31_0) & 0xFF;
  (pToData)[45] |= ((pFrom)->m_uSb1Pattern1_31_0 >> 8) &0xFF;
  (pToData)[46] |= ((pFrom)->m_uSb1Pattern1_31_0 >> 16) &0xFF;
  (pToData)[47] |= ((pFrom)->m_uSb1Pattern1_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern1_63_32 */
  (pToData)[40] |= ((pFrom)->m_uSb1Pattern1_63_32) & 0xFF;
  (pToData)[41] |= ((pFrom)->m_uSb1Pattern1_63_32 >> 8) &0xFF;
  (pToData)[42] |= ((pFrom)->m_uSb1Pattern1_63_32 >> 16) &0xFF;
  (pToData)[43] |= ((pFrom)->m_uSb1Pattern1_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern1_95_64 */
  (pToData)[36] |= ((pFrom)->m_uSb1Pattern1_95_64) & 0xFF;
  (pToData)[37] |= ((pFrom)->m_uSb1Pattern1_95_64 >> 8) &0xFF;
  (pToData)[38] |= ((pFrom)->m_uSb1Pattern1_95_64 >> 16) &0xFF;
  (pToData)[39] |= ((pFrom)->m_uSb1Pattern1_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern1_127_96 */
  (pToData)[32] |= ((pFrom)->m_uSb1Pattern1_127_96) & 0xFF;
  (pToData)[33] |= ((pFrom)->m_uSb1Pattern1_127_96 >> 8) &0xFF;
  (pToData)[34] |= ((pFrom)->m_uSb1Pattern1_127_96 >> 16) &0xFF;
  (pToData)[35] |= ((pFrom)->m_uSb1Pattern1_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern2_31_0 */
  (pToData)[28] |= ((pFrom)->m_uSb1Pattern2_31_0) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_uSb1Pattern2_31_0 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_uSb1Pattern2_31_0 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_uSb1Pattern2_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern2_63_32 */
  (pToData)[24] |= ((pFrom)->m_uSb1Pattern2_63_32) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_uSb1Pattern2_63_32 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_uSb1Pattern2_63_32 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_uSb1Pattern2_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern2_95_64 */
  (pToData)[20] |= ((pFrom)->m_uSb1Pattern2_95_64) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_uSb1Pattern2_95_64 >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uSb1Pattern2_95_64 >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->m_uSb1Pattern2_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern2_127_96 */
  (pToData)[16] |= ((pFrom)->m_uSb1Pattern2_127_96) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_uSb1Pattern2_127_96 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uSb1Pattern2_127_96 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uSb1Pattern2_127_96 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern3_31_0 */
  (pToData)[12] |= ((pFrom)->m_uSb1Pattern3_31_0) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_uSb1Pattern3_31_0 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uSb1Pattern3_31_0 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uSb1Pattern3_31_0 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern3_63_32 */
  (pToData)[8] |= ((pFrom)->m_uSb1Pattern3_63_32) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_uSb1Pattern3_63_32 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uSb1Pattern3_63_32 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uSb1Pattern3_63_32 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern3_95_64 */
  (pToData)[4] |= ((pFrom)->m_uSb1Pattern3_95_64) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uSb1Pattern3_95_64 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uSb1Pattern3_95_64 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uSb1Pattern3_95_64 >> 24) &0xFF;

  /* Pack Member: m_uSb1Pattern3_127_96 */
  (pToData)[0] |= ((pFrom)->m_uSb1Pattern3_127_96) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uSb1Pattern3_127_96 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uSb1Pattern3_127_96 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uSb1Pattern3_127_96 >> 24) &0xFF;

  return SB_ZF_CARCDMAFORMAT_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaRcDmaFormat_Unpack(sbZfCaRcDmaFormat_t *pToStruct,
                         uint8_t *pFromData,
                         uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uInstruction0 */
  (pToStruct)->m_uInstruction0 =  (uint32_t)  (pFromData)[252] ;
  (pToStruct)->m_uInstruction0 |=  (uint32_t)  (pFromData)[253] << 8;
  (pToStruct)->m_uInstruction0 |=  (uint32_t)  (pFromData)[254] << 16;
  (pToStruct)->m_uInstruction0 |=  (uint32_t)  (pFromData)[255] << 24;

  /* Unpack Member: m_uInstruction1 */
  (pToStruct)->m_uInstruction1 =  (uint32_t)  (pFromData)[248] ;
  (pToStruct)->m_uInstruction1 |=  (uint32_t)  (pFromData)[249] << 8;
  (pToStruct)->m_uInstruction1 |=  (uint32_t)  (pFromData)[250] << 16;
  (pToStruct)->m_uInstruction1 |=  (uint32_t)  (pFromData)[251] << 24;

  /* Unpack Member: m_uInstruction2 */
  (pToStruct)->m_uInstruction2 =  (uint32_t)  (pFromData)[244] ;
  (pToStruct)->m_uInstruction2 |=  (uint32_t)  (pFromData)[245] << 8;
  (pToStruct)->m_uInstruction2 |=  (uint32_t)  (pFromData)[246] << 16;
  (pToStruct)->m_uInstruction2 |=  (uint32_t)  (pFromData)[247] << 24;

  /* Unpack Member: m_uInstruction3 */
  (pToStruct)->m_uInstruction3 =  (uint32_t)  (pFromData)[240] ;
  (pToStruct)->m_uInstruction3 |=  (uint32_t)  (pFromData)[241] << 8;
  (pToStruct)->m_uInstruction3 |=  (uint32_t)  (pFromData)[242] << 16;
  (pToStruct)->m_uInstruction3 |=  (uint32_t)  (pFromData)[243] << 24;

  /* Unpack Member: m_uResv0 */
  (pToStruct)->m_uResv0 =  (uint32_t)  (pFromData)[236] ;
  (pToStruct)->m_uResv0 |=  (uint32_t)  (pFromData)[237] << 8;
  (pToStruct)->m_uResv0 |=  (uint32_t)  (pFromData)[238] << 16;
  (pToStruct)->m_uResv0 |=  (uint32_t)  (pFromData)[239] << 24;

  /* Unpack Member: m_uResv1 */
  (pToStruct)->m_uResv1 =  (uint32_t)  (pFromData)[232] ;
  (pToStruct)->m_uResv1 |=  (uint32_t)  (pFromData)[233] << 8;
  (pToStruct)->m_uResv1 |=  (uint32_t)  (pFromData)[234] << 16;
  (pToStruct)->m_uResv1 |=  (uint32_t)  (pFromData)[235] << 24;

  /* Unpack Member: m_uResv2 */
  (pToStruct)->m_uResv2 =  (uint32_t)  (pFromData)[228] ;
  (pToStruct)->m_uResv2 |=  (uint32_t)  (pFromData)[229] << 8;
  (pToStruct)->m_uResv2 |=  (uint32_t)  (pFromData)[230] << 16;
  (pToStruct)->m_uResv2 |=  (uint32_t)  (pFromData)[231] << 24;

  /* Unpack Member: m_uResv3 */
  (pToStruct)->m_uResv3 =  (uint32_t)  (pFromData)[224] ;
  (pToStruct)->m_uResv3 |=  (uint32_t)  (pFromData)[225] << 8;
  (pToStruct)->m_uResv3 |=  (uint32_t)  (pFromData)[226] << 16;
  (pToStruct)->m_uResv3 |=  (uint32_t)  (pFromData)[227] << 24;

  /* Unpack Member: m_uResv4 */
  (pToStruct)->m_uResv4 =  (uint32_t)  (pFromData)[220] ;
  (pToStruct)->m_uResv4 |=  (uint32_t)  (pFromData)[221] << 8;
  (pToStruct)->m_uResv4 |=  (uint32_t)  (pFromData)[222] << 16;
  (pToStruct)->m_uResv4 |=  (uint32_t)  (pFromData)[223] << 24;

  /* Unpack Member: m_uResv5 */
  (pToStruct)->m_uResv5 =  (uint32_t)  (pFromData)[216] ;
  (pToStruct)->m_uResv5 |=  (uint32_t)  (pFromData)[217] << 8;
  (pToStruct)->m_uResv5 |=  (uint32_t)  (pFromData)[218] << 16;
  (pToStruct)->m_uResv5 |=  (uint32_t)  (pFromData)[219] << 24;

  /* Unpack Member: m_uResv6 */
  (pToStruct)->m_uResv6 =  (uint32_t)  (pFromData)[212] ;
  (pToStruct)->m_uResv6 |=  (uint32_t)  (pFromData)[213] << 8;
  (pToStruct)->m_uResv6 |=  (uint32_t)  (pFromData)[214] << 16;
  (pToStruct)->m_uResv6 |=  (uint32_t)  (pFromData)[215] << 24;

  /* Unpack Member: m_uResv7 */
  (pToStruct)->m_uResv7 =  (uint32_t)  (pFromData)[208] ;
  (pToStruct)->m_uResv7 |=  (uint32_t)  (pFromData)[209] << 8;
  (pToStruct)->m_uResv7 |=  (uint32_t)  (pFromData)[210] << 16;
  (pToStruct)->m_uResv7 |=  (uint32_t)  (pFromData)[211] << 24;

  /* Unpack Member: m_uResv8 */
  (pToStruct)->m_uResv8 =  (uint32_t)  (pFromData)[204] ;
  (pToStruct)->m_uResv8 |=  (uint32_t)  (pFromData)[205] << 8;
  (pToStruct)->m_uResv8 |=  (uint32_t)  (pFromData)[206] << 16;
  (pToStruct)->m_uResv8 |=  (uint32_t)  (pFromData)[207] << 24;

  /* Unpack Member: m_uResv9 */
  (pToStruct)->m_uResv9 =  (uint32_t)  (pFromData)[200] ;
  (pToStruct)->m_uResv9 |=  (uint32_t)  (pFromData)[201] << 8;
  (pToStruct)->m_uResv9 |=  (uint32_t)  (pFromData)[202] << 16;
  (pToStruct)->m_uResv9 |=  (uint32_t)  (pFromData)[203] << 24;

  /* Unpack Member: m_uResv10 */
  (pToStruct)->m_uResv10 =  (uint32_t)  (pFromData)[196] ;
  (pToStruct)->m_uResv10 |=  (uint32_t)  (pFromData)[197] << 8;
  (pToStruct)->m_uResv10 |=  (uint32_t)  (pFromData)[198] << 16;
  (pToStruct)->m_uResv10 |=  (uint32_t)  (pFromData)[199] << 24;

  /* Unpack Member: m_uResv11 */
  (pToStruct)->m_uResv11 =  (uint32_t)  (pFromData)[192] ;
  (pToStruct)->m_uResv11 |=  (uint32_t)  (pFromData)[193] << 8;
  (pToStruct)->m_uResv11 |=  (uint32_t)  (pFromData)[194] << 16;
  (pToStruct)->m_uResv11 |=  (uint32_t)  (pFromData)[195] << 24;

  /* Unpack Member: m_uResv12 */
  (pToStruct)->m_uResv12 =  (uint32_t)  (pFromData)[188] ;
  (pToStruct)->m_uResv12 |=  (uint32_t)  (pFromData)[189] << 8;
  (pToStruct)->m_uResv12 |=  (uint32_t)  (pFromData)[190] << 16;
  (pToStruct)->m_uResv12 |=  (uint32_t)  (pFromData)[191] << 24;

  /* Unpack Member: m_uResv13 */
  (pToStruct)->m_uResv13 =  (uint32_t)  (pFromData)[184] ;
  (pToStruct)->m_uResv13 |=  (uint32_t)  (pFromData)[185] << 8;
  (pToStruct)->m_uResv13 |=  (uint32_t)  (pFromData)[186] << 16;
  (pToStruct)->m_uResv13 |=  (uint32_t)  (pFromData)[187] << 24;

  /* Unpack Member: m_uResv14 */
  (pToStruct)->m_uResv14 =  (uint32_t)  (pFromData)[180] ;
  (pToStruct)->m_uResv14 |=  (uint32_t)  (pFromData)[181] << 8;
  (pToStruct)->m_uResv14 |=  (uint32_t)  (pFromData)[182] << 16;
  (pToStruct)->m_uResv14 |=  (uint32_t)  (pFromData)[183] << 24;

  /* Unpack Member: m_uResv15 */
  (pToStruct)->m_uResv15 =  (uint32_t)  (pFromData)[176] ;
  (pToStruct)->m_uResv15 |=  (uint32_t)  (pFromData)[177] << 8;
  (pToStruct)->m_uResv15 |=  (uint32_t)  (pFromData)[178] << 16;
  (pToStruct)->m_uResv15 |=  (uint32_t)  (pFromData)[179] << 24;

  /* Unpack Member: m_uResv16 */
  (pToStruct)->m_uResv16 =  (uint32_t)  (pFromData)[172] ;
  (pToStruct)->m_uResv16 |=  (uint32_t)  (pFromData)[173] << 8;
  (pToStruct)->m_uResv16 |=  (uint32_t)  (pFromData)[174] << 16;
  (pToStruct)->m_uResv16 |=  (uint32_t)  (pFromData)[175] << 24;

  /* Unpack Member: m_uResv17 */
  (pToStruct)->m_uResv17 =  (uint32_t)  (pFromData)[168] ;
  (pToStruct)->m_uResv17 |=  (uint32_t)  (pFromData)[169] << 8;
  (pToStruct)->m_uResv17 |=  (uint32_t)  (pFromData)[170] << 16;
  (pToStruct)->m_uResv17 |=  (uint32_t)  (pFromData)[171] << 24;

  /* Unpack Member: m_uResv18 */
  (pToStruct)->m_uResv18 =  (uint32_t)  (pFromData)[164] ;
  (pToStruct)->m_uResv18 |=  (uint32_t)  (pFromData)[165] << 8;
  (pToStruct)->m_uResv18 |=  (uint32_t)  (pFromData)[166] << 16;
  (pToStruct)->m_uResv18 |=  (uint32_t)  (pFromData)[167] << 24;

  /* Unpack Member: m_uResv19 */
  (pToStruct)->m_uResv19 =  (uint32_t)  (pFromData)[160] ;
  (pToStruct)->m_uResv19 |=  (uint32_t)  (pFromData)[161] << 8;
  (pToStruct)->m_uResv19 |=  (uint32_t)  (pFromData)[162] << 16;
  (pToStruct)->m_uResv19 |=  (uint32_t)  (pFromData)[163] << 24;

  /* Unpack Member: m_uResv20 */
  (pToStruct)->m_uResv20 =  (uint32_t)  (pFromData)[156] ;
  (pToStruct)->m_uResv20 |=  (uint32_t)  (pFromData)[157] << 8;
  (pToStruct)->m_uResv20 |=  (uint32_t)  (pFromData)[158] << 16;
  (pToStruct)->m_uResv20 |=  (uint32_t)  (pFromData)[159] << 24;

  /* Unpack Member: m_uResv21 */
  (pToStruct)->m_uResv21 =  (uint32_t)  (pFromData)[152] ;
  (pToStruct)->m_uResv21 |=  (uint32_t)  (pFromData)[153] << 8;
  (pToStruct)->m_uResv21 |=  (uint32_t)  (pFromData)[154] << 16;
  (pToStruct)->m_uResv21 |=  (uint32_t)  (pFromData)[155] << 24;

  /* Unpack Member: m_uResv22 */
  (pToStruct)->m_uResv22 =  (uint32_t)  (pFromData)[148] ;
  (pToStruct)->m_uResv22 |=  (uint32_t)  (pFromData)[149] << 8;
  (pToStruct)->m_uResv22 |=  (uint32_t)  (pFromData)[150] << 16;
  (pToStruct)->m_uResv22 |=  (uint32_t)  (pFromData)[151] << 24;

  /* Unpack Member: m_uResv23 */
  (pToStruct)->m_uResv23 =  (uint32_t)  (pFromData)[144] ;
  (pToStruct)->m_uResv23 |=  (uint32_t)  (pFromData)[145] << 8;
  (pToStruct)->m_uResv23 |=  (uint32_t)  (pFromData)[146] << 16;
  (pToStruct)->m_uResv23 |=  (uint32_t)  (pFromData)[147] << 24;

  /* Unpack Member: m_uResv24 */
  (pToStruct)->m_uResv24 =  (uint32_t)  (pFromData)[140] ;
  (pToStruct)->m_uResv24 |=  (uint32_t)  (pFromData)[141] << 8;
  (pToStruct)->m_uResv24 |=  (uint32_t)  (pFromData)[142] << 16;
  (pToStruct)->m_uResv24 |=  (uint32_t)  (pFromData)[143] << 24;

  /* Unpack Member: m_uResv25 */
  (pToStruct)->m_uResv25 =  (uint32_t)  (pFromData)[136] ;
  (pToStruct)->m_uResv25 |=  (uint32_t)  (pFromData)[137] << 8;
  (pToStruct)->m_uResv25 |=  (uint32_t)  (pFromData)[138] << 16;
  (pToStruct)->m_uResv25 |=  (uint32_t)  (pFromData)[139] << 24;

  /* Unpack Member: m_uResv26 */
  (pToStruct)->m_uResv26 =  (uint32_t)  (pFromData)[132] ;
  (pToStruct)->m_uResv26 |=  (uint32_t)  (pFromData)[133] << 8;
  (pToStruct)->m_uResv26 |=  (uint32_t)  (pFromData)[134] << 16;
  (pToStruct)->m_uResv26 |=  (uint32_t)  (pFromData)[135] << 24;

  /* Unpack Member: m_uResv27 */
  (pToStruct)->m_uResv27 =  (uint32_t)  (pFromData)[128] ;
  (pToStruct)->m_uResv27 |=  (uint32_t)  (pFromData)[129] << 8;
  (pToStruct)->m_uResv27 |=  (uint32_t)  (pFromData)[130] << 16;
  (pToStruct)->m_uResv27 |=  (uint32_t)  (pFromData)[131] << 24;

  /* Unpack Member: m_uSb0Pattern0_31_0 */
  (pToStruct)->m_uSb0Pattern0_31_0 =  (uint32_t)  (pFromData)[124] ;
  (pToStruct)->m_uSb0Pattern0_31_0 |=  (uint32_t)  (pFromData)[125] << 8;
  (pToStruct)->m_uSb0Pattern0_31_0 |=  (uint32_t)  (pFromData)[126] << 16;
  (pToStruct)->m_uSb0Pattern0_31_0 |=  (uint32_t)  (pFromData)[127] << 24;

  /* Unpack Member: m_uSb0Pattern0_63_32 */
  (pToStruct)->m_uSb0Pattern0_63_32 =  (uint32_t)  (pFromData)[120] ;
  (pToStruct)->m_uSb0Pattern0_63_32 |=  (uint32_t)  (pFromData)[121] << 8;
  (pToStruct)->m_uSb0Pattern0_63_32 |=  (uint32_t)  (pFromData)[122] << 16;
  (pToStruct)->m_uSb0Pattern0_63_32 |=  (uint32_t)  (pFromData)[123] << 24;

  /* Unpack Member: m_uSb0Pattern0_95_64 */
  (pToStruct)->m_uSb0Pattern0_95_64 =  (uint32_t)  (pFromData)[116] ;
  (pToStruct)->m_uSb0Pattern0_95_64 |=  (uint32_t)  (pFromData)[117] << 8;
  (pToStruct)->m_uSb0Pattern0_95_64 |=  (uint32_t)  (pFromData)[118] << 16;
  (pToStruct)->m_uSb0Pattern0_95_64 |=  (uint32_t)  (pFromData)[119] << 24;

  /* Unpack Member: m_uSb0Pattern0_127_96 */
  (pToStruct)->m_uSb0Pattern0_127_96 =  (uint32_t)  (pFromData)[112] ;
  (pToStruct)->m_uSb0Pattern0_127_96 |=  (uint32_t)  (pFromData)[113] << 8;
  (pToStruct)->m_uSb0Pattern0_127_96 |=  (uint32_t)  (pFromData)[114] << 16;
  (pToStruct)->m_uSb0Pattern0_127_96 |=  (uint32_t)  (pFromData)[115] << 24;

  /* Unpack Member: m_uSb0Pattern1_31_0 */
  (pToStruct)->m_uSb0Pattern1_31_0 =  (uint32_t)  (pFromData)[108] ;
  (pToStruct)->m_uSb0Pattern1_31_0 |=  (uint32_t)  (pFromData)[109] << 8;
  (pToStruct)->m_uSb0Pattern1_31_0 |=  (uint32_t)  (pFromData)[110] << 16;
  (pToStruct)->m_uSb0Pattern1_31_0 |=  (uint32_t)  (pFromData)[111] << 24;

  /* Unpack Member: m_uSb0Pattern1_63_32 */
  (pToStruct)->m_uSb0Pattern1_63_32 =  (uint32_t)  (pFromData)[104] ;
  (pToStruct)->m_uSb0Pattern1_63_32 |=  (uint32_t)  (pFromData)[105] << 8;
  (pToStruct)->m_uSb0Pattern1_63_32 |=  (uint32_t)  (pFromData)[106] << 16;
  (pToStruct)->m_uSb0Pattern1_63_32 |=  (uint32_t)  (pFromData)[107] << 24;

  /* Unpack Member: m_uSb0Pattern1_95_64 */
  (pToStruct)->m_uSb0Pattern1_95_64 =  (uint32_t)  (pFromData)[100] ;
  (pToStruct)->m_uSb0Pattern1_95_64 |=  (uint32_t)  (pFromData)[101] << 8;
  (pToStruct)->m_uSb0Pattern1_95_64 |=  (uint32_t)  (pFromData)[102] << 16;
  (pToStruct)->m_uSb0Pattern1_95_64 |=  (uint32_t)  (pFromData)[103] << 24;

  /* Unpack Member: m_uSb0Pattern1_127_96 */
  (pToStruct)->m_uSb0Pattern1_127_96 =  (uint32_t)  (pFromData)[96] ;
  (pToStruct)->m_uSb0Pattern1_127_96 |=  (uint32_t)  (pFromData)[97] << 8;
  (pToStruct)->m_uSb0Pattern1_127_96 |=  (uint32_t)  (pFromData)[98] << 16;
  (pToStruct)->m_uSb0Pattern1_127_96 |=  (uint32_t)  (pFromData)[99] << 24;

  /* Unpack Member: m_uSb0Pattern2_31_0 */
  (pToStruct)->m_uSb0Pattern2_31_0 =  (uint32_t)  (pFromData)[92] ;
  (pToStruct)->m_uSb0Pattern2_31_0 |=  (uint32_t)  (pFromData)[93] << 8;
  (pToStruct)->m_uSb0Pattern2_31_0 |=  (uint32_t)  (pFromData)[94] << 16;
  (pToStruct)->m_uSb0Pattern2_31_0 |=  (uint32_t)  (pFromData)[95] << 24;

  /* Unpack Member: m_uSb0Pattern2_63_32 */
  (pToStruct)->m_uSb0Pattern2_63_32 =  (uint32_t)  (pFromData)[88] ;
  (pToStruct)->m_uSb0Pattern2_63_32 |=  (uint32_t)  (pFromData)[89] << 8;
  (pToStruct)->m_uSb0Pattern2_63_32 |=  (uint32_t)  (pFromData)[90] << 16;
  (pToStruct)->m_uSb0Pattern2_63_32 |=  (uint32_t)  (pFromData)[91] << 24;

  /* Unpack Member: m_uSb0Pattern2_95_64 */
  (pToStruct)->m_uSb0Pattern2_95_64 =  (uint32_t)  (pFromData)[84] ;
  (pToStruct)->m_uSb0Pattern2_95_64 |=  (uint32_t)  (pFromData)[85] << 8;
  (pToStruct)->m_uSb0Pattern2_95_64 |=  (uint32_t)  (pFromData)[86] << 16;
  (pToStruct)->m_uSb0Pattern2_95_64 |=  (uint32_t)  (pFromData)[87] << 24;

  /* Unpack Member: m_uSb0Pattern2_127_96 */
  (pToStruct)->m_uSb0Pattern2_127_96 =  (uint32_t)  (pFromData)[80] ;
  (pToStruct)->m_uSb0Pattern2_127_96 |=  (uint32_t)  (pFromData)[81] << 8;
  (pToStruct)->m_uSb0Pattern2_127_96 |=  (uint32_t)  (pFromData)[82] << 16;
  (pToStruct)->m_uSb0Pattern2_127_96 |=  (uint32_t)  (pFromData)[83] << 24;

  /* Unpack Member: m_uSb0Pattern3_31_0 */
  (pToStruct)->m_uSb0Pattern3_31_0 =  (uint32_t)  (pFromData)[76] ;
  (pToStruct)->m_uSb0Pattern3_31_0 |=  (uint32_t)  (pFromData)[77] << 8;
  (pToStruct)->m_uSb0Pattern3_31_0 |=  (uint32_t)  (pFromData)[78] << 16;
  (pToStruct)->m_uSb0Pattern3_31_0 |=  (uint32_t)  (pFromData)[79] << 24;

  /* Unpack Member: m_uSb0Pattern3_63_32 */
  (pToStruct)->m_uSb0Pattern3_63_32 =  (uint32_t)  (pFromData)[72] ;
  (pToStruct)->m_uSb0Pattern3_63_32 |=  (uint32_t)  (pFromData)[73] << 8;
  (pToStruct)->m_uSb0Pattern3_63_32 |=  (uint32_t)  (pFromData)[74] << 16;
  (pToStruct)->m_uSb0Pattern3_63_32 |=  (uint32_t)  (pFromData)[75] << 24;

  /* Unpack Member: m_uSb0Pattern3_95_64 */
  (pToStruct)->m_uSb0Pattern3_95_64 =  (uint32_t)  (pFromData)[68] ;
  (pToStruct)->m_uSb0Pattern3_95_64 |=  (uint32_t)  (pFromData)[69] << 8;
  (pToStruct)->m_uSb0Pattern3_95_64 |=  (uint32_t)  (pFromData)[70] << 16;
  (pToStruct)->m_uSb0Pattern3_95_64 |=  (uint32_t)  (pFromData)[71] << 24;

  /* Unpack Member: m_uSb0Pattern3_127_96 */
  (pToStruct)->m_uSb0Pattern3_127_96 =  (uint32_t)  (pFromData)[64] ;
  (pToStruct)->m_uSb0Pattern3_127_96 |=  (uint32_t)  (pFromData)[65] << 8;
  (pToStruct)->m_uSb0Pattern3_127_96 |=  (uint32_t)  (pFromData)[66] << 16;
  (pToStruct)->m_uSb0Pattern3_127_96 |=  (uint32_t)  (pFromData)[67] << 24;

  /* Unpack Member: m_uSb1Pattern0_31_0 */
  (pToStruct)->m_uSb1Pattern0_31_0 =  (uint32_t)  (pFromData)[60] ;
  (pToStruct)->m_uSb1Pattern0_31_0 |=  (uint32_t)  (pFromData)[61] << 8;
  (pToStruct)->m_uSb1Pattern0_31_0 |=  (uint32_t)  (pFromData)[62] << 16;
  (pToStruct)->m_uSb1Pattern0_31_0 |=  (uint32_t)  (pFromData)[63] << 24;

  /* Unpack Member: m_uSb1Pattern0_63_32 */
  (pToStruct)->m_uSb1Pattern0_63_32 =  (uint32_t)  (pFromData)[56] ;
  (pToStruct)->m_uSb1Pattern0_63_32 |=  (uint32_t)  (pFromData)[57] << 8;
  (pToStruct)->m_uSb1Pattern0_63_32 |=  (uint32_t)  (pFromData)[58] << 16;
  (pToStruct)->m_uSb1Pattern0_63_32 |=  (uint32_t)  (pFromData)[59] << 24;

  /* Unpack Member: m_uSb1Pattern0_95_64 */
  (pToStruct)->m_uSb1Pattern0_95_64 =  (uint32_t)  (pFromData)[52] ;
  (pToStruct)->m_uSb1Pattern0_95_64 |=  (uint32_t)  (pFromData)[53] << 8;
  (pToStruct)->m_uSb1Pattern0_95_64 |=  (uint32_t)  (pFromData)[54] << 16;
  (pToStruct)->m_uSb1Pattern0_95_64 |=  (uint32_t)  (pFromData)[55] << 24;

  /* Unpack Member: m_uSb1Pattern0_127_96 */
  (pToStruct)->m_uSb1Pattern0_127_96 =  (uint32_t)  (pFromData)[48] ;
  (pToStruct)->m_uSb1Pattern0_127_96 |=  (uint32_t)  (pFromData)[49] << 8;
  (pToStruct)->m_uSb1Pattern0_127_96 |=  (uint32_t)  (pFromData)[50] << 16;
  (pToStruct)->m_uSb1Pattern0_127_96 |=  (uint32_t)  (pFromData)[51] << 24;

  /* Unpack Member: m_uSb1Pattern1_31_0 */
  (pToStruct)->m_uSb1Pattern1_31_0 =  (uint32_t)  (pFromData)[44] ;
  (pToStruct)->m_uSb1Pattern1_31_0 |=  (uint32_t)  (pFromData)[45] << 8;
  (pToStruct)->m_uSb1Pattern1_31_0 |=  (uint32_t)  (pFromData)[46] << 16;
  (pToStruct)->m_uSb1Pattern1_31_0 |=  (uint32_t)  (pFromData)[47] << 24;

  /* Unpack Member: m_uSb1Pattern1_63_32 */
  (pToStruct)->m_uSb1Pattern1_63_32 =  (uint32_t)  (pFromData)[40] ;
  (pToStruct)->m_uSb1Pattern1_63_32 |=  (uint32_t)  (pFromData)[41] << 8;
  (pToStruct)->m_uSb1Pattern1_63_32 |=  (uint32_t)  (pFromData)[42] << 16;
  (pToStruct)->m_uSb1Pattern1_63_32 |=  (uint32_t)  (pFromData)[43] << 24;

  /* Unpack Member: m_uSb1Pattern1_95_64 */
  (pToStruct)->m_uSb1Pattern1_95_64 =  (uint32_t)  (pFromData)[36] ;
  (pToStruct)->m_uSb1Pattern1_95_64 |=  (uint32_t)  (pFromData)[37] << 8;
  (pToStruct)->m_uSb1Pattern1_95_64 |=  (uint32_t)  (pFromData)[38] << 16;
  (pToStruct)->m_uSb1Pattern1_95_64 |=  (uint32_t)  (pFromData)[39] << 24;

  /* Unpack Member: m_uSb1Pattern1_127_96 */
  (pToStruct)->m_uSb1Pattern1_127_96 =  (uint32_t)  (pFromData)[32] ;
  (pToStruct)->m_uSb1Pattern1_127_96 |=  (uint32_t)  (pFromData)[33] << 8;
  (pToStruct)->m_uSb1Pattern1_127_96 |=  (uint32_t)  (pFromData)[34] << 16;
  (pToStruct)->m_uSb1Pattern1_127_96 |=  (uint32_t)  (pFromData)[35] << 24;

  /* Unpack Member: m_uSb1Pattern2_31_0 */
  (pToStruct)->m_uSb1Pattern2_31_0 =  (uint32_t)  (pFromData)[28] ;
  (pToStruct)->m_uSb1Pattern2_31_0 |=  (uint32_t)  (pFromData)[29] << 8;
  (pToStruct)->m_uSb1Pattern2_31_0 |=  (uint32_t)  (pFromData)[30] << 16;
  (pToStruct)->m_uSb1Pattern2_31_0 |=  (uint32_t)  (pFromData)[31] << 24;

  /* Unpack Member: m_uSb1Pattern2_63_32 */
  (pToStruct)->m_uSb1Pattern2_63_32 =  (uint32_t)  (pFromData)[24] ;
  (pToStruct)->m_uSb1Pattern2_63_32 |=  (uint32_t)  (pFromData)[25] << 8;
  (pToStruct)->m_uSb1Pattern2_63_32 |=  (uint32_t)  (pFromData)[26] << 16;
  (pToStruct)->m_uSb1Pattern2_63_32 |=  (uint32_t)  (pFromData)[27] << 24;

  /* Unpack Member: m_uSb1Pattern2_95_64 */
  (pToStruct)->m_uSb1Pattern2_95_64 =  (uint32_t)  (pFromData)[20] ;
  (pToStruct)->m_uSb1Pattern2_95_64 |=  (uint32_t)  (pFromData)[21] << 8;
  (pToStruct)->m_uSb1Pattern2_95_64 |=  (uint32_t)  (pFromData)[22] << 16;
  (pToStruct)->m_uSb1Pattern2_95_64 |=  (uint32_t)  (pFromData)[23] << 24;

  /* Unpack Member: m_uSb1Pattern2_127_96 */
  (pToStruct)->m_uSb1Pattern2_127_96 =  (uint32_t)  (pFromData)[16] ;
  (pToStruct)->m_uSb1Pattern2_127_96 |=  (uint32_t)  (pFromData)[17] << 8;
  (pToStruct)->m_uSb1Pattern2_127_96 |=  (uint32_t)  (pFromData)[18] << 16;
  (pToStruct)->m_uSb1Pattern2_127_96 |=  (uint32_t)  (pFromData)[19] << 24;

  /* Unpack Member: m_uSb1Pattern3_31_0 */
  (pToStruct)->m_uSb1Pattern3_31_0 =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->m_uSb1Pattern3_31_0 |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->m_uSb1Pattern3_31_0 |=  (uint32_t)  (pFromData)[14] << 16;
  (pToStruct)->m_uSb1Pattern3_31_0 |=  (uint32_t)  (pFromData)[15] << 24;

  /* Unpack Member: m_uSb1Pattern3_63_32 */
  (pToStruct)->m_uSb1Pattern3_63_32 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_uSb1Pattern3_63_32 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_uSb1Pattern3_63_32 |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_uSb1Pattern3_63_32 |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_uSb1Pattern3_95_64 */
  (pToStruct)->m_uSb1Pattern3_95_64 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uSb1Pattern3_95_64 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uSb1Pattern3_95_64 |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_uSb1Pattern3_95_64 |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_uSb1Pattern3_127_96 */
  (pToStruct)->m_uSb1Pattern3_127_96 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uSb1Pattern3_127_96 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_uSb1Pattern3_127_96 |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_uSb1Pattern3_127_96 |=  (uint32_t)  (pFromData)[3] << 24;

}



/* initialize an instance of this zframe */
void
sbZfCaRcDmaFormat_InitInstance(sbZfCaRcDmaFormat_t *pFrame) {

  pFrame->m_uInstruction0 =  (unsigned int)  0;
  pFrame->m_uInstruction1 =  (unsigned int)  0;
  pFrame->m_uInstruction2 =  (unsigned int)  0;
  pFrame->m_uInstruction3 =  (unsigned int)  0;
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
  pFrame->m_uResv24 =  (unsigned int)  0;
  pFrame->m_uResv25 =  (unsigned int)  0;
  pFrame->m_uResv26 =  (unsigned int)  0;
  pFrame->m_uResv27 =  (unsigned int)  0;
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

}
