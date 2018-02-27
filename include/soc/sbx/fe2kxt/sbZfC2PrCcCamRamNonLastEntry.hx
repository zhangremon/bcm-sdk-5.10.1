/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PrCcCamRamNonLastEntry.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PRCCCAMRAMNONLASTENTRY_H
#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_H

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SIZE_IN_BYTES 10
#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SIZE 10
#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_M_URESERVE3_BITS "76:64"
#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_M_URESERVE2_BITS "63:32"
#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_M_URESERVE1_BITS "31:22"
#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_M_USHIFT_BITS "21:16"
#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_M_UNEXTSTATE_BITS "15:2"
#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_M_UE2ECC_BITS "1:1"
#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_M_ULAST_BITS "0:0"


typedef struct _sbZfC2PrCcCamRamNonLastEntry {
  uint32_t m_uReserve3;
  uint32_t m_uReserve2;
  uint32_t m_uReserve1;
  uint32_t m_uShift;
  uint32_t m_uNextState;
  uint8_t m_uE2ECC;
  uint8_t m_uLast;
} sbZfC2PrCcCamRamNonLastEntry_t;

uint32_t
sbZfC2PrCcCamRamNonLastEntry_Pack(sbZfC2PrCcCamRamNonLastEntry_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex);
void
sbZfC2PrCcCamRamNonLastEntry_Unpack(sbZfC2PrCcCamRamNonLastEntry_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex);
void
sbZfC2PrCcCamRamNonLastEntry_InitInstance(sbZfC2PrCcCamRamNonLastEntry_t *pFrame);

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SET_RESERVE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~ 0x1f) | (((nFromData) >> 8) & 0x1f); \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SET_RESERVE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SET_RESERVE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SET_SHIFT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SET_NEXTSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SET_E2ECC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SET_LAST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_GET_RESERVE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x1f) << 8; \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_GET_RESERVE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
           (nToData) |= (uint32_t) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_GET_RESERVE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[3] << 2; \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_GET_SHIFT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x3f; \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_GET_NEXTSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 6; \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_GET_E2ECC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PRCCCAMRAMNONLASTENTRY_GET_LAST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
