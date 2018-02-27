/*
 * $Id: ipmc.c 1.3.212.1 Broadcom SDK $
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
 * IP Multicast API
 */

#ifdef INCLUDE_L3

#include <soc/drv.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm/error.h>
#include <bcm/ipmc.h>

int
bcm_sbx_ipmc_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_detach(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_enable(int unit,
                    int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_source_port_check(int unit,
                               int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_source_ip_search(int unit,
                              int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_add(int unit,
                 bcm_ipmc_addr_t *data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_remove(int unit,
                    bcm_ipmc_addr_t *data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_delete(int unit,
                    bcm_ip_t s_ip_addr,
                    bcm_ip_t mc_ip_addr,
		    bcm_vlan_t vid,
                    int keep)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_remove_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_get(int unit,
                 bcm_ip_t s_ip_addr,
                 bcm_ip_t mc_ip_addr,
		 bcm_vlan_t vid,
                 bcm_ipmc_addr_t *data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_get_by_index(int unit,
                          int index,
                          bcm_ipmc_addr_t *data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_find(int unit,
                  bcm_ipmc_addr_t *data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_entry_enable_set(int unit,
			      bcm_ip_t s_ip_addr,
                              bcm_ip_t mc_ip_addr,
			      bcm_vlan_t vid,
                              int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_cos_set(int unit,
                     bcm_ip_t s_ip_addr,
                     bcm_ip_t mc_ip_addr,
		     bcm_vlan_t vid,
                     int cos)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_port_tgid_set(int unit,
                           bcm_ip_t s_ip_addr,
                           bcm_ip_t mc_ip_addr,
			   bcm_vlan_t vid,
                           int ts,
                           int port_tgid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_port_modid_set(int unit,
                            bcm_ip_t s_ip_addr,
                            bcm_ip_t mc_ip_addr,
			    bcm_vlan_t vid,
                            int mod_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_add_l2_ports(int unit,
                          bcm_ip_t s_ip_addr,
                          bcm_ip_t mc_ip_addr,
			  bcm_vlan_t vid,
                          bcm_pbmp_t pbmp,
                          bcm_pbmp_t ut_pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_delete_l2_ports(int unit,
                             bcm_ip_t s_ip_addr,
                             bcm_ip_t mc_ip_addr,
			     bcm_vlan_t vid,
                             bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_add_l3_ports(int unit,
                          bcm_ip_t s_ip_addr,
                          bcm_ip_t mc_ip_addr,
			  bcm_vlan_t vid,
                          bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_delete_l3_port(int unit,
                            bcm_ip_t s_ip_addr,
                            bcm_ip_t mc_ip_addr,
			    bcm_vlan_t vid,
                            bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_egress_port_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_egress_port_set(int unit,
                             bcm_port_t port,
                             const bcm_mac_t mac,
			     int untag,
                             bcm_vlan_t vid,
                             int ttl_thresh)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_egress_port_get(int unit,
                             bcm_port_t port,
                             sal_mac_addr_t mac,
			     int *untag,
                             bcm_vlan_t *vid,
                             int *ttl_thresh)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_ipmc_counters_get(int unit,
                          bcm_port_t port,
			  bcm_ipmc_counters_t *counters)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_bitmap_max_get(int unit,
                            int *max_index)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_bitmap_set(int unit,
                        int index,
			bcm_port_t in_port,
                        bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_bitmap_get(int unit,
                        int index,
			bcm_port_t in_port,
                        bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_bitmap_del(int unit,
                        int index,
			bcm_port_t in_port,
                        bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_repl_get(int unit,
                      int index,
                      bcm_port_t port,
		      bcm_vlan_vector_t vlan_vec)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_repl_add(int unit,
                      int index,
                      bcm_pbmp_t pbmp,
                      bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_repl_delete(int unit,
                         int index,
                         bcm_pbmp_t pbmp,
                         bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_repl_delete_all(int unit,
                             int index,
                             bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_ipmc_repl_set(int unit,
                      int mc_index,
                      bcm_port_t port,
                      bcm_vlan_vector_t vlan_vec)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_egress_intf_set(int unit,
                             int mc_index,
                             bcm_port_t port,
                             int if_count,
                             bcm_if_t *if_array)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_egress_intf_get(int unit,
                             int mc_index,
                             bcm_port_t port,
                             int if_max,
                             bcm_if_t *if_array,
                             int *if_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_egress_intf_add(int unit,
                             int index,
                             bcm_port_t port, 
                             bcm_l3_intf_t *l3_intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_egress_intf_delete(int unit,
                                int index,
                                bcm_port_t port, 
                                bcm_l3_intf_t *l3_intf)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_ipmc_egress_intf_delete_all(int unit,
                                    int index,
                                    bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_age(int unit,
                 uint32 flags,
                 bcm_ipmc_traverse_cb age_cb, 
                 void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ipmc_traverse(int unit,
                      uint32 flags,
                      bcm_ipmc_traverse_cb cb,
                      void *user_data)
{
    return BCM_E_UNAVAIL;
}

#endif	/* INCLUDE_L3 */
