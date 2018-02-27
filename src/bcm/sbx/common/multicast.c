/*
 * $Id: multicast.c 1.16 Broadcom SDK $
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
 * File:    multicast.c
 * Purpose: Manages multicast functions
 */

#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <bcm/debug.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/multicast.h>
#include <bcm/l2.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/multicast.h>

#include <bcm_int/sbx/lock.h>
#ifdef BCM_SIRIUS_SUPPORT
#include <bcm_int/sbx/sirius.h>
#endif /* BCM_SIRIUS_SUPPORT */

int
bcm_sbx_multicast_init(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    /* some SBX devices do not implement this call, but have MC support */
    rc = MBCM_SBX_DRIVER_MAYBE_CALL(unit, mbcm_multicast_init, (unit));

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_detach(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit]!=NULL) &&
        (mbcm_sbx_driver[unit]->mbcm_multicast_init == NULL) &&
        (mbcm_sbx_driver[unit]->mbcm_multicast_detach == NULL)) {
        /* implied init, so implied detach is not invalid here */
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_NONE);
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_detach, unit);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_create(int unit,
                         uint32 flags,
                         bcm_multicast_t *group)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_create == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_create(unit, flags, group);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_destroy(int unit, bcm_multicast_t group)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_destroy == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_destroy(unit, group);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_group_get(int unit,
                            bcm_multicast_t group,
                            uint32 *flags)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (!flags) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_group_get, unit, group, flags);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_group_traverse(int unit,
                                 bcm_multicast_group_traverse_cb_t cb,
                                 uint32 flags,
                                 void *user_data)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (!cb) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_group_traverse, unit, cb, flags, user_data);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_fabric_distribution_set(int unit, bcm_multicast_t group, bcm_fabric_distribution_t ds_id)
{
    int rv = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (ds_id >= SOC_SBX_CFG(unit)->num_ds_ids) {
        BCM_ERR(("ERROR: ds_id(%d) out of range (0-%d) valid\n", ds_id, (SOC_SBX_CFG(unit)->num_ds_ids -1)));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_fabric_distribution_set, unit, group, ds_id);

    if (rv != BCM_E_NONE) {
        BCM_ERR(("ERROR: multicast fabric distribution set failed for group(%d) ds_id(%d) error(%d)\n", group, ds_id, rv));
    }
    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_multicast_fabric_distribution_get(int unit, bcm_multicast_t group,  bcm_fabric_distribution_t *ds_id)
{
    int rv = BCM_E_NONE;

    if (!ds_id) {
        BCM_ERR(("ERROR: required outbound pointer is NULL\n"));
        return BCM_E_PARAM;
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_fabric_distribution_get, unit, group, ds_id);

    if (rv != BCM_E_NONE) {
        BCM_ERR(("ERROR: multicast fabric distribution get failed for group(%d) error(%d)\n", group, rv));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}



int
bcm_sbx_multicast_l3_encap_get(int unit,
                               bcm_multicast_t group,
                               bcm_gport_t port,
                               bcm_if_t intf,
                               bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_multicast_l2_encap_get(int unit,
                               bcm_multicast_t group,
                               bcm_gport_t port,
                               bcm_vlan_t vlan,
                               bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_multicast_vpls_encap_get(int unit,
                                 bcm_multicast_t group,
                                 bcm_gport_t port,
                                 bcm_gport_t mpls_port_id,
                                 bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_multicast_subport_encap_get(int unit,
                                    bcm_multicast_t group,
                                    bcm_gport_t port,
                                    bcm_gport_t subport,
                                    bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_multicast_mim_encap_get(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_gport_t mim_port_id,
                                bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_multicast_wlan_encap_get(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_gport_t wlan_port_id,
                                bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_multicast_vlan_encap_get(int unit,
                                 bcm_multicast_t group,
                                 bcm_gport_t port,
                                 bcm_gport_t vlan_port_id,
                                 bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_multicast_egress_add(int unit,
                             bcm_multicast_t group,
                             bcm_gport_t port,
                             bcm_if_t encap_id)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_egress_add == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_add(unit, group, port, encap_id);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_egress_delete(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_if_t encap_id)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_egress_delete == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_delete(unit, group, port, encap_id);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_egress_subscriber_add(int unit,
                                        bcm_multicast_t group,
                                        bcm_gport_t port,
                                        bcm_if_t encap_id,
                                        bcm_gport_t subscriber_queue)
{
    int rc = BCM_E_NONE;

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        return BCM_E_UNAVAIL;
    }

    if ((NULL != mbcm_sbx_driver[unit]) &&
        (NULL != mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_add)) {
        BCM_SBX_LOCK(unit);
        rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_add(unit,
                                                                         group,
                                                                         port,
                                                                         encap_id,
                                                                         subscriber_queue);
        BCM_SBX_UNLOCK(unit);
    } else {
        rc = BCM_E_UNAVAIL;
    }

    return rc;
}

int
bcm_sbx_multicast_egress_subscriber_delete(int unit,
                                           bcm_multicast_t group,
                                           bcm_gport_t port,
                                           bcm_if_t encap_id,
                                           bcm_gport_t subscriber_queue)
{
    int rc = BCM_E_NONE;

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        return BCM_E_UNAVAIL;
    }

    if ((NULL != mbcm_sbx_driver[unit]) &&
        (NULL != mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_delete)) {
        BCM_SBX_LOCK(unit);
        rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_delete(unit,
                                                                            group,
                                                                            port,
                                                                            encap_id,
                                                                            subscriber_queue);
        BCM_SBX_UNLOCK(unit);
    } else {
        rc = BCM_E_UNAVAIL;
    }

    return rc;
}

int
bcm_sbx_multicast_egress_delete_all(int unit,
                                    bcm_multicast_t group)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_multicast_egress_delete_all == NULL)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_delete_all(unit, group);

    BCM_SBX_UNLOCK(unit);

    return(rc);
}

int
bcm_sbx_multicast_egress_set(int unit,
                             bcm_multicast_t group,
                             int port_count,
                             bcm_gport_t *port_array,
                             bcm_if_t *encap_id_array)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_multicast_egress_set == NULL)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if (port_count == 0) {
        BCM_ERR(("ERROR: %s, invalid port_count parameter, Unit(%d), port_max: 0x%x\n",
                      FUNCTION_NAME(), unit, port_count));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (port_array == NULL) {
        BCM_ERR(("ERROR: %s, port_array parameter is NULL, Unit(%d)\n", FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (encap_id_array == NULL) {
        BCM_ERR(("ERROR: %s, encap_id_array parameter is NULL, Unit(%d)\n", FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_set(unit, group, port_count, port_array, encap_id_array);

    BCM_SBX_UNLOCK(unit);

    return(rc);
}

int
bcm_sbx_multicast_egress_get(int unit,
                             bcm_multicast_t group,
                             int port_max,
                             bcm_gport_t *port_array,
                             bcm_if_t *encap_id_array,
                             int *port_count)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_egress_get == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_get(unit, group, port_max, port_array, encap_id_array, port_count);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_egress_subscriber_set(int unit,
                                        bcm_multicast_t group,
                                        int port_count,
                                        bcm_gport_t *port_array,
                                        bcm_if_t *encap_id_array,
                                        bcm_gport_t *subscriber_queue_array)
{
    int rc;

    if (soc_feature(unit, soc_feature_mc_group_ability) &&
        (NULL != mbcm_sbx_driver[unit]) &&
        (NULL != mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_set)) {
        if ((0 < port_count) &&
            (NULL != port_array) &&
            (NULL != encap_id_array) &&
            (NULL != subscriber_queue_array)) {
            BCM_SBX_LOCK(unit);
            rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_set(unit,
                                                                             group,
                                                                             port_count,
                                                                             port_array,
                                                                             encap_id_array,
                                                                             subscriber_queue_array);
            BCM_SBX_UNLOCK(unit);
        } else {
            rc = BCM_E_PARAM;
        }
    } else {
        rc = BCM_E_UNAVAIL;
    }

    return rc;
}

int
bcm_sbx_multicast_egress_subscriber_get(int unit,
                                        bcm_multicast_t group,
                                        int port_max,
                                        bcm_gport_t *port_array,
                                        bcm_if_t *encap_id_array,
                                        bcm_gport_t *subscriber_queue_array,
                                        int *port_count)
{
    int rc;

    if (soc_feature(unit, soc_feature_mc_group_ability) &&
        (NULL != mbcm_sbx_driver[unit]) &&
        (NULL != mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_get)) {
        if ((0 == port_count) ||
            ((0 < port_count) &&
             (NULL != port_array) &&
             (NULL != encap_id_array) &&
             (NULL != subscriber_queue_array))) {
            BCM_SBX_LOCK(unit);
            rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_get(unit,
                                                                             group,
                                                                             port_max,
                                                                             port_array,
                                                                             encap_id_array,
                                                                             subscriber_queue_array,
                                                                             port_count);
            BCM_SBX_UNLOCK(unit);
        } else {
            rc = BCM_E_PARAM;
        }
    } else {
        rc = BCM_E_UNAVAIL;
    }

    return rc;
}

int
bcm_sbx_multicast_repl_set(int unit, int mc_index, bcm_port_t port,
                      bcm_vlan_vector_t vlan_vec)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_multicast_repl_get(int unit, int index, bcm_port_t port,
                      bcm_vlan_vector_t vlan_vec)
{
    return BCM_E_UNAVAIL;
}


#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
int
bcm_sbx_multicast_get_state(int unit, char *pbuf)
{
    int rv;
    char *pbuf_current = pbuf;

    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_state_get == NULL) {
        return(BCM_E_UNAVAIL);
    }

    rv = mbcm_sbx_driver[unit]->mbcm_multicast_state_get(unit, pbuf_current);

    return rv;
}

#endif /* EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* EASY_RELOAD_SUPPORT */
