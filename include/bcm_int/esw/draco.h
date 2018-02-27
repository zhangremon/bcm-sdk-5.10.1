/*
 * $Id: draco.h 1.63.20.1 Broadcom SDK $
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
 * File:        draco.h
 * Purpose:     Function declarations for Draco bcm functions
 */

#ifndef _BCM_INT_DRACO_H_
#define _BCM_INT_DRACO_H_

#include <bcm/ipmc.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l2.h>

/* 6 bit field, msb indicates trunk */
#define BCM_TGID_TRUNK_LO_BITS(unit)       (SOC_TRUNK_BIT_POS(unit))
#define BCM_TGID_TRUNK_INDICATOR(unit)     (1 << SOC_TRUNK_BIT_POS(unit))
#define BCM_TGID_PORT_TRUNK_MASK(unit)     ((1 << SOC_TRUNK_BIT_POS(unit)) - 1)
#define BCM_TGID_PORT_TRUNK_MASK_HI(unit)  (0x3 << SOC_TRUNK_BIT_POS(unit))
                                           /* Draco 1.5 support */

/*
 * Routines for internal use
 */

extern int _bcm_l2_from_l2x(int unit,
		     bcm_l2_addr_t *l2addr, l2x_entry_t *l2x_entry);
extern int _bcm_l2_to_l2x(int unit,
		   l2x_entry_t *l2x_entry, bcm_l2_addr_t *l2addr);

#ifdef INCLUDE_L3

extern int _bcm_l3_from_l3x(int unit,
		     _bcm_l3_cfg_t *l3cfg, l3x_entry_t *l3x_entry);
extern int _bcm_l3_to_l3x(int unit,
		   l3x_entry_t *l3x_entry, _bcm_l3_cfg_t *l3cfg);

#endif /* INCLUDE_L3 */

/****************************************************************
 *
 * Draco functions
 *
 ****************************************************************/

extern int bcm_draco_l2_init(int unit);
extern int bcm_draco_l2_term(int unit);
extern int bcm_draco_l2_addr_get(int unit, bcm_mac_t mac_addr, bcm_vlan_t vid,
			  bcm_l2_addr_t *l2addr);
extern int bcm_draco_l2_addr_add(int unit, bcm_l2_addr_t *l2addr);
extern int bcm_draco_l2_addr_delete(int, bcm_mac_t, bcm_vlan_t);
extern int bcm_draco_l2_addr_delete_by_mac(int, bcm_mac_t, uint32);
extern int bcm_draco_l2_addr_delete_by_mac_port(int, bcm_mac_t,
					 bcm_module_t, bcm_port_t,
					 uint32);
extern int _bcm_draco_l2_addr_replace_by_vlan_dest(int unit, uint32 flags, 
                                                   _bcm_l2_replace_t *rep_st);

extern int bcm_draco_l2_addr_delete_mcast(int unit, uint32 flags);
extern int bcm_draco_l2_conflict_get(int, bcm_l2_addr_t *, bcm_l2_addr_t *,
			      int, int *);

extern int bcm_draco_vlan_init(int, bcm_vlan_data_t *);
extern int bcm_draco_vlan_reload(int, vbmp_t *, int *);
extern int bcm_draco_vlan_create(int, bcm_vlan_t);
extern int bcm_draco_vlan_destroy(int, bcm_vlan_t);
extern int bcm_draco_vlan_destroy_all(int);
extern int bcm_draco_vlan_port_add(int, bcm_vlan_t, bcm_pbmp_t, bcm_pbmp_t,
                                   bcm_pbmp_t);
extern int bcm_draco_vlan_port_remove(int, bcm_vlan_t, bcm_pbmp_t);
extern int bcm_draco_vlan_port_get(int, bcm_vlan_t, bcm_pbmp_t *,
                                   bcm_pbmp_t *, bcm_pbmp_t *);
extern int bcm_draco_vlan_list(int, bcm_vlan_data_t **, int *);
extern int bcm_draco_vlan_stg_get(int, bcm_vlan_t, int *);
extern int bcm_draco_vlan_stg_set(int, bcm_vlan_t, int);
extern int bcm_draco_vlan_stg_update(int, bcm_vlan_t, bcm_pbmp_t, int);
extern int _bcm_d15_vlan_translate_add(int unit, int port, bcm_vlan_t old_vid,
                                       bcm_vlan_t new_vid, int prio);
extern int _bcm_d15_vlan_translate_delete(int unit, int port, 
                                          bcm_vlan_t old_vid);
extern int _bcm_d15_vlan_translate_get(int unit, int port, bcm_vlan_t old_vid,
                                       bcm_vlan_t *new_vid, int *prio);
extern int _bcm_d15_vlan_translate_parse(int unit, soc_mem_t mem, uint32* vent,
                                         _bcm_vlan_translate_traverse_t *trvs_info);

extern int bcm_draco_port_cfg_init(int, bcm_port_t, bcm_vlan_data_t *);
extern int bcm_draco_port_cfg_get(int, bcm_port_t, bcm_port_cfg_t *);
extern int bcm_draco_port_cfg_set(int, bcm_port_t, bcm_port_cfg_t *);
#if defined(BCM_DRACO15_SUPPORT)
extern int _bcm_draco15_port_protocol_vlan_add(int unit, bcm_port_t port,
                                               bcm_port_frametype_t frame,
                                               bcm_port_ethertype_t ether, 
                                               bcm_vlan_t vid);
extern int _bcm_draco15_port_protocol_vlan_delete(int unit, bcm_port_t port,
                                                bcm_port_frametype_t frame,
                                                bcm_port_ethertype_t ether);
extern int _bcm_d15_port_untagged_vlan_data_update(int unit, bcm_port_t port,
                                                   bcm_vlan_t pdvid, 
                                                   bcm_vlan_t ndvid);
extern int  _bcm_draco15_port_dtag_mode_set(int unit, bcm_port_t port, int mode, 
                                            int dt_mode, int ignore_tag);
extern int _bcm_draco15_port_dtag_mode_get(int unit, bcm_port_t port, int *mode);

#endif /* BCM_DRACO15_SUPPORT */
#if defined(BCM_DRACO1_SUPPORT) || defined(BCM_TUCANA_SUPPORT)
extern int _bcm_draco_port_protocol_vlan_add(int unit, bcm_port_t port,
                                             bcm_port_frametype_t frame,
                                             bcm_port_ethertype_t ether,
                                             bcm_vlan_t vid);
extern int _bcm_draco_port_protocol_vlan_delete(int unit, bcm_port_t port,
                                                bcm_port_frametype_t frame,
                                                bcm_port_ethertype_t ether);
#endif /* BCM_DRACO1_SUPPORT || BCM_TUCANA_SUPPORT */
#if (defined(BCM_DRACO15_SUPPORT) || defined(BCM_LYNX_SUPPORT) || \
     defined(BCM_TUCANA_SUPPORT))
extern int _bcm_draco_port_tpid_set(int unit, bcm_port_t port, uint16 tpid);
extern int _bcm_draco_port_tpid_get(int unit, bcm_port_t port, uint16 *tpid);
#endif /* BCM_DRACO15_SUPPORT || BCM_LYNX_SUPPORT || BCM_TUCANA_SUPPORT */

extern int bcm_draco_trunk_set(int, bcm_trunk_t,
			bcm_trunk_add_info_t *, trunk_private_t *);
extern int bcm_draco_trunk_get(int, bcm_trunk_t,
			bcm_trunk_add_info_t *, trunk_private_t *);
extern int bcm_draco_trunk_destroy(int, bcm_trunk_t, trunk_private_t *);
extern int bcm_draco_trunk_mcast_join(int, bcm_trunk_t,
			       bcm_vlan_t, bcm_mac_t,
			       trunk_private_t *);
extern int _bcm_draco_mcast_detach(int unit);

extern int bcm_draco_stg_stp_init(int, bcm_stg_t);
extern int bcm_draco_stg_stp_get(int, bcm_stg_t, bcm_port_t, int *);
extern int bcm_draco_stg_stp_set(int, bcm_stg_t, bcm_port_t, int);

extern int bcm_draco_mcast_addr_add(int, bcm_mcast_addr_t *);
extern int bcm_draco_mcast_addr_remove(int, bcm_mac_t, bcm_vlan_t);
extern int bcm_draco_mcast_port_get(int, bcm_mac_t, bcm_vlan_t, 
			     bcm_mcast_addr_t *);
extern int bcm_draco_mcast_init(int);
extern int bcm_draco_mcast_addr_init(bcm_mcast_addr_t *, bcm_mac_t,
			      bcm_vlan_t);
extern int bcm_draco_mcast_addr_add_w_l2mcindex(int unit, 
					 bcm_mcast_addr_t *mcaddr);
extern int bcm_draco_mcast_addr_remove_w_l2mcindex(int unit, 
					    bcm_mcast_addr_t *mcaddr);
extern int bcm_draco_mcast_port_add(int unit, bcm_mcast_addr_t *mcaddr);
extern int bcm_draco_mcast_port_remove(int unit, bcm_mcast_addr_t *mcaddr);



extern int bcm_draco_cosq_init(int unit);
extern int bcm_draco_cosq_detach(int unit, int software_state_only);
extern int bcm_draco_cosq_config_set(int unit, int numq);
extern int bcm_draco_cosq_config_get(int unit, int *numq);
extern int bcm_draco_cosq_mapping_set(int unit,
				      bcm_port_t port,
				      bcm_cos_t priority,
				      bcm_cos_queue_t cosq);
extern int bcm_draco_cosq_mapping_get(int unit,
				      bcm_port_t port,
				      bcm_cos_t priority,
				      bcm_cos_queue_t *cosq);
extern int bcm_draco_cosq_port_sched_set(int unit, bcm_pbmp_t, int mode,
				  const int weights[], int delay);
extern int bcm_draco_cosq_port_sched_get(int unit, bcm_pbmp_t, int *mode,
				  int weights[], int *delay);
extern int bcm_draco_cosq_sched_weight_max_get(int unit, int mode,
					int *weight_max);
extern int bcm_draco_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                                             bcm_cos_queue_t cosq,
                                             uint32 kbits_sec_min,
                                             uint32 kbits_sec_max,
                                             uint32 kbits_sec_burst,
                                             uint32 flags);
extern int bcm_draco_cosq_port_bandwidth_get(int unit, bcm_port_t port,
                                             bcm_cos_queue_t cosq,
                                             uint32 *kbits_sec_min,
                                             uint32 *kbits_sec_max,
                                             uint32 *kbits_sec_burst,
                                             uint32 *flags);
extern int bcm_draco_cosq_discard_set(int unit, uint32 flags);
extern int bcm_draco_cosq_discard_get(int unit, uint32 *flags);
extern int bcm_draco_cosq_discard_port_set(int unit, bcm_port_t port,
                                    bcm_cos_queue_t cosq,
                                    uint32 color,
                                    int drop_start,
                                    int drop_slope,
                                    int average_time);
extern int bcm_draco_cosq_discard_port_get(int unit, bcm_port_t port,
                                    bcm_cos_queue_t cosq,
                                    uint32 color,
                                    int *drop_start,
                                    int *drop_slope,
                                    int *average_time);
#ifdef BCM_WARM_BOOT_SUPPORT
extern int bcm_draco_cosq_sync(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
extern void bcm_draco_cosq_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */


#ifdef INCLUDE_L3
extern int bcm_xgs_l3_tables_init(int);
extern int bcm_xgs_l3_tables_cleanup(int);
extern int bcm_xgs_l3_enable(int, int);
extern int bcm_xgs_l3_intf_get(int, _bcm_l3_intf_cfg_t *);
extern int bcm_xgs_l3_intf_get_by_vid(int, _bcm_l3_intf_cfg_t *);
extern int bcm_xgs_l3_intf_create(int, _bcm_l3_intf_cfg_t *);
extern int bcm_xgs_l3_intf_id_create(int, _bcm_l3_intf_cfg_t *);
extern int bcm_xgs_l3_intf_lookup(int, _bcm_l3_intf_cfg_t *);
extern int bcm_xgs_l3_intf_del(int, _bcm_l3_intf_cfg_t *);
extern int bcm_xgs_l3_intf_del_all(int);

extern int bcm_xgs_l3_tables_entries(int);

extern int bcm_xgs_l3_get(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_get_by_index(int unit, int index, _bcm_l3_cfg_t *l3cfg);
extern int bcm_xgs_l3_add(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_del(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_del_by_index(int unit, int index);
extern int bcm_xgs_l3_del_prefix(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_del_intf(int, _bcm_l3_cfg_t *, int);
extern int bcm_xgs_l3_del_all(int);
extern int bcm_xgs_l3_replace(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_age(int unit, uint32 flags, bcm_l3_host_traverse_cb
                          age_out, void *);
extern int bcm_xgs_l3_traverse(int unit, uint32 start, uint32 end,
                      bcm_l3_host_traverse_cb cb, void *user_data);

extern int bcm_xgs_l3_ip6_get(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_ip6_add(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_ip6_del(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_ip6_del_prefix(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_ip6_del_intf(int, _bcm_l3_cfg_t *, int);
extern int bcm_xgs_l3_ip6_del_all(int);
extern int bcm_xgs_l3_ip6_replace(int, _bcm_l3_cfg_t *);
extern int bcm_xgs_l3_ip6_age(int unit, uint32 flags, bcm_l3_host_traverse_cb
                              age_out, void *);
extern int bcm_xgs_l3_ip6_traverse(int unit, uint32 start, uint32 end,
                      bcm_l3_host_traverse_cb cb, void *user_data);

extern int bcm_xgs_defip_cfg_get(int, _bcm_defip_cfg_t *);
extern int bcm_xgs_defip_ecmp_get_all(int, _bcm_defip_cfg_t *,
                               bcm_l3_route_t *, int, int *);
extern int bcm_xgs_defip_add(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int bcm_xgs_defip_del(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int bcm_xgs_defip_del_intf(int, _bcm_defip_cfg_t *, int);
extern int bcm_xgs_defip_del_all(int);
extern int bcm_xgs_defip_find_index(int unit, int index, bcm_l3_route_t *info);
extern int bcm_xgs_lpm_age(int unit, bcm_l3_route_traverse_cb age_out, void *);
extern int bcm_xgs_defip_traverse(int unit, uint32 start, uint32 end,
                       bcm_l3_route_traverse_cb trav_fn, void *user_data);

extern int bcm_xgs_ip6_defip_cfg_get(int, _bcm_defip_cfg_t *);
extern int bcm_xgs_ip6_defip_ecmp_get_all(int, _bcm_defip_cfg_t *,
                               bcm_l3_route_t *, int, int *);
extern int bcm_xgs_ip6_defip_add(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int bcm_xgs_ip6_defip_del(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int bcm_xgs_ip6_defip_del_intf(int, _bcm_defip_cfg_t *, int);
extern int bcm_xgs_ip6_defip_del_all(int);
extern int bcm_xgs_ip6_defip_age(int unit, bcm_l3_route_traverse_cb age_fn,
                                 void *);
extern int bcm_xgs_ip6_defip_traverse(int unit, uint32 start, uint32 end,
                       bcm_l3_route_traverse_cb trav_fn, void *user_data);

extern int bcm_xgs_l3_invalidate_entry(int unit, bcm_ip_t ipaddr);
extern int bcm_xgs_l3_validate_entry(int unit, bcm_ip_t ipaddr);

extern int bcm_xgs_l3_conflict_get(int unit, bcm_l3_key_t *ipkey,
			   bcm_l3_key_t *cf_array,
			   int cf_max, int *cf_count);
extern int bcm_xgs_l3_info(int unit, bcm_l3_info_t *l3_info);
extern int _bcm_xgs_max_ecmp_set(int unit, int max);
extern int _bcm_xgs_max_ecmp_get(int unit, int *max);

extern int bcm_xgs_lpm_sw_tbl_empty(int unit, int *count);
extern int bcm_xgs_lpm_hw_tbl_empty(int unit, int *count);
extern int bcm_xgs_lpm_compare_table(int unit);
extern int bcm_xgs_lpm_delete(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int bcm_xgs_lpm_insert(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int bcm_xgs_lpm_lookup(int unit, ip_addr_t iproute, 
                          _bcm_defip_cfg_t* lpm_cfg);

#ifdef BCM_WARM_BOOT_SUPPORT
extern int _bcm_esw_l3_sync(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */

/* IPMC functions */
extern int bcm_xgs_ipmc_init(int unit);
extern int bcm_xgs_ipmc_detach(int unit);
extern int bcm_xgs_ipmc_get(int unit, int index, bcm_ipmc_addr_t *ipmc);
extern int bcm_xgs_ipmc_lookup(int unit, int *index, bcm_ipmc_addr_t *ipmc);
extern int bcm_xgs_ipmc_add(int unit, bcm_ipmc_addr_t *ipmc);
extern int bcm_xgs_ipmc_put(int unit, int index, bcm_ipmc_addr_t *ipmc);
extern int bcm_xgs_ipmc_delete(int unit, bcm_ipmc_addr_t *ipmc);
extern int bcm_xgs_ipmc_delete_all(int unit);
extern int bcm_xgs_ipmc_enable(int unit, int enable);
extern int bcm_xgs_ipmc_src_port_check(int unit, int enable);
extern int bcm_xgs_ipmc_src_ip_search(int unit, int enable);
extern int bcm_xgs_ipmc_egress_port_set(int unit, bcm_port_t port, 
				 const bcm_mac_t mac,  int untag, 
				 bcm_vlan_t vid, int ttl_threshold);
extern int bcm_xgs_ipmc_egress_port_get(int unit, bcm_port_t port, 
				 sal_mac_addr_t mac, int *untag, 
				 bcm_vlan_t *vid, 
				 int *ttl_threshold);
extern int bcm_xgs_ipmc_traverse(int unit, uint32 flags, bcm_ipmc_traverse_cb cb,
                                 void *user_data);
extern int bcm_xgs_ipmc_age(int unit, uint32 flags, bcm_ipmc_traverse_cb age_cb, 
                            void *user_data);

extern int bcm_draco15_ipmc_repl_init(int unit);
extern int bcm_draco15_ipmc_repl_reload(int unit);
extern int bcm_draco15_ipmc_repl_detach(int unit);
extern int bcm_draco15_ipmc_repl_get(int unit, int index, bcm_port_t port, 
			      bcm_vlan_vector_t vlan_vec);
extern int bcm_draco15_ipmc_repl_add(int unit, int index, bcm_port_t port, 
			      bcm_vlan_t vlan);
extern int bcm_draco15_ipmc_repl_delete(int unit, int index, bcm_port_t port, 
				 bcm_vlan_t vlan);
extern int bcm_draco15_ipmc_repl_delete_all(int unit, int index, 
				     bcm_port_t port);
extern int bcm_draco15_ipmc_repl_set(int unit, int mc_index, bcm_port_t port,
                                     bcm_vlan_vector_t vlan_vec);
extern int bcm_draco15_ipmc_egress_intf_add(int unit, int index, bcm_port_t port, 
			             bcm_l3_intf_t *l3_intf);
extern int bcm_draco15_ipmc_egress_intf_delete(int unit, int index, bcm_port_t port, 
		  	                bcm_l3_intf_t *l3_intf);
extern int bcm_draco15_ipmc_egress_intf_set(int unit, int mc_index, bcm_port_t port,
                                    int if_count, bcm_if_t *if_array);
extern int bcm_draco15_ipmc_egress_intf_get(int unit, int mc_index,
                                       bcm_port_t port,
                                       int if_max, bcm_if_t *if_array,
                                       int *if_count);
extern int _bcm_draco_l3_dlf_action_set(int unit, int action);
extern int _bcm_draco_l3_dlf_action_get(int unit, int *action);
#endif	/* INCLUDE_L3 */

extern int bcm_xgs_switch_control_port_set(int unit, bcm_port_t port,
                                    bcm_switch_control_t type, int arg);
extern int bcm_xgs_switch_control_port_get(int unit, bcm_port_t port,
                                    bcm_switch_control_t type, int *arg);

#endif	/* !_BCM_INT_DRACO_H_ */
