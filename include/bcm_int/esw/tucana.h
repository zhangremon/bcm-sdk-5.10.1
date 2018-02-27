/*
 * $Id: tucana.h 1.19 Broadcom SDK $
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
 * File:        tucana.h
 * Purpose:     
 */

#ifndef   _BCM_INT_TUCANA_H_
#define   _BCM_INT_TUCANA_H_

#include <bcm/types.h>

extern int bcm_tucana_vlan_init(int, bcm_vlan_data_t *);
extern int bcm_tucana_vlan_reload(int, vbmp_t *, int *);
extern int bcm_tucana_vlan_create(int, bcm_vlan_t);
extern int bcm_tucana_vlan_port_add(int, bcm_vlan_t, bcm_pbmp_t, bcm_pbmp_t,
                                    bcm_pbmp_t);
extern int bcm_tucana_vlan_port_remove(int, bcm_vlan_t, bcm_pbmp_t);
extern int bcm_tucana_vlan_port_get(int, bcm_vlan_t, bcm_pbmp_t *,
                                    bcm_pbmp_t *, bcm_pbmp_t *);
extern int bcm_tucana_vlan_list(int, bcm_vlan_data_t **, int *);

extern int bcm_tucana_cosq_init(int unit);
extern int bcm_tucana_cosq_detach(int unit, int software_state_only);
extern int bcm_tucana_cosq_config_set(int unit, int numq);
extern int bcm_tucana_cosq_config_get(int unit, int *numq);
extern int bcm_tucana_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
				   int mode, const int weights[],
				   int delay);
extern int bcm_tucana_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
				   int *mode, int weights[],
				   int *delay);
extern int bcm_tucana_cosq_sched_weight_max_get(int unit, int mode,
					 int *weight_max);
extern int bcm_tucana_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                                              bcm_cos_queue_t cosq,
                                              uint32 kbits_sec_min,
                                              uint32 kbits_sec_max,
                                              uint32 kbits_sec_burst,
                                              uint32 flags);
extern int bcm_tucana_cosq_port_bandwidth_get(int unit, bcm_port_t port,
                                              bcm_cos_queue_t cosq,
                                              uint32 *kbits_sec_min,
                                              uint32 *kbits_sec_max,
                                              uint32 *kbits_sec_burst,
                                              uint32 *flags);
extern int bcm_tucana_cosq_discard_set(int unit, uint32 flags);
extern int bcm_tucana_cosq_discard_get(int unit, uint32 *flags);
extern int bcm_tucana_cosq_discard_port_set(int unit, bcm_port_t port,
                                     bcm_cos_queue_t cosq,
                                     uint32 color,
                                     int drop_start,
                                     int drop_slope,
                                     int average_time);
extern int bcm_tucana_cosq_discard_port_get(int unit, bcm_port_t port,
                                     bcm_cos_queue_t cosq,
                                     uint32 color,
                                     int *drop_start,
                                     int *drop_slope,
                                     int *average_time);
#ifdef BCM_WARM_BOOT_SUPPORT
extern int bcm_tucana_cosq_sync(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
extern void bcm_tucana_cosq_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#define BCM_TUCANA_COSQ_PORT_BW_MIN_DFLT        0x7ff

#ifdef INCLUDE_L3

extern int bcm_tucana_ipmc_repl_init(int unit);
extern int bcm_tucana_ipmc_repl_reload(int unit);
extern int bcm_tucana_ipmc_repl_detach(int unit);
extern int bcm_tucana_ipmc_repl_get(int unit, int index, bcm_port_t port, 
			     bcm_vlan_vector_t vlan_vec);
extern int bcm_tucana_ipmc_repl_add(int unit, int index, bcm_port_t port, 
			     bcm_vlan_t vlan);
extern int bcm_tucana_ipmc_repl_delete(int unit, int index, bcm_port_t port, 
				bcm_vlan_t vlan);
extern int bcm_tucana_ipmc_repl_delete_all(int unit, int index, 
				    bcm_port_t port);
extern int bcm_tucana_ipmc_egress_intf_add(int unit, int index, bcm_port_t port, 
			            bcm_l3_intf_t *l3_intf);
extern int bcm_tucana_ipmc_egress_intf_delete(int unit, int index, bcm_port_t port, 
			               bcm_l3_intf_t *l3_intf);
#ifdef BCM_TUCANA_SUPPORT
/* For MSTP mask usage */
extern int _bcm_tucana_ipmc_repl_vlan_stp_set(int unit, bcm_port_t port,
				       bcm_vlan_t vlan, int enable);
#else 
#define _bcm_tucana_ipmc_repl_vlan_stp_set(unit, port, vlan, enable) \
        (BCM_E_NONE)
#endif	/* BCM_TUCANA_SUPPORT */
#endif	/* INCLUDE_L3 */

extern int bcm_tucana_port_rate_ingress_set(int unit, bcm_port_t port,
				     uint32 kbits_sec, 
				     uint32 kbits_burst);
extern int bcm_tucana_port_rate_ingress_get(int unit, bcm_port_t port,
				     uint32 *kbits_sec, 
				     uint32 *kbits_burst);
extern int bcm_tucana_port_rate_pause_set(int unit, bcm_port_t port,
				   uint32 kbits_pause, 
				   uint32 kbits_resume);
extern int bcm_tucana_port_rate_pause_get(int unit, bcm_port_t port,
				   uint32 *kbits_pause, 
				   uint32 *kbits_resume);
extern int _bcm_tucana_port_dtag_mode_set(int unit, bcm_port_t port, int mode, 
                                          int dt_mode, int ignore_tag);
extern int _bcm_tucana_port_dtag_mode_get(int unit, bcm_port_t port, int *mode);

#endif	/* !_BCM_INT_TUCANA_H_ */
