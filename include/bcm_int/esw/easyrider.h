/*
 * $Id: easyrider.h 1.87.52.3 Broadcom SDK $
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
 * File:        easyrider.h
 * Purpose:     Function declarations for Easyrider bcm functions
 */

#ifndef _BCM_INT_EASYRIDER_H_
#define _BCM_INT_EASYRIDER_H_

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/field.h>
#include <bcm_int/esw/l2.h>
#include <soc/l2u.h>


/*
 * Routines for internal use
 */

extern int _bcm_er_l2_from_l2x(int unit, bcm_l2_addr_t *l2addr,
                               l2_entry_internal_entry_t *l2x_entry, 
                               bcm_pbmp_t *pbm);
extern int _bcm_er_l2_to_l2x(int unit, l2_entry_internal_entry_t *l2x_entry,
                             bcm_l2_addr_t *l2addr, bcm_pbmp_t pbm);

#ifdef INCLUDE_L3
struct _bcm_l3_nh_info_s;
extern int _bcm_er_nh_index_get(int unit, struct _bcm_l3_nh_info_s *nh_info, int *index);
extern int _bcm_er_nh_info_get(int unit, int index, struct _bcm_l3_nh_info_s  *nh_info);
extern int _bcm_er_nh_index_release(int unit, int index);
#endif /* INCLUDE_L3 */

/****************************************************************
 *
 * Easyrider functions
 *
 ****************************************************************/

extern int bcm_er_l2_init(int unit);
extern int bcm_er_l2_term(int unit);
extern int bcm_er_l2_addr_get(int unit, bcm_mac_t mac_addr, bcm_vlan_t vid,
		              bcm_l2_addr_t *l2addr);
extern int _bcm_er_l2_addr_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                               bcm_l2_addr_t *l2addr, bcm_pbmp_t *pbm);
extern int bcm_er_l2_addr_add(int unit, bcm_l2_addr_t *l2addr);
extern int _bcm_er_l2_addr_add(int unit, bcm_l2_addr_t *l2addr, bcm_pbmp_t pbm);
extern int bcm_er_l2_addr_delete(int, bcm_mac_t, bcm_vlan_t);
extern int bcm_er_l2_addr_delete_by_mac(int, bcm_mac_t, uint32);
extern int bcm_er_l2_addr_delete_by_mac_port(int, bcm_mac_t,
                                      bcm_module_t, bcm_port_t,
                                      uint32);
extern int _bcm_er_l2_addr_replace_by_vlan_dest(int unit, uint32 flags, 
                                                _bcm_l2_replace_t *rep_st);

extern int bcm_er_l2_conflict_get(int unit, bcm_l2_addr_t *addr, 
                           bcm_l2_addr_t *cf_array, int cf_max, int *cf_count);

#ifdef BCM_WARM_BOOT_SUPPORT
extern int _bcm_er_l2_reload_mbi(int unit);
#else
#define _bcm_er_l2_reload_mbi(_u)    (BCM_E_UNAVAIL)
#endif /* BCM_WARM_BOOT_SUPPORT */

extern int bcm_er_mcast_init(int unit);
extern int bcm_er_mcast_port_remove(int unit, bcm_mcast_addr_t *mcaddr);
extern int bcm_er_mcast_port_add(int unit, bcm_mcast_addr_t *mcaddr);
extern int bcm_er_mcast_addr_add_w_l2mcindex(int unit, bcm_mcast_addr_t *mcaddr);
extern int bcm_er_mcast_addr_add(int unit, bcm_mcast_addr_t *mcaddr);
extern int bcm_er_mcast_addr_remove(int unit, sal_mac_addr_t mac, bcm_vlan_t vid);
extern int bcm_er_mcast_addr_remove_w_l2mcindex(int unit,
                                                bcm_mcast_addr_t *mcaddr);
extern int bcm_er_mcast_port_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                                 bcm_mcast_addr_t *mcaddr);

extern int bcm_er_port_rate_egress_set(int unit, bcm_port_t port,
				uint32 kbits_sec, 
				uint32 kbits_burst);
extern int bcm_er_port_rate_egress_get(int unit, bcm_port_t port,
				uint32 *kbits_sec, 
				uint32 *kbits_burst);

extern int bcm_er_port_sample_rate_set(int unit, bcm_port_t port,
                                int ingress_rate, int egress_rate);
extern int bcm_er_port_sample_rate_get(int unit, bcm_port_t port,
                                int *ingress_rate, int *egress_rate);
extern int _bcm_er_port_tpid_set(int unit, bcm_port_t port, uint16 tpid);
extern int _bcm_er_port_tpid_get(int unit, bcm_port_t port, uint16 *tpid);
extern int _bcm_er_port_inner_tpid_set(int unit, bcm_port_t port, uint16 tpid);
extern int _bcm_er_port_inner_tpid_get(int unit, bcm_port_t port, uint16 *tpid);
extern int _bcm_er_port_protocol_vlan_delete(int unit, bcm_port_t port,
                                             bcm_port_frametype_t frame,
                                             bcm_port_ethertype_t ether);
extern int _bcm_er_port_protocol_vlan_add(int unit, bcm_port_t port,
                                          bcm_port_frametype_t frame,
                                          bcm_port_ethertype_t ether, 
                                          bcm_vlan_t vid);
extern int _bcm_er_port_dtag_mode_set(int unit, bcm_port_t port, int mode, 
                                      int dt_mode, int ignore_tag);
extern int _bcm_er_port_dtag_mode_get(int unit, bcm_port_t port, int *mode);

extern int bcm_er_cosq_init(int unit);
extern int bcm_er_cosq_detach(int unit, int software_state_only);
extern int bcm_er_cosq_config_set(int unit, int numq);
extern int bcm_er_cosq_config_get(int unit, int *numq);
extern int bcm_er_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
                               int mode, const int weights[],
                               int delay);
extern int bcm_er_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
                               int *mode, int weights[],
                               int *delay);
extern int bcm_er_cosq_sched_weight_max_get(int unit, int mode,
                                     int *weight_max);
extern int bcm_er_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                                          bcm_cos_queue_t cosq,
                                          uint32 kbits_sec_min,
                                          uint32 kbits_sec_max,
                                          uint32 kbits_sec_burst,
                                          uint32 flags);
extern int bcm_er_cosq_port_bandwidth_get(int unit, bcm_port_t port,
                                          bcm_cos_queue_t cosq,
                                          uint32 *kbits_sec_min,
                                          uint32 *kbits_sec_max,
                                          uint32 *kbits_sec_burst,
                                          uint32 *flags);
extern int bcm_er_cosq_discard_set(int unit, uint32 flags);
extern int bcm_er_cosq_discard_get(int unit, uint32 *flags);
extern int bcm_er_cosq_discard_port_set(int unit, bcm_port_t port,
                                 bcm_cos_queue_t cosq,
                                 uint32 color,
                                 int drop_start,
                                 int drop_slope,
                                 int average_time);
extern int bcm_er_cosq_discard_port_get(int unit, bcm_port_t port,
                                 bcm_cos_queue_t cosq,
                                 uint32 color,
                                 int *drop_start,
                                 int *drop_slope,
                                 int *average_time);
#ifdef BCM_WARM_BOOT_SUPPORT
extern int bcm_er_cosq_sync(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
extern void bcm_er_cosq_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#define BCM_ER_COSQ_PORT_BW_MIN_DFLT        0x7ff

#ifdef INCLUDE_L3
extern int bcm_er_ipmc_repl_init(int unit);
extern int bcm_er_ipmc_repl_detach(int unit);
extern int bcm_er_ipmc_repl_get(int unit, int index, bcm_port_t port,
                         bcm_vlan_vector_t vlan_vec);
extern int bcm_er_ipmc_repl_add(int unit, int index, bcm_port_t port,
                         bcm_vlan_t vlan);
extern int bcm_er_ipmc_repl_delete(int unit, int index, bcm_port_t port,
                            bcm_vlan_t vlan);
extern int bcm_er_ipmc_repl_delete_all(int unit, int index,
                                bcm_port_t port);
extern int bcm_er_ipmc_repl_set(int unit, int mc_index, bcm_port_t port,
                                bcm_vlan_vector_t vlan_vec);
extern int bcm_er_ipmc_egress_intf_add(int unit, int index, bcm_port_t port,
                                bcm_l3_intf_t *l3_intf);
extern int bcm_er_ipmc_egress_intf_delete(int unit, int index, bcm_port_t port,
                                   bcm_l3_intf_t *l3_intf);
extern int bcm_er_ipmc_egress_intf_set(int unit, int mc_index,
                                       bcm_port_t port,
                                       int if_count, bcm_if_t *if_array);
extern int bcm_er_ipmc_egress_intf_get(int unit, int mc_index,
                                       bcm_port_t port,
                                       int if_max, bcm_if_t *if_array,
                                       int *if_count);
extern int _bcm_er_ipmc_egress_intf_list_add(int unit, int ipmc_id,
                                             bcm_port_t port, 
                                             int nh_index);
extern int _bcm_er_ipmc_egress_intf_list_delete(int unit, int ipmc_id,
                                                bcm_port_t port, int if_max,
                                                int nh_index);
extern int bcm_er_ipmc_port_mtu_update(int unit, bcm_port_t port);
extern int _bcm_er_ipmc_min_mtu_get(int unit, pbmp_t pbmp, uint32 *min_mtu);
extern int _bcm_er_ipmc_reload(int unit);

extern int bcm_er_mpls_init(int unit); 
extern int bcm_er_mpls_cleanup(int unit); 
extern int bcm_er_mpls_vpn_id_create(int unit, bcm_mpls_vpn_config_t *info);
extern int bcm_er_mpls_vpn_id_destroy(int unit, bcm_vpn_t vpn);
extern int bcm_er_mpls_vpn_id_destroy_all(int unit);
extern int bcm_er_mpls_port_add(int unit, bcm_vpn_t vpn, bcm_mpls_port_t *mpls_port);
extern int bcm_er_mpls_port_delete(int unit, bcm_vpn_t vpn, bcm_gport_t mpls_port_id);
extern int bcm_er_mpls_port_delete_all(int unit, bcm_vpn_t vpn);
extern int bcm_er_mpls_port_get(int unit, bcm_vpn_t vpn, bcm_mpls_port_t *mpls_port);
extern int bcm_er_mpls_port_get_all(int unit, bcm_vpn_t vpn, int port_max,
                                      bcm_mpls_port_t *port_array, int *port_count);
extern int bcm_er_mpls_tunnel_initiator_set(int unit, bcm_if_t intf, int num_labels,
                                            bcm_mpls_egress_label_t *label_array);
extern int bcm_er_mpls_tunnel_initiator_clear(int unit, bcm_if_t intf);
extern int bcm_er_mpls_tunnel_initiator_get(int unit, bcm_if_t intf, int label_max,
                                            bcm_mpls_egress_label_t *label_array,
                                            int *label_count);
extern int bcm_er_mpls_tunnel_initiator_clear_all(int unit);
extern int bcm_er_mpls_tunnel_switch_add(int unit, bcm_mpls_tunnel_switch_t *info);
extern int bcm_er_mpls_tunnel_switch_delete(int unit, bcm_mpls_tunnel_switch_t *info);
extern int bcm_er_mpls_tunnel_switch_delete_all(int unit);
extern int bcm_er_mpls_tunnel_switch_get(int unit, bcm_mpls_tunnel_switch_t *info);
extern int bcm_er_mpls_tunnel_switch_traverse(int unit,
                                              bcm_mpls_tunnel_switch_traverse_cb cb,
                                              void *user_data);
extern int bcm_er_mpls_port_block_set(int unit, bcm_vpn_t vpn,
                                      bcm_port_t port, int enable);
extern int bcm_er_mpls_port_block_get(int unit, bcm_vpn_t vpn,
                                      bcm_port_t port, int *enable);
extern int bcm_er_mpls_exp_map_create(int unit, uint32 flags, int *exp_map_id);
extern int bcm_er_mpls_exp_map_destroy(int unit, int exp_map_id);
extern int bcm_er_mpls_exp_map_set(int unit, int exp_map_id, 
                                   bcm_mpls_exp_map_t *exp_map);
extern int bcm_er_mpls_exp_map_get(int unit, int exp_map_id, 
                                   bcm_mpls_exp_map_t *exp_map);

/****************************************************************
 * Below are the legacy APIs supported only on BCM5660x devices *
 ****************************************************************/
extern int bcm_er_mpls_info(int unit, bcm_mpls_info_t *info);
extern int bcm_er_mpls_vpn_create(int unit, bcm_vpn_t vpn, uint32 flags);
extern int bcm_er_mpls_vpn_destroy(int unit, bcm_vpn_t vpn);
extern int bcm_er_mpls_vpn_add(int unit, bcm_vpn_t vpn,  
                               bcm_mpls_vpn_t *vpn_info);
extern int bcm_er_mpls_vpn_delete(int unit, bcm_vpn_t vpn,  
                                  bcm_mpls_vpn_t *vpn_info);
extern int bcm_er_mpls_vpn_delete_all(int unit, bcm_vpn_t vpn);
extern int bcm_er_mpls_vpn_get(int unit, bcm_vpn_t vpn, int max_out,
                               bcm_mpls_vpn_t *vpn_info, int *real_count);
extern int bcm_er_mpls_vpn_circuit_add(int unit, bcm_vpn_t vpn,
                                       bcm_mpls_circuit_t *vc_info);
extern int bcm_er_mpls_vpn_circuit_delete(int unit, bcm_vpn_t vpn,
                                          bcm_mpls_circuit_t *vc_info);
extern int bcm_er_mpls_vpn_circuit_delete_all(int unit, bcm_vpn_t vpn);
extern int bcm_er_mpls_vpn_circuit_get(int unit, bcm_vpn_t vpn, int vc_array,
                                       bcm_mpls_circuit_t *vc_info,  
                                       int *vc_count);
extern int bcm_er_mpls_l3_initiator_set(int unit, bcm_l3_intf_t *intf,
                                        bcm_mpls_l3_initiator_t *mpls_label);
extern int bcm_er_mpls_l3_initiator_clear(int unit, bcm_l3_intf_t *intf);
extern int bcm_er_mpls_l3_initiator_get(int unit, bcm_l3_intf_t *intf,
                                        bcm_mpls_l3_initiator_t *mpls_label);
extern int bcm_er_mpls_l3_initiator_clear_all(int unit);
extern int bcm_er_mpls_switch_add(int unit, bcm_mpls_switch_t *si);
extern int bcm_er_mpls_switch_get(int unit, bcm_mpls_switch_t *si);
extern int bcm_er_mpls_switch_delete(int unit, bcm_mpls_switch_t *switch_info);
extern int bcm_er_mpls_switch_delete_all(int unit, int flag);
extern int bcm_er_mpls_switch_traverse(int unit, uint32 flags,
                                       uint32 start, uint32 end,
                                       bcm_mpls_switch_traverse_cb trav_fn,                
                                       void *user_data);

/* The following are not implemented for Easyrider */
extern int bcm_er_lpm_defip_sw_tbl_empty(int unit, int *count);
extern int bcm_er_lpm_defip_hw_tbl_empty(int unit, int *count);
extern int bcm_er_lpm_check_table(int unit);
extern int bcm_er_lpm_delete(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int bcm_er_lpm_insert(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int bcm_er_lpm_lookup(int unit, ip_addr_t iproute,
                      _bcm_defip_cfg_t* lpm_cfg);

#endif /* INCLUDE_L3 */

int bcm_er_vlan_range_profile_init(int unit);
int bcm_er_vlan_translate_add(int unit, int port, bcm_module_t modid, 
                              bcm_trunk_t tgid, bcm_vlan_t old_vid,
                              bcm_vlan_t new_vid, int prio, int xtable);
int bcm_er_vlan_translate_get(int unit, int port, bcm_module_t modid,
                              bcm_trunk_t tgid, bcm_vlan_t old_vid,
                              bcm_vlan_t *new_vid, int *prio, int xtable);
int bcm_er_vlan_translate_delete(int unit, int port, bcm_module_t modid, 
                                 bcm_trunk_t tgid, bcm_vlan_t old_vid,
                                 int xtable);
int bcm_er_vlan_translate_delete_all(int unit, int xtable);
int _bcm_er_vlan_translate_parse(int unit, soc_mem_t mem, uint32* vent,
                             _bcm_vlan_translate_traverse_t *trvs_info);

int bcm_er_vlan_translate_range_add(int unit, bcm_gport_t gport, bcm_vlan_t old_vid_low, 
                                    bcm_vlan_t old_vid_high, bcm_vlan_t new_vid, 
                                    int int_prio, int xtable);
int bcm_er_vlan_translate_range_get(int unit, bcm_gport_t gport, bcm_vlan_t old_vid_low,
                                    bcm_vlan_t old_vid_high, bcm_vlan_t *new_vid,
                                    int *int_prio, int xtable);
int bcm_er_vlan_translate_range_traverse(int unit, 
                                       _bcm_vlan_translate_traverse_t *trvs_st,
                                         int xtable);
int bcm_er_vlan_translate_range_delete(int unit, bcm_gport_t gport, bcm_vlan_t old_vid_low,
                                       bcm_vlan_t old_vid_high, int xtable);
int bcm_er_vlan_translate_range_delete_all(int unit, int xtable);
int _bcm_er_vlan_control_vlan_set(int unit, bcm_vlan_t vid, bcm_vlan_control_vlan_t *control);

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
extern void _bcm_er_l2_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#ifdef BCM_FIELD_SUPPORT
/* Field Processor (ContextAware) routines */
extern int _bcm_field_er_init(int unit, _field_control_t *control);
extern int bcm_er_field_range_check_set(int unit, int range,
                                 uint32 flags,
                                 bcm_l4_port_t min,
                                 bcm_l4_port_t max);
extern int _bcm_field_er_action_chain_add(int unit, _field_entry_t *f_ent, 
                                          uint32 param0);
extern int _bcm_field_er_data_qualifier_ethertype_add(int unit,  int qual_id,
                                 bcm_field_data_ethertype_t *etype);
extern int _bcm_field_er_data_qualifier_ethertype_delete(int unit, int qual_id,
                                 bcm_field_data_ethertype_t *etype);
extern int _bcm_field_er_data_qualifier_ip_protocol_add(int unit,  int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_proto);
extern int _bcm_field_er_data_qualifier_ip_protocol_delete(int unit, int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_proto);
extern int _bcm_field_er_data_qualifier_packet_format_add(int unit,  int qual_id,
                                 bcm_field_data_packet_format_t *pkt_format);
extern int _bcm_field_er_data_qualifier_packet_format_delete(int unit, int qual_id,
                                 bcm_field_data_packet_format_t *pkt_format);
#endif /* BCM_FIELD_SUPPORT */

extern int _bcm_mpls_vlan_enable_set(int unit, bcm_vlan_t vlan, int enable);
extern int _bcm_mpls_vlan_enable_get(int unit, bcm_vlan_t vlan, int *enable);

#endif /* !_BCM_INT_EASYRIDER_H_ */
