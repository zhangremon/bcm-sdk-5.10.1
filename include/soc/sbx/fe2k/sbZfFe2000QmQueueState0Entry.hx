/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000QmQueueState0Entry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_FE2000QMQUEUESTATE0ENTRY_H
#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_H

#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_SIZE 4
#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_M_UTAILPTR_BITS "26:15"
#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_M_UDEPTHINPAGES_BITS "14:2"
#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_M_UEMPTY_BITS "1:1"
#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_M_UENABLE_BITS "0:0"


typedef struct _sbZfFe2000QmQueueState0Entry {
  uint32_t m_uTailPtr;
  uint32_t m_uDepthInPages;
  uint32_t m_uEmpty;
  uint32_t m_uEnable;
} sbZfFe2000QmQueueState0Entry_t;

uint32_t
sbZfFe2000QmQueueState0Entry_Pack(sbZfFe2000QmQueueState0Entry_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex);
void
sbZfFe2000QmQueueState0Entry_Unpack(sbZfFe2000QmQueueState0Entry_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex);
void
sbZfFe2000QmQueueState0Entry_InitInstance(sbZfFe2000QmQueueState0Entry_t *pFrame);

#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_SET_TAILPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x07) | (((nFromData) >> 9) & 0x07); \
          } while(0)

#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_SET_DEPTHINPAGES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 6) & 0x7f); \
          } while(0)

#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_SET_EMPTY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_GET_TAILPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[2] << 1; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x07) << 9; \
          } while(0)

#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_GET_DEPTHINPAGES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x7f) << 6; \
          } while(0)

#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_GET_EMPTY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FE2000QMQUEUESTATE0ENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
