/*
 * $Id: mpls.h 1.17.6.2 Broadcom SDK $
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
 * 
 * DO NOT EDIT THIS FILE!
 * This file is auto-generated.
 * Edits to this file will be lost when it is regenerated.
 */

#ifndef __BCMX_MPLS_H__
#define __BCMX_MPLS_H__

#if defined(INCLUDE_L3)

#include <bcm/types.h>
#include <bcmx/bcmx.h>
#include <bcmx/lplist.h>
#include <bcmx/l3.h>
#include <bcm/mpls.h>

/* 
 * Information for mapping incoming packets to VPN instance for L3 MPLS
 * VPN Routing/Forwarding Instance (VRF) or L2 MPLS Virtual Forwarding
 * Instance (VFI).
 */
typedef struct bcmx_mpls_vpn_s {
    uint32 flags;       /* VRF/VFI select, i.e. L3 or L2; Trunk. */
    bcm_vpn_t vpn;      /* VRF/VFI index - remove. */
    bcm_if_t l3_intf;   /* L3 interface for RPF check, L3 VPN only. */
    bcmx_lport_t lport; /* Source Port. */
    bcm_trunk_t trunk;  /* Source trunk Group ID. */
    bcm_vlan_t vlan;    /* Source VLAN ID. */
} bcmx_mpls_vpn_t;

/* 
 * Virtual Circuit Structure
 * 
 * A VPN can have one or more VCs associated with it. For point-to-point
 * MPLS tunneling, there is one VC for the VPN; for VPLS
 * point-to-multipoint VPN, there can be more the one VCs for a VPN.
 * 
 * A Virtual Circuit is marked by MPLS VC label and header properties,
 * i.e. it contains information for encapsulating incoming Ethernet
 * packets (with no MPLS label) with MPLS headers, happens at L2 MPLS
 * tunnel initiation PE..
 */
typedef struct bcmx_mpls_circuit_s {
    uint32 flags;           /* BCM_MPLS_ORIGINAL_PKT? */
    bcm_vpn_t vpn;          /* The VPN this VC belongs to. */
    bcm_vlan_t vlan;        /* New VID in MPLS header. */
    bcm_mpls_label_t label; /* L2 MPLS VC label. */
    bcm_if_t l3_intf;       /* "L3 interface" (source MAC address, labels). */
    bcm_mac_t dst_mac;      /* MPLS packet's egress destination MAC. */
    bcmx_lport_t dst_port;  /* MPLS packet's egress port. */
    bcm_trunk_t dst_trunk;  /* MPLS packet's egress trunk. */
} bcmx_mpls_circuit_t;

/* 
 * MPLS Switch structure
 * 
 * The MPLS switch, i.e. incoming label mapping (ILM), structure contains
 * information for for label swapping and forwarding to next hop; label
 * popping at the penultimate switch, as well as information for
 * terminating L2/L3 MPLS tunnels.
 */
typedef struct bcmx_mpls_switch_s {
    uint32 flags;                   /* BCM_MPLS_ONE_LABEL_LOOKUP etc. etc. */
    bcm_mpls_label_t src_label1;    /* Lookup key: outer label. */
    bcm_mpls_label_t src_label2;    /* Lookup key: inner label if applicable. */
    bcmx_lport_t src_lport;         /* Lookup key: Source port. */
    bcm_trunk_t src_trunk;          /* Lookup key: Source trunk ID. */
    bcm_mpls_action_t action;       /* MPLS action type. */
    bcm_if_t src_l3_intf;           /* Ingress L3 interface for the tunnel. */
    int label_pri;                  /* Priority for the associated label. */
    bcm_mpls_label_t swap_label;    /* New label for label swapping. */
    bcm_mpls_label_t push_label1;   /* New label for label pushing. */
    bcm_mpls_label_t push_label2;   /* New label for label pushing. */
    bcm_if_t next_intf;             /* Egress L3 interface for the tunnel. */
    bcm_mac_t next_mac;             /* Next hop MAC to get NH index. */
    bcm_vlan_t next_vlan;           /* VLAN ID to get NH index. */
    bcmx_lport_t next_lport;        /* Port to get NH index. */
    bcm_trunk_t next_trunk;         /* Trunk to get NH index. */
    bcm_vpn_t vpn;                  /* VRF for L3 forwarding of IP payload; VFI
                                       ID used for VC table lookup. */
    bcmx_lport_t dst_lport;         /* Port L2 payload is sent to. */
    bcm_trunk_t dst_trunk;          /* Trunk L2 payload is sent to. */
    bcm_vlan_t dst_vlan;            /* New VLAN for overriding in L2 payload. */
} bcmx_mpls_switch_t;

/* 
 * MPLS Information Structure
 * 
 * Contains information about MPLS hardware capacity and utilization.
 */
typedef bcm_mpls_info_t bcmx_mpls_info_t;

/* MPLS VPN Config Structure */
typedef bcm_mpls_vpn_config_t bcmx_mpls_vpn_config_t;

/* MPLS Port Structure */
typedef bcm_mpls_port_t bcmx_mpls_port_t;

/* MPLS Egress Label Structure */
typedef bcm_mpls_egress_label_t bcmx_mpls_egress_label_t;

/* MPLS Tunnel Switch Structure */
typedef bcm_mpls_tunnel_switch_t bcmx_mpls_tunnel_switch_t;

/* Initialize the MPLS VPN structure. */
extern int bcmx_mpls_vpn_t_init(
    bcmx_mpls_vpn_t *vpn_info);

/* Initialize the MPLS L3 initiator structure. */
extern int bcmx_mpls_l3_initiator_t_init(
    bcm_mpls_l3_initiator_t *ftn);

/* Initialize the MPLS switch structure. */
extern int bcmx_mpls_switch_t_init(
    bcmx_mpls_switch_t *mswitch);

/* Initialize the MPLS VPN config structure. */
extern void bcmx_mpls_vpn_config_t_init(
    bcmx_mpls_vpn_config_t *info);

/* Initialize the MPLS port structure. */
extern void bcmx_mpls_port_t_init(
    bcmx_mpls_port_t *mpls_port);

/* Initialize the MPLS egress label structure. */
extern void bcmx_mpls_egress_label_t_init(
    bcmx_mpls_egress_label_t *label);

/* Initialize the MPLS tunnel switch structure. */
extern void bcmx_mpls_tunnel_switch_t_init(
    bcmx_mpls_tunnel_switch_t *info);

/* Initialize the MPLS virtual circuit structure. */
extern void bcmx_mpls_circuit_t_init(
    bcmx_mpls_circuit_t *mpls_vc);

/* Initialize the MPLS subsystem. */
extern int bcmx_mpls_init(void);

/* Detach the MPLS software module. */
extern int bcmx_mpls_cleanup(void);

/* Get MPLS info. */
extern int bcmx_mpls_info(void);

extern int bcmx_mpls_vpn_id_create(
    bcmx_mpls_vpn_config_t *info);

extern int bcmx_mpls_vpn_id_destroy(
    bcm_vpn_t vpn);

extern int bcmx_mpls_vpn_id_destroy_all(void);

extern int bcmx_mpls_vpn_id_get(
    bcm_vpn_t vpn, 
    bcmx_mpls_vpn_config_t *info);

/* Add a MPLS logical port to the specified VPN */
extern int bcmx_mpls_port_add(
    bcm_vpn_t vpn, 
    bcmx_mpls_port_t *mpls_port);

/* Delete the given MPLS logical port from the specified VPN */
extern int bcmx_mpls_port_delete(
    bcm_vpn_t vpn, 
    bcm_gport_t mpls_port_id);

extern int bcmx_mpls_port_delete_all(
    bcm_vpn_t vpn);

extern int bcmx_mpls_port_get(
    bcm_vpn_t vpn, 
    bcmx_mpls_port_t *mpls_port);

extern int bcmx_mpls_port_get_all(
    bcm_vpn_t vpn, 
    int port_max, 
    bcmx_mpls_port_t *port_array, 
    int *port_count);

extern int bcmx_mpls_tunnel_initiator_set(
    bcm_if_t intf, 
    int num_labels, 
    bcmx_mpls_egress_label_t *label_array);

extern int bcmx_mpls_tunnel_initiator_clear(
    bcm_if_t intf);

extern int bcmx_mpls_tunnel_initiator_clear_all(void);

extern int bcmx_mpls_tunnel_initiator_get(
    bcm_if_t intf, 
    int label_max, 
    bcmx_mpls_egress_label_t *label_array, 
    int *label_count);

extern int bcmx_mpls_tunnel_switch_add(
    bcmx_mpls_tunnel_switch_t *info);

extern int bcmx_mpls_tunnel_switch_delete(
    bcmx_mpls_tunnel_switch_t *info);

extern int bcmx_mpls_tunnel_switch_delete_all(void);

extern int bcmx_mpls_tunnel_switch_get(
    bcmx_mpls_tunnel_switch_t *info);

/* Create an MPLS EXP map instance. */
extern int bcmx_mpls_exp_map_create(
    uint32 flags, 
    int *exp_map_id);

/* Destroy an MPLS EXP map instance. */
extern int bcmx_mpls_exp_map_destroy(
    int exp_map_id);

/* 
 * Set the { internal priority, color }-to-EXP mapping in the specified
 * EXP map.
 */
extern int bcmx_mpls_exp_map_set(
    int exp_map_id, 
    bcm_mpls_exp_map_t *exp_map);

/* 
 * Get the { internal priority, color }-to-EXP mapping in the specified
 * EXP map.
 */
extern int bcmx_mpls_exp_map_get(
    int exp_map_id, 
    bcm_mpls_exp_map_t *exp_map);

extern int bcmx_mpls_label_stat_get(
    bcm_mpls_label_t label, 
    bcm_gport_t port, 
    bcm_mpls_stat_t stat, 
    uint64 *val);

extern int bcmx_mpls_label_stat_get32(
    bcm_mpls_label_t label, 
    bcm_gport_t port, 
    bcm_mpls_stat_t stat, 
    uint32 *val);

extern int bcmx_mpls_label_stat_clear(
    bcm_mpls_label_t label, 
    bcm_gport_t port, 
    bcm_mpls_stat_t stat);

/* Prevent or allow L2 MPLS packets from egressing a port. */
extern int bcmx_mpls_port_block_set(
    bcm_vpn_t vpn, 
    bcmx_lport_t lport, 
    int enable);

/* Get the L2 MPLS packet blocking state on a port. */
extern int bcmx_mpls_port_block_get(
    bcm_vpn_t vpn, 
    bcmx_lport_t lport, 
    int *enable);

/* Create a VPN instance. */
extern int bcmx_mpls_vpn_create(
    bcm_vpn_t vpn, 
    uint32 flags);

/* Delete a VPN instance. */
extern int bcmx_mpls_vpn_destroy(
    bcm_vpn_t vpn);

/* Bind interface/port/VLAN to VPN instance. */
extern int bcmx_mpls_vpn_add(
    bcm_vpn_t vpn, 
    bcmx_mpls_vpn_t *vpn_info);

/* Unbind interface/port/VLAN to VPN instance. */
extern int bcmx_mpls_vpn_delete(
    bcm_vpn_t vpn, 
    bcmx_mpls_vpn_t *vpn_info);

/* Unbind all interface/port/VLAN to the VPN instance. */
extern int bcmx_mpls_vpn_delete_all(
    bcm_vpn_t vpn);

/* Get the binding of interface/port/VLAN to VPN instance. */
extern int bcmx_mpls_vpn_get(
    bcm_vpn_t vpn, 
    int max_out, 
    bcmx_mpls_vpn_t *vpn_info, 
    int *real_count);

/* Add a Virtual Circuit to an L2 MPLS VPN. */
extern int bcmx_mpls_vpn_circuit_add(
    bcm_vpn_t vpn, 
    bcmx_mpls_circuit_t *vc_info);

/* Delete a Virtual Circuit from an L2 MPLS VPN. */
extern int bcmx_mpls_vpn_circuit_delete(
    bcm_vpn_t vpn, 
    bcmx_mpls_circuit_t *vc_info);

/* Delete all Virtual Circuits associated with an L2 MPLS VPN. */
extern int bcmx_mpls_vpn_circuit_delete_all(
    bcm_vpn_t vpn);

/* Retrieve Virtual Circuit information for an L2 MPLS VPN. */
extern int bcmx_mpls_vpn_circuit_get(
    bcm_vpn_t vpn, 
    int vc_array, 
    bcmx_mpls_circuit_t *vc_info, 
    int *vc_count);

/* Set the FEC to MPLS label mapping. */
extern int bcmx_mpls_l3_initiator_set(
    bcm_l3_intf_t *intf, 
    bcm_mpls_l3_initiator_t *mpls_label);

/* Clear the FEC to MPLS label mapping. */
extern int bcmx_mpls_l3_initiator_clear(
    bcm_l3_intf_t *intf);

/* Get the FEC to MPLS label mapping info. */
extern int bcmx_mpls_l3_initiator_get(
    bcm_l3_intf_t *intf, 
    bcm_mpls_l3_initiator_t *mpls_label, 
    int *count);

/* Clear all the FEC to MPLS label mappings. */
extern int bcmx_mpls_l3_initiator_clear_all(void);

/* 
 * Add MPLS label swapping, label popping, and packet forwarding
 * information.
 */
extern int bcmx_mpls_switch_add(
    bcmx_mpls_switch_t *mswitch_info);

/* Delete MPLS label swapping and packet forwarding information. */
extern int bcmx_mpls_switch_delete(
    bcmx_mpls_switch_t *mswitch_info);

/* Get MPLS label swapping and packet forwarding information. */
extern int bcmx_mpls_switch_get(
    bcm_mpls_switch_t *mswitch_info);

/* Delete all MPLS label swapping and packet forwarding information. */
extern int bcmx_mpls_switch_delete_all(
    int flag);

#endif /* defined(INCLUDE_L3) */

#endif /* __BCMX_MPLS_H__ */