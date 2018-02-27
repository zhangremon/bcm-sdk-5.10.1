/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PmLrMeterRequest.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PMLRMETERREQUEST_H
#define SB_ZF_C2PMLRMETERREQUEST_H

#define SB_ZF_C2PMLRMETERREQUEST_SIZE_IN_BYTES 8
#define SB_ZF_C2PMLRMETERREQUEST_SIZE 8
#define SB_ZF_C2PMLRMETERREQUEST_M_URESV0_BITS "63:55"
#define SB_ZF_C2PMLRMETERREQUEST_M_UPOLICERID_BITS "54:32"
#define SB_ZF_C2PMLRMETERREQUEST_M_URESV1_BITS "31:18"
#define SB_ZF_C2PMLRMETERREQUEST_M_UBLIND_BITS "17:17"
#define SB_ZF_C2PMLRMETERREQUEST_M_UDROPONRED_BITS "16:16"
#define SB_ZF_C2PMLRMETERREQUEST_M_UPACKETLENGTH_BITS "15:2"
#define SB_ZF_C2PMLRMETERREQUEST_M_ULRPDP_BITS "1:0"


typedef struct _sbZfC2PmLrMeterRequest {
  uint32_t m_uResv0;
  uint32_t m_uPolicerId;
  uint32_t m_uResv1;
  uint8_t m_uBlind;
  uint8_t m_uDropOnRed;
  uint32_t m_uPacketLength;
  uint8_t m_uLrpDp;
} sbZfC2PmLrMeterRequest_t;

uint32_t
sbZfC2PmLrMeterRequest_Pack(sbZfC2PmLrMeterRequest_t *pFrom,
                            uint8_t *pToData,
                            uint32_t nMaxToDataIndex);
void
sbZfC2PmLrMeterRequest_Unpack(sbZfC2PmLrMeterRequest_t *pToStruct,
                              uint8_t *pFromData,
                              uint32_t nMaxToDataIndex);
void
sbZfC2PmLrMeterRequest_InitInstance(sbZfC2PmLrMeterRequest_t *pFrame);

#define SB_ZF_C2PMLRMETERREQUEST_SET_RESV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_SET_PID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_SET_RESV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_SET_BLND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_SET_DRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_SET_PLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_SET_LDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_GET_RESV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[7] << 1; \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_GET_PID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x7f) << 16; \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_GET_RESV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[3] << 6; \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_GET_BLND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_GET_DRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_GET_PLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 6; \
          } while(0)

#define SB_ZF_C2PMLRMETERREQUEST_GET_LDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x03; \
          } while(0)

#endif
