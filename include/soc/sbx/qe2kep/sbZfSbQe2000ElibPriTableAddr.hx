/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibPriTableAddr.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_H
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_H

#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES 4
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE 4
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_M_NPORT_BITS "11:6"
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_M_NCOS_BITS "5:3"
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_M_NDP_BITS "2:1"
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_M_NECN_BITS "0:0"


/**
 * @file sbZfSbQe2000ElibPriTableAddr.h
 *
 * <pre>
 *
 * =================================================================================
 * ==  sbZfSbQe2000ElibPriTableAddr.h - PRI Rewrite Table Address ZFrame  ==
 * =================================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibPriTableAddr.hx,v 1.3.36.4 2011/05/22 05:39:03 iakramov Exp $
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
 *     sbZfSbQe2000ElibPriTableAddr.h
 *
 * ABSTRACT:
 *
 *     PRI Rewrite Table Address ZFrame Definition.
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
 *     07-December-2004
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibPriTableAddr {
  uint32_t m_nPort;
  uint32_t m_nCos;
  uint32_t m_nDp;
  uint32_t m_nEcn;
} sbZfSbQe2000ElibPriTableAddr_t;

uint32_t
sbZfSbQe2000ElibPriTableAddr_Pack(sbZfSbQe2000ElibPriTableAddr_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibPriTableAddr_Unpack(sbZfSbQe2000ElibPriTableAddr_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibPriTableAddr_InitInstance(sbZfSbQe2000ElibPriTableAddr_t *pFrame);

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_SET_DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_GET_DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#endif
