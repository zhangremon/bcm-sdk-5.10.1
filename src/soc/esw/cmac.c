/*
 * $Id: cmac.c 1.5.16.2 Broadcom SDK $
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
 * CMAC driver - derived from XMAC driver
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>
#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/portmode.h>
#include <soc/ll.h>
#include <soc/counter.h>
#include <soc/phyctrl.h>
#include <soc/debug.h>

#ifdef BCM_CMAC_SUPPORT

#define DBG_100G_OUT(flags, stuff) SOC_DEBUG(SOC_DBG_100G | flags, stuff)
#define DBG_100G_VERB(stuff) DBG_100G_OUT(SOC_DBG_VERBOSE, stuff)
#define DBG_100G_WARN(stuff) DBG_100G_OUT(SOC_DBG_WARN, stuff)

/*
 * CMAC Register field definitions.
 */

/* Transmit CRC Modes (Receive has bit field definitions in register) */
#define CMAC_CRC_APPEND          0x00   /* Append CRC (Default) */
#define CMAC_CRC_KEEP            0x01   /* CRC Assumed correct */
#define CMAC_CRC_REGEN           0x02   /* Replace CRC with a new one */
#define CMAC_CRC_RSVP            0x03   /* Reserved (does Replace) */

#define JUMBO_MAXSZ              0x3fe8 /* Max legal value (per regsfile) */

mac_driver_t soc_mac_c;

#ifdef BROADCOM_DEBUG
static char *mac_c_encap_mode[] = SOC_ENCAP_MODE_NAMES_INITIALIZER;
static char *mac_c_port_if_names[] = SOC_PORT_IF_NAMES_INITIALIZER;
#endif /* BROADCOM_DEBUG */

/* speed to SOC_PA_SPEED flags mapping table, need to be in ascending order */
struct {
    int speed;
    int pa_flag;
} _mac_c_hg_speed_list[] = {
    { 100000, SOC_PA_SPEED_100GB },
    { 120000, SOC_PA_SPEED_120GB },
};

void
_mac_c_speed_to_pa_flag(int unit, soc_port_t port, int speed, uint32 *pa_flag)
{
    int idx;

    *pa_flag = 0;
    for (idx = 0; idx < sizeof(_mac_c_hg_speed_list) /
             sizeof(_mac_c_hg_speed_list[0]); idx++) {
        if (speed == _mac_c_hg_speed_list[idx].speed) {
            *pa_flag = _mac_c_hg_speed_list[idx].pa_flag;
            break;
        }
    }
}

/*
 * Function:
 *      mac_c_init
 * Purpose:
 *      Initialize Cmac into a known good state.
 * Parameters:
 *      unit - XGS unit #.
 *      port - Port number on unit.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *
 */
STATIC int
mac_c_init(int unit, soc_port_t port)
{
    soc_info_t *si;
    uint64 mac_ctrl, rx_ctrl, tx_ctrl, rval;

    DBG_100G_VERB(("mac_c_init: unit %d port %s\n",
                  unit, SOC_PORT_NAME(unit, port)));

    si = &SOC_INFO(unit);

    /* Disable Tx/Rx, assume that MAC is stable (or out of reset) */
    SOC_IF_ERROR_RETURN(READ_CMAC_CTRLr(unit, port, &mac_ctrl));
    soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, SOFT_RESETf, 0);
    soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, RX_ENf, 0);
    soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, TX_ENf, 0);
    if (si->port_speed_max[port] == 40000) {
        soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, XLGMII_ALIGN_ENBf,
                              1);
    }
    SOC_IF_ERROR_RETURN(WRITE_CMAC_CTRLr(unit, port, mac_ctrl));

    SOC_IF_ERROR_RETURN(READ_CMAC_RX_CTRLr(unit, port, &rx_ctrl));
    soc_reg64_field32_set(unit, CMAC_RX_CTRLr, &rx_ctrl, STRIP_CRCf, 0);
    SOC_IF_ERROR_RETURN(WRITE_CMAC_RX_CTRLr(unit, port, rx_ctrl));

    SOC_IF_ERROR_RETURN(READ_CMAC_TX_CTRLr(unit, port, &tx_ctrl));
    soc_reg64_field32_set(unit, CMAC_TX_CTRLr, &tx_ctrl, AVERAGE_IPGf,
                          (SOC_PERSIST(unit)->ipg[port].fd_10000 / 8) & 0x1f);
    soc_reg64_field32_set(unit, CMAC_TX_CTRLr, &tx_ctrl, CRC_MODEf, 2);
    SOC_IF_ERROR_RETURN(WRITE_CMAC_TX_CTRLr(unit, port, tx_ctrl));

    if (IS_ST_PORT(unit, port)) {
        soc_mac_c.md_pause_set(unit, port, FALSE, FALSE);
    } else {
        soc_mac_c.md_pause_set(unit, port, TRUE, TRUE);
    }

    if (soc_property_port_get(unit, port, spn_PHY_WAN_MODE, FALSE)) {
        /* Max speed for WAN mode is 9.294Gbps.
         * This setting gives 10Gbps * (13/14) or 9.286 Gbps */
        SOC_IF_ERROR_RETURN
            (soc_mac_c.md_control_set(unit, port,
                                      SOC_MAC_CONTROL_FRAME_SPACING_STRETCH,
                                      13));
    }

    /* Set jumbo max size (8000 byte payload) */
    COMPILER_64_ZERO(rval);
    soc_reg64_field32_set(unit, CMAC_RX_MAX_SIZEr, &rval, RX_MAX_SIZEf,
                          JUMBO_MAXSZ);
    SOC_IF_ERROR_RETURN(WRITE_CMAC_RX_MAX_SIZEr(unit, port, rval));

    /* Setup header mode, check for property for bcm5632 mode */
    COMPILER_64_ZERO(rval);
    if (!IS_XE_PORT(unit, port)) {
        soc_reg64_field32_set(unit, CMAC_MODEr, &rval, HDR_MODEf, 1);
    }
    SOC_IF_ERROR_RETURN(WRITE_CMAC_MODEr(unit, port, rval));

    /* Disable loopback and bring CMAC out of reset */
    soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, CORE_REMOTE_LPBKf, 0);
    soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, LINE_REMOTE_LPBKf, 0);
    soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, CORE_LOCAL_LPBKf, 0);
    soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, LINE_LOCAL_LPBKf, 0);
    soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, RX_ENf, 1);
    soc_reg64_field32_set(unit, CMAC_CTRLr, &mac_ctrl, TX_ENf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CMAC_CTRLr(unit, port, mac_ctrl));

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_enable_set
 * Purpose:
 *      Enable or disable MAC
 * Parameters:
 *      unit - XGS unit #.
 *      port - Port number on unit.
 *      enable - TRUE to enable, FALSE to disable
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_enable_set(int unit, soc_port_t port, int enable)
{
    uint64 ctrl, octrl;
    pbmp_t mask;

    DBG_100G_VERB(("mac_c_enable_set: unit %d port %s enable=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  enable ? "True" : "False"));

    SOC_IF_ERROR_RETURN(READ_CMAC_CTRLr(unit, port, &ctrl));
    octrl = ctrl;
    /* Don't disable TX since it stops egress and hangs if CPU sends */
    soc_reg64_field32_set(unit, CMAC_CTRLr, &ctrl, TX_ENf, 1);
    soc_reg64_field32_set(unit, CMAC_CTRLr, &ctrl, RX_ENf, enable ? 1 : 0);
    if (COMPILER_64_NE(ctrl, octrl)) {
        SOC_IF_ERROR_RETURN(WRITE_CMAC_CTRLr(unit, port, ctrl));
    }

    if (enable) {
        soc_link_mask2_get(unit, &mask);
        SOC_PBMP_PORT_ADD(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));
    } else {
        soc_link_mask2_get(unit, &mask);
        SOC_PBMP_PORT_REMOVE(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_enable_get
 * Purpose:
 *      Get MAC enable state
 * Parameters:
 *      unit - XGS unit #.
 *      port - Port number on unit.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_enable_get(int unit, soc_port_t port, int *enable)
{
    uint64 ctrl;

    SOC_IF_ERROR_RETURN(READ_CMAC_CTRLr(unit, port, &ctrl));

    *enable = soc_reg64_field32_get(unit, CMAC_CTRLr, ctrl, RX_ENf);

    DBG_100G_VERB(("mac_c_enable_get: unit %d port %s enable=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  *enable ? "True" : "False"));

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_duplex_set
 * Purpose:
 *      Set  CMAC in the specified duplex mode.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      duplex - Boolean: true --> full duplex, false --> half duplex.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 */
STATIC int
mac_c_duplex_set(int unit, soc_port_t port, int duplex)
{
    DBG_100G_VERB(("mac_c_duplex_set: unit %d port %s duplex=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  duplex ? "Full" : "Half"));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_duplex_get
 * Purpose:
 *      Get CMAC duplex mode.
 * Parameters:
 *      unit - XGS unit #.
 *      duplex - (OUT) Boolean: true --> full duplex, false --> half duplex.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_duplex_get(int unit, soc_port_t port, int *duplex)
{
    *duplex = TRUE; /* Always full duplex */

    DBG_100G_VERB(("mac_c_duplex_get: unit %d port %s duplex=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  *duplex ? "Full" : "Half"));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_pause_set
 * Purpose:
 *      Configure CMAC to transmit/receive pause frames.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      pause_tx - Boolean: transmit pause or -1 (don't change)
 *      pause_rx - Boolean: receive pause or -1 (don't change)
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_pause_set(int unit, soc_port_t port, int pause_tx, int pause_rx)
{
    soc_field_t fields[2] = { TX_PAUSE_ENf, RX_PAUSE_ENf };
    uint32 values[2];

    DBG_100G_VERB(("mac_c_pause_set: unit %d port %s TX=%s RX=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  pause_tx != FALSE ? "on" : "off",
                  pause_rx != FALSE ? "on" : "off"));

    values[0] = pause_tx != FALSE ? 1 : 0;
    values[1] = pause_rx != FALSE ? 1 : 0;
    return soc_reg_fields32_modify(unit, CMAC_PAUSE_CTRLr, port, 2,
                                   fields, values);
}

/*
 * Function:
 *      mac_c_pause_get
 * Purpose:
 *      Return the pause ability of CMAC
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      pause_tx - Boolean: transmit pause
 *      pause_rx - Boolean: receive pause
 *      pause_mac - MAC address used for pause transmission.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_pause_get(int unit, soc_port_t port, int *pause_tx, int *pause_rx)
{
    uint64 rval;

    SOC_IF_ERROR_RETURN(READ_CMAC_PAUSE_CTRLr(unit, port, &rval));
    *pause_tx =
        soc_reg64_field32_get(unit, CMAC_PAUSE_CTRLr, rval, TX_PAUSE_ENf);
    *pause_rx =
        soc_reg64_field32_get(unit, CMAC_PAUSE_CTRLr, rval, RX_PAUSE_ENf);
    DBG_100G_VERB(("mac_c_pause_get: unit %d port %s TX=%s RX=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  *pause_tx ? "on" : "off",
                  *pause_rx ? "on" : "off"));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_speed_set
 * Purpose:
 *      Set CMAC in the specified speed.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      speed - 100000, 120000.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_speed_set(int unit, soc_port_t port, int speed)
{
    uint32 rval;
    /*uint32 phy_mode, phy_mode_org;
    uint32 xlgmii_en;
    uint64 mac_ctrl;*/

    DBG_100G_VERB(("mac_c_speed_set: unit %d port %s speed=%dMb\n",
                  unit, SOC_PORT_NAME(unit, port),
                  speed));
    rval = 0; 
#ifdef TT
    /* get the original phy_mode */


    /* if using MLD, XLGMII_ALIGN_ENB bit needs to be set.
     * brcm 30g and 40g should not set this bit because it doesn't use MLD
     * for now we're using standard 30G/40G
     */
    if (speed >= 30000) {
        xlgmii_en = 1;
    } else {
        xlgmii_en = 0;
    }
    if (speed > 20000) {
        phy_mode = 0;
    } else if ((speed > 10000) && (speed <= 20000)) {
        phy_mode = 1;
    } else {
        phy_mode = 2;
    }
    if (phy_mode_org != phy_mode) {

        /* mac init */
        mac_c_init(unit,port);
    }
#endif
    /* program the XLGMII field */

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_speed_get
 * Purpose:
 *      Get CMAC speed
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      speed - (OUT) speed in Mb
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_speed_get(int unit, soc_port_t port, int *speed)
{
    *speed = 100000;

    
    DBG_100G_VERB(("mac_c_speed_get: unit %d port %s speed=%dMb\n",
                  unit, SOC_PORT_NAME(unit, port),
                  *speed));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_loopback_set
 * Purpose:
 *      Set a CMAC into/out-of loopback mode
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS unit # on unit.
 *      loopback - Boolean: true -> loopback mode, false -> normal operation
 * Note:
 *      On Cmac, when setting loopback, we enable the TX/RX function also.
 *      Note that to test the PHY, we use the remote loopback facility.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_loopback_set(int unit, soc_port_t port, int lb)
{
    DBG_100G_VERB(("mac_c_loopback_set: unit %d port %s loopback=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  lb ? "local" : "no"));

    return soc_reg_field32_modify(unit, CMAC_CTRLr, port, LINE_LOCAL_LPBKf,
                                  lb ? 1 : 0);

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_loopback_get
 * Purpose:
 *      Get current CMAC loopback mode setting.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      loopback - (OUT) Boolean: true = loopback, false = normal
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_loopback_get(int unit, soc_port_t port, int *lb)
{
    uint64 ctrl;
    int local, remote;

    SOC_IF_ERROR_RETURN(READ_CMAC_CTRLr(unit, port, &ctrl));

    remote = soc_reg64_field32_get(unit, CMAC_CTRLr, ctrl, CORE_REMOTE_LPBKf) |
        soc_reg64_field32_get(unit, CMAC_CTRLr, ctrl, LINE_REMOTE_LPBKf);
    local = soc_reg64_field32_get(unit, CMAC_CTRLr, ctrl, CORE_LOCAL_LPBKf) |
        soc_reg64_field32_get(unit, CMAC_CTRLr, ctrl, LINE_LOCAL_LPBKf);
    *lb = local | remote;

    DBG_100G_VERB(("mac_c_loopback_get: unit %d port %s loopback=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  *lb ? (remote ? "remote" : "local") : "no"));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_pause_addr_set
 * Purpose:
 *      Configure PAUSE frame source address.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      pause_mac - (OUT) MAC address used for pause transmission.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_pause_addr_set(int unit, soc_port_t port, sal_mac_addr_t m)
{
    uint64 r, tmp;
    int i;

    DBG_100G_VERB(("mac_c_pause_addr_set: unit %d port %s MAC=<"
                  "%02x:%02x:%02x:%02x:%02x:%02x>\n",
                  unit, SOC_PORT_NAME(unit, port),
                  m[0], m[1], m[2], m[3], m[4], m[5]));

    COMPILER_64_ZERO(r);
    for (i = 0; i< 6; i++) {
        COMPILER_64_SET(tmp, 0, m[i]);
        COMPILER_64_SHL(r, 8);
        COMPILER_64_OR(r, tmp);
    }

    SOC_IF_ERROR_RETURN(WRITE_CMAC_TX_MAC_SAr(unit, port, r));
    SOC_IF_ERROR_RETURN(WRITE_CMAC_RX_MAC_SAr(unit, port, r));

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_pause_addr_get
 * Purpose:
 *      Retrieve PAUSE frame source address.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      pause_mac - (OUT) MAC address used for pause transmission.
 * Returns:
 *      SOC_E_XXX
 * NOTE: We always write the same thing to TX & RX SA
 *       so, we just return the contects on RCMACSA.
 */
STATIC int
mac_c_pause_addr_get(int unit, soc_port_t port, sal_mac_addr_t m)
{
    uint64 reg;
    uint32 msw;
    uint32 lsw;

    SOC_IF_ERROR_RETURN(READ_CMAC_RX_MAC_SAr(unit, port, &reg));
    COMPILER_64_TO_32_HI(msw, reg);
    COMPILER_64_TO_32_LO(lsw, reg);

    m[0] = (uint8) ( ( msw & 0x0000ff00 ) >> 8 );
    m[1] = (uint8) ( msw & 0x000000ff );

    m[2] = (uint8)  ( ( lsw & 0xff000000) >> 24 );
    m[3] = (uint8)  ( ( lsw & 0x00ff0000) >> 16 );
    m[4] = (uint8)  ( ( lsw & 0x0000ff00) >> 8 );
    m[5] = (uint8)  ( lsw & 0x000000ff );

    DBG_100G_VERB(("mac_c_pause_addr_get: unit %d port %s MAC=<"
                  "%02x:%02x:%02x:%02x:%02x:%02x>\n",
                  unit, SOC_PORT_NAME(unit, port),
                  m[0], m[1], m[2], m[3], m[4], m[5]));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_interface_set
 * Purpose:
 *      Set a CMAC interface type
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      pif - one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_UNAVAIL - requested mode not supported.
 * Notes:
 *
 */
STATIC int
mac_c_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    DBG_100G_VERB(("mac_c_interface_set: unit %d port %s interface=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  mac_c_port_if_names[pif]));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_interface_get
 * Purpose:
 *      Retrieve CMAC interface type
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      pif - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
mac_c_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *pif = SOC_PORT_IF_MII;

    DBG_100G_VERB(("mac_c_interface_get: unit %d port %s interface=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  mac_c_port_if_names[*pif]));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_frame_max_set
 * Description:
 *      Set the maximum receive frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 */
STATIC int
mac_c_frame_max_set(int unit, soc_port_t port, int size)
{
    uint64 rval;

    DBG_100G_VERB(("mac_c_frame_max_set: unit %d port %s size=%d\n",
                  unit, SOC_PORT_NAME(unit, port),
                  size));

    if (IS_XE_PORT(unit, port)) {
        /* For VLAN tagged packets */
        size += 4;
    }
    COMPILER_64_ZERO(rval);
    soc_reg64_field32_set(unit, CMAC_RX_MAX_SIZEr, &rval, RX_MAX_SIZEf, size);
    SOC_IF_ERROR_RETURN(WRITE_CMAC_RX_MAX_SIZEr(unit, port, rval));

    /* Do we have such register for CMAC?
     * SOC_IF_ERROR_RETURN(WRITE_CMAC_TX_MAX_SIZEr(unit, port, rval));
     */

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_frame_max_get
 * Description:
 *      Set the maximum receive frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 */
STATIC int
mac_c_frame_max_get(int unit, soc_port_t port, int *size)
{
    uint64 rval;

    SOC_IF_ERROR_RETURN(READ_CMAC_RX_MAX_SIZEr(unit, port, &rval));
    *size = soc_reg64_field32_get(unit, CMAC_RX_MAX_SIZEr, rval, RX_MAX_SIZEf);
    if (IS_XE_PORT(unit, port)) {
        /* For VLAN tagged packets */
        *size -= 4;
    }

    DBG_100G_VERB(("mac_c_frame_max_get: unit %d port %s size=%d\n",
                  unit, SOC_PORT_NAME(unit, port),
                  *size));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_ifg_set
 * Description:
 *      Sets the new ifg (Inter-frame gap) value
 * Parameters:
 *      unit   - Device number
 *      port   - Port number
 *      speed  - the speed for which the IFG is being set
 *      duplex - the duplex for which the IFG is being set
 *      ifg - number of bits to use for average inter-frame gap
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      The function makes sure the IFG value makes sense and updates the
 *      IPG register in case the speed/duplex match the current settings
 */
STATIC int
mac_c_ifg_set(int unit, soc_port_t port, int speed,
                soc_port_duplex_t duplex, int ifg)
{
    int         cur_speed;
    int         cur_duplex;
    int         real_ifg;
    soc_ipg_t  *si = &SOC_PERSIST(unit)->ipg[port];
    uint64      rval, orval;
    soc_port_ability_t ability;
    uint32      pa_flag;

    DBG_100G_VERB(("mac_c_ifg_set: unit %d port %s speed=%dMb duplex=%s "
                  "ifg=%d\n",
                  unit, SOC_PORT_NAME(unit, port),
                  speed, duplex ? "True" : "False", ifg));

    _mac_c_speed_to_pa_flag(unit, port, speed, &pa_flag); 
    soc_mac_c.md_ability_local_get(unit, port, &ability);
    if (!(pa_flag & ability.speed_full_duplex)) {
        return SOC_E_PARAM;
    }

    /* Silently adjust the specified ifp bits to valid value */
    /* valid value: 8 to 31 bytes (i.e. multiple of 8 bits) */
    real_ifg = ifg < 64 ? 64 : (ifg + 7) & (0x1f << 3);

    if (IS_XE_PORT(unit, port)) {
        si->fd_xe = real_ifg;
    } else {
        si->fd_hg = real_ifg;
    }

    SOC_IF_ERROR_RETURN(mac_c_duplex_get(unit, port, &cur_duplex));
    SOC_IF_ERROR_RETURN(mac_c_speed_get(unit, port, &cur_speed));

    if (cur_speed == speed && cur_duplex == duplex) {
        SOC_IF_ERROR_RETURN(READ_CMAC_TX_CTRLr(unit, port, &rval));
        orval = rval;
        soc_reg64_field32_set(unit, CMAC_TX_CTRLr, &rval, AVERAGE_IPGf,
                              si->fd_10000 / 8);
        if (COMPILER_64_NE(rval, orval)) {
            SOC_IF_ERROR_RETURN(WRITE_CMAC_TX_CTRLr(unit, port, rval));
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_ifg_get
 * Description:
 *      Sets the new ifg (Inter-frame gap) value
 * Parameters:
 *      unit   - Device number
 *      port   - Port number
 *      speed  - the speed for which the IFG is being set
 *      duplex - the duplex for which the IFG is being set
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      The function makes sure the IFG value makes sense and updates the
 *      IPG register in case the speed/duplex match the current settings
 */
STATIC int
mac_c_ifg_get(int unit, soc_port_t port, int speed,
                soc_port_duplex_t duplex, int *ifg)
{
    soc_ipg_t  *si = &SOC_PERSIST(unit)->ipg[port];
    soc_port_ability_t ability;
    uint32      pa_flag;

    if (!duplex) {
        return SOC_E_PARAM;
    }

    _mac_c_speed_to_pa_flag(unit, port, speed, &pa_flag); 
    soc_mac_c.md_ability_local_get(unit, port, &ability);
    if (!(pa_flag & ability.speed_full_duplex)) {
        return SOC_E_PARAM;
    }

    if (IS_XE_PORT(unit, port)) {
        *ifg = si->fd_xe;
    } else {
        *ifg = si->fd_hg;
    }

    DBG_100G_VERB(("mac_c_ifg_get: unit %d port %s speed=%dMb duplex=%s "
                  "ifg=%d\n",
                  unit, SOC_PORT_NAME(unit, port),
                  speed, duplex ? "True" : "False", *ifg));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_loopback_remote_set
 * Purpose:
 *      Set the CMAC into remote-loopback state.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      mode - (INT) loopback enable state
 * Returns:
 *      SOC_E_XXX
 */
int
mac_c_loopback_remote_set(int unit, soc_port_t port, int lb)
{
    uint64 ctrl, octrl;

    DBG_100G_VERB(("mac_c_loopback_remote_set: unit %d port %s loopback=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  lb ? "remote" : "no"));

    SOC_IF_ERROR_RETURN(READ_CMAC_CTRLr(unit, port, &ctrl));
    octrl = ctrl;
    soc_reg64_field32_set(unit, CMAC_CTRLr, &ctrl, RMTLOOPf,
                          lb ? 1 : 0);
    if (COMPILER_64_NE(ctrl, octrl)) {
        SOC_IF_ERROR_RETURN(WRITE_CMAC_CTRLr(unit, port, ctrl));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      _mac_c_port_mode_update
 * Purpose:
 *      Set the CMAC port encapsulation mode.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      to_hg_port - (TRUE/FALSE)
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_mac_c_port_mode_update(int unit, soc_port_t port, int to_hg_port)
{
    uint32              rval;
    uint64              val64;
    soc_pbmp_t          ctr_pbmp;
    int                 rv = SOC_E_NONE;

    /* Pause linkscan */
    soc_linkscan_pause(unit);

    /* Pause counter collection */
    COUNTER_LOCK(unit);

    soc_xport_type_update(unit, port, to_hg_port);

    rv = soc_xgxs_reset(unit, port);

    if (SOC_SUCCESS(rv)) {
        rv = soc_phyctrl_init(unit, port);
    }

    if (SOC_SUCCESS(rv)) {
        rv = mac_c_init(unit, port);
    }

    if (SOC_SUCCESS(rv)) {
        rv = mac_c_enable_set(unit, port, 0);
    }

    if (SOC_SUCCESS(rv)) {
        SOC_PBMP_CLEAR(ctr_pbmp);
        SOC_PBMP_PORT_SET(ctr_pbmp, port);
        COMPILER_64_SET(val64, 0, 0);
        rv = soc_counter_set_by_port(unit, ctr_pbmp, val64);
    }

    COUNTER_UNLOCK(unit);
    soc_linkscan_continue(unit);
    rval = 0;
    SOC_IF_ERROR_RETURN(READ_PORT_CONFIGr(unit, port, &rval));
    if (to_hg_port) {
        soc_reg_field_set(unit, PORT_CONFIGr, &rval, HIGIG_MODEf, 1);
    } else {
        soc_reg_field_set(unit, PORT_CONFIGr, &rval, HIGIG_MODEf, 0);
    }
    SOC_IF_ERROR_RETURN(WRITE_PORT_CONFIGr(unit, port, rval));
    return rv;
}

/*
 * Function:
 *      mac_c_encap_set
 * Purpose:
 *      Set the CMAC port encapsulation mode.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      mode - (IN) encap bits (defined above)
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_encap_set(int unit, soc_port_t port, int mode)
{
    int enable, encap, rv;

    DBG_100G_VERB(("mac_c_encap_set: unit %d port %s encapsulation=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  mac_c_encap_mode[mode]));

    switch (mode) {
    case SOC_ENCAP_IEEE:
        encap = 0;
        break;
    case SOC_ENCAP_HIGIG:
        encap = 1;
        break;
    case SOC_ENCAP_HIGIG2:
        encap = 2;
        break;
    default:
        return SOC_E_PARAM;
    }

    if (!soc_feature(unit, soc_feature_xport_convertible)) {
        if ((IS_E_PORT(unit, port) && mode != SOC_ENCAP_IEEE) ||
            (IS_ST_PORT(unit, port) && mode == SOC_ENCAP_IEEE)) {
            return SOC_E_PARAM;
        }
    }

    SOC_IF_ERROR_RETURN(mac_c_enable_get(unit, port, &enable));

    if (enable) {
        /* Turn off TX/RX enable */
        SOC_IF_ERROR_RETURN(mac_c_enable_set(unit, port, 0));
    }

    if (IS_E_PORT(unit, port) && mode != SOC_ENCAP_IEEE) {
        /* XE -> HG */
        SOC_IF_ERROR_RETURN(_mac_c_port_mode_update(unit, port, TRUE));
    } else if (IS_ST_PORT(unit, port) && mode == SOC_ENCAP_IEEE) {
        /* HG -> XE */
        SOC_IF_ERROR_RETURN(_mac_c_port_mode_update(unit, port, FALSE));
    }

    /* Update the encapsulation mode */
    rv = soc_reg_field32_modify(unit, CMAC_MODEr, port, HDR_MODEf, encap);

    if (enable) {
        /* Re-enable transmitter and receiver */
        SOC_IF_ERROR_RETURN(mac_c_enable_set(unit, port, 1));
    }

    return rv;
}

/*
 * Function:
 *      mac_c_encap_get
 * Purpose:
 *      Get the CMAC port encapsulation mode.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      mode - (INT) encap bits (defined above)
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_encap_get(int unit, soc_port_t port, int *mode)
{
    uint64 rval;

    if (!mode) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(READ_CMAC_MODEr(unit, port, &rval));
    switch (soc_reg64_field32_get(unit, CMAC_MODEr, rval, HDR_MODEf)) {
    case 0:
        *mode = SOC_ENCAP_IEEE;
        break;
    case 1:
        *mode = SOC_ENCAP_HIGIG;
        break;
    case 2:
        *mode = SOC_ENCAP_HIGIG2;
        break;
    default:
        *mode = SOC_ENCAP_COUNT;
    }

    DBG_100G_VERB(("mac_c_encap_get: unit %d port %s encapsulation=%s\n",
                  unit, SOC_PORT_NAME(unit, port),
                  mac_c_encap_mode[*mode]));
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_control_set
 * Purpose:
 *      To configure MAC control properties.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      type - MAC control property to set.
 *      int  - New setting for MAC control.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_control_set(int unit, soc_port_t port, soc_mac_control_t type,
                  int value)
{
    uint64 rval, copy;
    uint64 da;
    uint32 da_low, da_hi;

    DBG_100G_VERB(("mac_c_control_set: unit %d port %s type=%d value=%d\n",
                  unit, SOC_PORT_NAME(unit, port),
                  type, value));

    switch (type) {
    case SOC_MAC_CONTROL_RX_SET:
        SOC_IF_ERROR_RETURN(READ_CMAC_CTRLr(unit, port, &rval));
        copy = rval;
        soc_reg64_field32_set(unit, CMAC_CTRLr, &rval, RX_ENf, value ? 1 : 0);
        if (COMPILER_64_NE(rval, copy)) {
            SOC_IF_ERROR_RETURN(WRITE_CMAC_CTRLr(unit, port, rval));
        }    
        break;
    case SOC_MAC_CONTROL_FRAME_SPACING_STRETCH:
        if (value < 0 || value > 255) {
            return SOC_E_PARAM;
        } else {
            SOC_IF_ERROR_RETURN(READ_CMAC_TX_CTRLr(unit, port, &rval));
            if (value >= 8) {
                soc_reg64_field32_set(unit, CMAC_TX_CTRLr, &rval, THROT_DENOMf,
                                      value);
                soc_reg64_field32_set(unit, CMAC_TX_CTRLr, &rval, THROT_NUMf,
                                      1);
            } else {
                soc_reg64_field32_set(unit, CMAC_TX_CTRLr, &rval, THROT_DENOMf,
                                      0);
                soc_reg64_field32_set(unit, CMAC_TX_CTRLr, &rval, THROT_NUMf,
                                      0);
            }
            SOC_IF_ERROR_RETURN(WRITE_CMAC_TX_CTRLr(unit, port, rval));
        }
        return SOC_E_NONE;

    case SOC_MAC_PASS_CONTROL_FRAME:
        SOC_IF_ERROR_RETURN(READ_CMAC_RX_CTRLr(unit, port, &rval));
        soc_reg64_field32_set(unit, CMAC_RX_CTRLr, &rval, RX_PASS_CTRLf,
                              value ? 1 : 0);
        SOC_IF_ERROR_RETURN(WRITE_CMAC_RX_CTRLr(unit, port, rval));
        break;

    case SOC_MAC_CONTROL_PFC_TYPE:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_TYPEr(unit, port, &rval));
        soc_reg64_field32_set(unit, CMAC_PFC_TYPEr, &rval, PFC_ETH_TYPEf,
                              (value & 0xffff));
        SOC_IF_ERROR_RETURN(WRITE_CMAC_PFC_TYPEr(unit, port, rval));
        break;

    case SOC_MAC_CONTROL_PFC_OPCODE:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_OPCODEr(unit, port, &rval));
        soc_reg64_field32_set(unit, CMAC_PFC_OPCODEr, &rval, PFC_OPCODEf,
                              (value & 0xffff));
        SOC_IF_ERROR_RETURN(WRITE_CMAC_PFC_OPCODEr(unit, port, rval));
        break;

    case SOC_MAC_CONTROL_PFC_CLASSES:
        break;

    case SOC_MAC_CONTROL_PFC_RX_PASS:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_CTRLr(unit, port, &rval));
        soc_reg64_field32_set(unit, CMAC_PFC_CTRLr, &rval, RX_PASS_PFCf,
                              value ? 1 : 0);
        SOC_IF_ERROR_RETURN(WRITE_CMAC_PFC_CTRLr(unit, port, rval));
        break;

    case SOC_MAC_CONTROL_PFC_RX_ENABLE:
    case SOC_MAC_CONTROL_PFC_TX_ENABLE:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_CTRLr(unit, port, &rval));
        soc_reg64_field32_set(unit, CMAC_PFC_CTRLr, &rval, 
                              (type == SOC_MAC_CONTROL_PFC_RX_ENABLE) ? RX_PFC_ENf :
                              TX_PFC_ENf,
                              value ? 1 : 0);

        /* Flush MMU XOFF state with toggle bit */
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CMAC_PFC_CTRLr, port,
                                    FORCE_PFC_XONf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CMAC_PFC_CTRLr, port,
                                    FORCE_PFC_XONf, 0));
        /* Update stats collection */
        if (soc_reg64_field32_get(unit, CMAC_PFC_CTRLr, rval, RX_PFC_ENf) ||
            soc_reg64_field32_get(unit, CMAC_PFC_CTRLr, rval, TX_PFC_ENf)) {
            soc_reg64_field32_set(unit, CMAC_PFC_CTRLr, &rval, 
                                  PFC_STATS_ENf, 1);
        } else {
            soc_reg64_field32_set(unit, CMAC_PFC_CTRLr, &rval, 
                                  PFC_STATS_ENf, 0);
        }
        SOC_IF_ERROR_RETURN(WRITE_CMAC_PFC_CTRLr(unit, port, rval));
        break;

    case SOC_MAC_CONTROL_PFC_MAC_DA_OUI:
    case SOC_MAC_CONTROL_PFC_MAC_DA_NONOUI:
        COMPILER_64_ZERO(da);
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_DAr(unit, port, &rval));
        if (type == SOC_MAC_CONTROL_PFC_MAC_DA_OUI) {
            da_low = (value & 0x000000ff) << 24 | 
                             (COMPILER_64_LO(rval) & 0x00ffffff);
            da_hi = (value & 0x00ffff00) >> 8;
        }else {
            da_low = (value & 0x00ffffff) | 
                             (COMPILER_64_LO(rval) & 0xff000000);
            da_hi = COMPILER_64_HI(rval);

        }
        COMPILER_64_SET(da, da_hi, da_low);
        soc_reg64_field_set(unit, CMAC_PFC_DAr, &rval, PFC_MACDAf,
                            da);
        SOC_IF_ERROR_RETURN(WRITE_CMAC_PFC_DAr(unit, port, rval));
        break;

    default:
        return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_c_control_get
 * Purpose:
 *      To get current MAC control setting.
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      type - MAC control property to set.
 *      int  - New setting for MAC control.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_control_get(int unit, soc_port_t port, soc_mac_control_t type,
                  int *value)
{
    int rv;
    uint64 rval;

    if (value == NULL) {
        return SOC_E_PARAM;
    }

    rv = SOC_E_UNAVAIL;
    switch (type) {
    case SOC_MAC_CONTROL_RX_SET:
        SOC_IF_ERROR_RETURN(READ_CMAC_CTRLr(unit, port, &rval));
        *value = soc_reg64_field32_get(unit, CMAC_CTRLr, rval, RX_ENf);
        break;
    case SOC_MAC_CONTROL_FRAME_SPACING_STRETCH:
        SOC_IF_ERROR_RETURN(READ_CMAC_TX_CTRLr(unit, port, &rval));
        *value = soc_reg64_field32_get(unit, CMAC_TX_CTRLr, rval,
                                       THROT_DENOMf);
        rv = SOC_E_NONE;
        break;

    case SOC_MAC_PASS_CONTROL_FRAME:
        SOC_IF_ERROR_RETURN(READ_CMAC_RX_CTRLr(unit, port, &rval));
        *value = soc_reg64_field32_get(unit, CMAC_RX_CTRLr, rval,
                                       RX_PASS_CTRLf);
        break;

    case SOC_MAC_CONTROL_PFC_TYPE:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_TYPEr(unit, port, &rval));
        *value = soc_reg64_field32_get(unit, CMAC_PFC_TYPEr, rval, 
                                       PFC_ETH_TYPEf);
        *value = (*value & 0xffff);
        break;

    case SOC_MAC_CONTROL_PFC_OPCODE:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_OPCODEr(unit, port, &rval));
        *value = soc_reg64_field32_get(unit, CMAC_PFC_OPCODEr, rval, 
                                       PFC_OPCODEf);
        *value = (*value & 0xffff);
        break;

    case SOC_MAC_CONTROL_PFC_CLASSES:
        *value = 8;
        break;
    case SOC_MAC_CONTROL_PFC_RX_PASS:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_CTRLr(unit, port, &rval));
        *value = soc_reg64_field32_get(unit, CMAC_PFC_CTRLr, rval, 
                                       RX_PASS_PFCf);
        break;

    case SOC_MAC_CONTROL_PFC_RX_ENABLE:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_CTRLr(unit, port, &rval));
        *value = soc_reg64_field32_get(unit, CMAC_PFC_CTRLr, rval, 
                                       RX_PFC_ENf);

    case SOC_MAC_CONTROL_PFC_TX_ENABLE:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_CTRLr(unit, port, &rval));
        *value = soc_reg64_field32_get(unit, CMAC_PFC_CTRLr, rval, 
                                       TX_PFC_ENf);

    case SOC_MAC_CONTROL_PFC_MAC_DA_OUI:
    case SOC_MAC_CONTROL_PFC_MAC_DA_NONOUI:
        SOC_IF_ERROR_RETURN(READ_CMAC_PFC_DAr(unit, port, &rval));
        if (type == SOC_MAC_CONTROL_PFC_MAC_DA_OUI) {
            *value = (((COMPILER_64_HI(rval) & 0x0000ffff) << 8 )  |
                      ((COMPILER_64_LO(rval) & 0xff000000) >> 24));
        }else {
            *value = (COMPILER_64_LO(rval) & 0x00ffffff);
        }
        break;

    default:
        return SOC_E_UNAVAIL;
    }

    DBG_100G_VERB(("mac_c_control_get: unit %d port %s type=%d value=%d "
                  "rv=%d\n",
                  unit, SOC_PORT_NAME(unit, port),
                  type, *value, rv));
    return rv;
}

/*
 * Function:
 *      mac_c_ability_local_get
 * Purpose:
 *      Return the abilities of CMAC
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      mode - (OUT) Supported operating modes as a mask of abilities.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
mac_c_ability_local_get(int unit, soc_port_t port,
                          soc_port_ability_t *ability)
{
    if (NULL == ability) {
        return SOC_E_PARAM;
    }

    ability->speed_half_duplex  = SOC_PA_ABILITY_NONE;
    ability->speed_full_duplex  = SOC_PA_SPEED_100GB;
    if (SOC_INFO(unit).port_speed_max[port] >= 120000) {
        ability->speed_full_duplex |= SOC_PA_SPEED_120GB;
    }
    ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
    ability->interface = SOC_PA_INTF_MII | SOC_PA_INTF_XGMII;
    ability->medium    = SOC_PA_ABILITY_NONE;
    ability->loopback  = SOC_PA_LB_MAC;
    ability->flags     = SOC_PA_ABILITY_NONE;

    if (!IS_HG_PORT(unit , port)) {
        DBG_100G_VERB(("mac_c_ability_local_get: unit %d port %s "
                      "speed_half=0x%x speed_full=0x%x pause=0x%x "
                      "interface=0x%x medium=0x%x loopback=0x%x flags=0x%x\n",
                      unit, SOC_PORT_NAME(unit, port),
                      ability->speed_half_duplex, ability->speed_full_duplex,
                      ability->pause, ability->interface, ability->medium,
                      ability->loopback, ability->flags));
        return SOC_E_NONE;
    }

    switch (SOC_INFO(unit).port_speed_max[port]) {
    case 120000:
        ability->speed_full_duplex |= SOC_PA_SPEED_120GB;
        /* fall through */
    case 100000:
        ability->speed_full_duplex |= SOC_PA_SPEED_100GB;
        /* fall through */
    default:
        break;
    }

    DBG_100G_VERB(("mac_c_ability_local_get: unit %d port %s "
                  "speed_half=0x%x speed_full=0x%x pause=0x%x "
                  "interface=0x%x medium=0x%x loopback=0x%x flags=0x%x\n",
                  unit, SOC_PORT_NAME(unit, port),
                  ability->speed_half_duplex, ability->speed_full_duplex,
                  ability->pause, ability->interface, ability->medium,
                  ability->loopback, ability->flags));
    return SOC_E_NONE;
}

/* Exported CMAC driver structure */
mac_driver_t soc_mac_c = {
    "CMAC Driver",                /* drv_name */
    mac_c_init,                   /* md_init  */
    mac_c_enable_set,             /* md_enable_set */
    mac_c_enable_get,             /* md_enable_get */
    mac_c_duplex_set,             /* md_duplex_set */
    mac_c_duplex_get,             /* md_duplex_get */
    mac_c_speed_set,              /* md_speed_set */
    mac_c_speed_get,              /* md_speed_get */
    mac_c_pause_set,              /* md_pause_set */
    mac_c_pause_get,              /* md_pause_get */
    mac_c_pause_addr_set,         /* md_pause_addr_set */
    mac_c_pause_addr_get,         /* md_pause_addr_get */
    mac_c_loopback_set,           /* md_lb_set */
    mac_c_loopback_get,           /* md_lb_get */
    mac_c_interface_set,          /* md_interface_set */
    mac_c_interface_get,          /* md_interface_get */
    NULL,                         /* md_ability_get - Deprecated */
    mac_c_frame_max_set,          /* md_frame_max_set */
    mac_c_frame_max_get,          /* md_frame_max_get */
    mac_c_ifg_set,                /* md_ifg_set */
    mac_c_ifg_get,                /* md_ifg_get */
    mac_c_encap_set,              /* md_encap_set */
    mac_c_encap_get,              /* md_encap_get */
    mac_c_control_set,            /* md_control_set */
    mac_c_control_get,            /* md_control_get */
    mac_c_ability_local_get       /* md_ability_local_get */
 };

#endif /* BCM_CMAC_SUPPORT */
