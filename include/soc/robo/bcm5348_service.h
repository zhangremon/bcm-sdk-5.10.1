/*
 * $Id: bcm5348_service.h 1.2 Broadcom SDK $
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
 
#ifndef _BCM5348_SERVICE_H
#define _BCM5348_SERVICE_H

#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>

int drv_bcm5348_mac_set
    (int unit, soc_pbmp_t pbmp, uint32 mac_type, uint8* mac, uint32 bpdu_idx);
int drv_bcm5348_mac_get
    (int unit, uint32 val, uint32 mac_type, soc_pbmp_t *bmp, uint8* mac);
int drv_bcm5348_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp);
int drv_bcm5348_mirror_get(int unit, uint32 * enable, soc_pbmp_t * mport_bmp, 
    soc_pbmp_t * ingress_bmp, soc_pbmp_t * egress_bmp);
int drv_bcm5348_queue_mapping_type_set
    (int unit, soc_pbmp_t bmp, uint32 mapping_type, uint8 state);
int drv_bcm5348_queue_mapping_type_get
    (int unit, uint32 port, uint32 mapping_type, uint8 *state);
int drv_bcm5348_trunk_set
    (int unit, int tid, soc_pbmp_t bmp, uint32 flag, uint32 hash_op);
int drv_bcm5348_trunk_get
    (int unit, int tid, soc_pbmp_t *bmp, uint32 flag, uint32 *hash_op);
int drv_bcm5348_mstp_port_set
    (int unit, uint32 mstp_gid, uint32 port, uint32 port_state);
int drv_bcm5348_mstp_port_get
    (int unit, uint32 mstp_gid, uint32 port, uint32 *port_state);
int drv_bcm5348_mem_read(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_bcm5348_mem_write(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_bcm5348_mem_field_get(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_bcm5348_mem_field_set(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_bcm5348_mem_clear(int unit, uint32 mem);
int drv_bcm5348_mem_search(int unit, uint32 mem, uint32 *key, 
                                                    uint32 *entry, uint32 *entry1, uint32 flags);
int drv_bcm5348_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_bcm5348_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_bcm5348_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_bcm5348_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val);

int drv5348_cfp_init(int unit);
int drv5348_cfp_action_get(int unit, uint32* action, drv_cfp_entry_t* entry, uint32* act_param);

int drv5348_cfp_action_set(int unit, uint32 action, drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2);

int drv5348_cfp_control_get(int unit, uint32 control_type, uint32 param1, uint32 *param2);

int drv5348_cfp_control_set(int unit, uint32 control_type, uint32 param1, uint32 param2);

int drv5348_cfp_entry_read(int unit, uint32 index, uint32 ram_type, drv_cfp_entry_t *entry);

int drv5348_cfp_entry_search(int unit, uint32 flags, uint32 *index, drv_cfp_entry_t *entry);

int drv5348_cfp_entry_write(int unit, uint32 index, uint32 ram_type, drv_cfp_entry_t *entry);

int drv5348_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, drv_cfp_entry_t* entry, uint32* fld_val);

int drv5348_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, drv_cfp_entry_t* entry, uint32* fld_val);

int drv5348_cfp_meter_get(int unit, drv_cfp_entry_t* entry, uint32 *kbits_sec, uint32 *kbits_burst) ;

int drv5348_cfp_meter_set(int unit, drv_cfp_entry_t* entry, uint32 kbits_sec, uint32 kbits_burst);

int drv5348_cfp_qset_get(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 *val);

int drv5348_cfp_qset_set(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 val);

int drv5348_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags);

int drv5348_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry);

int drv5348_cfp_stat_get(int unit, uint32 stat_type, uint32 index, uint32* counter);

int drv5348_cfp_stat_set(int unit, uint32 stat_type, uint32 index, uint32 counter);

int drv5348_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base);

int drv5348_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base);
int drv5348_cfp_meter_rate_transform(int unit, uint32 kbits_sec, 
    uint32 kbits_burst, uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit);

int drv5348_cfp_ranger(int unit, uint32 flags, uint32 min, uint32 max);
int drv5348_cfp_range_set(int unit, uint32 type, uint32 id, uint32 param1, uint32 param2);
int drv5348_cfp_range_get(int unit, uint32 type, uint32 id, uint32 *param1, uint32 *param2);

int drv_bcm5348_mcast_bmp_get(int unit, uint32 *entry, soc_pbmp_t *bmp);
int drv_bcm5348_mcast_bmp_set(int unit, uint32 *entry, soc_pbmp_t bmp, uint32 flag);
int drv_bcm5348_bcm53242_led_funcgrp_select_get(int unit,int port,int *led_group);
int drv_bcm5348_bcm53242_led_funcgrp_select_set(int unit,int port,int led_group);
int drv_bcm5348_bcm53242_led_mode_get(int unit,int port,uint32 *led_mode);
int drv_bcm5348_bcm53242_led_mode_set(int unit,int port,uint32 led_mode);
int drv_bcm5348_igmp_mld_snoop_mode_get(int unit, int type, int *mode);
int drv_bcm5348_igmp_mld_snoop_mode_set(int unit, int type, int mode);

extern drv_if_t drv_bcm5348_services;

#endif
