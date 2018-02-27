/*
 * $Id: mpls.c 1.3 Broadcom SDK $
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
 * File:    mpls.c
 * Purpose: Manages MPLS functions
 */

#ifdef INCLUDE_L3

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/mpls.h>

int 
bcm_sbx_mpls_init (int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_mpls_cleanup(int unit)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_info (int unit,
                   bcm_mpls_info_t *info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_t_init (bcm_mpls_vpn_t *vpn_info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_l3_initiator_t_init (bcm_mpls_l3_initiator_t *ftn)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_switch_t_init (bcm_mpls_switch_t *switch_t)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_create (int unit,
                         bcm_vpn_t vpn,
                         uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_destroy (int unit,
                          bcm_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_add (int unit,
                      bcm_vpn_t vpn,
                      bcm_mpls_vpn_t *vpn_info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_delete (int unit,
                         bcm_vpn_t vpn,
                         bcm_mpls_vpn_t *key)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_delete_all (int unit,
                             bcm_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_get (int unit,
                      bcm_vpn_t vpn,
                      int max_out,
                      bcm_mpls_vpn_t *vpn_info,
                      int *real_count)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_circuit_add (int unit,
                              bcm_vpn_t vpn,
                              bcm_mpls_circuit_t *vc_info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_circuit_delete (int unit,
                                 bcm_vpn_t vpn,
                                 bcm_mpls_circuit_t *vc_info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_circuit_delete_all (int unit,
                                     bcm_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_vpn_circuit_get (int unit,
                              bcm_vpn_t vpn,
                              int vc_array,
                              bcm_mpls_circuit_t *vc_info,
                              int *vc_count)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_l3_initiator_set (int unit,
                               bcm_l3_intf_t *intf,
                               bcm_mpls_l3_initiator_t *mpls_label)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_l3_initiator_clear (int unit,
                                 bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_l3_initiator_get (int unit,
                               bcm_l3_intf_t *intf,
                               bcm_mpls_l3_initiator_t *mpls_label)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_l3_initiator_clear_all (int unit)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_switch_add (int unit,
                         bcm_mpls_switch_t *mswitch_info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_switch_delete (int unit,
                            bcm_mpls_switch_t *mswitch_info)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_mpls_switch_get (int unit,
                         bcm_mpls_switch_t *mswitch_info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_mpls_switch_delete_all (int unit,
                                int flag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_mpls_switch_traverse(int unit,
                             uint32 flags, 
                             uint32 start,
                             uint32 end,
                             bcm_mpls_switch_traverse_cb trav_fn,
                             void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_mpls_exp_map_create(int unit,
                            uint32 flags,
                            int *exp_map_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_mpls_exp_map_destroy(int unit,
                             int exp_map_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_mpls_exp_map_set(int unit,
                         int exp_map_id,
                         bcm_mpls_exp_map_t *exp_map)
{
    return BCM_E_UNAVAIL;
}

int     
bcm_sbx_mpls_exp_map_get(int unit,
                         int exp_map_id,
                         bcm_mpls_exp_map_t *exp_map)
{
    return BCM_E_UNAVAIL;
}

#else   /* INCLUDE_L3 */
int bcm_sbx_mpls_not_empty;
#endif  /* INCLUDE_L3 */
