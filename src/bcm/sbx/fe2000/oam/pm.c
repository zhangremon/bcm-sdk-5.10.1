/*
 * $Id: pm.c 1.31 Broadcom SDK $
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
 * FE2000 OAM Performance measurement function
 */

#if defined(INCLUDE_L3)

#include <bcm_int/sbx/fe2000/oam/oam.h>
#include <bcm_int/sbx/fe2000/oam/pm.h>
#include <bcm_int/sbx/fe2000/oam/mpls.h>
#include <bcm_int/sbx/fe2000/oam/enet.h>

extern oam_state_t* _state[SOC_MAX_NUM_DEVICES];

/*
 *   Function
 *      _oam_g2p3_lm_create
 *   Purpose
 *      Set a g2p3 oamep structure for loss measurements
 *   Parameters
 *       unit        = BCM device number
 *       flags       = BCM flags describing additional features
 *       lm_idx      = loss measurement hw index
 *       ep_idx      = endpoint hw index associated with this DM
 *       coco_idx    = coherent counter index
 *       mult_id     = loss thrshold mupliplier table id
 *   Returns
 *       BCM_E_*
 */
int 
_bcm_fe2000_oam_lm_create(int unit,
                          int flags, uint16_t lm_idx, uint16_t ep_idx,
                          uint32 fti, int coco_idx, int mult_id, 
                          int ep_type, int singletx)
{
    int rv;
    soc_sbx_g2p3_oamep_t lm_ep, local_ep;

    soc_sbx_g2p3_oamep_t_init(&lm_ep);

    rv = soc_sbx_g2p3_oamep_get(unit, ep_idx, &local_ep);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to read local endpoint at idx 0x%04x: "
                        "%d %s\n"), ep_idx, rv, bcm_errmsg(rv)));
        return rv;
    }

    lm_ep.mode         = !(flags & BCM_OAM_LOSS_SINGLE_ENDED);
    lm_ep.function     = _state[unit]->ep_type_ids.lm;
    lm_ep.type         = ep_type;
    lm_ep.dir          = local_ep.dir;
    lm_ep.conttohost   = (flags & BCM_OAM_LOSS_ALL_RX_COPY_TO_CPU) ? 1 : 0;
    lm_ep.singletohost = (flags & BCM_OAM_LOSS_FIRST_RX_COPY_TO_CPU) ? 1 : 0; 
    lm_ep.mdlevel      = local_ep.mdlevel;
    lm_ep.counteridx   = coco_idx;
    lm_ep.multiplieridx = mult_id;
    /* for mpls, this looks OK. MPLS endpoint is only useful when 
     * LM/DM are attached to it. So the encap information can be used by
     * LM/DM attached this will save forwarding resources */
    lm_ep.ftidx_store   = fti;
    lm_ep.nextentry_store = 0;

    /* FIMXE when ucode supports */
    /* lm_ep.singletx = singletx; */
    
    rv = soc_sbx_g2p3_oamep_set(unit, lm_idx, &lm_ep);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to write loss data at idx 0x%04x"
                        ": %d %s\n"), lm_idx, rv, bcm_errmsg(rv)));
        return rv;
    }

    OAM_VERB((_SBX_D(unit, "Wrote LossIdx=0x%04x, EpIdx=0x%04x\n"),
              lm_idx, ep_idx));


    return rv;
}





int
bcm_fe2000_oam_loss_add(int unit, bcm_oam_loss_t *loss_ptr)
{
    oam_sw_hash_data_t        *ep_data, *remote_data;
    oam_sw_hash_data_t        *lm_data;
    uint32_t                   loss_idx, coco_idx = OAM_COCO_INVAID_ID;
    tcal_id_t                  tcal_id, tmp_tcal;
    int                        update, enable, coco_configured = 0;
    int                        rv = BCM_E_NONE;
    int                        multId = _OAM_LTM_INVALID_ID;
    soc_sbx_g2p3_oamep_t       oamep[2];
    soc_sbx_g2p3_oamepremap_t  oamep_remap;
    uint8_t                    singletx = 0;
    egr_path_desc_t            loss_egr_path;
    int                        single_ended;

    sal_memset(&loss_egr_path, 0, sizeof(loss_egr_path));

    /*  Dual-ended uses CCM messages to relay frame counts between stations.
     *  Single-ended requires an additional timer calender entry to initate
     *    the LM Message in ucode.
     */

    if (!BCM_OAM_IS_INIT(unit)) {
        return BCM_E_INIT;
    }
    
    if (loss_ptr == NULL) {
        return BCM_E_PARAM;
    }

    if (loss_ptr->period > OAM_SBX_MAX_PERIOD) {
        OAM_ERR((_SBX_D(unit, "Maximim period supported by this device is %dms\n"),
                 OAM_SBX_MAX_PERIOD));
        return BCM_E_PARAM;
    }

    /* tcals are needed for Single-ended mode only */
    tcal_id   = _BCM_TCAL_INVALID_ID;
    loss_idx = INVALID_RECORD_INDEX;

    ep_data = remote_data = lm_data = NULL;

    if (loss_ptr->flags & ~SUPPORTED_LOSS_FLAGS) {
        OAM_ERR((_SBX_D(unit, "Invalid flags: 0x%08x supported=0x%08x\n"),
                 loss_ptr->flags, SUPPORTED_LOSS_FLAGS));
        return BCM_E_PARAM;
    }

    if (!ENDPOINT_ID_VALID(unit, loss_ptr->id) ||
        !ENDPOINT_ID_VALID(unit, loss_ptr->remote_id)) {
        OAM_ERR((_SBX_D(unit, "Invalid endpoint local=0x%08x %s  "
                        "remote=0x%08x %s\n"), 
                 loss_ptr->id, 
                 (ENDPOINT_ID_VALID(unit, loss_ptr->id) ?
                  "valid" : "invalid"),
                 loss_ptr->remote_id,
                 (ENDPOINT_ID_VALID(unit, loss_ptr->remote_id) ?
                  "valid" : "invalid")));
        return BCM_E_PARAM;
    }
    
    if (loss_ptr->loss_threshold == 0) {
        OAM_ERR((_SBX_D(unit, "Invalid loss threshold\n")));
        return BCM_E_PARAM;
    }

    single_ended = ( (loss_ptr->flags &  BCM_OAM_LOSS_SINGLE_ENDED) ? TRUE : FALSE);


    OAM_LOCK(unit);

    /* get the local endpoint sw state */ 
    ep_data = &_state[unit]->hash_data_store[loss_ptr->id];

    if (!ENDPOINT_ID_VALID(unit, ep_data->rec_idx) ||
        (ep_data->flags & BCM_OAM_ENDPOINT_REMOTE) ||
        (ep_data->oam_type != oamEp && !OAM_IS_ENDPOINT_MPLS_Y1731(ep_data->oam_type))) {
        OAM_ERR((_SBX_D(unit, "Local endpoint not found at endpoint id: 0x%08x\n"), 
                 loss_ptr->id));
        rv = BCM_E_PARAM;
    } 

    /* get the remote endpoint sw state */
    remote_data = &_state[unit]->hash_data_store[loss_ptr->remote_id];
    
    if (!ENDPOINT_ID_VALID(unit, remote_data->rec_idx) ||
        ((remote_data->flags & BCM_OAM_ENDPOINT_REMOTE) == FALSE) ||
        (remote_data->oam_type != oamEp && 
         !OAM_IS_ENDPOINT_MPLS_Y1731(remote_data->oam_type))) {
        OAM_ERR((_SBX_D(unit, "Remote endpoint not found at endpoint id: 0x%08x\n"), 
                 loss_ptr->remote_id));
        rv = BCM_E_PARAM;
    } 
    
    /* MPLS PM --- */
    if (OAM_IS_ENDPOINT_MPLS_Y1731(ep_data->oam_type)) {
        rv = _bcm_fe2000_mpls_oam_loss_set(unit, loss_ptr, ep_data, TRUE);
        OAM_UNLOCK(unit);
        return rv;
    } 

    /* Ethernet PM */
    if (loss_ptr->period < 0) {
        /* Loss Period of (-1) signifies single on-demand transmission 
         * for mpls 1731 oam. Not supported on other endpoints */
        OAM_ERR((_SBX_D(unit, "Invalid loss measurement period \n")));
        rv = BCM_E_PARAM;
    }

    /* An update/replace is when the delay_idx already exists for the endpoint
     */
    update = ENDPOINT_ID_VALID(unit, ep_data->loss_idx);

    /* Everything looks good, allocate a new record if not already available */
    if (BCM_SUCCESS(rv)) {
        if (update) {
            OAM_VERB((_SBX_D(unit, "loss record 0x%04x found to exist; re-using\n"),
                      ep_data->loss_idx));
                
            loss_idx = ep_data->loss_idx;
            lm_data = &_state[unit]->hash_data_store[loss_idx];
                
            if ((single_ended == TRUE) &&
                !_BCM_TCAL_ID_VALID(lm_data->tcal_id)) {
                OAM_ERR((_SBX_D(unit, "Invalid timer calendar found in loss state; "
                                "idx=0x%04x\n"), loss_idx));
                rv = BCM_E_INTERNAL;
            }

            if (BCM_SUCCESS(rv)) {
                tcal_id   = lm_data->tcal_id;
                coco_idx  = lm_data->cocounter;
            }
                
        } else {

            rv = shr_idxres_list_alloc(_state[unit]->ep_pool, &loss_idx);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to allocate an loss record: %d %s\n"),
                         rv, bcm_errmsg(rv)));

            } else {
                OAM_VERB((_SBX_D(unit, "Allocated loss record 0x%04x\n"), 
                          loss_idx));

                lm_data = &_state[unit]->hash_data_store[loss_idx];
            }

            if (BCM_SUCCESS(rv)) {
                rv = shr_idxres_list_alloc(_state[unit]->coco_pool,
                                           &coco_idx);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Out of coherent counter resources\n")));
                } else {
                    OAM_VERB((_SBX_D(unit, "Allocated Coherent Counter resource 0x%04x, base index 0x%04x\n"), 
                              coco_idx, OAM_COCO_BASE(coco_idx)));
                    coco_idx = OAM_COCO_BASE(coco_idx);
                }
            }
        }
    }

    /* free the previous loss threshold, on update */
    if (BCM_SUCCESS(rv) && update) {
        soc_sbx_g2p3_oamep_t loss_ep;
        rv = soc_sbx_g2p3_oamep_get(unit, loss_idx, &loss_ep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to read LossEP 0x%x: %d %s\n"),
                     loss_idx, rv, bcm_errmsg(rv)));
        }

        if (BCM_SUCCESS(rv)) {
            _bcm_ltm_threshold_free(unit, loss_ep.multiplieridx);
        }
    }

    /* allocate a loss threshold entry */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_ltm_threshold_alloc(unit, loss_ptr->loss_threshold, &multId);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to allocate a loss threshold table entry"
                            ": %d %s\n"), rv, bcm_errmsg(rv)));
        }
    }



    /* get or create an egress path */
    if (BCM_SUCCESS(rv)) {

        if (single_ended == TRUE) {

            if (update) {
                rv = _oam_egr_path_get(unit, &loss_egr_path, loss_idx);
            } else {

                /* create the egress path with local ep as smac, 
                 * and remote ep as dmac
                 */
                if (ep_data->flags & BCM_OAM_ENDPOINT_UP_FACING) {
                    /* up MEP uses peer's gport for FTE.QID */
                    rv = _oam_egr_path_clone_upmep(unit, &loss_egr_path, remote_data->gport,
                                                   ep_data->vid_label, ep_data->mac_address);
                } else {
                    rv = _oam_egr_path_clone_downmep(unit, &loss_egr_path, ep_data->rec_idx);
                }
            }

            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "failed to %s an egress path for "
                                "SE LM: %s\n"), 
                         (update)? "get" : "allocate", bcm_errmsg(rv)));
            }


            if (BCM_SUCCESS(rv)) {
                rv = _oam_egr_path_dmac_set(unit, &loss_egr_path, 
                                            remote_data->mac_address, TRUE);

                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "failed to update dmac for SE LM: %s\n"),
                             bcm_errmsg(rv)));
                }
            }

            if (BCM_SUCCESS(rv)) {

                rv = _oam_egr_path_commit(unit, &loss_egr_path);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed to commit egress path: %s\n"),
                             bcm_errmsg(rv)));
                }
            }
        } else {
            /* DE LM uses the CCM egr-path. */
            rv = _oam_egr_path_get(unit, &loss_egr_path, ep_data->rec_idx);
        }
    }


    /* set the basic loss hw data  */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_oam_lm_create(unit, loss_ptr->flags, 
                                       loss_idx, ep_data->rec_idx, 
                                       loss_egr_path.ftIdx, coco_idx, multId,
                                       _state[unit]->ep_transport.enet,
                                       singletx);
    }

    ep_data->loss_idx  = loss_idx;

    /* configure the allocated coherent counter idx in the lp, oi & evp2e */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_oam_enet_coco_configure(unit, ep_data, coco_idx, loss_ptr->pkt_pri_bitmap, single_ended);
        coco_configured = (BCM_SUCCESS(rv)) ? 1 : 0;
    }

    /* set the loss measurement transmission period, and optionally enable
     *
     *  For update entry:
     *     If transition from Single->Dual,
     *        free single's tcal; continue to new entry logic     
     * 
     *  For new entry:
     *    SingleEnded  - allocate timer calender
     *    DualEnded    - If Tx_Enable, verify ccm is transmitting & set 
     *                                 ep.lmEpIdx
     *                   if Tx_Disable, clear ep.lmEpIdx
     */

    if (BCM_SUCCESS(rv)) {
        /* update & transition from Single->Dual? */
        if (update && 
            (lm_data->flags & BCM_OAM_LOSS_SINGLE_ENDED) && 
            !(loss_ptr->flags & BCM_OAM_LOSS_SINGLE_ENDED)) {

            _bcm_tcal_free(unit, &lm_data->tcal_id);
            tcal_id = lm_data->tcal_id = _BCM_TCAL_INVALID_ID;
        }
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_oamep_get(unit, ep_data->rec_idx, &oamep[0]);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%04x\n"),
                     ep_data->rec_idx));
        }
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_oamep_get(unit, oamep[0].nextentry, &oamep[1]);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%04x\n"),
                     oamep[0].nextentry));
        }        
    }

    /* Common new loss entry allocations/updates */
    if (BCM_SUCCESS(rv)) {
        enable = !!(loss_ptr->flags & BCM_OAM_LOSS_TX_ENABLE);
        tmp_tcal = tcal_id;
        
        if (single_ended == TRUE) {
            if (_BCM_TCAL_ID_VALID(tcal_id)) {
                rv = _bcm_tcal_update(unit, loss_ptr->period, enable, loss_idx,
                                      &tmp_tcal);
            } else {
                rv = _bcm_tcal_alloc(unit, loss_ptr->period, enable, loss_idx,
                                     &tmp_tcal);
            }
                    
            if (BCM_SUCCESS(rv)) {
                tcal_id = tmp_tcal;
                OAM_VERB((_SBX_D(unit, "%s timer calender entry 0x%05x\n"),
                          _BCM_TCAL_ID_VALID(tcal_id)?"Updated":"Allocated",
                          tmp_tcal));
            } else {
                OAM_ERR((_SBX_D(unit, "Failed to %s tcal id\n"),
                         _BCM_TCAL_ID_VALID(tcal_id)?"update":"allocate"));
            }


            oamep[1].nextentry = 0;
        } else {
                
            /* DualEnded Mode */            
            /*  If Tx_Enable, verify ccm is transmitting & set  ep.lmEpIdx */
            if (BCM_SUCCESS(rv)) {
                if (enable) {
                    if (!(ep_data->flags & _SBX_OAM_CCM_TX_ENABLE)) {
                        OAM_WARN((_SBX_D(unit, "Dual-Ended Loss measurement enabled"
                                         " for transmit, but attached CCM "
                                         " endpoint is not - no effect.\n")));
                    }
                    oamep[1].nextentry = loss_idx;
                } else {
                    oamep[1].nextentry = 0;
                }
            }           
        }
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_oamep_set(unit, oamep[0].nextentry, &oamep[1]);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to set oamep 0x%04x->0x%04x\n"),
                     ep_data->rec_idx, oamep[0].nextentry));
        } else {
            OAM_VERB((_SBX_D(unit, "Updated EP 0x%04x->0x%04x lmEpIdx=0x%04x\n"),
                      ep_data->rec_idx, 
                      oamep[0].nextentry, oamep[1].nextentry));
        }
    }

    /* Set up the remap table to redirect LMMs to the correct OAM endpoint */
    if (BCM_SUCCESS(rv)) {
        if (single_ended == TRUE) {
            
            soc_sbx_g2p3_oamepremap_t_init(&oamep_remap);
            oamep_remap.epIdx = loss_idx;
            
            rv = soc_sbx_g2p3_oamepremap_set(unit, 
                                             ep_data->rec_idx,  oamep[0].mdlevel,
                                             _state[unit]->ep_subtypes.lm,
                                             &oamep_remap);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to set oamep remap"
                                "[0x%04x,0x%1x,%d]: %s\n"),
                         ep_data->rec_idx, oamep[0].mdlevel, 
                         _state[unit]->ep_subtypes.lm, bcm_errmsg(rv)));
            }
        }
    }
        
    /* Update software state */
    if (BCM_SUCCESS(rv)) {
        /* link endpoint to loss data to recognize 'replace' on next add */
        lm_data->tcal_id   = tcal_id;
        lm_data->rec_idx   = loss_idx;
        lm_data->peer_idx  = loss_ptr->remote_id;;
        lm_data->cocounter = coco_idx;
        lm_data->flags     = loss_ptr->flags;
        lm_data->oam_type  = oamLoss;
        lm_data->pkt_pri_bitmap = loss_ptr->pkt_pri_bitmap;
    }

    if (BCM_FAILURE(rv)) {
        /* If the loss idx was allocated for the first time,
         * free it on error
         */
        if (!update) {

            if (ENDPOINT_ID_VALID(unit, loss_idx)) {
                shr_idxres_list_free(_state[unit]->ep_pool, loss_idx);
                OAM_VERB((_SBX_D(unit, "(error %d) Freeing loss ep 0x%x\n"),
                          rv, loss_idx));
            }

            if (_BCM_TCAL_ID_VALID(tcal_id)) {
                _bcm_tcal_free(unit, &tcal_id);
                OAM_VERB((_SBX_D(unit, "(error %d) Freeing tcalId 0x%x\n"),
                          rv, tcal_id));
            }

            if (OAM_COCO_VALID(coco_idx)) {
                shr_idxres_list_free(_state[unit]->coco_pool, 
                                     OAM_COCO_RES(coco_idx));
                OAM_VERB((_SBX_D(unit, "(error %d) Freeing coherent counter 0x%x\n"),
                          rv, coco_idx));
            }
                
            if (multId != _OAM_LTM_INVALID_ID) {
                _bcm_ltm_threshold_free(unit, multId);
                OAM_VERB((_SBX_D(unit, "(error %d) Freeing loss threshold multiplier"
                                 " table 0x%x\n"),
                          rv, multId));
            }

            if(coco_configured) {
                rv = _bcm_fe2000_oam_enet_coco_configure(unit, ep_data, 0, 0, single_ended);
            }
        }
    }
    
    OAM_UNLOCK(unit);
    return rv;
}

int
bcm_fe2000_oam_loss_get(int unit, bcm_oam_loss_t *loss_ptr)
{
    oam_sw_hash_data_t *ep_data;
    oam_sw_hash_data_t *lm_data;
    int                 rv = BCM_E_NONE;
    soc_sbx_g2p3_oamep_t oamep;
    ep_data = lm_data = NULL;

    if (!BCM_OAM_IS_INIT(unit)) {
        rv = BCM_E_INIT;

    } else if (loss_ptr->flags & ~SUPPORTED_LOSS_FLAGS) {
        OAM_ERR((_SBX_D(unit, "Invalid flags: 0x%08x supported=0x%08x\n"),
                 loss_ptr->flags, SUPPORTED_LOSS_FLAGS));
        rv = BCM_E_PARAM;

    } else if (!ENDPOINT_ID_VALID(unit, loss_ptr->id)) {
        OAM_ERR((_SBX_D(unit, "Invalid local endpoint id: 0x%08x\n"), 
                 loss_ptr->id));
        rv = BCM_E_PARAM;

    } else {
        /* get the local endpoint sw state */ 
        OAM_LOCK(unit);
        ep_data = &_state[unit]->hash_data_store[loss_ptr->id];

        if (!ENDPOINT_ID_VALID(unit, ep_data->rec_idx) ||
            (ep_data->flags & BCM_OAM_ENDPOINT_REMOTE) ||
            (ep_data->oam_type != oamEp && !OAM_IS_ENDPOINT_MPLS_Y1731(ep_data->oam_type))) {
            OAM_ERR((_SBX_D(unit, "Local endpoint not found at endpoint id: 0x%08x\n"), 
                     loss_ptr->id));
            
            rv = BCM_E_PARAM;

        } else { 
            /* get the loss measurement endpoint */
            lm_data = &_state[unit]->hash_data_store[ep_data->loss_idx];

            if (!ENDPOINT_ID_VALID(unit, ep_data->loss_idx) ||
                (lm_data->oam_type != oamLoss)) {
                OAM_ERR((_SBX_D(unit, "Loss measurement not enabled on endpoint "
                                " 0x%04x\n"), loss_ptr->id));
                rv = BCM_E_CONFIG;
            } else {
                /* Everything looks good, retrieve the data */
                rv = soc_sbx_g2p3_oamep_get(unit, ep_data->loss_idx, &oamep);
                OAM_VERB((_SBX_D(unit, "oamep_get 0x%04x returned %d %s\n"),
                          ep_data->loss_idx, rv, bcm_errmsg(rv)));

                if (BCM_SUCCESS(rv)) {
                    loss_ptr->loss_nearend  = oamep.framelossnear;
                    loss_ptr->loss_farend   = oamep.framelossfar;
                    loss_ptr->remote_id     = lm_data->peer_idx;
                    loss_ptr->flags         = lm_data->flags;

                    if (loss_ptr->flags & BCM_OAM_LOSS_SINGLE_ENDED) {
                        loss_ptr->tx_nearend    = oamep.txfcf;
                        loss_ptr->rx_nearend    = oamep.rxfcl;
                        loss_ptr->tx_farend     = oamep.txfcb;
                        loss_ptr->rx_farend     = oamep.rxfc;
                    } else { /* DUAL-ENDED */
                        loss_ptr->tx_nearend    = oamep.txfcb;
                        loss_ptr->rx_nearend    = oamep.rxfcl;
                        loss_ptr->tx_farend     = oamep.txfcf;
                        loss_ptr->rx_farend     = oamep.rxfc;
                    }
                }
            }
        }
        
        OAM_UNLOCK(unit);
    }
    
    return rv;
}

int
bcm_fe2000_oam_loss_delete(int unit, bcm_oam_loss_t *loss_ptr)
{
    
    oam_sw_hash_data_t   *ep_data;
    oam_sw_hash_data_t   *lm_data;
    soc_sbx_g2p3_oamep_t  loss_ep;
    soc_sbx_g2p3_oamepremap_t  oamep_remap;
    soc_sbx_g2p3_oamep_t       oamep;
    int                   rv = BCM_E_NONE;
    egr_path_desc_t       loss_egr_path;
    int                   single_ended;

    ep_data = lm_data = NULL;

    OAM_DEBUG((_SBX_D(unit, "Entering OAM Loss Delete for Loss_IDX = 0x%08x\n"),
               loss_ptr->id));

    if (!BCM_OAM_IS_INIT(unit)) {
        rv = BCM_E_INIT;

    } else if (loss_ptr->flags & ~SUPPORTED_LOSS_FLAGS) {
        OAM_ERR((_SBX_D(unit, "Invalid flags: 0x%08x supported=0x%08x\n"),
                 loss_ptr->flags, SUPPORTED_LOSS_FLAGS));
        rv = BCM_E_PARAM;

    } else if (!ENDPOINT_ID_VALID(unit, loss_ptr->id) ||
               !ENDPOINT_ID_VALID(unit, loss_ptr->remote_id)) {

        OAM_ERR((_SBX_D(unit, "Invalid endpoint local=0x%08x %s  "
                        "remote=0x%08x %s\n"), 
                 loss_ptr->id, 
                 (ENDPOINT_ID_VALID(unit, loss_ptr->id) ?
                  "valid" : "invalid"),
                 loss_ptr->remote_id,
                 (ENDPOINT_ID_VALID(unit, loss_ptr->remote_id) ?
                  "valid" : "invalid")));
        rv = BCM_E_PARAM;
    }
    
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    single_ended = ( (loss_ptr->flags & BCM_OAM_LOSS_SINGLE_ENDED) ? TRUE : FALSE);

    
    OAM_LOCK(unit);

    /* get the local (CCM) endpoint sw state */ 
    ep_data = &_state[unit]->hash_data_store[loss_ptr->id];

    if (!ENDPOINT_ID_VALID(unit, ep_data->rec_idx) ||
        (ep_data->flags & BCM_OAM_ENDPOINT_REMOTE) ||
        (ep_data->oam_type != oamEp && !OAM_IS_ENDPOINT_MPLS_Y1731(ep_data->oam_type))) {
        OAM_ERR((_SBX_D(unit, "Local endpoint not found at endpoint id: 0x%08x\n"), 
                 loss_ptr->id));
            
        rv = BCM_E_PARAM;
    }


    if (BCM_SUCCESS(rv) && OAM_IS_ENDPOINT_MPLS_Y1731(ep_data->oam_type)) {
        /* delete mpls endpoint */
        rv = _bcm_fe2000_mpls_oam_loss_set(unit, loss_ptr, ep_data, 0);
        OAM_UNLOCK(unit);
        return rv;
    } 


    /* Ethernet endpoint delete */
    /* validate loss measurement 'endpoint' */
    if (BCM_SUCCESS(rv)) {
        lm_data = &_state[unit]->hash_data_store[ep_data->loss_idx];
        
        if (!ENDPOINT_ID_VALID(unit, ep_data->loss_idx) ||
            (lm_data->oam_type != oamLoss)) {
            OAM_ERR((_SBX_D(unit, "Loss measurement not enabled on endpoint id:"
                            " 0x%08x\n"), loss_ptr->id));
            rv = BCM_E_PARAM;
        } else {
            
            rv = soc_sbx_g2p3_oamep_get(unit, ep_data->loss_idx, &loss_ep);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to read oamep[0x%x]: %d %s\n"),
                         ep_data->loss_idx));
                rv = BCM_E_PARAM;
            } else {
                rv = _bcm_ltm_threshold_free(unit, loss_ep.multiplieridx);
            }
        }
    }

    /* validate FTI based on LM type, then free the egress path for SE LM */
    if (BCM_SUCCESS(rv)) {
        if ((single_ended == TRUE) &&
            (loss_ep.ftidx_store == 0)) {
            rv = BCM_E_CONFIG;
            OAM_ERR((_SBX_D(unit, "No fti found on SE LM\n")));
        }
        if ((single_ended == FALSE) &&
            loss_ep.ftidx_store != 0) {
            rv = BCM_E_CONFIG;            
            OAM_ERR((_SBX_D(unit, "fti found on DE LM\n")));
        }
    }    

    /* clear the coherent counter idx in the lp, oi & evp2e
     * BEFORE freeing the egrPath (CoCo-config uses EgrPath).
     */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_oam_enet_coco_configure(unit, ep_data, 0, 0, single_ended);
    }

    if (BCM_SUCCESS(rv) &&
        (single_ended == TRUE)) {
        rv = _oam_egr_path_get(unit, &loss_egr_path, 
                               ep_data->loss_idx);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get SE LM[0x%04x] "
                            "egress path: %s\n"),
                     ep_data->loss_idx, bcm_errmsg(rv)));
        }
        
        if (BCM_SUCCESS(rv)) {
            rv = _oam_egr_path_free(unit, &loss_egr_path);
        }
    }


    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_oamep_get(unit, ep_data->rec_idx, &oamep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%04x\n"),
                     ep_data->rec_idx));
        }
    }
    
    /* clear out the remap table for LMMs (SE LM) */
    if (BCM_SUCCESS(rv)) {
        if (single_ended == TRUE) {

            soc_sbx_g2p3_oamepremap_t_init(&oamep_remap);            
            rv = soc_sbx_g2p3_oamepremap_set(unit, 
                                             ep_data->rec_idx, oamep.mdlevel,
                                             _state[unit]->ep_subtypes.lm,
                                             &oamep_remap);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to clear oamep remap"
                                "[0x%04x,0x%1x,%d]: %s\n"),
                         ep_data->rec_idx, oamep.mdlevel, 
                         _state[unit]->ep_subtypes.lm, bcm_errmsg(rv)));
            }
        }
    }
            


    /* Single-ended - free timer calendar entry
     * Dual-ended - clear the linkage between local ep and the lm
     */
    if (BCM_SUCCESS(rv)) {
        if (lm_data->flags & BCM_OAM_LOSS_SINGLE_ENDED) {
            if (_BCM_TCAL_ID_VALID(lm_data->tcal_id)) {
                _bcm_tcal_free(unit, &lm_data->tcal_id);
                OAM_VERB((_SBX_D(unit, "(error %d) Freeing tcalId 0x%x\n"),
                          rv, lm_data->tcal_id));
            } else {
                OAM_ERR((_SBX_D(unit, "Invalid timer calendar entry foudn on LM in"
                                "single-ended mode\n")));
                rv = BCM_E_CONFIG;
            }

        } else {
            soc_sbx_g2p3_oamep_t oamep;

            rv = soc_sbx_g2p3_oamep_get(unit, ep_data->rec_idx, &oamep);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%04x\n"),
                         ep_data->rec_idx));
            }

            oamep.nextentry = 0;

            if (BCM_SUCCESS(rv)) {
                rv = soc_sbx_g2p3_oamep_set(unit, ep_data->rec_idx, &oamep);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed to set oamep 0x%04x\n"),
                             ep_data->rec_idx));
                }
            }
        }
    }

    if (BCM_SUCCESS(rv)) {
        rv = shr_idxres_list_free(_state[unit]->coco_pool,
                                  OAM_COCO_RES(lm_data->cocounter));
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to free coherent counter 0x%x: %d %s\n"),
                     lm_data->cocounter, rv, bcm_errmsg(rv)));
        }
    }

    if (BCM_SUCCESS(rv)) {
        soc_sbx_g2p3_oamep_t_init(&loss_ep);
        rv = soc_sbx_g2p3_oamep_set(unit, lm_data->loss_idx, &loss_ep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to clear LM[0x%x]:%d %s\n"),
                     lm_data->loss_idx, rv, bcm_errmsg(rv)));
        } else {
            _oam_hash_data_clear(lm_data);
            ep_data->loss_idx = INVALID_RECORD_INDEX;
        }
    }

    if (BCM_SUCCESS(rv)) {
        OAM_VERB((_SBX_D(unit, "Removed loss measurement from EP %d: %d %s\n"),
                  loss_ptr->id, rv, bcm_errmsg(rv)));
    }
    
    OAM_UNLOCK(unit);


    return rv;
}


/*
 *   Function
 *      bcm_fe2000_oam_delay_add
 *   Purpose
 *      Add delay measurement object to an existing local endpoint
 *   Parameters
 *       unit        = BCM device number
 *       delay_ptr   = delay object to add
 *   Returns
 *       BCM_E_*
 */
int
bcm_fe2000_oam_delay_add(int unit, bcm_oam_delay_t *delay_ptr)
{
    oam_sw_hash_data_t *local_ep, *peer_ep;
    oam_sw_hash_data_t *dm_data;
    egr_path_desc_t     egr_path;
    uint32_t            delay_idx;
    tcal_id_t           tcal_id;
    int                 update;
    int                 rv = BCM_E_NONE;
    soc_sbx_g2p3_oamepremap_t  oamep_remap;

    if (!BCM_OAM_IS_INIT(unit)) {
        return BCM_E_INIT;
    }
    if (delay_ptr == NULL) {
        return BCM_E_PARAM;
    }

    if (delay_ptr->period > OAM_SBX_MAX_PERIOD) {
        OAM_ERR((_SBX_D(unit, "Maximim period supported by this device is %dms\n"),
                 OAM_SBX_MAX_PERIOD));
        return BCM_E_PARAM;
    }

    delay_idx = INVALID_RECORD_INDEX;
    tcal_id   = _BCM_TCAL_INVALID_ID;

    local_ep = peer_ep = dm_data = NULL;

    if (delay_ptr->flags & ~SUPPORTED_DELAY_FLAGS) {
        OAM_ERR((_SBX_D(unit, "Invalid flags: 0x%08x supported=0x%08x\n"),
                 delay_ptr->flags, SUPPORTED_DELAY_FLAGS));
        return BCM_E_PARAM;
    }
    
    if (!ENDPOINT_ID_VALID(unit, delay_ptr->id)) {
        OAM_ERR((_SBX_D(unit, "Invalid local endpoint id: 0x%04x\n"), 
                 delay_ptr->id));
        return BCM_E_PARAM;
    }
    if (!ENDPOINT_ID_VALID(unit, delay_ptr->remote_id)) {
        OAM_ERR((_SBX_D(unit, "Invalid remote endpoint id: 0x%04x\n"), 
                 delay_ptr->remote_id));
        return BCM_E_PARAM;
    }

    /* get the local endpoint sw state */ 
    OAM_LOCK(unit);
    local_ep = &_state[unit]->hash_data_store[delay_ptr->id];
    peer_ep =  &_state[unit]->hash_data_store[delay_ptr->remote_id];

    _oam_egr_path_init(unit, &egr_path);

    if (!ENDPOINT_ID_VALID(unit, local_ep->rec_idx) ||
        (local_ep->flags & BCM_OAM_ENDPOINT_REMOTE) ||
        (local_ep->oam_type != oamEp &&
         !OAM_IS_ENDPOINT_MPLS_Y1731(local_ep->oam_type))) {
        OAM_ERR((_SBX_D(unit, "Local endpoint not found at endpoint id: 0x%04x\n"),
                 delay_ptr->id));

        rv = BCM_E_PARAM;

    } else if (!ENDPOINT_ID_VALID(unit, peer_ep->rec_idx) ||
        !(peer_ep->flags & BCM_OAM_ENDPOINT_REMOTE) ||
        (peer_ep->oam_type != oamEp && 
         !OAM_IS_ENDPOINT_MPLS_Y1731(peer_ep->oam_type))) {
        OAM_ERR((_SBX_D(unit, "Remote endpoint not found at endpoint id: 0x%04x\n"),
                 delay_ptr->remote_id));

        rv = BCM_E_PARAM;

    } else {
        if(OAM_IS_ENDPOINT_MPLS_Y1731(local_ep->oam_type)) {
            rv = _bcm_fe2000_mpls_oam_delay_set (unit, delay_ptr, local_ep, peer_ep, TRUE);

        } else {
            if (delay_ptr->period < 0) {
                /* Loss Period of (-1) signifies single on-demand transmission 
                 * for mpls 1731 oam. Not supported on other endpoints */
                OAM_ERR((_SBX_D(unit, "Invalid delay measurement period \n")));
                rv = BCM_E_PARAM;
                
            }
            /* An update/replace is when the delay_idx already exists for the endpoint
             */
            update = ENDPOINT_ID_VALID(unit, local_ep->delay_idx);

            /* Everything looks good, allocate a new record if not already available */
            if (BCM_SUCCESS(rv)) {
                if (update) {
                    OAM_VERB((_SBX_D(unit, "delay record 0x%04x found to exist; re-using\n"),
                              local_ep->delay_idx));
                    
                    delay_idx = local_ep->delay_idx;
                    dm_data = &_state[unit]->hash_data_store[delay_idx];
                    
                    if (!_BCM_TCAL_ID_VALID(dm_data->tcal_id)) {
                        OAM_ERR((_SBX_D(unit, "Invalid timer calendar found in delay state; "
                                        "idx=0x%04x\n"), delay_idx));
                        rv = BCM_E_INTERNAL;
                    }

                    if (BCM_SUCCESS(rv)) {
                        tcal_id = dm_data->tcal_id;
                    }
                    
                } else {
                    rv = shr_idxres_list_alloc(_state[unit]->ep_pool, &delay_idx);
                    if (BCM_FAILURE(rv)) {
                        OAM_ERR((_SBX_D(unit, "Failed to allocate an delay record: %d %s\n"),
                                 rv, bcm_errmsg(rv)));

                    } else {
                        OAM_VERB((_SBX_D(unit, "Allocated delay record 0x%04x\n"), 
                                  delay_idx));

                        dm_data = &_state[unit]->hash_data_store[delay_idx];
                    }
                }
            }

            if (BCM_SUCCESS(rv)) {
                if (update) {
                    rv = _oam_egr_path_get(unit, &egr_path, delay_idx);
                    if (BCM_FAILURE(rv)) {
                        OAM_ERR((_SBX_D(unit, "Failed to get egress path for delay ep 0x%04x"
                                        ": %d %s\n"), delay_idx, rv, bcm_errmsg(rv)));
                    }
                } else {

                    /* create the egress path with local ep as smac, 
                     * and remote ep as dmac
                     */
                    if (local_ep->flags & BCM_OAM_ENDPOINT_UP_FACING) {
                        /* up MEP uses peer's gport for FTE.QID */
                        rv = _oam_egr_path_clone_upmep(unit, &egr_path, peer_ep->gport,
                                                       local_ep->vid_label, local_ep->mac_address);
                    } else {
                        rv = _oam_egr_path_clone_downmep(unit, &egr_path, local_ep->rec_idx);
                    }
                }

                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "failed to %s an egress path for "
                                    "DM: %s\n"), 
                             (update)? "get" : "allocate", bcm_errmsg(rv)));
                }


                if (BCM_SUCCESS(rv)) {
                    rv = _oam_egr_path_dmac_set(unit, &egr_path, 
                                                peer_ep->mac_address, TRUE);

                    if (BCM_FAILURE(rv)) {
                        OAM_ERR((_SBX_D(unit, "failed to update dmac for DM: %s\n"),
                                 bcm_errmsg(rv)));
                    }
                }

                if (BCM_SUCCESS(rv)) {

                    rv = _oam_egr_path_commit(unit, &egr_path);
                    if (BCM_FAILURE(rv)) {
                        OAM_ERR((_SBX_D(unit, "Failed to commit egress path: %s\n"),
                                 bcm_errmsg(rv)));
                    }
                }
            }


            /* set the basic delay hw data  */
            if (BCM_SUCCESS(rv)) {
                rv = _bcm_fe2000_oam_enet_dm_create(unit, delay_ptr->flags, 
                                                    delay_idx, local_ep->rec_idx, egr_path.ftIdx);
            }

            /* set the delay measurement transmission period, and optionally enable */
            if (BCM_SUCCESS(rv)) {

                int enable = !!(delay_ptr->flags & BCM_OAM_DELAY_TX_ENABLE);
                tcal_id_t tmp_tcal = tcal_id;

                if (_BCM_TCAL_ID_VALID(tcal_id)) {
                    rv = _bcm_tcal_update(unit, delay_ptr->period, enable, delay_idx,
                                          &tmp_tcal);
                } else {
                    rv = _bcm_tcal_alloc(unit, delay_ptr->period, enable, delay_idx,
                                         &tmp_tcal);
                }
                
                if (BCM_SUCCESS(rv)) {
                    OAM_VERB((_SBX_D(unit, "%s timer calender entry 0x%05x\n"),
                              _BCM_TCAL_ID_VALID(tcal_id)?"Updated":"Allocated",
                              tmp_tcal));
                } else {
                    OAM_ERR((_SBX_D(unit, "Failed to %s tcal id\n"),
                             _BCM_TCAL_ID_VALID(tcal_id)?"update":"allocate"));
                }
                tcal_id = tmp_tcal;
            }

            /* Set up the remap table to redirect DMMs to the correct OAM endpoint */
            if (BCM_SUCCESS(rv)) {
                soc_sbx_g2p3_oamepremap_t_init(&oamep_remap);
                oamep_remap.epIdx = delay_idx;

                rv = soc_sbx_g2p3_oamepremap_set(unit, 
                                                 local_ep->rec_idx,  local_ep->mdlevel,
                                                 _state[unit]->ep_subtypes.dm,
                                                 &oamep_remap);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed to set oamep remap"
                                    "[0x%04x,0x%1x,%d]: %s\n"),
                             local_ep->rec_idx, local_ep->mdlevel, 
                             _state[unit]->ep_subtypes.dm, bcm_errmsg(rv)));
                }
            }
        
            /* Update software state */
            if (BCM_SUCCESS(rv)) {
                /* link endpoint to delay data to recognize 'replace' on next add */
                local_ep->delay_idx = delay_idx;
                dm_data->tcal_id    = tcal_id;
                dm_data->rec_idx    = delay_idx;
                dm_data->peer_idx   = delay_ptr->remote_id;
                dm_data->flags      = delay_ptr->flags;
                dm_data->oam_type   = oamDelay;
            }

            if (BCM_FAILURE(rv) && !update) {
                /* If the delay idx was allocated for the first time,
                 * free it on error
                 */
                if (ENDPOINT_ID_VALID(unit, delay_idx)) {
                    shr_idxres_list_free(_state[unit]->ep_pool, delay_idx);
                    OAM_VERB((_SBX_D(unit, "(error %d) Freeing delay ep 0x%x\n"),
                              rv, delay_idx));

                    if (_BCM_TCAL_ID_VALID(tcal_id)) {
                        _bcm_tcal_free(unit, &tcal_id);
                        OAM_VERB((_SBX_D(unit, "(error %d) Freeing tcalId 0x%x\n"),
                                  rv, tcal_id));
                    }
                }

                _oam_egr_path_free(unit, &egr_path);
            }
        }
    }
    
    OAM_UNLOCK(unit);
    return rv;
}


int
bcm_fe2000_oam_delay_get(int unit, bcm_oam_delay_t *delay_ptr)
{
    oam_sw_hash_data_t  *ep_data;
    oam_sw_hash_data_t  *dm_data;
    int                  rv = BCM_E_NONE;
    soc_sbx_g2p3_oamep_t oamep;
    bcm_time_spec_t      delay, remote;

    if (!BCM_OAM_IS_INIT(unit)) {
        return BCM_E_INIT;
    }

    ep_data = dm_data = NULL;

    if (delay_ptr->flags & ~SUPPORTED_DELAY_FLAGS) {
        OAM_ERR((_SBX_D(unit, "Invalid flags: 0x%08x supported=0x%08x\n"),
                 delay_ptr->flags, SUPPORTED_DELAY_FLAGS));
        return BCM_E_PARAM;
    }

    if (!ENDPOINT_ID_VALID(unit, delay_ptr->id)) {
        OAM_ERR((_SBX_D(unit, "Invalid local endpoint id: 0x%04x\n"), 
                 delay_ptr->id));
        return BCM_E_PARAM;
    }

    /* get the local endpoint sw state */ 
    OAM_LOCK(unit);

    /* validate local endpoint */
    ep_data = &_state[unit]->hash_data_store[delay_ptr->id];

    if (!ENDPOINT_ID_VALID(unit, ep_data->rec_idx) ||
        (ep_data->flags & BCM_OAM_ENDPOINT_REMOTE) ||
        (ep_data->oam_type != oamEp)) {
        OAM_ERR((_SBX_D(unit, "Local endpoint not found at endpoint id: 0x%04x\n"), 
                 delay_ptr->id));

        rv = BCM_E_PARAM;
    }

    /* validate delay measurement 'endpoint' */
    dm_data = &_state[unit]->hash_data_store[ep_data->delay_idx];

    if (!ENDPOINT_ID_VALID(unit, ep_data->delay_idx) ||
        (dm_data->oam_type != oamDelay)) {
        OAM_ERR((_SBX_D(unit, "Delay measurement not enabled on endpoint id:"
                        " 0x%04x\n"), delay_ptr->id));
        rv = BCM_E_PARAM;
    }

    /* Everything looks good, get the necessary data */
    soc_sbx_g2p3_oamep_t_init(&oamep);
    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_oamep_get(unit, dm_data->rec_idx, &oamep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to read oamep at idx 0x%04x\n"),
                     dm_data->rec_idx));
        }

        OAM_VERB((_SBX_D(unit, "Read oamep idx 0x%04x rv=%d\n"), 
                  dm_data->rec_idx, rv));
    }

    if (BCM_SUCCESS(rv)) {
        
        delay_ptr->period = 0;
        delay_ptr->remote_id = dm_data->peer_idx;
        delay_ptr->flags  = dm_data->flags;

        if (dm_data->flags & BCM_OAM_DELAY_ONE_WAY) {
            /* for one-way mode, put the ucode's delay seconds and nanoseconds
             * directly into a bcm_time_spec struct.  For one-way mode, the delay
             * value can be negative if the clocks are not synced.
             */
            rv = convert_ep_to_time_spec( &(delay_ptr->delay), oamep.delaysec, oamep.delaynanosec );
            OAM_VERB((_SBX_D(unit, "One-way total delay: %d second, %d nanoseconds\n"),
                      oamep.delaysec, oamep.delaynanosec));

        } else {
            /* if two-way mode, then subtract out the optional remote processing time. */
            OAM_VERB((_SBX_D(unit, "Two-way round-trip delay: %d second, %d nanoseconds\n"),
                      oamep.delaysec, oamep.delaynanosec));
            OAM_VERB((_SBX_D(unit, "Two-way remote-processing delay: %d second, %d nanoseconds\n"),
                      oamep.remotesec, oamep.remotenanosec));

            /* put the ucode's delay seconds and nanoseconds into a bcm_time_spec struct. */
            rv = convert_ep_to_time_spec(&delay, oamep.delaysec, oamep.delaynanosec);

            /* put the ucode's remote seconds and nanoseconds into a bcm_time_spec struct. */
            rv = convert_ep_to_time_spec(&remote, oamep.remotesec, oamep.remotenanosec);

            /* in two-way mode, neither delay nor remote should be negative. */
            if (delay.isnegative || remote.isnegative) {
                OAM_VERB((_SBX_D(unit, "Two-way delay or remote processing is negative")));
                rv = BCM_E_INTERNAL;
            }

            /* subtract off the remote processing time from total delay. */
            rv = time_spec_subtract( &(delay_ptr->delay), &delay, &remote );

            OAM_VERB((_SBX_D(unit, "Two-way total delay: %u seconds, %u nanoseconds\n"),
                      delay_ptr->delay.seconds, delay_ptr->delay.nanoseconds));

            /* Total delay (after subtracting out the remote processing) should not be negative. */
            if (delay_ptr->delay.isnegative) {
                OAM_VERB((_SBX_D(unit, "Two-way total delay is negative")));
                rv = BCM_E_INTERNAL;
            }
        }
    }
    
    OAM_UNLOCK(unit);
    return rv;

}


/* The seconds and nanoseconds stored in the endpoint entry are
 * the result of a simple subtraction of sec & ns values.  There
 * is no "borrow" if the nanoseconds value goes negative.  This
 * Function handles the borrow, and fills in the BCM_time_spec
 * structure.  This function is specific to the BCM88025's OAM
 * Endpoint Entry (type=DM).
 */
int
convert_ep_to_time_spec(bcm_time_spec_t* bts, int sec, int ns)
{
    int rv = BCM_E_NONE;

    if (bts != NULL) {
        /* if both seconds and nanoseconds are negative or both positive,
         * then the ucode's subtraction is ok.
         * if seconds and nanoseconds have different signs, then "borrow"
         * 1000000000 nanoseconds from seconds.
         */
        if ((sec < 0) && (ns > 0)) {
            ns -= 1000000000;
            sec += 1;
        } else if ((sec > 0) && (ns < 0)) {
            ns += 1000000000;
            sec -= 1;
        }

        if (ns < 0) {
            /* if still negative, then something else is wrong.
             * the nanoseconds field is the difference between two
             * (non-negative) time-stamps.
             */
            rv = BCM_E_INTERNAL;
        }

        /* if seconds is negative then set the bts is-negative flag,
         * and use the absolute value of seconds & nanoseconds.
         */
        bts->isnegative  = (sec < 0 ? 1 : 0);
        bts->seconds     = abs(sec);
        bts->nanoseconds = abs(ns);
    } else {
        rv = BCM_E_INTERNAL;
    }

    return rv;
}

/* d (difference) = m (minuend) - s (subtrahend) */
int
time_spec_subtract(bcm_time_spec_t* d, bcm_time_spec_t* m, bcm_time_spec_t* s)
{
    int rv = BCM_E_NONE;
    int32_t d_ns = 0;
    int32_t d_s = 0;

    if ((d != NULL) && (m != NULL) && (s != NULL)) {
        /* subtract the nanoseconds first, then borrow is necessary. */
        d_ns = m->nanoseconds - s->nanoseconds;
        if (d_ns < 0) {
            m->seconds = m->seconds - 1;
            d_ns = 1000000000 + d_ns;
        }
        if (d_ns < 0) {
            /* if still negative, then error */
            rv = BCM_E_INTERNAL;
            d_ns = abs(d_ns);
        }
        d->nanoseconds = d_ns;

        /* subtract the seconds next, check for negative. */
        d_s = m->seconds - s->seconds;
        if (d_s < 0) {
            d->isnegative = TRUE;
            d_s = abs(d_s);
        } else {
            d->isnegative = FALSE;
        }
        d->seconds = d_s;

    } else {
        rv = BCM_E_INTERNAL;
    }

    return rv;
}


int
bcm_fe2000_oam_delay_delete(int unit, bcm_oam_delay_t *delay_ptr)
{
    oam_sw_hash_data_t *ep_data, *peer_ep;
    oam_sw_hash_data_t *dm_data;
    egr_path_desc_t    egr_path;
    int                 rv;
    soc_sbx_g2p3_oamep_t oamep;
    soc_sbx_g2p3_oamepremap_t  oamep_remap;

    if (!BCM_OAM_IS_INIT(unit)) {
        return BCM_E_INIT;
    }

    ep_data = dm_data = NULL;

    if (delay_ptr->flags & ~SUPPORTED_DELAY_FLAGS) {
        OAM_ERR((_SBX_D(unit, "Invalid flags: 0x%08x supported=0x%04x\n"),
                 delay_ptr->flags, SUPPORTED_DELAY_FLAGS));
        return BCM_E_PARAM;
    }

    if (!ENDPOINT_ID_VALID(unit, delay_ptr->id) ||
        !ENDPOINT_ID_VALID(unit, delay_ptr->remote_id)) {
        OAM_ERR((_SBX_D(unit, "Invalid local endpoint id: 0x%04x\n"), 
                 delay_ptr->id));
        return BCM_E_PARAM;
    }

    /* get the local endpoint sw state */ 
    OAM_LOCK(unit);

    /* validate local endpoint */
    ep_data = &_state[unit]->hash_data_store[delay_ptr->id];
    peer_ep =  &_state[unit]->hash_data_store[delay_ptr->remote_id];

    if (!ENDPOINT_ID_VALID(unit, ep_data->rec_idx) ||
        (ep_data->flags & BCM_OAM_ENDPOINT_REMOTE) ||
        (ep_data->oam_type != oamEp && 
         !OAM_IS_ENDPOINT_MPLS_Y1731(ep_data->oam_type))) {
        OAM_ERR((_SBX_D(unit, "Local endpoint not found at endpoint id: 0x%04x\n"), 
                 delay_ptr->id));
        rv = BCM_E_PARAM;

    } else if (!ENDPOINT_ID_VALID(unit, peer_ep->rec_idx) ||
        !(peer_ep->flags & BCM_OAM_ENDPOINT_REMOTE) ||
        (peer_ep->oam_type != oamEp && 
         !OAM_IS_ENDPOINT_MPLS_Y1731(peer_ep->oam_type))) {
        OAM_ERR((_SBX_D(unit, "Remote endpoint not found at endpoint id: 0x%04x\n"),
                 delay_ptr->remote_id));

        rv = BCM_E_PARAM;

    } else {
        if(OAM_IS_ENDPOINT_MPLS_Y1731(ep_data->oam_type)) {
            rv = _bcm_fe2000_mpls_oam_delay_set(unit, delay_ptr, ep_data, peer_ep, FALSE);
        } else {

            /* validate delay measurement 'endpoint' */
            dm_data = &_state[unit]->hash_data_store[ep_data->delay_idx];

            if (!ENDPOINT_ID_VALID(unit, ep_data->delay_idx) ||
                (dm_data->oam_type != oamDelay)) {
                OAM_ERR((_SBX_D(unit, "Delay measurement not enabled on endpoint id:"
                                " 0x%04x\n"), delay_ptr->id));
                OAM_UNLOCK(unit);
                return BCM_E_PARAM;
            }

    
            /* clear out the remap table for DMMs */
            soc_sbx_g2p3_oamepremap_t_init(&oamep_remap);            
            rv = soc_sbx_g2p3_oamepremap_set(unit, 
                                             ep_data->rec_idx, ep_data->mdlevel,
                                             _state[unit]->ep_subtypes.dm,
                                             &oamep_remap);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to clear oamep remap"
                                "[0x%04x,0x%1x,%d]: %s\n"),
                         ep_data->rec_idx, ep_data->mdlevel, 
                         _state[unit]->ep_subtypes.dm, bcm_errmsg(rv)));
            }
            
            /* Everything looks good, free resources */
            rv = _oam_egr_path_get(unit, &egr_path, dm_data->rec_idx);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to get egress path for oamep[0x%04x]: "
                                "%d %s\n"), dm_data->rec_idx, rv, bcm_errmsg(rv)));
            } else {
                _oam_egr_path_free(unit, &egr_path);
            }

            OAM_VERB((_SBX_D(unit, "Clearing and freeing oamep idx 0x%04x\n"), 
                      dm_data->rec_idx));

            soc_sbx_g2p3_oamep_t_init(&oamep);
            rv = soc_sbx_g2p3_oamep_set(unit, dm_data->rec_idx, &oamep);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to clear read oamep at idx 0x%04x\n"),
                         dm_data->rec_idx));
                rv = BCM_E_NONE; /* keep trying */
            }
            
            rv = shr_idxres_list_free(_state[unit]->ep_pool, dm_data->rec_idx);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to clear free ep idx 0x%04x\n"),
                         dm_data->rec_idx));
            }

            if (BCM_SUCCESS(rv)) {
                _oam_hash_data_clear(dm_data);
                ep_data->delay_idx = INVALID_RECORD_INDEX;
            }
        }
    }
    OAM_UNLOCK(unit);
    return rv;
}

#endif  /* INCLUDE_L3 */
