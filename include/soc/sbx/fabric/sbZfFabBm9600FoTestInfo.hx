/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600FoTestInfo.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_FAB_BM9600_FOTESTINFO_H
#define SB_ZF_FAB_BM9600_FOTESTINFO_H

#define SB_ZF_FAB_BM9600_FOTESTINFO_SIZE_IN_BYTES 22
#define SB_ZF_FAB_BM9600_FOTESTINFO_SIZE 22
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_BISACTIVE_BITS "170:170"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UUDUTTIMESLOTSIZE_BITS "169:106"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UUSIMTIMETHATQEFAILOVEROCCURRED_BITS "105:42"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UDUTBASEADDRESS_BITS "41:10"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UNUMQESTHATFAILEDOVER_BITS "9:3"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UPREVIOUSACTIVEBM_BITS "2:2"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_BFAILOVERPREVIOUSLYASSERTED_BITS "1:1"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_BEXPECTFAILOVER_BITS "0:0"


typedef struct _sbZfFabBm9600FoTestInfo {
  uint8_t m_bIsActive;
  uint64_t m_uuDUTTimeslotsize;
  uint64_t m_uuSimTimeThatQEFailoverOccurred;
  uint32_t m_uDUTBaseAddress;
  uint32_t m_uNumQEsThatFailedOver;
  uint32_t m_uPreviousActiveBm;
  uint8_t m_bFailoverPreviouslyAsserted;
  uint8_t m_bExpectFailover;
} sbZfFabBm9600FoTestInfo_t;

uint32_t
sbZfFabBm9600FoTestInfo_Pack(sbZfFabBm9600FoTestInfo_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex);
void
sbZfFabBm9600FoTestInfo_Unpack(sbZfFabBm9600FoTestInfo_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex);
void
sbZfFabBm9600FoTestInfo_InitInstance(sbZfFabBm9600FoTestInfo_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOTESTINFO_SET_ISACTIVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((pToData)[22] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTSTIMESLOTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_QEFOSIMTIME(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_NUMFAILEDQE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 5) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_PREVIOUSACTIVEBM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_FOPREVIOUSLYASSERTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_EXPECTFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_FABBM9600FOTESTINFO_SET_ISACTIVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTSTIMESLOTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_QEFOSIMTIME(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_NUMFAILEDQE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 5) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_PREVIOUSACTIVEBM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_FOPREVIOUSLYASSERTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_EXPECTFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOTESTINFO_SET_ISACTIVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((pToData)[22] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTSTIMESLOTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_QEFOSIMTIME(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_NUMFAILEDQE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 5) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_PREVIOUSACTIVEBM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_FOPREVIOUSLYASSERTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_EXPECTFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_FABBM9600FOTESTINFO_SET_ISACTIVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTSTIMESLOTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_QEFOSIMTIME(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_NUMFAILEDQE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 5) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_PREVIOUSACTIVEBM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_FOPREVIOUSLYASSERTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_EXPECTFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOTESTINFO_GET_ISACTIVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[22] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTSTIMESLOTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[14]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_QEFOSIMTIME(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[6]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 6; \
           (nToData) |= (uint32_t) (pFromData)[0] << 14; \
           (nToData) |= (uint32_t) (pFromData)[7] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_NUMFAILEDQE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 5; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_PREVIOUSACTIVEBM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_FOPREVIOUSLYASSERTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_EXPECTFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_FABBM9600FOTESTINFO_GET_ISACTIVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[21] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTSTIMESLOTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[13]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_QEFOSIMTIME(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[5]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[2] << 6; \
           (nToData) |= (uint32_t) (pFromData)[3] << 14; \
           (nToData) |= (uint32_t) (pFromData)[4] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_NUMFAILEDQE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 5; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_PREVIOUSACTIVEBM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_FOPREVIOUSLYASSERTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_EXPECTFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOTESTINFO_GET_ISACTIVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[22] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTSTIMESLOTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[14]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_QEFOSIMTIME(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[6]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 6; \
           (nToData) |= (uint32_t) (pFromData)[0] << 14; \
           (nToData) |= (uint32_t) (pFromData)[7] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_NUMFAILEDQE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 5; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_PREVIOUSACTIVEBM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_FOPREVIOUSLYASSERTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_EXPECTFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_FABBM9600FOTESTINFO_GET_ISACTIVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[21] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTSTIMESLOTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[13]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_QEFOSIMTIME(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[5]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[2] << 6; \
           (nToData) |= (uint32_t) (pFromData)[3] << 14; \
           (nToData) |= (uint32_t) (pFromData)[4] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_NUMFAILEDQE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 5; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_PREVIOUSACTIVEBM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_FOPREVIOUSLYASSERTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_EXPECTFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
