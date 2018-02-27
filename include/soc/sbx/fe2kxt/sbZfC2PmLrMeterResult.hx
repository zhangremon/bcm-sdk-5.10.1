/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PmLrMeterResult.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PMLRMETERRESULT_H
#define SB_ZF_C2PMLRMETERRESULT_H

#define SB_ZF_C2PMLRMETERRESULT_SIZE_IN_BYTES 4
#define SB_ZF_C2PMLRMETERRESULT_SIZE 4
#define SB_ZF_C2PMLRMETERRESULT_M_UDROP_BITS "31:31"
#define SB_ZF_C2PMLRMETERRESULT_M_UINVALIDPOLCIERID_BITS "30:30"
#define SB_ZF_C2PMLRMETERRESULT_M_UMMUERROR_BITS "29:29"
#define SB_ZF_C2PMLRMETERRESULT_M_URESV_BITS "28:27"
#define SB_ZF_C2PMLRMETERRESULT_M_UPOLICERID_BITS "26:4"
#define SB_ZF_C2PMLRMETERRESULT_M_UPMUDP_BITS "3:2"
#define SB_ZF_C2PMLRMETERRESULT_M_ULRPDP_BITS "1:0"


typedef struct _sbZfC2PmLrMeterResult {
  uint8_t m_uDrop;
  uint32_t m_uInvalidPolcierId;
  uint32_t m_uMmuError;
  uint32_t m_uResv;
  uint32_t m_uPolicerId;
  uint8_t m_uPmuDp;
  uint8_t m_uLrpDp;
} sbZfC2PmLrMeterResult_t;

uint32_t
sbZfC2PmLrMeterResult_Pack(sbZfC2PmLrMeterResult_t *pFrom,
                           uint8_t *pToData,
                           uint32_t nMaxToDataIndex);
void
sbZfC2PmLrMeterResult_Unpack(sbZfC2PmLrMeterResult_t *pToStruct,
                             uint8_t *pFromData,
                             uint32_t nMaxToDataIndex);
void
sbZfC2PmLrMeterResult_InitInstance(sbZfC2PmLrMeterResult_t *pFrame);

#define SB_ZF_C2PMLRMETERRESULT_SET_DRP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_SET_INVPOLID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_SET_MMERR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_SET_RESV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 3)) | (((nFromData) & 0x03) << 3); \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_SET_PID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x07) | (((nFromData) >> 20) & 0x07); \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_SET_PMDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 2)) | (((nFromData) & 0x03) << 2); \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_SET_LDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_GET_DRP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_GET_INVPOLID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_GET_MMERR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_GET_RESV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 3) & 0x03; \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_GET_PID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 4; \
           (nToData) |= (uint32_t) (pFromData)[2] << 12; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x07) << 20; \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_GET_PMDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 2) & 0x03; \
          } while(0)

#define SB_ZF_C2PMLRMETERRESULT_GET_LDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x03; \
          } while(0)

#endif
