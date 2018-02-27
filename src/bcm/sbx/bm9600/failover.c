/*
 * $Id: failover.c 1.4 Broadcom SDK $
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
 * BM9600 failover API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pl_auto.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/bm9600_init.h>
#include <soc/sbx/sbFabCommon.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/bm9600.h>
#include <bcm_int/sbx/failover.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/stack.h>


int
bcm_bm9600_failover_enable(int unit,
			   int sysport,
			   int node, 
			   int port,
			   int old_node, 
			   int old_port)
{
    int rv = BCM_E_NONE;
    int dummy_sysport;
    int ps, os, dummy_ps, dummy_os;
    int indp_fc_sysport, dummy_indp_fc_sysport;
    int indp_fc_ps, indp_fc_os, dummy_indp_fc_ps, dummy_indp_fc_os;

    if ( (sysport < 0) || (sysport >= SOC_SBX_CFG(unit)->num_sysports) ) {
	return BCM_E_PARAM;
    }

    dummy_sysport = BCM_INT_SBX_SYSPORT_DUMMY(sysport);
    indp_fc_sysport = BCM_INT_SBX_INVALID_SYSPORT;
    dummy_indp_fc_sysport = BCM_INT_SBX_INVALID_SYSPORT;

    /* by now, both the sysport and dummy sysport should be in portset,
     * search for their ps/os
     */
    rv = bcm_bm9600_get_portset_from_sysport(unit, sysport, &ps, &os);
    if (rv != BCM_E_NONE) {
	return BCM_E_INTERNAL;
    }

    rv = bcm_bm9600_get_portset_from_sysport(unit, dummy_sysport, &dummy_ps, &dummy_os);
    if (rv != BCM_E_NONE) {
	return BCM_E_INTERNAL;
    }

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        indp_fc_sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
        dummy_indp_fc_sysport = BCM_INT_SBX_SYSPORT_DUMMY(indp_fc_sysport);

        rv = bcm_bm9600_get_portset_from_sysport(unit, indp_fc_sysport, &indp_fc_ps, &indp_fc_os);
        if (rv != BCM_E_NONE) {
            return BCM_E_INTERNAL;
        }

        rv = bcm_bm9600_get_portset_from_sysport(unit, dummy_indp_fc_sysport, &dummy_indp_fc_ps, &dummy_indp_fc_os);
        if (rv != BCM_E_NONE) {
            return BCM_E_INTERNAL;
        }
    }

    /* Swap to the portset/offset used by dummy sysport */
    rv = bcm_bm9600_map_sysport_to_portset(unit, sysport, dummy_ps, dummy_os, 0);
    if (rv != BCM_E_NONE) {
	return rv;
    }

    rv = bcm_bm9600_map_sysport_to_portset(unit, dummy_sysport, ps, os, 0);

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        rv = bcm_bm9600_map_sysport_to_portset(unit, indp_fc_sysport, dummy_indp_fc_ps, dummy_indp_fc_os, 0);
        if (rv != BCM_E_NONE) {
            return rv;
        }

        rv = bcm_bm9600_map_sysport_to_portset(unit, dummy_indp_fc_sysport, indp_fc_ps, indp_fc_os, 0);
        if (rv != BCM_E_NONE) {
            return rv;
        }
    }

    return rv;
}

int
bcm_bm9600_failover_set(int unit,
			int sysport,
			int protect_node, 
			int protect_port,
			int active_node, 
			int active_port)
{
    int rv = BCM_E_NONE;
    int mapped_protect_port;
    sbBool_t is_prot_resource_alloc = FALSE, is_independent_fc_prot_resource_alloc = FALSE;

    if ( !BCM_INT_SBX_SYSPORT_IS_DUMMY(sysport) ) {
	return BCM_E_INTERNAL;
    }

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
    }
		
    /* Remove the dummy sysport from portset if exist */
    rv = bcm_bm9600_unmap_sysport(unit, sysport);
    if (rv != BCM_E_NONE) {
	return rv;
    }

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        rv = bcm_bm9600_unmap_sysport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport));
        if (rv != BCM_E_NONE) {
            return rv;
        }
    }

    /* Insert the dummy sysport for node/port */
    mapped_protect_port = (soc_feature(unit, soc_feature_egr_independent_fc)) ?
                                  BCM_INT_SBX_PORT_TO_EF_PORT(protect_port) : protect_port;
    /* Insert the dummy sysport for node/port */
    rv = bcm_bm9600_map_sysport_to_nodeport(unit, sysport, protect_node, mapped_protect_port);
    if (rv != BCM_E_NONE) {
        goto err;
    }
    is_prot_resource_alloc = TRUE;

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        rv = bcm_bm9600_map_sysport_to_nodeport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport), protect_node,
                                                      BCM_INT_SBX_PORT_TO_NEF_PORT(protect_port));
        if (rv != BCM_E_NONE) {
            goto err;
        }
        is_independent_fc_prot_resource_alloc = TRUE;
    }

    return rv;

err:

    if (is_prot_resource_alloc == TRUE) {
        bcm_bm9600_unmap_sysport(unit, sysport);
    }
    if (is_independent_fc_prot_resource_alloc == TRUE) {
        bcm_bm9600_unmap_sysport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport));
    }

    return rv;
}


int
bcm_bm9600_failover_destroy(int unit,
			    int sysport)

{
    int rv = BCM_E_NONE, rv1 = BCM_E_NONE;

    if ( !BCM_INT_SBX_SYSPORT_IS_DUMMY(sysport) ) {
	return BCM_E_INTERNAL;
    }
		
    /* Remove the dummy sysport from portset */
    rv = bcm_bm9600_unmap_sysport(unit, sysport);

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        rv1 = bcm_bm9600_unmap_sysport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport));
        rv = (rv != BCM_E_NONE) ? rv : rv1;
    }

    return rv;
}
