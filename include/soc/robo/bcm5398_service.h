/*
 * $Id: bcm5398_service.h 1.2 Broadcom SDK $
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
 
#ifndef _BCM5398_SERVICE_H
#define _BCM5398_SERVICE_H

#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>

int drv_bcm5398_mem_read(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_bcm5398_mem_write(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_bcm5398_mem_field_get(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_bcm5398_mem_field_set(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_bcm5398_mem_clear(int unit, uint32 mem);
int drv_bcm5398_mem_search(int unit, uint32 mem, uint32 *key, 
                                                    uint32 *entry, uint32 *entry1, uint32 flags);
int drv_bcm5398_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_bcm5398_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags);


int drv_bcm5398_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, uint8* mac, uint32 bpdu_idx);
int drv_bcm5398_mac_get(int unit, uint32 port, uint32 mac_type, soc_pbmp_t *bmp, 
                                                uint8* mac);
int drv_bcm5398_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_bcm5398_mstp_port_set(int unit, uint32 mstp_gid, 
                                        uint32 port, uint32 port_state);
int drv_bcm5398_mstp_port_get(int unit, uint32 mstp_gid, 
                                        uint32 port, uint32 *port_state);
int drv_bcm5398_port_get(int unit, int port, uint32 prop_type, 
                            uint32 *prop_val);
int drv_bcm5398_port_set(int unit, soc_pbmp_t bmp, uint32 prop_type, 
                            uint32 prop_val);
int drv_bcm5398_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask);
int drv_bcm5398_security_get(int unit, uint32 port, uint32 *state, uint32 *mask);
int drv_bcm5398_storm_control_enable_set(int unit, uint32 port, uint8 enable);
int drv_bcm5398_storm_control_enable_get(int unit, uint32 port, uint8 *enable);
int drv_bcm5398_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size);
int drv_bcm5398_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size);


extern drv_if_t drv_bcm5398_services;

#endif
