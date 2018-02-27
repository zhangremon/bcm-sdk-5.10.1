/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PtQueueToPortEntry.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PTQUEUETOPORTENTRY_H
#define SB_ZF_C2PTQUEUETOPORTENTRY_H

#define SB_ZF_C2PTQUEUETOPORTENTRY_SIZE_IN_BYTES 3
#define SB_ZF_C2PTQUEUETOPORTENTRY_SIZE 3
#define SB_ZF_C2PTQUEUETOPORTENTRY_M_UPBSEL_BITS "19:19"
#define SB_ZF_C2PTQUEUETOPORTENTRY_M_UPPEBOUND_BITS "18:18"
#define SB_ZF_C2PTQUEUETOPORTENTRY_M_UPPEENQUEUE_BITS "17:17"
#define SB_ZF_C2PTQUEUETOPORTENTRY_M_UDESTINTERFACE_BITS "16:14"
#define SB_ZF_C2PTQUEUETOPORTENTRY_M_UDESTPORT_BITS "13:8"
#define SB_ZF_C2PTQUEUETOPORTENTRY_M_UDESTQUEUE_BITS "7:0"


typedef struct _sbZfC2PtQueueToPortEntry {
  uint32_t m_uPbSel;
  uint32_t m_uPpeBound;
  uint32_t m_uPpeEnqueue;
  uint32_t m_uDestInterface;
  uint32_t m_uDestPort;
  uint32_t m_uDestQueue;
} sbZfC2PtQueueToPortEntry_t;

uint32_t
sbZfC2PtQueueToPortEntry_Pack(sbZfC2PtQueueToPortEntry_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex);
void
sbZfC2PtQueueToPortEntry_Unpack(sbZfC2PtQueueToPortEntry_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex);
void
sbZfC2PtQueueToPortEntry_InitInstance(sbZfC2PtQueueToPortEntry_t *pFrame);

#define SB_ZF_C2PTQUEUETOPORTENTRY_SET_PBSEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_SET_PPEBOUND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_SET_PPEENQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_SET_DESTINTERFACE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 2) & 0x01); \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_SET_DESTPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_SET_DESTQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_GET_PBSEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_GET_PPEBOUND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_GET_PPEENQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_GET_DESTINTERFACE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 2; \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_GET_DESTPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x3f; \
          } while(0)

#define SB_ZF_C2PTQUEUETOPORTENTRY_GET_DESTQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
