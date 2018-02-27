/*
 * $Id: debug.h 1.13 Broadcom SDK $
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
 * File:        debug.h
 * Purpose:     Debug flag definitions for cpudb, cputrans, discover,
 *              and stktask directories.
 */

#ifndef   _CPUDB_DEBUG_H_
#define   _CPUDB_DEBUG_H_

#include <bcm/types.h>

#if defined(BROADCOM_DEBUG)

#define TKS_DBG_NORMAL       (1 << 0)   /* Normal Output */
#define TKS_DBG_ERR          (1 << 1)   /* Print errors */
#define TKS_DBG_WARN         (1 << 2)   /* Print warnings */
#define TKS_DBG_VERBOSE      (1 << 3)   /* General verbose output */
#define TKS_DBG_VVERBOSE     (1 << 4)   /* Very verbose output */
#define TKS_DBG_DISCOVER     (1 << 5)   /* Discovery/stacking */
#define TKS_DBG_DISCV        (1 << 6)   /* Discovery verbose */
#define TKS_DBG_TOPOLOGY     (1 << 7)   /* Topology */
#define TKS_DBG_TOPOV        (1 << 8)   /* Topology verbose */
#define TKS_DBG_STKTASK      (1 << 9)   /* Stack task */
#define TKS_DBG_STV          (1 << 10)  /* Stack task verbose */
#define TKS_DBG_TX           (1 << 11)  /* TX */
#define TKS_DBG_NH           (1 << 12)  /* Next Hop */
#define TKS_DBG_NHV          (1 << 13)  /* Next Hop Verbose */
#define TKS_DBG_ATP          (1 << 14)  /* ATP */
#define TKS_DBG_ATPV         (1 << 15)  /* ATP Verbose */
#define TKS_DBG_C2C          (1 << 16)  /* CPU-to-CPU */
#define TKS_DBG_C2CV         (1 << 17)  /* CPU-to-CPU Verbose */
#define TKS_DBG_CTPKT        (1 << 18)  /* CPUTRANS packet */
#define TKS_DBG_CTPKTV       (1 << 19)  /* CPUTRANS packet verbose */
#define TKS_DBG_TUNNEL       (1 << 20)  /* Tunnel related */
#define TKS_DBG_TUNNELV      (1 << 21)  /* Tunnel verbose */
#define TKS_DBG_COUNT        22

#if defined(NO_DEBUG_OUTPUT_DEFAULT)
#define TKS_DBG_DEFAULT 0
#else
#define TKS_DBG_DEFAULT (TKS_DBG_NORMAL | TKS_DBG_ERR | TKS_DBG_WARN | \
                         TKS_DBG_ATP | TKS_DBG_DISCOVER | TKS_DBG_STKTASK | \
                         TKS_DBG_TOPOLOGY | TKS_DBG_CTPKT)
#endif

#define TKS_DBG_NAMES    \
    "NORmal",            \
    "ERRor",             \
    "WARN",              \
    "VERbose",           \
    "VVERbose",          \
    "DISCover",          \
    "DISCV",             \
    "TOPOlogy",          \
    "TOPOV",             \
    "STKtask",           \
    "STKV",              \
    "TX",                \
    "NH",                \
    "NHV",               \
    "ATP",               \
    "ATPV",              \
    "C2C",               \
    "C2CV",              \
    "CTPKT",             \
    "CTPKTV",            \
    "TUNNEL",            \
    "TUNNELV"


/*
 * Proper use requires parentheses.  E.g.:
 *     TKS_DEBUG(TKS_DBG_FOO, ("Problem %d with unit %d\n", pr, unit));
 */

extern uint32 tks_debug_level;
extern char *tks_debug_names[];
extern int (*tks_debug_print)(const char *format, ...);

#define TKS_DEBUG_CHECK(flags) (((flags) & tks_debug_level) == (flags))
#define TKS_DEBUG(flags, stuff) \
    if (TKS_DEBUG_CHECK(flags) && tks_debug_print != 0) \
	(*tks_debug_print) stuff

#else

#define TKS_DEBUG_CHECK(flags) 0
#define TKS_DEBUG(flags, stuff)

#endif /* defined(BROADCOM_DEBUG) */

#endif /* _CPUDB_DEBUG_H_ */
