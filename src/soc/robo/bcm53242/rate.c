/*
 * $Id: rate.c 1.19.66.4 Broadcom SDK $
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
#include <soc/mem.h>

static int robo_ingress_rate_init_flag = 0;

#define RATE_53242_IRC_PKT_MASK    0x7f
#define RATE_53242_MAX_REF_CNTS    0x3fff
#define RATE_53242_MAX_BUCKET_SIZE    0x7f

#define RATE_53242_REF_UNIT_64K    0
#define RATE_53242_REF_UNIT_1M    1

#define RATE_53242_METER_MAX 16383000 /* (RATE_53242_MAX_REF_CNTS * 1000) */
#define RATE_53242_METER_MIN 0
#define RATE_53242_BURST_MAX 8128 /* (1016Kbyte * 8) */
#define RATE_53242_BURST_MIN 0

/*
 *  Function : _drv_bcm53242_port_irc_set
 *
 *  Purpose :
 *  Set the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits per second)
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
_drv_bcm53242_port_irc_set(int unit, uint32 port, uint32 limit,
    uint32 burst_size)
{
    uint32  retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv= SOC_E_NONE;
    soc_pbmp_t pbmp;
    uint32 ref_u;
    uint32 quotient_64k, remainder_64k, quotient_1m, remainder_1m;
    uint32  burst_kbyte = 0;

    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /* coverity[unsigned_compare] */
    if ((limit > RATE_53242_METER_MAX) ||
        (limit < RATE_53242_METER_MIN)) {
        soc_cm_debug(DK_ERR, 
            "drv_bcm53242_port_irc_set : rate unsupported. \n");
        return SOC_E_PARAM;
    }

    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /* coverity[unsigned_compare] */
    if ((burst_size > RATE_53242_BURST_MAX) ||
        (burst_size < RATE_53242_BURST_MIN)) {
        soc_cm_debug(DK_ERR, 
            "drv_bcm53242_port_irc_set : burst size unsupported. \n");
        return SOC_E_PARAM;
    }

    /* 
     * Check maximum supported rate limit of FE and GE ports, 
     * respectively.
     */
    if (SOC_PBMP_MEMBER(PBMP_GE_ALL(unit), port)) {
        if (limit > 1000000) {
            return SOC_E_PARAM;
        }
    } else {
        if (limit > 100000) {
            return SOC_E_PARAM;
        }
    }

    /* check if global ingress rate config is set */ 
    if(limit != 0) {
        if (!robo_ingress_rate_init_flag) {
            pbmp = PBMP_ALL(unit);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->rate_config_set)(
                                unit, pbmp, 
                                DRV_RATE_CONFIG_PKT_MASK, 
                                RATE_53242_IRC_PKT_MASK));
            robo_ingress_rate_init_flag = 1;
        }
    }
    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers */
    /* Use RCM_DATA0 register set bucket 0 */
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto irc_set_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto irc_set_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto irc_set_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto irc_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto irc_set_exit;
        }

        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto irc_set_exit;
    }
        
    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto irc_set_exit;
    }
    
    if (limit == 0) { /* Disable ingress rate control */
        
        /* Disable ingress rate control 
          *    - IE_RC_ENf can't be set as 0, it will stop this port's storm 
          *       control rate also.
          *    - to prevent the affecting on other ports' ingress rate cotrol, 
          *       global ingress rate setting is not allowed been modified on 
          *       trying to disable this port's ingress rate control also. 
          *    - set the REF_CNT to the MAX value means packets could 
          *       be forwarded by no limit rate. (set to 0 will block all this 
          *       port's traffic)
          */
         ref_u = RATE_53242_REF_UNIT_1M;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_UNITf, &ref_u);
        temp = RATE_53242_MAX_REF_CNTS;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
        
        temp = RATE_53242_MAX_BUCKET_SIZE;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);

        /* enable ingress rate control */
        temp = 1;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            IE_RC_ENf, &temp);

        if ((rv = REG_WRITE_RCM_DATA0r(unit, &reg_value)) < 0) {
            goto irc_set_exit;
        }
          
    } else {    /* Enable ingress rate control */
        /* burst size */
        burst_kbyte = burst_size / 8;
        temp = burst_kbyte / 8; /* unit is 8Kbytes */
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, RCM_DATA0r, &reg_value, BUCKET_SIZEf, &temp);

        /* refresh count by refresh unit */
        /* Can be divided by 1000K with no remainder? */
        quotient_1m = limit / 1000;
        remainder_1m = limit - (quotient_1m * 1000);

        /* Can be divided by 62.5K with no remainder? */
        quotient_64k = (limit * 10) / 625;
        remainder_64k = (limit * 10) - (quotient_64k * 625);
        
        if (remainder_1m == 0) {
            ref_u = RATE_53242_REF_UNIT_1M;
            temp = quotient_1m;
        } else if (remainder_64k == 0) {
            ref_u = RATE_53242_REF_UNIT_64K;
            temp = quotient_64k;
        } else {
            /* Others */
            if (limit <= (1024000)) { /* (2^14 * 62.5) */
                /* Use 62.5K as unit */
                ref_u = RATE_53242_REF_UNIT_64K;
                temp = quotient_64k;
            } else {
                /* 62.5K unit can't represent, so use 1M as unit */
                ref_u = RATE_53242_REF_UNIT_1M;
                temp = quotient_1m;
            }
        }
        /* Setting ingress rate 
         *    - here we defined ingress rate control will be disable if 
         *       REF_CNT=16383. (means no rate control) 
         *    - this definition is for seperate different rate between 
         *       "Ingress rate control" and "Strom rate control"
         *    - thus if the gave limit value trasfer REF_CNT is 16383, we reasign
         *       REF_CNT to be 16382
         */
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_UNITf, &ref_u);

        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
        
        /* enable ingress rate control */
        temp = 1;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            IE_RC_ENf, &temp);

        if ((rv = REG_WRITE_RCM_DATA0r(unit, &reg_value)) < 0) {
            goto irc_set_exit;
        }
    }
    /* write to Rate Control Memory */
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto irc_set_exit;
    }

    temp = MEM_TABLE_WRITE;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Write Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto irc_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto irc_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto irc_set_exit;
    }
    
 irc_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : _drv_bcm53242_port_irc_get
 *
 *  Purpose :
 *   Get the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits per second)
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
_drv_bcm53242_port_irc_get(uint32 unit, uint32 port, uint32 *limit, 
    uint32 *burst_size)
{
    uint32  retry, temp, ref_u = 0;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv= SOC_E_NONE;

    /* check global ingress rate control setting */
    if (robo_ingress_rate_init_flag) {
        temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->rate_config_get)(
                            unit, port, 
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

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers */
    /* Use RCM_DATA0 register set bucket 0 */
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto irc_get_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto irc_get_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto irc_get_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto irc_get_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto irc_get_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto irc_get_exit;
    }
        
    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto irc_get_exit;
    }

    soc_RCM_DATA0r_field_get(unit, &reg_value,
        REF_UNITf, &ref_u);

    soc_RCM_DATA0r_field_get(unit, &reg_value,
        REF_CNTSf, &temp);
     
    if (temp == RATE_53242_MAX_REF_CNTS) {
        *limit = 0;
        *burst_size = 0;
    } else {
        if(ref_u == RATE_53242_REF_UNIT_64K) {
            temp *= 625;
            *limit = temp / 10;
        } else {
            *limit = temp * 1000;
        }
        soc_RCM_DATA0r_field_get(unit, &reg_value,
            BUCKET_SIZEf, &temp);
        
        *burst_size = temp * 8 * 8;
    }

 irc_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}


/*
 *  Function : _drv_bcm53242_port_erc_set
 *
 *  Purpose :
 *     Set the burst size and rate limit value of the selected port egress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value (Kbits per second).
 *      burst_size  :   max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53242_port_erc_set(uint32 unit, uint32 port, uint32 limit, uint32 burst_size)
{
    uint32  retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv = SOC_E_NONE;
    uint32 ref_u;
    uint32 quotient_64k, remainder_64k, quotient_1m, remainder_1m;
    uint32 burst_kbyte = 0;

    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /*    coverity[unsigned_compare]    */
    if ((limit > RATE_53242_METER_MAX) ||
        (limit < RATE_53242_METER_MIN)) {
        soc_cm_debug(DK_ERR, 
            "drv_bcm53242_port_erc_set : rate unsupported. \n");
        return  SOC_E_PARAM;
    }

    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /*    coverity[unsigned_compare]    */
    if ((burst_size > RATE_53242_BURST_MAX) ||
        (burst_size < RATE_53242_BURST_MIN)) {
        soc_cm_debug(DK_ERR, 
            "drv_bcm53242_port_erc_set : burst size unsupported. \n");
        return SOC_E_PARAM;
    }

    /* 
     * Check maximum supported rate limit of FE and GE ports, 
     * respectively.
     */
    if (SOC_PBMP_MEMBER(PBMP_GE_ALL(unit), port)) {
        if (limit > 1000000) {
            return SOC_E_PARAM;
        }
    } else {
        if (limit > 100000) {
            return SOC_E_PARAM;
        }
    }

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers */
    /* Use RCM_DATA0 register set bucket 0 */
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto erc_set_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b1 : Egress Rate Control Memory access */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto erc_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto erc_set_exit;
    }
        
    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto erc_set_exit;
    }
    
    if (limit == 0) { /* Disable egress rate control */
        temp = 0;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            IE_RC_ENf, &temp);
    } else {    /* Enable ingress rate control */
        /* burst size */
        burst_kbyte = burst_size / 8;
        temp = burst_kbyte / 8;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);

        /* refresh count by refresh unit */
        /* Can be divided by 1000K with no remainder? */
        quotient_1m = limit / 1000;
        remainder_1m = limit - (quotient_1m * 1000);

        /* Can be divided by 62.5K with no remainder? */
        quotient_64k = (limit * 10) / 625;
        remainder_64k = (limit * 10) - (quotient_64k * 625);
        
        if (remainder_1m == 0) {
            ref_u = RATE_53242_REF_UNIT_1M;
            temp = quotient_1m;
        } else if (remainder_64k == 0) {
            ref_u = RATE_53242_REF_UNIT_64K;
            temp = quotient_64k;
        } else {
            /* Others */
            if (limit <= (1024000)) { /* (2^14 * 62.5) */
                /* Use 62.5K as unit */
                ref_u = RATE_53242_REF_UNIT_64K;
                temp = quotient_64k;
            } else {
                /* 62.5K unit can't represent, so use 1M as unit */
                ref_u = RATE_53242_REF_UNIT_1M;
                temp = quotient_1m;
            }
        }

        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_UNITf, &ref_u);

        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);

        /* enable egress rate control */
        temp = 1;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            IE_RC_ENf, &temp);

    }
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto erc_set_exit;
    }

    /* write to Rate Control Memory */
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }

    temp = MEM_TABLE_WRITE;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b1 : Egress Rate Control Memory access */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Write Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto erc_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto erc_set_exit;
    }
    
 erc_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : _drv_bcm53242_port_erc_get
 *
 *  Purpose :
 *     Get the burst size and rate limit value of the selected port.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value (Kbits per second).
 *      burst_size  :   max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53242_port_erc_get(uint32 unit, uint32 port, uint32 *limit, uint32 *burst_size)
{
    uint32  retry, temp, ref_u;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv= SOC_E_NONE;

    /* process write action */
    MEM_LOCK(unit, GEN_MEMORYm);
    
    /* Clear Rate Control Memory Data access registers */
    /* Use RCM_DATA0 register set bucket 0 */
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto erc_get_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto erc_get_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_get_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b1 : Egress Rate Control Memory access */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_get_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto erc_get_exit;
        }
        
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto erc_get_exit;
    }
        
    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto erc_get_exit;
    }

    soc_RCM_DATA0r_field_get(unit, &reg_value,
            REF_UNITf, &ref_u);

    soc_RCM_DATA0r_field_get(unit, &reg_value,
            IE_RC_ENf, &temp);
     
    if (temp ==0) {
        *limit = 0;
        *burst_size = 0;
    } else {
        soc_RCM_DATA0r_field_get(unit, &reg_value,
            REF_CNTSf, &temp);

        if(ref_u == RATE_53242_REF_UNIT_64K) {
            temp *= 625;
            *limit = temp / 10;
        } else {
            *limit = temp * 1000;
        }

        soc_RCM_DATA0r_field_get(unit, &reg_value,
            BUCKET_SIZEf, &temp);

        *burst_size = temp * 8 * 8; 
    }

 erc_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : _drv_bcm53242_port_queue_erc_set
 *
 *  Purpose :
 *     Set the burst size and rate limit value of the selected port egress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      queue_n:   COSQ id.
 *      limit  :   rate limit value. (Kbits per second)
 *      burst_size  :   max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53242_port_queue_erc_set(uint32 unit, uint32 port, uint8 queue_n, 
    uint32 limit, uint32 burst_size)
{
    uint32  retry, temp, fld_value = 0;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv= SOC_E_NONE;
    uint32 ref_u;
    uint32 quotient_64k, remainder_64k, quotient_1m, remainder_1m;
    uint32 burst_kbyte = 0;

    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /*    coverity[unsigned_compare]    */
    if ((limit > RATE_53242_METER_MAX) ||
        (limit < RATE_53242_METER_MIN)) {
        soc_cm_debug(DK_ERR, 
            "drv_bcm53242_port_erc_set : rate unsupported. \n");
        return  SOC_E_PARAM;
    }

    /*
      * COVERITY
      *
      * Comparing unsigned less than zero is never true.
      * It is kept intentionally as a defensive check.
      */
    /*    coverity[unsigned_compare]    */
    if ((burst_size > RATE_53242_BURST_MAX) ||
        (burst_size < RATE_53242_BURST_MIN)) {
        soc_cm_debug(DK_ERR, 
            "drv_bcm53242_port_erc_set : burst size unsupported. \n");
        return SOC_E_PARAM;
    }

    /* 
     * Check maximum supported rate limit of FE and GE ports, 
     * respectively.
     */
    if (SOC_PBMP_MEMBER(PBMP_GE_ALL(unit), port)) {
        if (limit > 1000000) {
            return SOC_E_PARAM;
        }
    } else {
        if (limit > 100000) {
            return SOC_E_PARAM;
        }
    }

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers RCM_DATA1/2/3/4*/
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA1r(unit, &zero_value)) < 0) {
        goto erc_set_exit;
    }
    if ((rv = REG_WRITE_RCM_DATA2r(unit, &zero_value)) < 0) {
        goto erc_set_exit;
    }
    if ((rv = REG_WRITE_RCM_DATA3r(unit, &zero_value)) < 0) {
        goto erc_set_exit;
    }
    if ((rv = REG_WRITE_RCM_DATA4r(unit, &zero_value)) < 0) {
        goto erc_set_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b1 : Egress Rate Control Memory access */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto erc_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto erc_set_exit;
    }
        
    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA1r(unit, &reg_value)) < 0) {
        goto erc_set_exit;
    }

    /* 
     * Check if the port already enabled 
     * "Per Queue Egress Rate Control Enable" bit on RCM_DATA1r
     * If yes, go on rate control configuration
     * If not, enable the bit and set all queues to maximum limits.
     * 
     * Since BCM53242 only support egress rate control of all queues 
     * enable/disable at the same time. The above operation can achieve
     * per queue enable/disable.
     */
    soc_RCM_DATA1r_field_get(unit, &reg_value,
        ERC_Q_ENf, &fld_value);

    if (!fld_value) {
        /* Enable Maximum rate limit for Q0 */
        temp = 1;
        soc_RCM_DATA1r_field_set(unit, &reg_value,
            ERC_Q_ENf, &temp);
        temp = RATE_53242_MAX_REF_CNTS;
        soc_RCM_DATA1r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
        temp = RATE_53242_MAX_BUCKET_SIZE;     
        soc_RCM_DATA1r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);
        temp = RATE_53242_REF_UNIT_1M;   
        soc_RCM_DATA1r_field_set(unit, &reg_value,
            REF_UNITf, &temp);
        if ((rv = REG_WRITE_RCM_DATA1r(unit, &reg_value)) < 0) {
            goto erc_set_exit;
        }

          /* Enable Maximum rate limit for Q1 */
        if ((rv = REG_READ_RCM_DATA2r(unit, &reg_value)) < 0) {
            goto erc_set_exit;
        }
        temp = RATE_53242_MAX_REF_CNTS;
        soc_RCM_DATA2r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
        temp = RATE_53242_MAX_BUCKET_SIZE;     
        soc_RCM_DATA2r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);
        temp = RATE_53242_REF_UNIT_1M;        
        soc_RCM_DATA2r_field_set(unit, &reg_value,
            REF_UNITf, &temp);
        if ((rv = REG_WRITE_RCM_DATA2r(unit, &reg_value)) < 0) {
            goto erc_set_exit;
        }

          /* Enable Maximum rate limit for Q2 */
        if ((rv = REG_READ_RCM_DATA3r(unit, &reg_value)) < 0) {
            goto erc_set_exit;
        }
        
        temp = RATE_53242_MAX_REF_CNTS;
        soc_RCM_DATA3r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
        temp = RATE_53242_MAX_BUCKET_SIZE;     
        soc_RCM_DATA3r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);
        temp = RATE_53242_REF_UNIT_1M;        
        soc_RCM_DATA3r_field_set(unit, &reg_value,
            REF_UNITf, &temp);
        if ((rv = REG_WRITE_RCM_DATA3r(unit, &reg_value)) < 0) {
            goto erc_set_exit;
        }

          /* Enable Maximum rate limit for Q3 */
        if ((rv = REG_READ_RCM_DATA4r(unit, &reg_value)) < 0) {
            goto erc_set_exit;
        }
        temp = RATE_53242_MAX_REF_CNTS;
        soc_RCM_DATA4r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
        temp = RATE_53242_MAX_BUCKET_SIZE;     
        soc_RCM_DATA4r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);
        temp = RATE_53242_REF_UNIT_1M;        
        soc_RCM_DATA4r_field_set(unit, &reg_value,
            REF_UNITf, &temp);
        if ((rv = REG_WRITE_RCM_DATA4r(unit, &reg_value)) < 0) {
            goto erc_set_exit;
        }
    }
    
    switch (queue_n) {
        case 0:
            if ((rv = REG_READ_RCM_DATA1r(unit, &reg_value)) < 0) {
                goto erc_set_exit;
            }
            break;
        case 1:
            if ((rv = REG_READ_RCM_DATA2r(unit, &reg_value)) < 0) {
                goto erc_set_exit;
            }
            break;
        case 2:
            if ((rv = REG_READ_RCM_DATA3r(unit, &reg_value)) < 0) {
                goto erc_set_exit;
            }
            break;
        case 3:
            if ((rv = REG_READ_RCM_DATA4r(unit, &reg_value)) < 0) {
                goto erc_set_exit;
            }
            break;
        default:
            rv = SOC_E_PARAM;
            goto erc_set_exit;
    }
    
    if (limit == 0) { /* Disable egress rate control */
        temp = RATE_53242_MAX_REF_CNTS;
        soc_RCM_DATA1r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
        temp = RATE_53242_MAX_BUCKET_SIZE;     
        soc_RCM_DATA1r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);
        temp = RATE_53242_REF_UNIT_1M;        
        soc_RCM_DATA1r_field_set(unit, &reg_value,
            REF_UNITf, &temp);
    } else {    /* Enable ingress rate control */
        /* burst size */
        burst_kbyte = burst_size / 8;
        temp = burst_kbyte / 8;
        soc_RCM_DATA1r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);

        /* refresh count by refresh unit */
        /* Can be divided by 1000K with no remainder? */
        quotient_1m = limit / 1000;
        remainder_1m = limit - (quotient_1m * 1000);

        /* Can be divided by 62.5K with no remainder? */
        quotient_64k = (limit * 10) / 625;
        remainder_64k = (limit * 10) - (quotient_64k * 625);
        
        if (remainder_1m == 0) {
            ref_u = RATE_53242_REF_UNIT_1M;
            temp = quotient_1m;
        } else if (remainder_64k == 0) {
            ref_u = RATE_53242_REF_UNIT_64K;
            temp = quotient_64k;
        } else {
            /* Others */
            if (limit <= (1024000)) { /* (2^14 * 62.5) */
                /* Use 62.5K as unit */
                ref_u = RATE_53242_REF_UNIT_64K;
                temp = quotient_64k;
            } else {
                /* 62.5K unit can't represent, so use 1M as unit */
                ref_u = RATE_53242_REF_UNIT_1M;
                temp = quotient_1m;
            }
        }

        soc_RCM_DATA1r_field_set(unit, &reg_value,
            REF_UNITf, &ref_u);

        soc_RCM_DATA1r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
    }
    switch (queue_n) {
        case 0:
            if ((rv = REG_WRITE_RCM_DATA1r(unit, &reg_value)) < 0) {
                goto erc_set_exit;
            }
            break;
        case 1:
            if ((rv = REG_WRITE_RCM_DATA2r(unit, &reg_value)) < 0) {
                goto erc_set_exit;
            }
            break;
        case 2:
            if ((rv = REG_WRITE_RCM_DATA3r(unit, &reg_value)) < 0) {
                goto erc_set_exit;
            }
            break;
        case 3:
            if ((rv = REG_WRITE_RCM_DATA4r(unit, &reg_value)) < 0) {
                goto erc_set_exit;
            }
            break;
    }

    /* write to Rate Control Memory */
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }

    temp = MEM_TABLE_WRITE;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b1 : Egress Rate Control Memory access */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Write Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto erc_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto erc_set_exit;
    }
    
 erc_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : _drv_bcm53242_queue_port_erc_get
 *
 *  Purpose :
 *     Get the burst size and rate limit value of the selected port.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      queue_n:   COSQ id.
 *      limit  :   rate limit value (Kbits per second).
 *      burst_size  :   max burst size (Kbits).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53242_port_queue_erc_get(uint32 unit, uint32 port, uint8 queue_n,
    uint32 *limit, uint32 *burst_size)
{
    uint32  retry, temp, ref_u;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv= SOC_E_NONE;

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers RCM_DATA1/2/3/4*/
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA1r(unit, &zero_value)) < 0) {
        goto erc_get_exit;
    }
    if ((rv = REG_WRITE_RCM_DATA2r(unit, &zero_value)) < 0) {
        goto erc_get_exit;
    }
    if ((rv = REG_WRITE_RCM_DATA3r(unit, &zero_value)) < 0) {
        goto erc_get_exit;
    }
    if ((rv = REG_WRITE_RCM_DATA4r(unit, &zero_value)) < 0) {
        goto erc_get_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto erc_get_exit;
    }
    
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_get_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b1 : Egress Rate Control Memory access */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto erc_get_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto erc_get_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto erc_get_exit;
    }

    /* 
     * Get global control bit of Per Queue Egress Rate Control.
     * If it is in disabled state, return disabled value of rate control.
     */
    if ((rv = REG_READ_RCM_DATA1r(unit, &reg_value)) < 0) {
        goto erc_get_exit;
    }
    soc_RCM_DATA1r_field_get(unit, &reg_value,
        ERC_Q_ENf, &temp);
    if (temp ==0) {
        *limit = 0;
        *burst_size = 0;
        rv = SOC_E_NONE;
        goto erc_get_exit;
    }


    /* 
     * Get per queue rate limit and bucket size.
     */
    switch(queue_n) {
        case 0:
            /* Already read */
            break;
        case 1:
            if ((rv = REG_READ_RCM_DATA2r(unit, &reg_value)) < 0) {
                goto erc_get_exit;
            }
            break;
        case 2:
            if ((rv = REG_READ_RCM_DATA3r(unit, &reg_value)) < 0) {
                goto erc_get_exit;
            }
            break;
        case 3:
            if ((rv = REG_READ_RCM_DATA4r(unit, &reg_value)) < 0) {
                goto erc_get_exit;
            }
            break;
        default:
            rv = SOC_E_PARAM;
            goto erc_get_exit;
    }
    /* Read Rate Control Memory DATA 1/2/3/4 register */

    soc_RCM_DATA1r_field_get(unit, &reg_value,
        REF_UNITf, &ref_u);

    soc_RCM_DATA1r_field_get(unit, &reg_value,
        REF_CNTSf, &temp);

    if(ref_u == RATE_53242_REF_UNIT_64K) {
        temp *= 625;
        *limit = temp / 10;
    } else {
        *limit = temp * 1000;
    }

    /* 
     * If the value of rate limit meets maximum,
     * return *limit=0 adn *burst_size=0 to represent the rate control is disable.
     */
    if (*limit == RATE_53242_METER_MAX) {
        *limit = 0;
        *burst_size = 0;
        rv = SOC_E_NONE;
        goto erc_get_exit;
    }

    soc_RCM_DATA1r_field_get(unit, &reg_value,
        BUCKET_SIZEf, &temp);
    *burst_size = temp * 8 * 8; 

 erc_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : drv_bcm53242_rate_config_set
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
drv_bcm53242_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type, 
    uint32 value)
{
    uint32  retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv = SOC_E_NONE;
    soc_port_t port;

    soc_cm_debug(DK_PORT, 
        "drv_rate_config_set: unit = %d, bmp = %x, type = %d, value = %d\n",
        unit, SOC_PBMP_WORD_GET(pbmp, 0), config_type, value);

    /* set bucket 0*/
    /* per port */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    PBMP_ITER(pbmp, port) {
        /* process write action */
        /* Clear Rate Control Memory Data access registers */
        /* Use RCM_DATA0 register set bucket 0 */
        zero_value = 0;
        if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
            goto rate_config_set_exit;
        }

        /* Set Rate Control Memory Port register */
        soc_RCM_PORTr_field_set(unit, &acc_ctrl,
            RCM_PORTf, (uint32 *)&port);
        if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
            goto rate_config_set_exit;
        }
        
        
        /* Read Rate Control Memory register */
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto rate_config_set_exit;
        }

        temp = MEM_TABLE_READ;
        soc_RCM_CTLr_field_set(unit, &acc_ctrl,
            RCM_RWf, &temp);
            
        /* 1'b0 : Ingress Rate Control Memory access */
        temp = 0;
        soc_RCM_CTLr_field_set(unit, &acc_ctrl,
            IE_INDf, &temp);
        
        /* Start Read Process */
        temp = 1;
        soc_RCM_CTLr_field_set(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto rate_config_set_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto rate_config_set_exit;
            }

            soc_RCM_CTLr_field_get(unit, &acc_ctrl,
                RCM_RW_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto rate_config_set_exit;
        }
            
        /* Read Rate Control Memory DATA 0 register */
        if ((rv = REG_READ_RCM_DATA0r(unit, &reg_value)) < 0) {
            goto rate_config_set_exit;
        }

        temp = value;
        switch (config_type) {
            case DRV_RATE_CONFIG_RATE_TYPE: 
                soc_RCM_DATA0r_field_set(unit, &reg_value,
                    REF_UNITf, &temp);
                break;
            case DRV_RATE_CONFIG_DROP_ENABLE: 
                soc_RCM_DATA0r_field_set(unit, &reg_value,
                    IRC_DROP_ENf, &temp);
                break;
            case DRV_RATE_CONFIG_PKT_MASK: 
                soc_RCM_DATA0r_field_set(unit, &reg_value,
                    IRC_PKT_MASK_B0f, &temp);
                break;
            default:
                MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
                return SOC_E_PARAM;
        }

        if ((rv = REG_WRITE_RCM_DATA0r(unit, &reg_value)) < 0) {
            goto rate_config_set_exit;
        }
        
        /* write to Rate Control Memory */
        /* Read Rate Control Memory register */
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto rate_config_set_exit;
        }

        temp = MEM_TABLE_WRITE;
        soc_RCM_CTLr_field_set(unit, &acc_ctrl,
            RCM_RWf, &temp);
            
        /* 1'b0 : Ingress Rate Control Memory access */
        temp = 0;
        soc_RCM_CTLr_field_set(unit, &acc_ctrl,
            IE_INDf, &temp);
        
        /* Start Write Process */
        temp = 1;
        soc_RCM_CTLr_field_set(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto rate_config_set_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto rate_config_set_exit;
            }

            soc_RCM_CTLr_field_get(unit, &acc_ctrl,
                RCM_RW_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto rate_config_set_exit;
        }
    }

 rate_config_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : drv_bcm53242_rate_config_get
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
drv_bcm53242_rate_config_get(int unit, uint32 port, uint32 config_type, uint32 *value)
{
    uint32  retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv = SOC_E_NONE;

    /* get bucket 0*/
    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers */
    /* Use RCM_DATA0 register set bucket 0 */
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto rate_config_get_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto rate_config_get_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto rate_config_get_exit;
    }

    temp = MEM_TABLE_READ;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RWf, &temp);
        
    /* 1'b0 : Ingress Rate Control Memory access */
    temp = 0;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        IE_INDf, &temp);
    
    /* Start Read Process */
    temp = 1;
    soc_RCM_CTLr_field_set(unit, &acc_ctrl,
        RCM_RW_STRTDNf, &temp);
    if ((rv = REG_WRITE_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto rate_config_get_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto rate_config_get_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto rate_config_get_exit;
    }
        
    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto rate_config_get_exit;
    }

    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: 
            soc_RCM_DATA0r_field_get(unit, &reg_value,
                REF_UNITf, &temp);
            *value = temp;
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE: 
            soc_RCM_DATA0r_field_get(unit, &reg_value,
                IRC_DROP_ENf, &temp);
            *value = temp;
            break;
        case DRV_RATE_CONFIG_PKT_MASK:
            soc_RCM_DATA0r_field_get(unit, &reg_value,
                IRC_PKT_MASK_B0f, &temp);
            *value = temp;
            break;
        default:
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
    }
    
    soc_cm_debug(DK_PORT, 
        "drv_rate_config_get: unit = %d, port = %d, type = %d, value = %d\n",
        unit, port, config_type, *value);

 rate_config_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : drv_bcm53242_rate_config_set
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
drv_bcm53242_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, 
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
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    _drv_bcm53242_port_irc_set(unit, port, kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    _drv_bcm53242_port_erc_set(unit, port, kbits_sec_max, burst_size));
            }                
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    _drv_bcm53242_port_queue_erc_set
                        (unit, port, queue_n, kbits_sec_min, burst_size));
            }                
            break;
        default:
            return SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_bcm53242_rate_config_get
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
drv_bcm53242_rate_get(int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *kbits_sec_min, uint32 *kbits_sec_max, uint32 *burst_size)
{
    int         rv = SOC_E_NONE;
    uint32 min_rate = 0; /* Dummy variable */

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            SOC_IF_ERROR_RETURN(
                _drv_bcm53242_port_irc_get(unit, port, kbits_sec_max, burst_size));
        break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            SOC_IF_ERROR_RETURN(
                _drv_bcm53242_port_erc_get(unit, port, kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            SOC_IF_ERROR_RETURN(
                _drv_bcm53242_port_queue_erc_get
                    (unit, port, queue_n, kbits_sec_min, burst_size));
            *kbits_sec_max = *burst_size;
            break;
        default:
            return SOC_E_PARAM;
    }
    soc_cm_debug(DK_PORT, 
        "drv_rate_get: unit = %d, port = %d, %s, kbits_sec_min = %dK, kbits_sec_max = %dK, \
        burst size = %dKB\n", unit, port, (direction - 1) ? "EGRESS" : "INGRESS", 
        min_rate, *kbits_sec_max, *burst_size);

    return rv;
}
