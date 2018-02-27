/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPpCamRamConfigurationEntry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_H
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_H

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SIZE_IN_BYTES 17
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SIZE 17
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UUNUSED_SPARE_BITS "132:132"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_USTATE_BITS "131:108"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UHASHSTART_BITS "107:107"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UHASHTEMPLATE_BITS "106:102"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UCHECKEROFFSET_BITS "101:96"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UCHECKERTYPE_BITS "95:92"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UHEADERAVALID_BITS "91:91"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UHEADERATYPE_BITS "90:87"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UHEADERABASELENGTH_BITS "86:81"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UHEADERBVALID_BITS "80:80"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UHEADERBTYPE_BITS "79:76"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UHEADERBBASELENGTH_BITS "75:70"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_USHIFT_BITS "69:64"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UVARIABLEMASK_BITS "63:32"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_M_UVARIABLEDATA_BITS "31:0"


typedef struct _sbZfCaPpCamRamConfigurationEntry {
  uint32_t m_uUnused_Spare;
  uint32_t m_uState;
  uint32_t m_uHashStart;
  uint32_t m_uHashTemplate;
  uint32_t m_uCheckerOffset;
  uint32_t m_uCheckerType;
  uint32_t m_uHeaderAValid;
  uint32_t m_uHeaderAType;
  uint32_t m_uHeaderABaseLength;
  uint32_t m_uHeaderBValid;
  uint32_t m_uHeaderBType;
  uint32_t m_uHeaderBBaseLength;
  uint32_t m_uShift;
  uint32_t m_uVariableMask;
  uint32_t m_uVariableData;
} sbZfCaPpCamRamConfigurationEntry_t;

uint32_t
sbZfCaPpCamRamConfigurationEntry_Pack(sbZfCaPpCamRamConfigurationEntry_t *pFrom,
                                      uint8_t *pToData,
                                      uint32_t nMaxToDataIndex);
void
sbZfCaPpCamRamConfigurationEntry_Unpack(sbZfCaPpCamRamConfigurationEntry_t *pToStruct,
                                        uint8_t *pFromData,
                                        uint32_t nMaxToDataIndex);
void
sbZfCaPpCamRamConfigurationEntry_InitInstance(sbZfCaPpCamRamConfigurationEntry_t *pFrame);

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_UNUSED_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_STATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData) >> 20) & 0x0f); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_HASHSTART(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_HASHTEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_CHECKEROFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_CHECKERTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_HEADERAVALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_HEADERATYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[11] = ((pToData)[11] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_HEADERABASELENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_HEADERBVALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_HEADERBTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_HEADERBBASELENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_SHIFT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_VARIABLEMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SET_VARIABLEDATA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_UNUSED_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_STATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[14] << 4; \
           (nToData) |= (uint32_t) (pFromData)[15] << 12; \
           (nToData) |= (uint32_t) ((pFromData)[16] & 0x0f) << 20; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_HASHSTART(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_HASHTEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[13] & 0x07) << 2; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_CHECKEROFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_CHECKERTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_HEADERAVALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_HEADERATYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[11] & 0x07) << 1; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_HEADERABASELENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_HEADERBVALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10]) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_HEADERBTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_HEADERBBASELENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_SHIFT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8]) & 0x3f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_VARIABLEMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
           (nToData) |= (uint32_t) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_GET_VARIABLEDATA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
