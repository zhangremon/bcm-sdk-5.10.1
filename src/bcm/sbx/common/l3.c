/*
 * $Id: l3.c 1.2 Broadcom SDK $
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
 * File:    l3.c
 * Purpose: Manages L3 interface table, forwarding table, routing table
 */

#include <soc/defs.h>

#ifdef INCLUDE_L3

#include <soc/drv.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/l3.h>

int
bcm_sbx_l3_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_intf_create(int unit,
                       bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_intf_delete(int unit,
                       bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_intf_find(int unit,
                     bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_intf_get(int unit,
                    bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_intf_find_vlan(int unit,
                          bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_intf_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_l3_interface_create(int unit,
                            bcm_mac_t mac_addr,
                            bcm_vlan_t vid,
                            int add_to_arl)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_interface_id_create(int unit,
                               bcm_mac_t mac_addr,
                               bcm_vlan_t vid,
                               int intf_idx,
                               int add_to_arl)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_interface_id_update(int unit,
                               bcm_mac_t mac_addr,
                               bcm_vlan_t vid,
                               int intf_idx,
                               int add_to_arl)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_interface_lookup(int unit,
                            bcm_mac_t mac_addr,
                            bcm_vlan_t vid,
                            int *intf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_interface_destroy(int unit,
                             int intf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_interface_find(int unit,
                          int intf_id,
                          bcm_vlan_t *vid,
                          bcm_mac_t mac_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_interface_find_by_vlan(int unit,
                                  bcm_vlan_t vid,
                                  int *intf_id)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_l3_host_find(int unit,
                     bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip_find(int unit,
                   bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip_find_index(int unit,
                         int index,
                         bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_host_add(int unit,
                    bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip_add(int unit,
                  bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_host_delete(int unit,
                       bcm_l3_host_t *ip_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip_delete(int unit,
                     ip_addr_t ip_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_host_delete_by_network(int unit,
                                  bcm_l3_route_t *net_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip_delete_by_prefix(int unit,
                               ip_addr_t ip_addr,
                               ip_addr_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_host_delete_by_interface(int unit,
                                    bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip_delete_by_interface(int unit,
                                  int intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_host_delete_all(int unit,
                           bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip_update_entry_by_key(int unit,
                                  bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_host_conflict_get(int unit,
                             bcm_l3_key_t *ipkey,
                             bcm_l3_key_t *cf_array,
                             int cf_max,
                             int *cf_count)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_l3_host_age(int unit,
                    uint32 flags,
                    bcm_l3_host_traverse_cb age_cb)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_l3_host_traverse(int unit,
                         uint32 flags,
                         uint32 start,
                         uint32 end,
                         bcm_l3_host_traverse_cb cb,
                         void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_age(int unit,
               uint32 flags,
               bcm_l3_age_cb age_out)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_conflict_get(int unit,
                        bcm_l3_key_t *ipkey,
                        bcm_l3_key_t *cf_array,
                        int cf_max,
                        int *cf_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_host_invalidate_entry(int unit,
                                 bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_host_validate_entry(int unit,
                               bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_invalidate_entry(int unit,
                            bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_validate_entry(int unit,
                          bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_l3_route_add(int unit,
                     bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_add(int unit,
                     bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_route_delete(int unit,
                        bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_delete(int unit,
                        bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_l3_route_get(int unit,
                     bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_get(int unit,
                     bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_route_multipath_get(int unit,
                               bcm_l3_route_t *the_route,
                               bcm_l3_route_t *path_array,
                               int max_path,
                               int *path_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_find_index(int unit,
                            int index,
                            bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_route_delete_by_interface(int unit,
                                     bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_delete_by_interface(int unit,
                                     int intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_route_delete_all(int unit,
                            bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_route_traverse(int unit,
                          uint32 flags,
                          uint32 start,
                          uint32 end,
                          bcm_l3_route_traverse_cb trav_fn,
                          void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_traverse(int unit,
                          bcm_l3_route_traverse_cb trav_fn,
                          uint32 start,
                          uint32 end)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_route_age(int unit,
                     uint32 flags,
                     bcm_l3_route_traverse_cb age_out)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_age(int unit,
                     bcm_l3_route_traverse_cb age_out)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_status(int unit,
                  int *free_l3intf,
                  int *free_l3,
                  int *free_defip,
                  int *free_lpm_blk)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_route_max_ecmp_set(int unit,
                              int max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_max_ecmp_set(int unit,
                              int max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_route_max_ecmp_get(int unit,
                              int *max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_defip_max_ecmp_get(int unit,
                              int *max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_cleanup(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_enable_set(int unit,
                      int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_info(int unit,
                bcm_l3_info_t *l3info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_untagged_update(int unit)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_create(int unit,
                         uint32 flags,
                         bcm_l3_egress_t *egr, 
                         bcm_if_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_destroy(int unit,
                          bcm_if_t intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_get (int unit,
                       bcm_if_t intf,
                       bcm_l3_egress_t *egr) 
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_find(int unit,
                       bcm_l3_egress_t *egr, 
                       bcm_if_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_traverse(int unit, 
                           bcm_l3_egress_traverse_cb trav_fn,
                           void *user_data)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_multipath_create(int unit,
                                   uint32 flags,
                                   int intf_count,
                                   bcm_if_t *intf_array,
                                   bcm_if_t *mpintf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_multipath_destroy(int unit,
                                    bcm_if_t mpintf) 
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_multipath_get(int unit,
                                bcm_if_t mpintf,
                                int intf_size,
                                bcm_if_t *intf_array,
                                int *intf_count)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_multipath_add(int unit,
                                bcm_if_t mpintf,
                                bcm_if_t intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_multipath_delete(int unit,
                                   bcm_if_t mpintf,
                                   bcm_if_t intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_multipath_find(int unit,
                                 int intf_count,
                                 bcm_if_t *intf_array,
                                 bcm_if_t *mpintf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_multipath_traverse(int unit, 
                          bcm_l3_egress_multipath_traverse_cb trav_fn,
                                     void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip6_prefix_map_get(int unit,
                              int map_size, 
                              bcm_ip6_t *ip6_array,
                              int *ip6_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip6_prefix_map_add(int unit,
                              bcm_ip6_t ip6_addr) 
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip6_prefix_map_delete(int unit,
                                 bcm_ip6_t ip6_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l3_ip6_prefix_map_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_stat_get(int unit, bcm_l3_egress_t *egr, 
                                  bcm_l3_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_stat_get32(int unit, bcm_l3_egress_t *egr, 
                                    bcm_l3_stat_t stat, uint32 *val)
{
    return BCM_E_UNAVAIL;
}    

int 
bcm_sbx_l3_egress_stat_set(int unit, bcm_l3_egress_t *egr, 
                                  bcm_l3_stat_t stat, uint64 val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_stat_set32(int unit, bcm_l3_egress_t *egr, 
                                    bcm_l3_stat_t stat, uint32 val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_egress_stat_enable_set(int unit, bcm_l3_host_t *egr, 
                                         int enable)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_host_stat_get(int unit, bcm_l3_host_t *host, 
                                bcm_l3_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_host_stat_get32(int unit, bcm_l3_host_t *host, 
                                  bcm_l3_stat_t stat, uint32 *val)
{
    return BCM_E_UNAVAIL;
}    

int 
bcm_sbx_l3_host_stat_set(int unit, bcm_l3_host_t *host, 
                                bcm_l3_stat_t stat, uint64 val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_host_stat_set32(int unit, bcm_l3_host_t *host, 
                                  bcm_l3_stat_t stat, uint32 val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_host_stat_enable_set(int unit, bcm_l3_host_t *host, 
                                       int enable)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_route_stat_get(int unit, bcm_l3_route_t *route, 
                                 bcm_l3_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_route_stat_get32(int unit, bcm_l3_route_t *route, 
                                   bcm_l3_stat_t stat, uint32 *val)
{
    return BCM_E_UNAVAIL;
}    

int 
bcm_sbx_l3_route_stat_set(int unit, bcm_l3_route_t *route, 
                                 bcm_l3_stat_t stat, uint64 val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_route_stat_set32(int unit, bcm_l3_route_t *route, 
                                   bcm_l3_stat_t stat, uint32 val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l3_route_stat_enable_set(int unit, bcm_l3_route_t *route, 
                                        int enable)
{
    return BCM_E_UNAVAIL;
}

#endif  /* INCLUDE_L3 */
