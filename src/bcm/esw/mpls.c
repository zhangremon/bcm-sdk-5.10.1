/*
 * $Id: mpls.c 1.104.2.11 Broadcom SDK $
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
 * File:    mpls.c
 * Purpose: Manages MPLS functions
 */

#ifdef INCLUDE_L3

#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/l2u.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/rate.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/stack.h>
#include <bcm/topo.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/firebolt.h>
#if defined(BCM_EASYRIDER_SUPPORT)
#include <bcm_int/esw/easyrider.h>
#endif /* BCM_EASYRIDER_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT)
#include <bcm_int/esw/triumph.h>
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT)
#include <bcm_int/esw/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/katana.h>
#endif /* BCM_TRIUMPH_SUPPORT */

#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/xgs3.h>

#define L3_LOCK(_unit_)   _bcm_esw_l3_lock(_unit_)
#define L3_UNLOCK(_unit_) _bcm_esw_l3_unlock(_unit_)

/*
 * Function:
 *      bcm_mpls_init
 * Purpose:
 *      Initialize the MPLS software module, clear all HW MPLS states
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_init(int unit)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
        return bcm_tr_mpls_init(unit);
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_init(unit);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_cleanup
 * Purpose:
 *      Detach the MPLS software module, clear all HW MPLS states
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_cleanup(int unit)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
        return bcm_tr_mpls_cleanup(unit);
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_cleanup(unit);
    }
#endif
#endif
    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT
int
_bcm_esw_mpls_sync(int unit)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
        return bcm_tr_mpls_sync(unit);
    }
#endif
#endif

    return BCM_E_NONE;
}
#endif

/*
 * Function:
 *      bcm_mpls_vpn_id_create
 * Purpose:
 *      Create a VPN
 * Parameters:
 *      unit  - (IN)  Device Number
 *      info  - (IN/OUT) VPN configuration info
 * Returns:
 *      BCM_E_XXXX
 */
int
bcm_esw_mpls_vpn_id_create(int unit, bcm_mpls_vpn_config_t *info)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
        rv = bcm_tr_mpls_lock (unit) ;
        if ( rv == BCM_E_NONE ) {
             rv = bcm_tr_mpls_vpn_id_create(unit, info);
             bcm_tr_mpls_unlock (unit);
        }
        return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_vpn_id_create(unit, info);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_vpn_id_destroy
 * Purpose:
 *      Destroy a VPN
 * Parameters:
 *      unit       - (IN)  Device Number
 *      vpn        - (IN)  VPN instance
 * Returns:
 *      BCM_E_XXXX
 */
int
bcm_esw_mpls_vpn_id_destroy(int unit, bcm_vpn_t vpn)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
       rv = bcm_tr_mpls_lock(unit);
       if ( rv == BCM_E_NONE ) {
             rv = bcm_tr_mpls_vpn_id_destroy(unit, vpn);
             bcm_tr_mpls_unlock (unit);
       }
       return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_vpn_id_destroy(unit, vpn);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_vpn_id_destroy_all
 * Purpose:
 *      Destroy all VPNs
 * Parameters:
 *      unit       - (IN)  Device Number
 * Returns:
 *      BCM_E_XXXX
 */
int
bcm_esw_mpls_vpn_id_destroy_all(int unit)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
       rv = bcm_tr_mpls_lock(unit);
       if ( rv == BCM_E_NONE ) {
           rv = bcm_tr_mpls_vpn_id_destroy_all(unit);
           bcm_tr_mpls_unlock (unit);
       }
       return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_vpn_id_destroy_all(unit);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_vpn_id_get
 * Purpose:
 *      Get a VPN
 * Parameters:
 *      unit  - (IN)  Device Number
 *      vpn   - (IN)  VPN instance
 *      info  - (OUT) VPN configuration info
 * Returns:
 *      BCM_E_XXXX
 */
int
bcm_esw_mpls_vpn_id_get(int unit, bcm_vpn_t vpn, bcm_mpls_vpn_config_t *info)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
        rv = bcm_tr_mpls_lock(unit);
        if ( rv == BCM_E_NONE ) {
            rv =  bcm_tr_mpls_vpn_id_get(unit, vpn, info);
            bcm_tr_mpls_unlock (unit);
        }
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_vpn_traverse
 * Purpose:
 *      MPLS VPN Traverse
 * Parameters:
 *      unit  - (IN)  Device Number
 *      cb   - (IN)  User callback function, called once per entry
 *      user_data  -  (IN/OUT) Cookie
 * Returns:
 *      BCM_E_XXXX
 */
int 
bcm_esw_mpls_vpn_traverse(int unit, bcm_mpls_vpn_traverse_cb cb, 
                                                          void *user_data)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
        rv = bcm_tr_mpls_lock(unit);
        if ( rv == BCM_E_NONE ) {
            rv = bcm_tr_mpls_vpn_traverse(unit, cb, user_data);
            bcm_tr_mpls_unlock (unit);
        }
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_port_add
 * Purpose:
 *      Add an mpls port to a VPN
 * Parameters:
 *      unit    - (IN) Device Number
 *      vpn     - (IN) VPN instance ID
 *      mpls_port - (IN/OUT) mpls port information (OUT : mpls_port_id)
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_port_add(int unit, bcm_vpn_t vpn, bcm_mpls_port_t *mpls_port)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
       L3_LOCK(unit);
       rv = bcm_tr_mpls_lock(unit);
       if ( rv == BCM_E_NONE ) {
           rv = bcm_tr_mpls_port_add(unit, vpn, mpls_port);
           bcm_tr_mpls_unlock (unit);
       }
       L3_UNLOCK(unit);
       return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_port_add(unit, vpn, mpls_port);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_port_delete
 * Purpose:
 *      Delete an mpls port from a VPN
 * Parameters:
 *      unit       - (IN) Device Number
 *      vpn        - (IN) VPN instance ID
 *      mpls_port_id - (IN) mpls port ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_port_delete(int unit, bcm_vpn_t vpn, bcm_gport_t mpls_port_id)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
       L3_LOCK(unit);
       rv = bcm_tr_mpls_lock(unit);
       if ( rv == BCM_E_NONE ) {
           rv = bcm_tr_mpls_port_delete(unit, vpn, mpls_port_id);
           bcm_tr_mpls_unlock (unit);
       }
       L3_UNLOCK(unit);
       return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_port_delete(unit, vpn, mpls_port_id);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_port_delete_all
 * Purpose:
 *      Delete all mpls ports from a VPN
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_port_delete_all(int unit, bcm_vpn_t vpn)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
       L3_LOCK(unit);
       rv = bcm_tr_mpls_lock(unit);
       if ( rv == BCM_E_NONE ) {
           rv = bcm_tr_mpls_port_delete_all(unit, vpn);
           bcm_tr_mpls_unlock (unit);
       }
       L3_UNLOCK(unit);
       return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
       rv = bcm_er_mpls_port_delete_all(unit, vpn);	
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_port_get
 * Purpose:
 *      Get an mpls port from a VPN
 * Parameters:
 *      unit    - (IN) Device Number
 *      vpn     - (IN) VPN instance ID
 *      mpls_port - (IN/OUT) mpls port information (IN : mpls_port_id)
 */
int
bcm_esw_mpls_port_get(int unit, bcm_vpn_t vpn, bcm_mpls_port_t *mpls_port)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
               rv = bcm_tr_mpls_port_get (unit, vpn, mpls_port);
               bcm_tr_mpls_unlock (unit);
           }
           return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_port_get(unit, vpn, mpls_port);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_port_get_all
 * Purpose:
 *      Get an mpls port from a VPN
 * Parameters:
 *      unit     - (IN) Device Number
 *      vpn      - (IN) VPN instance ID
 *      port_max   - (IN) Maximum number of ports in array
 *      port_array - (OUT) Array of mpls ports
 *      port_count - (OUT) Number of ports returned in array
 *
 */
int
bcm_esw_mpls_port_get_all(int unit, bcm_vpn_t vpn, int port_max,
                            bcm_mpls_port_t *port_array, int *port_count)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
       rv = bcm_tr_mpls_lock(unit);
       if ( rv == BCM_E_NONE ) {
            rv = bcm_tr_mpls_port_get_all(unit, vpn, port_max,
                                  port_array, port_count);
            bcm_tr_mpls_unlock (unit);
       }
       return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
         rv = bcm_er_mpls_port_get_all(unit, vpn, port_max,
                             port_array, port_count);
         return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_initiator_set
 * Purpose:
 *      Set the MPLS tunnel initiator parameters for an L3 interface.
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 *      num_labels  - Number of labels in the array
 *      label_array - Array of MPLS label and header information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_tunnel_initiator_set (int unit, bcm_if_t intf, int num_labels,
                                   bcm_mpls_egress_label_t *label_array)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
                rv = bcm_tr_mpls_tunnel_initiator_set(unit, intf, num_labels,
                                            label_array);
                bcm_tr_mpls_unlock (unit);
           }
           return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_tunnel_initiator_set(unit, intf, num_labels,
                                                label_array);	
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_initiator_clear
 * Purpose:
 *      Clear the MPLS tunnel initiator parameters for an L3 interface.
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_tunnel_initiator_clear (int unit, bcm_if_t intf)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
               rv = bcm_tr_mpls_tunnel_initiator_clear(unit, intf);
               bcm_tr_mpls_unlock (unit);
           }
           return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_tunnel_initiator_clear(unit, intf);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_initiator_clear_all
 * Purpose:
 *      Clear all the MPLS tunnel initiator parameters all L3 Interfaces
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_tunnel_initiator_clear_all (int unit)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
       if ( bcm_tr_mpls_lock (unit) == BCM_E_NONE ) {
           rv = bcm_tr_mpls_tunnel_initiator_clear_all(unit);
           bcm_tr_mpls_unlock (unit);
       }
       return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_initiator_get
 * Purpose:
 *      Get the the MPLS tunnel initiator parameters for an L3 interface.
 * Parameters:
 *      unit        - (IN) Device Number
 *      intf        - (IN) The egress L3 interface
 *      label_max   - (IN) Number of entries in label_array
 *      label_array - (OUT) MPLS header information
 *      label_count - (OUT) Actual number of labels returned
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_tunnel_initiator_get (int unit, bcm_if_t intf, int label_max,
                                   bcm_mpls_egress_label_t *label_array,
                                   int *label_count)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
               rv = bcm_tr_mpls_tunnel_initiator_get(unit, intf, label_max,
                                  label_array, label_count);
               bcm_tr_mpls_unlock (unit);
           }
           return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_tunnel_initiator_get(unit, intf, label_max,
                                                label_array, label_count);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_add
 * Purpose:
 *      Add an MPLS label entry.
 * Parameters:
 *      unit - Device Number
 *      info - Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_tunnel_switch_add (int unit, bcm_mpls_tunnel_switch_t *info)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit) && soc_feature(unit, soc_feature_mpls)) {
           L3_LOCK(unit);
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
              rv = bcm_kt_mpls_tunnel_switch_add(unit, info);
              bcm_tr_mpls_unlock (unit);
           }
           L3_UNLOCK(unit);
           return rv;
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
           L3_LOCK(unit);
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
              rv = bcm_tr_mpls_tunnel_switch_add(unit, info);
              bcm_tr_mpls_unlock (unit);
           }
           L3_UNLOCK(unit);
           return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_tunnel_switch_add(unit, info);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_delete
 * Purpose:
 *      Delete an MPLS label entry.
 * Parameters:
 *      unit - Device Number
 *      info - Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_tunnel_switch_delete (int unit, bcm_mpls_tunnel_switch_t *info)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit) && soc_feature(unit, soc_feature_mpls)) {
           L3_LOCK(unit);
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
              rv = bcm_kt_mpls_tunnel_switch_delete(unit, info);
              bcm_tr_mpls_unlock (unit);
           }
           L3_UNLOCK(unit);
           return rv;
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
           L3_LOCK(unit);
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
              rv = bcm_tr_mpls_tunnel_switch_delete(unit, info);
              bcm_tr_mpls_unlock (unit);
           }
           L3_UNLOCK(unit);
           return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_tunnel_switch_delete(unit, info);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_delete_all
 * Purpose:
 *      Delete all MPLS label entries.
 * Parameters:
 *      unit   - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_tunnel_switch_delete_all (int unit)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit) && soc_feature(unit, soc_feature_mpls)) {
           L3_LOCK(unit);
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
              rv = bcm_kt_mpls_tunnel_switch_delete_all(unit);
              bcm_tr_mpls_unlock (unit);
           }
           L3_UNLOCK(unit);
           return rv;
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
              L3_LOCK(unit);
              rv = bcm_tr_mpls_lock(unit);
              if ( rv == BCM_E_NONE ) {
                   rv = bcm_tr_mpls_tunnel_switch_delete_all(unit);
                   bcm_tr_mpls_unlock (unit);
              }
              L3_UNLOCK(unit);
              return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_tunnel_switch_delete_all(unit);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_get
 * Purpose:
 *      Get an MPLS label entry.
 * Parameters:
 *      unit - Device Number
 *      info - Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_tunnel_switch_get (int unit, bcm_mpls_tunnel_switch_t *info)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit) && soc_feature(unit, soc_feature_mpls)) {
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
              rv = bcm_kt_mpls_tunnel_switch_get(unit, info);
              bcm_tr_mpls_unlock (unit);
           }
           return rv;
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
              rv = bcm_tr_mpls_lock(unit);
              if ( rv == BCM_E_NONE ) {
                   rv = bcm_tr_mpls_tunnel_switch_get(unit, info);
                   bcm_tr_mpls_unlock (unit);
              }
              return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_tunnel_switch_get(unit, info);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_traverse
 * Purpose:
 *      Traverse all valid MPLS label entries and call the
 *      supplied callback routine.
 * Parameters:
 *      unit      - Device Number
 *      cb        - User callback function, called once per MPLS entry.
 *      user_data - cookie
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_tunnel_switch_traverse(int unit,
                                    bcm_mpls_tunnel_switch_traverse_cb cb,
                                    void *user_data)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit) && soc_feature(unit, soc_feature_mpls)) {
           rv = bcm_tr_mpls_lock(unit);
           if ( rv == BCM_E_NONE ) {
              rv = bcm_kt_mpls_tunnel_switch_traverse(unit, cb, user_data);
              bcm_tr_mpls_unlock (unit);
           }
           return rv;
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
              rv = bcm_tr_mpls_lock(unit);
              if ( rv == BCM_E_NONE ) {
                   rv = bcm_tr_mpls_tunnel_switch_traverse(unit, cb, user_data);
                   bcm_tr_mpls_unlock (unit);
              }
              return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_tunnel_switch_traverse(unit, cb, user_data);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_mpls_exp_map_create
 * Purpose:
 *      Create an MPLS EXP map instance.
 * Parameters:
 *      unit        - (IN)  SOC unit #
 *      flags       - (IN)  MPLS flags
 *      exp_map_id  - (OUT) Allocated EXP map ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_exp_map_create(int unit, uint32 flags, int *exp_map_id)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
              rv = bcm_tr_mpls_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr_mpls_exp_map_create(unit, flags, exp_map_id);
              bcm_tr_mpls_unlock (unit);
         }
         return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_exp_map_create(unit, flags, exp_map_id);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_mpls_exp_map_destroy
 * Purpose:
 *      Destroy an existing MPLS EXP map instance.
 * Parameters:
 *      unit       - (IN) SOC unit #
 *      exp_map_id - (IN) EXP map ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_exp_map_destroy(int unit, int exp_map_id)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
              rv = bcm_tr_mpls_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr_mpls_exp_map_destroy(unit, exp_map_id);
              bcm_tr_mpls_unlock (unit);
         }
         return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_exp_map_destroy(unit, exp_map_id);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_mpls_exp_map_set
 * Purpose:
 *      Set the mapping of { internal priority, color }
 *      to a EXP value for MPLS headers
 *      in the specified EXP map instance.
 * Parameters:
 *      unit         - (IN) SOC unit #
 *      exp_map_id   - (IN) EXP map ID
 *      priority     - (IN) Internal priority
 *      color        - (IN) bcmColor*
 *      exp          - (IN) EXP value
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_exp_map_set(int unit, int exp_map_id,
                         bcm_mpls_exp_map_t *exp_map)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
              rv = bcm_tr_mpls_lock(unit);
       if ( rv == BCM_E_NONE ) {
             rv = bcm_tr_mpls_exp_map_set(unit, exp_map_id, exp_map);
             bcm_tr_mpls_unlock (unit);
       }
       return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_exp_map_set(unit, exp_map_id, exp_map);
        return rv;
    }
#endif
#endif
    return rv;
}
    
/*
 * Function:
 *      bcm_esw_mpls_exp_map_get
 * Purpose:
 *      Get the mapping of { internal priority, color }
 *      to a EXP value for MPLS headers
 *      in the specified EXP map instance.
 * Parameters:
 *      unit         - (IN)  SOC unit #
 *      exp_map_id   - (IN)  EXP map ID
 *      priority     - (IN)  Internal priority
 *      color        - (IN)  bcmColor*
 *      exp          - (OUT) EXP value
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_exp_map_get(int unit, int exp_map_id,
                         bcm_mpls_exp_map_t *exp_map)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls)) {
              rv = bcm_tr_mpls_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr_mpls_exp_map_get(unit, exp_map_id, exp_map);
              bcm_tr_mpls_unlock (unit);
         }
         return rv;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = bcm_er_mpls_exp_map_get(unit, exp_map_id, exp_map);
        return rv;
    }
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_mpls_label_stat_enable_set
 * Purpose:
 *      Enable statistics collection for MPLS label or MPLS gport
 * Parameters:
 *      unit - (IN) Unit number.
 *      label - (IN) MPLS label
 *      port - (IN) MPLS gport
 *      enable - (IN) Non-zero to enable counter collection, zero to disable.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_mpls_label_stat_enable_set(int unit, bcm_mpls_label_t label, 
                                   bcm_gport_t port, int enable)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_gport_service_counters) &&
        soc_feature(unit, soc_feature_mpls)) {
        rv = bcm_tr_mpls_lock(unit);
        if (BCM_E_NONE == rv) {
            rv = bcm_tr2_mpls_label_stat_enable_set(unit, label,
                                                    port, enable);
            bcm_tr_mpls_unlock(unit);
        }
    } 
#ifdef BCM_KATANA_SUPPORT
      else if (soc_feature(unit,soc_feature_advanced_flex_counter) &&
               soc_feature(unit, soc_feature_mpls)) {
               rv = bcm_tr_mpls_lock(unit);
               if (BCM_E_NONE == rv) {
                   rv = bcm_kt_mpls_label_stat_enable_set(unit, label,
                                                          port, enable);
                   bcm_tr_mpls_unlock(unit);
               }
    }
#endif
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_mpls_label_stat_get
 * Purpose:
 *      Get L2 MPLS PW Stats
 * Parameters:
 *      unit   - (IN) SOC unit #
 *      label  - (IN) MPLS label
 *      port   - (IN) MPLS gport
 *      stat   - (IN)  specify the Stat type
 *      val    - (OUT) 64-bit Stats value
 * Returns:
 *      BCM_E_XXX
 */     

int
bcm_esw_mpls_label_stat_get(int unit, bcm_mpls_label_t label, bcm_gport_t port,
                            bcm_mpls_stat_t stat, uint64 *val)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls) &&
        soc_feature(unit, soc_feature_gport_service_counters)) {
              rv = bcm_tr_mpls_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr_mpls_label_stat_get(unit, label, port, stat, val);
              bcm_tr_mpls_unlock (unit);
         }
         return rv;
    }
#ifdef BCM_KATANA_SUPPORT
      else if (soc_feature(unit,soc_feature_advanced_flex_counter) &&
               soc_feature(unit, soc_feature_mpls)) {
               rv = bcm_tr_mpls_lock(unit);
               if (BCM_E_NONE == rv) {
                   rv = bcm_kt_mpls_label_stat_get(unit, label,
                                                   port, stat, val);
                   bcm_tr_mpls_unlock(unit);
               }
    }
#endif
#endif
#endif
    return rv;
}

int
bcm_esw_mpls_label_stat_get32(int unit, bcm_mpls_label_t label, bcm_gport_t port,
                              bcm_mpls_stat_t stat, uint32 *val)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls) &&
        soc_feature(unit, soc_feature_gport_service_counters)) {
              rv = bcm_tr_mpls_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr_mpls_label_stat_get32(unit, label, port, stat, val);
              bcm_tr_mpls_unlock (unit);
         }
         return rv;
    }
#ifdef BCM_KATANA_SUPPORT
      else if (soc_feature(unit,soc_feature_advanced_flex_counter) &&
               soc_feature(unit, soc_feature_mpls)) {
               rv = bcm_tr_mpls_lock(unit);
               if (BCM_E_NONE == rv) {
                   rv = bcm_kt_mpls_label_stat_get32(unit, label,
                                                     port, stat, val);
                   bcm_tr_mpls_unlock(unit);
               }
    }
#endif
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_mpls_label_stat_clear
 * Purpose:
 *      Clear L2 MPLS PW Stats
 * Parameters:
 *      unit   - (IN) SOC unit #
 *      label  - (IN) MPLS label
 *      port   - (IN) MPLS gport
 *      stat   - (IN)  specify the Stat type
 * Returns:
 *      BCM_E_XXX
 */     

int
bcm_esw_mpls_label_stat_clear(int unit, bcm_mpls_label_t label, bcm_gport_t port,
                            bcm_mpls_stat_t stat)
{
int rv = BCM_E_UNAVAIL;
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && soc_feature(unit, soc_feature_mpls) &&
        soc_feature(unit, soc_feature_gport_service_counters)) {
              rv = bcm_tr_mpls_lock(unit);
         if ( rv == BCM_E_NONE ) {
              rv = bcm_tr_mpls_label_stat_clear(unit, label, port, stat);
              bcm_tr_mpls_unlock (unit);
         }
         return rv;
    }
#ifdef BCM_KATANA_SUPPORT
      else if (soc_feature(unit,soc_feature_advanced_flex_counter) &&
               soc_feature(unit, soc_feature_mpls)) {
               rv = bcm_tr_mpls_lock(unit);
               if (BCM_E_NONE == rv) {
                   rv = bcm_kt_mpls_label_stat_clear(unit, label, port, stat);
                   bcm_tr_mpls_unlock(unit);
               }
    }
#endif
#endif
#endif
    return rv;
}

/*
 * Function:
 *      bcm_mpls_info
 * Purpose:
 *      Get the HW MPLS table information such as table size etc
 * Parameters:
 *      unit - Device Number
 *      info - (OUT) table size and used info
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_info(int unit, bcm_mpls_info_t *info)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_info(unit, info);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_port_block_set
 * Purpose:
 *      Stop L2 MPLS packet to go out on certain ports
 * Parameters:
 *      unit   - Device Number
 *      vpn    - the VFI index
 *      port   - Port bitmap
 *      enable - enable or not
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Use pbmp 0 to clear the bitmap
 */

int
bcm_esw_mpls_port_block_set (int unit, bcm_vpn_t vpn,
                             bcm_port_t port, int enable)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    bcm_port_t  local_port;
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &local_port));
    } else {
        local_port = port;
    }
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_port_block_set(unit, vpn, local_port, enable);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_port_block_get
 * Purpose:
 *      Stop L2 MPLS packet to go out on certain ports
 * Parameters:
 *      unit   - Device Number
 *      vpn    - the VFI index
 *      port   - Port bitmap
 *      enable - (OUT) enabled or not
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_port_block_get (int unit, bcm_vpn_t vpn,
                             bcm_port_t port, int *enable)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    bcm_port_t  local_port;
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &local_port));
    } else {
        local_port = port;
    }
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_port_block_get(unit, vpn, local_port, enable);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_create
 * Purpose:
 *      Create a VPN instance (no hardware write)
 * Parameters:
 *      unit   - Device Number
 *      vpn - VPN instance
 *      flags  - VPN property (BCM_MPLS_L2_VPN or BCM_MPLS_L3_VPN)
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      No hardware action
 */

int
bcm_esw_mpls_vpn_create (int unit, bcm_vpn_t vpn, uint32 flags)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_vpn_create(unit, vpn, flags);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_destroy
 * Purpose:
 *      Delete a VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      This automatically unbinds all ports to this VPN
 */

int
bcm_esw_mpls_vpn_destroy (int unit, bcm_vpn_t vpn)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_vpn_destroy(unit, vpn);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_add (bind)
 * Purpose:
 *      Bind interface/port/VLAN to VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 *      vpn_info - VPN association information : interface/port/VLAN
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      This port/VID binding of customer ports to VPN.
 *      For L3 VPNs, the port info is not used (only vlan).
 *      For L2 VPNs, both the port and vlan info is used.
 *      Instance is normally done as a result of admin configuration.
 */

int
bcm_esw_mpls_vpn_add (int unit, bcm_vpn_t vpn, bcm_mpls_vpn_t *vpn_info)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        bcm_mpls_vpn_t  vpn_info_local;

        sal_memcpy(&vpn_info_local, vpn_info, sizeof(bcm_mpls_vpn_t));
        if (BCM_GPORT_IS_SET(vpn_info_local.port)) {
            bcm_trunk_t     tgid;
            int             id;

            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, vpn_info_local.port, 
                                       &vpn_info_local.module, 
                                       &vpn_info_local.port, &tgid, &id));
            if ((tgid != -1) || (id != -1)) {
                return BCM_E_PORT;
            }
        }
        return bcm_er_mpls_vpn_add(unit, vpn, &vpn_info_local);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_delete (unbind)
 * Purpose:
 *      Unbind interface/port/VLAN to VPN instance
 * Parameters:
 *      unit     - Device Number
 *      vpn      - VPN instance ID
 *      vpn_info - interface/port/VLAN
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_vpn_delete (int unit, bcm_vpn_t vpn, bcm_mpls_vpn_t *vpn_info)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_vpn_delete(unit, vpn, vpn_info);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_delete_all
 * Purpose:
 *      Unbind all interface/port/VLAN to the VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_vpn_delete_all (int unit, bcm_vpn_t vpn)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_vpn_delete_all(unit, vpn);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_get
 * Purpose:
 *      Get the binding of interface/port/VLAN to VPN instance
 * Parameters:
 *      unit       - Device Number
 *      vpn     - VPN instance ID
 *      max_out    - max elements of the array
 *      vpn_info   - (OUT) array of interface/port/VLAN struct
 *      real_count - (OUT) actual counts
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      This port/VID binding of customer ports to VPN
 *      instance is normally done as a result of admin configuration.
 */

int
bcm_esw_mpls_vpn_get (int unit, bcm_vpn_t vpn, int max_out,
                      bcm_mpls_vpn_t *vpn_info, int *real_count)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        int isGport;

        BCM_IF_ERROR_RETURN(
            bcm_er_mpls_vpn_get(unit, vpn, max_out, vpn_info, real_count));
        BCM_IF_ERROR_RETURN(
            bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
        if (isGport) {
            if (-1 == vpn_info->port) {
                vpn_info->port = BCM_GPORT_INVALID;
            } else {
                bcm_gport_t     gport;

                BCM_GPORT_MODPORT_SET(gport, vpn_info->module, vpn_info->port);
                vpn_info->port = gport;
            }
        }

        return BCM_E_NONE;
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_circuit_add
 * Purpose:
 *      Add (Bind) a VC to existing VPN
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 *      vc_info  - Virtual circuit info
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Assign L2 MPLS label and L2 outer header info to L2 MPLS pseudo-wire
 *      For Point-to-Multipoint connection, this function is called
 *      multiple times.
 */

int
bcm_esw_mpls_vpn_circuit_add (int unit, bcm_vpn_t vpn,
                        bcm_mpls_circuit_t *vc_info)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        bcm_mpls_circuit_t vc_local;

        sal_memcpy(&vc_local, vc_info, sizeof(bcm_mpls_circuit_t));
        if (BCM_GPORT_IS_SET(vc_local.dst_port)) {
            int id;

            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, vc_local.dst_port, 
                                       &vc_local.dst_mod, 
                                       &vc_local.dst_port, 
                                       &vc_local.dst_trunk, &id));
            if (-1 != id) {
                return BCM_E_PORT;
            }
            if (vc_local.dst_trunk != -1) {
                vc_local.flags |= BCM_MPLS_DST_TRUNK;
            }

        }
        return bcm_er_mpls_vpn_circuit_add(unit, vpn, &vc_local);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_circuit_delete
 * Purpose:
 *      Delete (unbind) a VC from existing VPN
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 *      vc_info  - Virtual circuit info
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Delete L2 MPLS label and L2 outer header info to L2 MPLS pseudo-wire
 *      For Point-to-Multipoint connection, this function is called
 *      multiple times.
 */

int
bcm_esw_mpls_vpn_circuit_delete (int unit, bcm_vpn_t vpn,
                                       bcm_mpls_circuit_t *vc_info)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_vpn_circuit_delete(unit, vpn, vc_info);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_circuit_delete_all
 * Purpose:
 *      Delete (unbind) all VCs from existing VPN
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_vpn_circuit_delete_all (int unit, bcm_vpn_t vpn)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_vpn_circuit_delete_all(unit, vpn);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_circuit_get
 * Purpose:
 *      Get the VC label and header info for the VPN
 * Parameters:
 *      unit       - Device Number
 *      vpn     - The VPN to bind VC to
 *      vc_array    - max elements of the array
 *      vc_info    - (OUT) Virtual circuit info
 *      vc_count - (OUT) actual counts
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      The current ER implementation only supports Point-to-Point
 *      pseudo-wire, so one VC per VPN.
 */

int
bcm_esw_mpls_vpn_circuit_get (int unit, bcm_vpn_t vpn, int vc_array,
                        bcm_mpls_circuit_t *vc_info, int *vc_count)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        int isGport, i;

        BCM_IF_ERROR_RETURN(
            bcm_er_mpls_vpn_circuit_get(unit, vpn, vc_array, vc_info, vc_count));
        BCM_IF_ERROR_RETURN(
            bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
        if (isGport) {
            bcm_gport_t gport;
            for (i = 0; i < *vc_count; i ++) {
                if (vc_info[i].flags & BCM_MPLS_DST_TRUNK) {
                    BCM_GPORT_TRUNK_SET(gport, vc_info[i].dst_trunk);
                    vc_info[i].dst_port = gport;
                } else {
                    BCM_GPORT_MODPORT_SET(gport, vc_info[i].dst_mod, 
                                          vc_info[i].dst_port);
                    vc_info[i].dst_port = gport;
                }
            }
        }
        return BCM_E_NONE;
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_l3_initiator_set
 * Purpose:
 *      Set the FEC to MPLS label mapping
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 *      mpls_label - MPLS label and header information
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_l3_initiator_set (int unit, bcm_l3_intf_t *intf,
                           bcm_mpls_l3_initiator_t *mpls_label)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_l3_initiator_set(unit, intf, mpls_label);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_l3_initiator_clear
 * Purpose:
 *      Clear the FEC to MPLS label mapping
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_l3_initiator_clear (int unit, bcm_l3_intf_t *intf)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_l3_initiator_clear(unit, intf);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_l3_initiator_get
 * Purpose:
 *      Get the FEC to MPLS label mapping info
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 *      mpls_label - MPLS header information
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_l3_initiator_get (int unit, bcm_l3_intf_t *intf,
                           bcm_mpls_l3_initiator_t *mpls_label)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_l3_initiator_get(unit, intf, mpls_label);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_l3_initiator_clear_all
 * Purpose:
 *      Clear all the FEC to MPLS label mapping
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_l3_initiator_clear_all (int unit)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_l3_initiator_clear_all(unit);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_switch_add
 * Purpose:
 *      Add MPLS label swapping, label popping, and packet forwarding
 *      information.
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_switch_add (int unit, bcm_mpls_switch_t *si)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        bcm_mpls_switch_t   sl;
        int                 id;

        sal_memcpy(&sl, si, sizeof(bcm_mpls_switch_t));
        if (BCM_GPORT_IS_SET(sl.dst_port)) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, sl.dst_port, &sl.dst_mod, 
                                       &sl.dst_port, &sl.dst_trunk, &id));
            if (-1 != id) {
                return BCM_E_PORT;
            }
            if (-1 != sl.dst_trunk) {
                sl.flags |= BCM_MPLS_DST_TRUNK;
            }
        }
        if (BCM_GPORT_IS_SET(sl.src_port)) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, sl.src_port, &sl.src_mod, 
                                       &sl.src_port, &sl.src_trunk, &id));
            if (-1 != id) {
                return BCM_E_PORT;
            }
            if (-1 != sl.src_trunk) {
                sl.flags |= BCM_MPLS_SRC_TRUNK;
            }
        }
        if (BCM_GPORT_IS_SET(sl.next_port)) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, sl.next_port, &sl.next_mod, 
                                       &sl.next_port, &sl.next_trunk, &id));
            if (-1 != id) {
                return BCM_E_PORT;
            }
            if (-1 != sl.next_trunk) {
                sl.flags |= BCM_MPLS_NEXT_TRUNK;
            }
        }
        return bcm_er_mpls_switch_add(unit, &sl);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_switch_get
 * Purpose:
 *      Get MPLS label swapping and packet forwarding information
 *   1) label swapping, popping at LSR
 *   2) Ethernet MPLS payload forwarding at the VC termination point
 *   3) L3 MPLS payload (IP packet) forwarding at the LSP termination point
 * Parameters:
 *      unit - Device Number
 *      switch_info - (IN) L3 MPLS label switching and packet forwarding info
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_switch_get (int unit, bcm_mpls_switch_t *si)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        bcm_mpls_switch_t   sl;
        int                 id, isGport;

        sal_memcpy(&sl, si, sizeof(bcm_mpls_switch_t));
        if (BCM_GPORT_IS_SET(sl.src_port)) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, sl.src_port, &sl.src_mod, 
                                       &sl.src_port, &sl.src_trunk, &id));
            if (-1 != id) {
                return BCM_E_PORT;
            }
            if (-1 != sl.src_trunk) {
                sl.flags |= BCM_MPLS_SRC_TRUNK;
            }
        }
        BCM_IF_ERROR_RETURN(bcm_er_mpls_switch_get(unit, &sl));
        BCM_IF_ERROR_RETURN(
            bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
        if (isGport) {
            bcm_gport_t gport;

            if ((sl.flags & BCM_MPLS_DST_TRUNK)) {
                BCM_GPORT_TRUNK_SET(gport, sl.dst_trunk);
                sl.dst_port = gport;
            } else {
                BCM_GPORT_MODPORT_SET(gport, sl.dst_mod, sl.dst_port);
                sl.dst_port = gport;
            }
            if ((sl.flags & BCM_MPLS_NEXT_TRUNK)) {
                BCM_GPORT_TRUNK_SET(gport, sl.next_trunk);
                sl.next_port = gport;
            } else {
                BCM_GPORT_MODPORT_SET(gport, sl.next_mod, sl.next_port);
                sl.next_port = gport;
            }
        }

        return BCM_E_NONE;
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_switch_delete
 * Purpose:
 *      Delete MPLS label swapping and packet forwarding information
 *   1) label swapping, popping at LSR
 *   2) Ethernet MPLS payload forwarding at the VC termination point
 *   3) L3 MPLS payload (IP packet) forwarding at the LSP termination point
 * Parameters:
 *      unit - Device Number
 *      switch_info - (IN) L3 MPLS label switching and packet forwarding info
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_switch_delete (int unit, bcm_mpls_switch_t *switch_info)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        bcm_mpls_switch_t   sl;
        int                 id;

        sal_memcpy(&sl, switch_info, sizeof(bcm_mpls_switch_t));
        if (BCM_GPORT_IS_SET(sl.src_port)) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, sl.src_port, &sl.src_mod, 
                                       &sl.src_port, &sl.src_trunk, &id));
            if (-1 != id) {
                return BCM_E_PORT;
            }
            if (-1 != sl.src_trunk) {
                sl.flags |= BCM_MPLS_SRC_TRUNK;
            }
        }

        return bcm_er_mpls_switch_delete(unit, &sl);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_switch_delete_all
 * Purpose:
 *      Delete all MPLS label swapping and packet forwarding information
 * Parameters:
 *      unit - Device Number
 *      flag - BCM_MPLS_LSR_SWITCH delete LSR action entries
 *             BCM_MPLS_L2_VPN     delete L2 VC termination action entries
 *             BCM_MPLS_L3_VPN     delete L3 LSP termination action entries
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_esw_mpls_switch_delete_all (int unit, int flag)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_switch_delete_all(unit, flag);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_switch_traverse
 * Purpose:
 *      Find MPLS switch entries from the LPM table by traversing the LPM table
 *      and run the function at each MPLS entry
 * Parameters:
 *      unit - SOC unit #
 *      flags - User supplied flags
 *      trav_fn - User callback function, called once per MPLS entry.
 *      start - Start index of interest.
 *      end   - End index of interest.
 *      user_data - cookie
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mpls_switch_traverse(int unit, uint32 flags,
                             uint32 start, uint32 end,
                             bcm_mpls_switch_traverse_cb trav_fn,
                             void *user_data)
{
#ifdef BCM_MPLS_SUPPORT
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return bcm_er_mpls_switch_traverse(unit, flags, start, end,
                                           trav_fn, user_data);
    }
#endif
#endif
    return BCM_E_UNAVAIL;
}
/*
 * Function:
 *      bcm_esw_mpls_port_stat_attach
 * Description:
 *      Attach counters entries to the given mpls gport and vpn 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vpn              - (IN) VPN Id
 *      port             - (IN) MPLS GPORT ID
 *      stat_counter_id  - (IN) Stat Counter ID.
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_mpls_port_stat_attach(
            int         unit, 
            bcm_vpn_t   vpn, 
            bcm_gport_t port, 
            uint32      stat_counter_id)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_port_stat_attach(
               unit,vpn,port,stat_counter_id);
    } 
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_mpls_port_stat_detach
 * Description:
 *      Detach counters entries to the given mpls port and vpn 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vpn              - (IN) VPN Id
 *      port             - (IN) MPLS GPORT ID
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_mpls_port_stat_detach(
            int         unit, 
            bcm_vpn_t   vpn, 
            bcm_gport_t port)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_port_stat_detach(unit,vpn,port);
    }
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_mpls_port_stat_counter_get
 * Description:
 *      Get counter statistic values for specific vpn and gport 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vpn              - (IN) VPN Id
 *      port             - (IN) MPLS GPORT ID
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (OUT) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */

bcm_error_t bcm_esw_mpls_port_stat_counter_get(
            int                  unit, 
            bcm_vpn_t            vpn, 
            bcm_gport_t          port, 
            bcm_mpls_stat_t      stat, 
            uint32               num_entries, 
            uint32               *counter_indexes, 
            bcm_stat_value_t     *counter_values)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_port_stat_counter_get(
               unit,vpn,port,stat,num_entries,counter_indexes,counter_values);
    }
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      bcm_esw_mpls_port_stat_counter_set
 * Description:
 *      Set counter statistic values for specific vpn and gport 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vpn              - (IN) VPN Id
 *      port             - (IN) MPLS GPORT ID
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (OUT) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_mpls_port_stat_counter_set(
            int                  unit, 
            bcm_vpn_t            vpn, 
            bcm_gport_t          port, 
            bcm_mpls_stat_t      stat, 
            uint32               num_entries, 
            uint32               *counter_indexes, 
            bcm_stat_value_t     *counter_values)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_port_stat_counter_get(
               unit,vpn,port,stat,num_entries,counter_indexes,counter_values);
    }
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      bcm_esw_mpls_port_stat_id_get
 * Description:
 *      Get stat counter id associated with specific vpn and gport
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vpn              - (IN) VPN Id
 *      port             - (IN) MPLS GPORT ID
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      Stat_counter_id  - (OUT) Stat Counter ID
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_mpls_port_stat_id_get(
            int                  unit,
            bcm_vpn_t            vpn, 
            bcm_gport_t          port, 
            bcm_mpls_stat_t      stat, 
            uint32               *stat_counter_id)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_port_stat_id_get(
               unit,vpn,port,stat,stat_counter_id);
    }
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_mpls_label_stat_attach
 * Description:
 *      Attach counters entries to the given mpls label and gport 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      label            - (IN) MPLS Label  
 *      port             - (IN) MPLS Gport  
 *      stat_counter_id  - (IN) Stat Counter ID.
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_mpls_label_stat_attach(
            int              unit, 
            bcm_mpls_label_t label, 
            bcm_gport_t      port, 
            uint32           stat_counter_id)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_label_stat_attach(
               unit,label,port,stat_counter_id);
    } 
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      bcm_esw_mpls_label_stat_detach
 * Description:
 *      Detach counters entries to the given mpls label and gport 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      label            - (IN) MPLS Label  
 *      port             - (IN) MPLS Gport  
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_mpls_label_stat_detach(
            int              unit, 
            bcm_mpls_label_t label, 
            bcm_gport_t      port)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_label_stat_detach(
               unit,label,port);
    } 
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      bcm_esw_mpls_label_stat_counter_get
 * Description:
 *      Get counter statistic values for specific MPLS label and gport 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      label            - (IN) MPLS Label  
 *      port             - (IN) MPLS Gport  
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (OUT) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_mpls_label_stat_counter_get(
            int                  unit, 
            bcm_mpls_label_t     label, 
            bcm_gport_t          port, 
            bcm_mpls_stat_t      stat, 
            uint32               num_entries, 
            uint32               *counter_indexes, 
            bcm_stat_value_t     *counter_values)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_label_stat_counter_get(
               unit,label,port,stat,num_entries,counter_indexes,counter_values);
    }
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      bcm_esw_mpls_label_stat_counter_set
 * Description:
 *      Set counter statistic values for specific MPLS label and gport 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      label            - (IN) MPLS Label  
 *      port             - (IN) MPLS Gport  
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (IN) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_mpls_label_stat_counter_set(
            int                  unit, 
            bcm_mpls_label_t     label, 
            bcm_gport_t          port, 
            bcm_mpls_stat_t      stat, 
            uint32               num_entries, 
            uint32               *counter_indexes, 
            bcm_stat_value_t     *counter_values)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_label_stat_counter_set(
               unit,label,port,stat,num_entries,counter_indexes,counter_values);
    }
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      bcm_esw_mpls_label_stat_id_get
 * Description:
 *      Get stat counter id associated with specific MPLS label and gport
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      label            - (IN) MPLS Label  
 *      port             - (IN) MPLS Gport  
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      Stat_counter_id  - (OUT) Stat Counter ID
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_mpls_label_stat_id_get(
            int                  unit,
            bcm_mpls_label_t     label, 
            bcm_gport_t          port, 
            bcm_mpls_stat_t      stat, 
            uint32               *stat_counter_id)
{
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        return bcm_kt_mpls_label_stat_id_get(
               unit,label,port,stat,stat_counter_id);
    }
    return BCM_E_UNAVAIL;
#else
    return BCM_E_UNAVAIL;
#endif
}

#else   /* INCLUDE_L3 */
int bcm_esw_mpls_not_empty;
#endif  /* INCLUDE_L3 */
