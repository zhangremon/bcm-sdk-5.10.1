/*
 * $Id: port.c 1.91.2.7 Broadcom SDK $
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
 * Common driver routines for accessing the CMIC functional unit.
 */

#include <shared/error.h>
#include <soc/robo.h>
#include <soc/types.h>
#include <soc/debug.h>
#include <soc/mcm/robo/driver.h>
#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phyreg.h>
#include "../phy/phyreg.h"
#include "../phy/phyfege.h"
#include "../phy/phyident.h"


/*
 * soc_robo_port_info
 */

soc_robo_port_info_t *soc_robo_port_info[SOC_MAX_NUM_DEVICES];

#define DRV_TEST_BIT_IN_MAP(bit, bitmap) ((1 << bit) & bitmap)


/* The loopback set on ROBO's internal FE PHY can't work at speed=10.
 *  - tested on bcm5347 and found 2 cases with speed=10 but loopback still 
 *      working properly.
 *      a. set loopback at speed=10 and Auto-Neg is on.
 *      b. set loopback at speed=100 with AN=on and than set loopback to 
 *          speed=10 with AN=off
 */
#define ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED   0


/*
 *  Function : _drv_port_detach
 *
 *  Purpose :
 *      detach the sw port_info database.
 *
 *  Parameters :
 *      unit    :   unit id
 *      okay    :   Output parameter indicates port can be enabled.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      If error is returned, the port should not be enabled.
 *
 */
STATIC int
_drv_port_sw_detach(int unit)
{
    if (soc_robo_port_info[unit] == NULL){
        return SOC_E_NONE;
    }
    
    sal_free(soc_robo_port_info[unit]);
    soc_robo_port_info[unit] = NULL;
    
    return SOC_E_NONE; 
}

/*
 *  Function : _drv_port_probe
 *
 *  Purpose :
 *      Probe the phy and set up the phy and mac of the indicated port.
 *
 *  Parameters :
 *      unit        :   unit id
 *      p    :   port to probe.
 *      okay  :   Output parameter indicates port can be enabled.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      If error is returned, the port should not be enabled.
 *
 */
STATIC int
_drv_port_probe(int unit, soc_port_t p, int *okay)
{
    int            rv;
    mac_driver_t    *macd;
    uint32 default_port_medium;

    *okay = FALSE;


    if (soc_robo_port_info[unit] == NULL) 
    {
        soc_robo_port_info[unit] = 
            sal_alloc(sizeof(soc_robo_port_info_t) * SOC_ROBO_MAX_NUM_PORTS, "soc_port_info");
        if (soc_robo_port_info[unit] == NULL) 
        {
            return SOC_E_MEMORY;
        }
        sal_memset(soc_robo_port_info[unit], 0, sizeof(soc_robo_port_info_t) * SOC_ROBO_MAX_NUM_PORTS);
    }
    

    if ((rv = soc_phyctrl_probe(unit, p)) < 0) {
        return rv;
    } 

    if ((rv = soc_phyctrl_init(unit, p)) < 0) {    
        return rv;
    }

    /*
     * set default phy medium on each port (FE/GE)
     */

    SOC_IF_ERROR_RETURN(DRV_PORT_GET(unit, p, DRV_PORT_PROP_PHY_MEDIUM, 
        &default_port_medium));
    SOC_ROBO_PORT_MEDIUM_MODE(unit, p) = default_port_medium;


    /*
     * Currently initializing MAC after PHY is required.
     */

    soc_cm_debug(DK_PORT | DK_VERBOSE, "Init port %d MAC...\n", p);

    if ((rv = soc_robo_mac_probe(unit, p, &macd)) < 0) {
        soc_cm_debug(DK_WARN | DK_PORT,
             "Unit %d Port %s: Failed to probe MAC: %s\n",
             unit, SOC_PORT_NAME(unit, p), soc_errmsg(rv));
        return rv;
    }

    SOC_ROBO_PORT_MAC_DRIVER(unit, p) = macd;

    if ((rv = MAC_INIT(macd, unit, p)) < 0) {
        soc_cm_debug(DK_WARN | DK_PORT,
             "Unit %d Port %s: Failed to initialize MAC: %s\n",
             unit, SOC_PORT_NAME(unit, p), soc_errmsg(rv));
        return rv;
    }


    *okay = TRUE;

    return SOC_E_NONE;
}


int
_drv_port_security_mode_set(int unit, uint32 port, uint32 mode, uint32 sa_num)
{
    uint32 reg_index = 0, fld_index = 0;
    uint32 temp = 0;
    uint32  reg_addr, reg_value;
    uint64  reg_value64;
    int reg_len;
    int rv = SOC_E_NONE;

    /* the security mode */
    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            temp = 0;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
            temp = 4;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
            temp = 5;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            temp = 6;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            temp = 7;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* get the index of register and field */
    if (port < 16) {
        reg_index = MAC_SEC_CON0r;
        switch (port) {
            case 0:
                fld_index = MAC_SEC_CON_P0f;
                break;
            case 1:
                fld_index = MAC_SEC_CON_P1f;
                break;
            case 2:
                fld_index = MAC_SEC_CON_P2f;
                break;
            case 3:
                fld_index = MAC_SEC_CON_P3f;
                break;
            case 4:
                fld_index = MAC_SEC_CON_P4f;
                break;
            case 5:
                fld_index = MAC_SEC_CON_P5f;
                break;
            case 6:
                fld_index = MAC_SEC_CON_P6f;
                break;
            case 7:
                fld_index = MAC_SEC_CON_P7f;
                break;
            case 8:
                fld_index = MAC_SEC_CON_P8f;
                break;
            case 9:
                fld_index = MAC_SEC_CON_P9f;
                break;
            case 10:
                fld_index = MAC_SEC_CON_P10f;
                break;
            case 11:
                fld_index = MAC_SEC_CON_P11f;
                break;
            case 12:
                fld_index = MAC_SEC_CON_P12f;
                break;
            case 13:
                fld_index = MAC_SEC_CON_P13f;
                break;
            case 14:
                fld_index = MAC_SEC_CON_P14f;
                break;
            case 15:
                fld_index = MAC_SEC_CON_P15f;
                break;
        }
    } else if (port < 24){
        reg_index = MAC_SEC_CON1r;
        switch (port) {
            case 16:
                fld_index = MAC_SEC_CON_P16f;
                break;
            case 17:
                fld_index = MAC_SEC_CON_P17f;
                break;
            case 18:
                fld_index = MAC_SEC_CON_P18f;
                break;
            case 19:
                fld_index = MAC_SEC_CON_P19f;
                break;
            case 20:
                fld_index = MAC_SEC_CON_P20f;
                break;
            case 21:
                fld_index = MAC_SEC_CON_P21f;
                break;
            case 22:
                fld_index = MAC_SEC_CON_P22f;
                break;
            case 23:
                fld_index = MAC_SEC_CON_P23f;
                break;
        }
    } else {
        rv = SOC_E_PARAM;
        return rv;;
    }
    /* write to chip */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }

    if (mode != DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM) {
        /* Config Port x Dynamic Learning Threshold register 
         *  - set SA_NO for MAC security (default value is 0x1fff)
         */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_MAX_LEARNr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, PORT_MAX_LEARNr, port, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        
        temp = 0x1fff;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_MAX_LEARNr, &reg_value, DYN_MAX_MAC_NOf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
    }

    return rv;
}

int
_drv_port_security_mode_get(int unit, uint32 port, uint32 mode, 
    uint32 *prop_val)
{
    uint32 reg_index = 0, fld_index = 0;
    uint32 temp;
    uint32  reg_addr;
    uint64  reg_value64;
    int reg_len;
    int rv = SOC_E_NONE;

    /* get the index of register and field */
    if (port < 16) {
        reg_index = MAC_SEC_CON0r;
        switch (port) {
            case 0:
                fld_index = MAC_SEC_CON_P0f;
                break;
            case 1:
                fld_index = MAC_SEC_CON_P1f;
                break;
            case 2:
                fld_index = MAC_SEC_CON_P2f;
                break;
            case 3:
                fld_index = MAC_SEC_CON_P3f;
                break;
            case 4:
                fld_index = MAC_SEC_CON_P4f;
                break;
            case 5:
                fld_index = MAC_SEC_CON_P5f;
                break;
            case 6:
                fld_index = MAC_SEC_CON_P6f;
                break;
            case 7:
                fld_index = MAC_SEC_CON_P7f;
                break;
            case 8:
                fld_index = MAC_SEC_CON_P8f;
                break;
            case 9:
                fld_index = MAC_SEC_CON_P9f;
                break;
            case 10:
                fld_index = MAC_SEC_CON_P10f;
                break;
            case 11:
                fld_index = MAC_SEC_CON_P11f;
                break;
            case 12:
                fld_index = MAC_SEC_CON_P12f;
                break;
            case 13:
                fld_index = MAC_SEC_CON_P13f;
                break;
            case 14:
                fld_index = MAC_SEC_CON_P14f;
                break;
            case 15:
                fld_index = MAC_SEC_CON_P15f;
                break;
        }
    } else if (port < 24){
        reg_index = MAC_SEC_CON0r;
        switch (port) {
            case 16:
                fld_index = MAC_SEC_CON_P6f;
                break;
            case 17:
                fld_index = MAC_SEC_CON_P7f;
                break;
            case 18:
                fld_index = MAC_SEC_CON_P8f;
                break;
            case 19:
                fld_index = MAC_SEC_CON_P9f;
                break;
            case 20:
                fld_index = MAC_SEC_CON_P10f;
                break;
            case 21:
                fld_index = MAC_SEC_CON_P11f;
                break;
            case 22:
                fld_index = MAC_SEC_CON_P12f;
                break;
            case 23:
                fld_index = MAC_SEC_CON_P13f;
                break;
        }
    } else {
        rv = SOC_E_PARAM;
        return rv;
    }
    /* write to chip */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp));
    switch (temp) {
        case 0:
        case 1:
        case 2:
        case 3:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_NONE) {
                *prop_val = TRUE;
            }
            break;
        case 4:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT) {
                *prop_val = TRUE;
            }
            break;
        case 5:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT) {
                *prop_val = TRUE;
            }
            break;
        case 6:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM) {
                *prop_val = TRUE;
            }
            break;
        case 7:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH) {
                *prop_val = TRUE;
            }
            break;
        default:
            return SOC_E_INTERNAL;
    }

    return rv;
}

int
_drv_port_802_1x_config_set(int unit, int port, uint32 mode, uint32 enable)
{
    int             rv = SOC_E_NONE;
    uint32          reg_addr, temp;
    uint64          reg_value64;
    int             reg_len;


    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, PORT_EAP_CONr);

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, PORT_EAP_CONr, port, 0);


    if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }

    if (enable ){
        temp = 1;
    } else {
        temp = 0;
    }
    switch (mode) {
        case DRV_PORT_PROP_802_1X_MODE:
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, PORT_EAP_CONr, (uint32 *)&reg_value64, EAP_ENf, 
                    &temp));

            break;
        case DRV_PORT_PROP_802_1X_BLK_RX:
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, PORT_EAP_CONr, (uint32 *)&reg_value64, EAP_BLK_MODEf, 
                    &temp));
            break;
        default:
            return SOC_E_INTERNAL;
    }    

    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    return rv;
}
int
_drv_port_802_1x_config_get(int unit, int port, uint32 mode, uint32 *enable)
{
 int             rv = SOC_E_NONE;
    uint32          reg_addr, temp;
    uint64          reg_value64;
    int             reg_len;


    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, PORT_EAP_CONr);

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, PORT_EAP_CONr, port, 0);

    if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    *enable = 0;
    switch (mode) {
        case DRV_PORT_PROP_802_1X_MODE:
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, PORT_EAP_CONr, (uint32 *)&reg_value64, EAP_ENf, 
                    &temp));

            break;
        case DRV_PORT_PROP_802_1X_BLK_RX:
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, PORT_EAP_CONr, (uint32 *)&reg_value64, EAP_BLK_MODEf, 
                    &temp));
            break;
        default:
            return SOC_E_INTERNAL;
    }    
    *enable = temp;
    return rv;

}

int
_drv_bcm5348_port_security_mode_set(int unit, uint32 port, uint32 mode, uint32 sa_num)
{
    uint32 reg_index = 0, fld_index = 0;
    uint32 temp = 0, temp32 = 0;
    uint32  reg_addr, reg_value;
    uint64  reg_value64, temp64, val64;
    int reg_len;
    int rv = SOC_E_NONE;

    /* the security mode */
    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            temp = 0;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            temp = 2;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            temp = 3;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* get the index of register and field */
    if (SOC_IS_ROBO5348(unit)) {
        if (port < 32) {
            reg_index = MAC_SEC_CON0r;
            fld_index = MAC_SEC_CONf;
        } else if (port < 53){
            reg_index = MAC_SEC_CON1r;
            fld_index = MAC_SEC_CON_P52_32f;
        } else {
            rv = SOC_E_PARAM;
            return rv;
        }
    } else { /* 5347 */
        if (port < 8) {
            reg_index = MAC_SEC_CON0r;
            fld_index = MAC_SEC_CONf;
        } else if (port < 29){
            reg_index = MAC_SEC_CON1r;
            fld_index = MAC_SEC_CON_P52_32f;
        } else {
            rv = SOC_E_PARAM;
            return rv;
        }
    }
    /* write to chip */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    if (SOC_IS_ROBO5347(unit) && reg_index == MAC_SEC_CON0r) {
        /* field length <= 32 bit */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp32));
        COMPILER_64_SET(temp64, 0, temp32);
    } else {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_value64, fld_index, (uint32 *)&temp64));
    }

    if (SOC_IS_ROBO5348(unit)) {
        if (port < 32) {
            /* Clear original value */
            /* temp64 &= ~(0x3 << (port * 2)); */
            COMPILER_64_SET(val64, 0, 0x3);
            COMPILER_64_SHL(val64, (port*2));
            COMPILER_64_NOT(val64);
            COMPILER_64_AND(temp64, val64);
            /* Set new value */
            /* temp64 |= (temp << (port * 2)); */
            COMPILER_64_SET(val64, 0, temp);
            COMPILER_64_SHL(val64, (port*2));
            COMPILER_64_OR(temp64, val64);
        } else {
            /* temp64 &= ~(0x3 << ((port - 32) * 2));*/
            COMPILER_64_SET(val64, 0, 0x3);
            COMPILER_64_SHL(val64, ((port-32)*2));
            COMPILER_64_NOT(val64);
            COMPILER_64_AND(temp64, val64);
            /* temp64 |= (temp << ((port - 32) * 2)); */
            COMPILER_64_SET(val64, 0, temp);
            COMPILER_64_SHL(val64, ((port-32)*2));
            COMPILER_64_OR(temp64, val64);

        }
    } else { /* 5347 */
        if (port < 8) {
            /* Clear original value */
            /* temp64 &= ~(0x3 << (port * 2));*/
            COMPILER_64_SET(val64, 0, 0x3);
            COMPILER_64_SHL(val64, (port*2));
            COMPILER_64_NOT(val64);
            COMPILER_64_AND(temp64, val64);
            /* Set new value */
            /* temp64 |= (temp << (port * 2)); */
            COMPILER_64_SET(val64, 0, temp);
            COMPILER_64_SHL(val64, (port*2));
            COMPILER_64_OR(temp64, val64);
        } else {
            /* temp64 &= ~(0x3 << ((port - 8) * 2)); */
            COMPILER_64_SET(val64, 0, 0x3);
            COMPILER_64_SHL(val64, ((port-8)*2));
            COMPILER_64_NOT(val64);
            COMPILER_64_AND(temp64, val64);            
            /* temp64 |= (temp << ((port - 8) * 2)); */
            COMPILER_64_SET(val64, 0, temp);
            COMPILER_64_SHL(val64, ((port-8)*2));
            COMPILER_64_OR(temp64, val64);
            
        }
    }

     if (SOC_IS_ROBO5347(unit) && reg_index == MAC_SEC_CON0r) {
        /* field length <= 32 bit */
        temp = COMPILER_64_LO(temp64);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp));
    } else {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, (uint32 *)&reg_value64, fld_index, (uint32 *)&temp64));
    }
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }

    if (mode != DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM) {
        /* Config Port x Dynamic Learning Threshold register 
         *  - set SA_NO for MAC security (default value is 0x1fff)
         */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_MAX_LEARNr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, PORT_MAX_LEARNr, port, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        
        temp = 0x1fff;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_MAX_LEARNr, &reg_value, DYN_MAX_MAC_NOf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
    }

    return rv;
}

int
_drv_bcm5348_port_security_mode_get(int unit, uint32 port, uint32 mode, 
    uint32 *prop_val)
{
    uint32 reg_index = 0, fld_index = 0;
    uint32 temp;
    uint32  reg_addr;
    uint64  reg_value64, temp64, val64;
    int reg_len;
    int rv = SOC_E_NONE;

    /* get the index of register and field */
    if (SOC_IS_ROBO5348(unit)) {
        if (port < 32) {
            reg_index = MAC_SEC_CON0r;
            fld_index = MAC_SEC_CONf;
        } else if (port < 53){
            reg_index = MAC_SEC_CON1r;
            fld_index = MAC_SEC_CON_P52_32f;
        } else {
            rv = SOC_E_PARAM;
            return rv;
        }
    } else { /* 5347 */
        if (port < 8) {
            reg_index = MAC_SEC_CON0r;
            fld_index = MAC_SEC_CONf;
        } else if (port < 29){
            reg_index = MAC_SEC_CON1r;
            fld_index = MAC_SEC_CON_P52_32f;
        } else {
            rv = SOC_E_PARAM;
            return rv;
        }
    }
    /* write to chip */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    if (SOC_IS_ROBO5347(unit) && reg_index == MAC_SEC_CON0r) {
        /* field length <= 32 bit */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp));
        COMPILER_64_SET(temp64, 0, temp);
    } else {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_value64, fld_index, (uint32 *)&temp64));
    }


    
    if (SOC_IS_ROBO5348(unit)) {
        if (port < 32) {
            /* temp =  (temp64 >> (port * 2)) & 0x3; */
            COMPILER_64_SET(val64, 0, 0x3);
            COMPILER_64_SHR(temp64, port*2);
            COMPILER_64_AND(temp64, val64);
            COMPILER_64_TO_32_LO(temp, temp64);
        } else {
            /* temp =  (temp64 >> ((port - 32) * 2)) & 0x3; */
            COMPILER_64_SET(val64, 0, 0x3);
            COMPILER_64_SHR(temp64, (port-32)*2);
            COMPILER_64_AND(temp64, val64);
            COMPILER_64_TO_32_LO(temp, temp64);
        }
    } else { /* 5347 */
        if (port < 8) {
            /* temp =  (temp >> (port * 2)) & 0x3; */
            COMPILER_64_SET(val64, 0, 0x3);
            COMPILER_64_SHR(temp64, port*2);
            COMPILER_64_AND(temp64, val64);
            COMPILER_64_TO_32_LO(temp, temp64);
        } else {
            /* temp =  (temp >> ((port - 8) * 2)) & 0x3; */
            COMPILER_64_SET(val64, 0, 0x3);
            COMPILER_64_SHR(temp64, (port-8)*2);
            COMPILER_64_AND(temp64, val64);
            COMPILER_64_TO_32_LO(temp, temp64);
        }
    }

    *prop_val = FALSE;
    switch (temp) {
        case 0:
        case 1:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_NONE) {
                *prop_val = TRUE;
            }
            break;
        case 2:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM) {
                *prop_val = TRUE;
            }
            break;
        case 3:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH) {
                *prop_val = TRUE;
            }
            break;
        default:
            return SOC_E_INTERNAL;
    }

    return rv;
}

int
_drv_bcm5398_port_security_mode_set(int unit, uint32 port, uint32 mode, uint32 sa_num)
{
    uint32 temp = 0;
    uint64  reg_value64;
    int rv = SOC_E_NONE;

    /* the security mode */
    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            temp = 0;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            temp = 2;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            temp = 3;
            break;
        default:
            return SOC_E_PARAM;
    }
    /* write to chip */
    if ((rv = REG_READ_PORT_EAP_CONr(
        unit, port, (uint32 *)&reg_value64)) < 0) {
        return rv;
    }
    soc_PORT_EAP_CONr_field_set(unit, (uint32 *)&reg_value64,
        EAP_MODEf, &temp);
    if ((rv = REG_WRITE_PORT_EAP_CONr(
        unit, port, (uint32 *)&reg_value64)) < 0) {
        return rv;
    }

    return rv;
}

int
_drv_bcm5398_port_security_mode_get(int unit, uint32 port, uint32 mode, 
    uint32 *prop_val)
{
    uint32 temp;
    uint64  reg_value64;
    int rv = SOC_E_NONE;

    if ((rv = REG_READ_PORT_EAP_CONr(
        unit, port, (uint32 *)&reg_value64)) < 0) {
        return rv;
    }
    soc_PORT_EAP_CONr_field_get(unit, (uint32 *)&reg_value64,
        EAP_MODEf, &temp);
    switch (temp) {
        case 0:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_NONE) {
                *prop_val = TRUE;
            }
            break;
        case 2:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_EXTEND) {
                *prop_val = TRUE;
            }
            break;
        case 3:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY) {
                *prop_val = TRUE;
            }
            break;
        default:
            return SOC_E_INTERNAL;
    }
    return rv;
}

int
_drv_bcm53242_port_security_mode_set(int unit, uint32 port, uint32 mode, uint32 sa_num)
{
    uint32 temp = 0, trap = 0;
    uint32 reg_value;
    int rv = SOC_E_NONE;
    int change_eap = 0, change_trap = 0;

    /* the security mode */
    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            temp = 0;
            change_eap = 1;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            temp = 2;
            change_eap = 1;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            temp = 1;
            change_eap = 1;
            break;            
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            trap = 0; 
            change_trap = 1;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            trap = 1;
            change_trap = 1;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* write mode to chip */
    SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr(unit, port, &reg_value));
    if (change_eap) {
        SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_set
                (unit, &reg_value, EAP_MODEf, &temp));
    }
    if (change_trap) {
        SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_set
                (unit, &reg_value, SA_VIO_OPTf, &trap));
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_PORT_SEC_CONr(unit, port, &reg_value));

    if (mode != DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM) {
        /* Config Port x Dynamic Learning Threshold register 
         *  - set SA_NO for MAC security (default value is 0x0000)
         */
        SOC_IF_ERROR_RETURN(REG_READ_PORT_MAX_LEARNr(unit, port, &reg_value));

        temp = 0;
        SOC_IF_ERROR_RETURN(soc_PORT_MAX_LEARNr_field_set
                (unit, &reg_value, DYN_MAX_MAC_NOf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_PORT_MAX_LEARNr(unit, port, &reg_value));
    }

    return rv;
}

int
_drv_bcm53242_port_security_mode_get(int unit, uint32 port, uint32 mode, 
    uint32 *prop_val)
{
    uint32 temp_mode, temp;
    uint32 reg_value;
    int rv = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr(unit, port, &reg_value));
    SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_get
                (unit, &reg_value, EAP_MODEf, &temp_mode));
    SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_get
                (unit, &reg_value, SA_VIO_OPTf, &temp));

    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            *prop_val = (temp_mode == 0) ? TRUE : FALSE;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            *prop_val = (temp_mode == 2) ? TRUE : FALSE;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            *prop_val = (temp_mode == 1) ? TRUE : FALSE;
            break;            
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            *prop_val = (temp == 0) ? TRUE : FALSE;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            *prop_val = (temp == 1) ? TRUE : FALSE;
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

#if defined(BCM_53125) || defined(BCM_53128)

int 
_drv_bcm53125_port_eee_set(int unit, soc_pbmp_t bmp, 
                uint32 prop_type, uint32 prop_val)
{
    int rv = SOC_E_NONE, port;
    uint32 reg_val = 0, temp = 0;
    uint32 freq_khz = 0;
    
    switch(prop_type) {
        case DRV_PORT_PROP_EEE_ENABLE:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_EN_CTRLr(unit, &reg_val));
            soc_EEE_EN_CTRLr_field_get(unit, &reg_val, 
                EN_EEEf, &temp);
            if (prop_val) {
                /* If current state is MAC LOW POWER Mode,
                 * don't allow to enable the EEE.
                 * It will casue the port hand or other abnormal behavior.
                 */
                SOC_IF_ERROR_RETURN(
                    DRV_DEV_PROP_GET(unit, 
                    DRV_DEV_PROP_LOW_POWER_ENABLE, &freq_khz));
                if (freq_khz) {
                    soc_cm_debug(DK_ERR, 
                        "Don't allow to enable EEE in Low Power Mode.\n");
                    return SOC_E_FAIL;
                }
                temp |= (SOC_PBMP_WORD_GET(bmp, 0));
            } else {
                temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
            }
            soc_EEE_EN_CTRLr_field_set(unit, &reg_val, 
                EN_EEEf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_EEE_EN_CTRLr(unit, &reg_val));
            break;
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G:
            /* 20-bit length */
            if (prop_val > 1048576) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_SLEEP_TIMER_Gr(unit, port, &reg_val));
                soc_EEE_SLEEP_TIMER_Gr_field_set(unit, &reg_val, 
                    SLEEP_TIMER_Gf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_SLEEP_TIMER_Gr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H:
            /* 20-bit length */
            if (prop_val > 1048576) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_SLEEP_TIMER_Hr(unit, port, &reg_val));
                soc_EEE_SLEEP_TIMER_Hr_field_set(unit, &reg_val, 
                    SLEEP_TIMER_Hf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_SLEEP_TIMER_Hr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G:
            /* 20-bit length */
            if (prop_val > 1048576) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_MIN_LP_TIMER_Gr(unit, port, &reg_val));
                soc_EEE_MIN_LP_TIMER_Gr_field_set(unit, &reg_val, 
                    MIN_LP_TIMER_Gf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_MIN_LP_TIMER_Gr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H:
            /* 20-bit length */
            if (prop_val > 1048576) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_MIN_LP_TIMER_Hr(unit, port, &reg_val));
                soc_EEE_MIN_LP_TIMER_Hr_field_set(unit, &reg_val, 
                    MIN_LP_TIMER_Hf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_MIN_LP_TIMER_Hr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G:
            /* 10-bit length */
            if (prop_val > 1024) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_WAKE_TIMER_Gr(unit, port, &reg_val));
                soc_EEE_WAKE_TIMER_Gr_field_set(unit, &reg_val, 
                    WAKE_TIMER_Gf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_WAKE_TIMER_Gr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H:
            /* 10-bit length */
            if (prop_val > 1024) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_WAKE_TIMER_Hr(unit, port, &reg_val));
                soc_EEE_WAKE_TIMER_Hr_field_set(unit, &reg_val, 
                    WAKE_TIMER_Hf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_WAKE_TIMER_Hr(unit, port, &reg_val));
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}

int 
_drv_bcm53125_port_eee_get(int unit, int port, 
                uint32 prop_type, uint32 *prop_val)
{
    int rv = SOC_E_NONE;
    uint32 reg_val = 0, temp = 0;
    
    switch(prop_type) {
        case DRV_PORT_PROP_EEE_ENABLE:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_EN_CTRLr(unit, &reg_val));
            soc_EEE_EN_CTRLr_field_get(unit, &reg_val, 
                EN_EEEf, &temp);
            if (temp & (0x1 << port)) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_SLEEP_TIMER_Gr(unit, port, &reg_val));
            soc_EEE_SLEEP_TIMER_Gr_field_get(unit, &reg_val, 
                SLEEP_TIMER_Gf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_SLEEP_TIMER_Hr(unit, port, &reg_val));
            soc_EEE_SLEEP_TIMER_Hr_field_get(unit, &reg_val, 
                SLEEP_TIMER_Hf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_MIN_LP_TIMER_Gr(unit, port, &reg_val));
            soc_EEE_MIN_LP_TIMER_Gr_field_get(unit, &reg_val, 
                MIN_LP_TIMER_Gf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_MIN_LP_TIMER_Hr(unit, port, &reg_val));
            soc_EEE_MIN_LP_TIMER_Hr_field_get(unit, &reg_val, 
                MIN_LP_TIMER_Hf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_WAKE_TIMER_Gr(unit, port, &reg_val));
            soc_EEE_WAKE_TIMER_Gr_field_get(unit, &reg_val, 
                WAKE_TIMER_Gf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_WAKE_TIMER_Hr(unit, port, &reg_val));
            soc_EEE_WAKE_TIMER_Hr_field_get(unit, &reg_val, 
                WAKE_TIMER_Hf, &temp);
            *prop_val = temp;
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}

#endif /* BCM_53125 */

/*
 *  Function : drv_set_MAC_auto_negotiation
 *
 *  Purpose :
 *      Set PHY auto-nego through MAC register.
 *      (This is not suitable for ROBO)
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      enable  :   enable/disable MAC auto-negotiation.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_set_MAC_auto_negotiation(int unit, int port, int enable)
{

  soc_cm_debug(DK_PORT, "drv_set_MAC_autonegotiation.\n");
  
  return SOC_E_UNAVAIL;
}


/*
 *  Function : drv_port_property_enable_set
 *
 *  Purpose :
 *      Set the port based properties enable/disable.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      property  :   port property type.
 *      enable  :   enable/disable
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_property_enable_set(int unit, int port, int property, uint32 enable)
{
    uint32    addr = 0, temp = 0, vlan_ctrl3;
    int        rv = SOC_E_NONE;
    int         length = 0;
    uint32    reg_index = 0, fld_index = 0;
    uint32    reg_value;
    uint64    vlan_ctrl3_64, reg_value64, temp64;
    soc_pbmp_t pbmp;


    COMPILER_64_ZERO(temp64);
    switch (property) {
        case DRV_PORT_PROP_ENABLE_RX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_ROBO53101(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }
                soc_TH_PCTLr_field_set(unit, &reg_value,
                    MIRX_DISf, &temp);
                if ((rv = REG_WRITE_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_ROBO53101(unit)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }

                if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        RX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        MIRX_DISf, &temp);
                }
                if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_ROBO53101(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }
                soc_TH_PCTLr_field_set(unit, &reg_value,
                    MITX_DISf, &temp);
                if ((rv = REG_WRITE_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_ROBO53101(unit)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }
                if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        TX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        MITX_DISf, &temp);
                }
                if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }

                /* specific Tx disale setting for bcm5396 */
                if (SOC_IS_ROBO5396(unit)){
                    /* Tx enable/disable have to set "DIS_NEW_TXDISf" in 
                     *   TXQ_FLUSH_MODE register for bcm5396 also.
                     */
                    if ((rv = REG_READ_TXQ_FLUSH_MODEr(
                        unit, &reg_value)) < 0) {
                        return rv;
                    }

                    soc_TXQ_FLUSH_MODEr_field_set(unit, &reg_value, 
                        DIS_NEW_TXDISf, &temp);
                    
                    /* Write TXQ_FLUSH_MODE Register */
                    if ((rv = REG_WRITE_TXQ_FLUSH_MODEr(
                        unit, &reg_value)) < 0) {
                        return rv;
                    }
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TXRX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_ROBO53101(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }
                soc_TH_PCTLr_field_set(unit, &reg_value,
                    MITX_DISf, &temp);
                soc_TH_PCTLr_field_set(unit, &reg_value,
                    MIRX_DISf, &temp);
                if ((rv = REG_WRITE_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_ROBO53101(unit))  {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }

                if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        TX_DISf, &temp);
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        RX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        MITX_DISf, &temp);
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        MIRX_DISf, &temp);
                }
                if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                
                /* specific Tx disale setting for bcm5396 */
                if (SOC_IS_ROBO5396(unit)){
                    /* Tx enable/disable have to set "DIS_NEW_TXDISf" in 
                     *   TXQ_FLUSH_MODE register for bcm5396 also.
                     */
                    if ((rv = REG_READ_TXQ_FLUSH_MODEr(
                        unit, &reg_value)) < 0) {
                        return rv;
                    }
                              
                    soc_TXQ_FLUSH_MODEr_field_set(unit, &reg_value, 
                        DIS_NEW_TXDISf, &temp);
                    
                    /* Write TXQ_FLUSH_MODE Register */
                    if ((rv = REG_WRITE_TXQ_FLUSH_MODEr(
                        unit, &reg_value)) < 0) {
                        return rv;
                    }
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_VLAN_CTRL3r(
                    unit, (uint32 *)&vlan_ctrl3_64)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO5348(unit)) {
                soc_robo_64_val_to_pbmp(unit, &pbmp, vlan_ctrl3_64);
                } else { /* 5347, 53242 */
                    soc_VLAN_CTRL3r_field_get(unit, (uint32 *)&vlan_ctrl3_64, 
                        EN_DROP_NON1Qf, &temp);
                    SOC_PBMP_WORD_SET(pbmp, 0, temp);
                }
                if (enable) {
                    SOC_PBMP_PORT_ADD(pbmp, port);
                } else {
                    SOC_PBMP_PORT_REMOVE(pbmp, port);
                }
                if (SOC_IS_ROBO5348(unit)) {
                    soc_robo_64_pbmp_to_val(unit, &pbmp, &vlan_ctrl3_64);
                } else { /* 5347, 53242 */
                    temp = SOC_PBMP_WORD_GET(pbmp, 0);
                    soc_VLAN_CTRL3r_field_set(unit, (uint32 *)&vlan_ctrl3_64, 
                        EN_DROP_NON1Qf, &temp);
                }
                if ((rv = REG_WRITE_VLAN_CTRL3r(
                    unit, (uint32 *)&vlan_ctrl3_64)) < 0) {
                    return rv;
                }
            } else {
                /* bcm53115, bcm53118 , bcm53101, can apply this section */
                if ((rv = REG_READ_VLAN_CTRL3r(
                    unit, &vlan_ctrl3)) < 0) {
                    return rv;
                }
                if (enable) {
                    vlan_ctrl3 |= ( 1 << port);
                } else {
                    vlan_ctrl3 &= ~( 1 << port);
                }
                if ((rv = REG_WRITE_VLAN_CTRL3r(
                    unit, &vlan_ctrl3)) < 0) {
                    return rv;
                }
            }
            return rv;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
            return SOC_E_UNAVAIL;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            if (!(SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5347(unit)||
                  SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5397(unit)||
                  SOC_IS_ROBO5395(unit)||SOC_IS_ROBO53242(unit)||
                  SOC_IS_ROBO53262(unit)||SOC_IS_ROBO_ARCH_VULCAN(unit))) {
                return SOC_E_UNAVAIL;
            }
            if (property == DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX) {
                reg_index = INDEX(RX_PAUSE_PASSr);
                fld_index = INDEX(RX_PAUSE_PASSf);
            } else {
                reg_index = INDEX(TX_PAUSE_PASSr);
                fld_index = INDEX(TX_PAUSE_PASSf);
            }
            addr =  (DRV_SERVICES(unit)->reg_addr)
                (unit, reg_index, 0, 0);
            length = (DRV_SERVICES(unit)->reg_length_get)
                (unit, reg_index);
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, addr, (uint32 *)&reg_value64, length)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO5348(unit)) {
                soc_robo_64_val_to_pbmp(unit, &pbmp, reg_value64);
                } else { /* 5347, 53242 */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp);
                    SOC_PBMP_WORD_SET(pbmp, 0, temp);
                }
                if (enable) {
                    SOC_PBMP_PORT_ADD(pbmp, port);
                } else {
                    SOC_PBMP_PORT_REMOVE(pbmp, port);
                }
                if (SOC_IS_ROBO5348(unit)) {
                    soc_robo_64_pbmp_to_val(unit, &pbmp, &reg_value64);
                } else {
                    temp = SOC_PBMP_WORD_GET(pbmp, 0);
                    (DRV_SERVICES(unit)->reg_field_set)
                        (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp);
                }

                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp);
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, addr, (uint32 *)&reg_value64, length)) < 0) {
                    return rv;
                }
            } else {
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, addr, &reg_value, length)) < 0) {
                    return rv;
                }
                temp = 0;
                if (enable) {
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }
    
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, reg_index, &reg_value, fld_index, &temp);
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, addr, &reg_value, length)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            if (SOC_IS_ROBO5348(unit) ||SOC_IS_ROBO5347(unit) ||
                  SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_DTAG_GLO_CTLr(unit, &reg_value)) < 0) {
                    return rv;
                }
                soc_DTAG_GLO_CTLr_field_set(unit, &reg_value, 
                    EN_DTAG_ISPf, &enable);
                if ((rv = REG_WRITE_DTAG_GLO_CTLr(unit, &reg_value)) < 0) {
                    return rv;
                }
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
                
                (DRV_SERVICES(unit)->vlan_prop_set)
                    (unit, DRV_VLAN_PROP_DOUBLE_TAG_MODE, enable);
                
            } else {
                if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                    return rv;
                }
                soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                    EN_DOU_TAGf, &enable);
                if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
                if ((rv = REG_READ_ISP_PORT_SEL_PBMr(
                    unit, (uint32 *)&reg_value64)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO5348(unit)) {
                    soc_ISP_PORT_SEL_PBMr_field_get(unit,
                        (uint32 *)&reg_value64, ISP_PORT_PBMPf, 
                        (uint32 *)&temp64);
                    soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
                } else { /* 5347 */
                    soc_ISP_PORT_SEL_PBMr_field_get(unit,
                        (uint32 *)&reg_value64, ISP_PORT_PBMPf, &temp);
                    SOC_PBMP_WORD_SET(pbmp, 0, temp);
                }
                if (enable) {
                    SOC_PBMP_PORT_ADD(pbmp, port);
                } else {
                    SOC_PBMP_PORT_REMOVE(pbmp, port);
                }
                
                /* CPU is suggested to be ISP port (ASIC suggest it) */
                /* remarked : still allow user to set CPU to none-ISP for the
                 * regression test have test item to set CPU to none-ISP.
                SOC_PBMP_PORT_ADD(pbmp, CMIC_PORT(unit));
                */
                
                if (SOC_IS_ROBO5348(unit)) {
                    soc_robo_64_pbmp_to_val(unit, &pbmp, &temp64);
                    soc_ISP_PORT_SEL_PBMr_field_set(unit,
                        (uint32 *)&reg_value64, ISP_PORT_PBMPf, 
                        (uint32 *)&temp64);
                } else { /* 5347 */
                    temp = SOC_PBMP_WORD_GET(pbmp, 0);
                    soc_ISP_PORT_SEL_PBMr_field_set(unit,
                        (uint32 *)&reg_value64, ISP_PORT_PBMPf, &temp);
               }
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, addr, (uint32 *)&reg_value64, length)) < 0) {
                    return rv;
                }
            } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_ISP_SEL_PORTMAPr(
                    unit,(uint32 *)&reg_value64)) < 0) {
                    return rv;
                }

                soc_ISP_SEL_PORTMAPr_field_get(unit, (uint32 *)&reg_value64,
                    ISP_PORT_PBMPf, &temp);
                SOC_PBMP_WORD_SET(pbmp, 0, temp);
                
                if (enable) {
                    SOC_PBMP_PORT_ADD(pbmp, port);
                } else {
                    SOC_PBMP_PORT_REMOVE(pbmp, port);
                }

                /* CPU is suggested to be ISP port (ASIC suggest it) */
                /* remarked : still allow user to set CPU to none-ISP for the
                 * regression test have test item to set CPU to none-ISP.
                SOC_PBMP_PORT_ADD(pbmp, CMIC_PORT(unit));
                */

                temp = SOC_PBMP_WORD_GET(pbmp, 0);
                soc_ISP_SEL_PORTMAPr_field_set(unit, (uint32 *)&reg_value64,
                    ISP_PORT_PBMPf, &temp);

                if ((rv = REG_WRITE_ISP_SEL_PORTMAPr(
                    unit,(uint32 *)&reg_value64)) < 0) {
                    return rv;
                }
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                
                soc_pbmp_t t_bmp;
                
                SOC_PBMP_CLEAR(t_bmp);
                SOC_PBMP_PORT_ADD(t_bmp, port);
                
                rv = (DRV_SERVICES(unit)->vlan_prop_port_enable_set)
                        (unit, DRV_VLAN_PROP_ISP_PORT, t_bmp, enable);                 
                 
            } else {
                if ((rv = REG_READ_ISP_PORT_SEL_PBMr(unit, &reg_value)) < 0) {
                    return rv;
                }
                soc_ISP_PORT_SEL_PBMr_field_get(unit, &reg_value, 
                    ISP_PORT_PBMPf, &temp);
                if (enable) {
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }

                /* CPU is suggested to be ISP port (ASIC suggest it) */
                /* remarked : still allow user to set CPU to none-ISP for the
                 * regression test have test item to set CPU to none-ISP.
               temp |= (1 << CMIC_PORT(unit));
               */

                soc_ISP_PORT_SEL_PBMr_field_set(unit, &reg_value, 
                    ISP_PORT_PBMPf, &temp);
                if ((rv = REG_WRITE_ISP_PORT_SEL_PBMr(unit, &reg_value)) < 0) {
                    return rv;
                }
            }
            break;
            
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
            /* This feature currently is supported in bcm5395 chip only */
            /* --- TBD --- */
            return SOC_E_UNAVAIL;
            break;
            
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
            /* This feature currently is supported in 5395/53115/53118 robo
             *  chips.
             *
             * Note :
             *  1. bcm5395 can serve PCP remark on SP-tag only. That means 
             *      this feature can be work only if the DT_Mode enabled and 
             *      egress port must be NNI port. Thus we didn't support
             *      bcm5395 here.
             *      
             */
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
                if ((rv = REG_READ_TRREG_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_TRREG_CTRLr_field_get(unit, &reg_value, 
                    PCP_RMK_ENf, &temp);
                if (enable) {
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }

                soc_TRREG_CTRLr_field_set(unit, &reg_value, 
                    PCP_RMK_ENf, &temp);

                if ((rv = REG_WRITE_TRREG_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }
            } else {
                return SOC_E_UNAVAIL;
            }
             
            break;
            
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            /* This feature currently is supported in 53115 robo chip only */
            if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)){
                if ((rv = REG_READ_TRREG_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_TRREG_CTRLr_field_get(unit, &reg_value, 
                    CFI_RMK_ENf, &temp);
                            
                if (enable) {
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }
                
                soc_TRREG_CTRLr_field_set(unit, &reg_value, 
                    CFI_RMK_ENf, &temp);
                    
                if ((rv = REG_WRITE_TRREG_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
            
        default:
            return SOC_E_PARAM;
            break;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_port_property_enable_get
 *
 *  Purpose :
 *      Get the status of port related properties
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      property  :   port property type.
 *      enable  :   status of this property.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_property_enable_get(int unit, int port, int property, uint32 *enable)
{
    uint32    addr, temp = 0, vlan_ctrl3;
    int        rv = SOC_E_NONE;
    int        length;
    uint32    reg_index = 0, fld_index = 0;
    uint32    reg_value;
    uint64    vlan_ctrl3_64, reg_value64, temp64;
    soc_pbmp_t pbmp;

    COMPILER_64_ZERO(temp64);
    switch (property) {
        case DRV_PORT_PROP_ENABLE_RX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_ROBO53101(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                soc_TH_PCTLr_field_get(unit, &reg_value,
                    MIRX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_ROBO53101(unit)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        RX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        MIRX_DISf, &temp);
                }
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_ROBO53101(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                soc_TH_PCTLr_field_get(unit, &reg_value,
                    MITX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_ROBO53101(unit)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        TX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        MITX_DISf, &temp);
                }
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TXRX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_ROBO53101(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                soc_TH_PCTLr_field_get(unit, &reg_value,
                    MIRX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                    return rv;
                }
                soc_TH_PCTLr_field_get(unit, &reg_value,
                    MITX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_ROBO53101(unit)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        RX_DISf, &temp);
                    if (temp) {
                        *enable = FALSE;
                        return rv;
                    }
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        TX_DISf, &temp);
                    if (temp) {
                        *enable = FALSE;
                    } else {
                        *enable = TRUE;
                    }
                } else {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        MIRX_DISf, &temp);
                    if (temp) {
                        *enable = FALSE;
                        return rv;
                    }
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        MITX_DISf, &temp);
                    if (temp) {
                        *enable = FALSE;
                    } else {
                        *enable = TRUE;
                    }
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
            addr =  (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL3r, 0, 0);
            length = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL3r);
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_VLAN_CTRL3r(
                    unit, (uint32 *)&vlan_ctrl3_64)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO5348(unit)) {
                    soc_robo_64_val_to_pbmp(unit, &pbmp, vlan_ctrl3_64);
                } else { /* 5347, 53242 */
                    soc_VLAN_CTRL3r_field_get(unit, (uint32 *)&vlan_ctrl3_64, 
                        EN_DROP_NON1Qf, &temp);
                    SOC_PBMP_WORD_SET(pbmp, 0, temp);
                }
                if (SOC_PBMP_MEMBER(pbmp, port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else {
                /* bcm53115, bcm53118, 53101 can apply this section */
                if ((rv = REG_READ_VLAN_CTRL3r(
                    unit, &vlan_ctrl3)) < 0) {
                    return rv;
                }
                if (vlan_ctrl3 & ( 1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            }
            return rv;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
            return SOC_E_UNAVAIL;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            if (property == DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX) {
                reg_index = INDEX(RX_PAUSE_PASSr);
                fld_index = INDEX(RX_PAUSE_PASSf);
            } else {
                reg_index = INDEX(TX_PAUSE_PASSr);
                fld_index = INDEX(TX_PAUSE_PASSf);
            }
            addr =  (DRV_SERVICES(unit)->reg_addr)
                (unit, reg_index, 0, 0);
            length = (DRV_SERVICES(unit)->reg_length_get)
                (unit, reg_index);
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, addr, (uint32 *)&reg_value64, length)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO5348(unit)) {
                    (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_index, (uint32 *)&reg_value64, fld_index, (uint32 *)&temp64);
                    soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
                } else { /* 5347, 53242 */
                    (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp);
                    SOC_PBMP_WORD_SET(pbmp, 0, temp);
                }
                if (SOC_PBMP_MEMBER(pbmp, port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else {
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, addr, &reg_value, length)) < 0) {
                    return rv;
                }
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_index, &reg_value, fld_index, &temp);
                if (temp & ( 1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_DTAG_GLO_CTLr(unit, &reg_value)) < 0) {
                    return rv;
                }
                soc_DTAG_GLO_CTLr_field_get(unit, &reg_value, 
                    EN_DTAG_ISPf, &temp);
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
                
                (DRV_SERVICES(unit)->vlan_prop_get)
                    (unit, DRV_VLAN_PROP_DOUBLE_TAG_MODE, &temp);
                
            } else {
                if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                    return rv;
                }
                soc_VLAN_CTRL4r_field_get(unit, &reg_value, 
                    EN_DOU_TAGf, &temp);
            }
            *enable = temp;
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            /* 
             * This is temporary write for 5348, 5348's register name should 
             * be the same as other chip. Wait for register profile update to modify
             * here.
             */
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
                addr =  (DRV_SERVICES(unit)->reg_addr)
                    (unit, ISP_PORT_SEL_PBMr, 0, 0);
                length = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, ISP_PORT_SEL_PBMr);
                if ((rv = REG_READ_ISP_PORT_SEL_PBMr(
                    unit, (uint32 *)&reg_value64)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO5348(unit)) {
                    soc_ISP_PORT_SEL_PBMr_field_get(unit,
                        (uint32 *)&reg_value64, ISP_PORT_PBMPf, 
                        (uint32 *)&temp64);
                soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
                } else { /* 5347 */
                    soc_ISP_PORT_SEL_PBMr_field_get(unit,
                        (uint32 *)&reg_value64, ISP_PORT_PBMPf, 
                        &temp);
                    SOC_PBMP_WORD_SET(pbmp, 0, temp);
                }
                if (SOC_PBMP_MEMBER(pbmp, port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_ISP_SEL_PORTMAPr(
                    unit,(uint32 *)&reg_value64)) < 0) {
                    return rv;
                }

                soc_ISP_SEL_PORTMAPr_field_get(unit, (uint32 *)&reg_value64,
                    ISP_PORT_PBMPf, &temp);
                SOC_PBMP_WORD_SET(pbmp, 0, temp);

                if (SOC_PBMP_MEMBER(pbmp, port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                if ((rv = REG_READ_ISP_SEL_PORTMAPr(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_ISP_SEL_PORTMAPr_field_get(unit, &reg_value,
                    ISP_PORTMAPf, &temp);

                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }

            } else if (SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit)  ||
                SOC_IS_ROBO5395(unit)){
                if ((rv = REG_READ_ISP_PORT_SEL_PBMr(unit, &reg_value)) < 0) {
                    return rv;
                }
                soc_ISP_PORT_SEL_PBMr_field_get(unit, &reg_value, 
                    ISP_PORT_PBMPf, &temp);
                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else {
                *enable = FALSE;
            }
            break;

        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
            /* This feature currently is supported in bcm5395 chip only */
            /* --- TBD --- */
            return SOC_E_UNAVAIL;
            break;
            
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
            /* This feature currently is supported in 5395/53115/53118 robo
             *  chips.
             *
             * Note :
             *  1. bcm5395 can serve PCP remark on SP-tag only. That means 
             *      this feature can be work only if the DT_Mode enabled and 
             *      egress port must be NNI port. Thus we didn't support
             *      bcm5395 here.
             *      
             */
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
                if ((rv = REG_READ_TRREG_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_TRREG_CTRLr_field_get(unit, &reg_value, 
                    PCP_RMK_ENf, &temp);
                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;

        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            /* This feature currently is supported in bcm53115 and bcm53118 
             * chip only.
             */
            if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit) ||
                SOC_IS_ROBO53125(unit)){
                if ((rv = REG_READ_TRREG_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }
                
                soc_TRREG_CTRLr_field_get(unit, &reg_value, 
                    CFI_RMK_ENf, &temp);
                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;

        default:
            return SOC_E_PARAM;
            break;
    }
    return SOC_E_NONE;
}

uint32
_drv_sample_rate_get(int unit, uint32 val)
{
    int i,  tmp1 = 0, tmp2 = 0;
    uint32  ret_val = 0, cap = 0, ing_max = 0;
    
    /* Get the maximum value can be set in the register's field */
    /* current design check ingress max value only :
     *  - the ingress/egress RMON/SFLOW feature currently supported in ROBO 
     *      chips in bcm53242/bcm53262/bcm5348/bcm5347/bcm5328x
     */
    if (DRV_DEV_PROP_GET(
            unit, DRV_DEV_PROP_MAX_INGRESS_SFLOW_VALUE, &ing_max)) {
        cap = 0;
        return cap;
    } else {
        if (ing_max == 0){
            cap = 0;
            return cap;
        }
    } 
    cap = ing_max;
    
    /* Get the value to be set, "2" is the base and "i" is the exponenet */
    tmp1 = 1;
    for (i = 0; i < cap; i++) {
        if (tmp1 == val) {
            /* If 2^i equal to the "val", then "i" is the value to be set */
            return i;
        } else {
            /* 
             * Check if the "val" is between 2^i and 2^(i+1)
             * If yes, the nearest exponent is found and stop the loop.
             */
            tmp2 = tmp1 * 2;
            if ((val > tmp1) && (val < tmp2)) {
                /* get the nearest i */
                break;
            }
        }
        tmp1 *= 2;
    }

    /* Check whether 2^i or 2^(i+1) is closer to "val" */
    if ((val - tmp1) <= (tmp2 - val)) {
        ret_val = i;
    } else {
        ret_val = i + 1;
    }

    return ret_val;
}

#define DOT1P_PRI_MASK  0x7
#define DOT1P_CFI_MASK  0x1
#define DOT1P_PRI_SHIFT 13
#define DOT1P_CFI_SHIFT 12

/*
 *  Function : drv_port_pri_mapop_set
 *
 *  Purpose :
 *      Port basis priority mapping operation configuration set
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      op_type     :   operation type
 *      pri_old     :   old priority.
 *      pri_new     :   new priority.
 *      cfi_new     :   new cfi.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. This driver service is designed for priority operation exchange.
 *  2. Priority type could be dot1p, DSCP, port based.
 *
 */
int 
drv_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new)
{
    uint32  temp;
    uint64  reg_value64;
    
    switch (op_type) {
        case DRV_PORT_OP_PCP2TC :
            /* this feature is supported on 5395/53115/53118 robo chips.
             * For the bcm5395 can serves SP-Tag only, here SW to design this 
             *  feature on bcm53115 and 53118 only. 
             *  (bcm5395 need more information and confirmation to implement)
             */
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
                temp = pri_new & DOT1P_PRI_MASK;
                SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_SET
                    (unit, port, pri_old, (uint8)temp));
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_OP_NORMAL_TC2PCP:
            /* this feature is supported on 5395/53115/53118 robo chips.
             * For the bcm5395 can serves SP-Tag only, here SW to design this 
             *  feature on bcm53115 and 53118 only. 
             *  (bcm5395 need more information and confirmation to implement)
             */
            if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit) ||
                SOC_IS_ROBO53101(unit)){
                SOC_IF_ERROR_RETURN(
                    REG_READ_EGRESS_NRESE_PKT_TC2PCP_MAPr
                    (unit, port, (uint32 *)&reg_value64));

                temp = ((cfi_new & DOT1P_CFI_MASK) << 3) | 
                        (pri_new & DOT1P_PRI_MASK);
                
                /* assigning the field-id */
                switch(pri_old) {
                    case 0:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC0f, &temp));
                        break;
                    case 1:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC1f, &temp));
                        break;
                    case 2:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC2f, &temp));
                        break;
                    case 3:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC3f, &temp));
                        break;
                    case 4:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC4f, &temp));
                        break;
                    case 5:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC5f, &temp));
                        break;
                    case 6:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC6f, &temp));
                        break;
                    case 7:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC7f, &temp));
                        break;
                    default :
                        return SOC_E_PARAM;
                        break;
                }
                
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EGRESS_NRESE_PKT_TC2PCP_MAPr
                    (unit, port, (uint32 *)&reg_value64));
            } else if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EGRESS_NAVB_PKT_TC2PCP_MAPr(
                    unit, port, (uint32 *)&reg_value64));

                if (SOC_IS_ROBO53128(unit) && (cfi_new != 0)) {
                    /* BCM53128 didn't support CFI remarking */
                    return SOC_E_UNAVAIL;
                }
                temp = ((cfi_new & DOT1P_CFI_MASK) << 3) | 
                        (pri_new & DOT1P_PRI_MASK);
                
                /* assigning the field-id */
                switch(pri_old) {
                    case 0:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC0f, &temp));
                        break;
                    case 1:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC1f, &temp));
                        break;
                    case 2:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC2f, &temp));
                        break;
                    case 3:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC3f, &temp));
                        break;
                    case 4:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC4f, &temp));
                        break;
                    case 5:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC5f, &temp));
                        break;
                    case 6:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC6f, &temp));
                        break;
                    case 7:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC7f, &temp));
                        break;
                    default :
                        return SOC_E_PARAM;
                        break;
                }
                    
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EGRESS_NAVB_PKT_TC2PCP_MAPr(
                    unit, port, (uint32 *)&reg_value64));
            } else {
                return SOC_E_UNAVAIL;
            }

            break;
        case DRV_PORT_OP_OUTBAND_TC2PCP:
            /* this feature is supported on bcm53115 only. */
            if (SOC_IS_ROBO53115(unit)){
                SOC_IF_ERROR_RETURN(
                    REG_READ_EGRESS_NRESE_PKT_TC2PCP_MAPr
                    (unit, port, (uint32 *)&reg_value64));

                temp = ((cfi_new & DOT1P_CFI_MASK) << 3) | 
                        (pri_new & DOT1P_PRI_MASK);

                /* assigning the field-id */
                switch(pri_old) {
                    case 0:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC0f, &temp));
                        break;
                    case 1:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC1f, &temp));
                        break;
                    case 2:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC2f, &temp));
                        break;
                    case 3:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC3f, &temp));
                        break;
                    case 4:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC4f, &temp));
                        break;
                    case 5:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC5f, &temp));
                        break;
                    case 6:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC6f, &temp));
                        break;
                    case 7:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC7f, &temp));
                        break;
                    default :
                        return SOC_E_PARAM;
                        break;
                }
                
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EGRESS_NRESE_PKT_TC2PCP_MAPr
                    (unit, port, (uint32 *)&reg_value64));
            } else if (SOC_IS_ROBO53125(unit)) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EGRESS_NAVB_PKT_TC2PCP_MAPr(
                    unit, port, (uint32 *)&reg_value64));

                temp = ((cfi_new & DOT1P_CFI_MASK) << 3) | 
                        (pri_new & DOT1P_PRI_MASK);
                
                /* assigning the field-id */
                switch(pri_old) {
                    case 0:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC0f, &temp));
                        break;
                    case 1:
                           SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC1f, &temp));
                        break;
                    case 2:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC2f, &temp));
                        break;
                    case 3:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC3f, &temp));
                        break;
                    case 4:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC4f, &temp));
                        break;
                    case 5:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC5f, &temp));
                        break;
                    case 6:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC6f, &temp));
                        break;
                    case 7:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_set
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC7f, &temp));
                        break;
                    default :
                        return SOC_E_PARAM;
                        break;
                }

                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EGRESS_NAVB_PKT_TC2PCP_MAPr(
                    unit, port, (uint32 *)&reg_value64));
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        default:
            break;
    }
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_port_pri_mapop_get
 *
 *  Purpose :
 *      Port basis priority mapping operation configuration get
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      pri_old     :   (in)old priority.
 *      cfi_old     :   (in)old cfi (No used).
 *      pri_new     :   (out)new priority.
 *      cfi_new     :   (out)new cfi.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. This driver service is designed for priority operation exchange.
 *  2. Priority type could be dot1p, DSCP, port based.
 *
 */
int 
drv_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new)
{
    uint32  temp;
    uint64  reg_value64;
    uint8  temp8 = 0;

    switch (op_type) {
        case DRV_PORT_OP_PCP2TC :
            /* this feature is supported on 5395/53115/53118 robo chips.
             * For the bcm5395 can serves SP-Tag only, here SW to design this 
             *  feature on bcm53115 and 53118 only. 
             *  (bcm5395 need more information and confirmation to implement)
             */
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
                temp8 = 0;
                SOC_IF_ERROR_RETURN(DRV_QUEUE_PRIO_REMAP_GET
                    (unit, port, pri_old, &temp8));
                temp = (uint32)temp8;
                *pri_new = temp;
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_OP_NORMAL_TC2PCP:
            /* this feature is supported on 5395/53115/53118 robo chips.
             * For the bcm5395 can serves SP-Tag only, here SW to design this 
             *  feature on bcm53115 and 53118 only. 
             *  (bcm5395 need more information and confirmation to implement)
             */
            if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit) ||
                SOC_IS_ROBO53101(unit)){
                SOC_IF_ERROR_RETURN(
                    REG_READ_EGRESS_NRESE_PKT_TC2PCP_MAPr
                    (unit, port, (uint32 *)&reg_value64));
                
                /* assigning the field-id */
                switch(pri_old) {
                    case 0:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC0f, &temp));
                        break;
                    case 1:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC1f, &temp));
                        break;
                    case 2:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC2f, &temp));
                        break;
                    case 3:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC3f, &temp));
                        break;
                    case 4:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC4f, &temp));
                        break;
                    case 5:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC5f, &temp));
                        break;
                    case 6:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC6f, &temp));
                        break;
                    case 7:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC7f, &temp));
                        break;
                    default :
                        return SOC_E_PARAM;
                        break;
                }
                
                *pri_new = temp & DOT1P_PRI_MASK;
                *cfi_new = (temp >> 3) & DOT1P_CFI_MASK;
                        
            } else if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EGRESS_NAVB_PKT_TC2PCP_MAPr(
                    unit, port, (uint32 *)&reg_value64));
                
                /* assigning the field-id */
                switch(pri_old) {
                    case 0:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC0f, &temp));
                        break;
                    case 1:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC1f, &temp));
                        break;
                    case 2:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC2f, &temp));
                        break;
                    case 3:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC3f, &temp));
                        break;
                    case 4:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC4f, &temp));
                        break;
                    case 5:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC5f, &temp));
                        break;
                    case 6:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC6f, &temp));
                        break;
                    case 7:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV0_TC7f, &temp));
                        break;
                    default :
                        return SOC_E_PARAM;
                        break;
                }
                    
                *pri_new = temp & DOT1P_PRI_MASK;
                *cfi_new = (temp >> 3) & DOT1P_CFI_MASK;
                
            } else {
                return SOC_E_UNAVAIL;
            }

            break;
        case DRV_PORT_OP_OUTBAND_TC2PCP:
            /* this feature is supported on bcm53115 only. */
            if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit)){
                SOC_IF_ERROR_RETURN(
                    REG_READ_EGRESS_NRESE_PKT_TC2PCP_MAPr
                    (unit, port, (uint32 *)&reg_value64));
                
                /* assigning the field-id */
                switch(pri_old) {
                    case 0:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC0f, &temp));
                        break;
                    case 1:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC1f, &temp));
                        break;
                    case 2:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC2f, &temp));
                        break;
                    case 3:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC3f, &temp));
                        break;
                    case 4:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC4f, &temp));
                        break;
                    case 5:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC5f, &temp));
                        break;
                    case 6:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC6f, &temp));
                        break;
                    case 7:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NRESE_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC7f, &temp));
                        break;
                    default :
                        return SOC_E_PARAM;
                        break;
                }
                
                *pri_new = temp & DOT1P_PRI_MASK;
                *cfi_new = (temp & DOT1P_CFI_MASK) >> 3;
                        
            } else if (SOC_IS_ROBO53125(unit)) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EGRESS_NAVB_PKT_TC2PCP_MAPr(
                    unit, port, (uint32 *)&reg_value64));
                
                /* assigning the field-id */
                switch(pri_old) {
                    case 0:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC0f, &temp));
                        break;
                    case 1:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC1f, &temp));
                        break;
                    case 2:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC2f, &temp));
                        break;
                    case 3:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC3f, &temp));
                        break;
                    case 4:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC4f, &temp));
                        break;
                    case 5:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC5f, &temp));
                        break;
                    case 6:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC6f, &temp));
                        break;
                    case 7:
                        SOC_IF_ERROR_RETURN(
                            soc_EGRESS_NAVB_PKT_TC2PCP_MAPr_field_get
                            (unit, (uint32 *)&reg_value64, PCP_FOR_RV1_TC7f, &temp));
                        break;
                    default :
                        return SOC_E_PARAM;
                        break;
                }
                    
                *pri_new = temp & DOT1P_PRI_MASK;
                *cfi_new = (temp >> 3) & DOT1P_CFI_MASK;
            } else {
                return SOC_E_UNAVAIL;
            }

            break;
        default:
            break;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_port_oper_mode_set
 *
 *  Purpose :
 *      Set operation mode to  specific ports.
 *      (This is not suitable for ROBO)
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      oper_mode  :   operation mode.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
 int 
 drv_port_oper_mode_set(int unit, soc_pbmp_t bmp, uint32 oper_mode)
{
    soc_cm_debug(DK_PORT, 
    "drv_port_oper_mode_set : unit %d, bmp = %x,  mode = %d not support\n",
         unit, SOC_PBMP_WORD_GET(bmp, 0), oper_mode);
    return SOC_E_UNAVAIL;
}

 /*
 *  Function : drv_port_oper_mode_get
 *
 *  Purpose :
 *      Get operation mode to specific ports.
 *      (This is not suitable for ROBO)
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      oper_mode  :   operation mode.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_oper_mode_get(int unit, int port_n, uint32 *oper_mode)
{
    soc_cm_debug(DK_PORT, 
    "drv_port_oper_mode_get : unit %d, port = %x,  mode = %d not support\n",
         unit, port_n, *oper_mode);
    return SOC_E_UNAVAIL;
}


 /*
 *  Function : drv_port_set
 *
 *  Purpose :
 *      Set the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_set(int unit, soc_pbmp_t bmp, uint32 prop_type, uint32 prop_val)
{
    uint32 reg_value, temp = 0;
    int port, conf_port = 0;
    uint64  reg_value64, temp64;
    soc_pbmp_t tmp_pbmp;
    int rv = SOC_E_NONE;
    uint8 cosq = 0;
#if ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED
    int no_fe10_loopback = 0;   /* special limitation for ROBO fe phy */
#endif  /* #if ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED */
    mac_driver_t *p_mac = NULL;

    soc_cm_debug(DK_PORT, "drv_port_set: unit=%d bmp=%x %x\n", \
        unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1));

    SOC_ROBO_PORT_INIT(unit);

    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            soc_cm_debug(DK_PORT,"drv_port_set: PROP_SPEED\n");

            /* BCM53222 is 24FE+2FE */
            if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                if (bcm53222_attached) {
                    PBMP_ITER(bmp, port) {
                        if ((port == 25) || (port == 26)) {
                            if (prop_val == DRV_PORT_STATUS_SPEED_1G) {
                                return SOC_E_PARAM;
                            }
                        }
                    }
                }
            }

            temp = 0;
            switch (prop_val) {
                case DRV_PORT_STATUS_SPEED_10M:
                    temp = 10;
                    break;
                case DRV_PORT_STATUS_SPEED_100M:
                    temp =100;
                    break;
                case DRV_PORT_STATUS_SPEED_1G:
                    temp = 1000;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY and MAC auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);
             
                /* set MAC auto-negotiation OFF */
                drv_set_MAC_auto_negotiation(unit, port, FALSE);

                /* Set PHY registers anyway. */
                rv = soc_phyctrl_speed_set(unit, port, temp);

                /* if auto-negotiation is OFF, */
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_speed_set != NULL) {
                        rv = MAC_SPEED_SET(
                            p_mac, 
                            unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;

        case DRV_PORT_PROP_DUPLEX:
            soc_cm_debug(DK_PORT,"drv_port_set: PROP_DUPLEX\n");
            switch (prop_val) {
                case DRV_PORT_STATUS_DUPLEX_HALF:
                    temp = FALSE;
                    break;
                case DRV_PORT_STATUS_DUPLEX_FULL:
                    temp =TRUE;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);

                /* set MAC auto-negotiation OFF */
                drv_set_MAC_auto_negotiation(unit, port, FALSE);

                /* Set PHY registers anyway. */
                rv = soc_phyctrl_duplex_set(unit, port, temp);

                /* set MAC duplex */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_duplex_set != NULL) {
                        rv = MAC_DUPLEX_SET(
                            p_mac, unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_AUTONEG:
        case DRV_PORT_PROP_RESTART_AUTONEG:
            soc_cm_debug(DK_PORT,"drv_port_set: PROP_AUTONEG\n");
            /* RE_AN in PHY driver to set AN will be executed also */
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, prop_val);

                /* set MAC auto-negotiation */
                drv_set_MAC_auto_negotiation(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_TX_PAUSE\n");
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_RX_PAUSE\n");
            PBMP_ITER(bmp, port) {
                 /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, -1, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            soc_cm_debug(DK_PORT, "drv_port_set: LOCAL_ADVER\n");
            PBMP_ITER(bmp, port) {
                /* set advertise to PHY accordingly */
                rv = soc_phyctrl_adv_local_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: REMOTE_ADVER not support\n");
            /* can not set remote advert */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PORT_ABILITY not support\n");
            /* can not be set */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_MAC_ABILITY not support\n");
            /* can not be set */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_ABILITY not support\n");
            /* can not be set */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_INTERFACE:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_INTERFACE\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_interface_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_MAC_ENABLE:
            /* This case is called for _bcm_robo_port_update() only. */
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MAC_ENABLE\n");
            PBMP_ITER(bmp, port) {
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_enable_set != NULL) {
                        rv = MAC_ENABLE_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_ENABLE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_ENABLE\n");
            PBMP_ITER(bmp, port) {
                /* for enable, set MAC first and than PHY.
                 * for disable, set PHY first and than MAC
                 */
                if (prop_val) {
                    /* MAC register(s) should be set also */
                    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                    if (p_mac != NULL) {
                        if (p_mac->md_enable_set != NULL) {
                            rv = MAC_ENABLE_SET(
                                p_mac, unit, port, prop_val);
                        } else {
                            rv = SOC_E_UNAVAIL;
                        }
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                    /* Set PHY registers anyway. */
                    rv = soc_phyctrl_enable_set(unit, port, prop_val);
                } else {
                    /* Set PHY registers anyway. */
                    rv = soc_phyctrl_enable_set(unit, port, prop_val);
                    /* MAC register(s) should be set also */
                    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                    if (p_mac != NULL) {
                        if (p_mac->md_enable_set != NULL) {
                            rv = MAC_ENABLE_SET(
                                p_mac, unit, port, prop_val);
                        } else {
                            rv = SOC_E_UNAVAIL;
                        }
                    } else {
                        rv = SOC_E_PARAM;
                    }
                }
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_ENABLE_SET\n");
            PBMP_ITER(bmp, port) {
                rv = drv_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_IPG_FE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_IPG_FE\n");
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) { /* per system */
                if ((rv = REG_READ_SWMODEr(unit, &reg_value)) < 0) {
                    return rv;
                }
                if (prop_val > 92) {
                    temp = 3;
                } else if (prop_val > 88) {
                    temp = 2;
                } else if (prop_val > 84) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                soc_SWMODEr_field_set(unit, &reg_value,
                    IPGf, &temp);
                if ((rv = REG_WRITE_SWMODEr(unit, &reg_value)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_IPG_GE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_IPG_GE\n");
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) { /* per system */
                if ((rv = REG_READ_SWMODEr(unit, &reg_value)) < 0) {
                    return rv;
                }
                if (prop_val > 88) {
                    temp = 3;
                } else {
                    temp = 0;
                }
                soc_SWMODEr_field_set(unit, &reg_value,
                    IPGf, &temp);
                if ((rv = REG_WRITE_SWMODEr(unit, &reg_value)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_JAM:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_JAM\n");
            
            /* Robo chip use binding Jamming/Pause, Jamming/Pause can't be set
             * independent.
             */
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_BPDU_RX:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_BPDU_RX\n");
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) {
                if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
                    return rv;
                }
                if (prop_val) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                soc_GMNGCFGr_field_set(unit, &reg_value, 
                    RXBPDU_ENf, &temp);
                if ((rv = REG_WRITE_GMNGCFGr(unit, &reg_value)) < 0) {
                    return rv;
                }
            }
        break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MAC_LOOPBACK\n");
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_lb_set != NULL) {
                        rv = MAC_LOOPBACK_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_PHY_LOOPBACK\n");
            PBMP_ITER(bmp, port) {
                /* Special case :
                 *  - Design team confirmed that integrated FE PHY after 
                 *      robo5348 device will be limited to serve phy loopback
                 *      at 10MB speed.
                 */
#if ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED
                if (IS_FE_PORT(unit, port)){
                    if (SOC_IS_ROBO5348(unit)){
                        no_fe10_loopback = (port > 23) ? 1 : 0;
                    } else if (SOC_IS_ROBO5347(unit)){
                        no_fe10_loopback = (port < 24) ? 1 : 0;
                    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
                        no_fe10_loopback = (port < 24) ? 1 : 0;
                    }
                    if (no_fe10_loopback){
                        SOC_IF_ERROR_RETURN(
                                REG_READ_MIICTLr(unit, port, &reg_value));
                        soc_MIICTLr_field_get(unit, &reg_value, F_SPD_SELf, 
                                &temp);
                        if (!temp) {    /* F_SPD_SEL == b0 means speed=10 */
                            soc_cm_debug(DK_WARN, 
                                    "No loopback on port%d for speed=10!\n",
                                     port);
                            return SOC_E_UNAVAIL;
                        }
                    }
                }
#endif  /* ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED */
                rv = soc_phyctrl_loopback_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MEDIUM not support\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_COPPER:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MEDIUM_CONFIG_COPPER\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_medium_config_set(unit, port,
                                                SOC_PORT_MEDIUM_COPPER,
                                                (soc_phy_config_t *)prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_FIBER:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MEDIUM_CONFIG_FIBER\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_medium_config_set(unit, port, 
                                                SOC_PORT_MEDIUM_FIBER,
                                                (soc_phy_config_t *)prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_PHY_MDIX\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_mdix_set(unit, port, prop_val); 
            }
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MDIX_STATUS not support\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MS:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MS not support\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_master_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_SEC_MODE_NONE\n");
            PBMP_ITER(bmp, port) {
                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                    rv = _drv_bcm5348_port_security_mode_set(unit, port, prop_type, 0);
                } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                    rv = _drv_bcm53242_port_security_mode_set(unit, port, prop_type, 0);
                } else {
                    rv = _drv_port_security_mode_set(unit, port, prop_type, 0);
                }
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_SEC_MODE_STATIC_ACCEPT\n");
            PBMP_ITER(bmp, port) {
                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) ||
                    SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                    rv = SOC_E_UNAVAIL;
                } else {
                    rv = _drv_port_security_mode_set(unit, port, prop_type, 0);
                }
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_SEC_MODE_STATIC_REJECT\n");
            PBMP_ITER(bmp, port) {
                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) ||
                    SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                    rv = SOC_E_UNAVAIL;
                } else {
                    rv = _drv_port_security_mode_set(unit, port, prop_type, 0);
                }
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_SEC_MODE_DYNAMIC_SA_NUM\n");
            PBMP_ITER(bmp, port) {
                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                    rv = _drv_bcm5348_port_security_mode_set(
                        unit, port, prop_type, prop_val);
                } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                    rv = _drv_bcm53242_port_security_mode_set(
                        unit, port, prop_type, prop_val);
                } else {
                    rv = _drv_port_security_mode_set(
                        unit, port, prop_type, prop_val);
                }
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_SEC_MODE_DYNAMIC_SA_MATCH\n");
            PBMP_ITER(bmp, port) {
                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                    rv = _drv_bcm5348_port_security_mode_set(unit, port, prop_type, 0);
                } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                    rv = _drv_bcm53242_port_security_mode_set(unit, port, prop_type, 0);
                } else {
                    rv = _drv_port_security_mode_set(unit, port, prop_type, 0);
                }
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_SEC_MAC_MODE_EXTEND\n");
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                PBMP_ITER(bmp, port) {
                    rv = _drv_bcm53242_port_security_mode_set(unit, port, prop_type, 0);
                }
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;

        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_SEC_MAC_MODE_SIMPLIFY\n");
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                PBMP_ITER(bmp, port) {
                    rv = _drv_bcm53242_port_security_mode_set(unit, port, prop_type, 0);
                }
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP\n");
            PBMP_ITER(bmp, port) {
                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                    SOC_IF_ERROR_RETURN(_drv_bcm5348_port_security_mode_set
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, TRUE));
                } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                    SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_set
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_EXTEND, TRUE));
                    SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_set
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, TRUE));
                } else {
                    SOC_IF_ERROR_RETURN(_drv_port_security_mode_set
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, TRUE));
                }
                temp = ((int)prop_val < 0) ? 0 : prop_val;
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET(unit, 
                        port, DRV_PORT_SA_LRN_CNT_LIMIT, (int)temp));
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU\n");
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                PBMP_ITER(bmp, port) {
                    SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_set
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY, TRUE));
                    SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_set
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, TRUE));
                    temp = ((int)prop_val < 0) ? 0 : prop_val;
                    SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET(unit, 
                            port, DRV_PORT_SA_LRN_CNT_LIMIT, (int)temp));
                }
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
            PBMP_ITER(bmp, port) {
                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                    SOC_IF_ERROR_RETURN(_drv_bcm5348_port_security_mode_set
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_NONE, TRUE));
                } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                    SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_set
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_NONE, TRUE));
                } else {
                    SOC_IF_ERROR_RETURN(_drv_port_security_mode_set
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_NONE, TRUE));
                }
                /* clear learn limit */
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET
                        (unit, port, DRV_PORT_SA_LRN_CNT_LIMIT, 0));
            }
            break;
        case DRV_PORT_PROP_PHY_LINKUP_EVT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_LINKUP_EVT\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkup_evt(unit, port);
                if (SOC_FAILURE(rv) && (rv != SOC_E_UNAVAIL)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_LINKDN_EVT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_LINKDN_EVT\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkdn_evt(unit, port);
                if (SOC_FAILURE(rv) && (rv != SOC_E_UNAVAIL)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_RESET:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_RESET\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phy_reset(unit, port);
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_DTAG_MODE\n");
            rv = drv_port_property_enable_set(unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_DTAG_ISP_PORT\n");
            if (!(SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit))) {
                return SOC_E_UNAVAIL;
            }
            PBMP_ITER(bmp, port) {
                rv = drv_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_DTAG_TPID\n");
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_ISP_VIDr(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_ISP_VIDr_field_set(unit, &reg_value,
                    ISP_VLAN_DELIMITERf, &prop_val);
                if ((rv = REG_WRITE_ISP_VIDr(unit, &reg_value)) < 0) {
                    return rv;
                }
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_802_1X_MODE :
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_802_1X_MODE\n");
            
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = SOC_E_PARAM;
            } else {
            PBMP_ITER(bmp, port) {
                _drv_port_802_1x_config_set(unit, port, prop_type, prop_val);
            }
            }
            break;
        case DRV_PORT_PROP_802_1X_BLK_RX :
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_802_1X_BLK_RX\n");
            
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = SOC_E_PARAM;
            } else {
            PBMP_ITER(bmp, port) {
                _drv_port_802_1x_config_set(unit, port, prop_type, prop_val);
                }
            }
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = ((DRV_SERVICES(unit)->vlan_prop_port_enable_set)
                    (unit, DRV_VLAN_PROP_MAC2V_PORT, bmp,
                    prop_val));
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_frame_max_set != NULL) {
                        rv = MAC_FRAME_MAX_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_INGRESS_VLAN_CHK\n");
            if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            
            soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                INGR_VID_CHKf, &prop_val);
            if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)||
                SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                /*
                 * The following 2 steps are implemented for sFlow configuration.
                 * Step 1. Enable or disable the port's ingress sFlow.
                 * Step 2. Set sFlow rate if it's not a disabled port.
                 */
                /* 
                 * Step 1. enable or disable port's sFlow.
                 */
                if ((rv = REG_READ_INGRESS_RMONr(
                    unit,(uint32 *)&reg_value64)) < 0) {
                    return rv;
                }
                COMPILER_64_ZERO(temp64);
                if (SOC_IS_ROBO5348(unit)) {
                    soc_INGRESS_RMONr_field_get(unit, (uint32 *)&reg_value64,
                        EN_INGRESS_PORTMAPf, (uint32 *)&temp64);
                    soc_robo_64_val_to_pbmp(unit, &tmp_pbmp, temp64);
                } else { /* 5347, 53242, 53262 */
                    soc_INGRESS_RMONr_field_get(unit, (uint32 *)&reg_value64,
                        EN_INGRESS_PORTMAPf, &temp);
                    SOC_PBMP_WORD_SET(tmp_pbmp, 0, temp);
                }

                if (!prop_val) {
                    /* disable the port's sFlow */
                    SOC_PBMP_REMOVE(tmp_pbmp, bmp);
                } else {
                    /* enable the port's sFlow */
                    SOC_PBMP_OR(tmp_pbmp, bmp);
                }

                if (SOC_IS_ROBO5348(unit)) {
                    soc_robo_64_pbmp_to_val(unit, &tmp_pbmp, &temp64);
                    soc_INGRESS_RMONr_field_set(unit, (uint32 *)&reg_value64,
                        EN_INGRESS_PORTMAPf, (uint32 *)&temp64);
                } else {
                    temp = SOC_PBMP_WORD_GET(tmp_pbmp, 0);
                    soc_INGRESS_RMONr_field_set(unit, (uint32 *)&reg_value64,
                        EN_INGRESS_PORTMAPf, &temp);
                }

                /* 
                 * Step 2. Set sample rate. 
                 * If the port is going to be disabled, 
                 * no sample rate configuration needed.
                 */
                if (prop_val) {
                    temp = _drv_sample_rate_get(unit, prop_val);
                    if (!temp) {
                        /* 
                         * For BCM5348/5347/53242/53262, 
                         * sample rate 1/1 is not supported.
                         */
                        return SOC_E_PARAM;
                    } else {
                        temp -= 1;
                    }
                    soc_INGRESS_RMONr_field_set(unit, (uint32 *)&reg_value64,
                        INGRESS_CFGf, &temp);

                    /* update software copy of enabled ports. */
                    PBMP_ITER(bmp, port) {
                        SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate = 
                            prop_val;
                    }
                    /* 
                     * Then, update software copy of other enabled ports. 
                     * since all enabled ports share one configured value.
                     */
                    PBMP_ITER(PBMP_ALL(unit), port) {
                        if (SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate) {
                            SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate = 
                                prop_val;
                        }
                    }
                } else {
                    /* update software copy of disabled ports. */
                    PBMP_ITER(bmp, port) {
                        SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate = 
                            prop_val;
                    }
                }

                if ((rv = REG_WRITE_INGRESS_RMONr(
                    unit,(uint32 *)&reg_value64)) < 0) {
                    return rv;
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)||
                SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_EGRESS_RMONr(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_EGRESS_RMONr_field_get(unit, &reg_value, 
                    EGRESS_Pf, &temp);
                port = (int)temp;
                if (SOC_PORT_VALID(unit, port)) {
                    /* 
                     * If the port that desired to disable is not 
                     * the previous enabled port, 
                     * do nothing.
                     */
                    if ((!prop_val) && !SOC_PBMP_MEMBER(bmp, port)) {
                        return SOC_E_NONE;
                    }
                } else {
                    /* 
                     * If the value of field EGRESS_Pf is not a valid port,
                     * the field is not initialized or configured yet. Only
                     * the parameter "prop_val" needs to be checked.
                     */
                    if (!prop_val) {
                        return SOC_E_NONE;
                    }
                }

                if (!prop_val) {
                    /* disable the port's sFlow */
                    temp = 0;
                } else {
                    /* enable the port's sFlow */
                    temp = 1;
                }
                soc_EGRESS_RMONr_field_set(unit, &reg_value, 
                    EN_EGRESS_RMONf, &temp);

                PBMP_ITER(bmp, port) {
                    soc_EGRESS_RMONr_field_set(unit, &reg_value, 
                        EGRESS_Pf, (uint32 *)&port);
                    conf_port = port;
                }

                if (prop_val) {
                    temp = _drv_sample_rate_get(unit, prop_val);
                    if (!temp) {
                        /* 
                         * For BCM5348/5347/53242/53262, 
                         * sample rate 1/1 is not supported.
                         */
                        return SOC_E_PARAM;
                    } else {
                        temp -= 1;
                    }
                    soc_EGRESS_RMONr_field_set(unit, &reg_value, 
                        EGRESS_CFGf, &temp);

                    /* 
                     * Clear software copy of all ports. 
                     * Since there is only one port can be 
                     * egress sample port at one time.
                     */
                    PBMP_ITER(PBMP_ALL(unit), port) {
                        SOC_ROBO_PORT_INFO(unit, port).eg_sample_rate = 0;
                    }

                    /* Only update software copy of the configured port. */
                    SOC_ROBO_PORT_INFO(unit, conf_port).eg_sample_rate = prop_val;
                } else {
                    /* 
                     * Clear software copy of all ports. 
                     * Since there is only one port can be 
                     * egress sample port at one time.
                     */
                    PBMP_ITER(PBMP_ALL(unit), port) {
                        SOC_ROBO_PORT_INFO(unit, port).eg_sample_rate = 0;
                    }
                }
                if ((rv = REG_WRITE_EGRESS_RMONr(unit, &reg_value)) < 0) {
                    return rv;
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)||
                SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_INGRESS_RMONr(
                    unit,(uint32 *)&reg_value64)) < 0) {
                    return rv;
                }

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->queue_prio_get)
                                    (unit, -1, prop_val, &cosq));

                temp = cosq;
                soc_INGRESS_RMONr_field_set(unit, (uint32 *)&reg_value64, 
                    INGRESS_PRIf, &temp);

                if ((rv = REG_WRITE_INGRESS_RMONr(
                    unit,(uint32 *)&reg_value64)) < 0) {
                    return rv;
                }

                /* 
                 * Priority of sFlow packets is a global configuration, 
                 * save the configured value at port #0.
                 */
                port = 0;
                SOC_ROBO_PORT_INFO(unit, port).ing_sample_prio = prop_val;
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)||
                SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {

                if ((rv = REG_READ_EGRESS_RMONr(unit, &reg_value)) < 0) {
                    return rv;
                }

                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->queue_prio_get)
                                    (unit, -1, prop_val, &cosq));
                temp = cosq;
                soc_EGRESS_RMONr_field_set(unit, &reg_value, 
                    EGRESS_PRIf, &temp);

                if ((rv = REG_WRITE_EGRESS_RMONr(unit, &reg_value)) < 0) {
                    return rv;
                }

                /* 
                 * Priority of sFlow packets is a global configuration, 
                 * save the configured value at port #0.
                 */
                port = 0;
                SOC_ROBO_PORT_INFO(unit, port).eg_sample_prio = prop_val;
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_ROAMING_OPT:
            if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
                if (prop_val & !((uint32)DRV_SA_MOVE_ARL)){
                    /* bcm53242/bcm53262 support DRV_SA_MOVE_ARL only */
                    rv = SOC_E_UNAVAIL;
                } else {
                    PBMP_ITER(bmp, port) {
                        SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr(unit, 
                                port, &reg_value));
                        temp = (prop_val & DRV_SA_MOVE_ARL) ? 1 : 0;
                        SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_set(unit, 
                                &reg_value, ROAMING_OPTf, &temp));
                        SOC_IF_ERROR_RETURN(REG_WRITE_PORT_SEC_CONr(unit, 
                                port, &reg_value));
                    }
                }
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        /* not supported list */
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
            rv = SOC_E_UNAVAIL;
            break;
        default: 
            rv = SOC_E_PARAM; 
            break;
    }

    soc_cm_debug(DK_PORT, "drv_port_set: Exit\n");
    return rv;
}

/*
 *  Function : drv_port_get
 *
 *  Purpose :
 *      Get the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int drv_port_get(int unit, int port, uint32 prop_type, uint32 *prop_val)
{
    int         rv = SOC_E_NONE;
    uint32     reg_value, temp = 0;
    uint32     mac_ability = 0, phy_ability;
    int        pause_tx, pause_rx;
    int        autoneg, done;
    soc_pbmp_t tmp_pbmp;
    uint64  reg_value64, temp64;
    mac_driver_t *p_mac = NULL;

    soc_cm_debug(DK_PORT, "drv_port_get: unit=%d port=%d\n", unit, port);

    SOC_ROBO_PORT_INIT(unit);
    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            soc_cm_debug(DK_PORT, "drv_port_get: Speed\n");
            temp = 0;

            rv = soc_phyctrl_speed_get(unit, port, (int *) &temp);
            if (rv != 0){
                return rv;
            }

            switch(temp) {
                case 10:
                    *prop_val = DRV_PORT_STATUS_SPEED_10M;
                    break;
                case 100:
                    *prop_val = DRV_PORT_STATUS_SPEED_100M;
                    break;
                case 1000:
                    *prop_val = DRV_PORT_STATUS_SPEED_1G;
                    break;
                default:
                    *prop_val = 0;
                    break;
            }
            break;
        case DRV_PORT_PROP_DUPLEX:
            soc_cm_debug(DK_PORT, "drv_port_get: Duplex\n");
            temp = 0;

            rv = soc_phyctrl_duplex_get(unit, port, (int *) &temp);
            if (rv != 0){
                return rv;
            }

            switch(temp) {
                case 0:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_HALF;
                    break;
                case 1:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_FULL;
                    break;
                default:
                    break;
            }
            break;
        case DRV_PORT_PROP_AUTONEG:
            soc_cm_debug(DK_PORT, "drv_port_get: Autoneg\n");
            
            rv = soc_phyctrl_auto_negotiate_get(unit, port,
                                                    &autoneg, &done);
            *prop_val = (autoneg) ? DRV_PORT_STATUS_AUTONEG_ENABLE :
                        DRV_PORT_STATUS_AUTONEG_DISABLED;
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_get: TX Pause\n");
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac, 
                        unit, port, (int *) prop_val, &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_get: RX Pause\n");
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac,
                        unit, port, &pause_tx, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            soc_cm_debug(DK_PORT, "drv_port_get: Local Advertise\n");
            rv = soc_phyctrl_adv_local_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            soc_cm_debug(DK_PORT, "drv_port_get: Remote Advertise\n");
            /* if auto-negotiation is ON and negotiation is completed */
            /*   get remote advertisement from PHY */
            rv = soc_phyctrl_adv_remote_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            soc_cm_debug(DK_PORT, "drv_port_get: Port Ability\n");
            rv = soc_phyctrl_ability_get(unit, port, &phy_ability);
            SOC_IF_ERROR_RETURN(rv);
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }

            *prop_val  = mac_ability & phy_ability;
            *prop_val |= phy_ability & SOC_PM_ABILITY_PHY;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            soc_cm_debug(DK_PORT, "drv_port_get: MAC Ability\n");
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            *prop_val = mac_ability;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            soc_cm_debug(DK_PORT, "drv_port_get: PHY Ability\n");
            rv = soc_phyctrl_ability_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_INTERFACE:
            soc_cm_debug(DK_PORT, "drv_port_get: Interface\n");
            if (p_mac != NULL) {
                if (p_mac->md_interface_get != NULL) {
                    rv = MAC_INTERFACE_GET(
                        p_mac, unit, port, prop_val); 
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_ENABLE:
            soc_cm_debug(DK_PORT, "drv_port_get: Enable\n");
            rv = soc_phyctrl_enable_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            soc_cm_debug(DK_PORT, "drv_port_get: Enable Get\n");
            rv = drv_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_IPG_FE:
            soc_cm_debug(DK_PORT, "drv_port_get: IPG FE\n");
            if ((rv = REG_READ_SWMODEr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_SWMODEr_field_get(unit, &reg_value,
                IPGf, &temp);
            switch (temp) {
                case 0:
                    *prop_val = 84;
                    break;
                case 1:
                    *prop_val = 88;
                    break;
                case 2:
                    *prop_val = 92;
                    break;
                case 3:
                    *prop_val = 96;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    break;
            }
            break;
        case DRV_PORT_PROP_IPG_GE:
            soc_cm_debug(DK_PORT, "drv_port_get: IPG GE\n");
            if ((rv = REG_READ_SWMODEr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_SWMODEr_field_get(unit, &reg_value,
                IPGf, &temp);
            switch (temp) {
                case 0:
                case 1:
                case 2:
                    *prop_val = 88;
                    break;
                case 3:
                    *prop_val = 96;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    break;
            }
            break;
        case DRV_PORT_PROP_JAM:
            soc_cm_debug(DK_PORT, "drv_port_get: JAM\n");
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(
                        p_mac, unit, port, (int *) prop_val,
                        &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_BPDU_RX:
            soc_cm_debug(DK_PORT, "drv_port_get: BPDU RX\n");
            if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_GMNGCFGr_field_get(unit, &reg_value, 
                RXBPDU_ENf, &temp);
            if (temp) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        case DRV_PORT_PROP_RESTART_AUTONEG:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_RESTART_AUTONEG not support\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_MAC_LOOPBACK\n");
            if (p_mac != NULL) {
                if (p_mac->md_lb_get != NULL) {
                    rv = MAC_LOOPBACK_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_LOOPBACK\n");
            rv = soc_phyctrl_loopback_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_MEDIUM\n");
            rv = soc_phyctrl_medium_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_COPPER:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_MEDIUM_CONFIG_COPPER\n");
            rv = soc_phyctrl_medium_config_get(unit, port,
                                               SOC_PORT_MEDIUM_COPPER,
                                               (soc_phy_config_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_FIBER:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_MEDIUM_CONFIG_FIBER\n");
            rv = soc_phyctrl_medium_config_get(unit, port,
                                               SOC_PORT_MEDIUM_FIBER,
                                               (soc_phy_config_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_MDIX\n");
            rv = soc_phyctrl_mdix_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_MDIX_STATUS\n");
            rv = soc_phyctrl_mdix_status_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_MS:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_MS\n");
            rv = soc_phyctrl_master_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_SEC_MODE_NONE\n");
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                rv = _drv_bcm5348_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = _drv_bcm53242_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            } else {
                rv = _drv_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_SEC_MODE_STATIC_ACCEPT\n");
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = SOC_E_UNAVAIL;
            } else {
                rv = _drv_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_SEC_MODE_STATIC_REJECT\n");
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = SOC_E_UNAVAIL;
            } else {
                rv = _drv_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_SEC_MODE_DYNAMIC_SA_NUM\n");
            /* worse case
             * While the prop_val = 0, it has two meannings. 
             * One is the SA_NUM mode is not enabling.
             * The other is the SA_NUM is enabling and the number of SA is zero.
             * We assume that the BCM layer has software 
             * database to distinguish them.
             */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                rv = _drv_bcm5348_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                 rv = _drv_bcm53242_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            } else {
                rv = _drv_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_SEC_MODE_DYNAMIC_SA_MATCH\n");
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                rv = _drv_bcm5348_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = _drv_bcm53242_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            } else {
                rv = _drv_port_security_mode_get(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP\n");

            temp = 0;
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                SOC_IF_ERROR_RETURN(_drv_bcm5348_port_security_mode_get
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
            } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_get
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
                if (temp) {
                    SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_get
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_EXTEND, &temp));
                    /* temp=0 is Drop; temp=1 is Trap */
                    temp = (temp == 0) ? TRUE : FALSE;
                }
            } else {
                SOC_IF_ERROR_RETURN(_drv_port_security_mode_get
                        (unit, port,
                        DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
            }

            *prop_val = (temp) ? TRUE : FALSE;
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU\n");
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                temp = 0;
                SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_get
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
                if (temp) {
                    SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_get
                            (unit, port, 
                            DRV_PORT_PROP_SEC_MAC_MODE_EXTEND, &temp));
                }
                *prop_val = (temp) ? TRUE : FALSE;

            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
            temp = FALSE;
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                SOC_IF_ERROR_RETURN(_drv_bcm5348_port_security_mode_get
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
            } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                SOC_IF_ERROR_RETURN(_drv_bcm53242_port_security_mode_get
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
            } else {
                SOC_IF_ERROR_RETURN(_drv_port_security_mode_get
                        (unit, port,
                        DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
            }

            *prop_val = (!temp) ? TRUE : FALSE;
            break;
        case DRV_PORT_PROP_PHY_CABLE_DIAG:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_CABLE_DIAG\n");
            rv = soc_phyctrl_cable_diag(unit, port, 
                                        (soc_port_cable_diag_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_LINK_CHANGE:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_LINKCHANGE\n");
            rv = soc_phyctrl_link_change(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_DTAG_MODE\n");
            rv = drv_port_property_enable_get(unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_DTAG_ISP_PORT\n");
            rv = drv_port_property_enable_get(unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_DTAG_TPID\n");
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_ISP_VIDr(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_ISP_VIDr_field_get(unit, &reg_value,
                    ISP_VLAN_DELIMITERf, &temp);
                *prop_val = temp;
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_802_1X_MODE :
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_802_1X_MODE\n");
            
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = SOC_E_PARAM;
            } else {
            _drv_port_802_1x_config_get(unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_802_1X_BLK_RX :
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_802_1X_BLK_RX\n");
            
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = SOC_E_PARAM;
            } else {
            _drv_port_802_1x_config_get(unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                rv = ((DRV_SERVICES(unit)->vlan_prop_port_enable_get)
                    (unit, DRV_VLAN_PROP_MAC2V_PORT, port, &temp));
                *prop_val = temp;
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            if (p_mac != NULL) {
                if (p_mac->md_frame_max_get != NULL) {
                    rv = MAC_FRAME_MAX_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_INGRESS_VLAN_CHK\n");
            if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            
            soc_VLAN_CTRL4r_field_get(unit, &reg_value, 
                INGR_VID_CHKf, prop_val);
            
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)||
                SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_INGRESS_RMONr(
                    unit,(uint32 *)&reg_value64)) < 0) {
                    return rv;
                }
                COMPILER_64_ZERO(temp64);
                if (SOC_IS_ROBO5348(unit)) {
                    soc_INGRESS_RMONr_field_get(unit, (uint32 *)&reg_value64,
                        EN_INGRESS_PORTMAPf, (uint32 *)&temp64);
                    soc_robo_64_val_to_pbmp(unit, &tmp_pbmp, temp64);
                } else { /* 5347, 53242, 53262 */
                    soc_INGRESS_RMONr_field_get(unit, (uint32 *)&reg_value64,
                        EN_INGRESS_PORTMAPf, &temp);
                    SOC_PBMP_WORD_SET(tmp_pbmp, 0, temp);
                }
    
                if (SOC_PBMP_MEMBER(tmp_pbmp, port)) {
                    *prop_val = SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate;
                } else {
                    *prop_val = 0;
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)||
                SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                if ((rv = REG_READ_EGRESS_RMONr(unit, &reg_value)) < 0) {
                    return rv;
                }
                soc_EGRESS_RMONr_field_get(unit, &reg_value, 
                    EGRESS_Pf, &temp);
                if (temp != port) {
                    *prop_val = 0;
                } else {
                    soc_EGRESS_RMONr_field_get(unit, &reg_value, 
                        EN_EGRESS_RMONf, &temp);
                    if (!temp) {
                        *prop_val = 0;
                    } else {
                        *prop_val = SOC_ROBO_PORT_INFO(unit, port).eg_sample_rate;
                    }
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)||
                SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                *prop_val = SOC_ROBO_PORT_INFO(unit, port).ing_sample_prio;
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)||
                SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                *prop_val = SOC_ROBO_PORT_INFO(unit, port).eg_sample_prio;
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_ROAMING_OPT:
            if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
                SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr(unit, 
                        port, &reg_value));
                SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_get(unit, 
                        &reg_value, ROAMING_OPTf, &temp));
                *prop_val = (temp) ? DRV_SA_MOVE_ARL : 0;
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
            
        /* not supported list */
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
            rv = SOC_E_UNAVAIL;
            break;
        default: 
            rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_port_advertise_set
 *
 *  Purpose :
 *      Set the advertise capability to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      prop_mask  :   port advertise capability.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_advertise_set(int unit, soc_pbmp_t bmp, uint32 prop_mask)
{
    int             rv = SOC_E_NONE;
    int             port;

    soc_cm_debug(DK_PORT, 
    "drv_port_advertise_set: unit = %d, bmp = %x %x, adv_value = %x\n",
    unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1), prop_mask);

    SOC_ROBO_PORT_INIT(unit);
    PBMP_ITER(bmp, port) {
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_adv_local_set(unit, port, prop_mask));
    }

    return rv;
}

/*
 *  Function : drv_port_advertise_get
 *
 *  Purpose :
 *      Get the advertise capability to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port_n   :   port number.
 *      prop_val  :   port advertise capability.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_advertise_get(int unit, int port_n, uint32 *prop_val)
{
    int     rv = SOC_E_NONE;

    soc_cm_debug(DK_PORT, "drv_port_advertise_get\n");

    SOC_ROBO_PORT_INIT(unit);
    rv = soc_phyctrl_adv_local_get(unit, port_n, prop_val);
    soc_cm_debug(DK_PORT, 
        "drv_port_advertise_get: unit = %d, port = %d, adv_value = %x\n",
        unit, port_n, *prop_val);

    return rv;
}

/*
 *  Function : drv_port_status_get
 *
 *  Purpose :
 *      Get the status of the port for selected status type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port number.
 *      status_type  :   port status type.
 *      vla     :   status value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr = 0, reg_value = 0, reg_len = 0, temp = 0;
    int     okay = 0;
    
    uint32  phy_medium = SOC_PORT_MEDIUM_COPPER;
    
    uint32  port_lb_phy = 0;
    uint64  reg_value64, temp64;
    soc_pbmp_t pbmp;
    int up = 0;
    
    /* int_pd and pd used to prevent the runpacket issue on the GE port.
     * (with Internal SerDes bounded)
     */
    phy_ctrl_t      *int_pc = NULL, *ext_pc = NULL;  
    
    COMPILER_64_ZERO(reg_value64);
    COMPILER_64_ZERO(temp64);
    
    /* special process to detach port driver */
    if (status_type == DRV_PORT_STATUS_DETACH){
        *val = TRUE;
        rv = _drv_port_sw_detach(unit);
        if (rv < 0) {
            soc_cm_debug(DK_WARN,"Port detach failed!\n");
            *val = FALSE;
        }
        soc_cm_debug(DK_PORT, "drv_port_status_get: DETACH %s\n",
            *val ? "OK" : "FAIL");
        return SOC_E_NONE;
    }
    
    /* To prevent the runpacket issue on the GE port which bounded 
     *  with internal SerDes and connected to an external PHY through SGMII.
     *  The linked information report will be designed to retrieved from   
     *  internal SerDes instead of Ext_PHY.
     *  (original design check external PHY only) 
     */
    if (IS_GE_PORT(unit, port)){
        int_pc = INT_PHY_SW_STATE(unit, port);
        ext_pc = EXT_PHY_SW_STATE(unit, port);
    }
     

    /* remarked for performance issue when debugging LinkScan
    soc_cm_debug(DK_PORT, "drv_port_status_get: unit = %d, port = %d\n",
        unit, port);
    */
    switch (status_type)
    {
        case DRV_PORT_STATUS_LINK_UP:
            /* get the port loopback status (for Robo Chip)
             */
            rv = soc_phyctrl_loopback_get(unit, port, (int *) &port_lb_phy);

            if (port_lb_phy){
                soc_cm_debug(DK_PORT, "port%d at loopback status.\n", port);
                reg_addr = DRV_REG_ADDR(unit, INDEX(LNKSTSr), 0, 0);
                reg_len = DRV_REG_LENGTH_GET(unit, INDEX(LNKSTSr));

                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) ||
                    SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                    if ((rv = DRV_REG_READ(unit, reg_addr, 
                            (uint32 *)&reg_value64, reg_len)) < 0) {
                        return rv;
                    }
                    if (SOC_IS_ROBO5348(unit)) {
                        DRV_REG_FIELD_GET(unit, INDEX(LNKSTSr), 
                                (uint32 *)&reg_value64, INDEX(LNK_STSf), 
                                (uint32 *)&temp64);
                        soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
                    } else { /* 5347, 53242 */
                        DRV_REG_FIELD_GET(unit, INDEX(LNKSTSr), 
                                (uint32 *)&reg_value64, INDEX(LNK_STSf), 
                                &temp);
                        SOC_PBMP_WORD_SET(pbmp, 0, temp);
                    }
                    if (SOC_PBMP_MEMBER(pbmp, port)) {
                        *val = TRUE;
                    } else {
                        *val = FALSE;
                    }
                } else {
                    if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, 
                            reg_len)) < 0) {
                        return rv;
                    }
                    if (reg_value & (1 << port)) {
                        *val = TRUE;
                    } else {
                        *val = FALSE;
                    }
                }
                
                return SOC_E_NONE;
            }
            
            rv = soc_phyctrl_link_get(unit, port, &up);
            
            /* Section to prevent the runpacket issue on the GE port */
            if (IS_GE_PORT(unit, port)){
                if ((int_pc != NULL) && (ext_pc != NULL)  && 
                        (ext_pc != int_pc)){
                    rv = PHY_LINK_GET(int_pc->pd, unit, port, (&up));
                }
            }

            if (rv < 0){
                *val = FALSE;
            } else {
                *val = (up) ? TRUE : FALSE;
            }  

           /* If link down, set default COPPER mode. */
            if (*val) {
                
                /* 1. GE port get the medium from PHY.
                 * 2. FE port on geting medium will got COPPER medium properly 
                 *    - for the PHY_MEDIUM_GET will direct to the mapping 
                 *      routine in phy.c
                 */
                (DRV_SERVICES(unit)->port_get) \
                    (unit, port, 
                    DRV_PORT_PROP_PHY_MEDIUM, &phy_medium);
            }
            else {
                phy_medium = SOC_PORT_MEDIUM_COPPER;
            }

            /* Program MAC if medium mode is changed. */
            if (phy_medium != SOC_ROBO_PORT_MEDIUM_MODE(unit, port)) {
                    
                    /* SW port medium update */
                    SOC_ROBO_PORT_MEDIUM_MODE_SET(unit, port, phy_medium);
            }

            break;
        case DRV_PORT_STATUS_LINK_SPEED:
        
            if (SOC_IS_ROBO5324(unit)){
                reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, SPDSTSr, 0, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, 4)) < 0) {
                    return rv;
                }
                if (reg_value & (1 << port)) {
                    *val = DRV_PORT_STATUS_SPEED_100M;
                } else {
                    *val = DRV_PORT_STATUS_SPEED_10M;
                }
            } else if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5398(unit)||
                       SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5397(unit) ||
                       SOC_IS_ROBO5347(unit)||SOC_IS_ROBO5395(unit) ||
                       SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) ||
                       SOC_IS_ROBO_ARCH_VULCAN(unit)|| SOC_IS_TBX(unit)){
                /* In bcm5396, the port link/speed/duplex won't update at all,  
                 *   for we use SW override mode on all GE port. (HW polling 
                 *   status will be correct in originally).
                 */  
                int speed = 0;
                
                /* Section to prevent the runpacket issue on the GE port */
                if (IS_GE_PORT(unit, port)){
                    if ((int_pc != NULL) && (ext_pc != NULL) && 
                            (ext_pc != int_pc)){
                        rv = PHY_SPEED_GET(int_pc->pd, unit, port, (&speed));
                    } else {
                        rv = soc_phyctrl_speed_get(unit, port, &speed);
                    }
                }

                if (rv){
                    soc_cm_debug(DK_WARN, 
                            "%s, Can't get the PHY speed!\n", FUNCTION_NAME()); 
                    return rv;
                }
                
                *val = (speed == 2500) ? DRV_PORT_STATUS_SPEED_2500M :
                        (speed == 1000) ? DRV_PORT_STATUS_SPEED_1G :
                        (speed == 100) ? DRV_PORT_STATUS_SPEED_100M :
                                        DRV_PORT_STATUS_SPEED_10M;
                
            } else {
                return SOC_E_RESOURCE;
            }
            
            soc_cm_debug(DK_PORT, "drv_port_status_get: SPEED = %d\n",
                *val);
            break;
        case DRV_PORT_STATUS_LINK_DUPLEX:
            soc_cm_debug(DK_PORT, "drv_port_status_get: DUPLEX\n");
            if (SOC_IS_ROBO5324(unit)){
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, INDEX(DUPSTSr), 0, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, 4)) < 0) {
                    return rv;
                }
                if (reg_value & (1 << port)) {
                    *val = DRV_PORT_STATUS_DUPLEX_FULL;
                } else {
                    *val = DRV_PORT_STATUS_DUPLEX_HALF;
                }
                soc_cm_debug(DK_PORT, "drv_port_status_get: DUPLEX = %d\n",
                    *val);
            } else if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5398(unit)||
                       SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5397(unit)||
                       SOC_IS_ROBO5347(unit)||SOC_IS_ROBO5395(unit) ||
                       SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit) ||
                       SOC_IS_ROBO_ARCH_VULCAN(unit)){
                /* In bcm5396, the port link/speed/duplex won't update at all,  
                 *   for we use SW override mode on all GE port. (HW polling 
                 *   status will be correct in originally).
                 */  
                int duplex = 0;
                
                /* Section to prevent the runpacket issue on the GE port */
                if (IS_GE_PORT(unit, port)){
                    if ((int_pc != NULL) && (ext_pc != NULL) && 
                            (ext_pc != int_pc)){
                        rv = PHY_DUPLEX_GET(int_pc->pd, unit, port, (&duplex));
                    } else {
                        rv = soc_phyctrl_duplex_get(unit, port, &duplex);
                    }
                }
 
                if (rv){
                    soc_cm_debug(DK_WARN, 
                            "%s, Can't get the PHY duplex!\n", FUNCTION_NAME()); 
                    return rv;
                }
                
                *val = (duplex == TRUE) ? DRV_PORT_STATUS_DUPLEX_FULL :
                                        DRV_PORT_STATUS_DUPLEX_HALF;
                
            } else {
                return SOC_E_RESOURCE;
            }

            break;
        case DRV_PORT_STATUS_PROBE:
            *val = 0;
            rv = _drv_port_probe(unit, port, &okay);
            *val = okay;
            if (rv < 0) {
                soc_cm_debug(DK_WARN,"Port probe failed on port %s\n",
                            SOC_PORT_NAME(unit, port));
            }
            soc_cm_debug(DK_PORT, "drv_port_status_get: PROBE %s\n",
                *val ? "OK" : "FAIL");
            break;
        case DRV_PORT_STATUS_INIT:
            if (soc_robo_port_info[unit] == NULL) {
                *val = FALSE;
            } else {
                *val = TRUE;
            }
            break;
        case DRV_PORT_STATUS_PHY_DRV_NAME:
            soc_cm_debug(DK_PORT, "drv_port_status_get: PHY_DRV_NAME\n");
            SOC_ROBO_PORT_INIT(unit);            
            
            *val = (uint32)soc_phyctrl_drv_name(unit, port);
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}


/*
 *  Function : drv_port_sw_mac_update
 *
 *  Purpose :
 *      Update the port state and SW override into MAC.
 *
 *  Parameters :
 *      unit    :   unit id
 *      port    :   port number.
 *
 *  Return :
 *      None.
 *
 *  Note :
 *      
 *
 */
int 
drv_port_sw_mac_update(int unit, soc_pbmp_t bmp)
{
    int port, duplex, speed;
    int rv;
    mac_driver_t *p_mac = NULL;      

    soc_cm_debug(DK_PORT, 
        "drv_port_sw_mac_update: unit = %d, bmp = %x %x\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1));
        
    PBMP_ITER(bmp, port) {
        rv = soc_phyctrl_speed_get(unit, port, &speed);
        p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
        if (SOC_FAILURE(rv) && (SOC_E_UNAVAIL != rv)) {
            SOC_DEBUG_PRINT((DK_WARN,
                "u=%d p=%d phyctrl_speed_get rv=%d\n",unit, port, rv));
            return rv;
        }
        if (SOC_E_UNAVAIL == rv ) {
            /* If PHY driver doesn't support speed_get, don't change 
             * MAC speed. E.g, Null PHY driver 
             */
            rv = SOC_E_NONE;
        } else {            
            if (p_mac != NULL) {
                rv =  (MAC_SPEED_SET(p_mac, unit, port, speed));
            }
        }
        if (SOC_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN, 
                            "u=%d p=%d MAC_SPEED_SET speed=%d rv=%d\n",
                            unit, port, speed, rv));
            return rv;
        }

        rv =   (soc_phyctrl_duplex_get(unit, port, &duplex));
        if (SOC_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN, "u=%d p=%d phyctrl_duplex_get rv=%d\n",
                                unit, port, rv));
            return rv;
        }
        if (p_mac != NULL) {
            rv = (MAC_DUPLEX_SET(p_mac, unit, port, duplex));
        }
        if (SOC_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN, "u=%d p=%d MAC_DUPLEX_SET %s sp=%d rv=%d\n", 
                             unit, port, 
                             duplex ? "FULL" : "HALF", speed, rv));
            return rv;
        }            
    }
    
    return SOC_E_NONE;

}

/*
 *  Function : drv_port_bitmap_get
 *
 *  Purpose :
 *      Get the port bitmap of the selected port type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port_type   :   port type.
 *      bitmap  :   port bitmap.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      The GE type will include 1G and 2.5G ports.
 *
 */
int
drv_port_bitmap_get(int unit, uint32 port_type, soc_pbmp_t * bitmap)
{
    switch (port_type) {
        case DRV_PORT_TYPE_10_100:
            *bitmap = PBMP_FE_ALL(unit);
            break;
        case DRV_PORT_TYPE_G:
            /* 2.5G port will be grouped here as well */
            *bitmap = PBMP_GE_ALL(unit);
            break;
        case DRV_PORT_TYPE_XG:
            *bitmap = PBMP_XE_ALL(unit);
            break;
        case DRV_PORT_TYPE_CPU:
            *bitmap = PBMP_CMIC(unit);
            break;
        case DRV_PORT_TYPE_MGNT:
            *bitmap = PBMP_SPI(unit);
            break;
        case DRV_PORT_TYPE_ALL:
            *bitmap = PBMP_ALL(unit);
            break;
    }
    soc_cm_debug(DK_PORT, 
        "drv_port_bitmap_get: unit = %d, port type = %d, bmp = %x %x\n",
        unit, port_type, SOC_PBMP_WORD_GET(*bitmap, 0), SOC_PBMP_WORD_GET(*bitmap, 1));

    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm5396_port_set
 *
 *  Purpose :
 *      Set the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5396_port_set(int unit, soc_pbmp_t bmp, 
                uint32 prop_type, uint32 prop_val)
{
    uint32 reg_value, reg_addr, temp;
    int port;
    int rv = SOC_E_NONE, reg_len;
    mac_driver_t *p_mac = NULL;    

    soc_cm_debug(DK_PORT, "drv_bcm5396_port_set: unit=%d bmp=%x\n", 
        unit, SOC_PBMP_WORD_GET(bmp, 0));

    SOC_ROBO_PORT_INIT(unit);
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            soc_cm_debug(DK_PORT,"drv_port_set: PROP_SPEED\n");
            temp = 0;
            switch (prop_val) {
                case DRV_PORT_STATUS_SPEED_10M:
                    temp = 10;
                    break;
                case DRV_PORT_STATUS_SPEED_100M:
                    temp =100;
                    break;
                case DRV_PORT_STATUS_SPEED_1G:
                    temp = 1000;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY and MAC auto-negotiation OFF */
                /* set PHY auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);

                /* Set PHY registers anyway. */
                rv = soc_phyctrl_speed_set(unit, port, temp);

                /* if auto-negotiation is OFF, */
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_speed_set != NULL) {
                        rv = MAC_SPEED_SET(
                            p_mac, unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;

        case DRV_PORT_PROP_DUPLEX:
            soc_cm_debug(DK_PORT,"drv_port_set: PROP_DUPLEX\n");
            switch (prop_val) {
                case DRV_PORT_STATUS_DUPLEX_HALF:
                    temp = FALSE;
                    break;
                case DRV_PORT_STATUS_DUPLEX_FULL:
                    temp =TRUE;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);

                /* Set PHY registers anyway. */
                rv = soc_phyctrl_duplex_set(unit, port, temp);

                /* set MAC duplex */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_duplex_set != NULL) {
                        rv = MAC_DUPLEX_SET(
                            p_mac,
                            unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_AUTONEG:
            soc_cm_debug(DK_PORT,"drv_port_set: PROP_AUTONEG\n");
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, prop_val); 
            }
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_TX_PAUSE\n");
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, 
                            unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_RX_PAUSE\n");
            PBMP_ITER(bmp, port) {
                 /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, -1, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            soc_cm_debug(DK_PORT, "drv_port_set: LOCAL_ADVER\n");
            PBMP_ITER(bmp, port) {
                /* set advertise to PHY accordingly */
                rv = soc_phyctrl_adv_local_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: REMOTE_ADVER not support\n");
            /* can not set remote advert */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PORT_ABILITY not support\n");
            /* can not be set */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_MAC_ABILITY not support\n");
            /* can not be set */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_ABILITY not support\n");
            /* can not be set */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_INTERFACE:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_INTERFACE %x %x\n", 
                SOC_PBMP_WORD_GET(bmp, 0), prop_val);
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_interface_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_MAC_ENABLE:
            /* This case is called for _bcm_robo_port_update() only. */
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MAC_ENABLE\n");
            PBMP_ITER(bmp, port) {
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_enable_set != NULL) {
                        rv = MAC_ENABLE_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_ENABLE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_ENABLE\n");
            PBMP_ITER(bmp, port) {
                /* Set PHY registers anyway. */
                rv = soc_phyctrl_enable_set(unit, port, prop_val);

                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_enable_set != NULL) {
                        rv = MAC_ENABLE_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            soc_cm_debug(DK_PORT, "%s: PROP_ENABLE_SET\n", FUNCTION_NAME());
            PBMP_ITER(bmp, port) {
                rv = drv_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_IPG_FE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_IPG_FE\n");
            /* can not be set in bcm5396 */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_IPG_GE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_IPG_GE\n");
            /* can not be set in bcm5396 */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_JAM:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_JAM\n");
            
            /* Robo chip use binding Jamming/Pause, Jamming/Pause can't be set
             * independent.
             */
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_BPDU_RX:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_BPDU_RX\n");
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) {
                reg_addr= (DRV_SERVICES(unit)->reg_addr)(unit, GMNGCFGr, 0, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, 1)) < 0) {
                    return rv;
                }
                if (prop_val) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, GMNGCFGr, &reg_value, RXBPDU_ENf, &temp);
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value, 1)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_RESTART_AUTONEG:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_RESTART_AUTONEG\n");
            PBMP_ITER(bmp, port) {
                reg_addr= (DRV_SERVICES(unit)->reg_addr)
                    (unit, MIICTLr, port, 0);

                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, 2)) < 0) {
                    return rv;
                }
                reg_value |= MII_CTRL_RAN;
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value, 2)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MAC_LOOPBACK\n");
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_lb_set != NULL) {
                        rv = MAC_LOOPBACK_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_PHY_LOOPBACK\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_loopback_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MEDIUM not support\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_COPPER:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MEDIUM_CONFIG_COPPER\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_medium_config_set(unit, port,
                                                 SOC_PORT_MEDIUM_COPPER,
                                                 (soc_phy_config_t *)prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_FIBER:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MEDIUM_CONFIG_FIBER\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_medium_config_set(unit, port,
                                                 SOC_PORT_MEDIUM_FIBER,
                                                 (soc_phy_config_t *)prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_PHY_MDIX\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_mdix_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MDIX_STATUS not support\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MS:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MS not support\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_master_set(unit, port, prop_val);
            }
            break;
            
        /* BCM5396 may support SEC_MAC_MODE_NONE but such setting in bcm5396 is 
         *  binding closely with 802.1x so such setting was move to security 
         *  driver already(drv/sec.c). 
         * Not suitable to set in port driver here.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_SEC_MODE_NONE\n");
            rv = SOC_E_CONFIG;
            break;
            
        /* BCM5396 may support SEC_MAC_MODE_NONE but such setting in bcm5396 is 
         *  binding closely with 802.1x so such setting was move to security 
         *  driver already(drv/sec.c). 
         * Not suitable to set in port driver here.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_SEC_MODE_STATIC_ACCEPT\n");
            rv = SOC_E_CONFIG;
            break;

        /* BCM5396 not support SEC_MAC_MODE_STATIC_REJECT.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_SEC_MODE_STATIC_REJECT\n");
            rv = SOC_E_UNAVAIL;
            break;

        /* BCM5396 not support SEC_MAC_MODE_STATIC_REJECT.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_SEC_MODE_DYNAMIC_SA_NUM\n");
            rv = SOC_E_UNAVAIL;
            break;

        /* BCM5396 not support SEC_MAC_MODE_STATIC_REJECT.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_SEC_MODE_DYNAMIC_SA_MATCH\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_LINKUP_EVT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_LINKUP_EVT\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkup_evt(unit, port);
                if (SOC_FAILURE(rv) && (rv != SOC_E_UNAVAIL)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_LINKDN_EVT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_LINKDN_EVT\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkdn_evt(unit, port);
                if (SOC_FAILURE(rv) && (rv != SOC_E_UNAVAIL)) {
                    return rv;
                } 
                rv = SOC_E_NONE;   
            }
            break;
        case DRV_PORT_PROP_PHY_RESET:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_RESET\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phy_reset(unit, port);
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_DTAG_MODE\n");
             if (prop_val == 1) {
                /* bcm5396 5389 does not support dtag mode*/
                 rv = SOC_E_UNAVAIL;
             } else {
                rv = SOC_E_NONE;
             }
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_DTAG_ISP_PORT\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_DTAG_TPID\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MAC_BASE_VLAN\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_frame_max_set != NULL) {
                        rv = MAC_FRAME_MAX_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_INGRESS_VLAN_CHK\n");
            reg_addr =  (DRV_SERVICES(unit)->reg_addr)
                (unit, VLAN_CTRL4r, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, VLAN_CTRL4r);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, VLAN_CTRL4r, &reg_value, 
                INGR_VID_CHKf, &prop_val);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            break;
        /* not supported list */
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
        case DRV_PORT_PROP_ROAMING_OPT:
            rv = SOC_E_UNAVAIL;
            break;
        default: 
            rv = SOC_E_PARAM; 
        break;
    }

    soc_cm_debug(DK_PORT, "drv_port_set: Exit\n");
    return rv;
}

/*
 *  Function : drv_bcm5396_port_get
 *
 *  Purpose :
 *      Get the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5396_port_get(int unit, int port, 
                uint32 prop_type, uint32 *prop_val)
{
    int         rv = SOC_E_NONE, reg_len;
    uint32     reg_value, reg_addr, temp;
    uint32     mac_ability = 0, phy_ability;
    int        pause_tx, pause_rx;
    int        autoneg, done;
    mac_driver_t *p_mac = NULL;
    
    /* remarked for linkscan called each time
     *  (too many messages will be printed out)
    soc_cm_debug(DK_PORT, "drv_port_get: unit=%d port=%d\n", unit, port);
    */

    SOC_ROBO_PORT_INIT(unit);
    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            soc_cm_debug(DK_PORT, "drv_bcm5396_port_get: Speed\n");
            temp = 0;

            rv = soc_phyctrl_speed_get(unit, port, (int *) &temp);
            if (rv != 0){
                return rv;
            }

            switch(temp) {
                case 10:
                    *prop_val = DRV_PORT_STATUS_SPEED_10M;
                    break;
                case 100:
                    *prop_val = DRV_PORT_STATUS_SPEED_100M;
                    break;
                case 1000:
                    *prop_val = DRV_PORT_STATUS_SPEED_1G;
                    break;
                default:
                    *prop_val = 0;
                    break;
            }
            break;
        case DRV_PORT_PROP_DUPLEX:
            soc_cm_debug(DK_PORT, "drv_port_get: Duplex\n");
            temp = 0;

            rv = soc_phyctrl_duplex_get(unit, port, (int *) &temp);
            if (rv != 0){
                return rv;
            }

            switch(temp) {
                case 0:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_HALF;
                    break;
                case 1:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_FULL;
                    break;
                default:
                    break;
            }
            break;
        case DRV_PORT_PROP_AUTONEG:
            soc_cm_debug(DK_PORT, "drv_port_get: Autoneg\n");
            
            rv = soc_phyctrl_auto_negotiate_get(unit, port,
                                                    &autoneg, &done);
            *prop_val = (autoneg) ? DRV_PORT_STATUS_AUTONEG_ENABLE :
                        DRV_PORT_STATUS_AUTONEG_DISABLED;
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_get: TX Pause\n");
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac,
                        unit, port, (int *) prop_val, &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_get: RX Pause\n");
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac,
                        unit, port, &pause_tx, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            soc_cm_debug(DK_PORT, "drv_port_get: Local Advertise\n");
            rv = soc_phyctrl_adv_local_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            soc_cm_debug(DK_PORT, "drv_port_get: Remote Advertise\n");
            /* if auto-negotiation is ON and negotiation is completed */
            /*   get remote advertisement from PHY */
            rv = soc_phyctrl_adv_remote_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            soc_cm_debug(DK_PORT, "drv_port_get: Port Ability\n");
            rv = soc_phyctrl_ability_get(unit, port, &phy_ability);
            SOC_IF_ERROR_RETURN(rv);
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }

            *prop_val  = mac_ability & phy_ability;
            *prop_val |= phy_ability & SOC_PM_ABILITY_PHY;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            soc_cm_debug(DK_PORT, "drv_port_get: MAC Ability\n");
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            *prop_val = mac_ability;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            soc_cm_debug(DK_PORT, "drv_port_get: PHY Ability\n");
            rv = soc_phyctrl_ability_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_INTERFACE:
            soc_cm_debug(DK_PORT, "drv_port_get: Interface\n");
            if (p_mac != NULL) {
                if (p_mac->md_interface_get != NULL) {
                    rv = MAC_INTERFACE_GET(
                        p_mac, unit, port, prop_val); 
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_ENABLE:
            soc_cm_debug(DK_PORT, "drv_port_get: Enable\n");
            rv = soc_phyctrl_enable_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            soc_cm_debug(DK_PORT, "drv_port_get: Enable Get\n");
            rv = drv_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_IPG_FE:
            soc_cm_debug(DK_PORT, "drv_port_get: IPG FE\n");
            /* can not be get in bcm5396 */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_IPG_GE:
            soc_cm_debug(DK_PORT, "drv_port_get: IPG GE\n");
            /* can not be get in bcm5396 */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_JAM:
            soc_cm_debug(DK_PORT, "drv_port_get: JAM\n");
            p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(
                        p_mac, unit, port, (int *) prop_val,
                        &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_BPDU_RX:
            soc_cm_debug(DK_PORT, "drv_port_get: BPDU RX\n");
            reg_addr= (DRV_SERVICES(unit)->reg_addr)(unit, GMNGCFGr, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, 1)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, GMNGCFGr, &reg_value, RXBPDU_ENf, &temp);
            if (temp) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        case DRV_PORT_PROP_RESTART_AUTONEG:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_RESTART_AUTONEG not support\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_MAC_LOOPBACK\n");
            if (p_mac != NULL) {
                if (p_mac->md_lb_get != NULL) {
                    rv = MAC_LOOPBACK_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            /* Remarked for avoid too many message when linkscan.
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_LOOPBACK\n");
            */
            rv = soc_phyctrl_loopback_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            /* remarked for linkscan called each time
             *  (too many messages will be printed out)
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_MEDIUM\n");
            */
            rv = soc_phyctrl_medium_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_COPPER:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_MEDIUM_CONFIG_COPPER\n");
            rv = soc_phyctrl_medium_config_get(unit, port,
                                             SOC_PORT_MEDIUM_COPPER,
                                             (soc_phy_config_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_FIBER:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_MEDIUM_CONFIG_FIBER\n");
            rv = soc_phyctrl_medium_config_get(unit, port,
                                         SOC_PORT_MEDIUM_FIBER,
                                         (soc_phy_config_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_MDIX\n");
            rv = soc_phyctrl_mdix_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_MDIX_STATUS\n");
            rv = soc_phyctrl_mdix_status_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_MS:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_MS\n");
            rv = soc_phyctrl_master_get(unit, port, (int *) prop_val);
            break;
            
        /* BCM5396 may support SEC_MAC_MODE_NONE but such setting in bcm5396 is 
         *  binding closely with 802.1x so such setting was move to security 
         *  driver already(drv/sec.c). 
         * The current PORT_SEC_MODE should be get from security driver.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_SEC_MODE_NONE\n");
            /* TBD when security driver ready.
             * CheckMe : This getting is meaning only when the port is at 
             *           authenticated mode.
             * Here we return SOC_E_UNAVAIL before the code been prepared.
             */
            rv = SOC_E_UNAVAIL;
            break;

        /* BCM5396 may support SEC_MAC_MODE_NONE but such setting in bcm5396 is 
         *  binding closely with 802.1x so such setting was move to security 
         *  driver already(drv/sec.c). 
         * The current PORT_SEC_MODE should be get from security driver.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_SEC_MODE_STATIC_ACCEPT\n");
            /* TBD when security driver ready.
             * CheckMe : This getting is meaning only when the port is at 
             *           authenticated mode.
             * Here we return SOC_E_UNAVAIL before the code been prepared.
             */
            rv = SOC_E_UNAVAIL;
            break;

        /* BCM5396 not support SEC_MAC_MODE_STATIC_REJECT.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_SEC_MODE_STATIC_REJECT\n");
            prop_val = FALSE;
            rv = SOC_E_NONE;
            break;

        /* BCM5396 not support SEC_MAC_MODE_STATIC_REJECT.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_SEC_MODE_DYNAMIC_SA_NUM\n");
            prop_val = FALSE;
            rv = SOC_E_NONE;
            break;

        /* BCM5396 not support SEC_MAC_MODE_STATIC_REJECT.
         */
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_SEC_MODE_DYNAMIC_SA_MATCH\n");
            prop_val = FALSE;
            rv = SOC_E_NONE;
            break;
            
        case DRV_PORT_PROP_PHY_CABLE_DIAG:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_CABLE_DIAG\n");
            rv = soc_phyctrl_cable_diag(unit, port,
                                      (soc_port_cable_diag_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_LINK_CHANGE:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_LINKCHANGE\n");
            rv = soc_phyctrl_link_change(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_DTAG_MODE\n");
            /* bcm5389 5396 do not support dtag mode*/
            *prop_val = 0;
            rv = SOC_E_NONE;
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_DTAG_ISP_PORT\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_DTAG_TPID\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_MAC_BASE_VLAN\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            if (p_mac != NULL) {
                if (p_mac->md_frame_max_get != NULL) {
                    rv = MAC_FRAME_MAX_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_INGRESS_VLAN_CHK\n");
            reg_addr =  (DRV_SERVICES(unit)->reg_addr)
                (unit, VLAN_CTRL4r, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, VLAN_CTRL4r);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, VLAN_CTRL4r, &reg_value, 
                INGR_VID_CHKf, prop_val);
            break;
        /* not supported list */
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
        case DRV_PORT_PROP_ROAMING_OPT:
            rv = SOC_E_UNAVAIL;
            break;
        default: 
            return SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_bcm5398_port_set
 *
 *  Purpose :
 *      Set the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5398_port_set(int unit, soc_pbmp_t bmp, 
                uint32 prop_type, uint32 prop_val)
{
    uint32 reg_value, reg_addr, reg_len, temp;
    int port;
    int rv = SOC_E_NONE;
    mac_driver_t *p_mac = NULL;

    soc_cm_debug(DK_PORT, "drv_bcm5398_port_set: unit=%d bmp=%x\n", 
        unit, SOC_PBMP_WORD_GET(bmp, 0));

    SOC_ROBO_PORT_INIT(unit); 
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            soc_cm_debug(DK_PORT,"drv_port_set: PROP_SPEED\n");
            temp = 0;
            switch (prop_val) {
                case DRV_PORT_STATUS_SPEED_10M:
                    temp = 10;
                    break;
                case DRV_PORT_STATUS_SPEED_100M:
                    temp =100;
                    break;
                case DRV_PORT_STATUS_SPEED_1G:
                    temp = 1000;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY and MAC auto-negotiation OFF */
                /* set PHY auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);

                /* Set PHY registers anyway. */
                rv = soc_phyctrl_speed_set(unit, port, temp);

                /* if auto-negotiation is OFF, */
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_speed_set != NULL) {
                        rv = MAC_SPEED_SET(
                            p_mac, unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;

        case DRV_PORT_PROP_DUPLEX:
            soc_cm_debug(DK_PORT,"drv_port_set: PROP_DUPLEX\n");
            switch (prop_val) {
                case DRV_PORT_STATUS_DUPLEX_HALF:
                    temp = FALSE;
                    break;
                case DRV_PORT_STATUS_DUPLEX_FULL:
                    temp =TRUE;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);
             
                /* Set PHY registers anyway. */
                rv = soc_phyctrl_duplex_set(unit, port, temp);

                /* set MAC duplex */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);                
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_duplex_set != NULL) {
                        rv = MAC_DUPLEX_SET(
                            p_mac, unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_RESTART_AUTONEG:
        case DRV_PORT_PROP_AUTONEG:
            soc_cm_debug(DK_PORT,"drv_port_set: PROP_AUTONEG\n");
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_TX_PAUSE\n");
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_RX_PAUSE\n");
            PBMP_ITER(bmp, port) {
                 /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, -1, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            soc_cm_debug(DK_PORT, "drv_port_set: LOCAL_ADVER\n");
            PBMP_ITER(bmp, port) {
                /* set advertise to PHY accordingly */
                rv = soc_phyctrl_adv_local_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: REMOTE_ADVER not support\n");
            /* can not set remote advert */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PORT_ABILITY not support\n");
            /* can not be set */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_MAC_ABILITY not support\n");
            /* can not be set */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_ABILITY not support\n");
            /* can not be set */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_INTERFACE:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_INTERFACE %x %x\n", 
                SOC_PBMP_WORD_GET(bmp, 0), prop_val);
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_interface_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_MAC_ENABLE:
            /* This case is called for _bcm_robo_port_update() only. */
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MAC_ENABLE\n");
            PBMP_ITER(bmp, port) {
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_enable_set != NULL) {
                        rv = MAC_ENABLE_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_ENABLE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_ENABLE\n");
            PBMP_ITER(bmp, port) {
                /* Set PHY registers anyway. */
                rv = soc_phyctrl_enable_set(unit, port, prop_val);
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_enable_set != NULL) {
                        rv = MAC_ENABLE_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            soc_cm_debug(DK_PORT, "%s: PROP_ENABLE_SET\n", FUNCTION_NAME());
            PBMP_ITER(bmp, port) {
                rv = drv_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_IPG_FE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_IPG_FE\n");
            /* can not be set in bcm5396 */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_IPG_GE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_IPG_GE\n");
            /* can not be set in bcm5396 */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_JAM:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_JAM\n");
            
            /* Robo chip use binding Jamming/Pause, Jamming/Pause can't be set
             * independent.
             */
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_BPDU_RX:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_BPDU_RX\n");
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) {
                if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
                    return rv;
                }
                if (prop_val) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                soc_GMNGCFGr_field_set(unit, &reg_value, 
                    RXBPDU_ENf, &temp);
                if ((rv = REG_WRITE_GMNGCFGr(unit, &reg_value)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MAC_LOOPBACK\n");
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_lb_set != NULL) {
                        rv = MAC_LOOPBACK_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_PHY_LOOPBACK\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_loopback_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MEDIUM not support\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_COPPER:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MEDIUM_CONFIG_COPPER\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_medium_config_set(unit, port,
                                            SOC_PORT_MEDIUM_COPPER,
                                            (soc_phy_config_t *)prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_FIBER:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MEDIUM_CONFIG_FIBER\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_medium_config_set(unit, port,
                                            SOC_PORT_MEDIUM_FIBER,
                                            (soc_phy_config_t *)prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_PHY_MDIX\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_mdix_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_MDIX_STATUS not support\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MS:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MS not support\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_master_set(unit, port, prop_val);
            }
            break;
            
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_SEC_MODE_NONE\n");
            PBMP_ITER(bmp, port) {
                rv = _drv_bcm5398_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_SEC_MAC_MODE_EXTEND\n");
            PBMP_ITER(bmp, port) {
                rv = _drv_bcm5398_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_SEC_MAC_MODE_SIMPLIFY\n");
            PBMP_ITER(bmp, port) {
                rv = _drv_bcm5398_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_PHY_LINKUP_EVT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_LINKUP_EVT\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkup_evt(unit, port);
                if (SOC_FAILURE(rv) && (SOC_E_UNAVAIL != rv)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_LINKDN_EVT:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_LINKDN_EVT\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkdn_evt(unit, port);
                if (SOC_FAILURE(rv) && (SOC_E_UNAVAIL != rv)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_RESET:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_PHY_RESET\n");
            PBMP_ITER(bmp, port) {
                rv = soc_phy_reset(unit, port);
            }
            break;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_PAUSE_FRAME_BYPASS\n");
            PBMP_ITER(bmp, port) {
                rv = drv_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_DTAG_MODE\n");
            rv = drv_port_property_enable_set(unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_DTAG_ISP_PORT\n");
            PBMP_ITER(bmp, port) {
                rv = drv_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            {
                uint32  reg_id, field_id;
                
                soc_cm_debug(DK_PORT, "drv_port_set: PROP_DTAG_TPID\n");
                if (!(SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)||
                    SOC_IS_ROBO5395(unit)||SOC_IS_ROBO_ARCH_VULCAN(unit))) {
                    return SOC_E_UNAVAIL;
                }
                if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
                    reg_id = INDEX(DTAG_TPIDr);
                    field_id = INDEX(ISP_TPIDf);
                } else {
                    reg_id = INDEX(DOU_TAG_TPIDr);
                    field_id = INDEX(ISP_TPIDf);
                }
                reg_addr =  (DRV_SERVICES(unit)->reg_addr)
                    (unit, reg_id, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, reg_id);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }
                
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, reg_id, &reg_value, field_id, &prop_val);
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }
                break;
            }
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            soc_cm_debug(DK_PORT, "drv_port_set: PROP_MAC_BASE_VLAN\n");
            if (SOC_IS_ROBO5395(unit)) {
                if ((rv = REG_READ_VLAN_MAC_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }

                temp = 0;
                soc_VLAN_MAC_CTRLr_field_get(unit, &reg_value, 
                    VLAN_MAC_ENf, &temp);
                if (prop_val) {
                    temp |= SOC_PBMP_WORD_GET(bmp, 0);
                } else {
                    temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
                }
                soc_VLAN_MAC_CTRLr_field_set(unit, &reg_value, 
                    VLAN_MAC_ENf, &temp);
                if ((rv = REG_WRITE_VLAN_MAC_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }
            } else { /* 5398/5397/53115/53118 */
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_frame_max_set != NULL) {
                        rv = MAC_FRAME_MAX_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            soc_cm_debug(DK_PORT, 
                "drv_port_set: PROP_INGRESS_VLAN_CHK\n");
            if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }

            soc_VLAN_CTRL4r_field_set(unit, &reg_value,
                INGR_VID_CHKf, &prop_val);
            if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_EEE_ENABLE:
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G:
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H:
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G:
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H:
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G:
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H:
#if defined(BCM_53125) || defined(BCM_53128)            
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                rv = _drv_bcm53125_port_eee_set(unit, bmp, 
                    prop_type, prop_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
#else /* !BCM_53125 */
            rv = SOC_E_UNAVAIL;
#endif
            break;
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)){              

                PBMP_ITER(bmp, port) {
                    /* Read the default value */
                    SOC_IF_ERROR_RETURN(
                        REG_READ_DEFAULT_1Q_TAGr(unit, port, &reg_value));
                    reg_value &= ~(DOT1P_PRI_MASK << DOT1P_PRI_SHIFT);
                    reg_value |= ((prop_val & DOT1P_PRI_MASK) << 
                        DOT1P_PRI_SHIFT);

                    /* Write the default value */
                    SOC_IF_ERROR_RETURN(
                        REG_WRITE_DEFAULT_1Q_TAGr(unit, port, &reg_value));
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        /* not supported list */
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
        case DRV_PORT_PROP_ROAMING_OPT:
            rv = SOC_E_UNAVAIL;
            break;
            
        default: 
            rv = SOC_E_PARAM; 
        break;
    }

    soc_cm_debug(DK_PORT, "drv_port_set: Exit\n");
    return rv;
}

/*
 *  Function : drv_bcm5398_port_get
 *
 *  Purpose :
 *      Get the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5398_port_get(int unit, int port, 
                uint32 prop_type, uint32 *prop_val)
{
    int         rv = SOC_E_NONE;
    uint32     reg_value, reg_addr, reg_len, temp = 0;
    uint32     mac_ability = 0, phy_ability;
    int        pause_tx, pause_rx;
    int        autoneg, done;
    mac_driver_t *p_mac = NULL;

    /* remarked for linkscan called each time
     *  (too many messages will be printed out)
    soc_cm_debug(DK_PORT, "drv_port_get: unit=%d port=%d\n", unit, port);
    */
    SOC_ROBO_PORT_INIT(unit);
    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            soc_cm_debug(DK_PORT, "drv_bcm5398_port_get: Speed\n");
            temp = 0;

            rv = soc_phyctrl_speed_get(unit, port, (int *) &temp);
            if (rv != 0){
                return rv;
            }
            switch(temp) {
                case 10:
                    *prop_val = DRV_PORT_STATUS_SPEED_10M;
                    break;
                case 100:
                    *prop_val = DRV_PORT_STATUS_SPEED_100M;
                    break;
                case 1000:
                    *prop_val = DRV_PORT_STATUS_SPEED_1G;
                    break;
                default:
                    *prop_val = 0;
                    break;
            }
            break;
        case DRV_PORT_PROP_DUPLEX:
            soc_cm_debug(DK_PORT, "drv_port_get: Duplex\n");
            temp = 0;

            rv = soc_phyctrl_duplex_get(unit, port, (int *) &temp);
            if (rv != 0){
                return rv;
            }

            switch(temp) {
                case 0:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_HALF;
                    break;
                case 1:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_FULL;
                    break;
                default:
                    break;
            }
            break;
        case DRV_PORT_PROP_AUTONEG:
            soc_cm_debug(DK_PORT, "drv_port_get: Autoneg\n");
            rv = soc_phyctrl_auto_negotiate_get(unit, port,
                                                &autoneg, &done);
            *prop_val = (autoneg) ? DRV_PORT_STATUS_AUTONEG_ENABLE :
                        DRV_PORT_STATUS_AUTONEG_DISABLED;
            
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_get: TX Pause\n");
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac,
                        unit, port, (int *) prop_val, &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            soc_cm_debug(DK_PORT, "drv_port_get: RX Pause\n");
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac,
                        unit, port, &pause_tx, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            soc_cm_debug(DK_PORT, "drv_port_get: Local Advertise\n");
            rv = soc_phyctrl_adv_local_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            soc_cm_debug(DK_PORT, "drv_port_get: Remote Advertise\n");
            /* if auto-negotiation is ON and negotiation is completed */
            /*   get remote advertisement from PHY */
            rv = soc_phyctrl_adv_remote_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            soc_cm_debug(DK_PORT, "drv_port_get: Port Ability\n");
            rv = soc_phyctrl_ability_get(unit, port, &phy_ability);
            SOC_IF_ERROR_RETURN(rv);
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }

            *prop_val  = mac_ability & phy_ability;
            *prop_val |= phy_ability & SOC_PM_ABILITY_PHY;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            soc_cm_debug(DK_PORT, "drv_port_get: MAC Ability\n");
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            *prop_val = mac_ability;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            soc_cm_debug(DK_PORT, "drv_port_get: PHY Ability\n");
            rv = soc_phyctrl_ability_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_INTERFACE:
            soc_cm_debug(DK_PORT, "drv_port_get: Interface\n");
            if (p_mac != NULL) {
                if (p_mac->md_interface_get != NULL) {
                    rv = MAC_INTERFACE_GET(
                        p_mac, unit, port, prop_val); 
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_ENABLE:
            soc_cm_debug(DK_PORT, "drv_port_get: Enable\n");
            rv = soc_phyctrl_enable_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            soc_cm_debug(DK_PORT, "%s: Enable Get\n", FUNCTION_NAME());
            rv = drv_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_IPG_FE:
            soc_cm_debug(DK_PORT, "drv_port_get: IPG FE\n");
            /* can not be get in bcm5396 */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_IPG_GE:
            soc_cm_debug(DK_PORT, "drv_port_get: IPG GE\n");
            /* can not be get in bcm5396 */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_JAM:
            soc_cm_debug(DK_PORT, "drv_port_get: JAM\n");
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(
                        p_mac, unit, port, (int *) prop_val,
                        &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_BPDU_RX:
            soc_cm_debug(DK_PORT, "drv_port_get: BPDU RX\n");
            if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_GMNGCFGr_field_get(unit, &reg_value, 
                    RXBPDU_ENf, &temp);
            if (temp) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        case DRV_PORT_PROP_RESTART_AUTONEG:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_RESTART_AUTONEG not support\n");
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_MAC_LOOPBACK\n");
            if (p_mac != NULL) {
                if (p_mac->md_lb_get != NULL) {
                    rv = MAC_LOOPBACK_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            /* Remarked for avoid too many message when linkscan.
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_LOOPBACK\n");
            */
            rv = soc_phyctrl_loopback_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            /* remarked for linkscan called each time
             *  (too many messages will be printed out)
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_MEDIUM\n");
            */
            rv = soc_phyctrl_medium_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_COPPER:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_MEDIUM_CONFIG_COPPER\n");
            rv = soc_phyctrl_medium_config_get(unit, port,
                                            SOC_PORT_MEDIUM_COPPER,
                                            (soc_phy_config_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM_CONFIG_FIBER:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_PHY_MEDIUM_CONFIG_FIBER\n");
            rv = soc_phyctrl_medium_config_get(unit, port, 
                                            SOC_PORT_MEDIUM_FIBER,
                                            (soc_phy_config_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_MDIX\n");
            rv = soc_phyctrl_mdix_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_MDIX_STATUS\n");
            rv = soc_phyctrl_mdix_status_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_MS:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_MS\n");
            rv = soc_phyctrl_master_get(unit, port, (int *) prop_val);
            break;
            
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_SEC_MODE_NONE\n");
            rv = _drv_bcm5398_port_security_mode_get(unit, port, prop_type, prop_val);
            break;

        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_SEC_MAC_MODE_EXTEND\n");
            rv = _drv_bcm5398_port_security_mode_get(unit, port, prop_type, prop_val);
            break;

        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            soc_cm_debug(DK_PORT, "drv_port_get: DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY\n");
            rv = _drv_bcm5398_port_security_mode_get(unit, port, prop_type, prop_val);
            break;

        case DRV_PORT_PROP_PHY_CABLE_DIAG:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_CABLE_DIAG\n");
            rv = soc_phyctrl_cable_diag(unit, port,
                                        (soc_port_cable_diag_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_LINK_CHANGE:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PHY_LINKCHANGE\n");
            rv = soc_phyctrl_link_change(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_PAUSE_FRAME_BYPASS\n");
            rv = drv_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_DTAG_MODE\n");
            rv = drv_port_property_enable_get(unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_DTAG_ISP_PORT\n");
            rv = drv_port_property_enable_get(unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            {
                uint32  reg_id, field_id;
            
                soc_cm_debug(DK_PORT, "drv_port_get: PROP_DTAG_TPID\n");
                if (!(SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)||
                    SOC_IS_ROBO5395(unit)||SOC_IS_ROBO_ARCH_VULCAN(unit))) {
                    return SOC_E_UNAVAIL;
                }
                
                if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
                    reg_id = INDEX(DTAG_TPIDr);
                    field_id = INDEX(ISP_TPIDf);
                } else {
                    reg_id = INDEX(DOU_TAG_TPIDr);
                    field_id = INDEX(ISP_TPIDf);
                }
                reg_addr =  (DRV_SERVICES(unit)->reg_addr)
                    (unit, reg_id, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, reg_id);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }
                
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_id, &reg_value, field_id, &temp);
                *prop_val = temp;
                break;
            }
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            soc_cm_debug(DK_PORT, "drv_port_get: PROP_MAC_BASE_VLAN\n");
            if (SOC_IS_ROBO5395(unit)) {
                if ((rv = REG_READ_VLAN_MAC_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }

                temp = 0;
                soc_VLAN_MAC_CTRLr_field_get(unit, &reg_value, 
                    VLAN_MAC_ENf, &temp);
                if (temp & (0x1 << port)) {
                    *prop_val = TRUE;
                } else {
                    *prop_val = FALSE;
                }
            } else { /* 5398/5397 */
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            if (p_mac != NULL) {
                if (p_mac->md_frame_max_get != NULL) {
                    rv = MAC_FRAME_MAX_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            soc_cm_debug(DK_PORT, 
                "drv_port_get: PROP_INGRESS_VLAN_CHK\n");
            if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }

            soc_VLAN_CTRL4r_field_get(unit, &reg_value,
                INGR_VID_CHKf, prop_val);
            break;
        case DRV_PORT_PROP_EEE_ENABLE:
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G:
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H:
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G:
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H:
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G:
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H:
#if defined(BCM_53125) || defined(BCM_53128)            
            if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
                rv = _drv_bcm53125_port_eee_get(unit, port, 
                    prop_type, prop_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
#else /* !BCM_53125 */
            rv = SOC_E_UNAVAIL;
#endif /* BCM_53125 */
            break;
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)){                

                /* Read the default value */
                SOC_IF_ERROR_RETURN(
                    REG_READ_DEFAULT_1Q_TAGr(unit, port, &reg_value));

                *prop_val = ((reg_value >> DOT1P_PRI_SHIFT) & DOT1P_PRI_MASK);
                
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        /* not supported list */
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
        case DRV_PORT_PROP_ROAMING_OPT:
            rv = SOC_E_UNAVAIL;
            break;
            
        default: 
            return SOC_E_PARAM;
    }

    return rv;
}
