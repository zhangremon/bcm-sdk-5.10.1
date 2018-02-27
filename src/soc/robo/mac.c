/*
 * $Id: mac.c 1.60.2.1 Broadcom SDK $
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
 * Modena Media Access Controller API
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>

#include <soc/mcm/robo/driver.h>        
#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/portmode.h>
#include <soc/ll.h>

#include <soc/debug.h>
#include <soc/mem.h>

/* Define symbols for frame size set and get */
#define _DRV_ROBO_FRAME_SIZE_1518 (1518)
#define _DRV_ROBO_FRAME_SIZE_2000 (2000)
#define _DRV_ROBO_FRAME_SIZE_2032 (2032)
#define _DRV_ROBO_FRAME_SIZE_9712 (9712)

/*
 * Forward Declarations
 *
 *  mac_ge_xxx  Routines that work on GE ports and
 *          GE ports that support 10/100 mode.
 */

STATIC int  drv_mac_fe_init(int unit, soc_port_t port);
STATIC int  drv_mac_fe_enable_set(int unit, soc_port_t port, int enable);
STATIC int  drv_mac_fe_enable_get(int unit, soc_port_t port, int *enable);

STATIC int  drv_mac_fe_ge_duplex_set(int unit, soc_port_t port, int duplex);
STATIC int  drv_mac_fe_ge_duplex_get(int unit, soc_port_t port, int *duplex);
STATIC int  drv_mac_fe_ge_speed_set(int unit, soc_port_t port, int speed);
STATIC int  drv_mac_fe_ge_speed_get(int unit, soc_port_t port, int *speed);

STATIC int  drv_mac_fe_ge_pause_set(int unit, soc_port_t port,
                 int pause_tx, int pause_rx);
STATIC int  drv_mac_fe_ge_pause_get(int unit, soc_port_t port,
                 int *pause_tx, int *pause_rx);
STATIC int  drv_mac_fe_ge_pause_addr_set(int unit, soc_port_t port,
                      sal_mac_addr_t);
STATIC int  drv_mac_fe_ge_pause_addr_get(int unit, soc_port_t port,
                      sal_mac_addr_t);
STATIC int  drv_mac_fe_ge_loopback_set(int unit, soc_port_t port, int);
STATIC int  drv_mac_fe_ge_loopback_get(int unit, soc_port_t port, int *);
STATIC int  drv_mac_fe_ge_interface_set(int unit, soc_port_t port,
                     soc_port_if_t pif);
STATIC int  drv_mac_fe_ge_interface_get(int unit, soc_port_t port, 
    soc_port_if_t *pif);

STATIC int  drv_mac_fe_ge_ability_get(int unit, soc_port_t port, 
    soc_port_mode_t *mode);

STATIC int  drv_mac_fe_ge_frame_max_set(int unit, soc_port_t port, 
    int size);
STATIC int  drv_mac_fe_ge_frame_max_get(int unit, soc_port_t port, 
    int *size);
STATIC int      drv_mac_fe_ge_ifg_set(int unit, soc_port_t port, int speed, 
    soc_port_duplex_t duplex, int ifg);
STATIC int      drv_mac_fe_ge_ifg_get(int unit, soc_port_t port, int speed, 
    soc_port_duplex_t duplex, int *ifg);


STATIC int  drv_mac_ge_init(int unit, soc_port_t port);
STATIC int  drv_mac_ge_enable_set(int unit, soc_port_t port, int enable);
STATIC int  drv_mac_ge_enable_get(int unit, soc_port_t port, int *enable);

STATIC int  drv_mac_fe_ge_ability_local_get(int unit, soc_port_t port, 
    soc_port_ability_t *ability);

/* Exported MAC driver structure */

mac_driver_t drv_mac_fe = {
    "drv 10/100mb MAC Driver",
    drv_mac_fe_init,
    drv_mac_fe_enable_set,
    drv_mac_fe_enable_get,
    drv_mac_fe_ge_duplex_set,
    drv_mac_fe_ge_duplex_get,
    drv_mac_fe_ge_speed_set,
    drv_mac_fe_ge_speed_get,
    drv_mac_fe_ge_pause_set,
    drv_mac_fe_ge_pause_get,
    drv_mac_fe_ge_pause_addr_set,
    drv_mac_fe_ge_pause_addr_get,
    drv_mac_fe_ge_loopback_set,
    drv_mac_fe_ge_loopback_get,
    drv_mac_fe_ge_interface_set,
    drv_mac_fe_ge_interface_get,
    drv_mac_fe_ge_ability_get,
    drv_mac_fe_ge_frame_max_set,
    drv_mac_fe_ge_frame_max_get,
    drv_mac_fe_ge_ifg_set,
    drv_mac_fe_ge_ifg_get,
    NULL,
    NULL,
    NULL,
    NULL,    
    drv_mac_fe_ge_ability_local_get,
};

mac_driver_t drv_mac_ge = {
    "drv 10/100/1000Mb MAC Driver",
    drv_mac_ge_init,
    drv_mac_ge_enable_set,
    drv_mac_ge_enable_get,
    drv_mac_fe_ge_duplex_set,
    drv_mac_fe_ge_duplex_get,
    drv_mac_fe_ge_speed_set,
    drv_mac_fe_ge_speed_get,
    drv_mac_fe_ge_pause_set,
    drv_mac_fe_ge_pause_get,
    drv_mac_fe_ge_pause_addr_set,
    drv_mac_fe_ge_pause_addr_get,
    drv_mac_fe_ge_loopback_set,
    drv_mac_fe_ge_loopback_get,
    drv_mac_fe_ge_interface_set,
    drv_mac_fe_ge_interface_get,
    drv_mac_fe_ge_ability_get,
    drv_mac_fe_ge_frame_max_set,
    drv_mac_fe_ge_frame_max_get,
    drv_mac_fe_ge_ifg_set,
    drv_mac_fe_ge_ifg_get,
    NULL,
    NULL,
    NULL,
    NULL,
    drv_mac_fe_ge_ability_local_get,
};

/*
 * Function:
 *  drv_mac_fe_init
 * Purpose:
 *  Initialize the gigabit MAC into a known good state.
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  Both the FE and GE MACs are initialized.
 *  The initialization speed/duplex is arbitrary and must be
 *  updated by linkscan before enabling the MAC.
 */

STATIC int
drv_mac_fe_init(int unit, soc_port_t port)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;
    uint64  reg_value64;
    uint64  val64;
    int shift;
    
    /* initialize per port mac */
    soc_cm_debug(DK_VERBOSE, "drv_mac_fe_init\n");

    /* Enable TX/RX */
    SOC_IF_ERROR_RETURN(REG_READ_TH_PCTLr(unit, port, &reg_value));
    temp = 0;
    if (SOC_IS_TBX(unit)){
        SOC_IF_ERROR_RETURN(soc_TH_PCTLr_field_set(
                unit, &reg_value, TX_DISf, &temp));
        SOC_IF_ERROR_RETURN(soc_TH_PCTLr_field_set(
                unit, &reg_value, RX_DISf, &temp));
    } else {
        SOC_IF_ERROR_RETURN(soc_TH_PCTLr_field_set(
                unit, &reg_value, MITX_DISf, &temp));
        SOC_IF_ERROR_RETURN(soc_TH_PCTLr_field_set(
                unit, &reg_value, MIRX_DISf, &temp));
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_TH_PCTLr(unit, port, &reg_value));
    
    soc_cm_debug(DK_TESTS, 
                "drv_mac_fe_init: diable SW flow control bit at port=%d\n",
                port);
    if (SOC_IS_TBX(unit)){
        /* init Flow Control configuration :
         *  1. make sure the Port Flow control are disabled
         *
         *  Note :
         *  1. Global flow control is enabled in soc_robo_misc_init().
         *     Such design can also be applied to other 5324/53242/5348
         *     after verified.
         */ 

        SOC_IF_ERROR_RETURN(REG_READ_SW_XOFF_PORT_CTLr(unit, 
                (uint32 *)&reg_value64));
        
        /* disable all port's flow control when init */
        temp = 0;
        COMPILER_64_SET(val64, 0, temp); 
        COMPILER_64_SHL(val64, port * 2);

        /* OR to original configuration */
        COMPILER_64_OR(reg_value64, val64);
        SOC_IF_ERROR_RETURN(
                REG_WRITE_SW_XOFF_PORT_CTLr(unit, (uint32 *)&reg_value64));
    } else {
        /* init SW_FLOW_CON register 
         *      1. set global enable bit
         *      2. disable the port bit for init. (must include CPU port)
         *      3. enable flow control in half duplex mode.
         */
        if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)
            || SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {

            SOC_IF_ERROR_RETURN(REG_READ_SW_FLOW_CONr(
                    unit, (uint32 *)&reg_value64));

            /* Enable global enable SW flow control bit */
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_SW_FLOW_CONr_field_set(
                    unit, (uint32 *)&reg_value64, SW_FLOE_CON_ENf, &temp));

            /* reset port bit set(disable) */
            if (SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit) ||
                SOC_IS_ROBO53262(unit)) {
                shift = port + 24;
            } else {
                shift = port;
            }
            COMPILER_64_SET(val64, 0, 1);
            COMPILER_64_SHL(val64, shift);
            COMPILER_64_NOT(val64);
            COMPILER_64_AND(reg_value64, val64);
            
            /* Enable port flow control in Half duplex */
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_SW_FLOW_CONr_field_set(
                    unit, (uint32 *)&reg_value64, TXFLOW_HALF_MODEf, &temp));
        
            /* Write SW_FLOW_CON Register */
            SOC_IF_ERROR_RETURN(REG_WRITE_SW_FLOW_CONr(
                    unit, (uint32 *)&reg_value64));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_SW_FLOW_CONr(unit, &reg_value));
            
            /* Enable global enable SW flow control bit */
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_SW_FLOW_CONr_field_set(
                    unit, &reg_value, SW_FLOE_CON_ENf, &temp));

            /* reset port bit set(disable) */
            reg_value &= ~(uint32)(1 << port);
            
            /* Enable port flow control in Half duplex */
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_SW_FLOW_CONr_field_set(
                    unit, &reg_value, TXFLOW_HALF_MODEf, &temp));
        
            /* Write SW_FLOW_CON Register */
            SOC_IF_ERROR_RETURN(REG_WRITE_SW_FLOW_CONr(unit, &reg_value));
        }
    }

    /* disable the port PHY auto-scan function :
     *  - Not all ROBO chip have such design about PHY scan. (only 5348 fe 
     *      ports has such register field.)
     *  - PHY scan is disabled for the SW override is applied in SDK solution.
     */
    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)){
        temp = 0;
        
        SOC_IF_ERROR_RETURN(
                REG_READ_STS_OVERRIDE_Pr(unit, port, &reg_value));
        SOC_IF_ERROR_RETURN(
                soc_STS_OVERRIDE_Pr_field_set(unit, &reg_value, 
                EN_PHY_SCANf, &temp));
        SOC_IF_ERROR_RETURN(
                REG_WRITE_STS_OVERRIDE_Pr(unit, port, &reg_value));                
    }
    
    return rv;
}

STATIC int
drv_mac_fe_enable_set(int unit, soc_port_t port, int enable)
{
    int     rv = SOC_E_NONE;
    uint32  port_ctrl_addr, reg_len, temp;
    uint32  port_ctrl;
    uint32  field_id;

    /* initialize per port mac */
    soc_cm_debug(DK_VERBOSE, "drv_mac_fe_enable_set\n");

    /* Read Port Control Register */ 
    port_ctrl_addr = DRV_REG_ADDR(unit, TH_PCTLr, port, 0);
    reg_len = DRV_REG_LENGTH_GET(unit, TH_PCTLr);
    if ((rv = DRV_REG_READ(unit, port_ctrl_addr, &port_ctrl, reg_len)) < 0) {
        return rv;
    }
    /* Enable TX/RX */
    if (enable) {
        temp = 0;
    } else { /* Disable TX/RX */
        temp = 1;
    }
    if (SOC_IS_TBX(unit)){
        field_id = INDEX(TX_DISf);
    } else {
        field_id = MITX_DISf;
    }
    DRV_REG_FIELD_SET(unit, TH_PCTLr, &port_ctrl, field_id, &temp);
    if (SOC_IS_TBX(unit)){
        field_id = INDEX(RX_DISf);
    } else {
        /* GNATS 40307 : Unexpected override on Port discard configuration
         *          while the link been re-established.
         *
         *  Note :
         *  1. Except TB chips, all ROBO chips use RX disable to implemente the 
         *     port discard related configuration.
         *  2. This definition is used to be a special MAC enable case to  
         *     indicate the MAC been enabled exclude RX (RX is disabled).
         */
        if (enable == DRV_SPECIAL_MAC_ENABLE_NORX){
            temp = 1;
        }
        field_id = MIRX_DISf;
    }
    DRV_REG_FIELD_SET(unit, TH_PCTLr, &port_ctrl, field_id, &temp);
    /* Write Port Control Register */
    if ((rv = DRV_REG_WRITE(unit, port_ctrl_addr, &port_ctrl, reg_len)) < 0) {
        return rv;
    }

    return rv;
}

/*
 * Function:
 *  mac_ge_enable_get
 * Purpose:
 *  Get MAC enable state
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - Port number on unit.
 *  enable - (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
drv_mac_fe_enable_get(int unit, soc_port_t port, int *enable)
{
    int     rv = SOC_E_NONE;
    uint32  port_ctrl_addr, reg_len, temp;
    uint32  port_ctrl;
    uint32  field_id;

    /* initialize per port mac */
    soc_cm_debug(DK_VERBOSE, "drv_mac_fe_enable_get\n");

    /* Read Port Control Register */ 
    port_ctrl_addr = DRV_REG_ADDR(unit, TH_PCTLr, port, 0);
    reg_len = DRV_REG_LENGTH_GET(unit, TH_PCTLr);
    if ((rv = DRV_REG_READ(unit, port_ctrl_addr, &port_ctrl, reg_len)) < 0) {
        return rv;
    }

    /* Read TX value */
    temp = 0;
    if (SOC_IS_TBX(unit)){
        field_id = INDEX(TX_DISf);
    } else {
        field_id = MITX_DISf;
    }
    DRV_REG_FIELD_GET(unit, TH_PCTLr, &port_ctrl, field_id, &temp);
    if (temp) {
        *enable = FALSE;
    } else {
        *enable = TRUE;
    }

    return rv;
}



/*
 * Function:
 *  mac_ge_duplex_set
 * Purpose:
 *  Set GE MAC in the specified duplex mode.
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit.
 *  duplex - Boolean: true --> full duplex, false --> half duplex.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  
 */

STATIC int
drv_mac_fe_ge_duplex_set(int unit, soc_port_t port, int duplex)
{
    int     rv = SOC_E_NONE;
    uint32  reg_index, field_index, reg_addr, reg_len, temp;
    uint32  reg_value;
    soc_port_t temp_port = port;

    soc_cm_debug(DK_VERBOSE, "drv_mac_fe_ge_duplex_set: u=%d p=%d FD=%x\n",
        unit, port, duplex);

    /* here for ROBO chip SW override setting about the MAC duplex only.
     *  and the sw_override field won't be set for this field has be set 
     *  already when set the MAC link bit.
     */

    field_index = DUPLX_MODEf;
    temp = (duplex) ? 1 : 0;
    if (IS_FE_PORT(unit, port)){
        reg_index = INDEX(STS_OVERRIDE_Pr);
        
        /* bcm5324 duplex field valuse is invered as normal */
        if (SOC_IS_ROBO5324(unit)){
            temp = (duplex) ? 0 : 1;
        }
        if (SOC_IS_ROBO53101(unit)) {
            reg_index = INDEX(STS_OVERRIDE_GMIIPr);               
        }
    } else if (IS_GE_PORT(unit, port)){
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit) ||
            SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)){
            reg_index = INDEX(STS_OVERRIDE_GMIIPr);               
        } else {
            reg_index = INDEX(STS_OVERRIDE_GPr);               
        }
    } else {
        return SOC_E_PARAM;
    }

    OVERRIDE_LOCK(unit);
    /* Read Port Override Register */ 
    reg_addr = DRV_REG_ADDR(unit, reg_index, temp_port, 0);
    reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
    if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
        OVERRIDE_UNLOCK(unit);
        return rv;
    }

    DRV_REG_FIELD_SET(unit, reg_index, &reg_value, field_index, &temp);
    
    /* Set port MAC duplex */
    if ((rv = DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len)) < 0) {
        OVERRIDE_UNLOCK(unit);
        return rv;
    }
    OVERRIDE_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *  mac_ge_duplex_get
 * Purpose:
 *  Get GE MAC duplex mode.
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit.
 *  duplex - (OUT) Boolean: true --> full duplex, false --> half duplex.
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
drv_mac_fe_ge_duplex_get(int unit, soc_port_t port, int *duplex)
{
    uint32  addr = DRV_REG_ADDR(unit, INDEX(DUPSTSr), 0, 0);
    uint32  reg_len, reg_value32;
    uint64  reg_value64;
    uint64 val64;
    int     rv = SOC_E_NONE;

    reg_len = DRV_REG_LENGTH_GET(unit, DUPSTSr);
    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
        SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        if ((rv = DRV_REG_READ(unit, addr, 
                (uint32 *)&reg_value64, reg_len)) < 0) {
            return rv;
        }

        if (IS_PORT(unit, port)) {
            if (SOC_IS_ROBO5348(unit)) {
                /*  reg_value64 = (reg_value64 >> port) & 0x1; */
                COMPILER_64_SET(val64, 0, 1);
                COMPILER_64_SHR(reg_value64, port);
                COMPILER_64_AND(reg_value64, val64);  
            } else {
                /*reg_value64 = (reg_value64 >> (port + 24)) & 0x1;*/
                COMPILER_64_SET(val64, 0, 1);
                COMPILER_64_SHR(reg_value64, port+24);
                COMPILER_64_AND(reg_value64, val64);  
            }
            if (COMPILER_64_EQ(reg_value64, val64)) {
                *duplex = TRUE;
            } else {
                *duplex = FALSE;
            }

        } else {
            return SOC_E_PARAM;
        }
    } else {
        if ((rv = DRV_REG_READ(unit, addr, 
                (uint32 *)&reg_value32, reg_len)) < 0) {
            return rv;
        }
        if (IS_PORT(unit, port) || IS_CPU_PORT(unit, port)) {
            if (reg_value32 & (1 << port)) {
                *duplex = TRUE;
            } else {
                *duplex = FALSE;
            }
        } else {
            return SOC_E_PARAM;
        }
    }
     soc_cm_debug(DK_VERBOSE, "drv_mac_fe_ge_duplex_get: u=%d p=%d FD=%x\n",
        unit, port, *duplex);

    return(SOC_E_NONE);
}


/*
 * Function:
 *  mac_ge_speed_set
 * Purpose:
 *  Set GE MAC in the specified speed.
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - Port number on unit.
 *  speed - 10,100,1000 for speed.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  
 */

STATIC int
drv_mac_fe_ge_speed_set(int unit, soc_port_t port, int speed)
{
    int     rv = SOC_E_NONE;
    uint32  reg_index, field_index, reg_addr, reg_len, temp;
    uint32  reg_value;
    soc_port_t temp_port = port;

    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_speed_set: u=%d p=%d sp=%d\n",
        unit, port, speed);
    
    /* here for ROBO chip SW override setting about the MAC speed only.
     *  and the sw_override field won't be set for this field has be set 
     *  already when set the MAC link bit.
     */
    if (IS_FE_PORT(unit, port)){
        reg_index = INDEX(STS_OVERRIDE_Pr);
        if (SOC_IS_TBX(unit)){
            field_index = INDEX(SPEEDf);
        } else {
            field_index = INDEX(SPEED_Rf);
        }
        
        if ((speed != 100) && (speed != 10)){
            return  SOC_E_PARAM;
        }

        /* bcm5324 speed field valuse is invered as normal */
        if (SOC_IS_ROBO5324(unit)){
            temp = (speed == 100) ? 0 : 1;
        } else {
            temp = (speed == 100) ? 1 : 0;
        }

        if (SOC_IS_ROBO53101(unit)) {
            reg_index = INDEX(STS_OVERRIDE_GMIIPr);
            field_index = INDEX(SPEEDf);
        }
        
    } else if (IS_GE_PORT(unit, port)){
        /* up to 2500 speed for ROBO GE MAC (2.5G for S_PORT only)*/
        if ((speed != 2500) && (speed != 1000) && 
                (speed != 100) && (speed != 10)){
            return  SOC_E_PARAM;
        }
        if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit)){
            reg_index = INDEX(STS_OVERRIDE_GMIIPr);
            field_index = INDEX(SPEED_Rf);
        } else if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
            reg_index = INDEX(STS_OVERRIDE_GMIIPr);
            field_index = INDEX(SPEEDf);
        } else {
            reg_index = INDEX(STS_OVERRIDE_GPr);
            if (SOC_IS_TBX(unit)){
                field_index = INDEX(SPEEDf);
            } else {
                field_index = INDEX(GIGA_SPEEDf);
            }
        }
        
        if (IS_S_PORT(unit, port)){
            temp = (speed == 2500) ? 0x3 : 
                    ((speed == 1000) ? 0x2 : 
                    ((speed == 100) ? 0x1 : 0x0));
        } else {
            if (speed == 2500){
                return  SOC_E_PARAM;
            }
            temp = (speed == 1000) ? 0x2 : 
                        ((speed == 100) ? 0x1 : 0x0);
        }
    } else {
        soc_cm_debug(DK_WARN,"%s: u=%d p=%d is incorrect!\n",
                FUNCTION_NAME(), unit, port);
        return SOC_E_PARAM;
    }

    OVERRIDE_LOCK(unit);
    /* Read Port Override Register */ 
    reg_addr = DRV_REG_ADDR(unit, reg_index, temp_port, 0);
    reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
    if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
        OVERRIDE_UNLOCK(unit);
        return rv;
    }
    
    DRV_REG_FIELD_SET(unit, reg_index, &reg_value, field_index, &temp);
    
    /* Write Port Override Register */
    if ((rv = DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len)) < 0) {
        OVERRIDE_UNLOCK(unit);
        return rv;
    }
    OVERRIDE_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *  mac_ge_speed_get
 * Purpose:
 *  Get GE MAC speed
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - Port number on unit.
 *  speed - (OUT) speed in Mb (0/10/100/1000)
 * Returns:
 *  SOC_E_NONE
 */

STATIC int
drv_mac_fe_ge_speed_get(int unit, soc_port_t port, int *speed)
{
    uint32  addr = DRV_REG_ADDR(unit, INDEX(SPDSTSr), 0, 0);
    uint32  reg_len = 0;
    uint64  temp64;
    uint64  val64;
    uint32  temp = 0;
    uint32  temp32 = 0;
    int     shift = 0;
    int     rv = SOC_E_NONE;
    
    COMPILER_64_ZERO(temp64);
    COMPILER_64_ZERO(val64);

    reg_len = DRV_REG_LENGTH_GET(unit, SPDSTSr);
    if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5389(unit)|| \
        SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit) ||
        SOC_IS_ROBO5395(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        if ((rv = DRV_REG_READ(unit, addr, (uint32 *)&temp32, reg_len)) < 0) {
            return rv;
        }
    } else if (SOC_IS_ROBO5396(unit) || SOC_IS_ROBO5348(unit) ||
        SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit) || SOC_IS_TBX(unit)) {
        if ((rv = DRV_REG_READ(unit, addr, (uint32 *)&temp64, reg_len)) < 0) {
            return rv;
        }
        
        if (SOC_IS_TBX(unit)){
#define _TB_SPD_STATUS_MASK_PORT   0x3

            /* retrieve the speed status of the port */
            COMPILER_64_SET(val64, 0, _TB_SPD_STATUS_MASK_PORT);
            COMPILER_64_SHR(temp64, port*2);
            COMPILER_64_AND(temp64, val64);
            temp32 = COMPILER_64_LO(temp64);
            
            *speed = 0;
            if (IS_FE_PORT(unit, port)){
                *speed = (temp32 == 0) ? 10 : 
                        ((temp32 == 1) ? 100 : 0);
                if (*speed == 0){
                    return SOC_E_INTERNAL;
                }
            } else if (IS_GE_PORT(unit, port)){
                *speed = (temp32 == 0) ? 10 : 
                        ((temp32 == 1) ? 100 : 
                        ((temp32 == 2) ? 1000 : 2500));
            } else {
                return SOC_E_PARAM;
            }
            return SOC_E_NONE;
        }
    }
    
    if (IS_CPU_PORT(unit, port)){
        if (SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5389(unit)|| \
            SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit) ||
            SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53115(unit) || 
            SOC_IS_ROBO53118(unit)){
            *speed = (temp32 & (1 << port)) ? 100 : 10;
        } else if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
            SOC_IS_ROBO53128(unit)) {
             temp32 = (temp32 >> (port * 2)) & 3;
            *speed = (temp32 == 2) ? 1000 : 
                    (temp32 == 1) ? 100 : 10;
        } else if (SOC_IS_ROBO5396(unit)){
            shift = port * 2;
            /* temp64 = (temp64 >> shift) & 3; */
            COMPILER_64_SET(val64, 0, 3);
            COMPILER_64_SHR(temp64, shift);
            COMPILER_64_AND(temp64, val64);
            COMPILER_64_TO_32_LO(temp, temp64) ;
            if (temp == 3){
                return SOC_E_FAIL;
            }
            *speed = (temp == 2) ? 1000 : 
                    (temp == 1) ? 100 : 10;
        } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                   SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
            if (SOC_IS_ROBO5348(unit)) {
                /*  temp64 = (temp64 >> NUM_FE_PORT(unit)) & 0x3; */
                COMPILER_64_SET(val64, 0, 3);
                COMPILER_64_SHR(temp64, NUM_FE_PORT(unit));
                COMPILER_64_AND(temp64, val64);                
            } else { /* 5347 */
                /* temp64 = (temp64 >> (NUM_FE_PORT(unit) + 24)) & 0x3; */
                COMPILER_64_SET(val64, 0, 3);
                COMPILER_64_SHR(temp64, (NUM_FE_PORT(unit)+24));
                COMPILER_64_AND(temp64, val64);                
            }
            COMPILER_64_TO_32_LO(temp, temp64) ;
            if (temp == 3){
                return SOC_E_FAIL;
            }
            *speed = (temp == 2) ? 1000 : 
                    (temp == 1) ? 100 : 10;
        } else {
            return SOC_E_RESOURCE;
        }
    } else if (IS_FE_PORT(unit, port)) {
        if (SOC_IS_ROBO5324(unit)){
        /* 10/100Mb port */
            if (temp32 & (1 << port)) {
                *speed = 100;
            } else {
                *speed = 10;
            }
        } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                   SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
            /* 10/100Mb port */
            if (SOC_IS_ROBO5348(unit)) {
                /* temp64 = (temp64 >> port) & 0x1; */
                COMPILER_64_SET(val64, 0, 1);
                COMPILER_64_SHR(temp64, port);
                COMPILER_64_AND(temp64, val64);                
            } else { /* 5347 */
                /* temp64 = (temp64 >> (port + 24)) & 0x1; */
                COMPILER_64_SET(val64, 0, 1);
                COMPILER_64_SHR(temp64, (port+24));
                COMPILER_64_AND(temp64, val64);                
            }
            COMPILER_64_TO_32_LO(temp, temp64) ;
            if (temp == 1) {
                *speed = 100;
            } else {
                *speed = 10;
            }
        } else if (SOC_IS_ROBO53101(unit)) {
             temp32 = (temp32 >> (port * 2)) & 3;
            *speed = (temp32 == 2) ? 1000 : 
                    (temp32 == 1) ? 100 : 10;
        }
    } else if (IS_GE_PORT(unit, port)){
        if (SOC_IS_ROBO5324(unit)){
            shift = 25 + (port - 25) * 2;
            if (temp32 == 3){
                return SOC_E_FAIL;
            }
            temp32 = (temp32 >> shift) & 3;
            *speed = (temp32 == 2) ? 1000 : 
                    (temp32 == 1) ? 100 : 10;
        } else if (SOC_IS_ROBO5396(unit)){
            shift = port * 2;
            /* temp64 = (temp64 >> shift) & 3; */
            COMPILER_64_SET(val64, 0, 3);
            COMPILER_64_SHR(temp64, shift);
            COMPILER_64_AND(temp64, val64);
            COMPILER_64_TO_32_LO(temp, temp64) ;

            if (temp == 3){
                return SOC_E_FAIL;
            }
            *speed = (temp == 2) ? 1000 : 
                    (temp == 1) ? 100 : 10;
        } else if(SOC_IS_ROBO5389(unit)||SOC_IS_ROBO5395(unit) ||
                  SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit) || 
                  SOC_IS_ROBO53115(unit) ||SOC_IS_ROBO53118(unit) ||
                  SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) { 
            shift = port * 2;
            if (temp32 == 3){
                return SOC_E_FAIL;
            }
            temp32 = (temp32 >> shift) & 3;
            *speed = (temp32 == 2) ? 1000 : 
                    (temp32 == 1) ? 100 : 10;
        } else if(SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                  SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) { 
            if (SOC_IS_ROBO5348(unit)) {
            shift = 50 + (port - 49) * 2;
            } else { /* 5347, 53242 */
                shift = 50 + (port - 25) * 2;
            }
            /* temp64 = (temp64 >> shift) & 3; */
            COMPILER_64_SET(val64, 0, 3);
            COMPILER_64_SHR(temp64, shift);
            COMPILER_64_AND(temp64, val64);
            COMPILER_64_TO_32_LO(temp, temp64) ;
            if (temp == 3){
                return SOC_E_FAIL;
            }
            *speed = (temp == 2) ? 1000 : 
                    (temp == 1) ? 100 : 10;
        }else {
            return SOC_E_RESOURCE;
        }

    } else {
        return SOC_E_PARAM;
    }
    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_speed_get: u=%d p=%d sp=%d\n", 
        unit, port, *speed);
    return SOC_E_NONE;
}

#define _TB_XOFF_STATUS_MASK_PORT   0x3
/*
 * Function:
 *  mac_ge_pause_set
 * Purpose:
 *  Configure GE MAC to transmit/receive pause frames.
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit.
 *  pause_tx - Boolean: transmit pause, or -1 (don't change)
 *  pause_rx - Boolean: receive pause, or -1 (don't change)
 * Returns:
 *  SOC_E_XXX
 *
 */

STATIC  int
drv_mac_fe_ge_pause_set(int unit, soc_port_t port, int pause_tx, int pause_rx)
{
    uint32  reg_index, overrid_findex, field_index;
    uint32  reg_len, reg_addr = 0;
    uint32  reg_value = 0, shift_t = 0, temp;
    uint64  reg_value64, temp64;
    int     qpause_sync = -1;
    int     rv = SOC_E_NONE;
    
    COMPILER_64_ZERO(reg_value64);
    COMPILER_64_ZERO(temp64);

    soc_cm_debug(DK_VERBOSE,
        "drv_mac_fe_ge_pause_set: u=%d p=%d rxpause=0x%x txpause=0x%x\n",
        unit, port, pause_rx, pause_tx);
        
    if (SOC_IS_ROBO5324(unit)){
        if (!((pause_tx == -1) && (pause_rx == -1))){
            reg_index = SW_FLOW_CONr;
            reg_addr = DRV_REG_ADDR(unit, reg_index, 0, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
            field_index = SW_FLOE_CON_ENf;
            if ((rv = DRV_REG_READ(unit, reg_addr, 
                    &reg_value, reg_len)) < 0) {
                return rv;
            }
            
            if ((IS_FE_PORT(unit, port)) || (IS_CPU_PORT(unit,port))) { 
                /* the rx/tx pause setting should be the same at FE port
                 *   - here for robo we use tx pause value first for pause 
                 *      setting 
                 */
                if (pause_tx != -1){
                    if (pause_tx){
                        reg_value |= (1 << port);
                        qpause_sync = 1;
                    } else {
                        reg_value &= ~(uint32)(1 << port);
                        qpause_sync = 0;
                    }
                } else {
                    
                    return SOC_E_NONE;
                    /*
                    if (pause_rx){
                        reg_value |= (1 << port);
                        qpause_sync = 1;
                    } else {
                        reg_value &= ~(uint32)(1 << port);
                        qpause_sync = 0;
                    }
                    */
                }
            } else if (IS_GE_PORT(unit, port)){
                /* 
                 * the shift_t is for setting GE port's rx/tx pause and such 
                 *  definition currently is only for Robo5324. Please check 
                 *  the Data Sheet on Page 0x0 for such register spec.
                 */
                
                shift_t = (port == 25) ? 0 : 1;
                qpause_sync = 0;
                if (pause_rx != -1) {
                    if (pause_rx){
                        reg_value |= (1 << (port + shift_t));
                        qpause_sync = 1;
                    } else {
                        reg_value &= ~(uint32)(1 << (port + shift_t));
                    }
                }
                if (pause_tx != -1){
                    shift_t += 1;
                    if (pause_tx){
                        reg_value |= (1 << (port + shift_t));
                        qpause_sync |= 1;
                    } else {
                        reg_value &= ~(uint32)(1 << (port + shift_t));
                    }
                } 
    
            } else {
                return SOC_E_PARAM;
            }
            
            /* set the sw flow control global enable bit
             *   - set it each time if any port flow control been enable, but 
             *     never disable the global enable bit.
             */
            if (qpause_sync){
                temp = 1;
                DRV_REG_FIELD_SET(unit, reg_index, &reg_value, 
                        field_index, &temp);
            }
                
            /* Write SW_FLOW_CONr Register */
            if ((rv = DRV_REG_WRITE(unit, reg_addr, 
                    &reg_value, reg_len)) < 0) {
                return rv;
            }
            
            /* Sync. 1Q base flow control with above setting 
             *      - Here for Robo we check the Tx pause setting only.
             *      - to prevent set GE port rx-pause off only but affecct 
             *          Q_Pause been reset)
             */
            if (qpause_sync != -1){
                reg_addr = DRV_REG_ADDR(unit, QOS_PAUSE_ENr, 0, 0);
                reg_len = DRV_REG_LENGTH_GET(unit, QOS_PAUSE_ENr);
                reg_value = 0;
                if ((rv = DRV_REG_READ(unit, reg_addr, 
                        &reg_value, reg_len)) < 0) {
                    return rv;
                }
                
                if (qpause_sync){
                    reg_value |= (1 << port);
                } else {
                    if (IS_GE_PORT(unit, port) && (pause_tx != -1))
                    reg_value &= ~(uint32)(1 << port);
                }
                
                /* Write QOS_PAUSE_ENr Register */
                if ((rv = DRV_REG_WRITE(unit, reg_addr, 
                        &reg_value, reg_len)) < 0) {
                    return rv;
                }
            }
            
        }
    } else if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5398(unit)|| 
            SOC_IS_ROBO5397(unit) || SOC_IS_ROBO5395(unit) || 
            SOC_IS_ROBO_ARCH_VULCAN(unit)){
        if (soc_feature(unit, soc_feature_robo_sw_override)) {
            
            /* check if pause setting is not requirred */
            if (((pause_tx == -1) && (pause_rx == -1))){
                return SOC_E_NONE;
            }
            
            if (IS_GE_PORT(unit, port)){
                if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit)){
                    reg_index = STS_OVERRIDE_GMIIPr;
                    overrid_findex = SW_OVERRIDE_Rf;
                } else if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                    reg_index = STS_OVERRIDE_GMIIPr;
                    overrid_findex = INDEX(SW_OVERRIDEf);
                } else  {
                    reg_index = STS_OVERRIDE_GPr;
                    overrid_findex = SW_ORDf;
                }
            } else if (IS_FE_PORT(unit, port) && SOC_IS_ROBO53101(unit)) {
                reg_index = STS_OVERRIDE_GMIIPr;
                overrid_findex = INDEX(SW_OVERRIDEf);
            } else {
                return SOC_E_PARAM;
            }
            
            OVERRIDE_LOCK(unit);
            /* Read Port Override Register */ 
            reg_addr = DRV_REG_ADDR(unit, reg_index, port, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
            if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
                OVERRIDE_UNLOCK(unit);
                return rv;
            }
            
            temp = 1;
            DRV_REG_FIELD_SET(unit, reg_index, &reg_value, 
                    overrid_findex, &temp);
    
            /* set TxPause */
            if (pause_tx != -1){
                field_index = TXFLOW_CNTLf;
               
                temp = pause_tx ? 1 : 0;
                DRV_REG_FIELD_SET(unit, reg_index, &reg_value, 
                        field_index, &temp);
            }
            
            /* set RxPause */
            if (pause_rx != -1){
                field_index = RXFLOW_CNTLf;
               
                temp = pause_rx ? 1 : 0;
                DRV_REG_FIELD_SET(unit, reg_index, &reg_value, 
                        field_index, &temp);
            }
            
            /* Write Port Override Register */
            if ((rv = DRV_REG_WRITE(unit, reg_addr, 
                    &reg_value, reg_len)) < 0) {
                OVERRIDE_UNLOCK(unit);
                return rv;
            }
            OVERRIDE_UNLOCK(unit);
        } else {
            /* not designed for the pause setting without SW override mode */
        }
    } else if (SOC_IS_TBX(unit)){
        uint32  old_val = 0;
        
        /* check if pause setting is not required */
        if (((pause_tx == -1) && (pause_rx == -1))){
            return SOC_E_NONE;
        }

        /* two bits constructed for a port's XOFF :
         *  - FE port : only bit 0 is available
         *  - CPU and GE port : bit0 for Rx_Pause, bit1 for Tx_Pause
         */
         
        SOC_IF_ERROR_RETURN(
                REG_READ_SW_XOFF_PORT_CTLr(unit, &reg_value64));

        /* get the original setting */
        old_val = (COMPILER_64_BITTEST(reg_value64, (port*2))) ? 0x1 : 0;
        old_val |= (COMPILER_64_BITTEST(reg_value64, (port*2+1))) ? 0x2 : 0;

        /* clear original setting of the port */
        COMPILER_64_SET(temp64, 0, 0x3); 
        COMPILER_64_SHL(temp64, port * 2);
        COMPILER_64_NOT(temp64);
        COMPILER_64_AND(reg_value64, temp64);
        
        temp = 0x0;
        if (IS_FE_PORT(unit, port)){
            if (pause_tx != -1){
                temp |= (pause_tx) ? 0x1 : 0x0;
            }
            if (pause_rx != -1){
                temp |= (pause_rx) ? 0x1 : 0x0;
            }
        } else {    /* CPU and GE */
            if (pause_rx != -1){
                temp |= (pause_rx) ? 0x1 : 0x0;
            } else {
                temp |= old_val & 0x1;
            }
            
            if (pause_tx != -1){
                temp |= (pause_tx) ? 0x2 : 0x0;
            } else {
                temp |= old_val & 0x2;
            }
        }
        
        /* set the setting of the port */
        COMPILER_64_SET(temp64, 0, temp); 
        COMPILER_64_SHL(temp64, port * 2);
        COMPILER_64_OR(reg_value64, temp64);
        
        SOC_IF_ERROR_RETURN(REG_WRITE_SW_XOFF_PORT_CTLr(
                unit, (uint32 *)&reg_value64));
       
    } else if (SOC_IS_ROBO5348(unit) ||SOC_IS_ROBO5347(unit) ||
        SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){
        if (!((pause_tx == -1) && (pause_rx == -1))){
            reg_index = INDEX(SW_FLOW_CONr);
            reg_addr = DRV_REG_ADDR(unit, reg_index, 0, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
            field_index = INDEX(SW_FLOE_CON_ENf);
            if ((rv = DRV_REG_READ(unit, reg_addr, 
                    (uint32 *)&reg_value64, reg_len)) < 0) {
                return rv;
            }
            
            if (IS_FE_PORT(unit, port)) { 
                /* the rx/tx pause setting should be the same at FE port
                 *   - here for robo we use tx pause value first for pause 
                 *      setting 
                 */
                if (pause_tx != -1){
                    if (pause_tx){
                        COMPILER_64_SET(temp64, 0, 1);
                        if (SOC_IS_ROBO5348(unit)) {
                        COMPILER_64_SHL(temp64, port);
                        } else { /* 5347, 53242 */
                            COMPILER_64_SHL(temp64, port+24);
                        }
                        COMPILER_64_OR(reg_value64, temp64);
                        qpause_sync = 1;
                    } else {
                        COMPILER_64_SET(temp64, 0, 1);
                        if (SOC_IS_ROBO5348(unit)) {
                        COMPILER_64_SHL(temp64, port);
                        } else { /* 5347, 53242 */
                            COMPILER_64_SHL(temp64, port+24);
                        }
                        COMPILER_64_NOT(temp64);
                        COMPILER_64_AND(reg_value64, temp64);
                        qpause_sync = 0;
                    }
                } else {
                    
                    return SOC_E_NONE;
                    /*
                    if (pause_rx){
                        reg_value |= (1 << port);
                        qpause_sync = 1;
                    } else {
                        reg_value &= ~(uint32)(1 << port);
                        qpause_sync = 0;
                    }
                    */
                }
            } else if (IS_CPU_PORT(unit, port) || IS_GE_PORT(unit, port)){
                /* 
                 * the shift_t is for setting GE port's rx/tx pause and such 
                 *  definition currently is only for Robo5324. Please check 
                 *  the Data Sheet on Page 0x0 for such register spec.
                 */
                if (SOC_IS_ROBO5348(unit)) {
                    shift_t = NUM_FE_PORT(unit) + 
                            (2 * (port - NUM_FE_PORT(unit)));
                } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) { 
                    /* 
                     * There are 3 value of NUM_FE_PORT(), 
                     * 8, 16 and 24 for BCM53202/212/242 respectively.
                     * So, add hardcode NUM_FE_PORT(unit) as "24" here. 
                     */
                    shift_t = 24 + 24 + (2 * (port - 24));  
                } else { /* 5347 */
                    shift_t = NUM_FE_PORT(unit) + 24 + 
                            (2 * (port - NUM_FE_PORT(unit)));
                }
                qpause_sync = 0;
                if (pause_rx != -1) {
                    if (pause_rx){
                        COMPILER_64_SET(temp64, 0, 1);
                        COMPILER_64_SHL(temp64, shift_t);
                        COMPILER_64_OR(reg_value64, temp64);
                        qpause_sync = 1;
                    } else {
                        COMPILER_64_SET(temp64, 0, 1);
                        COMPILER_64_SHL(temp64, shift_t);
                        COMPILER_64_NOT(temp64);
                        COMPILER_64_AND(reg_value64, temp64);
                    }
                }
                if (pause_tx != -1){
                    shift_t += 1;
                    if (pause_tx){
                        COMPILER_64_SET(temp64, 0, 1);
                        COMPILER_64_SHL(temp64, shift_t);
                        COMPILER_64_OR(reg_value64, temp64);
                        qpause_sync |= 1;
                    } else {
                        COMPILER_64_SET(temp64, 0, 1);
                        COMPILER_64_SHL(temp64, shift_t);
                        COMPILER_64_NOT(temp64);
                        COMPILER_64_AND(reg_value64, temp64);
                    }
                } 
    
            } else {
                return SOC_E_PARAM;
            }
            
            /* set the sw flow control global enable bit
             *   - set it each time if any port flow control been enable, but 
             *     never disable the global enable bit.
             */
            if (qpause_sync){
                temp = 1;
                DRV_REG_FIELD_SET(unit, reg_index, 
                        (uint32 *)&reg_value64, field_index, &temp);
            }
                
            /* Write SW_FLOW_CONr Register */
            if ((rv = DRV_REG_WRITE(unit, reg_addr, 
                    (uint32 *)&reg_value64, reg_len)) < 0) {
                return rv;
            }
            
            /* Sync. 1Q base flow control with above setting 
             *      - Here for Robo we check the Tx pause setting only.
             *      - to prevent set GE port rx-pause off only but affecct 
             *          Q_Pause been reset)
             */
            if (qpause_sync != -1){
                reg_addr = DRV_REG_ADDR(unit, QOS_PAUSE_ENr, 0, 0);
                reg_len = DRV_REG_LENGTH_GET(unit, QOS_PAUSE_ENr);
                COMPILER_64_ZERO(reg_value64);
                if ((rv = DRV_REG_READ(unit, reg_addr, 
                        (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
                
                if (qpause_sync){
                    COMPILER_64_SET(temp64, 0, 1);
                    if (SOC_IS_ROBO5348(unit)) {
                    COMPILER_64_SHL(temp64, port);
                    } else { /* 5347, 53242 */
                        COMPILER_64_SHL(temp64, port+24);
                    }
                    COMPILER_64_OR(reg_value64, temp64);
                } else {
                    if ((IS_CPU_PORT(unit, port) || IS_GE_PORT(unit, port)) \
                        && (pause_tx != -1)) {
                        COMPILER_64_SET(temp64, 0, 1);
                        COMPILER_64_SHL(temp64, shift_t);
                        COMPILER_64_NOT(temp64);
                        COMPILER_64_AND(reg_value64, temp64);
                    }
                }
                
                /* Write QOS_PAUSE_ENr Register */
                if ((rv = DRV_REG_WRITE(unit, reg_addr, 
                        (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
            }
            
        }
    } else {
            return SOC_E_RESOURCE;
    }
    
    return SOC_E_NONE;
}

/*
 * Function:
 *  mac_ge_pause_get
 * Purpose:
 *  Return current GE MAC transmit/receive pause frame configuration.
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit.
 *  pause_tx - (OUT) TRUE is TX pause enabled.
 *  pause_rx - (OUT) TRUE if obey RX pause enabled.
 * Returns:
 *  SOC_E_XXX
 */

STATIC  int
drv_mac_fe_ge_pause_get(int unit, soc_port_t port, 
        int *pause_tx, int *pause_rx)
{
    uint32  reg_len = 0, addr = 0;
    uint32  temp = 0, reg_value = 0;
    uint64  reg_value64, temp64;
    int     shift = 0;
    int     rv = SOC_E_NONE;

    COMPILER_64_ZERO(reg_value64);
    COMPILER_64_ZERO(temp64);

    if (SOC_IS_ROBO5324(unit)){
        soc_cm_debug(DK_VERBOSE,
            "drv_mac_fe_ge_pause_get: bcm5324..\n");
        addr = DRV_REG_ADDR(unit, SW_FLOW_CONr, 0, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, SW_FLOW_CONr);
        if ((rv = DRV_REG_READ(unit, addr, &temp, reg_len)) < 0) {
            return rv;
        }
        if ((IS_FE_PORT(unit, port)) || (IS_CPU_PORT(unit,port))) { 
            /* 10/100Mb port */
            if (temp & (1 << port)) {
                *pause_tx = TRUE;
                *pause_rx = TRUE;
            } else {
                *pause_tx = FALSE;
                *pause_rx = FALSE;
            }
        } else if (IS_GE_PORT(unit, port)){ /*giga port */
            shift = 25 + (port - 25) * 2;
            temp = (temp >> shift) & 3;
            switch (temp) {
                case 0:
                    *pause_tx = FALSE;
                    *pause_rx = FALSE;
                    break;
                case 1:
                    *pause_tx = FALSE;
                    *pause_rx = TRUE;
                    break;
                case 2:
                    *pause_tx = TRUE;
                    *pause_rx = FALSE;
                    break;
                case 3:
                    *pause_tx = TRUE;
                    *pause_rx = TRUE;
                    break;
                default :
                    return SOC_E_FAIL;
            }
        } else {
            return SOC_E_PARAM;
        }
    } else if (SOC_IS_ROBO5396(unit)){

        if (IS_GE_PORT(unit, port) || IS_CPU_PORT(unit,port)) {
            addr = DRV_REG_ADDR(unit, TXPAUSESTSr, 0, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, TXPAUSESTSr);
            if ((rv = DRV_REG_READ(unit, addr, &temp, reg_len)) < 0) {
                return rv;
            }
                
            *pause_tx = (temp & (1 << port)) ? TRUE : FALSE;
            soc_cm_debug(DK_VERBOSE,"\t TxPause=%d\n",*pause_tx);
            
            addr = DRV_REG_ADDR(unit, RXPAUSESTSr, 0, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, RXPAUSESTSr);
            if ((rv = DRV_REG_READ(unit, addr, &temp, reg_len)) < 0) {
                return rv;
            }
            *pause_rx = (temp & (1 << port)) ? TRUE : FALSE;
            soc_cm_debug(DK_VERBOSE,"\t RxPause=%d\n",*pause_rx);
        } else {
            return SOC_E_PARAM;
        }
    }else if(SOC_IS_ROBO5389(unit)||SOC_IS_ROBO5398(unit)|| 
            SOC_IS_ROBO5397(unit) || SOC_IS_ROBO5395(unit) || 
            SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        if (IS_GE_PORT(unit, port) || IS_CPU_PORT(unit,port) ||
            SOC_IS_ROBO53101(unit)) {
            addr = DRV_REG_ADDR(unit, PAUSESTSr, 0, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, PAUSESTSr);
            if ((rv = DRV_REG_READ(unit, addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
                DRV_REG_FIELD_GET(unit, PAUSESTSr, 
                        &reg_value, PAUSE_STSf, &temp);
                    
                *pause_tx = (temp & (1 << port)) ? TRUE : FALSE;
                /* hard coded below for register specific bit mask defined */
                temp = temp >> 9;  
                *pause_rx = (temp & (1 << port)) ? TRUE : FALSE;
                soc_cm_debug(DK_VERBOSE,"\t TxPause=%d,RxPause=%d\n",
                        *pause_tx,*pause_rx);
            } else {
                DRV_REG_FIELD_GET(unit, PAUSESTSr, &reg_value, 
                        TXPAUSE_STSf, &temp);
                    
                *pause_tx = (temp & (1 << port)) ? TRUE : FALSE;
                soc_cm_debug(DK_VERBOSE,"\t TxPause=%d\n",*pause_tx);
                
                DRV_REG_FIELD_GET(unit, PAUSESTSr, &reg_value, 
                        RXPAUSE_STSf, &temp);
    
                *pause_rx = (temp & (1 << port)) ? TRUE : FALSE;
                soc_cm_debug(DK_VERBOSE,"\t RxPause=%d\n",*pause_rx);
            }
        } else {
                return SOC_E_PARAM;
        }
    } else if (SOC_IS_TBX(unit)){
        /* in TB, all port's flow control status were deisgned to report
         * status through the same bits length(two bits) no matter the port 
         * is GE or FE.
         */
        SOC_IF_ERROR_RETURN(
                REG_READ_PORT_XOFF_STSr(unit, (uint32 *)&reg_value64));
        COMPILER_64_SHR(reg_value64, port*2);
        temp = COMPILER_64_LO(reg_value64);
        switch (temp & _TB_XOFF_STATUS_MASK_PORT){
            case 0 :
                *pause_tx = FALSE;
                *pause_rx = FALSE;
                break;
            case 1 :
                *pause_tx = FALSE;
                *pause_rx = TRUE;
                break;
            case 2 :
                *pause_tx = TRUE;
                *pause_rx = FALSE;
                break;
            case 3 :
                *pause_tx = TRUE;
                *pause_rx = TRUE;
                break;
            default :
                return SOC_E_INTERNAL;
        }
        
        /* if the second bit of the XOFF status for FE port is reserved */
        if (IS_FE_PORT(unit, port)){
            *pause_tx = *pause_rx;
        }
                
    } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
        SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){

        soc_cm_debug(DK_VERBOSE,
            "drv_mac_fe_ge_pause_get: bcm5348..\n");
        addr = DRV_REG_ADDR(unit, SW_FLOW_CONr, 0, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, SW_FLOW_CONr);
        if ((rv = DRV_REG_READ(unit, addr, 
                (uint32 *)&reg_value64, reg_len)) < 0) {
            return rv;
        }

        if (IS_FE_PORT(unit, port)) { 
            /* 10/100Mb port */
            COMPILER_64_SET(temp64, 0, 1);
            if (SOC_IS_ROBO5348(unit)) {
            COMPILER_64_SHL(temp64, port);
            } else { /* 5347, 53242 */
                COMPILER_64_SHL(temp64, port+24);
            }
            COMPILER_64_AND(reg_value64, temp64);
            if (!COMPILER_64_IS_ZERO(reg_value64)) {
                *pause_tx = TRUE;
                *pause_rx = TRUE;
            } else {
                *pause_tx = FALSE;
                *pause_rx = FALSE;
            }
        } else if (IS_CPU_PORT(unit, port) || 
                IS_GE_PORT(unit, port)){ /*giga port */

            if (SOC_IS_ROBO5348(unit)) {
                shift = NUM_FE_PORT(unit) + (port - NUM_FE_PORT(unit)) * 2;
            } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                /* 
                 * There are 3 value of NUM_FE_PORT(), 
                 * 8, 16 and 24 for BCM53202/212/242 respectively.
                 * So, add hardcode NUM_FE_PORT(unit) as "24" here. 
                 */
                shift = 24 + 24 + (2 * (port - 24));
            } else { /* 5347 */
                shift = NUM_FE_PORT(unit) + 24 + 
                        (port - NUM_FE_PORT(unit)) * 2;
            }

            COMPILER_64_SHR(reg_value64, shift);
            COMPILER_64_SET(temp64, 0, 3);
            COMPILER_64_AND(reg_value64, temp64);
   
            temp = COMPILER_64_LO(reg_value64);
            switch (temp) {
                case 0:
                    *pause_tx = FALSE;
                    *pause_rx = FALSE;
                    break;
                case 1:
                    *pause_tx = FALSE;
                    *pause_rx = TRUE;
                    break;
                case 2:
                    *pause_tx = TRUE;
                    *pause_rx = FALSE;
                    break;
                case 3:
                    *pause_tx = TRUE;
                    *pause_rx = TRUE;
                    break;
                default :
                    return SOC_E_FAIL;
            }

        } else {
            return SOC_E_PARAM;
        }
    }else {
        return SOC_E_RESOURCE;
    }

    soc_cm_debug(DK_VERBOSE,
        "drv_mac_fe_ge_pause_get: %x %x\n", *pause_tx, *pause_rx);

    return SOC_E_NONE;
}


/*
 * Function:
 *  mac_ge_pause_addr_get
 * Purpose:
 *  Set GE MAC source address for transmitted PAUSE frame
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit.
 *  pause_mac - MAC address used for pause transmission.
 * Returns:
 *  SOC_E_XXX
 */

STATIC  int
drv_mac_fe_ge_pause_addr_set(int unit, soc_port_t port, 
    sal_mac_addr_t pause_mac)
{
    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_pause_addr_set:\n");
    
    if (SOC_IS_TBX(unit)){
        uint64      reg64_value, reg_macaddr;
        
        SOC_IF_ERROR_RETURN(
                REG_READ_PAUSE_FM_SAr(unit, (uint32 *)&reg64_value));
        SAL_MAC_ADDR_TO_UINT64(pause_mac, reg_macaddr);
        soc_PAUSE_FM_SAr_field_set(unit, (uint32 *)&reg64_value, 
                PAUSE_SAf, (uint32 *)&reg_macaddr);
        SOC_IF_ERROR_RETURN(
                REG_WRITE_PAUSE_FM_SAr(unit, (uint32 *)&reg64_value));
        
        return SOC_E_NONE;
    } else {
        return SOC_E_UNAVAIL;
    }
}



/*
 * Function:
 *  mac_ge_pause_addr_get
 * Purpose:
 *  Return current GE MAC source address for transmitted PAUSE frames
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit.
 *  pause_mac - (OUT) MAC address used for pause transmission.
 * Returns:
 *  SOC_E_XXX
 */

STATIC  int
drv_mac_fe_ge_pause_addr_get(int unit, soc_port_t port, 
    sal_mac_addr_t pause_mac)
{
    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_pause_addr_get:\n");
    
    if (SOC_IS_TBX(unit)){
        uint64      reg64_value, reg_macaddr;
        
        COMPILER_64_ZERO(reg64_value);
        COMPILER_64_ZERO(reg_macaddr);
        
        SOC_IF_ERROR_RETURN(
                REG_READ_PAUSE_FM_SAr(unit, (uint32 *)&reg64_value));
        soc_PAUSE_FM_SAr_field_get(unit, (uint32 *)&reg64_value, 
                PAUSE_SAf, (uint32 *)&reg_macaddr);
        SAL_MAC_ADDR_FROM_UINT64(pause_mac, reg_macaddr);
        
        return SOC_E_NONE;
    } else {
        return SOC_E_UNAVAIL;
    }
    
}



/*
 * Function:
 *  mac_ge_loopback_set
 * Purpose:
 *  Set GE MAC into/out-of loopback mode
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - Port number on unit.
 *  loopback - Boolean: true -> loopback mode, false -> normal operation
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
drv_mac_fe_ge_loopback_set(int unit, soc_port_t port, int loopback)
{
    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_loopback_set\n");    
    if (loopback == TRUE){
        return SOC_E_UNAVAIL;
    }
    
    return SOC_E_NONE;
}

/*
 * Function:
 *  mac_ge_loopback_get
 * Purpose:
 *  Get current GE MAC loopback mode setting.
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit
 *  loopback - (OUT) Boolean: true = loopback, false = normal
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
drv_mac_fe_ge_loopback_get(int unit, soc_port_t port, int *loopback)
{
    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_loopback_get\n");
    *loopback = FALSE;    
    return SOC_E_NONE;
}



/*
 * Function:
 *  mac_ge_interface_set
 * Purpose:
 *  Set GE MAC interface type
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - Port number on unit.
 *  pif - one of SOC_PORT_IF_*
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
drv_mac_fe_ge_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_interface_set\n");  
    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *  mac_ge_interface_get
 * Purpose:
 *  Retrieve GE MAC interface type
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit.
 *  pif - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *  SOC_E_XXX
 * Note :
 */

STATIC int
drv_mac_fe_ge_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    int     rv = SOC_E_NONE;
    uint32  addr, reg_len, temp, ifset;
    
    if (IS_FE_PORT(unit, port)){
        *pif = SOC_PORT_IF_MII;
        
    /* Check Me: check the "Note" in front of this routine */    
    } else if (IS_CPU_PORT(unit, port)){
        if (SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5389(unit)||
            SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit) ||
            SOC_IS_ROBO5395(unit)){
            *pif = SOC_PORT_IF_MII;
        } else if (SOC_IS_ROBO5396(unit)){
            addr = DRV_REG_ADDR(unit, STRAP_VALUEr, 0, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, STRAP_VALUEr);
            if ((rv = DRV_REG_READ(unit, addr, &temp, reg_len)) < 0) {
                return rv;
            }
            switch ((temp >> 2) && 0x3){
                case 0 :
                    *pif = SOC_PORT_IF_RvMII;
                    break;
                case 1 :
                    *pif = SOC_PORT_IF_MII;
                    break;
                case 2 :
                    *pif = SOC_PORT_IF_GMII;
                    break;
                case 3 :
                    *pif = SOC_PORT_IF_RGMII;
            }
        } else if (SOC_IS_ROBO5348(unit) ||SOC_IS_ROBO5347(unit) ||
            SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) || 
            SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_TBX(unit)){
            *pif = SOC_PORT_IF_GMII;
        } else {
            return SOC_E_RESOURCE;
        }
        
    } else if (IS_GE_PORT(unit, port)){
        if (SOC_IS_ROBO5324(unit)){
            
            addr = DRV_REG_ADDR(unit, STRAP_STSr, 0, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, STRAP_STSr);
            if ((rv = DRV_REG_READ(unit, addr, &temp, reg_len)) < 0) {
                return rv;
            }
            if (port == 25) {
                DRV_REG_FIELD_GET(unit, STRAP_STSr, &temp, 
                        GIGA0_IFSELf, &ifset);
            }else if (port == 26) {
                DRV_REG_FIELD_GET(unit, STRAP_STSr, &temp, 
                        GIGA1_IFSELf, &ifset);
            }
            switch (ifset) {
                case 0:
                    *pif = SOC_PORT_IF_RGMII;
                    break;
                case 1:
                    *pif = SOC_PORT_IF_TBI;
                    break;
                case 2:
                    *pif = SOC_PORT_IF_GMII;
                    break;
                case 3:
                    *pif = SOC_PORT_IF_MII;
                    break;
                default:
                    return SOC_E_FAIL;
            }
        } else if (SOC_IS_ROBODINO(unit)){
            *pif = SOC_PORT_IF_SGMII;
        } else if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit) ||
            SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53118(unit) || 
            SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)){
            *pif = SOC_PORT_IF_GMII;
        } else if (SOC_IS_ROBO53115(unit)) {
            if (port == 5){
                *pif = SOC_PORT_IF_SGMII;
            } else {
                *pif = SOC_PORT_IF_GMII;
            }
        } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
            *pif = SOC_PORT_IF_SGMII;
        } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
            if (SOC_IS_ROBO53262(unit)) {
                *pif = SOC_PORT_IF_SGMII;
            } else {
                *pif = SOC_PORT_IF_RGMII;
            }
        } else if (SOC_IS_TBX(unit)){
            /* check the unimac design, the 2.5G serdes still be assigned 
             *  as GMII interface.
             *
             * For other GE ports in TB, there will be SGMII or RGMII  
             *  interface connected between MAC(or interface SerDes) and PHY 
             *  based on the differnece of the chip bounding. Here we still 
             *  assign SGMII or RGMII.
             * For Voyager, assgin 2.5G GMII port (port26) as S_PORT. 
             *  It's interface is GMII.
             */ 
            if (IS_S_PORT(unit, port)){
                *pif = SOC_PORT_IF_GMII;
            } else {
                if (IS_GMII_PORT(unit, port)){
                    *pif = SOC_PORT_IF_RGMII;
                } else {
                    *pif = SOC_PORT_IF_SGMII;
                }
            }
        } else {
            return SOC_E_RESOURCE;
        }
        
    } else {
        return SOC_E_PARAM;
    }
    
    soc_cm_debug(DK_VERBOSE,
        "drv_mac_fe_ge_interface_get : pif = %d\n", *pif);  
    
    return SOC_E_NONE;

}




/*
 * Function:
 *  mac_ge_ability_get
 * Purpose:
 *  Return the GE MAC abilities
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch Port # on unit.
 *  mode - (OUT) Mask of MAC abilities returned.
 * Returns:
 *  SOC_E_NONE
 */

STATIC  int
drv_mac_fe_ge_ability_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    if (IS_FE_PORT(unit, port)){
        /* Asymmetric pause not available (SOC_PM_PAUSE_ASYMM) */
        *mode = SOC_PM_MII | SOC_PM_PAUSE |SOC_PM_100MB |SOC_PM_10MB;
    } else if (IS_GE_PORT(unit, port)){
        *mode = SOC_PM_MII | SOC_PM_PAUSE |SOC_PM_100MB |SOC_PM_10MB
        | SOC_PM_1000MB_FD | SOC_PM_GMII | SOC_PM_TBI | SOC_PM_SGMII 
        | SOC_PM_PAUSE_ASYMM;
        /* in TB's new design, the Giga MAC allowed RGMII/SGMII/2.5G_SerDes */
        if (SOC_IS_TBX(unit)){
            *mode &= ~SOC_PM_TBI;
            if (IS_S_PORT(unit, port)){
                *mode = SOC_PM_2500MB_FD;
            }
        }
    } else if (IS_CPU_PORT(unit, port)){
        if (SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5348(unit) || 
                SOC_IS_ROBO5397(unit) || SOC_IS_ROBO5347(unit) || 
                SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53242(unit) || 
                SOC_IS_ROBO53262(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)){
            *mode = SOC_PM_MII | SOC_PM_PAUSE | SOC_PM_100MB | SOC_PM_10MB |
                    SOC_PM_1000MB_FD | SOC_PM_GMII | SOC_PM_TBI | 
                    SOC_PM_SGMII | SOC_PM_PAUSE_ASYMM;
        } else if (SOC_IS_TBX(unit)){
            *mode = SOC_PM_MII | SOC_PM_PAUSE |SOC_PM_100MB |SOC_PM_10MB |
                    SOC_PM_1000MB_FD | SOC_PM_GMII | SOC_PM_PAUSE_ASYMM;
        } else {
            *mode = SOC_PM_MII | SOC_PM_PAUSE |SOC_PM_100MB |SOC_PM_10MB;
        }
    } else {
        return SOC_E_PARAM;
    }
    
    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_ability_get: ability=0x%x\n", 
        *mode);
    return (SOC_E_NONE);
}

/*
 * Function:
 *      mac_fe_ge_ability_local_get
 * Purpose:
 *      Return the full abilities of FE GE MAC
 * Parameters:
 *      unit - RoboSwitch unit #.
 *      port - RoboSwitch port # on unit.
 *      mode - (OUT) Supported operating modes as a mask of abilities.
 * Returns:
 *      SOC_E_XXX
 */

STATIC  int
drv_mac_fe_ge_ability_local_get(int unit, soc_port_t port, 
        soc_port_ability_t *ability)
{
    uint16 dev_id= 0;
    uint8 rev_id = 0;
    char *s;

    if (IS_FE_PORT(unit, port)) {
        ability->speed_half_duplex  = SOC_PA_SPEED_10MB | SOC_PA_SPEED_100MB; 
        ability->speed_full_duplex  = SOC_PA_SPEED_10MB | SOC_PA_SPEED_100MB;
        ability->pause     = SOC_PA_PAUSE;
        ability->interface = SOC_PA_INTF_MII;
        ability->medium    = SOC_PA_ABILITY_NONE;
        ability->loopback  = SOC_PA_ABILITY_NONE;
        ability->flags     = SOC_PA_ABILITY_NONE;
        ability->encap     = SOC_PA_ENCAP_IEEE;
    } else if (IS_GE_PORT(unit, port)) {
        ability->speed_half_duplex = SOC_PA_SPEED_10MB | SOC_PA_SPEED_100MB;
        s = soc_property_get_str(unit, "board_name");
        if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
            ability->speed_full_duplex = SOC_PA_SPEED_10MB | SOC_PA_SPEED_100MB; 
        } else {
            ability->speed_full_duplex = SOC_PA_SPEED_10MB | SOC_PA_SPEED_100MB | 
                    SOC_PA_SPEED_1000MB; 
        }

        ability->pause = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;

        if (SOC_IS_ROBO5324(unit)||SOC_IS_ROBO53242(unit)) {
            ability->interface = SOC_PA_INTF_MII |SOC_PA_INTF_GMII | 
                    SOC_PA_INTF_RGMII |SOC_PA_INTF_TBI;
        }
        if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5348(unit)||
                SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53262(unit)) {
            ability->interface = SOC_PA_INTF_SGMII;
        }
        if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)||
                SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53118(unit) ||
                SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)){
            ability->interface = SOC_PA_INTF_MII |SOC_PA_INTF_GMII;
        }
        if (SOC_IS_ROBO53115(unit)) {
            if (port == 5){
                ability->interface = SOC_PA_INTF_GMII | SOC_PA_INTF_SGMII;
            }else {
                ability->interface = SOC_PA_INTF_MII |SOC_PA_INTF_GMII;
            }
        }
        if (SOC_IS_TBX(unit)) {
            /* In ESW unimac design, all GE port is assigned to GMII MAC 
             *  interface. (speed allowed on unimac is 10/100/1000/2500)
             */
            ability->interface = SOC_PA_INTF_GMII;
            if (IS_S_PORT(unit, port)){
                /* for the SGMII mode on 2.5G serdes can support upto 1G 
                 * speed only, we keep the MAC interface ability as GMII 
                 * still, and add 2500 speed ablity only.
                 */
                ability->speed_full_duplex = SOC_PA_SPEED_2500MB;
            } else {
            
                if (IS_GMII_PORT(unit, port)){
                    /* our design after SOC probed will assigned the 
                     *  si->gmii_pbm. That means the bounding for GE port is   
                     *  RGMII interface from MAC to PHY.
                     */
                    ability->interface = SOC_PA_INTF_RGMII;
                } else {
                    /* after SOC probed only RGMII bounding GE will assigned 
                     *  into si->gmii_pbm. For other bounding of TB's GE that 
                     *  si-ge_pmb assigned already but without si->gmii_pbm 
                     *  assignment, those GE is designed as SGMII beteen 
                     *  Internal SerDes to EXT_PHY.
                     */
                    ability->interface = SOC_PA_INTF_SGMII;
                }
            }
        }
        ability->medium    = SOC_PA_ABILITY_NONE;
        ability->loopback  = SOC_PA_ABILITY_NONE;
        ability->flags     = SOC_PA_ABILITY_NONE;
        ability->encap     = SOC_PA_ENCAP_IEEE;
    } else if (IS_CPU_PORT(unit, port)) {

        ability->speed_half_duplex  = SOC_PA_SPEED_10MB | SOC_PA_SPEED_100MB;
        ability->medium    = SOC_PA_ABILITY_NONE;
        ability->loopback  = SOC_PA_ABILITY_NONE;
        ability->flags     = SOC_PA_ABILITY_NONE;

        if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit) ||
            SOC_IS_ROBO5348(unit)|| SOC_IS_ROBO5347(unit) || 
            SOC_IS_ROBO5395(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {
            ability->speed_full_duplex  = SOC_PA_SPEED_10MB | 
                    SOC_PA_SPEED_100MB | SOC_PA_SPEED_1000MB; 
            ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
            ability->interface = SOC_PA_INTF_MII | SOC_PA_INTF_GMII; 
        }else if (SOC_IS_ROBO53262(unit)){
             soc_cm_get_id(unit, &dev_id, &rev_id);
            if ((dev_id == BCM53262_DEVICE_ID) && 
                    (rev_id == BCM53262_A0_REV_ID)) {
                ability->speed_full_duplex  = SOC_PA_SPEED_10MB | 
                        SOC_PA_SPEED_100MB;
                ability->pause     = SOC_PA_PAUSE;
                ability->interface = SOC_PA_INTF_MII;
            } else {
                ability->speed_full_duplex  = SOC_PA_SPEED_10MB | 
                        SOC_PA_SPEED_100MB | SOC_PA_SPEED_1000MB; 
                ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
                ability->interface = SOC_PA_INTF_MII | SOC_PA_INTF_GMII; 
            }
        } else if (SOC_IS_TBX(unit)){
            ability->speed_full_duplex  = SOC_PA_SPEED_10MB | 
                    SOC_PA_SPEED_100MB | SOC_PA_SPEED_1000MB; 
            ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
            ability->interface = SOC_PA_INTF_MII | SOC_PA_INTF_GMII; 
        } else {
           /* bcm5324 /53242 */
            ability->speed_full_duplex  = SOC_PA_SPEED_10MB | 
                    SOC_PA_SPEED_100MB;
            ability->pause     = SOC_PA_PAUSE;
            ability->interface = SOC_PA_INTF_MII;
        }
    } else {
        return SOC_E_PARAM;
    }

    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_ability_get\n");
    return (SOC_E_NONE);
}

STATIC int  
drv_mac_fe_ge_frame_max_set(int unit, soc_port_t port, int size)
{
    uint32  reg_addr, reg_value, temp, enable = 0;
    int reg_len, rv = SOC_E_NONE;
    uint16 dev_id;
    uint8 rev_id;
    uint32 ctrl_type = 0;
    uint32 ctrl_cnt = 0;

    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_frame_max_set\n"); 
    
    if (SOC_IS_ROBO5396(unit)) {
        /* 
         * Only support receive Jumbo frames or not 
         * It can't set the max frame size
         */
        if ((size != 1518) || (size != 9724)) {
            return SOC_E_UNAVAIL;
        }
        reg_addr = DRV_REG_ADDR(unit, JUMBO_PORT_MASKr, 0, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, JUMBO_PORT_MASKr);
        if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
         }
        /* Enable jumbo frame */
        temp = 0;
        DRV_REG_FIELD_GET(unit, JUMBO_PORT_MASKr, &reg_value, 
                JUMBO_FM_PORT_MASKf, &temp);
        if (size == 9724) {
            temp |= 0x1 << port;
        } else if (size == 1518) {
            temp &= ~(0x1 << port);
        } else {
            return SOC_E_PARAM;
        }
        DRV_REG_FIELD_SET(unit, JUMBO_PORT_MASKr, &reg_value, 
                JUMBO_FM_PORT_MASKf, &temp);
        if ((rv = DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
         }
    } else if ((SOC_IS_ROBO5389(unit)) || (SOC_IS_ROBO5398(unit)) ||
        (SOC_IS_ROBO5397(unit)) || (SOC_IS_ROBO5395(unit)) || 
        SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        if ((size < 1518) || (size > 9724)) {
            return SOC_E_UNAVAIL;
        }
        /* Set Good Frame max size */
        reg_addr = DRV_REG_ADDR(unit, MIB_GD_FM_MAX_SIZEr, 0, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, MIB_GD_FM_MAX_SIZEr);
        temp = size;
        
        /* Support feature 802.3as : either 1518 or 2000 bytes */
        if (soc_feature(unit, soc_feature_802_3as)) {
            if ((size == 1518) || (size == 2000)) {
                DRV_REG_FIELD_SET(unit, MIB_GD_FM_MAX_SIZEr, &reg_value, 
                        MAX_SIZEf, &temp);
                if ((rv = DRV_REG_WRITE(unit, reg_addr, 
                        &reg_value, reg_len)) < 0) {
                    return rv;
                 }
            }
        } else {
            DRV_REG_FIELD_SET(unit, MIB_GD_FM_MAX_SIZEr, &reg_value, 
                    MAX_SIZEf, &temp);
            if ((rv = DRV_REG_WRITE(unit, reg_addr, 
                    &reg_value, reg_len)) < 0) {
                return rv;
             }
        }
        /* Set jumbo frame mask */
        reg_addr = DRV_REG_ADDR(unit, JUMBO_PORT_MASKr, 0, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, JUMBO_PORT_MASKr);
        if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
         }
        /* Enable jumbo frame */
        temp = 0;
        DRV_REG_FIELD_GET(unit, JUMBO_PORT_MASKr, &reg_value, 
            JUMBO_FM_PORT_MASKf, &temp);

        /* Support feature 802.3as : If size <= 2000, disable jumbo frame 
         *  capability.
         * Others, If size = 1518, disable jumbo frame capability 
         */
        if (soc_feature(unit, soc_feature_802_3as)) {
            if (SOC_IS_ROBO53101(unit)) {
                if (size <= 2000) {
                    temp &= ~(0x1 << port);
                } else {
                    temp |= 0x1 << port;
                    /* Enable to receive and transmit jumbo */
                    enable = 1;
                    DRV_REG_FIELD_SET(unit, JUMBO_PORT_MASKr, &reg_value, 
                            EN_10_100_JUMBOf, &enable);
                }
            } else {
                if (size <= 2000) {
                    temp &= ~(0x1 << port);
                    /* Disable CPU(10/100) Port can receive and transmit jumbo */
                    if (IS_CPU_PORT(unit, port)) {
                        enable = 0;
                        DRV_REG_FIELD_SET(unit, JUMBO_PORT_MASKr, &reg_value, 
                            EN_10_100_JUMBOf, &enable);
                    }
                } else {
                    temp |= 0x1 << port;
                    /* Enable CPU(10/100) Port can receive and transmit jumbo */
                    if (IS_CPU_PORT(unit, port)) {
                        enable = 1;
                        DRV_REG_FIELD_SET(unit, JUMBO_PORT_MASKr, &reg_value, 
                            EN_10_100_JUMBOf, &enable);
                    }  
                }
            }
        } else {
            if (size == 1518) {
                temp &= ~(0x1 << port);
            } else {
                temp |= 0x1 << port;
            }
        }
        DRV_REG_FIELD_SET(unit, JUMBO_PORT_MASKr, &reg_value, 
                JUMBO_FM_PORT_MASKf, &temp);

        if ((rv = DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
         }
    } else if ((SOC_IS_ROBO53242(unit))||SOC_IS_ROBO53262(unit)) {
        soc_cm_get_id(unit, &dev_id, &rev_id);
        if ((rev_id == BCM53242_A0_REV_ID) || 
                (rev_id == BCM53262_A0_REV_ID)) {
            /* The feature is only available on chips after B0 version. */
            return SOC_E_UNAVAIL;
        } else {
            switch (size) {
                case 2048:
                    temp = 0x0;
                    break;
                case 1536:
                    temp = 0x1;
                    break;
                case 1518:
                    temp = 0x2;
                    break;
                case 2000:
                    temp = 0x3;
                    break;
                default:
                    return SOC_E_UNAVAIL;
            }
            reg_addr = DRV_REG_ADDR(unit, NEW_CONTROLr, 0, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, NEW_CONTROLr);
            if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, 
                    reg_len)) < 0) {
                return rv;
            }
            DRV_REG_FIELD_SET(unit, NEW_CONTROLr, &reg_value, 
                    MAX_RX_LIMITf, &temp);
            if ((rv = DRV_REG_WRITE(unit, reg_addr, 
                    &reg_value, reg_len)) < 0) {
                return rv;
            }
        }
    } else if ((SOC_IS_ROBO5324(unit)) || (SOC_IS_ROBO5348(unit)) || 
        (SOC_IS_ROBO5347(unit))) {
        return SOC_E_UNAVAIL;
    } else if (SOC_IS_TBX(unit)) {
        /* IEEE 802.3as-2006 defined the EnvelopFrameSize at 2000 octects to 
         *  cover original maxFrameSize(1518 for untag and 1522/1526/1532 for 
         *  tagged). And TB's design followed this new Spec. already.
         */
        if (SOC_IS_TB_AX(unit)) {
            if (size != _DRV_ROBO_FRAME_SIZE_2000){
                rv = SOC_E_UNAVAIL;
            } else {
                rv = SOC_E_NONE;
            }
        } else {
            switch(size) {
                case _DRV_ROBO_FRAME_SIZE_1518:
                case _DRV_ROBO_FRAME_SIZE_2000:
                case _DRV_ROBO_FRAME_SIZE_2032:
                case _DRV_ROBO_FRAME_SIZE_9712:
                    break;
                default:
                    return SOC_E_UNAVAIL;
            }

            SOC_IF_ERROR_RETURN(REG_READ_FM_SIZE_CTLr(unit, &reg_value));

            /* Standard frame size */
            if (size == _DRV_ROBO_FRAME_SIZE_1518) {
                temp = 0;
                soc_FM_SIZE_CTLr_field_set(unit, &reg_value, STD_MAX_FM_SIZEf, &temp);    
            }
            if (size == _DRV_ROBO_FRAME_SIZE_2000) {
                temp = 1;
                soc_FM_SIZE_CTLr_field_set(unit, &reg_value, STD_MAX_FM_SIZEf, &temp);    
            }
            enable = 0;

            /* Jumbo frame size */
            if (size == _DRV_ROBO_FRAME_SIZE_2032) {
                temp = 0;
                soc_FM_SIZE_CTLr_field_set(unit, &reg_value, JUMBO_FM_SIZEf, &temp);    
                enable = 1;
            }
            if (size == _DRV_ROBO_FRAME_SIZE_9712) {
                temp = 1;
                soc_FM_SIZE_CTLr_field_set(unit, &reg_value, JUMBO_FM_SIZEf, &temp);    
                enable = 1;
            }
            SOC_IF_ERROR_RETURN(REG_WRITE_FM_SIZE_CTLr(unit, &reg_value));

            ctrl_cnt = 1;
            ctrl_type = DRV_DEV_CTRL_JUMBO_FRAME_DROP;
            if (enable) {
                /* Jumbo Frame size is set, then disable "Jumbo Frame Drop" */
                temp = 0;
            } else {
                /* Jumbo Frame size is NOT set, then enable "Jumbo Frame Drop" */
                temp = 1;
            }
            rv = DRV_DEV_CONTROL_SET(unit, &ctrl_cnt, &ctrl_type, (int *)&temp);
            SOC_IF_ERROR_RETURN(rv);
        } 
    } else {
        return SOC_E_RESOURCE;
    }
    return rv;
    
}

STATIC int  
drv_mac_fe_ge_frame_max_get(int unit, soc_port_t port, int *size)
{
    uint32  reg_addr = 0, reg_value = 0, temp = 0;
    int     reg_len = 0, rv = SOC_E_NONE;
    uint16  dev_id = 0;
    uint8   rev_id = 0;
    uint32 ctrl_type = 0;
    uint32 ctrl_cnt = 0;
    uint32 enable = 0;

    soc_cm_debug(DK_VERBOSE,"drv_mac_fe_ge_frame_max_get\n");  

    if (SOC_IS_ROBO5396(unit)) {
        /* 
         * Only support receive Jumbo frames or not 
         * It can't set the max frame size
         */
        reg_addr = DRV_REG_ADDR(unit, JUMBO_PORT_MASKr, 0, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, JUMBO_PORT_MASKr);
        if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
         }
        temp = 0;
        DRV_REG_FIELD_GET(unit, JUMBO_PORT_MASKr, &reg_value, 
                JUMBO_FM_PORT_MASKf, &temp);
        if (temp &  (0x1 << port)) {
            *size = 9724;
        } else {
            *size = 1518;
        }
    } else if ((SOC_IS_ROBO5389(unit)) || (SOC_IS_ROBO5398(unit)) ||
        (SOC_IS_ROBO5397(unit)) || (SOC_IS_ROBO5395(unit)) || 
        (SOC_IS_ROBO_ARCH_VULCAN(unit))) {
        /* Get Good Frame max size */
        reg_addr = DRV_REG_ADDR(unit, MIB_GD_FM_MAX_SIZEr, 0, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, MIB_GD_FM_MAX_SIZEr);
        if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
         }
        DRV_REG_FIELD_GET(unit, MIB_GD_FM_MAX_SIZEr, &reg_value, 
            MAX_SIZEf, &temp);
        *size = temp;
    } else if (SOC_IS_ROBO5324(unit)) {
        *size = 1536;
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        soc_cm_get_id(unit, &dev_id, &rev_id);
        if ((rev_id == BCM53242_A0_REV_ID) || 
                (rev_id == BCM53262_A0_REV_ID)) {
            *size = 2048;
        } else {
            reg_addr = DRV_REG_ADDR(unit, NEW_CONTROLr, 0, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, NEW_CONTROLr);
            if ((rv = DRV_REG_READ(unit, reg_addr, 
                    &reg_value, reg_len)) < 0) {
                return rv;
            }
            DRV_REG_FIELD_GET(unit, NEW_CONTROLr, &reg_value, 
                    MAX_RX_LIMITf, &temp);
            if (temp == 0x0) {
                *size = 2048;
            } else if (temp == 0x1) {
                *size = 1536;
            } else if (temp == 0x2) {
                *size = 1518;
            } else {
                *size = 2000;
            }
        }
    } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        *size = 2048;
    } else if (SOC_IS_TBX(unit)) {
        /* IEEE 802.3as-2006 defined the EnvelopFrameSize at 2000 octects to 
         *  cover original maxFrameSize(1518 for untag and 1522/1526/1532 for 
         *  tagged). And TB's design followed this new Spec. already.
         */
        if (SOC_IS_TB_AX(unit)) {
            *size = _DRV_ROBO_FRAME_SIZE_2000;
        } else {
            ctrl_cnt = 1;
            ctrl_type = DRV_DEV_CTRL_JUMBO_FRAME_DROP;
            rv = DRV_DEV_CONTROL_GET(unit, &ctrl_cnt, &ctrl_type, (int *)&enable);
            SOC_IF_ERROR_RETURN(rv);
            
            SOC_IF_ERROR_RETURN(REG_READ_FM_SIZE_CTLr(unit, &reg_value));
            if (enable) {
                /* Jumbo Frame Drop Enabled */
                soc_FM_SIZE_CTLr_field_get(unit, &reg_value, STD_MAX_FM_SIZEf, &temp);
                if (temp) {
                    *size = _DRV_ROBO_FRAME_SIZE_2000;
                } else {
                    *size = _DRV_ROBO_FRAME_SIZE_1518;
                }
            } else {
                /* Jumbo Frame Drop Disabled */
                soc_FM_SIZE_CTLr_field_get(unit, &reg_value, JUMBO_FM_SIZEf, &temp);    
                if (temp) {
                    *size = _DRV_ROBO_FRAME_SIZE_9712;
                } else {
                    *size = _DRV_ROBO_FRAME_SIZE_2032;
                }
            }
        }
    } else {
        return SOC_E_RESOURCE;
    }
    return rv;
}

STATIC int      
drv_mac_fe_ge_ifg_set(int unit, soc_port_t port, int speed, 
    soc_port_duplex_t duplex, int ifg)
{
    uint32      ifg_val = ifg;
    soc_pbmp_t  t_pbm;
    /* for ROBO devices, the IFG get/set will be no different for port's speed 
     * and duplex.
     */
    SOC_PBMP_CLEAR(t_pbm);
    SOC_PBMP_PORT_ADD(t_pbm, port); 
    if (IS_GE_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm, DRV_PORT_PROP_IPG_GE, 
            ifg_val));
    } else {
        SOC_IF_ERROR_RETURN(DRV_PORT_SET(unit, t_pbm, DRV_PORT_PROP_IPG_FE, 
            ifg_val));
    }
    return SOC_E_NONE;
}

STATIC int      
drv_mac_fe_ge_ifg_get(int unit, soc_port_t port, int speed, 
    soc_port_duplex_t duplex, int *ifg)
{
    /* for ROBO devices, the IFG get/set will be no different for port's speed 
     * and duplex.
     */
    if (IS_GE_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, DRV_PORT_PROP_IPG_GE, 
            (uint32 *) ifg));
    } else {
        SOC_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, DRV_PORT_PROP_IPG_FE, 
            (uint32 *) ifg));
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *  drv_mac_ge_init
 * Purpose:
 *  Initialize the gigabit MAC into a known good state.
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - RoboSwitch port # on unit.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  Both the FE and GE MACs are initialized.
 *  The initialization speed/duplex is arbitrary and must be
 *  updated by linkscan before enabling the MAC.
 */

STATIC  int
drv_mac_ge_init(int unit, soc_port_t port)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value = 0, temp = 0;
    uint64  reg_value64;
    uint64  val64;
    int shift = 0;

    COMPILER_64_ZERO(reg_value64);
    COMPILER_64_ZERO(val64);

    /* initialize per port mac */
    soc_cm_debug(DK_VERBOSE, "drv_mac_ge_init\n");

    /* Enable TX/RX on each port */ 
    if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) || 
        SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) { 
        if (IS_CPU_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr(unit, port, &reg_value));

            /* Enable TX/RX */
            temp = 0;
            SOC_IF_ERROR_RETURN(soc_MII_PCTLr_field_set(
                    unit, &reg_value, MITX_DISf, &temp));
            SOC_IF_ERROR_RETURN(soc_MII_PCTLr_field_set(
                    unit, &reg_value, MIRX_DISf, &temp));
                    
            /* Write Port Control Register */
            SOC_IF_ERROR_RETURN(REG_WRITE_MII_PCTLr(unit, port, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_G_PCTLr(unit, port, &reg_value));

            /* Enable TX/RX */
            temp = 0;
            SOC_IF_ERROR_RETURN(soc_G_PCTLr_field_set(
                    unit, &reg_value, MITX_DISf, &temp));
            SOC_IF_ERROR_RETURN(soc_G_PCTLr_field_set(
                    unit, &reg_value, MIRX_DISf, &temp));

            /* Write Port Control Register */
            SOC_IF_ERROR_RETURN(REG_WRITE_G_PCTLr(unit, port, &reg_value));
        }
    } else if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_TBX(unit)) {
        if (IS_CPU_PORT(unit, port)) {
            if ((rv = REG_READ_IMP_CTLr(unit, port, &reg_value)) < 0) {
                return rv;
            }
            /* Enable TX/RX */
            temp = 0;
            soc_IMP_CTLr_field_set(unit, &reg_value, TX_DISf, &temp);
            soc_IMP_CTLr_field_set(unit, &reg_value, RX_DISf, &temp);
            if ((rv = REG_WRITE_IMP_CTLr(unit, port, &reg_value)) < 0) {
                return rv;
            }
        } else {
            if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                return rv;
            }
            /* Enable TX/RX */
            temp = 0;
            soc_G_PCTLr_field_set(unit, &reg_value, TX_DISf, &temp);
            soc_G_PCTLr_field_set(unit, &reg_value, RX_DISf, &temp);
            if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                return rv;
            }
        }
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_G_PCTLr(unit, port, &reg_value));


        /* Enable TX/RX */
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_G_PCTLr_field_set(
                unit, &reg_value, MITX_DISf, &temp));
        SOC_IF_ERROR_RETURN(soc_G_PCTLr_field_set(
                unit, &reg_value, MIRX_DISf, &temp));

        /* Write Port Control Register */
        SOC_IF_ERROR_RETURN(REG_WRITE_G_PCTLr(unit, port, &reg_value));
    }
    
    /* Flow Control configuration */
    if (SOC_IS_TBX(unit)){
        /* init Flow Control configuration :
         *  1. make sure the Port Flow control are disabled
         *
         *  Note :
         *  1. Global flow control is enabled in soc_robo_misc_init().
         *     Such design can also be applied to other 5324/53242/5348
         *     after verified.
         */ 
        SOC_IF_ERROR_RETURN(REG_READ_SW_XOFF_PORT_CTLr(unit, 
                (uint32 *)&reg_value64));
        
        /* disable all port's flow control when init */
        temp = 0;
        COMPILER_64_SET(val64, 0, temp); 
        COMPILER_64_SHL(val64, port * 2);

        /* OR to original configuration */
        COMPILER_64_OR(reg_value64, val64);
        SOC_IF_ERROR_RETURN(
                REG_WRITE_SW_XOFF_PORT_CTLr(unit, (uint32 *)&reg_value64));

    } else if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) 
            || SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit)||
            SOC_IS_ROBO53262(unit)){
        /* init SW_FLOW_CON register 
         *      1. set global enable bit
         *      2. disable the port bit for init. 
         *      3. enable flow control in half duplex mode.
         */
        SOC_IF_ERROR_RETURN(REG_READ_SW_FLOW_CONr(
                unit, (uint32 *)&reg_value64));

        /* Enable blobal enable SW flow control bit */
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_SW_FLOW_CONr_field_set(
                unit, (uint32 *)&reg_value64, SW_FLOE_CON_ENf, &temp));
            
        /* reset port bit set(disable) */
        /*reg_value64 &= ~(3 << (port + (port - NUM_FE_PORT(unit) - 1)));*/
        if (SOC_IS_ROBO5347(unit)) {
            shift = port + (port - NUM_FE_PORT(unit)) + 24;
        } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
            /* 
             * Since the shift of GE ports is a fixed value (24).
             * But there are 3 value of NUM_FE_PORT(), 
             * 8, 16 and 24 for BCM53202/212/242 respectively.
             * So, add hardcode here. 
             */
            shift = port + (port - 24) + 24;
        } else if (SOC_IS_ROBO5324(unit)) {
            /* 
             * Since the shift of GE ports is a fixed value (24 + 1).
             * But there are 3 value of NUM_FE_PORT(), 
             * 8, 16 and 24 for BCM5320/21/24 respectively.
             * So, add hardcode here. 
             */
            shift = port + (port - 24 - 1);
        } else if (SOC_IS_ROBO5348(unit)) {
            shift = port + (port - NUM_FE_PORT(unit));            
        } else {
            shift = port + (port - NUM_FE_PORT(unit) - 1);            
        }
        
        COMPILER_64_SET(val64, 0, 3);
        COMPILER_64_SHL(val64, shift);
        COMPILER_64_NOT(val64);
        COMPILER_64_AND(reg_value64, val64);

        /* enable port flow control in Half duplex */
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_SW_FLOW_CONr_field_set(
                unit, (uint32 *)&reg_value64, TXFLOW_HALF_MODEf, &temp));
    
        /* Write SW_FLOW_CON Register */
        SOC_IF_ERROR_RETURN(REG_WRITE_SW_FLOW_CONr(
                unit, (uint32 *)&reg_value64));
    }
    
    if (SOC_IS_ROBO5396(unit)){
        /* Tx enable/disable have to set "DIS_NEW_TXDISf" in TXQ_FLUSH_MODE 
         *  register also.
         */
        SOC_IF_ERROR_RETURN(REG_READ_TXQ_FLUSH_MODEr(unit, &reg_value));
                        
        /* reset the new tx_disable bit in this init routine for robo5396 */
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_TXQ_FLUSH_MODEr_field_set(
                unit, &reg_value, DIS_NEW_TXDISf, &temp));
        
        /* Write TXQ_FLUSH_MODE Register */
        SOC_IF_ERROR_RETURN(REG_WRITE_TXQ_FLUSH_MODEr(unit, &reg_value));
    }

    /* disable the port PHY auto-scan function */
    if(SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) || 
        SOC_IS_ROBO5389(unit) || SOC_IS_ROBO5347(unit) ||
        SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) ||
        SOC_IS_TBX(unit)){

        /* CPU has no STS_OVERRIDE for bcm53242 */
        if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
            if(IS_CPU_PORT(unit, port)) {
                return rv;
            }
        }
        
        OVERRIDE_LOCK(unit);
        rv = REG_READ_STS_OVERRIDE_GPr(unit, port, &reg_value);
        if (rv) {
            OVERRIDE_UNLOCK(unit);
            return rv;
        }
        
        temp = 0;
        if (SOC_IS_ROBO5324(unit)){
            /* EN_STS_OVERRIDEf is for enable/disable PHY auto-scan */
            rv = soc_STS_OVERRIDE_GPr_field_set(unit, &reg_value, 
                    EN_STS_OVERRIDEf, &temp);
        } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                    SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) ||
                    SOC_IS_TBX(unit)){
            rv = soc_STS_OVERRIDE_GPr_field_set(unit, &reg_value, 
                    EN_PHY_SCANf, &temp);
        } else {
            rv = soc_STS_OVERRIDE_GPr_field_set(unit, &reg_value, 
                    PHY_SCAN_ENf, &temp);
        }
        if (rv) {
            OVERRIDE_UNLOCK(unit);
            return rv;
        }
        rv = REG_WRITE_STS_OVERRIDE_GPr(unit, port, &reg_value);
        
        OVERRIDE_UNLOCK(unit);
    }

    /* Set the default override speed to 100MB for BCM53101 */
    if (SOC_IS_ROBO53101(unit)) {
        rv = drv_mac_fe_ge_speed_set(unit, port, 100);
    }

    return rv;
}

STATIC int
drv_mac_ge_enable_set(int unit, soc_port_t port, int enable)
{
    int         rv = SOC_E_NONE;
    uint32      port_ctrl_addr, reg_len, temp;
    uint32      port_ctrl, reg_id, field_id;

    /* initialize per port mac */
    soc_cm_debug(DK_VERBOSE, "drv_mac_ge_enable_set\n");

    /* Read Port Control Register */ 
    if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) ||
        SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) ||
        SOC_IS_TBX(unit)) {
            
        if (IS_CPU_PORT(unit, port)) {
            if (SOC_IS_TBX(unit)){
                reg_id = INDEX(IMP_PCTLr);
            } else {
                reg_id = INDEX(MII_PCTLr);
            }
        } else {
            reg_id = INDEX(G_PCTLr);
        }
        port_ctrl_addr = DRV_REG_ADDR(unit, reg_id, port,0);
        reg_len = DRV_REG_LENGTH_GET(unit, reg_id);
        if ((rv = DRV_REG_READ(unit, port_ctrl_addr, 
                &port_ctrl, reg_len)) < 0) {
            return rv;
        }
        /* Enable TX/RX */
        if (enable) {
            temp = 0;
        } else { /* Disable TX/RX */
            temp = 1;
        }
            
        if (SOC_IS_TBX(unit)){
            field_id = INDEX(TX_DISf);
        } else {
            field_id = INDEX(MITX_DISf);
        }
        DRV_REG_FIELD_SET(unit, reg_id, &port_ctrl, field_id, &temp);
        if (SOC_IS_TBX(unit)){
            field_id = INDEX(RX_DISf);
        } else {
            /* GNATS 40307 : Unexpected override on Port discard configuration
             *          while the link been re-established.
             *
             *  Note :
             *  1. Except TB chips, all ROBO chips use RX disable to implemente
             *      the port discard related configuration.
             *  2. This definition is used to be a special MAC enable case to  
             *     indicate the MAC been enabled exclude RX (RX is disabled).
             */
            if (enable == DRV_SPECIAL_MAC_ENABLE_NORX){
                temp = 1;
            }
            field_id = INDEX(MIRX_DISf);
        }
        DRV_REG_FIELD_SET(unit, reg_id, &port_ctrl, field_id, &temp);
            
        /* Write Port Control Register */
        if ((rv = DRV_REG_WRITE(unit, port_ctrl_addr, 
                &port_ctrl, reg_len)) < 0) {
            return rv;
        }
    } else {        
        port_ctrl_addr = DRV_REG_ADDR(unit, INDEX(G_PCTLr), port, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, INDEX(G_PCTLr));
        if ((rv = DRV_REG_READ(unit, port_ctrl_addr, 
                &port_ctrl, reg_len)) < 0) {
            return rv;
        }
        /* Enable TX/RX */
        if (enable) {
            temp = 0;
        } else { /* Disable TX/RX */
            temp = 1;
        }
        
        if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
            field_id = TX_DISf;
        } else {
            field_id = MITX_DISf;
        }
        DRV_REG_FIELD_SET(unit, INDEX(G_PCTLr), &port_ctrl, field_id, &temp);
        if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
            field_id = INDEX(RX_DISf);
        } else {
            field_id = INDEX(MIRX_DISf);
        }
        /* GNATS 40307 : Unexpected override on Port discard configuration
         *          while the link been re-established.
         *
         *  Note :
         *  1. Except TB chips, all ROBO chips use RX disable to implemente
         *      the port discard related configuration.
         *  2. This definition is used to be a special MAC enable case to  
         *     indicate the MAC been enabled exclude RX (RX is disabled).
         */
        if (enable == DRV_SPECIAL_MAC_ENABLE_NORX){
            temp = 1;
        }
        DRV_REG_FIELD_SET(unit, INDEX(G_PCTLr), &port_ctrl, field_id, &temp);
        /* Write Port Control Register */
        if ((rv = DRV_REG_WRITE(unit, port_ctrl_addr, 
                &port_ctrl, reg_len)) < 0) {
            return rv;
        }
    }
    if (SOC_IS_ROBO5396(unit)){
        /* Tx enable/disable have to set "DIS_NEW_TXDISf" in TXQ_FLUSH_MODE 
         *  register for bcm5396 also.
         */
        port_ctrl_addr = DRV_REG_ADDR(unit, INDEX(TXQ_FLUSH_MODEr), 0, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, INDEX(TXQ_FLUSH_MODEr));
        if ((rv = DRV_REG_READ(unit, port_ctrl_addr, 
                &port_ctrl, reg_len)) < 0) {
            return rv;
        }
                        
        DRV_REG_FIELD_SET(unit, INDEX(TXQ_FLUSH_MODEr), &port_ctrl, 
                DIS_NEW_TXDISf, &temp);
        
        /* Write TXQ_FLUSH_MODE Register */
        if ((rv = DRV_REG_WRITE(unit, port_ctrl_addr, 
                &port_ctrl, reg_len)) < 0) {
            return rv;
        }
    }

    return rv;
}

/*
 * Function:
 *  mac_ge_enable_get
 * Purpose:
 *  Get MAC enable state
 * Parameters:
 *  unit - RoboSwitch unit #.
 *  port - Port number on unit.
 *  enable - (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *  SOC_E_XXX
 */

STATIC int
drv_mac_ge_enable_get(int unit, soc_port_t port, int *enable)
{
    int         rv = SOC_E_NONE;
    uint32  port_ctrl_addr, temp, reg_len;
    uint32      port_ctrl;

    /* initialize per port mac */
    soc_cm_debug(DK_VERBOSE, "drv_mac_ge_enable_get\n");

    /* Read Port Control Register */ 
    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) || 
        SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        if (IS_CPU_PORT(unit, port)) {
            if ((rv = REG_READ_MII_PCTLr(unit, port, &port_ctrl))< 0){
                return rv;
            }                
            /* Read TX value */
            temp = 0;
            soc_MII_PCTLr_field_get(unit, &port_ctrl, MITX_DISf, &temp);
            if (temp) {
                *enable = FALSE;
            } else {
                *enable = TRUE;
            }
        } else {
            port_ctrl_addr = DRV_REG_ADDR(unit, G_PCTLr, port, 0);
            reg_len = DRV_REG_LENGTH_GET(unit, G_PCTLr);
            if ((rv = DRV_REG_READ(unit, port_ctrl_addr, 
                    &port_ctrl, reg_len)) < 0) {
                return rv;
            }
            /* Read TX value */
            temp = 0;
            DRV_REG_FIELD_GET(unit, G_PCTLr, &port_ctrl, MITX_DISf, &temp);
            if (temp) {
                *enable = FALSE;
            } else {
                *enable = TRUE;
            }
        }
    } else {
        uint32  field_id;
        
        port_ctrl_addr = DRV_REG_ADDR(unit, G_PCTLr, port, 0);
        reg_len = DRV_REG_LENGTH_GET(unit, G_PCTLr);
        if ((rv = DRV_REG_READ(unit, port_ctrl_addr, 
                &port_ctrl, reg_len)) < 0) {
            return rv;
        }
    
        /* Read TX value */
        temp = 0;
        if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
            field_id = TX_DISf;
        } else {
            field_id = MITX_DISf;
        }
        DRV_REG_FIELD_GET(unit, G_PCTLr, &port_ctrl, field_id, &temp);
        if (temp) {
            *enable = FALSE;
        } else {
            *enable = TRUE;
        }
    }
    return rv;
}


/*
 * Function:
 *  soc_mac_probe
 * Purpose:
 *  Probe the specified unit/port for MAC type and return
 *  a mac_driver_t pointer used to drive the MAC.
 * Parameters:
 *  unit - RoboSwitch Unit #.
 *  port - RoboSwitch Port # on unit.
 *  macd_ptr - (OUT) Pointer to MAC driver.
 * Returns:
 *  SOC_E_XXX
 */

int
soc_robo_mac_probe(int unit, soc_port_t port, mac_driver_t **macdp)
{
    if (IS_FE_PORT(unit, port) && (!SOC_IS_ROBO53101(unit))){
        *macdp = &drv_mac_fe;
    } else if (IS_GE_PORT(unit, port) || SOC_IS_ROBO53101(unit)){
        *macdp = &drv_mac_ge;
    } else if (IS_CPU_PORT(unit,port)){
        if (SOC_IS_ROBO5324(unit)){
            *macdp = &drv_mac_fe;
        } else if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5398(unit)|| \
                   SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5397(unit) || \
                   SOC_IS_ROBO5347(unit)|| SOC_IS_ROBO5395(unit) ||\
                   SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) || \
                   SOC_IS_ROBO53115(unit) ||SOC_IS_ROBO53118(unit) || 
                   SOC_IS_TBX(unit) || SOC_IS_ROBO53125(unit) || 
                   SOC_IS_ROBO53128(unit)){
            /* bcm5396 still use MII interface in reference board */
            *macdp = &drv_mac_ge;
        } else {
            return SOC_E_RESOURCE;
        }
    } else {
        return SOC_E_UNAVAIL;
    }
    
    return SOC_E_NONE;
}


int drv_mac_driver_set(int unit, uint32 **p)
{
    /* this routine is not suitable for robo chip */
    if (SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5348(unit)||
        SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit)||
        SOC_IS_ROBO53262(unit)){
        *p = (uint32 *)&drv_mac_fe;
    } else if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5398(unit)||
        SOC_IS_ROBO5397(unit) || SOC_IS_ROBO5395(unit) || 
        SOC_IS_ROBO_ARCH_VULCAN(unit)){
        *p = (uint32 *)&drv_mac_ge;
    }

    return SOC_E_NONE;
}
