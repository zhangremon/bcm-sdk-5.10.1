/*
 * $Id: l2.c 1.64.30.1 Broadcom SDK $
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
 * File:        l2.c
 * Purpose:     BCM Layer-2 switch API
 */
#ifdef BCM_FE2000_P3_SUPPORT

#include <sal/core/sync.h>

#include <soc/macipadr.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/g2p3/g2p3.h>

#include <shared/gport.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/l2.h>
#include <bcm/stack.h>
#include <bcm/mcast.h>
#include <bcm/vlan.h>
#include <bcm/tunnel.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/vswitch.h>

#include <shared/idxres_fl.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/l2.h>
#include <bcm_int/sbx/mcast.h>
#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/fe2000/vlan.h>
#include <bcm_int/sbx/fe2000/mpls.h>
#include <bcm_int/sbx/fe2000/port.h>
#include <bcm_int/sbx/fe2000/g2p3.h>

static bcm_mac_t _mac_null = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

extern int _bcm_fe2000_mim_fte_gport_get(int unit,
                                         bcm_gport_t gport,
                                         uint32 *ftidx);

extern bcm_gport_t _bcm_fe2000_mim_fte_to_gport_id(int unit,
                                            uint32 ftidx);

void _bcm_l2_fe2000_g2p3_dump_smac_entry(soc_sbx_g2p3_6_byte_t mac,
                                         int vid, 
                                         soc_sbx_g2p3_cmac_t *data)
{
    L2_DUMP("  %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x  %4d  "
            "0x%6.6x   %c    %c   %6d\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            vid, data->pid,
            data->sdrop ? CHAR_SET : CHAR_CLEAR,
            data->dontage ? CHAR_SET : CHAR_CLEAR, data->age);
}

void _bcm_l2_fe2000_g2p3_dump_dmac_entry(soc_sbx_g2p3_6_byte_t mac,
                                         int vid, 
                                         soc_sbx_g2p3_cmac_t *data)
{
    /* macData.pid is the FTE */
    L2_DUMP("  %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x  %4d  "
            "%8d   %c   %6d\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            vid, data->pid,
            data->dontage ? CHAR_SET : CHAR_CLEAR, data->age);
}

typedef void (*mac_dump_f) (soc_sbx_g2p3_6_byte_t mac,
                            int vid, 
                            soc_sbx_g2p3_cmac_t *data);



int
_bcm_fe2000_g2p3_l2_hw_init(int unit)
{
    int                   rv = BCM_E_NONE;
#ifdef BCM_QE2000_SUPPORT
    soc_sbx_g2p3_xt_t     sbx_xt;    /* exception table */
    uint32                exc_idx;
#endif
    soc_sbx_g2p3_state_t *pFe;

    G2P3_FE_HANDLER_GET(unit, pFe);

    /*
     * L2 learning
     */
#ifdef BCM_QE2000_SUPPORT
    /* Set exception for L2 learning */
    BCM_IF_ERROR_RETURN
      (soc_sbx_g2p3_exc_smac_learn_idx_get(unit, &exc_idx));
    soc_sbx_g2p3_xt_t_init(&sbx_xt);

    rv = soc_sbx_g2p3_xt_get(unit, exc_idx, &sbx_xt);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "failed to get L2 learn exception entry rv=%d(%s)\n"),
                rv, bcm_errmsg(rv)));
        return rv;
    }

    sbx_xt.dp     = 0;
    /*sbx_xt.qid    = SBX_EXC_QID_BASE;*/
    sbx_xt.learn  = 0;
    sbx_xt.forward = 1;
    sbx_xt.trunc  = 1;
    sbx_xt.ppspolice = 1;

    rv = soc_sbx_g2p3_xt_set(unit, exc_idx, &sbx_xt);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "failed to set L2 learn exception entry rv=%d(%s)\n"),
                rv, bcm_errmsg(rv)));
        return rv;
    }
#endif /* BCM_QE2000_SUPPORT */

    return rv;
}

int
_bcm_fe2000_g2p3_l2_egress_hw_entry_delete(int unit, bcm_if_t encap_id)
{
    int                         rv = BCM_E_NONE;
    uint32                      ohi_idx, encap_ete_idx, ete_idx;
    soc_sbx_g2p3_oi2e_t         sbx_ohi;
    soc_sbx_g2p3_eteencap_t     sbx_ete_encap;
    soc_sbx_g2p3_etel2_t        sbx_ete_l2;


    ohi_idx = SOC_SBX_OHI_FROM_L2_ENCAP_ID(encap_id);
    
    rv = soc_sbx_g2p3_oi2e_get(unit, ohi_idx - SBX_RAW_OHI_BASE, &sbx_ohi);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    
    encap_ete_idx = sbx_ohi.eteptr;
    rv = soc_sbx_g2p3_eteencap_get(unit, encap_ete_idx, &sbx_ete_encap);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    
    ete_idx = sbx_ete_encap.l2ete;
    rv = soc_sbx_g2p3_etel2_get(unit, ete_idx, &sbx_ete_l2);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    soc_sbx_g2p3_eteencap_t_init(&sbx_ete_encap);
    rv = soc_sbx_g2p3_eteencap_set(unit, encap_ete_idx, &sbx_ete_encap);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    soc_sbx_g2p3_oi2e_t_init(&sbx_ohi);        
    rv = soc_sbx_g2p3_oi2e_set(unit, ohi_idx - SBX_RAW_OHI_BASE, &sbx_ohi);

    if (rv == BCM_E_NONE) {
        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_ENCAP, 1, 
                               &encap_ete_idx, 0);
        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_L2, 1, 
                               &ete_idx, 0);
    }

    return rv;
}


int
_bcm_fe2000_g2p3_l2addr_hw_add(int unit, bcm_l2_addr_t *l2addr)
{
    int                   rv;
    soc_sbx_g2p3_cmac_t   sbx_mac_data;
 
    soc_sbx_g2p3_cmac_t_init(&sbx_mac_data);

    if (l2addr->flags & BCM_L2_MCAST) {
        /* Mcast */
        /* Note: mcast address is handled separately */

    } else if (BCM_GPORT_IS_VLAN_PORT(l2addr->port)) {
        /* VLAN GPORT */
        uint32 portFte = 0;
        rv = _bcm_fe2000_vlan_fte_gport_get(unit, l2addr->port, &portFte);

        if (BCM_E_NONE == rv) {
            sbx_mac_data.pid = portFte;
        } else {
            return rv;
        }
    } else if (BCM_GPORT_IS_MIM_PORT(l2addr->port)) {
        /* MiM GPORT */
        uint32 portFte = 0;
        rv = _bcm_fe2000_mim_fte_gport_get(unit, l2addr->port, &portFte);

        if (BCM_E_NONE == rv) {
            sbx_mac_data.pid = portFte;
        } else {
            return rv;
        }
    } else if (BCM_GPORT_IS_MPLS_PORT(l2addr->port)) {
        /* MPLS GPORT */
        sbx_mac_data.pid = BCM_GPORT_MPLS_PORT_ID_GET(l2addr->port);

    } else if (l2addr->flags & BCM_L2_TRUNK_MEMBER) {    /* Trunk */
        if (!SBX_TRUNK_VALID(l2addr->tgid)) {
            return BCM_E_BADID;
        }
    
        sbx_mac_data.pid = SOC_SBX_TRUNK_FTE(unit, l2addr->tgid);
    } else {    /* Modid and port */
        int                   node, fab_unit, fab_port;

        if (!SOC_SBX_MODID_ADDRESSABLE(unit, l2addr->modid)) {
            return BCM_E_BADID;
        }
        if (!SOC_SBX_PORT_ADDRESSABLE(unit, l2addr->port)) {
            return BCM_E_PORT;
        }

        /* Get Node associate to module-id */
        node = SOC_SBX_REMOTE_NODE_GET(unit, l2addr->modid, l2addr->port);
        if (!SOC_SBX_NODE_ADDRESSABLE(unit, node)) {
            return BCM_E_BADID;
        }

        if (soc_sbx_node_port_get(unit, l2addr->modid, l2addr->port,
                                   &fab_unit, &node, &fab_port) == 0) {

            sbx_mac_data.pid = SOC_SBX_PORT_FTE(unit, node, fab_port);

        } else {
            L2_ERR((_SBX_D(unit, "soc_sbx_node_port_get for modid %d port %d "
                           "returned node %d port %d\n"), 
                    l2addr->modid, l2addr->port, node, fab_port));
            return BCM_E_INTERNAL;
        }
    }

    /* Drop */
    if (l2addr->flags & BCM_L2_DISCARD_SRC) {
        sbx_mac_data.sdrop = 1;
    }
    if (l2addr->flags & BCM_L2_DISCARD_DST) {
        sbx_mac_data.ddrop = 1;
    }

    /* Static */
    if (l2addr->flags & BCM_L2_STATIC) {
        sbx_mac_data.dontage = 1;
    }

    /* Age */
    sbx_mac_data.age = _bcm_fe2000_l2_age_ager_get(unit);


    /* there is only one table for both SMAC an DMAC in g2p3 */
    /* this call adds/updates and commits to hw */
    if (SOC_SBX_STATE(unit)->cache_l2 == FALSE) {
        rv = soc_sbx_g2p3_cmac_set(unit,
                                  l2addr->mac,
                                  l2addr->vid,
                                  &sbx_mac_data);
    } else {
        rv = soc_sbx_g2p3_cmac_add(unit,
                                  l2addr->mac,
                                  l2addr->vid,
                                  &sbx_mac_data);
    }
    return rv;
}


STATIC int
_bcm_fe2000_g2p3_mac_dump(int unit, mac_dump_f dump_f,  bcm_mac_t mac,
                          bcm_vlan_t vid, int max_count)
{
    soc_sbx_g2p3_6_byte_t  sbx_mac;
    soc_sbx_g2p3_cmac_t     sbx_mac_data;
    int                    sbx_vid;
    int                    count;
    int                    nullMac;
    int                    rv;

    nullMac = (ENET_CMP_MACADDR(mac, _mac_null) == 0);
    /* Initialize the iterator if both supplied, else get the first.  */
    if ((vid != 0) && !nullMac) {
        sbx_vid = vid;
        sal_memcpy(sbx_mac, mac, sizeof(sbx_mac));
        rv = BCM_E_NONE;
    } else {
        rv = soc_sbx_g2p3_cmac_first(unit, sbx_mac, &sbx_vid);
    }

    count = 0;

    while (rv == BCM_E_NONE && ((max_count <= 0) || (count < max_count))) {
        
        rv = soc_sbx_g2p3_cmac_get(unit, sbx_mac, sbx_vid, &sbx_mac_data);
        if (BCM_FAILURE(rv)) {
            break;
        }
        
        if (((vid == 0 || vid == sbx_vid) && nullMac) ||
            ((vid == 0 || vid == sbx_vid) && 
             (ENET_CMP_MACADDR(mac, sbx_mac) == 0))) 
        {
            dump_f(sbx_mac, sbx_vid, &sbx_mac_data);
            count++;
        }          

        rv = soc_sbx_g2p3_cmac_next(unit, sbx_mac, sbx_vid,
                                    sbx_mac, &sbx_vid); 
    }
    
    L2_DUMP("Total entries = %d\n", count);   
    return BCM_E_NONE;


}


int
_bcm_fe2000_g2p3_smac_dump(int unit, bcm_mac_t mac, 
                           bcm_vlan_t vid, int max_count)
{
    L2_DUMP("SMAC Table\n");
    L2_DUMP("  SMAC               VLAN       POE  DROP !AGE    AGE\n");
    L2_DUMP("  ---------------------------------------------------\n");

    _bcm_fe2000_g2p3_mac_dump(unit, 
                              _bcm_l2_fe2000_g2p3_dump_smac_entry,
                              mac, vid, max_count);
    return BCM_E_NONE;
}


int
_bcm_fe2000_g2p3_dmac_dump(int unit, bcm_mac_t mac, 
                           bcm_vlan_t vid, int max_count)
{

    L2_DUMP("DMAC Table\n");
    L2_DUMP("  DMAC               VLAN       FTE  !AGE    AGE\n");
    L2_DUMP("  ----------------------------------------------\n");

    _bcm_fe2000_g2p3_mac_dump(unit, 
                              _bcm_l2_fe2000_g2p3_dump_dmac_entry,
                              mac, vid, max_count);

    return BCM_E_NONE;
}


int _bcm_fe2000_g2p3_l2_mac_cmp(int unit, int vid, soc_sbx_g2p3_6_byte_t mac,
                                soc_sbx_g2p3_cmac_t *mac_data,
                                bcm_l2_addr_t *match, 
                                uint32 flags)
{
    int          port;
    int          modid;
    bcm_trunk_t  tgid;

    /* Skip non-static entries if DELETE flag is not set */
    if (mac_data->dontage && !(match->flags & BCM_L2_DELETE_STATIC)) {
        return -1;
    }
    
    if (flags == 0) {
        return -1;
    }

    SOC_SBX_PORT_TGID_GET(unit, mac_data->pid, modid, port, tgid);

    /* use short-circuit to compare any combination of values based on flags
     */
    if( ( !(flags & L2_CMP_MAC)   || !ENET_CMP_MACADDR(mac, match->mac)) &&
        ( !(flags & L2_CMP_VLAN)  || !CMP_VLAN(vid, match->vid))         &&
        ( !(flags & L2_CMP_PORT)  || ((modid == match->modid) && 
                                      (port == match->port)))            &&
        ( !(flags & L2_CMP_TRUNK) || tgid == match->tgid)
        )
    {
        return 0;
    }
    return -1;
}


int
_bcm_fe2000_g2p3_l2_addr_delete_by(int unit, bcm_l2_addr_t *match,
                                   uint32 cmp_flags)
{
    int                    rv = BCM_E_NONE;
    int                    count;
    int                    idx;
    int                    sbx_vid[2];
    soc_sbx_g2p3_6_byte_t  sbx_mac[2];
    soc_sbx_g2p3_cmac_t     sbx_mac_data;

    count = 0;
    soc_sbx_g2p3_cmac_t_init(&sbx_mac_data);

    idx = 0;
    rv = soc_sbx_g2p3_cmac_first(unit, sbx_mac[idx], &sbx_vid[idx]);
    if (BCM_FAILURE(rv)){
        if ((cmp_flags & L2_CMP_VLAN) && rv == SOC_E_NOT_FOUND) {
            return BCM_E_NONE;
        }
        L2_ERR((_SBX_D(unit, "Failed mac_first_get: %d (%s)\n"), 
                rv, bcm_errmsg(rv)));
        return rv;
    }

    
    do {
        int compare;

        rv = soc_sbx_g2p3_cmac_get(unit, sbx_mac[idx], sbx_vid[idx],
                                  &sbx_mac_data);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to get mac data. vid=%d mac="
                           L2_6B_MAC_FMT "\n"),
                    sbx_vid[idx], L2_6B_MAC_PFMT(sbx_mac[idx])));

            break;
        }

        compare = _bcm_fe2000_g2p3_l2_mac_cmp(unit, sbx_vid[idx], 
                                              sbx_mac[idx], &sbx_mac_data,
                                              match, cmp_flags);

        /* get the next before deleting */
        rv = soc_sbx_g2p3_cmac_next(unit, sbx_mac[idx], sbx_vid[idx],
                                    sbx_mac[!idx], &sbx_vid[!idx]);

        if (compare == 0) {
            L2_VERB((_SBX_D(unit, "removed vid=%d mac=" L2_6B_MAC_FMT "\n"),
                     sbx_vid[idx], L2_6B_MAC_PFMT(sbx_mac[idx])));
            soc_sbx_g2p3_cmac_remove(unit, sbx_mac[idx], sbx_vid[idx]);
            count++;
        }

        idx = !idx;

    } while (BCM_SUCCESS(rv));

    if (BCM_E_NOT_FOUND == rv) {
        /* exited the loop after scanning all of the addresses */
        rv = BCM_E_NONE;
    }

    /* Commit */
    if (count) {
        if (SOC_SBX_STATE(unit)->cache_l2 == FALSE) {
            rv = soc_sbx_g2p3_cmac_commit(unit, SB_COMMIT_COMPLETE);
        }
    }

    L2_VERB((_SBX_D(unit, "Total entries deleted %d, rv=%d\n"), count, rv));
    
    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_l2_addr_delete
 * Purpose:
 *     Delete an L2 address (MAC+VLAN) from the device.
 * Parameters:
 *     unit - Device number
 *     mac  - MAC address to delete
 *     vid  - VLAN id 
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_NOT_FOUND - L2 address entry (MAC+VLAN) not found
 *     BCM_E_XXX       - Failure
 * Notes:
 *     Assumes valid unit.
 *     Assumes lock is held.
 */
int
_bcm_fe2000_g2p3_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    int  rv = BCM_E_NONE;

    rv = soc_sbx_g2p3_cmac_remove(unit, mac, vid);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "failed to remove L2 MAC entry rv=%d(%s)\n"),
                rv, bcm_errmsg(rv)));
    } else {
        if (SOC_SBX_STATE(unit)->cache_l2 == FALSE) {
            rv = soc_sbx_g2p3_cmac_commit(unit, SB_COMMIT_COMPLETE);
            if (BCM_FAILURE(rv)) {
                L2_ERR((_SBX_D(unit, "failed to commit L2 remove rv=%d(%s)\n"), 
                        rv, bcm_errmsg(rv)));
            }
        }
    }
    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_l2_addr_get
 * Purpose:
 *     Given a MAC address and VLAN ID, return all associated information
 *     if entry is present in the L2 tables.
 * Parameters:
 *     unit   - Device number
 *     mac    - MAC address to search
 *     vid    - VLAN id to search
 *     l2addr - (OUT) Pointer to bcm_l2_addr_t structure to return L2 entry
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_NOT_FOUND - L2 address entry (MAC+VLAN) not found
 *     BCM_E_PARAM     - Illegal parameter (NULL pointer)
 *     BCM_E_XXX       - Failure, other
 * Notes:
 *     Assumes valid unit and non-null params.
 *     Assumes lock is held.
 */
int
_bcm_fe2000_g2p3_l2_addr_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                             bcm_l2_addr_t *l2addr)
{
    int                 rv;
    int                 node, port, tgid;
    soc_sbx_g2p3_cmac_t mac_data;
    bcm_gport_t         fabric_port, switch_port;
    bcm_vlan_t          vsi;

    rv = soc_sbx_g2p3_cmac_get(unit, mac, vid, &mac_data);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "failed to get L2 MAC entry rv=%d(%s)\n"),
                rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Init struct */
    bcm_l2_addr_t_init(l2addr, mac, vid);

    /* Mod/port, trunk or mcast index */
    SOC_SBX_PORT_TGID_GET(unit, mac_data.pid, node, port, tgid);
    if ((node >= 0) && (port >= 0)) {
        /* Modid and port */
        /* Convert qid node port to switch port for local settings */
        SOC_SBX_MODID_FROM_NODE(node, node);
        BCM_GPORT_MODPORT_SET(fabric_port, node, port);
        rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, 
                                                    fabric_port, &switch_port);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to convert fabric port to switch"
                           " port: %s\n"), bcm_errmsg(rv)));
            return rv;
        }

        l2addr->modid = BCM_GPORT_MODPORT_MODID_GET(switch_port);
        l2addr->port  = BCM_GPORT_MODPORT_PORT_GET(switch_port);

    } else if (tgid >= 0) {
        /* Trunk */
        l2addr->tgid   = tgid;
        l2addr->flags |= BCM_L2_TRUNK_MEMBER;
    } else {

        /* If FTIDX is a global GPORT FT, verify if its a mim port */
        if (mac_data.pid >= SBX_GLOBAL_GPORT_FTE_BASE(unit) &&
            mac_data.pid <= SBX_GLOBAL_GPORT_FTE_END(unit)) {
            uint8 type = 0;
            
            rv = _sbx_gu2_get_global_fte_type(unit, mac_data.pid, &type);
            
            if (BCM_SUCCESS(rv)) {
                /* if its a MiM GPORT FTE, return GPORT ID on port */
                if (BCM_GPORT_MIM_PORT == type) {
                    l2addr->port = _bcm_fe2000_mim_fte_to_gport_id(unit, mac_data.pid);
                } else {
                    L2_WARN((_SBX_D(unit, "PID 0x%x in gport range, but unexpected type %d\n"),
                             mac_data.pid, type));
                }
            } else {
                L2_ERR((_SBX_D(unit, "Failed to get type for gport 0x%x: %s\n"),
                        mac_data.pid, bcm_errmsg(rv)));
            }
        } else if (mac_data.pid >= SBX_LOCAL_GPORT_FTE_BASE(unit) &&
                   mac_data.pid <= SBX_LOCAL_GPORT_FTE_END(unit)) {

            l2addr->port = VLAN_FT_INDEX_TO_VGPORT_ID(unit, mac_data.pid);
            BCM_GPORT_VLAN_PORT_ID_SET(l2addr->port, l2addr->port);

            rv = bcm_vswitch_port_get(unit, l2addr->port, &vsi);
            if (BCM_FAILURE(rv)) {
                L2_ERR((_SBX_D(unit, "Failed to get vsi for gport 0x%x: %s\n"),
                        l2addr->port, bcm_errmsg(rv)));
                return rv;
            }

            if (vsi != vid) {
                /* no match */
                l2addr->port = 0;
            }
        }

        if(!l2addr->port) {
            /* Mcast */
            BCM_IF_ERROR_RETURN(_bcm_fe2000_l2_mcast_get(unit, l2addr));
        }
    }

    /* Drop */
    if (mac_data.sdrop) {
        l2addr->flags |= BCM_L2_DISCARD_SRC;
    }
    if (mac_data.ddrop) {
        l2addr->flags |= BCM_L2_DISCARD_DST;
    }

    /* Static */
    if(mac_data.dontage) {
        l2addr->flags |= BCM_L2_STATIC;
    }

    return rv;
}


int
_bcm_fe2000_g2p3_l2_flush_cache(int unit) 
{
    return soc_sbx_g2p3_cmac_commit(unit, SB_COMMIT_COMPLETE);
}


int
_bcm_fe2000_g2p3_l2_addr_update_dest(int unit, bcm_l2_addr_t *l2addr, 
                                     int qidunion)
{
    int                   rv = BCM_E_NONE;
    uint32_t              fteIdx;
    soc_sbx_g2p3_cmac_t    macData;
    soc_sbx_g2p3_ft_t     sbxFte;

    /* Special handling on a mcast entry */
    if (l2addr->flags & BCM_L2_MCAST) {
        return BCM_E_PARAM;        
    }
        
    /* If LOCAL_CPU, get local CPU port and modid */
    if (l2addr->flags & BCM_L2_LOCAL_CPU) {
        return BCM_E_PARAM;
    }

    rv = soc_sbx_g2p3_cmac_get(unit, l2addr->mac, l2addr->vid, &macData);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "failed to read MAC Payload: %d %s\n"),
                rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Read the FTE */
    fteIdx = macData.pid;
    rv = soc_sbx_g2p3_ft_get(unit, fteIdx, &sbxFte);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Error getting FTE: fte_idx=0x%x rv=%d %s\n"),
                fteIdx, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    /* 
     * will allocate FTE. It will be up to the l2 management code
     *  to de-allocate FTEs which are in the dynamic range 
     * HACK: allocate from local gports for now..fits in pid 
     * space (14b) in mac payload
     */
    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_FTE_LOCAL_GPORT, 
                                 1, &fteIdx, 0);
    if (BCM_FAILURE(rv)) { 
        L2_ERR((_SBX_D(unit, "failed to allocate an FTE: %d %s\n"),
                rv, bcm_errmsg(rv)));
        return rv;
    }

    /* change only the qidunion */
    sbxFte.qid = qidunion;
    
    rv = soc_sbx_g2p3_ft_set(unit, fteIdx, &sbxFte);
    if (BCM_FAILURE(rv)) {
        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_FTE_LOCAL_GPORT, 1,
                               &fteIdx, 0);
        L2_ERR((_SBX_D(unit, "Error setting FTE: fte_idx=0x%x rv=%d\n"),
                fteIdx, rv));
        return rv;
    }
    
    /* update FTE on dmacyload */
    macData.pid = fteIdx;

    /* only update the DMAC, and only do so if it exists already */
    /* Add Dmac entry */
    rv = soc_sbx_g2p3_cmac_update(unit, l2addr->mac, l2addr->vid, &macData);
    if (BCM_FAILURE(rv)) {
        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_FTE_LOCAL_GPORT, 1,
                               &fteIdx, 0);
        L2_ERR((_SBX_D(unit, "failed to update L2 DMAC entry rv=%d (%s)\n"),
                rv, bcm_errmsg(rv)));
        return rv;
    }
    
    /* Entries were successfully added, commit transaction if caching not enabled*/
    if (SOC_SBX_STATE(unit)->cache_l2 == FALSE) {
        rv = soc_sbx_g2p3_cmac_commit(unit, SB_COMMIT_COMPLETE);
        if (BCM_FAILURE(rv)) {
            _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_FTE_LOCAL_GPORT, 1, 
                                   &fteIdx, 0);
            L2_ERR((_SBX_D(unit, "failed to commit L2 add rv=%d(%s)\n"),
                    rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_l2_mac_size_get
 * Purpose:
 *     Get the L2 MAC table size.
 * Parameters:
 *     unit       - Device number
 *     table_size - Returns the L2 MAC table size.
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid params.
 */
int
_bcm_fe2000_g2p3_l2_mac_size_get(int unit, int *table_size)
{
    soc_sbx_g2p3_table_bank_params_t tbparams;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_mac_bank_params_get(unit, 1, &tbparams));

    *table_size = tbparams.size;
    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_l2_age_ager_set
 * Purpose:
 *     Set the L2 ager timestamp (age).
 * Parameters:
 *     unit - Device number
 *     ager - Age ager to set, 0..15
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit/ager values.
 */
int
_bcm_fe2000_g2p3_l2_age_ager_set(int unit, uint32 ager)
{
    return soc_sbx_g2p3_age_set(unit, ager);
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_l2_age_remove
 * Purpose:
 *     Remove given MAC entry from Smac and Dmac tables, but
 *     but it does perform a commit, which is expected to be done
 *     in the l2 age thread when the aging cycle is complete.
 * Parameters:
 *     unit       - Device number
 *     mackey     - packed MAC table key
 *     payload    - If non-null, returns payload for given MAC entry.
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
_bcm_fe2000_g2p3_l2_age_remove(int unit, int *mackey, bcm_l2_addr_t *payload)
{
    sal_mac_addr_t         mac;
    int                    vlan;
    int                    bmac;
    int                    rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_mac_index_unpack(unit, mackey, mac, 
                                                      &vlan, &bmac));
    if (bmac) {
        rv = BCM_E_NOT_FOUND;
    } else {
        /* Get MAC payload data into l2_addr before entry is removed */
        if (payload) {
            _bcm_fe2000_g2p3_l2_addr_get(unit, mac, vlan, payload);
        }
        
        /*
         * Remove entry from MAC table.
         * Do not commit here.  Commit will be done by L2 age timer thread.
         */
        rv = soc_sbx_g2p3_cmac_remove(unit, mac, vlan);
        
        L2_VERB((_SBX_D(unit, "Remove L2 aged entry MAC=" L2_6B_MAC_FMT 
                        "  VLAN=%d: %s\n"),
                 L2_6B_MAC_PFMT(mac), vlan, bcm_errmsg(rv)));
    }

    return rv;
}


int
_bcm_fe2000_g2p3_l2_egress_dest_port_process(int unit, bcm_gport_t dest_port,
                                             int pri_remark, 
                                             soc_sbx_g2p3_eteencap_t *eteencap)
{
    int                 rv = BCM_E_NONE;
    bcm_module_t        my_modid, modid; 
    int                 port, use_pid;
    bcm_vlan_t          vlan, vsi;
    uint32_t            lpi, pid, egr_remark_idx;
    bcm_mpls_port_t     mpls_port;
    soc_sbx_g2p3_lp_t   lp;
    bcm_trunk_t         trunkId;
    uint32              trunkFte = ~0;
    int                 trunk_rv = BCM_E_NONE;
    uint8_t             vpwsuni=0;

    if (!eteencap) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &my_modid));

    modid = port = -1;
    pid = ~0;
    use_pid = 0;
    if (BCM_GPORT_IS_MODPORT(dest_port)) {
        modid = BCM_GPORT_MODPORT_MODID_GET(dest_port);
        port = BCM_GPORT_MODPORT_PORT_GET(dest_port);
    } else if (BCM_GPORT_IS_MPLS_PORT(dest_port)) {
        bcm_mpls_port_t_init(&mpls_port);
        rv = _bcm_fe2000_mpls_port_vlan_vector_internal(unit, dest_port, 
                                                        &port, &vlan, &vsi, 
                                                        &lpi, &mpls_port, 
                                                        &vpwsuni);
        if (rv == BCM_E_NONE) {
            modid = my_modid;

            if (vpwsuni) {
                lpi += _BCM_FE2000_VPWS_UNI_OFFSET;
            }

            soc_sbx_g2p3_lp_t_init(&lp);
            rv = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
        }
        if (rv == BCM_E_NONE) {
            use_pid = 1;
            pid = lp.pid;
        }
    } else if (BCM_GPORT_IS_VLAN_PORT(dest_port)) {
        rv = _bcm_fe2000_map_vlan_gport_target(unit, dest_port, &my_modid, 
                                               &modid, &port, &pid, NULL);
        if (rv == BCM_E_NONE) {
            use_pid = 1;
        }
        /* check for trunk membership.  If this vlan port exists on a trunk
         * a different PID/color is used to avoid reflection on the trunk.
         */
        trunk_rv = bcm_trunk_find(unit, my_modid, port, &trunkId);
        if (trunk_rv == BCM_E_NONE) {
           trunkFte = SOC_SBX_TRUNK_FTE(unit, trunkId);
        }
        if (trunkFte != ~0) {
            pid = trunkFte;
        }
    } else {
        L2_ERR((_SBX_D(unit, "Un-supported gport (0x%x) specified. MODPORT, "
                       " MPLS & VLAN gports are supported.\n"), 
                dest_port));
        rv = BCM_E_PARAM;
    }

    if (rv == BCM_E_NONE) {
        if (my_modid != modid) {
            L2_ERR((_SBX_D(unit, "Destination port (0x%x) is not on local unit.\n"), 
                    dest_port));
            return BCM_E_PARAM;
        }
        /* program the eteencap. NOTE: not written to HW at this stage. */
        if (pri_remark) {
            rv = _bcm_fe2000_port_egr_remark_idx_get(unit, port, 
                                                     &egr_remark_idx);
            if (rv == BCM_E_NONE) {
                eteencap->remark = egr_remark_idx;
            }
        }
        if (use_pid) {
            eteencap->etepid = 1;
            eteencap->pid = pid;
        }
    }

    return rv;
}


int
_bcm_fe2000_g2p3_l2_egress_hw_entry_add(int unit, bcm_if_t encap_id, 
                                        bcm_l2_egress_t *egr)
{
    int                         rv = BCM_E_NONE;
    uint32                      ohi_idx, encap_ete_idx, ete_idx;
    soc_sbx_g2p3_oi2e_t         sbx_ohi;
    soc_sbx_g2p3_eteencap_t     sbx_ete_encap;
    soc_sbx_g2p3_etel2_t        sbx_ete_l2;
    soc_sbx_g2p3_tpid_t         tpidstr;

    encap_ete_idx = ~0;
    ete_idx = ~0;
    ohi_idx = SOC_SBX_OHI_FROM_L2_ENCAP_ID(encap_id);

    /* get an Encap ETE */
    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_ENCAP, 1, 
                                 &encap_ete_idx, 0);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    /* get an L2 ETE */
    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_L2,1, &ete_idx, 0);
    if (rv != BCM_E_NONE) {
        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_ENCAP, 1, 
                               &encap_ete_idx, 0);
        return rv;
    }

    /* initialize the OHI */
    soc_sbx_g2p3_oi2e_t_init(&sbx_ohi);
    sbx_ohi.eteptr  = encap_ete_idx;

    /* initialize the encap ete */
    soc_sbx_g2p3_eteencap_t_init(&sbx_ete_encap);
    sbx_ete_encap.l2ete = ete_idx;
    sbx_ete_encap.dmacset = 0;
    sbx_ete_encap.dmacsetlsb = 0;
    sbx_ete_encap.nosplitcheck = 0;
    sbx_ete_encap.ipttldec = 0;
    sbx_ete_encap.ttlcheck = 0;
    sbx_ete_encap.smacset = 0;
    sbx_ete_encap.etype = 0;

    if (egr->flags & BCM_L2_EGRESS_DEST_MAC_PREFIX5_REPLACE) {
        sbx_ete_encap.dmacset = 1;
    }
    if (egr->flags & BCM_L2_EGRESS_DEST_MAC_REPLACE) {
        sbx_ete_encap.dmacset = 1;
        sbx_ete_encap.dmacsetlsb = 1;
    }

    if (sbx_ete_encap.dmacset) {
        sbx_ete_encap.dmac0 = egr->dest_mac[0];
        sbx_ete_encap.dmac1 = egr->dest_mac[1];
        sbx_ete_encap.dmac2 = egr->dest_mac[2];
        sbx_ete_encap.dmac3 = egr->dest_mac[3];
        sbx_ete_encap.dmac4 = egr->dest_mac[4];
        if (sbx_ete_encap.dmacsetlsb) {
            sbx_ete_encap.dmac5 = egr->dest_mac[5];
        } else {
            /* dmac5 is not really used */
            sbx_ete_encap.dmac5 = 0xff;
        }
    }
    
    if (egr->flags & BCM_L2_EGRESS_SRC_MAC_REPLACE) {
        /* TODO: add support to replace src mac */
        rv = BCM_E_UNAVAIL;
    }

    if (egr->flags & BCM_L2_EGRESS_ETHERTYPE_REPLACE) {
        sbx_ete_encap.etype = egr->ethertype;
        sbx_ete_encap.encaplen = 2;
    }

    /* initialize the L2 ETE */
    soc_sbx_g2p3_etel2_t_init(&sbx_ete_l2);
    sbx_ete_l2.mtu = SBX_DEFAULT_MTU_SIZE;
    
    /* default to untagged */
    sbx_ete_l2.usevid       = 1;
    sbx_ete_l2.vid          = _BCM_VLAN_G2P3_UNTAGGED_VID;

    if (egr->flags & BCM_L2_EGRESS_INNER_VLAN_REPLACE) {
        rv = soc_sbx_g2p3_tpid_get(unit, SB_G2P3_FE_CTPID_INDEX, &tpidstr);
        sbx_ete_encap.encaplen = 4;
        sbx_ete_encap.tpid = tpidstr.tpid;
        sbx_ete_encap.vid = egr->inner_vlan & 0xfff;
        if (egr->flags & BCM_L2_EGRESS_INNER_PRIO_REPLACE) {
            /* pricfi comes directly from the ETE */
            sbx_ete_encap.pricfi = ((egr->inner_vlan & 0xf000) >> 12);
        }
    }

    if ((rv == BCM_E_NONE) && (egr->flags & BCM_L2_EGRESS_OUTER_VLAN_REPLACE)){
        sbx_ete_l2.usevid       = 1;
        sbx_ete_l2.vid          = egr->outer_vlan & 0xfff;
        sbx_ete_encap.remark    = 0; /* default remarking table */
        /* - pricfi comes from sbx_ete_encap.remark...
           - tpid comes from ep2e table...based on egress port
        */
    }

    if ((rv == BCM_E_NONE) && (egr->flags & BCM_L2_EGRESS_DEST_PORT)) {
        /* set the remark index and pid */
        rv = _bcm_fe2000_g2p3_l2_egress_dest_port_process(unit, egr->dest_port,
                                                          sbx_ete_l2.usevid, 
                                                          &sbx_ete_encap);
    }

    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g2p3_eteencap_set(unit, encap_ete_idx, &sbx_ete_encap);
    }
    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g2p3_etel2_set(unit, ete_idx, &sbx_ete_l2);
    }
    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g2p3_oi2e_set(unit, ohi_idx - SBX_RAW_OHI_BASE, &sbx_ohi);
    }

    if (rv == BCM_E_NONE) {
        egr->encap_id = encap_id;
    } else {
        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_ENCAP, 1, 
                               &encap_ete_idx, 0);
        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_L2, 1, &ete_idx, 0);
    }

    return rv;
}

int
_bcm_fe2000_g2p3_l2_egress_hw_entry_get(int unit, bcm_if_t encap_id,
                                        bcm_l2_egress_t *egr)
{
    int                         rv = BCM_E_NONE;
    uint32                      ohi_idx, encap_ete_idx, ete_idx;
    soc_sbx_g2p3_oi2e_t         sbx_ohi;
    soc_sbx_g2p3_eteencap_t     sbx_ete_encap;
    soc_sbx_g2p3_etel2_t        sbx_ete_l2;
    
    PARAM_NULL_CHECK(egr);
    ohi_idx = SOC_SBX_OHI_FROM_L2_ENCAP_ID(encap_id);

    soc_sbx_g2p3_oi2e_t_init(&sbx_ohi);
    rv = soc_sbx_g2p3_oi2e_get(unit, ohi_idx - SBX_RAW_OHI_BASE, &sbx_ohi);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    encap_ete_idx = sbx_ohi.eteptr;
    soc_sbx_g2p3_eteencap_t_init(&sbx_ete_encap);
    rv = soc_sbx_g2p3_eteencap_get(unit, encap_ete_idx, &sbx_ete_encap);
    if (rv != BCM_E_NONE) {
        return rv;
    }
    
    ete_idx = sbx_ete_encap.l2ete;
    soc_sbx_g2p3_etel2_t_init(&sbx_ete_l2);
    rv = soc_sbx_g2p3_etel2_get(unit, ete_idx, &sbx_ete_l2);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    sal_memset(egr, 0, sizeof(bcm_l2_egress_t));

    if ((sbx_ete_l2.usevid) && (sbx_ete_l2.vid != _BCM_VLAN_G2P3_UNTAGGED_VID)) {
        egr->outer_vlan = sbx_ete_l2.vid;
        egr->flags |= BCM_L2_EGRESS_OUTER_VLAN_REPLACE;
        
        egr->flags |= BCM_L2_EGRESS_OUTER_PRIO_REPLACE;
    }
    if (sbx_ete_encap.encaplen == 4) {
        egr->inner_vlan = (sbx_ete_encap.vid & 0xfff);
        egr->flags |= BCM_L2_EGRESS_INNER_VLAN_REPLACE;
        if (sbx_ete_encap.pricfi) {
            
            egr->flags |= BCM_L2_EGRESS_INNER_PRIO_REPLACE;
            egr->inner_vlan |= ((sbx_ete_encap.pricfi & 0xf) << 12);
        }
    }

    if (sbx_ete_encap.encaplen == 2) {
        egr->ethertype = sbx_ete_encap.etype;
        egr->flags |= BCM_L2_EGRESS_ETHERTYPE_REPLACE;
    }

    if (sbx_ete_encap.dmacset) {
        egr->dest_mac[0] = sbx_ete_encap.dmac0;
        egr->dest_mac[1] = sbx_ete_encap.dmac1;
        egr->dest_mac[2] = sbx_ete_encap.dmac2;
        egr->dest_mac[3] = sbx_ete_encap.dmac3;
        egr->dest_mac[4] = sbx_ete_encap.dmac4;
        if (sbx_ete_encap.dmacsetlsb) {
           egr->flags |= BCM_L2_EGRESS_DEST_MAC_REPLACE;
           egr->dest_mac[5] = sbx_ete_encap.dmac5;
        } else {
           egr->flags |= BCM_L2_EGRESS_DEST_MAC_PREFIX5_REPLACE;
           egr->dest_mac[5] = 0xff;
        }
    }

    egr->encap_id = encap_id;

    return rv;
}


#endif /* BCM_FE2000_P3_SUPPORT */
