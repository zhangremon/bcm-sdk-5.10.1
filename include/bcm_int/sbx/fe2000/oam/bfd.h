/*
 * $Id: bfd.h 1.5 Broadcom SDK $
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
#ifndef _BCM_INT_SBX_FE2000_BFD_H_
#define _BCM_INT_SBX_FE2000_BFD_H_

/* BFD currently only supports point to point bidirecitonal BFD */
#define OAM_BFD_MAX_FE2000_ENDPOINT (2)

#define OAM_IS_LSP_BFD(t)  ((t) == bcmOAMEndpointTypeBFDMPLS              || \
                            (t) == bcmOAMEndpointTypeBFDMPLSCcCv)

#define OAM_IS_BFD(t)   ((t) == bcmOAMEndpointTypeBFDVccvCcCv          || \
                         (t) == bcmOAMEndpointTypeBFDVccv )


#define _BCM_FE2000_BFD_SUPPORTED_FLAGS (BCM_OAM_BFD_CV_TYPE_RAW | \
                                        BCM_OAM_BFD_CV_TYPE_IP  |\
                                        BCM_OAM_BFD_POLL | BCM_OAM_BFD_FINAL |\
                                        BCM_OAM_BFD_CONTROL_INDEPENDENT |\
                                        BCM_OAM_BFD_MULTIPOINT | \
                                        BCM_OAM_BFD_UNIDIRECTIONAL)

#define _BCM_FE2000_BFD_SUPPORTED_EPTYPE (bcmOAMEndpointTypeBFDVccv | \
                                          bcmOAMEndpointTypeBFDMPLS)

extern int _bcm_fe2000_validate_oam_lsp_bfd_endpoint(int unit, 
                                                 bcm_oam_endpoint_info_t *endpoint_info);
extern int _bcm_fe2000_validate_oam_bfd_endpoint(int unit, 
                                                 bcm_oam_endpoint_info_t *endpoint_info);

extern int _oam_bfd_endpoint_set(int unit,
                                 bcm_oam_endpoint_info_t *ep_info, 
                                 uint32_t ep_rec_index, 
                                 egr_path_desc_t *egrPath, 
                                 tcal_id_t *tcal_id,
                                 uint32_t label);

extern int _oam_bfd_endpoint_get(int unit,
                                 bcm_oam_endpoint_info_t *ep_info, 
                                 uint32_t ep_rec_index);
extern int _oam_bfd_endpoint_delete(int unit, 
                                    bcm_oam_endpoint_info_t *ep_info);

extern int _oam_bfd_egr_path_update (int unit,
                                     egr_path_desc_t *egrPath,  
                                     bcm_oam_endpoint_info_t *ep_info,
                                     _fe2k_vpn_sap_t *vpn_sap);

extern int _oam_fe2000_bfd_chain_psc (int      unit, 
                                      uint32_t bfd_ep_index,
                                      uint32_t psc_ep_index,
                                      uint8_t  chain /* 0 -unchain, 1- chain*/);

#endif  /* _BCM_INT_SBX_FE2000_BFD_H_  */
