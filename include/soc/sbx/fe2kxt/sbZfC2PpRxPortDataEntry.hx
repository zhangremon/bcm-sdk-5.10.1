/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PpRxPortDataEntry.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PPRXPORTDATAENTRY_H
#define SB_ZF_C2PPRXPORTDATAENTRY_H

#define SB_ZF_C2PPRXPORTDATAENTRY_SIZE_IN_BYTES 15
#define SB_ZF_C2PPRXPORTDATAENTRY_SIZE 15
#define SB_ZF_C2PPRXPORTDATAENTRY_M_UMASK1_BITS "115:90"
#define SB_ZF_C2PPRXPORTDATAENTRY_M_UMASK0_BITS "89:58"
#define SB_ZF_C2PPRXPORTDATAENTRY_M_UDATA1_BITS "57:32"
#define SB_ZF_C2PPRXPORTDATAENTRY_M_UDATA0_BITS "31:0"


typedef struct _sbZfC2PpRxPortDataEntry {
  uint32_t m_uMask1;
  uint32_t m_uMask0;
  uint32_t m_uData1;
  uint32_t m_uData0;
} sbZfC2PpRxPortDataEntry_t;

uint32_t
sbZfC2PpRxPortDataEntry_Pack(sbZfC2PpRxPortDataEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex);
void
sbZfC2PpRxPortDataEntry_Unpack(sbZfC2PpRxPortDataEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex);
void
sbZfC2PpRxPortDataEntry_InitInstance(sbZfC2PpRxPortDataEntry_t *pFrame);

#define SB_ZF_C2PPRXPORTDATAENTRY_SET_MASK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData) >> 22) & 0x0f); \
          } while(0)

#define SB_ZF_C2PPRXPORTDATAENTRY_SET_MASK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_C2PPRXPORTDATAENTRY_SET_DATA1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 24) & 0x03); \
          } while(0)

#define SB_ZF_C2PPRXPORTDATAENTRY_SET_DATA0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPRXPORTDATAENTRY_GET_MASK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[12] << 6; \
           (nToData) |= (uint32_t) (pFromData)[13] << 14; \
           (nToData) |= (uint32_t) ((pFromData)[14] & 0x0f) << 22; \
          } while(0)

#define SB_ZF_C2PPRXPORTDATAENTRY_GET_MASK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[8] << 6; \
           (nToData) |= (uint32_t) (pFromData)[9] << 14; \
           (nToData) |= (uint32_t) (pFromData)[10] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[11] & 0x03) << 30; \
          } while(0)

#define SB_ZF_C2PPRXPORTDATAENTRY_GET_DATA1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x03) << 24; \
          } while(0)

#define SB_ZF_C2PPRXPORTDATAENTRY_GET_DATA0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
