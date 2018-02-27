/*
 * $Id: mpls.h 1.10 Broadcom SDK $
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
 */
#ifndef _BCM_INT_SBX_FE2000_OAM_MPLS_H_
#define _BCM_INT_SBX_FE2000_OAM_MPLS_H_

#define OAM_IS_MPLS_Y1731(t) ((t) == bcmOAMEndpointTypeMPLSPerformance)

#define OAM_MPLS_DEFAULT_TTL (0xff)
#define OAM_MPLS_DEFAULT_EXP (0) /* since control traffic set exp to 0 */
#define OAM_MPLS_LSP_GAL_LABEL (13)

#define _BCM_FE2000_MPLS_SUPPORTED_EPTYPE (bcmOAMEndpointTypeMPLSPerformance)

typedef enum bcm_fe2000_mpls_tunnel_direction_e {
    _FE2K_OUTGOING_LSP,
    _FE2K_INCOMING_LSP,
    _FE2K_MAX_DIRECTION
} bcm_fe2000_mpls_tunnel_direction_t;

extern int
_bcm_fe2000_get_tunnel_interface_info(int unit, 
                                      bcm_if_t l3_egr_ifid,
                                      bcm_fe2000_mpls_tunnel_direction_t direction);

extern int _bcm_fe2000_validate_oam_mpls_endpoint(int unit, 
                                                  bcm_oam_endpoint_info_t *endpoint_info);

extern int _oam_mpls_egr_path_update (int unit,
                                      egr_path_desc_t *egrPath,  
                                      bcm_oam_endpoint_info_t *endpoint_info);

extern int _oam_mpls_endpoint_set(int unit,
                                  bcm_oam_endpoint_info_t *ep_info, 
                                  uint32_t ep_rec_index, 
                                  egr_path_desc_t *egrPath,
                                  tcal_id_t *tcal_id) ;

extern int _oam_mpls_endpoint_get(int unit,
                                  bcm_oam_endpoint_info_t *ep_info, 
                                  uint32_t ep_rec_index);

extern int _oam_mpls_endpoint_delete(int unit, 
                                     bcm_oam_endpoint_info_t *ep_info);

/*
 *   Function
 *     _bcm_fe2000_mpls_oam_loss_set
 *   Purpose
 *      Provision Delay measurement on MPLS LSP OAM endpoint
 *   Parameters
 *       unit        = BCM device number
 *       delay_ptr   = delay parameter
 *       local_ep    = mpls oam local endpoint datum
 *       add_lm_endpoint = TRUE(add loss measurement)
 *                         FALSE(delete loss measurement)
 *   Returns
 *       BCM_E_*
 */
extern  int 
_bcm_fe2000_mpls_oam_loss_set(int unit, 
                              bcm_oam_loss_t *loss_ptr,
                              oam_sw_hash_data_t *local_ep,
                              uint8_t add_lm_endpoint);

/*
 *   Function
 *     _bcm_fe2000_mpls_oam_delay_set
 *   Purpose
 *      Provision Delay measurement on MPLS LSP OAM endpoint
 *   Parameters
 *       unit        = BCM device number
 *       delay_ptr   = delay parameter
 *       local_ep    = mpls oam local endpoint datum
 *       peer_dp     = mpls peer oam endpoint datum
 *       add_dm_endpoint = TRUE(add delay measurement)
 *                         FALSE(delete delay measurement)
 *   Returns
 *       BCM_E_*
 */
extern 
int _bcm_fe2000_mpls_oam_delay_set(int unit, 
                                   bcm_oam_delay_t *delay_ptr,
                                   oam_sw_hash_data_t *local_ep,
                                   oam_sw_hash_data_t *peer_ep,
                                   uint8_t add_dm_endpoint); 

extern int
_bcm_fe2000_lsp_intf_ep_list_add(int unit, 
                                 bcm_if_t l3_egr_ifid,
                                 oam_sw_hash_data_t *hash_data);

int
extern _bcm_fe2000_lsp_intf_ep_list_remove(int unit, 
                                           bcm_if_t l3_egr_ifid,
                                           oam_sw_hash_data_t *hash_data);

#endif  /* _BCM_INT_SBX_FE2000_OAM_MPLS_H_  */
