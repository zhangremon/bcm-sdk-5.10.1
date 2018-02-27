/* 
 * $Id: fe2000_link.c 1.4 Broadcom SDK $
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
 * File:        fe2000_link.c
 * Purpose:     Hardware Linkscan module
 *
 * Hardware linkscan is available, but its use is not recommended
 * because a software linkscan task is very low overhead and much more
 * flexible.
 *
 * If hardware linkscan is used, each MII operation must temporarily
 * disable it and wait for the current scan to complete, increasing the
 * latency.  PHY status register 1 may contain clear-on-read bits that
 * will be cleared by hardware linkscan and not seen later.  Special
 * support is provided for the Serdes MAC.
 */

#include <soc/types.h>
#include <soc/error.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/link.h>

#include <soc/linkctrl.h>

/*
 * Provides port number defined in PC MIIM registers for
 * writes/reads of a port PHY link status.
 * Port values in MIIM are fixed based on the port physical
 * block type/instance/index.
 */
static soc_linkctrl_port_info_t  _soc_fe2000_linkctrl_port_info[] = {
    { SOC_BLK_GPORT,    0,    0,   0 },  /* ge0  */
    { SOC_BLK_GPORT,    0,    1,   1 },  /* ge1  */
    { SOC_BLK_GPORT,    0,    2,   2 },  /* ge2  */
    { SOC_BLK_GPORT,    0,    3,   3 },  /* ge3  */
    { SOC_BLK_GPORT,    0,    4,   4 },  /* ge4  */
    { SOC_BLK_GPORT,    0,    5,   5 },  /* ge5  */
    { SOC_BLK_GPORT,    0,    6,   6 },  /* ge6  */
    { SOC_BLK_GPORT,    0,    7,   7 },  /* ge7  */
    { SOC_BLK_GPORT,    0,    8,   8 },  /* ge8  */
    { SOC_BLK_GPORT,    0,    9,   9 },  /* ge9  */
    { SOC_BLK_GPORT,    0,   10,  10 },  /* ge10 */
    { SOC_BLK_GPORT,    0,   11,  11 },  /* ge11 */
    { SOC_BLK_GPORT,    1,    0,  12 },  /* ge12 */
    { SOC_BLK_GPORT,    1,    1,  13 },  /* ge13 */
    { SOC_BLK_GPORT,    1,    2,  14 },  /* ge14 */
    { SOC_BLK_GPORT,    1,    3,  15 },  /* ge15 */
    { SOC_BLK_GPORT,    1,    4,  16 },  /* ge16 */
    { SOC_BLK_GPORT,    1,    5,  17 },  /* ge17 */
    { SOC_BLK_GPORT,    1,    6,  18 },  /* ge18 */
    { SOC_BLK_GPORT,    1,    7,  19 },  /* ge19 */
    { SOC_BLK_GPORT,    1,    8,  20 },  /* ge20 */
    { SOC_BLK_GPORT,    1,    9,  21 },  /* ge21 */
    { SOC_BLK_GPORT,    1,   10,  22 },  /* ge22 */
    { SOC_BLK_GPORT,    1,   11,  23 },  /* ge23 */
    { SOC_BLK_XPORT,    0,    0,  24 },  /* xe0  */
    { SOC_BLK_XPORT,    1,    0,  25 },  /* xe1  */
    {            -1,   -1,   -1,  -1 }   /* Last */
};


/*
 * Function:
 *     _soc_fe2000_linkctrl_linkscan_hw_init
 * Purpose:
 *     Initialize hardware linkscan.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_fe2000_linkctrl_linkscan_hw_init(int unit)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_fe2000_linkctrl_linkscan_config
 * Purpose:
 *     Set ports to hardware linkscan.
 * Parameters:
 *     unit          - Device number
 *     hw_mii_pbm    - Port bit map of ports to scan with MIIM registers
 *     hw_direct_pbm - Port bit map of ports to scan using NON MII
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_fe2000_linkctrl_linkscan_config(int unit, pbmp_t hw_mii_pbm,
                                     pbmp_t hw_direct_pbm)
{
    COMPILER_REFERENCE(unit);

    if (SOC_PBMP_NOT_NULL(hw_mii_pbm) || SOC_PBMP_NOT_NULL(hw_direct_pbm)) {
         return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_fe2000_linkctrl_linkscan_pause
 * Purpose:
 *     Pause link scanning, without disabling it.
 *     This call is used to pause scanning temporarily.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_fe2000_linkctrl_linkscan_pause(int unit)
{
    sbhandle  sb_handle;
    uint32  data;
    uint32  busy;

    sb_handle = SOC_SBX_SBHANDLE(unit);

    /* Stop link scan and wait for current pass to finish */
    data = SAND_HAL_FE2000_READ(unit, PC_MIIM_CONTROL0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN,
                              data, 0x0);
    SAND_HAL_FE2000_WRITE(unit, PC_MIIM_CONTROL0, data);

    busy = 1;
    while (busy) {
             busy = SAND_HAL_FE2000_GET_FIELD(unit, PC_MIIM_STATUS, MIIM_SCAN_BUSY,
                                  SAND_HAL_FE2000_READ(unit, PC_MIIM_STATUS));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_fe2000_linkctrl_linkscan_continue
 * Purpose:
 *     Continue link scanning after it has been paused.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_fe2000_linkctrl_linkscan_continue(int unit)
{
    sbhandle  sb_handle;
    uint32    data = 0;
    uint32    miim_addr;

    sb_handle = SOC_SBX_SBHANDLE(unit);

    /*
     * NOTE: whenever hardware linkscan is running, the PHY_REG_ADDR
     * field of the MIIM_PARAM register must be set to 1 (PHY Link
     * Status register address).
     */

    /*
     * Clause 22 Register 0x01 GE
     *             bits  [4:0] supply the register address REGAD
     *             bits [15:0] are ignored
     *
     * Clause 45 Register 0x01 Devad = 0x1 (PMA_PMD) for XE
     *             bits [20:16] supply the device address DEVAD
     *             bits [15:0]  supply the register address REGAD
     *
     * (Since 'Devad' bits are ignored in Clause 22, just used
     *  Clause 45 setting)
     */
    miim_addr = ((0x1) << 16) | 0x1;
    data = SAND_HAL_FE2000_MOD_FIELD(unit, PC_MIIM_ADDRESS, MIIM_ADDRESS,
                              data, miim_addr);
    SAND_HAL_FE2000_WRITE(unit, PC_MIIM_ADDRESS, data);

    /* Enable linkscan */
    data = SAND_HAL_FE2000_READ(unit, PC_MIIM_CONTROL0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN,
                              data, 0x1);
    SAND_HAL_FE2000_WRITE(unit, PC_MIIM_CONTROL0, data);

    return SOC_E_NONE;
}

/*
 * Function:    
 *     _soc_fe2000_linkctrl_update
 * Purpose:
 *     Update the forwarding state in device.
 * Parameters:  
 *      unit - Device unit number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_fe2000_linkctrl_update(int unit)
{
    return SOC_E_NONE;
}

/*
 * Link Control Driver - FE2000
 */
soc_linkctrl_driver_t  soc_linkctrl_driver_fe2000 = {
    _soc_fe2000_linkctrl_port_info,            /* port mapping */
    _soc_fe2000_linkctrl_linkscan_hw_init,     /* ld_linkscan_hw_init */
    _soc_fe2000_linkctrl_linkscan_config,      /* ld_linkscan_config */
    _soc_fe2000_linkctrl_linkscan_pause,       /* ld_linkscan_pause */
    _soc_fe2000_linkctrl_linkscan_continue,    /* ld_linkscan_continue */
    _soc_fe2000_linkctrl_update                /* ld_update */
};
