/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2QmQueueConfigEntry.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2QMQUEUECONFIGENTRY_H
#define SB_ZF_C2QMQUEUECONFIGENTRY_H

#define SB_ZF_C2QMQUEUECONFIGENTRY_SIZE_IN_BYTES 11
#define SB_ZF_C2QMQUEUECONFIGENTRY_SIZE 11
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UENFORCEPOLICEMARKINGS_BITS "84:84"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UDROPTHRESH2_BITS "83:72"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UDROPTHRESH1_BITS "71:60"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UFLOWCONTROLTHRESH2_BITS "59:48"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UFLOWCONTROLTHRESH1_BITS "47:36"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UFLOWCONTROLENABLE_BITS "35:35"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UFLOWCONTROLTARGET_BITS "34:34"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UFLOWCONTROLPORT_BITS "33:25"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UMAXPAGES_BITS "24:13"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UMINPAGES_BITS "12:1"
#define SB_ZF_C2QMQUEUECONFIGENTRY_M_UENABLE_BITS "0:0"


typedef struct _sbZfC2QmQueueConfigEntry {
  uint32_t m_uEnforcePoliceMarkings;
  uint32_t m_uDropThresh2;
  uint32_t m_uDropThresh1;
  uint32_t m_uFlowControlThresh2;
  uint32_t m_uFlowControlThresh1;
  uint32_t m_uFlowControlEnable;
  uint32_t m_uFlowControlTarget;
  uint32_t m_uFlowControlPort;
  uint32_t m_uMaxPages;
  uint32_t m_uMinPages;
  uint32_t m_uEnable;
} sbZfC2QmQueueConfigEntry_t;

uint32_t
sbZfC2QmQueueConfigEntry_Pack(sbZfC2QmQueueConfigEntry_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex);
void
sbZfC2QmQueueConfigEntry_Unpack(sbZfC2QmQueueConfigEntry_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex);
void
sbZfC2QmQueueConfigEntry_InitInstance(sbZfC2QmQueueConfigEntry_t *pFrame);

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_ENFORCEPOLICEMARKINGS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_DROPTHRESH2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_DROPTHRESH1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_FLOWCONTROLTHRESH2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_FLOWCONTROLTHRESH1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_FLOWCONTROLENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_FLOWCONTROLTARGET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_FLOWCONTROLPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 7) & 0x03); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_MAXPAGES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 11) & 0x01); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_MINPAGES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[1] = ((pToData)[1] & ~ 0x1f) | (((nFromData) >> 7) & 0x1f); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_ENFORCEPOLICEMARKINGS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_DROPTHRESH2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[9] ; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_DROPTHRESH1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[8] << 4; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_FLOWCONTROLTHRESH2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[6] ; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_FLOWCONTROLTHRESH1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[5] << 4; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_FLOWCONTROLENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_FLOWCONTROLTARGET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_FLOWCONTROLPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x03) << 7; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_MAXPAGES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[2] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x01) << 11; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_MINPAGES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x1f) << 7; \
          } while(0)

#define SB_ZF_C2QMQUEUECONFIGENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
