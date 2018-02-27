/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PrCcTcamConfigEntryNonFirst.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_H
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_H

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SIZE_IN_BYTES 54
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SIZE 54
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UMASK0_31_BITS "429:398"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UMASK32_63_BITS "397:366"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UMASK64_95_BITS "365:334"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UMASK96_127_BITS "333:302"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UMASK128_159_BITS "301:270"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UMASK160_191_BITS "269:238"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UMASK192_199_BITS "237:230"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UMASKSTATE_BITS "229:216"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UPACKET0_31_BITS "215:184"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UPACKET32_63_BITS "183:152"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UPACKET64_95_BITS "151:120"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UPACKET96_127_BITS "119:88"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UPACKET128_159_BITS "87:56"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UPACKET160_191_BITS "55:24"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UPACKET192_199_BITS "23:16"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_USTATE_BITS "15:2"
#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_M_UVALID_BITS "1:0"


typedef struct _sbZfC2PrCcTcamConfigEntryNonFirst {
  uint32_t m_uMask0_31;
  uint32_t m_uMask32_63;
  uint32_t m_uMask64_95;
  uint32_t m_uMask96_127;
  uint32_t m_uMask128_159;
  uint32_t m_uMask160_191;
  uint32_t m_uMask192_199;
  uint32_t m_uMaskState;
  uint32_t m_uPacket0_31;
  uint32_t m_uPacket32_63;
  uint32_t m_uPacket64_95;
  uint32_t m_uPacket96_127;
  uint32_t m_uPacket128_159;
  uint32_t m_uPacket160_191;
  uint32_t m_uPacket192_199;
  uint32_t m_uState;
  uint32_t m_uValid;
} sbZfC2PrCcTcamConfigEntryNonFirst_t;

uint32_t
sbZfC2PrCcTcamConfigEntryNonFirst_Pack(sbZfC2PrCcTcamConfigEntryNonFirst_t *pFrom,
                                       uint8_t *pToData,
                                       uint32_t nMaxToDataIndex);
void
sbZfC2PrCcTcamConfigEntryNonFirst_Unpack(sbZfC2PrCcTcamConfigEntryNonFirst_t *pToStruct,
                                         uint8_t *pFromData,
                                         uint32_t nMaxToDataIndex);
void
sbZfC2PrCcTcamConfigEntryNonFirst_InitInstance(sbZfC2PrCcTcamConfigEntryNonFirst_t *pFrame);

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_MASK0_31(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[49] = ((pToData)[49] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[50] = ((pToData)[50] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[51] = ((pToData)[51] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[52] = ((pToData)[52] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[53] = ((pToData)[53] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_MASK32_63(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[45] = ((pToData)[45] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[48] = ((pToData)[48] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[49] = ((pToData)[49] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_MASK64_95(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[41] = ((pToData)[41] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_MASK96_127(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[37] = ((pToData)[37] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[41] = ((pToData)[41] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_MASK128_159(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[33] = ((pToData)[33] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_MASK160_191(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[29] = ((pToData)[29] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[33] = ((pToData)[33] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_MASK192_199(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((pToData)[28] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[29] = ((pToData)[29] & ~ 0x3f) | (((nFromData) >> 2) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_MASKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[27] = ((nFromData)) & 0xFF; \
           (pToData)[28] = ((pToData)[28] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_PACKET0_31(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((nFromData)) & 0xFF; \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_PACKET32_63(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_PACKET64_95(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_PACKET96_127(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_PACKET128_159(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_PACKET160_191(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_PACKET192_199(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_STATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_SET_VALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_MASK0_31(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[49] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[50] << 2; \
           (nToData) |= (uint32_t) (pFromData)[51] << 10; \
           (nToData) |= (uint32_t) (pFromData)[52] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[53] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_MASK32_63(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[45] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[46] << 2; \
           (nToData) |= (uint32_t) (pFromData)[47] << 10; \
           (nToData) |= (uint32_t) (pFromData)[48] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[49] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_MASK64_95(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[41] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[42] << 2; \
           (nToData) |= (uint32_t) (pFromData)[43] << 10; \
           (nToData) |= (uint32_t) (pFromData)[44] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[45] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_MASK96_127(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[37] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[38] << 2; \
           (nToData) |= (uint32_t) (pFromData)[39] << 10; \
           (nToData) |= (uint32_t) (pFromData)[40] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[41] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_MASK128_159(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[33] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[34] << 2; \
           (nToData) |= (uint32_t) (pFromData)[35] << 10; \
           (nToData) |= (uint32_t) (pFromData)[36] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[37] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_MASK160_191(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[29] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[30] << 2; \
           (nToData) |= (uint32_t) (pFromData)[31] << 10; \
           (nToData) |= (uint32_t) (pFromData)[32] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[33] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_MASK192_199(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[28] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[29] & 0x3f) << 2; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_MASKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[27] ; \
           (nToData) |= (uint32_t) ((pFromData)[28] & 0x3f) << 8; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_PACKET0_31(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[23] ; \
           (nToData) |= (uint32_t) (pFromData)[24] << 8; \
           (nToData) |= (uint32_t) (pFromData)[25] << 16; \
           (nToData) |= (uint32_t) (pFromData)[26] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_PACKET32_63(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[19] ; \
           (nToData) |= (uint32_t) (pFromData)[20] << 8; \
           (nToData) |= (uint32_t) (pFromData)[21] << 16; \
           (nToData) |= (uint32_t) (pFromData)[22] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_PACKET64_95(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[15] ; \
           (nToData) |= (uint32_t) (pFromData)[16] << 8; \
           (nToData) |= (uint32_t) (pFromData)[17] << 16; \
           (nToData) |= (uint32_t) (pFromData)[18] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_PACKET96_127(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[11] ; \
           (nToData) |= (uint32_t) (pFromData)[12] << 8; \
           (nToData) |= (uint32_t) (pFromData)[13] << 16; \
           (nToData) |= (uint32_t) (pFromData)[14] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_PACKET128_159(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
           (nToData) |= (uint32_t) (pFromData)[8] << 8; \
           (nToData) |= (uint32_t) (pFromData)[9] << 16; \
           (nToData) |= (uint32_t) (pFromData)[10] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_PACKET160_191(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[4] << 8; \
           (nToData) |= (uint32_t) (pFromData)[5] << 16; \
           (nToData) |= (uint32_t) (pFromData)[6] << 24; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_PACKET192_199(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_STATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 6; \
          } while(0)

#define SB_ZF_C2PRCCTCAMCONFIGENTRYNONFIRST_GET_VALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x03; \
          } while(0)

#endif
