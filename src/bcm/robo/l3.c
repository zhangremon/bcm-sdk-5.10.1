/*
 * $Id: l3.c 1.28 Broadcom SDK $
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
 * Purpose: Manages l3 interface table, forwarding table, routing table
 *
 * Note : Not for RoboSwitch currently.
 */

#ifdef INCLUDE_L3

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/l3.h>

/*
 * L3 general APIs
 */


/*
 * Function:
 *      bcm_robo_l3_ip6_prefix_map_get
 * Purpose:
 *      Get a list of IPv6 96 bit prefixes which are mapped to ipv4 lookup
 *      space.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      map_size   - (IN) Size of allocated entries in ip6_array.
 *      ip6_array  - (OUT) Array of mapped prefixes.
 *      ip6_count  - (OUT) Number of entries of ip6_array actually filled in.
 *                      This will be a value less than or equal to the value.
 *                      passed in as map_size unless map_size is 0.  If
 *                      map_size is 0 then ip6_array is ignored and
 *                      ip6_count is filled in with the number of entries
 *                      that would have been filled into ip6_array if
 *                      map_size was arbitrarily large.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l3_ip6_prefix_map_get(int unit, int map_size, 
                              bcm_ip6_t *ip6_array, int *ip6_count)
{
    return BCM_E_UNAVAIL;
}
/*
 * Function:
 *      bcm_robo_l3_ip6_prefix_map_add
 * Purpose:
 *      Create an IPv6 prefix map into IPv4 entry. In case Ipv6 traffic
 *      destination or source IP address matches upper 96 bits of
 *      translation entry. traffic will be routed/switched  based on
 *      lower 32 bits of destination/source IP address treated as IPv4 address.
 * Parameters:
 *      unit     - (IN)  bcm device.
 *      ip6_addr - (IN)  New IPv6 translation address.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l3_ip6_prefix_map_add(int unit, bcm_ip6_t ip6_addr) 
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_l3_ip6_prefix_map_delete
 * Purpose:
 *      Destroy an IPv6 prefix map entry.
 * Parameters:
 *      unit     - (IN)  bcm device.
 *      ip6_addr - (IN)  IPv6 translation address.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l3_ip6_prefix_map_delete(int unit, bcm_ip6_t ip6_addr)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_l3_ip6_prefix_map_delete_all
 * Purpose:
 *      Flush all IPv6 prefix maps.
 * Parameters:
 *      unit     - (IN)  bcm device.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l3_ip6_prefix_map_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_robo_l3_egress_create
 * Purpose:
 *      Create an Egress forwarding object.
 * Parameters:
 *      unit    - (IN)  bcm device.
 *      flags   - (IN)  BCM_L3_REPLACE: replace existing.
 *                      BCM_L3_WITH_ID: intf argument is given.
 *      egr     - (IN) Egress forwarding destination.
 *      intf    - (OUT) L3 interface id pointing to Egress object.
 *                      This is an IN argument if either BCM_L3_REPLACE
 *                      or BCM_L3_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l3_egress_create(int unit, uint32 flags, bcm_l3_egress_t *egr, 
                         bcm_if_t *intf)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_l3_egress_destroy
 * Purpose:
 *      Destroy an Egress forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf    - (IN) L3 interface id pointing to Egress object.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l3_egress_destroy(int unit, bcm_if_t intf)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_l3_egress_get
 * Purpose:
 *      Get an Egress forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf    - (IN) L3 interface id pointing to Egress object.
 *      egr     - (OUT) Egress forwarding destination.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l3_egress_get (int unit, bcm_if_t intf, bcm_l3_egress_t *egr) 
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_l3_egress_multipath_create 
 * Purpose:
 *      Create an Egress Multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      flags      - (IN) BCM_L3_REPLACE: replace existing.
 *                        BCM_L3_WITH_ID: intf argument is given.
 *      intf_count - (IN) Number of elements in intf_array.
 *      intf_array - (IN) Array of Egress forwarding objects.
 *      mpintf     - (OUT) L3 interface id pointing to Egress multipath object.
 *                         This is an IN argument if either BCM_L3_REPLACE
 *                          or BCM_L3_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l3_egress_multipath_create(int unit, uint32 flags, int intf_count,
                                   bcm_if_t *intf_array, bcm_if_t *mpintf)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_l3_egress_multipath_destroy
 * Purpose:
 *      Destroy an Egress Multipath forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) L3 interface id pointing to Egress multipath object.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l3_egress_multipath_destroy(int unit, bcm_if_t mpintf) 
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_l3_egress_multipath_get
 * Purpose:
 *      Get an Egress Multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      mpintf     - (IN) L3 interface id pointing to Egress multipath object.
 *      intf_size  - (IN) Size of allocated entries in intf_array.
 *      intf_array - (OUT) Array of Egress forwarding objects.
 *      intf_count - (OUT) Number of entries of intf_count actually filled in.
 *                      This will be a value less than or equal to the value.
 *                      passed in as intf_size unless intf_size is 0.  If
 *                      intf_size is 0 then intf_array is ignored and
 *                      intf_count is filled in with the number of entries
 *                      that would have been filled into intf_array if
 *                      intf_size was arbitrarily large.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l3_egress_multipath_get(int unit, bcm_if_t mpintf, int intf_size,
                                bcm_if_t *intf_array, int *intf_count)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_l3_egress_multipath_add
 * Purpose:
 *      Add an Egress forwarding object to an Egress Multipath 
 *      forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) L3 interface id pointing to Egress multipath object.
 *      intf    - (IN) L3 interface id pointing to Egress forwarding object.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l3_egress_multipath_add(int unit, bcm_if_t mpintf, bcm_if_t intf)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_l3_egress_multipath_delete
 * Purpose:
 *      Delete an Egress forwarding object to an Egress Multipath 
 *      forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) L3 interface id pointing to Egress multipath object
 *      intf    - (IN) L3 interface id pointing to Egress forwarding object
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l3_egress_multipath_delete(int unit, bcm_if_t mpintf, bcm_if_t intf)
{
    return BCM_E_UNAVAIL;
}


int 
bcm_robo_l3_init(int unit)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_cleanup(int unit)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_age(int unit, uint32 flags, bcm_l3_age_cb age_out)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_status(int unit, int *free_l3intf, int *free_l3,
              int *free_defip, int *free_lpm_blk)
{
	return BCM_E_UNAVAIL;
}	
              
int 
bcm_robo_l3_info(int unit, bcm_l3_info_t *l3info)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_untagged_update(int unit)
{
	return BCM_E_UNAVAIL;
}	
	
/*
 * Function:
 *      bcm_robo_l3_ingress_create
 * Purpose:
 *      Create an Ingress Interface object.
 * Parameters:
 *      unit    - (IN)  bcm device.
 *      flags   - (IN)  BCM_L3_INGRESS_REPLACE: replace existing.
 *                          BCM_L3_INGRESS_WITH_ID: intf argument is given.
 *                          BCM_L3_INGRESS_GLOBAL_ROUTE : 
 *                          BCM_L3_INGRESS_DSCP_TRUST : 
 *      ing_intf     - (IN) Ingress Interface information.
 *      intf_id    - (OUT) L3 Ingress interface id pointing to Ingress object.
 *                      This is an IN argument if either BCM_L3_INGRESS_REPLACE
 *                      or BCM_L3_INGRESS_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
*/

int 
bcm_robo_l3_ingress_create(int unit, bcm_l3_ingress_t *ing_intf, bcm_if_t *intf_id)
{
    int rv = BCM_E_UNAVAIL;

    return rv;
}

/*
 * Function:
 *      bcm_robo_l3_ingress_destroy
 * Purpose:
 *      Destroy an Ingress Interface object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf_id    - (IN) L3 Ingress interface id pointing to Ingress object.
 * Returns:
 *      BCM_E_XXX
 */

int 
bcm_robo_l3_ingress_destroy(int unit, bcm_if_t intf_id)
{
    int rv = BCM_E_UNAVAIL;

    return rv;
}

/*
 * Function:
 *      bcm_robo_l3_ingress_find
 * Purpose:
 *      Find an Ingress Interface object.     
 * Parameters:
 *      unit       - (IN) bcm device.
 *      ing_intf        - (IN) Ingress Interface information.
 *      intf_id       - (OUT) L3 Ingress interface id pointing to Ingress object.
 * Returns:
 *      BCM_E_XXX
 */

int 
bcm_robo_l3_ingress_find(int unit, bcm_l3_ingress_t *ing_intf, bcm_if_t *intf_id)
{
    int rv = BCM_E_UNAVAIL;

    return rv;
}

/*
 * Function:
 *      bcm_robo_l3_ingress_get
 * Purpose:
 *      Get an Ingress Interface object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf_id    - (IN) L3 Ingress interface id pointing to Ingress object.
 *      ing_intf  - (OUT) Ingress Interface information.
 * Returns:
 *      BCM_E_XXX
 */

int 
bcm_robo_l3_ingress_get(int unit, bcm_if_t intf_id, bcm_l3_ingress_t *ing_intf)
{
    int rv = BCM_E_UNAVAIL;

    return rv;
}

/*
 * Function:
 *      bcm_robo_l3_ingress_traverse
 * Purpose:
 *      Goes through ijgress interface objects table and runs the user callback
 *      function at each valid ingress objects entry passing back the
 *      information for that object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      trav_fn    - (IN) Callback function. 
 *      user_data  - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l3_ingress_traverse(int unit, 
                           bcm_l3_ingress_traverse_cb trav_fn, void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    return rv;
}

/*
 * L3 interface related APIs
 */
int 
bcm_robo_l3_interface_create(int unit, bcm_mac_t mac_addr,
				   bcm_vlan_t vid, int add_to_arl)
{
	return BCM_E_UNAVAIL;
}	
				   
int 
bcm_robo_l3_interface_id_create(int unit, bcm_mac_t mac_addr,
                           bcm_vlan_t vid, int intf_idx, int add_to_arl)
{
	return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_interface_id_update(int unit, bcm_mac_t mac_addr,
                           bcm_vlan_t vid, int intf_idx, int add_to_arl)
{
	return BCM_E_UNAVAIL;
}	
                           
int 
bcm_robo_l3_interface_lookup(int unit, bcm_mac_t mac_addr,
                                   bcm_vlan_t vid, int *intf_id)
{
	return BCM_E_UNAVAIL;
}	
                                   
int 
bcm_robo_l3_interface_destroy(int unit, int interface)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_interface_find(int unit, int interface, bcm_vlan_t *vid,
				 bcm_mac_t mac_addr)
{
	return BCM_E_UNAVAIL;
}	
				 
int 
bcm_robo_l3_interface_find_by_vlan(int unit, bcm_vlan_t vid,
					 int *interface)
{
	return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_create(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_create_id(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_delete(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_get(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_destroy(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_find(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_find_vlan(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_id_create(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_id_update(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_lookup(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_intf_lookup_vlan(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}					 

/*
 * L3 IP table related APIs
 */
int 
bcm_robo_l3_ip_find(int unit, bcm_l3_ip_t *info)
{
	return BCM_E_UNAVAIL;
}	

int 
bcm_robo_l3_ip_find_index(int unit, int index, bcm_l3_ip_t *info)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_ip_add(int unit, bcm_l3_ip_t *info)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_ip_delete(int unit, bcm_ip_t ip_addr)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_ip_delete_by_prefix(int unit, bcm_ip_t ip_addr,
				      bcm_ip_t mask)
{
	return BCM_E_UNAVAIL;
}	
				      
int 
bcm_robo_l3_ip_delete_by_interface(int unit, int intf)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_ip_delete_all(int unit)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_ip_update_entry_by_key(int unit, bcm_l3_ip_t *info)
{
	return BCM_E_UNAVAIL;
}	

int
bcm_robo_l3_host_conflict_get(int unit, bcm_l3_key_t *ipkey, bcm_l3_key_t *cf_array,
                    int cf_max, int *cf_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_conflict_get(int unit, bcm_l3_key_t *ipkey, bcm_l3_key_t *cf_array,
                    int cf_max, int *cf_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_invalidate_entry(int unit, bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_validate_entry(int unit, bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int bcm_robo_l3_route_add(int unit, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_route_age(int unit, uint32 flags, bcm_l3_route_traverse_cb age_out)
{
    return BCM_E_UNAVAIL;
}

int bcm_robo_l3_route_delete_all(int unit, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}	

int bcm_robo_l3_route_delete_by_interface(int unit, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}	

int bcm_robo_l3_route_delete(int unit, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}	

int bcm_robo_l3_route_find_index(int unit, int index, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}	

int bcm_robo_l3_route_get(int unit, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}	

int 
bcm_robo_l3_route_max_ecmp_get(int unit, int *max)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_route_max_ecmp_set(int unit, int max)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_route_traverse(int unit, uint32 flags,
        uint32 start, uint32 end,
        bcm_l3_route_traverse_cb tran_fn, void *user_data)
{
    return BCM_E_UNAVAIL;
}

	
/*
 * L3 IP routing table related APIs
 */
int 
bcm_robo_l3_defip_add(int unit, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}	

int 
bcm_robo_l3_defip_delete(int unit, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_defip_find_index(int unit, int index, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_defip_get(int unit, bcm_l3_route_t *info)
{
	return BCM_E_UNAVAIL;
}	

int 
bcm_robo_l3_defip_max_ecmp_get(int unit, int *max)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_defip_max_ecmp_set(int unit, int max)
{
    return BCM_E_UNAVAIL;
}
	
int 
bcm_robo_l3_defip_delete_by_interface(int unit, int intf)
{
	return BCM_E_UNAVAIL;
}	
	
int 
bcm_robo_l3_defip_delete_all(int unit)
{
	return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_defip_age(int unit, bcm_l3_route_traverse_cb age_out)
{
	return BCM_E_UNAVAIL;
}	 

int
bcm_robo_l3_host_add(int unit, bcm_l3_ip_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_host_age(int unit, uint32 flags, bcm_l3_host_traverse_cb age_cb)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_host_traverse(int unit, uint32 flags,
                         uint32 start, uint32 end,
                         bcm_l3_host_traverse_cb cb, void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_host_delete(int unit, bcm_l3_ip_t *ip_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_host_delete_all(int unit, bcm_l3_ip_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_host_delete_by_interface(int unit, bcm_l3_ip_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_host_delete_by_network(int unit, bcm_l3_route_t *net_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_host_find(int unit, bcm_l3_ip_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_host_find_index(int unit, int index, bcm_l3_ip_t *info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_host_invalidate_entry(int unit, bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_host_update_entry_by_key(int unit, bcm_l3_ip_t *info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_host_validate_entry(int unit, bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l3_intf_del(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_l3_intf_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_vr_create(int unit, int vri)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_vr_delete(int unit, int vri)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_vr_port_add(int unit, int vri, bcm_pbmp_t ports)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_l3_vr_port_delete(int unit, int vri, bcm_pbmp_t ports)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_l3_enable_set(int unit, int enable)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_l3_route_multipath_get(int unit, bcm_l3_route_t *the_route,
       bcm_l3_route_t *path_array, int max_path, int *path_count)
{
    return BCM_E_UNAVAIL;
}
int 
bcm_robo_l3_egress_find(int unit, bcm_l3_egress_t *egr, 
                                 bcm_if_t *intf)
{
    return BCM_E_UNAVAIL;
}
int 
bcm_robo_l3_egress_traverse(int unit, 
                           bcm_l3_egress_traverse_cb trav_fn, void *user_data)

{
    return BCM_E_UNAVAIL;
}
int 
bcm_robo_l3_egress_multipath_find(int unit, int intf_count, bcm_if_t
                                 *intf_array, bcm_if_t *mpintf)
{
    return BCM_E_UNAVAIL;
}
int 
bcm_robo_l3_egress_multipath_traverse(int unit, 
                          bcm_l3_egress_multipath_traverse_cb trav_fn,
                          void *user_data)
{
    return BCM_E_UNAVAIL;
}
#endif  /* INCLUDE_L3 */
