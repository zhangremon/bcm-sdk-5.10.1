/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaRcDataEntry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CARCDATAENTRY_H
#define SB_ZF_CARCDATAENTRY_H

#define SB_ZF_CARCDATAENTRY_SIZE_IN_BYTES 144
#define SB_ZF_CARCDATAENTRY_SIZE 144
#define SB_ZF_CARCDATAENTRY_M_UDATA35_BITS "1151:1120"
#define SB_ZF_CARCDATAENTRY_M_UDATA34_BITS "1119:1088"
#define SB_ZF_CARCDATAENTRY_M_UDATA33_BITS "1087:1056"
#define SB_ZF_CARCDATAENTRY_M_UDATA32_BITS "1055:1024"
#define SB_ZF_CARCDATAENTRY_M_UDATA31_BITS "1023:992"
#define SB_ZF_CARCDATAENTRY_M_UDATA30_BITS "991:960"
#define SB_ZF_CARCDATAENTRY_M_UDATA29_BITS "959:928"
#define SB_ZF_CARCDATAENTRY_M_UDATA28_BITS "927:896"
#define SB_ZF_CARCDATAENTRY_M_UDATA27_BITS "895:864"
#define SB_ZF_CARCDATAENTRY_M_UDATA26_BITS "863:832"
#define SB_ZF_CARCDATAENTRY_M_UDATA25_BITS "831:800"
#define SB_ZF_CARCDATAENTRY_M_UDATA24_BITS "799:768"
#define SB_ZF_CARCDATAENTRY_M_UDATA23_BITS "767:736"
#define SB_ZF_CARCDATAENTRY_M_UDATA22_BITS "735:704"
#define SB_ZF_CARCDATAENTRY_M_UDATA21_BITS "703:672"
#define SB_ZF_CARCDATAENTRY_M_UDATA20_BITS "671:640"
#define SB_ZF_CARCDATAENTRY_M_UDATA19_BITS "639:608"
#define SB_ZF_CARCDATAENTRY_M_UDATA18_BITS "607:576"
#define SB_ZF_CARCDATAENTRY_M_UDATA17_BITS "575:544"
#define SB_ZF_CARCDATAENTRY_M_UDATA16_BITS "543:512"
#define SB_ZF_CARCDATAENTRY_M_UDATA15_BITS "511:480"
#define SB_ZF_CARCDATAENTRY_M_UDATA14_BITS "479:448"
#define SB_ZF_CARCDATAENTRY_M_UDATA13_BITS "447:416"
#define SB_ZF_CARCDATAENTRY_M_UDATA12_BITS "415:384"
#define SB_ZF_CARCDATAENTRY_M_UDATA11_BITS "383:352"
#define SB_ZF_CARCDATAENTRY_M_UDATA10_BITS "351:320"
#define SB_ZF_CARCDATAENTRY_M_UDATA9_BITS "319:288"
#define SB_ZF_CARCDATAENTRY_M_UDATA8_BITS "287:256"
#define SB_ZF_CARCDATAENTRY_M_UDATA7_BITS "255:224"
#define SB_ZF_CARCDATAENTRY_M_UDATA6_BITS "223:192"
#define SB_ZF_CARCDATAENTRY_M_UDATA5_BITS "191:160"
#define SB_ZF_CARCDATAENTRY_M_UDATA4_BITS "159:128"
#define SB_ZF_CARCDATAENTRY_M_USPARE3_BITS "127:116"
#define SB_ZF_CARCDATAENTRY_M_UDATA3_BITS "115:96"
#define SB_ZF_CARCDATAENTRY_M_USPARE2_BITS "95:84"
#define SB_ZF_CARCDATAENTRY_M_UDATA2_BITS "83:64"
#define SB_ZF_CARCDATAENTRY_M_USPARE1_BITS "63:52"
#define SB_ZF_CARCDATAENTRY_M_UDATA1_BITS "51:32"
#define SB_ZF_CARCDATAENTRY_M_USPARE0_BITS "31:20"
#define SB_ZF_CARCDATAENTRY_M_UDATA0_BITS "19:0"


typedef struct _sbZfCaRcDataEntry {
  uint32_t m_uData35;
  uint32_t m_uData34;
  uint32_t m_uData33;
  uint32_t m_uData32;
  uint32_t m_uData31;
  uint32_t m_uData30;
  uint32_t m_uData29;
  uint32_t m_uData28;
  uint32_t m_uData27;
  uint32_t m_uData26;
  uint32_t m_uData25;
  uint32_t m_uData24;
  uint32_t m_uData23;
  uint32_t m_uData22;
  uint32_t m_uData21;
  uint32_t m_uData20;
  uint32_t m_uData19;
  uint32_t m_uData18;
  uint32_t m_uData17;
  uint32_t m_uData16;
  uint32_t m_uData15;
  uint32_t m_uData14;
  uint32_t m_uData13;
  uint32_t m_uData12;
  uint32_t m_uData11;
  uint32_t m_uData10;
  uint32_t m_uData9;
  uint32_t m_uData8;
  uint32_t m_uData7;
  uint32_t m_uData6;
  uint32_t m_uData5;
  uint32_t m_uData4;
  uint32_t m_uSpare3;
  uint32_t m_uData3;
  uint32_t m_uSpare2;
  uint32_t m_uData2;
  uint32_t m_uSpare1;
  uint32_t m_uData1;
  uint32_t m_uSpare0;
  uint32_t m_uData0;
} sbZfCaRcDataEntry_t;

uint32_t
sbZfCaRcDataEntry_Pack(sbZfCaRcDataEntry_t *pFrom,
                       uint8_t *pToData,
                       uint32_t nMaxToDataIndex);
void
sbZfCaRcDataEntry_Unpack(sbZfCaRcDataEntry_t *pToStruct,
                         uint8_t *pFromData,
                         uint32_t nMaxToDataIndex);
void
sbZfCaRcDataEntry_InitInstance(sbZfCaRcDataEntry_t *pFrame);

#define SB_ZF_CARCDATAENTRY_SET_DATA35(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[140] = ((nFromData)) & 0xFF; \
           (pToData)[141] = ((pToData)[141] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[142] = ((pToData)[142] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[143] = ((pToData)[143] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA34(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[136] = ((nFromData)) & 0xFF; \
           (pToData)[137] = ((pToData)[137] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[138] = ((pToData)[138] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[139] = ((pToData)[139] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA33(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[132] = ((nFromData)) & 0xFF; \
           (pToData)[133] = ((pToData)[133] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[134] = ((pToData)[134] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[135] = ((pToData)[135] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA32(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[128] = ((nFromData)) & 0xFF; \
           (pToData)[129] = ((pToData)[129] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[130] = ((pToData)[130] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[131] = ((pToData)[131] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA31(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[124] = ((nFromData)) & 0xFF; \
           (pToData)[125] = ((pToData)[125] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[126] = ((pToData)[126] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[127] = ((pToData)[127] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA30(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[120] = ((nFromData)) & 0xFF; \
           (pToData)[121] = ((pToData)[121] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[122] = ((pToData)[122] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[123] = ((pToData)[123] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA29(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[116] = ((nFromData)) & 0xFF; \
           (pToData)[117] = ((pToData)[117] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[118] = ((pToData)[118] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[119] = ((pToData)[119] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA28(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[112] = ((nFromData)) & 0xFF; \
           (pToData)[113] = ((pToData)[113] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[114] = ((pToData)[114] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[115] = ((pToData)[115] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA27(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[108] = ((nFromData)) & 0xFF; \
           (pToData)[109] = ((pToData)[109] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[110] = ((pToData)[110] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[111] = ((pToData)[111] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA26(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[104] = ((nFromData)) & 0xFF; \
           (pToData)[105] = ((pToData)[105] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[106] = ((pToData)[106] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[107] = ((pToData)[107] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA25(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[100] = ((nFromData)) & 0xFF; \
           (pToData)[101] = ((pToData)[101] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[102] = ((pToData)[102] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[103] = ((pToData)[103] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[96] = ((nFromData)) & 0xFF; \
           (pToData)[97] = ((pToData)[97] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[98] = ((pToData)[98] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[99] = ((pToData)[99] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[92] = ((nFromData)) & 0xFF; \
           (pToData)[93] = ((pToData)[93] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[94] = ((pToData)[94] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[95] = ((pToData)[95] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[88] = ((nFromData)) & 0xFF; \
           (pToData)[89] = ((pToData)[89] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[90] = ((pToData)[90] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[91] = ((pToData)[91] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[84] = ((nFromData)) & 0xFF; \
           (pToData)[85] = ((pToData)[85] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[86] = ((pToData)[86] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[87] = ((pToData)[87] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[80] = ((nFromData)) & 0xFF; \
           (pToData)[81] = ((pToData)[81] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[82] = ((pToData)[82] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[83] = ((pToData)[83] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[76] = ((nFromData)) & 0xFF; \
           (pToData)[77] = ((pToData)[77] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[78] = ((pToData)[78] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[79] = ((pToData)[79] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[72] = ((nFromData)) & 0xFF; \
           (pToData)[73] = ((pToData)[73] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[74] = ((pToData)[74] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[75] = ((pToData)[75] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[68] = ((nFromData)) & 0xFF; \
           (pToData)[69] = ((pToData)[69] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[70] = ((pToData)[70] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[71] = ((pToData)[71] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[64] = ((nFromData)) & 0xFF; \
           (pToData)[65] = ((pToData)[65] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[66] = ((pToData)[66] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[67] = ((pToData)[67] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[60] = ((nFromData)) & 0xFF; \
           (pToData)[61] = ((pToData)[61] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[62] = ((pToData)[62] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[63] = ((pToData)[63] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[56] = ((nFromData)) & 0xFF; \
           (pToData)[57] = ((pToData)[57] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[58] = ((pToData)[58] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[59] = ((pToData)[59] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[52] = ((nFromData)) & 0xFF; \
           (pToData)[53] = ((pToData)[53] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[54] = ((pToData)[54] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[55] = ((pToData)[55] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[48] = ((nFromData)) & 0xFF; \
           (pToData)[49] = ((pToData)[49] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[50] = ((pToData)[50] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[51] = ((pToData)[51] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[44] = ((nFromData)) & 0xFF; \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[40] = ((nFromData)) & 0xFF; \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[36] = ((nFromData)) & 0xFF; \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[32] = ((nFromData)) & 0xFF; \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_SPARE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_SPARE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_SET_DATA0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA35(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[140] ; \
           (nToData) |= (uint32_t) (pFromData)[141] << 8; \
           (nToData) |= (uint32_t) (pFromData)[142] << 16; \
           (nToData) |= (uint32_t) (pFromData)[143] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA34(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[136] ; \
           (nToData) |= (uint32_t) (pFromData)[137] << 8; \
           (nToData) |= (uint32_t) (pFromData)[138] << 16; \
           (nToData) |= (uint32_t) (pFromData)[139] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA33(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[132] ; \
           (nToData) |= (uint32_t) (pFromData)[133] << 8; \
           (nToData) |= (uint32_t) (pFromData)[134] << 16; \
           (nToData) |= (uint32_t) (pFromData)[135] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA32(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[128] ; \
           (nToData) |= (uint32_t) (pFromData)[129] << 8; \
           (nToData) |= (uint32_t) (pFromData)[130] << 16; \
           (nToData) |= (uint32_t) (pFromData)[131] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA31(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[124] ; \
           (nToData) |= (uint32_t) (pFromData)[125] << 8; \
           (nToData) |= (uint32_t) (pFromData)[126] << 16; \
           (nToData) |= (uint32_t) (pFromData)[127] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA30(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[120] ; \
           (nToData) |= (uint32_t) (pFromData)[121] << 8; \
           (nToData) |= (uint32_t) (pFromData)[122] << 16; \
           (nToData) |= (uint32_t) (pFromData)[123] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA29(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[116] ; \
           (nToData) |= (uint32_t) (pFromData)[117] << 8; \
           (nToData) |= (uint32_t) (pFromData)[118] << 16; \
           (nToData) |= (uint32_t) (pFromData)[119] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA28(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[112] ; \
           (nToData) |= (uint32_t) (pFromData)[113] << 8; \
           (nToData) |= (uint32_t) (pFromData)[114] << 16; \
           (nToData) |= (uint32_t) (pFromData)[115] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA27(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[108] ; \
           (nToData) |= (uint32_t) (pFromData)[109] << 8; \
           (nToData) |= (uint32_t) (pFromData)[110] << 16; \
           (nToData) |= (uint32_t) (pFromData)[111] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA26(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[104] ; \
           (nToData) |= (uint32_t) (pFromData)[105] << 8; \
           (nToData) |= (uint32_t) (pFromData)[106] << 16; \
           (nToData) |= (uint32_t) (pFromData)[107] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA25(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[100] ; \
           (nToData) |= (uint32_t) (pFromData)[101] << 8; \
           (nToData) |= (uint32_t) (pFromData)[102] << 16; \
           (nToData) |= (uint32_t) (pFromData)[103] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[96] ; \
           (nToData) |= (uint32_t) (pFromData)[97] << 8; \
           (nToData) |= (uint32_t) (pFromData)[98] << 16; \
           (nToData) |= (uint32_t) (pFromData)[99] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[92] ; \
           (nToData) |= (uint32_t) (pFromData)[93] << 8; \
           (nToData) |= (uint32_t) (pFromData)[94] << 16; \
           (nToData) |= (uint32_t) (pFromData)[95] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[88] ; \
           (nToData) |= (uint32_t) (pFromData)[89] << 8; \
           (nToData) |= (uint32_t) (pFromData)[90] << 16; \
           (nToData) |= (uint32_t) (pFromData)[91] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[84] ; \
           (nToData) |= (uint32_t) (pFromData)[85] << 8; \
           (nToData) |= (uint32_t) (pFromData)[86] << 16; \
           (nToData) |= (uint32_t) (pFromData)[87] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[80] ; \
           (nToData) |= (uint32_t) (pFromData)[81] << 8; \
           (nToData) |= (uint32_t) (pFromData)[82] << 16; \
           (nToData) |= (uint32_t) (pFromData)[83] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[76] ; \
           (nToData) |= (uint32_t) (pFromData)[77] << 8; \
           (nToData) |= (uint32_t) (pFromData)[78] << 16; \
           (nToData) |= (uint32_t) (pFromData)[79] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[72] ; \
           (nToData) |= (uint32_t) (pFromData)[73] << 8; \
           (nToData) |= (uint32_t) (pFromData)[74] << 16; \
           (nToData) |= (uint32_t) (pFromData)[75] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[68] ; \
           (nToData) |= (uint32_t) (pFromData)[69] << 8; \
           (nToData) |= (uint32_t) (pFromData)[70] << 16; \
           (nToData) |= (uint32_t) (pFromData)[71] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[64] ; \
           (nToData) |= (uint32_t) (pFromData)[65] << 8; \
           (nToData) |= (uint32_t) (pFromData)[66] << 16; \
           (nToData) |= (uint32_t) (pFromData)[67] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[60] ; \
           (nToData) |= (uint32_t) (pFromData)[61] << 8; \
           (nToData) |= (uint32_t) (pFromData)[62] << 16; \
           (nToData) |= (uint32_t) (pFromData)[63] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[56] ; \
           (nToData) |= (uint32_t) (pFromData)[57] << 8; \
           (nToData) |= (uint32_t) (pFromData)[58] << 16; \
           (nToData) |= (uint32_t) (pFromData)[59] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[52] ; \
           (nToData) |= (uint32_t) (pFromData)[53] << 8; \
           (nToData) |= (uint32_t) (pFromData)[54] << 16; \
           (nToData) |= (uint32_t) (pFromData)[55] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[48] ; \
           (nToData) |= (uint32_t) (pFromData)[49] << 8; \
           (nToData) |= (uint32_t) (pFromData)[50] << 16; \
           (nToData) |= (uint32_t) (pFromData)[51] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[44] ; \
           (nToData) |= (uint32_t) (pFromData)[45] << 8; \
           (nToData) |= (uint32_t) (pFromData)[46] << 16; \
           (nToData) |= (uint32_t) (pFromData)[47] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[40] ; \
           (nToData) |= (uint32_t) (pFromData)[41] << 8; \
           (nToData) |= (uint32_t) (pFromData)[42] << 16; \
           (nToData) |= (uint32_t) (pFromData)[43] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[36] ; \
           (nToData) |= (uint32_t) (pFromData)[37] << 8; \
           (nToData) |= (uint32_t) (pFromData)[38] << 16; \
           (nToData) |= (uint32_t) (pFromData)[39] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[32] ; \
           (nToData) |= (uint32_t) (pFromData)[33] << 8; \
           (nToData) |= (uint32_t) (pFromData)[34] << 16; \
           (nToData) |= (uint32_t) (pFromData)[35] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[28] ; \
           (nToData) |= (uint32_t) (pFromData)[29] << 8; \
           (nToData) |= (uint32_t) (pFromData)[30] << 16; \
           (nToData) |= (uint32_t) (pFromData)[31] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[24] ; \
           (nToData) |= (uint32_t) (pFromData)[25] << 8; \
           (nToData) |= (uint32_t) (pFromData)[26] << 16; \
           (nToData) |= (uint32_t) (pFromData)[27] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[20] ; \
           (nToData) |= (uint32_t) (pFromData)[21] << 8; \
           (nToData) |= (uint32_t) (pFromData)[22] << 16; \
           (nToData) |= (uint32_t) (pFromData)[23] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[16] ; \
           (nToData) |= (uint32_t) (pFromData)[17] << 8; \
           (nToData) |= (uint32_t) (pFromData)[18] << 16; \
           (nToData) |= (uint32_t) (pFromData)[19] << 24; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_SPARE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[15] << 4; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[12] ; \
           (nToData) |= (uint32_t) (pFromData)[13] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[14] & 0x0f) << 16; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_SPARE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[11] << 4; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) (pFromData)[9] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x0f) << 16; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[7] << 4; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x0f) << 16; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[3] << 4; \
          } while(0)

#define SB_ZF_CARCDATAENTRY_GET_DATA0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x0f) << 16; \
          } while(0)

#endif
