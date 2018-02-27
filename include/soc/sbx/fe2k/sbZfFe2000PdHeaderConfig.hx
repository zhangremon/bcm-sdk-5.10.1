/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000PdHeaderConfig.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_FE2000PDHEADERCONFIG_H
#define SB_ZF_FE2000PDHEADERCONFIG_H

#define SB_ZF_FE2000PDHEADERCONFIG_SIZE_IN_BYTES 4
#define SB_ZF_FE2000PDHEADERCONFIG_SIZE 4
#define SB_ZF_FE2000PDHEADERCONFIG_M_USPARE0_BITS "31:26"
#define SB_ZF_FE2000PDHEADERCONFIG_M_ULENGTHUNITS_BITS "25:24"
#define SB_ZF_FE2000PDHEADERCONFIG_M_USPARE1_BITS "23:23"
#define SB_ZF_FE2000PDHEADERCONFIG_M_ULENGTHSIZE_BITS "22:20"
#define SB_ZF_FE2000PDHEADERCONFIG_M_USPARE2_BITS "19:17"
#define SB_ZF_FE2000PDHEADERCONFIG_M_ULENGTHPOSITION_BITS "16:8"
#define SB_ZF_FE2000PDHEADERCONFIG_M_USPARE3_BITS "7:6"
#define SB_ZF_FE2000PDHEADERCONFIG_M_UBASELENGTH_BITS "5:0"


typedef struct _sbZfFe2000PdHeaderConfig {
  uint32_t m_uSpare0;
  uint32_t m_uLengthUnits;
  uint32_t m_uSpare1;
  uint32_t m_uLengthSize;
  uint32_t m_uSpare2;
  uint32_t m_uLengthPosition;
  uint32_t m_uSpare3;
  uint32_t m_uBaseLength;
} sbZfFe2000PdHeaderConfig_t;

uint32_t
sbZfFe2000PdHeaderConfig_Pack(sbZfFe2000PdHeaderConfig_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex);
void
sbZfFe2000PdHeaderConfig_Unpack(sbZfFe2000PdHeaderConfig_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex);
void
sbZfFe2000PdHeaderConfig_InitInstance(sbZfFe2000PdHeaderConfig_t *pFrame);

#define SB_ZF_FE2000PDHEADERCONFIG_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_SET_LENUNITS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_SET_LENSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_SET_SPARE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_SET_LENPOSN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_SET_SPARE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_SET_BASELEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_GET_LENUNITS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x03; \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_GET_LENSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_GET_SPARE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_GET_LENPOSN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[1] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 8; \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_GET_SPARE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_FE2000PDHEADERCONFIG_GET_BASELEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
