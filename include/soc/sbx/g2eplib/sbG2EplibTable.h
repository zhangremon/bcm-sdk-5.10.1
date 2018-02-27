#ifndef __GU_ELIBTABLE_H__
#define __GU_ELIBTABLE_H__
/**
 * @file sbG2EplibTable.h  Logical Table Access Functions
 *
 * <pre>
 * ===================================================
 * ==  sbG2EplibTable.h - Logical Table Access Functions ==
 * ===================================================
 *
 * WORKING REVISION: $Id: sbG2EplibTable.h 1.3.156.1 Broadcom SDK $
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
 *     sbG2EplibTable.h
 *
 * ABSTRACT:
 *
 *      Logical Table Access Functions
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

#include "sbQe2000Elib.h"
#include "sbElibStatus.h"
#include "sbG2Eplib.h"

/**
 * Enumerated Type of different counters within the IP Counter
 * segment memory.
 */
typedef enum sbG2EplibIpCntIdx_es {
    SEG_CNT_TOPROC = 0,
    SEG_CNT_FROMPROC = 1,
    SEG_CNT_DROPCLASS = 2,
    /* leave as last */
    SEG_CNT_COUNTS
} sbG2EplibIpCntIdx_et;

/**
 *
 * Enumerated Type of the different SMAC tables that are at the bottom of
 * IP memory before the first IP segment. These are all physically in the
 * same memory portion but we still maintain the distinction of between them.
 *
 **/
typedef enum sbG2EplibSMACTbls_et {
    SMAC_TBL_PORT = 0,   /**< Port-Based SMAC Table */
    SMAC_TBL_VRID,       /**< Vrid-Based SMAC Table */
    /* leave as last */
    SMAC_TBL_TABLES
} sbG2EplibSMACTbls_et;

/**
 *
 *  Provide logical write access to the different tables within the QE2k's IP
 *  memory. All table entries are accessed with a given IP segment that
 *  denotes the table, and an index within that table.
 *
 *  @param  pCtxt   - Pointer to the Egress context structure
 *  @param  eSeg    - Enumerated Type describing which table to access
 *  @param  nIdx    - Index of the entry within that table
 *  @param  nData   - Array (Pointer) to Packed word data
 *  @return status  - error code (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibIpTableWrite(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
		     uint32_t nIdx, uint32_t nData[]);

/**
 *
 *  Provide logical read access to the different tables within the QE2k's IP
 *  memory. All table entries are accessed with a given IP segment that
 *  denotes the table, and an index within that table.
 *
 *  @param  pCtxt   - Pointer to the Egress context structure
 *  @param  eSeg    - Enumerated Type describing which table to access
 *  @param  nIdx    - Index of the entry within that table
 *  @return nData   - Array (Pointer) to Packed Word data
 *  @return status  - error code (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibIpTableRead(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
		    uint32_t nIdx, uint32_t nData[]);

/**
 *
 *  Define an entry in one of the SMAC tables located at the bottom of
 *  IP memory. This access function is shared between the different tables
 *  that reside, an enumerated type selects which table to access.
 *
 *  @param  pCtxt   - Pointer to the Egress context structure
 *  @param  eTble   - Enumerated Type selecting table
 *  @param  nIdx    - Index of entry within that table
 *  @param  nData   - Array (pointer) of packed data to be written
 *  @return status  - error code (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibIpSMACWrite(sbG2EplibCtxt_pst pCtxt, sbG2EplibSMACTbls_et eTbl,
		    uint32_t nIdx, uint32_t nData[]);

/**
 *
 *  Retrieve an entry in one of the SMAC tables located at the bottom of
 *  IP memory. This access function is shared between the different tables
 *  that reside, an enumerated type selects which table to access.
 *
 *  @param  pCtxt   - Pointer to the Egress context structure
 *  @param  eTble   - Enumerated Type selecting table
 *  @param  nIdx    - Index of entry within that table
 *  @return nData   - Array (pointer) where packed data is exported
 *  @return status  - error code (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibIpSMACRead(sbG2EplibCtxt_pst pCtxt, sbG2EplibSMACTbls_et eTbl,
		   uint32_t nIdx, uint32_t nData[]);

/**
 *
 * Utility functions to access the descriptor tables. These provide
 * access to the table and impose a read-only interface to the module-scoped
 * access functions and descriptor data.
 *
 **/
uint32_t sbG2EplibGetSegBaseDefault(sbG2EplibIpSegs_et segment);
uint32_t sbG2EplibGetSegWidthDefault(sbG2EplibIpSegs_et segment);
uint32_t sbG2EplibGetSegEntrySizeDefault(sbG2EplibIpSegs_et segment);
uint32_t sbG2EplibGetSegEndDefault(sbG2EplibIpSegs_et segment);

uint32_t sbG2EplibGetSegBase(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment);
uint32_t sbG2EplibGetSegWidth(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment);
uint32_t sbG2EplibGetSegEntrySize(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment);
uint32_t sbG2EplibGetSegEnd(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment);

uint32_t sbG2EplibGetSMACBase(sbG2EplibSMACTbls_et tbl);
uint32_t sbG2EplibGetSMACEnd(sbG2EplibSMACTbls_et tbl);

#endif /* __GU_ELIBTABLE_H__ */
