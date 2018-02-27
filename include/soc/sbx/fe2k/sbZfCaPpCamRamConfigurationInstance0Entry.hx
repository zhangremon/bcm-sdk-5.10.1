/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPpCamRamConfigurationInstance0Entry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_H
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_H

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_BYTES 19
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SIZE 19
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_ULENGTHUNITS_BITS "147:146"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_ULENGTHMASK_BITS "145:143"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_ULENGTHPTR_BITS "142:138"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_USTATEMASK4_BITS "137:137"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_USTATEMASK3_BITS "136:136"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_USTATEMASK2_BITS "135:135"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_USTATEMASK1_BITS "134:134"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_USTATEMASK0_BITS "133:133"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_USTATESTREAMMASK_BITS "132:132"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_USTATE_BITS "131:108"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UHASHSTART_BITS "107:107"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UHASHTEMPLATE_BITS "106:102"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UCHECKEROFFSET_BITS "101:96"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UCHECKERTYPE_BITS "95:92"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UHEADERAVALID_BITS "91:91"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UHEADERATYPE_BITS "90:87"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UHEADERABASELENGTH_BITS "86:81"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UHEADERBVALID_BITS "80:80"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UHEADERBTYPE_BITS "79:76"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UHEADERBBASELENGTH_BITS "75:70"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_USHIFT_BITS "69:64"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UVARIABLEMASK_BITS "63:32"
#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_M_UVARIABLEDATA_BITS "31:0"


typedef struct _sbZfCaPpCamRamConfigurationInstance0Entry {
  uint32_t m_uLengthUnits;
  uint32_t m_uLengthMask;
  uint32_t m_uLengthPtr;
  uint32_t m_uStateMask4;
  uint32_t m_uStateMask3;
  uint32_t m_uStateMask2;
  uint32_t m_uStateMask1;
  uint32_t m_uStateMask0;
  uint32_t m_uStateStreamMask;
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
} sbZfCaPpCamRamConfigurationInstance0Entry_t;

uint32_t
sbZfCaPpCamRamConfigurationInstance0Entry_Pack(sbZfCaPpCamRamConfigurationInstance0Entry_t *pFrom,
                                               uint8_t *pToData,
                                               uint32_t nMaxToDataIndex);
void
sbZfCaPpCamRamConfigurationInstance0Entry_Unpack(sbZfCaPpCamRamConfigurationInstance0Entry_t *pToStruct,
                                                 uint8_t *pFromData,
                                                 uint32_t nMaxToDataIndex);
void
sbZfCaPpCamRamConfigurationInstance0Entry_InitInstance(sbZfCaPpCamRamConfigurationInstance0Entry_t *pFrame);

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_LENGTHUNITS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~(0x03 << 2)) | (((nFromData) & 0x03) << 2); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_LENGTHMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[18] = ((pToData)[18] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_LENGTHPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_STATEMASK4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_STATEMASK3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_STATEMASK2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_STATEMASK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_STATEMASK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_STATESTREAMMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_STATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData) >> 20) & 0x0f); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_HASHSTART(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_HASHTEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_CHECKEROFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_CHECKERTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_HEADERAVALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_HEADERATYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[11] = ((pToData)[11] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_HEADERABASELENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_HEADERBVALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_HEADERBTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_HEADERBBASELENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_SHIFT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_VARIABLEMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SET_VARIABLEDATA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_LENGTHUNITS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[18] >> 2) & 0x03; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_LENGTHMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[18] & 0x03) << 1; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_LENGTHPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_STATEMASK4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_STATEMASK3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17]) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_STATEMASK2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_STATEMASK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_STATEMASK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_STATESTREAMMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_STATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[14] << 4; \
           (nToData) |= (uint32_t) (pFromData)[15] << 12; \
           (nToData) |= (uint32_t) ((pFromData)[16] & 0x0f) << 20; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_HASHSTART(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_HASHTEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[13] & 0x07) << 2; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_CHECKEROFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_CHECKERTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_HEADERAVALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_HEADERATYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[11] & 0x07) << 1; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_HEADERABASELENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_HEADERBVALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10]) & 0x01; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_HEADERBTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_HEADERBBASELENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[9] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_SHIFT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8]) & 0x3f; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_VARIABLEMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
           (nToData) |= (uint32_t) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONINSTANCE0ENTRY_GET_VARIABLEDATA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
