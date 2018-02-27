/*  -*-  Mode:C; c-basic-offset:4 -*- */

/*
$Id: sbG2SsFeIngRouteHeader.h 1.3.110.1 Broadcom SDK $
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

#ifndef SB_ZF_G2_SS_FEINGROUTEHEADER_H
#define SB_ZF_G2_SS_FEINGROUTEHEADER_H

#define SB_ZF_G2_SS_FEINGROUTEHEADER_SIZE_IN_BYTES 12
#define SB_ZF_G2_SS_FEINGROUTEHEADER_SIZE 12
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULTTL_BITS "7:0"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULS_BITS "8:8"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULRDP_BITS "10:9"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULRCOS_BITS "13:11"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULRSVD1_BITS "23:14"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULLBID_BITS "26:24"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULFCOS2_BITS "29:27"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULFDP_BITS "31:30"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULRSVD_BITS "33:32"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULSID_BITS "47:34"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULOUTUNION_BITS "63:48"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULQID_BITS "79:64"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULLENADJ_BITS "83:80"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULMC_BITS "84:84"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULTEST_BITS "85:85"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULECN_BITS "86:86"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULECT_BITS "87:87"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULKSOP_BITS "95:88"


typedef struct _sbZfG2SsFeIngRouteHeader {
  uint32_t ulTtl;
  uint32_t ulS;
  uint32_t ulRDp;
  uint32_t ulRCos;
  uint32_t ulRsvd1;
  uint32_t ulLBId;
  uint32_t ulFCos2;
  uint32_t ulFDp;
  uint32_t ulRsvd;
  uint32_t ulSid;
  uint32_t ulOutUnion;
  uint32_t ulQid;
  uint32_t ulLenAdj;
  uint32_t ulMc;
  uint32_t ulTest;
  uint32_t ulEcn;
  uint32_t ulEct;
  uint32_t ulKsop;
} sbZfG2SsFeIngRouteHeader_t;

uint32_t
sbZfG2SsFeIngRouteHeader_Pack(sbZfG2SsFeIngRouteHeader_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex);
void
sbZfG2SsFeIngRouteHeader_Unpack(sbZfG2SsFeIngRouteHeader_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex);
void
sbZfG2SsFeIngRouteHeader_InitInstance(sbZfG2SsFeIngRouteHeader_t *pFrame);

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_TTL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_S(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_RDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_RCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_RSVD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_LBID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_FCOS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_FDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_RSVD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_SID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_OUTUNION(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_QID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_LENADJ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_TEST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_ECT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_KSOP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_TTL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[11] ; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_S(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10]) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_RDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_RCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_RSVD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[9] << 2; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_LBID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8]) & 0x07; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_FCOS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_FDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_RSVD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7]) & 0x03; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_SID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[6] << 6; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_OUTUNION(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[5] ; \
           (nToData) |= (uint32_t) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_QID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_LENADJ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_TEST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_ECT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_KSOP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
