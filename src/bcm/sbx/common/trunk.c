/*
 * $Id: trunk.c 1.4.216.1 Broadcom SDK $
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
 * File:    trunk.c
 * Purpose: BCM level APIs for trunking (a.k.a. Port Aggregation)
 */

#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/lock.h>
#include <bcm/trunk.h>

int
bcm_sbx_trunk_init(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
	return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_init == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_init(unit);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_detach(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_detach == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_detach(unit);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_create(int unit,
                     bcm_trunk_t *tid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_create == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_create(unit, tid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_create_id(int unit,
                        bcm_trunk_t tid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_create_id == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_create_id(unit, tid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_psc_set(int unit,
                      bcm_trunk_t tid,
                      int psc)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_psc_get(int unit,
                      bcm_trunk_t tid,
                      int *psc)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_chip_info_get(int unit,
                            bcm_trunk_chip_info_t *ta_info)
{
    int rc = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);
    if (soc_feature(unit, soc_feature_mc_group_ability)) {
        if ((mbcm_sbx_driver[unit]) &&
            (mbcm_sbx_driver[unit]->mbcm_trunk_chip_info_get)) {
            rc = mbcm_sbx_driver[unit]->mbcm_trunk_chip_info_get(unit, 
                                                                 ta_info);
        }
    }
    BCM_SBX_UNLOCK(unit);
    return rc;
}

int
bcm_sbx_trunk_set(int unit,
                  bcm_trunk_t tid,
                  bcm_trunk_add_info_t *add_info)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_set == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_set(unit, tid, add_info);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_destroy(int unit,
                      bcm_trunk_t tid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_destroy == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_destroy(unit, tid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_get(int unit,
                  bcm_trunk_t tid,
                  bcm_trunk_add_info_t *t_data)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_get == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_get(unit, tid, t_data);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_mcast_join(int unit,
                         bcm_trunk_t tid,
                         bcm_vlan_t vid,
                         sal_mac_addr_t mac)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_bitmap_expand(int unit,
                            pbmp_t *pbmp_ptr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_egress_set(int unit,
                         bcm_trunk_t tid,
                         bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_egress_get(int unit,
                         bcm_trunk_t tid,
                         bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_override_ucast_set(int unit,
                                 bcm_port_t port,
                                 bcm_trunk_t tid,
                                 int modid,
                                 int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_override_ucast_get(int unit,
                                 bcm_port_t port,
                                 bcm_trunk_t tid,
                                 int modid,
                                 int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_override_mcast_set(int unit,
                                 bcm_port_t port,
                                 bcm_trunk_t tid,
                                 int idx,
                                 int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_override_mcast_get(int unit,
                                 bcm_port_t port,
                                 bcm_trunk_t tid,
                                 int idx,
                                 int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_override_ipmc_set(int unit,
                                bcm_port_t port,
                                bcm_trunk_t tid,
                                int idx,
                                int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_override_ipmc_get(int unit,
                                bcm_port_t port,
                                bcm_trunk_t tid,
                                int idx,
                                int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_override_vlan_set(int unit,
                                bcm_port_t port,
                                bcm_trunk_t tid,
                                bcm_vlan_t vid,
                                int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_override_vlan_get(int unit,
                                bcm_port_t port,
                                bcm_trunk_t tid,
                                bcm_vlan_t vid,
                                int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_pool_set(int unit,
                       bcm_port_t port,
                       bcm_trunk_t tid,
                       int size,
                       const int weights[])
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_pool_get(int unit,
                       bcm_port_t port,
                       bcm_trunk_t tid,
                       int *size,
                       int weights[])
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_trunk_find(int unit,
                   bcm_module_t modid,
                   bcm_port_t port,
                   bcm_trunk_t *tid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_find == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_find(unit, modid, port, tid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}
