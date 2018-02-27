/*
 * $Id: psc.c 1.6.2.1 Broadcom SDK $
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
 * FE2000 OAM Protection State Coordination function
 */

#if defined(INCLUDE_L3)

#include <bcm_int/sbx/fe2000/oam/oam.h>
#include <bcm_int/sbx/fe2000/oam/mpls.h>
#include <bcm_int/sbx/fe2000/oam/psc.h>
#include <bcm_int/sbx/fe2000/oam/bfd.h>

extern oam_state_t *_state[SOC_MAX_NUM_DEVICES];

int _bcm_fe2000_validate_oam_lsp_psc_endpoint(int unit, 
                                              bcm_oam_endpoint_info_t *endpoint_info)
{
    int index = 0;
    int tid = 0;
    bcm_trunk_add_info_t trunk_info;
    int status = BCM_E_PARAM;

    if(!endpoint_info) {
        OAM_ERR((_SBX_D(unit, "Null endpoint info\n")));

    } else if (endpoint_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
        OAM_ERR((_SBX_D(unit, "Unsupported Endpoing Flags: INTERMEDIATE\n")));

    }  else if (endpoint_info->type & ~_BCM_FE2000_MPLS_PSC_SUPPORTED_EPTYPE) {
        OAM_ERR((_SBX_D(unit, "Unsupported Endpoint Type: %d\n"), endpoint_info->type));

    } else {
        status = BCM_E_NONE;
    }

    if(BCM_SUCCESS(status)) {
        if (endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE) {
            soc_sbx_g2p3_label2e_t  label2e;
    
            /* verify that if remote endpoint, associated mpls label is pop */
            if (!endpoint_info->mpls_label) {
                OAM_ERR((_SBX_D(unit, "Remote Endpoints should have an associated POP label.\n")));
                return BCM_E_PARAM;
            }
                
            status = soc_sbx_g2p3_label2e_get(unit, endpoint_info->mpls_label,
                                              &label2e);
            if ((BCM_FAILURE(status)) ||\
                (!_BCM_FE2K_IS_LABEL_LER(unit,label2e.opcode))) {
                OAM_ERR((_SBX_D(unit, "Remote Endpoints should have an associated POP label.\n")));
                return status;
            }
         
            /* verify if this local endpoint exist on the group */
            if (OAM_GROUP_EP_LIST_EMPTY(unit, endpoint_info->group)) {
                OAM_ERR((_SBX_D(unit, "Group has no Endpoints associated. "
                                "Local Endpoint must exist before creating"
                                " remote endpointID\n")));
                return BCM_E_PARAM;
            } else {
                dq_p_t ep_elem;
    
                DQ_TRAVERSE(&OAM_GROUP_EP_LIST(unit, endpoint_info->group), ep_elem) {
                    oam_sw_hash_data_t *hash_data = NULL;
                    _BCM_FE2000_OAM_GET_EP_DATA_FROM_LIST(ep_elem, hash_data);
                    if (!(hash_data->flags & BCM_OAM_ENDPOINT_REMOTE)) {
                       if(hash_data->rec_idx != endpoint_info->local_id) {
                          OAM_ERR((_SBX_D(unit, "Local Endpoint does not match with one on group\n")));
                          status = BCM_E_PARAM;
                          break;
                       }
                    }
                } DQ_TRAVERSE_END(&OAM_GROUP_EP_LIST(unit, endpoint_info->group), ep_elem);
            }
        } else {
           if (endpoint_info->intf_id) {
               status = _bcm_fe2000_get_tunnel_interface_info(unit, 
                                                   endpoint_info->intf_id,
                                                   _FE2K_OUTGOING_LSP);
           } else {
               OAM_ERR((_SBX_D(unit, "Invalid Egress Interface specified\n")));
               status = BCM_E_PARAM;
           }
           /* Validate trunk_index */
           if (BCM_SUCCESS(status) && \
               (OAM_MPLS_LSP_OUT_EGR_IF(unit).flags & BCM_L3_TGID)) {
                tid = OAM_MPLS_LSP_OUT_EGR_IF(unit).trunk;
                status = bcm_trunk_get(unit, tid, &trunk_info);
                if (status != BCM_E_NONE) {
                    OAM_ERR((_SBX_D(unit, "Could not retreive trunk info for trunkId %d\n"), tid));
                    status = BCM_E_PARAM;
                } else {
                    index = endpoint_info->trunk_index;
                    if ((index > trunk_info.num_ports) || (index < 0)) {
                        OAM_ERR((_SBX_D(unit, "Invalid trunk_index %d for trunk %d\n"), index, tid));
                        status = BCM_E_PARAM;
                    }
                }
            }
        }
    }

    
    
    return status;
}

STATIC 
int _bcm_fe2000_oam_psc_rxlsp_remote_oamep_associate(int unit,
                                                     uint32_t rx_lsp_label,
                                                     uint32_t local_ep_index,
                                                     uint8_t  add)
{
    int status = BCM_E_NONE;
    soc_sbx_g2p3_label2e_t label2e;
    soc_sbx_g2p3_lp_t      label_lp;
    soc_sbx_g2p3_label2e_t_init(&label2e);
    
    /* if remote endpoint, associate endpoint to Label2e LP */
    status = soc_sbx_g2p3_label2e_get(unit, rx_lsp_label, &label2e);
    if  (BCM_FAILURE(status)) {
        OAM_ERR((_SBX_D(unit, "Failed to get label2e 0x%x: %d %s\n"),
                 rx_lsp_label, status, bcm_errmsg(status)));
    } else {
        if (!label2e.lpidx) {
            status = BCM_E_INTERNAL;
            OAM_ERR((_SBX_D(unit, "Label2e for label 0x%x has not associate logical port\n"),rx_lsp_label));
        } else {
            status = soc_sbx_g2p3_lp_get(unit, label2e.lpidx, &label_lp);
            if  (BCM_FAILURE(status)) {
                OAM_ERR((_SBX_D(unit, "Failed to get lpi 0x%x: %d %s\n"),
                         label2e.lpidx, status, bcm_errmsg(status)));
            } else {
                if (label_lp.oamepi && \
                   (label_lp.oamepi != local_ep_index)) {
                    /* for now only bfd endpoint chaining is supported, defer the validation
                     * to bfd chain function */
                    status = _oam_fe2000_bfd_chain_psc(unit, label_lp.oamepi,
                                                       local_ep_index, 
                                                       (add)?1:0);
                    if  (BCM_FAILURE(status)) {
                        OAM_ERR((_SBX_D(unit, "Failed to chain to BFD endpoint: 0x%x %d %s\n"),
                                 label_lp.oamepi, status, bcm_errmsg(status)));
                    } 
                } else {
                    label_lp.oamepi = (add) ? local_ep_index:0;
                    status = soc_sbx_g2p3_lp_set(unit, label2e.lpidx, &label_lp);
                    if  (BCM_FAILURE(status)) {
                        OAM_ERR((_SBX_D(unit, "Failed to set lpi 0x%x: %d %s\n"),
                                 label2e.lpidx, status, bcm_errmsg(status)));
                    }    
                }
            }
        }
    }  
    return status;
}

int _oam_psc_endpoint_set(int unit,
                          bcm_oam_endpoint_info_t *ep_info,
                          uint32_t ep_rec_index, 
                          egr_path_desc_t *egrPath,
                          tcal_id_t *tcal_id,
                          uint32_t     rx_lsp_label)
{
    int status = BCM_E_NONE;
    int status2 = BCM_E_NONE;
    soc_sbx_g2p3_oamep_t              oamep;
    oam_sw_hash_data_t               *hash_data = NULL;
    uint32                            ep_type=0, lsmIdx=0;
    soc_sbx_g2p3_lsmac_t              lsm;

    if (!ep_info || !egrPath || !tcal_id) {
        status = BCM_E_PARAM;
    } else {
        hash_data = &_state[unit]->hash_data_store[ep_rec_index];
        soc_sbx_g2p3_oamep_t_init(&oamep);
        soc_sbx_g2p3_oam_type_mpls_lsp_get(unit, &ep_type);
        *tcal_id = _BCM_TCAL_INVALID_ID;

        /************* Local Endpoint ****************/
        if (!(ep_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {
            /* if needed, init endpoint & burn on hardware */
            oamep.function      = _state[unit]->ep_type_ids.psc;
            oamep.type          = ep_type;
            oamep.ftidx         = egrPath->ftIdx;

            if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
                lsmIdx = ~0;
                status = _sbx_gu2_ismac_idx_free(unit, hash_data->mac_address,
                                                 _SBX_GU2_RES_UNUSED_PORT, &lsmIdx);
                OAM_VERB((_SBX_D(unit, "freed mac " L2_6B_MAC_FMT " at %d on replace: "
                                 "%s\n"),
                          L2_6B_MAC_PFMT(hash_data->mac_address), lsmIdx, 
                          bcm_errmsg(status)));
                if (status == BCM_E_EMPTY) {
                    status = BCM_E_NONE;
                }
            } 

            
            soc_sbx_g2p3_lsmac_t_init (&lsm);
            status = _sbx_gu2_ismac_idx_alloc(unit, 0, ep_info->src_mac_address, 
                                              _SBX_GU2_RES_UNUSED_PORT, &lsmIdx);
            
            if (BCM_FAILURE(status)) {
                OAM_ERR((_SBX_D(unit, "Failed to allocate local station MAC idx\n")));
            } else {
                OAM_VERB((_SBX_D(unit, "Stored mac " L2_6B_MAC_FMT " at %d\n"),
                          L2_6B_MAC_PFMT(ep_info->src_mac_address), lsmIdx));
                
                hash_data->lsm_idx = lsmIdx;
                sal_memcpy (lsm.mac, ep_info->src_mac_address, sizeof(bcm_mac_t));
                lsm.useport = 0;

                status = soc_sbx_g2p3_lsmac_set (unit, lsmIdx, &lsm);
                if (BCM_FAILURE(status)) {
                    OAM_ERR((_SBX_D(unit, "Failed to write local station MAC\n")));
                }                  
            }

            if (BCM_SUCCESS(status)) { 
                if (!(ep_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {

                    status = soc_sbx_g2p3_oamep_set(unit, ep_rec_index, &oamep);  
                    if (BCM_FAILURE(status)) {
                        OAM_ERR((_SBX_D(unit, "Failed to write oamep 0x%x: %d %s\n"),
                                 ep_info->local_id, status, bcm_errmsg(status)));
                    } else {
                        status = _bcm_fe2000_lsp_intf_ep_list_add(unit, ep_info->intf_id, hash_data); 
                        if (BCM_FAILURE(status)) {
                            OAM_ERR((_SBX_D(unit, "Failed to insert oam into interface endpoint list: %d %s\n"),
                                     status, bcm_errmsg(status)));
                        } 
                    }
                } else {
                    /* if PSC moved from one interface to another, remove the old PSC pointer from
                     * interface list */
                    status = _bcm_fe2000_lsp_intf_ep_list_remove(unit, hash_data->intf_id, hash_data); 
                    if (BCM_FAILURE(status)) {
                        OAM_ERR((_SBX_D(unit, "Failed to remove oam into interface endpoint list: %d %s\n"),
                                 status, bcm_errmsg(status)));
                    } 
                }
            }

            /* Error Handling */
            if (BCM_FAILURE(status)) {
                if (!(ep_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {
                    soc_sbx_g2p3_oamep_t_init(&oamep);

                    /* really dont care what the result is.  This is already
                     * an error path.  But, dont over-write "status" as that
                     * contains the original failure, and will be returned to
                     * the caller.
                     */
                    status2 = soc_sbx_g2p3_oamep_set(unit, ep_rec_index, &oamep);  
                    if (BCM_FAILURE(status2)) {
                        OAM_ERR((_SBX_D(unit, "Failed to initialize oamep 0x%x: %d %s\n"),
                                 ep_rec_index, status2, bcm_errmsg(status2)));
                    }
                }

                _sbx_gu2_ismac_idx_free(unit, hash_data->mac_address,
                                        _SBX_GU2_RES_UNUSED_PORT, &lsmIdx);            
            }
        } else { /* end !remote endpoint */
            if ((ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) &&
                (hash_data->vid_label != rx_lsp_label)) {
                status = BCM_E_NONE;
                /* nothing to replace */
            } else {
                /* associate the oam endpoint with the RX LSP */
                status = _bcm_fe2000_oam_psc_rxlsp_remote_oamep_associate (unit, rx_lsp_label,
                                                                       ep_info->local_id, 1);
                if (BCM_SUCCESS(status) && 
                    (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {
                    /* if this is movement of PSC to a different tunnel */
                    if (hash_data->vid_label != rx_lsp_label) {
                        /* if replace, remove association with old LSP */
                        status = _bcm_fe2000_oam_psc_rxlsp_remote_oamep_associate (unit,
                                                                             hash_data->vid_label,
                                                                             ep_info->local_id, 0);
                    }
                }
            }
        } /* end remote endpoint */
    }
    return status;
}

int _oam_psc_endpoint_delete(int unit, 
                             bcm_oam_endpoint_info_t *ep_info)
{
    int status = BCM_E_NONE;
    oam_sw_hash_data_t *hash_data = NULL;
    uint32_t oamep_idx =0;

    /* sanity check pointers */
    if (ep_info && ENDPOINT_ID_VALID(unit, ep_info->id)) {
        
        hash_data = &_state[unit]->hash_data_store[ep_info->id];

        oamep_idx = (hash_data->flags & BCM_OAM_ENDPOINT_REMOTE)? \
                     ep_info->local_id:ep_info->id;

        if (hash_data->flags & BCM_OAM_ENDPOINT_REMOTE) {
            /* if replace, remove association with old LSP */
            status = _bcm_fe2000_oam_psc_rxlsp_remote_oamep_associate(unit,
                                                                      hash_data->vid_label,
                                                                      hash_data->local_rec_idx, 0); 
        } else {
            uint32 unknownIdx = ~0;

            /* remove the LSM entry */
            status = _sbx_gu2_ismac_idx_free(unit, ep_info->src_mac_address, 
                                             _SBX_GU2_RES_UNUSED_PORT, &unknownIdx);
            if (status == BCM_E_EMPTY) {
                soc_sbx_g2p3_lsmac_t lsm;
                soc_sbx_g2p3_lsmac_t_init (&lsm);
                
                OAM_VERB((_SBX_D(unit, "Found no more users of idx %d,"
                                 " clearing hw table\n"),
                          unknownIdx));

                status = soc_sbx_g2p3_lsmac_set (unit, unknownIdx, &lsm);
                if (BCM_FAILURE(status)) {
                    OAM_WARN((_SBX_D(unit, "Failed to delete local station MAC: %d %s\n"),
                              status, bcm_errmsg(status)));
                }
            }

            if (BCM_FAILURE(status)) {
                OAM_WARN((_SBX_D(unit, "Failed to delete local station MAC: %d %s\n"),
                          status, bcm_errmsg(status)));
            }

            /* Remove the local endpoint into L3 interface OAM endpoint list */
            status = _bcm_fe2000_lsp_intf_ep_list_remove(unit, ep_info->intf_id, hash_data); 
            if (BCM_FAILURE(status)) {
                OAM_ERR((_SBX_D(unit, "Failed to delete oam into interface endpoint list: %d %s\n"),
                         status, bcm_errmsg(status)));
            }
        }
    } else {
        status = BCM_E_PARAM;
    }
    return status;
}

int _oam_psc_endpoint_get(int unit,
                          bcm_oam_endpoint_info_t *ep_info, 
                          uint32_t ep_rec_index)
{
    int status = BCM_E_NONE;
    oam_sw_hash_data_t *hash_data = NULL;

    if (ep_info && ENDPOINT_ID_VALID(unit,ep_rec_index)) {

        hash_data = &_state[unit]->hash_data_store[ep_rec_index];
        ep_info->name       = hash_data->ep_name;
        ep_info->flags      = hash_data->flags;
        ep_info->type       = hash_data->type;
        ep_info->group      = hash_data->group;
        ep_info->gport      = hash_data->gport;
        ep_info->vlan       = hash_data->vid_label;
        ep_info->name       = hash_data->ep_name;
        ep_info->local_id   = hash_data->local_rec_idx;
        ep_info->mpls_label = hash_data->vid_label;
        ep_info->intf_id    = hash_data->intf_id;
        ep_info->trunk_index  = hash_data->trunk_index;

        if (!(hash_data->flags & BCM_OAM_ENDPOINT_REMOTE)) {
            if(hash_data->lsm_idx) {
                soc_sbx_g2p3_lsmac_t      lsm;
                soc_sbx_g2p3_lsmac_get(unit, hash_data->lsm_idx, &lsm);
                sal_memcpy(&ep_info->src_mac_address, lsm.mac, sizeof(bcm_mac_t));
            }

            sal_memcpy(ep_info->dst_mac_address, hash_data->mac_address,
                       sizeof(bcm_mac_t));
        }
    } else {
        status = BCM_E_PARAM;
    }

    return status;
}

int bcm_fe2000_oam_psc_add(int unit,
                           bcm_oam_psc_t *psc_info)
{
    int status = BCM_E_NONE;
    oam_sw_hash_data_t *local_hash_data = NULL;
    oam_sw_hash_data_t *remote_hash_data = NULL;
    soc_sbx_g2p3_oamep_t oamep;
    uint32_t             tcal_id=_BCM_TCAL_INVALID_ID;
  
    /* sanity check */
    /* validate input argumnets */
    if (!BCM_OAM_IS_INIT(unit)) {
        OAM_ERR((_SBX_D(unit, "Failed to initialize OAM Module\n")));
        return BCM_E_INIT;
    }

    if (psc_info->flags & ~_BCM_FE2000_MPLS_PSC_SUPPORTED_FLAGS ) {
        OAM_ERR((_SBX_D(unit, "Unsupported Endpoint Flag: 0x%x\n"), psc_info->flags));
        return BCM_E_PARAM;
    }

    if ((psc_info->flags & BCM_OAM_PSC_TX_BURST) && 
        (psc_info->burst_rate != BCM_OAM_ENDPOINT_CCM_PERIOD_3MS)) {
        OAM_ERR((_SBX_D(unit, "Unsupported Burst Rate: 0x%d\n"), psc_info->slow_rate));
        return BCM_E_PARAM;
    }

    if ((psc_info->flags & BCM_OAM_PSC_TX_SLOW) && 
         (psc_info->slow_rate < BCM_OAM_ENDPOINT_CCM_PERIOD_3MS ||
          psc_info->slow_rate > BCM_OAM_ENDPOINT_CCM_PERIOD_10M)) {
        OAM_ERR((_SBX_D(unit, "Unsupported Slow Rate: 0x%d\n"), psc_info->slow_rate));
        return BCM_E_PARAM;
    }
    
    if ((psc_info->flags & BCM_OAM_PSC_TX_BURST)  && \
        (psc_info->flags & BCM_OAM_PSC_TX_SLOW) && \
        (psc_info->slow_rate < psc_info->burst_rate)) {
        OAM_ERR((_SBX_D(unit, "Unsupported Slow Rate: 0x%d\n"), psc_info->slow_rate));
        return BCM_E_PARAM;
    }

    /* check endpoint & get endpoint */
    if (!ENDPOINT_ID_VALID(unit, psc_info->id)) {
        OAM_ERR((_SBX_D(unit, "Invalid Local Endpoint: 0x%x\n"), psc_info->id));
        return BCM_E_PARAM;
    }

    if (!ENDPOINT_ID_VALID(unit, psc_info->remote_id)) {
        OAM_ERR((_SBX_D(unit, "Invalid Remote Endpoint: 0x%x\n"), psc_info->id));
        return BCM_E_PARAM;
    }

    OAM_LOCK(unit);

    local_hash_data = &_state[unit]->hash_data_store[psc_info->id];
    remote_hash_data = &_state[unit]->hash_data_store[psc_info->remote_id];

    if (local_hash_data->type  != _BCM_FE2000_MPLS_PSC_SUPPORTED_EPTYPE &&
        remote_hash_data->type != _BCM_FE2000_MPLS_PSC_SUPPORTED_EPTYPE) {
        OAM_ERR((_SBX_D(unit, "Invalid PSC oam endpoint specified\n")));
        status = BCM_E_PARAM;
    } else {
        if (remote_hash_data->local_rec_idx != psc_info->id) {
            OAM_ERR((_SBX_D(unit, "Invalid PSC remote oam endpoint, mismatches with local endpoint\n")));
            status = BCM_E_PARAM;
        } else {
            /* Always consider as update */
            soc_sbx_g2p3_oamep_t_init(&oamep);
            status = soc_sbx_g2p3_oamep_get(unit, psc_info->id, &oamep);
            if(BCM_FAILURE(status)) {
                OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x: %d %s\n"), 
                         psc_info->id, status, bcm_errmsg(status)));
                
            } else {
                _oam_oamep_prepare(unit, &oamep);

                /* If timer is enabled allocate tcal */
                if ((psc_info->flags & _BCM_FE2000_MPLS_PSC_TX_ENABLE_FLAGS_) &&
                    (!(local_hash_data->psc_flags & _BCM_FE2000_MPLS_PSC_TX_ENABLE_FLAGS_))) {
                    status = _bcm_tcal_alloc(unit, 
                                             BCM_OAM_ENDPOINT_CCM_PERIOD_3MS,
                                             FALSE,
                                             local_hash_data->rec_idx, 
                                             &tcal_id);
                }
                
                if (BCM_SUCCESS(status)) {
                    /* Based on slow rate, burst setting adjust oam endpoint entry */
                    oamep.fpath     = psc_info->fpath;
                    oamep.path      = psc_info->path;
                    oamep.revertive = (psc_info->flags & BCM_OAM_PSC_REVERTIVE)?1:0;
                    oamep.version   = psc_info->version;
                    oamep.request   = psc_info->request;
                    oamep.pt        = psc_info->pt;

                    
                    /* default slow rate is 5 sec. */
                    oamep.maxslowrate = (psc_info->flags & BCM_OAM_PSC_TX_SLOW) ? \
                       (psc_info->slow_rate/BCM_OAM_ENDPOINT_CCM_PERIOD_3MS) : 
                       ((5*BCM_OAM_ENDPOINT_CCM_PERIOD_1S)/BCM_OAM_ENDPOINT_CCM_PERIOD_3MS);

                    oamep.burst     = (psc_info->flags & BCM_OAM_PSC_TX_BURST) ? \
                                               _BCM_FE2000_MPLS_PSC_BURST_PKT_COUNT_:0;

                    /* these flags dont actually exist in the PSC endpoint entry (or maybe they exist,
                     * but there is no ucode to support them). PSC ALWAYS copies to host unless
                     * the RxFilter hasnt expired yet.  we can leave these flags in for now until
                     * we can figure out use cases for not copying to host, or over-riding the the
                     * RxFilter hold-down timer.
                     */
                    oamep.conttohost = (psc_info->flags & BCM_OAM_PSC_ALL_RX_COPY_TO_CPU)?1:0;
                    oamep.singletohost = (psc_info->flags & BCM_OAM_PSC_FIRST_RX_COPY_TO_CPU)?1:0;

                    /* update PSC specific information into endpoint */
                    status = soc_sbx_g2p3_oamep_set(unit, psc_info->id, &oamep);
                    if(BCM_FAILURE(status)) {
                        OAM_ERR((_SBX_D(unit, "Failed to set oamep 0x%x: %d %s\n"), 
                                 psc_info->id, status, bcm_errmsg(status)));
                        /* If tcal allocated, free it */
                        if ((psc_info->flags & _BCM_FE2000_MPLS_PSC_TX_ENABLE_FLAGS_) &&
                             (!(local_hash_data->psc_flags & _BCM_FE2000_MPLS_PSC_TX_ENABLE_FLAGS_))) {
                            if (_BCM_TCAL_ID_VALID(tcal_id)) {
                                _bcm_tcal_free(unit, &tcal_id);
                            }                        
                        }
                    } else {
                        /* If tcal allocated, enable it */
                        if ((psc_info->flags & _BCM_FE2000_MPLS_PSC_TX_ENABLE_FLAGS_) &&
                            (!(local_hash_data->psc_flags & _BCM_FE2000_MPLS_PSC_TX_ENABLE_FLAGS_))) {
                            /* enable tcal transmit */
                            status = _bcm_tcal_update(unit, BCM_OAM_ENDPOINT_CCM_PERIOD_3MS,
                                                      TRUE, local_hash_data->rec_idx, &tcal_id);
                            if(BCM_FAILURE(status)) {
                                OAM_ERR((_SBX_D(unit, "Failed to Enable TCalendar: %d %s\n"), 
                                         status, bcm_errmsg(status)));
                                
                                if (_BCM_TCAL_ID_VALID(tcal_id)) {
                                    _bcm_tcal_free(unit, &tcal_id);
                                }
                            } else {                            
                                local_hash_data->tcal_id = tcal_id;
                            }
                        }

                        /* If tx was enabled previously & getting disabled release the calendar */
                        if ((!(psc_info->flags & _BCM_FE2000_MPLS_PSC_TX_ENABLE_FLAGS_)) &&
                            (local_hash_data->psc_flags & _BCM_FE2000_MPLS_PSC_TX_ENABLE_FLAGS_)) {
                            if (_BCM_TCAL_ID_VALID(local_hash_data->tcal_id)) {
                                _bcm_tcal_free(unit, &local_hash_data->tcal_id);
                                local_hash_data->tcal_id = _BCM_TCAL_INVALID_ID;
                            }                
                        }
                    }
                }
            }
        }
    }

    if (BCM_SUCCESS(status)) {
        local_hash_data->psc_flags = psc_info->flags;
    }

    OAM_UNLOCK(unit);

    return status;
}

int bcm_fe2000_oam_psc_delete(int unit,
                              bcm_oam_psc_t *psc_info)
{
    int status = BCM_E_NONE;
    oam_sw_hash_data_t *local_hash_data = NULL;
    oam_sw_hash_data_t *remote_hash_data = NULL;
    soc_sbx_g2p3_oamep_t oamep;
  
    /* sanity check */
    /* validate input argumnets */
    if (!BCM_OAM_IS_INIT(unit)) {
        OAM_ERR((_SBX_D(unit, "Failed to initialize OAM Module\n")));
        return BCM_E_INIT;
    }

    /* check endpoint & get endpoint */
    if (!ENDPOINT_ID_VALID(unit, psc_info->id)) {
        OAM_ERR((_SBX_D(unit, "Invalid Local Endpoint: 0x%x\n"), psc_info->id));
        return BCM_E_PARAM;
    }

    if (!ENDPOINT_ID_VALID(unit, psc_info->remote_id)) {
        OAM_ERR((_SBX_D(unit, "Invalid Remote Endpoint: 0x%x\n"), psc_info->id));
        return BCM_E_PARAM;
    }

    OAM_LOCK(unit);

    local_hash_data = &_state[unit]->hash_data_store[psc_info->id];
    remote_hash_data = &_state[unit]->hash_data_store[psc_info->remote_id];

    if (local_hash_data->type  != _BCM_FE2000_MPLS_PSC_SUPPORTED_EPTYPE &&
        remote_hash_data->type != _BCM_FE2000_MPLS_PSC_SUPPORTED_EPTYPE) {
        OAM_ERR((_SBX_D(unit, "Invalid PSC oam endpoint specified\n")));
        status = BCM_E_PARAM;
    } else {
        if (remote_hash_data->local_rec_idx != psc_info->id) {
            OAM_ERR((_SBX_D(unit, "Invalid PSC remote oam endpoint, mismatches with local endpoint\n")));
            status = BCM_E_PARAM;
        } else {
            if (_BCM_TCAL_ID_VALID(local_hash_data->tcal_id)) {
                /* free this timer calendar */
                _bcm_tcal_free(unit, &local_hash_data->tcal_id);
                local_hash_data->tcal_id = _BCM_TCAL_INVALID_ID;
            }
            
            /* Always consider as update */
            soc_sbx_g2p3_oamep_t_init(&oamep);
            status = soc_sbx_g2p3_oamep_set(unit, psc_info->id, &oamep);
            if(BCM_FAILURE(status)) {
                OAM_ERR((_SBX_D(unit, "Failed to set oamep 0x%x: %d %s\n"), 
                         psc_info->id, status, bcm_errmsg(status)));
                
            }
        }
    }

    OAM_UNLOCK(unit);

    return status;
}

int bcm_fe2000_oam_psc_get(int unit,
                           bcm_oam_psc_t *psc_info)
{
    int status = BCM_E_NONE;
    oam_sw_hash_data_t *local_hash_data = NULL;
    oam_sw_hash_data_t *remote_hash_data = NULL;
    soc_sbx_g2p3_oamep_t oamep;
  
    /* sanity check */
    /* validate input argumnets */
    if (!BCM_OAM_IS_INIT(unit)) {
        OAM_ERR((_SBX_D(unit, "Failed to initialize OAM Module\n")));
        return BCM_E_INIT;
    }

    /* check endpoint & get endpoint */
    if (!ENDPOINT_ID_VALID(unit, psc_info->id)) {
        OAM_ERR((_SBX_D(unit, "Invalid Local Endpoint: 0x%x\n"), psc_info->id));
        return BCM_E_PARAM;
    }

    if (!ENDPOINT_ID_VALID(unit, psc_info->remote_id)) {
        OAM_ERR((_SBX_D(unit, "Invalid Remote Endpoint: 0x%x\n"), psc_info->id));
        return BCM_E_PARAM;
    }

    OAM_LOCK(unit);

    local_hash_data = &_state[unit]->hash_data_store[psc_info->id];
    remote_hash_data = &_state[unit]->hash_data_store[psc_info->remote_id];

    if (local_hash_data->type  != _BCM_FE2000_MPLS_PSC_SUPPORTED_EPTYPE &&
        remote_hash_data->type != _BCM_FE2000_MPLS_PSC_SUPPORTED_EPTYPE) {
        OAM_ERR((_SBX_D(unit, "Invalid PSC oam endpoint specified\n")));
        status = BCM_E_PARAM;
    } else {
        if (remote_hash_data->local_rec_idx != psc_info->id) {
            OAM_ERR((_SBX_D(unit, "Invalid PSC remote oam endpoint, mismatches with local endpoint\n")));
            status = BCM_E_PARAM;
        } else {
            soc_sbx_g2p3_oamep_t_init(&oamep);
            status = soc_sbx_g2p3_oamep_get(unit, psc_info->id, &oamep);
            if(BCM_FAILURE(status)) {
                OAM_ERR((_SBX_D(unit, "Failed to set oamep 0x%x: %d %s\n"), 
                         psc_info->id, status, bcm_errmsg(status)));
            } else {
                /* Based on slow rate, burst setting adjust oam endpoint entry */
                psc_info->fpath   = oamep.fpath;
                psc_info->path    = oamep.path;
                psc_info->version = oamep.version;
                psc_info->request = oamep.request;
                psc_info->pt      = oamep.pt;

                psc_info->flags  |= ((oamep.revertive)?BCM_OAM_PSC_REVERTIVE:0);
                psc_info->flags  |= ((oamep.burst)?BCM_OAM_PSC_TX_BURST:0);

                /* these flags dont actually exist in the PSC endpoint entry (or maybe they exist,
                 * but there is not ucode to support them). PSC ALWAYS copies to host unless
                 * the RxFilter hasnt expired yet.  we can leave these flags in for now until
                 * we can figure out use cases for not copying to host, or over-riding the the
                 * RxFilter hold-down timer.
                 */
                psc_info->flags  |= ((oamep.conttohost)?BCM_OAM_PSC_ALL_RX_COPY_TO_CPU:0);
                psc_info->flags  |= ((oamep.singletohost)?BCM_OAM_PSC_FIRST_RX_COPY_TO_CPU:0);
            }
        }
    }

    OAM_UNLOCK(unit);
    return status;
}

#endif
