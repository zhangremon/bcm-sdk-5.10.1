/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PpFilterDataOverlayFormat.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_H
#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_H

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_SIZE_IN_BYTES 2
#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_SIZE 2
#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_M_UFRAGMENT_BITS "15:15"
#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_M_ULOCALSTATIONID_BITS "14:10"
#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_M_UFILTERMATCH0_BITS "9:9"
#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_M_UFILTERMATCH1_BITS "8:8"
#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_M_UFILTERID0_BITS "7:4"
#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_M_UFILTERID1_BITS "3:0"


typedef struct _sbZfC2PpFilterDataOverlayFormat {
  uint8_t m_uFragment;
  uint8_t m_uLocalStationId;
  uint8_t m_uFilterMatch0;
  uint8_t m_uFilterMatch1;
  uint8_t m_uFilterId0;
  uint8_t m_uFilterId1;
} sbZfC2PpFilterDataOverlayFormat_t;

uint32_t
sbZfC2PpFilterDataOverlayFormat_Pack(sbZfC2PpFilterDataOverlayFormat_t *pFrom,
                                     uint8_t *pToData,
                                     uint32_t nMaxToDataIndex);
void
sbZfC2PpFilterDataOverlayFormat_Unpack(sbZfC2PpFilterDataOverlayFormat_t *pToStruct,
                                       uint8_t *pFromData,
                                       uint32_t nMaxToDataIndex);
void
sbZfC2PpFilterDataOverlayFormat_InitInstance(sbZfC2PpFilterDataOverlayFormat_t *pFrame);

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_SET_FRAG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_SET_LSTAID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_SET_FM0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_SET_FM1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_SET_FID0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_SET_FID1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_GET_FRAG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_GET_LSTAID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_GET_FM0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_GET_FM1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_GET_FID0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPFILTERDATAOVERLAYFORMAT_GET_FID1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
