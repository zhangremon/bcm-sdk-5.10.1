/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000HdrDescLrp2Ped.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_FE2000HDRDESCLRP2PED_H
#define SB_ZF_FE2000HDRDESCLRP2PED_H

#define SB_ZF_FE2000HDRDESCLRP2PED_SIZE_IN_BYTES 36
#define SB_ZF_FE2000HDRDESCLRP2PED_SIZE 36
#define SB_ZF_FE2000HDRDESCLRP2PED_ULCONTINUEBYTE_BITS "31:24"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR0_BITS "23:20"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR1_BITS "19:16"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR2_BITS "15:12"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR3_BITS "11:8"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR4_BITS "7:4"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR5_BITS "3:0"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR6_BITS "63:60"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR7_BITS "59:56"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR8_BITS "55:52"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHDR9_BITS "51:48"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN0_BITS "47:40"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN1_BITS "39:32"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN2_BITS "95:88"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN3_BITS "87:80"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN4_BITS "79:72"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN5_BITS "71:64"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN6_BITS "127:120"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN7_BITS "119:112"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN8_BITS "111:104"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULLOCN9_BITS "103:96"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULSTR_BITS "159:157"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULBUFFER_BITS "156:144"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULEXCEPTINDEX_BITS "143:136"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULSQUEUE_BITS "135:128"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULTC_BITS "191:189"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULHC_BITS "188:188"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULMIRRORINDEX_BITS "187:184"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULCOPYCNT_BITS "183:172"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULCM_BITS "171:170"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULD_BITS "169:169"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULENQ_BITS "168:168"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULDQUEUE_BITS "167:160"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULXFERLEN_BITS "223:210"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULPM_BITS "209:208"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULPADDING0_BITS "207:206"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULFRAMELEN_BITS "205:192"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULPADDING1_BITS "255:224"
#define SB_ZF_FE2000HDRDESCLRP2PED_ULPADDING2_BITS "287:256"


typedef struct _sbZfFe2000HdrDescLrp2Ped {
  uint32_t ulContinueByte;
  uint32_t ulHdr0;
  uint32_t ulHdr1;
  uint32_t ulHdr2;
  uint32_t ulHdr3;
  uint32_t ulHdr4;
  uint32_t ulHdr5;
  uint32_t ulHdr6;
  uint32_t ulHdr7;
  uint32_t ulHdr8;
  uint32_t ulHdr9;
  uint32_t ulLocn0;
  uint32_t ulLocn1;
  uint32_t ulLocn2;
  uint32_t ulLocn3;
  uint32_t ulLocn4;
  uint32_t ulLocn5;
  uint32_t ulLocn6;
  uint32_t ulLocn7;
  uint32_t ulLocn8;
  uint32_t ulLocn9;
  uint32_t ulStr;
  uint32_t ulBuffer;
  uint32_t ulExceptIndex;
  uint32_t ulSQueue;
  uint32_t ulTc;
  uint32_t ulHc;
  uint32_t ulMirrorIndex;
  uint32_t ulCopyCnt;
  uint32_t ulCM;
  uint32_t ulD;
  uint32_t ulEnq;
  uint32_t ulDQueue;
  uint32_t ulXferLen;
  uint32_t ulPm;
  uint32_t ulPadding0;
  uint32_t ulFrameLen;
  uint32_t ulPadding1;
  uint32_t ulPadding2;
} sbZfFe2000HdrDescLrp2Ped_t;

uint32_t
sbZfFe2000HdrDescLrp2Ped_Pack(sbZfFe2000HdrDescLrp2Ped_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex);
void
sbZfFe2000HdrDescLrp2Ped_Unpack(sbZfFe2000HdrDescLrp2Ped_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex);
void
sbZfFe2000HdrDescLrp2Ped_InitInstance(sbZfFe2000HdrDescLrp2Ped_t *pFrame);

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_CONTINUEBYTE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HDR9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_LOCN9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_ULSTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_ULBUFFER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((nFromData)) & 0xFF; \
           (pToData)[19] = ((pToData)[19] & ~ 0x1f) | (((nFromData) >> 8) & 0x1f); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_EXCEPTINDEX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_SQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_TC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((pToData)[23] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_HC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((pToData)[23] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_MIRRORINDEX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((pToData)[23] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_COPYCNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_CM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x03 << 2)) | (((nFromData) & 0x03) << 2); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_D(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_ENQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_DQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_XFERLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[26] = ((pToData)[26] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_PM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[26] = ((pToData)[26] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_PADDING0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((pToData)[25] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_FRAMELEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_PADDING1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_SET_PADDING2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[32] = ((nFromData)) & 0xFF; \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_CONTINUEBYTE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HDR9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[5] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[11] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[10] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[9] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[15] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[14] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[13] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_LOCN9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[12] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_ULSTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[19] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_ULBUFFER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[18] ; \
           (nToData) |= (uint32_t) ((pFromData)[19] & 0x1f) << 8; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_EXCEPTINDEX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[17] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_SQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[16] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_TC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[23] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_HC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[23] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_MIRRORINDEX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[23]) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_COPYCNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[21] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[22] << 4; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_CM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[21] >> 2) & 0x03; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_D(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[21] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_ENQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[21]) & 0x01; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_DQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[20] ; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_XFERLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[26] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[27] << 6; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_PM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[26]) & 0x03; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_PADDING0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[25] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_FRAMELEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[24] ; \
           (nToData) |= (uint32_t) ((pFromData)[25] & 0x3f) << 8; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_PADDING1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[28] ; \
           (nToData) |= (uint32_t) (pFromData)[29] << 8; \
           (nToData) |= (uint32_t) (pFromData)[30] << 16; \
           (nToData) |= (uint32_t) (pFromData)[31] << 24; \
          } while(0)

#define SB_ZF_FE2000HDRDESCLRP2PED_GET_PADDING2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[32] ; \
           (nToData) |= (uint32_t) (pFromData)[33] << 8; \
           (nToData) |= (uint32_t) (pFromData)[34] << 16; \
           (nToData) |= (uint32_t) (pFromData)[35] << 24; \
          } while(0)

#endif
