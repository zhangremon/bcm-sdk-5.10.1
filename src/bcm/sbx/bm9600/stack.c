/*
 * $Id: stack.c 1.17.2.2 Broadcom SDK $
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
 * BM9600 Stack API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pl_auto.h>
#include <soc/sbx/bm9600.h>
#include <bcm/error.h>
#include <bcm/stack.h>
#include <bcm/vlan.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/bm9600.h>


int
bcm_bm9600_stk_modid_set(int unit, int modid)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_bm9600_stk_modid_get(int unit, int *modid)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_bm9600_stk_my_modid_set(int unit, int modid)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_bm9600_stk_my_modid_get(int unit, int *modid)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}
int
bcm_bm9600_stk_module_enable(int unit,
			     int modid,
			     int nports, 
			     int enable)
{
    uint32 uData, ina_enable;
    int node, phy_node;
    int rv = BCM_E_NONE;
    /* initial release, nports is fixed */


    if (!(BCM_STK_MOD_IS_NODE(modid))) {
        return(BCM_E_PARAM);
    }

    node = BCM_STK_MOD_TO_NODE(modid);
    phy_node = SOC_SBX_L2P_NODE(unit, node);

    if (phy_node < 24) {
      /* update INA_ENABLE when enable/disable nodes */
      uData = SAND_HAL_READ(unit, PL, FO_CONFIG4);
      ina_enable = SAND_HAL_GET_FIELD(PL, FO_CONFIG4, INA_ENABLE_0_23, uData);
      
      if (enable) {
	ina_enable |= (1 << phy_node);
      } else {
	ina_enable &= ~(1 << phy_node);
      }
      
      uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG4, INA_ENABLE_0_23, uData, ina_enable);
      SAND_HAL_WRITE(unit, PL, FO_CONFIG4, uData);
      
    } else if (phy_node < 48) {
      /* update INA_ENABLE when enable/disable nodes */
      phy_node -= 24;
      uData = SAND_HAL_READ(unit, PL, FO_CONFIG5);
      ina_enable = SAND_HAL_GET_FIELD(PL, FO_CONFIG5, INA_ENABLE_24_47, uData);
      
      if (enable) {
	ina_enable |= (1 << phy_node);
      } else {
	ina_enable &= ~(1 << phy_node);
      }
      
      uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG5, INA_ENABLE_24_47, uData, ina_enable);
      SAND_HAL_WRITE(unit, PL, FO_CONFIG5, uData);
      
    } else {
      /* update INA_ENABLE when enable/disable nodes */
      phy_node -= 48;
      uData = SAND_HAL_READ(unit, PL, FO_CONFIG6);
      ina_enable = SAND_HAL_GET_FIELD(PL, FO_CONFIG6, INA_ENABLE_48_71, uData);
      
      if (enable) {
	ina_enable |= (1 << phy_node);
      } else {
	ina_enable &= ~(1 << phy_node);
      }
      
      uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG6, INA_ENABLE_48_71, uData, ina_enable);
      SAND_HAL_WRITE(unit, PL, FO_CONFIG6, uData);
    }
    return(rv);
}

int
bcm_bm9600_stk_module_protocol_set(int unit,
				   int node,
				   bcm_module_protocol_t  protocol)
{
    uint32 uData;
    uint32 phy_node, phy_reg_node;
    int rv = BCM_E_NONE;

    phy_node = SOC_SBX_L2P_NODE(unit, node);

    /* retreive current configuration */
    if ( (phy_node <= SB_FAB_DEVICE_BM9600_WRD0_HI_NODE) ) {
        phy_reg_node = phy_node;
        uData = SAND_HAL_READ(unit, PL, NM_DUAL_GRANT_CONFIG0);
    } else if ( (phy_node >= SB_FAB_DEVICE_BM9600_WRD1_LOW_NODE) &&
                                             (phy_node <= SB_FAB_DEVICE_BM9600_WRD1_HI_NODE) ) {
        phy_reg_node = phy_node - SB_FAB_DEVICE_BM9600_WRD1_LOW_NODE;
        uData = SAND_HAL_READ(unit, PL, NM_DUAL_GRANT_CONFIG1);
    } else if ( (phy_node >= SB_FAB_DEVICE_BM9600_WRD2_LOW_NODE) &&
                                             (phy_node <= SB_FAB_DEVICE_BM9600_WRD2_HI_NODE) ) {
        phy_reg_node = phy_node - SB_FAB_DEVICE_BM9600_WRD2_LOW_NODE;
        uData = SAND_HAL_READ(unit, PL, NM_DUAL_GRANT_CONFIG2);
    } else {
        return(BCM_E_PARAM);
    }

    /* modify configuration */
    if (protocol == bcmModuleProtocol3) {
        uData |= (1 << phy_reg_node);
    } else {
        uData &= ~(1 << phy_reg_node);
    }

    /* update configuration */
    if ( (phy_node <= SB_FAB_DEVICE_BM9600_WRD0_HI_NODE) ) {
        SAND_HAL_WRITE(unit, PL, NM_DUAL_GRANT_CONFIG0, uData);
    } else if ( (phy_node >= SB_FAB_DEVICE_BM9600_WRD1_LOW_NODE) &&
                                             (phy_node <= SB_FAB_DEVICE_BM9600_WRD1_HI_NODE) ) {
        SAND_HAL_WRITE(unit, PL, NM_DUAL_GRANT_CONFIG1, uData);
    } else if ( (phy_node >= SB_FAB_DEVICE_BM9600_WRD2_LOW_NODE) &&
                                             (phy_node <= SB_FAB_DEVICE_BM9600_WRD2_HI_NODE) ) {
        SAND_HAL_WRITE(unit, PL, NM_DUAL_GRANT_CONFIG2, uData);
    }

    if ((protocol == bcmModuleProtocol3) || (protocol == bcmModuleProtocol4) || (protocol == bcmModuleProtocol5)) {
        uData = SAND_HAL_READ_STRIDE(unit, PL, AI, phy_node, AI_CONFIG);
        uData = SAND_HAL_MOD_FIELD(PL, AI_CONFIG, QE40_MODE, uData, 1);
        SAND_HAL_WRITE_STRIDE(unit, PL, AI, phy_node, AI_CONFIG, uData);
    } else {
        uData = SAND_HAL_READ_STRIDE(unit, PL, AI, phy_node, AI_CONFIG);
        uData = SAND_HAL_MOD_FIELD(PL, AI_CONFIG, QE40_MODE, uData, 0);
        SAND_HAL_WRITE_STRIDE(unit, PL, AI, phy_node, AI_CONFIG, uData);
    }
    
    /* phy_node = SI port which is SCI link */
    rv = bcm_bm9600_ability_matching_speed_set(unit, phy_node, SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[phy_node]); 
    if (rv) {
        SOC_ERROR_PRINT((DK_ERR,
                         "protocol_set: unit %d problem setting ability for SCI link for physical node(%d)\n",
                         unit, phy_node));
    }

    /* update ina_pri_full_thresh setting */
    uData = SAND_HAL_READ_STRIDE(unit, PL, INA, phy_node, INA_PRI_FULL_THRESH);
    if ( (!soc_feature(unit, soc_feature_egr_independent_fc)) &&
                                                         (protocol == bcmModuleProtocol3) ) {
        /* 2 bit flow control and dual grant configuration */

        /* setup pri_full_thresh corresponding to hungry, satisfied levels */
        if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
            uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI8_FULL_THRESH, uData, 0x2);
            uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI3_FULL_THRESH, uData, 0x1);
        }
        else {
            uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI11_FULL_THRESH, uData, 0x2);
            uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI2_FULL_THRESH, uData, 0x1);
        }
    }
    else {
        /* !(2 bit flow control and dual grant configuration) */

        /* setup pri_full_thresh corresponding to hungry, satisfied levels */
        if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
            uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI8_FULL_THRESH, uData, 0x1);
            uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI3_FULL_THRESH, uData, 0x1);
        }
        else {
            uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI11_FULL_THRESH, uData, 0x1);
            uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI2_FULL_THRESH, uData, 0x1);
        }
    }

    /* SDK-31881 set pri full threshold for QE2000 to 0 for holdpri       */
    /* Only if QE2000 node, must set full threshold to 0 or 3 for holdpri */
    if ( (protocol == bcmModuleProtocol1) || 
	 (protocol == bcmModuleProtocol2) )  {
	if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
	    uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI13_FULL_THRESH, uData, 0x0);
	}
	else {
	    uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI12_FULL_THRESH, uData, 0x0);
	}
    }

    SAND_HAL_WRITE_STRIDE(unit, PL, INA, phy_node, INA_PRI_FULL_THRESH, uData);

    /* for QE2000 node, set link active select to be 1, all others to be 0 */
    if (phy_node < 1*12) {
	uData = SAND_HAL_READ(unit, PL, FO_CONFIG7);
    } else if (phy_node < 2*12) {
	uData = SAND_HAL_READ(unit, PL, FO_CONFIG8);
    } else if (phy_node < 3*12) {
	uData = SAND_HAL_READ(unit, PL, FO_CONFIG9);
    } else if (phy_node < 4*12) {
	uData = SAND_HAL_READ(unit, PL, FO_CONFIG10);
    } else if (phy_node < 5*12) {
	uData = SAND_HAL_READ(unit, PL, FO_CONFIG11);
    } else {
	uData = SAND_HAL_READ(unit, PL, FO_CONFIG12);
    }

    uData &= ~(0x3 << ((phy_node % 12) * 2));

    if ( (protocol == bcmModuleProtocol1) || 
	 (protocol == bcmModuleProtocol2) )  {
	uData |= (0x1 << ((phy_node % 12) * 2));
    } else {
	uData |= (0x0 << ((phy_node % 12) * 2));    
    }

    if (phy_node < 1*12) {
        SAND_HAL_WRITE(unit, PL, FO_CONFIG7, uData);
    } else if (phy_node < 2*12) {
        SAND_HAL_WRITE(unit, PL, FO_CONFIG8, uData);
    } else if (phy_node < 3*12) {
        SAND_HAL_WRITE(unit, PL, FO_CONFIG9, uData);
    } else if (phy_node < 4*12) {
        SAND_HAL_WRITE(unit, PL, FO_CONFIG10, uData);
    } else if (phy_node < 5*12) {
        SAND_HAL_WRITE(unit, PL, FO_CONFIG11, uData);
    } else {
        SAND_HAL_WRITE(unit, PL, FO_CONFIG12, uData);
    }

    return(rv);
}

int
bcm_bm9600_stk_module_protocol_get(int unit,
				   int node,
				   bcm_module_protocol_t *protocol)
{
    return BCM_E_NONE;
}
