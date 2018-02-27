/*
 * $Id: rate.c 1.8 Broadcom SDK $
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

static int robo5395_ingress_rate_init_flag = 0;

#define BCM5395_IRC_PKT_MASK    0x7f
/*
 *  Function : _drv_bcm5395_port_irc_set
 *
 *  Purpose :
 *  Set the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0(storm control use bucket1). 
 *
 */
int
_drv_bcm5395_port_irc_set(int unit, uint32 port, 
                              uint32 limit, uint32 burst_size)
{
    uint32  reg_addr, reg_value, reg_len, temp;
    int     rv= SOC_E_NONE;
    soc_pbmp_t pbmp;
    uint32  burst_kbyte = 0;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    if (limit == 0) { /* Disable ingress rate control */
        
        /* disable ingress rate control bucket 0*/
        temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, &reg_value, ING_RC_ENf, &temp));
       
          
    } else {    /* Enable ingress rate control */
        /* check if global ingress rate config is set */ 
        if (!robo5395_ingress_rate_init_flag) {
            pbmp = PBMP_ALL(unit);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->rate_config_set)(
                            unit, pbmp, 
                            DRV_RATE_CONFIG_PKT_MASK, BCM5395_IRC_PKT_MASK));
            robo5395_ingress_rate_init_flag = 1;
        }

        if (burst_size > (500 * 8)) { /* 500 KB */
            return SOC_E_PARAM;
        }
        burst_kbyte = burst_size / 8;
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
        } else if (burst_kbyte <= 500){ /* 500KB */
            temp = 7;
        } 
        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, &reg_value, BUCKET_SIZE0f, &temp));

        /* refresh count  (fixed type)*/
        if (limit <= 1792) { /* 64KB ~ 1.792MB */
            temp = ((limit-1) / 64) +1;
        } else if (limit <= 100000){ /* 2MB ~ 100MB */
            temp = (limit /1000 ) + 27;
        } else if (limit <= 1000000){ /* 104MB ~ 1000MB */
            temp = (limit /8000) + 115;
        } else {
            rv = SOC_E_PARAM;
            return rv;
        }
        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, &reg_value, REF_CNT0f, &temp));

        /* enable ingress rate control */
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, &reg_value, ING_RC_ENf, &temp));
    }
    /* write register */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &reg_value,reg_len));
    return rv;
}

/*
 *  Function : _drv_bcm5395_port_irc_get
 *
 *  Purpose :
 *   Get the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0(storm control use bucket1). 
 *
 */
int
_drv_bcm5395_port_irc_get(uint32 unit, uint32 port, 
                                 uint32 *limit, uint32 *burst_size)
{
    uint32  reg_addr, reg_value, reg_len, temp;
    int     rv= SOC_E_NONE;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    /* check global ingress rate control setting */
    if (robo5395_ingress_rate_init_flag) {
        temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->rate_config_get)(
                            unit, SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0), 
                            DRV_RATE_CONFIG_PKT_MASK, &temp));
                            
        /* if robo_ingress_rate_init_flag=0, only next ingress rate setting may 
         * set the properly PKT_MASK0 again currenly.
         */ 
        robo5395_ingress_rate_init_flag = (temp == 0) ? 0 : 1;
    }

     temp = 0;
     SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_IRC_CONr, &reg_value, ING_RC_ENf, &temp));

    if (temp == 0) {
        *limit = 0;
        *burst_size = 0;
    } else {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, &reg_value, BUCKET_SIZE0f, &temp));
        
        switch (temp) {
        case 0:
            *burst_size = 16 * 8; /* 16KB */
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
            
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, &reg_value, REF_CNT0f, &temp));
        if (temp <= 28) {
            *limit = temp * 64;
        } else if (temp <= 127) {
            *limit = (temp -27) * 1000;
        } else if (temp <=240) {
            *limit = (temp -115) * 1000 * 8;
        } else {
            return SOC_E_INTERNAL;
        }
    }

    return rv;
}


/*
 *  Function : _drv_bcm5395_port_erc_set
 *
 *  Purpose :
 *     Set the burst size and rate limit value of the selected port egress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm5395_port_erc_set(uint32 unit, uint32 port, 
                                 uint32 limit, uint32 burst_size)
{
    uint32  reg_addr, reg_value, reg_len, temp;
    int     rv= SOC_E_NONE;
    uint32  burst_kbyte = 0;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_ERC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_ERC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    if (limit == 0) { /* Disable ingress rate control */
        temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_ERC_CONr, &reg_value, ENG_RC_ENf, &temp));
    } else {    /* Enable ingress rate control */
        if (burst_size > (500 * 8)) { /* 500 KB */
            return SOC_E_PARAM;
        }
        /* burst size */
        burst_kbyte = burst_size / 8;
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
        } else if (burst_kbyte <= 140) { /* 140KB */
            temp = 5;
        } else if (burst_kbyte <= 268) { /* 268KB */
            temp = 6;
        } else if (burst_kbyte <= 500) { /* 500KB */
            temp = 7;
        } 
    
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_ERC_CONr, &reg_value, BUCKET_SIZEf, &temp));

        /* refresh count  (fixed type)*/
        if (limit <= 1792) { /* 64KB ~ 1.792MB */
            temp = ((limit-1) / 64) +1;
        } else if (limit <= 100000){ /* 2MB ~ 100MB */
            temp = (limit /1000 ) + 27;
        } else if (limit <= 1000000) { /* 104MB ~ 1000MB */
            temp = (limit /8000) + 115;
        } else {
            rv = SOC_E_PARAM;
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_ERC_CONr, &reg_value, REF_CNTSf, &temp));

        /* enable ingress rate control */
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_ERC_CONr, &reg_value, ENG_RC_ENf, &temp));
    }
    /* write register */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &reg_value,reg_len));
    return rv;
}

/*
 *  Function : _drv_bcm5395_port_erc_get
 *
 *  Purpose :
 *     Get the burst size and rate limit value of the selected port.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm5395_port_erc_get(uint32 unit, uint32 port, 
                                 uint32 *limit, uint32 *burst_size)
{
    uint32  reg_addr, reg_value, reg_len, temp;
    int     rv= SOC_E_NONE;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_ERC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_ERC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_ERC_CONr, &reg_value, ENG_RC_ENf, &temp));
    if (temp ==0) {
        *limit = 0;
        *burst_size = 0;
    } else {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_ERC_CONr, &reg_value, BUCKET_SIZEf, &temp));
        
        switch (temp) {
            case 0:
                *burst_size = 16 * 8; /* 16KB */
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
        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_ERC_CONr, &reg_value, REF_CNTSf, &temp));
        if (temp <= 28) {
            *limit = temp * 64;
        } else if (temp <= 127) {
            *limit = (temp -27) * 1000;
        } else if (temp <=243) {
            *limit = (temp -115) * 1000 * 8;
        } else {
            return SOC_E_INTERNAL;
        }
    }

    return rv;
}

/*
 *  Function : drv_bcm5395_rate_config_set
 *
 *  Purpose :
 *      Set the rate control type value to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      pbmp   :   port bitmap.
 *      config_type  :   rate control type.
 *      value  :   value of rate control type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm5395_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type, 
    uint32 value)
{
    uint32  reg_addr, reg_value, reg_len, temp;
    int     rv = SOC_E_NONE;

    soc_cm_debug(DK_PORT, 
    "drv_bcm5395_rate_config_set: unit = %d, bmp = %x, type = %d, value = %d\n",
        unit, SOC_PBMP_WORD_GET(pbmp, 0), config_type, value);
    /* set bucket 0*/
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: 
            /* per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, COMM_IRC_CONr, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, COMM_IRC_CONr);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }
                temp = value;            
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, COMM_IRC_CONr, &reg_value, RATE_TYPE0f, &temp));
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value,reg_len));
            }
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE:
            /* per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, COMM_IRC_CONr, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, COMM_IRC_CONr);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }
                temp = value;
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, COMM_IRC_CONr, &reg_value, DROP_EN0f, &temp));
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value,reg_len));
            }
            break;
        case DRV_RATE_CONFIG_PKT_MASK: 
            /* per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, COMM_IRC_CONr, 0, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, COMM_IRC_CONr);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }
                temp = value & BCM5395_IRC_PKT_MASK;
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, COMM_IRC_CONr, &reg_value, PKT_MSK0f, &temp));
                    
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value,reg_len));

            }
            break;
        default:
            return SOC_E_PARAM;
    }
    return rv;
}

/*
 *  Function : drv_bcm_5395_rate_config_get
 *
 *  Purpose :
 *      Get the rate control type value to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      pbmp   :   port bitmap.
 *      config_type  :   rate control type.
 *      value  :   value of rate control type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm5395_rate_config_get(int unit, uint32 port, uint32 config_type, 
     uint32 *value)
{
    uint32  reg_addr, reg_value, reg_len, temp;
    int     rv = SOC_E_NONE;

    /* get bucket 0*/
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: /* per chip */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, COMM_IRC_CONr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, COMM_IRC_CONr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, COMM_IRC_CONr, &reg_value, RATE_TYPE0f, &temp));
            *value = temp;
            break;

        case DRV_RATE_CONFIG_DROP_ENABLE: /* per chip */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, COMM_IRC_CONr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, COMM_IRC_CONr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, COMM_IRC_CONr, &reg_value, DROP_EN0f, &temp));
            *value = temp;
            break;
        case DRV_RATE_CONFIG_PKT_MASK: /* per chip */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, COMM_IRC_CONr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, COMM_IRC_CONr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, COMM_IRC_CONr, &reg_value, PKT_MSK0f, &temp));
            *value = temp;
            break;
        default:
            return SOC_E_PARAM;
    }
    soc_cm_debug(DK_PORT, 
   "drv_bcm5395_rate_config_get: unit = %d, port = %d, type = %d, value = %d\n",
        unit, port, config_type, *value);
    return rv;
}

/*
 *  Function : drv_bcm5395_rate_set
 *
 *  Purpose :
 *      Set the ingress/egress rate control to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      direction   :   direction of rate control (ingress/egress). 
 *      kbits_sec_min  :  minimum bandwidth, kbits/sec.
 *      kbits_sec_max  :  maximum bandwidth, kbits/sec.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm5395_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, 
    uint32 kbits_sec_min, uint32 kbits_sec_max, uint32 burst_size)
{
    uint32      port;
    int         rv = SOC_E_NONE;

    soc_cm_debug(DK_PORT, 
        "drv_bcm5395_rate_set: unit = %d, bmp = %x, %s, \
        kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), (direction - 1) ? "EGRESS" : "INGRESS", 
        kbits_sec_min, kbits_sec_max, burst_size);
    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    _drv_bcm5395_port_irc_set(unit, port, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    _drv_bcm5395_port_erc_set(unit, port, kbits_sec_max, burst_size));
            }    
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }
    return rv;
}

/*
 *  Function : drv_bcm5395_rate_get
 *
 *  Purpose :
 *      Get the ingress/egress rate control to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      direction   :   direction of rate control (ingress/egress). 
 *      kbits_sec_min  :  (OUT) minimum bandwidth, kbits/sec.
 *      kbits_sec_max  :  (OUT) maximum bandwidth, kbits/sec.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm5395_rate_get(int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *kbits_sec_min, uint32 *kbits_sec_max, uint32 *burst_size)
{
    int         rv = SOC_E_NONE;
    uint32 min_rate = 0; /* Dummy variable */

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            SOC_IF_ERROR_RETURN(
                _drv_bcm5395_port_irc_get(unit, port, kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            SOC_IF_ERROR_RETURN(
                _drv_bcm5395_port_erc_get(unit, port, kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }
    soc_cm_debug(DK_PORT, 
        "drv_bcm5395_rate_get: unit = %d, port = %d, %s, \
        kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n",
        unit, port, (direction - 1) ? "EGRESS" : "INGRESS", 
        min_rate, *kbits_sec_max, *burst_size);

    return rv;
}
