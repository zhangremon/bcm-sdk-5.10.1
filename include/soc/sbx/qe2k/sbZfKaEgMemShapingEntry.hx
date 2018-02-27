/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgMemShapingEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAEGMEMSHAPINGENTRY_H
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_H

#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_SIZE_IN_BYTES 10
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_SIZE 10
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_M_NRESERVED_BITS "79:77"
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_M_NBUCKETDEPTH_BITS "76:53"
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_M_NSHAPERATE_BITS "52:29"
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_M_NMAXDEPTH_BITS "28:14"
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_M_NPORT_BITS "13:8"
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_M_NHISIDE_BITS "7:7"
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_M_NSHAPESRC_BITS "6:1"
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_M_NENABLE_BITS "0:0"
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_KA_EG_SHAPER_INTERVAL             (1e09/9728.0)


typedef struct _sbZfKaEgMemShapingEntry {
  uint32_t m_nReserved;
  uint32_t m_nBucketDepth;
  uint32_t m_nShapeRate;
  uint32_t m_nMaxDepth;
  uint32_t m_nPort;
  uint32_t m_nHiSide;
  uint32_t m_nShapeSrc;
  uint32_t m_nEnable;
} sbZfKaEgMemShapingEntry_t;

uint32_t
sbZfKaEgMemShapingEntry_Pack(sbZfKaEgMemShapingEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex);
void
sbZfKaEgMemShapingEntry_Unpack(sbZfKaEgMemShapingEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex);
void
sbZfKaEgMemShapingEntry_InitInstance(sbZfKaEgMemShapingEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_BUCKETDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x1f) | (((nFromData) >> 19) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_SHAPERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x1f) | (((nFromData) >> 19) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_MAXDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x1f) | (((nFromData) >> 10) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_HISIDE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_SHAPESRC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_BUCKETDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~ 0x1f) | (((nFromData) >> 19) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_SHAPERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x1f) | (((nFromData) >> 19) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_MAXDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x1f) | (((nFromData) >> 10) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_HISIDE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_SHAPESRC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_BUCKETDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x1f) | (((nFromData) >> 19) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_SHAPERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x1f) | (((nFromData) >> 19) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_MAXDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x1f) | (((nFromData) >> 10) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_HISIDE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_SHAPESRC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_BUCKETDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~ 0x1f) | (((nFromData) >> 19) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_SHAPERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x1f) | (((nFromData) >> 19) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_MAXDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x1f) | (((nFromData) >> 10) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_HISIDE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_SHAPESRC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_BUCKETDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[4] << 3; \
           (nToData) |= (uint32_t) (pFromData)[11] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x1f) << 19; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_SHAPERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[7] << 3; \
           (nToData) |= (uint32_t) (pFromData)[6] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x1f) << 19; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_MAXDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[1] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x1f) << 10; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x3f; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_HISIDE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_SHAPESRC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_BUCKETDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[7] << 3; \
           (nToData) |= (uint32_t) (pFromData)[8] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x1f) << 19; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_SHAPERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[4] << 3; \
           (nToData) |= (uint32_t) (pFromData)[5] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x1f) << 19; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_MAXDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[2] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x1f) << 10; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x3f; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_HISIDE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_SHAPESRC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_BUCKETDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[4] << 3; \
           (nToData) |= (uint32_t) (pFromData)[11] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x1f) << 19; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_SHAPERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[7] << 3; \
           (nToData) |= (uint32_t) (pFromData)[6] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x1f) << 19; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_MAXDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[1] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x1f) << 10; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x3f; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_HISIDE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_SHAPESRC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_BUCKETDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[7] << 3; \
           (nToData) |= (uint32_t) (pFromData)[8] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x1f) << 19; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_SHAPERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[4] << 3; \
           (nToData) |= (uint32_t) (pFromData)[5] << 11; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x1f) << 19; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_MAXDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[2] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x1f) << 10; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x3f; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_HISIDE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_SHAPESRC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAEGMEMSHAPINGENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
