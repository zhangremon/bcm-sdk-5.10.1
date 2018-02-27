/*
 * $Id: enet.h 1.6 Broadcom SDK $
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
#ifndef _BCM_INT_SBX_FE2000_OAM_ENET_H_
#define _BCM_INT_SBX_FE2000_OAM_ENET_H_

#define OAM_IS_ETHERNET(t) ((t) == bcmOAMEndpointTypeEthernet)

extern int _bcm_fe2000_init_upmep(int unit);

extern int _oam_enet_endpoint_set(int unit, 
                                  bcm_oam_endpoint_info_t *ep_info, 
                                  uint32_t ep_rec_index, 
                                  egr_path_desc_t *egrPath, 
                                  tcal_id_t *tcal_id,
                                  bcm_trunk_add_info_t *trunk_info);

extern int _oam_enet_endpoint_delete(int unit, 
                                     bcm_oam_endpoint_info_t *ep_info,
                                     bcm_trunk_add_info_t *trunk_info);

extern int _bcm_fe2000_validate_oam_eth_endpoint(int unit, 
                                                 bcm_oam_endpoint_info_t *endpoint_info);

extern int _bcm_fe2000_oam_enet_coco_configure(int unit,
                                         oam_sw_hash_data_t *ep_data, 
                                         int coco_idx,
                                         uint8_t pkt_pri_bitmap,
                                         int single_ended);

extern int _bcm_fe2000_oam_enet_dm_create(int unit, int flags, uint16_t dmIdx, 
                                          uint16_t epIdx, uint32_t ftIdx);

#endif  /* _BCM_INT_SBX_FE2000_OAM_ENET_H_  */
