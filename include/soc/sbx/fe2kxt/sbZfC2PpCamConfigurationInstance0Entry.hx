/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PpCamConfigurationInstance0Entry.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_H
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_H

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_BYTES 54
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE 54
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UMASK6_BITS "429:406"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UMASK5_BITS "405:374"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UMASK4_BITS "373:342"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UMASK3_BITS "341:310"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UMASK2_BITS "309:278"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UMASK1_BITS "277:246"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UMASK0_BITS "245:216"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UDATA6_BITS "215:192"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UDATA5_BITS "191:160"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UDATA4_BITS "159:128"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UDATA3_BITS "127:96"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UDATA2_BITS "95:64"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UDATA1_BITS "63:32"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UDATA0_BITS "31:2"
#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_M_UVALID_BITS "1:0"


typedef struct _sbZfC2PpCamConfigurationInstance0Entry {
  uint32_t m_uMask6;
  uint32_t m_uMask5;
  uint32_t m_uMask4;
  uint32_t m_uMask3;
  uint32_t m_uMask2;
  uint32_t m_uMask1;
  uint32_t m_uMask0;
  uint32_t m_uData6;
  uint32_t m_uData5;
  uint32_t m_uData4;
  uint32_t m_uData3;
  uint32_t m_uData2;
  uint32_t m_uData1;
  uint32_t m_uData0;
  uint32_t m_uValid;
} sbZfC2PpCamConfigurationInstance0Entry_t;

uint32_t
sbZfC2PpCamConfigurationInstance0Entry_Pack(sbZfC2PpCamConfigurationInstance0Entry_t *pFrom,
                                            uint8_t *pToData,
                                            uint32_t nMaxToDataIndex);
void
sbZfC2PpCamConfigurationInstance0Entry_Unpack(sbZfC2PpCamConfigurationInstance0Entry_t *pToStruct,
                                              uint8_t *pFromData,
                                              uint32_t nMaxToDataIndex);
void
sbZfC2PpCamConfigurationInstance0Entry_InitInstance(sbZfC2PpCamConfigurationInstance0Entry_t *pFrame);

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_MASK6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[50] = ((pToData)[50] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[51] = ((pToData)[51] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[52] = ((pToData)[52] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[53] = ((pToData)[53] & ~ 0x3f) | (((nFromData) >> 18) & 0x3f); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_MASK5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[46] = ((pToData)[46] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[48] = ((pToData)[48] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[49] = ((pToData)[49] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[50] = ((pToData)[50] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_MASK4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[42] = ((pToData)[42] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_MASK3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[38] = ((pToData)[38] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_MASK2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[34] = ((pToData)[34] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_MASK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[30] = ((pToData)[30] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_MASK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[27] = ((nFromData)) & 0xFF; \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~ 0x3f) | (((nFromData) >> 24) & 0x3f); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_DATA6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_DATA5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_DATA4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_DATA3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_DATA2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_DATA1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_DATA0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SET_VALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_MASK6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[50] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[51] << 2; \
           (nToData) |= (uint32_t) (pFromData)[52] << 10; \
           (nToData) |= (uint32_t) ((pFromData)[53] & 0x3f) << 18; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_MASK5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[46] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[47] << 2; \
           (nToData) |= (uint32_t) (pFromData)[48] << 10; \
           (nToData) |= (uint32_t) (pFromData)[49] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[50] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_MASK4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[42] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[43] << 2; \
           (nToData) |= (uint32_t) (pFromData)[44] << 10; \
           (nToData) |= (uint32_t) (pFromData)[45] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[46] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_MASK3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[38] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[39] << 2; \
           (nToData) |= (uint32_t) (pFromData)[40] << 10; \
           (nToData) |= (uint32_t) (pFromData)[41] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[42] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_MASK2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[34] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[35] << 2; \
           (nToData) |= (uint32_t) (pFromData)[36] << 10; \
           (nToData) |= (uint32_t) (pFromData)[37] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[38] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_MASK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[30] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[31] << 2; \
           (nToData) |= (uint32_t) (pFromData)[32] << 10; \
           (nToData) |= (uint32_t) (pFromData)[33] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[34] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_MASK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[27] ; \
           (nToData) |= (uint32_t) (pFromData)[28] << 8; \
           (nToData) |= (uint32_t) (pFromData)[29] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[30] & 0x3f) << 24; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_DATA6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[24] ; \
           (nToData) |= (uint32_t) (pFromData)[25] << 8; \
           (nToData) |= (uint32_t) (pFromData)[26] << 16; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_DATA5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[20] ; \
           (nToData) |= (uint32_t) (pFromData)[21] << 8; \
           (nToData) |= (uint32_t) (pFromData)[22] << 16; \
           (nToData) |= (uint32_t) (pFromData)[23] << 24; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_DATA4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[16] ; \
           (nToData) |= (uint32_t) (pFromData)[17] << 8; \
           (nToData) |= (uint32_t) (pFromData)[18] << 16; \
           (nToData) |= (uint32_t) (pFromData)[19] << 24; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_DATA3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[12] ; \
           (nToData) |= (uint32_t) (pFromData)[13] << 8; \
           (nToData) |= (uint32_t) (pFromData)[14] << 16; \
           (nToData) |= (uint32_t) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_DATA2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) (pFromData)[9] << 8; \
           (nToData) |= (uint32_t) (pFromData)[10] << 16; \
           (nToData) |= (uint32_t) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_DATA1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
           (nToData) |= (uint32_t) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_DATA0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 6; \
           (nToData) |= (uint32_t) (pFromData)[2] << 14; \
           (nToData) |= (uint32_t) (pFromData)[3] << 22; \
          } while(0)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_GET_VALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x03; \
          } while(0)

#endif
