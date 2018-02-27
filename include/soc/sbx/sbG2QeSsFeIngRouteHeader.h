/*  -*-  Mode:C; c-basic-offset:4 -*- */

/*
$Id: sbG2QeSsFeIngRouteHeader.h 1.3.110.1 Broadcom SDK $
$Copyright: Copyright 2011 Broadcom Corporation.
This program is the proprietary software of Broadcom Corporation
and/or its licensors, and may only be used, duplicated, modified
or distributed pursuant to the terms and conditions of a separate,
written license agreement executed between you and Broadcom
(an "Authorized License").  Except as set forth in an Authorized
License, Broadcom grants no license (express or implied), right
to use, or waiver of any kind with respect to the Software, and
Broadcom expressly reserves all rights in and to the Software
and all intellectual property rights therein.  IF YOU HAVE
NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
ALL USE OF THE SOFTWARE.  
 
Except as expressly set forth in the Authorized License,
 
1.     This program, including its structure, sequence and organization,
constitutes the valuable trade secrets of Broadcom, and you shall use
all reasonable efforts to protect the confidentiality thereof,
and to use this information only in connection with your use of
Broadcom integrated circuit products.
 
2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
*/

#ifndef SB_ZF_G2_QESS_FEINGROUTEHEADER_H
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_H

#define SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES 14
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE 14
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULTTL_BITS "7:0"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULS_BITS "8:8"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULRDP_BITS "10:9"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULRCOS_BITS "13:11"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULRSVD1_BITS "23:14"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULLBID_BITS "26:24"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULFCOS2_BITS "29:27"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULRSVD_BITS "31:30"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULSID_BITS "47:34"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULLRSVD2_BITS "55:48"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULOUTUNION_BITS "71:56"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULZERO_BITS "73:72"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULECT_BITS "74:74"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULMC_BITS "75:75"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULLENADJ_BITS "79:76"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULFRMLEN_BITS "93:80"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULTEST_BITS "94:94"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULECN_BITS "95:95"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULFDP_BITS "97:96"
#define SB_ZF_G2_QESS_FEINGROUTEHEADER_ULQID_BITS "111:98"


typedef struct _sbZfG2QeSsFeIngRouteHeader {
  uint32_t ulTtl;
  uint32_t ulS;
  uint32_t ulRDp;
  uint32_t ulRCos;
  uint32_t ulRsvd1;
  uint32_t ulLBId;
  uint32_t ulFCos2;
  uint32_t ulRsvd;
  uint32_t ulSid;
  uint32_t ullRsvd2;
  uint32_t ulOutUnion;
  uint32_t ulZero;
  uint32_t ulEct;
  uint32_t ulMc;
  uint32_t ulLenAdj;
  uint32_t ulFrmLen;
  uint32_t ulTest;
  uint32_t ulEcn;
  uint32_t ulFDp;
  uint32_t ulQid;
} sbZfG2QeSsFeIngRouteHeader_t;

uint32_t
sbZfG2QeSsFeIngRouteHeader_Pack(sbZfG2QeSsFeIngRouteHeader_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex);
void
sbZfG2QeSsFeIngRouteHeader_Unpack(sbZfG2QeSsFeIngRouteHeader_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex);
void
sbZfG2QeSsFeIngRouteHeader_InitInstance(sbZfG2QeSsFeIngRouteHeader_t *pFrame);

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_TTL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_S(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_RDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_RCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_RSVD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_LBID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_FCOS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_RSVD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_SID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_RSVD2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_OUTUNION(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_ZERO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_ECT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_LENADJ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_FRMLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_TEST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_FDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_SET_QID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_TTL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[13] ; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_S(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12]) & 0x01; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_RDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_RCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_RSVD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[11] << 2; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_LBID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10]) & 0x07; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_FCOS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_RSVD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_SID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[9] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[8] << 6; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_RSVD2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_OUTUNION(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[6] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_ZERO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4]) & 0x03; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_ECT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_LENADJ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_FRMLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_TEST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_FDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x03; \
          } while(0)

#define SB_ZF_G2QESSFEINGROUTEHEADER_GET_QID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[0] << 6; \
          } while(0)

#endif
