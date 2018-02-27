/*
 * $Id: vlan.c 1.20 Broadcom SDK $
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
 * Module: VLAN management
 */

#include <soc/drv.h>

#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/types.h>


#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/lock.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbStatus.h>

#define VLAN_OUT(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_VLAN, stuff)

#define VLAN_WARN(stuff)        VLAN_OUT(BCM_DBG_WARN, stuff)
#define VLAN_ERR(stuff)         VLAN_OUT(BCM_DBG_ERR, stuff)
#define VLAN_VERB(stuff)        VLAN_OUT(BCM_DBG_VERBOSE, stuff)

int
bcm_sbx_vlan_init(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_init == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_init(unit);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_create(int unit,
                    bcm_vlan_t vid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_create == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_create(unit, vid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_destroy(int unit,
                     bcm_vlan_t vid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_destroy == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_destroy(unit, vid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_destroy_all(int unit)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_vlan_destroy_all == NULL)) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_destroy_all(unit);

    BCM_SBX_UNLOCK(unit);
    return(rc);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_port_add(int unit,
                      bcm_vlan_t vid,
                      pbmp_t pbmp,
                      pbmp_t ubmp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_port_add == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_port_add(unit, vid, pbmp, ubmp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_port_remove(int unit,
                         bcm_vlan_t vid,
                         pbmp_t pbmp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_port_remove == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_port_remove(unit, vid, pbmp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_port_protocol_action_add(int unit, bcm_port_t port,
                                      bcm_port_frametype_t f,
                                      bcm_port_ethertype_t ethertype,
                                      bcm_vlan_action_set_t *a)
{
    int rv;

    BCM_SBX_LOCK(unit);

    
    rv = translate_sbx_result(SB_UNIMPLEMENTED_FUNCTION_ERR_CODE);
    /* return BCM_E_UNAVAIL; */

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_vlan_port_default_action_get(int unit, bcm_port_t port,
                                     bcm_vlan_action_set_t *a)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_port_default_action_set(int unit, bcm_port_t port,
                                     bcm_vlan_action_set_t *a)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_port_egress_default_action_get(int unit, bcm_port_t port,
                                            bcm_vlan_action_set_t *a)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_port_egress_default_action_set(int unit, bcm_port_t port,
                                               bcm_vlan_action_set_t *a)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_port_get(int unit,
                      bcm_vlan_t vid,
                      pbmp_t *pbmp,
                      pbmp_t *ubmp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_port_get == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_port_get(unit, vid, pbmp, ubmp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_list(int unit,
                  bcm_vlan_data_t **listp,
                  int *countp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_list == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_list(unit, listp, countp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_list_by_pbmp(int unit,
                          pbmp_t pbmp,
                          bcm_vlan_data_t **listp,
                          int *countp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_list_by_pbmp == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_list_by_pbmp(unit, pbmp, listp, countp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_list_destroy(int unit,
                          bcm_vlan_data_t *list,
                          int count)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_list_destroy == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_list_destroy(unit, list, count);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_default_get(int unit,
                         bcm_vlan_t *vid_ptr)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_vlan_default_get == NULL)) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_default_get(unit, vid_ptr);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_default_set(int unit,
                         bcm_vlan_t vid)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_vlan_default_set == NULL)) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_default_set(unit, vid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_stg_get(int unit,
                     bcm_vlan_t vid,
                     bcm_stg_t *stg_ptr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_stg_set(int unit,
                     bcm_vlan_t vid,
                     bcm_stg_t stg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_stp_set(int unit,
                     bcm_vlan_t vid,
                     bcm_port_t port,
                     int stp_state)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_stp_get(int unit,
                     bcm_vlan_t vid,
                     bcm_port_t port,
                     int *stp_state)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_mac_add(int unit,
                     bcm_mac_t mac,
                     bcm_vlan_t vid,
                     int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_mac_delete(int unit,
                        bcm_mac_t mac)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_vlan_mac_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_add(int unit,
                           int port,
                           bcm_vlan_t old_vid,
                           bcm_vlan_t new_vid,
                           int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_delete(int unit,
                              int port,
                              bcm_vlan_t old_vid)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_vlan_translate_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_range_add(int unit,
                                 int port,
                                 bcm_vlan_t old_vid_low,
                                 bcm_vlan_t old_vid_high,
                                 bcm_vlan_t new_vid,
                                 int int_prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_range_delete(int unit,
                                    int port,
                                    bcm_vlan_t old_vid_low,
                                    bcm_vlan_t old_vid_high)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_range_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int bcm_sbx_vlan_translate_range_get (int unit, bcm_port_t port,
                                      bcm_vlan_t old_vlan_low,
                                      bcm_vlan_t old_vlan_high,
                                      bcm_vlan_t *new_vid, int *prio)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_dtag_range_add(int unit,
                            int port,
                            bcm_vlan_t old_vid_low, 
                            bcm_vlan_t old_vid_high,
                            bcm_vlan_t new_vid,
                            int int_prio)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_dtag_range_delete(int unit,
                               int port, 
                               bcm_vlan_t old_vid_low,
                               bcm_vlan_t old_vid_high)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_dtag_range_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_action_range_add(int unit,
                                        bcm_gport_t port,
                                        bcm_vlan_t outer_vlan_low,
                                        bcm_vlan_t outer_vlan_high,
                                        bcm_vlan_t inner_vlan_low,
                                        bcm_vlan_t inner_vlan_high,
                                        bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_action_range_delete(int unit,
                                           bcm_gport_t port,
                                           bcm_vlan_t outer_vlan_low,
                                           bcm_vlan_t outer_vlan_high,
                                           bcm_vlan_t inner_vlan_low,
                                           bcm_vlan_t inner_vlan_high)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_action_range_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_dtag_range_get (int unit, bcm_port_t port,
                             bcm_vlan_t old_vid_low,
                             bcm_vlan_t old_vid_high,
                             bcm_vlan_t *new_vid,
                             int *prio)
{

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_egress_add(int unit,
                                  int port,
                                  bcm_vlan_t old_vid,
                                  bcm_vlan_t new_vid,
                                  int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_egress_delete(int unit,
                                     int port,
                                     bcm_vlan_t old_vid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_egress_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_dtag_add(int unit,
                      int port,
                      bcm_vlan_t inner_vid,
                      bcm_vlan_t outer_vid,
                     int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_dtag_delete(int unit,
                         int port,
                         bcm_vlan_t vid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_dtag_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Vlan selection based on IPv4 addresses
 */

int
bcm_sbx_vlan_ip4_add(int unit,
                     bcm_ip_t ipaddr,
                     bcm_ip_t netmask,
                     bcm_vlan_t vid,
                     int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_ip4_delete(int unit,
                        bcm_ip_t ipaddr,
                        bcm_ip_t netmask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_ip4_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Vlan selection based on unified IPv4/IPv6 information structure.
 */

int
bcm_sbx_vlan_ip_add(int unit,
                    bcm_vlan_ip_t *vlan_ip)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_ip_delete(int unit,
                       bcm_vlan_ip_t *vlan_ip)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_ip_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_control_set(int unit,
                         bcm_vlan_control_t type,
                         int arg)
{
    VLAN_VERB(("VLAN %d: control set: type %d, arg %d\n",
               unit, type, arg));

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_control_get(int unit,
                         bcm_vlan_control_t type,
                         int *arg)
{
    if (arg == 0) {
        return BCM_E_PARAM;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_control_port_set(int unit,
                              bcm_port_t port,
                              bcm_vlan_control_port_t type,
                              int arg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_control_port_get(int unit,
                              bcm_port_t port,
                              bcm_vlan_control_port_t type,
                              int * arg)
{
    if (arg == 0) {
        return BCM_E_PARAM;
    }
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_mcast_flood_set(int unit,
                             bcm_vlan_t vlan,
                             bcm_vlan_mcast_flood_t mode)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_mcast_flood_get(int unit,
                             bcm_vlan_t vlan,
                             bcm_vlan_mcast_flood_t *mode)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_control_vlan_set(int unit,
                              bcm_vlan_t vid,
                              bcm_vlan_control_vlan_t control)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_control_vlan_get(int unit,
                              bcm_vlan_t vid,
                              bcm_vlan_control_vlan_t *control)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_vector_flags_set(int unit,
                              bcm_vlan_vector_t vlan_vector,
                              uint32 flags_mask,
                              uint32 flags_value)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_cross_connect_add(int unit, 
                               bcm_vlan_t outer_vlan,
                               bcm_vlan_t inner_vlan,
                               bcm_gport_t port_1,
                               bcm_gport_t port_2)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_cross_connect_delete(int unit,
                                  bcm_vlan_t outer_vlan, 
                                  bcm_vlan_t inner_vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_cross_connect_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_cross_connect_traverse(int unit,
                                    bcm_vlan_cross_connect_traverse_cb cb,
                                    void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_mac_action_add(int unit,
                            bcm_mac_t mac,
                            bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_mac_action_get(int unit, bcm_mac_t mac, bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_mac_action_delete(int unit, bcm_mac_t mac)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_mac_action_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_mac_action_traverse(int unit, 
                                 bcm_vlan_mac_action_traverse_cb cb, 
                                 void *user_data)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_vlan_translate_action_add(int unit,
                                  bcm_port_t port,
                                  bcm_vlan_translate_key_t key_type,
                                  bcm_vlan_t outer_vlan,
                                  bcm_vlan_t inner_vlan,
                                  bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_translate_action_get(int unit,
                                  bcm_gport_t port,
                                  bcm_vlan_translate_key_t key_type,
                                  bcm_vlan_t outer_vlan,
                                  bcm_vlan_t inner_vlan,
                                  bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_action_delete(int unit,
                                     bcm_port_t port,
                                     bcm_vlan_translate_key_t key_type,
                                     bcm_vlan_t outer_vlan,
                                     bcm_vlan_t inner_vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_action_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_action_range_get(int unit, 
                                        bcm_gport_t port, 
                                        bcm_vlan_translate_key_t key_type, 
                                        bcm_vlan_t outer_vlan, 
                                        bcm_vlan_t inner_vlan, 
                                        bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_egress_action_add(int unit, int port_class,
                                         bcm_vlan_t outer_vlan,
                                         bcm_vlan_t inner_vlan,
                                         bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_translate_egress_action_get(int unit,
                                         int port_class,
                                         bcm_vlan_t outer_vlan,
                                         bcm_vlan_t inner_vlan,
                                         bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_egress_action_delete(int unit,
                                            int port_class,
                                            bcm_vlan_t outer_vlan,
                                            bcm_vlan_t inner_vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_egress_action_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}


int 
bcm_sbx_vlan_translate_egress_action_traverse(int unit,
                               bcm_vlan_translate_egress_action_traverse_cb cb,
                                              void *user_data)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_translate_action_traverse(int unit,
                                       bcm_vlan_translate_action_traverse_cb cb,
                                       void *user_data)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_translate_traverse(int unit,
                                bcm_vlan_translate_traverse_cb cb,
                                void *user_data)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_translate_egress_traverse(int unit,
                                bcm_vlan_translate_egress_traverse_cb cb,
                                       void *user_data)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_dtag_traverse(int unit,
                           bcm_vlan_dtag_traverse_cb cb, void *user_data)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_stat_get(int unit, bcm_vlan_t vlan, bcm_vlan_stat_t stat, 
                      uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_stat_get32(int unit, bcm_vlan_t vlan, bcm_vlan_stat_t stat, 
                        uint32 *val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_stat_set(int unit, bcm_vlan_t vlan, bcm_vlan_stat_t stat, 
                      uint64 val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_stat_set32(int unit, bcm_vlan_t vlan, bcm_vlan_stat_t stat, 
                        uint32 val)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_vlan_stat_enable_set(int unit, bcm_vlan_t vlan, int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_policer_get(int unit, bcm_vlan_t vlan, bcm_policer_t *pol_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_policer_set(int unit, bcm_vlan_t vlan, bcm_policer_t pol_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_dtag_get(int unit,
                      int port,
                      bcm_vlan_t old_vid,
                      bcm_vlan_t * new_vid,
                      int * prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_egress_get(int unit,
                                  int port,
                                  bcm_vlan_t old_vid,
                                  bcm_vlan_t * new_vid,
                                  int * prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_get(int unit,
                           int port,
                           bcm_vlan_t old_vid,
                           bcm_vlan_t * new_vid,
                           int * prio)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_vlan_translate_action_range_traverse(int unit, 
                   bcm_vlan_translate_action_range_traverse_cb cb, 
                                             void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_translate_range_traverse(int unit, 
                   bcm_vlan_translate_action_range_traverse_cb cb, 
                                      void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_vlan_dtag_range_traverse(int unit, 
                                bcm_vlan_translate_action_range_traverse_cb cb,
                                 void *user_data)
{
    return BCM_E_UNAVAIL;
}

/* Create a VLAN queue map entry. */
int
bcm_sbx_vlan_queue_map_create(int unit,
                              uint32 flags,
                              int *qmid)
{
    return BCM_E_UNAVAIL;
}

/* Delete a VLAN queue map entry. */
int
bcm_sbx_vlan_queue_map_destroy(int unit,
                               int qmid)
{
    return BCM_E_UNAVAIL;
}

/* Delete all VLAN queue map entries. */
int
bcm_sbx_vlan_queue_map_destroy_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/* Set a VLAN queue map entry. */
int
bcm_sbx_vlan_queue_map_set(int unit,
                           int qmid,
                           int pkt_pri,
                           int cfi,
                           int queue,
                           int color)
{
    return BCM_E_UNAVAIL;
}

/* Get a VLAN queue map entry. */
int
bcm_sbx_vlan_queue_map_get(int unit,
                           int qmid,
                           int pkt_pri,
                           int cfi,
                           int *queue,
                           int *color)
{
    return BCM_E_UNAVAIL;
}

/* Attach a queue map object to a VLAN or VFI. */
int
bcm_sbx_vlan_queue_map_attach(int unit,
                              bcm_vlan_t vlan,
                              int qmid)
{
    return BCM_E_UNAVAIL;
}

/* Get the queue map object which is attached to a VLAN or VFI. */
int
bcm_sbx_vlan_queue_map_attach_get(int unit,
                                  bcm_vlan_t vlan,
                                  int *qmid)
{
    return BCM_E_UNAVAIL;
}

/* Detach a queue map object from a VLAN or VFI. */
int
bcm_sbx_vlan_queue_map_detach(int unit,
                              bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

/* Detach queue map objects from all VLAN or VFI. */
int
bcm_sbx_vlan_queue_map_detach_all(int unit)
{
    return BCM_E_UNAVAIL;
}
