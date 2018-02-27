/*
 * $Id: phy84728.c 1.3.2.8 Broadcom SDK $
 *
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
 * File:        phy84728.c
 * Purpose:     SDK PHY driver for BCM84728 (FCMAP)
 *
 * Supported BCM546X Family of PHY devices:
 *
 *      Device  Ports   Media                           MAC Interface
 *      84728    4       2 10G SFP+, 2 1G SFP            XFI
 *
 * Workarounds:
 *
 * References:
 *     
 * Notes:
 */ 


#include <sal/types.h>
#include <sal/core/spl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>

#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>

#include "phydefs.h"      /* Must include before other phy related includes */



#if defined(INCLUDE_PHY_84728)
#include "phyconfig.h"    /* Must be the first phy include after phydefs.h */
#include "phyident.h"
#include "phyreg.h"
#include "phynull.h"
#include "phyfege.h"
#include "phyxehg.h"

#include "phy84728_int.h"
#include "phy84728.h"

#define ADVERT_ALL_COPPER \
        (SOC_PA_PAUSE | SOC_PA_SPEED_10MB | \
         SOC_PA_SPEED_100MB | SOC_PA_SPEED_1000MB)

#define ADVERT_ALL_FIBER \
        (SOC_PA_PAUSE | SOC_PA_SPEED_1000MB) 

#define ADVERT_ALL_10GFIBER \
        (SOC_PA_PAUSE | SOC_PA_SPEED_10GB) 


#define PHY84728_REG_READ(_unit, _phy_ctrl, _addr, _val) \
            READ_PHY_REG((_unit), (_phy_ctrl), (_addr), (_val))

#define READ_PHY84728_PMA_PMD_REG(_unit, _phy_ctrl, _reg,_val) \
            PHY84728_REG_READ((_unit), (_phy_ctrl), \
              SOC_PHY_CLAUSE45_ADDR(1, (_reg)), (_val))


#define BSDK_PHY84728_LINE_MAC_PORT(p)    (p)
#define BSDK_PHY84728_SWITCH_MAC_PORT(p)  ((p) + 1)

/*
 * PHY Map structure for mapping mac drivers
 */
typedef struct phy84728_map_s {
    bsdk_phy84728_intf_t line_mode;     /* configured line mode */

}phy84728_map_t;

#define PHY84728_MAP(_pc) ((phy84728_map_t *)((_pc) + 1))

/* OUI from ID0 and ID1 registers contents */
#define BSDK_PHY84728_PHY_OUI(id0, id1)                          \
    bsdk_phy84728_bit_rev_by_byte_word32((uint32)(id0) << 6 | \
                                           ((id1) >> 10 & 0x3f))

/* PHY model from ID0 and ID1 register Contents */
#define BSDK_PHY84728_PHY_MODEL(id0, id1) ((id1) >> 4 & 0x3f)

/* PHY revision from ID0 and ID1 register Contents */
#define BSDK_PHY84728_PHY_REV(id0, id1)   ((id1) & 0xf)

/*
 * Function:     
 *    bsdk_bit_rev_by_byte_word32
 * Purpose:    
 *    Reverse the bits in each byte of a 32 bit long 
 * Parameters:
 *    n - 32bit input
 * Notes: 
 */
STATIC uint32
bsdk_phy84728_bit_rev_by_byte_word32(uint32 n)
{
    n = (((n & 0xaaaaaaaa) >> 1) | ((n & 0x55555555) << 1));
    n = (((n & 0xcccccccc) >> 2) | ((n & 0x33333333) << 2));
    n = (((n & 0xf0f0f0f0) >> 4) | ((n & 0x0f0f0f0f) << 4));
    return n;
}

STATIC int phy_84728_init(int unit, soc_port_t port);
STATIC int phy_84728_link_get(int unit, soc_port_t port, int *link);
STATIC int phy_84728_enable_set(int unit, soc_port_t port, int enable);
STATIC int phy_84728_duplex_set(int unit, soc_port_t port, int duplex);
STATIC int phy_84728_duplex_get(int unit, soc_port_t port, int *duplex);
STATIC int phy_84728_speed_set(int unit, soc_port_t port, int speed);
STATIC int phy_84728_speed_get(int unit, soc_port_t port, int *speed);
#if 0
STATIC int phy_84728_master_set(int unit, soc_port_t port, int master);
STATIC int phy_84728_master_get(int unit, soc_port_t port, int *master);
#endif
STATIC int phy_84728_an_set(int unit, soc_port_t port, int autoneg);
STATIC int phy_84728_an_get(int unit, soc_port_t port,
                                int *autoneg, int *autoneg_done);
STATIC int phy_84728_lb_set(int unit, soc_port_t port, int enable);
STATIC int phy_84728_lb_get(int unit, soc_port_t port, int *enable);
STATIC int phy_84728_medium_config_set(int unit, soc_port_t port, 
                                      soc_port_medium_t  medium,
                                      soc_phy_config_t  *cfg);
STATIC int phy_84728_medium_config_get(int unit, soc_port_t port, 
                                      soc_port_medium_t medium,
                                      soc_phy_config_t *cfg);
STATIC int phy_84728_ability_advert_set(int unit, soc_port_t port, 
                                       soc_port_ability_t *ability);
STATIC int phy_84728_ability_advert_get(int unit, soc_port_t port,
                                       soc_port_ability_t *ability);
STATIC int phy_84728_ability_remote_get(int unit, soc_port_t port,
                                       soc_port_ability_t *ability);
STATIC int phy_84728_ability_local_get(int unit, soc_port_t port, 
                                soc_port_ability_t *ability);
STATIC int phy_84728_firmware_set(int unit, int port, int offset, 
                                 uint8 *data, int len);
STATIC int phy_84728_control_set(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 value);
STATIC int phy_84728_control_get(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 *value);
STATIC int phy_84728_linkup(int unit, soc_port_t port);

STATIC int
_bsdk_phy84728_system_sgmii_init(phy_ctrl_t *pc, int dev_port);
STATIC int
_bsdk_phy84728_system_sgmii_duplex_set(phy_ctrl_t *pc, int dev_port, int duplex);
STATIC int
_bsdk_phy84728_system_sgmii_sync(phy_ctrl_t *pc, int dev_port);

/*
 * Function:     
 *    _phy84728_system_xfi_speed_set
 * Purpose:    
 *    To set system side SGMII speed
 * Parameters:
 *    phy_id    - PHY's device address
 *    speed     - Speed to set
 *               100000 = 10Gbps
 * Returns:    
 */
static int
_phy84728_system_xfi_speed_set(phy_ctrl_t *pc, int dev_port, int speed)
{
    if (speed != 10000) {
        return SOC_E_CONFIG;
    }
    /* Disable Clause 37 Autoneg */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_XFI_DEV7_AN_MII_CTRLr(pc, 0,
                                  BSDK_PHY84728_AN_MII_CTRL_AE));
    /* Force Speed in PDM */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_SYS_DEV1_PMD_CTRLr(pc, 
                          BSDK_PHY84728_PMD_CTRL_SS_10000,
                          BSDK_PHY84728_PMD_CTRL_10GSS_MASK));
    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_84728_medium_config_update
 * Purpose:
 *      Update the PHY with config parameters
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      cfg - Config structure.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
_phy_84728_medium_config_update(int unit, soc_port_t port,
                                soc_phy_config_t *cfg)
{

    SOC_IF_ERROR_RETURN
        (phy_84728_speed_set(unit, port, cfg->force_speed));
    SOC_IF_ERROR_RETURN
        (phy_84728_duplex_set(unit, port, cfg->force_duplex));
    SOC_IF_ERROR_RETURN
        (phy_84728_ability_advert_set(unit, port, &cfg->advert_ability));
    SOC_IF_ERROR_RETURN
        (phy_84728_an_set(unit, port, cfg->autoneg_enable));

    return SOC_E_NONE;
}

/* Function:
 *    phy_84728_init
 * Purpose:    
 *    Initialize 84728 phys
 * Parameters:
 *    unit - StrataSwitch unit #.
 *    port - StrataSwitch port #. 
 * Returns:    
 *    SOC_E_NONE
 */

STATIC int
phy_84728_init(int unit, soc_port_t port)
{
    phy_ctrl_t *pc;
    int line_mode;
    int fiber_preferred;
    int phy_ext_rom_boot;
    /*
    _phy84728_event_callback_data_t *tmp;
    */
    /*uint16          ctrl, tmp2;*/
    phy_ctrl_t *int_pc;

    pc     = EXT_PHY_SW_STATE(unit, port);

    sal_memset(PHY84728_MAP(pc), 0, sizeof(phy84728_map_t));


    phy_ext_rom_boot = soc_property_port_get(unit, port,
                                            spn_PHY_EXT_ROM_BOOT, 1);

    SOC_DEBUG_PRINT((DK_PHY, "phy_84728_init: u=%d p=%d\n",unit, port));

    /* Preferred mode:
     * phy_fiber_pref = 1 ; Port is in SFI mode in 10G Default
     *                      when speed is changed to 1G, its 1000X
     * phy_fiber_pref = 0; Port is in SGMII mode, allows only 1G/100M/10M
     */
    fiber_preferred =
        soc_property_port_get(unit, port, spn_PHY_FIBER_PREF, 1);

    /* Initially configure for the preferred medium. */
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_COPPER);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_PASSTHRU);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_100FX);

    if (fiber_preferred) {
        line_mode = BSDK_PHY84728_INTF_SFI;
        pc->copper.enable = FALSE;
        pc->fiber.enable = TRUE;
        pc->interface = SOC_PORT_IF_XFI;
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_FIBER);
    } else {
        pc->interface = SOC_PORT_IF_SGMII;
        line_mode = BSDK_PHY84728_INTF_SGMII;
        pc->copper.enable = TRUE;
        pc->fiber.enable = FALSE;
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_COPPER);
    }
    PHY_FLAGS_SET(unit, port, PHY_FLAGS_C45);

    pc->copper.preferred = !fiber_preferred;
    pc->copper.autoneg_enable = TRUE;
    pc->copper.autoneg_advert = ADVERT_ALL_COPPER;
    pc->copper.force_speed = 1000;
    pc->copper.force_duplex = TRUE;
    pc->copper.master = SOC_PORT_MS_AUTO;
    pc->copper.mdix = SOC_PORT_MDIX_AUTO;

    pc->fiber.preferred = TRUE;
    pc->fiber.autoneg_enable = FALSE;
    pc->fiber.autoneg_advert = ADVERT_ALL_FIBER;
    pc->fiber.force_speed = 10000;
    pc->fiber.force_duplex = TRUE;
    pc->fiber.master = SOC_PORT_MS_NONE;
    pc->fiber.mdix = SOC_PORT_MDIX_NORMAL;

    SOC_IF_ERROR_RETURN(
            bsdk_phy84728_reset(pc));

    SOC_IF_ERROR_RETURN(
            bsdk_phy84728_no_reset_setup(pc,
                                           line_mode,
                                           phy_ext_rom_boot));

    /* Get Requested LED selectors (defaults are hardware defaults) */
    pc->ledmode[0] = soc_property_port_get(unit, port, spn_PHY_LED1_MODE, 0);
    pc->ledmode[1] = soc_property_port_get(unit, port, spn_PHY_LED2_MODE, 1);
    pc->ledmode[2] = soc_property_port_get(unit, port, spn_PHY_LED3_MODE, 3);
    pc->ledmode[3] = soc_property_port_get(unit, port, spn_PHY_LED4_MODE, 6);
    pc->ledctrl = soc_property_port_get(unit, port, spn_PHY_LED_CTRL, 0x8);
    pc->ledselect = soc_property_port_get(unit, port, spn_PHY_LED_SELECT, 0);


        SOC_IF_ERROR_RETURN
            (_phy_84728_medium_config_update(unit, port,
                                    PHY_COPPER_MODE(unit, port) ?
                                    &pc->copper :
                                    &pc->fiber));

        int_pc = INT_PHY_SW_STATE(unit, port);
        if (NULL != int_pc) {
            PHY_AUTO_NEGOTIATE_SET (int_pc->pd, unit, port, 0);
            PHY_SPEED_SET(int_pc->pd, unit, port, 10000);
        }

    return SOC_E_NONE;
}


/*
 * Function:
 *    phy_84728_link_get
 * Purpose:
 *    Get layer2 connection status.
 * Parameters:
 *    unit - StrataSwitch unit #.
 *    port - StrataSwitch port #. 
 *      link - address of memory to store link up/down state.
 * Returns:    
 *    SOC_E_NONE
 */

STATIC int
phy_84728_link_get(int unit, soc_port_t port, int *link)
{
    phy_ctrl_t *pc;
    int rv = SOC_E_NONE;
    bsdk_phy84728_intf_t bsdk_mode;
    int bsdk_link;

    pc    = EXT_PHY_SW_STATE(unit, port);
    *link = FALSE;      /* Default return */


    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
        return SOC_E_NONE;
    }


    rv = bsdk_phy84728_line_intf_get(pc, 0, &bsdk_mode);
    if (!SOC_SUCCESS(rv)) {
        return SOC_E_FAIL;
    }

    if (bsdk_mode == BSDK_PHY84728_INTF_SGMII) {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_COPPER);
        PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
    } else {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_FIBER);
        PHY_FLAGS_CLR(unit, port, PHY_FLAGS_COPPER);
    }

    rv = bsdk_phy84728_link_get(pc, &bsdk_link);
    if (!SOC_SUCCESS(rv)) {
        return SOC_E_FAIL;
    }

    if (bsdk_link) {
        *link = TRUE;
    } else {
        *link = FALSE;
    }
    SOC_DEBUG_PRINT((DK_PHY | DK_VERBOSE,
                     "phy_84728_link_get: u=%d p=%d link=%d\n",
                      unit, port,
                      *link));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84728_enable_set
 * Purpose:
 *      Enable or disable the physical interface.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      enable - Boolean, true = enable PHY, false = disable.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_84728_enable_set(int unit, soc_port_t port, int enable)
{
    phy_ctrl_t *pc;
    uint16 power;

    pc     = EXT_PHY_SW_STATE(unit, port);

    if (enable && !(PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE))) {
        /* Already enabled state */
        return SOC_E_NONE;
    } else {
        if (!enable && PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
            /* Already Disabled */
            return SOC_E_NONE;
        }
    }

    if (enable) {
        PHY_FLAGS_CLR(unit, port, PHY_FLAGS_DISABLE);
    } else {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_DISABLE);
    }

    power = (enable) ? 0 : 1;

    /* PMA/PMD */
    SOC_IF_ERROR_RETURN(
    BSDK_MOD_PHY84728_LN_DEV1_PMD_CTRLr(pc, power, 
                                         BSDK_PHY84728_PMD_CTRL_PD));
    /* PCS */
    SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV3_PCS_CTRLr(pc, power, 
                                         BSDK_PHY84728_PCS_PCS_CTRL_PD));
    /* AN */
    SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, power, 
                                         BSDK_PHY84728_AN_MII_CTRL_PD));


    SOC_DEBUG_PRINT((DK_PHY, "phy_84728_enable_set: "
             "Power %s fiber medium\n", (enable) ? "up" : "down"));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84728_duplex_set
 * Purpose:
 *      Set the current duplex mode
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      duplex - Boolean, true indicates full duplex, false indicates half.
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_UNAVAIL - Half duplex requested, and not supported.
 * Notes:
 */
STATIC int
phy_84728_duplex_set(int unit, soc_port_t port, int duplex_in)
{
    int                          rv = SOC_E_NONE ;
    phy_ctrl_t                   *pc;
    bsdk_phy84728_duplex_t    duplex;
    uint16 mii_ctrl;
    bsdk_phy84728_intf_t line_mode;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (duplex_in) {
        duplex = BSDK_PHY84728_FULL_DUPLEX;
    } else {
        duplex = BSDK_PHY84728_HALF_DUPLEX;
    }


    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if ((line_mode == BSDK_PHY84728_INTF_1000X ) ||
        (line_mode == BSDK_PHY84728_INTF_SFI )) {

        if (duplex == BSDK_PHY84728_FULL_DUPLEX) {
            rv =  SOC_E_NONE;
            goto exit;
        } else {
            rv =  SOC_E_UNAVAIL;
            goto exit;
        }
    }
    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, &mii_ctrl));

    if (duplex == BSDK_PHY84728_FULL_DUPLEX) {
        mii_ctrl |= BSDK_PHY84728_AN_MII_CTRL_FD;
    } else {
        if(duplex == BSDK_PHY84728_HALF_DUPLEX) {
        mii_ctrl &= ~BSDK_PHY84728_AN_MII_CTRL_FD;
        } else {
            /* coverity[dead_error_begin] */
            rv = SOC_E_CONFIG; 
            goto exit;
        }
    }
    SOC_IF_ERROR_RETURN(
        BSDK_WR_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, mii_ctrl));

exit:

    if(SOC_SUCCESS(rv)) {
        pc->copper.force_duplex = duplex_in;
    }


    SOC_DEBUG_PRINT((DK_PHY,
                    "phy_84728_duplex_set: u=%d p=%d d=%d rv=%d\n",
                     unit, port, duplex, rv));
    return rv;
}

/*
 * Function:
 *      phy_84728_duplex_get
 * Purpose:
 *      Get the current operating duplex mode. If autoneg is enabled,
 *      then operating mode is returned, otherwise forced mode is returned.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      duplex - (OUT) Boolean, true indicates full duplex, false
 *              indicates half.
 * Returns:
 *      SOC_E_NONE
 * Notes:
 *      The duplex is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level. Autonegotiation is
 *      not manipulated.
 */

STATIC int
phy_84728_duplex_get(int unit, soc_port_t port, int *duplex)
{

    phy_ctrl_t                   *pc;
    uint16	status;
    bsdk_phy84728_intf_t line_mode;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if ((line_mode == BSDK_PHY84728_INTF_1000X ) ||
        (line_mode == BSDK_PHY84728_INTF_SFI )) {
        *duplex = TRUE;
    } else {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV7_AN_BASE1000X_STAT1r(pc, &status));
        if (status & (1U << 2)) {
            *duplex = TRUE;
        } else {
            *duplex = FALSE;
        }
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *      phy_84728_speed_set
 * Purpose:
 *      Set PHY speed
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      speed - link speed in Mbps
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
phy_84728_speed_set(int unit, soc_port_t port, int speed)
{
    phy_ctrl_t *pc;
    phy_ctrl_t  *int_pc;
    int            rv; 

    pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

    if (speed > 10000) {
        return SOC_E_UNAVAIL;
    }

    if(PHY_FIBER_MODE(unit, port)) {
        if (speed < 1000) {
            return SOC_E_UNAVAIL;
        }
    }

    rv = bsdk_phy84728_speed_set(pc, speed);
    if(SOC_SUCCESS(rv)) {
        if (NULL != int_pc) {
            PHY_AUTO_NEGOTIATE_SET (int_pc->pd, unit, port, 0);
            PHY_SPEED_SET(int_pc->pd, unit, port, speed);
        }
        if(PHY_COPPER_MODE(unit, port)) {
            pc->copper.force_speed = speed;
        } else {
            if(PHY_FIBER_MODE(unit, port)) {
                pc->fiber.force_speed = speed;
            }
        }
    }
    SOC_DEBUG_PRINT((DK_PHY,
                     "phy_84728_speed_set: u=%d p=%d s=%d fiber=%d rv=%d\n",
                     unit, port, speed, PHY_FIBER_MODE(unit, port), rv));

    return rv;
}

/*
 * Function:
 *      phy_84728_speed_get
 * Purpose:
 *      Get the current operating speed for a 84728 device.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      speed - (OUT) Speed of the phy
 * Returns:
 *      SOC_E_NONE
 * Notes:
 *      The speed is retrieved for the ACTIVE medium.
 */

STATIC int
phy_84728_speed_get(int unit, soc_port_t port, int *speed)
{
    int rv = SOC_E_NONE;
    phy_ctrl_t    *pc;
    uint16 data = 0;
    uint16 fval = 0;
    bsdk_phy84728_intf_t line_mode;

    if (speed == NULL) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    fval = 0;
    {
        SOC_IF_ERROR_RETURN(
            bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

        if (line_mode == BSDK_PHY84728_INTF_SFI) {
            *speed = 10000;
            return SOC_E_NONE;
        }
        if (line_mode == BSDK_PHY84728_INTF_1000X) {
            *speed = 1000;
            return SOC_E_NONE;
        }
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV1_SPEED_LINK_DETECT_STATr(pc, &data));
        switch(data & BSDK_PHY84728_PMD_SPEED_LD_STATr_LN_PDM_SPEED_MASK) {
        case BSDK_PHY84728_PMD_SPEED_LD_STATr_LN_PDM_SPEED_10M:
            *speed = 10;
            break;
        case BSDK_PHY84728_PMD_SPEED_LD_STATr_LN_PDM_SPEED_100M:
            *speed = 100;
            break;
        case BSDK_PHY84728_PMD_SPEED_LD_STATr_LN_PDM_SPEED_1G:
            *speed = 1000;
            break;
        case BSDK_PHY84728_PMD_SPEED_LD_STATr_LN_PDM_SPEED_10G:
            *speed = 10000;
            break;
        default :
            *speed = 0;
            break;
        }
    }

    if(!SOC_SUCCESS(rv)) {
        SOC_DEBUG_PRINT((DK_WARN,
                        "phy_84728_speed_get: u=%d p=%d invalid speed\n",
                         unit, port));
    } else {
        SOC_DEBUG_PRINT((DK_PHY,
                        "phy_84728_speed_get: u=%d p=%d speed=%d",
                         unit, port, *speed));
    }

    return rv;
}

/*
 * Function:
 *      phy_84728_an_set
 * Purpose:
 *      Enable or disable auto-negotiation on the specified port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      autoneg - Boolean, if true, auto-negotiation is enabled
 *              (and/or restarted). If false, autonegotiation is disabled.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The autoneg mode is set only for the ACTIVE medium.
 */

STATIC int
phy_84728_an_set(int unit, soc_port_t port, int autoneg)
{
    int                 rv;
    phy_ctrl_t   *pc;
    int autoneg_advert = 0;
    uint16 data = 0;

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

    if(PHY_COPPER_MODE(unit, port)) {
        autoneg_advert = 0;
    } else {
        if(PHY_FIBER_MODE(unit, port)) {
            autoneg_advert = pc->fiber.autoneg_advert;
        }
    }

    SOC_IF_ERROR_RETURN
        (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0x0096, autoneg ? 0x2 : 0x0));

    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_XFI_DEV7_AN_CTRLr(pc, &data));
    if (autoneg) {
        data |= (BSDK_PHY84728_AN_AN_CTRL_AE |
                BSDK_PHY84728_AN_AN_CTRL_RAN);
    } else {
        data &= ~(BSDK_PHY84728_AN_AN_CTRL_AE |
                  BSDK_PHY84728_AN_AN_CTRL_RAN);
    }
    SOC_IF_ERROR_RETURN(
        BSDK_WR_PHY84728_XFI_DEV7_AN_CTRLr(pc, data));


    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, &data));
    if (autoneg) {
        data |= (BSDK_PHY84728_AN_MII_CTRL_AE | 
                 BSDK_PHY84728_AN_MII_CTRL_RAN);
    } else {
        data &= ~(BSDK_PHY84728_AN_MII_CTRL_AE | 
                  BSDK_PHY84728_AN_MII_CTRL_RAN);
    }
    SOC_IF_ERROR_RETURN(
        BSDK_WR_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, data));

    if(SOC_SUCCESS(rv)) {
        if(PHY_COPPER_MODE(unit, port)) {
            pc->copper.autoneg_enable = autoneg ? TRUE : FALSE;
        } else {
            if(PHY_FIBER_MODE(unit, port)) {
                pc->fiber.autoneg_enable = autoneg ? TRUE : FALSE;
            }
        }
    }

    SOC_DEBUG_PRINT((DK_PHY,
                     "phy_84728_an_set: u=%d p=%d autoneg=%d rv=%d\n",
                     unit, port, autoneg, rv));
    return rv;
}

/*
 * Function:
 *      phy_84728_an_get
 * Purpose:
 *      Get the current auto-negotiation status (enabled/busy).
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      autoneg - (OUT) if true, auto-negotiation is enabled.
 *      autoneg_done - (OUT) if true, auto-negotiation is complete. This
 *              value is undefined if autoneg == FALSE.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The autoneg mode is retrieved for the ACTIVE medium.
 */

STATIC int
phy_84728_an_get(int unit, soc_port_t port,
                     int *autoneg, int *autoneg_done)
{
    int           rv;
    phy_ctrl_t   *pc;
    uint16 an_ctrl = 0, an_stat = 0;
    uint16 an_mii_ctrl = 0, an_mii_stat = 0;
    bsdk_phy84728_intf_t line_mode;

    pc = EXT_PHY_SW_STATE(unit, port);

    rv            = SOC_E_NONE;
    *autoneg      = FALSE;
    *autoneg_done = FALSE;

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_SFI) {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV7_AN_CTRLr(pc, &an_ctrl));
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV7_AN_STATr(pc, &an_stat));
        *autoneg = (an_ctrl & BSDK_PHY84728_AN_AN_CTRL_AE) ? 1 : 0;
        *autoneg_done = (an_stat & BSDK_PHY84728_AN_AN_STAT_AN_DONE) ? 1 : 0;
    } else {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, &an_mii_ctrl));
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV7_AN_MII_STATr(pc, &an_mii_stat));
        *autoneg = (an_mii_ctrl & BSDK_PHY84728_AN_MII_CTRL_AE) ? 1 : 0;
        *autoneg_done = 
            (an_mii_stat & BSDK_PHY84728_AN_MII_STAT_AN_DONE) ? 1 : 0;
    }

    return rv;
}

/*
 * Function:
 *      phy_84728_lb_set
 * Purpose:
 *      Set the local PHY loopback mode.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      loopback - Boolean: true = enable loopback, false = disable.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The loopback mode is set only for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
phy_84728_lb_set(int unit, soc_port_t port, int enable)
{
    int           rv;
    phy_ctrl_t    *pc;
    uint16 data = 0;
    bsdk_phy84728_intf_t line_mode;

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;


    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_SFI) {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV1_PMD_CTRLr(pc, &data));
        data &= ~(1);
        data |= (enable) ? 1 : 0x0000;
        SOC_IF_ERROR_RETURN(
            BSDK_WR_PHY84728_LN_DEV1_PMD_CTRLr(pc, data));
    } else {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, &data));
        data &= ~BSDK_PHY84728_AN_MII_CTRL_LE;
        data |= (enable) ? BSDK_PHY84728_AN_MII_CTRL_LE : 0;
        SOC_IF_ERROR_RETURN(
            BSDK_WR_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, data));
    }

    if (rv == SOC_E_NONE) {
        /* wait for link up when loopback is enabled */
        
        if (enable) {
            sal_usleep(2000000);
        }
    }

    SOC_DEBUG_PRINT((DK_PHY,
                    "phy_84728_lb_set: u=%d p=%d en=%d rv=%d\n", 
                    unit, port, enable, rv));

    return rv; 
}

/*
 * Function:
 *      phy_84728_lb_get
 * Purpose:
 *      Get the local PHY loopback mode.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      loopback - (OUT) Boolean: true = enable loopback, false = disable.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The loopback mode is retrieved for the ACTIVE medium.
 */

STATIC int
phy_84728_lb_get(int unit, soc_port_t port, int *enable)
{
    int                  rv;
    phy_ctrl_t    *pc;
    uint16 data = 0;
    bsdk_phy84728_intf_t line_mode;

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_SFI) {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV1_PMD_CTRLr(pc, &data));
        *enable = (data & 1) ? 1 : 0;
    } else {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, &data));
        *enable = (data & BSDK_PHY84728_AN_MII_CTRL_LE) ? 1 : 0;
    }

    return rv; 
}

/*
 * Function:
 *      phy_84728_medium_config_set
 * Purpose:
 *      Set the operating parameters that are automatically selected
 *      when medium switches type.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - Port number
 *      medium - SOC_PORT_MEDIUM_COPPER/FIBER
 *      cfg - Operating parameters
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_84728_medium_config_set(int unit, soc_port_t port, 
                           soc_port_medium_t  medium,
                           soc_phy_config_t  *cfg)
{
    phy_ctrl_t    *pc;
#if 0
    soc_phy_config_t *active_medium;  /* Currently active medium */
#endif
    soc_phy_config_t *change_medium;  /* Requested medium */
    soc_phy_config_t *other_medium;   /* The other medium */

    int               medium_update;
    soc_port_mode_t   advert_mask;

    if (NULL == cfg) {
        return SOC_E_PARAM;
    }

    pc            = EXT_PHY_SW_STATE(unit, port);
    medium_update = FALSE;

   switch (medium) {
    case SOC_PORT_MEDIUM_COPPER:
        if (!pc->automedium && !PHY_COPPER_MODE(unit, port)) {
            return SOC_E_UNAVAIL;
        }
        change_medium  = &pc->copper;
        other_medium   = &pc->fiber;
        advert_mask    = ADVERT_ALL_COPPER;
        break;
    case SOC_PORT_MEDIUM_FIBER:
        if (!pc->automedium && !PHY_FIBER_MODE(unit, port)) {
            return SOC_E_UNAVAIL;
        }
        change_medium  = &pc->fiber;
        other_medium   = &pc->copper;
        advert_mask    = ADVERT_ALL_FIBER;
        break;
    default:
        return SOC_E_PARAM;
    }

    /*
     * Changes take effect immediately if the target medium is active or
     * the preferred medium changes.
     */
    if (change_medium->enable != cfg->enable) {
        medium_update = TRUE;
    }
    if (change_medium->preferred != cfg->preferred) {
        /* Make sure that only one medium is preferred */
        other_medium->preferred = !cfg->preferred;
        medium_update = TRUE;
    }

    sal_memcpy(change_medium, cfg, sizeof(*change_medium));
    change_medium->autoneg_advert &= advert_mask;

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84728_medium_config_get
 * Purpose:
 *      Get the operating parameters that are automatically selected
 *      when medium switches type.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - Port number
 *      medium - SOC_PORT_MEDIUM_COPPER/FIBER
 *      cfg - (OUT) Operating parameters
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_84728_medium_config_get(int unit, soc_port_t port, 
                           soc_port_medium_t medium,
                           soc_phy_config_t *cfg)
{
    int            rv;
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

    switch (medium) {
    case SOC_PORT_MEDIUM_COPPER:
        if (pc->automedium || PHY_COPPER_MODE(unit, port)) {
            sal_memcpy(cfg, &pc->copper, sizeof (*cfg));
        } else {
            rv = SOC_E_UNAVAIL;
        }
        break;
    case SOC_PORT_MEDIUM_FIBER:
        if (pc->automedium || PHY_FIBER_MODE(unit, port)) {
            sal_memcpy(cfg, &pc->fiber, sizeof (*cfg));
        } else {
            rv = SOC_E_UNAVAIL;
        }
        break;
    default:
        rv = SOC_E_PARAM;
        break;
    }

    return rv;
}

/*
 * Function:
 *      phy_84728_ability_advert_set
 * Purpose:
 *      Set the current advertisement for auto-negotiation.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      mode - Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is set only for the ACTIVE medium.
 *      No synchronization performed at this level.
 */
STATIC int
phy_84728_ability_advert_set(int unit, soc_port_t port,soc_port_ability_t *ability)
{
    int        rv;
    phy_ctrl_t *pc;
    bsdk_phy84728_port_ability_t local_ability;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
        return SOC_E_NONE;
    }

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    if (PHY_COPPER_MODE(unit, port)) {
        pc->copper.advert_ability = *ability;
    } else {
        pc->fiber.advert_ability = *ability;
    }

    local_ability = 0;

    if(ability->speed_full_duplex & SOC_PA_SPEED_1000MB) {
        local_ability |= BSDK_PHY84728_PA_1000MB_FD;
    }
    if(ability->speed_full_duplex & SOC_PA_SPEED_10GB) {
        local_ability |= BSDK_PHY84728_PA_10000MB_FD;
    }

    if(ability->pause & SOC_PA_PAUSE_TX) {
        local_ability |= BSDK_PHY84728_PA_PAUSE_TX;
    }
    if(ability->pause & SOC_PA_PAUSE_RX) {
        local_ability |= BSDK_PHY84728_PA_PAUSE_RX;
    }
    if(ability->pause & SOC_PA_PAUSE_ASYMM) {
        local_ability |= BSDK_PHY84728_PA_PAUSE_ASYMM;
    }

    if(ability->loopback & SOC_PA_LB_NONE) {
        local_ability |= BSDK_PHY84728_PA_LB_NONE;
    }
    if(ability->loopback & SOC_PA_LB_PHY) {
        local_ability |= BSDK_PHY84728_PA_LB_PHY;
    }
    if(ability->flags & SOC_PA_AUTONEG) {
        local_ability |= BSDK_PHY84728_PA_AN;
    }

    rv = bsdk_phy84728_ability_advert_set(pc, local_ability);

    return rv;
}

/*
 * Function:
 *      phy_84728_ability_advert_get
 * Purpose:
 *      Get the current advertisement for auto-negotiation.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level.
 */
STATIC int
phy_84728_ability_advert_get(int unit, soc_port_t port,soc_port_ability_t *ability)
{
    phy_ctrl_t      *pc;
    int        rv;
    bsdk_phy84728_port_ability_t local_ability;

    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
        return SOC_E_NONE;
    }

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    local_ability = 0;

    rv = bsdk_phy84728_ability_advert_get(pc, &local_ability);
    if (rv != SOC_E_NONE) {
        return SOC_E_FAIL;
    }

    if(local_ability & BSDK_PHY84728_PA_1000MB_FD) {
        ability->speed_full_duplex |= SOC_PA_SPEED_1000MB;
    }
    if(local_ability & BSDK_PHY84728_PA_10000MB_FD) {
        ability->speed_full_duplex |= SOC_PA_SPEED_10GB;
    }

    if(local_ability & BSDK_PHY84728_PA_PAUSE_TX) {
        ability->pause |= SOC_PA_PAUSE_TX;
    }
    if(local_ability & BSDK_PHY84728_PA_PAUSE_RX) {
        ability->pause |= SOC_PA_PAUSE_RX;
    }
    if(local_ability & BSDK_PHY84728_PA_PAUSE_ASYMM) {
        ability->pause |= SOC_PA_PAUSE_ASYMM;
    }

    if(local_ability & BSDK_PHY84728_PA_LB_NONE) {
        ability->loopback |= SOC_PA_LB_NONE;
    }
    if(local_ability & BSDK_PHY84728_PA_LB_PHY) {
        ability->loopback |= SOC_PA_LB_PHY;
    }
    if(local_ability & BSDK_PHY84728_PA_AN) {
        ability->flags = SOC_PA_AUTONEG;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84728_ability_remote_get
 * Purpose:
 *      Get the current remote advertisement for auto-negotiation.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level.
 */
STATIC int
phy_84728_ability_remote_get(int unit, soc_port_t port,soc_port_ability_t *ability)
{
    phy_ctrl_t      *pc;
    int        rv;
    bsdk_phy84728_port_ability_t remote_ability;

    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
        return SOC_E_NONE;
    }

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    remote_ability = 0;

    rv = bsdk_phy84728_remote_ability_advert_get(pc,
                                                   &remote_ability);
    if (rv != SOC_E_NONE) {
        return SOC_E_FAIL;
    }

    if(remote_ability & BSDK_PHY84728_PA_1000MB_FD) {
        ability->speed_full_duplex |= SOC_PA_SPEED_1000MB;
    }
    if(remote_ability & BSDK_PHY84728_PA_10000MB_FD) {
        ability->speed_full_duplex |= SOC_PA_SPEED_10GB;
    }

    if(remote_ability & BSDK_PHY84728_PA_PAUSE_TX) {
        ability->pause |= SOC_PA_PAUSE_TX;
    }
    if(remote_ability & BSDK_PHY84728_PA_PAUSE_RX) {
        ability->pause |= SOC_PA_PAUSE_RX;
    }
    if(remote_ability & BSDK_PHY84728_PA_PAUSE_ASYMM) {
        ability->pause |= SOC_PA_PAUSE_ASYMM;
    }

    if(remote_ability & BSDK_PHY84728_PA_LB_NONE) {
        ability->loopback |= SOC_PA_LB_NONE;
    }
    if(remote_ability & BSDK_PHY84728_PA_LB_PHY) {
        ability->loopback |= SOC_PA_LB_PHY;
    }
    if(remote_ability & BSDK_PHY84728_PA_AN) {
        ability->flags = SOC_PA_AUTONEG;
    }

    return SOC_E_NONE;
}



/*
 * Function:
 *      phy_84728_ability_local_get
 * Purpose:
 *      Get local abilities 
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_NONE
 * Notes:
 *      The ability is retrieved only for the ACTIVE medium.
 */

STATIC int
phy_84728_ability_local_get(int unit, soc_port_t port, soc_port_ability_t *ability)
{
    int         rv = SOC_E_NONE;
    phy_ctrl_t *pc;
    bsdk_phy84728_port_ability_t local_ability = 0;

    if (NULL == ability) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    rv = bsdk_phy84728_ability_local_get(pc, &local_ability);
    if (rv != SOC_E_NONE) {
        return SOC_E_FAIL;
    }

    if(local_ability & BSDK_PHY84728_PA_1000MB_FD) {
        ability->speed_full_duplex |= SOC_PA_SPEED_1000MB;
    }
    if(local_ability & BSDK_PHY84728_PA_10000MB_FD) {
        ability->speed_full_duplex |= SOC_PA_SPEED_10GB;
    }

    if(local_ability & BSDK_PHY84728_PA_PAUSE_TX) {
        ability->pause |= SOC_PA_PAUSE_TX;
    }
    if(local_ability & BSDK_PHY84728_PA_PAUSE_RX) {
        ability->pause |= SOC_PA_PAUSE_RX;
    }
    if(local_ability & BSDK_PHY84728_PA_PAUSE_ASYMM) {
        ability->pause |= SOC_PA_PAUSE_ASYMM;
    }
    if(local_ability & BSDK_PHY84728_PA_LB_NONE) {
        ability->loopback |= SOC_PA_LB_NONE;
    }
    if(local_ability & BSDK_PHY84728_PA_LB_PHY) {
        ability->loopback |= SOC_PA_LB_PHY;
    }
    if(local_ability & BSDK_PHY84728_PA_AN) {
        ability->flags = SOC_PA_AUTONEG;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84728_firmware_set
 * Purpose:
 *      program the given firmware into the SPI-ROM
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      offset - offset to the data stream
 *      array  - the given data
 *      datalen- the data length
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
phy_84728_firmware_set(int unit, int port, int offset, uint8 *data,int len)
{
    int            rv;
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    rv = bsdk_phy84728_spi_firmware_update(pc, data, len);
    if (rv != SOC_E_NONE) {
            SOC_DEBUG_PRINT((DK_WARN,
                            "PHY84728 firmware upgrade possibly failed:"
                            "u=%d p=%d\n", unit, port));
        return (SOC_E_FAIL);
    }
    SOC_DEBUG_PRINT((DK_PHY,"PHY84728 firmware upgrade successful:"
                             "u=%d p=%d\n", unit, port));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84728_medium_status
 * Purpose:
 *      Indicate the current active medium
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      medium - (OUT) One of:
 *              SOC_PORT_MEDIUM_COPPER
 *              SOC_PORT_MEDIUM_XAUI
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
phy_84728_medium_status(int unit, soc_port_t port, soc_port_medium_t *medium)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *medium = SOC_PORT_MEDIUM_FIBER;

    return SOC_E_NONE;
}


/*
 * Function:
 *      phy_84728_control_set
 * Purpose:
 *      Configure PHY device specific control fucntion.
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #.
 *      type  - Control to update
 *      value - New setting for the control
 * Returns:    
 *      SOC_E_NONE
 */
STATIC int
phy_84728_control_set(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 value)
{
    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *      phy_84728_control_get
 * Purpose:
 *      Get current control settign of the PHY.
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #.
 *      type  - Control to update
 *      value - (OUT)Current setting for the control
 * Returns:
 *      SOC_E_NONE
 */
STATIC int 
phy_84728_control_get(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 *value)
{
    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *      phy_84728_linkup
 * Purpose:
 *      Link up handler
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #.
 * Returns:
 *      SOC_E_NONE
 */
STATIC int 
phy_84728_linkup(int unit, soc_port_t port)
{

    /* After link up configure the system side to operate at 10G */
    phy_ctrl_t *pc;
    phy_ctrl_t  *int_pc;
    bsdk_phy84728_intf_t line_mode;

    pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);

    {
        SOC_IF_ERROR_RETURN(
            bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

        if (line_mode == BSDK_PHY84728_INTF_SGMII) {
            SOC_IF_ERROR_RETURN(
                _bsdk_phy84728_system_sgmii_sync(pc, 0));
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84728_interface_get
 * Purpose:
 *      Get interface on a given port.
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #.
 *      pif   - Interface.
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
phy_84728_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    *pif = PHY_COPPER_MODE(unit, port) ? SOC_PORT_IF_SGMII : SOC_PORT_IF_XFI;
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84728_probe
 * Purpose:
 *      Complement the generic phy probe routine to identify this phy when its
 *      phy id0 and id1 is same as some other phy's.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      pc   - phy ctrl descriptor.
 * Returns:
 *      SOC_E_NONE,SOC_E_NOT_FOUND and SOC_E_<error>
 */
STATIC int
phy_84728_probe(int unit, phy_ctrl_t *pc)
{
    uint16  data1, data2;

    if (READ_PHY84728_PMA_PMD_REG(unit, pc, 0xc802, &data1) < 0) {
        return SOC_E_NOT_FOUND;
    }

    if (READ_PHY84728_PMA_PMD_REG(unit, pc, 0xc803, &data2) < 0) {
        return SOC_E_NOT_FOUND;
    }

    if ((data1 == 0x4728) && ((data2 & 0xf) == 0x8)) {
        pc->size = sizeof(phy84728_map_t);
        return SOC_E_NONE;
    }

    return SOC_E_NOT_FOUND;
}

/*
 * Function:     
 *    _phy84728_get_model_rev
 * Purpose:    
 *    Get OUI, Model and Revision of the PHY
 * Parameters:
 *    phy_dev_addr - PHY Device Address
 *    oui          - (OUT) Organization Unique Identifier
 *    model        - (OUT)Device Model number`
 *    rev          - (OUT)Device Revision number
 * Notes: 
 */
int 
_phy84728_get_model_rev(phy_ctrl_t *pc,
                       int *oui, int *model, int *rev)
{
    uint16  id0, id1;

    SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMD_ID0r(pc, &id0));

    SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMD_ID1r(pc, &id1));

    *oui   = BSDK_PHY84728_PHY_OUI(id0, id1);
    *model = BSDK_PHY84728_PHY_MODEL(id0, id1);
    *rev   = BSDK_PHY84728_PHY_REV(id0, id1);

    return SOC_E_NONE;
}



/*
 * Function:
 *     bsdk_phy84728_firmware_spi_download
 * Purpose:
 *     Download firmware via SPI
 *         - Check if firmware download is complete
 *         - Check if the checksum is good
 * Parameters:
 *    phy_id    - PHY Device Address
 * Returns:
 */
/* STATIC int */
int
bsdk_phy84728_firmware_spi_download(phy_ctrl_t *pc)
{
    uint16 data = 0;
    int i;

    /* 0xc848[15]=1, SPI-ROM downloading to RAM, 0xc848[14]=1, serial boot */
    /* 0xc848[13]=0, SPI-ROM downloading not done, 0xc848[2]=0, spi port enable */
    SOC_IF_ERROR_RETURN(
           BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc848, 
               (1 << 15)|(1 << 14), 
               ((1 << 15)|(1 << 14)|(1 << 13)|(1 << 2))));

    /* apply software reset to download code from SPI-ROM */
    /* Reset the PHY **/
    SOC_IF_ERROR_RETURN(
           BSDK_MOD_PHY84728_LN_DEV1_PMD_CTRLr(pc, 
               BSDK_PHY84728_PMD_CTRL_RESET, BSDK_PHY84728_PMD_CTRL_RESET));

    for (i = 0; i < 5; i++) {
        sal_usleep(100000);
        SOC_IF_ERROR_RETURN(
               BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc848, &data));
        if (data & 0x2000) {  /* Check for download complete */
            /* Need to check if checksum is correct */
            SOC_IF_ERROR_RETURN(
                   BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xca1c, &data));
            if (data == 0x600D) {
                break;
            }
        }
    }
    if (i >= 5) { /* Bad CHECKSUM */
        soc_cm_print("SPI-Download Firmware download failure:"
                       "Incorrect Checksum %x\n", data);
        return SOC_E_FAIL;
    }

    SOC_IF_ERROR_RETURN(
           BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xca1a, &data));

    soc_cm_print("BCM84728: SPI-ROM download complete: Version: 0x%x\n", data);

    return SOC_E_NONE;
}

/*
 * Function:     
 *    bsdk_phy84728_line_intf_get
 * Purpose:    
 *    Determine the Line side Interface
 * Parameters:
 *    phy_id    - PHY's device address
 *    dev_port  - Channel number 
 *    mode      - (OUT) Line Port mode status
 * Returns:    
 */
int 
bsdk_phy84728_line_intf_get(phy_ctrl_t *pc, int dev_port, 
                              bsdk_phy84728_intf_t *mode)
{

    phy84728_map_t      *phy_info;

    if (mode == NULL) {
        return SOC_E_PARAM;
    }

    phy_info = PHY84728_MAP(pc);

    *mode = phy_info->line_mode;
    return SOC_E_NONE;
}



/*
 * Function:     
 *    bsdk_phy84728_line_mode_set
 * Purpose:    
 *    Set the Line side mode
 * Parameters:
 *    phy_id    - PHY's device address
 *    dev_port  - Channel number 
 *    mode      - Desired mode (SGMII/1000x/SFI)
 * Returns:    
 */
int
bsdk_phy84728_line_mode_set(phy_ctrl_t *pc, int dev_port, 
                              bsdk_phy84728_intf_t mode)
{

    int rv = SOC_E_NONE;
    phy84728_map_t      *phy_info;

    phy_info = PHY84728_MAP(pc);

    /* Disable Autodetect */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_LN_DEV7_AN_BASE1000X_CTRL1r(pc, 0,
                       BSDK_PHY84728_AN_BASE1000X_CTRL1r_AUTODETECT_EN));
    switch (mode) {

    case BSDK_PHY84728_INTF_SGMII :
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_BASE1000X_CTRL1r(pc, 0,
                     BSDK_PHY84728_AN_BASE1000X_CTRL1r_FIBER_MODE));
        break;
        
    case BSDK_PHY84728_INTF_1000X :
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_BASE1000X_CTRL1r(pc, 1,
                     BSDK_PHY84728_AN_BASE1000X_CTRL1r_FIBER_MODE));
        break;

    case BSDK_PHY84728_INTF_SFI :
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_BASE1000X_CTRL1r(pc, 1,
                     BSDK_PHY84728_AN_BASE1000X_CTRL1r_FIBER_MODE));
        /* Set PMA type */
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV1_PMD_CTRL2r(pc, 0x8, 0x003f));
        break;

    case BSDK_PHY84728_INTF_XFI :
    default :
        rv = SOC_E_PARAM;
        break;
        
    }

    if (SOC_SUCCESS(rv)) {
        phy_info->line_mode = mode;
    }
    return rv;
}


/*
 * Function:     
 *    bsdk_phy84728_sys_intf_get
 * Purpose:    
 *    Determine the System side Interface
 * Parameters:
 *    phy_id    - PHY's device address
 *    dev_port  - Channel number 
 *    mode      - (OUT) System side mode status
 * Returns:    
 */
int 
bsdk_phy84728_sys_intf_get(phy_ctrl_t *pc, int dev_port, 
                             bsdk_phy84728_intf_t *mode)
{

    phy84728_map_t      *phy_info;

    if (mode == NULL) {
        return SOC_E_PARAM;
    }

    phy_info = PHY84728_MAP(pc);

    /* System will always follow line side interface */
    *mode = phy_info->line_mode;

    return SOC_E_NONE;
}



/*
 * Function:     
 *    bsdk_phy84728_sys_mode_set
 * Purpose:    
 *    Set the system side mode
 * Parameters:
 *    phy_id    - PHY's device address
 *    dev_port  - Channel number 
 *    mode      - Desired mode (SGMII/1000x/XFI)
 * Returns:    
 */
int
bsdk_phy84728_sys_mode_set(phy_ctrl_t *pc, int dev_port, 
                              bsdk_phy84728_intf_t mode)
{
    int rv = SOC_E_NONE;

    /* Disable Autodetect */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_XFI_DEV7_AN_BASE1000X_CTRL1r(pc, 0,
                       BSDK_PHY84728_AN_BASE1000X_CTRL1r_AUTODETECT_EN));
    switch (mode) {
    case BSDK_PHY84728_INTF_SGMII :
        SOC_IF_ERROR_RETURN(
            _bsdk_phy84728_system_sgmii_init(pc, dev_port));
        break;
        
    case BSDK_PHY84728_INTF_1000X :
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_XFI_DEV7_AN_BASE1000X_CTRL1r(pc, 1,
                     BSDK_PHY84728_AN_BASE1000X_CTRL1r_FIBER_MODE));
        break;
    case BSDK_PHY84728_INTF_XFI :
#if 0
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_XFI_DEV7_AN_BASE1000X_CTRL1r(pc, 1,
                     BSDK_PHY84728_AN_BASE1000X_CTRL1r_FIBER_MODE));
        /* Set PMA type */
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_SYS_DEV1_PMD_CTRL2r(pc, 0x8, 0x003f));
#endif
        break;

    case BSDK_PHY84728_INTF_XAUI :
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_XGXS_DEV7_AN_BASE1000X_CTRL1r(pc, 1,
                     BSDK_PHY84728_AN_BASE1000X_CTRL1r_FIBER_MODE));
        /* Set PMA type */
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_SYS_DEV1_PMD_CTRL2r(pc, 0x8,
                     0x000f));
        break;
    case BSDK_PHY84728_INTF_SFI :
    default :
        rv = SOC_E_PARAM;
        break;
    }

    return rv;
}


/*
 * Function:     
 *    bsdk_phy84728_no_reset_setup
 * Purpose:    
 *    Initialize the PHY without reset.
 * Parameters:
 *    phy_dev_addr        - PHY Device Address
 *    line_mode           - Line side Mode (SGMII/1000X/SFI).
 *    fcmap_enable        - Fibre Channel enabled or disabled.
 * Notes: 
 */
int
bsdk_phy84728_no_reset_setup(phy_ctrl_t *pc, int line_mode,
                               int phy_ext_boot)
{
    phy84728_map_t      *phy_info;
    uint16 val;
    uint16 data;

    val = 0xDEAD;

    phy_info = PHY84728_MAP(pc);

    if ((line_mode != BSDK_PHY84728_INTF_SGMII) &&
        (line_mode != BSDK_PHY84728_INTF_1000X) && 
        (line_mode != BSDK_PHY84728_INTF_SFI)) {
        return SOC_E_CONFIG;
    }

    /* Enable Power - PMA/PMD, PCS, AN */
    SOC_IF_ERROR_RETURN(
         BSDK_MOD_PHY84728_LN_DEV1_PMD_CTRLr(pc, 0,
                                        BSDK_PHY84728_PMD_CTRL_PD));
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_LN_DEV3_PCS_CTRLr(pc, 0,
                                       BSDK_PHY84728_PCS_PCS_CTRL_PD));
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, 0,
                                       BSDK_PHY84728_AN_MII_CTRL_PD));
#if 0
    if (phy_ext_boot) {
        /* Download firmware if via SPI */
        SOC_IF_ERROR_RETURN(bsdk_phy84728_firmware_spi_download(pc));
    }  else {
        /* MDIO based firmware download to RAM */
        SOC_IF_ERROR_RETURN(
            bsdk_phy84728_mdio_firmware_download(pc,
                                                    bcm84728_fw, bcm84728_fw_length));
    } 
#endif
    if ((!soc_property_port_get(pc->unit, pc->port, spn_FCMAP_ENABLE, 0)) &&
        (!soc_property_port_get(pc->unit, pc->port, spn_MACSEC_ENABLE, 0))) {
        /* Set Line mode */
        SOC_IF_ERROR_RETURN(
            bsdk_phy84728_line_mode_set(pc, 0, line_mode));

        /* Disable MACSEC */
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV1_MACSEC_BYPASS_CTLRr(pc, &data));

        data &= ~(BSDK_PHY84728_LN_DEV1_MACSEC_CTRL_BYPASS_MODE |
                  BSDK_PHY84728_LN_DEV1_MACSEC_CTRL_ENABLE_PWRDN_MACSEC |
                  BSDK_PHY84728_LN_DEV1_MACSEC_CTRL_UDSW_PWRDW_MACSEC |
                  BSDK_PHY84728_LN_DEV1_MACSEC_CTRL_UDSW_RESET_MACSEC);

        data |= BSDK_PHY84728_LN_DEV1_MACSEC_CTRL_BYPASS_MODE;
        /* Power down MACSEC to reduce power consumption */
        data |= BSDK_PHY84728_LN_DEV1_MACSEC_CTRL_ENABLE_PWRDN_MACSEC;
        data |= BSDK_PHY84728_LN_DEV1_MACSEC_CTRL_UDSW_PWRDW_MACSEC;

        SOC_IF_ERROR_RETURN(
            BSDK_WR_PHY84728_LN_DEV1_MACSEC_BYPASS_CTLRr(pc, data));

        /* System side mode */
        if (line_mode == BSDK_PHY84728_INTF_SFI) {
            phy_info->line_mode = BSDK_PHY84728_INTF_SFI;
            SOC_IF_ERROR_RETURN(
                bsdk_phy84728_sys_mode_set(pc, 0, 
                                     BSDK_PHY84728_INTF_XFI));
            /* Force Speed to 10G */
            SOC_IF_ERROR_RETURN(
                _phy84728_system_xfi_speed_set(pc, 0, 10000));
            SOC_IF_ERROR_RETURN(
                bsdk_phy84728_speed_set(pc, 10000));
        } else {
            if (line_mode == BSDK_PHY84728_INTF_1000X) {
                phy_info->line_mode = BSDK_PHY84728_INTF_1000X;
                SOC_IF_ERROR_RETURN(
                    bsdk_phy84728_sys_mode_set(pc, 0, 
                                         BSDK_PHY84728_INTF_1000X));
                /* Force Speed to 1G */
                SOC_IF_ERROR_RETURN(
                    bsdk_phy84728_speed_set(pc, 1000));
            } else { /* SGMII MODE */
                phy_info->line_mode = BSDK_PHY84728_INTF_SGMII;
                SOC_IF_ERROR_RETURN(
                    bsdk_phy84728_sys_mode_set(pc, 0, 
                                         BSDK_PHY84728_INTF_SGMII));
            }
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:     
 *    bsdk_phy84728_reset 
 * Purpose:    
 *    Reset the PHY.
 * Parameters:
 *    phy_id - PHY Device Address of the PHY to reset
 */
int 
bsdk_phy84728_reset(phy_ctrl_t *pc)
{
    uint16          ctrl, tmp;


    /* Reset the PHY **/
    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_LN_DEV1_PMD_CTRLr(pc, &ctrl));

    SOC_IF_ERROR_RETURN(
        BSDK_WR_PHY84728_LN_DEV1_PMD_CTRLr(pc, 
                               (ctrl | BSDK_PHY84728_PMD_CTRL_RESET)));
    /* Needs minimum of 5us to complete the reset */
    sal_usleep(30);

    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_LN_DEV1_PMD_CTRLr(pc, &tmp));

    if ((tmp & BSDK_PHY84728_PMD_CTRL_RESET) != 0) {
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMD_CTRLr(pc, ctrl));
        return SOC_E_FAIL;
    }

    return SOC_E_NONE;
}


/*
 * Function:     
 *    bsdk_phy84728_speed_set
 * Purpose:    
 *    To set the PHY's speed  
 * Parameters:
 *    phy_id    - PHY's device address
 *    speed     - Speed to set
 *               10   = 10Mbps
 *               100  = 100Mbps
 *               1000 = 1000Mbps
 * Returns:    
 */
int
bsdk_phy84728_speed_set(phy_ctrl_t *pc, int speed)
{
    int value = 0;
    bsdk_phy84728_intf_t line_mode;

    if ((speed != 10) && (speed != 100) && 
         (speed != 1000) && (speed != 10000)) {
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    switch (line_mode) {
    case BSDK_PHY84728_INTF_SFI:
    case BSDK_PHY84728_INTF_1000X:
        /* Disable Clause 37 Autoneg */
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, 0x0000,
                                   0x3040));
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_BASE1000X_CTRL1r(pc, 1,
                                   1));
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_MSIC2r(pc, 0x2020,
                                   0x2020));
        /* Disable Clause 37 Autoneg */
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_XFI_DEV7_AN_MII_CTRLr(pc, 0x0000,
                                   0x3040));
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_XFI_DEV7_AN_BASE1000X_CTRL1r(pc, 1,
                                   1));
        if (speed == 10000) {

            /* Set Line mode */
            SOC_IF_ERROR_RETURN(
                bsdk_phy84728_line_mode_set(pc, 0,
                BSDK_PHY84728_INTF_SFI));

            /* Set System side to match Line side */
            SOC_IF_ERROR_RETURN(
                bsdk_phy84728_sys_mode_set(pc, 0, 
                                         BSDK_PHY84728_INTF_XFI));
            /* Force Speed in PMD */
            SOC_IF_ERROR_RETURN(
                BSDK_MOD_PHY84728_LN_DEV1_PMD_CTRLr(pc, 
                                     BSDK_PHY84728_PMD_CTRL_SS_10000,
                                     BSDK_PHY84728_PMD_CTRL_10GSS_MASK));
            /* Select 10G-LRM PMA */
            SOC_IF_ERROR_RETURN(
                BSDK_MOD_PHY84728_LN_DEV1_PMD_CTRL2r(pc,
                             BSDK_PHY84728_PMD_CTRL2_PMA_10GLRMPMD_TYPE,
                             BSDK_PHY84728_PMD_CTRL2_PMA_SELECT_MASK));
            /* Select 10G-LRM PMA */
            SOC_IF_ERROR_RETURN(
                BSDK_MOD_PHY84728_SYS_DEV1_PMD_CTRL2r(pc,
                             BSDK_PHY84728_PMD_CTRL2_PMA_10GLRMPMD_TYPE,
                             BSDK_PHY84728_PMD_CTRL2_PMA_SELECT_MASK));

            /* Force Speed in PMD */
            SOC_IF_ERROR_RETURN(
                BSDK_MOD_PHY84728_SYS_DEV1_PMD_CTRLr(pc, 
                                         BSDK_PHY84728_PMD_CTRL_SS_10000,
                                         BSDK_PHY84728_PMD_CTRL_10GSS_MASK));
        } else {
            if (speed == 1000) {
                /* Set Line mode */
                SOC_IF_ERROR_RETURN(
                    bsdk_phy84728_line_mode_set(pc, 0,
                                                   BSDK_PHY84728_INTF_1000X));
                /* Set System side to match Line side */
                SOC_IF_ERROR_RETURN(
                    bsdk_phy84728_sys_mode_set(pc, 0, 
                                             BSDK_PHY84728_INTF_1000X));
                /* Force Speed in PMD */
                SOC_IF_ERROR_RETURN(
                    BSDK_MOD_PHY84728_LN_DEV1_PMD_CTRLr(pc, 
                                         BSDK_PHY84728_PMD_CTRL_SS_1000,
                                         BSDK_PHY84728_PMD_CTRL_10GSS_MASK));

                /* Force Speed in PMD */
                SOC_IF_ERROR_RETURN(
                    BSDK_MOD_PHY84728_SYS_DEV1_PMD_CTRLr(pc, 
                                             BSDK_PHY84728_PMD_CTRL_SS_1000,
                                             BSDK_PHY84728_PMD_CTRL_10GSS_MASK));
            } else {
                return SOC_E_CONFIG;
            }
        }
    break;

    case BSDK_PHY84728_INTF_SGMII:
        if (speed > 1000) {
            return SOC_E_CONFIG;
        }
        /* Disable Clause 37 Autoneg */
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_MII_CTRLr(pc, 0x0000,
                                   0x3040));
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_BASE1000X_CTRL1r(pc, 0,
                                   1));
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV7_AN_MSIC2r(pc, 0x2020,
                                   0x2020));
        if (speed == 1000) {
            value = BSDK_PHY84728_PMD_CTRL_SS_1000;
        }
        if (speed == 100) {
            value = BSDK_PHY84728_PMD_CTRL_SS_100;
        }
        if (speed == 100) {
            value = BSDK_PHY84728_PMD_CTRL_SS_10;
        }
        /* Force Speed in PMD */
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_LN_DEV1_PMD_CTRLr(pc, 
                                 value,
                                 BSDK_PHY84728_PMD_CTRL_10GSS_MASK));

        /* Set System side to match Line side */
        SOC_IF_ERROR_RETURN(
            bsdk_phy84728_sys_mode_set(pc, 0, 
                                     BSDK_PHY84728_INTF_SGMII));
        /* Disable Clause 37 Autoneg */
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_XFI_DEV7_AN_MII_CTRLr(pc, 0x0000,
                                   0x3040));
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_XFI_DEV7_AN_BASE1000X_CTRL1r(pc, 0,
                                   1));
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_XFI_DEV7_AN_MISC2r(pc, 0x2020,
                                   0x2020));
        /* Force Speed in PMD */
        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_SYS_DEV1_PMD_CTRLr(pc, 
                                     value,
                                     BSDK_PHY84728_PMD_CTRL_10GSS_MASK));
    break;
    default:
        return SOC_E_CONFIG;
    }

    return SOC_E_NONE;
}




/*
 * Function:
 *      bsdk_phy84728_mdix_set
 * Description:
 *      Set the Auto-MDIX mode of a PHY
 * Parameters:
 *    phy_id    - PHY's device address
 *    mdix_mode - 
 *              BSDK_PHY84728_PORT_MDIX_AUTO
 *                      Enable auto-MDIX when autonegotiation is enabled
 *              BSDK_PHY84728_PORT_MDIX_FORCE_AUTO
 *                      Enable auto-MDIX always
 *              BSDK_PHY84728_PORT_MDIX_NORMAL
 *                      Disable auto-MDIX
 *              BSDK_PHY84728_PORT_MDIX_XOVER
 *                      Disable auto-MDIX, and swap cable pairs
 * Return Value:
 */
int
bsdk_phy84728_mdix_set(phy_ctrl_t *pc, bsdk_phy84728_port_mdix_t mdix_mode)
{
    bsdk_phy84728_intf_t line_mode;

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_1000X) {
        if (mdix_mode != BSDK_PHY84728_PORT_MDIX_NORMAL) {
            return SOC_E_UNAVAIL;
        }
    } else {
        if (line_mode == BSDK_PHY84728_INTF_SGMII) {
            
        } else {
            
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    bsdk_phy84728_mdix_status_get
 * Description:
 *    Get the current MDIX status on a PHY
 * Parameters:
 *    phy_id    - PHY Device Address
 *    status    - (OUT) One of:
 *              BSDK_PHY84728_PORT_MDIX_STATUS_NORMAL
 *                      Straight connection
 *              BSDK_PHY84728_PORT_MDIX_STATUS_XOVER
 *                      Crossover has been performed
 */
int
bsdk_phy84728_mdix_status_get(phy_ctrl_t *pc, bsdk_phy84728_port_mdix_status_t *status)
{
    bsdk_phy84728_intf_t line_mode;

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_1000X) {
        *status = BSDK_PHY84728_PORT_MDIX_NORMAL;
    } else {
        if (line_mode == BSDK_PHY84728_INTF_SGMII) {
            
        } else {
            
            *status = BSDK_PHY84728_PORT_MDIX_NORMAL;
        }
    }
    return SOC_E_NONE;
}    


#if 0
/*
 * Function:
 *    bsdk_phy84728_master_set
 * Purpose:
 *    Set the current master mode
 * Parameters:
 *    phy_id    - PHY Device Address
 *    master    - 
 *                Slave  = BSDK_PHY84728_PORT_MS_SLAVE
 *                Master = BSDK_PHY84728_PORT_MS_MASTER
 *                Auto   = BSDK_PHY84728_PORT_MS_AUTO
 *                None   = BSDK_PHY84728_PORT_MS_NONE
 * Notes:
 */
int
bsdk_phy84728_master_set(phy_ctrl_t *pc, bsdk_phy84728_master_t master)
{
    bsdk_phy84728_intf_t line_mode;

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_SFI) {
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      bsdk_phy84728_master_get
 * Purpose:
 *      Get the current master mode for a 84728 device.
 * Parameters:
 *    phy_id    - PHY Device Address
 *    master    - (OUT)
 *                BSDK_PHY84728_PORT_MS_SLAVE  = Slave 
 *                BSDK_PHY84728_PORT_MS_MASTER = Master
 *                BSDK_PHY84728_PORT_MS_AUTO   = Auto
 *                BSDK_PHY84728_PORT_MS_NONE   = None
 * Returns:
 */

int
bsdk_phy84728_master_get(phy_ctrl_t *pc, int *master)
{
    bsdk_phy84728_intf_t line_mode;

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(phy_id, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_SFI) {
    }
    return SOC_E_NONE;
}
#endif

/*
 * Function:
 *      bsdk_phy84728_link_get
 * Purpose:
 *      Determine the current link up/down status for a 84728 device.
 * Parameters:
 *    phy_id    - PHY Device Address
 *    link      - (OUT) 
 *                1 = Indicates Link is established
 *                0 = No Link.
 * Returns:
 */
int
bsdk_phy84728_link_get(phy_ctrl_t *pc, int *link)
{

    uint16 data = 0, pma_mii_stat = 0, pcs_mii_stat = 0;
    uint16 fval;
    int an, an_done;
    bsdk_phy84728_intf_t line_mode;
    phy84728_map_t      *phy_info;

    phy_info = PHY84728_MAP(pc);

    fval = 0;

    *link = 0;
    SOC_IF_ERROR_RETURN(
        phy_84728_an_get(pc->unit, pc->port, &an, &an_done)); 

    if (an && (an_done == 0)) {
        /* Auto neg in progess, return no link */
        *link = 0;
        return SOC_E_NONE;
    }


     SOC_IF_ERROR_RETURN(
         bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

     switch (line_mode) {
        case BSDK_PHY84728_INTF_SFI:
            SOC_IF_ERROR_RETURN(
                BSDK_RD_PHY84728_LN_DEV1_PMD_CTRL2r(pc, &data));

            SOC_IF_ERROR_RETURN(
                BSDK_RD_PHY84728_LN_DEV1_PMD_CTRL2r(pc, &data));

            SOC_IF_ERROR_RETURN(
                BSDK_RD_PHY84728_LN_DEV1_PMD_STATr(pc, &pma_mii_stat));

            SOC_IF_ERROR_RETURN(
                BSDK_RD_PHY84728_LN_DEV3_PCS_STATr(pc, &pcs_mii_stat));

            *link = pma_mii_stat & pcs_mii_stat & BSDK_PHY84728_PMD_STAT_RX_LINK_STATUS;

            break;

        case BSDK_PHY84728_INTF_1000X:
        case BSDK_PHY84728_INTF_SGMII:
            SOC_IF_ERROR_RETURN(
                BSDK_RD_PHY84728_LN_DEV1_SPEED_LINK_DETECT_STATr(pc, &data));
            if ((data & 0x8080) == 0x8080) {
                /* System side and line side rx signal detect OK */
                if (data & 0x1700) { /* system side link up */
                    if (data & 0x0017) { /* line side line up */
                        *link = 1;
                    }
                }
            }


#if 0
            SOC_IF_ERROR_RETURN(
                BSDK_RD_PHY84728_LN_DEV7_AN_MII_STATr(pc, &data));

            *link = data & BSDK_PHY84728_AN_MII_STAT_LINK_STATUS;
            break;
#endif
            break;
        default:
            break;
     }

     if (*link == 0) {
         return SOC_E_NONE;
     }

     switch (line_mode) {
         case BSDK_PHY84728_INTF_SGMII:
#if 0
                    SOC_IF_ERROR_RETURN(
                                              BSDK_RD_PHY84728_SGMII_DEV1_BASE1000X_STAT1r(pc, &data));

                    /* Match the speed on the system side interface */
                    speed = (data & 0x0018) >> 3;

                    /*
                     * Update system side SGMII interface.
                     */
                    if (data & 1) {
                        switch (speed) {
                            case 0: speed = 10; break;
                            case 1: speed = 100; break;
                            case 2: speed = 1000; break;
                            default: break;
                        }
                        duplex = ((data & 0x0004) >> 2) ? BSDK_PHY84728_FULL_DUPLEX :
                            BSDK_PHY84728_HALF_DUPLEX;
                        SOC_IF_ERROR_RETURN(
                                                  _phy84728_system_sgmii_speed_set(pc, dev_port, speed));

                        SOC_IF_ERROR_RETURN(
                                                  _phy84728_system_sgmii_duplex_set(pc, dev_port, duplex));
                    }
#endif
                    break;
                case BSDK_PHY84728_INTF_XFI:
                case BSDK_PHY84728_INTF_SFI:
                case BSDK_PHY84728_INTF_1000X:
                    break;
                default:
                    break;
            }

    return SOC_E_NONE;
}


/*
 * Function:
 *     bsdk_phy84728_ability_advert_set 
 * Purpose:
 *      Set the current advertisement for auto-negotiation.
 * Parameters:
 *    phy_id    - PHY Device Address
 *    ability   - Ability indicating supported options/speeds.
 *                For Speed & Duplex:
 *                BSDK_PM_10MB_HD    = 10Mb, Half Duplex
 *                BSDK_PM_10MB_FD    = 10Mb, Full Duplex
 *                BSDK_PM_100MB_HD   = 100Mb, Half Duplex
 *                BSDK_PM_100MB_FD   = 100Mb, Full Duplex
 *                BSDK_PM_1000MB_HD  = 1000Mb, Half Duplex
 *                BSDK_PM_1000MB_FD  = 1000Mb, Full Duplex
 *                BSDK_PM_PAUSE_TX   = TX Pause
 *                BSDK_PM_PAUSE_RX   = RX Pause
 *                BSDK_PM_PAUSE_ASYMM = Asymmetric Pause
 * Returns:
 */

int
bsdk_phy84728_ability_advert_set(phy_ctrl_t *pc,
                               bsdk_phy84728_port_ability_t ability)
{
    uint16 data = 0;
    bsdk_phy84728_intf_t line_mode;

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_1000X) {
        /* Always advertise 1000X full duplex */
        data = BSDK_PHY84728_DEV7_1000X_ANA_C37_FD;  
        if ((ability & BSDK_PHY84728_PA_PAUSE) == 
                                             BSDK_PHY84728_PA_PAUSE) {
            data |= BSDK_PHY84728_DEV7_1000X_ANA_C37_PAUSE;
        } else {
            if (ability & BSDK_PHY84728_PA_PAUSE_TX) {
                data |= BSDK_PHY84728_DEV7_1000X_ANA_C37_ASYM_PAUSE;
            } else {
                if (ability & BSDK_PHY84728_PA_PAUSE_RX) {
                    data |= BSDK_PHY84728_DEV7_1000X_ANA_C37_ASYM_PAUSE;
                    data |= BSDK_PHY84728_DEV7_1000X_ANA_C37_PAUSE;
                }
            }
        }
        SOC_IF_ERROR_RETURN(
            BSDK_WR_PHY84728_LN_DEV7_AN_ANAr(pc, data));

    }
    if (line_mode == BSDK_PHY84728_INTF_SGMII) {
        
        
    } else {
        /* No Autoneg in SFI mode */
        /* return SOC_E_UNAVAIL; */
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *      bsdk_phy84728_ability_advert_get
 * Purpose:
 *      Get the current advertisement for auto-negotiation.
 * Parameters:
 *    phy_id    - PHY Device Address
 *    ability   - (OUT) Port ability mask indicating supported options/speeds.
 *                For Speed & Duplex:
 *                BSDK_PM_10MB_HD    = 10Mb, Half Duplex
 *                BSDK_PM_10MB_FD    = 10Mb, Full Duplex
 *                BSDK_PM_100MB_HD   = 100Mb, Half Duplex
 *                BSDK_PM_100MB_FD   = 100Mb, Full Duplex
 *                BSDK_PM_1000MB_HD  = 1000Mb, Half Duplex
 *                BSDK_PM_1000MB_FD  = 1000Mb, Full Duplex
 *                BSDK_PM_PAUSE_TX   = TX Pause
 *                BSDK_PM_PAUSE_RX   = RX Pause
 *                BSDK_PM_PAUSE_ASYMM = Asymmetric Pause
 * Returns:
 */

int
bsdk_phy84728_ability_advert_get(phy_ctrl_t *pc,
                               bsdk_phy84728_port_ability_t *ability)
{
    uint16 data = 0;
    bsdk_phy84728_intf_t line_mode;

    if (ability == NULL) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_1000X) {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV7_AN_ANAr(pc, &data));

        *ability |= (data & BSDK_PHY84728_DEV7_1000X_ANA_C37_FD) ? 
                  BSDK_PHY84728_PA_1000MB_FD : 0;
        *ability |= (data & BSDK_PHY84728_DEV7_1000X_ANA_C37_HD) ?
                  BSDK_PHY84728_PA_1000MB_HD : 0;
        switch (data & (BSDK_PHY84728_DEV7_1000X_ANA_C37_PAUSE | 
                        BSDK_PHY84728_DEV7_1000X_ANA_C37_ASYM_PAUSE)) {
        case BSDK_PHY84728_DEV7_1000X_ANA_C37_PAUSE:
            *ability |= BSDK_PHY84728_PA_PAUSE;
            break;
        case BSDK_PHY84728_DEV7_1000X_ANA_C37_ASYM_PAUSE:
            *ability |= BSDK_PHY84728_PA_PAUSE_TX;
            break;
        case (BSDK_PHY84728_DEV7_1000X_ANA_C37_PAUSE | 
              BSDK_PHY84728_DEV7_1000X_ANA_C37_ASYM_PAUSE):
            *ability |= BSDK_PHY84728_PA_PAUSE_RX;
            break;
        }
    } else {
        if (line_mode == BSDK_PHY84728_INTF_SGMII) {
            *ability = BSDK_PHY84728_PA_SPEED_ALL |
                       BSDK_PHY84728_PA_AN        |
                       BSDK_PHY84728_PA_PAUSE     |
                       BSDK_PHY84728_PA_PAUSE_ASYMM;
        
        } else {
            /* No Autoneg in SFI mode */
            *ability = BSDK_PHY84728_PA_10000MB_FD |
                       BSDK_PHY84728_PA_PAUSE;
        }
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *      bsdk_phy84728_remote_ability_advert_get
 * Purpose:
 *      Get partners current advertisement for auto-negotiation.
 * Parameters:
 *    phy_id         - PHY Device Address
 *    remote_ability - (OUT) Port ability mask indicating supported options/speeds.
 *                For Speed & Duplex:
 *                BSDK_PM_10MB_HD    = 10Mb, Half Duplex
 *                BSDK_PM_10MB_FD    = 10Mb, Full Duplex
 *                BSDK_PM_100MB_HD   = 100Mb, Half Duplex
 *                BSDK_PM_100MB_FD   = 100Mb, Full Duplex
 *                BSDK_PM_1000MB_HD  = 1000Mb, Half Duplex
 *                BSDK_PM_1000MB_FD  = 1000Mb, Full Duplex
 *                BSDK_PM_PAUSE_TX   = TX Pause
 *                BSDK_PM_PAUSE_RX   = RX Pause
 *                BSDK_PM_PAUSE_ASYMM = Asymmetric Pause
 * Returns:
 */

int
bsdk_phy84728_remote_ability_advert_get(phy_ctrl_t *pc,
                                bsdk_phy84728_port_ability_t *remote_ability)
{
    uint16 data = 0;
    bsdk_phy84728_intf_t line_mode;

    if (remote_ability == NULL) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    if (line_mode == BSDK_PHY84728_INTF_1000X) {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_LN_DEV7_AN_ANPr(pc, &data));

        *remote_ability |= (data & BSDK_PHY84728_DEV7_1000X_ANA_C37_FD) ?
                  BSDK_PHY84728_PA_1000MB_FD : 0;
        *remote_ability |= (data & BSDK_PHY84728_DEV7_1000X_ANA_C37_HD) ?
                               BSDK_PHY84728_PA_1000MB_HD : 0;
        switch (data & (BSDK_PHY84728_DEV7_1000X_ANA_C37_PAUSE | 
                        BSDK_PHY84728_DEV7_1000X_ANA_C37_ASYM_PAUSE)) {
        case BSDK_PHY84728_DEV7_1000X_ANA_C37_PAUSE:
            *remote_ability |= BSDK_PHY84728_PA_PAUSE;
            break;
        case BSDK_PHY84728_DEV7_1000X_ANA_C37_ASYM_PAUSE:
            *remote_ability |= BSDK_PHY84728_PA_PAUSE_TX;
            break;
        case (BSDK_PHY84728_DEV7_1000X_ANA_C37_PAUSE | 
              BSDK_PHY84728_DEV7_1000X_ANA_C37_ASYM_PAUSE):
            *remote_ability |= BSDK_PHY84728_PA_PAUSE_RX;
            break;
        }
    }
    if (line_mode == BSDK_PHY84728_INTF_SGMII) {
        
        
    } else {
        /* No Autoneg in SFI mode */
        return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *      bsdk_phy84728_ability_local_get
 * Purpose:
 *      Get the PHY abilities
 * Parameters:
 *    phy_id    - PHY Device Address
 *    mode      - Mask indicating supported options/speeds.
 *                For Speed & Duplex:
 *                BSDK_PM_10MB_HD    = 10Mb, Half Duplex
 *                BSDK_PM_10MB_FD    = 10Mb, Full Duplex
 *                BSDK_PM_100MB_HD   = 100Mb, Half Duplex
 *                BSDK_PM_100MB_FD   = 100Mb, Full Duplex
 *                BSDK_PM_1000MB_HD  = 1000Mb, Half Duplex
 *                BSDK_PM_1000MB_FD  = 1000Mb, Full Duplex
 *                BSDK_PM_PAUSE_TX   = TX Pause
 *                BSDK_PM_PAUSE_RX   = RX Pause
 *                BSDK_PM_PAUSE_ASYMM = Asymmetric Pause
 *                BSDK_PM_SGMII       = SGMIII Supported
 *                BSDK_PM_XSGMII      = XSGMIII Supported
 * Returns:
 */
int
bsdk_phy84728_ability_local_get(phy_ctrl_t *pc,
                             bsdk_phy84728_port_ability_t *ability)
{
    bsdk_phy84728_intf_t line_mode;

    if (ability == NULL) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(
        bsdk_phy84728_line_intf_get(pc, 0, &line_mode));

    *ability = BSDK_PHY84728_PA_LB_PHY | BSDK_PHY84728_PA_PAUSE;
    *ability |= BSDK_PHY84728_PA_SGMII;

    if (line_mode == BSDK_PHY84728_INTF_1000X) {
        *ability |= BSDK_PHY84728_PA_1000MB;
    }
    if (line_mode == BSDK_PHY84728_INTF_SGMII) {
        *ability |= BSDK_PHY84728_PA_AN;
        *ability |= BSDK_PHY84728_PA_SPEED_ALL;
        
    } else {
        *ability |= BSDK_PHY84728_PA_XSGMII;
        *ability |= BSDK_PHY84728_PA_10000MB_FD;
    }

    return(SOC_E_NONE);
}

STATIC int
_bsdk_phy84728_system_sgmii_speed_set(phy_ctrl_t *pc, int dev_port, int speed)
{
    uint16 mii_ctrl = 0;

    if (speed == 0) {
        return SOC_E_NONE;
    }

    {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_XFI_DEV7_AN_MII_CTRLr(pc, &mii_ctrl));

        mii_ctrl &= ~(BSDK_PHY84728_SGMII_MII_CTRL_SS_MASK);
        switch(speed) {
        case 10:
            mii_ctrl |= BSDK_PHY84728_AN_MII_CTRL_SS_10;
            break;
        case 100:
            mii_ctrl |= BSDK_PHY84728_AN_MII_CTRL_SS_100;
            break;
        case 1000:
            mii_ctrl |= BSDK_PHY84728_AN_MII_CTRL_SS_1000;
            break;
        default:
            return SOC_E_CONFIG;
        }
        SOC_IF_ERROR_RETURN(
            BSDK_WR_PHY84728_XFI_DEV7_AN_MII_CTRLr(pc, mii_ctrl));
    }
    return SOC_E_NONE;
}

STATIC int
_bsdk_phy84728_system_sgmii_duplex_set(phy_ctrl_t *pc, int dev_port,
                                 int duplex)
{
    uint16 mii_ctrl;

    {
        SOC_IF_ERROR_RETURN(
            BSDK_RD_PHY84728_XFI_DEV7_AN_MII_CTRLr(pc, &mii_ctrl));

        if ( duplex ) {
            mii_ctrl |= BSDK_PHY84728_AN_MII_CTRL_FD;
        } else {
            mii_ctrl &= ~BSDK_PHY84728_AN_MII_CTRL_FD;
        }
        SOC_IF_ERROR_RETURN(
            BSDK_WR_PHY84728_XFI_DEV7_AN_MII_CTRLr(pc, mii_ctrl));
    }

    return SOC_E_NONE;
}

STATIC int
_bsdk_phy84728_system_sgmii_init(phy_ctrl_t *pc, int dev_port)
{
        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_REG_MOD(pc, 0x01, 0x0007, 0x8000, 0x0000, 0x2000));

        SOC_IF_ERROR_RETURN(
            BSDK_WR_PHY84728_XFI_DEV7_AN_BASE1000X_CTRL1r(pc, 0x01a0));

        SOC_IF_ERROR_RETURN(
            BSDK_MOD_PHY84728_XFI_DEV7_AN_MISC2r(pc, 0x6000, 0x6020));

        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_REG_WR(pc, 0x01, 0x0007, 0x835c, 0x0001));

        SOC_IF_ERROR_RETURN(
            BSDK_WR_PHY84728_XFI_DEV7_AN_MII_CTRLr(pc, 0x0140));

        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_REG_MOD(pc, 0x01, 0x0007, 0x8000, 0x2000, 0x2000));

    return SOC_E_NONE;
}

STATIC int
_bsdk_phy84728_system_sgmii_sync(phy_ctrl_t *pc, int dev_port)
{
    int speed;
    int duplex;

        SOC_IF_ERROR_RETURN(
            phy_84728_speed_get(pc->unit, pc->port, &speed));

        SOC_IF_ERROR_RETURN(
            phy_84728_duplex_get(pc->unit, pc->port, &duplex));

        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_REG_MOD(pc, 0x01, 0x0007, 0x8000, 0x0000, 0x2000));

        SOC_IF_ERROR_RETURN(
            _bsdk_phy84728_system_sgmii_speed_set(pc, dev_port, speed));

        SOC_IF_ERROR_RETURN(
            _bsdk_phy84728_system_sgmii_duplex_set(pc, dev_port, duplex));

        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_REG_MOD(pc, 0x01, 0x0007, 0x8000, 0x2000, 0x2000));

    return SOC_E_NONE;
}

/*
 * Function:
 *     bsdk_phy84728_mdio_firmware_download
 * Purpose:
 *     Download new firmware via MDIO.
 * Parameters:
 *    phy_id    - PHY Device Address
 *    new_fw    - Pointer to new firmware
 *    fw_length    - Length of the firmware
 * Returns:
 */
int
bsdk_phy84728_mdio_firmware_download(phy_ctrl_t *pc,
                                  uint8 *new_fw, uint32 fw_length)
{

    uint16 data16;
    int j;
    uint16 num_words;

    /* 0xc848[15]=0, MDIO downloading to RAM, 0xc848[14]=1, serial boot */
    /* 0xc848[13]=0, SPI-ROM downloading not done, 0xc848[2]=0, spi port enable */
    SOC_IF_ERROR_RETURN(
           BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc848, 
               (0 << 15)|(1 << 14),
               (1 << 15)|(1 << 14)|((1 << 13) | (1 << 2))));

    /* Reset, to download code from MDIO */
    SOC_IF_ERROR_RETURN(
           BSDK_MOD_PHY84728_LN_DEV1_PMD_CTRLr(pc, 
               BSDK_PHY84728_PMD_CTRL_RESET, BSDK_PHY84728_PMD_CTRL_RESET));

    sal_usleep(50000);

    /* Write Starting Address, where the Code will reside in SRAM */
    data16 = 0x8000;
    SOC_IF_ERROR_RETURN
        (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, BSDK_PHY84728_PMAD_M8051_MSGIN_REG, data16));

    /* make sure address word is read by the micro */
    sal_usleep(10); /* Wait for 10us */

    /* Write SPI SRAM Count Size */
    data16 = (fw_length)/2;
    SOC_IF_ERROR_RETURN
        (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, BSDK_PHY84728_PMAD_M8051_MSGIN_REG, data16));

    /* make sure read by the micro */
    sal_usleep(10); /* Wait for 10us */

    /* Fill in the SRAM */
    num_words = (fw_length - 1);
    for (j = 0; j < num_words; j+=2) {
        /* Make sure the word is read by the Micro */
        sal_usleep(10);

        data16 = (new_fw[j] << 8) | new_fw[j+1];
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, BSDK_PHY84728_PMAD_M8051_MSGIN_REG,
                 data16));
    }

    /* make sure last code word is read by the micro */
    sal_usleep(20);

    /* Read Hand-Shake message (Done) from Micro */
    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc,BSDK_PHY84728_PMAD_M8051_MSGOUT_REG, &data16)); 

    if (data16 != 0x4321 ) {
        /* Download done message */
        soc_cm_print("MDIO firmware download failed. Message: 0x%x\n", data16);
    }

    /* Clear LASI status */
    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0x9003, &data16));

    /* Wait for LASI to be asserted when M8051 writes checksum to MSG_OUTr */
    sal_usleep(100); /* Wait for 100 usecs */

    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc,BSDK_PHY84728_PMAD_M8051_MSGOUT_REG, &data16));
    
    /* Need to check if checksum is correct */
    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xCA1C, &data16));

    if (data16 != 0x600D) {
        /* Bad CHECKSUM */
        soc_cm_print("MDIO Firmware downlad failure:"
                     "Incorrect Checksum %x\n", data16);
        return SOC_E_FAIL;
    }

    /* read Rev-ID */
    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xCA1A, &data16));
    soc_cm_print("MDIO Firmware download completed. Version : 0x%x\n", data16);

    return SOC_E_NONE;
}

/*
 *  Function:
 *       _phy_84728_phy_84728_write_message
 *
 *  Purpose:
 *      Write into Message In and Read from MSG Out register.
 *  Input:
 *      phy_id
 *      wrdata
 *      rddata
 */

STATIC int
_phy_84728_write_message(phy_ctrl_t *pc, uint16 wrdata, uint16 *rddata)
{
    uint16 tmp_data = 0;
    int iter = 0;

    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_M8501_MSGOUTr(pc, &tmp_data));
    SOC_IF_ERROR_RETURN         
        (BSDK_WR_PHY84728_LN_DEV1_M8501_MSGINr(pc, wrdata));

    do {
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_LASI_STATr(pc, &tmp_data));

        if (tmp_data & 0x4) {
            break;
        }
        sal_usleep(BSDK_WR_TIMEOUT);
        iter++;
    } while (iter < BSDK_WR_ITERATIONS);

    if (!(tmp_data & 0x4)) {
        soc_cm_print("write message failed due to timeout\n");
        return SOC_E_FAIL;
    }

    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_M8501_MSGOUTr(pc, &tmp_data));

    *rddata = tmp_data;
    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_RX_ALARMr(pc, &tmp_data));

    return SOC_E_NONE;
}


/*
 *  Function:
 *       _phy_84728_rom_wait
 *
 *  Purpose:
 *      Wait for data to be written to the SPI-ROM.
 *  Input:
 *      phy_id
 */

STATIC int
_phy_84728_rom_wait(phy_ctrl_t *pc)
{
    uint16  rd_data = 0, wr_data;
    int        count;
    int        SPI_READY;
    int        iter = 0;

    do {
        count = 1;
        wr_data = ((BSDK_RD_CPU_CTRL_REGS * 0x0100) | count);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        wr_data = BSDK_SPI_CTRL_1_L;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        if (rd_data & 0x0100) {
            break;
        }
        sal_usleep(BSDK_WR_TIMEOUT);
        iter++;
    } while (iter < BSDK_WR_ITERATIONS);

    if (!(rd_data & 0x0100)) {
        soc_cm_print("_phy_84728_rom_wait: write timeout\n");
        return SOC_E_TIMEOUT;
    }

     SPI_READY = 1;
     while (SPI_READY == 1) {
        /* Set-up SPI Controller To Receive SPI EEPROM Status. */
        count = 1;
        wr_data = ((BSDK_WR_CPU_CTRL_REGS * 0x0100) | count);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        wr_data = BSDK_SPI_CTRL_2_H;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        wr_data = 0x0100;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Fill-up SPI Transmit Fifo To check SPI Status. */
        count = 2;
        wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write Tx Fifo Register Address. */
        wr_data = BSDK_SPI_TXFIFO;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-1. */
        wr_data = ((1 * 0x0100) | BSDK_MSGTYPE_HRD);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write SPI Tx Fifo Control Word-2. */
        wr_data = BSDK_RDSR_OPCODE;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write SPI Control Register Write Command. */
        count = 2;
        wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Address. */
        wr_data = BSDK_SPI_CTRL_1_L;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Word-1. */
        wr_data = 0x0101;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Word-2. */
        wr_data = 0x0100;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write SPI Control Register Write Command. */
        count = 1;
        wr_data = ((BSDK_WR_CPU_CTRL_REGS * 0x0100) | count);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Address. */
        wr_data = BSDK_SPI_CTRL_1_H;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Word-2. */
        wr_data = 0x0103;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Wait For 64 bytes To be written.   */
        rd_data = 0x0000;
        
        do {
            count = 1;
            wr_data = ((BSDK_RD_CPU_CTRL_REGS * 0x0100) | count);
            SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
            wr_data = BSDK_SPI_CTRL_1_L;
            SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
            if (rd_data & 0x0100) {
                break;
            }
            sal_usleep(BSDK_WR_TIMEOUT);
            iter ++;
        } while (iter < BSDK_WR_ITERATIONS); 

        if (!(rd_data & 0x0100)) {
            soc_cm_print("_phy_84728_rom_program:timeout 2\n");
            return SOC_E_TIMEOUT;
        }
        /* Write SPI Control Register Read Command. */
        count = 1;
        wr_data = ((BSDK_RD_CPU_CTRL_REGS * 0x0100) | count);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Address. */
        wr_data = BSDK_SPI_RXFIFO;

        SOC_IF_ERROR_RETURN         
            (BSDK_WR_PHY84728_LN_DEV1_M8501_MSGINr(pc, wr_data));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_M8501_MSGOUTr(pc, &rd_data));

        /* Clear LASI Message Out Status. */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_LASI_STATr(pc, &wr_data));

        if ((rd_data & 0x1) == 0) {
            SPI_READY = 0;
        }
     } /* SPI_READY */
     return SOC_E_NONE;
}


/*
 * Function:
 *      _phy_84728_rom_write_enable_set
 *
 * Purpose:
 *      Enable disable protection on SPI_EEPROM
 *
 * Input:
 *      phy_id
 * Output:
 *      SOC_E_xxx
 *
 * Notes:
 *          25AA256 256Kbit Serial EEPROM
 *          STATUS Register
 *          +------------------------------------------+
 *          | WPEN | x | x | x | BP1 | BP0 | WEL | WIP |
 *          +------------------------------------------+
 *      BP1 BP0  :   Protected Blocks
 *       0   0   :  Protect None
 *       1   1   :  Protect All
 *
 *      WEL : Write Latch Enable
 *       0  : Do not allow writes
 *       1  : Allow writes
 */
STATIC int
_phy_84728_rom_write_enable_set(phy_ctrl_t *pc, int enable)
{
    uint16  rd_data, wr_data;
    uint8   wrsr_data;
    int        count;

    /*
     * Write SPI Control Register Write Command.
     */
    count = 2;
    wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Control -2 Register Address.
     */
    wr_data = BSDK_SPI_CTRL_2_L;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Control -2 Register Word-1.
     */
    wr_data = 0x8200;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Control -2 Register Word-2.
     */
    wr_data = 0x0100;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Fill-up SPI Transmit Fifo With SPI EEPROM Messages.
     * Write SPI Control Register Write Command.
     */
    count = 4;
    wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write Tx Fifo Register Address.
     */
    wr_data = BSDK_SPI_TXFIFO;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Tx Fifo Control Word-1.
     */
    wr_data = ((1 * 0x0100) | BSDK_MSGTYPE_HWR);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Tx Fifo Control Word-2.
     */
    wr_data = ((BSDK_MSGTYPE_HWR * 0x0100) | BSDK_WREN_OPCODE);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Tx Fifo Control Word-3.
     */
    wr_data = ((BSDK_WRSR_OPCODE * 0x100) | (0x2));
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Tx Fifo Control Word-4.
     */
    wrsr_data = enable ? 0x2 : 0xc;
    wr_data = ((wrsr_data * 0x0100) | wrsr_data);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Control Register Write Command.
     */
    count = 2;
    wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Control -1 Register Address.
     */
    wr_data = BSDK_SPI_CTRL_1_L;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Control -1 Register Word-1.
     */
    wr_data = 0x0101;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Write SPI Control -1 Register Word-2.
     */
    wr_data = 0x0003;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /*
     * Wait For WRSR Command To be written.
     */
    SOC_IF_ERROR_RETURN(_phy_84728_rom_wait(pc));

    return SOC_E_NONE;
}

/*
 * Function:
 *     bsdk_phy84728_spi_firmware_update
 * Purpose:
 *     Update the firmware in SPI ROM 
 * Parameters:
 *    phy_id    - PHY Device Address
 *    new_fw    - Pointer to new firmware
 *    fw_length    - Length of the firmware
 * Returns:
 */
int
bsdk_phy84728_spi_firmware_update(phy_ctrl_t *pc,
                                  uint8 *array, uint32 datalen)
{

    uint16   data = 0;
    int      j;
    uint16   rd_data, wr_data;
    uint8    spi_values[BSDK_WR_BLOCK_SIZE];
    int count, i = 0;


    if (array == NULL) {
        return SOC_E_PARAM;
    }

    /* Set Bit 2 and Bit 0 in SPI Control */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_LN_DEV1_SPI_CTRL_STATr(pc, 0x80FD, 0xffff));

    /* ser_boot pin high */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_LN_DEV1_MISC_CNTL2r(pc, 0x1, 0x1));

    /* Read LASI Status */
    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_LN_DEV1_RX_ALARM_STATr(pc, &data));
    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_LN_DEV1_TX_ALARM_STATr(pc, &data));
    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_LN_DEV1_LASI_STATr(pc, &data));

    /* Enable LASI for Message Out */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_LN_DEV1_RX_ALARM_CNTLr(pc, 0x4, 0x4));

    /* Enable RX Alarm in LASI */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_LN_DEV1_LASI_CNTLr(pc, 0x4, 0x4));

    /* Read any residual Message out register */
    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_LN_DEV1_M8501_MSGOUTr(pc, &data));

    /* Clear LASI Message OUT status */
    SOC_IF_ERROR_RETURN(
        BSDK_RD_PHY84728_LN_DEV1_RX_ALARM_STATr(pc, &data));

    /* Set SPI-ROM write enable */
    SOC_IF_ERROR_RETURN(
        _phy_84728_rom_write_enable_set(pc, 1));


    for (j = 0; j < datalen; j +=BSDK_WR_BLOCK_SIZE) {
        /* Setup SPI Controller */
        count = 2;
        wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Control -2 Register Address.*/
        wr_data = BSDK_SPI_CTRL_2_L;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Control -2 Register Word-1. */
        wr_data = 0x8200;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Control -2 Register Word-2. */
        wr_data = 0x0100;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
        
        /* Fill-up SPI Transmit Fifo.
         * Write SPI Control Register Write Command.
         */
        count = 4 + (BSDK_WR_BLOCK_SIZE / 2);
        wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write Tx Fifo Register Address. */
        wr_data = BSDK_SPI_TXFIFO;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-1. */
        wr_data = ((1 * 0x0100) | BSDK_MSGTYPE_HWR);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-2. */
        wr_data = ((BSDK_MSGTYPE_HWR * 0x0100) | BSDK_WREN_OPCODE);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-3. */
        wr_data = ((BSDK_WR_OPCODE * 0x0100) | (0x3 + BSDK_WR_BLOCK_SIZE));
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-4. */
        wr_data = (((j & 0x00FF) * 0x0100) | ((j & 0xFF00) / 0x0100));
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        if (datalen < (j + BSDK_WR_BLOCK_SIZE)) {   /* last block */ 
            sal_memset(spi_values,0,BSDK_WR_BLOCK_SIZE);
            sal_memcpy(spi_values,&array[j],datalen - j);

            for (i = 0; i < BSDK_WR_BLOCK_SIZE; i += 2) {
                /* Write SPI Tx Fifo Data Word-4. */
                wr_data = ((spi_values[i+1] * 0x0100) | spi_values[i]);
                SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
            }           
        } else {        
            for (i = 0; i < BSDK_WR_BLOCK_SIZE; i += 2) {
                /* Write SPI Tx Fifo Data Word-4. */
                wr_data = ((array[j+i+1] * 0x0100) | array[j+i]);
                SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));
            }
        }

        /* 
         * Set-up SPI Controller To Transmit.
         * Write SPI Control Register Write Command.
         */
        count = 2;
        wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        wr_data = BSDK_SPI_CTRL_1_L;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Control -1 Register Word-1. */
        wr_data = 0x0501;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Write SPI Control -1 Register Word-2. */
        wr_data = 0x0003;
        SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

        /* Wait For 64 bytes To be written.   */
        SOC_IF_ERROR_RETURN(_phy_84728_rom_wait(pc));
    }

    /* Clear SPI-ROM write enable */
    SOC_IF_ERROR_RETURN(
        _phy_84728_rom_write_enable_set(pc, 0));

    /* Disable SPI EEPROM. */
    count = 2;
    wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write SPI Control -2 Register Address. */
    wr_data = BSDK_SPI_CTRL_2_L;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write SPI Control -2 Register Word-1. */
    wr_data = 0x8200;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write SPI Control -2 Register Word-2. */
    wr_data = 0x0100;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));


    /* Fill-up SPI Transmit Fifo With SPI EEPROM Messages. */
    count = 2;
    wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write Tx Fifo Register Address. */
    wr_data = BSDK_SPI_TXFIFO;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write SPI Tx Fifo Control Word-1. */
    wr_data = ((0x1*0x0100) | BSDK_MSGTYPE_HWR);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write SPI Tx Fifo Control Word-2. */
    wr_data = BSDK_WRDI_OPCODE;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write SPI Control Register Write Command. */
    count = 2;
    wr_data = ((BSDK_WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write SPI Control -1 Register Address. */
    wr_data = BSDK_SPI_CTRL_1_L;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write SPI Control -1 Register Word-1. */
    wr_data = 0x0101;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* Write SPI Control -1 Register Word-2. */
    wr_data = 0x0003;
    SOC_IF_ERROR_RETURN(_phy_84728_write_message(pc, wr_data, &rd_data));

    /* ser_boot pin LOW */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_LN_DEV1_MISC_CNTL2r(pc, 0x0, 0x1));

    /* Disable Bit 2 and Bit 0 in SPI Control */
    SOC_IF_ERROR_RETURN(
        BSDK_MOD_PHY84728_LN_DEV1_SPI_CTRL_STATr(pc, 0xc0F9, 0xffff));

    return SOC_E_NONE;
}

STATIC int
_phy_84728_bsc_rw(phy_ctrl_t *pc, int dev_addr, int opr,
                    int addr, int count, void *data_array,uint32 ram_start)
{
    int rv = SOC_E_NONE;
    int iter = 0;
    uint16 data16;
    int i;
    int access_type;
    int data_type;

    if (!data_array) {
        return SOC_E_PARAM;
    }

    if (count > BSDK_PHY84728_BSC_XFER_MAX) {
        return SOC_E_PARAM;
    }

    data_type = BSDK_PHY84728_I2C_DATA_TYPE(opr);
    access_type = BSDK_PHY84728_I2C_ACCESS_TYPE(opr);

    if (access_type == BSDK_PHY84728_I2CDEV_WRITE) {
        for (i = 0; i < count; i++) {
            if (data_type == BSDK_PHY84728_I2C_8BIT) {
                SOC_IF_ERROR_RETURN
                    (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, ram_start + i,
                          ((uint8 *)data_array)[i]));
            } else {  /* 16 bit */
                SOC_IF_ERROR_RETURN
                    (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, ram_start + i,
                          ((uint16 *)data_array)[i]));
            }
        }
    }

    data16 = ram_start;
    SOC_IF_ERROR_RETURN
        (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0x8004, data16));
    SOC_IF_ERROR_RETURN
        (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0x8003, addr));
    SOC_IF_ERROR_RETURN
        (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0x8002, count));

    data16 = 1;
    data16 |= (dev_addr<<9);
    if (access_type == BSDK_PHY84728_I2CDEV_WRITE) {
        data16 |= BSDK_PHY84728_WR_FREQ_400KHZ;
    }

    SOC_IF_ERROR_RETURN
        (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0x8005,data16));

    if (access_type == BSDK_PHY84728_I2CDEV_WRITE) {
        data16 =  0x8000 | BSDK_PHY84728_BSC_WRITE_OP;
    } else {
        data16 =  0x8000 | BSDK_PHY84728_BSC_READ_OP;
    }

    if (data_type == BSDK_PHY84728_I2C_16BIT) {
        data16 |= (1 << 12);
    }

    /* for single port mode, there should be only one I2C interface active
     * from lane0. The 0x800x register block is bcst type registers. If writing
     * to 0x8000 directly, it will enable all four I2C masters. Use indirect access
     * to enable only the lane 0.
     */

    SOC_IF_ERROR_RETURN
        (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0x8000, data16));

    while (iter < 100000) {
        rv = BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0x8000, &data16);
        if (((data16 & BSDK_PHY84728_2W_STAT) == BSDK_PHY84728_2W_STAT_COMPLETE)) {
            break;
        }
        iter++;
    }

    /* need some delays */
    sal_usleep(10000);

    soc_cm_print("BSC command status %d\n",(data16 & BSDK_PHY84728_2W_STAT));

    if (access_type == BSDK_PHY84728_I2CDEV_WRITE) {
        return SOC_E_NONE;
    }

    if ((data16 & BSDK_PHY84728_2W_STAT) == BSDK_PHY84728_2W_STAT_COMPLETE) {
        for (i = 0; i < count; i++) {
            SOC_IF_ERROR_RETURN
                (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, (ram_start+i), &data16));
            if (data_type == BSDK_PHY84728_I2C_16BIT) {
                ((uint16 *)data_array)[i] = data16;
                soc_cm_print("%04x ", data16);
            } else {
                ((uint8 *)data_array)[i] = (uint8)data16;
                soc_cm_print("%02x ", data16);
            }
        }
    }
    return SOC_E_NONE;
}

/*
 * Read a slave device such as NVRAM/EEPROM connected to the 84728's I2C
 * interface. This function will be mainly used for diagnostic or workaround
 * purpose.
 * Note:
 * The size of read_array buffer must be greater than or equal to the
 * parameter nbytes.
 * usage example:
 *   Retrieve the first 100 byte data of the non-volatile storage device with
 *   I2C bus device address 0x50(default SPF eeprom I2C address) on unit 0,
 *   port 2.
 *   uint8 data8[100];
 *   phy_84728_i2cdev_read(0,2,0x50,0,100,data8);
 */
int
bsdk_phy_84728_i2cdev_read(phy_ctrl_t *pc,
                     int dev_addr,  /* 7 bit I2C bus device address */
                     int offset,    /* starting data address to read */
                     int nbytes,    /* number of bytes to read */
                     uint8 *read_array)   /* buffer to hold retrieved data */
{
    return _phy_84728_bsc_rw(pc, dev_addr,BSDK_PHY84728_I2CDEV_READ,
             offset, nbytes, (void *)read_array,BSDK_PHY84728_READ_START_ADDR);

}

/*
 * Write to a slave device such as NVRAM/EEPROM connected to the 84728's I2C
 * interface. This function will be mainly used for diagnostic or workaround
 * purpose.
 * Note:
 * The size of write_array buffer should be equal to the parameter nbytes.
 * The EEPROM may limit the maximun write size to 16 bytes
 * usage example:
 *   Write to first 100 byte space of the non-volatile storage device with
 *   I2C bus device address 0x50(default SPF eeprom I2C address) on unit 0,
 *   port 2, with written data specified in array data8.
 *   uint8 data8[100];
 *   *** initialize the data8 array with written data ***
 *
 *   phy_84728_i2cdev_write(0,2,0x50,0,100,data8);
 */

int
bsdk_phy_84728_i2cdev_write(phy_ctrl_t *pc,
                     int dev_addr,  /* I2C bus device address */
                     int offset,    /* starting data address to write to */
                     int nbytes,    /* number of bytes to write */
                     uint8 *write_array)   /* buffer to hold written data */
{
    int j;
    int rv = SOC_E_NONE;

    for (j = 0; j < (nbytes/BSDK_PHY84728_BSC_WR_MAX); j++) {
        rv = _phy_84728_bsc_rw(pc, dev_addr,BSDK_PHY84728_I2CDEV_WRITE,
                    offset + j * BSDK_PHY84728_BSC_WR_MAX, BSDK_PHY84728_BSC_WR_MAX,
                    (void *)(write_array + j * BSDK_PHY84728_BSC_WR_MAX),
                    BSDK_PHY84728_WRITE_START_ADDR);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        sal_usleep(20000);
    }
    if (nbytes%BSDK_PHY84728_BSC_WR_MAX) {
        rv = _phy_84728_bsc_rw(pc, dev_addr,BSDK_PHY84728_I2CDEV_WRITE,
                offset + j * BSDK_PHY84728_BSC_WR_MAX, nbytes%BSDK_PHY84728_BSC_WR_MAX,
                (void *)(write_array + j * BSDK_PHY84728_BSC_WR_MAX),
                BSDK_PHY84728_WRITE_START_ADDR);
    }
    return rv;
}

void _phy_84728_encode_egress_message_mode(soc_port_phy_timesync_event_message_egress_mode_t mode,
                                            int offset, uint16 *value)
{

    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_NONE:
        *value |= (0x0 << offset);
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_UPDATE_CORRECTIONFIELD:
        *value |= (0x1 << offset);
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_REPLACE_CORRECTIONFIELD_ORIGIN:
        *value |= (0x2 << offset);
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_CAPTURE_TIMESTAMP:
        *value |= (0x3 << offset);
        break;
    default:
        break;
    }

}

void _phy_84728_encode_ingress_message_mode(soc_port_phy_timesync_event_message_ingress_mode_t mode,
                                            int offset, uint16 *value)
{

    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_NONE:
        *value |= (0x0 << offset);
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_UPDATE_CORRECTIONFIELD:
        *value |= (0x1 << offset);
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_TIMESTAMP:
        *value |= (0x2 << offset);
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_DELAYTIME:
        *value |= (0x3 << offset);
        break;
    default:
        break;
    }

}

void _phy_84728_decode_egress_message_mode(uint16 value, int offset,
                                            soc_port_phy_timesync_event_message_egress_mode_t *mode)
{

    switch ((value >> offset) & 0x3) {
    case 0x0:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_NONE;
        break;
    case 0x1:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_UPDATE_CORRECTIONFIELD;
        break;
    case 0x2:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_REPLACE_CORRECTIONFIELD_ORIGIN;
        break;
    case 0x3:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_CAPTURE_TIMESTAMP;
        break;
    default:
        break;
    }

}

void _phy_84728_decode_ingress_message_mode(uint16 value, int offset,
                                            soc_port_phy_timesync_event_message_ingress_mode_t *mode)
{

    switch ((value >> offset) & 0x3) {
    case 0x0:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_NONE;
        break;
    case 0x1:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_UPDATE_CORRECTIONFIELD;
        break;
    case 0x2:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_TIMESTAMP;
        break;
    case 0x3:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_DELAYTIME;
        break;
    default:
        break;
    }

}

void _phy_84728_encode_gmode(soc_port_phy_timesync_global_mode_t mode,
                                            uint16 *value)
{

    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_MODE_FREE:
        *value = 0x1;
        break;
    case SOC_PORT_PHY_TIMESYNC_MODE_SYNCIN:
        *value = 0x2;
        break;
    case SOC_PORT_PHY_TIMESYNC_MODE_CPU:
        *value = 0x3;
        break;
    default:
        break;
    }

}

void _phy_84728_decode_gmode(uint16 value,
                                            soc_port_phy_timesync_global_mode_t *mode)
{

    switch (value & 0x3) {
    case 0x1:
        *mode = SOC_PORT_PHY_TIMESYNC_MODE_FREE;
        break;
    case 0x2:
        *mode = SOC_PORT_PHY_TIMESYNC_MODE_SYNCIN;
        break;
    case 0x3:
        *mode = SOC_PORT_PHY_TIMESYNC_MODE_CPU;
        break;
    default:
        break;
    }

}


void _phy_84728_encode_framesync_mode(soc_port_phy_timesync_framesync_mode_t mode,
                                            uint16 *value)
{

    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCIN0:
        *value = 1U << 0;
        break;
    case SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCIN1:
        *value = 1U << 1;
        break;
    case SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCOUT:
        *value = 1U << 2;
        break;
    case SOC_PORT_PHY_TIMESYNC_FRAMESYNC_CPU:
        *value = 1U << 3;
        break;
    default:
        break;
    }

}

void _phy_84728_decode_framesync_mode(uint16 value,
                                            soc_port_phy_timesync_framesync_mode_t *mode)
{

    switch (value & 0xf) {
    case 1U << 0:
        *mode = SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCIN0;
        break;
    case 1U << 1:
        *mode = SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCIN1;
        break;
    case 1U << 2:
        *mode = SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCOUT;
        break;
    case 1U << 3:
        *mode = SOC_PORT_PHY_TIMESYNC_FRAMESYNC_CPU;
        break;
    default:
        break;
    }

}

void _phy_84728_encode_syncout_mode(soc_port_phy_timesync_syncout_mode_t mode,
                                            uint16 *value)
{

    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_SYNCOUT_DISABLE:
        *value = 0x0;
        break;
    case SOC_PORT_PHY_TIMESYNC_SYNCOUT_ONE_TIME:
        *value = 0x1;
        break;
    case SOC_PORT_PHY_TIMESYNC_SYNCOUT_PULSE_TRAIN:
        *value = 0x2;
        break;
    case SOC_PORT_PHY_TIMESYNC_SYNCOUT_PULSE_TRAIN_WITH_SYNC:
        *value = 0x3;
        break;
    default:
        break;
    }

}

void _phy_84728_decode_syncout_mode(uint16 value,
                                            soc_port_phy_timesync_syncout_mode_t *mode)
{

    switch (value & 0x3) {
    case 0x0:
        *mode = SOC_PORT_PHY_TIMESYNC_SYNCOUT_DISABLE;
        break;
    case 0x1:
        *mode = SOC_PORT_PHY_TIMESYNC_SYNCOUT_ONE_TIME;
        break;
    case 0x2:
        *mode = SOC_PORT_PHY_TIMESYNC_SYNCOUT_PULSE_TRAIN;
        break;
    case 0x3:
        *mode = SOC_PORT_PHY_TIMESYNC_SYNCOUT_PULSE_TRAIN_WITH_SYNC;
        break;
    default:
        break;
    }

}


int
phy_84728_timesync_config_set(int unit, soc_port_t port, soc_port_phy_timesync_config_t *conf)
{
    uint16 rx_control_reg = 0, tx_control_reg = 0, rx_tx_control_reg = 0,
           en_control_reg = 0, capture_en_reg = 0,
           nse_sc_8 = 0, nse_nco_3 = 0, value, mpls_control_reg = 0, temp = 0,
           gmode = 0, framesync_mode = 0, syncout_mode = 0, mask = 0;
    int i;
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FLAGS) {

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_ENABLE) {
            SOC_IF_ERROR_RETURN
                (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc600, 0x8000));
            SOC_IF_ERROR_RETURN
                (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc8f0, 0x0000));
            SOC_IF_ERROR_RETURN
                (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xcd58, 0x0300));
            SOC_IF_ERROR_RETURN
                (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xcd53, 0x0000));
            en_control_reg |= 3U;
            nse_sc_8 |= (1U << 12);
        }

        /* slice enable control reg */
        SOC_IF_ERROR_RETURN
            (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc600, en_control_reg, 
                                           3U));

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TS_ENABLE) {
            capture_en_reg |= 3U;
        }

        /* TXRX SOP TS CAPTURE ENABLE reg */
        SOC_IF_ERROR_RETURN
            (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc607, capture_en_reg, 3U));

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_HEARTBEAT_TS_ENABLE) {
            nse_sc_8 |= (1U << 13);
        }

        /* NSE SC 8 register */
        SOC_IF_ERROR_RETURN
            (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc63a, nse_sc_8, 
                                           (1U << 12)| (1U << 13)));

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_RX_CRC_ENABLE) {
            rx_tx_control_reg |= (1U << 3);
        }

        /* RX TX CONTROL register */
        SOC_IF_ERROR_RETURN
            (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc61c, rx_tx_control_reg, (1U << 3)));

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_8021AS_ENABLE) {
            rx_control_reg |= (1U << 3);
            tx_control_reg |= (1U << 3);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_L2_ENABLE) {
            rx_control_reg |= (1U << 2);
            tx_control_reg |= (1U << 2);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_IP4_ENABLE) {
            rx_control_reg |= (1U << 1);
            tx_control_reg |= (1U << 1);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_IP6_ENABLE) {
            rx_control_reg |= (1U << 0);
            tx_control_reg |= (1U << 0);
        }

        /* TX CONTROL register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc618, tx_control_reg));
                                          
        /* RX CONTROL register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc61a, rx_control_reg));
                                          
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CLOCK_SRC_EXT) {
            nse_nco_3 &= ~(1U << 14);
        } else {
            nse_nco_3 |= (1U << 14);
        }

        /* NSE NCO 3 register */
        SOC_IF_ERROR_RETURN
            (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc630, nse_nco_3, 
                                           (1U << 14)));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ITPID) {
        /* VLAN TAG register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc61d, conf->itpid));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID) {
        /* OUTER VLAN TAG register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc61e, conf->otpid));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID2) {
        /* INNER VLAN TAG register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc61f, conf->otpid2));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TS_DIVIDER) {
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc636, conf->ts_divider & 0xfff));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_LINK_DELAY) {
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc605, conf->rx_link_delay & 0xffff));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc606, (conf->rx_link_delay >> 16) & 0xffff));
    }


    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ORIGINAL_TIMECODE) {

        /* TIME CODE 5 register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc610, (uint16)(conf->original_timecode.nanoseconds & 0xffff)));

        /* TIME CODE 4 register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc60f, (uint16)((conf->original_timecode.nanoseconds >> 16) & 0xffff)));

        /* TIME CODE 3 register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc60e, (uint16)(COMPILER_64_LO(conf->original_timecode.seconds) & 0xffff)));

        /* TIME CODE 2 register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc60d, (uint16)((COMPILER_64_LO(conf->original_timecode.seconds) >> 16) & 0xffff)));

        /* TIME CODE 1 register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc60c, (uint16)(COMPILER_64_HI(conf->original_timecode.seconds) & 0xffff)));
    }

    mask = 0;

    /* NSE SC 8 register */

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_GMODE) {
        _phy_84728_encode_gmode(conf->gmode,&gmode);
        mask |= (0x3 << 14);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FRAMESYNC_MODE) {
        _phy_84728_encode_framesync_mode(conf->framesync.mode, &framesync_mode);
        mask |= (0xf << 2);
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63c, conf->framesync.length_threshold));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63b, conf->framesync.event_offset));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_SYNCOUT_MODE) {
        _phy_84728_encode_syncout_mode(conf->syncout.mode, &syncout_mode);
        mask |= (0x3 << 0);

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc635, conf->syncout.interval & 0xffff));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc634,
                ((conf->syncout.pulse_1_length & 0x3) << 14) | ((conf->syncout.interval >> 16) & 0x3fff)));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc633,
                ((conf->syncout.pulse_2_length & 0x1ff) << 7) | ((conf->syncout.pulse_1_length >> 2) & 0x7f)));

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc639,
                (uint16)(COMPILER_64_LO(conf->syncout.syncout_ts) & 0xfff0)));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc638,
                (uint16)((COMPILER_64_LO(conf->syncout.syncout_ts) >> 16) & 0xffff)));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc637,
                (uint16)(COMPILER_64_HI(conf->syncout.syncout_ts) & 0xffff)));
    }

    SOC_IF_ERROR_RETURN
        (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc63a, (gmode << 14) | (framesync_mode << 2) | (syncout_mode << 0), 
                                       mask));

    mask = 0;

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_TIMESTAMP_OFFSET) {
        /* RX TS OFFSET register */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc60b, (uint16)(conf->rx_timestamp_offset & 0xffff)));
        mask |= 0xf000;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_TIMESTAMP_OFFSET) {
        mask |= 0x0fff;
    }

    /* TXRX TS OFFSET register */
    SOC_IF_ERROR_RETURN
        (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc60a, (uint16)(((conf->rx_timestamp_offset & 0xf0000) >> 4) |
                                          (conf->tx_timestamp_offset & 0xfff)), mask));
    value = 0;
    mask = 0;

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_SYNC_MODE) {
        _phy_84728_encode_egress_message_mode(conf->tx_sync_mode, 0, &value);
        mask = 0x3 << 0;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_DELAY_REQUEST_MODE) {
        _phy_84728_encode_egress_message_mode(conf->tx_delay_request_mode, 2, &value);
        mask = 0x3 << 2;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_REQUEST_MODE) {
        _phy_84728_encode_egress_message_mode(conf->tx_pdelay_request_mode, 4, &value);
        mask = 0x3 << 4;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_RESPONSE_MODE) {
        _phy_84728_encode_egress_message_mode(conf->tx_pdelay_response_mode, 6, &value);
        mask = 0x3 << 6;
    }
                                            
    /* TX EVENT MESSAGE MODE SEL register */
    SOC_IF_ERROR_RETURN
        (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc601, value, 0x00ff)); 

    value = 0;
    mask = 0;

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_SYNC_MODE) {
        _phy_84728_encode_ingress_message_mode(conf->rx_sync_mode, 0, &value);
        mask = 0x3 << 0;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_DELAY_REQUEST_MODE) {
        _phy_84728_encode_ingress_message_mode(conf->rx_delay_request_mode, 2, &value);
        mask = 0x3 << 2;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_REQUEST_MODE) {
        _phy_84728_encode_ingress_message_mode(conf->rx_pdelay_request_mode, 4, &value);
        mask = 0x3 << 4;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_RESPONSE_MODE) {
        _phy_84728_encode_ingress_message_mode(conf->rx_pdelay_response_mode, 6, &value);
        mask = 0x3 << 6;
    }
                                            
    /* RX EVENT MESSAGE MODE SEL register */
    SOC_IF_ERROR_RETURN
        (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc603, value, 0x00ff)); 

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE) {

        /* Initial ref phase [15:0] */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc623, (uint16)(COMPILER_64_LO(conf->phy_1588_dpll_ref_phase) & 0xffff)));

        /* Initial ref phase [31:16]  */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc622, (uint16)((COMPILER_64_LO(conf->phy_1588_dpll_ref_phase) >> 16) & 0xffff)));

        /*  Initial ref phase [47:32] */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc621, (uint16)(COMPILER_64_HI(conf->phy_1588_dpll_ref_phase) & 0xffff)));
    }


    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE_DELTA) {
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc625, conf->phy_1588_dpll_ref_phase_delta & 0xffff));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc624, (conf->phy_1588_dpll_ref_phase_delta >> 16) & 0xffff));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K1) {
        /* DPLL K1 */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc626, conf->phy_1588_dpll_k1 & 0xff)); 
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K2) {
        /* DPLL K2 */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc627, conf->phy_1588_dpll_k2 & 0xff)); 
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K3) {
        /* DPLL K3 */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc628, conf->phy_1588_dpll_k3 & 0xff)); 
    }


    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_MPLS_CONTROL) {

        /* MPLS controls */
        if (conf->mpls_control.flags | SOC_PORT_PHY_TIMESYNC_MPLS_ENABLE) {
            mpls_control_reg |= (1U << 7) | (1U << 3);
        }

        if (conf->mpls_control.flags | SOC_PORT_PHY_TIMESYNC_MPLS_ENTROPY_ENABLE) {
            mpls_control_reg |= (1U << 6) | (1U << 2);
        }

        if (conf->mpls_control.flags | SOC_PORT_PHY_TIMESYNC_MPLS_SPECIAL_LABEL_ENABLE) {
            mpls_control_reg |= (1U << 4);
        }

        if (conf->mpls_control.flags | SOC_PORT_PHY_TIMESYNC_MPLS_CONTROL_WORD_ENABLE) {
            mpls_control_reg |= (1U << 5) | (1U << 1);
        }

        SOC_IF_ERROR_RETURN
            (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc651, mpls_control_reg, 
                (1U << 7) | (1U << 3)| (1U << 6) | (1U << 2) | (1U << 4) | (1U << 5) | (1U << 1))); 

        /* special label [19:16] */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc653, (uint16)((conf->mpls_control.special_label >> 16 ) & 0xf)));
                                          
        /* special label [15:0] */
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc652, (uint16)(conf->mpls_control.special_label & 0xffff)));

        for (i = 0; i < 10; i++ ) {
            SOC_IF_ERROR_RETURN
                (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc654 + i, (uint16)(conf->mpls_control.labels[i].value & 0xffff)));
            SOC_IF_ERROR_RETURN
                (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc661 + i, (uint16)(conf->mpls_control.labels[i].mask & 0xffff)));
        }
 
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc65e, (uint16)(
                ((conf->mpls_control.labels[3].value >> 4)  & 0xf000) | 
                ((conf->mpls_control.labels[2].value >> 8)  & 0x0f00) | 
                ((conf->mpls_control.labels[1].value >> 12) & 0x00f0) | 
                ((conf->mpls_control.labels[0].value >> 16) & 0x000f))));

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc65f, (uint16)(
                ((conf->mpls_control.labels[7].value >> 4)  & 0xf000) | 
                ((conf->mpls_control.labels[6].value >> 8)  & 0x0f00) | 
                ((conf->mpls_control.labels[5].value >> 12) & 0x00f0) | 
                ((conf->mpls_control.labels[4].value >> 16) & 0x000f))));

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc660, (uint16)(
                ((conf->mpls_control.labels[9].value >> 12) & 0x00f0) | 
                ((conf->mpls_control.labels[8].value >> 16) & 0x000f))));

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc66b, (uint16)(
                ((conf->mpls_control.labels[3].mask >> 4)  & 0xf000) | 
                ((conf->mpls_control.labels[2].mask >> 8)  & 0x0f00) | 
                ((conf->mpls_control.labels[1].mask >> 12) & 0x00f0) | 
                ((conf->mpls_control.labels[0].mask >> 16) & 0x000f))));

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc66c, (uint16)(
                ((conf->mpls_control.labels[7].mask >> 4)  & 0xf000) | 
                ((conf->mpls_control.labels[6].mask >> 8)  & 0x0f00) | 
                ((conf->mpls_control.labels[5].mask >> 12) & 0x00f0) | 
                ((conf->mpls_control.labels[4].mask >> 16) & 0x000f))));

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc66d, (uint16)(
                ((conf->mpls_control.labels[9].mask >> 12) & 0x00f0) | 
                ((conf->mpls_control.labels[8].mask >> 16) & 0x000f))));

        /* label flags */
        temp = 0;
        for (i = 0; i < 8; i++ ) {
            temp |= conf->mpls_control.labels[i].flags & SOC_PORT_PHY_TIMESYNC_MPLS_LABEL_OUT ? (0x2 << (i<<1)) : 0;
            temp |= (conf->mpls_control.labels[i].flags & SOC_PORT_PHY_TIMESYNC_MPLS_LABEL_IN)  ? (0x1 << (i<<1)) : 0;
        }

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc66e, temp)); 

        temp = 0;
        for (i = 0; i < 2; i++ ) {
            temp |= (conf->mpls_control.labels[i + 8].flags & SOC_PORT_PHY_TIMESYNC_MPLS_LABEL_OUT) ? (0x2 << (i<<1)) : 0;
            temp |= (conf->mpls_control.labels[i + 8].flags & SOC_PORT_PHY_TIMESYNC_MPLS_LABEL_IN)  ? (0x1 << (i<<1)) : 0;
        }

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc66f, temp)); 
    }


    return SOC_E_NONE;
}

int
phy_84728_timesync_config_get(int unit, soc_port_t port, soc_port_phy_timesync_config_t *conf)
{
    uint16 rx_tx_control_reg = 0, tx_control_reg = 0,
              en_control_reg = 0, capture_en_reg = 0,
              nse_sc_8 = 0, nse_nco_3 = 0, temp1, temp2, temp3, value, mpls_control_reg;
    soc_port_phy_timesync_global_mode_t gmode = SOC_PORT_PHY_TIMESYNC_MODE_FREE;
    soc_port_phy_timesync_framesync_mode_t framesync_mode = SOC_PORT_PHY_TIMESYNC_FRAMESYNC_NONE;
    soc_port_phy_timesync_syncout_mode_t syncout_mode = SOC_PORT_PHY_TIMESYNC_SYNCOUT_DISABLE;
    int i;
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    conf->flags = 0;

    /* NSE SC 8 register */
    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc63a, &nse_sc_8));

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FLAGS) {
        /* SLICE ENABLE CONTROL register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc600, &en_control_reg)); 

        if (en_control_reg & 1U) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_ENABLE;
        }

        /* TXRX SOP TS CAPTURE ENABLE reg */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc607, &capture_en_reg)); 

        if (capture_en_reg & 1U) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_CAPTURE_TS_ENABLE;
        }

        /* NSE SC 8 register */
        /* SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc63a, &nse_sc_8));
         */

        if (nse_sc_8 & (1U << 13)) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_HEARTBEAT_TS_ENABLE;
        }

         /* RX TX CONTROL register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc61c, &rx_tx_control_reg));

        if (rx_tx_control_reg & (1U << 3)) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_RX_CRC_ENABLE;
        }

        /* TX CONTROL register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc618, &tx_control_reg));
                                          
        if (tx_control_reg & (1U << 3)) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_8021AS_ENABLE;
        }

        if (tx_control_reg & (1U << 2)) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_L2_ENABLE;
        }

        if (tx_control_reg & (1U << 1)) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_IP4_ENABLE;
        }

        if (tx_control_reg & (1U << 0)) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_IP6_ENABLE;
        }

        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc630, &nse_nco_3));

        if (!(nse_nco_3 & (1U << 14))) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_CLOCK_SRC_EXT;
        }
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_GMODE) {
        _phy_84728_decode_gmode((nse_sc_8 >> 14),&gmode);
        conf->gmode = gmode;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FRAMESYNC_MODE) {
        _phy_84728_decode_framesync_mode((nse_sc_8 >> 2), &framesync_mode);
        conf->framesync.mode = framesync_mode;
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc63c, &temp1));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc63b, &temp2));
        conf->framesync.length_threshold = temp1;
        conf->framesync.event_offset = temp2;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_SYNCOUT_MODE) {
        _phy_84728_decode_syncout_mode((nse_sc_8 >> 2), &syncout_mode);
        conf->syncout.mode = syncout_mode;

        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc635, &temp1));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc634, &temp2));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc633, &temp3));

        conf->syncout.pulse_1_length =  ((temp3 & 0x7f) << 2) | ((temp2 >> 14) & 0x3);
        conf->syncout.pulse_2_length = (temp3 >> 7) & 0x1ff;
        conf->syncout.interval =  ((temp2 & 0x3fff) << 16) | temp1;

        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc639, &temp1));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc638, &temp2));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc637, &temp3));

        COMPILER_64_SET(conf->syncout.syncout_ts, ((uint32)temp3), (((uint32)temp2<<16)|((uint32)temp1)));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ITPID) {
        /* VLAN TAG register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc61d, &conf->itpid));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID) {
        /* OUTER VLAN TAG register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc61e, &conf->otpid));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID2) {
        /* INNER VLAN TAG register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc61f, &conf->otpid2));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TS_DIVIDER) {
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc636, &conf->ts_divider));
        conf->ts_divider &= 0xfff;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_LINK_DELAY) {
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc605, &temp1));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc606, &temp2));

        conf->rx_link_delay = ((uint32)temp2 << 16) | temp1; 
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ORIGINAL_TIMECODE) {
        /* TIME CODE 5 register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc610, &temp1));

        /* TIME CODE 4 register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc60f, &temp2));

        conf->original_timecode.nanoseconds = ((uint32)temp2 << 16) | temp1; 

        /* TIME CODE 3 register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc60e, &temp1));

        /* TIME CODE 2 register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc60d, &temp2));

        /* TIME CODE 1 register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc60c, &temp3));

        /* conf->original_timecode.seconds = ((uint64)temp3 << 32) | ((uint32)temp2 << 16) | temp1; */

        COMPILER_64_SET(conf->original_timecode.seconds, ((uint32)temp3),  (((uint32)temp2<<16)|((uint32)temp1)));
    }

    /* TXRX TS OFFSET register */
    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc60a, &temp1));

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_TIMESTAMP_OFFSET) {
    /* RX TS OFFSET register */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc60b, &temp2));
        conf->rx_timestamp_offset = (((uint32)(temp1 & 0xf000)) << 4) | temp2;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_TIMESTAMP_OFFSET) {
        conf->tx_timestamp_offset = temp1 & 0xfff;
    }

    /* TX EVENT MESSAGE MODE SEL register */
    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc601, &value)); 

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_SYNC_MODE) {
        _phy_84728_decode_egress_message_mode(value, 0, &conf->tx_sync_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_DELAY_REQUEST_MODE) {
        _phy_84728_decode_egress_message_mode(value, 2, &conf->tx_delay_request_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_REQUEST_MODE) {
        _phy_84728_decode_egress_message_mode(value, 4, &conf->tx_pdelay_request_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_RESPONSE_MODE) {
        _phy_84728_decode_egress_message_mode(value, 6, &conf->tx_pdelay_response_mode);
    }
                                            
    /* RX EVENT MESSAGE MODE SEL register */
    SOC_IF_ERROR_RETURN
        (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc603, &value)); 

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_SYNC_MODE) {
        _phy_84728_decode_ingress_message_mode(value, 0, &conf->rx_sync_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_DELAY_REQUEST_MODE) {
        _phy_84728_decode_ingress_message_mode(value, 2, &conf->rx_delay_request_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_REQUEST_MODE) {
        _phy_84728_decode_ingress_message_mode(value, 4, &conf->rx_pdelay_request_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_RESPONSE_MODE) {
        _phy_84728_decode_ingress_message_mode(value, 6, &conf->rx_pdelay_response_mode);
    }
                                            
    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE) {
        /* Initial ref phase [15:0] */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc623, &temp1));

        /* Initial ref phase [31:16] */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc622, &temp2));

        /* Initial ref phase [47:32] */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc621, &temp3));

        /* conf->phy_1588_dpll_phase_initial = ((uint64)temp3 << 32) | ((uint32)temp2 << 16) | temp1; */

        COMPILER_64_SET(conf->phy_1588_dpll_ref_phase, ((uint32)temp3), (((uint32)temp2<<16)|((uint32)temp1)));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE_DELTA) {
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc625, &temp1));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc624, &temp2));

        conf->phy_1588_dpll_ref_phase_delta = ((uint32)temp2 << 16) | temp1; 
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K1) {
        /* DPLL K1 */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc626, &conf->phy_1588_dpll_k1)); 
        conf->phy_1588_dpll_k1 &= 0xff;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K2) {
        /* DPLL K2 */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc627, &conf->phy_1588_dpll_k2)); 
        conf->phy_1588_dpll_k2 &= 0xff;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K3) {
        /* DPLL K3 */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc628, &conf->phy_1588_dpll_k3)); 
        conf->phy_1588_dpll_k3 &= 0xff;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_MPLS_CONTROL) {
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc651, &mpls_control_reg));
                                          
        conf->mpls_control.flags = 0;

        if (mpls_control_reg & (1U << 7)) {
            conf->mpls_control.flags |= SOC_PORT_PHY_TIMESYNC_MPLS_ENABLE;
        }

        if (mpls_control_reg & (1U << 6)) {
            conf->mpls_control.flags |= SOC_PORT_PHY_TIMESYNC_MPLS_ENTROPY_ENABLE;
        }

        if (mpls_control_reg & (1U << 4)) {
            conf->mpls_control.flags |= SOC_PORT_PHY_TIMESYNC_MPLS_SPECIAL_LABEL_ENABLE;
        }

        if (mpls_control_reg & (1U << 5)) {
            conf->mpls_control.flags |= SOC_PORT_PHY_TIMESYNC_MPLS_CONTROL_WORD_ENABLE;
        }

        /* special label [19:16] */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc653, &temp2));
                                          
        /* special label [15:0] */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc652, &temp1));

        conf->mpls_control.special_label = ((uint32)(temp2 & 0x0f) << 16) | temp1; 


        for (i = 0; i < 10; i++ ) {

            SOC_IF_ERROR_RETURN
                (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc654 + i, &temp1));
            conf->mpls_control.labels[i].value = temp1;

            SOC_IF_ERROR_RETURN
                (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc661 + i, &temp1));
            conf->mpls_control.labels[i].mask = temp1;
        }
 
        /* now get [19:16] of labels */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc65e, &temp1));

        conf->mpls_control.labels[0].value |= ((uint32)(temp1 & 0x000f) << 16); 
        conf->mpls_control.labels[1].value |= ((uint32)(temp1 & 0x00f0) << 12); 
        conf->mpls_control.labels[2].value |= ((uint32)(temp1 & 0x0f00) << 8); 
        conf->mpls_control.labels[3].value |= ((uint32)(temp1 & 0xf000) << 4); 


        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc65f, &temp1));

        conf->mpls_control.labels[4].value |= ((uint32)(temp1 & 0x000f) << 16); 
        conf->mpls_control.labels[5].value |= ((uint32)(temp1 & 0x00f0) << 12); 
        conf->mpls_control.labels[6].value |= ((uint32)(temp1 & 0x0f00) << 8); 
        conf->mpls_control.labels[7].value |= ((uint32)(temp1 & 0xf000) << 4); 


        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc660, &temp1));

        conf->mpls_control.labels[8].value |= ((uint32)(temp1 & 0x000f) << 16); 
        conf->mpls_control.labels[9].value |= ((uint32)(temp1 & 0x00f0) << 12); 

        /* now get [19:16] of masks */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc66b, &temp1));

        conf->mpls_control.labels[0].mask |= ((uint32)(temp1 & 0x000f) << 16); 
        conf->mpls_control.labels[1].mask |= ((uint32)(temp1 & 0x00f0) << 12); 
        conf->mpls_control.labels[2].mask |= ((uint32)(temp1 & 0x0f00) << 8); 
        conf->mpls_control.labels[3].mask |= ((uint32)(temp1 & 0xf000) << 4); 


        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc66c, &temp1));

        conf->mpls_control.labels[4].mask |= ((uint32)(temp1 & 0x000f) << 16); 
        conf->mpls_control.labels[5].mask |= ((uint32)(temp1 & 0x00f0) << 12); 
        conf->mpls_control.labels[6].mask |= ((uint32)(temp1 & 0x0f00) << 8); 
        conf->mpls_control.labels[7].mask |= ((uint32)(temp1 & 0xf000) << 4); 


        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc66d, &temp1));

        conf->mpls_control.labels[8].mask |= ((uint32)(temp1 & 0x000f) << 16); 
        conf->mpls_control.labels[9].mask |= ((uint32)(temp1 & 0x00f0) << 12); 


        /* label flags */
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc66e, &temp1)); 
        for (i = 0; i < 8; i++ ) {
            conf->mpls_control.labels[i].flags |= temp1 & (0x2 << (i<<1)) ? SOC_PORT_PHY_TIMESYNC_MPLS_LABEL_OUT : 0;
            conf->mpls_control.labels[i].flags |= temp1 & (0x1 << (i<<1)) ? SOC_PORT_PHY_TIMESYNC_MPLS_LABEL_IN : 0;
        }
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc66f, &temp2)); 
        for (i = 0; i < 2; i++ ) {
            conf->mpls_control.labels[i + 8].flags |= temp2 & (0x2 << (i<<1)) ? SOC_PORT_PHY_TIMESYNC_MPLS_LABEL_OUT : 0;
            conf->mpls_control.labels[i + 8].flags |= temp2 & (0x1 << (i<<1)) ? SOC_PORT_PHY_TIMESYNC_MPLS_LABEL_IN : 0;
        }

    }
    return SOC_E_NONE;
}

int
phy_84728_timesync_control_set(int unit, soc_port_t port, soc_port_control_phy_timesync_t type, uint64 value)
{
    uint16 temp1, temp2; 
    uint32 value0;
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    switch (type) {

    case SOC_PORT_CONTROL_PHY_TIMESYNC_CAPTURE_TIMESTAMP:
    case SOC_PORT_CONTROL_PHY_TIMESYNC_HEARTBEAT_TIMESTAMP:
        return SOC_E_FAIL;


    case SOC_PORT_CONTROL_PHY_TIMESYNC_NCOADDEND:
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc62f, (uint16)(COMPILER_64_LO(value) & 0xffff)));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc62e, (uint16)((COMPILER_64_LO(value) >> 16) & 0xffff)));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_FRAMESYNC:

        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc63a, &temp1));
        temp2 = ((temp1 | (0x3 << 14) | (0x1 << 12)) & ~(0xf << 2)) | (0x1 << 5);
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63a, temp2));
        sal_usleep(1);
        temp2 &= ~((0x1 << 5) | (0x1 << 12));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63a, temp2));
        sal_usleep(1);
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63a, temp1));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_LOCAL_TIME:
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc632, (uint16)(COMPILER_64_LO(value) & 0xffff)));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc631, (uint16)((COMPILER_64_LO(value) >> 16) & 0xffff)));
        SOC_IF_ERROR_RETURN
            (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc630, (uint16)COMPILER_64_HI(value), 0x0fff));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_LOAD_CONTROL:
        temp1 = 0;
        temp2 = 0;

        value0 = COMPILER_64_LO(value);

        if (value0 &  SOC_PORT_PHY_TIMESYNC_TN_LOAD) {
            temp1 |= 1U << 11;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_TN_ALWAYS_LOAD) {
            temp2 |= 1U << 11;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_TIMECODE_LOAD) {
            temp1 |= 1U << 10;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_TIMECODE_ALWAYS_LOAD) {
            temp2 |= 1U << 10;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_SYNCOUT_LOAD) {
            temp1 |= 1U << 9;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_SYNCOUT_ALWAYS_LOAD) {
            temp2 |= 1U << 9;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_LOAD) {
            temp1 |= 1U << 8;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_ALWAYS_LOAD) {
            temp2 |= 1U << 8;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_LOAD) {
            temp1 |= 1U << 7;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_ALWAYS_LOAD) {
            temp2 |= 1U << 7;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_LOAD) {
            temp1 |= 1U << 6;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_ALWAYS_LOAD) {
            temp2 |= 1U << 6;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_LOAD) {
            temp1 |= 1U << 5;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_ALWAYS_LOAD) {
            temp2 |= 1U << 5;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_LOAD) {
            temp1 |= 1U << 4;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_ALWAYS_LOAD) {
            temp2 |= 1U << 4;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_LOAD) {
            temp1 |= 1U << 3;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_ALWAYS_LOAD) {
            temp2 |= 1U << 3;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_K3_LOAD) {
            temp1 |= 1U << 2;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_K3_ALWAYS_LOAD) {
            temp2 |= 1U << 2;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_K2_LOAD) {
            temp1 |= 1U << 1;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_K2_ALWAYS_LOAD) {
            temp2 |= 1U << 1;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_K1_LOAD) {
            temp1 |= 1U << 0;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_DPLL_K1_ALWAYS_LOAD) {
            temp2 |= 1U << 0;
        }
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc614, temp1));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc615, temp2));

        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT:
        temp1 = 0;

        value0 = COMPILER_64_LO(value);

        if (value0 &  SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT) {
            temp1 |= 1U << 1;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT) {
            temp1 |= 1U << 0;
        }
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc617, temp1));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT_MASK:
        temp1 = 0;

        value0 = COMPILER_64_LO(value);

        if (value0 &  SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT_MASK) {
            temp1 |= 1U << 1;
        }
        if (value0 &  SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT_MASK) {
            temp1 |= 1U << 0;
        }
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc616, temp1));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_TX_TIMESTAMP_OFFSET:
        value0 = COMPILER_64_LO(value);

        SOC_IF_ERROR_RETURN
            (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc60a, (uint16)(value0 & 0x0fff), 0x0fff));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_RX_TIMESTAMP_OFFSET:
        value0 = COMPILER_64_LO(value);

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc60b, (uint16)(value0 & 0xffff)));
        SOC_IF_ERROR_RETURN
            (BSDK_MOD_PHY84728_LN_DEV1_PMDr(pc, 0xc60a, (uint16)((value0 >> 4) & 0xf000), 0xf000));
        break;

    default:
        return SOC_E_NONE;
        break;
    }

    return SOC_E_NONE;
}

int
phy_84728_timesync_control_get(int unit, soc_port_t port, soc_port_control_phy_timesync_t type, uint64 *value)
{
    uint16 value0 = 0;
    uint16 value1 = 0;
    uint16 value2 = 0;
    uint16 value3 = 0;
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    switch (type) {
    case SOC_PORT_CONTROL_PHY_TIMESYNC_HEARTBEAT_TIMESTAMP:

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63d, 0x4));

        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc647, &value0)); 
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc646, &value1)); 
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc645, &value2)); 

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63d, 0x8));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63d, 0x0));

    /*    *value = (((uint64)value3) << 48) | (((uint64)value2) << 32) | (((uint64)value1) << 16) | ((uint64)value0); */
        COMPILER_64_SET((*value), (((uint32)value3<<16)|((uint32)value2)),  (((uint32)value1<<16)|((uint32)value0)));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_CAPTURE_TIMESTAMP:

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63d, 0x1));

        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc640, &value0)); 
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc63f, &value1)); 
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc63e, &value2)); 

        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63d, 0x2));
        SOC_IF_ERROR_RETURN
            (BSDK_WR_PHY84728_LN_DEV1_PMDr(pc, 0xc63d, 0x0));

    /*   *value = (((uint64)value3) << 48) | (((uint64)value2) << 32) | (((uint64)value1) << 16) | ((uint64)value0); */
        COMPILER_64_SET((*value), (((uint32)value3<<16)|((uint32)value2)),  (((uint32)value1<<16)|((uint32)value0)));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_NCOADDEND:
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc62f, &value0));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc62e, &value1));
        COMPILER_64_SET((*value), (uint32)value1, (uint32)value0);
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_LOCAL_TIME:
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc632, &value0)); 
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc631, &value1)); 
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc630, &value2)); 

        COMPILER_64_SET((*value), ((uint32)(value2 & 0x0fff)),  (((uint32)value1<<16)|((uint32)value0)));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_LOAD_CONTROL:

        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0x2e, &value1));
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0x2f, &value2));

        value0 = 0;

        if (value1 & (1U << 11)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_TN_LOAD;
        }
        if (value2 & (1U << 11)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_TN_ALWAYS_LOAD;
        }
        if (value1 & (1U << 10)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_TIMECODE_LOAD;
        }
        if (value2 & (1U << 10)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_TIMECODE_ALWAYS_LOAD;
        }
        if (value1 & (1U << 9)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_SYNCOUT_LOAD;
        }
        if (value2 & (1U << 9)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_SYNCOUT_ALWAYS_LOAD;
        }
        if (value1 & (1U << 8)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_LOAD;
        }
        if (value1 & (1U << 8)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_ALWAYS_LOAD;
        }
        if (value1 & (1U << 7)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_LOAD;
        }
        if (value1 & (1U << 7)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_ALWAYS_LOAD;
        }
        if (value1 & (1U << 6)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_LOAD;
        }
        if (value2 & (1U << 6)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_ALWAYS_LOAD;
        }
        if (value1 & (1U << 5)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_LOAD;
        }
        if (value2 & (1U << 5)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_ALWAYS_LOAD;
        }
        if (value1 & (1U << 4)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_LOAD;
        }
        if (value2 & (1U << 4)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_ALWAYS_LOAD;
        }
        if (value1 & (1U << 3)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_LOAD;
        }
        if (value2 & (1U << 3)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_ALWAYS_LOAD;
        }
        if (value1 & (1U << 2)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_K3_LOAD;
        }
        if (value2 & (1U << 2)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_K3_ALWAYS_LOAD;
        }
        if (value1 & (1U << 1)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_K2_LOAD;
        }
        if (value2 & (1U << 1)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_K2_ALWAYS_LOAD;
        }
        if (value1 & (1U << 0)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_K1_LOAD;
        }
        if (value2 & (1U << 0)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_DPLL_K1_ALWAYS_LOAD;
        }
        COMPILER_64_SET((*value), 0, (uint32)value0);

        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT:
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc617, &value1));

        value0 = 0;

        if (value1 & (1U << 1)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT;
        }
        if (value1 & (1U << 0)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT;
        }
        COMPILER_64_SET((*value), 0, (uint32)value0);

        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT_MASK:
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc616, &value1));

        value0 = 0;

        if (value1 & (1U << 1)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT_MASK;
        }
        if (value1 & (1U << 0)) {
            value0 |= SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT_MASK;
        }
        COMPILER_64_SET((*value), 0, (uint32)value0);

        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_TX_TIMESTAMP_OFFSET:
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc60a, &value0));

        COMPILER_64_SET((*value), 0, (uint32)(value0 & 0x0fff));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_RX_TIMESTAMP_OFFSET:
        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc60b, &value0));

        SOC_IF_ERROR_RETURN
            (BSDK_RD_PHY84728_LN_DEV1_PMDr(pc, 0xc60a, &value1));

        COMPILER_64_SET((*value), 0, (uint32)(((((uint32)value1) << 4) & 0xf0000) | value0));
        break;

    default:
        return SOC_E_FAIL;
    }

    return SOC_E_NONE;
}

int
bsdk_phy84728_reg_read(phy_ctrl_t *  pc, uint32 flags, uint8 reg_bank,
                          uint16 reg_addr, uint16 *data)
{
    int rv;
    uint16 reg_data, xpmd_reg_sel = 0xffff;
    uint32 cl45_reg_addr;

    rv = SOC_E_NONE;

    if (flags & BSDK_PHY84728_SYS_SIDE) {
        /* Select System side registers */
        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(1, xpmd_reg_sel);
        reg_data = 0x01;
        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, reg_data));

        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(reg_bank, reg_addr);
        rv = BSDK_PHY84728_IO_MDIO_READ(pc, cl45_reg_addr, data);

        /* De-Select System side registers */
        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(1, xpmd_reg_sel);
        reg_data = 0x0;
        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, reg_data));

    } else {
        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(reg_bank, reg_addr);
        rv = BSDK_PHY84728_IO_MDIO_READ(pc, cl45_reg_addr, data);
    }

    return rv;
}

int
bsdk_phy84728_reg_write(phy_ctrl_t *  pc, uint32 flags, uint8 reg_bank,
                           uint16 reg_addr, uint16 data)
{
    int rv;
    uint16 reg_data, xpmd_reg_sel = 0xffff;
    uint32 cl45_reg_addr;

    rv = SOC_E_NONE;

    if (flags & BSDK_PHY84728_SYS_SIDE) {
        /* Select System side registers */
        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(1, xpmd_reg_sel);
        reg_data = 0x01;
        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, reg_data));

        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(reg_bank, reg_addr);
        rv = BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, data);

        /* De-Select System side registers */
        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(1, xpmd_reg_sel);
        reg_data = 0x00;
        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, reg_data));

    } else {
        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(reg_bank, reg_addr);
        rv = BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, data);
    }

    return rv;
}


int
bsdk_phy84728_reg_modify(phy_ctrl_t *  pc, uint32 flags, uint8 reg_bank,
                            uint16 reg_addr, uint16 data, uint16 mask)
{
    uint16  tmp, otmp, reg_data;
    uint32 cl45_reg_addr = 0;
    uint16 xpmd_reg_sel = 0xffff;


    if (flags & BSDK_PHY84728_SYS_SIDE) {
        /* Select System side registers */
        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(1, xpmd_reg_sel);
        reg_data = 0x01;
        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, reg_data));

        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(reg_bank, reg_addr);

        SOC_IF_ERROR_RETURN
            (BSDK_PHY84728_IO_MDIO_READ(pc, cl45_reg_addr, &tmp));

        reg_data = data & mask; /* Mask off other bits */
        otmp = tmp;
        tmp &= ~(mask);
        tmp |= reg_data;

        if (otmp != tmp) {
            SOC_IF_ERROR_RETURN
                (BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, tmp));
        }

        /* De-Select System side registers */
        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(1, xpmd_reg_sel);
        data = 0x00;
        SOC_IF_ERROR_RETURN(
            BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, data));

    } else {
        cl45_reg_addr = SOC_PHY_CLAUSE45_ADDR(reg_bank, reg_addr);

        SOC_IF_ERROR_RETURN
            (BSDK_PHY84728_IO_MDIO_READ(pc, cl45_reg_addr, &tmp));

        reg_data = data & mask; /* Mask off other bits */
        otmp = tmp;
        tmp &= ~(mask);
        tmp |= reg_data;

        if (otmp != tmp) {
            SOC_IF_ERROR_RETURN
                (BSDK_PHY84728_IO_MDIO_WRITE(pc, cl45_reg_addr, tmp));
        }
    }
    return SOC_E_NONE;
}

/*
 * Variable:
 *    phy_84728_drv
 * Purpose:
 *    Phy Driver for BCM84728 FCMAP PHY
 */

phy_driver_t phy_84728drv_xe = {
    "84728 1G/10-Gigabit PHY-FCMAP Driver",
    phy_84728_init,        /* Init */
    phy_null_reset,       /* Reset */
    phy_84728_link_get,    /* Link get   */
    phy_84728_enable_set,  /* Enable set */
    phy_null_enable_get,  /* Enable get */
    phy_84728_duplex_set,  /* Duplex set */
    phy_84728_duplex_get,  /* Duplex get */
    phy_84728_speed_set,   /* Speed set  */
    phy_84728_speed_get,   /* Speed get  */
    phy_null_set,          /* Master set */
    phy_null_zero_get,     /* Master get */
    phy_84728_an_set,      /* ANA set */
    phy_84728_an_get,      /* ANA get */
    NULL,                 /* Local Advert set, deprecated */
    NULL,                 /* Local Advert get, deprecated */
    NULL,                 /* Remote Advert get, deprecated */
    phy_84728_lb_set,      /* PHY loopback set */
    phy_84728_lb_get,      /* PHY loopback set */
    phy_null_interface_set, /* IO Interface set */
    phy_84728_interface_get, /* IO Interface get */
    NULL,                   /* pd_ability, deprecated */
    phy_84728_linkup,
    NULL,
    phy_null_mdix_set,        /* phy_84728_mdix_set */
    phy_null_mdix_get,        /* phy_84728_mdix_get */
    phy_null_mdix_status_get, /* phy_84728_mdix_status_get */
    phy_84728_medium_config_set, /* medium config setting set */
    phy_84728_medium_config_get, /* medium config setting get */
    phy_84728_medium_status,        /* active medium */
    NULL,                    /* phy_cable_diag  */
    NULL,                    /* phy_link_change */
    phy_84728_control_set,    /* phy_control_set */
    phy_84728_control_get,    /* phy_control_get */
    NULL,                    /* phy_reg_read */
    NULL,                    /* phy_reg_write */
    NULL,                    /* phy_reg_modify */
    NULL,                    /* phy_notify */
    phy_84728_probe,         /* pd_probe  */
    phy_84728_ability_advert_set,  /* pd_ability_advert_set */
    phy_84728_ability_advert_get,  /* pd_ability_advert_get */
    phy_84728_ability_remote_get,  /* pd_ability_remote_get */
    phy_84728_ability_local_get,   /* pd_ability_local_get  */
    phy_84728_firmware_set,
    phy_84728_timesync_config_set,
    phy_84728_timesync_config_get,
    phy_84728_timesync_control_set,
    phy_84728_timesync_control_get
};
#else /* INCLUDE_PHY_84728 */
int _phy_84728_not_empty;
#endif /* INCLUDE_PHY_84728 */
