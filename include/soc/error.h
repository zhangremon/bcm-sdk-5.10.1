/*
 * $Id: error.h 1.14.12.4 Broadcom SDK $
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

#ifndef _SOC_ERROR_H
#define _SOC_ERROR_H

#include <shared/error.h>

/*
 * Defines:	SOC_E_XXX
 * Purpose:	SOC API error codes
 * Notes:
 *      An error code may be converted to a string by passing
 *      the code to soc_errmsg().
 */

typedef enum {
    SOC_E_NONE                  = _SHR_E_NONE,
    SOC_E_INTERNAL              = _SHR_E_INTERNAL,
    SOC_E_MEMORY                = _SHR_E_MEMORY,
    SOC_E_UNIT                  = _SHR_E_UNIT,
    SOC_E_PARAM                 = _SHR_E_PARAM,
    SOC_E_EMPTY                 = _SHR_E_EMPTY,
    SOC_E_FULL                  = _SHR_E_FULL,
    SOC_E_NOT_FOUND             = _SHR_E_NOT_FOUND,
    SOC_E_EXISTS                = _SHR_E_EXISTS,
    SOC_E_TIMEOUT               = _SHR_E_TIMEOUT,
    SOC_E_BUSY                  = _SHR_E_BUSY,
    SOC_E_FAIL                  = _SHR_E_FAIL,
    SOC_E_DISABLED              = _SHR_E_DISABLED,
    SOC_E_BADID                 = _SHR_E_BADID,
    SOC_E_RESOURCE              = _SHR_E_RESOURCE,
    SOC_E_CONFIG                = _SHR_E_CONFIG,
    SOC_E_UNAVAIL               = _SHR_E_UNAVAIL,
    SOC_E_INIT                  = _SHR_E_INIT,
    SOC_E_PORT                  = _SHR_E_PORT,
    SOC_E_LIMIT                 = _SHR_E_LIMIT
} soc_error_t;

#define SOC_SUCCESS(rv)			_SHR_E_SUCCESS(rv)
#define SOC_FAILURE(rv)			_SHR_E_FAILURE(rv)
#define SOC_IF_ERROR_RETURN(op)		_SHR_E_IF_ERROR_RETURN(op)
#define SOC_IF_ERROR_JUST_RETURN(op) \
    do { int __rv__; if ((__rv__ = (op)) < 0) return ; } while(0)

#define	soc_errmsg(rv)			_SHR_ERRMSG(rv)

typedef enum {
    SOC_SWITCH_EVENT_IO_ERROR       = _SHR_SWITCH_EVENT_IO_ERROR,
    SOC_SWITCH_EVENT_PARITY_ERROR   = _SHR_SWITCH_EVENT_PARITY_ERROR,
    SOC_SWITCH_EVENT_THREAD_ERROR   = _SHR_SWITCH_EVENT_THREAD_ERROR,
    SOC_SWITCH_EVENT_ACCESS_ERROR   = _SHR_SWITCH_EVENT_ACCESS_ERROR,
    SOC_SWITCH_EVENT_ASSERT_ERROR   = _SHR_SWITCH_EVENT_ASSERT_ERROR,
    SOC_SWITCH_EVENT_MODID_CHANGE   = _SHR_SWITCH_EVENT_MODID_CHANGE,
    SOC_SWITCH_EVENT_DOS_ATTACK     = _SHR_SWITCH_EVENT_DOS_ATTACK,
    SOC_SWITCH_EVENT_STABLE_FULL    = _SHR_SWITCH_EVENT_STABLE_FULL,
    SOC_SWITCH_EVENT_STABLE_ERROR   = _SHR_SWITCH_EVENT_STABLE_ERROR,
    SOC_SWITCH_EVENT_UNCONTROLLED_SHUTDOWN   = _SHR_SWITCH_EVENT_UNCONTROLLED_SHUTDOWN,
    SOC_SWITCH_EVENT_WARM_BOOT_DOWNGRADE     = _SHR_SWITCH_EVENT_WARM_BOOT_DOWNGRADE,
    SOC_SWITCH_EVENT_TUNE_ERROR     = _SHR_SWITCH_EVENT_TUNE_ERROR,
    SOC_SWITCH_EVENT_EPON_ALARM
} soc_switch_event_t;

/* Early demand request error event */
#define SOC_SWITCH_EVENT_TUNE_ERROR_QM_BLOCK  1

/* Top level parity error info returned in event callbacks */
typedef enum {
    SOC_SWITCH_EVENT_DATA_ERROR_PARITY = 1,
    SOC_SWITCH_EVENT_DATA_ERROR_ECC,
    SOC_SWITCH_EVENT_DATA_ERROR_UNSPECIFIED,
    SOC_SWITCH_EVENT_DATA_ERROR_FATAL
} soc_switch_data_error_t;

#define SOC_ERROR_BLK_BP          24
#define SOC_ERROR_PIPE_BP         16
#define SOC_ERROR_FIELD_ENUM_MASK 0xffff

typedef enum {
    SOC_SWITCH_EVENT_THREAD_L2X      = 1,
    SOC_SWITCH_EVENT_THREAD_COUNTER,
    SOC_SWITCH_EVENT_THREAD_MEMSCAN,
    SOC_SWITCH_EVENT_THREAD_L2MOD,
    SOC_SWITCH_EVENT_THREAD_L2MOD_DMA,
    SOC_SWITCH_EVENT_THREAD_IPFIX,
    SOC_SWITCH_EVENT_THREAD_LINKSCAN,
    SOC_SWITCH_EVENT_THREAD_REGEX_REPORT,
    SOC_SWITCH_EVENT_THREAD_HWDOS_MONITOR
} soc_switch_thread_id_t;
#endif  /* !_SOC_ERROR_H */
