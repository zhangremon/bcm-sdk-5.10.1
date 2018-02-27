/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsQueueTableEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAQSQUEUETABLEENTRY_H
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_H

#define SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE 8
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NCREDIT_BITS "63:39"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NHPLEN_BITS "38:37"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NDEPTH_BITS "36:33"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NQ2EC_BITS "32:16"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NLOCALQ_BITS "15:15"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NMAXHOLDTS_BITS "14:12"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NQUEUETYPE_BITS "11:8"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NSHAPERATEMSB_BITS "7:0"


typedef struct _sbZfKaQsQueueTableEntry {
  uint32_t m_nCredit;
  uint32_t m_nHpLen;
  uint32_t m_nDepth;
  uint32_t m_nQ2Ec;
  uint32_t m_nLocalQ;
  uint32_t m_nMaxHoldTs;
  uint32_t m_nQueueType;
  uint32_t m_nShapeRateMSB;
} sbZfKaQsQueueTableEntry_t;

uint32_t
sbZfKaQsQueueTableEntry_Pack(sbZfKaQsQueueTableEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex);
void
sbZfKaQsQueueTableEntry_Unpack(sbZfKaQsQueueTableEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex);
void
sbZfKaQsQueueTableEntry_InitInstance(sbZfKaQsQueueTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUETABLEENTRY_SET_CREDIT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_Q2EC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_LOCALQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_QUEUETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_SHAPERATEMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUETABLEENTRY_SET_CREDIT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_Q2EC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_LOCALQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_QUEUETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_SHAPERATEMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUETABLEENTRY_SET_CREDIT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_Q2EC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_LOCALQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_QUEUETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_SHAPERATEMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUETABLEENTRY_SET_CREDIT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_Q2EC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_LOCALQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_QUEUETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_SHAPERATEMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUETABLEENTRY_GET_CREDIT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[6] << 1; \
           (nToData) |= (uint32_t) (pFromData)[5] << 9; \
           (nToData) |= (uint32_t) (pFromData)[4] << 17; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_Q2EC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[1] ; \
           (nToData) |= (uint32_t) (pFromData)[0] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x01) << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_LOCALQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_QUEUETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_SHAPERATEMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUETABLEENTRY_GET_CREDIT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[5] << 1; \
           (nToData) |= (uint32_t) (pFromData)[6] << 9; \
           (nToData) |= (uint32_t) (pFromData)[7] << 17; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_Q2EC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) (pFromData)[3] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x01) << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_LOCALQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_QUEUETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_SHAPERATEMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUETABLEENTRY_GET_CREDIT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[6] << 1; \
           (nToData) |= (uint32_t) (pFromData)[5] << 9; \
           (nToData) |= (uint32_t) (pFromData)[4] << 17; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_Q2EC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[1] ; \
           (nToData) |= (uint32_t) (pFromData)[0] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x01) << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_LOCALQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_QUEUETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_SHAPERATEMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUETABLEENTRY_GET_CREDIT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[5] << 1; \
           (nToData) |= (uint32_t) (pFromData)[6] << 9; \
           (nToData) |= (uint32_t) (pFromData)[7] << 17; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_Q2EC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) (pFromData)[3] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x01) << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_LOCALQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_QUEUETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_SHAPERATEMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
#endif
