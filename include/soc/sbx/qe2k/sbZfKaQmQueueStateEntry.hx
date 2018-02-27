/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmQueueStateEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAQMQUEUESTATEENTRY_H
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_H

#define SB_ZF_ZFKAQMQUEUESTATEENTRY_SIZE_IN_BYTES 16
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_SIZE 16
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NALLOCATEDBUFFSCNT_BITS "127:111"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NQTAILPTR_BITS "110:86"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NQHEADPTR_BITS "85:61"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NNOBUFFSALLOCATED_BITS "60:60"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NOVERFLOW_BITS "59:59"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NMINBUFFERS_BITS "58:45"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NMAXBUFFERS_BITS "44:31"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NLOCAL_BITS "30:30"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NQUEUEDEPTHINLINE16B_BITS "29:5"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NANEMICWATERMARKSEL_BITS "4:2"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NQETYPE_BITS "1:1"
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_M_NENABLE_BITS "0:0"


typedef struct _sbZfKaQmQueueStateEntry {
  uint32_t m_nAllocatedBuffsCnt;
  uint32_t m_nQTailPtr;
  uint32_t m_nQHeadPtr;
  uint8_t m_nNoBuffsAllocated;
  uint8_t m_nOverflow;
  uint32_t m_nMinBuffers;
  uint32_t m_nMaxBuffers;
  uint32_t m_nLocal;
  uint32_t m_nQueueDepthInLine16B;
  uint32_t m_nAnemicWatermarkSel;
  uint32_t m_nQeType;
  uint8_t m_nEnable;
} sbZfKaQmQueueStateEntry_t;

uint32_t
sbZfKaQmQueueStateEntry_Pack(sbZfKaQmQueueStateEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex);
void
sbZfKaQmQueueStateEntry_Unpack(sbZfKaQmQueueStateEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex);
void
sbZfKaQmQueueStateEntry_InitInstance(sbZfKaQmQueueStateEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ALLOCATEDBUFFSCNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QTAILPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~ 0x7f) | (((nFromData) >> 18) & 0x7f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QHEADPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~ 0x3f) | (((nFromData) >> 19) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_NOBUFFSALLOCATED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_OVERFLOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_MINBUFFERS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 11) & 0x07); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_MAXBUFFERS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x1f) | (((nFromData) >> 9) & 0x1f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_LOCAL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QUEUEDEPTHINLINE16B(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x3f) | (((nFromData) >> 19) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ANEMICWATERMARKSEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ALLOCATEDBUFFSCNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QTAILPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~ 0x7f) | (((nFromData) >> 18) & 0x7f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QHEADPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x3f) | (((nFromData) >> 19) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_NOBUFFSALLOCATED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_OVERFLOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_MINBUFFERS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x07) | (((nFromData) >> 11) & 0x07); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_MAXBUFFERS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x1f) | (((nFromData) >> 9) & 0x1f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_LOCAL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QUEUEDEPTHINLINE16B(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x3f) | (((nFromData) >> 19) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ANEMICWATERMARKSEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ALLOCATEDBUFFSCNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QTAILPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~ 0x7f) | (((nFromData) >> 18) & 0x7f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QHEADPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~ 0x3f) | (((nFromData) >> 19) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_NOBUFFSALLOCATED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_OVERFLOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_MINBUFFERS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 11) & 0x07); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_MAXBUFFERS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x1f) | (((nFromData) >> 9) & 0x1f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_LOCAL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QUEUEDEPTHINLINE16B(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x3f) | (((nFromData) >> 19) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ANEMICWATERMARKSEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ALLOCATEDBUFFSCNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QTAILPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~ 0x7f) | (((nFromData) >> 18) & 0x7f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QHEADPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x3f) | (((nFromData) >> 19) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_NOBUFFSALLOCATED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_OVERFLOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_MINBUFFERS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x07) | (((nFromData) >> 11) & 0x07); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_MAXBUFFERS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x1f) | (((nFromData) >> 9) & 0x1f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_LOCAL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QUEUEDEPTHINLINE16B(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x3f) | (((nFromData) >> 19) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ANEMICWATERMARKSEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_QETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ALLOCATEDBUFFSCNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[13] << 1; \
           (nToData) |= (uint32_t) (pFromData)[12] << 9; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QTAILPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[8] << 2; \
           (nToData) |= (uint32_t) (pFromData)[15] << 10; \
           (nToData) |= (uint32_t) ((pFromData)[14] & 0x7f) << 18; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QHEADPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[11] << 3; \
           (nToData) |= (uint32_t) (pFromData)[10] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x3f) << 19; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_NOBUFFSALLOCATED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_OVERFLOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_MINBUFFERS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[5] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x07) << 11; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_MAXBUFFERS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[7] << 1; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x1f) << 9; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_LOCAL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QUEUEDEPTHINLINE16B(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[2] << 3; \
           (nToData) |= (uint32_t) (pFromData)[1] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x3f) << 19; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ANEMICWATERMARKSEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ALLOCATEDBUFFSCNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[14] << 1; \
           (nToData) |= (uint32_t) (pFromData)[15] << 9; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QTAILPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[11] << 2; \
           (nToData) |= (uint32_t) (pFromData)[12] << 10; \
           (nToData) |= (uint32_t) ((pFromData)[13] & 0x7f) << 18; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QHEADPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[8] << 3; \
           (nToData) |= (uint32_t) (pFromData)[9] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x3f) << 19; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_NOBUFFSALLOCATED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_OVERFLOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_MINBUFFERS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[6] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x07) << 11; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_MAXBUFFERS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[4] << 1; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x1f) << 9; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_LOCAL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QUEUEDEPTHINLINE16B(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[1] << 3; \
           (nToData) |= (uint32_t) (pFromData)[2] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x3f) << 19; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ANEMICWATERMARKSEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ALLOCATEDBUFFSCNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[13] << 1; \
           (nToData) |= (uint32_t) (pFromData)[12] << 9; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QTAILPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[8] << 2; \
           (nToData) |= (uint32_t) (pFromData)[15] << 10; \
           (nToData) |= (uint32_t) ((pFromData)[14] & 0x7f) << 18; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QHEADPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[11] << 3; \
           (nToData) |= (uint32_t) (pFromData)[10] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x3f) << 19; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_NOBUFFSALLOCATED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_OVERFLOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_MINBUFFERS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[5] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x07) << 11; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_MAXBUFFERS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[7] << 1; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x1f) << 9; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_LOCAL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QUEUEDEPTHINLINE16B(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[2] << 3; \
           (nToData) |= (uint32_t) (pFromData)[1] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x3f) << 19; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ANEMICWATERMARKSEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ALLOCATEDBUFFSCNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[14] << 1; \
           (nToData) |= (uint32_t) (pFromData)[15] << 9; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QTAILPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[11] << 2; \
           (nToData) |= (uint32_t) (pFromData)[12] << 10; \
           (nToData) |= (uint32_t) ((pFromData)[13] & 0x7f) << 18; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QHEADPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[8] << 3; \
           (nToData) |= (uint32_t) (pFromData)[9] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x3f) << 19; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_NOBUFFSALLOCATED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_OVERFLOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_MINBUFFERS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[6] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x07) << 11; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_MAXBUFFERS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[4] << 1; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x1f) << 9; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_LOCAL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QUEUEDEPTHINLINE16B(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[1] << 3; \
           (nToData) |= (uint32_t) (pFromData)[2] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x3f) << 19; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ANEMICWATERMARKSEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_QETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUESTATEENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
