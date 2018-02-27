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
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/sbFabCommon.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/qe2000.h>
#include <bcm_int/sbx/failover.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>

#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/port.h>
#include <bcm/stack.h>


int
bcm_qe2000_failover_enable(int unit,
			   int sysport,
			   int node, 
			   int port,
			   int old_node, 
			   int old_port)
{
    int rv = BCM_E_NONE;
    int ef;

    /* port remap when the protect port are on the same node */
    if ( (node == old_node) && (SOC_SBX_CONTROL(unit)->node_id == node) ) {
	ef = 0;
        if (soc_feature(unit, soc_feature_egr_independent_fc)) {
            sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
        }
	rv = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, port);
	if (rv != BCM_E_NONE) {
	    BCM_ERR(("ERROR: Failed to remap sysport\n"));
	    return rv;
	}

	ef = 1;
        if (soc_feature(unit, soc_feature_egr_independent_fc)) {
            sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
        }
	rv = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, port);
	if (rv != BCM_E_NONE) {
	    BCM_ERR(("ERROR: Failed to remap sysport\n"));
	    return rv;
	}
    }

    return rv;
}

int
bcm_qe2000_failover_set(int unit,
			int sysport,
			int protect_node, 
			int protect_port,
			int active_node, 
			int active_port)
{
    int rv = BCM_E_NONE;
    int ef;

    /* port remap when the protect port are on the different node */
    if ( (protect_node != active_node) && (SOC_SBX_CONTROL(unit)->node_id == protect_node) ) {
	ef = 0;
        if (soc_feature(unit, soc_feature_egr_independent_fc)) {
            sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
        }
	rv = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, protect_port);
	if (rv != BCM_E_NONE) {
	    BCM_ERR(("ERROR: Failed to remap sysport\n"));
	    return rv;
	}

	ef = 1;
        if (soc_feature(unit, soc_feature_egr_independent_fc)) {
            sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
        }
	rv = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, protect_port);
	if (rv != BCM_E_NONE) {
	    BCM_ERR(("ERROR: Failed to remap sysport\n"));
	    return rv;
	}
    }

    return rv;
}
