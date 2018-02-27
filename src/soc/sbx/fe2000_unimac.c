/* 
 * $Id: fe2000_unimac.c 1.7 Broadcom SDK $
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
 * File:        fe2000_unimac.c
 * Purpose:     XGS 10/100/1000/2500 Megabit
 *              Media Access Controller Driver (UniMAC)
 *
 * This module is used for:
 *   - GE ports on SBX FE2000 device
 *
 * NOTE:
 * Driver is up-to-date to ESW driver /soc/esw/unimac.c, version 1.36
 * MAC drivers will share the same code when restructure of MAC
 * drivers to remove device dependency is complete.
 */

#include <soc/error.h>
#include <soc/debug.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>

#define JUMBO_MAXSZ  0x3fe8

#define SOC_UNIMAC_SPEED_10     0x0
#define SOC_UNIMAC_SPEED_100    0x1
#define SOC_UNIMAC_SPEED_1000   0x2
#define SOC_UNIMAC_SPEED_2500   0x3

/* To check if Unimac is in auto_cfg mode or not */ 
STATIC int fe2000_auto_cfg[SOC_MAX_NUM_DEVICES][SOC_MAX_NUM_PORTS]; 

#define REG_OFFSET(_unit_, _reg_)    \
        (SAND_HAL_FE2000_REG_OFFSET(_unit_, _reg_))

/* Forward declarations */
STATIC int _fe2000_unimac_pause_set(int unit, soc_port_t port,
                                    int pause_tx, int pause_rx);
STATIC int _fe2000_unimac_pause_get(int unit, soc_port_t port,
                                    int *pause_tx, int *pause_rx);
STATIC int _fe2000_unimac_duplex_set(int unit, soc_port_t port, int duplex);
STATIC int _fe2000_unimac_duplex_get(int unit, soc_port_t port, int *duplex);
STATIC int _fe2000_unimac_speed_get(int unit, soc_port_t port, int *speed);
STATIC int _fe2000_unimac_frame_max_set(int unit, soc_port_t port, int size);


    
STATIC int
_fe2000_unimac_sw_reset(int unit, soc_port_t port, int reset_assert)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    int     reset_sleep_usec;

    if (SAL_BOOT_QUICKTURN) {
        reset_sleep_usec = 50000;
    } else {
        /* Minimum of 5 clocks period with the slowest clock is required
         * between each reset step.
         *   10Mbps (2.5MHz) = 2000ns
         *  100Mbps  (25MHz) = 200ns
         * 1000Mbps (125MHz) = 40ns
         * 2500Mbps (133MHz) = 37.5ns
         */
        reset_sleep_usec = 2;  /* choose max delay */
    }

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, SOFT_RESET, data,
                              reset_assert ? 0x1 : 0x0);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_CONFIG), data));

    sal_usleep(reset_sleep_usec);

    return SOC_E_NONE;
}

/*
 * Function:
 *     _fe2000_unimac_drain_cells
 * Purpose:
 *     Drop all packets in TX fifo for a specified port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_unimac_drain_cells(int unit, soc_port_t port)
{
    int32   block_num;
    int32   block_port;
    uint32  tx_debug;
    int     pause_tx = 0, pause_rx = 0;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    /* Disable pause function */
    SOC_IF_ERROR_RETURN
        (_fe2000_unimac_pause_get(unit, port, &pause_tx, &pause_rx));
    SOC_IF_ERROR_RETURN
        (_fe2000_unimac_pause_set(unit, port, 0, 0));

    /* Drop out all packets in TX FIFO without egressing any packets */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_TX_DEBUG), &tx_debug));
    tx_debug = SAND_HAL_FE2000_MOD_FIELD(unit, AM_TX_DEBUG, FLUSH, tx_debug, 0x1);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_TX_DEBUG), tx_debug));

    /* Notify PHY driver */
    SOC_IF_ERROR_RETURN
        (soc_phyctrl_notify(unit, port, phyEventStop, PHY_STOP_DRAIN));

    /*
     * Disable egress metering 
     * In XGS, this is in the MMU block.  For SBX, n/a?
     */

    /* Notify PHY driver */
    SOC_IF_ERROR_RETURN
        (soc_phyctrl_notify(unit, port, phyEventResume, PHY_STOP_DRAIN));

    /* Soft-reset is recommended here. 
     * SOC_IF_ERROR_RETURN
     *     (mac_uni_sw_reset(unit, port, TRUE));
     * SOC_IF_ERROR_RETURN
     *     (mac_uni_sw_reset(unit, port, FALSE));
     */
  
    /* Bring the TxFifo out of flush */
    tx_debug = SAND_HAL_FE2000_MOD_FIELD(unit, AM_TX_DEBUG, FLUSH, tx_debug, 0x0);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_TX_DEBUG), tx_debug));

    /* Restore original pause configuration */
    SOC_IF_ERROR_RETURN
        (_fe2000_unimac_pause_set(unit, port, pause_tx, pause_rx));

    return SOC_E_NONE;
}


/*
 * Function:
 *     _fe2000_unimac_ifg_to_ipg
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
_fe2000_unimac_ifg_to_ipg(int unit, soc_port_t port, int speed, int duplex,
                          int ifg, int *ipg)
{
    ifg += 7;
    *ipg = ifg / 8;

    if (*ipg < 8) {
        *ipg = 8;
    } else if (*ipg > 26) {
        *ipg = 26;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _fe2000_unimac_ipg_to_ifg
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
_fe2000_unimac_ipg_to_ifg(int unit, soc_port_t port, int speed, int duplex,
                          int ipg, int *ifg)
{
    /*
     * Now we need to convert the value according to various chips'
     * peculiarities (there are none as of now)
     */
    *ifg = ipg * 8;

    return SOC_E_NONE;
}

/*
 * Function:
 *     _fe2000_unimac_ipg_update
 * Purpose:
 *     Set the IPG appropriate for current duplex.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 * Notes:
 *     The current duplex is read from the hardware registers.
 */
STATIC int
_fe2000_unimac_ipg_update(int unit, int port)
{
    int32      block_num;
    int32      block_port;
    uint32     data;
    int        fd, speed, ipg, ifg;
    soc_ipg_t  *si = &SOC_PERSIST(unit)->ipg[port];


    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN(_fe2000_unimac_duplex_get(unit, port, &fd));
    SOC_IF_ERROR_RETURN(_fe2000_unimac_speed_get(unit, port, &speed));

    if (fd) {
        switch (speed) {
        case 10:
            ifg = si->fd_10;
            break;
        case 100:
            ifg = si->fd_100;
            break;
        case 1000:
            ifg = si->fd_1000;
            break;
        case 2500:
            ifg = si->fd_2500;
            break;
        default:
            return SOC_E_INTERNAL;
            break;
        }
    } else {
        switch (speed) {
        case 10:
            ifg = si->hd_10;
            break;
        case 100:
            ifg = si->hd_100;
            break;
        case 1000:
            ifg = si->hd_1000;
            break;
        case 2500:
            ifg = si->hd_2500;
            break;
        default:
            return SOC_E_INTERNAL;
            break;
        }
    }

    /*
     * Convert the ifg value from bit-times into IPG register-specific value
     */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_ifg_to_ipg(unit, port, speed, fd,
                                                  ifg, &ipg));

    /*
     * Program the appropriate register
     */
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_TX_IPG_LENGTH), &data));
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_TX_IPG_LENGTH, BYTE_TIMES, data, ipg);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_TX_IPG_LENGTH), data));

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_init
 * Purpose:
 *     Initialize MAC into a known good state.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     The initialization speed/duplex is arbitrary and must be
 *     updated by linkscan before enabling the MAC.
 */
STATIC int
_fe2000_unimac_init(int unit, soc_port_t port)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  data_prev;
    uint32  speed;
    int     frame_max;
    int     ignore_pause;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    fe2000_auto_cfg[unit][port] = PHY_SGMII_AUTONEG_MODE(unit, port);

    frame_max = IS_ST_PORT(unit, port) ? JUMBO_MAXSZ : 1518;

    _fe2000_unimac_frame_max_set(unit, port, frame_max);

    /* First put the MAC in reset and sleep */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, TRUE));

    /* Do the initialization */

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));
    data_prev = data;
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, TX_ENABLE, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, RX_ENABLE, data, 0x0);
    speed = IS_FE_PORT(unit, port) ?
        SOC_UNIMAC_SPEED_100 : SOC_UNIMAC_SPEED_1000;
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, MAC_SPEED, data, speed);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, PROMISCUOUS_ENABLE, data, 0x1);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, PAD_TX_FRAMES, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, FORWARD_CRC, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, FORWARD_PAUSE_FRAMES, data, 0);

    /* Ignore pause if using as stack port */
    ignore_pause = IS_ST_PORT(unit, port) ? 0x1 : 0x0;
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, IGNORE_PAUSE, data,
                              ignore_pause);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, IGNORE_TX_PAUSE, data,
                              ignore_pause);

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, TX_OVERWITE_SA, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, HALF_DUPLEX, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, LOOPBACK_ENABLE, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, LENGTH_CHECK_DISABLE, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, LINE_LOOPBACK_ENABLE, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, DISCARD_ERRORED_FRAMES, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, ENABLE_EFM_PREAMBLE, data, 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, CONTROL_FRAME_ENABLE, data, 0x1);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, ENABLE_EXTERNAL_CONFIG, data,
                              (fe2000_auto_cfg[unit][port]) ? 0x1 : 0x0);
    if (data != data_prev) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                     REG_OFFSET(unit, AM_CONFIG), data));
    }

    /* Bring the MAC out of reset */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, FALSE));

    /* Pulse the Serdes AN if using auto_cfg mode */
    if (fe2000_auto_cfg[unit][port]) {
        SOC_IF_ERROR_RETURN(soc_phyctrl_notify(unit, port, phyEventAutoneg, 0));
        SOC_IF_ERROR_RETURN(soc_phyctrl_notify(unit, port, phyEventAutoneg, 1));
    }

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_RX_PAUSE_QUANTA), 0xffff));
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_TX_IPG_LENGTH), 12));

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_enable_set
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
_fe2000_unimac_enable_set(int unit, soc_port_t port, int enable)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  data_prev;
    pbmp_t  mask;


    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));
    data_prev = data;
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, TX_ENABLE, data,
                              enable ? 0x1 : 0x0);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, RX_ENABLE, data,
                              enable ? 0x1 : 0x0);

    if (data == data_prev) {
        return SOC_E_NONE;
    }

    /* First put the MAC in reset */ 
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, 1));

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, SOFT_RESET, data, 0x1);
    if (enable) {
        /* First de-assert RX_ENA and TX_ENA while in reset */
        data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, TX_ENABLE, data, 0x0);
        data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, RX_ENABLE, data, 0x0);
    }
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_CONFIG), data));

    /* Bring the MAC out of reset */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, 0));
    if (enable) {
        /* Now assert RX_ENA and TX_ENA when out of reset */
        sal_usleep(2);
        data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, SOFT_RESET, data, 0x0);
        data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, TX_ENABLE, data, 0x1);
        data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, RX_ENABLE, data, 0x1);
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                     REG_OFFSET(unit, AM_CONFIG), data));
    }
    
    /*
     * Use EPC_LINK to control other ports sending to this port.  This
     * blocking is independent and in addition to what Linkscan does.
     *
     * Single-step mode is used to stop traffic from going from the
     * egress to the MAC, except on Strata GE where drain is used.
     */

    if (enable) {
        soc_link_mask2_get(unit, &mask);
        SOC_PBMP_PORT_ADD(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_notify(unit, port,
                                phyEventResume, PHY_STOP_MAC_DIS));
    } else {
        soc_link_mask2_get(unit, &mask);
        SOC_PBMP_PORT_REMOVE(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));
        SOC_IF_ERROR_RETURN(_fe2000_unimac_drain_cells(unit, port));
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_notify(unit, port,
                                phyEventStop, PHY_STOP_MAC_DIS));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_enable_get
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
_fe2000_unimac_enable_get(int unit, soc_port_t port, int *enable)
{
    int32   block_num;
    int32   block_port;
    uint32  data;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));

    *enable = SAND_HAL_FE2000_GET_FIELD(unit, AM_CONFIG, TX_ENABLE, data);

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_duplex_set
 * Purpose:
 *     Set MAC in the specified duplex mode.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     duplex - Boolean: true --> full duplex, false --> half duplex
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     Programs an IFG time appropriate to speed and duplex.
 */
STATIC int
_fe2000_unimac_duplex_set(int unit, soc_port_t port, int duplex)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  data_prev;
    int     speed;

    if (fe2000_auto_cfg[unit][port]) {
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN
        (_fe2000_unimac_speed_get(unit, port, &speed)); 

    if ((1000 == speed) || (2500 == speed)) {
        if (!duplex) {
            /* Half duplex is not support for 1000Mbps and 2500Mbps */
            /* Duplex bit is ignored at 1000Mbps and 2500Mbps */
            return SOC_E_NONE;
        }
    }

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));
    data_prev = data;
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, HALF_DUPLEX, data,
                              duplex ? 0x0 : 0x1);
    if (data == data_prev) {
        return SOC_E_NONE;
    }        

    /* First put the MAC in reset */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, 1));

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, SOFT_RESET, data, 0x1);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_CONFIG), data));

    /* Set IPG to match new duplex */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_ipg_update(unit, port));

    /*
     * Notify internal PHY driver of duplex change in case it is being
     * used as pass-through to an external PHY.
     */
    SOC_IF_ERROR_RETURN
        (soc_phyctrl_notify(unit, port, phyEventDuplex, duplex));

    /* Bring the MAC out of reset */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, 0));

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_duplex_get
 * Purpose:
 *     Get MAC duplex mode.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     duplex - (OUT) Boolean: true --> full duplex, false --> half duplex
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_unimac_duplex_get(int unit, soc_port_t port, int *duplex)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    int     speed;

    SOC_IF_ERROR_RETURN
        (_fe2000_unimac_speed_get(unit, port, &speed));

    if ((1000 == speed) || (2500 == speed)) {
        *duplex = TRUE;
    } else {
        block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
        block_port = SOC_PORT_BLOCK_INDEX(unit, port);

        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                    REG_OFFSET(unit, AM_CONFIG), &data));

        *duplex = (SAND_HAL_FE2000_GET_FIELD(unit, AM_CONFIG, HALF_DUPLEX, data)
                   ) ? FALSE : TRUE ;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_speed_set
 * Purpose:
 *     Set BigMAC in the specified speed.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     speed - 10,100,1000, 2500 for speed
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     Programs an IFG time appropriate to speed and duplex.
 */
STATIC int
_fe2000_unimac_speed_set(int unit, soc_port_t port, int speed)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  data_prev;
    int     speed_select;

    if (fe2000_auto_cfg[unit][port]) {
        return SOC_E_NONE;
    }

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));
    data_prev = data;

    switch (speed) {
    case 10:
        speed_select = SOC_UNIMAC_SPEED_10;
        break;
    case 100:
        speed_select = SOC_UNIMAC_SPEED_100;
        break;
    case 1000:
        speed_select = SOC_UNIMAC_SPEED_1000;
        break;
    case 2500:
        speed_select = SOC_UNIMAC_SPEED_2500;
        break;
    case 0:
        return SOC_E_NONE;              /* Support NULL PHY */            
    default:
        return SOC_E_CONFIG;
    }

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, MAC_SPEED, data, speed_select);
    if (data == data_prev) {
        return SOC_E_NONE;
    }

    /* First reset the MAC */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, 1));

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, SOFT_RESET, data, 0x1);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_CONFIG), data));

    /*
     * Notify internal PHY driver of speed change in case it is being
     * used as pass-through to an external PHY.
     */
    SOC_IF_ERROR_RETURN
        (soc_phyctrl_notify(unit, port, phyEventSpeed, speed));

    /* Set IPG to match new speed */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_ipg_update(unit, port));

    /* Bring the MAC out of reset */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, 0));

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_speed_get
 * Purpose:
 *     Get MAC speed.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     speed - (OUT) speed in Mb (10/100/1000)
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_unimac_speed_get(int unit, soc_port_t port, int *speed)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  uni_speed;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));

    uni_speed = SAND_HAL_FE2000_GET_FIELD(unit, AM_CONFIG, MAC_SPEED, data);

    switch(uni_speed) {
    case SOC_UNIMAC_SPEED_10:
        *speed = 10;
        break;
    case SOC_UNIMAC_SPEED_100:
        *speed = 100;
        break;
    case SOC_UNIMAC_SPEED_1000:
        *speed = 1000;
        break;
    case SOC_UNIMAC_SPEED_2500:
        *speed = 2500;
        break;
    default:
        return SOC_E_INTERNAL;
        break;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_pause_set
 * Purpose:
 *     Configure MAC to transmit/receive pause frames.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     pause_tx - Boolean: transmit pause or -1 (don't change)
 *     pause_rx - Boolean: receive pause or -1 (don't change)
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_unimac_pause_set(int unit, soc_port_t port, int pause_tx, int pause_rx)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  data_prev;

    if (fe2000_auto_cfg[unit][port]) {
        return SOC_E_NONE;
    }

    if (pause_tx < 0 && pause_rx < 0) {
        return SOC_E_NONE;
    }

    if (IS_ST_PORT(unit, port)) {
        if ((pause_tx == 0) && (pause_rx == 0)) {
            return SOC_E_NONE;
        } else {
            /* Flow control cannot be enabled in HiGL mode */
            return SOC_E_CONFIG;
        }
    }

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));
    data_prev = data;

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, IGNORE_PAUSE, data,
                              pause_rx ? 0x0 : 0x1);
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, IGNORE_TX_PAUSE, data,
                              pause_tx ? 0x0 : 0x1);

    if (data == data_prev) {
        return SOC_E_NONE;
    }
    
    /* First reset the MAC */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, 1));

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, SOFT_RESET, data, 0x1);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_CONFIG), data));

    /* Bring the MAC out of reset */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_sw_reset(unit, port, 0));

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_pause_get
 * Purpose:
 *     Return the pause ability of MAC.
 * Parameters:
 *     unit      - Device number
 *     port      - Device port number
 *     pause_tx  - (OUT) Boolean: transmit pause
 *     pause_rx  - (OUT) Boolean: receive pause
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_fe2000_unimac_pause_get(int unit, soc_port_t port,
                         int *pause_tx, int *pause_rx)
{
    int32   block_num;
    int32   block_port;
    uint32  data;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));

    *pause_rx = (SAND_HAL_FE2000_GET_FIELD(unit, AM_CONFIG, IGNORE_PAUSE, data)
                 ) ? FALSE : TRUE;
    *pause_tx = (SAND_HAL_FE2000_GET_FIELD(unit, AM_CONFIG, IGNORE_TX_PAUSE, data)
                 ) ? FALSE : TRUE;

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_pause_addr_set
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
_fe2000_unimac_pause_addr_set(int unit, soc_port_t port, sal_mac_addr_t mac)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  mac_0;
    uint32  mac_1;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    mac_0 = mac[0] << 24 | mac[1] << 16 | mac[2] << 8 | mac[3] << 0;
    mac_1 = mac[4] << 8  | mac[5];

    data = SAND_HAL_FE2000_SET_FIELD(unit, AM_MAC_ADDRESS0, ADDR0_31, mac_0);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_MAC_ADDRESS0), data));

    data = SAND_HAL_FE2000_SET_FIELD(unit, AM_MAC_ADDRESS1, MAC_ADDR32_47, mac_1);
    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                 REG_OFFSET(unit, AM_MAC_ADDRESS1), data));

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_pause_addr_get
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
_fe2000_unimac_pause_addr_get(int unit, soc_port_t port, sal_mac_addr_t mac)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  mac_0;
    uint32  mac_1;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_MAC_ADDRESS0), &data));
    mac_0 = SAND_HAL_FE2000_GET_FIELD(unit, AM_MAC_ADDRESS0, ADDR0_31, data);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_MAC_ADDRESS1), &data));
    mac_1 = SAND_HAL_FE2000_GET_FIELD(unit, AM_MAC_ADDRESS1, MAC_ADDR32_47, data);

    mac[0] = (uint8)(mac_0 >> 24);
    mac[1] = (uint8)(mac_0 >> 16);
    mac[2] = (uint8)(mac_0 >> 8);
    mac[3] = (uint8)(mac_0 >> 0);
    mac[4] = (uint8)(mac_1 >> 8);
    mac[5] = (uint8)(mac_1 >> 0);

    return SOC_E_NONE;
}
    
/*
 * Function:
 *     unimac_loopback_set
 * Purpose:
 *     Set a MAC into/out-of loopback mode.
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
_fe2000_unimac_loopback_set(int unit, soc_port_t port, int lb)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  data_prev;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));
    data_prev = data;

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_CONFIG, LOOPBACK_ENABLE, data,
                              lb ? 0x1 : 0x0);

    if (data != data_prev) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                     REG_OFFSET(unit, AM_CONFIG), data));
    }
    
    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_loopback_get
 * Purpose:
 *     Get current MAC loopback mode setting.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     lb   - (OUT) Boolean: true = loopback, false = normal
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_fe2000_unimac_loopback_get(int unit, soc_port_t port, int *lb)
{
    int32   block_num;
    int32   block_port;
    uint32  data;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_CONFIG), &data));

    *lb = SAND_HAL_FE2000_GET_FIELD(unit, AM_CONFIG, LOOPBACK_ENABLE, data);

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_interface_set
 * Purpose:
 *     Set a MAC interface type.
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
_fe2000_unimac_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    switch (pif) {
    case SOC_PORT_IF_MII:
    case SOC_PORT_IF_GMII:
    case SOC_PORT_IF_SGMII:
        return SOC_E_NONE;
        break;
    default:
        return SOC_E_UNAVAIL;
        break;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_interface_get
 * Purpose:
 *     Retrieve MAC interface type.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     pif  - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *     SOC_E_NONE
 */
STATIC int
_fe2000_unimac_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *pif = SOC_PORT_IF_GMII;

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_ability_get
 * Purpose:
 *     Return the abilities of MAC.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - (OUT) Mask of MAC abilities returned
 * Returns:
 *     SOC_E_XXX
 */
STATIC  int
_fe2000_unimac_ability_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *mode = (SOC_PM_10MB | SOC_PM_100MB | SOC_PM_1000MB_FD | SOC_PM_2500MB_FD |
             SOC_PM_MII | SOC_PM_GMII | SOC_PM_LB_MAC | SOC_PM_PAUSE);

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_frame_max_set
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
_fe2000_unimac_frame_max_set(int unit, soc_port_t port, int size)
{
    int32   block_num;
    int32   block_port;
    uint32  data;
    uint32  data_prev;

    if (IS_ST_PORT(unit, port)) {
        size += 16;  /* Account for 16 bytes of Higig2 header */
    }

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_RX_MAX_PKT_LENGTH), &data));
    data_prev = data;

    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_RX_MAX_PKT_LENGTH, LENGTH, data, size);

    if (data != data_prev) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                     REG_OFFSET(unit, AM_RX_MAX_PKT_LENGTH), data));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_frame_max_get
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
_fe2000_unimac_frame_max_get(int unit, soc_port_t port, int *size)
{
    int32   block_num;
    int32   block_port;
    uint32  data;

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_RX_MAX_PKT_LENGTH), &data));

    *size = SAND_HAL_FE2000_GET_FIELD(unit, AM_RX_MAX_PKT_LENGTH, LENGTH, data);

    if (IS_ST_PORT(unit, port)) {
        *size -= 16;  /* Account for 16 bytes of Higig2 header */
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_ifg_set
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
_fe2000_unimac_ifg_set(int unit, soc_port_t port, int speed,
                     soc_port_duplex_t duplex, int ifg)
{
    int32      block_num;
    int32      block_port;
    uint32     data;
    uint32     data_prev;
    int        ipg, real_ifg;
    soc_ipg_t  *si = &SOC_PERSIST(unit)->ipg[port];

    block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_port = SOC_PORT_BLOCK_INDEX(unit, port);

    /* Get IPG, check range of IFG */
    SOC_IF_ERROR_RETURN(_fe2000_unimac_ifg_to_ipg(unit, port, speed, duplex,
                                                  ifg, &ipg));
    SOC_IF_ERROR_RETURN(_fe2000_unimac_ipg_to_ifg(unit, port, speed, duplex,
                                                  ipg, &real_ifg));

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_am_read(unit, block_num, block_port,
                                REG_OFFSET(unit, AM_TX_IPG_LENGTH), &data));
    data_prev = data;
    data = SAND_HAL_FE2000_MOD_FIELD(unit, AM_TX_IPG_LENGTH, BYTE_TIMES, data, ipg);
    if (data != data_prev) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_am_write(unit, block_num, block_port,
                                     REG_OFFSET(unit, AM_TX_IPG_LENGTH), data));
    }

    if (duplex) {
        switch (speed) {
        case 10:
            si->fd_10 = real_ifg;
            break;
        case 100:
            si->fd_100 = real_ifg;
            break;
        case 1000:
            si->fd_1000 = real_ifg;
            break;
        case 2500:
            si->fd_2500 = real_ifg;
            break;
        default:
            return SOC_E_PARAM;
            break;
        }
    } else {
        switch (speed) {
        case 10:
            si->hd_10 = real_ifg;
            break;
        case 100:
            si->hd_100 = real_ifg;
            break;
        case 1000:
            si->hd_1000 = real_ifg;
            break;
        case 2500:
            si->hd_2500 = real_ifg;
            break;
        default:
            return SOC_E_PARAM;
            break;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_ifg_get
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
_fe2000_unimac_ifg_get(int unit, soc_port_t port, int speed,
                        soc_port_duplex_t duplex, int *ifg)
{
    soc_ipg_t  *si = &SOC_PERSIST(unit)->ipg[port];

    if (duplex) {
        switch (speed) {
        case 10:
            *ifg = si->fd_10;
            break;
        case 100:
            *ifg = si->fd_100;
            break;
        case 1000:
            *ifg = si->fd_1000;
            break;
        case 2500:
            *ifg = si->fd_2500;
            break;
        default:
            return SOC_E_PARAM;
            break;
        }
    } else {
        switch (speed) {
        case 10:
            *ifg = si->hd_10;
            break;
        case 100:
            *ifg = si->hd_100;
            break;
        case 1000:
            *ifg = si->hd_1000;
            break;
        case 2500:
            *ifg = si->hd_2500;
            break;
        default:
            return SOC_E_PARAM;
            break;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     unimac_encap_set
 * Purpose:
 *     Set the MAC port encapsulation mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - Encap bits (defined above)
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_unimac_encap_set(int unit, soc_port_t port, int mode)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    if (mode == SOC_ENCAP_IEEE) {
        return SOC_E_NONE;
    }

    return SOC_E_PARAM;
}

/*
 * Function:
 *     unimac_encap_get
 * Purpose:
 *     Get the MAC port encapsulation mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - (OUT) Encap bits (defined above)
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_fe2000_unimac_encap_get(int unit, soc_port_t port, int *mode)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *mode = SOC_ENCAP_IEEE;

    return SOC_E_NONE;
}

/* Exported SBX FE2000 UNIMAC driver structure */
mac_driver_t soc_fe2000_unimac = {
    "SBX 10G MAC Driver",
    _fe2000_unimac_init,
    _fe2000_unimac_enable_set,
    _fe2000_unimac_enable_get,
    _fe2000_unimac_duplex_set,
    _fe2000_unimac_duplex_get,
    _fe2000_unimac_speed_set,
    _fe2000_unimac_speed_get,
    _fe2000_unimac_pause_set,
    _fe2000_unimac_pause_get,
    _fe2000_unimac_pause_addr_set,
    _fe2000_unimac_pause_addr_get,
    _fe2000_unimac_loopback_set,
    _fe2000_unimac_loopback_get,
    _fe2000_unimac_interface_set,
    _fe2000_unimac_interface_get,
    _fe2000_unimac_ability_get,
    _fe2000_unimac_frame_max_set,
    _fe2000_unimac_frame_max_get,
    _fe2000_unimac_ifg_set,
    _fe2000_unimac_ifg_get,
    _fe2000_unimac_encap_set,
    _fe2000_unimac_encap_get,
    NULL,                        /* mac_control_set */
    NULL,                        /* mac_control_get */
};
