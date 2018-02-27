/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2DiagUcodeShared.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2DiagUcodeShared_H
#define SB_ZF_C2DiagUcodeShared_H

#define SB_ZF_C2DiagUcodeShared_SIZE_IN_BYTES 4
#define SB_ZF_C2DiagUcodeShared_SIZE 4
#define SB_ZF_C2DiagUcodeShared_M_ULMEMSIZEMM0N0_BITS "2:0"
#define SB_ZF_C2DiagUcodeShared_M_ULMEMSIZEMM0N1_BITS "5:3"
#define SB_ZF_C2DiagUcodeShared_M_ULMEMSIZEMM0W_BITS "8:6"
#define SB_ZF_C2DiagUcodeShared_M_ULMEMSIZEMM1N0_BITS "11:9"
#define SB_ZF_C2DiagUcodeShared_M_ULMEMSIZEMM1N1_BITS "14:12"
#define SB_ZF_C2DiagUcodeShared_M_ULMEMSIZEMM1W_BITS "17:15"
#define SB_ZF_C2DiagUcodeShared_M_ULRANDOMSEED_BITS "31:18"


typedef struct _sbZfC2DiagUcodeShared {
  uint32_t m_ulMemSizeMm0N0;
  uint32_t m_ulMemSizeMm0N1;
  uint32_t m_ulMemSizeMm0W;
  uint32_t m_ulMemSizeMm1N0;
  uint32_t m_ulMemSizeMm1N1;
  uint32_t m_ulMemSizeMm1W;
  uint32_t m_ulRandomSeed;
} sbZfC2DiagUcodeShared_t;

uint32_t
sbZfC2DiagUcodeShared_Pack(sbZfC2DiagUcodeShared_t *pFrom,
                           uint8_t *pToData,
                           uint32_t nMaxToDataIndex);
void
sbZfC2DiagUcodeShared_Unpack(sbZfC2DiagUcodeShared_t *pToStruct,
                             uint8_t *pFromData,
                             uint32_t nMaxToDataIndex);
void
sbZfC2DiagUcodeShared_InitInstance(sbZfC2DiagUcodeShared_t *pFrame);

#define SB_ZF_C2DIAGUCODESHARED_SET_MEMSIZE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_SET_MEMSIZE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_SET_MEMSIZE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 2) & 0x01); \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_SET_MEMSIZE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_SET_MEMSIZE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_SET_MEMSIZE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_SET_RNDMSEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_GET_MEMSIZE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x07; \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_GET_MEMSIZE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_GET_MEMSIZE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x01) << 2; \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_GET_MEMSIZE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_GET_MEMSIZE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_GET_MEMSIZE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 1; \
          } while(0)

#define SB_ZF_C2DIAGUCODESHARED_GET_RNDMSEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[3] << 6; \
          } while(0)

#endif
