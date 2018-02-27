/*
 * $Id: fe2000_xmac.c 1.11 Broadcom SDK $
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
 * File:        fe2000_xmac.c
 * Purpose:     XGS 10Gigabit Media Access Controller Driver (Bigmac)
 *
 * This module is used for:
 *   - XE ports on SBX FE2000 device
 *
 * NOTE:
 * Driver is up-to-date to ESW driver /soc/esw/xmac.c, version 1.20
 * MAC drivers will share the same code when restructure of MAC
 * drivers to remove device dependency is complete.
 *
 * This supports only phy revision 5 (FE2000).
 * HiGig mode is currently not supported.
 */

#include <soc/error.h>
#include <soc/debug.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>


/*
 * Debug Output Macros
 */
#define DBG_10G_OUT(flags, stuff) SOC_DEBUG(SOC_DBG_10G | flags, stuff)
#define DBG_10G_VERB(stuff)       DBG_10G_OUT(SOC_DBG_VERBOSE, stuff)
#define DBG_10G_WARN(stuff)       DBG_10G_OUT(SOC_DBG_WARN, stuff)

/*
 * General Utility Macros
 */
#define REG_OFFSET(unit, reg)    SAND_HAL_FE2000_REG_OFFSET(unit, reg)

/*
 * Transmit CRC Modes
 * (Receive has bit field definitions in register)
 */
#define XMAC_CRC_APPEND          0x00   /* Append CRC (Default) */
#define XMAC_CRC_KEEP            0x01   /* CRC Assumed correct */
#define XMAC_CRC_REGEN           0x02   /* Replace CRC with a new one */
#define XMAC_CRC_RSVP            0x03   /* Reserved (does Replace) */

/* Max legal value 16360 */
#define JUMBO_MAXSZ              0x3fe8


/*
 * Some Forward Declarations
 */
STATIC int _fe2000_xmac_control_set(int unit, soc_port_t port, 
                                    soc_mac_control_t type, int value);
STATIC int _fe2000_xmac_duplex_get(int unit, soc_port_t port, int *duplex);
STATIC int _fe2000_xmac_speed_get(int unit, soc_port_t port, int *speed);


#ifdef BROADCOM_DEBUG
static char *xmac_encap_mode[] = SOC_ENCAP_MODE_NAMES_INITIALIZER;
static char *xmac_port_if_names[] = SOC_PORT_IF_NAMES_INITIALIZER;
#endif /* BROADCOM_DEBUG */

/*
 * LCPLL lock check
 */
STATIC void
_soc_fe2000_xgxs_lcpll_lock_check(int unit, soc_port_t port)
{
    if (soc_feature(unit, soc_feature_xgxs_lcpll)) {
        sbhandle  sb_handle;
        int32     block_num;
        uint32    data;        

        sb_handle = SOC_SBX_SBHANDLE(unit);
        block_num = 0;  /* XGXS PLL reg is shared for both XG blocks */

        data = SAND_HAL_FE2000_READ_STRIDE(sb_handle, XG, block_num,
                                    XG_XGXS_PLL_STATUS);
        if (SOC_IS_SBX_FE2KXT(unit)) {
            if (!(SAND_HAL_GET_FIELD(C2, XG_XGXS_PLL_STATUS,
                                     DONE_CAL, data))) {
                SOC_ERROR_PRINT((DK_ERR,
                                 "soc_xgxs_lcpll_lock_check: "
                                 "LCPLL not locked on unit=%d port=%d (%s) "
                                 "status=0x%08x\n",
                                 unit, port, SOC_PORT_NAME(unit, port), data));
            }
        }else{
            if (!(SAND_HAL_GET_FIELD(CA, XG_XGXS_PLL_STATUS,
                                     FREQ_PASS_SM, data))) {
                SOC_ERROR_PRINT((DK_ERR,
                                 "soc_xgxs_lcpll_lock_check: "
                                 "LCPLL not locked on unit=%d port=%d (%s) "
                                 "status=0x%08x\n",
                                 unit, port, SOC_PORT_NAME(unit, port), data));
            }
        }
    }

    return;
}

/*
 * Function:
 *     soc_fe2000_fusioncore_reset (10(X)-Gig/Xaui/Serdes (XGXS) reset)
 * Purpose:
 *     Reset and initialize the BigMAC and Fusion core technology.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     speed - in Mbps (10000, 12000) or -1 to leave speed unchanged.
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     This supports phy revision 5 (FE2000)
 */
int
soc_fe2000_fusioncore_reset(int unit, soc_port_t port, int speed)
{
    sbhandle       sb_handle;
    int32          block_num;
    int32          max_block;
    uint32         stride, offset, data;
    uint32         cfg_lo, cfg_hi;
    uint8          phy_addr;
    int            preemph, idriver, pdriver;
    int            plllock;
    int            reset_sleep_usec;
    
    DBG_10G_VERB(("xmac: unit %d port %s: fusioncore reset\n",
                  unit, SOC_PORT_NAME(unit, port)));

    if (!soc_feature(unit, soc_feature_xgxs_v5)) {
        return SOC_E_INTERNAL;
    }

    sb_handle = SOC_SBX_SBHANDLE(unit);
    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);
    if (SOC_IS_SBX_FE2KXT(unit)) {
        max_block = 3;
    }else{
        max_block = 1;
    }

    if (block_num > max_block) {
        return SOC_E_INTERNAL;
    }

    if (SAL_BOOT_BCMSIM) {
        return SOC_E_NONE;
    }

    /* CX4 compliance related */    
    if (SOC_IS_SBX_FE2KXT(unit)) {
        offset = SAND_HAL_REG_OFFSET(C2, PC_MIIM_XGXS_CONFIG0);
        stride = SAND_HAL_REG_OFFSET(C2, PC_MIIM_XGXS_CONFIG1) - offset;
        offset = offset + (stride * block_num);

        data = SAND_HAL_READ_OFFS(sb_handle, offset);

        data = SAND_HAL_MOD_FIELD(C2, PC_MIIM_XGXS_CONFIG0,
                                  MDIO_DEVAD, data, 5);
        data = SAND_HAL_MOD_FIELD(C2, PC_MIIM_XGXS_CONFIG0,
                                  PHY_CLAUSE_45, data, 0x0);
        data = SAND_HAL_MOD_FIELD(C2, PC_MIIM_XGXS_CONFIG0,
                                  IEEE_DEVICES_IN_PKG, data,
                                  (IS_HG_PORT(unit, port)) ? 0x03 : 0x15);
        SAND_HAL_WRITE_OFFS(sb_handle, offset, data);
    }else{
        offset = SAND_HAL_REG_OFFSET(CA, PC_MIIM_XGXS_CONFIG_0);
        stride = SAND_HAL_REG_OFFSET(CA, PC_MIIM_XGXS_CONFIG_1) - offset;
        offset = offset + (stride * block_num);

        data = SAND_HAL_READ_OFFS(sb_handle, offset);

        data = SAND_HAL_MOD_FIELD(CA, PC_MIIM_XGXS_CONFIG_0,
                                  MDIO_DEVAD, data, 0x0);
        data = SAND_HAL_MOD_FIELD(CA, PC_MIIM_XGXS_CONFIG_0,
                                  PHY_CLAUSE_45, data, 0x0);
        data = SAND_HAL_MOD_FIELD(CA, PC_MIIM_XGXS_CONFIG_0,
                                  IEEE_DEVICES_IN_PKG, data,
                                  (IS_HG_PORT(unit, port)) ? 0x03 : 0x15);
        SAND_HAL_WRITE_OFFS(sb_handle, offset, data);
    }

    if (soc_feature(unit, soc_feature_xgxs_lcpll)) {
        uint32 lcpll; /* 10 Gbps */

        if (IS_HG_PORT(unit, port) && speed > 0) {
            /*
             * LCPLL clock speed selection
             *
             * TODO:  Check out_sel field
             *        outsel has 4 bits, is only 1 bit used?
             *        so xg0 is for first port, and xg1 for second port?
             */
            lcpll = (speed == 12000) ? 1 : 0;

            if (SOC_IS_SBX_FE2KXT(unit)) {
	      /* 10g phy needs this param set to FALSE to setup 10G HiGig mode */
	      if (soc_property_port_get(unit, port, spn_10G_IS_CX4, TRUE)) {
		soc_cm_print("%s.%d = 0 in config.bcm required for HiGig mode\n",spn_10G_IS_CX4, port);
		assert(0);
	      }	      
            }else{
                data = SAND_HAL_READ_STRIDE(sb_handle, CA, XG, block_num,
                                            XG_XGXS_PLL_CONTROL0);
                data = SAND_HAL_MOD_FIELD(CA, XG_XGXS_PLL_CONTROL0,
                                          OUT_SEL, data, lcpll);
                SAND_HAL_WRITE_STRIDE(sb_handle, CA, XG, block_num,
                                      XG_XGXS_PLL_CONTROL0, data);
            }
        }

        /*
         * Fusion core reference clock selection
         */
        lcpll = 0; /* External reference clock for Fusion Core */
        if (soc_property_port_get(unit, port, spn_XGXS_LCPLL, 1)) {
            lcpll = 1; /* Internal LCPLL reference clock for Fusion Core */
            /* Double-check LCPLL lock */
            _soc_fe2000_xgxs_lcpll_lock_check(unit, port);
        }

        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_read(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                    &cfg_hi, &cfg_lo));
        cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                    SELECT_LOCAL_PLL, cfg_lo, lcpll);
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_write(unit, block_num,
                                     REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                     cfg_hi, cfg_lo));
    }

    /*
     * Allow properties to change important variables.
     */

    plllock = soc_property_port_get(unit, port, spn_XGXS_PLLLOCK, 0x7);
    /* NB: preemphasis, driver, pre-driver currents are bit-reversed in HW */
    preemph = soc_property_port_get(unit, port, spn_XGXS_PREEMPHASIS, 0);
    preemph = _shr_bit_rev8(preemph) >> 4;

    idriver = soc_property_port_get(unit, port, 
                                    spn_XGXS_DRIVER_CURRENT, 0xb);
    idriver = _shr_bit_rev8(idriver) >> 4;

    pdriver = soc_property_port_get(unit, port, 
                                    spn_XGXS_PRE_DRIVER_CURRENT, 0xb);
    pdriver = _shr_bit_rev8(pdriver) >> 4;

    phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);
    reset_sleep_usec = 100;

    /*
     * XGXS MAC initialization steps.
     *
     * A minimum delay is required between various initialization steps.
     * There is no maximum delay.  The values given are very conservative
     * including the timeout for PLL lock.
     */

    /* Release reset (if asserted) to allow bigmac to initialize */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                &cfg_hi, &cfg_lo));
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                ANALOG_CLK_SHUTDOWN, cfg_lo, 0x0);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                DIGITAL_CLK_SHUTDOWN, cfg_lo, 0x0);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                RESET_N, cfg_lo, 0x1);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                 cfg_hi, cfg_lo));
    sal_usleep(reset_sleep_usec);

    /* PowerDown fusion-Core and PHY */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                &cfg_hi, &cfg_lo));
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                ANALOG_CLK_SHUTDOWN, cfg_lo, 0x1);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                DIGITAL_CLK_SHUTDOWN, cfg_lo, 0x1);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                RESET_N, cfg_lo, 0x0);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                TX_FIFO_AUTO_RESET_ENABLE, cfg_lo, 0x1);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                TX_FIFO_RESET_N, cfg_lo, 0x1);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                 cfg_hi, cfg_lo));
    sal_usleep(reset_sleep_usec);

    /*
     * Powerup FusionCore interface (analog and clocks).
     *
     * NOTE: Many MAC registers are not accessible until the FusionCore
     * achieves PLL lock.  An S-Channel timeout will occur before that.
     */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                &cfg_hi, &cfg_lo));
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                ANALOG_CLK_SHUTDOWN, cfg_lo, 0x0);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                DIGITAL_CLK_SHUTDOWN, cfg_lo, 0x0);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                 cfg_hi, cfg_lo));
    sal_usleep(reset_sleep_usec);

    /* Bring FusionCore out of reset.  AFIFO_RST stays 1.  */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                &cfg_hi, &cfg_lo));
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_XGXS_CONFIG_LO,
                                RESET_N, cfg_lo, 0x1);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_XGXS_CONFIG_LO),
                                 cfg_hi, cfg_lo));
    sal_usleep(reset_sleep_usec);


    /* Release LMD reset */
    if (soc_feature(unit, soc_feature_lmd)) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_read(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                    &cfg_hi, &cfg_lo));
        cfg_lo = SAND_HAL_FE2000_MOD_FIELD(sb_handle, XM_MAC_CONFIG_LO,
                                    LMD_RESET_N, cfg_lo, 0x1);
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_write(unit, block_num,
                                     REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                     cfg_hi, cfg_lo));
    }

    /* For XGXS5 PHY, the PHY initialization is done only after PHY
     * probing. Therefore, PLL lock is checked in xmac_init.
     */

    return SOC_E_NONE;
}

/*
 * Function:
 *     _fe2000_xmac_ifg_to_ipg
 * Description:
 *     Converts the inter-frame gap specified in bit-times into a value 
 *     suitable to be programmed into the IPG register
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     speed  - the speed for which the IFG is being set
 *     ifg    - Inter-frame gap in bit-times
 *     ipg    - (OUT) the value to be written into IPG
 * Return Value:
 *     SOC_E_XXX
 * Notes:
 *     The function makes sure the calculated IPG value will not cause
 *     hardware to fail. If the requested ifg value cannot be supported in
 *     hardware, the function will choose a value that approximates the 
 *     requested value as best as possible. 
 *
 *     Specifically:
 *        -- Current chips only support ifg which is divisible by 8. If 
 *           the specifieg ifg is not divisible by 8, it will be rounded 
 *           to the next multiplier of 8 (65 will result in 72).
 *        -- ifg < 64 are not supported
 */
STATIC int
_fe2000_xmac_ifg_to_ipg(int unit, soc_port_t port, int speed,
                        int ifg, int *ipg)
{
    /*
     * The inter-frame gap should be a multiple of 8 bit-times.
     */
    ifg = (ifg + 7) & ~7;

    /*
     * The smallest supported ifg is 64 bit-times
     */
    ifg = (ifg < 64) ? 64 : ifg;
     
    if (IS_HG_PORT(unit, port)) {
        switch (speed) {
        case 10000:
        case 12000:
            *ipg = ifg / 8 - 4;
            break;
        default:
            return SOC_E_PARAM;
            break;
        }
    } else if (IS_XE_PORT(unit, port)) {
        switch (speed) {
        case 10000:
            *ipg = ifg / 8;
            break;
        default:
            return SOC_E_PARAM;
            break;
        }
    } else {
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _fe2000_xmac_ipg_to_ifg
 * Description:
 *     Converts the IPG register value into the inter-frame gap expressed in
 *     bit-times
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     speed  - the speed for which the IFG is being set
 *     ipg    - the value in the IPG register
 *     ifg    - Inter-frame gap in bit-times
 * Return Value:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_ipg_to_ifg(int unit, soc_port_t port, int speed,
                        int ipg, int *ifg)
{
    /*
     * Now we need to convert the value accoring to various chips' 
     * peculiarities
     */
    if (IS_HG_PORT(unit, port)) {
        switch (speed) {
        case 10000:
        case 12000:
            *ifg = (ipg + 4) * 8;
            break;
        default:
            return SOC_E_PARAM;
            break;
        }
    } else if (IS_XE_PORT(unit, port)) {
        switch (speed) {
        case 10000:
            *ifg = ipg * 8;
            break;
        default:
            return SOC_E_PARAM;
            break;
        }
    } else {
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _fe2000_xmac_ipg_update
 * Purpose:
 *     Set the IPG appropriate for current speed/duplex
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 * Notes:
 *     The current speed/duplex are read from the hardware registers.
 */
STATIC int
_fe2000_xmac_ipg_update(int unit, int port)
{
    int32      block_num;
    uint32     data_hi, data_lo;
    uint32     data_prev;
    int        fd, speed, ifg, ipg;
    soc_ipg_t  *si = &SOC_PERSIST(unit)->ipg[port];


    SOC_IF_ERROR_RETURN(_fe2000_xmac_speed_get(unit, port, &speed));
    SOC_IF_ERROR_RETURN(_fe2000_xmac_duplex_get(unit, port, &fd));

    /*
     * Get the ifg value in bit-times
     */
    if (fd) {
        switch (speed) {
        case 10000:
        case 12000:
            ifg = si->fd_10000;
            break;
        default:
            return SOC_E_INTERNAL;
        }
    } else {
        return SOC_E_INTERNAL;
    }

    /*
     * Convert the ifg value from bit-times into IPG register-specific value
     */
    SOC_IF_ERROR_RETURN(_fe2000_xmac_ifg_to_ipg(unit, port, speed,
                                                ifg, &ipg));

    /*
     * Program the appropriate register
     */
    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                &data_hi, &data_lo));
    data_prev = data_lo;
    data_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_TX_CONFIG_LO, AVG_IPG,
                                 data_lo, ipg & 0x1f);
    if (data_lo != data_prev) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_write(unit, block_num,
                                     REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                     data_hi, data_lo));
    }

    return SOC_E_NONE;
}
                        
/*
 * Function:
 *     _fe2000_xmac_frame_spacing_stretch_set
 * Purpose:
 *     Set frame spacing stretch parameters.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_frame_spacing_stretch_set(int unit, soc_port_t port, int value)
{
    int32   block_num;
    uint32  data_hi, data_lo;
    uint32  data_prev;
    int     field_width, max_value;

    field_width = SAND_HAL_FE2000_FIELD_MSB(unit, XM_MAC_TX_CONFIG_LO, THROTTLE_DENOM)
        - SAND_HAL_FE2000_FIELD_LSB(unit, XM_MAC_TX_CONFIG_LO, THROTTLE_DENOM) + 1;
    max_value = (1 << (field_width + 1)) - 1;
    if ((value > max_value) || (value < 0)) {
        return SOC_E_PARAM;
    }
        
    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                &data_hi, &data_lo));
    data_prev = data_lo;
    data_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_TX_CONFIG_LO, THROTTLE_DENOM,
                                 data_lo, value);
    data_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_TX_CONFIG_LO, THROTTLE_NUM,
                                 data_lo, value ? 1 : 0);
    if (data_lo != data_prev) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_write(unit, block_num,
                                     REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                     data_hi, data_lo));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_init
 * Purpose:
 *     Initialize Xmac / FusionCore MAC into a known good state.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 * Returns:
 *     SOC_E_XXX
 * Notes:
 */
STATIC int
_fe2000_xmac_init(int unit, soc_port_t port)
{
    int32          block_num;
    uint32         rx_cfg_hi, rx_cfg_lo;
    uint32         tx_cfg_hi, tx_cfg_lo;
    uint32         cfg_hi, cfg_lo;
    int            wan_mode;
    int            encap;
    int            pll_lock_usec;
    int            locked;
    soc_timeout_t  to;


    DBG_10G_VERB(("xmac_init: unit=%d port %s\n",
                  unit, SOC_PORT_NAME(unit, port)));

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    /* Disable Tx/Rx, assume that MAC is stable (or out of reset) */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                &rx_cfg_hi, &rx_cfg_lo));
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                &tx_cfg_hi, &tx_cfg_lo));
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                &cfg_hi, &cfg_lo));
    
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, RX_ENABLE, cfg_lo, 0);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, TX_ENABLE, cfg_lo, 0);
    tx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_TX_CONFIG_LO, PAUSE_ENABLE,
                                   tx_cfg_lo, 1);
    rx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_RX_CONFIG_LO, RX_PAUSE_ENABLE,
                                   rx_cfg_lo, 1);
  
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                 cfg_hi, cfg_lo));
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                 tx_cfg_hi, tx_cfg_lo));
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                 rx_cfg_hi, rx_cfg_lo));

    /* Wan mode */
    wan_mode = soc_property_port_get(unit, port, spn_PHY_WAN_MODE, FALSE);
    if (wan_mode) {
        /* Max speed for WAN mode is 9.294Gbps.  
         * This setting gives 10Gbps * (13/14) or 9.286 Gbps */
        SOC_IF_ERROR_RETURN
            (_fe2000_xmac_control_set(unit, port, 
                                      SOC_MAC_CONTROL_FRAME_SPACING_STRETCH,
                                      13));
    }

    /* Update IPG separately */
    SOC_IF_ERROR_RETURN(_fe2000_xmac_ipg_update(unit, port));

    /* Re-read MAC_TXCTRL, since it gets affected by ipg_update() */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                &tx_cfg_hi, &tx_cfg_lo));

    /* Set jumbo max size */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_TX_MAX_SIZE_LO),
                                 0, JUMBO_MAXSZ));
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_RX_MAX_SIZE_LO),
                                 0, JUMBO_MAXSZ));

    /* Program MAC settings */
    /* Setup header mode */
    if (IS_XE_PORT(unit, port)) {
        encap = SOC_ENCAP_IEEE;         /* Ethernet port, so only IEEE mode */
    } else {
        encap = SOC_ENCAP_HIGIG;        /* Default: Enable HIGIG header mode */
    }
    tx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_TX_CONFIG_LO, HDR_MODE,
                                   tx_cfg_lo, encap);
    rx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_RX_CONFIG_LO, HDR_MODE,
                                   rx_cfg_lo, encap);

    /*
     * Power-up defaults are CRC_APPEND for TX and STRIPCRC for RX.
     * Keep them at these values to be consistant with SBX architecture.
     */
    tx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_TX_CONFIG_LO, CRC_MODE,
                                   tx_cfg_lo, XMAC_CRC_APPEND);
    rx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_RX_CONFIG_LO, STRIP_CRC,
                                   rx_cfg_lo, 1);
    
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                 tx_cfg_hi, tx_cfg_lo));
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                 rx_cfg_hi, rx_cfg_lo));

    /* Disable loopback */
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, REMOTE_LOOPBACK_ENABLE,
                                cfg_lo, 0);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, LOCAL_LOOPBACK_ENABLE,
                                cfg_lo, 0);

    /* Finally, take MAC out of reset (program enables) */
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, RX_ENABLE, cfg_lo, 1);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, TX_ENABLE, cfg_lo, 1);
    cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, LMD_RESET_N, cfg_lo, 1);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                 cfg_hi, cfg_lo));

    /* After XMAC is initialized, Check to make sure PLL is stable. */
    /* Wait up to 0.1 sec for TX PLL lock. */
    pll_lock_usec = 100000;
    soc_timeout_init(&to, pll_lock_usec, 0);

    locked = 0;
    while (!soc_timeout_check(&to)) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_read(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_XGXS_STATUS_LO),
                                    &cfg_hi, &cfg_lo));

        locked = SAND_HAL_FE2000_GET_FIELD(unit, XM_MAC_XGXS_STATUS_LO,
                                    TX_PLL_LOCK, cfg_lo);
        if (locked) {
            break;
        }
    }

    if (!locked) {
        soc_cm_debug(DK_ERR,
            "unit %d port %s: FusionCore PLL did not lock after xmac_init\n",
             unit, SOC_PORT_NAME(unit, port));
    } else {
        DBG_10G_VERB(("unit %d port %s: FusionCore PLL locked in %d usec\n",
                      unit, SOC_PORT_NAME(unit, port),
                      soc_timeout_elapsed(&to)));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_enable_set
 * Purpose:
 *     Enable or disable MAC.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     enable - TRUE to enable, FALSE to disable
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_enable_set(int unit, soc_port_t port, int enable)
{
    int32   block_num;
    uint32  data_hi, data_lo;
    uint32  data_prev;

    DBG_10G_VERB(("xmac_enable_set: unit=%d port=%s %sable\n",
                  unit, SOC_PORT_NAME(unit, port),
                  enable ? "en" : "dis"));

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                &data_hi, &data_lo));

    /* Don't disable TX since it stops egress and hangs if CPU sends */
    data_prev = data_lo;
    data_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, TX_ENABLE, data_lo, 1);
    data_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, RX_ENABLE, data_lo,
                                 enable ? 1 : 0);
    if (data_lo != data_prev) {
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                 data_hi, data_lo));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_enable_get
 * Purpose:
 *     Get MAC enable state.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     enable - (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_enable_get(int unit, soc_port_t port, int *enable)
{
    int32   block_num;
    uint32  data_hi, data_lo;

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                &data_hi, &data_lo));

    *enable = SAND_HAL_FE2000_GET_FIELD(unit, XM_MAC_CONFIG_LO, RX_ENABLE, data_lo);

    DBG_10G_VERB(("xmac_enable_get:unit=%d port=%d enable=%c\n",
                  unit, port, *enable ? 'T' : 'F'));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_duplex_set
 * Purpose:
 *     Set XMAC in the specified duplex mode.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     duplex - Boolean: true --> full duplex, false --> half duplex
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     Only duplex mode is available.
 */
STATIC int
_fe2000_xmac_duplex_set(int unit, soc_port_t port, int duplex)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    /* Always full duplex */
    if (!duplex) {
        return SOC_E_PARAM;
    }

    DBG_10G_VERB(("xmac_duplex_set: unit=%d port=%s %s duplex\n",
                  unit, SOC_PORT_NAME(unit, port),
                  duplex ? "FULL" : "HALF"));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_duplex_get
 * Purpose:
 *     Get XMAC duplex mode.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     duplex - (OUT) Boolean: true --> full duplex, false --> half duplex
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_duplex_get(int unit, soc_port_t port, int *duplex)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);
    *duplex = TRUE; /* Always full duplex */
    DBG_10G_VERB(("xmac_duplex_get: unit=%d port=%s %s duplex\n",
                  unit, SOC_PORT_NAME(unit, port),
                  *duplex ? "FULL" : "HALF"));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_pause_set
 * Purpose:
 *     Configure XMAC to transmit/receive pause frames.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     pause_tx - Boolean: transmit pause or -1 (don't change)
 *     pause_rx - Boolean: receive pause or -1 (don't change)
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_pause_set(int unit, soc_port_t port, int pause_tx, int pause_rx)
{
    int32   block_num;
    uint32  data_hi, data_lo;
    uint32  data_prev;

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    /* Transmit Pause frame function */
    if (pause_tx >= 0) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_read(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                    &data_hi, &data_lo));
        data_prev = data_lo;
        data_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_TX_CONFIG_LO, PAUSE_ENABLE,
                                     data_lo, pause_tx ? 1 : 0);
        if (data_lo != data_prev) {
            SOC_IF_ERROR_RETURN
                (soc_sbx_fe2000_xm_write(unit, block_num,
                                         REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                         data_hi, data_lo));
        }
    }

    /* Receive Pause frame function */
    if (pause_rx >= 0) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_read(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                    &data_hi, &data_lo));
        data_prev = data_lo;
        data_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_RX_CONFIG_LO, RX_PAUSE_ENABLE,
                                     data_lo, pause_rx ? 1 : 0);
        if (data_lo != data_prev) {
            SOC_IF_ERROR_RETURN
                (soc_sbx_fe2000_xm_write(unit, block_num,
                                         REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                         data_hi, data_lo));
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_pause_get
 * Purpose:
 *     Return the pause ability of XMAC.
 * Parameters:
 *     unit      - Device number
 *     port      - Device port number
 *     pause_tx  - (OUT) Boolean: transmit pause
 *     pause_rx  - (OUT) Boolean: receive pause
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_fe2000_xmac_pause_get(int unit, soc_port_t port, int *pause_tx, int *pause_rx)
{
    int32   block_num;
    uint32  data_hi, data_lo;

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                &data_hi, &data_lo));
    *pause_rx = SAND_HAL_FE2000_GET_FIELD(unit, XM_MAC_RX_CONFIG_LO, RX_PAUSE_ENABLE,
                                   data_lo);
    
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                &data_hi, &data_lo));
    *pause_tx =  SAND_HAL_FE2000_GET_FIELD(unit, XM_MAC_TX_CONFIG_LO, PAUSE_ENABLE,
                                    data_lo);

    DBG_10G_VERB(("xmac_pause_get: unit=%d port=%s RX=%s TX=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  *pause_rx ? "on" : "off",
                  *pause_tx ? "on" : "off"));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_pause_addr_set
 * Purpose:
 *     Configure PAUSE frame source address.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mac  - MAC address used for pause transmission
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_pause_addr_set(int unit, soc_port_t port, sal_mac_addr_t mac)
{
    int32   block_num;
    uint32  data_hi, data_lo;

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    data_hi = (mac[0] << 8) | mac[1];
    data_lo = (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5];

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_TX_SA_LO),
                                 data_hi, data_lo));
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_RX_SA_LO),
                                 data_hi, data_lo));

    DBG_10G_VERB(("xmac_pause_addr_set: unit=%d port=%s MAC=<"
                  "%02x:%02x:%02x:%02x:%02x:%02x>\n",
                  unit, SOC_PORT_NAME(unit, port),
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_pause_addr_get
 * Purpose:
 *     Retrieve PAUSE frame source address.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mac  - (OUT) MAC address used for pause transmission
 * Returns:
 *     SOC_E_XXX
 * NOTE:
 *     We always write the same thing to TX & RX SA
 *     so, we just return the contents on XM_MAC_RX_SA_LO.
 */
STATIC int
_fe2000_xmac_pause_addr_get(int unit, soc_port_t port, sal_mac_addr_t mac)
{
    int32   block_num;
    uint32  data_hi, data_lo;

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_RX_SA_LO),
                                &data_hi, &data_lo));

    mac[0] = (uint8)((data_hi >>  8) & 0xff);
    mac[1] = (uint8)( data_hi        & 0xff);
    mac[2] = (uint8)((data_lo >> 24) & 0xff);
    mac[3] = (uint8)((data_lo >> 16) & 0xff);
    mac[4] = (uint8)((data_lo >>  8) & 0xff);
    mac[5] = (uint8)( data_lo        & 0xff);
        
    DBG_10G_VERB(("xmac_pause_addr_get: unit=%d port=%s MAC=<"
                  "%02x:%02x:%02x:%02x:%02x:%02x>\n",
                  unit, SOC_PORT_NAME(unit, port),
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));

    return SOC_E_NONE;
}
    
/*
 * Function:
 *     xmac_loopback_set
 * Purpose:
 *     Set a XMAC into/out-of loopback mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     lb   - Boolean: true -> loopback mode, false -> normal operation
 * Note:
 *     On bigmac, when setting loopback, we enable the TX/RX function also.
 *     Note that to test the PHY, we use the remote loopback facility.
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_loopback_set(int unit, soc_port_t port, int lb)
{
    int32   block_num;
    uint32  data_hi, data_lo;
    uint32  data_prev;

    DBG_10G_VERB(("xmac_loopback_set: unit=%d port=%s %s loopback\n",
                  unit, SOC_PORT_NAME(unit, port),
                  lb ? "local" : "no"));

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                &data_hi, &data_lo));
    data_prev = data_lo;
    data_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_CONFIG_LO, LOCAL_LOOPBACK_ENABLE,
                                 data_lo, lb ? 1 : 0);
    if (data_lo != data_prev) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_write(unit, block_num,
                                     REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                     data_hi, data_lo));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_loopback_get
 * Purpose:
 *     Get current XMAC loopback mode setting.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     lb   - (OUT) Boolean: true = loopback, false = normal
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_fe2000_xmac_loopback_get(int unit, soc_port_t port, int *lb)
{
    int32   block_num;
    uint32  data_hi, data_lo;
    int     remote, local;

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_CONFIG_LO),
                                &data_hi, &data_lo));

    remote = SAND_HAL_FE2000_GET_FIELD(unit, XM_MAC_CONFIG_LO, REMOTE_LOOPBACK_ENABLE,
                                data_lo);
    local  = SAND_HAL_FE2000_GET_FIELD(unit, XM_MAC_CONFIG_LO, LOCAL_LOOPBACK_ENABLE,
                                data_lo);
    *lb = local | remote;

    DBG_10G_VERB(("xmac_loopback_get: unit=%d port=%s %s loopback\n",
                  unit, SOC_PORT_NAME(unit, port),
                  *lb ? (remote ? "remote" : "local") : "no"));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_interface_set
 * Purpose:
 *     Set a XMAC interface type.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     pif  - one of SOC_PORT_IF_*
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_UNAVAIL - Requested mode not supported
 * Notes:
 *
 */
STATIC int
_fe2000_xmac_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    DBG_10G_VERB(("xmac_interface_set: unit=%d port=%s %s interface\n",
                  unit, SOC_PORT_NAME(unit, port), xmac_port_if_names[pif]));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_interface_get
 * Purpose:
 *     Retrieve XMAC interface type.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     pif  - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_fe2000_xmac_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *pif = SOC_PORT_IF_MII;

    DBG_10G_VERB(("xmac_interface_get: unit=%d port=%s %s interface\n",
                  unit, SOC_PORT_NAME(unit, port), xmac_port_if_names[*pif]));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_ability_get
 * Purpose:
 *     Return the abilities of XMAC.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - (OUT) Supported operating modes as a mask of abilities.
 * Returns:
 *     SOC_E_XXX
 */
STATIC  int
_fe2000_xmac_ability_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *mode = (SOC_PM_10GB_FD | SOC_PM_MII | SOC_PM_XGMII | SOC_PM_LB_MAC |
             SOC_PM_PAUSE | SOC_PM_PAUSE_ASYMM);

    DBG_10G_VERB(("xmac_ability_get: unit=%d port=%s mode=0x%x\n",
                  unit, SOC_PORT_NAME(unit, port), *mode));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_frame_max_set
 * Description:
 *     Set the maximum receive frame size for the port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     size - Maximum frame size in bytes
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_frame_max_set(int unit, soc_port_t port, int size)
{
    int32  block_num;
    uint32 rx_cfg_hi, rx_cfg_lo;
    int    strip_crc;

    DBG_10G_VERB(("xmac_frame_max_set: unit=%d port=%s size=%d\n",
                  unit, SOC_PORT_NAME(unit, port), size));

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_TX_MAX_SIZE_LO),
                                 0, (uint32) size));

    SOC_IF_ERROR_RETURN(soc_sbx_fe2000_xm_read(unit, block_num,
			    REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
			    &rx_cfg_hi, &rx_cfg_lo));
    strip_crc = SAND_HAL_FE2000_GET_FIELD(unit, XM_MAC_RX_CONFIG_LO, STRIP_CRC, rx_cfg_lo);
    if (strip_crc == 1)
      size -= 4;
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_write(unit, block_num,
                                 REG_OFFSET(unit, XM_MAC_RX_MAX_SIZE_LO),
                                 0, (uint32) size));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_frame_max_get
 * Description:
 *     Get the maximum receive frame size for the port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     size - Maximum frame size in bytes
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_frame_max_get(int unit, soc_port_t port, int *size)
{
    int32   block_num;
    uint32  data_hi, data_lo;

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_TX_MAX_SIZE_LO),
                                &data_hi, &data_lo));
    *size = data_lo;

    DBG_10G_VERB(("xmac_frame_max_set: unit=%d port=%s size=%d\n",
                  unit, SOC_PORT_NAME(unit, port), *size));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_ifg_set
 * Description:
 *     Set the new ifg (Inter-frame gap) value.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     speed  - the speed for which the IFG is being set
 *     duplex - the duplex for which the IFG is being set
 *     size   - Maximum frame size in bytes
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     The function makes sure the IFG value makes sense and updates the
 *     IPG register in case the speed/duplex match the current settings.
 */
STATIC int
_fe2000_xmac_ifg_set(int unit, soc_port_t port, int speed,
                     soc_port_duplex_t duplex, int ifg)
{
    int         rv = SOC_E_NONE;
    int         cur_speed;
    int         cur_duplex;
    int         ipg;
    int         real_ifg;
    soc_ipg_t  *si = &SOC_PERSIST(unit)->ipg[port];

    /* Always full duplex */
    if (!duplex) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(_fe2000_xmac_ifg_to_ipg(unit, port, speed,
                                                ifg, &ipg));
    SOC_IF_ERROR_RETURN(_fe2000_xmac_ipg_to_ifg(unit, port, speed,
                                                ipg, &real_ifg));
    switch (speed) {
    case 10000:
    case 12000:
        si->fd_10000 = real_ifg;
        break;
    default:
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(_fe2000_xmac_duplex_get(unit, port, &cur_duplex));
    SOC_IF_ERROR_RETURN(_fe2000_xmac_speed_get(unit, port, &cur_speed));

    if (cur_speed == speed && cur_duplex == duplex) {
        rv = _fe2000_xmac_ipg_update(unit, port);
    }
    
    return rv;
}

/*
 * Function:
 *     xmac_ifg_get
 * Description:
 *     Get the ifg (Inter-frame gap) value.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     speed  - the speed for which the IFG is being set
 *     duplex - the duplex for which the IFG is being set
 *     size   - Maximum frame size in bytes
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     The function makes sure the IFG value makes sense and updates the
 *     IPG register in case the speed/duplex match the current settings.
 */
STATIC int
_fe2000_xmac_ifg_get(int unit, soc_port_t port, int speed,
                     soc_port_duplex_t duplex, int *ifg)
{
    soc_ipg_t  *si = &SOC_PERSIST(unit)->ipg[port];

    /* Always full duplex */
    if (!duplex) {
        return SOC_E_PARAM;
    }

    switch (speed) {
    case 10000:
    case 12000:
        *ifg = si->fd_10000;
        break;
    default:
        return SOC_E_PARAM;
        break;
    }

    return SOC_E_NONE;
}

STATIC int
_fe2000_xmac_port_mode_update(int unit, soc_port_t port, int to_hg_port)
{
    uint32              block_num;
    /* uint32              rval;
    uint64              val64;
    soc_pbmp_t          ctr_pbmp; */
    int                 speed, rv = SOC_E_NONE;
    uint32              rx_cfg_hi, rx_cfg_lo;
    uint32              tx_cfg_hi, tx_cfg_lo;
    uint32              rx_cfg_prev, tx_cfg_prev;

    /* Pause linkscan */
    /* soc_linkscan_pause(unit); */

    /* Pause counter collection */
    /* COUNTER_LOCK(unit); */

    /* basically add port to HG bitmap and remove from XE */
    soc_sbx_xport_type_update(unit, port, to_hg_port);

    /* Now that we've patched, reinit the port */
    speed = 10000;
    if (to_hg_port && soc_feature(unit, soc_feature_xgxs_lcpll) &&
        soc_property_port_get(unit, port, spn_XGXS_LCPLL_12GBPS, 0)) {
        speed = 12000; /* 12 Gbps */
    }

    rv = soc_fe2000_fusioncore_reset(unit, port, speed);

    if (rv >= 0) {
        rv = soc_phyctrl_init(unit, port);
    }

    if (rv >= 0) {
        rv = _fe2000_xmac_init(unit, port);
    }

    if (rv >= 0) {
        rv = _fe2000_xmac_enable_set(unit, port, 0);
    }

    /**
    if (rv >= 0) {
        SOC_PBMP_CLEAR(ctr_pbmp);
        SOC_PBMP_PORT_SET(ctr_pbmp, port);
        COMPILER_64_SET(val64, 0, 0);
        rv = soc_counter_set_by_port(unit, ctr_pbmp, val64);
    }

    COUNTER_UNLOCK(unit);
    soc_linkscan_continue(unit);
    **/

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    /* Read modify write the encapsulation mode */
    SOC_IF_ERROR_RETURN
       (soc_sbx_fe2000_xm_read(unit, block_num,
                               REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                               &rx_cfg_hi, &rx_cfg_lo));
    SOC_IF_ERROR_RETURN
       (soc_sbx_fe2000_xm_read(unit, block_num,
                               REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                               &tx_cfg_hi, &tx_cfg_lo));
    tx_cfg_prev = tx_cfg_lo;
    rx_cfg_prev = rx_cfg_lo;
    tx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_TX_CONFIG_LO, HDR_MODE,
                                   tx_cfg_lo, to_hg_port ? 1 : 0);
    rx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_RX_CONFIG_LO, HDR_MODE,
                                   rx_cfg_lo, to_hg_port ? 1 : 0);

    if (tx_cfg_lo != tx_cfg_prev) {
        SOC_IF_ERROR_RETURN
           (soc_sbx_fe2000_xm_write(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                    tx_cfg_hi, tx_cfg_lo));
    }
    if (rx_cfg_lo != rx_cfg_prev) {
        SOC_IF_ERROR_RETURN
           (soc_sbx_fe2000_xm_write(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                    rx_cfg_hi, rx_cfg_lo));
    }

    return rv;
}

/*
 * Function:
 *    xmac_encap_set
 * Purpose:
 *     Set the XMAC port encapsulation mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - Encap bits (defined above)
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_encap_set(int unit, soc_port_t port, int mode)
{
    int  enable, higig2;
    int  encap = 0; /* IEEE */

    DBG_10G_VERB(("xmac_encap_set: unit=%d port=%s %s encapsulation\n",
                  unit, SOC_PORT_NAME(unit, port), xmac_encap_mode[mode]));

    /* if ((mode != SOC_ENCAP_IEEE) && (mode != SOC_ENCAP_HIGIG)) {
        return SOC_E_PARAM;
    } */

    switch (mode) {
    case SOC_ENCAP_IEEE:
        break;
    case SOC_ENCAP_HIGIG:
        encap = 1;
        break;
    case SOC_ENCAP_B5632:
        encap = 2;
        break;
    case SOC_ENCAP_HIGIG2:
        if (soc_feature(unit, soc_feature_higig2)) {
            higig2 = 1;
            encap = 1;
        } else {
            return SOC_E_PARAM;
        }
        break;
    default:
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(_fe2000_xmac_enable_get(unit, port, &enable));
    if (enable) {
        /* Turn off TX/RX enable */
        SOC_IF_ERROR_RETURN(_fe2000_xmac_enable_set(unit, port, 0));
    }

    /* TODO: HG support to be implemented */
    if ((mode == SOC_ENCAP_HIGIG) && IS_E_PORT(unit, port)) {
        /* XE -> HG */
        if (soc_feature(unit, soc_feature_xport_convertible)) {
            SOC_IF_ERROR_RETURN
                (_fe2000_xmac_port_mode_update(unit, port, TRUE));
        } else {
            return SOC_E_PARAM;
        }
    } else if ((mode == SOC_ENCAP_IEEE) && IS_ST_PORT(unit, port)) {
        /* HG -> XE */
        /* TODO */
        return SOC_E_UNAVAIL;

    } else { /* Changing within type */
        int32   block_num;
        uint32  rx_cfg_hi, rx_cfg_lo;
        uint32  tx_cfg_hi, tx_cfg_lo;
        uint32  rx_cfg_prev, tx_cfg_prev;

        block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

        /* Read modify write the encapsulation mode */
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_read(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                    &rx_cfg_hi, &rx_cfg_lo));
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_read(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                    &tx_cfg_hi, &tx_cfg_lo));
        tx_cfg_prev = tx_cfg_lo;
        rx_cfg_prev = rx_cfg_lo;
        tx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_TX_CONFIG_LO, HDR_MODE,
                                       tx_cfg_lo, encap);
        rx_cfg_lo = SAND_HAL_FE2000_MOD_FIELD(unit, XM_MAC_RX_CONFIG_LO, HDR_MODE,
                                       rx_cfg_lo, encap);

        if (tx_cfg_lo != tx_cfg_prev) {
            SOC_IF_ERROR_RETURN
                (soc_sbx_fe2000_xm_write(unit, block_num,
                                         REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                         tx_cfg_hi, tx_cfg_lo));
        }
        if (rx_cfg_lo != rx_cfg_prev) {
            SOC_IF_ERROR_RETURN
                (soc_sbx_fe2000_xm_write(unit, block_num,
                                         REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                         rx_cfg_hi, rx_cfg_lo));
        }
    }

    if (enable) {
        /* Re-enable transmitter and receiver */
        SOC_IF_ERROR_RETURN(_fe2000_xmac_enable_set(unit, port, 1));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_encap_get
 * Purpose:
 *     Get the XMAC port encapsulation mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - (OUT) Encap bits (defined above)
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_xmac_encap_get(int unit, soc_port_t port, int *mode)
{
    int32   block_num;
    uint32  data_hi, data_lo;

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    /*
     * TX/RX mode should always be programmed to the same
     * value (or all sorts of weird things will happen).
     * So, for purpose of S/W state, we only need to read
     * one or the other and return the state.
     *
     * WARNING: The following assumes BCM_PORT_ENCAP_xxx values equal
     * the hardware register field values.
     */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, block_num,
                                REG_OFFSET(unit, XM_MAC_RX_CONFIG_LO),
                                &data_hi, &data_lo));
    *mode = SAND_HAL_FE2000_GET_FIELD(unit, XM_MAC_RX_CONFIG_LO, HDR_MODE, data_lo);

    DBG_10G_VERB(("xmac_encap_get: unit=%d port=%s %s encapsulation\n",
                  unit, SOC_PORT_NAME(unit, port), xmac_encap_mode[*mode]));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_control_set
 * Purpose:
 *     To configure MAC control properties.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     type - MAC control property to set
 *     int  - New setting for MAC control
 * Returns:
 *     SOC_E_XXX
 */
STATIC int 
_fe2000_xmac_control_set(int unit, soc_port_t port, 
                         soc_mac_control_t type, int value)
{
    int  rv = SOC_E_NONE;

    DBG_10G_VERB(("xmac_control_set: unit=%d port=%s: %d = %d\n",
                  unit, SOC_PORT_NAME(unit, port),
                  type, value));

    if ((type < 0) || (type >= SOC_MAC_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }

    switch(type) {
    case SOC_MAC_CONTROL_FRAME_SPACING_STRETCH:
        rv = _fe2000_xmac_frame_spacing_stretch_set(unit, port, value);
        break;
    default:
        rv = SOC_E_UNAVAIL;
        break;
    }

    return rv;
}

/*
 * Function:
 *     xmac_control_get
 * Purpose:
 *     To get current MAC control setting.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     type - MAC control property to set
 *     int  - (OUT) MAC control setting
 * Returns:
 *     SOC_E_XXX
 */
STATIC int 
_fe2000_xmac_control_get(int unit, soc_port_t port,
                         soc_mac_control_t type, int *value)
{
    int32   block_num;
    uint32  data_hi, data_lo;
    int     rv = SOC_E_NONE;

    if ((type < 0) || (type >= SOC_MAC_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }

    switch(type) {
    case SOC_MAC_CONTROL_FRAME_SPACING_STRETCH:
        block_num = SOC_PORT_BLOCK_NUMBER(unit, port);
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_xm_read(unit, block_num,
                                    REG_OFFSET(unit, XM_MAC_TX_CONFIG_LO),
                                    &data_hi, &data_lo));
        *value = SAND_HAL_FE2000_GET_FIELD(unit, XM_MAC_TX_CONFIG_LO, THROTTLE_DENOM,
                                    data_lo);

        break;
    default:
        rv = SOC_E_UNAVAIL;
        break;
    }

    DBG_10G_VERB(("xmac_control_get: unit=%d port=%s: %d = %d: rv = %d\n",
                   unit, SOC_PORT_NAME(unit, port),
                   type, *value, rv));

    return rv;
}

/*
 * Function:
 *     xmac_speed_get
 * Purpose:
 *     Get XMAC speed.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     speed - (OUT) speed in Mb (2500/3000/10000/12000)
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     Speed for XE can only be 10000
 */
STATIC int
_fe2000_xmac_speed_get(int unit, soc_port_t port, int *speed)
{
    int block_num;

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    block_num = SOC_PORT_BLOCK_NUMBER(unit, port);

    *speed = 10000;

    if (IS_HG_PORT(unit, port)) {
        if (soc_feature(unit, soc_feature_xgxs_lcpll)) {
            uint32         data;
            /*
             * Check for internal/external ref clock.
             * If external clock is used then leave speed at 10G
             * even if this may be incorrect.
             */
            data = SAND_HAL_FE2000_READ_STRIDE(unit, XG, block_num,
                                        XG_XGXS_PLL_CONTROL0);
            
            
            if (SOC_IS_SBX_FE2KXT(unit)) {
                data = SAND_HAL_READ_STRIDE(unit, C2, XG, block_num, XG_XGXS_SPEED);
                if (data & 0x10) {
                    *speed = 12000;
                }else if (data & 0x100) {
                    *speed = 16000;
                }
            }else{
                if (SAND_HAL_GET_FIELD(CA, XG_XGXS_PLL_CONTROL0,
                                          OUT_SEL, data)) {
                    *speed = 12000;
                }
            }
        }
     }

    DBG_10G_VERB(("xmac_speed_get: unit=%d port=%s speed %dMb\n",
                  unit, SOC_PORT_NAME(unit, port), *speed));

    return SOC_E_NONE;
}

/*
 * Function:
 *     xmac_speed_set
 * Purpose:
 *     Set BigMAC in the specified speed.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     speed - 2500, 3000, 10000, 12000 for speed
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     Speed for XE can only be 10000
 */
STATIC int
_fe2000_xmac_speed_set(int unit, soc_port_t port, int speed)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    if (speed != 10000 && speed != 12000) {
        return SOC_E_PARAM;
    }

    if (IS_HG_PORT(unit, port) &&
        soc_feature(unit, soc_feature_xgxs_lcpll)) {
        int enable, cur_speed, cur_max, cur_lb;

        SOC_IF_ERROR_RETURN
            (_fe2000_xmac_speed_get(unit, port, &cur_speed));
        if (speed != cur_speed) {
            switch (speed) {
            case 10000:
            case 12000:
                SOC_IF_ERROR_RETURN
                    (_fe2000_xmac_enable_get(unit, port, &enable));
                SOC_IF_ERROR_RETURN
                    (_fe2000_xmac_frame_max_get(unit, port, &cur_max));
                SOC_IF_ERROR_RETURN
                    (_fe2000_xmac_loopback_get(unit, port, &cur_lb));

                SOC_IF_ERROR_RETURN
                    (_fe2000_xmac_enable_set(unit, port, 0));

                /* Reset with new speed */
                SOC_IF_ERROR_RETURN
                    (soc_fe2000_fusioncore_reset(unit, port, speed));
                SOC_IF_ERROR_RETURN
                    (soc_phyctrl_init(unit, port));
                SOC_IF_ERROR_RETURN
                    (_fe2000_xmac_init(unit, port));
                /* Restore counter DMA settings and port state */
                SOC_IF_ERROR_RETURN
                    (_fe2000_xmac_frame_max_set(unit, port, cur_max));
                SOC_IF_ERROR_RETURN
                    (_fe2000_xmac_loopback_set(unit, port, cur_lb));
                SOC_IF_ERROR_RETURN
                    (_fe2000_xmac_enable_set(unit, port, enable));
                break;
            }
        }
    }

    DBG_10G_VERB(("xmac_speed_set: unit=%d port=%s speed %dMb\n",
                  unit, SOC_PORT_NAME(unit, port), speed));

    return SOC_E_NONE;
}

/* Exported SBX FE2000 MAC driver structure */
mac_driver_t soc_fe2000_xmac = {
    "SBX 10G MAC Driver",
    _fe2000_xmac_init,
    _fe2000_xmac_enable_set,
    _fe2000_xmac_enable_get,
    _fe2000_xmac_duplex_set,
    _fe2000_xmac_duplex_get,
    _fe2000_xmac_speed_set,
    _fe2000_xmac_speed_get,
    _fe2000_xmac_pause_set,
    _fe2000_xmac_pause_get,
    _fe2000_xmac_pause_addr_set,
    _fe2000_xmac_pause_addr_get,
    _fe2000_xmac_loopback_set,
    _fe2000_xmac_loopback_get,
    _fe2000_xmac_interface_set,
    _fe2000_xmac_interface_get,
    _fe2000_xmac_ability_get,
    _fe2000_xmac_frame_max_set,
    _fe2000_xmac_frame_max_get,
    _fe2000_xmac_ifg_set,
    _fe2000_xmac_ifg_get,
    _fe2000_xmac_encap_set,
    _fe2000_xmac_encap_get,
    _fe2000_xmac_control_set,                  /* MAC control set */
    _fe2000_xmac_control_get                   /* MAC control get */
};
