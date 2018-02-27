/*
 * $Id: ptp.c 1.2.2.3 Broadcom SDK $
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
 */

#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/profile_mem.h>
#include <soc/debug.h>
#include <soc/hash.h>
#include <soc/l2x.h>
#include <soc/l3x.h>
#include <soc/katana.h>

#include <bcm/l3.h>
#include <bcm/ptp.h>

#include <bcm_int/esw/port.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/switch.h>
#include <bcm_int/esw/katana.h>
#include <bcm_int/esw/virtual.h>
#include <bcm_int/esw_dispatch.h>

#if defined(INCLUDE_PTP)
#include <bcm_int/esw/ptp.h>
#endif

#if defined(BCM_TRIUMPH_SUPPORT)
#include <bcm_int/esw/triumph.h>
#endif /* BCM_TRIUMPH_SUPPORT */


#if defined(BCM_KATANA_SUPPORT)

#if defined(INCLUDE_PTP)

#define CHECK_INIT \
    if (!ptp_info_p->initialized) \
    { \
        return BCM_E_INIT; \
    }

#define SET_KT_PTP_INFO SET_PTP_INFO
/* was ptp_info_p = &kt_ptp_info[unit]; */

/* static _bcm_ptp_info_t kt_ptp_info[BCM_MAX_NUM_UNITS]; */

#if 0
static void *_bcm_kt_ptp_alloc_clear(unsigned int size, char *description)
{
    void *block_p;

    block_p = sal_alloc(size, description);

    if (block_p != NULL)
    {
        sal_memset(block_p, 0, size);
    }

    return block_p;
}
#endif
#ifdef BCM_WARM_BOOT_SUPPORT

#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0

static int _bcm_kt_ptp_warm_boot(int unit)
{
    return BCM_E_NONE;
}

#endif /* BCM_WARM_BOOT_SUPPORT */


static void _bcm_kt_ptp_free_memory(_bcm_ptp_info_t *ptp_info_p)
{
}

/*
 * Function:
 *      bcm_kt_ptp_init
 * Purpose:
 *      Initialize the PTP subsystem
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_kt_ptp_init(
    int unit)
{
    _bcm_ptp_info_t *ptp_info_p;

    SET_KT_PTP_INFO;

    if (ptp_info_p->initialized)
    {
        /* Reset everything */

        _bcm_kt_ptp_free_memory(ptp_info_p);
    }
    
#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit))
    {
        _bcm_kt_ptp_warm_boot(unit);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */
   
    ptp_info_p->initialized = 1;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_kt_ptp_detach
 * Purpose:
 *      Shut down the PTP subsystem
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_kt_ptp_detach(
    int unit)
{
   
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_kt_ptp_stack_create
 * Purpose:
 *      Create a PTP stack instance
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_info - (IN/OUT) Pointer to an PTP Stack Info structure
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_kt_ptp_stack_create(
    int unit, 
    bcm_ptp_stack_info_t *ptp_info)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_kt_ptp_firmware_load
 * Purpose:
 *      Load PTP stack firmware onto RCPU.  Keystone only.
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_info - (IN) PTP Stack ID
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_kt_ptp_firmware_load(
    int unit, 
    bcm_ptp_stack_id_t ptp_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_kt_ptp_time_format_set
 * Purpose:
 *      Set Time Of Day format for PTP stack instance.
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_info - (IN) PTP Stack ID
 *      format - (IN) Time of Day format
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_kt_ptp_time_format_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    bcm_ptp_time_type_t type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_kt_ptp_event_register
 * Purpose:
 *      Register a callback for handling PTP events
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb_types - (IN) The set of PTP callback types for which the specified callback should be called
 *      cb - (IN) A pointer to the callback function to call
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_kt_ptp_cb_register(
    int unit, 
    bcm_ptp_cb_types_t cb_types, 
    bcm_ptp_cb cb, 
    void *user_data)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_kt_ptp_cb_unregister
 * Purpose:
 *      Unregister a callback for handling PTP events
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb_types - (IN) The set of PTP callbacks to unregister for the specified callback
 *      cb - (IN) A pointer to the callback function to unregister
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_kt_ptp_cb_unregister(
    int unit, 
    bcm_ptp_cb_types_t cb_types, 
    bcm_ptp_cb cb)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_kt_ptp_clock_create
 * Purpose:
 *      Create PTP Clock instance.
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_id - (IN) PTP Stack ID
 *      clock_info - (IN/OUT) PTP Clock Information
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_kt_ptp_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    bcm_ptp_clock_info_t *clock_info)
{
    return BCM_E_UNAVAIL; 
}

#if defined(BCM_WARM_BOOT_SUPPORT)
int _bcm_kt_ptp_sync(int unit)
{
   
    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_ptp_sw_dump
 * Purpose:
 *     Displays PTP information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_kt_ptp_sw_dump(int unit)
{
    _bcm_ptp_info_t *ptp_info_p;

    SET_KT_PTP_INFO;

    soc_cm_print("\nSW Information PTP - Unit %d\n", unit);

    return;
}
#endif
#endif /* defined(INCLUDE_PTP) */
#endif /* defined(BCM_KATANA) */
