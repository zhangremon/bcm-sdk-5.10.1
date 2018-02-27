/*
 * $Id: lynx.h 1.15 Broadcom SDK $
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
 * File:        lynx.h
 * Purpose:     Function declarations for Lynx bcm functions
 */

#ifndef _BCM_INT_LYNX_H_
#define _BCM_INT_LYNX_H_

#include <bcm/ipmc.h>

#include <bcm_int/esw/mbcm.h>

/* 6-bit field; MSB indicates trunk */
#define BCM_TGID_TRUNK_INDICATOR(unit) (1 << SOC_TRUNK_BIT_POS(unit))
#define BCM_TGID_PORT_TRUNK_MASK(unit) ((1 << SOC_TRUNK_BIT_POS(unit)) - 1)

/****************************************************************
 *
 * Lynx functions
 *
 ****************************************************************/

extern int bcm_lynx_port_cfg_init(int, bcm_port_t, bcm_vlan_data_t *);
extern int bcm_lynx_port_cfg_get(int, bcm_port_t, bcm_port_cfg_t *);
extern int bcm_lynx_port_cfg_set(int, bcm_port_t, bcm_port_cfg_t *);
extern int _bcm_lynx_port_protocol_vlan_add(int unit, bcm_port_t port,
                                            bcm_port_frametype_t frame,
                                            bcm_port_ethertype_t ether, 
                                            bcm_vlan_t vid);
extern int _bcm_lynx_port_protocol_vlan_delete(int unit, bcm_port_t port,
                                               bcm_port_frametype_t frame,
                                               bcm_port_ethertype_t ether);

extern int bcm_lynx_vlan_init(int, bcm_vlan_data_t *);

extern int bcm_lynx_cosq_init(int unit);
extern int bcm_lynx_cosq_config_set(int unit, int numq);
extern int bcm_lynx_cosq_config_get(int unit, int *numq);
extern int bcm_lynx_cosq_port_sched_set(int unit, bcm_pbmp_t, int mode,
				 const int weights[], int delay);
extern int bcm_lynx_cosq_port_sched_get(int unit, bcm_pbmp_t, int *mode,
				 int weights[], int *delay);
extern int bcm_lynx_cosq_discard_set(int unit, uint32 flags);
extern int bcm_lynx_cosq_discard_get(int unit, uint32 *flags);
extern int bcm_lynx_cosq_discard_port_set(int unit, bcm_port_t port,
                                   bcm_cos_queue_t cosq,
                                   uint32 color,
                                   int drop_start,
                                   int drop_slope,
                                   int average_time);
extern int bcm_lynx_cosq_discard_port_get(int unit, bcm_port_t port,
                                   bcm_cos_queue_t cosq,
                                   uint32 color,
                                   int *drop_start,
                                   int *drop_slope,
                                   int *average_time);
#ifdef BCM_WARM_BOOT_SUPPORT
extern int bcm_lynx_cosq_sync(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
extern void bcm_lynx_cosq_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

extern int bcm_lynx_meter_init(int unit);
extern int bcm_lynx_meter_create(int unit, int port, int *mid);
extern int bcm_lynx_meter_delete(int unit, int port, int mid);
extern int bcm_lynx_meter_delete_all(int unit);
extern int bcm_lynx_meter_get(int unit, int port, int mid, 
		       uint32 *kbits_sec, uint32 *kbits_burst);
extern int bcm_lynx_meter_set(int unit, int port, int mid, 
		       uint32 kbits_sec, uint32 kbits_burst);
extern void _bcm_lynx_meter_size_get(int unit, int *size);
extern int bcm_lynx_ffppacketcounter_set(int unit, int port, int mid, 
				  uint64 val);
extern int bcm_lynx_ffppacketcounter_get(int unit, int port, int mid, 
				  uint64 *val);
extern int bcm_lynx_ffpcounter_init(int unit);
extern int bcm_lynx_ffpcounter_create(int unit, int port, int *ffpcounterid);
extern int bcm_lynx_ffpcounter_delete(int unit, int port, int ffpcounterid);
extern int bcm_lynx_ffpcounter_delete_all(int unit);
extern int bcm_lynx_ffpcounter_set(int unit, int port, int ffpcounterid,
			    uint64 val);
extern int bcm_lynx_ffpcounter_get(int unit, int port, int ffpcounterid,
			    uint64 *val);
extern int bcm_lynx_port_rate_egress_set(int unit, bcm_port_t port,
				  uint32 kbits_sec,
				  uint32 kbits_burst);
extern int bcm_lynx_port_rate_egress_get(int unit, bcm_port_t port,
				  uint32 *kbits_sec,
				  uint32 *kbits_burst);
extern int _bcm_lynx_port_dtag_mode_get(int unit, bcm_port_t port, int *mode);
extern int _bcm_lynx_port_dtag_mode_set(int unit, bcm_port_t port, int mode, 
                                        int dt_mode, int ignore_tag);

/* Utility routines for metering and egress rate control */
extern uint8 _bcm_lynx_kbits_to_bucketsize(uint32 kbits_burst);
extern uint32 _bcm_lynx_bucketsize_to_kbits(uint8 reg_val);

#ifdef INCLUDE_L3

extern int bcm_lynx_ipmc_repl_init(int unit);
extern int bcm_lynx_ipmc_repl_reload(int unit);
extern int bcm_lynx_ipmc_repl_detach(int unit);
extern int bcm_lynx_ipmc_repl_get(int unit, int index, bcm_port_t port, 
			   bcm_vlan_vector_t vlan_vec);
extern int bcm_lynx_ipmc_repl_add(int unit, int index, bcm_port_t port, 
			   bcm_vlan_t vlan);
extern int bcm_lynx_ipmc_repl_delete(int unit, int index, bcm_port_t port, 
			      bcm_vlan_t vlan);
extern int bcm_lynx_ipmc_repl_delete_all(int unit, int index, 
				  bcm_port_t port);
extern int bcm_lynx_ipmc_egress_intf_add(int unit, int index, bcm_port_t port, 
			          bcm_l3_intf_t *l3_intf);
extern int bcm_lynx_ipmc_egress_intf_delete(int unit, int index, bcm_port_t port, 
			             bcm_l3_intf_t *l3_intf);
#endif	/* INCLUDE_L3 */

#endif	/* !_BCM_INT_LYNX_H_ */
