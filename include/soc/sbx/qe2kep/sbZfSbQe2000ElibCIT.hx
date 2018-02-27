/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibCIT.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE_IN_BYTES 32
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE 32
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION7_BITS "255:224"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION6_BITS "223:192"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION5_BITS "191:160"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION4_BITS "159:128"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION3_BITS "127:96"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION2_BITS "95:64"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION1_BITS "63:32"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION0_BITS "31:0"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_OFFSET         0x040 /**< Offset into the classifier memory for the CIT */


/**
 * @file sbZfSbQe2000ElibCIT.h
 *
 * <pre>
 *
 * ==============================================================
 * ==  sbZfSbQe2000ElibCIT.h - Class Instruction Table ZFrame  ==
 * ==============================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibCIT.hx,v 1.3.36.4 2011/05/22 05:39:03 iakramov Exp $
 *
 * Copyright (c) Sandburst, Inc. 2004
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
 *     sbZfSbQe2000ElibCIT.h
 *
 * ABSTRACT:
 *
 *     Class Instruction Table ZFrame Definition.
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
 *     30-November-2004
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibCIT {
  int32_t m_Instruction7;
  int32_t m_Instruction6;
  int32_t m_Instruction5;
  int32_t m_Instruction4;
  int32_t m_Instruction3;
  int32_t m_Instruction2;
  int32_t m_Instruction1;
  int32_t m_Instruction0;
} sbZfSbQe2000ElibCIT_t;

uint32_t
sbZfSbQe2000ElibCIT_Pack(sbZfSbQe2000ElibCIT_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibCIT_Unpack(sbZfSbQe2000ElibCIT_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibCIT_InitInstance(sbZfSbQe2000ElibCIT_t *pFrame);

#define SB_ZF_SBQE2000ELIBCIT_SET_INST7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[28] ; \
           (nToData) |= (int32_t) (pFromData)[29] << 8; \
           (nToData) |= (int32_t) (pFromData)[30] << 16; \
           (nToData) |= (int32_t) (pFromData)[31] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[24] ; \
           (nToData) |= (int32_t) (pFromData)[25] << 8; \
           (nToData) |= (int32_t) (pFromData)[26] << 16; \
           (nToData) |= (int32_t) (pFromData)[27] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[20] ; \
           (nToData) |= (int32_t) (pFromData)[21] << 8; \
           (nToData) |= (int32_t) (pFromData)[22] << 16; \
           (nToData) |= (int32_t) (pFromData)[23] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[16] ; \
           (nToData) |= (int32_t) (pFromData)[17] << 8; \
           (nToData) |= (int32_t) (pFromData)[18] << 16; \
           (nToData) |= (int32_t) (pFromData)[19] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[12] ; \
           (nToData) |= (int32_t) (pFromData)[13] << 8; \
           (nToData) |= (int32_t) (pFromData)[14] << 16; \
           (nToData) |= (int32_t) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[8] ; \
           (nToData) |= (int32_t) (pFromData)[9] << 8; \
           (nToData) |= (int32_t) (pFromData)[10] << 16; \
           (nToData) |= (int32_t) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[4] ; \
           (nToData) |= (int32_t) (pFromData)[5] << 8; \
           (nToData) |= (int32_t) (pFromData)[6] << 16; \
           (nToData) |= (int32_t) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[0] ; \
           (nToData) |= (int32_t) (pFromData)[1] << 8; \
           (nToData) |= (int32_t) (pFromData)[2] << 16; \
           (nToData) |= (int32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
