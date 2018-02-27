/*
 * $Id: trill.c 1.13.6.4 Broadcom SDK $
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
 * ESW Trill API
 */

#ifdef INCLUDE_L3

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/trident.h>
#include <bcm/trill.h>

/*
 * Function:
 *    bcm_esw_trill_init
 * Purpose:
 *    Init  TRILL module
 * Parameters:
 *    IN :  unit
 * Returns:
 *    BCM_E_XXX
 */

int
bcm_esw_trill_init(int unit)
{
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
        return bcm_td_trill_init(unit);
    }
#endif
    return BCM_E_UNAVAIL;
}


 /* Function:
 *      bcm_trill_cleanup
 * Purpose:
 *      Detach the TRILL module, clear all HW states
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_trill_cleanup(int unit)
{
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
        return bcm_td_trill_cleanup(unit);
    }
#endif
    return BCM_E_UNAVAIL;
}

 /*
  * Purpose:
  *     Add Trill port to TRILL Cloud
  * Parameters:
  *  unit  - (IN) Device Number
  *  trill_port - (IN/OUT) trill port information (OUT : trill_port_id)
 */
int
bcm_esw_trill_port_add(int unit, bcm_trill_port_t *trill_port)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =  bcm_td_trill_port_add(unit, trill_port);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}
  
 /*
  * Purpose:
  *   Delete TRILL port from TRILL Cloud
  * Parameters:
  *    unit  - (IN) Device Number
  *    trill_port_id - (IN) Trill port ID
  */
  
int 
bcm_esw_trill_port_delete(int unit, bcm_gport_t trill_port_id)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =  bcm_td_trill_port_delete(unit, trill_port_id);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

 /*
  * Purpose:
  *	  Delete all TRILL ports from TRILL Cloud
  * Parameters:
  *	  unit - Device Number
  */

int 
bcm_esw_trill_port_delete_all(int unit)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =  bcm_td_trill_port_delete_all(unit);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

 /*
  * Purpose:
  *      Get a TRILL port 
  * Parameters:
  *      unit    - (IN) Device Number
  *      trill_port - (IN/OUT) Trill port information (IN : trill_port_id)
  */

int 
bcm_esw_trill_port_get(int unit, bcm_trill_port_t *trill_port)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =  bcm_td_trill_port_get(unit, trill_port);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

/*
 * Purpose:
 *      Get all TRILL ports from TRILL Cloud
 * Parameters:
 *      unit     - (IN) Device Number
 *      port_max   - (IN) Maximum number of TRILL ports in array
 *      port_array - (OUT) Array of TRILL ports
 *      port_count - (OUT) Number of TRILL ports returned in array
 *
 */

int 
bcm_esw_trill_port_get_all(int unit, 
                           int port_max, 
                           bcm_trill_port_t *trill_port, 
                           int *port_count )
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =  bcm_td_trill_port_get_all(unit, port_max, trill_port, port_count);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

 /*
 * Purpose:
 *     Add TRILL multicast entry
 * Parameters:
 *     unit   - Device Number
 *     flags
 *     root_name  - (IN) trill Root_RBridge
 *     vlan - (IN) Customer VLAN
 *     c_dmac - (IN) Customer D-MAC
 *     group  - (IN) Multicast Group ID
 */

int
bcm_esw_trill_multicast_add (int unit,  uint32 flags,
                             bcm_trill_name_t  root_name, 
                             bcm_vlan_t vlan, 
                             bcm_mac_t c_dmac,
                             bcm_multicast_t group)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =  bcm_td_trill_multicast_add(unit, flags, root_name, 
                                               vlan, c_dmac, group);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

 /*
 * Purpose:
 *        Delete TRILL multicast entry
 * Parameters:
 *        unit   - Device Number
 *        flags
 *        root_name  - (IN) Trill Root_RBridge
 *         vlan		- (IN) Customer VLAN
 *         c_dmac	- (IN) Customer D-MAC
 *         group        - (IN) Multicast Group ID
 */

int
bcm_esw_trill_multicast_delete ( int unit, uint32 flags,
                                 bcm_trill_name_t	root_name,
                                 bcm_vlan_t vlan,
                                 bcm_mac_t c_dmac,                                                               
                                 bcm_multicast_t group)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =  bcm_td_trill_multicast_delete(unit, flags, root_name, 
                             vlan, c_dmac, group);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}
 
 /*
 * Purpose:
 *    Delete all TRILL multicast entries
 * Parameters:
 *    unit   - Device Number
 *    root_name  - (IN) Trill Root_RBridge
 */

int
bcm_esw_trill_multicast_delete_all (int  unit,  bcm_trill_name_t  root_name)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =  bcm_td_trill_multicast_delete_all(unit, root_name);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}
 
 /*
 * Purpose:
  *    Traverse all valid TRILL Multicast entries and call the
  *     supplied callback routine.
  * Parameters:
  *    unit   - Device Number
  *    cb    - User callback function, called once per Trill Multicast entry.
  *    user_data - cookie
 */

int
bcm_esw_trill_multicast_traverse( int unit, 
                                  bcm_trill_multicast_traverse_cb  cb,
                                  void *user_data)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =  bcm_td_trill_multicast_traverse(unit, cb, user_data);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

/*
* Purpose:
*     Add TRILL multicast RPF entry
* Parameters:
*     unit  - Device Number
*     root_name  - (IN) Trill Root_RBridge
*     source_name - (IN) Source Rbridge
*     port   - (IN) Ingress port
*/

int bcm_esw_trill_multicast_source_add(int unit, 
                                                                            bcm_trill_name_t root_name, 
                                                                            bcm_trill_name_t source_name, 
                                                                            bcm_gport_t port)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =	bcm_td_trill_multicast_source_add(unit, root_name, 
                                                                             source_name, port);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT

/*
 * Function:
 *    _bcm_esw_trill_sync
 * Purpose:
 *    Store Trill state to persistent memory.
 * Parameters:
 *    IN :  unit
 * Returns:
 *    BCM_E_XXX
 */

int
_bcm_esw_trill_sync(int unit)
{
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
        return bcm_td_trill_sync(unit);
    }
#endif
    return BCM_E_UNAVAIL;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
* Purpose:
*      Delete TRILL multicast RPF entry
* Parameters:
*        unit  - Device Number
*        root_name  - (IN) Trill Root_RBridge
*        source_name - (IN) Source Rbridge
*        port   - (IN) Ingress port
*/

int bcm_esw_trill_multicast_source_delete(int unit, 
                                                                                 bcm_trill_name_t root_name, 
                                                                                 bcm_trill_name_t source_name, 
                                                                                 bcm_gport_t port)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =	bcm_td_trill_multicast_source_delete(unit, root_name, 
                                                                               source_name, port);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

/*
* Purpose:
*    Get TRILL multicast RPF entry
* Parameters:
*   unit  - Device Number
*   root_name (IN) Trill Root_RBridge
*   source_name - (IN) Source Rbridge
*   port    - (OUT) Ingress port
*/

int bcm_esw_trill_multicast_source_get(int unit, 
                                                                           bcm_trill_name_t root_name, 
                                                                           bcm_trill_name_t source_name, 
                                                                           bcm_gport_t *port)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =	bcm_td_trill_multicast_source_get(unit, root_name, 
                                                                     source_name, port);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

/*
* Purpose:
 *  Traverse all valid TRILL Multicast RPF entries and call the
 *  supplied callback routine.
 * Parameters:
 *  unit  - Device Number
 *     cb  - User callback function, called once per Trill Multicast RPF entry.
 *     user_data - cookie
*/

int bcm_esw_trill_multicast_source_traverse(int unit, 
                                                                                     bcm_trill_multicast_source_traverse_cb cb, 
                                                                                     void *user_data)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv =	bcm_td_trill_multicast_source_traverse(unit, cb, user_data);
              bcm_td_trill_unlock (unit);
         }
    }
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_trill_stat_get
 * Purpose:
 *      Get TRILL Stats
 * Parameters:
 *      unit   - (IN) SOC unit #
 *      port   - (IN) port
 *      stat   - (IN)  specify the Stat type
 *      val    - (OUT) 64-bit Stats value
 * Returns:
 *      BCM_E_XXX
 */     

int
bcm_esw_trill_stat_get(int unit, bcm_port_t port,
                            bcm_trill_stat_t stat, uint64 *val)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
       if ( rv == BCM_E_NONE ) {
           rv = bcm_td_trill_stat_get(unit, port, stat, val);
           bcm_td_trill_unlock (unit);
       }
       return rv;
    }
#endif
    return rv;
}

int
bcm_esw_trill_stat_get32(int unit, bcm_port_t port,
                              bcm_trill_stat_t stat, uint32 *val)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
       if ( rv == BCM_E_NONE ) {
           rv = bcm_td_trill_stat_get32(unit, port, stat, val);
           bcm_td_trill_unlock (unit);
       }
       return rv;
    }
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_trill_stat_clear
 * Purpose:
 *      Clear TRILL Stats
 * Parameters:
 *      unit   - (IN) SOC unit #
 *      port   - (IN) port
 *      stat   - (IN)  specify the Stat type
 * Returns:
 *      BCM_E_XXX
 */     

int
bcm_esw_trill_stat_clear(int unit, bcm_port_t port,
                            bcm_trill_stat_t stat)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_lock(unit);
       if ( rv == BCM_E_NONE ) {
             rv = bcm_td_trill_stat_clear(unit, port, stat);
             bcm_td_trill_unlock (unit);
         }
         return rv;
    }
#endif
    return rv;
}
#else   /* INCLUDE_L3 */
 int bcm_esw_trill_not_empty;
#endif  /* INCLUDE_L3 */
