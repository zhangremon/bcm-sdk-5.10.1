/* 
 * $Id: port.h 1.16 Broadcom SDK $
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
 * File:        port.h
 * Purpose:     PORT internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_FE2000_PORT_H_
#define _BCM_INT_SBX_FE2000_PORT_H_

/*
 * Port Handler
 *
 * The following structure stores the PORT module internal software
 * information on a device.
 *
 * Access to following structure should protected by PORT_LOCK.
 */
typedef struct _port_info_s {
    mac_driver_t        *p_mac;         /* Per port MAC driver */
    uint32_t            egr_remark_table_idx; /* egress remark index */
    bcm_port_congestion_config_t *e2ecc_config; 
                                        /* End-to-end congestion control configuration */ 
} _sbx_port_info_t;

typedef struct _port_handler_s {
    uint32              ete_l2;         /* Raw OHI untagged-ete */
    int                 ete_l2_valid;
    _sbx_port_info_t    port[SOC_MAX_NUM_PORTS];
} _sbx_port_handler_t;

extern _sbx_port_handler_t              *_sbx_port_handler[BCM_LOCAL_UNITS_MAX];

extern int _bcm_fe2000_port_link_get(int unit,
                                     bcm_port_t port, int hw, int *up);

extern int _bcm_fe2000_port_modid_set(int unit, 
                                      bcm_module_t modid);

extern int _bcm_fe2000_port_egr_remark_idx_get(int unit, bcm_port_t port,
                                               uint32_t *idx);

extern int _bcm_fe2000_port_ilib_lp_init(int unit, bcm_module_t modid, 
                                         bcm_port_t port);

extern int _bcm_fe2000_port_vlan_lp_set(int unit, bcm_module_t modid,
                                        bcm_port_t port, bcm_vlan_t vlan);

extern int _bcm_fe2000_port_strip_tag(int unit, bcm_port_t port, int strip);

extern int _bcm_fe2000_port_default_qos_profile_set(int unit, bcm_port_t port,
                                                    bcm_vlan_t vid);
extern int _bcm_fe2000_port_untagged_vlan_touch(int unit, bcm_port_t phyPort,
                                                bcm_vlan_t vid);

extern int bcm_fe2000_port_qosmap_set(int unit, bcm_port_t port, 
                                      int ingrMap, int egrMap,
                                      uint32 ingFlags, uint32 egrFlags);

extern int bcm_fe2000_port_qosmap_get(int unit, bcm_port_t port, 
                                      int *ing_map, int *egr_map,
                                      uint32 *ing_flags, uint32 *egr_flags);

extern int bcm_fe2000_port_vlan_qosmap_set(int unit, bcm_port_t port, 
                                           bcm_vlan_t vid,
                                           int ing_map, int egr_map,
                                           uint32 ing_flags, uint32 egr_flags);

extern int bcm_fe2000_port_vlan_qosmap_get(int unit, bcm_port_t port, 
                                           bcm_vlan_t vid,
                                           int *ing_map, int *egr_map,
                                           uint32 *ing_flags, uint32 *egr_flags);

#endif /* _BCM_INT_SBX_FE2000_PORT_H_ */
