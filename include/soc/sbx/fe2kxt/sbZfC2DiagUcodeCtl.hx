/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2DiagUcodeCtl.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2DIAGUCODECTL_H
#define SB_ZF_C2DIAGUCODECTL_H

#define SB_ZF_C2DIAGUCODECTL_SIZE_IN_BYTES 4
#define SB_ZF_C2DIAGUCODECTL_SIZE 4
#define SB_ZF_C2DIAGUCODECTL_M_ULSTATUS_BITS "0:0"
#define SB_ZF_C2DIAGUCODECTL_M_ULERRINJECT0_BITS "1:1"
#define SB_ZF_C2DIAGUCODECTL_M_ULERRINJECT1_BITS "2:2"
#define SB_ZF_C2DIAGUCODECTL_M_ULERRIND_BITS "3:3"
#define SB_ZF_C2DIAGUCODECTL_M_ULUCLOADED_BITS "4:4"
#define SB_ZF_C2DIAGUCODECTL_M_ULLRPSTATE_BITS "6:5"
#define SB_ZF_C2DIAGUCODECTL_M_ULMEMEXSTMM0N0_BITS "7:7"
#define SB_ZF_C2DIAGUCODECTL_M_ULMEMEXSTMM0N1_BITS "8:8"
#define SB_ZF_C2DIAGUCODECTL_M_ULMEMEXSTMM0W_BITS "9:9"
#define SB_ZF_C2DIAGUCODECTL_M_ULMEMEXSTMM1N0_BITS "10:10"
#define SB_ZF_C2DIAGUCODECTL_M_ULMEMEXSTMM1N1_BITS "11:11"
#define SB_ZF_C2DIAGUCODECTL_M_ULMEMEXSTMM1W_BITS "12:12"
#define SB_ZF_C2DIAGUCODECTL_M_ULTESTOFFSET_BITS "31:13"


typedef struct _sbZfC2DiagUcodeCtl {
  uint32_t m_ulStatus;
  uint32_t m_ulErrInject0;
  uint32_t m_ulErrInject1;
  uint32_t m_ulErrInd;
  uint32_t m_ulUcLoaded;
  uint32_t m_ulLrpState;
  uint32_t m_ulMemExstMm0N0;
  uint32_t m_ulMemExstMm0N1;
  uint32_t m_ulMemExstMm0W;
  uint32_t m_ulMemExstMm1N0;
  uint32_t m_ulMemExstMm1N1;
  uint32_t m_ulMemExstMm1W;
  uint32_t m_ulTestOffset;
} sbZfC2DiagUcodeCtl_t;

uint32_t
sbZfC2DiagUcodeCtl_Pack(sbZfC2DiagUcodeCtl_t *pFrom,
                        uint8_t *pToData,
                        uint32_t nMaxToDataIndex);
void
sbZfC2DiagUcodeCtl_Unpack(sbZfC2DiagUcodeCtl_t *pToStruct,
                          uint8_t *pFromData,
                          uint32_t nMaxToDataIndex);
void
sbZfC2DiagUcodeCtl_InitInstance(sbZfC2DiagUcodeCtl_t *pFrame);

#define SB_ZF_C2DIAGUCODECTL_SET_STATUS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_ERRINJECT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_ERRINJECT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_ERRIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_UCLOADED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_LRPSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_MEMEXST0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_MEMEXST1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_MEMEXST2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_MEMEXST3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_MEMEXST4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_MEMEXST5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_SET_TESTOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_STATUS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_ERRINJECT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_ERRINJECT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_ERRIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_UCLOADED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_LRPSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_MEMEXST0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_MEMEXST1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_MEMEXST2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_MEMEXST3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_MEMEXST4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_MEMEXST5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2DIAGUCODECTL_GET_TESTOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[2] << 3; \
           (nToData) |= (uint32_t) (pFromData)[3] << 11; \
          } while(0)

#endif
