/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPtPte4MirrorPortStateEntry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_H
#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_H

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_SIZE_IN_BYTES 7
#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_SIZE 7
#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_M_UHDRPRESENT_BITS "49:49"
#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_M_UPKTLENGTH_BITS "48:35"
#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_M_UNUMPAGESHI_BITS "34:32"
#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_M_UNUMPAGESLO_BITS "31:28"
#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_M_UBUFFER_BITS "27:16"
#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_M_USOURCEQUEUE_BITS "15:8"
#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_M_UQUEUE_BITS "7:0"


typedef struct _sbZfCaPtPte4MirrorPortStateEntry {
  uint32_t m_uHdrPresent;
  uint32_t m_uPktLength;
  uint32_t m_uNumPagesHi;
  uint32_t m_uNumPagesLo;
  uint32_t m_uBuffer;
  uint32_t m_uSourceQueue;
  uint32_t m_uQueue;
} sbZfCaPtPte4MirrorPortStateEntry_t;

uint32_t
sbZfCaPtPte4MirrorPortStateEntry_Pack(sbZfCaPtPte4MirrorPortStateEntry_t *pFrom,
                                      uint8_t *pToData,
                                      uint32_t nMaxToDataIndex);
void
sbZfCaPtPte4MirrorPortStateEntry_Unpack(sbZfCaPtPte4MirrorPortStateEntry_t *pToStruct,
                                        uint8_t *pFromData,
                                        uint32_t nMaxToDataIndex);
void
sbZfCaPtPte4MirrorPortStateEntry_InitInstance(sbZfCaPtPte4MirrorPortStateEntry_t *pFrame);

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_SET_HDRPRESENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_SET_PKTLENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x01) | (((nFromData) >> 13) & 0x01); \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_SET_NUMPAGESHI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_SET_NUMPAGESLO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_SET_BUFFER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_SET_SOURCEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_SET_QUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_GET_HDRPRESENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_GET_PKTLENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) (pFromData)[5] << 5; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x01) << 13; \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_GET_NUMPAGESHI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4]) & 0x07; \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_GET_NUMPAGESLO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_GET_BUFFER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_GET_SOURCEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_CAPTPTE4MIRRORPORTSTATEENTRY_GET_QUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
