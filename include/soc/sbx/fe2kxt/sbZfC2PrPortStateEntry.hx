/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PrPortStateEntry.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFC2PRPORTSTATEENTRY_H
#define SB_ZF_ZFC2PRPORTSTATEENTRY_H

#define SB_ZF_ZFC2PRPORTSTATEENTRY_SIZE_IN_BYTES 7
#define SB_ZF_ZFC2PRPORTSTATEENTRY_SIZE 7
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_NRESERVED_BITS "55:50"
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_BTESTPKT_BITS "49:49"
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_NBUFFER_BITS "48:37"
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_NLENGTH_BITS "36:26"
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_NNUMPAGES_BITS "25:19"
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_BSTARTOFPKT_BITS "18:18"
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_NPAGEADDR_BITS "17:6"
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_NLINEADDR_BITS "5:5"
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_NWORDNUMBER_BITS "4:1"
#define SB_ZF_ZFC2PRPORTSTATEENTRY_M_BDROPPKT_BITS "0:0"


typedef struct _sbZfC2PrPortStateEntry {
  uint32_t m_nReserved;
  uint32_t m_bTestPkt;
  uint32_t m_nBuffer;
  uint32_t m_nLength;
  uint32_t m_nNumPages;
  uint32_t m_bStartOfPkt;
  uint32_t m_nPageAddr;
  uint32_t m_nLineAddr;
  uint32_t m_nWordNumber;
  uint32_t m_bDropPkt;
} sbZfC2PrPortStateEntry_t;

uint32_t
sbZfC2PrPortStateEntry_Pack(sbZfC2PrPortStateEntry_t *pFrom,
                            uint8_t *pToData,
                            uint32_t nMaxToDataIndex);
void
sbZfC2PrPortStateEntry_Unpack(sbZfC2PrPortStateEntry_t *pToStruct,
                              uint8_t *pFromData,
                              uint32_t nMaxToDataIndex);
void
sbZfC2PrPortStateEntry_InitInstance(sbZfC2PrPortStateEntry_t *pFrame);

#define SB_ZF_C2PRPORTSTATEENTRY_SET_RESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_SET_TESTPKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_SET_BUFFER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x01) | (((nFromData) >> 11) & 0x01); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_SET_LENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[4] = ((pToData)[4] & ~ 0x1f) | (((nFromData) >> 6) & 0x1f); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_SET_NPAGES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 5) & 0x03); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_SET_SOP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_SET_PAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 10) & 0x03); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_SET_LINE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_SET_WORD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_SET_DROP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_RESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_TESTPKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_BUFFER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[5] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x01) << 11; \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_LENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x1f) << 6; \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_NPAGES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x03) << 5; \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_SOP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_PAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[1] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 10; \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_LINE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_WORD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_C2PRPORTSTATEENTRY_GET_DROP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
