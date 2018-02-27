/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpIpMplsLabels.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAEPIPMPLSLABELS_H
#define SB_ZF_ZFKAEPIPMPLSLABELS_H

#define SB_ZF_ZFKAEPIPMPLSLABELS_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAEPIPMPLSLABELS_SIZE 8
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NLABEL1_BITS "63:44"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NOP_BITS "43:43"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NLINK_BITS "42:41"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NSTACK1_BITS "40:40"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NTTTL1_BITS "39:32"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NLABEL0_BITS "31:12"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NEXP_BITS "11:9"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NSTACK0_BITS "8:8"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NTTTL0_BITS "7:0"


typedef struct _sbZfKaEpIpMplsLabels {
  uint32_t m_nLabel1;
  uint32_t m_nOp;
  uint32_t m_nLink;
  uint32_t m_nStack1;
  uint32_t m_nTttl1;
  uint32_t m_nLabel0;
  uint32_t m_nExp;
  uint32_t m_nStack0;
  uint32_t m_nTttl0;
} sbZfKaEpIpMplsLabels_t;

uint32_t
sbZfKaEpIpMplsLabels_Pack(sbZfKaEpIpMplsLabels_t *pFrom,
                          uint8_t *pToData,
                          uint32_t nMaxToDataIndex);
void
sbZfKaEpIpMplsLabels_Unpack(sbZfKaEpIpMplsLabels_t *pToStruct,
                            uint8_t *pFromData,
                            uint32_t nMaxToDataIndex);
void
sbZfKaEpIpMplsLabels_InitInstance(sbZfKaEpIpMplsLabels_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_OP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_EXP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_OP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_EXP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_OP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_EXP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_OP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_EXP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[5] << 4; \
           (nToData) |= (uint32_t) (pFromData)[4] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_OP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 4; \
           (nToData) |= (uint32_t) (pFromData)[0] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_EXP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[6] << 4; \
           (nToData) |= (uint32_t) (pFromData)[7] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_OP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[2] << 4; \
           (nToData) |= (uint32_t) (pFromData)[3] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_EXP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[5] << 4; \
           (nToData) |= (uint32_t) (pFromData)[4] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_OP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 4; \
           (nToData) |= (uint32_t) (pFromData)[0] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_EXP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[6] << 4; \
           (nToData) |= (uint32_t) (pFromData)[7] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_OP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[2] << 4; \
           (nToData) |= (uint32_t) (pFromData)[3] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_EXP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
#endif
