/*
 * $Id: phynull.c 1.10.2.1 Broadcom SDK $
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
 * File: 	null.c
 * Purpose:	Defines NULL PHY driver routines
 */

#include <sal/types.h>
#include <sal/core/thread.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>

#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>

#include "phynull.h"
#include "phyfege.h"

/*
 * Functions:
 *	phy_null_XXX
 *
 * Purpose:	
 *	Some dummy routines for null and no
 *      connection PHY drivers.
 */

int
phy_null_init(int unit, soc_port_t port)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    return SOC_E_NONE;
}

int
phy_null_reset(int unit, soc_port_t port, void *user_arg)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);
    COMPILER_REFERENCE(user_arg);

    return SOC_E_NONE;
}

int
phy_null_set(int unit, soc_port_t port, int parm)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);
    COMPILER_REFERENCE(parm);

    return SOC_E_NONE;
}

int
phy_null_one_get(int unit, soc_port_t port, int *parm)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *parm = 1;

    return SOC_E_NONE;
}

int
phy_null_zero_get(int unit, soc_port_t port, int *parm)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *parm = 0;

    return SOC_E_NONE;
}

int
phy_null_link_get(int unit, soc_port_t port, int *up)
{
    /* link is up if phy is enabled and down otherwise */
    return phy_null_enable_get(unit, port, up);
}

int
phy_null_enable_set(int unit, soc_port_t port, int enable)
{
    if (enable) {
        PHY_FLAGS_CLR(unit, port, PHY_FLAGS_DISABLE);
    } else {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_DISABLE);
    }
    return SOC_E_NONE;
}

int
phy_null_enable_get(int unit, soc_port_t port, int *enable)
{
    *enable = !PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE);
    return SOC_E_NONE;
}

int
phy_null_an_get(int unit, soc_port_t port, int *an, int *an_done)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *an = 0;
    *an_done = 0;

    return SOC_E_NONE;
}

int
phy_null_mode_set(int unit, soc_port_t port, soc_port_mode_t mode)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);
    COMPILER_REFERENCE(mode);

    return SOC_E_NONE;
}

int
phy_null_mode_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port)) {
        *mode = SOC_PM_10GB_FD | SOC_PM_XGMII;
    } else if (IS_GE_PORT(unit, port)) {
        *mode = SOC_PM_1000MB_FD | SOC_PM_GMII |
                SOC_PM_100MB_FD | SOC_PM_100MB_HD |
                SOC_PM_10MB_FD | SOC_PM_10MB_HD | SOC_PM_MII;
    } else {
        *mode = SOC_PM_100MB_FD | SOC_PM_100MB_HD |
                SOC_PM_10MB_FD | SOC_PM_10MB_HD | SOC_PM_MII;
    }

    return SOC_E_NONE;
}

int
phy_null_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    soc_phy_info_t *pi;

    pi = &SOC_PHY_INFO(unit, port);

    if (NULL == pi) {
        return SOC_E_INIT;
    }

    if (pif == SOC_PORT_IF_TBI) {
	pi->phy_flags |= PHY_FLAGS_10B;
    } else {
	pi->phy_flags &= ~PHY_FLAGS_10B;
    }

    return SOC_E_NONE;
}

int
phy_null_duplex_set(int unit, soc_port_t port, int duplex)
{
    /* Draco Quickturn specific code. */
    if (SAL_BOOT_QUICKTURN && IS_GE_PORT(unit, port)) {
        return phy_fe_ge_duplex_set(unit, port, duplex);
    }

    return SOC_E_NONE;
}

int
phy_null_duplex_get(int unit, soc_port_t port, int *duplex)
{
    uint8	phy_addr;
    uint16	mii_ctrl;

    if (SAL_BOOT_QUICKTURN && IS_GE_PORT(unit, port)) {
        phy_addr = PORT_TO_PHY_ADDR(unit, port);
        /* Quickturn (may) have mii hooks for reconfiguring */
        SOC_IF_ERROR_RETURN(
            soc_miim_read(unit, phy_addr, MII_CTRL_REG, &mii_ctrl));
        *duplex = (mii_ctrl & MII_CTRL_FD) ? 1 : 0;
	return SOC_E_NONE;
    }

    /* Otherwise, just get speed from mac */
    *duplex = 1;
    return SOC_E_NONE;
}

int
phy_null_speed_set(int unit, soc_port_t port, int speed)
{
    uint8	phy_addr;
    uint16	mii_ctrl;

    /* Quickturn specific code. */
    if (SAL_BOOT_QUICKTURN && IS_GE_PORT(unit, port)) {
        if (0 == speed) {
           return SOC_E_NONE;
        } 
        phy_addr = PORT_TO_PHY_ADDR(unit, port);

        SOC_IF_ERROR_RETURN(
            soc_miim_read(unit, phy_addr, MII_CTRL_REG, &mii_ctrl));
        mii_ctrl &= ~(MII_CTRL_SS_LSB | MII_CTRL_SS_MSB | (1 << 9));
        switch(speed) {
        case 10:
            mii_ctrl |= MII_CTRL_SS_10;
            break;
        case 100:
            mii_ctrl |= MII_CTRL_SS_100;
            break;
        case 1000:
            mii_ctrl |= MII_CTRL_SS_1000;
            break;
        case 2500:
            /* Quickturn uses bit 9 to simulate 2.5Gbps mode */ 
            mii_ctrl |= MII_CTRL_SS_1000 | (1 << 9);
            break;
        default:
            return(SOC_E_CONFIG);
        }
        SOC_IF_ERROR_RETURN(
            soc_miim_write(unit, phy_addr, MII_CTRL_REG, mii_ctrl));
    }

    return SOC_E_NONE;
}

int
phy_null_speed_get(int unit, soc_port_t port, int *speed)
{
#if defined(BCM_ROBO_SUPPORT) || defined(BCM_ESW_SUPPORT)
    uint8	phy_addr;
    uint16	mii_ctrl;
#endif

#ifdef BCM_ROBO_SUPPORT    
    uint32	temp;
#endif

#ifdef BCM_ROBO_SUPPORT
    if(SOC_IS_ROBO(unit)) {
        if (SAL_BOOT_QUICKTURN && IS_GE_PORT(unit, port)) {
            phy_addr  = PORT_TO_PHY_ADDR(unit, port);
            /* Quickturn (may) have mii hooks for reconfiguring */
            SOC_IF_ERROR_RETURN(
                soc_miim_read(unit, phy_addr, MII_CTRL_REG, &mii_ctrl));
            switch(MII_CTRL_SS(mii_ctrl)) {
        	case MII_CTRL_SS_10:
	           *speed = 10;
        	    break;
        	case MII_CTRL_SS_100:
	           *speed = 100;
        	    break;
        	case MII_CTRL_SS_1000:
	           *speed = 1000;
        	    break;
        	default:        
                (DRV_SERVICES(unit)->port_get)
		        (unit, port, DRV_PORT_PROP_SPEED, &temp);
                return  (int)temp;
	    }
	    return SOC_E_NONE;
        }

        /* Otherwise, just get speed from mac */
        (DRV_SERVICES(unit)->port_get)
            (unit, port, DRV_PORT_PROP_SPEED, &temp);
        return  (int)temp;
    }
#endif
#ifdef BCM_ESW_SUPPORT
    if (SAL_BOOT_QUICKTURN && IS_GE_PORT(unit, port)) {
        phy_addr  = PORT_TO_PHY_ADDR(unit, port);
        /* Quickturn (may) have mii hooks for reconfiguring */
        SOC_IF_ERROR_RETURN(
            soc_miim_read(unit, phy_addr, MII_CTRL_REG, &mii_ctrl));
	switch(MII_CTRL_SS(mii_ctrl)) {
	case MII_CTRL_SS_10:
	    *speed = 10;
	    break;
	case MII_CTRL_SS_100:
	    *speed = 100;
	    break;
	case MII_CTRL_SS_1000:
	    *speed = 1000;
	    break; 
        /* Quickturn uses bit 9 to simulate 2.5Gbps mode */ 
        case (MII_CTRL_SS_1000 | (1 << 9)):
            *speed = 2500;
            break;
	default:
           return SOC_E_UNAVAIL; 
	}
	return SOC_E_NONE;
    }

    /* Otherwise, just get speed from mac */
    return SOC_E_UNAVAIL;
#endif
    return SOC_E_UNAVAIL;
}

int
phy_null_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    soc_phy_info_t *pi;

    pi = &SOC_PHY_INFO(unit, port);

    if (NULL == pi) {
        return SOC_E_INIT;
    }

    if (pi->phy_flags & PHY_FLAGS_10B) {
	*pif = SOC_PORT_IF_TBI;
    } else if (IS_GE_PORT(unit, port)) {
	*pif = SOC_PORT_IF_GMII;
    } else if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port)) {
	*pif = SOC_PORT_IF_XGMII;
    } else {
	*pif = SOC_PORT_IF_MII;
    }

    return SOC_E_NONE;
}



int
phy_nocxn_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);
    COMPILER_REFERENCE(pif);

    return SOC_E_NONE;
}

int
phy_nocxn_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *pif = SOC_PORT_IF_NOCXN;

    return SOC_E_NONE;
}

/*
 * Function:
 *	phy_null_mdix_set
 * Description:
 *	Set the Auto-MDIX mode of a port/PHY
 * Parameters:
 *	unit - Device number
 *	port - Port number
 *	mode - One of:
 *	        BCM_PORT_MDIX_AUTO
 *			Enable auto-MDIX when autonegotiation is enabled
 *	        BCM_PORT_MDIX_FORCE_AUTO
 *			Enable auto-MDIX always
 *		BCM_PORT_MDIX_NORMAL
 *			Disable auto-MDIX
 *		BCM_PORT_MDIX_XOVER
 *			Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *	BCM_E_UNAVAIL - feature unsupported by hardware
 *	BCM_E_XXX - other error
 */
int
phy_null_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mode)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    if (mode == SOC_PORT_MDIX_NORMAL) {
        return SOC_E_NONE;
    } else {
        return SOC_E_UNAVAIL;
    }
}    

/*
 * Function:
 *	phy_null_mdix_get
 * Description:
 *	Get the Auto-MDIX mode of a port/PHY
 * Parameters:
 *	unit - Device number
 *	port - Port number
 *	mode - (Out) One of:
 *	        BCM_PORT_MDIX_AUTO
 *			Enable auto-MDIX when autonegotiation is enabled
 *	        BCM_PORT_MDIX_FORCE_AUTO
 *			Enable auto-MDIX always
 *		BCM_PORT_MDIX_NORMAL
 *			Disable auto-MDIX
 *		BCM_PORT_MDIX_XOVER
 *			Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *	BCM_E_UNAVAIL - feature unsupported by hardware
 *	BCM_E_XXX - other error
 */
int
phy_null_mdix_get(int unit, soc_port_t port, soc_port_mdix_t *mode)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    if (mode == NULL) {
        return SOC_E_PARAM;
    }

    *mode = SOC_PORT_MDIX_NORMAL;

    return SOC_E_NONE;
}    

/*
 * Function:
 *	phy_null_mdix_status_get
 * Description:
 *	Get the current MDIX status on a port/PHY
 * Parameters:
 *	unit    - Device number
 *	port    - Port number
 *	status  - (OUT) One of:
 *	        BCM_PORT_MDIX_STATUS_NORMAL
 *			Straight connection
 *	        BCM_PORT_MDIX_STATUS_XOVER
 *			Crossover has been performed
 * Return Value:
 *	BCM_E_UNAVAIL - feature unsupported by hardware
 *	BCM_E_XXX - other error
 */
int
phy_null_mdix_status_get(int unit, soc_port_t port, 
                         soc_port_mdix_status_t *status)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    if (status == NULL) {
        return SOC_E_PARAM;
    }

    *status = SOC_PORT_MDIX_STATUS_NORMAL;

    return SOC_E_NONE;
}    

/*
 * Function:
 *      phy_null_medium_get
 * Description:
 *      Get the currently chosen medium for the dual-medium PHY
 * Parameters:
 *      unit    -- Device number
 *      port    -- Port number
 *      medium  -- (Out) One of
 *                 SOC_PORT_MEDIUM_NONE  
 *                 SOC_PORT_MEDIUM_COPPER
 *                 SOC_PORT_MEDIUM_FIBER
 *
 * Return value:
 *      BCM_E_NONE
 *      BCM_E_PARAM
 *      BCM_E_UNAVAIL
 */
int
phy_null_medium_get(int unit, soc_port_t port,
                    soc_port_medium_t *medium)
{
    int rv;

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    if (medium != NULL) {
        *medium = SOC_PORT_MEDIUM_NONE;
        rv = SOC_E_NONE;
    } else {    
        rv = SOC_E_PARAM;
    }
    
    return (rv);
}

/*
 * Variable:	phy_null
 * Purpose:	Phy driver callouts for a null phy (e.g. quickturn)
 * Notes:       This is the driver for a direct connection without a phy.
 */
phy_driver_t phy_null = {
    "Null PHY Driver",
    phy_null_init,		/* phy_init */
    phy_null_reset,             /* phy_reset */
    phy_null_link_get,	    /* phy_link_get */
    phy_null_enable_set,	/* phy_enable_set */
    phy_null_enable_get,	/* phy_enable_get */
    phy_null_duplex_set,	/* phy_duplex_set */
    phy_null_duplex_get,	/* phy_duplex_get */
    phy_null_speed_set,		/* phy_speed_set */
    phy_null_speed_get,		/* phy_speed_get */
    phy_null_set,		/* phy_master_set */
    phy_null_zero_get,		/* phy_master_get */
    phy_null_set,		/* phy_an_set */
    phy_null_an_get,		/* phy_an_get */
    phy_null_mode_set,		/* phy_adv_local_set */
    phy_null_mode_get,		/* phy_adv_local_get */
    phy_null_mode_get,		/* phy_adv_remote_get */
    phy_null_set,		/* phy_lb_set */
    phy_null_zero_get,		/* phy_lb_get */
    phy_null_interface_set,	/* phy_interface_set */
    phy_null_interface_get,	/* phy_interface_get */
    phy_null_mode_get,		/* phy_ability */
    NULL,			/* phy_linkup_evt */
    NULL,			/* phy_linkdn_evt */
    phy_null_mdix_set,          /* phy_mdix_set */
    phy_null_mdix_get,          /* phy_mdix_get */
    phy_null_mdix_status_get,   /* phy_mdix_status_get */
    NULL,                       /* phy_medium_config_set */
    NULL,                       /* phy_medium_config_get */
    phy_null_medium_get,        /* phy_medium_get        */
    NULL,                       /* phy_cable_diag */
    NULL                       /* phy_link_change */
};

/*
 * Variable:	phy_nocxn
 * Purpose:	Phy driver callouts for no connection
 * Notes:       This is the driver used when a port is
 *              de-initialized (eg, hotswap card is pulled.)
 */
phy_driver_t phy_nocxn = {
    "No-connection PHY Driver",
    phy_null_init,		/* phy_init */
    phy_null_reset,             /* phy_reset */
    phy_null_zero_get,		/* phy_link_get */
    phy_null_enable_set,	/* phy_enable_set */
    phy_null_enable_get,	/* phy_enable_get */
    phy_null_set,		/* phy_duplex_set */
    phy_null_one_get,		/* phy_duplex_get */
    phy_null_set,		/* phy_speed_set */
    phy_null_zero_get,		/* phy_speed_get */
    phy_null_set,		/* phy_master_set */
    phy_null_zero_get,		/* phy_master_get */
    phy_null_set,		/* phy_an_set */
    phy_null_an_get,		/* phy_an_get */
    phy_null_mode_set,		/* phy_adv_local_set */
    phy_null_mode_get,		/* phy_adv_local_get */
    phy_null_mode_get,		/* phy_adv_remote_get */
    phy_null_set,		/* phy_lb_set */
    phy_null_zero_get,		/* phy_lb_get */
    phy_nocxn_interface_set,	/* phy_interface_set */
    phy_nocxn_interface_get,	/* phy_interface_get */
    phy_null_mode_get,		/* phy_ability */
    NULL,			/* phy_linkup_evt */
    NULL,			/* phy_linkdn_evt */
    phy_null_mdix_set,          /* phy_mdix_set */
    phy_null_mdix_get,          /* phy_mdix_get */
    phy_null_mdix_status_get,   /* phy_mdix_status_get */
    NULL,                       /* phy_medium_config_set */
    NULL,                       /* phy_medium_config_get */
    phy_null_medium_get,        /* phy_medium_get        */
    NULL,                       /* phy_cable_diag */
    NULL                       /* phy_link_change */
};
