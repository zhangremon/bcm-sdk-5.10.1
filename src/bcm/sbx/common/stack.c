/*
 * $Id: stack.c 1.52.18.1 Broadcom SDK $
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
 * File:        stack.c
 * Purpose:     BCM level APIs for stacking applications
 */

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/sirius.h>

#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/fe2000/port.h>
#include <bcm_int/sbx/l3.h>
#include <bcm_int/sbx/error.h>

#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/stack.h>

#include <bcm_int/control.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif  /*  BCM_FE2000_P3_SUPPORT */


#define STK_DEBUG(flags, stuff) BCM_DEBUG(BCM_DBG_STK + (flags), stuff)
#define STK_WARN(stuff) STK_DEBUG(BCM_DBG_WARN, stuff)
#define STK_ERR(stuff) STK_DEBUG(BCM_DBG_ERR, stuff)
#define STK_VERB(stuff) STK_DEBUG(BCM_DBG_VERBOSE, stuff)
#define STK_OUT(stuff) STK_DEBUG(0, stuff)

STATIC bcm_sbx_stack_state_t stack_state[SOC_MAX_NUM_DEVICES];

int
bcm_sbx_stk_init(int unit)
{
    int i;


    STK_VERB(("STK %d: Init\n", unit));

    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);
    if (SOC_IS_SBX_FE2000(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_WARM_BOOT(unit)) {
            uint32 mod = -1;
            int rv;

            rv = soc_sbx_g2p3_node_get(unit, &mod);
            if (BCM_FAILURE(rv)) {
                STK_ERR((_SBX_D(unit, "Failed to recover module id: %s\n"),
                         bcm_errmsg(rv)));
            }
            SOC_SBX_CONTROL(unit)->module_id0 = mod;
        }
#endif  /*  BCM_FE2000_P3_SUPPORT */
        BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    SOC_SBX_STATE(unit)->stack_state = &stack_state[unit];
    SOC_SBX_STATE(unit)->stack_state->gport_map = NULL;

    for (i=0; i < BCM_STK_MAX_MODULES; i++) {
	SOC_SBX_STATE(unit)->stack_state->protocol[i] = bcmModuleProtocolNone;
	SOC_SBX_STATE(unit)->stack_state->is_module_enabled[i] = FALSE;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_stk_update_callback_register(int unit,
                                     bcm_stk_cb_f cb,
                                     void *cookie)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_update_callback_unregister(int unit,
                                       bcm_stk_cb_f cb,
                                       void *cookie)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_set(int unit,
                     bcm_port_t port,
                     uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_pbmp_get(int unit,
                     bcm_pbmp_t *cur_pbm,
                     bcm_pbmp_t *inactive_pbm)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_mode_get(int unit,
                     uint32 *flags)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    
    *flags = FALSE;
    return BCM_E_NONE;
}

int
bcm_sbx_stk_mode_set(int unit,
                     uint32 flags)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_get(int unit,
                     bcm_port_t port,
                     uint32 *flags)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_sl_simplex_count_set(int unit,
                                 int count)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    STK_VERB(("STK %d: SL count set to %d\n", unit, count));

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_sl_simplex_count_get(int unit,
                                 int *count)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modid_set(int unit,
                      int modid)
{
    int rv;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        SOC_SBX_CONTROL(unit)->module_id0 = modid;
#if defined(BCM_FE2000_P3_SUPPORT)
        if (SOC_IS_SBX_FE2000(unit)) {
            rv = _bcm_fe2000_port_modid_set(unit, modid);

            if (rv == BCM_E_NONE) {
                rv = _bcm_fe2000_l3_modid_set(unit, modid);
            }

	    BCM_SBX_UNLOCK(unit);
            return rv;
        }
#endif  /* BCM_FE2000_P3_SUPPORT*/

	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    } 

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_modid_set,unit, modid));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_modid_get(int unit,
                      int *modid)
{
    int rv;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        *modid = SOC_SBX_CONTROL(unit)->module_id0;
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_modid_get,unit, modid));
    
    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_modid_count(int unit,
                        int *num_modid)
{
    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit) || SOC_IS_SBX_QE(unit)) {
        *num_modid = 1;
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_my_modid_set(int unit,
                         int my_modid)
{
    int rv;

    BCM_SBX_LOCK(unit);

    if ( !SOC_MODID_ADDRESSABLE(unit, my_modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_BADID;
    }

    if (SOC_IS_SBX_FE(unit)) {
        SOC_SBX_CONTROL(unit)->module_id0 = my_modid;
#if defined(BCM_FE2000_P3_SUPPORT)
        if (SOC_IS_SBX_FE2000(unit)) {
            rv = _bcm_fe2000_port_modid_set(unit, my_modid);

            if (rv == BCM_E_NONE) {
                rv = _bcm_fe2000_l3_modid_set(unit, my_modid);
            }

	    BCM_SBX_UNLOCK(unit);
            return rv;
        }
#endif /* BCM_FE2000_P3_SUPPORT */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }
    
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_my_modid_set,unit, my_modid));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_my_modid_get(int unit,
                         int *my_modid)
{
    int rv;

    if (SOC_IS_SBX_FE(unit)) {
        *my_modid = SOC_SBX_CONTROL(unit)->module_id0;
        return BCM_E_NONE;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_my_modid_get,unit, my_modid));

    return rv;
}

int
bcm_sbx_stk_modport_set(int unit,
                        int modid,
                        bcm_port_t port)
{
    BCM_SBX_LOCK(unit);

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_get_all(int unit,
                        int modid,
                        int port_max,
                        bcm_port_t *port_array,
                        int *port_count)
{
    BCM_SBX_LOCK(unit);

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_get(int unit,
                        int modid,
                        bcm_port_t *port)
{
    *port = -1;
    return bcm_sbx_stk_modport_get_all(unit, modid, 1, port, NULL);
}

int
bcm_sbx_stk_modport_clear(int unit,
                          int modid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_clear_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_add(int unit,
                        int modid,
                        bcm_port_t port)
{
    BCM_SBX_LOCK(unit);

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_delete(int unit,
                           int modid,
                           bcm_port_t port)
{
    BCM_SBX_LOCK(unit);

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modport_set(int unit, bcm_port_t ing_port,
                             bcm_module_t dest_modid, bcm_port_t dest_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modport_get(int unit, bcm_port_t ing_port,
                             bcm_module_t dest_modid, bcm_port_t *dest_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modport_get_all(int unit, bcm_port_t ing_port,
                                 bcm_module_t dest_modid, int dest_port_max,
                                 bcm_port_t *dest_port_array,
                                 int *dest_port_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modport_clear(int unit, bcm_port_t ing_port,
                               bcm_module_t dest_modid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modport_clear_all(int unit, bcm_port_t ing_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modport_add(int unit, bcm_port_t ing_port,
                             bcm_module_t dest_modid, bcm_port_t dest_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modport_delete(int unit, bcm_port_t ing_port,
                                bcm_module_t dest_modid, bcm_port_t dest_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_ucbitmap_set(int unit,
                         bcm_port_t port,
                         int modid,
                         pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_ucbitmap_get(int unit,
                         bcm_port_t port,
                         int modid,
                         pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_ucbitmap_del(int unit,
                         bcm_port_t port,
                         int modid,
                         bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_fmod_lmod_mapping_set(int unit,
                                  bcm_port_t port,
                                  bcm_module_t fmod,
                                  bcm_module_t lmod)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_fmod_lmod_mapping_get(int unit,
                                  bcm_port_t port,
                                  bcm_module_t *fmod,
                                  bcm_module_t *lmod)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_fmod_smod_mapping_set(int unit,
                                  bcm_port_t port,
                                  bcm_module_t fmod,
                                  bcm_module_t smod,
                                  bcm_port_t sport,
                                  uint32 nports)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_fmod_smod_mapping_get(int unit,
                                  bcm_port_t port,
                                  bcm_module_t fmod,
                                  bcm_module_t *smod,
                                  bcm_port_t *sport,
                                  uint32 *nports)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modmap_group_set(int unit,
                                  bcm_port_t port,
                                  int group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modmap_group_get(int unit,
                                  bcm_port_t port,
                                  int *group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_modmap_group_max_get(int unit,
                                      int *max_group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modmap_enable_set(int unit,
                              bcm_port_t port,
                              int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modmap_enable_get(int unit,
                              bcm_port_t port,
                              int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modmap_register(int unit,
                            bcm_stk_modmap_cb_t func)
{

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modmap_unregister(int unit)
{
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modmap_map(int unit,
                       int setget,
                       bcm_module_t mod_in,
                       bcm_port_t port_in,
                       bcm_module_t *mod_out,
                       bcm_port_t *port_out)
{
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_module_enable(int unit,
                          bcm_module_t mod,
                          int nports,
                          int enable)
{
    int node;
    int rv = BCM_E_NONE;


    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!BCM_STK_MOD_IS_NODE(mod)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if ((nports != -1) &&
	(nports < 0)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if (SOC_IS_SBX_FE(unit)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    if (!(BCM_STK_MOD_IS_NODE(mod))) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    node = BCM_STK_MOD_TO_NODE(mod);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_module_enable,unit, mod, nports, enable);

    if (rv != BCM_E_NONE) {
	BCM_SBX_UNLOCK(unit);
        return(rv);
    }

    /* NOTE: If in future this API is supported for other devices              */
    /*       the following data structure will have to be accordingly modified */
    SOC_SBX_STATE(unit)->stack_state->is_module_enabled[node] = (enable == FALSE) ? FALSE: TRUE;
    rv = bcm_sbx_update_templates(unit, 0);

    BCM_SBX_UNLOCK(unit);
    return(rv);
}

int
bcm_sbx_stk_get_modules_enabled(int unit, int *nbr_modules_enabled)
{
    int node;
    int rv = BCM_E_NONE;

    if (nbr_modules_enabled == NULL) {
        return BCM_E_PARAM;
    }

    BCM_SBX_LOCK(unit);

    (*nbr_modules_enabled) = 0;

    for (node = 0; node < BCM_STK_MAX_MODULES; node++) {
        if (SOC_SBX_STATE(unit)->stack_state->is_module_enabled[node] == TRUE) {
            (*nbr_modules_enabled)++;
        }
    }

    BCM_SBX_UNLOCK(unit);
    return(rv);
}

int
bcm_sbx_stk_module_protocol_set(int unit,
                                bcm_module_t mod,
                                bcm_module_protocol_t protocol)
{
    int node;
    int rv;


    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) ||
	(SOC_SBX_STATE(unit)->stack_state != &stack_state[unit])) {
        return BCM_E_UNIT;
    }
    
    BCM_SBX_LOCK(unit);
    if (!BCM_STK_MOD_IS_NODE(mod)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if ((protocol < bcmModuleProtocolNone) ||
	(protocol > bcmModuleProtocol5)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    /* additional consistency checks */
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        if ( (protocol != bcmModuleProtocolNone) && (protocol != bcmModuleProtocol1) &&
	     (protocol != bcmModuleProtocol2) ) {
            BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }
    } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
        if ( (protocol != bcmModuleProtocolNone) && (protocol != bcmModuleProtocol3) &&
	     (protocol != bcmModuleProtocol4) && (protocol != bcmModuleProtocol5) ) {
            BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }
    } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) {
        if ( (protocol != bcmModuleProtocolNone) && (protocol != bcmModuleProtocol1) &&
	     (protocol != bcmModuleProtocol2)) {
            BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }
    } else if (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
        STK_WARN(("Unexpected Fabric configuration(%d)\n", SOC_SBX_CFG(unit)->uFabricConfig));
    }

    node = BCM_STK_MOD_TO_NODE(mod);

    SOC_SBX_STATE(unit)->stack_state->protocol[node] = protocol;

    if (SOC_IS_SBX_FE(unit)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_module_protocol_set, unit, node, protocol));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_module_protocol_get(int unit,
				bcm_module_t mod,
				bcm_module_protocol_t *protocol)
{
    int node;
    int rv;


    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) ||
	(SOC_SBX_STATE(unit)->stack_state != &stack_state[unit])) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);
    if (!BCM_STK_MOD_IS_NODE(mod)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    node = BCM_STK_MOD_TO_NODE(mod);
    *protocol = SOC_SBX_STATE(unit)->stack_state->protocol[node];

    if (SOC_IS_SBX_FE(unit)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_module_protocol_get,unit, node, protocol));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_fabric_map_set(int unit, 
			   bcm_gport_t switch_port, 
			   bcm_gport_t fabric_port)
{
    int fe_mod, qe_mod = -1, nodeid, subport = -1, eg_n = 0;
    bcm_port_t fe_port = -1, qe_port = -1, rq_port = FALSE;
    uint32 entry;
    uint32 found;
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_gport_t *match_entry = NULL;
    bcm_sbx_stack_portmap_block_t *match_map = NULL;
    bcm_sbx_stack_portmap_block_t * map;
    bcm_sbx_stack_portmap_block_t * prev_map = NULL;
    int rv = BCM_E_NONE;
    int PFC_mapping = FALSE;

    BCM_SBX_LOCK(unit);


    if (switch_port == BCM_GPORT_INVALID) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    } else if ((switch_port >= 0) && (switch_port <= 15)) {
	/* for Sirius PFC source port ID generation */
	PFC_mapping = TRUE;
    }
    
    if (BCM_GPORT_IS_MODPORT(fabric_port)) { 
	qe_mod = BCM_GPORT_MODPORT_MODID_GET(fabric_port);
	qe_port = BCM_GPORT_MODPORT_PORT_GET(fabric_port);       
    } else if (BCM_GPORT_IS_CHILD(fabric_port)) {
	qe_mod = BCM_GPORT_CHILD_MODID_GET(fabric_port);
	qe_port = BCM_GPORT_CHILD_PORT_GET(fabric_port);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(fabric_port)) {
	qe_mod = BCM_GPORT_EGRESS_CHILD_MODID_GET(fabric_port);
	qe_port = BCM_GPORT_EGRESS_CHILD_PORT_GET(fabric_port);
    } else if (BCM_GPORT_IS_EGRESS_GROUP(fabric_port)) {
	rv = bcm_sbx_cosq_egress_group_info_get(unit, fabric_port, &subport, &eg_n, NULL);
	if (rv != BCM_E_NONE) {
	    BCM_ERR(("ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n",
		     FUNCTION_NAME(), fabric_port, unit));
	    return BCM_E_PARAM;
	}
	qe_mod = BCM_GPORT_EGRESS_GROUP_MODID_GET(fabric_port);
	qe_port = subport;
    } else {
	if (SOC_IS_SBX_FE2000(unit)) {
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
    }

    if (PFC_mapping) {
	rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_fabric_map_set, unit, switch_port, fabric_port));
	if (rv != SOC_E_NONE) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (SOC_IS_SBX_FE2000(unit)) {
	rv = BCM_E_NONE;

        if (!BCM_GPORT_IS_MODPORT(switch_port)) {
            /* !BCM_GPORT_IS_MODPORT(fabric_port)) */
            STK_ERR(("unit %d: switch_port %d or fabric_port %d is not modport "
                     "gport\n", unit, switch_port, fabric_port));
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }

        fe_mod = BCM_GPORT_MODPORT_MODID_GET(switch_port);
        fe_port = BCM_GPORT_MODPORT_PORT_GET(switch_port);
        if (fe_mod >= SBX_MAX_MODIDS || fe_port >= SBX_MAX_PORTS) {
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
       
        SOC_SBX_CONTROL(unit)->modport[fe_mod][fe_port] =
            (qe_mod << 16) | qe_port;

        /* set the reverse mapping fabport -> feport */
        nodeid = BCM_STK_MOD_TO_NODE(qe_mod);

        if(nodeid < 0 || nodeid >= SBX_MAX_NODES) {
            return BCM_E_INIT;
        }

        SOC_SBX_CONTROL(unit)->fabnodeport2feport[nodeid][qe_port] =
            (fe_mod << 16) | fe_port;

    } else {
	if ((fabric_port != BCM_GPORT_INVALID) && (qe_port != -1) &&
	    SOC_SBX_STATE(unit)->port_state->subport_info) {
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[qe_port]);
	
            if ((sp_info != NULL) && (sp_info->parent_gport != 0xffffffff) &&
                IS_REQ_PORT(unit, BCM_GPORT_MODPORT_PORT_GET(sp_info->parent_gport))) {
                rq_port = TRUE;
            }
        }

	/* Find the entry */
	for(found=0, map=SOC_SBX_STATE(unit)->stack_state->gport_map; !found && map!=NULL;
	    prev_map=map, map=map->next) {
	    if (map->count > 0) {
		for (entry=0; entry<2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE; entry+=2) {
		    if (map->portmap[entry] == switch_port) {
			/* save the matched entry */
			found = 1;
			match_entry = &(map->portmap[entry]);
			match_map = map;
			break;
		    } else if ((map->portmap[entry] == BCM_GPORT_INVALID) && (match_entry == NULL)) {
			/* save first unused entry */
			match_entry = &(map->portmap[entry]);
			match_map = map;
		    }
		}
	    }
	}

	if (fabric_port == BCM_GPORT_INVALID) {
	    /* clear the map */
	    if (found == 1) {
		*match_entry = BCM_GPORT_INVALID;
		*(match_entry+1) = BCM_GPORT_INVALID;
		match_map->count--;
	    }
	} else {
	    /* set the map */
	    if (found == 1) {
		if (rq_port == FALSE) {
		    /* replacing the existing entry except for requeue path */
		    *(match_entry+1) = fabric_port;
		}
	    } else {
		/* creating a new entry */
		if (match_entry != NULL) {
		    /* found a unused entry, fill it */
		    *match_entry = switch_port;
		    *(match_entry+1) = fabric_port;
		    match_map->count++;
		} else {
		    /* only possible when all existing maps are full */
		    map = sal_alloc(sizeof(bcm_sbx_stack_portmap_block_t), "fabric map");
		    if (map == NULL) {
			BCM_SBX_UNLOCK(unit);
			return SOC_E_MEMORY;
		    }

		    sal_memset(map, BCM_GPORT_INVALID, sizeof(bcm_sbx_stack_portmap_block_t));
		    map->next = NULL;
		    map->count = 0;
		    if ( SOC_SBX_STATE(unit)->stack_state->gport_map == NULL ) {
			/* init linklist head */
			SOC_SBX_STATE(unit)->stack_state->gport_map = map;
		    } else {
			/* chain the single linklist */
			prev_map->next = map;
		    }
		    map->portmap[0] = switch_port;
		    map->portmap[1] = fabric_port;
		    map->count++;
		}
	    }
	}
    }

    if (SOC_IS_SBX_SIRIUS(unit)) {
	rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_fabric_map_set, unit, switch_port, fabric_port));
	if (rv != SOC_E_NONE) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_stk_fabric_map_get(int unit, 
			   bcm_gport_t switch_port, 
			   bcm_gport_t *fabric_port)
{
    int fe_mod, qe_mod;
    bcm_port_t fe_port, qe_port;
    uint32 entry;
    bcm_sbx_stack_portmap_block_t * map;
    int rv = BCM_E_NONE;
    int PFC_mapping = FALSE;
	
    BCM_SBX_LOCK(unit);

    if (switch_port == BCM_GPORT_INVALID || fabric_port == NULL) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    } else if ((switch_port >= 0) && (switch_port <= 15)) {
	/* for Sirius PFC source port ID generation */
	PFC_mapping = TRUE;
    }

    if (PFC_mapping) {
	rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_fabric_map_get, unit, switch_port, fabric_port));
	if (rv != SOC_E_NONE) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (SOC_IS_SBX_FE2000(unit)) {
        if (!BCM_GPORT_IS_MODPORT(switch_port)) {
            STK_ERR(("unit %d: switch_port %d is not a modport "
                     "gport\n", unit, switch_port));
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }

        fe_mod = BCM_GPORT_MODPORT_MODID_GET(switch_port);
        fe_port = BCM_GPORT_MODPORT_PORT_GET(switch_port);

        qe_mod = ((SOC_SBX_CONTROL(unit)->modport[fe_mod][fe_port] >> 16) 
                  & 0xffff);
        qe_port = (SOC_SBX_CONTROL(unit)->modport[fe_mod][fe_port]
                   & 0xffff);

        BCM_GPORT_MODPORT_SET(*fabric_port, qe_mod, qe_port);
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    } else { 

	/* Find the entry */
	for(map=SOC_SBX_STATE(unit)->stack_state->gport_map; map!=NULL; map=map->next) {
	    if (map->count > 0) {
		for (entry=0; entry<2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE; entry+=2) {
		    if (map->portmap[entry] == switch_port) {
			/* return the matched entry */
			*fabric_port = map->portmap[entry+1];
			BCM_SBX_UNLOCK(unit);
			return BCM_E_NONE;
		    }
		}
	    }
	}
	
	*fabric_port = BCM_GPORT_INVALID;
	BCM_SBX_UNLOCK(unit);
	return BCM_E_NOT_FOUND;
    }
}

/* Internal use only */
int
bcm_sbx_stk_fabric_map_get_switch_port(int unit, 
				       bcm_gport_t fabric_port, 
				       bcm_gport_t *switch_port)
{
    uint32 entry;
    bcm_sbx_stack_portmap_block_t * map;
	
    BCM_SBX_LOCK(unit);

    if (fabric_port == BCM_GPORT_INVALID || switch_port == NULL) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }
    if (SOC_IS_SBX_FE2000(unit)) {
        int  qe_mod, nodeid;
        bcm_port_t  qe_port;
        uint32_t femodport=0;
    
        qe_mod = BCM_GPORT_MODPORT_MODID_GET(fabric_port);
        qe_port = BCM_GPORT_MODPORT_PORT_GET(fabric_port);       
        nodeid = BCM_STK_MOD_TO_NODE(qe_mod);

        if(nodeid < 0 || nodeid >= SBX_MAX_NODES) {
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_INIT;
        }
        
        femodport = SOC_SBX_CONTROL(unit)->fabnodeport2feport[nodeid][qe_port];
        
        BCM_GPORT_MODPORT_SET(*switch_port, 
                              femodport >> 16, 
                              femodport & ((1 << 16) -1));
        
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    } else {

	/* Find the entry */
	for(map=SOC_SBX_STATE(unit)->stack_state->gport_map; map!=NULL; map=map->next) {
	    if (map->count > 0) {
		for (entry=0; entry<2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE; entry+=2) {
#if 000
		  soc_cm_print("entry[%d]=0x%08x\n", entry+1, map->portmap[entry+1]);
#endif
		    if (map->portmap[entry+1] == fabric_port) {
			/* return the matched entry */
			*switch_port = map->portmap[entry];
			BCM_SBX_UNLOCK(unit);
			return BCM_E_NONE;
		    }
		}
	    }
	}

	*switch_port = BCM_GPORT_INVALID;
	BCM_SBX_UNLOCK(unit);
	return BCM_E_NOT_FOUND;
    }
}

int
bcm_sbx_stk_steering_unicast_set(int unit,
                                 int steer_id,
                                 bcm_module_t destmod_lo,
                                 bcm_module_t destmod_hi,
                                 int num_queue_groups,
                                 bcm_gport_t *queue_groups)
{
    int result;

    BCM_SBX_LOCK(unit);
    result = MBCM_SBX_DRIVER_CALL(unit,
                                  mbcm_stk_steering_unicast_set,
                                  unit,
                                  steer_id,
                                  destmod_lo,
                                  destmod_hi,
                                  num_queue_groups,
                                  queue_groups);
    BCM_SBX_UNLOCK(unit);
    return result;
}

int
bcm_sbx_stk_steering_multicast_set(int unit,
                                      int steer_id,
                                      bcm_multicast_t mgid_lo,
                                      bcm_multicast_t mgid_hi,
                                      int num_queue_groups,
                                      bcm_gport_t *queue_groups)
{
    int result;

    BCM_SBX_LOCK(unit);
    result = MBCM_SBX_DRIVER_CALL(unit,
                                  mbcm_stk_steering_multicast_set,
                                  unit,
                                  steer_id,
                                  mgid_lo,
                                  mgid_hi,
                                  num_queue_groups,
                                  queue_groups);
    BCM_SBX_UNLOCK(unit);
    return result;
}

int
bcm_sbx_stk_steering_clear(int unit,
                           int steer_id)
{
    int result;

    BCM_SBX_LOCK(unit);
    result = MBCM_SBX_DRIVER_CALL(unit,
                                  mbcm_stk_steering_clear,
                                  unit,
                                  steer_id);
    BCM_SBX_UNLOCK(unit);
    return result;
}

int
bcm_sbx_stk_steering_clear_all(int unit)
{
    int result;

    BCM_SBX_LOCK(unit);
    result = MBCM_SBX_DRIVER_CALL(unit,
                                  mbcm_stk_steering_clear_all,
                                  unit);
    BCM_SBX_UNLOCK(unit);
    return result;
}
