#ifndef __GU_ELIBCONTEXT_H__
#define __GU_ELIBCONTEXT_H__
/**
 * @file sbMkEplibContext.h - Define handle used for reference
 *
 * <pre>
 * =======================================================
 * ==  sbMkEplibContext.h - Define handle used for reference ==
 * =======================================================
 *
 * WORKING REVISION: $Id: sbG2EplibContext.h 1.4.110.1 Broadcom SDK $
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
 *     sbMkEplibContext.h
 *
 * ABSTRACT:
 *
 *     Define handle used for reference
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *     Josh Weissman
 *
 * CREATION DATE:
 *
 *     14-February-2005
 * </pre>
 */

#include "sbQe2000Elib.h"
#include "sbZfG2EplibIpSegment.hx"

typedef enum {
    G2EPLIB_UCODE = 0,
    G2EPLIB_QESS_UCODE = 1
}sbG2EplibUcode_t;

/* elib context struct */
typedef struct sbG2EplibCtxt_s {
    SB_QE2000_ELIB_HANDLE EpHandle;     /**< Handle to elib */
    void                  *pHalCtx;     /**< HAL Context */
    bool_t                bPCTCounts;   /**< Poll the PCT */
    sbZfG2EplibIpSegment_t tIpSegment[SB_QE2000_ELIB_NUM_SEGMENTS_K]; /**< IP Segment Memory Information */
    sbG2EplibUcode_t     eUcode;
} sbG2EplibCtxt_st, *sbG2EplibCtxt_pst;

#endif /* __GU_ELIBCONTEXT_H__ */
