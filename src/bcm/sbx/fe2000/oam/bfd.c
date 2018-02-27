/*
 * $Id: bfd.c 1.20 Broadcom SDK $
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
 * FE2000 OAM API
 */

#if defined(INCLUDE_L3)

#include <bcm_int/sbx/fe2000/oam/oam.h>
#include <bcm_int/sbx/fe2000/oam/bfd.h>
#include <bcm_int/sbx/fe2000/oam/mpls.h>
#include <bcm_int/sbx/fe2000/mpls.h>
extern oam_state_t* _state[SOC_MAX_NUM_DEVICES];

int _bcm_fe2000_validate_oam_lsp_bfd_endpoint(int unit, 
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

    } else if (endpoint_info->bfd_flags & ~_BCM_FE2000_BFD_SUPPORTED_FLAGS) {
        OAM_ERR((_SBX_D(unit, "Unsupported Endpoint Flag: 0x%x\n"), endpoint_info->flags));

    } else if (endpoint_info->type & ~_BCM_FE2000_BFD_SUPPORTED_EPTYPE) {
        OAM_ERR((_SBX_D(unit, "Unsupported Endpoint Type: %d\n"), endpoint_info->type));

    } else if (endpoint_info->bfd_flags & BCM_OAM_BFD_UNIDIRECTIONAL) {
        OAM_WARN((_SBX_D(unit, "Unidirectional Endpoint set, Feature not supported: %d\n"), endpoint_info->type));

    } else if (endpoint_info->bfd_flags & BCM_OAM_BFD_MULTIPOINT) {
        OAM_WARN((_SBX_D(unit, "Multipoint flag set, feature not supported\n")));

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
           if (OAM_MPLS_LSP_OUT_EGR_IF(unit).flags & BCM_L3_TGID) {
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

int _bcm_fe2000_validate_oam_bfd_endpoint(int unit, 
                                          bcm_oam_endpoint_info_t *endpoint_info)
{
    int status = BCM_E_PARAM;

    /* TODO : Do we need a validate if multiple BFD session must not be enabled on same GPORT ??*/

    if(!endpoint_info) {
        OAM_ERR((_SBX_D(unit, "Null endpoint info\n")));

    } else if (endpoint_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
        OAM_ERR((_SBX_D(unit, "Unsupported Endpoing Flags: INTERMEDIATE\n")));

    } else if (endpoint_info->bfd_flags & ~_BCM_FE2000_BFD_SUPPORTED_FLAGS) {
        OAM_ERR((_SBX_D(unit, "Unsupported Endpoint Flag: 0x%x\n"), endpoint_info->flags));

    } else if (endpoint_info->type & ~_BCM_FE2000_BFD_SUPPORTED_EPTYPE) {
        OAM_ERR((_SBX_D(unit, "Unsupported Endpoint Type: %d\n"), endpoint_info->type));

    } else if (!BCM_GPORT_IS_MPLS_PORT(endpoint_info->gport)) {
        OAM_ERR((_SBX_D(unit, "BFD Supported only on MPLS Gport\n")));

    } else if (endpoint_info->bfd_flags & BCM_OAM_BFD_UNIDIRECTIONAL) {
        OAM_WARN((_SBX_D(unit, "Unidirectional Endpoint set, Feature not supported: %d\n"), endpoint_info->type));

    } else if (endpoint_info->bfd_flags & BCM_OAM_BFD_MULTIPOINT) {
        OAM_WARN((_SBX_D(unit, "Multipoint flag set, feature not supported\n")));

    } else {
        status = BCM_E_NONE;
    }

    if(BCM_SUCCESS(status) && (endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {
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
                
                if(hash_data->rec_idx != endpoint_info->local_id) {
                    OAM_ERR((_SBX_D(unit, "Local Endpoint does not match with one on group\n")));
                    status = BCM_E_PARAM;
                    break;
                }
            } DQ_TRAVERSE_END(&trunk_data->endpoint_list, ep_elem);
        }
    }
    return status;
}

int _oam_bfd_endpoint_set(int unit, bcm_oam_endpoint_info_t *ep_info, 
                          uint32_t ep_rec_index, egr_path_desc_t *egrPath, 
                          tcal_id_t *tcal_id, uint32_t label)
{
    int                               rv = BCM_E_NONE, rv2 = BCM_E_NONE;
    uint32_t                          watchdog_id = INVALID_POLICER_ID;
    uint32                            bfd_type=0, ep_type=0;
    soc_sbx_g2p3_oamep_t              oamep;
    bcm_oam_group_info_t             *group_info = NULL;
    oam_sw_hash_data_t               *hash_data = NULL;
    uint32                            lsmIdx = 0;
    soc_sbx_g2p3_lsmac_t              lsm;
    soc_sbx_g2p3_label2e_t            label2e;
    soc_sbx_g2p3_lp_t                 label_lp;

    /* sanity check pointers */
    if ((ep_info==NULL) || (egrPath==NULL) || (tcal_id==NULL)) {
        goto exit;
    }

    /* group */
    group_info = OAM_GROUP_INFO(unit, ep_info->group);

    hash_data = &_state[unit]->hash_data_store[ep_rec_index];

    soc_sbx_g2p3_oam_function_bfd_get(unit, &bfd_type);
    if (OAM_IS_BFD(ep_info->type)) {
        soc_sbx_g2p3_oam_type_mpls_pwe_get(unit, &ep_type);
    } else {
        soc_sbx_g2p3_oam_type_mpls_lsp_get(unit, &ep_type);
    }

    soc_sbx_g2p3_oamep_t_init(&oamep);
    soc_sbx_g2p3_label2e_t_init(&label2e);

    if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
        rv = soc_sbx_g2p3_oamep_get(unit, ep_rec_index, &oamep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x: %d %s\n"),
                     ep_rec_index, rv, bcm_errmsg(rv)));
            goto exit;
        }

        /* the soc layer doesn't handle overlays very well.
         * Must clear all fields not related to this type to ensure
         * bits are set as expected
         */
        _oam_oamep_prepare(unit, &oamep);
    }

    /************* PEER entry ****************/
    if (ep_info->flags & BCM_OAM_ENDPOINT_REMOTE) {

        rv = soc_sbx_g2p3_oamep_get(unit, ep_info->local_id, &oamep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x: %d %s\n"),
                      ep_info->local_id, rv, bcm_errmsg(rv)));
            return rv;
        }

        _oam_oamep_prepare(unit, &oamep);

        if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {

            /* free the timer and reallocate to simulate a reset */
            if (BCM_SUCCESS(rv) && oamep.policerid != INVALID_POLICER_ID) {
                rv = _oam_timer_free(unit, &oamep.policerid);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed to free watchdog timer 0x%x: %d %s\n"),
                             oamep.policerid, rv, bcm_errmsg(rv)));
                    return rv;
                }
            }
        } 

        /* only peer/remote endpoints require watchdog timers */
        if (ep_info->min_rx_interval && ep_info->detect_mult) {
            /* use the "peer" entry's ID. */
            rv = _oam_timer_allocate(unit, ep_rec_index,
                                     ep_info->min_rx_interval,
                                     ep_info->detect_mult,
                                     &watchdog_id);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to allocated watchdog timer of period %d"
                                " for epId 0x%x: %d %s\n"),
                         ep_info->min_rx_interval * ep_info->detect_mult,
                         ep_rec_index, 
                         rv, bcm_errmsg(rv)));
                goto exit;
            }
        } else {
            OAM_WARN((_SBX_D(unit, "Receive watchdog timer not allocated epId 0x%x, rx_interval %d detect_mult %d \n"),
                         ep_rec_index, ep_info->min_rx_interval ,
                         ep_info->detect_mult));
        }

        /* If remote endpoint, configure local ep idx to label2e Logical port */
        if (!(ep_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {

           rv = soc_sbx_g2p3_label2e_get(unit, label, &label2e);
           if  (BCM_FAILURE(rv)) {
               OAM_ERR((_SBX_D(unit, "Failed to get label2e 0x%x: %d %s\n"),
                        label, rv, bcm_errmsg(rv)));
               goto exit;
           }

           if (!label2e.lpidx) {
               rv = BCM_E_INTERNAL;
               OAM_ERR((_SBX_D(unit, "Label2e for label 0x%x has not associate logical port\n"),label));
               goto exit;

           } else {
               rv = soc_sbx_g2p3_lp_get(unit, label2e.lpidx, &label_lp);
               if  (BCM_FAILURE(rv)) {
                   OAM_ERR((_SBX_D(unit, "Failed to get lpi 0x%x: %d %s\n"),
                            label2e.lpidx, rv, bcm_errmsg(rv)));
                   goto exit;

               } else {
                   /* if psc endpoint exists on the LSP, associate it to bfd endpoint */
                   if (label_lp.oamepi) {
                       soc_sbx_g2p3_oamep_t pscep;
                       soc_sbx_g2p3_oamep_t_init(&pscep); 
                       rv = soc_sbx_g2p3_oamep_get(unit, label_lp.oamepi, &pscep);
                       if (BCM_FAILURE(rv)) {
                           OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x: %d %s\n"),
                                    label_lp.oamepi, rv, bcm_errmsg(rv)));
                           goto exit;
                       } else {
                           _oam_oamep_prepare(unit, &pscep);
                           if (pscep.function != _state[unit]->ep_type_ids.psc) {
                               OAM_ERR((_SBX_D(unit, "Unsupported chaining endpoint 0x%d\n"),
                                        pscep.function));
                               goto exit;
                           } else {
                               /* chain bfd to psc endpoint */
                               oamep.pscentry  = label_lp.oamepi;
                               label_lp.oamepi = ep_info->local_id;
                           }
                       }
                   }

                   label_lp.oamepi = ep_info->local_id;
                   rv = soc_sbx_g2p3_lp_set(unit, label2e.lpidx, &label_lp);
                   if  (BCM_FAILURE(rv)) {
                       OAM_ERR((_SBX_D(unit, "Failed to set lpi 0x%x: %d %s\n"),
                                label2e.lpidx, rv, bcm_errmsg(rv)));
                       goto exit;

                   }    
               }
           }       
       }

        oamep.peerdetectmulti = ep_info->detect_mult;
        oamep.yourdiscrim     = ep_info->my_discriminator_id;
        oamep.peerstate       = ep_info->bfd_state;
        oamep.policerid       = watchdog_id; /* watchdog_id is initialized to INVALID for local EP */
        /* Note: Currently TX/RX time must be same so ignore setting time interval on EP */

    } else {  /************* LOCAL entry ****************/

       if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
           lsmIdx = ~0;
           rv = _sbx_gu2_ismac_idx_free(unit, hash_data->mac_address,
                                        _SBX_GU2_RES_UNUSED_PORT, &lsmIdx);
           OAM_VERB((_SBX_D(unit, "freed mac " L2_6B_MAC_FMT " at %d on replace: "
                            "%s\n"),
                     L2_6B_MAC_PFMT(hash_data->mac_address), lsmIdx, 
                     bcm_errmsg(rv)));
           if (rv == BCM_E_EMPTY) {
               rv = BCM_E_NONE;
           }
       } 

       /* Add an LSM entry (for both MEPs and MIPs) */
       soc_sbx_g2p3_lsmac_t_init (&lsm);
       rv = _sbx_gu2_ismac_idx_alloc(unit, 0, ep_info->src_mac_address, 
                                     _SBX_GU2_RES_UNUSED_PORT, &lsmIdx);
       
       if (BCM_FAILURE(rv)) {
           OAM_ERR((_SBX_D(unit, "Failed to allocate local station MAC idx\n")));
           goto exit;
       } else {
           OAM_VERB((_SBX_D(unit, "Stored mac " L2_6B_MAC_FMT " at %d\n"),
                     L2_6B_MAC_PFMT(ep_info->src_mac_address), lsmIdx));

           hash_data->lsm_idx = lsmIdx;
       }

       sal_memcpy (lsm.mac, ep_info->src_mac_address, sizeof(bcm_mac_t));
       lsm.useport = 0;
       rv = soc_sbx_g2p3_lsmac_set (unit, lsmIdx, &lsm);
       
       if (BCM_FAILURE(rv)) {
           OAM_ERR((_SBX_D(unit, "Failed to write local station MAC\n")));
           goto exit;
       }
#ifdef BFD_AUTO_DETECT_POLL
       if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
           if (!(ep_info->bfd_flags & BCM_OAM_BFD_FINAL)) {

               uint32_t tmp_interval = 0;

               if ((oamep.mydiscrim != ep_info->my_discriminator_id) ||
                  (oamep.localstate != ep_info->bfd_state) ||
                  (oamep.localdetectmulti != ep_info->detect_mult)) {
                    ep_info->bfd_flags |= BCM_OAM_BFD_POLL;
               }
               rv = _bcm_tcal_interval_encode(unit, ep_info->min_tx_interval,
                                          &tmp_interval);
               if (BCM_FAILURE(rv)) {
                   OAM_ERR((_SBX_D(unit, "oam interval %d encode failed: %d %s\n"), 
                   ep_info->ccm_period, rv, bcm_errmsg(rv)));
                   goto exit;
               }
               if (oamep.interval != tmp_interval) {
                  ep_info->bfd_flags |= BCM_OAM_BFD_POLL;
               }
           } else {
                  /* Final clears Poll */
                  ep_info->bfd_flags &= ~BCM_OAM_BFD_POLL;
           }
       }
#endif
       
       oamep.function      = bfd_type;
       oamep.type          = ep_type;
       /* these "to-host" flags are for both CCM and BFD endpoints for as
        * long as the BFD and CCM share the same endpoint_info structure.
        * when/if BFD and CCm are seperated, there will be new BFD specific
        * flags.
        */
       oamep.conttohost    = (ep_info->bfd_flags & BCM_OAM_ENDPOINT_CCM_COPYTOCPU)?1:0;
       oamep.singletohost  = (ep_info->bfd_flags & BCM_OAM_ENDPOINT_CCM_COPYFIRSTTOCPU)?1:0;
       oamep.mepid         = ep_info->name;
       oamep.mode          = (ep_info->bfd_flags & BCM_OAM_BFD_CV_TYPE_IP)?1:0;
       oamep.localstate    = ep_info->bfd_state;
       oamep.diag          = ep_info->diag_code;
       oamep.poll          = (ep_info->bfd_flags & BCM_OAM_BFD_POLL)?1:0;
       oamep.final         = 0;
       oamep.fbit          = (ep_info->bfd_flags & BCM_OAM_BFD_FINAL)?1:0;
       /* CRA, CSP 339509 & SDK-31001 discuss the handling of the Final bit.
        *      It was agreed that when the RX ucode receives a Poll Message,
        *      the ucode will NOT automatically send a Final Message.  Rather,
        *      the Poll message must be sent to the host app for processing.  The
        *      host app must then update the endpoint_info with the new info in
        *      the Poll message, plus the host app will also set the Final Flag.
        *      The SDK will set the "fbit" ucode flag when the API FINAL flag is
        *      set.  The ucode's "fbit" flag will tell the TX ucode to transmit
        *      one and only one Final messsage, then auto-clear the fbit flag.
        *      The ucode "oamep.final" bit is un-used.  The oamep.fbit flag is
        *      used to transmit Final to peer.
        */
       oamep.c             = (ep_info->bfd_flags & BCM_OAM_BFD_CONTROL_INDEPENDENT)?1:0;
       oamep.a             = (ep_info->bfd_flags & BCM_OAM_BFD_OAM_BFD_AUTHENTICATE)?1:0;
       oamep.d             = (ep_info->bfd_flags & BCM_OAM_BFD_DEMAND_MODE)?1:0;
       oamep.m             = (ep_info->bfd_flags & BCM_OAM_BFD_MULTIPOINT)?1:0;
       oamep.unibi         = (ep_info->bfd_flags & BCM_OAM_BFD_UNIDIRECTIONAL)?0:1; /* 0=uni, 1=bi-directional. */
       oamep.udpsourceport = ep_info->udp_src_port;
       oamep.ipda          = ep_info->dest_ip;
       /* IPSA - push down global to ucode */
       /* UDP dest port - push down global to ucode */
       /* Control independent - hardwired to 0 in ucode */
       oamep.localdetectmulti = ep_info->detect_mult;
       oamep.mydiscrim  = ep_info->my_discriminator_id;

       /* when p2e.customer = 0, (provider port) the endpoint entry's
        * int_pri will be written into the erh.rcos/rdp field.
        */
       
       oamep.intpri = (((int)(ep_info->int_pri)) & 0x1f);
       oamep.rdi     = !!(group_info->flags & BCM_OAM_GROUP_REMOTE_DEFECT_TX);
       oamep.ftidx   = egrPath->ftIdx;

       /* the interval in the ep_info->min_tx_interval field is an actual time,
        * it is not an enumeration representing a time.  This function 
        * returns the enum corresponding to the interval.
        */
       rv = _bcm_tcal_interval_encode(unit, ep_info->min_tx_interval, &oamep.interval);
       if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "oam interval %d encode failed: %d %s\n"), 
                     ep_info->ccm_period, rv, bcm_errmsg(rv)));
            goto exit;
       }

    } /* end LOCAL entry */

    /* the remaining calls are common to local and peer/remote endpoint
     * configuration
     */
    rv = soc_sbx_g2p3_oamep_set(unit, 
                                (ep_info->flags & BCM_OAM_ENDPOINT_REMOTE)?\
                                ep_info->local_id:ep_rec_index,
                                &oamep);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to write oamep 0x%x: %d %s\n"),
                 ep_rec_index, rv, bcm_errmsg(rv)));

    } else {
        if (!(ep_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {

            /* Set up the timer calendar to trigger the transmission of
             * OAM packets.
             * ! the endpoint entry MUST be added before these tcal entries        !
             * ! because they (tcal) will trigger the start of packet transmission !
             * ! which reads the endpoint entries.  order matters.                 !
             */

            int tx_enable = !!(ep_info->flags & _SBX_OAM_CCM_TX_ENABLE);

            if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
                *tcal_id = _state[unit]->hash_data_store[ep_rec_index].tcal_id;
                rv = _bcm_tcal_update(unit, ep_info->min_tx_interval, tx_enable,
                                      ep_rec_index, tcal_id);
                OAM_VERB((_SBX_D(unit, "Timer Calendar: %d Updated for EP[0x%x] txEnable[%d]\n"),
                          *tcal_id, ep_rec_index, tx_enable));
            } else {
                rv = _bcm_tcal_alloc(unit, ep_info->min_tx_interval, tx_enable,
                                     ep_rec_index, tcal_id);
                OAM_VERB((_SBX_D(unit, "Timer Calendar: %d Allocated for EP[0x%x] txEnable[%d]\n"),
                          *tcal_id, ep_rec_index, tx_enable));
            }

            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to write/update timer calendar 0x%x: %d %s\n"),
                         *tcal_id, rv, bcm_errmsg(rv)));
            } else {
                /* Insert the local endpoint into L3 interface OAM endpoint list */
                if (!(ep_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {
                    /* If MPLS LSP related endpoints, hang it to interface endpoint list */
                    if (bcmOAMEndpointTypeBFDMPLS == ep_info->type || \
                        bcmOAMEndpointTypeBFDMPLSCcCv == ep_info->type) {
                        rv = _bcm_fe2000_lsp_intf_ep_list_add(unit, ep_info->intf_id, hash_data); 
                        if (BCM_FAILURE(rv)) {
                            OAM_ERR((_SBX_D(unit, "Failed to insert oam into interface endpoint list: %d %s\n"),
                                     rv, bcm_errmsg(rv)));
                        }
                    }
                }                
            }
        }
    }

 exit:
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "failed: err=%d %s\n"), rv, bcm_errmsg(rv)));
        OAM_DEBUG_EP(BCM_DBG_ERR, ep_info);

        if (ep_info->flags & BCM_OAM_ENDPOINT_REMOTE) {         

            if (oamep.policerid != INVALID_POLICER_ID) {
                rv2 = _oam_timer_free(unit, &oamep.policerid);
                if (BCM_FAILURE(rv2)) {
                    OAM_ERR((_SBX_D(unit, "Failed to free watchdog timer 0x%x: %d %s\n"),
                             oamep.policerid, rv2, bcm_errmsg(rv2)));
                } 

                /* remove policer from db record on error after it's been commmited to
                 * avoid resource leaks and pointers to free'd policers
                 */
                OAM_VERB((_SBX_D(unit, "clearing timers in oamep\n")));
                oamep.policerid = INVALID_POLICER_ID;
                rv2 = soc_sbx_g2p3_oamep_set(unit, ep_info->local_id, &oamep);  
                if (BCM_FAILURE(rv2)) {
                    OAM_ERR((_SBX_D(unit, "Failed to write oamep 0x%x: %d %s\n"),
                             ep_info->local_id, rv2, bcm_errmsg(rv2)));
                }
            }

            if (!(ep_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {
                if (label2e.lpidx) {
                    label_lp.oamepi = 0;
                    rv2 = soc_sbx_g2p3_lp_set(unit, label2e.lpidx, &label_lp);
                    if  (BCM_FAILURE(rv2)) {
                        OAM_ERR((_SBX_D(unit, "Failed to set lpi 0x%x: %d %s\n"),
                                 label2e.lpidx, rv2, bcm_errmsg(rv2)));
                    } 
                }
            }

        } else {
            if(hash_data->lsm_idx == lsmIdx) {
                _sbx_gu2_ismac_idx_free(unit, hash_data->mac_address, _SBX_GU2_RES_UNUSED_PORT, &lsmIdx);
            }
        }
    }

    return rv;
}

int _oam_bfd_endpoint_get(int unit,
                          bcm_oam_endpoint_info_t *ep_info, 
                          uint32_t ep_rec_index)
{
    int status = BCM_E_NONE;
    oam_sw_hash_data_t *hash_data = NULL;
    uint32_t oamep_idx = ep_rec_index;
    egr_path_desc_t egrPath;
    soc_sbx_g2p3_oamep_t oamep;

    if (ep_info && ENDPOINT_ID_VALID(unit,ep_rec_index)) {

        hash_data = &_state[unit]->hash_data_store[ep_rec_index];
        ep_info->name       = hash_data->ep_name;
        ep_info->flags      = hash_data->flags;
        ep_info->bfd_flags  = hash_data->bfd_flags;
        ep_info->type       = hash_data->type;
        ep_info->group      = hash_data->group;
        ep_info->gport      = hash_data->gport;
        ep_info->vlan       = hash_data->vid_label;
        ep_info->name       = hash_data->ep_name;
        ep_info->local_id   = hash_data->local_rec_idx;
        ep_info->mpls_label = hash_data->vid_label;
        ep_info->intf_id    = hash_data->intf_id;
        ep_info->trunk_index  = hash_data->trunk_index;
        
        if (hash_data->flags & BCM_OAM_ENDPOINT_REMOTE) {
            oamep_idx = hash_data->local_rec_idx;
            
        } else {
            status = _oam_egr_path_get(unit, &egrPath, oamep_idx);
        }
    
        if(BCM_SUCCESS(status)) {
            status = soc_sbx_g2p3_oamep_get(unit, oamep_idx, &oamep);
            if(BCM_SUCCESS(status)) {

                if (hash_data->flags & BCM_OAM_ENDPOINT_REMOTE) {
                    ep_info->min_rx_interval     = hash_data->bfd_interval;
                    ep_info->detect_mult         = oamep.peerdetectmulti;
                    ep_info->bfd_state           = oamep.peerstate;
                    ep_info->my_discriminator_id = oamep.yourdiscrim;

                } else {
                    ep_info->my_discriminator_id = oamep.mydiscrim;
                    ep_info->min_tx_interval     = hash_data->bfd_interval;
                    ep_info->detect_mult         = oamep.localdetectmulti;
                    ep_info->bfd_state           = oamep.localstate;
                    ep_info->diag_code           = oamep.diag;
                    if (OAM_IS_BFD(ep_info->type)) {
                        ep_info->udp_src_port        = oamep.udpsourceport;
                        ep_info->dest_ip             = oamep.ipda;
                        soc_sbx_g2p3_oam_pwe_udp_dest_port_get(unit, 
                                           (uint32*)&ep_info->udp_src_port);
                        soc_sbx_g2p3_oam_pwe_ip_sa_get(unit, 
                                           &ep_info->source_ip);
                    }
                    if(hash_data->lsm_idx) {
                        soc_sbx_g2p3_lsmac_t      lsm;
                        soc_sbx_g2p3_lsmac_get(unit, hash_data->lsm_idx, &lsm);
                        sal_memcpy(&ep_info->src_mac_address, lsm.mac, sizeof(bcm_mac_t));
                    }

                    sal_memcpy(ep_info->dst_mac_address, hash_data->mac_address,
                               sizeof(bcm_mac_t));
                    ep_info->pkt_pri = egrPath.eteL2.defpricfi >> 1;
                    ep_info->int_pri = egrPath.eteEncap.remark << 5 | oamep.intpri;
                }
            }
        }
    } else {
        status = BCM_E_PARAM;
    }

    return status;
}

int _oam_bfd_endpoint_delete(int unit, 
                             bcm_oam_endpoint_info_t *ep_info)
{
    int status = BCM_E_PARAM;
    oam_sw_hash_data_t *hash_data = NULL;
    uint32_t oamep_idx =0;
    soc_sbx_g2p3_oamep_t oamep;
    uint32 watchdog_id = INVALID_POLICER_ID;

    /* sanity check pointers */
    if (ep_info && ENDPOINT_ID_VALID(unit, ep_info->id)) {
        
        hash_data = &_state[unit]->hash_data_store[ep_info->id];

        oamep_idx = (hash_data->flags & BCM_OAM_ENDPOINT_REMOTE)? \
                     ep_info->local_id:ep_info->id;

        status  = soc_sbx_g2p3_oamep_get(unit, oamep_idx, &oamep);
        if (BCM_FAILURE(status)) {
            /* non-fatal error, keep trying */
            OAM_ERR((_SBX_D(unit, "Failed to find EP record idx=%d err=%d %s\n"),
                     oamep_idx, status, bcm_errmsg(status)));
        }

        if (hash_data->flags & BCM_OAM_ENDPOINT_REMOTE) {
            soc_sbx_g2p3_label2e_t  label2e;
            soc_sbx_g2p3_lp_t       label_lp;

            watchdog_id = oamep.policerid;
            oamep.peerdetectmulti = 0;
            oamep.yourdiscrim     = 0;
            oamep.peerstate       = 0;
            oamep.policerid       = 0;

            status = soc_sbx_g2p3_label2e_get(unit, hash_data->vid_label, &label2e);
            if  (BCM_FAILURE(status)) {
                OAM_WARN((_SBX_D(unit, "Failed to get label2e 0x%x: %d %s\n"),
                         hash_data->vid_label, status, bcm_errmsg(status)));

            } else {
                status = soc_sbx_g2p3_lp_get(unit, label2e.lpidx, &label_lp);
                if  (BCM_FAILURE(status)) {
                    OAM_WARN((_SBX_D(unit, "Failed to get lpi 0x%x: %d %s\n"),
                              label2e.lpidx, status, bcm_errmsg(status)));

                } else {
                    /* if PSC endpoint is chained, set lp to point to psc endpoint */
                    label_lp.oamepi = (oamep.pscentry)?oamep.pscentry:0;
                    status = soc_sbx_g2p3_lp_set(unit, label2e.lpidx, &label_lp);
                    if  (BCM_FAILURE(status)) {
                        OAM_WARN((_SBX_D(unit, "Failed to set lpi 0x%x: %d %s\n"),
                                 label2e.lpidx, status, bcm_errmsg(status)));

                    }  
                }                
            }
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
            /* 'clear' the entry */
            soc_sbx_g2p3_oamep_t_init(&oamep);        

            /* Remove the local endpoint into L3 interface OAM endpoint list */
            status = _bcm_fe2000_lsp_intf_ep_list_remove(unit, ep_info->intf_id, hash_data); 
            if (BCM_FAILURE(status)) {
                OAM_ERR((_SBX_D(unit, "Failed to delete oam into interface endpoint list: %d %s\n"),
                         status, bcm_errmsg(status)));
            }
        }

        status = soc_sbx_g2p3_oamep_set(unit, oamep_idx, &oamep);
        if (BCM_FAILURE(status)) {
            /* non-fatal error, keep trying */
            OAM_ERR((_SBX_D(unit, "Failed to set BFD oamEp idx=%d err=%d %s\n"),
                     oamep_idx, status, bcm_errmsg(status)));
        }
        
        if (watchdog_id != INVALID_POLICER_ID ) {
            _oam_timer_free(unit, &watchdog_id);
        }        


  
        status = BCM_E_NONE;
    }

    return status;
}

int _oam_bfd_egr_path_update (int unit,
                              egr_path_desc_t *egrPath,  
                              bcm_oam_endpoint_info_t *endpoint_info,
                              _fe2k_vpn_sap_t *vpn_sap)
{
    int status = BCM_E_NONE;

    if (!egrPath || !endpoint_info || !vpn_sap) {
        status = BCM_E_PARAM;
    } else {
        status = _oam_egr_path_dmac_set(unit, egrPath, endpoint_info->dst_mac_address, TRUE);
         if (BCM_FAILURE(status)) {
             OAM_ERR((_SBX_D(unit, "Failed to set dmac on egress path for EP %d\n"),
                      endpoint_info->id));

         } else {
             status = _oam_egr_path_smac_set(unit, egrPath, endpoint_info->src_mac_address);
             if (BCM_FAILURE(status)) {
                 OAM_ERR((_SBX_D(unit, "Failed to set smac on egress path for EP %d\n"),
                          endpoint_info->id));

             } else {
                 egrPath->eteEncap.s2          = 0;
                 egrPath->eteEncap.exp2        = vpn_sap->mpls_psn_label_exp;
                 egrPath->eteEncap.label2      = vpn_sap->mpls_psn_label;
                 egrPath->eteEncap.exp2remark  = vpn_sap->mpls_psn_label_exp;  
                 egrPath->eteEncap.ttl2        = vpn_sap->mpls_psn_label_ttl;

                 egrPath->eteEncap.s1          = 1;
                 egrPath->eteEncap.exp1        = vpn_sap->vc_mpls_port.egress_label.exp;
                 egrPath->eteEncap.label1      = vpn_sap->vc_mpls_port.match_label;
                 egrPath->eteEncap.exp1remark  = vpn_sap->vc_mpls_port.exp_map; 
                 egrPath->eteEncap.ttl1        = vpn_sap->vc_mpls_port.egress_label.ttl; 

                 egrPath->eteEncap.tunnelenter = 1;
                 egrPath->eteEncap.encaplen    = 10;

                 egrPath->eteEncap.etype       = 0x8847;
                 egrPath->eteEncap.ttlcheck    = 0;
                 egrPath->eteEncap.mplsttldec  = 0;

                 /* modport or local gport */
                 egrPath->port                 = vpn_sap->vc_mpls_port.port; 
             }
         }
    }

    return status;
}

int _oam_fe2000_bfd_chain_psc (int      unit, 
                               uint32_t bfd_ep_index,
                               uint32_t psc_ep_index,
                               uint8_t  chain /* 0 -unchain, 1- chain*/)
{
    int status = BCM_E_NONE;
    soc_sbx_g2p3_oamep_t oamep;

    if(bfd_ep_index > _state[unit]->max_endpoints ||\
       psc_ep_index > _state[unit]->max_endpoints)  {
        OAM_ERR((_SBX_D(unit, "Invalid endpoint id.  Must supply endpoint ID \n")));
        status = BCM_E_PARAM;
    } else {
        soc_sbx_g2p3_oamep_t_init(&oamep);   
        status = soc_sbx_g2p3_oamep_get(unit, bfd_ep_index, &oamep);
        if (BCM_FAILURE(status)) {
            OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x: %d %s\n"),
                     bfd_ep_index, status, bcm_errmsg(status)));
        } else {
            _oam_oamep_prepare(unit, &oamep);
            /* verify if this is a BFD endpoint */
            if (oamep.function != _state[unit]->ep_type_ids.bfd) {
                OAM_ERR((_SBX_D(unit, "Invalid BFD endpoint id 0x%x \n"), bfd_ep_index));
            } else {
                /* assumes psc ep index is valid */
                oamep.pscentry = (chain) ? psc_ep_index : 0;
                status = soc_sbx_g2p3_oamep_set(unit, bfd_ep_index, &oamep);
                if (BCM_FAILURE(status)) {
                    OAM_ERR((_SBX_D(unit, "Failed to write oamep 0x%x: %d %s\n"),
                             bfd_ep_index, status, bcm_errmsg(status)));
                }
            }
        }
    }   

    return status;
}


#else   /* INCLUDE_L3 */
int bcm_fe2000_oam_not_empty;
#endif  /* INCLUDE_L3 */
