/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibVIT.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_SIZE_IN_BYTES 8
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_SIZE 8
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_M_RECORD3_BITS "63:48"
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_M_RECORD2_BITS "47:32"
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_M_RECORD1_BITS "31:16"
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_M_RECORD0_BITS "15:00"


/**
 * @file sbZfSbQe2000ElibVIT.h
 *
 * <pre>
 *
 * ====================================================================
 * ==  sbZfSbQe2000ElibVIT.h - VLAN Indirection Record Table ZFrame  ==
 * ====================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibVIT.hx,v 1.3.36.4 2011/05/22 05:39:03 iakramov Exp $
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
 *     sbZfSbQe2000ElibVIT.h
 *
 * ABSTRACT:
 *
 *     VLAN Indirection Record Table ZFrame Definition.
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
 *     19-January-2005
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibVIT {
  int32_t m_record3;
  int32_t m_record2;
  int32_t m_record1;
  int32_t m_record0;
} sbZfSbQe2000ElibVIT_t;

uint32_t
sbZfSbQe2000ElibVIT_Pack(sbZfSbQe2000ElibVIT_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibVIT_Unpack(sbZfSbQe2000ElibVIT_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibVIT_InitInstance(sbZfSbQe2000ElibVIT_t *pFrame);

#define SB_ZF_SBQE2000ELIBVIT_SET_REC3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_SET_REC2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_SET_REC1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_SET_REC0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_GET_REC3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[6] ; \
           (nToData) |= (int32_t) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_GET_REC2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[4] ; \
           (nToData) |= (int32_t) (pFromData)[5] << 8; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_GET_REC1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[2] ; \
           (nToData) |= (int32_t) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_GET_REC0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[0] ; \
           (nToData) |= (int32_t) (pFromData)[1] << 8; \
          } while(0)

#endif
