/*
 * $Id: stack.c 1.3 Broadcom SDK $
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
 * BM3200 Stack API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pt_auto.h>

#include <bcm/error.h>
#include <bcm/stack.h>


int
bcm_bm3200_stk_modid_set(int unit, int modid)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_bm3200_stk_modid_get(int unit, int *modid)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_bm3200_stk_my_modid_set(int unit, int modid)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_bm3200_stk_my_modid_get(int unit, int *modid)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}
int
bcm_bm3200_stk_module_enable(int unit,
			     int modid,
			     int nports, 
			     int enable)
{
    uint32 uData, ina_enable;
    int node;
    /* initial release, nports is fixed as 50 */


    if (!(BCM_STK_MOD_IS_NODE(modid))) {
        return(BCM_E_PARAM);
    }

    node = BCM_STK_MOD_TO_NODE(modid);

    /* update INA_ENABLE when enable/disable nodes */
    uData = SAND_HAL_READ(unit, PT, FO_CONFIG1);
    ina_enable = SAND_HAL_GET_FIELD(PT, FO_CONFIG1, INA_ENABLE, uData);

    if (enable) {
	ina_enable |= (1 << node);
    } else {
	ina_enable &= ~(1 << node);
    }

    uData = SAND_HAL_MOD_FIELD(PT, FO_CONFIG1, INA_ENABLE, uData, ina_enable);
    SAND_HAL_WRITE(unit, PT, FO_CONFIG1, uData);

    return BCM_E_NONE;
}

int
bcm_bm3200_stk_module_protocol_set(int unit,
				   int node,
				   bcm_module_protocol_t  protocol)
{
    return BCM_E_NONE;
}

int
bcm_bm3200_stk_module_protocol_get(int unit,
				   int node,
				   bcm_module_protocol_t *protocol)
{
    return BCM_E_NONE;
}
