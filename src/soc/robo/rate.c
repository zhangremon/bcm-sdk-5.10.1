/*
 * $Id: rate.c 1.15 Broadcom SDK $
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

static int robo_ingress_rate_init_flag = 0;

#define IRC_PKT_MASK    0x3f
#define BCM5348_IRC_PKT_MASK    0x7f
#define BCM5395_IRC_PKT_MASK    0x7f
/*
 *  Function : _drv_port_irc_set
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
_drv_port_irc_set(int unit, uint32 port, uint32 limit, uint32 burst_size)
{
    uint32  reg_addr, reg_value, reg_len, temp = 0;
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
        
        /* Disable ingress rate control 
          *    - ING_RC_ENf can't be set as 0, it will stop this port's storm 
          *       control rate also.
          *    - to prevent the affecting on other ports' ingress rate cotrol, 
          *       global ingress rate setting is not allowed been modified on 
          *       trying to disable this port's ingress rate control also. 
          *    - set the REF_CNT to the MAX value means packets could 
          *       be forwarded by no limit rate. (set to 0 will block all this 
          *       port's traffic)
          */
         temp = 254;
         SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, &reg_value, REF_CNT0f, &temp));
       
          
    } else {    /* Enable ingress rate control */
        /* check if global ingress rate config is set */ 
        if (!robo_ingress_rate_init_flag) {
            pbmp = PBMP_ALL(unit);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->rate_config_set)(
                                unit, pbmp, 
                                DRV_RATE_CONFIG_PKT_MASK, IRC_PKT_MASK));
            robo_ingress_rate_init_flag = 1;
        }

        burst_kbyte = burst_size / 8;
        if (SOC_IS_ROBO5324(unit)) {
            if (burst_size > (130 * 8)) { /* 130 KB */
                return SOC_E_PARAM;
            }
            /* burst size */
            if (burst_kbyte <= 6) { /* 6KB */
                temp = 0;
            } else if (burst_kbyte <= 10) { /* 10KB */
                temp = 1;
            } else if (burst_kbyte <= 18) { /* 18KB */
                temp = 2;
            } else if (burst_kbyte <= 34) { /* 34KB */
                temp = 3;
            } else if (burst_kbyte <= 66) { /* 66KB */
                temp = 4;
            } else { /* 130KB */
                temp = 5;
            } 
        } else if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5395(unit) ||
                   SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)){
            if (burst_size > (500 * 8)) { /* 500 KB */
                return SOC_E_PARAM;
            }
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
            temp = 255;
        }

        /* Setting ingress rate 
         *    - here we defined ingress rate control will be disable if 
         *       REF_CNT=255. (means no rate control) 
         *    - this definition is for seperate different rate between 
         *       "Ingress rate control" and "Strom rate control"
         *    - thus if the gave limit value trasfer REF_CNT is 255, we reasign
         *       REF_CNT to be 254
         */
        temp = (temp == 255) ? 254 : temp;
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
 *  Function : _drv_port_irc_get
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
_drv_port_irc_get(uint32 unit, uint32 port, uint32 *limit, uint32 *burst_size)
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
    if (robo_ingress_rate_init_flag) {
        temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->rate_config_get)(
                            unit, SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0), 
                            DRV_RATE_CONFIG_PKT_MASK, &temp));
                            
        /* if robo_ingress_rate_init_flag=0, only next ingress rate setting may 
         * set the properly PKT_MASK0 again currenly.
         */ 
        robo_ingress_rate_init_flag = (temp == 0) ? 0 : 1;
    }

    /* Check ingress rate control  
      *    - ING_RC_ENf should not be 0 in the runtime except the system been 
      *       process without ingress rate setting or user manuel config 
      *       register value. It will stop this port's storm control rate also.
      *    - set the REF_CNT to the MAX value means packets could 
      *       be forwarded by no limit rate. (set to 0 will block all this 
      *       port's traffic)
      */
     temp = 0;
     SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_IRC_CONr, &reg_value, REF_CNT0f, &temp));

    if (temp == 254) {
        *limit = 0;
        *burst_size = 0;
    } else {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, &reg_value, BUCKET_SIZE0f, &temp));
        
        if (SOC_IS_ROBO5324(unit)) {
            switch (temp) {
            case 0:
                *burst_size = 6 * 8; /* 6KB = 48 Kbits */
                break;
            case 1:
                *burst_size = 10 * 8; /* 10KB */
                break;
            case 2:
                *burst_size = 18 * 8; /* 18KB */
                break;
            case 3:
                *burst_size = 34 * 8; /* 34KB */
                break;
            case 4:
                *burst_size = 66 * 8; /* 66KB */
                break;
            case 5:
                *burst_size = 130 * 8; /* 130KB */
                break;
            default:
                return SOC_E_INTERNAL;
            }
        }
        if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5395(unit) ||
            SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {
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
 *  Function : _drv_bcm5348_port_irc_set
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
_drv_bcm5348_port_irc_set(int unit, uint32 port, uint32 limit,
    uint32 burst_size)
{
    uint32  reg_addr, reg_len, temp;
    int     rv= SOC_E_NONE;
    uint64 reg_value64;
    uint32  burst_kbyte = 0;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    if (limit == 0) { /* Disable ingress rate control */
        
        /* Disable ingress rate control 
          *    - ING_RC_ENf can't be set as 0, it will stop this port's storm 
          *       control rate also.
          *    - to prevent the affecting on other ports' ingress rate cotrol, 
          *       global ingress rate setting is not allowed been modified on 
          *       trying to disable this port's ingress rate control also. 
          *    - set the REF_CNT to the MAX value means packets could 
          *       be forwarded by no limit rate. (set to 0 will block all this 
          *       port's traffic)
          */
         temp = 254;
         SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, REF_CNT0f, &temp));
        /* Clear packet types */
         temp = 0;
         SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, PKT_MSK0f, &temp));
       
          
    } else {    /* Enable ingress rate control */

        /* Set packet types */
        temp = BCM5348_IRC_PKT_MASK;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, PKT_MSK0f, &temp));

        if (burst_size > (130 * 8)) { /* 500 KB */
            return SOC_E_PARAM;
        }
        /* burst size */
        burst_kbyte = burst_size / 8;
        if (burst_kbyte <= 6) { /* 6KB */
            temp = 0;
        } else if (burst_kbyte <= 10) { /* 10KB */
            temp = 1;
        } else if (burst_kbyte <= 18) { /* 18KB */
            temp = 2;
        } else if (burst_kbyte <= 34) { /* 34KB */
            temp = 3;
        } else if (burst_kbyte <= 66) { /* 66KB */
            temp = 4;
        } else { /* 130KB */
            temp = 5;
        } 
        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                BUCKET_SIZE0f, &temp));

        /* refresh count  (fixed type)*/
        if (limit <= 1792) { /* 64KB ~ 1.792MB */
            temp = ((limit-1) / 64) +1;
        } else if (limit <= 100000){ /* 2MB ~ 100MB */
            temp = (limit /1000 ) + 27;
        } else if (limit <= 1000000){ /* 104MB ~ 1000MB */
            temp = (limit /8000) + 115;
        } else {
            temp = 255;
        }

        /* Setting ingress rate 
         *    - here we defined ingress rate control will be disable if 
         *       REF_CNT=255. (means no rate control) 
         *    - this definition is for seperate different rate between 
         *       "Ingress rate control" and "Strom rate control"
         *    - thus if the gave limit value trasfer REF_CNT is 255, we reasign
         *       REF_CNT to be 254
         */
        temp = (temp == 255) ? 254 : temp;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, REF_CNT0f, &temp));

        /* enable ingress rate control */
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, ING_RC_ENf, &temp));
    }
    /* write register */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
    return rv;
}

/*
 *  Function : _drv_bcm5348_port_irc_get
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
_drv_bcm5348_port_irc_get(uint32 unit, uint32 port, uint32 *limit, 
    uint32 *burst_size)
{
    uint32  reg_addr, reg_len, temp;
    int     rv= SOC_E_NONE;
    uint64 reg_value64;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }    

    /* Check ingress rate control  
      *    - ING_RC_ENf should not be 0 in the runtime except the system been 
      *       process without ingress rate setting or user manuel config 
      *       register value. It will stop this port's storm control rate also.
      *    - set the REF_CNT to the MAX value means packets could 
      *       be forwarded by no limit rate. (set to 0 will block all this 
      *       port's traffic)
      */
     temp = 0;
     SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, REF_CNT0f, &temp));

    if (temp == 254) {
        *limit = 0;
        *burst_size = 0;
    } else {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                BUCKET_SIZE0f, &temp));
        
        switch (temp) {
        case 0:
            *burst_size = 6 * 8; /* 6KB */
            break;
        case 1:
            *burst_size = 10 * 8; /* 10KB */
            break;
        case 2:
            *burst_size = 18 * 8; /* 18KB */
            break;
        case 3:
            *burst_size = 34 * 8; /* 34KB */
            break;
        case 4:
            *burst_size = 66 * 8; /* 66KB */
            break;
        case 5:
            *burst_size = 130 * 8; /* 130KB */
            break;
        default:
            return SOC_E_INTERNAL;
        }

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                REF_CNT0f, &temp));
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
 *  Function : _drv_port_erc_set
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
_drv_port_erc_set(uint32 unit, uint32 port, uint32 limit, uint32 burst_size)
{
    uint32  reg_addr, reg_value, reg_len, temp = 0;
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
        /* burst size */
        burst_kbyte = burst_size / 8;
        if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit)
            ||SOC_IS_ROBO5347(unit)) {
            if (burst_size > (130 * 8)) { /* 130 KB */
                return SOC_E_PARAM;
            }
            if (burst_kbyte <= 6) { /* 6KB */
                temp = 0;
            } else if (burst_kbyte <= 10) { /* 10KB */
                temp = 1;
            } else if (burst_kbyte <= 18) { /* 18KB */
                temp = 2;
            } else if (burst_kbyte <= 34) { /* 34KB */
                temp = 3;
            } else if (burst_kbyte <= 66) { /* 66KB */
                temp = 4;
            } else { /* 130KB */
                temp = 5;
            } 
        } else if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit) ||
            SOC_IS_ROBO5395(unit)) {
            if (burst_size > (500 * 8)) { /* 500 KB */
                return SOC_E_PARAM;
            }
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
        } else {
            return SOC_E_UNAVAIL;
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
 *  Function : _drv_port_erc_get
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
_drv_port_erc_get(uint32 unit, uint32 port, uint32 *limit, uint32 *burst_size)
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
        if (SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5348(unit)||
            SOC_IS_ROBO5347(unit)) {
            switch (temp) {
                case 0:
                    *burst_size = 6 * 8; /* 6KB */
                    break;
                case 1:
                    *burst_size = 10 * 8; /* 10KB */
                    break;
                case 2:
                    *burst_size = 18 * 8; /* 18KB */
                    break;
                case 3:
                    *burst_size = 34 * 8; /* 34KB */
                    break;
                case 4:
                    *burst_size = 66 * 8; /* 66KB */
                    break;
                case 5:
                    *burst_size = 130 * 8; /* 130KB */
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
        }
        if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)||
            SOC_IS_ROBO5395(unit)) {
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
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_ERC_CONr, &reg_value, REF_CNTSf, &temp));
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
 *  Function : _drv_rate_control_misc
 *
 *  Purpose :
 *     Enable XLEN_EN bit to include IPG for rate counting.
 *
 *  Parameters :
 *      unit        :   unit id
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_rate_control_misc(uint32 unit)
{
    int     reg_len;
    uint32  reg_addr, reg_value, temp;
    int rv = SOC_E_NONE;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, COMM_IRC_CONr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, COMM_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    temp = 1;
    if ((rv = (DRV_SERVICES(unit)->reg_field_set)
                (unit, COMM_IRC_CONr, &reg_value, XLEN_ENf, &temp)) < 0 ) {
        return rv;
    }
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    return rv;
}

/*
 *  Function : drv_rate_config_set
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
drv_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type, 
    uint32 value)
{
    uint32  reg_addr, reg_value, reg_len, temp;
    int     rv = SOC_E_NONE;
    soc_port_t port;
    uint64  reg_value64;

    soc_cm_debug(DK_PORT, 
        "drv_rate_config_set: unit = %d, bmp = %x, type = %d, value = %d\n",
        unit, SOC_PBMP_WORD_GET(pbmp, 0), config_type, value);
    /* set bucket 0*/
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: 
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                /* per port */
                PBMP_ITER(pbmp, port) {
                    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, PORT_IRC_CONr, port, 0);
                    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, PORT_IRC_CONr);
                    if ((rv = (DRV_SERVICES(unit)->reg_read)
                        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                        return rv;
                    }
                    temp = value;            
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                        (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                            RATE_TYPE0f, &temp));
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                        (unit, reg_addr, (uint32 *)&reg_value64, reg_len));
                }
            } else {
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
            }
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE:
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                /* per port */
                PBMP_ITER(pbmp, port) {
                    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, PORT_IRC_CONr, port, 0);
                    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, PORT_IRC_CONr);
                    if ((rv = (DRV_SERVICES(unit)->reg_read)
                        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                        return rv;
                    }
                    temp = value;            
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                        (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                            DROP_EN0f, &temp));
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                        (unit, reg_addr, (uint32 *)&reg_value64, reg_len));
                }
            } else {
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
            }
            break;
        case DRV_RATE_CONFIG_PKT_MASK: 
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                /* per port */
                PBMP_ITER(pbmp, port) {
                    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, PORT_IRC_CONr, port, 0);
                    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, PORT_IRC_CONr);
                    if ((rv = (DRV_SERVICES(unit)->reg_read)
                        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                        return rv;
                    }
                    temp = value & BCM5348_IRC_PKT_MASK;

                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                        (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                                PKT_MSK0f, &temp));
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                        (unit, reg_addr, (uint32 *)&reg_value64, reg_len));
                }
            } else {
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
                    temp = value & IRC_PKT_MASK;
                    if (SOC_IS_ROBO5395(unit)) {
                        temp = value & BCM5395_IRC_PKT_MASK;
                    }
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                        (unit, COMM_IRC_CONr, &reg_value, PKT_MSK0f, &temp));
                    if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {
                        /* Extended packet mask: SA lookup fail */
                        temp = value >> 6;
                        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                            (unit, COMM_IRC_CONr, &reg_value, 
                                EXT_PKT_MSK0f, &temp));
                    }
                        
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                        (unit, reg_addr, &reg_value,reg_len));
    
                }
            }
            break;
        default:
            return SOC_E_PARAM;
    }
    return rv;
}

/*
 *  Function : drv_rate_config_get
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
drv_rate_config_get(int unit, uint32 port, uint32 config_type, uint32 *value)
{
    uint32  reg_addr, reg_value, reg_len, temp;
    int     rv = SOC_E_NONE;
    uint64 reg_value64;

    /* get bucket 0*/
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: /* per chip */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, PORT_IRC_CONr, port, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, PORT_IRC_CONr);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, RATE_TYPE0f, &temp));
                *value = temp;
            } else {
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
            }
            break;

        case DRV_RATE_CONFIG_DROP_ENABLE: /* per chip */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, PORT_IRC_CONr, port, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, PORT_IRC_CONr);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                        DROP_EN0f, &temp));
                *value = temp;
            } else {
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
            }
            break;
        case DRV_RATE_CONFIG_PKT_MASK: /* per chip */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, PORT_IRC_CONr, port, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, PORT_IRC_CONr);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                        PKT_MSK0f, &temp));
                *value = temp;
            } else {
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
                if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                        (unit, COMM_IRC_CONr, &reg_value, 
                            EXT_PKT_MSK0f, &temp));
                    *value += (temp << 6);
                }
            }
            break;
        default:
            return SOC_E_PARAM;
    }
    soc_cm_debug(DK_PORT, 
        "drv_rate_config_get: unit = %d, port = %d, type = %d, value = %d\n",
        unit, port, config_type, *value);
    return rv;
}

/*
 *  Function : drv_rate_config_set
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
drv_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n,int direction, 
    uint32 kbits_sec_min, uint32 kbits_sec_max, uint32 burst_size)
{
    uint32      port;
    int         rv = SOC_E_NONE;

    soc_cm_debug(DK_PORT, 
        "drv_rate_set: unit = %d, bmp = %x, %s, kbits_sec_min = %dK, kbits_sec_max = %dK, \
        burst size = %dKB\n", unit, SOC_PBMP_WORD_GET(bmp, 0), 
        (direction - 1) ? "EGRESS" : "INGRESS", kbits_sec_min, kbits_sec_max, burst_size);
    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            if (SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit)) {
                rv = _drv_rate_control_misc(unit);
                if ( rv < 0) {
                    return rv;
                }
            }
            PBMP_ITER(bmp, port) {
                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                    SOC_IF_ERROR_RETURN(
                        _drv_bcm5348_port_irc_set(unit, port, kbits_sec_max, 
                            burst_size));
                } else {
                    SOC_IF_ERROR_RETURN(
                        _drv_port_irc_set(unit, port, kbits_sec_max, burst_size));
                }
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            if (SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit)) {
                rv = _drv_rate_control_misc(unit);
                if ( rv < 0) {
                    return rv;
                }
            }
            if (SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5398(unit)||
                SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5397(unit)||
                SOC_IS_ROBO5347(unit)||SOC_IS_ROBO5395(unit)) {
                PBMP_ITER(bmp, port) {
                    SOC_IF_ERROR_RETURN(
                        _drv_port_erc_set(unit, port, kbits_sec_max, burst_size));
                }                
            }
            if (SOC_IS_ROBODINO(unit)){
                return SOC_E_UNAVAIL;
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
 *  Function : drv_rate_config_get
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
drv_rate_get(int unit, uint32 port, uint8 queue_n,int direction, 
    uint32 *kbits_sec_min, uint32 *kbits_sec_max, uint32 *burst_size)
{
    int         rv = SOC_E_NONE;
    uint32 min_rate = 0; /* Dummy variable */

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                SOC_IF_ERROR_RETURN(
                    _drv_bcm5348_port_irc_get(unit, port, kbits_sec_max, burst_size));
            } else {
                SOC_IF_ERROR_RETURN(
                    _drv_port_irc_get(unit, port, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            if (SOC_IS_ROBODINO(unit)){
                return SOC_E_UNAVAIL;
            }
            if (SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5398(unit)||
                SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5397(unit)||
                SOC_IS_ROBO5347(unit) || SOC_IS_ROBO5395(unit)){
                SOC_IF_ERROR_RETURN(
                    _drv_port_erc_get(unit, port, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }
    soc_cm_debug(DK_PORT, 
        "drv_rate_get: unit = %d, port = %d, %s, kbits_sec_min = %dK, kbits_sec_max = %dK, \
        burst size = %dKB\n", unit, port, (direction - 1) ? "EGRESS" : "INGRESS", 
        min_rate, *kbits_sec_max, *burst_size);

    return rv;
}
