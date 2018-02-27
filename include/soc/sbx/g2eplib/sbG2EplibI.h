#ifndef __GU_ELIBI_H__
#define __GU_ELIBI_H__
/**
 * @file sbG2EplibI.h Definitions, types and macros local to the elib
 *
 * <pre>
 * =================================================================
 * ==  sbG2EplibI.h - Definitions, types and macros local to the eplib
 * =================================================================
 *
 * WORKING REVISION: $Id: sbG2EplibI.h 1.2.220.1 Broadcom SDK $
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
 *     sbG2EplibI.h
 *
 * ABSTRACT:
 *
 *     Definitions, types and macros local to the eplib
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


/* macro to calculate segment size in in 8-word segments */
#define SEG_BSIZE(start, end) (((end) + 1 - (start)) / SB_QE2000_ELIB_SEGMENT_SIZ_DIV_K)

/* Division, round up */
#define DIVUP(p,q) (((p) + (q) - 1) / (q))

/* Calculate Words required to hold zframe */
#define ZF_WDSIZE(x) \
 (DIVUP(SAND_HAL_FRAME_##x##_SIZE_IN_BYTES, sizeof(uint32_t)))

/* Single Bit Mask */
#define BIT_N(n)       (0x1 << (n))
#define GET_BIT_N(x,n) (((x) >> (n)) & 0x1)
#define MSKN_32(n)     (0xFFFFFFFF >> (32 - (n)))
#define MSKN_64(n)     (0xFFFFFFFFFFFFFFFFULL >> (64 - (n)))

/*
 * Compose Index into the Egress Tos/Exp Remap tables
 * Index = {Egress-Port, Cos, Dp, E}
 */
#define RMP_IDX(port, coss, dp, e) \
 ((((port) & 0x3f) << 6) | (((coss) & 0x7) << 3) | (((dp) & 0x3) << 1) | ((e) & 0x1))

#ifdef DENTER
#undef DENTER
#endif

#ifdef DEXIT
#undef DEXIT
#endif

/* debugging tracepoints */
#if defined (DEBUG_PRINT)
#include "sbWrappers.h"
#define DENTER() SB_LOG("%s: enter\n", __PRETTY_FUNCTION__)
#define DBG(x)   x
#define DEXIT(_code)  SB_LOG("%s: line: %d:  exit status %d\n", __PRETTY_FUNCTION__, __LINE__, _code)
#else
#define DENTER()
#define DBG(x)
#define DEXIT(_code)
#endif

#endif /* __GU_ELIBI_H__ */
