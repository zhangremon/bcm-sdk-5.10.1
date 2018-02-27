/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PrCcTcamConfigEntry.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PRCCTCAMCONFIGENTRY_H
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_H

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES 54
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE 54
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UMASK213_192_BITS "429:408"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UMASK191_160_BITS "407:376"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UMASK159_128_BITS "375:344"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UMASK127_96_BITS "343:312"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UMASK95_64_BITS "311:280"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UMASK63_32_BITS "279:248"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UMASK31_0_BITS "247:216"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UDATA213_192_BITS "215:194"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UDATA191_160_BITS "193:162"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UDATA159_128_BITS "161:130"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UDATA127_96_BITS "129:98"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UDATA95_64_BITS "97:66"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UDATA63_32_BITS "65:34"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UDATA31_0_BITS "33:2"
#define SB_ZF_C2PRCCTCAMCONFIGENTRY_M_UVALID_BITS "1:0"



#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS ((SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES+3)/4)

typedef struct _sbZfC2PrCcTcamConfigEntry {
  uint32_t m_uMask213_192;
  uint32_t m_uMask191_160;
  uint32_t m_uMask159_128;
  uint32_t m_uMask127_96;
  uint32_t m_uMask95_64;
  uint32_t m_uMask63_32;
  uint32_t m_uMask31_0;
  uint32_t m_uData213_192;
  uint32_t m_uData191_160;
  uint32_t m_uData159_128;
  uint32_t m_uData127_96;
  uint32_t m_uData95_64;
  uint32_t m_uData63_32;
  uint32_t m_uData31_0;
  uint32_t m_uValid;
} sbZfC2PrCcTcamConfigEntry_t;

uint32_t
sbZfC2PrCcTcamConfigEntry_Pack(sbZfC2PrCcTcamConfigEntry_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex);
void
sbZfC2PrCcTcamConfigEntry_Unpack(sbZfC2PrCcTcamConfigEntry_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex);
void
sbZfC2PrCcTcamConfigEntry_InitInstance(sbZfC2PrCcTcamConfigEntry_t *pFrame);

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_MASK213_192(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[51] = ((nFromData)) & 0xFF; \
           (pToData)[52] = ((pToData)[52] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[53] = ((pToData)[53] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_MASK191_160(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[47] = ((nFromData)) & 0xFF; \
           (pToData)[48] = ((pToData)[48] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[49] = ((pToData)[49] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[50] = ((pToData)[50] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_MASK159_128(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[43] = ((nFromData)) & 0xFF; \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_MASK127_96(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[39] = ((nFromData)) & 0xFF; \
           (pToData)[40] = ((pToData)[40] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_MASK95_64(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[35] = ((nFromData)) & 0xFF; \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_MASK63_32(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((nFromData)) & 0xFF; \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_MASK31_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[27] = ((nFromData)) & 0xFF; \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_DATA213_192(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((pToData)[24] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_DATA191_160(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((pToData)[20] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_DATA159_128(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_DATA127_96(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_DATA95_64(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_DATA63_32(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_DATA31_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_SET_VALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_MASK213_192(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[51] ; \
           (nToData) |= (uint32_t) (pFromData)[52] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[53] & 0x3f) << 16; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_MASK191_160(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[47] ; \
           (nToData) |= (uint32_t) (pFromData)[48] << 8; \
           (nToData) |= (uint32_t) (pFromData)[49] << 16; \
           (nToData) |= (uint32_t) (pFromData)[50] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_MASK159_128(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[43] ; \
           (nToData) |= (uint32_t) (pFromData)[44] << 8; \
           (nToData) |= (uint32_t) (pFromData)[45] << 16; \
           (nToData) |= (uint32_t) (pFromData)[46] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_MASK127_96(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[39] ; \
           (nToData) |= (uint32_t) (pFromData)[40] << 8; \
           (nToData) |= (uint32_t) (pFromData)[41] << 16; \
           (nToData) |= (uint32_t) (pFromData)[42] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_MASK95_64(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[35] ; \
           (nToData) |= (uint32_t) (pFromData)[36] << 8; \
           (nToData) |= (uint32_t) (pFromData)[37] << 16; \
           (nToData) |= (uint32_t) (pFromData)[38] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_MASK63_32(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[31] ; \
           (nToData) |= (uint32_t) (pFromData)[32] << 8; \
           (nToData) |= (uint32_t) (pFromData)[33] << 16; \
           (nToData) |= (uint32_t) (pFromData)[34] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_MASK31_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[27] ; \
           (nToData) |= (uint32_t) (pFromData)[28] << 8; \
           (nToData) |= (uint32_t) (pFromData)[29] << 16; \
           (nToData) |= (uint32_t) (pFromData)[30] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_DATA213_192(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[24] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[25] << 6; \
           (nToData) |= (uint32_t) (pFromData)[26] << 14; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_DATA191_160(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[20] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[21] << 6; \
           (nToData) |= (uint32_t) (pFromData)[22] << 14; \
           (nToData) |= (uint32_t) (pFromData)[23] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[24] & 0x03) << 30; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_DATA159_128(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[17] << 6; \
           (nToData) |= (uint32_t) (pFromData)[18] << 14; \
           (nToData) |= (uint32_t) (pFromData)[19] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[20] & 0x03) << 30; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_DATA127_96(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[13] << 6; \
           (nToData) |= (uint32_t) (pFromData)[14] << 14; \
           (nToData) |= (uint32_t) (pFromData)[15] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[16] & 0x03) << 30; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_DATA95_64(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[9] << 6; \
           (nToData) |= (uint32_t) (pFromData)[10] << 14; \
           (nToData) |= (uint32_t) (pFromData)[11] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[12] & 0x03) << 30; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_DATA63_32(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[5] << 6; \
           (nToData) |= (uint32_t) (pFromData)[6] << 14; \
           (nToData) |= (uint32_t) (pFromData)[7] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[8] & 0x03) << 30; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_DATA31_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 6; \
           (nToData) |= (uint32_t) (pFromData)[2] << 14; \
           (nToData) |= (uint32_t) (pFromData)[3] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x03) << 30; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRY_GET_VALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x03; \
          } while(0)

#endif
