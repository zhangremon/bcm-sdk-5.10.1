/*
 * $Id: bcm53242_service.h 1.2 Broadcom SDK $
 *
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
 */
 
#ifndef _BCM53242_SERVICE_H
#define _BCM53242_SERVICE_H

#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>

int drv_bcm53242_mac_set
    (int unit, soc_pbmp_t pbmp, uint32 mac_type, uint8* mac, uint32 bpdu_idx);
int drv_bcm53242_mac_get
    (int unit, uint32 val, uint32 mac_type, soc_pbmp_t *bmp, uint8* mac);
int drv_bcm53242_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp);
int drv_bcm53242_mirror_get(int unit, uint32 * enable, soc_pbmp_t * mport_bmp, 
    soc_pbmp_t * ingress_bmp, soc_pbmp_t * egress_bmp);
int drv_bcm53242_queue_port_prio_to_queue_set
    (int unit, uint8 port, uint8 prio, uint8 queue_n);
int drv_bcm53242_queue_port_prio_to_queue_get
    (int unit, uint8 port, uint8 prio, uint8 *queue_n);
int drv_bcm53242_queue_port_dfsv_set
    (int unit, uint8 port, uint8 dscp, uint8 prio, uint8 queue_n);
int drv_bcm53242_queue_port_dfsv_get
    (int unit, uint8 port, uint8 dscp, uint8 *prio, uint8 *queue_n);
int drv_bcm53242_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 mode);
int drv_bcm53242_queue_mode_get(int unit, uint32 port, uint32 *mode);
int drv_bcm53242_queue_count_set(int unit, uint32 port_type, uint8 count);
int drv_bcm53242_queue_count_get(int unit, uint32 port_type, uint8 *count);
int drv_bcm53242_queue_WRR_weight_set(int unit, 
    uint32 port_type, uint8 queue, uint32 weight);
int drv_bcm53242_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint8 queue, uint32 *weight);
int drv_bcm53242_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n);
int drv_bcm53242_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n);
int drv_bcm53242_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio, uint8 prio);
int drv_bcm53242_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, uint8 *prio);
int drv_bcm53242_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio);
int drv_bcm53242_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio);
int drv_bcm53242_queue_tos_set(int unit, uint8 precedence, uint8 queue_n);
int drv_bcm53242_queue_tos_get(int unit, uint8 precedence, uint8 *queue_n);
int drv_bcm53242_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n);
int drv_bcm53242_queue_dfsv_get(int unit, uint8 code_point, uint8 * queue_n);
int drv_bcm53242_queue_mapping_type_set
    (int unit, soc_pbmp_t bmp, uint32 mapping_type, uint8 state);
int drv_bcm53242_queue_mapping_type_get
    (int unit, uint32 port, uint32 mapping_type, uint8 *state);
int drv_bcm53242_queue_rx_reason_set
    (int unit, uint8 reason, uint32 queue);
int drv_bcm53242_queue_rx_reason_get
    (int unit, uint8 reason, uint32 *queue);
int drv_bcm53242_trunk_set
    (int unit, int tid, soc_pbmp_t bmp, uint32 flag, uint32 hash_op);
int drv_bcm53242_trunk_get
    (int unit, int tid, soc_pbmp_t *bmp, uint32 flag, uint32 *hash_op);
int drv_bcm53242_trunk_hash_field_add
    (int unit, uint32 field_type);
int drv_bcm53242_trunk_hash_field_remove
    (int unit, uint32 field_type);
int drv_bcm53242_mstp_port_set
    (int unit, uint32 mstp_gid, uint32 port, uint32 port_state);
int drv_bcm53242_mstp_port_get
    (int unit, uint32 mstp_gid, uint32 port, uint32 *port_state);
int drv_bcm53242_mem_length_get(int unit, uint32 mem, uint32 *data);
int drv_bcm53242_mem_width_get(int unit, uint32 mem, uint32 *data);
int drv_bcm53242_mem_read(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_bcm53242_mem_write(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_bcm53242_mem_field_get(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_bcm53242_mem_field_set(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_bcm53242_mem_clear(int unit, uint32 mem);
int drv_bcm53242_mem_search(int unit, uint32 mem, uint32 *key, 
                                                    uint32 *entry, uint32 *entry_1, uint32 flags);
int drv_bcm53242_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_bcm53242_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_bcm53242_mem_cache_get(int unit, uint32 mem, uint32 * enable);
int drv_bcm53242_mem_cache_set(int unit, uint32 mem, uint32 enable);
int drv_bcm53242_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_bcm53242_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val);

int drv53242_cfp_init(int unit);
int drv53242_cfp_action_get(int unit, uint32* action, drv_cfp_entry_t* entry, uint32* act_param);

int drv53242_cfp_action_set(int unit, uint32 action, drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2);

int drv53242_cfp_control_get(int unit, uint32 control_type, uint32 param1, uint32 *param2);

int drv53242_cfp_control_set(int unit, uint32 control_type, uint32 param1, uint32 param2);

int drv53242_cfp_entry_read(int unit, uint32 index, uint32 ram_type, drv_cfp_entry_t *entry);

int drv53242_cfp_entry_search(int unit, uint32 flags, uint32 *index, drv_cfp_entry_t *entry);

int drv53242_cfp_entry_write(int unit, uint32 index, uint32 ram_type, drv_cfp_entry_t *entry);

int drv53242_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, drv_cfp_entry_t* entry, uint32* fld_val);

int drv53242_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, drv_cfp_entry_t* entry, uint32* fld_val);

int drv53242_cfp_meter_get(int unit, drv_cfp_entry_t* entry, uint32 *kbits_sec, uint32 *kbits_burst)    ;

int drv53242_cfp_meter_set(int unit, drv_cfp_entry_t* entry, uint32 kbits_sec, uint32 kbits_burst);

int drv53242_cfp_qset_get(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 *val);

int drv53242_cfp_qset_set(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 val);

int drv53242_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags);

int drv53242_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry);

int drv53242_cfp_stat_get(int unit, uint32 stat_type, uint32 index, uint32* counter);

int drv53242_cfp_stat_set(int unit, uint32 stat_type, uint32 index, uint32 counter);

int drv53242_cfp_udf_get(int unit, uint32 port, uint32 udf_index, uint32 *offset, 
    uint32 *base);

int drv53242_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base);
int drv53242_cfp_meter_rate_transform(int unit, uint32 kbits_sec, 
    uint32 kbits_burst, uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit);

int drv53242_cfp_ranger(int unit, uint32 flags, uint32 min, uint32 max);
int drv53242_cfp_range_set(int unit, uint32 type, uint32 id, uint32 param1, uint32 param2);
int drv53242_cfp_range_get(int unit, uint32 type, uint32 id, uint32 *param1, uint32 *param2);

int drv_bcm53242_mcast_bmp_get(int unit, uint32 *entry, soc_pbmp_t *bmp);
int drv_bcm53242_mcast_bmp_set(int unit, uint32 *entry, soc_pbmp_t bmp, uint32 flag);

int drv_bcm53242_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask);
int drv_bcm53242_security_get(int unit, uint32 port, uint32 * state, uint32 * mask);
int drv_bcm53242_security_egress_set(int unit, soc_pbmp_t bmp, int enable);
int drv_bcm53242_security_egress_get(int unit, int port, int *enable);

int drv_bcm53242_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type, uint32 value);
int drv_bcm53242_rate_config_get(int unit, uint32 port, uint32 config_type, uint32 * value);
int drv_bcm53242_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, 
    int direction, uint32 kbits_sec_min, uint32 kbits_sec_max, uint32 burst_size);
int drv_bcm53242_rate_get(int unit, uint32 port, uint8 queue_n,
    int direction, uint32 * kbits_sec_min, uint32 * kbits_sec_max, uint32 * burst_size);
int drv_bcm53242_storm_control_enable_set(int unit, uint32 port, uint8 enable);
int drv_bcm53242_storm_control_enable_get(int unit, uint32 port, uint8 * enable);
int drv_bcm53242_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size);
int drv_bcm53242_storm_control_get(int unit, uint32 port, uint32 * type, 
    uint32 * limit, uint32 *burst_size);

int drv_bcm53242_vlan_prop_get(int unit, uint32 prop_type, uint32 * prop_val);
int drv_bcm53242_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val);
int drv_bcm53242_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 * val);
int drv_bcm53242_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val);
int drv_bcm53242_vlan_vt_set(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 prop_val);
int drv_bcm53242_vlan_vt_get(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 *prop_val);
int drv_bcm53242_vlan_vt_add(int unit, uint32 vt_type, uint32 port, uint32 cvid, uint32 sp_vid, 
                uint32 pri, uint32 mode);
int drv_bcm53242_vlan_vt_delete(int unit, uint32 vt_type, uint32 port, uint32 vid);
int drv_bcm53242_vlan_vt_delete_all(int unit, uint32 vt_type);
int drv_bcm53242_igmp_mld_snoop_mode_get(int unit, int type, int *mode);
int drv_bcm53242_igmp_mld_snoop_mode_set(int unit, int type, int mode);

extern drv_if_t drv_bcm53242_services;

#endif
