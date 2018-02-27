/* 
 * $Id: debug.h 1.1 Broadcom SDK $
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
 * Purpose:     Board module debug messages
 */

#ifndef   _BOARD_DEBUG_H_
#define   _BOARD_DEBUG_H_
#include <bcm/types.h>

#if defined(BROADCOM_DEBUG)

#define BOARD_DBG_NORMAL       (1 << 0)   /* Normal Output */
#define BOARD_DBG_ERR          (1 << 1)   /* Print errors */
#define BOARD_DBG_WARN         (1 << 2)   /* Print warnings */
#define BOARD_DBG_VERBOSE      (1 << 3)   /* General verbose output */
#define BOARD_DBG_COUNT        4

#if defined(NO_DEBUG_OUTPUT_DEFAULT)
#define BOARD_DBG_DEFAULT 0
#else
#define BOARD_DBG_DEFAULT (BOARD_DBG_NORMAL | BOARD_DBG_ERR | BOARD_DBG_WARN)
#endif

#define BOARD_DBG_NAMES    \
    "NORmal",            \
    "ERRor",             \
    "WARN",              \
    "VERbose"


/*
 * Proper use requires parentheses.  E.g.:
 *     BOARD_DEBUG(BOARD_DBG_FOO, ("Problem %d with unit %d\n", pr, unit));
 */

extern uint32 board_debug_level;
extern char *board_debug_names[];
extern int (*board_debug_print)(const char *format, ...);

#define BOARD_DEBUG_CHECK(flags) (((flags) & board_debug_level) == (flags))
#define BOARD_DEBUG(flags, stuff) \
    if (BOARD_DEBUG_CHECK(flags) && board_debug_print != 0) \
	(*board_debug_print) stuff

#define BOARD_OUT(stuff) BOARD_DEBUG(BOARD_DBG_NORMAL, stuff)
#define BOARD_VERB(stuff) BOARD_DEBUG(BOARD_DBG_VERBOSE, stuff)
#define BOARD_WARN(stuff) BOARD_DEBUG(BOARD_DBG_WARN, stuff)
#define BOARD_ERR(stuff) BOARD_DEBUG(BOARD_DBG_ERR, stuff)

#else

#define BOARD_DEBUG_CHECK(flags) 0
#define BOARD_DEBUG(flags, stuff)
#define BOARD_OUT(stuff)
#define BOARD_VERB(stuff)
#define BOARD_WARN(stuff)
#define BOARD_ERR(stuff)

#endif /* BROADCOM_DEBUG */




#endif /* _BOARD_DEBUG_H_ */
