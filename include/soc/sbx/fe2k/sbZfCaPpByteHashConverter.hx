/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPpByteHashConverter.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAPPBYTEHASHCONVERTER_H
#define SB_ZF_CAPPBYTEHASHCONVERTER_H

#define SB_ZF_CAPPBYTEHASHCONVERTER_SIZE_IN_BYTES 5
#define SB_ZF_CAPPBYTEHASHCONVERTER_SIZE 5
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT39_BITS "39:39"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT38_BITS "38:38"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT37_BITS "37:37"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT36_BITS "36:36"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT35_BITS "35:35"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT34_BITS "34:34"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT33_BITS "33:33"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT32_BITS "32:32"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT31_BITS "31:31"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT30_BITS "30:30"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT29_BITS "29:29"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT28_BITS "28:28"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT27_BITS "27:27"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT26_BITS "26:26"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT25_BITS "25:25"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT24_BITS "24:24"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT23_BITS "23:23"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT22_BITS "22:22"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT21_BITS "21:21"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT20_BITS "20:20"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT19_BITS "19:19"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT18_BITS "18:18"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT17_BITS "17:17"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT16_BITS "16:16"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT15_BITS "15:15"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT14_BITS "14:14"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT13_BITS "13:13"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT12_BITS "12:12"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT11_BITS "11:11"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT10_BITS "10:10"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT9_BITS "9:9"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT8_BITS "8:8"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT7_BITS "7:7"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT6_BITS "6:6"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT5_BITS "5:5"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT4_BITS "4:4"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT3_BITS "3:3"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT2_BITS "2:2"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT1_BITS "1:1"
#define SB_ZF_CAPPBYTEHASHCONVERTER_M_UHASHBYTEBIT0_BITS "0:0"


typedef struct _sbZfCaPpByteHashConverter {
  uint8_t m_uHashByteBit39;
  uint8_t m_uHashByteBit38;
  uint8_t m_uHashByteBit37;
  uint8_t m_uHashByteBit36;
  uint8_t m_uHashByteBit35;
  uint8_t m_uHashByteBit34;
  uint8_t m_uHashByteBit33;
  uint8_t m_uHashByteBit32;
  uint8_t m_uHashByteBit31;
  uint8_t m_uHashByteBit30;
  uint8_t m_uHashByteBit29;
  uint8_t m_uHashByteBit28;
  uint8_t m_uHashByteBit27;
  uint8_t m_uHashByteBit26;
  uint8_t m_uHashByteBit25;
  uint8_t m_uHashByteBit24;
  uint8_t m_uHashByteBit23;
  uint8_t m_uHashByteBit22;
  uint8_t m_uHashByteBit21;
  uint8_t m_uHashByteBit20;
  uint8_t m_uHashByteBit19;
  uint8_t m_uHashByteBit18;
  uint8_t m_uHashByteBit17;
  uint8_t m_uHashByteBit16;
  uint8_t m_uHashByteBit15;
  uint8_t m_uHashByteBit14;
  uint8_t m_uHashByteBit13;
  uint8_t m_uHashByteBit12;
  uint8_t m_uHashByteBit11;
  uint8_t m_uHashByteBit10;
  uint8_t m_uHashByteBit9;
  uint8_t m_uHashByteBit8;
  uint8_t m_uHashByteBit7;
  uint8_t m_uHashByteBit6;
  uint8_t m_uHashByteBit5;
  uint8_t m_uHashByteBit4;
  uint8_t m_uHashByteBit3;
  uint8_t m_uHashByteBit2;
  uint8_t m_uHashByteBit1;
  uint8_t m_uHashByteBit0;
} sbZfCaPpByteHashConverter_t;

uint32_t
sbZfCaPpByteHashConverter_Pack(sbZfCaPpByteHashConverter_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex);
void
sbZfCaPpByteHashConverter_Unpack(sbZfCaPpByteHashConverter_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex);
void
sbZfCaPpByteHashConverter_InitInstance(sbZfCaPpByteHashConverter_t *pFrame);

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT39(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT38(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT37(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT36(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT35(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT34(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT33(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT32(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT31(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT30(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT29(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT28(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT27(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT26(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT25(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_SET_HBIT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT39(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT38(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT37(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT36(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT35(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT34(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT33(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT32(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4]) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT31(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT30(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT29(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT28(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT27(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT26(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT25(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_CAPPBYTEHASHCONVERTER_GET_HBIT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
