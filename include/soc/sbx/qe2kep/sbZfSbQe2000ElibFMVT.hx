/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibFMVT.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES 32
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE 32
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NRESERVED_BITS "255:255"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NNPORTMAP2_BITS "254:205"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NMVTDA2_BITS "204:191"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NMVTDB2_BITS "190:187"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NNEXT2_BITS "186:171"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NKNOCKOUT2_BITS "170:170"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NNPORTMAP1_BITS "169:120"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NMVTDA1_BITS "119:106"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NMVTDB1_BITS "105:102"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NNEXT1_BITS "101:86"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NKNOCKOUT1_BITS "85:85"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NNPORTMAP0_BITS "84:35"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NMVTDA0_BITS "34:21"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NMVTDB0_BITS "20:17"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NNEXT0_BITS "16:1"
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_M_NKNOCKOUT0_BITS "0:0"


/**
 * @file sbZfSbQe2000ElibFMVT.h
 *
 * <pre>
 *
 * ===========================================================================
 * ==  sbZfSbQe2000ElibFMVT.h - Multicast Vector Table Entry ZFrame ==
 * ===========================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibFMVT.hx,v 1.3.36.4 2011/05/22 05:39:03 iakramov Exp $
 *
 * Copyright (c) Sandburst, Inc. 2005
 * All Rights Reserved.  Unpublished rights reserved under the copyright
 * laws of the United States.
 *
 * The software contained on this media is proprietary to and embodies the
 * confidential technology of Sandburst, Inc. Possession, use, duplication
 * or dissemination of the software and media is authorized only pursuant
 * to a valid written license from Sandburst, Inc.
 *
 * RESTRICTED RIGHTS LEGEND Use, duplication, or disclosure by the U.S.
 * Government is subject to restrictions as set forth in Subparagraph
 * (c) (1) (ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
 *
 *
 * MODULE NAME:
 *
 *     sbZfSbQe2000ElibFMVT.h
 *
 * ABSTRACT:
 *
 *     Multicast Vector Table Entry ZFrame Definition.
 *
 * LANGUAGE:
 *
 *     ZFrame
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     2-March-2005
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibFMVT {
  uint32_t m_nReserved;
  uint64_t m_nnPortMap2;
  uint32_t m_nMvtda2;
  uint32_t m_nMvtdb2;
  uint32_t m_nNext2;
  uint32_t m_nKnockout2;
  uint64_t m_nnPortMap1;
  uint32_t m_nMvtda1;
  uint32_t m_nMvtdb1;
  uint32_t m_nNext1;
  uint32_t m_nKnockout1;
  uint64_t m_nnPortMap0;
  uint32_t m_nMvtda0;
  uint32_t m_nMvtdb0;
  uint32_t m_nNext0;
  uint32_t m_nKnockout0;
} sbZfSbQe2000ElibFMVT_t;

uint32_t
sbZfSbQe2000ElibFMVT_Pack(sbZfSbQe2000ElibFMVT_t *pFrom,
                          uint8_t *pToData,
                          uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibFMVT_Unpack(sbZfSbQe2000ElibFMVT_t *pToStruct,
                            uint8_t *pFromData,
                            uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibFMVT_InitInstance(sbZfSbQe2000ElibFMVT_t *pFrame);

#define SB_ZF_SBQE2000ELIBFMVT_SET_RSVD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((pToData)[31] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_PORT_MAP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((pToData)[25] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 35) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~ 0x7f) | (((nFromData) >> 43) & 0x7f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_MVTD_A2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((pToData)[23] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~ 0x1f) | (((nFromData) >> 9) & 0x1f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_MVTD_B2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((pToData)[23] & ~(0x0f << 3)) | (((nFromData) & 0x0f) << 3); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_NEXT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~ 0x07) | (((nFromData) >> 13) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_KNOCKOUT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_PORT_MAP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~ 0x03) | (((nFromData) >> 48) & 0x03); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_MVTD_A1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_MVTD_B1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_NEXT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~ 0x3f) | (((nFromData) >> 10) & 0x3f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_KNOCKOUT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_PORT_MAP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 29) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 37) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x1f) | (((nFromData) >> 45) & 0x1f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_MVTD_A0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 11) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_MVTD_B0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_NEXT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 15) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_SET_KNOCKOUT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_RSVD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[31] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_PORT_MAP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[25]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[26]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[27]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[28]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[29]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[30]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[31]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_MVTD_A2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[23] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[24] << 1; \
           (nToData) |= (uint32_t) ((pFromData)[25] & 0x1f) << 9; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_MVTD_B2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[23] >> 3) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_NEXT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[21] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) (pFromData)[22] << 5; \
           (nToData) |= (uint32_t) ((pFromData)[23] & 0x07) << 13; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_KNOCKOUT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[21] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_PORT_MAP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[15]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_MVTD_A1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[14] << 6; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_MVTD_B1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[13] & 0x03) << 2; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_NEXT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[11] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[12] & 0x3f) << 10; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_KNOCKOUT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_PORT_MAP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[4]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_MVTD_A0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[3] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x07) << 11; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_MVTD_B0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_NEXT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 7; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 15; \
          } while(0)

#define SB_ZF_SBQE2000ELIBFMVT_GET_KNOCKOUT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
