/*
 * $Id: vlan.c 1.2 Broadcom SDK $
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
 * QE2000 Vlan API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx/trunk.h>

#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/vlan.h>



int
bcm_sirius_vlan_control_vlan_set(int unit,
                                 bcm_vlan_t vlan,
                                 bcm_vlan_control_vlan_t control)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_init
 */
int
bcm_sirius_vlan_init(int unit)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_create
 */
int
bcm_sirius_vlan_create(int unit,
                       bcm_vlan_t vid)
{
    int rv = BCM_E_NONE;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_destroy
 */
int
bcm_sirius_vlan_destroy(int unit,
                        bcm_vlan_t vid)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_destroy_all
 */
int
bcm_sirius_vlan_destroy_all(int unit)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      _bcm_sirius_vlan_port_remove
 */
int
_bcm_sirius_vlan_port_add(int unit,
                          bcm_vlan_t vid,
                          bcm_pbmp_t pbmp,
                          bcm_pbmp_t ubmp)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}


/*
 *   Function
 *      bcm_sirius_vlan_port_add
 */
int
bcm_sirius_vlan_port_add(int unit,
             bcm_vlan_t vid,
             bcm_pbmp_t pbmp,
             bcm_pbmp_t ubmp)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      _bcm_sirius_vlan_port_remove
 */
int
_bcm_sirius_vlan_port_remove(int unit,
                             bcm_vlan_t vid,
                             bcm_pbmp_t pbmp)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_port_remove
 */
int
bcm_sirius_vlan_port_remove(int unit,
                            bcm_vlan_t vid,
                            bcm_pbmp_t pbmp)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_port_get
 *   Purpose
 *      Get the list of member ports for the specified VID.  The pbmp is the
 *      union of tagged and untagged ports in the VID; the ubmp is the
 *      untagged ports only.
 *   Parameters
 *      unit = unit number whose VID is to be examined for ports
 *      vid  = VID to which the ports are to ba added
 *      pbmp = (out) ptr to bitmap of ports for result
 *      ubmp = (out) ptr to bitmap of untagged ports for result
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_sirius_vlan_port_get(int unit,
                         bcm_vlan_t vid,
                         pbmp_t *pbmp,
                         pbmp_t *ubmp)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_list
 *   Purpose
 *      Get a list of the VIDs that exist on this unit.
 *   Parameters
 *      unit   = unit number whose VLAN list is to be built
 *      listp  = (out) pointer to variable for address of list
 *      countp = (out) pointer to variable for number of list elements
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_sirius_vlan_list(int unit,
                     bcm_vlan_data_t **listp,
                     int *countp)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_list_by_pbmp
 *   Purpose
 *      Get a list of the VIDs that exist on this unit, that include any of the
 *      specified ports.
 *   Parameters
 *      unit   = unit number whose VLAN list is to be built
 *      pbmp   = ports to consider
 *      listp  = (out) pointer to variable for address of list
 *      countp = (out) pointer to variable for number of list elements
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_sirius_vlan_list_by_pbmp(int unit,
                             pbmp_t pbmp,
                             bcm_vlan_data_t **listp,
                             int *countp)
{
    int                 rv=BCM_E_UNAVAIL;
    return rv;
}

/*
 *   Function
 *      bcm_sirius_vlan_list_destroy
 *   Purpose
 *      Dispose of a vlan list obtained from _vlan_list or _vlan_list_by_pbmp.
 *   Parameters
 *      unit   = unit number from which this list was taken
 *      listp  = (out) pointer to variable for address of list
 *      countp = (out) number of list elements
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_sirius_vlan_list_destroy(int unit,
                             bcm_vlan_data_t *list,
                             int count)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_default_set
 *
 *      NOTE:
 *      All ports are implicitly members of the default vlan. The user
 *      application may explicitly add some ports as members of the
 *      default vlan. Ports that are explicitly added are tracked by software.
 *
 *      For the current default vlan all implicit port members will be removed.
 *      This vlan will continue to exist.
 *
 *      The new default vlan should have already been created. Any port
 *      membership that are already configured will be tracked as explicit
 *      port members. All other ports will be implicitly added.
 *
 *      default VID can be disabled by setting it to BCM_VLAN_VID_DISABLE.
 */
int
bcm_sirius_vlan_default_set(int unit,
                            bcm_vlan_t vid)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}

/*
 *   Function
 *      bcm_sirius_vlan_default_get
 */
int
bcm_sirius_vlan_default_get(int unit,
                            bcm_vlan_t *vid_ptr)
{
    int rv = BCM_E_UNAVAIL;
    return(rv);
}
