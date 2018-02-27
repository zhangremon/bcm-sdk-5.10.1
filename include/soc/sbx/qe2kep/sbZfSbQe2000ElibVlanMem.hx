/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibVlanMem.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_SB_QE2000_ELIB_VLAN_MEM_H
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_H

#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_SIZE_IN_BYTES 32
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_SIZE 32
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_M_PATCHES_BITS "0:0"
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_M_EDGES_BITS "0:0"
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_M_NUMFREE_BITS "0:0"
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_M_NUMUSED_BITS "0:0"


/**
 * @file sbZfSbQe2000ElibCIT.h
 *
 * <pre>
 *
 * ==============================================================
 * ==  sbZfSbQe2000ElibCIT.h - Class Instruction Table ZFrame  ==
 * ==============================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibVlanMem.hx,v 1.3.36.4 2011/05/22 05:39:03 iakramov Exp $
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
typedef struct _sbZfSbQe2000ElibVlanMem {
  int32_t m_Patches;
  int32_t m_Edges;
  int32_t m_NumFree;
  int32_t m_NumUsed;
} sbZfSbQe2000ElibVlanMem_t;

uint32_t
sbZfSbQe2000ElibVlanMem_Pack(sbZfSbQe2000ElibVlanMem_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibVlanMem_Unpack(sbZfSbQe2000ElibVlanMem_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex);
void
sbZfSbQe2000ElibVlanMem_InitInstance(sbZfSbQe2000ElibVlanMem_t *pFrame);

#define SB_ZF_SBQE2000ELIBVLANMEM_SET_PATCHES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_SET_EDGES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_SET_NUMFREE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_SET_NUMUSED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_GET_PATCHES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_GET_EDGES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_GET_NUMFREE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_GET_NUMUSED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
