/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPmProfileMemoryEntry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAPMPROFILEMEMORYENTRY_H
#define SB_ZF_CAPMPROFILEMEMORYENTRY_H

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SIZE_IN_BYTES 8
#define SB_ZF_CAPMPROFILEMEMORYENTRY_SIZE 8
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UTYPE2_BITS "62:62"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UDROPONRED_BITS "61:61"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UEXCESSBUCKETNODECREMENT_BITS "60:60"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UCOMMITTEDBUCKETNODECREMENT_BITS "59:59"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_ULENGTHSHIFT_BITS "58:56"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UEXCESSINFORMATIONRATE_BITS "55:44"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UCOMMITTEDINFORMATIONRATE_BITS "43:32"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UCOUPLINGFLAG_BITS "31:31"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_URFCMODE_BITS "30:30"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UCOLORBLIND_BITS "29:29"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UCOMMITEDBUCKETSIZE_BITS "28:24"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UEXCESSBURSTSIZE_BITS "23:12"
#define SB_ZF_CAPMPROFILEMEMORYENTRY_M_UCOMMITTEDBURSTSIZE_BITS "11:0"


typedef struct _sbZfCaPmProfileMemoryEntry {
  uint32_t m_uType2;
  uint32_t m_uDropOnRed;
  uint32_t m_uExcessBucketNoDecrement;
  uint32_t m_uCommittedBucketNoDecrement;
  uint32_t m_uLengthShift;
  uint32_t m_uExcessInformationRate;
  uint32_t m_uCommittedInformationRate;
  uint32_t m_uCouplingFlag;
  uint32_t m_uRfcMode;
  uint32_t m_uColorBlind;
  uint32_t m_uCommitedBucketSize;
  uint32_t m_uExcessBurstSize;
  uint32_t m_uCommittedBurstSize;
} sbZfCaPmProfileMemoryEntry_t;

uint32_t
sbZfCaPmProfileMemoryEntry_Pack(sbZfCaPmProfileMemoryEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex);
void
sbZfCaPmProfileMemoryEntry_Unpack(sbZfCaPmProfileMemoryEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex);
void
sbZfCaPmProfileMemoryEntry_InitInstance(sbZfCaPmProfileMemoryEntry_t *pFrame);

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_TYPE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_DROPONRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_EXCESSBUCKETNODECREMENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_COMMITTEDBUCKETNODECREMENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_LENGTHSHIFT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_EXCESSINFORMATIONRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_COMMITTEDINFORMATIONRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_COUPLINGFLAG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_RFCMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_COLORBLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_COMMITEDBUCKETSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_EXCESSBURSTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_SET_COMMITTEDBURSTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_TYPE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_DROPONRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_EXCESSBUCKETNODECREMENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_COMMITTEDBUCKETNODECREMENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_LENGTHSHIFT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7]) & 0x07; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_EXCESSINFORMATIONRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[6] << 4; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_COMMITTEDINFORMATIONRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_COUPLINGFLAG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_RFCMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_COLORBLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_COMMITEDBUCKETSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x1f; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_EXCESSBURSTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[2] << 4; \
          } while(0)

#define SB_ZF_CAPMPROFILEMEMORYENTRY_GET_COMMITTEDBURSTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
