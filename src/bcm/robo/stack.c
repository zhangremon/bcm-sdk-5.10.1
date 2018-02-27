/*
 * $Id: stack.c 1.17 Broadcom SDK $
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
 * File:    stack.c
 * Purpose:    BCM level APIs for stacking applications
 *
 * Note : Not for RoboSwitch currently.
 */

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/stack.h>

#include <soc/drv.h>


int
bcm_robo_stk_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_reinit(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_port_get(int unit, bcm_pbmp_t *sim_p, bcm_pbmp_t *dup_p)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_mode_get(int unit, int *mode)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_mode_set(int unit, int mode)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_modmap_unregister(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_modmap_map(int unit,
                  int setget,
                  bcm_module_t mod_in, bcm_port_t port_in,
                  bcm_module_t *mod_out, bcm_port_t *port_out)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    /* ROBO devices have no stacking design, return the default value */
    /* default identity mapping */
    if (mod_out != NULL) {
        *mod_out = mod_in;
    }
    if (port_out != NULL) {
        *port_out = port_in;
    }
    return BCM_E_NONE;
}

int 
bcm_robo_stk_pbmp_get(int unit, bcm_pbmp_t *cur_pbm,
                            bcm_pbmp_t *inactive_pbm)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_my_modid_get(int unit, int *my_modid)
{
    *my_modid = unit;
    return BCM_E_NONE;
}

int
bcm_robo_stk_my_modid_set(int unit, int my_modid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_modid_get(int unit, int *modid)
{
    if ((unit < 0) || (unit > BCM_UNITS_MAX)) {
        return BCM_E_PARAM;
    }
    *modid = unit;
    return BCM_E_NONE;
}

int
bcm_robo_stk_modid_set(int unit, int modid)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_stk_modid_count(int unit, int *num_modid)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    *num_modid = soc->info.modid_count;

    return BCM_E_NONE;
}


/* XGS Switch specific */
int
bcm_robo_stk_modport_set(int unit, int modid, int port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_modport_get(int unit, int modid, bcm_port_t *port)
{
    if (port == NULL) {
        return BCM_E_PARAM;
    }

    *port = -1;
    return bcm_stk_modport_get_all(unit, modid, 1, port, NULL);
}

int
bcm_robo_stk_modport_get_all(int unit,
                            int modid,
                            int port_max,
                            bcm_port_t *port_array,
                            int *port_count)
{
    if ((unit < 0) || (unit > BCM_UNITS_MAX)) {
	return BCM_E_PARAM;
    }
    if ((modid < 0) || (modid > BCM_UNITS_MAX)) {
        return BCM_E_PARAM;
    }
    if (port_array == NULL) {
        return BCM_E_PARAM;
    }

    if (port_max > 0) {
        *port_array = 0;
        *port_count = 1;
    } else {
        *port_count = 0;
    }

    return BCM_E_NONE;
}

int
bcm_robo_stk_modport_clear(int unit, int modid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_modport_clear_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/* XGS Fabric specific */
int
bcm_robo_stk_ucbitmap_set(int unit, int port, int modid,
                bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_ucbitmap_get(int unit, int port, int modid,
                bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_ucbitmap_del(int unit, bcm_port_t port, int modid,
                                bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_stk_modid_port(int unit, int modid, bcm_port_t *port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_fmod_lmod_mapping_set(int unit, bcm_port_t port,
                    bcm_module_t fmod, bcm_module_t lmod)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_fmod_lmod_mapping_get(int unit, bcm_port_t port,
                    bcm_module_t *fmod, bcm_module_t *lmod)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_fmod_smod_mapping_set(int unit, bcm_port_t port,
                    bcm_module_t fmod, bcm_module_t smod,
                    bcm_port_t sport, uint32 nports)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_fmod_smod_mapping_get(int unit, bcm_port_t port,
                    bcm_module_t fmod, bcm_module_t *smod,
                    bcm_port_t *sport, uint32 *nports)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_modmap_enable_set(int unit, bcm_port_t port, int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_modmap_enable_get(int unit, bcm_port_t port, int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_modmap_register(int unit, bcm_stk_modmap_cb_t func)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_stk_sl_simplex_count_get(int unit, int *count)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_stk_sl_simplex_count_set(int unit, int count)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_stk_update_callback_get(int unit,
                                       bcm_stk_cb_f cb,
                                       void **cookie)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_stk_update_callback_set(int unit,
                                       bcm_stk_cb_f cb,
                                       void *cookie)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_stk_update_callback_register(int a, bcm_stk_cb_f b, void *c)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_stk_update_callback_unregister(int a, bcm_stk_cb_f b, void *c)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_stk_port_modmap_group_set(int unit, bcm_port_t port, int group)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_stk_port_modmap_group_get(int unit, bcm_port_t port, int *group)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_stk_port_modmap_group_max_get(int unit, int *max_group)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_stk_modport_add(int unit, int modid, bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_stk_modport_delete(int unit, int modid, bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_port_modport_set(int unit, bcm_port_t ing_port,
                              bcm_module_t dest_modid, bcm_port_t dest_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_port_modport_get(int unit, bcm_port_t ing_port,
                              bcm_module_t dest_modid, bcm_port_t *dest_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_port_modport_get_all(int unit, bcm_port_t ing_port,
                                  bcm_module_t dest_modid, int dest_port_max,
                                  bcm_port_t *dest_port_array,
                                  int *dest_port_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_port_modport_clear(int unit, bcm_port_t ing_port,
                                bcm_module_t dest_modid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_port_modport_clear_all(int unit, bcm_port_t ing_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_port_modport_add(int unit, bcm_port_t ing_port,
                              bcm_module_t dest_modid, bcm_port_t dest_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_port_modport_delete(int unit, bcm_port_t ing_port,
                                 bcm_module_t dest_modid, bcm_port_t dest_port)
{
    return BCM_E_UNAVAIL;
}
