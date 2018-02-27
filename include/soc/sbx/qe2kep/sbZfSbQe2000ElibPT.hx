/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibPT.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_SB_QE2000_ELIB_PT_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_SIZE_IN_BYTES 8
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_SIZE 8
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB15_BITS "63:63"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB14_BITS "62:62"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB13_BITS "61:61"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB12_BITS "60:60"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB11_BITS "59:59"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB10_BITS "58:58"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB9_BITS "57:57"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB8_BITS "56:56"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB7_BITS "55:55"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB6_BITS "54:54"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB5_BITS "53:53"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB4_BITS "52:52"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB3_BITS "51:51"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB2_BITS "50:50"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB1_BITS "49:49"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BCLASSENB0_BITS "48:48"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_NCOUNTTRANS_BITS "47:40"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_NRESERVED1_BITS "39:33"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BPREPEND_BITS "32:32"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_BINSTVALID_BITS "31:31"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_M_INSTRUCTION_BITS "30:0"
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_OFFSET         0x080


/**
 * @file sbZfSbQe2000ElibPT.h
 *
 * <pre>
 *
 * =====================================================================
 * ==  sbZfSbQe2000ElibPT.h - Class Instruction Table ZFrame  ==
 * =====================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibPT.hx,v 1.3.36.4 2011/05/22 05:39:03 iakramov Exp $
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
 *     sbZfSbQe2000ElibPT.h
 *
 * ABSTRACT:
 *
 *     Port Table ZFrame Definition.
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
typedef struct _sbZfSbQe2000ElibPT {
  uint8_t m_bClassEnb15;
  uint8_t m_bClassEnb14;
  uint8_t m_bClassEnb13;
  uint8_t m_bClassEnb12;
  uint8_t m_bClassEnb11;
  uint8_t m_bClassEnb10;
  uint8_t m_bClassEnb9;
  uint8_t m_bClassEnb8;
  uint8_t m_bClassEnb7;
  uint8_t m_bClassEnb6;
  uint8_t m_bClassEnb5;
  uint8_t m_bClassEnb4;
  uint8_t m_bClassEnb3;
  uint8_t m_bClassEnb2;
  uint8_t m_bClassEnb1;
  uint8_t m_bClassEnb0;
  int32_t m_nCountTrans;
  int32_t m_nReserved1;
  int32_t m_bPrepend;
  uint8_t m_bInstValid;
  int32_t m_Instruction;
} sbZfSbQe2000ElibPT_t;

uint32_t
sbZfSbQe2000ElibPT_Pack(sbZfSbQe2000ElibPT_t *pFrom,
                        uint8_t *pToData,
                        uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibPT_Unpack(sbZfSbQe2000ElibPT_t *pToStruct,
                          uint8_t *pFromData,
                          uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibPT_InitInstance(sbZfSbQe2000ElibPT_t *pFrame);

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_ENABLE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_TRANS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_RESRV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_PREPEND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_VALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_SET_INST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x7f) | (((nFromData) >> 24) & 0x7f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7]) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_ENABLE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6]) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_TRANS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[5] ; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_RESRV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[4] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_PREPEND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[4]) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_VALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPT_GET_INST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[0] ; \
           (nToData) |= (int32_t) (pFromData)[1] << 8; \
           (nToData) |= (int32_t) (pFromData)[2] << 16; \
           (nToData) |= (int32_t) ((pFromData)[3] & 0x7f) << 24; \
          } while(0)

#endif
