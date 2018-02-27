/*
 * $Id: l3.c 1.8.2.2 Broadcom SDK $
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
 * File:        l3.c
 * Purpose:     Triumph2 L3 function implementations
 */


#include <soc/defs.h>

#include <assert.h>

#include <sal/core/libc.h>
#if defined(BCM_TRIUMPH2_SUPPORT)  && defined(INCLUDE_L3)

#include <shared/util.h>
#include <soc/mem.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/l3x.h>
#include <soc/lpm.h>
#include <soc/tnl_term.h>

#include <bcm/l3.h>
#include <bcm/tunnel.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/stack.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/triumph2.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/xgs3.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw_dispatch.h>

/*
 * Function:
 *      _bcm_tr2_l3_tnl_term_add
 * Purpose:
 *      Add tunnel termination entry to the hw.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      tnl_info - (IN)Tunnel terminator parameters. 
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_tr2_l3_tnl_term_add(int unit, uint32 *entry_ptr, bcm_tunnel_terminator_t *tnl_info)
{
    bcm_module_t mod_in, mod_out, my_mod;
    bcm_port_t port_in, port_out;
    _bcm_l3_ingress_intf_t iif;
    int tunnel, wlan;
    int rv;

    /* Program remote port */
    if ((tnl_info->type == bcmTunnelTypeWlanWtpToAc) || 
        (tnl_info->type == bcmTunnelTypeWlanAcToAc) ||
        (tnl_info->type == bcmTunnelTypeWlanWtpToAc6) ||
        (tnl_info->type == bcmTunnelTypeWlanAcToAc6)) {
        wlan = 1;
    } else {
        wlan = 0;
    }
    if (wlan) {
        if (tnl_info->flags & BCM_TUNNEL_TERM_WLAN_REMOTE_TERMINATE) {
            if (!BCM_GPORT_IS_MODPORT(tnl_info->remote_port)) {
                return BCM_E_PARAM;
            }
            mod_in = BCM_GPORT_MODPORT_MODID_GET(tnl_info->remote_port);
            port_in = BCM_GPORT_MODPORT_PORT_GET(tnl_info->remote_port); 
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET, mod_in, 
                                        port_in, &mod_out, &port_out));
            if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
                return (BCM_E_BADID);
            }
            if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
                return (BCM_E_PORT);
            }
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, REMOTE_TERM_GPPf,
                                       (mod_out << 6) | port_out);
        } else {
            /* Send to the local loopback port */
            rv = bcm_esw_stk_my_modid_get(unit, &my_mod);
            BCM_IF_ERROR_RETURN(rv);
            port_in = 54;
            mod_in = my_mod;
            rv = _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET, mod_in, 
                                        port_in, &mod_out, &port_out);
            BCM_IF_ERROR_RETURN(rv);

            soc_L3_TUNNELm_field32_set(unit, entry_ptr, REMOTE_TERM_GPPf,
                                       (mod_out << 6) | port_out);
        }
        /* Program tunnel id */
        if (tnl_info->flags & BCM_TUNNEL_TERM_TUNNEL_WITH_ID) {
            if (!BCM_GPORT_IS_TUNNEL(tnl_info->tunnel_id)) {
                return BCM_E_PARAM;
            }
            if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, TUNNEL_IDf)) {
                tunnel = BCM_GPORT_TUNNEL_ID_GET(tnl_info->tunnel_id);
                soc_L3_TUNNELm_field32_set(unit, entry_ptr, TUNNEL_IDf,
                                           tunnel);
            }
        }

    }
    if ((tnl_info->type == bcmTunnelTypeAutoMulticast) ||
        (tnl_info->type == bcmTunnelTypeAutoMulticast6)) {
        /* Program L3_IIFm */
        if(SOC_MEM_FIELD_VALID(unit, L3_IIFm, IPMC_L3_IIFf)) {
            sal_memset(&iif, 0, sizeof(_bcm_l3_ingress_intf_t));
            iif.intf_id = tnl_info->vlan;

            rv = _bcm_tr_l3_ingress_interface_get(unit, &iif);
            BCM_IF_ERROR_RETURN(rv);
            iif.vrf = tnl_info->vrf;
            rv = _bcm_tr_l3_ingress_interface_set(unit, &iif);
            BCM_IF_ERROR_RETURN(rv);
        }

        /* Program tunnel id */
        if (tnl_info->flags & BCM_TUNNEL_TERM_TUNNEL_WITH_ID) {
            if (!BCM_GPORT_IS_TUNNEL(tnl_info->tunnel_id)) {
                return BCM_E_PARAM;
            }
            if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, TUNNEL_IDf)) {
                tunnel = BCM_GPORT_TUNNEL_ID_GET(tnl_info->tunnel_id);
                soc_L3_TUNNELm_field32_set(unit, entry_ptr, TUNNEL_IDf,
                                           tunnel);
            }
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_tr2_l3_tnl_term_entry_init
 * Purpose:
 *      Initialize soc tunnel terminator entry key portion.
 * Parameters:
 *      unit     - (IN)  BCM device number. 
 *      tnl_info - (IN)  BCM buffer with tunnel info.
 *      entry    - (OUT) SOC buffer with key filled in.  
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_tr2_l3_tnl_term_entry_init(int unit, bcm_tunnel_terminator_t *tnl_info,
                                soc_tunnel_term_t *entry)
{
    int       idx;                /* Entry iteration index.     */
    int       idx_max;            /* Entry widht.               */
    uint32    *entry_ptr;         /* Filled entry pointer.      */
    _bcm_tnl_term_type_t tnl_type;/* Tunnel type.               */
    int       rv;                 /* Operation return status.   */

    /* Input parameters check. */
    if ((NULL == tnl_info) || (NULL == entry)) {
        return (BCM_E_PARAM);
    }

    /* Get tunnel type & sub_type. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_l3_set_tnl_term_type(unit, tnl_info, &tnl_type));

    /* Reset destination structure. */
    sal_memset(entry, 0, sizeof(soc_tunnel_term_t));

    /* Set Destination/Source pair. */
    entry_ptr = (uint32 *)&entry->entry_arr[0];
    if (tnl_type.tnl_outer_hdr_ipv6) {
        /* Apply mask on source address. */
        rv = bcm_xgs3_l3_mask6_apply(tnl_info->sip6_mask, tnl_info->sip6);
        BCM_IF_ERROR_RETURN(rv);

        /* SIP [0-63] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[0], IP_ADDRf,
                             tnl_info->sip6, SOC_MEM_IP6_LOWER_ONLY);
        /* SIP [64-127] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[1], IP_ADDRf,
                             tnl_info->sip6, SOC_MEM_IP6_UPPER_ONLY);
        /* DIP [0-63] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[2], IP_ADDRf,
                             tnl_info->dip6, SOC_MEM_IP6_LOWER_ONLY);
        /* DIP [64-127] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[3], IP_ADDRf,
                             tnl_info->dip6, SOC_MEM_IP6_UPPER_ONLY);

        /* SIP MASK [0-63] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[0], IP_ADDR_MASKf,
                             tnl_info->sip6_mask, SOC_MEM_IP6_LOWER_ONLY);
        /* SIP MASK [64-127] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[1], IP_ADDR_MASKf,
                             tnl_info->sip6_mask, SOC_MEM_IP6_UPPER_ONLY);
        /* DIP MASK [0-63] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[2], IP_ADDR_MASKf,
                             tnl_info->dip6_mask, SOC_MEM_IP6_LOWER_ONLY);
        /* DIP MASK [64-127] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[3], IP_ADDR_MASKf,
                             tnl_info->dip6_mask, SOC_MEM_IP6_UPPER_ONLY);
    }  else {
        tnl_info->sip &= tnl_info->sip_mask;

        /* Set destination ip. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, DIPf, tnl_info->dip);

        /* Set source ip. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, SIPf, tnl_info->sip);

        /* Set destination subnet mask. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, DIP_MASKf,
                                   tnl_info->dip_mask);

        /* Set source subnet mask. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, SIP_MASKf,
                                   tnl_info->sip_mask);
    }

    /* Resolve number of entries hw entry occupies. */
    idx_max = (tnl_type.tnl_outer_hdr_ipv6) ? SOC_TNL_TERM_IPV6_ENTRY_WIDTH : \
              SOC_TNL_TERM_IPV4_ENTRY_WIDTH;  
    
    for (idx = 0; idx < idx_max; idx++) {
        entry_ptr = (uint32 *)&entry->entry_arr[idx];

        /* Set valid bit. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, VALIDf, 1);

        /* Set tunnel subtype. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, SUB_TUNNEL_TYPEf,
                                   tnl_type.tnl_sub_type);

        /* Set tunnel type. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, TUNNEL_TYPEf,
                                   tnl_type.tnl_auto);

        /* Set entry mode. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, MODEf,
                                   tnl_type.tnl_outer_hdr_ipv6);
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, MODE_MASKf, 1);

        if (0 == idx) {
            /* Set the PROTOCOL field */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, PROTOCOLf,
                                       tnl_type.tnl_protocol);
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, PROTOCOL_MASKf, 0xff);
        }
        
        if ((tnl_info->type == bcmTunnelTypeWlanWtpToAc) ||
            (tnl_info->type == bcmTunnelTypeWlanAcToAc) ||
            (tnl_info->type == bcmTunnelTypeWlanWtpToAc6) ||
            (tnl_info->type == bcmTunnelTypeWlanAcToAc6)) { 

            /* Set the L4 ports - WLAN/AMT tunnels */
            if (0 == idx) {
                soc_L3_TUNNELm_field32_set(unit, entry_ptr, L4_DEST_PORTf,
                                           tnl_info->udp_dst_port);

                soc_L3_TUNNELm_field32_set(unit, entry_ptr,
                                           L4_DEST_PORT_MASKf, 0xffff);

                soc_L3_TUNNELm_field32_set(unit, entry_ptr, L4_SRC_PORTf,
                                           tnl_info->udp_src_port);
    
                soc_L3_TUNNELm_field32_set(unit, entry_ptr,
                                           L4_SRC_PORT_MASKf, 0xffff);
            }

            /* Set UDP tunnel type. */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, UDP_TUNNEL_TYPEf,
                                       tnl_type.tnl_udp_type);
            /* Ignore UDP checksum */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, IGNORE_UDP_CHECKSUMf,
   				       0x1);
        } else if (tnl_info->type == bcmTunnelTypeAutoMulticast) {

		/* Set UDP tunnel type. */
		soc_L3_TUNNELm_field32_set(unit, entry_ptr, UDP_TUNNEL_TYPEf,
					   tnl_type.tnl_udp_type);

		soc_L3_TUNNELm_field32_set(unit, entry_ptr, IGNORE_UDP_CHECKSUMf,
					   0x1);
		soc_L3_TUNNELm_field32_set(unit, entry_ptr, CTRL_PKTS_TO_CPUf,
                       0x1);
        } else if (tnl_info->type == bcmTunnelTypeAutoMulticast6) {

		/* Set UDP tunnel type. */
              soc_L3_TUNNELm_field32_set(unit, entry_ptr, UDP_TUNNEL_TYPEf,
						   tnl_type.tnl_udp_type);	
              soc_L3_TUNNELm_field32_set(unit, entry_ptr, IGNORE_UDP_CHECKSUMf,
						   0x1);
              soc_L3_TUNNELm_field32_set(unit, entry_ptr, CTRL_PKTS_TO_CPUf,
						   0x1);
        }

        /* Save vlan id for ipmc lookup.*/
        if((tnl_info->vlan) && SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, L3_IIFf)) {                 
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, L3_IIFf, tnl_info->vlan);
        }

        /* Set GRE payload */
        if (tnl_type.tnl_gre) {
            /* GRE IPv6 payload is allowed. */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, GRE_PAYLOAD_IPV6f,
                                       tnl_type.tnl_gre_v6_payload);

            /* GRE IPv6 payload is allowed. */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, GRE_PAYLOAD_IPV4f,
                                       tnl_type.tnl_gre_v4_payload);
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_tr2_l3_tnl_term_entry_parse
 * Purpose:
 *      Parse tunnel terminator entry portion.
 * Parameters:
 *      unit     - (IN)  BCM device number. 
 *      entry    - (IN)  SOC buffer with tunne information.  
 *      tnl_info - (OUT) BCM buffer with tunnel info.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_tr2_l3_tnl_term_entry_parse(int unit, soc_tunnel_term_t *entry,
                                  bcm_tunnel_terminator_t *tnl_info)
{
    _bcm_tnl_term_type_t tnl_type;     /* Tunnel type information.   */
    uint32 *entry_ptr;                 /* Filled entry pointer.      */
    int tunnel_id;                     /* Tunnel ID */
    int remote_port;                   /* Remote port */
    bcm_module_t mod, mod_out, my_mod; /* Module IDs */
    bcm_port_t port, port_out;         /* Physical ports */

    /* Input parameters check. */
    if ((NULL == tnl_info) || (NULL == entry)) {
        return (BCM_E_PARAM);
    }

    /* Reset destination structure. */
    sal_memset(tnl_info, 0, sizeof(bcm_tunnel_terminator_t));
    sal_memset(&tnl_type, 0, sizeof(_bcm_tnl_term_type_t));

    entry_ptr = (uint32 *)&entry->entry_arr[0];

    /* Get valid bit. */
    if (!soc_L3_TUNNELm_field32_get(unit, entry_ptr, VALIDf)) {
        return (BCM_E_NOT_FOUND);
    }

    if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, MODEf)) {
        tnl_type.tnl_outer_hdr_ipv6 =
            soc_L3_TUNNELm_field32_get(unit, entry_ptr, MODEf);
    }
     
    /* Get Destination/Source pair. */
    if (tnl_type.tnl_outer_hdr_ipv6) {
        /* SIP [0-63] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[0], IP_ADDRf,
                             tnl_info->sip6, SOC_MEM_IP6_LOWER_ONLY);
        /* SIP [64-127] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[1], IP_ADDRf,
                             tnl_info->sip6, SOC_MEM_IP6_UPPER_ONLY);
        /* DIP [0-63] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[2], IP_ADDRf,
                             tnl_info->dip6, SOC_MEM_IP6_LOWER_ONLY);
        /* DIP [64-127] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[3], IP_ADDRf,
                             tnl_info->dip6, SOC_MEM_IP6_UPPER_ONLY);

        /* SIP MASK [0-63] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[0], IP_ADDR_MASKf,
                             tnl_info->sip6_mask, SOC_MEM_IP6_LOWER_ONLY);
        /* SIP MASK [64-127] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[1], IP_ADDR_MASKf,
                             tnl_info->sip6_mask, SOC_MEM_IP6_UPPER_ONLY);
        /* DIP MASK [0-63] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[2], IP_ADDR_MASKf,
                             tnl_info->dip6_mask, SOC_MEM_IP6_LOWER_ONLY);
        /* DIP MASK [64-127] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[3], IP_ADDR_MASKf,
                             tnl_info->dip6_mask, SOC_MEM_IP6_UPPER_ONLY);

    }  else {
        /* Get destination ip. */
        tnl_info->dip = soc_L3_TUNNELm_field32_get(unit, entry_ptr, DIPf);

        /* Get source ip. */
        tnl_info->sip = soc_L3_TUNNELm_field32_get(unit, entry_ptr, SIPf);

        /* Destination subnet mask. */
        tnl_info->dip_mask = BCM_XGS3_L3_IP4_FULL_MASK;

        /* Source subnet mask. */
        tnl_info->sip_mask = soc_L3_TUNNELm_field32_get(unit, entry_ptr, 
                                                        SIP_MASKf);
    }

    /* Get tunnel subtype. */
    tnl_type.tnl_sub_type = 
        soc_L3_TUNNELm_field32_get(unit, entry_ptr, SUB_TUNNEL_TYPEf);

    /* Get UDP tunnel type. */
    tnl_type.tnl_udp_type = 
        soc_L3_TUNNELm_field32_get(unit, entry_ptr, UDP_TUNNEL_TYPEf);

    /* Get tunnel type. */
    tnl_type.tnl_auto = 
        soc_L3_TUNNELm_field32_get(unit, entry_ptr, TUNNEL_TYPEf);

    /* Copy DSCP from outer header flag. */
    if (soc_L3_TUNNELm_field32_get(unit, entry_ptr, USE_OUTER_HDR_DSCPf)) {
        tnl_info->flags |= BCM_TUNNEL_TERM_USE_OUTER_DSCP;
    }
    /* Copy TTL from outer header flag. */
    if (soc_L3_TUNNELm_field32_get(unit, entry_ptr, USE_OUTER_HDR_TTLf)) {
        tnl_info->flags |= BCM_TUNNEL_TERM_USE_OUTER_TTL;
    }
    /* Keep inner header DSCP flag. */
    if (soc_L3_TUNNELm_field32_get(unit, entry_ptr,
                                   DONOT_CHANGE_INNER_HDR_DSCPf)) {
        tnl_info->flags |= BCM_TUNNEL_TERM_KEEP_INNER_DSCP;
    }

    soc_mem_pbmp_field_get(unit, L3_TUNNELm, entry_ptr, ALLOWED_PORT_BITMAPf,
                           &tnl_info->pbmp);

    /* Tunnel or IPMC lookup vlan id */
    tnl_info->vlan = soc_L3_TUNNELm_field32_get(unit, entry_ptr, IINTFf);

    /*  Get trust dscp per tunnel */ 
    if (soc_L3_TUNNELm_field32_get(unit, entry_ptr, USE_OUTER_HDR_DSCPf)) {
        tnl_info->flags |= BCM_TUNNEL_TERM_DSCP_TRUST;
    }

    /* Get the protocol field and make some decisions */
    tnl_type.tnl_protocol = soc_L3_TUNNELm_field32_get(unit, entry_ptr, 
                                                       PROTOCOLf);
    switch (tnl_type.tnl_protocol) {
        case 0x2F:
            tnl_type.tnl_gre = 1;
            break;
        case 0x67:
            tnl_type.tnl_pim_sm = 1;
        default:
            break;
    }
    /* Get gre IPv4 payload allowed. */
    tnl_type.tnl_gre_v4_payload = 
        soc_L3_TUNNELm_field32_get(unit, entry_ptr, GRE_PAYLOAD_IPV4f); 

    /* Get gre IPv6 payload allowed. */
    tnl_type.tnl_gre_v6_payload = 
        soc_L3_TUNNELm_field32_get(unit, entry_ptr, GRE_PAYLOAD_IPV6f);

    /* Get the L4 data */
    if (soc_mem_field_valid (unit, L3_TUNNELm, L4_SRC_PORTf)) {
        tnl_info->udp_src_port = soc_L3_TUNNELm_field32_get(unit, entry_ptr, 
                                                        L4_SRC_PORTf);
    }
    if (soc_mem_field_valid (unit, L3_TUNNELm, L4_DEST_PORTf)) {
        tnl_info->udp_dst_port = soc_L3_TUNNELm_field32_get(unit, entry_ptr, 
                                                        L4_DEST_PORTf);
    }
    /* Get the tunnel ID */
    if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, TUNNEL_IDf)) {
        tunnel_id = soc_L3_TUNNELm_field32_get(unit, entry_ptr, TUNNEL_IDf);
        if (tunnel_id) {
            BCM_GPORT_TUNNEL_ID_SET(tnl_info->tunnel_id, tunnel_id);
            tnl_info->flags |= BCM_TUNNEL_TERM_TUNNEL_WITH_ID;
        }
    }

    /* Get the remote port member */
    if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, REMOTE_TERM_GPPf)) {
        remote_port = soc_L3_TUNNELm_field32_get(unit, entry_ptr,
                                                 REMOTE_TERM_GPPf);
        mod = (remote_port >> 6) & 0x7F;
        port = remote_port & 0x3F;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                    mod, port, &mod_out, &port_out));
        BCM_GPORT_MODPORT_SET(tnl_info->remote_port, mod_out, port_out);
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_mod));
        if (mod != my_mod) {
            tnl_info->flags |= BCM_TUNNEL_TERM_WLAN_REMOTE_TERMINATE;
        } 
    }
                                
    /* Get tunnel type, sub_type and protocol. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_l3_get_tnl_term_type(unit, tnl_info, &tnl_type));

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_tr2_l3_ecmp_grp_get
 * Purpose:
 *      Get ecmp group next hop members by index.
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      ecmp_grp - (IN)Ecmp group id to read. 
 *      ecmp_count - (IN)Maximum number of entries to read.
 *      nh_idx     - (OUT)Next hop indexes. 
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_tr2_l3_ecmp_grp_get(int unit, int ecmp_grp, int ecmp_group_size, int *nh_idx)
{
    int idx;                                /* Iteration index.              */
    int max_ent_count;                      /* Number of entries to read.    */
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry.      */
    int one_entry_grp = TRUE;               /* Single next hop entry group.  */ 
    int rv = BCM_E_UNAVAIL;                 /* Operation return status.      */
    int ecmp_idx;

    /* Input parameters sanity check. */
    if ((NULL == nh_idx) || (ecmp_group_size < 1)) {
        return (BCM_E_PARAM);
    }

    /* Zero all next hop indexes first. */
    sal_memset(nh_idx, 0, ecmp_group_size * sizeof(int));
    sal_memset(hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

    /* Get group base pointer. */
    BCM_IF_ERROR_RETURN(soc_mem_read(unit, L3_ECMP_COUNTm,
                                          MEM_BLOCK_ANY, ecmp_grp, hw_buf));
    ecmp_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, BASE_PTRf);

    /* Read zero based ecmp count. */
    rv = soc_mem_read(unit, L3_ECMP_COUNTm, MEM_BLOCK_ANY, ecmp_grp, hw_buf);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, COUNTf);
    max_ent_count++; /* Count is zero based. */ 

    /* Read all the indexes from hw. */
    for (idx = 0; idx < max_ent_count; idx++) {

        /* Read next hop index. */
        rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY, 
                          (ecmp_idx + idx), hw_buf);
        if (rv < 0) {
            break;
        }
        nh_idx[idx] = soc_mem_field32_get(unit, L3_ECMPm, 
                                          hw_buf, NEXT_HOP_INDEXf);
        /* Check if group contains . */ 
        if (idx && (nh_idx[idx] != nh_idx[0])) { 
            one_entry_grp = FALSE;
        }

         /* Next hops popuplated in cycle,stop once you read first entry again */
         if (idx && (FALSE == one_entry_grp) && (nh_idx[idx] == nh_idx[0])) {
             nh_idx[idx] = 0;
             break;
         }
    }
    /* Reset rest of the group if only 1 next hop is present. */
    if (one_entry_grp) {
       sal_memset(nh_idx + 1, 0, (ecmp_group_size - 1) * sizeof(int)); 
    }
    return rv;
}

/*
 * Function:
 *      _bcm_tr2_l3_ecmp_grp_add
 * Purpose:
 *      Add ecmp group next hop members, or reset ecmp group entry.  
 *      NOTE: Function always writes all the entries in ecmp group.
 *            If there is not enough nh indexes - next hops written
 *            in cycle. 
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      ecmp_grp   - (IN)ecmp group id to write.
 *      buf        - (IN)Next hop indexes or NULL for entry reset.
 *      max_paths - (IN) ECMP Max paths
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_tr2_l3_ecmp_grp_add(int unit, int ecmp_grp, void *buf, int max_paths)
{
    uint32 l3_ecmp[SOC_MAX_MEM_FIELD_WORDS];        /* l3_ecmp buf             */
    uint32 l3_ecmp_count[SOC_MAX_MEM_FIELD_WORDS];  /* l3_ecmp_count buf       */
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS];         /* l3_ecmp_count buf       */
    int max_grp_size=0;                             /* Maximum ecmp group size.*/
    int ecmp_idx;                                   /* Ecmp table entry index. */
    int *nh_idx;                                    /* Ecmp group nh indexes.  */
    int idx=0;                                      /* Iteration index.        */
    int rv = BCM_E_UNAVAIL;                         /* Operation return value. */
    _bcm_l3_tbl_op_t data;

    /* Input parameters check. */
    if (NULL == buf) {
        return (BCM_E_PARAM);
    }

    /* Cast input buffer. */
    nh_idx = (int *) buf;
    max_grp_size = max_paths;

    /* Calculate table index. */
    data.width = max_paths;
    data.tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp); 
    data.oper_flags = _BCM_L3_SHR_TABLE_TRAVERSE_CONTROL; 
    data.entry_index = -1;
    /* Get index to the first slot in the ECMP table
     * that can accomodate max_grp_size */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_tbl_free_idx_get(unit, &data));
    ecmp_idx = data.entry_index;
    BCM_XGS3_L3_ENT_REF_CNT_INC(data.tbl_ptr, data.entry_index, max_grp_size);
    if (ecmp_idx >= BCM_XGS3_L3_ECMP_TBL_SIZE(unit)) {
        return BCM_E_FULL;
    }

    sal_memset (hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

    /* Write all the indexes to hw. */
    for (idx = 0; idx < max_grp_size; idx++) {

        /* Set next hop index. */
        sal_memset (l3_ecmp, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

        if (!nh_idx[idx]) {
            break;
        }

        soc_mem_field32_set(unit, L3_ECMPm, l3_ecmp, NEXT_HOP_INDEXf,
                            nh_idx[idx]);
        /* Write buffer to hw L3_ECMPm table. */
        rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL, 
                           (ecmp_idx + idx), l3_ecmp);
        if (BCM_FAILURE(rv)) {
            break;
        }

        /* Write buffer to hw INITIAL_L3_ECMPm table. */
        rv = soc_mem_write(unit, INITIAL_L3_ECMPm, MEM_BLOCK_ALL, 
                                    (ecmp_idx + idx), l3_ecmp);
        if (BCM_FAILURE(rv)) {
            break;
        }
    }
    if (BCM_SUCCESS(rv)) {
        /* Set Max Group Size. */
        sal_memset (l3_ecmp_count, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));
        soc_mem_field32_set(unit, L3_ECMP_COUNTm, l3_ecmp_count, COUNTf, max_grp_size - 1);

        rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                           (ecmp_grp+1), l3_ecmp_count);
        BCM_IF_ERROR_RETURN(rv);

        /* Set zero based ecmp count. */
        if (!idx) {
            soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNTf, idx);
        } else {
            soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNTf, idx - 1);
        }

        /* Set group base pointer. */
        soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, 
                            BASE_PTRf, ecmp_idx);

        rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                           ecmp_grp, hw_buf);

        BCM_IF_ERROR_RETURN(rv);

        rv = soc_mem_write(unit, INITIAL_L3_ECMP_COUNTm,
                           MEM_BLOCK_ALL, ecmp_grp, hw_buf);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_tr2_l3_ecmp_grp_del
 * Purpose:
 *      Reset ecmp group next hop members
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      ecmp_grp   - (IN)ecmp group id to write.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_tr2_l3_ecmp_grp_del(int unit, int ecmp_grp, int max_grp_size)
{
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry.*/
    int ecmp_idx;               /* Ecmp table entry index. */
    int idx;                    /* Iteration index.        */
    int rv = BCM_E_UNAVAIL;     /* Operation return value. */
    _bcm_l3_tbl_op_t data;
    data.tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp); 

    /* Initialize ecmp entry. */
    sal_memset (hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

    /* Calculate table index. */
    BCM_IF_ERROR_RETURN(soc_mem_read(unit, L3_ECMP_COUNTm,
                                          MEM_BLOCK_ANY, ecmp_grp, hw_buf));
    ecmp_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, BASE_PTRf);

    /* Write all the indexes to hw. */
    for (idx = 0; idx < max_grp_size; idx++) {
        /* Write buffer to hw. */
        rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL, 
                           (ecmp_idx + idx), hw_buf);

        if (BCM_FAILURE(rv)) {
            return rv;
        }

        /* Write initial ecmp table. */
        if (SOC_MEM_IS_VALID(unit, INITIAL_L3_ECMPm)) {
            /* Write buffer to hw. */
            rv = soc_mem_write(unit, INITIAL_L3_ECMPm, MEM_BLOCK_ALL, 
                           (ecmp_idx + idx), hw_buf);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }
    }

    /* Decrement ref count for the entries in ecmp table
     * Ref count for ecmp_group table is decremented in common table del func. */
    BCM_XGS3_L3_ENT_REF_CNT_DEC(data.tbl_ptr, ecmp_idx, max_grp_size);

    /* Set group base pointer. */
    ecmp_idx = ecmp_grp;

    rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                       ecmp_idx, hw_buf);
    BCM_IF_ERROR_RETURN(rv);

    rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                       (ecmp_idx+1), hw_buf);
    BCM_IF_ERROR_RETURN(rv);

    rv = soc_mem_write(unit, INITIAL_L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                       ecmp_idx, hw_buf);
    return rv;
}

#else /* BCM_TRIUMPH2_SUPPORT && INCLUDE_L3 */
int bcm_esw_triumph2_l3_not_empty;
#endif /* BCM_TRIUMPH2_SUPPORT && INCLUDE_L3 */
