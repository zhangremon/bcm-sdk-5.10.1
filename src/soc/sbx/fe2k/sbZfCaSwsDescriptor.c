/*
 * $Id: sbZfCaSwsDescriptor.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaSwsDescriptor.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaSwsDescriptor_Pack(sbZfCaSwsDescriptor_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_ZFCASWSDESCRIPTOR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uResv0 */
  (pToData)[171] |= ((pFrom)->m_uResv0 & 0x0f) <<4;

  /* Pack Member: m_uSourceBuffer */
  (pToData)[169] |= ((pFrom)->m_uSourceBuffer & 0x03) <<6;
  (pToData)[170] |= ((pFrom)->m_uSourceBuffer >> 2) &0xFF;
  (pToData)[171] |= ((pFrom)->m_uSourceBuffer >> 10) & 0x0f;

  /* Pack Member: m_uResv1 */
  (pToData)[168] |= ((pFrom)->m_uResv1) & 0xFF;
  (pToData)[169] |= ((pFrom)->m_uResv1 >> 8) & 0x7f;

  /* Pack Member: m_uStreamSelector */
  (pToData)[167] |= ((pFrom)->m_uStreamSelector & 0x07) <<5;

  /* Pack Member: m_uBufferNumber */
  (pToData)[166] |= ((pFrom)->m_uBufferNumber) & 0xFF;
  (pToData)[167] |= ((pFrom)->m_uBufferNumber >> 8) & 0x1f;

  /* Pack Member: m_uExceptionIndex */
  (pToData)[165] |= ((pFrom)->m_uExceptionIndex) & 0xFF;

  /* Pack Member: m_uSourceQueue */
  (pToData)[164] |= ((pFrom)->m_uSourceQueue) & 0xFF;

  /* Pack Member: m_uResv2 */
  (pToData)[163] |= ((pFrom)->m_uResv2) & 0xFF;

  /* Pack Member: m_uReplicantCnt */
  (pToData)[161] |= ((pFrom)->m_uReplicantCnt & 0x0f) <<4;
  (pToData)[162] |= ((pFrom)->m_uReplicantCnt >> 4) &0xFF;

  /* Pack Member: m_uReplicantBit */
  (pToData)[161] |= ((pFrom)->m_uReplicantBit & 0x01) <<3;

  /* Pack Member: m_uResv3 */
  (pToData)[161] |= ((pFrom)->m_uResv3 & 0x01) <<2;

  /* Pack Member: m_uDrop */
  (pToData)[161] |= ((pFrom)->m_uDrop & 0x01) <<1;

  /* Pack Member: m_uEnqueue */
  (pToData)[161] |= ((pFrom)->m_uEnqueue & 0x01);

  /* Pack Member: m_uDqueue */
  (pToData)[160] |= ((pFrom)->m_uDqueue) & 0xFF;

  /* Pack Member: m_uResv4 */
  (pToData)[157] |= ((pFrom)->m_uResv4 & 0x03) <<6;
  (pToData)[158] |= ((pFrom)->m_uResv4 >> 2) &0xFF;
  (pToData)[159] |= ((pFrom)->m_uResv4 >> 10) &0xFF;

  /* Pack Member: m_uFrameLength */
  (pToData)[156] |= ((pFrom)->m_uFrameLength) & 0xFF;
  (pToData)[157] |= ((pFrom)->m_uFrameLength >> 8) & 0x3f;

  /* Pack Member: m_uPacketDataByte0 */
  (pToData)[155] |= ((pFrom)->m_uPacketDataByte0) & 0xFF;

  /* Pack Member: m_uPacketDataByte1 */
  (pToData)[154] |= ((pFrom)->m_uPacketDataByte1) & 0xFF;

  /* Pack Member: m_uPacketDataByte2 */
  (pToData)[153] |= ((pFrom)->m_uPacketDataByte2) & 0xFF;

  /* Pack Member: m_uPacketDataByte3 */
  (pToData)[152] |= ((pFrom)->m_uPacketDataByte3) & 0xFF;

  /* Pack Member: m_uPacketDataByte4 */
  (pToData)[151] |= ((pFrom)->m_uPacketDataByte4) & 0xFF;

  /* Pack Member: m_uPacketDataByte5 */
  (pToData)[150] |= ((pFrom)->m_uPacketDataByte5) & 0xFF;

  /* Pack Member: m_uPacketDataByte6 */
  (pToData)[149] |= ((pFrom)->m_uPacketDataByte6) & 0xFF;

  /* Pack Member: m_uPacketDataByte7 */
  (pToData)[148] |= ((pFrom)->m_uPacketDataByte7) & 0xFF;

  /* Pack Member: m_uPacketDataByte8 */
  (pToData)[147] |= ((pFrom)->m_uPacketDataByte8) & 0xFF;

  /* Pack Member: m_uPacketDataByte9 */
  (pToData)[146] |= ((pFrom)->m_uPacketDataByte9) & 0xFF;

  /* Pack Member: m_uPacketDataByte10 */
  (pToData)[145] |= ((pFrom)->m_uPacketDataByte10) & 0xFF;

  /* Pack Member: m_uPacketDataByte11 */
  (pToData)[144] |= ((pFrom)->m_uPacketDataByte11) & 0xFF;

  /* Pack Member: m_uPacketDataByte12 */
  (pToData)[143] |= ((pFrom)->m_uPacketDataByte12) & 0xFF;

  /* Pack Member: m_uPacketDataByte13 */
  (pToData)[142] |= ((pFrom)->m_uPacketDataByte13) & 0xFF;

  /* Pack Member: m_uPacketDataByte14 */
  (pToData)[141] |= ((pFrom)->m_uPacketDataByte14) & 0xFF;

  /* Pack Member: m_uPacketDataByte15 */
  (pToData)[140] |= ((pFrom)->m_uPacketDataByte15) & 0xFF;

  /* Pack Member: m_uPacketDataByte16 */
  (pToData)[139] |= ((pFrom)->m_uPacketDataByte16) & 0xFF;

  /* Pack Member: m_uPacketDataByte17 */
  (pToData)[138] |= ((pFrom)->m_uPacketDataByte17) & 0xFF;

  /* Pack Member: m_uPacketDataByte18 */
  (pToData)[137] |= ((pFrom)->m_uPacketDataByte18) & 0xFF;

  /* Pack Member: m_uPacketDataByte19 */
  (pToData)[136] |= ((pFrom)->m_uPacketDataByte19) & 0xFF;

  /* Pack Member: m_uPacketDataByte20 */
  (pToData)[135] |= ((pFrom)->m_uPacketDataByte20) & 0xFF;

  /* Pack Member: m_uPacketDataByte21 */
  (pToData)[134] |= ((pFrom)->m_uPacketDataByte21) & 0xFF;

  /* Pack Member: m_uPacketDataByte22 */
  (pToData)[133] |= ((pFrom)->m_uPacketDataByte22) & 0xFF;

  /* Pack Member: m_uPacketDataByte23 */
  (pToData)[132] |= ((pFrom)->m_uPacketDataByte23) & 0xFF;

  /* Pack Member: m_uPacketDataByte24 */
  (pToData)[131] |= ((pFrom)->m_uPacketDataByte24) & 0xFF;

  /* Pack Member: m_uPacketDataByte25 */
  (pToData)[130] |= ((pFrom)->m_uPacketDataByte25) & 0xFF;

  /* Pack Member: m_uPacketDataByte26 */
  (pToData)[129] |= ((pFrom)->m_uPacketDataByte26) & 0xFF;

  /* Pack Member: m_uPacketDataByte27 */
  (pToData)[128] |= ((pFrom)->m_uPacketDataByte27) & 0xFF;

  /* Pack Member: m_uPacketDataByte28 */
  (pToData)[127] |= ((pFrom)->m_uPacketDataByte28) & 0xFF;

  /* Pack Member: m_uPacketDataByte29 */
  (pToData)[126] |= ((pFrom)->m_uPacketDataByte29) & 0xFF;

  /* Pack Member: m_uPacketDataByte30 */
  (pToData)[125] |= ((pFrom)->m_uPacketDataByte30) & 0xFF;

  /* Pack Member: m_uPacketDataByte31 */
  (pToData)[124] |= ((pFrom)->m_uPacketDataByte31) & 0xFF;

  /* Pack Member: m_uPacketDataByte32 */
  (pToData)[123] |= ((pFrom)->m_uPacketDataByte32) & 0xFF;

  /* Pack Member: m_uPacketDataByte33 */
  (pToData)[122] |= ((pFrom)->m_uPacketDataByte33) & 0xFF;

  /* Pack Member: m_uPacketDataByte34 */
  (pToData)[121] |= ((pFrom)->m_uPacketDataByte34) & 0xFF;

  /* Pack Member: m_uPacketDataByte35 */
  (pToData)[120] |= ((pFrom)->m_uPacketDataByte35) & 0xFF;

  /* Pack Member: m_uPacketDataByte36 */
  (pToData)[119] |= ((pFrom)->m_uPacketDataByte36) & 0xFF;

  /* Pack Member: m_uPacketDataByte37 */
  (pToData)[118] |= ((pFrom)->m_uPacketDataByte37) & 0xFF;

  /* Pack Member: m_uPacketDataByte38 */
  (pToData)[117] |= ((pFrom)->m_uPacketDataByte38) & 0xFF;

  /* Pack Member: m_uPacketDataByte39 */
  (pToData)[116] |= ((pFrom)->m_uPacketDataByte39) & 0xFF;

  /* Pack Member: m_uPacketDataByte40 */
  (pToData)[115] |= ((pFrom)->m_uPacketDataByte40) & 0xFF;

  /* Pack Member: m_uPacketDataByte41 */
  (pToData)[114] |= ((pFrom)->m_uPacketDataByte41) & 0xFF;

  /* Pack Member: m_uPacketDataByte42 */
  (pToData)[113] |= ((pFrom)->m_uPacketDataByte42) & 0xFF;

  /* Pack Member: m_uPacketDataByte43 */
  (pToData)[112] |= ((pFrom)->m_uPacketDataByte43) & 0xFF;

  /* Pack Member: m_uPacketDataByte44 */
  (pToData)[111] |= ((pFrom)->m_uPacketDataByte44) & 0xFF;

  /* Pack Member: m_uPacketDataByte45 */
  (pToData)[110] |= ((pFrom)->m_uPacketDataByte45) & 0xFF;

  /* Pack Member: m_uPacketDataByte46 */
  (pToData)[109] |= ((pFrom)->m_uPacketDataByte46) & 0xFF;

  /* Pack Member: m_uPacketDataByte47 */
  (pToData)[108] |= ((pFrom)->m_uPacketDataByte47) & 0xFF;

  /* Pack Member: m_uPacketDataByte48 */
  (pToData)[107] |= ((pFrom)->m_uPacketDataByte48) & 0xFF;

  /* Pack Member: m_uPacketDataByte49 */
  (pToData)[106] |= ((pFrom)->m_uPacketDataByte49) & 0xFF;

  /* Pack Member: m_uPacketDataByte50 */
  (pToData)[105] |= ((pFrom)->m_uPacketDataByte50) & 0xFF;

  /* Pack Member: m_uPacketDataByte51 */
  (pToData)[104] |= ((pFrom)->m_uPacketDataByte51) & 0xFF;

  /* Pack Member: m_uPacketDataByte52 */
  (pToData)[103] |= ((pFrom)->m_uPacketDataByte52) & 0xFF;

  /* Pack Member: m_uPacketDataByte53 */
  (pToData)[102] |= ((pFrom)->m_uPacketDataByte53) & 0xFF;

  /* Pack Member: m_uPacketDataByte54 */
  (pToData)[101] |= ((pFrom)->m_uPacketDataByte54) & 0xFF;

  /* Pack Member: m_uPacketDataByte55 */
  (pToData)[100] |= ((pFrom)->m_uPacketDataByte55) & 0xFF;

  /* Pack Member: m_uPacketDataByte56 */
  (pToData)[99] |= ((pFrom)->m_uPacketDataByte56) & 0xFF;

  /* Pack Member: m_uPacketDataByte57 */
  (pToData)[98] |= ((pFrom)->m_uPacketDataByte57) & 0xFF;

  /* Pack Member: m_uPacketDataByte58 */
  (pToData)[97] |= ((pFrom)->m_uPacketDataByte58) & 0xFF;

  /* Pack Member: m_uPacketDataByte59 */
  (pToData)[96] |= ((pFrom)->m_uPacketDataByte59) & 0xFF;

  /* Pack Member: m_uPacketDataByte60 */
  (pToData)[95] |= ((pFrom)->m_uPacketDataByte60) & 0xFF;

  /* Pack Member: m_uPacketDataByte61 */
  (pToData)[94] |= ((pFrom)->m_uPacketDataByte61) & 0xFF;

  /* Pack Member: m_uPacketDataByte62 */
  (pToData)[93] |= ((pFrom)->m_uPacketDataByte62) & 0xFF;

  /* Pack Member: m_uPacketDataByte63 */
  (pToData)[92] |= ((pFrom)->m_uPacketDataByte63) & 0xFF;

  /* Pack Member: m_uPacketDataByte64 */
  (pToData)[91] |= ((pFrom)->m_uPacketDataByte64) & 0xFF;

  /* Pack Member: m_uPacketDataByte65 */
  (pToData)[90] |= ((pFrom)->m_uPacketDataByte65) & 0xFF;

  /* Pack Member: m_uPacketDataByte66 */
  (pToData)[89] |= ((pFrom)->m_uPacketDataByte66) & 0xFF;

  /* Pack Member: m_uPacketDataByte67 */
  (pToData)[88] |= ((pFrom)->m_uPacketDataByte67) & 0xFF;

  /* Pack Member: m_uPacketDataByte68 */
  (pToData)[87] |= ((pFrom)->m_uPacketDataByte68) & 0xFF;

  /* Pack Member: m_uPacketDataByte69 */
  (pToData)[86] |= ((pFrom)->m_uPacketDataByte69) & 0xFF;

  /* Pack Member: m_uPacketDataByte70 */
  (pToData)[85] |= ((pFrom)->m_uPacketDataByte70) & 0xFF;

  /* Pack Member: m_uPacketDataByte71 */
  (pToData)[84] |= ((pFrom)->m_uPacketDataByte71) & 0xFF;

  /* Pack Member: m_uPacketDataByte72 */
  (pToData)[83] |= ((pFrom)->m_uPacketDataByte72) & 0xFF;

  /* Pack Member: m_uPacketDataByte73 */
  (pToData)[82] |= ((pFrom)->m_uPacketDataByte73) & 0xFF;

  /* Pack Member: m_uPacketDataByte74 */
  (pToData)[81] |= ((pFrom)->m_uPacketDataByte74) & 0xFF;

  /* Pack Member: m_uPacketDataByte75 */
  (pToData)[80] |= ((pFrom)->m_uPacketDataByte75) & 0xFF;

  /* Pack Member: m_uPacketDataByte76 */
  (pToData)[79] |= ((pFrom)->m_uPacketDataByte76) & 0xFF;

  /* Pack Member: m_uPacketDataByte77 */
  (pToData)[78] |= ((pFrom)->m_uPacketDataByte77) & 0xFF;

  /* Pack Member: m_uPacketDataByte78 */
  (pToData)[77] |= ((pFrom)->m_uPacketDataByte78) & 0xFF;

  /* Pack Member: m_uPacketDataByte79 */
  (pToData)[76] |= ((pFrom)->m_uPacketDataByte79) & 0xFF;

  /* Pack Member: m_uPacketDataByte80 */
  (pToData)[75] |= ((pFrom)->m_uPacketDataByte80) & 0xFF;

  /* Pack Member: m_uPacketDataByte81 */
  (pToData)[74] |= ((pFrom)->m_uPacketDataByte81) & 0xFF;

  /* Pack Member: m_uPacketDataByte82 */
  (pToData)[73] |= ((pFrom)->m_uPacketDataByte82) & 0xFF;

  /* Pack Member: m_uPacketDataByte83 */
  (pToData)[72] |= ((pFrom)->m_uPacketDataByte83) & 0xFF;

  /* Pack Member: m_uPacketDataByte84 */
  (pToData)[71] |= ((pFrom)->m_uPacketDataByte84) & 0xFF;

  /* Pack Member: m_uPacketDataByte85 */
  (pToData)[70] |= ((pFrom)->m_uPacketDataByte85) & 0xFF;

  /* Pack Member: m_uPacketDataByte86 */
  (pToData)[69] |= ((pFrom)->m_uPacketDataByte86) & 0xFF;

  /* Pack Member: m_uPacketDataByte87 */
  (pToData)[68] |= ((pFrom)->m_uPacketDataByte87) & 0xFF;

  /* Pack Member: m_uPacketDataByte88 */
  (pToData)[67] |= ((pFrom)->m_uPacketDataByte88) & 0xFF;

  /* Pack Member: m_uPacketDataByte89 */
  (pToData)[66] |= ((pFrom)->m_uPacketDataByte89) & 0xFF;

  /* Pack Member: m_uPacketDataByte90 */
  (pToData)[65] |= ((pFrom)->m_uPacketDataByte90) & 0xFF;

  /* Pack Member: m_uPacketDataByte91 */
  (pToData)[64] |= ((pFrom)->m_uPacketDataByte91) & 0xFF;

  /* Pack Member: m_uPacketDataByte92 */
  (pToData)[63] |= ((pFrom)->m_uPacketDataByte92) & 0xFF;

  /* Pack Member: m_uPacketDataByte93 */
  (pToData)[62] |= ((pFrom)->m_uPacketDataByte93) & 0xFF;

  /* Pack Member: m_uPacketDataByte94 */
  (pToData)[61] |= ((pFrom)->m_uPacketDataByte94) & 0xFF;

  /* Pack Member: m_uPacketDataByte95 */
  (pToData)[60] |= ((pFrom)->m_uPacketDataByte95) & 0xFF;

  /* Pack Member: m_uPacketDataByte96 */
  (pToData)[59] |= ((pFrom)->m_uPacketDataByte96) & 0xFF;

  /* Pack Member: m_uPacketDataByte97 */
  (pToData)[58] |= ((pFrom)->m_uPacketDataByte97) & 0xFF;

  /* Pack Member: m_uPacketDataByte98 */
  (pToData)[57] |= ((pFrom)->m_uPacketDataByte98) & 0xFF;

  /* Pack Member: m_uPacketDataByte99 */
  (pToData)[56] |= ((pFrom)->m_uPacketDataByte99) & 0xFF;

  /* Pack Member: m_uPacketDataByte100 */
  (pToData)[55] |= ((pFrom)->m_uPacketDataByte100) & 0xFF;

  /* Pack Member: m_uPacketDataByte101 */
  (pToData)[54] |= ((pFrom)->m_uPacketDataByte101) & 0xFF;

  /* Pack Member: m_uPacketDataByte102 */
  (pToData)[53] |= ((pFrom)->m_uPacketDataByte102) & 0xFF;

  /* Pack Member: m_uPacketDataByte103 */
  (pToData)[52] |= ((pFrom)->m_uPacketDataByte103) & 0xFF;

  /* Pack Member: m_uPacketDataByte104 */
  (pToData)[51] |= ((pFrom)->m_uPacketDataByte104) & 0xFF;

  /* Pack Member: m_uPacketDataByte105 */
  (pToData)[50] |= ((pFrom)->m_uPacketDataByte105) & 0xFF;

  /* Pack Member: m_uPacketDataByte106 */
  (pToData)[49] |= ((pFrom)->m_uPacketDataByte106) & 0xFF;

  /* Pack Member: m_uPacketDataByte107 */
  (pToData)[48] |= ((pFrom)->m_uPacketDataByte107) & 0xFF;

  /* Pack Member: m_uPacketDataByte108 */
  (pToData)[47] |= ((pFrom)->m_uPacketDataByte108) & 0xFF;

  /* Pack Member: m_uPacketDataByte109 */
  (pToData)[46] |= ((pFrom)->m_uPacketDataByte109) & 0xFF;

  /* Pack Member: m_uPacketDataByte110 */
  (pToData)[45] |= ((pFrom)->m_uPacketDataByte110) & 0xFF;

  /* Pack Member: m_uPacketDataByte111 */
  (pToData)[44] |= ((pFrom)->m_uPacketDataByte111) & 0xFF;

  /* Pack Member: m_uPacketDataByte112 */
  (pToData)[43] |= ((pFrom)->m_uPacketDataByte112) & 0xFF;

  /* Pack Member: m_uPacketDataByte113 */
  (pToData)[42] |= ((pFrom)->m_uPacketDataByte113) & 0xFF;

  /* Pack Member: m_uPacketDataByte114 */
  (pToData)[41] |= ((pFrom)->m_uPacketDataByte114) & 0xFF;

  /* Pack Member: m_uPacketDataByte115 */
  (pToData)[40] |= ((pFrom)->m_uPacketDataByte115) & 0xFF;

  /* Pack Member: m_uPacketDataByte116 */
  (pToData)[39] |= ((pFrom)->m_uPacketDataByte116) & 0xFF;

  /* Pack Member: m_uPacketDataByte117 */
  (pToData)[38] |= ((pFrom)->m_uPacketDataByte117) & 0xFF;

  /* Pack Member: m_uPacketDataByte118 */
  (pToData)[37] |= ((pFrom)->m_uPacketDataByte118) & 0xFF;

  /* Pack Member: m_uPacketDataByte119 */
  (pToData)[36] |= ((pFrom)->m_uPacketDataByte119) & 0xFF;

  /* Pack Member: m_uPacketDataByte120 */
  (pToData)[35] |= ((pFrom)->m_uPacketDataByte120) & 0xFF;

  /* Pack Member: m_uPacketDataByte121 */
  (pToData)[34] |= ((pFrom)->m_uPacketDataByte121) & 0xFF;

  /* Pack Member: m_uPacketDataByte122 */
  (pToData)[33] |= ((pFrom)->m_uPacketDataByte122) & 0xFF;

  /* Pack Member: m_uPacketDataByte123 */
  (pToData)[32] |= ((pFrom)->m_uPacketDataByte123) & 0xFF;

  /* Pack Member: m_uPacketDataByte124 */
  (pToData)[31] |= ((pFrom)->m_uPacketDataByte124) & 0xFF;

  /* Pack Member: m_uPacketDataByte125 */
  (pToData)[30] |= ((pFrom)->m_uPacketDataByte125) & 0xFF;

  /* Pack Member: m_uPacketDataByte126 */
  (pToData)[29] |= ((pFrom)->m_uPacketDataByte126) & 0xFF;

  /* Pack Member: m_uPacketDataByte127 */
  (pToData)[28] |= ((pFrom)->m_uPacketDataByte127) & 0xFF;

  /* Pack Member: m_uPacketDataByte128 */
  (pToData)[27] |= ((pFrom)->m_uPacketDataByte128) & 0xFF;

  /* Pack Member: m_uPacketDataByte129 */
  (pToData)[26] |= ((pFrom)->m_uPacketDataByte129) & 0xFF;

  /* Pack Member: m_uPacketDataByte130 */
  (pToData)[25] |= ((pFrom)->m_uPacketDataByte130) & 0xFF;

  /* Pack Member: m_uPacketDataByte131 */
  (pToData)[24] |= ((pFrom)->m_uPacketDataByte131) & 0xFF;

  /* Pack Member: m_uPacketDataByte132 */
  (pToData)[23] |= ((pFrom)->m_uPacketDataByte132) & 0xFF;

  /* Pack Member: m_uPacketDataByte133 */
  (pToData)[22] |= ((pFrom)->m_uPacketDataByte133) & 0xFF;

  /* Pack Member: m_uPacketDataByte134 */
  (pToData)[21] |= ((pFrom)->m_uPacketDataByte134) & 0xFF;

  /* Pack Member: m_uPacketDataByte135 */
  (pToData)[20] |= ((pFrom)->m_uPacketDataByte135) & 0xFF;

  /* Pack Member: m_uPacketDataByte136 */
  (pToData)[19] |= ((pFrom)->m_uPacketDataByte136) & 0xFF;

  /* Pack Member: m_uPacketDataByte137 */
  (pToData)[18] |= ((pFrom)->m_uPacketDataByte137) & 0xFF;

  /* Pack Member: m_uPacketDataByte138 */
  (pToData)[17] |= ((pFrom)->m_uPacketDataByte138) & 0xFF;

  /* Pack Member: m_uPacketDataByte139 */
  (pToData)[16] |= ((pFrom)->m_uPacketDataByte139) & 0xFF;

  /* Pack Member: m_uPacketDataByte140 */
  (pToData)[15] |= ((pFrom)->m_uPacketDataByte140) & 0xFF;

  /* Pack Member: m_uPacketDataByte141 */
  (pToData)[14] |= ((pFrom)->m_uPacketDataByte141) & 0xFF;

  /* Pack Member: m_uPacketDataByte142 */
  (pToData)[13] |= ((pFrom)->m_uPacketDataByte142) & 0xFF;

  /* Pack Member: m_uPacketDataByte143 */
  (pToData)[12] |= ((pFrom)->m_uPacketDataByte143) & 0xFF;

  /* Pack Member: m_uPacketDataByte144 */
  (pToData)[11] |= ((pFrom)->m_uPacketDataByte144) & 0xFF;

  /* Pack Member: m_uPacketDataByte145 */
  (pToData)[10] |= ((pFrom)->m_uPacketDataByte145) & 0xFF;

  /* Pack Member: m_uPacketDataByte146 */
  (pToData)[9] |= ((pFrom)->m_uPacketDataByte146) & 0xFF;

  /* Pack Member: m_uPacketDataByte147 */
  (pToData)[8] |= ((pFrom)->m_uPacketDataByte147) & 0xFF;

  /* Pack Member: m_uPacketDataByte148 */
  (pToData)[7] |= ((pFrom)->m_uPacketDataByte148) & 0xFF;

  /* Pack Member: m_uPacketDataByte149 */
  (pToData)[6] |= ((pFrom)->m_uPacketDataByte149) & 0xFF;

  /* Pack Member: m_uPacketDataByte150 */
  (pToData)[5] |= ((pFrom)->m_uPacketDataByte150) & 0xFF;

  /* Pack Member: m_uPacketDataByte151 */
  (pToData)[4] |= ((pFrom)->m_uPacketDataByte151) & 0xFF;

  /* Pack Member: m_uPacketDataByte152 */
  (pToData)[3] |= ((pFrom)->m_uPacketDataByte152) & 0xFF;

  /* Pack Member: m_uPacketDataByte153 */
  (pToData)[2] |= ((pFrom)->m_uPacketDataByte153) & 0xFF;

  /* Pack Member: m_uPacketDataByte154 */
  (pToData)[1] |= ((pFrom)->m_uPacketDataByte154) & 0xFF;

  /* Pack Member: m_uPacketDataByte155 */
  (pToData)[0] |= ((pFrom)->m_uPacketDataByte155) & 0xFF;

  return SB_ZF_ZFCASWSDESCRIPTOR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaSwsDescriptor_Unpack(sbZfCaSwsDescriptor_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uResv0 */
  (pToStruct)->m_uResv0 =  (uint32_t)  ((pFromData)[171] >> 4) & 0x0f;

  /* Unpack Member: m_uSourceBuffer */
  (pToStruct)->m_uSourceBuffer =  (uint32_t)  ((pFromData)[169] >> 6) & 0x03;
  (pToStruct)->m_uSourceBuffer |=  (uint32_t)  (pFromData)[170] << 2;
  (pToStruct)->m_uSourceBuffer |=  (uint32_t)  ((pFromData)[171] & 0x0f) << 10;

  /* Unpack Member: m_uResv1 */
  (pToStruct)->m_uResv1 =  (uint32_t)  (pFromData)[168] ;
  (pToStruct)->m_uResv1 |=  (uint32_t)  ((pFromData)[169] & 0x7f) << 8;

  /* Unpack Member: m_uStreamSelector */
  (pToStruct)->m_uStreamSelector =  (uint32_t)  ((pFromData)[167] >> 5) & 0x07;

  /* Unpack Member: m_uBufferNumber */
  (pToStruct)->m_uBufferNumber =  (uint32_t)  (pFromData)[166] ;
  (pToStruct)->m_uBufferNumber |=  (uint32_t)  ((pFromData)[167] & 0x1f) << 8;

  /* Unpack Member: m_uExceptionIndex */
  (pToStruct)->m_uExceptionIndex =  (uint32_t)  (pFromData)[165] ;

  /* Unpack Member: m_uSourceQueue */
  (pToStruct)->m_uSourceQueue =  (uint32_t)  (pFromData)[164] ;

  /* Unpack Member: m_uResv2 */
  (pToStruct)->m_uResv2 =  (uint32_t)  (pFromData)[163] ;

  /* Unpack Member: m_uReplicantCnt */
  (pToStruct)->m_uReplicantCnt =  (uint32_t)  ((pFromData)[161] >> 4) & 0x0f;
  (pToStruct)->m_uReplicantCnt |=  (uint32_t)  (pFromData)[162] << 4;

  /* Unpack Member: m_uReplicantBit */
  (pToStruct)->m_uReplicantBit =  (uint32_t)  ((pFromData)[161] >> 3) & 0x01;

  /* Unpack Member: m_uResv3 */
  (pToStruct)->m_uResv3 =  (uint32_t)  ((pFromData)[161] >> 2) & 0x01;

  /* Unpack Member: m_uDrop */
  (pToStruct)->m_uDrop =  (uint32_t)  ((pFromData)[161] >> 1) & 0x01;

  /* Unpack Member: m_uEnqueue */
  (pToStruct)->m_uEnqueue =  (uint32_t)  ((pFromData)[161] ) & 0x01;

  /* Unpack Member: m_uDqueue */
  (pToStruct)->m_uDqueue =  (uint32_t)  (pFromData)[160] ;

  /* Unpack Member: m_uResv4 */
  (pToStruct)->m_uResv4 =  (uint32_t)  ((pFromData)[157] >> 6) & 0x03;
  (pToStruct)->m_uResv4 |=  (uint32_t)  (pFromData)[158] << 2;
  (pToStruct)->m_uResv4 |=  (uint32_t)  (pFromData)[159] << 10;

  /* Unpack Member: m_uFrameLength */
  (pToStruct)->m_uFrameLength =  (uint32_t)  (pFromData)[156] ;
  (pToStruct)->m_uFrameLength |=  (uint32_t)  ((pFromData)[157] & 0x3f) << 8;

  /* Unpack Member: m_uPacketDataByte0 */
  (pToStruct)->m_uPacketDataByte0 =  (uint8_t)  (pFromData)[155] ;

  /* Unpack Member: m_uPacketDataByte1 */
  (pToStruct)->m_uPacketDataByte1 =  (uint8_t)  (pFromData)[154] ;

  /* Unpack Member: m_uPacketDataByte2 */
  (pToStruct)->m_uPacketDataByte2 =  (uint8_t)  (pFromData)[153] ;

  /* Unpack Member: m_uPacketDataByte3 */
  (pToStruct)->m_uPacketDataByte3 =  (uint8_t)  (pFromData)[152] ;

  /* Unpack Member: m_uPacketDataByte4 */
  (pToStruct)->m_uPacketDataByte4 =  (uint8_t)  (pFromData)[151] ;

  /* Unpack Member: m_uPacketDataByte5 */
  (pToStruct)->m_uPacketDataByte5 =  (uint8_t)  (pFromData)[150] ;

  /* Unpack Member: m_uPacketDataByte6 */
  (pToStruct)->m_uPacketDataByte6 =  (uint8_t)  (pFromData)[149] ;

  /* Unpack Member: m_uPacketDataByte7 */
  (pToStruct)->m_uPacketDataByte7 =  (uint8_t)  (pFromData)[148] ;

  /* Unpack Member: m_uPacketDataByte8 */
  (pToStruct)->m_uPacketDataByte8 =  (uint8_t)  (pFromData)[147] ;

  /* Unpack Member: m_uPacketDataByte9 */
  (pToStruct)->m_uPacketDataByte9 =  (uint8_t)  (pFromData)[146] ;

  /* Unpack Member: m_uPacketDataByte10 */
  (pToStruct)->m_uPacketDataByte10 =  (uint8_t)  (pFromData)[145] ;

  /* Unpack Member: m_uPacketDataByte11 */
  (pToStruct)->m_uPacketDataByte11 =  (uint8_t)  (pFromData)[144] ;

  /* Unpack Member: m_uPacketDataByte12 */
  (pToStruct)->m_uPacketDataByte12 =  (uint8_t)  (pFromData)[143] ;

  /* Unpack Member: m_uPacketDataByte13 */
  (pToStruct)->m_uPacketDataByte13 =  (uint8_t)  (pFromData)[142] ;

  /* Unpack Member: m_uPacketDataByte14 */
  (pToStruct)->m_uPacketDataByte14 =  (uint8_t)  (pFromData)[141] ;

  /* Unpack Member: m_uPacketDataByte15 */
  (pToStruct)->m_uPacketDataByte15 =  (uint8_t)  (pFromData)[140] ;

  /* Unpack Member: m_uPacketDataByte16 */
  (pToStruct)->m_uPacketDataByte16 =  (uint8_t)  (pFromData)[139] ;

  /* Unpack Member: m_uPacketDataByte17 */
  (pToStruct)->m_uPacketDataByte17 =  (uint8_t)  (pFromData)[138] ;

  /* Unpack Member: m_uPacketDataByte18 */
  (pToStruct)->m_uPacketDataByte18 =  (uint8_t)  (pFromData)[137] ;

  /* Unpack Member: m_uPacketDataByte19 */
  (pToStruct)->m_uPacketDataByte19 =  (uint8_t)  (pFromData)[136] ;

  /* Unpack Member: m_uPacketDataByte20 */
  (pToStruct)->m_uPacketDataByte20 =  (uint8_t)  (pFromData)[135] ;

  /* Unpack Member: m_uPacketDataByte21 */
  (pToStruct)->m_uPacketDataByte21 =  (uint8_t)  (pFromData)[134] ;

  /* Unpack Member: m_uPacketDataByte22 */
  (pToStruct)->m_uPacketDataByte22 =  (uint8_t)  (pFromData)[133] ;

  /* Unpack Member: m_uPacketDataByte23 */
  (pToStruct)->m_uPacketDataByte23 =  (uint8_t)  (pFromData)[132] ;

  /* Unpack Member: m_uPacketDataByte24 */
  (pToStruct)->m_uPacketDataByte24 =  (uint8_t)  (pFromData)[131] ;

  /* Unpack Member: m_uPacketDataByte25 */
  (pToStruct)->m_uPacketDataByte25 =  (uint8_t)  (pFromData)[130] ;

  /* Unpack Member: m_uPacketDataByte26 */
  (pToStruct)->m_uPacketDataByte26 =  (uint8_t)  (pFromData)[129] ;

  /* Unpack Member: m_uPacketDataByte27 */
  (pToStruct)->m_uPacketDataByte27 =  (uint8_t)  (pFromData)[128] ;

  /* Unpack Member: m_uPacketDataByte28 */
  (pToStruct)->m_uPacketDataByte28 =  (uint8_t)  (pFromData)[127] ;

  /* Unpack Member: m_uPacketDataByte29 */
  (pToStruct)->m_uPacketDataByte29 =  (uint8_t)  (pFromData)[126] ;

  /* Unpack Member: m_uPacketDataByte30 */
  (pToStruct)->m_uPacketDataByte30 =  (uint8_t)  (pFromData)[125] ;

  /* Unpack Member: m_uPacketDataByte31 */
  (pToStruct)->m_uPacketDataByte31 =  (uint8_t)  (pFromData)[124] ;

  /* Unpack Member: m_uPacketDataByte32 */
  (pToStruct)->m_uPacketDataByte32 =  (uint8_t)  (pFromData)[123] ;

  /* Unpack Member: m_uPacketDataByte33 */
  (pToStruct)->m_uPacketDataByte33 =  (uint8_t)  (pFromData)[122] ;

  /* Unpack Member: m_uPacketDataByte34 */
  (pToStruct)->m_uPacketDataByte34 =  (uint8_t)  (pFromData)[121] ;

  /* Unpack Member: m_uPacketDataByte35 */
  (pToStruct)->m_uPacketDataByte35 =  (uint8_t)  (pFromData)[120] ;

  /* Unpack Member: m_uPacketDataByte36 */
  (pToStruct)->m_uPacketDataByte36 =  (uint8_t)  (pFromData)[119] ;

  /* Unpack Member: m_uPacketDataByte37 */
  (pToStruct)->m_uPacketDataByte37 =  (uint8_t)  (pFromData)[118] ;

  /* Unpack Member: m_uPacketDataByte38 */
  (pToStruct)->m_uPacketDataByte38 =  (uint8_t)  (pFromData)[117] ;

  /* Unpack Member: m_uPacketDataByte39 */
  (pToStruct)->m_uPacketDataByte39 =  (uint8_t)  (pFromData)[116] ;

  /* Unpack Member: m_uPacketDataByte40 */
  (pToStruct)->m_uPacketDataByte40 =  (uint8_t)  (pFromData)[115] ;

  /* Unpack Member: m_uPacketDataByte41 */
  (pToStruct)->m_uPacketDataByte41 =  (uint8_t)  (pFromData)[114] ;

  /* Unpack Member: m_uPacketDataByte42 */
  (pToStruct)->m_uPacketDataByte42 =  (uint8_t)  (pFromData)[113] ;

  /* Unpack Member: m_uPacketDataByte43 */
  (pToStruct)->m_uPacketDataByte43 =  (uint8_t)  (pFromData)[112] ;

  /* Unpack Member: m_uPacketDataByte44 */
  (pToStruct)->m_uPacketDataByte44 =  (uint8_t)  (pFromData)[111] ;

  /* Unpack Member: m_uPacketDataByte45 */
  (pToStruct)->m_uPacketDataByte45 =  (uint8_t)  (pFromData)[110] ;

  /* Unpack Member: m_uPacketDataByte46 */
  (pToStruct)->m_uPacketDataByte46 =  (uint8_t)  (pFromData)[109] ;

  /* Unpack Member: m_uPacketDataByte47 */
  (pToStruct)->m_uPacketDataByte47 =  (uint8_t)  (pFromData)[108] ;

  /* Unpack Member: m_uPacketDataByte48 */
  (pToStruct)->m_uPacketDataByte48 =  (uint8_t)  (pFromData)[107] ;

  /* Unpack Member: m_uPacketDataByte49 */
  (pToStruct)->m_uPacketDataByte49 =  (uint8_t)  (pFromData)[106] ;

  /* Unpack Member: m_uPacketDataByte50 */
  (pToStruct)->m_uPacketDataByte50 =  (uint8_t)  (pFromData)[105] ;

  /* Unpack Member: m_uPacketDataByte51 */
  (pToStruct)->m_uPacketDataByte51 =  (uint8_t)  (pFromData)[104] ;

  /* Unpack Member: m_uPacketDataByte52 */
  (pToStruct)->m_uPacketDataByte52 =  (uint8_t)  (pFromData)[103] ;

  /* Unpack Member: m_uPacketDataByte53 */
  (pToStruct)->m_uPacketDataByte53 =  (uint8_t)  (pFromData)[102] ;

  /* Unpack Member: m_uPacketDataByte54 */
  (pToStruct)->m_uPacketDataByte54 =  (uint8_t)  (pFromData)[101] ;

  /* Unpack Member: m_uPacketDataByte55 */
  (pToStruct)->m_uPacketDataByte55 =  (uint8_t)  (pFromData)[100] ;

  /* Unpack Member: m_uPacketDataByte56 */
  (pToStruct)->m_uPacketDataByte56 =  (uint8_t)  (pFromData)[99] ;

  /* Unpack Member: m_uPacketDataByte57 */
  (pToStruct)->m_uPacketDataByte57 =  (uint8_t)  (pFromData)[98] ;

  /* Unpack Member: m_uPacketDataByte58 */
  (pToStruct)->m_uPacketDataByte58 =  (uint8_t)  (pFromData)[97] ;

  /* Unpack Member: m_uPacketDataByte59 */
  (pToStruct)->m_uPacketDataByte59 =  (uint8_t)  (pFromData)[96] ;

  /* Unpack Member: m_uPacketDataByte60 */
  (pToStruct)->m_uPacketDataByte60 =  (uint8_t)  (pFromData)[95] ;

  /* Unpack Member: m_uPacketDataByte61 */
  (pToStruct)->m_uPacketDataByte61 =  (uint8_t)  (pFromData)[94] ;

  /* Unpack Member: m_uPacketDataByte62 */
  (pToStruct)->m_uPacketDataByte62 =  (uint8_t)  (pFromData)[93] ;

  /* Unpack Member: m_uPacketDataByte63 */
  (pToStruct)->m_uPacketDataByte63 =  (uint8_t)  (pFromData)[92] ;

  /* Unpack Member: m_uPacketDataByte64 */
  (pToStruct)->m_uPacketDataByte64 =  (uint8_t)  (pFromData)[91] ;

  /* Unpack Member: m_uPacketDataByte65 */
  (pToStruct)->m_uPacketDataByte65 =  (uint8_t)  (pFromData)[90] ;

  /* Unpack Member: m_uPacketDataByte66 */
  (pToStruct)->m_uPacketDataByte66 =  (uint8_t)  (pFromData)[89] ;

  /* Unpack Member: m_uPacketDataByte67 */
  (pToStruct)->m_uPacketDataByte67 =  (uint8_t)  (pFromData)[88] ;

  /* Unpack Member: m_uPacketDataByte68 */
  (pToStruct)->m_uPacketDataByte68 =  (uint8_t)  (pFromData)[87] ;

  /* Unpack Member: m_uPacketDataByte69 */
  (pToStruct)->m_uPacketDataByte69 =  (uint8_t)  (pFromData)[86] ;

  /* Unpack Member: m_uPacketDataByte70 */
  (pToStruct)->m_uPacketDataByte70 =  (uint8_t)  (pFromData)[85] ;

  /* Unpack Member: m_uPacketDataByte71 */
  (pToStruct)->m_uPacketDataByte71 =  (uint8_t)  (pFromData)[84] ;

  /* Unpack Member: m_uPacketDataByte72 */
  (pToStruct)->m_uPacketDataByte72 =  (uint8_t)  (pFromData)[83] ;

  /* Unpack Member: m_uPacketDataByte73 */
  (pToStruct)->m_uPacketDataByte73 =  (uint8_t)  (pFromData)[82] ;

  /* Unpack Member: m_uPacketDataByte74 */
  (pToStruct)->m_uPacketDataByte74 =  (uint8_t)  (pFromData)[81] ;

  /* Unpack Member: m_uPacketDataByte75 */
  (pToStruct)->m_uPacketDataByte75 =  (uint8_t)  (pFromData)[80] ;

  /* Unpack Member: m_uPacketDataByte76 */
  (pToStruct)->m_uPacketDataByte76 =  (uint8_t)  (pFromData)[79] ;

  /* Unpack Member: m_uPacketDataByte77 */
  (pToStruct)->m_uPacketDataByte77 =  (uint8_t)  (pFromData)[78] ;

  /* Unpack Member: m_uPacketDataByte78 */
  (pToStruct)->m_uPacketDataByte78 =  (uint8_t)  (pFromData)[77] ;

  /* Unpack Member: m_uPacketDataByte79 */
  (pToStruct)->m_uPacketDataByte79 =  (uint8_t)  (pFromData)[76] ;

  /* Unpack Member: m_uPacketDataByte80 */
  (pToStruct)->m_uPacketDataByte80 =  (uint8_t)  (pFromData)[75] ;

  /* Unpack Member: m_uPacketDataByte81 */
  (pToStruct)->m_uPacketDataByte81 =  (uint8_t)  (pFromData)[74] ;

  /* Unpack Member: m_uPacketDataByte82 */
  (pToStruct)->m_uPacketDataByte82 =  (uint8_t)  (pFromData)[73] ;

  /* Unpack Member: m_uPacketDataByte83 */
  (pToStruct)->m_uPacketDataByte83 =  (uint8_t)  (pFromData)[72] ;

  /* Unpack Member: m_uPacketDataByte84 */
  (pToStruct)->m_uPacketDataByte84 =  (uint8_t)  (pFromData)[71] ;

  /* Unpack Member: m_uPacketDataByte85 */
  (pToStruct)->m_uPacketDataByte85 =  (uint8_t)  (pFromData)[70] ;

  /* Unpack Member: m_uPacketDataByte86 */
  (pToStruct)->m_uPacketDataByte86 =  (uint8_t)  (pFromData)[69] ;

  /* Unpack Member: m_uPacketDataByte87 */
  (pToStruct)->m_uPacketDataByte87 =  (uint8_t)  (pFromData)[68] ;

  /* Unpack Member: m_uPacketDataByte88 */
  (pToStruct)->m_uPacketDataByte88 =  (uint8_t)  (pFromData)[67] ;

  /* Unpack Member: m_uPacketDataByte89 */
  (pToStruct)->m_uPacketDataByte89 =  (uint8_t)  (pFromData)[66] ;

  /* Unpack Member: m_uPacketDataByte90 */
  (pToStruct)->m_uPacketDataByte90 =  (uint8_t)  (pFromData)[65] ;

  /* Unpack Member: m_uPacketDataByte91 */
  (pToStruct)->m_uPacketDataByte91 =  (uint8_t)  (pFromData)[64] ;

  /* Unpack Member: m_uPacketDataByte92 */
  (pToStruct)->m_uPacketDataByte92 =  (uint8_t)  (pFromData)[63] ;

  /* Unpack Member: m_uPacketDataByte93 */
  (pToStruct)->m_uPacketDataByte93 =  (uint8_t)  (pFromData)[62] ;

  /* Unpack Member: m_uPacketDataByte94 */
  (pToStruct)->m_uPacketDataByte94 =  (uint8_t)  (pFromData)[61] ;

  /* Unpack Member: m_uPacketDataByte95 */
  (pToStruct)->m_uPacketDataByte95 =  (uint8_t)  (pFromData)[60] ;

  /* Unpack Member: m_uPacketDataByte96 */
  (pToStruct)->m_uPacketDataByte96 =  (uint8_t)  (pFromData)[59] ;

  /* Unpack Member: m_uPacketDataByte97 */
  (pToStruct)->m_uPacketDataByte97 =  (uint8_t)  (pFromData)[58] ;

  /* Unpack Member: m_uPacketDataByte98 */
  (pToStruct)->m_uPacketDataByte98 =  (uint8_t)  (pFromData)[57] ;

  /* Unpack Member: m_uPacketDataByte99 */
  (pToStruct)->m_uPacketDataByte99 =  (uint8_t)  (pFromData)[56] ;

  /* Unpack Member: m_uPacketDataByte100 */
  (pToStruct)->m_uPacketDataByte100 =  (uint8_t)  (pFromData)[55] ;

  /* Unpack Member: m_uPacketDataByte101 */
  (pToStruct)->m_uPacketDataByte101 =  (uint8_t)  (pFromData)[54] ;

  /* Unpack Member: m_uPacketDataByte102 */
  (pToStruct)->m_uPacketDataByte102 =  (uint8_t)  (pFromData)[53] ;

  /* Unpack Member: m_uPacketDataByte103 */
  (pToStruct)->m_uPacketDataByte103 =  (uint8_t)  (pFromData)[52] ;

  /* Unpack Member: m_uPacketDataByte104 */
  (pToStruct)->m_uPacketDataByte104 =  (uint8_t)  (pFromData)[51] ;

  /* Unpack Member: m_uPacketDataByte105 */
  (pToStruct)->m_uPacketDataByte105 =  (uint8_t)  (pFromData)[50] ;

  /* Unpack Member: m_uPacketDataByte106 */
  (pToStruct)->m_uPacketDataByte106 =  (uint8_t)  (pFromData)[49] ;

  /* Unpack Member: m_uPacketDataByte107 */
  (pToStruct)->m_uPacketDataByte107 =  (uint8_t)  (pFromData)[48] ;

  /* Unpack Member: m_uPacketDataByte108 */
  (pToStruct)->m_uPacketDataByte108 =  (uint8_t)  (pFromData)[47] ;

  /* Unpack Member: m_uPacketDataByte109 */
  (pToStruct)->m_uPacketDataByte109 =  (uint8_t)  (pFromData)[46] ;

  /* Unpack Member: m_uPacketDataByte110 */
  (pToStruct)->m_uPacketDataByte110 =  (uint8_t)  (pFromData)[45] ;

  /* Unpack Member: m_uPacketDataByte111 */
  (pToStruct)->m_uPacketDataByte111 =  (uint8_t)  (pFromData)[44] ;

  /* Unpack Member: m_uPacketDataByte112 */
  (pToStruct)->m_uPacketDataByte112 =  (uint8_t)  (pFromData)[43] ;

  /* Unpack Member: m_uPacketDataByte113 */
  (pToStruct)->m_uPacketDataByte113 =  (uint8_t)  (pFromData)[42] ;

  /* Unpack Member: m_uPacketDataByte114 */
  (pToStruct)->m_uPacketDataByte114 =  (uint8_t)  (pFromData)[41] ;

  /* Unpack Member: m_uPacketDataByte115 */
  (pToStruct)->m_uPacketDataByte115 =  (uint8_t)  (pFromData)[40] ;

  /* Unpack Member: m_uPacketDataByte116 */
  (pToStruct)->m_uPacketDataByte116 =  (uint8_t)  (pFromData)[39] ;

  /* Unpack Member: m_uPacketDataByte117 */
  (pToStruct)->m_uPacketDataByte117 =  (uint8_t)  (pFromData)[38] ;

  /* Unpack Member: m_uPacketDataByte118 */
  (pToStruct)->m_uPacketDataByte118 =  (uint8_t)  (pFromData)[37] ;

  /* Unpack Member: m_uPacketDataByte119 */
  (pToStruct)->m_uPacketDataByte119 =  (uint8_t)  (pFromData)[36] ;

  /* Unpack Member: m_uPacketDataByte120 */
  (pToStruct)->m_uPacketDataByte120 =  (uint8_t)  (pFromData)[35] ;

  /* Unpack Member: m_uPacketDataByte121 */
  (pToStruct)->m_uPacketDataByte121 =  (uint8_t)  (pFromData)[34] ;

  /* Unpack Member: m_uPacketDataByte122 */
  (pToStruct)->m_uPacketDataByte122 =  (uint8_t)  (pFromData)[33] ;

  /* Unpack Member: m_uPacketDataByte123 */
  (pToStruct)->m_uPacketDataByte123 =  (uint8_t)  (pFromData)[32] ;

  /* Unpack Member: m_uPacketDataByte124 */
  (pToStruct)->m_uPacketDataByte124 =  (uint8_t)  (pFromData)[31] ;

  /* Unpack Member: m_uPacketDataByte125 */
  (pToStruct)->m_uPacketDataByte125 =  (uint8_t)  (pFromData)[30] ;

  /* Unpack Member: m_uPacketDataByte126 */
  (pToStruct)->m_uPacketDataByte126 =  (uint8_t)  (pFromData)[29] ;

  /* Unpack Member: m_uPacketDataByte127 */
  (pToStruct)->m_uPacketDataByte127 =  (uint8_t)  (pFromData)[28] ;

  /* Unpack Member: m_uPacketDataByte128 */
  (pToStruct)->m_uPacketDataByte128 =  (uint8_t)  (pFromData)[27] ;

  /* Unpack Member: m_uPacketDataByte129 */
  (pToStruct)->m_uPacketDataByte129 =  (uint8_t)  (pFromData)[26] ;

  /* Unpack Member: m_uPacketDataByte130 */
  (pToStruct)->m_uPacketDataByte130 =  (uint8_t)  (pFromData)[25] ;

  /* Unpack Member: m_uPacketDataByte131 */
  (pToStruct)->m_uPacketDataByte131 =  (uint8_t)  (pFromData)[24] ;

  /* Unpack Member: m_uPacketDataByte132 */
  (pToStruct)->m_uPacketDataByte132 =  (uint8_t)  (pFromData)[23] ;

  /* Unpack Member: m_uPacketDataByte133 */
  (pToStruct)->m_uPacketDataByte133 =  (uint8_t)  (pFromData)[22] ;

  /* Unpack Member: m_uPacketDataByte134 */
  (pToStruct)->m_uPacketDataByte134 =  (uint8_t)  (pFromData)[21] ;

  /* Unpack Member: m_uPacketDataByte135 */
  (pToStruct)->m_uPacketDataByte135 =  (uint8_t)  (pFromData)[20] ;

  /* Unpack Member: m_uPacketDataByte136 */
  (pToStruct)->m_uPacketDataByte136 =  (uint8_t)  (pFromData)[19] ;

  /* Unpack Member: m_uPacketDataByte137 */
  (pToStruct)->m_uPacketDataByte137 =  (uint8_t)  (pFromData)[18] ;

  /* Unpack Member: m_uPacketDataByte138 */
  (pToStruct)->m_uPacketDataByte138 =  (uint8_t)  (pFromData)[17] ;

  /* Unpack Member: m_uPacketDataByte139 */
  (pToStruct)->m_uPacketDataByte139 =  (uint8_t)  (pFromData)[16] ;

  /* Unpack Member: m_uPacketDataByte140 */
  (pToStruct)->m_uPacketDataByte140 =  (uint8_t)  (pFromData)[15] ;

  /* Unpack Member: m_uPacketDataByte141 */
  (pToStruct)->m_uPacketDataByte141 =  (uint8_t)  (pFromData)[14] ;

  /* Unpack Member: m_uPacketDataByte142 */
  (pToStruct)->m_uPacketDataByte142 =  (uint8_t)  (pFromData)[13] ;

  /* Unpack Member: m_uPacketDataByte143 */
  (pToStruct)->m_uPacketDataByte143 =  (uint8_t)  (pFromData)[12] ;

  /* Unpack Member: m_uPacketDataByte144 */
  (pToStruct)->m_uPacketDataByte144 =  (uint8_t)  (pFromData)[11] ;

  /* Unpack Member: m_uPacketDataByte145 */
  (pToStruct)->m_uPacketDataByte145 =  (uint8_t)  (pFromData)[10] ;

  /* Unpack Member: m_uPacketDataByte146 */
  (pToStruct)->m_uPacketDataByte146 =  (uint8_t)  (pFromData)[9] ;

  /* Unpack Member: m_uPacketDataByte147 */
  (pToStruct)->m_uPacketDataByte147 =  (uint8_t)  (pFromData)[8] ;

  /* Unpack Member: m_uPacketDataByte148 */
  (pToStruct)->m_uPacketDataByte148 =  (uint8_t)  (pFromData)[7] ;

  /* Unpack Member: m_uPacketDataByte149 */
  (pToStruct)->m_uPacketDataByte149 =  (uint8_t)  (pFromData)[6] ;

  /* Unpack Member: m_uPacketDataByte150 */
  (pToStruct)->m_uPacketDataByte150 =  (uint8_t)  (pFromData)[5] ;

  /* Unpack Member: m_uPacketDataByte151 */
  (pToStruct)->m_uPacketDataByte151 =  (uint8_t)  (pFromData)[4] ;

  /* Unpack Member: m_uPacketDataByte152 */
  (pToStruct)->m_uPacketDataByte152 =  (uint8_t)  (pFromData)[3] ;

  /* Unpack Member: m_uPacketDataByte153 */
  (pToStruct)->m_uPacketDataByte153 =  (uint8_t)  (pFromData)[2] ;

  /* Unpack Member: m_uPacketDataByte154 */
  (pToStruct)->m_uPacketDataByte154 =  (uint8_t)  (pFromData)[1] ;

  /* Unpack Member: m_uPacketDataByte155 */
  (pToStruct)->m_uPacketDataByte155 =  (uint8_t)  (pFromData)[0] ;

}



/* initialize an instance of this zframe */
void
sbZfCaSwsDescriptor_InitInstance(sbZfCaSwsDescriptor_t *pFrame) {

  pFrame->m_uResv0 =  (unsigned int)  0;
  pFrame->m_uSourceBuffer =  (unsigned int)  0;
  pFrame->m_uResv1 =  (unsigned int)  0;
  pFrame->m_uStreamSelector =  (unsigned int)  0;
  pFrame->m_uBufferNumber =  (unsigned int)  0;
  pFrame->m_uExceptionIndex =  (unsigned int)  0;
  pFrame->m_uSourceQueue =  (unsigned int)  0;
  pFrame->m_uResv2 =  (unsigned int)  0;
  pFrame->m_uReplicantCnt =  (unsigned int)  0;
  pFrame->m_uReplicantBit =  (unsigned int)  0;
  pFrame->m_uResv3 =  (unsigned int)  0;
  pFrame->m_uDrop =  (unsigned int)  0;
  pFrame->m_uEnqueue =  (unsigned int)  0;
  pFrame->m_uDqueue =  (unsigned int)  0;
  pFrame->m_uResv4 =  (unsigned int)  0;
  pFrame->m_uFrameLength =  (unsigned int)  0;
  pFrame->m_uPacketDataByte0 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte1 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte2 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte3 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte4 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte5 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte6 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte7 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte8 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte9 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte10 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte11 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte12 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte13 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte14 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte15 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte16 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte17 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte18 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte19 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte20 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte21 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte22 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte23 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte24 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte25 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte26 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte27 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte28 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte29 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte30 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte31 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte32 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte33 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte34 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte35 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte36 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte37 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte38 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte39 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte40 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte41 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte42 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte43 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte44 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte45 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte46 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte47 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte48 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte49 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte50 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte51 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte52 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte53 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte54 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte55 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte56 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte57 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte58 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte59 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte60 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte61 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte62 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte63 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte64 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte65 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte66 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte67 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte68 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte69 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte70 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte71 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte72 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte73 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte74 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte75 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte76 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte77 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte78 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte79 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte80 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte81 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte82 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte83 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte84 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte85 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte86 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte87 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte88 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte89 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte90 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte91 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte92 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte93 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte94 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte95 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte96 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte97 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte98 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte99 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte100 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte101 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte102 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte103 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte104 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte105 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte106 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte107 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte108 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte109 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte110 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte111 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte112 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte113 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte114 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte115 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte116 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte117 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte118 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte119 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte120 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte121 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte122 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte123 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte124 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte125 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte126 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte127 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte128 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte129 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte130 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte131 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte132 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte133 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte134 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte135 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte136 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte137 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte138 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte139 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte140 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte141 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte142 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte143 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte144 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte145 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte146 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte147 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte148 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte149 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte150 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte151 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte152 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte153 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte154 =  (unsigned int)  0;
  pFrame->m_uPacketDataByte155 =  (unsigned int)  0;

}
