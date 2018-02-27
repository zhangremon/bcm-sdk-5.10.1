/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmFbLine.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAQMFBLINE_H
#define SB_ZF_ZFKAQMFBLINE_H

#define SB_ZF_ZFKAQMFBLINE_SIZE_IN_BYTES 16
#define SB_ZF_ZFKAQMFBLINE_SIZE 16
#define SB_ZF_ZFKAQMFBLINE_M_NHEC1_BITS "127:120"
#define SB_ZF_ZFKAQMFBLINE_M_NHEC0_BITS "119:112"
#define SB_ZF_ZFKAQMFBLINE_M_NSPARE_BITS "111:102"
#define SB_ZF_ZFKAQMFBLINE_M_NPBEXTADDR5_BITS "101:85"
#define SB_ZF_ZFKAQMFBLINE_M_NPBEXTADDR4_BITS "84:68"
#define SB_ZF_ZFKAQMFBLINE_M_NPBEXTADDR3_BITS "67:51"
#define SB_ZF_ZFKAQMFBLINE_M_NPBEXTADDR2_BITS "50:34"
#define SB_ZF_ZFKAQMFBLINE_M_NPBEXTADDR1_BITS "33:17"
#define SB_ZF_ZFKAQMFBLINE_M_NPBEXTADDR0_BITS "16:0"


typedef struct _sbZfKaQmFbLine {
  uint32_t m_nHec1;
  uint32_t m_nHec0;
  uint32_t m_nSpare;
  uint32_t m_nPbExtAddr5;
  uint32_t m_nPbExtAddr4;
  uint32_t m_nPbExtAddr3;
  uint32_t m_nPbExtAddr2;
  uint32_t m_nPbExtAddr1;
  uint32_t m_nPbExtAddr0;
} sbZfKaQmFbLine_t;

uint32_t
sbZfKaQmFbLine_Pack(sbZfKaQmFbLine_t *pFrom,
                    uint8_t *pToData,
                    uint32_t nMaxToDataIndex);
void
sbZfKaQmFbLine_Unpack(sbZfKaQmFbLine_t *pToStruct,
                      uint8_t *pFromData,
                      uint32_t nMaxToDataIndex);
void
sbZfKaQmFbLine_InitInstance(sbZfKaQmFbLine_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMFBLINE_SET_HEC1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_HEC0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_SPR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB05(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~ 0x3f) | (((nFromData) >> 11) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB04(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~ 0x1f) | (((nFromData) >> 12) & 0x1f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB03(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~ 0x0f) | (((nFromData) >> 13) & 0x0f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB02(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x07) | (((nFromData) >> 14) & 0x07); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB01(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 15) & 0x03); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB00(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQMFBLINE_SET_HEC1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_HEC0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_SPR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB05(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~ 0x3f) | (((nFromData) >> 11) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB04(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x1f) | (((nFromData) >> 12) & 0x1f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB03(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~ 0x0f) | (((nFromData) >> 13) & 0x0f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB02(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x07) | (((nFromData) >> 14) & 0x07); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB01(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 15) & 0x03); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB00(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMFBLINE_SET_HEC1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_HEC0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_SPR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB05(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~ 0x3f) | (((nFromData) >> 11) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB04(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~ 0x1f) | (((nFromData) >> 12) & 0x1f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB03(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~ 0x0f) | (((nFromData) >> 13) & 0x0f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB02(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x07) | (((nFromData) >> 14) & 0x07); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB01(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 15) & 0x03); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB00(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQMFBLINE_SET_HEC1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_HEC0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_SPR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB05(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~ 0x3f) | (((nFromData) >> 11) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB04(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x1f) | (((nFromData) >> 12) & 0x1f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB03(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~ 0x0f) | (((nFromData) >> 13) & 0x0f); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB02(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x07) | (((nFromData) >> 14) & 0x07); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB01(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 15) & 0x03); \
          } while(0)

#define SB_ZF_KAQMFBLINE_SET_PB00(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMFBLINE_GET_HEC1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[12] ; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_HEC0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[13] ; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_SPR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[14] << 2; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB05(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[8] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[15] & 0x3f) << 11; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB04(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[10] << 4; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x1f) << 12; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB03(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) (pFromData)[4] << 5; \
           (nToData) |= (uint32_t) ((pFromData)[11] & 0x0f) << 13; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB02(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[6] << 6; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x07) << 14; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB01(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) (pFromData)[0] << 7; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x03) << 15; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB00(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x01) << 16; \
          } while(0)

#else
#define SB_ZF_KAQMFBLINE_GET_HEC1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[15] ; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_HEC0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[14] ; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_SPR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[13] << 2; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB05(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[11] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[12] & 0x3f) << 11; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB04(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[9] << 4; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x1f) << 12; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB03(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) (pFromData)[7] << 5; \
           (nToData) |= (uint32_t) ((pFromData)[8] & 0x0f) << 13; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB02(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[5] << 6; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x07) << 14; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB01(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) (pFromData)[3] << 7; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x03) << 15; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB00(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMFBLINE_GET_HEC1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[12] ; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_HEC0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[13] ; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_SPR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[14] << 2; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB05(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[8] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[15] & 0x3f) << 11; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB04(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[10] << 4; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x1f) << 12; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB03(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) (pFromData)[4] << 5; \
           (nToData) |= (uint32_t) ((pFromData)[11] & 0x0f) << 13; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB02(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[6] << 6; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x07) << 14; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB01(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) (pFromData)[0] << 7; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x03) << 15; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB00(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x01) << 16; \
          } while(0)

#else
#define SB_ZF_KAQMFBLINE_GET_HEC1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[15] ; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_HEC0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[14] ; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_SPR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[13] << 2; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB05(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[11] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[12] & 0x3f) << 11; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB04(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[9] << 4; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x1f) << 12; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB03(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) (pFromData)[7] << 5; \
           (nToData) |= (uint32_t) ((pFromData)[8] & 0x0f) << 13; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB02(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[5] << 6; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x07) << 14; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB01(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) (pFromData)[3] << 7; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x03) << 15; \
          } while(0)

#define SB_ZF_KAQMFBLINE_GET_PB00(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 16; \
          } while(0)

#endif
#endif
