/*
 * $Id: storm.c 1.13 Broadcom SDK $
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
 */
 #include <soc/drv.h>
 #include <soc/debug.h>

/*
 *  Function : _drv_bcm5398_storm_control_type_enable_set
 *
 *  Purpose :
 *      Set the status and types of storm control.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      type  :   storm control types.
 *      enable   :   status of the storm control types.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
_drv_bcm5398_storm_control_type_enable_set(int unit, uint32 port, 
    uint32 type, uint32 enable)
{
    uint32  reg_addr, reg_len, temp;
    uint32  reg_value;
    uint32 bcast_fld_index= 0, mcast_fld_index = 0, dlf_fld_index = 0;
    int     rv= SOC_E_NONE;

    if (type & DRV_STORM_CONTROL_BCAST) {
        bcast_fld_index = EN_BCAST_SUPf;
    }
    if (type & DRV_STORM_CONTROL_MCAST) {
        /* Storm Mcast Suppression : Mcast and Destination lookup success */
        mcast_fld_index = EN_MCAST_SUPf;
    } 
    if (type & DRV_STORM_CONTROL_DLF) {
        /* Storm DLF Suppression : Mcast and Unicast Destination lookup failed */
        dlf_fld_index = EN_DLF_SUPf;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    
    temp = enable;
    if (bcast_fld_index) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, &reg_value, bcast_fld_index, &temp));
    }
    if (mcast_fld_index) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, &reg_value, mcast_fld_index, &temp));
    }
    if (dlf_fld_index) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, &reg_value, dlf_fld_index, &temp));
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value,reg_len));

    return rv;
}

/*
 *  Function : _drv_bcm5398_storm_control_type_enable_get
 *
 *  Purpose :
 *      Get the status and types of storm control.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      type  :   storm control types.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
_drv_bcm5398_storm_control_type_enable_get(int unit, uint32 port, uint32 *type)
{
    uint32  reg_addr, reg_len, temp;
    uint32  reg_value;
    int     rv= SOC_E_NONE;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    *type = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_IRC_CONr, &reg_value, EN_BCAST_SUPf, &temp));
    if (temp) {
        *type |= DRV_STORM_CONTROL_BCAST;
    }

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_IRC_CONr, &reg_value, EN_MCAST_SUPf, &temp));
    if (temp) {
        *type |= DRV_STORM_CONTROL_MCAST;
    }
    
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_IRC_CONr, &reg_value, EN_DLF_SUPf, &temp));
    if (temp) {
        *type |= DRV_STORM_CONTROL_DLF;
    }

    return rv;
}

/*
 *  Function : drv_bcm5398_storm_control_enable_set
 *
 *  Purpose :
 *      Set the status for global storm control of selected port..
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      enable   :   status of the storm control. (global)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5398_storm_control_enable_set(int unit, uint32 port, uint8 enable)
{
    uint32  reg_addr, reg_len, temp;
    uint32  reg_value;
    int     rv= SOC_E_NONE;

    soc_cm_debug(DK_PORT, 
    "drv_storm_control_enable_set: unit = %d, port= %d, %sable\n",
        unit, port, enable ? "en" : "dis");
    /* Enable drop_enable bit of bucket 1 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, COMM_IRC_CONr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, COMM_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, COMM_IRC_CONr, &reg_value, DROP_EN1f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value,reg_len));

    /* Enable Ingress rate control bit */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = enable;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, PORT_IRC_CONr, &reg_value, ING_RC_EN1f, &temp));

    /* Enable Suppression bit of Storm Control */ 
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, PORT_IRC_CONr, &reg_value, EN_STORM_SUPf, &temp));

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    return rv;
}

/*
 *  Function : drv_bcm5398_storm_control_enable_get
 *
 *  Purpose :
 *      Get the status for global storm control of selected port..
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      enable   :   status of the storm control. (global)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5398_storm_control_enable_get(int unit, uint32 port, uint8 *enable)
{
    uint32  reg_addr, reg_len, temp;
    uint32  reg_value;
    int     rv= SOC_E_NONE;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_IRC_CONr, &reg_value, ING_RC_EN1f, &temp));
    *enable = temp;

    soc_cm_debug(DK_PORT, 
    "drv_storm_control_enable_get: unit = %d, port= %d, %sable\n",
        unit, port, *enable ? "en" : "dis");
    return rv;
}

/*
 *  Function : drv_bcm5398_storm_control_set
 *
 *  Purpose :
 *      Set the types and limit value for storm control of selected port..
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      bmp   :   port bitmap.
 *      type   :   types of strom control.
 *      limit   :   limit value of storm control. (Kbits)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5398_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size)
{
    uint32      reg_addr, reg_value, reg_len, temp;
    int         rv= SOC_E_NONE;
    uint32      port;
    uint32      disable_type = 0, burst_kbyte = 0;

    soc_cm_debug(DK_PORT, 
    "drv_storm_control_set: unit = %d, bmp= %x, type = 0x%x, limit = %dK\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), type, limit);

    PBMP_ITER(bmp, port) {
        disable_type = DRV_STORM_CONTROL_BCAST | 
                        DRV_STORM_CONTROL_MCAST |
                        DRV_STORM_CONTROL_DLF;
        if (limit == 0) { /* Disable storm suppression type */
            type = disable_type;
            SOC_IF_ERROR_RETURN(
                    _drv_bcm5398_storm_control_type_enable_set(unit, port, type, FALSE));
            /* need diable ingress rate control ? */
        } else {
            disable_type ^= type;
            /* set storm suppression type */
                SOC_IF_ERROR_RETURN(
                _drv_bcm5398_storm_control_type_enable_set(unit, port, type, TRUE));

            if (disable_type) {
                    SOC_IF_ERROR_RETURN(
                    _drv_bcm5398_storm_control_type_enable_set(
                                        unit, port, 
                                        disable_type, FALSE));
            }
            /* set bucket 1 refresh count */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, PORT_IRC_CONr, port, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, PORT_IRC_CONr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            /* burst size */
            if (burst_size) {
                if (burst_size > (500 * 8)) { /* 500 KB */
                    return SOC_E_PARAM;
                }
                burst_kbyte = (burst_size / 8);
                if (burst_kbyte <= 16) { /* 16KB */
                    temp = 0;
                } else if (burst_kbyte <= 20) { /* 20KB */
                    temp = 1;
                } else if (burst_kbyte <= 28) { /* 28KB */
                    temp = 2;
                } else if (burst_kbyte <= 40) { /* 40KB */
                    temp = 3;
                } else if (burst_kbyte <= 76) { /* 76KB */
                    temp = 4;
                } else if (burst_kbyte <= 140){ /* 140KB */
                    temp = 5;
                } else if (burst_kbyte <= 268){ /* 268KB */
                    temp = 6;
                } else { /* else burst_kbyte <=500 for 500KB */
                    temp = 7;
                }
                
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_IRC_CONr, &reg_value, BUCKET_SIZE1f, &temp));
            }
            
            /* refresh count  (fixed type)*/
            if (limit <= 1792) { /* 64KB ~ 1.792MB */
                temp = ((limit-1) / 64) +1;
            } else if (limit <= 102400){ /* 2MB ~ 100MB */
                temp = (limit /1024 ) + 27;
            } else if (limit <= 1048576) { /* 104MB ~ 1000MB */
                temp = (limit /8192) + 115;
            } else {
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, PORT_IRC_CONr, &reg_value, REF_CNT1f, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
        }
    }
    return rv;
}

/*
 *  Function : drv_bcm5398_storm_control_get
 *
 *  Purpose :
 *      Get the types and limit value for storm control of selected port..
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      bmp   :   port bitmap.
 *      type   :   types of strom control.
 *      limit   :   limit value of storm control. (Kbits)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5398_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size)
{
    uint32      reg_addr, reg_value, reg_len, temp;
    int         rv= SOC_E_NONE;

    SOC_IF_ERROR_RETURN(
        _drv_bcm5398_storm_control_type_enable_get(unit, port, type));
    if (!(*type)) {
        *limit = 0;
    } else {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, PORT_IRC_CONr, port, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, PORT_IRC_CONr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, &reg_value, REF_CNT1f, &temp));
        if (temp <= 28) {
            *limit = temp * 64;
        } else if (temp <= 127) {
            *limit = (temp -27) * 1024;
        } else if (temp <=243) {
            *limit = (temp -115) * 1024 * 8;
        } else {
            return SOC_E_INTERNAL;
        }
        /* burst size */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, &reg_value, BUCKET_SIZE1f, &temp));
        switch (temp) {
            case 0:
                *burst_size = 16 * 8; /* 16KB = 108 Kbits */
                break;
            case 1:
                *burst_size = 20 * 8; /* 20KB */
                break;
            case 2:
                *burst_size = 28 * 8; /* 28KB */
                break;
            case 3:
                *burst_size = 40 * 8; /* 40KB */
                break;
            case 4:
                *burst_size = 76 * 8; /* 76KB */
                break;
            case 5:
                *burst_size = 140 * 8; /* 140KB */
                break;
            case 6:
                *burst_size = 268 * 8; /* 268KB */
                break;
            case 7:
                *burst_size = 500 * 8; /* 500KB */
                break;

            default:
                return SOC_E_INTERNAL;
            }
    }
    soc_cm_debug(DK_PORT, 
    "drv_storm_control_get: unit = %d, port= %d, type = 0x%x, limit = %dK\n",
        unit, port, *type, *limit);

    return rv;
}
