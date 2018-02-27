/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPtPte2PortStateEntry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAPTPTE2PORTSTATEENTRY_H
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_H

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SIZE_IN_BYTES 12
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SIZE 12
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UPKTINPROGRESS_BITS "92:92"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UMIRROR_BITS "91:91"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UMIRRORSRCDROP_BITS "90:90"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UDOUBLEDEQUEUED_BITS "89:89"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UEOPEXTRABURST_BITS "88:88"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_URESIDUEBYTE_BITS "87:84"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UPKTREQUESTSTATE_BITS "83:80"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_ULINE_BITS "79:79"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_ULASTPAGE_BITS "78:78"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UNEXTPAGESTATE_BITS "77:76"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UNEXTPAGE_BITS "75:64"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UNEXTLENGTH_BITS "63:56"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UPAGE_BITS "55:44"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_USAVEDBYTES_BITS "43:40"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UREMAININGBYTES_BITS "39:32"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UEOP_BITS "31:31"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_USOP_BITS "30:30"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UPKTLENGTH_BITS "29:16"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_UCONTINUEBYTE_BITS "15:8"
#define SB_ZF_CAPTPTE2PORTSTATEENTRY_M_USOURCEQUEUE_BITS "7:0"


typedef struct _sbZfCaPtPte2PortStateEntry {
  uint32_t m_uPktInProgress;
  uint32_t m_uMirror;
  uint32_t m_uMirrorSrcDrop;
  uint32_t m_uDoubleDequeued;
  uint32_t m_uEopExtraBurst;
  uint32_t m_uResidueByte;
  uint32_t m_uPktRequestState;
  uint32_t m_uLine;
  uint32_t m_uLastPage;
  uint32_t m_uNextPageState;
  uint32_t m_uNextPage;
  uint32_t m_uNextLength;
  uint32_t m_uPage;
  uint32_t m_uSavedBytes;
  uint32_t m_uRemainingBytes;
  uint32_t m_uEop;
  uint32_t m_uSop;
  uint32_t m_uPktLength;
  uint32_t m_uContinueByte;
  uint32_t m_uSourceQueue;
} sbZfCaPtPte2PortStateEntry_t;

uint32_t
sbZfCaPtPte2PortStateEntry_Pack(sbZfCaPtPte2PortStateEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex);
void
sbZfCaPtPte2PortStateEntry_Unpack(sbZfCaPtPte2PortStateEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex);
void
sbZfCaPtPte2PortStateEntry_InitInstance(sbZfCaPtPte2PortStateEntry_t *pFrame);

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_PKTINPROGRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_MIRROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_MIRRORSRCDROP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_DOUBLEDEQUEUED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_EOPEXTRABURST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_RESIDUEBYTE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_PKTREQUESTSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_LINE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_LASTPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_NEXTPAGESTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_NEXTPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_NEXTLENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_PAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_SAVEDBYTES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_REMAININGBYTES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_EOP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_SOP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_PKTLENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_CONTINUEBYTE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_SET_SOURCEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_PKTINPROGRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_MIRROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_MIRRORSRCDROP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_DOUBLEDEQUEUED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_EOPEXTRABURST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11]) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_RESIDUEBYTE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_PKTREQUESTSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10]) & 0x0f; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_LINE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_LASTPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_NEXTPAGESTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_NEXTPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_NEXTLENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_PAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[6] << 4; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_SAVEDBYTES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_REMAININGBYTES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_EOP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_SOP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_PKTLENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x3f) << 8; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_CONTINUEBYTE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_CAPTPTE2PORTSTATEENTRY_GET_SOURCEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
