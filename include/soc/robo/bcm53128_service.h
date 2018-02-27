/*
 * $Id: bcm53128_service.h 1.2 Broadcom SDK $
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
 
#ifndef _BCM53128_SERVICE_H
#define _BCM53128_SERVICE_H

#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>


int drv_bcm53128_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_bcm53128_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val);

int drv_bcm53128_cfp_init(int unit);
int drv_bcm53128_cfp_action_get(int unit, uint32* action, drv_cfp_entry_t* entry, uint32* act_param);

int drv_bcm53128_cfp_action_set(int unit, uint32 action, drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2);

int drv_bcm53128_cfp_control_get(int unit, uint32 control_type, uint32 param1, uint32 *param2);

int drv_bcm53128_cfp_control_set(int unit, uint32 control_type, uint32 param1, uint32 param2);

int drv_bcm53128_cfp_entry_read(int unit, uint32 index, uint32 ram_type, drv_cfp_entry_t *entry);

int drv_bcm53128_cfp_entry_write(int unit, uint32 index, uint32 ram_type, drv_cfp_entry_t *entry);

int drv_bcm53128_cfp_field_get(int unit, uint32 mem_type, 
    uint32 field_type, drv_cfp_entry_t* entry, uint32* fld_val);

int drv_bcm53128_cfp_field_set(int unit, uint32 mem_type, 
    uint32 field_type, drv_cfp_entry_t* entry, uint32* fld_val);

int drv_bcm53128_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags);

int drv_bcm53128_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry);

int drv_bcm53128_eav_control_set(int unit, uint32 type, uint32 param);
int drv_bcm53128_eav_control_get(int unit, uint32 type, uint32 *param);
int drv_bcm53128_eav_enable_set(int unit, uint32 port, uint32 enable);
int drv_bcm53128_eav_enable_get(int unit, uint32 port, uint32 *enable);
int drv_bcm53128_eav_link_status_set(int unit, uint32 port, uint32 link);
int drv_bcm53128_eav_link_status_get(int unit, uint32 port, uint32 *link);
int drv_bcm53128_eav_egress_timestamp_get(int unit, uint32 port,
    uint32 *timestamp);
int drv_bcm53128_eav_time_sync_set(int unit, uint32 type, uint32 p0, uint32 p1);
int drv_bcm53128_eav_time_sync_get(int unit, uint32 type, uint32 *p0, uint32 *p1);
int drv_bcm53128_eav_queue_control_set(int unit, uint32 port, uint32 type, uint32 param);
int drv_bcm53128_eav_queue_control_get(int unit, uint32 port, uint32 type, uint32 *param);
int drv_bcm53128_eav_time_sync_mac_set(int unit, uint8 * mac, uint16 ethertype);
int drv_bcm53128_eav_time_sync_mac_get(int unit, uint8 * mac, uint16 *ethertype);
int drv_bcm53128_vlan_prop_get(int unit, uint32 prop_type, uint32 * prop_val);
int drv_bcm53128_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val);
int drv_bcm53128_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                            uint32 port_n, uint32 * val);
int drv_bcm53128_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                            soc_pbmp_t bmp, uint32 val);

int drv_bcm53128_port_cross_connect_set(int unit,uint32 port,soc_pbmp_t bmp);
int drv_bcm53128_port_cross_connect_get(int unit,uint32 port,soc_pbmp_t *bmp);


extern drv_if_t drv_bcm53128_services;


#endif
