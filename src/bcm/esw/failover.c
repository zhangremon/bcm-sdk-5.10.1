/*
 * $Id: failover.c 1.12.6.3 Broadcom SDK $
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
 * ESW failover API
 */


#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw_dispatch.h>
#if defined(BCM_TRIUMPH2_SUPPORT)
#include <bcm_int/esw/field.h>
#include <bcm_int/esw/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */


/*
 * Function:
 *		bcm_esw_failover_init
 * Purpose:
 *		Init  failover module
 * Parameters:
 *		IN :  unit
 * Returns:
 *		BCM_E_XXX
 */

int
bcm_esw_failover_init(int unit)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_failover)) {
        return bcm_tr2_failover_init(unit);
    }
#endif
    return BCM_E_UNAVAIL;
}


 /* Function:
 *      bcm_failover_cleanup
 * Purpose:
 *      Detach the failover module, clear all HW states
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_failover_cleanup(int unit)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_failover)) {
        return bcm_tr2_failover_cleanup(unit);
    }
#endif
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *		bcm_esw_failover_create
 * Purpose:
 *		Create a failover object
 * Parameters:
 *		IN :  unit
 *           IN :  flags
 *           OUT :  failover_id
 * Returns:
 *		BCM_E_XXX
 */

int 
bcm_esw_failover_create(int unit, uint32 flags, bcm_failover_t *failover_id)
{
    int rv = BCM_E_UNAVAIL;	
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_failover)) {
         rv = bcm_tr2_failover_lock (unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr2_failover_create(unit, flags, failover_id);
              bcm_tr2_failover_unlock (unit);
         }
         return rv;
    }
#endif
    return rv;
}

/*
 * Function:
 *		bcm_esw_failover_destroy
 * Purpose:
 *		Destroy a failover object
 * Parameters:
 *		IN :  unit
 *           IN :  failover_id
 * Returns:
 *		BCM_E_XXX
 */

int 
bcm_esw_failover_destroy(int unit, bcm_failover_t failover_id)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_failover)) {
         rv = bcm_tr2_failover_lock (unit);
         if (  rv == BCM_E_NONE ) {
              rv = bcm_tr2_failover_destroy(unit, failover_id);
              bcm_tr2_failover_unlock (unit);
         }
         return rv;
   }
#endif
    return rv;
}

/*
 * Function:
 *		bcm_esw_failover_set
 * Purpose:
 *		Set a failover object to enable or disable (note that failover object
 *            0 is reserved
 * Parameters:
 *		IN :  unit
 *           IN :  failover_id
 *           IN :  value
 * Returns:
 *		BCM_E_XXX
 */


int 
bcm_esw_failover_set(int unit, bcm_failover_t failover_id, int value)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_failover)) {
         rv = bcm_tr2_failover_lock (unit);
         if ( rv == BCM_E_NONE ) {
                   rv = bcm_tr2_failover_set(unit, failover_id, value);
                   bcm_tr2_failover_unlock (unit);
         }
         return rv;
    }
#endif
    return rv;
}

/*
 * Function:
 *		bcm_esw_failover_get
 * Purpose:
 *		Get the enable status of a failover object
 * Parameters:
 *		IN :  unit
 *           IN :  failover_id
 *           OUT :  value
 * Returns:
 *		BCM_E_XXX
 */


int 
bcm_esw_failover_get(int unit, bcm_failover_t failover_id, int *value)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_failover)) {
         rv = bcm_tr2_failover_lock (unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr2_failover_get(unit, failover_id, value);
              bcm_tr2_failover_unlock (unit);
         }
         return rv;
    }
#endif
    return rv;
}

/*
 * Function:
 *		bcm_esw_failover_prot_nhi_create
 * Purpose:
 *		Create  the  entry for  PROT_NHI
 * Parameters:
 *		IN :  unit
 *           IN :  Primary Next Hop Index
 *           IN :  Failover Group Index
 * Returns:
 *		BCM_E_XXX
 */


int
bcm_esw_failover_prot_nhi_create (int unit, int nh_index)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIUMPH2_SUPPORT
      if (soc_feature(unit, soc_feature_failover)) {
         rv = bcm_tr2_failover_lock (unit); 
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr2_failover_prot_nhi_create(unit, nh_index);
              bcm_tr2_failover_unlock (unit);
         }
         return rv;
    }
#endif
    return rv;
}


/*
 * Function:
 *		bcm_esw_failover_prot_nhi_set
 * Purpose:
 *		Set the parameters for PROT_NHI
 * Parameters:
 *		IN :  unit
 *           IN :  Primary Next Hop Index
 *           IN :  Protection Next Hop Index
 * Returns:
 *		BCM_E_XXX
 */

int
bcm_esw_failover_prot_nhi_set(int unit, int nh_index, uint32 prot_nh_index, bcm_failover_t failover_id)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_failover)) {
         rv = bcm_tr2_failover_lock (unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr2_failover_prot_nhi_set(unit, nh_index, prot_nh_index, failover_id);
              bcm_tr2_failover_unlock (unit);
         }
         return rv;
    }
#endif
    return rv;
}

/*
 * Function:
 *		bcm_esw_failover_prot_nhi_get
 * Purpose:
 *		Get the parameters for PROT_NHI
 * Parameters:
 *		IN :  unit
 *           IN :  primary Next Hop Index
 *           OUT :  Failover Group Index
 *           OUT : Protection Next Hop Index
 * Returns:
 *		BCM_E_XXX
 */


int
bcm_esw_failover_prot_nhi_get(int unit, int nh_index, bcm_failover_t  *failover_id, int  *prot_nh_index)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_failover)) {
         rv = bcm_tr2_failover_lock (unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr2_failover_prot_nhi_get(unit, nh_index, failover_id, prot_nh_index);
              bcm_tr2_failover_unlock (unit);
         }
         return rv;
    }
#endif
    return rv;
}
