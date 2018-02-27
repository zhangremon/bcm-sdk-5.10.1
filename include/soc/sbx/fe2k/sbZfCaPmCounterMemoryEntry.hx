/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPmCounterMemoryEntry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAPMCOUNTERMEMORYENTRY_H
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_H

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SIZE_IN_BYTES 128
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SIZE 128
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR15_BITS "1023:989"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR15_BITS "988:960"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR14_BITS "959:925"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR14_BITS "924:896"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR13_BITS "895:860"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR13_BITS "860:832"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR12_BITS "831:797"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR12_BITS "796:768"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR11_BITS "767:732"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR11_BITS "732:704"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR10_BITS "703:669"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR10_BITS "668:640"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR9_BITS "639:605"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR9_BITS "604:576"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR8_BITS "575:540"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR8_BITS "540:512"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR7_BITS "511:477"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR7_BITS "476:448"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR6_BITS "447:413"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR6_BITS "412:384"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR5_BITS "383:349"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR5_BITS "348:320"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR4_BITS "319:285"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR4_BITS "284:256"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR3_BITS "255:221"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR3_BITS "220:192"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR2_BITS "191:157"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR2_BITS "156:128"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR1_BITS "127:93"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR1_BITS "92:64"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UUBYTECNTR0_BITS "63:29"
#define SB_ZF_CAPMCOUNTERMEMORYENTRY_M_UPKTCNTR0_BITS "28:0"


typedef struct _sbZfCaPmCounterMemoryEntry {
  uint64_t m_uuByteCntr15;
  uint32_t m_uPktCntr15;
  uint64_t m_uuByteCntr14;
  uint32_t m_uPktCntr14;
  uint64_t m_uuByteCntr13;
  uint32_t m_uPktCntr13;
  uint64_t m_uuByteCntr12;
  uint32_t m_uPktCntr12;
  uint64_t m_uuByteCntr11;
  uint32_t m_uPktCntr11;
  uint64_t m_uuByteCntr10;
  uint32_t m_uPktCntr10;
  uint64_t m_uuByteCntr9;
  uint32_t m_uPktCntr9;
  uint64_t m_uuByteCntr8;
  uint32_t m_uPktCntr8;
  uint64_t m_uuByteCntr7;
  uint32_t m_uPktCntr7;
  uint64_t m_uuByteCntr6;
  uint32_t m_uPktCntr6;
  uint64_t m_uuByteCntr5;
  uint32_t m_uPktCntr5;
  uint64_t m_uuByteCntr4;
  uint32_t m_uPktCntr4;
  uint64_t m_uuByteCntr3;
  uint32_t m_uPktCntr3;
  uint64_t m_uuByteCntr2;
  uint32_t m_uPktCntr2;
  uint64_t m_uuByteCntr1;
  uint32_t m_uPktCntr1;
  uint64_t m_uuByteCntr0;
  uint32_t m_uPktCntr0;
} sbZfCaPmCounterMemoryEntry_t;

uint32_t
sbZfCaPmCounterMemoryEntry_Pack(sbZfCaPmCounterMemoryEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex);
void
sbZfCaPmCounterMemoryEntry_Unpack(sbZfCaPmCounterMemoryEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex);
void
sbZfCaPmCounterMemoryEntry_InitInstance(sbZfCaPmCounterMemoryEntry_t *pFrame);

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[123] = ((pToData)[123] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[124] = ((pToData)[124] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[125] = ((pToData)[125] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[126] = ((pToData)[126] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[127] = ((pToData)[127] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[120] = ((nFromData)) & 0xFF; \
           (pToData)[121] = ((pToData)[121] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[122] = ((pToData)[122] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[123] = ((pToData)[123] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[115] = ((pToData)[115] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[116] = ((pToData)[116] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[117] = ((pToData)[117] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[118] = ((pToData)[118] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[119] = ((pToData)[119] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[112] = ((nFromData)) & 0xFF; \
           (pToData)[113] = ((pToData)[113] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[114] = ((pToData)[114] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[115] = ((pToData)[115] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[107] = ((pToData)[107] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[108] = ((pToData)[108] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[109] = ((pToData)[109] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
           (pToData)[110] = ((pToData)[110] & ~0xFF) | (((nFromData) >> 20) & 0xFF); \
           (pToData)[111] = ((pToData)[111] & ~0xFF) | (((nFromData) >> 28) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[104] = ((nFromData)) & 0xFF; \
           (pToData)[105] = ((pToData)[105] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[106] = ((pToData)[106] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[107] = ((pToData)[107] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[99] = ((pToData)[99] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[100] = ((pToData)[100] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[101] = ((pToData)[101] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[102] = ((pToData)[102] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[103] = ((pToData)[103] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[96] = ((nFromData)) & 0xFF; \
           (pToData)[97] = ((pToData)[97] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[98] = ((pToData)[98] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[99] = ((pToData)[99] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[91] = ((pToData)[91] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[92] = ((pToData)[92] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[93] = ((pToData)[93] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
           (pToData)[94] = ((pToData)[94] & ~0xFF) | (((nFromData) >> 20) & 0xFF); \
           (pToData)[95] = ((pToData)[95] & ~0xFF) | (((nFromData) >> 28) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[88] = ((nFromData)) & 0xFF; \
           (pToData)[89] = ((pToData)[89] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[90] = ((pToData)[90] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[91] = ((pToData)[91] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[83] = ((pToData)[83] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[84] = ((pToData)[84] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[85] = ((pToData)[85] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[86] = ((pToData)[86] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[87] = ((pToData)[87] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[80] = ((nFromData)) & 0xFF; \
           (pToData)[81] = ((pToData)[81] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[82] = ((pToData)[82] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[83] = ((pToData)[83] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[75] = ((pToData)[75] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[76] = ((pToData)[76] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[77] = ((pToData)[77] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[78] = ((pToData)[78] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[79] = ((pToData)[79] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[72] = ((nFromData)) & 0xFF; \
           (pToData)[73] = ((pToData)[73] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[74] = ((pToData)[74] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[75] = ((pToData)[75] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[67] = ((pToData)[67] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[68] = ((pToData)[68] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[69] = ((pToData)[69] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
           (pToData)[70] = ((pToData)[70] & ~0xFF) | (((nFromData) >> 20) & 0xFF); \
           (pToData)[71] = ((pToData)[71] & ~0xFF) | (((nFromData) >> 28) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[64] = ((nFromData)) & 0xFF; \
           (pToData)[65] = ((pToData)[65] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[66] = ((pToData)[66] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[67] = ((pToData)[67] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[59] = ((pToData)[59] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[60] = ((pToData)[60] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[61] = ((pToData)[61] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[62] = ((pToData)[62] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[63] = ((pToData)[63] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[56] = ((nFromData)) & 0xFF; \
           (pToData)[57] = ((pToData)[57] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[58] = ((pToData)[58] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[59] = ((pToData)[59] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[51] = ((pToData)[51] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[52] = ((pToData)[52] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[53] = ((pToData)[53] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[54] = ((pToData)[54] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[55] = ((pToData)[55] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[48] = ((nFromData)) & 0xFF; \
           (pToData)[49] = ((pToData)[49] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[50] = ((pToData)[50] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[51] = ((pToData)[51] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[43] = ((pToData)[43] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[40] = ((nFromData)) & 0xFF; \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[35] = ((pToData)[35] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[32] = ((nFromData)) & 0xFF; \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[27] = ((pToData)[27] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_BYTECNTR0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_SET_PKTCNTR0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x1f) | (((nFromData) >> 24) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[123]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[124]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[125]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[126]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[127]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[120] ; \
           (nToData) |= (uint32_t) (pFromData)[121] << 8; \
           (nToData) |= (uint32_t) (pFromData)[122] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[123] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[115]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[116]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[117]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[118]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[119]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[112] ; \
           (nToData) |= (uint32_t) (pFromData)[113] << 8; \
           (nToData) |= (uint32_t) (pFromData)[114] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[115] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[107]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[108]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[109]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[110]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[111]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[104] ; \
           (nToData) |= (uint32_t) (pFromData)[105] << 8; \
           (nToData) |= (uint32_t) (pFromData)[106] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[107] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[99]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[100]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[101]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[102]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[103]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[96] ; \
           (nToData) |= (uint32_t) (pFromData)[97] << 8; \
           (nToData) |= (uint32_t) (pFromData)[98] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[99] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[91]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[92]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[93]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[94]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[95]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[88] ; \
           (nToData) |= (uint32_t) (pFromData)[89] << 8; \
           (nToData) |= (uint32_t) (pFromData)[90] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[91] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[83]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[84]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[85]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[86]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[87]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[80] ; \
           (nToData) |= (uint32_t) (pFromData)[81] << 8; \
           (nToData) |= (uint32_t) (pFromData)[82] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[83] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[75]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[76]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[77]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[78]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[79]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[72] ; \
           (nToData) |= (uint32_t) (pFromData)[73] << 8; \
           (nToData) |= (uint32_t) (pFromData)[74] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[75] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[67]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[68]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[69]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[70]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[71]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[64] ; \
           (nToData) |= (uint32_t) (pFromData)[65] << 8; \
           (nToData) |= (uint32_t) (pFromData)[66] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[67] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[59]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[60]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[61]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[62]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[63]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[56] ; \
           (nToData) |= (uint32_t) (pFromData)[57] << 8; \
           (nToData) |= (uint32_t) (pFromData)[58] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[59] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[51]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[52]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[53]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[54]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[55]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[48] ; \
           (nToData) |= (uint32_t) (pFromData)[49] << 8; \
           (nToData) |= (uint32_t) (pFromData)[50] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[51] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[43]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[44]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[47]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[40] ; \
           (nToData) |= (uint32_t) (pFromData)[41] << 8; \
           (nToData) |= (uint32_t) (pFromData)[42] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[43] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[35]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[36]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[39]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[32] ; \
           (nToData) |= (uint32_t) (pFromData)[33] << 8; \
           (nToData) |= (uint32_t) (pFromData)[34] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[35] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[27]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[28]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[29]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[30]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[31]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[24] ; \
           (nToData) |= (uint32_t) (pFromData)[25] << 8; \
           (nToData) |= (uint32_t) (pFromData)[26] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[27] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[19]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[16] ; \
           (nToData) |= (uint32_t) (pFromData)[17] << 8; \
           (nToData) |= (uint32_t) (pFromData)[18] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[19] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[11]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) (pFromData)[9] << 8; \
           (nToData) |= (uint32_t) (pFromData)[10] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[11] & 0x1f) << 24; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_BYTECNTR0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[3]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_CAPMCOUNTERMEMORYENTRY_GET_PKTCNTR0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x1f) << 24; \
          } while(0)

#endif
