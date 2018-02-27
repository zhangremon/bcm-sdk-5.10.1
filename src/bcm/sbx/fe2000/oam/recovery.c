/*
 * $Id: recovery.c 1.12 Broadcom SDK $
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
 * OAM Warm boot module
 */

#if defined(INCLUDE_L3)

#ifdef BCM_WARM_BOOT_SUPPORT

#include <bcm_int/sbx/fe2000/oam/oam.h>
#include <bcm_int/sbx/fe2000/g2p3.h>

extern oam_state_t* _state[SOC_MAX_NUM_DEVICES];

int
_oam_wb_maidmep_scan(int unit)
{
    int rv;
    fe2k_oam_wb_mem_layout_t    *layout;
    oam_sw_hash_data_t          *local_hash_data, *remote_hash_data;
    soc_sbx_g2p3_oammaidmep2e_t  mamep;
    int                          maid, mep;

    rv = _oam_wb_layout_get(unit, &layout);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "failed to get wb layout: %s\n"), 
                 bcm_errmsg(rv)));
        return rv;
    }

    soc_scache_handle_lock(unit, _state[unit]->wb_hdl);

    /* scanning the maidmep2e table will produce all remote endpoints */
    rv = soc_sbx_g2p3_oammaidmep2e_first(unit, &maid, &mep);
    while (BCM_SUCCESS(rv)) {
        OAM_VERB((_SBX_D(unit, "Processing maid=0x%x mep=0x%x\n"), maid, mep));

        rv = soc_sbx_g2p3_oammaidmep2e_get(unit, maid, mep, &mamep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "failed to read oammaidmep[0x%x, 0x%x]:"
                            "%s\n"), maid, mep, bcm_errmsg(rv)));
            break;
        }

        /* maid is the Local EP; endpointPtr is the the Remote EP */
        if (mamep.epValid) {

            remote_hash_data =
                &_state[unit]->hash_data_store[mamep.endpointPtr];
            local_hash_data = &_state[unit]->hash_data_store[maid];
            
            remote_hash_data->ep_name     = mep;
            remote_hash_data->maid_cookie = maid;
            remote_hash_data->mdlevel     = local_hash_data->mdlevel;
            remote_hash_data->vid_label   = local_hash_data->vid_label;

        }

        rv = soc_sbx_g2p3_oammaidmep2e_next(unit, maid, mep, &maid, &mep);
    }

    /* Clear out the end-of-loop error; it's not an error */
    if (rv == BCM_E_NOT_FOUND) {
        rv = BCM_E_NONE;
    }

    soc_scache_handle_unlock(unit, _state[unit]->wb_hdl);

    return rv;
}


int 
_oam_wb_recover_ep(int unit, int group, int lsmi, int ep_idx)
{
    int                       rv, update_ep;
    oam_sw_hash_data_t       *hash_data = NULL;
    bcm_oam_group_info_t     *group_info = NULL;
    soc_sbx_g2p3_oamep_t      oam_ep, extended_oam_ep;
    soc_sbx_g2p3_lsmac_t      lsm;
    soc_sbx_g2p3_timer_t      timer_cfg;
    egr_path_desc_t           egr_path;

    update_ep = FALSE;

    rv = shr_idxres_list_reserve(_state[unit]->group_pool, group, group);

    if (BCM_SUCCESS(rv)) {
        OAM_VERB((_SBX_D(unit, "Recovering OAM Group %d...\n"), group));
        OAM_GROUP_INFO(unit, group) = 
            sal_alloc(sizeof(*OAM_GROUP_INFO(unit, group)), "oam group info");

        if (OAM_GROUP_INFO(unit, group) == NULL){
            OAM_ERR((_SBX_D(unit, "Failed to allocate group_info state\n")));
            return BCM_E_MEMORY;
        }

        group_info = OAM_GROUP_INFO(unit, group);
        sal_memset(group_info, 0, sizeof(*group_info));

        group_info->id = group;
        DQ_INIT(&OAM_GROUP_EP_LIST(unit, group));
    }

    if (rv == BCM_E_RESOURCE) {
        /* First EP for this group already created, no error */
        group_info = OAM_GROUP_INFO(unit, group);
        rv         = BCM_E_NONE;
    }
    if (BCM_FAILURE(rv) || (group_info == NULL)) {
        OAM_ERR((_SBX_D(unit, "Failed to reserve group %d: %s\n"),
                 group, bcm_errmsg(rv)));
        return rv;
    }

    /* Recover the Endpoint */
    OAM_VERB((_SBX_D(unit, "Recovering Endpoint %d...\n"), ep_idx));
    rv = shr_idxres_list_reserve(_state[unit]->ep_pool, ep_idx, ep_idx);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to reserve endpoint %d: %s\n"),
                 ep_idx, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g2p3_oamep_get(unit, ep_idx, &oam_ep);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to read oamep[0x%x]: %s\n"),
                 ep_idx, bcm_errmsg(rv)));
        return rv;
    }

    hash_data = &_state[unit]->hash_data_store[ep_idx];

    hash_data->rec_idx = ep_idx;
    hash_data->group   = group;
    hash_data->lsm_idx = lsmi;

    /* Get the local station mac, if present */
    if (hash_data->lsm_idx) {

        rv = soc_sbx_g2p3_lsmac_get(unit, hash_data->lsm_idx, &lsm);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to read lsmac[0x%x]: %s\n"),
                     hash_data->lsm_idx, bcm_errmsg(rv)));
            return rv;   
        }
        sal_memcpy(hash_data->mac_address, lsm.mac, 
                   sizeof(hash_data->mac_address));
    }

    if (oam_ep.nextentry) {
        rv = soc_sbx_g2p3_oamep_get(unit, oam_ep.nextentry, &extended_oam_ep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to read next entry [0x%x]:%s\n"),
                     oam_ep.nextentry, bcm_errmsg(rv)));
            return rv;
        }

        rv = shr_idxres_list_reserve(_state[unit]->ep_pool, 
                                     oam_ep.nextentry, oam_ep.nextentry);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to reserve extented ep 0x%x: %s\n"),
                     oam_ep.nextentry, bcm_errmsg(rv)));
            return rv;
        }
    }

    if (oam_ep.function == _state[unit]->ep_type_ids.bfd) {
        hash_data->oam_type = oamEp;
    } else if (oam_ep.function == _state[unit]->ep_type_ids.lm) {
        hash_data->oam_type = oamLoss;
    } else if (oam_ep.function == _state[unit]->ep_type_ids.dm) {
        hash_data->oam_type = oamDelay;
    } else if (oam_ep.function == _state[unit]->ep_type_ids.ccm_first) {
        /* ccm_firsts are by definition Local endpoints */
        hash_data->oam_type    = oamEp;
        hash_data->mdlevel     = oam_ep.mdlevel;
        hash_data->ep_name     = oam_ep.mepid;
        hash_data->maid_cookie = BCM_OAM_ENDPOINT_INVALID;

        rv = _oam_egr_path_get(unit, &egr_path, ep_idx);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get egress path for "
                            "oamep[0x%x]: %s"), ep_idx, bcm_errmsg(rv)));
            return rv;
        }
        
        hash_data->vid_label = (egr_path.eteL2.usevid ?
                                egr_path.eteL2.vid : 0);

        ZF_TO_MAID_ID(oam_ep.maidw0, &group_info->name[0]);
        ZF_TO_MAID_ID(oam_ep.maidw1, &group_info->name[4]);
        ZF_TO_MAID_ID(oam_ep.maidw2, &group_info->name[8]);
        ZF_TO_MAID_ID(oam_ep.maidw3, &group_info->name[12]);
        ZF_TO_MAID_ID(oam_ep.maidw4, &group_info->name[16]);
        ZF_TO_MAID_ID(oam_ep.maidw5, &group_info->name[20]);

        if (oam_ep.nextentry) {
            ZF_TO_MAID_ID(extended_oam_ep.maidw6, &group_info->name[24]);
            ZF_TO_MAID_ID(extended_oam_ep.maidw7, &group_info->name[28]);
            ZF_TO_MAID_ID(extended_oam_ep.maidw8, &group_info->name[32]);
            ZF_TO_MAID_ID(extended_oam_ep.maidw9, &group_info->name[36]);
            ZF_TO_MAID_ID(extended_oam_ep.maidw10, &group_info->name[40]);
            ZF_TO_MAID_ID(extended_oam_ep.maidw11, &group_info->name[44]);
        }

        if (oam_ep.mip) {
            hash_data->flags |= BCM_OAM_ENDPOINT_INTERMEDIATE;
        }

        /* upMEPs use the sid for the port; and mod is _this_ unit
         * downMEPs use stuff the qid for now, during post-init recovery
         *   the mod map is queried to resolve the node/port
         */
        if (oam_ep.dir == OAM_DIR_UP) {
            hash_data->flags |= BCM_OAM_ENDPOINT_UP_FACING;
            if (oam_ep.nextentry) {
                hash_data->gport = extended_oam_ep.sid;
            }
        } else {
            hash_data->gport = egr_path.fte.qid;
        }

    } else if (oam_ep.function == _state[unit]->ep_type_ids.ccm_second) {
        OAM_ERR((_SBX_D(unit,"Extended local endpoint found as root node\n")));
        rv = BCM_E_INTERNAL;

    } else if (oam_ep.function == _state[unit]->ep_type_ids.ccm_peer) {

        /* ccm_peers are by definition REMOTE endpoints. */
        hash_data->oam_type = oamEp;
        hash_data->flags   |= BCM_OAM_ENDPOINT_REMOTE;

        /* Never recover watchdog timers on warmboot */
        if (oam_ep.policerid != INVALID_POLICER_ID) {

            OAM_VERB((_SBX_D(unit, "Clearing timer 0x%04x\n"), 
                      oam_ep.policerid));

            /* clear out the watchdog timer so no interrupts occur */
            soc_sbx_g2p3_timer_t_init(&timer_cfg);

            SOC_WARM_BOOT_DONE(unit);
            rv = soc_sbx_g2p3_oamtimer_set(unit, oam_ep.policerid, &timer_cfg);
            SOC_WARM_BOOT_START(unit);

            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit,"Failed to clear timer calendar for 0x%x "
                                ": %s (ignoring)\n"), 
                         oam_ep.policerid, bcm_errmsg(rv)));
                rv = BCM_E_NONE;
            }

            oam_ep.policerid = INVALID_POLICER_ID;
            update_ep = TRUE;

        }

    } else if (oam_ep.function == _state[unit]->ep_type_ids.psc) {

        hash_data->oam_type = oamEp;
    } else {

        OAM_ERR((_SBX_D(unit, "Unrecognized ep type: %d\n"), oam_ep.function));
        rv = BCM_E_INTERNAL;
    }

    if (BCM_FAILURE(rv)) {
        return rv;
    }

    /* add the Endpoint to the list for this group */
    DQ_INSERT_HEAD(&OAM_GROUP_EP_LIST(unit, group), &hash_data->list_node);

    if (update_ep) {
        SOC_WARM_BOOT_DONE(unit);
        rv = soc_sbx_g2p3_oamep_set(unit, ep_idx, &oam_ep);
        SOC_WARM_BOOT_START(unit);
    }

    return rv;
}


int
_oam_wb_up_mep_recover(int unit)
{
    int                       rv = BCM_E_NONE;
    int                       port, size, alloc_flags, dmac_type;
    uint32                    etei, fti;
    soc_sbx_g2p3_p2fti_t      p2fti;
    soc_sbx_g2p3_ft_t         fte;
    soc_sbx_g2p3_eteencap_t   eteencap;
    soc_sbx_g2p3_etel2_t      etel2;
    soc_sbx_g2p3_oi2e_t       oi2e;

    /* reserve the per-port forwarding entries */
    size = soc_sbx_g2p3_p2fti_table_size_get(unit);
    alloc_flags = _SBX_GU2_RES_FLAGS_RESERVE;

    for (dmac_type=0; dmac_type < 2; dmac_type++) {

        /* initialize FTE invalid */
        fti = SBX_DROP_FTE(unit);

        for (port=0; port < (size / 2); port++) {
            rv = soc_sbx_g2p3_p2fti_get(unit, port, dmac_type, &p2fti);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to get p2fti[0x%x][%d]: %s\n"),
                         port, dmac_type, bcm_errmsg(rv)));
                return rv;
            }

            if (p2fti.ftidx == SBX_DROP_FTE(unit)) {
                continue;
            }
            fti = p2fti.ftidx;

            _state[unit]->up_init = 1;
            rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_FTE_UNICAST, 1,
                                         &p2fti.ftidx, alloc_flags);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to reserve ft[0x%08x]: %s\n"),
                         p2fti.ftidx, bcm_errmsg(rv)));
                return rv;
            }

        }

        if (fti == SBX_DROP_FTE(unit)) {
            OAM_VERB((_SBX_D(unit, "No UP meps configured; skipping this "
                             "phase of recovery")));
            return BCM_E_NONE;
        }

        /* All the FTEs of a dmac type share the same OI, read the last known
         * FTE and reserve it's egress path
         */
        rv = bcm_fe2k_egr_path_get(unit, fti, 
                                   &fte, &oi2e, &eteencap, &etel2, &etei);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get egress path: %s\n"), 
                     bcm_errmsg(rv)));
            return rv;
        }

        if (dmac_type == 0) {
            _state[unit]->eteencap_lsm = oi2e.eteptr;
        }

        rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_OHI, 1,
                                     &fte.oi, alloc_flags);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to reserve oi2e[0x%08x]: %s\n"),
                     fte.oi, bcm_errmsg(rv)));
            return rv;
        }

        rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_ENCAP, 1,
                                     &oi2e.eteptr, alloc_flags);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to reserve eteencap[0x%08x]: %s\n"),
                     oi2e.eteptr, bcm_errmsg(rv)));
            return rv;
        }

        rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_L2, 1,
                                     &eteencap.l2ete, alloc_flags);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to reserve ete[0x%08x]: %s\n"),
                     eteencap.l2ete, bcm_errmsg(rv)));
            return rv;
        }
    }

    return rv;
}


int 
_oam_wb_recover(int unit, fe2k_oam_wb_mem_layout_t *layout)
{
    int                       ep_idx, rv = BCM_E_NONE;

    for (ep_idx=0; ep_idx < _state[unit]->max_endpoints; ep_idx++) {
        if (layout->ep_meta[ep_idx].group) {
            rv = _oam_wb_recover_ep(unit, 
                                    layout->ep_meta[ep_idx].group, 
                                    layout->ep_meta[ep_idx].lsmi,
                                    ep_idx);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed EP recovery\n")));
                return rv;
            }
        }
    }

    /* after all known endpoints have been recovered, scan other oam 
     * hardware tables for additional data
     */
    rv = _oam_wb_maidmep_scan(unit);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed maidmep table walk: %s\n"),
                 bcm_errmsg(rv)));
        return rv;
    }

    rv = _oam_wb_up_mep_recover(unit);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed recover up mep data: %s\n"), 
                 bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}

int
_oam_wb_post_init_recover(int unit)
{
    int                       rv, idx;
    int                       fab_port, fab_node, port;
    oam_sw_hash_data_t       *local_data, *peer_data;
    bcm_gport_t               fab_gport;
    bcm_module_t              my_mod;

    rv = bcm_stk_my_modid_get(unit, &my_mod);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get mod id: %s\n"),
                 bcm_errmsg(rv)));
        return rv;
    }

    for (idx=0; idx < _state[unit]->max_endpoints; idx++) {

        local_data = &_state[unit]->hash_data_store[idx];
        
        /* only local endpoints have qid information, the gport is
         * propagated to the peer/remote endpoint in a second pass
         * of the hash data
         */
        if (local_data->rec_idx && 
            ((local_data->flags & BCM_OAM_ENDPOINT_REMOTE) == 0)) {


            if (local_data->flags & BCM_OAM_ENDPOINT_UP_FACING) {
                OAM_VERB((_SBX_D(unit, "ep 0x%x decoding sid 0x%x\n"),
                          idx, local_data->gport));

                port = SOC_SBX_PORT_FROM_SID(unit, my_mod, local_data->gport);
                BCM_GPORT_MODPORT_SET(local_data->gport, my_mod, port);

            } else {
                OAM_VERB((_SBX_D(unit, "ep 0x%x decoding qid 0x%x\n"),
                          idx, local_data->gport));


                /* The pre-init phase of warm boot set the gport to the qid,
                 * convert it to a gport now that the mod map is established
                 */
                rv = map_qid_to_np(unit, local_data->gport /* qid */,
                                   &fab_node, &fab_port, NUM_COS(unit));
                fab_node += SBX_QE_BASE_MODID;
                
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed to map qid 0x%x: %s\n"),
                             local_data->gport));
                    return rv;
                }
                
                /* Convert qid node port to switch port */
                BCM_GPORT_MODPORT_SET(fab_gport, fab_node, fab_port);
                rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, fab_gport,
                                                            &local_data->gport);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed to get switch port "
                                    "from gport 0x%08x: %s\n"), 
                             fab_gport, bcm_errmsg(rv)));
                    return rv;
                }
            }
        }
    }

    /* Propagate the local->peer info */
    for (idx=0; idx < _state[unit]->max_endpoints; idx++) {

        peer_data = &_state[unit]->hash_data_store[idx];
        
        if (peer_data->rec_idx && 
            (peer_data->flags & BCM_OAM_ENDPOINT_REMOTE)) {
            local_data =
                &_state[unit]->hash_data_store[peer_data->maid_cookie];

            peer_data->gport  = local_data->gport;
            peer_data->flags |= 
                (local_data->flags & BCM_OAM_ENDPOINT_UP_FACING);
        }
    }


    return rv;
}


int
oam_ep_store(int unit, bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv = BCM_E_NONE;
    oam_sw_hash_data_t       *hash_data = NULL;
    fe2k_oam_wb_mem_layout_t *layout = NULL;


    hash_data = &_state[unit]->hash_data_store[endpoint_info->id];

    OAM_VERB((_SBX_D(unit, "storing ep=0x%04x, group=%4d\n"),
              endpoint_info->group, endpoint_info->id));
    
    if (endpoint_info->id >= OAM_RCVR_MAX_EP) {
        return BCM_E_CONFIG;
    }

    rv = _oam_wb_layout_get(unit, &layout);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "failed to get wb layout: %s\n"), 
                 bcm_errmsg(rv)));
        return rv;
    }
    soc_scache_handle_lock(unit, _state[unit]->wb_hdl);

    layout->ep_meta[endpoint_info->id].group = endpoint_info->group;

    if (!(endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {
        /* local endpoint */
        layout->ep_meta[endpoint_info->id].lsmi = hash_data->lsm_idx;
    }

    soc_scache_handle_unlock(unit, _state[unit]->wb_hdl);
    return rv;
}


int
_oam_wb_layout_get(int unit, fe2k_oam_wb_mem_layout_t **lay)
{
    int rv = BCM_E_NONE;
    uint32 size;
    soc_wb_cache_t *wbc;

    BCM_IF_ERROR_RETURN(
        soc_scache_ptr_get(unit, _state[unit]->wb_hdl, (uint8**)&wbc, &size));
    *lay = (fe2k_oam_wb_mem_layout_t*)wbc->cache;
    return rv;
}

#endif /* BCM_WARM_BOOT_SUPPORT */
#endif  /* INCLUDE_L3 */
