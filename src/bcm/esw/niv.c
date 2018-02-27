/*
 * $Id: niv.c 1.11.6.2 Broadcom SDK $
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
 * Purpose: Implements ESW NIV APIs
 */

#if defined(INCLUDE_L3)

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/niv.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/trident.h>

/* Flag to check initialized status */
STATIC int niv_initialized[BCM_MAX_NUM_UNITS];

#define NIV_INIT(unit)                                    \
    do {                                                  \
        if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {  \
            return BCM_E_UNIT;                            \
        }                                                 \
        if (!niv_initialized[unit]) {                     \
            return BCM_E_INIT;                            \
        }                                                 \
    } while (0)

/* 
 * NIV module lock
 */
STATIC sal_mutex_t niv_mutex[BCM_MAX_NUM_UNITS] = {NULL};

#define NIV_LOCK(unit) \
        sal_mutex_take(niv_mutex[unit], sal_mutex_FOREVER);

#define NIV_UNLOCK(unit) \
        sal_mutex_give(niv_mutex[unit]); 

/*
 * Function:
 *      _bcm_niv_free_resources
 * Purpose:
 *      Free all allocated tables and memory
 * Parameters:
 *      unit - SOC unit number
 * Returns:
 *      Nothing
 */
STATIC void
_bcm_esw_niv_free_resources(int unit)
{
    if (niv_mutex[unit]) {
        sal_mutex_destroy(niv_mutex[unit]);
        niv_mutex[unit] = NULL;
    } 
}

/*
 * Function:
 *		bcm_esw_niv_init
 * Purpose:
 *		Init  NIV module
 * Parameters:
 *		IN :  unit
 * Returns:
 *		BCM_E_XXX
 */

int
bcm_esw_niv_init(int unit)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {

        if (niv_initialized[unit]) {
            BCM_IF_ERROR_RETURN(bcm_esw_niv_cleanup(unit));
        }

        BCM_IF_ERROR_RETURN(bcm_trident_niv_init(unit));

        if (niv_mutex[unit] == NULL) {
            niv_mutex[unit] = sal_mutex_create("niv mutex");
            if (niv_mutex[unit] == NULL) {
                _bcm_esw_niv_free_resources(unit);
                return BCM_E_MEMORY;
            }
        }

        niv_initialized[unit] = TRUE;

        return BCM_E_NONE;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}


 /* Function:
 *      bcm_niv_cleanup
 * Purpose:
 *      Detach NIV module, clear all HW states
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_niv_cleanup(int unit)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {

        BCM_IF_ERROR_RETURN(bcm_trident_niv_cleanup(unit));

        _bcm_esw_niv_free_resources(unit);

        niv_initialized[unit] = FALSE;

        return BCM_E_NONE;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

/*
 * Purpose:
 *	  Create NIV Port
 * Parameters:
 *	  unit	  - (IN) Device Number
 *	  niv_port - (IN/OUT) NIV port information (OUT : niv_port_id)
 */

int bcm_esw_niv_port_add(int unit, bcm_niv_port_t *niv_port)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {

        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_port_add(unit, niv_port);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}
  
 /*
  * Purpose:
  *	  Delete NIV Port
  * Parameters:
  *	 unit		 - (IN) Device Number
  * 	 niv_port_id - (IN) NIV port ID
  */
  
int bcm_esw_niv_port_delete(int unit, bcm_gport_t niv_port_id)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_port_delete(unit, niv_port_id);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

 /*
  * Purpose:
  *	  Delete all NIV Ports
  * Parameters:
  *	  unit - Device Number
  */

int bcm_esw_niv_port_delete_all(int unit)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_port_delete_all(unit);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

 /*
  * Purpose:
  *      Get a NIV Port
  * Parameters:
  *      unit    - (IN) Device Number
  *      niv_port - (IN/OUT) NIV port information (IN : niv_port_id)
  */

int bcm_esw_niv_port_get(int unit, bcm_niv_port_t *niv_port)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_port_get(unit, niv_port);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

/*
 * Purpose:
 *      Traverse all valid NIV port entries and call the
 *      supplied callback routine.
 * Parameters:
 *      unit      - Device Number
 *      cb        - User callback function, called once per NIV Port entry.
 *      user_data - cookie
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_niv_port_traverse(int unit,
                          bcm_niv_port_traverse_cb cb,
                          void *user_data)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_port_traverse(unit, cb, user_data);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

/*
 * Purpose:
 *	Create NIV Forwarding table entry
 * Parameters:
 *	unit - (IN) Device Number
 *	iv_fwd_entry - (IN) NIV Forwarding table entry
 */
int
bcm_esw_niv_forward_add(int unit, bcm_niv_forward_t *iv_fwd_entry)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_forward_add(unit, iv_fwd_entry);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}
  
/*
 * Purpose:
 *	Delete NIV Forwarding table entry
 * Parameters:
 *      unit - (IN) Device Number
 *      iv_fwd_entry - (IN) NIV Forwarding table entry
 */
int
bcm_esw_niv_forward_delete(int unit, bcm_niv_forward_t *iv_fwd_entry)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_forward_delete(unit, iv_fwd_entry);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

/*
 * Purpose:
 *	Delete all NIV Forwarding table entries
 * Parameters:
 *	unit - Device Number
 */
int
bcm_esw_niv_forward_delete_all(int unit)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_forward_delete_all(unit);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

/*
 * Purpose:
 *      Get NIV Forwarding table entry
 * Parameters:
 *      unit - (IN) Device Number
 *      iv_fwd_entry - (IN/OUT) NIV forwarding table info
 */
int
bcm_esw_niv_forward_get(int unit, bcm_niv_forward_t *iv_fwd_entry)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_forward_get(unit, iv_fwd_entry);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

/*
 * Purpose:
 *      Traverse all valid NIV forward entries and call the
 *      supplied callback routine.
 * Parameters:
 *      unit      - Device Number
 *      cb        - User callback function, called once per NIV forward entry.
 *      user_data - cookie
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_niv_forward_traverse(int unit,
                             bcm_niv_forward_traverse_cb cb,
                             void *user_data)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        int rv;

        NIV_INIT(unit);

        NIV_LOCK(unit);

        rv = bcm_trident_niv_forward_traverse(unit, cb, user_data);

        NIV_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_niv_sw_dump
 * Purpose:
 *     Displays NIV information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_niv_sw_dump(int unit)
{
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_niv)) {
        bcm_trident_niv_sw_dump(unit);
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#else   /* INCLUDE_L3 */
int bcm_esw_niv_not_empty;
#endif  /* INCLUDE_L3 */
