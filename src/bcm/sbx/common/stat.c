/*
 * $Id: stat.c 1.9.178.1 Broadcom SDK $
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
 * Broadcom StrataSwitch SNMP Statistics API.
 */

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/counter.h>

#include <bcm/error.h>
#include <bcm/stat.h>

#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/stat.h>

bcm_stat_info_t stat_info[BCM_MAX_NUM_UNITS];

int
bcm_sbx_stat_init(int unit)
{
    int rv = BCM_E_NONE;

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_init, unit);

    return rv;
}

int
bcm_sbx_stat_sync(int unit)
{
    int rv = BCM_E_NONE;

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_sync, unit);

    return rv;
}

int
bcm_sbx_stat_get(int unit,
                 bcm_port_t port,
                 bcm_stat_val_t type,
                 uint64 *val)
{
    int        rv = BCM_E_NONE;


    if (port == CMIC_PORT(unit)) {
	return (BCM_E_UNAVAIL);
    } 

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_get, unit, port, type, val);

    return rv;
}

int
bcm_sbx_stat_get32(int unit,
                   bcm_port_t port,
                   bcm_stat_val_t type,
                   uint32 *val)
{
    int       rv = BCM_E_NONE;

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_get32, unit, port, type, val);

    return (rv);
}

int
bcm_sbx_stat_multi_get(int unit, 
		       bcm_port_t port, 
		       int nstat, 
		       bcm_stat_val_t *stat_arr, 
		       uint64 *value_arr)
{
    int        rv = BCM_E_NONE;
        
    if (port == CMIC_PORT(unit)) {
	return (BCM_E_UNAVAIL);
    } 
    
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_multi_get, unit, port, nstat, stat_arr, value_arr);

    return rv;
}

int
bcm_sbx_stat_multi_get32(int unit, 
			 bcm_port_t port, 
			 int nstat, 
			 bcm_stat_val_t *stat_arr, 
			 uint32 *value_arr)
{
    int       rv = BCM_E_NONE;
    
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }
    
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_multi_get32, unit, port, nstat, stat_arr, value_arr);

    return (rv);
}

int 
bcm_sbx_stat_clear(int unit,
                   bcm_port_t port)
{
    int       rv = BCM_E_NONE;

    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_clear, unit, port);

    return (rv);

}

int
bcm_sbx_stat_custom_set(int unit,
                        bcm_port_t port,
                        bcm_stat_val_t type,
                        uint32 flags)
{
    int rv = BCM_E_NONE;

    /* valid port */
    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_set, unit, port, type, flags);

    return (rv);
}

int
bcm_sbx_stat_custom_get(int unit,
                        bcm_port_t port,
                        bcm_stat_val_t type,
                        uint32 *flags)
{
    int rv = BCM_E_NONE;

    /* valid port */
    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_get, unit, port, type, flags);

    return (rv);
}

int 
bcm_sbx_stat_custom_add(int unit,
                        bcm_port_t port,
                        bcm_stat_val_t type,
                        bcm_custom_stat_trigger_t trigger)
{
    int rv = BCM_E_NONE;

    /* port = -1 is valid port in that case */
    if (!SOC_PORT_VALID(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_add, unit, port, type, trigger);

    return (rv);
}

int 
bcm_sbx_stat_custom_delete(int unit,
                           bcm_port_t port,
                           bcm_stat_val_t type, 
                           bcm_custom_stat_trigger_t trigger)
{
    int rv = BCM_E_NONE;

    /* port = -1 is valid port in that case */
    if (!SOC_PORT_VALID(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_delete, unit, port, type, trigger);

    return (rv);
}

int 
bcm_sbx_stat_custom_delete_all(int unit,
                               bcm_port_t port,
                               bcm_stat_val_t type)
{
    int rv = BCM_E_NONE;

    /* port = -1 is valid port in that case */
    if (!SOC_PORT_VALID(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_delete_all, unit, port, type);

    return (rv);
}

int 
bcm_sbx_stat_custom_check(int unit,
                          bcm_port_t port,
                          bcm_stat_val_t type, 
                          bcm_custom_stat_trigger_t trigger,
                          int *result)
{
    int rv = BCM_E_NONE;

    /* valid port */
    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stat_custom_check, unit, port, type, trigger, result);

    return (rv);
}
