#ifndef __GU_ELIBCT_H__
#define __GU_ELIBCT_H__
/**
 * @file sbG2EplibCt.h Egress Class Table Initialization Functions
 *
 * <pre>
 * ==================================================================
 * ==  sbG2Eplib.h - QE2000 Egress Class Table Initialization Functions ==
 * ==================================================================
 *
 * WORKING REVISION: $Id: sbG2EplibCt.h 1.4.110.1 Broadcom SDK $
 *
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
 *
 * MODULE NAME:
 *
 *     sbG2EplibCt.h
 *
 * ABSTRACT:
 *
 *     Egress Class Table Initialization Functions
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Josh Weissman
 *
 * CREATION DATE:
 *
 *     30-March-2005
 * </pre>
 */

#include <sbQe2000Elib.h>

/**
 *
 * Enumerated Type that describes IP hw types. In hardware, these will define
 * the expected header on a per-class basis. This allows the instruction
 * stream to do header-specific editing operations
 *
 **/
typedef enum sbG2EplibClassHwTypes_s {
  EN_CLS_HW_TYPE_VLAN = 0,   /**< Vlan-aware Bridging */
  EN_CLS_HW_TYPE_LI,         /**< Logical Interface */
  /* leave as last */
  EN_CLS_HW_TYPES
} sbG2EplibClassHwTypes_t;

/**
 *
 *  Setup the IP memory segment registers. We define the data width,
 *  the segment size, and the starting address of the segment.
 *
 *  @param  pCtxt   - Gu Eplib Context.
 *  @return status  - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibSegmentInit(sbG2EplibCtxt_pst pCtxt);

/**
 *
 *  Setup Class Resolution Table
 *
 *  @param  EpHandle - Pointer to Context Structure for Qe2k Eplib
 *  @return status   - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibCRTInit(SB_QE2000_ELIB_HANDLE EpHandle);

/**
 *
 *  Setup Class Instruction Table
 *
 *  @param  pEgCtxt - Pointer to Context Structure for Qe2k Eplib
 *  @return status   - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibCITInit(sbG2EplibCtxt_pst pEgCtxt);

/**
 *
 *  Configure the Expected header types on a per-class basis. This is used
 *  by the hardware to make assumptions about what type of headers will be
 *  arriving in each of the instruction streams.
 *
 *  @param  pHalCtx    - Pointer to Hal Context Structure for Qe2k Register Set
 *  @param  eHwClsType - Array of Enumerated Types of the different headers
 *  @return status     - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibClassTypesSet(void *pHalCtx, sbG2EplibClassHwTypes_t eHwClsType[]);

/**
 *
 *  Retrieve the Expected header types on a per-class basis. This is used
 *  by the hardware to make assumptions about what type of headers will be
 *  arriving in each of the instruction streams.
 *
 *  @param  pHalCtx    - Pointer to Hal Context Structure for Qe2k Register Set
 *  @return eHwClsType - Array of Enumerated Types of the different headers
 *  @return status     - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibClassTypesGet(void *pHalCtx, sbG2EplibClassHwTypes_t eHwClsType[]);

#endif /* __GU_ELIBCT_H__ */
