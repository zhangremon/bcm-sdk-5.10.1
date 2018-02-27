/*
 * $Id: multicast.c 1.11 Broadcom SDK $
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
 * File:    multicast.c
 * Purpose: Manages multicast functions
 */

#include <bcm/error.h>
#include <bcm/multicast.h>

extern int
_bcm_fe2000_multicast_l2_encap_get(int              unit, 
                                   bcm_multicast_t  group,
                                   bcm_gport_t      port,
                                   bcm_vlan_t       vlan,
                                   bcm_if_t        *encap_id);

extern int
_bcm_fe2000_ipmc_l3_encap_get(int                     unit,
                              int                     ipmc_index,
                              bcm_gport_t             gport,
                              bcm_if_t                intf_id,
                              bcm_if_t               *encap_id);

extern int
_bcm_fe2000_mpls_vpls_encap_get(int              unit, 
                                bcm_multicast_t  group,
                                bcm_gport_t      port,
                                bcm_gport_t      mpls_port_id,
                                bcm_if_t        *encap_id);

extern int _bcm_fe2000_multicast_mim_encap_get(int unit,
                                               bcm_multicast_t group,
                                               bcm_gport_t port,
                                               bcm_gport_t mim_port_id,
                                               bcm_if_t *encap_id);
int
bcm_fe2000_multicast_create(int unit,
                         uint32 flags,
                         bcm_multicast_t *group)
{

    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_multicast_destroy(int unit, bcm_multicast_t group)
{

    return BCM_E_UNAVAIL;
}

int 
bcm_fe2000_multicast_l3_encap_get(int unit, 
                               bcm_multicast_t group, 
                               bcm_gport_t port, 
                               bcm_if_t intf, 
                               bcm_if_t *encap_id)
{
    /*
     * Used for IP Multicast replication
     */
    
    return (_bcm_fe2000_ipmc_l3_encap_get(unit,
                                          group,
                                          port,
                                          intf,
                                          encap_id));
}

int 
bcm_fe2000_multicast_l2_encap_get(int unit, 
                               bcm_multicast_t group, 
                               bcm_gport_t port, 
                               bcm_vlan_t vlan, 
                               bcm_if_t *encap_id)
{
    return (_bcm_fe2000_multicast_l2_encap_get(unit,
                                               group,
                                               port,
                                               vlan,
                                               encap_id));
}

int
bcm_fe2000_multicast_vpls_encap_get(int              unit, 
                                    bcm_multicast_t  group,
                                    bcm_gport_t      port,
                                    bcm_gport_t      mpls_port_id,
                                    bcm_if_t        *encap_id)
{
    return (_bcm_fe2000_mpls_vpls_encap_get(unit, group,
                                            port, mpls_port_id,
                                            encap_id));
}

int
bcm_fe2000_multicast_subport_encap_get(int unit,
                                    bcm_multicast_t group,
                                    bcm_gport_t port,
                                    bcm_gport_t subport,
                                    bcm_if_t *encap_id)
{

    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_multicast_mim_encap_get(int unit,
                                   bcm_multicast_t group,
                                   bcm_gport_t port,
                                   bcm_gport_t mim_port_id,
                                   bcm_if_t *encap_id)
{
    return (_bcm_fe2000_multicast_mim_encap_get(unit,
						group,
						port,
						mim_port_id,
						encap_id));
}

int
bcm_fe2000_multicast_wlan_encap_get(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_gport_t wlan_port_id,
                                bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_multicast_vlan_encap_get(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_gport_t vlan_port_id,
                                bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_multicast_egress_add(int unit, 
                             bcm_multicast_t group, 
                             bcm_gport_t port,
                             bcm_if_t encap_id)
{

    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_multicast_egress_delete(int unit, 
                                bcm_multicast_t group, 
                                bcm_gport_t port,
                                bcm_if_t encap_id)
{

    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_multicast_egress_delete_all(int unit, 
                                bcm_multicast_t group)
{

    return BCM_E_UNAVAIL;
}

int     
bcm_fe2000_multicast_egress_set(int unit,
                             bcm_multicast_t group,
                             int port_count,
                             bcm_gport_t *port_array,
                             bcm_if_t *encap_id_array)
{

    return BCM_E_UNAVAIL;
}   

int     
bcm_fe2000_multicast_egress_get(int unit,
                             bcm_multicast_t group,
                             int port_max,
                             bcm_gport_t *port_array, 
                             bcm_if_t *encap_id_array, 
                             int *port_count)
{


    return BCM_E_UNAVAIL;
}   

int
bcm_fe2000_multicast_repl_set(int unit, int mc_index, bcm_port_t port,
                      bcm_vlan_vector_t vlan_vec)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_multicast_repl_get(int unit, int index, bcm_port_t port,
                      bcm_vlan_vector_t vlan_vec)
{
    return BCM_E_UNAVAIL;
}
