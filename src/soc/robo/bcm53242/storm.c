/*
 * $Id: storm.c 1.17.176.2 Broadcom SDK $
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

#define RATE_53242_MAX_REF_CNTS    0x3fff
#define RATE_53242_MAX_BUCKET_SIZE    0x7f

#define RATE_53242_REF_UNIT_64K    0
#define RATE_53242_REF_UNIT_1M    1

#define BCM53242_SUPPRESSION_UNKNOW_SA_MASK    0x40
#define BCM53242_SUPPRESSION_DLF_MASK          0x20
#define BCM53242_SUPPRESSION_MLF_MASK          0x10
#define BCM53242_SUPPRESSION_BROADCAST_MASK    0x08
#define BCM53242_SUPPRESSION_BPDU_MASK         0x04
#define BCM53242_SUPPRESSION_MULTICAST_MASK    0x02
#define BCM53242_SUPPRESSION_UNICAST_MASK      0x01

#define BCM53242_STORM_CONTROL_PKT_MASK \
    (DRV_STORM_CONTROL_BCAST | DRV_STORM_CONTROL_MCAST | \
     DRV_STORM_CONTROL_DLF | DRV_STORM_CONTROL_SALF | \
     DRV_STORM_CONTROL_RSV_MCAST | DRV_STORM_CONTROL_UCAST)

#define RATE_53242_METER_MAX 16383000 /* (RATE_53242_MAX_REF_CNTS * 1000) */
#define RATE_53242_METER_MIN 0
#define RATE_53242_BURST_MAX 8128 /* (1016 Kbyte * 8) */
#define RATE_53242_BURST_MIN 0

/*
 *  Function : _drv_bcm53242_storm_control_type_enable_set
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
_drv_bcm53242_storm_control_type_enable_set(int unit, uint32 port, 
    uint32 type, uint32 enable)
{
    uint32  reg_addr, reg_len, retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv= SOC_E_NONE;
    uint32 bucket_reg;
    uint32 bucket_field;

    /*
     * Bucket 0 is default used as per port rate control.
     * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
     * If no bucket number assigned, use bucket 1 as default bucket.
     */
    if (type & DRV_STORM_CONTROL_BUCKET_2) {
        bucket_reg = INDEX(RCM_DATA2r);
        bucket_field = INDEX(IRC_PKT_MASK_B2f);
    } else {
        bucket_reg = INDEX(RCM_DATA1r);
        bucket_field = INDEX(IRC_PKT_MASK_B1f);
    }

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers */
    zero_value = 0;

    /* Use RCM_DATA1 register set bucket x */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bucket_reg, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bucket_reg);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &zero_value, reg_len)) < 0) {
        goto storm_type_enable_set_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_set_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_set_exit;
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
        goto storm_type_enable_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_type_enable_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_type_enable_set_exit;
    }
        
    /* Read Rate Control Memory DATA x register */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bucket_reg, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bucket_reg);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        goto storm_type_enable_set_exit;
    }
    
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, bucket_reg, &reg_value, bucket_field, &temp));

    if (type & DRV_STORM_CONTROL_BCAST) {
        if (enable) {
            temp |= BCM53242_SUPPRESSION_BROADCAST_MASK;
        } else {
            temp &= ~BCM53242_SUPPRESSION_BROADCAST_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_MCAST) {
        if (enable) {
            temp |= BCM53242_SUPPRESSION_MULTICAST_MASK |
                BCM53242_SUPPRESSION_MLF_MASK;
        } else {
            temp &= ~(BCM53242_SUPPRESSION_MULTICAST_MASK |
                BCM53242_SUPPRESSION_MLF_MASK);
        }
    } 
    if (type & DRV_STORM_CONTROL_DLF) {
        if (enable) {
            temp |= (BCM53242_SUPPRESSION_DLF_MASK);
        } else {
            temp &= ~(BCM53242_SUPPRESSION_DLF_MASK);
        }
    }
    if (type & DRV_STORM_CONTROL_SALF) {
        if (enable) {
            temp |= BCM53242_SUPPRESSION_UNKNOW_SA_MASK;
        } else {
            temp &= ~BCM53242_SUPPRESSION_UNKNOW_SA_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_RSV_MCAST) {
        if (enable) {
            temp |= BCM53242_SUPPRESSION_BPDU_MASK;
        } else {
            temp &= ~BCM53242_SUPPRESSION_BPDU_MASK;
        }
    }
    if (type & DRV_STORM_CONTROL_UCAST) {
        if (enable) {
            temp |= BCM53242_SUPPRESSION_UNICAST_MASK;
        } else {
            temp &= ~BCM53242_SUPPRESSION_UNICAST_MASK;
        }
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, bucket_reg, &reg_value, bucket_field, &temp));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        goto storm_type_enable_set_exit;
    }
    
    /* write to Rate Control Memory */
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_set_exit;
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
        goto storm_type_enable_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_type_enable_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_type_enable_set_exit;
    }
    
 storm_type_enable_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : _drv_bcm53242_storm_control_type_enable_get
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
_drv_bcm53242_storm_control_type_enable_get(int unit, uint32 port, uint32 *type)
{
    uint32  reg_addr, reg_len, retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv= SOC_E_NONE;
    uint32 bucket_reg;
    uint32 bucket_field;

    /*
     * Bucket 0 is default used as per port rate control.
     * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
     * If no bucket number assigned, use bucket 1 as default bucket.
     */
    if (*type & DRV_STORM_CONTROL_BUCKET_2) {
        bucket_reg = INDEX(RCM_DATA2r);
        bucket_field = INDEX(IRC_PKT_MASK_B2f);
    } else {
        bucket_reg = INDEX(RCM_DATA1r);
        bucket_field = INDEX(IRC_PKT_MASK_B1f);
    }

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers */
    zero_value = 0;

    /* Use RCM_DATA1 register set bucket x */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bucket_reg, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bucket_reg);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &zero_value, reg_len)) < 0) {
        goto storm_type_enable_get_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_get_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_type_enable_get_exit;
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
        goto storm_type_enable_get_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_type_enable_get_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_type_enable_get_exit;
    }
        
    /* Read Rate Control Memory DATA x register */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bucket_reg, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bucket_reg);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        goto storm_type_enable_get_exit;
    }

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, bucket_reg, &reg_value, bucket_field, &temp));
    
    if (temp & BCM53242_SUPPRESSION_BROADCAST_MASK) {
        *type |= DRV_STORM_CONTROL_BCAST;
    }
    
    if (temp & BCM53242_SUPPRESSION_MULTICAST_MASK) {
        *type |= DRV_STORM_CONTROL_MCAST;
    }
    
    if (temp & BCM53242_SUPPRESSION_DLF_MASK){
        *type |= DRV_STORM_CONTROL_DLF;
    }

    if (temp & BCM53242_SUPPRESSION_UNKNOW_SA_MASK) {
        *type |= DRV_STORM_CONTROL_SALF;
    }

    if (temp & BCM53242_SUPPRESSION_BPDU_MASK) {
        *type |= DRV_STORM_CONTROL_RSV_MCAST;
    }

    if (temp & BCM53242_SUPPRESSION_UNICAST_MASK) {
        *type |= DRV_STORM_CONTROL_UCAST;
    }

 storm_type_enable_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : drv_bcm53242_storm_control_enable_set
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
drv_bcm53242_storm_control_enable_set(int unit, uint32 port, uint8 enable)
{
    uint32 retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int rv = SOC_E_NONE;

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));

    /* Clear Rate Control Memory Data access registers */
    /* Use RCM_DATA0 register set bucket 0 */
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto storm_enable_set_exit;
    }
    
    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_set_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_set_exit;
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
        goto storm_enable_set_exit;
    }
    
    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_enable_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_enable_set_exit;
    }
    
    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto storm_enable_set_exit;
    }

    /* Enable drop_enable bit */
    temp = 1;
    soc_RCM_DATA0r_field_set(unit, &reg_value,
        IRC_DROP_ENf, &temp);
    
    /* Enable Ingress rate control bit */
    soc_RCM_DATA0r_field_get(unit, &reg_value,
        IE_RC_ENf, &temp);
    if (!temp) {
        temp = 1;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            IE_RC_ENf, &temp);
    }
    
    /* 
     * When global ingress rate control bit enabled.
     * The rate limit and bucket size can't be 0.
     * Set to maxmimum value if no value in the fields.
     */
    soc_RCM_DATA0r_field_get(unit, &reg_value,
        REF_UNITf, &temp);
    if (!temp) {
        temp = RATE_53242_REF_UNIT_1M;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_UNITf, &temp);
    }

    soc_RCM_DATA0r_field_get(unit, &reg_value,
        REF_CNTSf, &temp);
    if (!temp) {
        temp = RATE_53242_MAX_REF_CNTS;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            REF_CNTSf, &temp);
    }

    soc_RCM_DATA0r_field_get(unit, &reg_value,
        BUCKET_SIZEf, &temp);
    if (!temp) {
        temp = RATE_53242_MAX_BUCKET_SIZE;
        soc_RCM_DATA0r_field_set(unit, &reg_value,
            BUCKET_SIZEf, &temp);
    }
    
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto storm_enable_set_exit;
    }
    
    /* write to Rate Control Memory */
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_set_exit;
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
        goto storm_enable_set_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_enable_set_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_enable_set_exit;
    }

storm_enable_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    
    return rv;
}

/*
 *  Function : drv_bcm53242_storm_control_enable_get
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
drv_bcm53242_storm_control_enable_get(int unit, uint32 port, uint8 *enable)
{
    uint32  retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int     rv= SOC_E_NONE;

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    /* Clear Rate Control Memory Data access registers */
    /* Use RCM_DATA0 register set bucket 0 */
    zero_value = 0;
    if ((rv = REG_WRITE_RCM_DATA0r(unit, &zero_value)) < 0) {
        goto storm_enable_get_exit;
    }

    /* Set Rate Control Memory Port register */
    soc_RCM_PORTr_field_set(unit, &acc_ctrl,
        RCM_PORTf, &port);
    if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_get_exit;
    }
    
    /* Read Rate Control Memory register */
    if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
        goto storm_enable_get_exit;
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
        goto storm_enable_get_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_enable_get_exit;
        }
        soc_RCM_CTLr_field_get(unit, &acc_ctrl,
            RCM_RW_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto storm_enable_get_exit;
    }
        
    /* Read Rate Control Memory DATA 0 register */
    if ((rv = REG_READ_RCM_DATA0r(unit, &reg_value)) < 0) {
        goto storm_enable_get_exit;
    }

    soc_RCM_DATA0r_field_get(unit, &reg_value,
        IE_RC_ENf, &temp);
    *enable = temp;

    soc_cm_debug(DK_PORT, 
    "drv_storm_control_enable_get: unit = %d, port= %d, %sable\n",
        unit, port, *enable ? "en" : "dis");
    
 storm_enable_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : drv_bcm53242_storm_control_set
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
drv_bcm53242_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size)
{
    uint32      retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    int         rv= SOC_E_NONE;
    uint32      port;
    uint32      disable_type = 0, burst_kbyte = 0;
    uint32 ref_u;
    uint32 quotient_64k, remainder_64k, quotient_1m, remainder_1m;
    uint32 bucket_reg = 0;

    soc_cm_debug(DK_PORT, 
    "drv_storm_control_set: unit = %d, bmp= %x, type = 0x%x, limit = %dK\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), type, limit);

    if (((int)limit > RATE_53242_METER_MAX) ||
        ((int)limit < RATE_53242_METER_MIN)) {
        soc_cm_debug(DK_ERR, 
            "drv_bcm53242_storm_control_set : rate unsupported. \n");
        return SOC_E_PARAM;
    }

    if (((int)burst_size > RATE_53242_BURST_MAX) ||
        ((int)burst_size < RATE_53242_BURST_MIN)) {
        soc_cm_debug(DK_ERR, 
            "drv_bcm53242_storm_control_set : burst size unsupported. \n");
        return SOC_E_PARAM;
    }

    PBMP_ITER(bmp, port) {
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
    
        if (limit == 0) { /* Disable storm suppression type */
            SOC_IF_ERROR_RETURN(
                _drv_bcm53242_storm_control_type_enable_set(unit, port, type, FALSE));
            /* need diable ingress rate control ? */
        } else {
            disable_type = BCM53242_STORM_CONTROL_PKT_MASK;
            disable_type ^= type;
            /* set storm suppression type */
            SOC_IF_ERROR_RETURN(
                _drv_bcm53242_storm_control_type_enable_set(unit, port, type, TRUE));
            if (disable_type){
                SOC_IF_ERROR_RETURN(
                    _drv_bcm53242_storm_control_type_enable_set(
                                    unit, port, 
                                    disable_type, FALSE));
            }

            /*
             * Bucket 0 is default used as per port rate control.
             * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
             * If no bucket number assigned, use bucket 1 as default bucket.
             */
            if (type & DRV_STORM_CONTROL_BUCKET_2) {
                bucket_reg = RCM_DATA2r;
            } else {
                bucket_reg = RCM_DATA1r;
            }

            /* set bucket 1 refresh count */
            /* process write action */
            MEM_LOCK(unit, INDEX(GEN_MEMORYm));
            
            /* Clear Rate Control Memory Data access registers */
            zero_value = 0;

            /* Use RCM_DATA1 register set bucket x */
            /*
             * Bucket 0 is default used as per port rate control.
             * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
             * If no bucket number assigned, use bucket 1 as default bucket.
             */
            if (type & DRV_STORM_CONTROL_BUCKET_2) {
                if ((rv = REG_WRITE_RCM_DATA2r(unit, &zero_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            } else {
                if ((rv = REG_WRITE_RCM_DATA1r(unit, &zero_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            }

            /* Set Rate Control Memory Port register */
            soc_RCM_PORTr_field_set(unit, &acc_ctrl,
                RCM_PORTf, &port);
            if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_set_exit;
            }
            
            /* Read Rate Control Memory register */
            if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_set_exit;
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
                goto storm_contol_set_exit;
            }

            /* wait for complete */
            for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                    goto storm_contol_set_exit;
                }
                soc_RCM_CTLr_field_get(unit, &acc_ctrl,
                    RCM_RW_STRTDNf, &temp);
                if (!temp) {
                    break;
                }
            }
            if (retry >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto storm_contol_set_exit;
            }
                
            /* Read Rate Control Memory DATA x register */
            if (type & DRV_STORM_CONTROL_BUCKET_2) {
                if ((rv = REG_READ_RCM_DATA2r(unit, &reg_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            } else {
                if ((rv = REG_READ_RCM_DATA1r(unit, &reg_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            }

            /* burst size : set maximum burst size for storm contol at bucket x*/
            /* need to set burst size if open ingress pkt mask for each bucket (0 ~2) */
            if (burst_size) {
                /* burst size */
                burst_kbyte = (burst_size / 8);
                temp = burst_kbyte / 8;
            } else {
                temp = RATE_53242_MAX_BUCKET_SIZE;
            }
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

            if (type & DRV_STORM_CONTROL_BUCKET_2) {
                if ((rv = REG_WRITE_RCM_DATA2r(unit, &reg_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            } else {
                if ((rv = REG_WRITE_RCM_DATA1r(unit, &reg_value)) < 0) {
                    goto storm_contol_set_exit;
                }
            }

            /* write to Rate Control Memory */
            /* Read Rate Control Memory register */
            if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_set_exit;
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
                goto storm_contol_set_exit;
            }

            /* wait for complete */
            for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                    goto storm_contol_set_exit;
                }
                soc_RCM_CTLr_field_get(unit, &acc_ctrl,
                    RCM_RW_STRTDNf, &temp);
                if (!temp) {
                    break;
                }
            }
            if (retry >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto storm_contol_set_exit;
            }
            
 storm_contol_set_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
        }
    }

    return rv;
}

/*
 *  Function : drv_bcm53242_storm_control_get
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
drv_bcm53242_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size)
{
    uint32      retry, temp;
    uint32 zero_value, reg_value, acc_ctrl = 0;
    uint32 ref_u = 0;
    int         rv= SOC_E_NONE;
    uint32 loc_type = 0;

     SOC_IF_ERROR_RETURN(
        _drv_bcm53242_storm_control_type_enable_get(unit, port, type));
    loc_type = *type & BCM53242_STORM_CONTROL_PKT_MASK;
    if (!loc_type) {
        *limit = 0;
    } else {

        /* process write action */
        MEM_LOCK(unit, INDEX(GEN_MEMORYm));
        /* Clear Rate Control Memory Data access registers */
        zero_value = 0;

        /* Use RCM_DATA1 register set bucket x */
        /*
         * Bucket 0 is default used as per port rate control.
         * Bucket 1 and 2 can be selected by DRV_STORM_CONTROL_BUCKET_x.
         * If no bucket number assigned, use bucket 1 as default bucket.
         */
        if (*type & DRV_STORM_CONTROL_BUCKET_2) {
            if ((rv = REG_WRITE_RCM_DATA2r(unit, &zero_value)) < 0) {
                goto storm_contol_get_exit;
            }
        } else {
            if ((rv = REG_WRITE_RCM_DATA1r(unit, &zero_value)) < 0) {
                goto storm_contol_get_exit;
            }
        }
        
        /* Set Rate Control Memory Port register */
        soc_RCM_PORTr_field_set(unit, &acc_ctrl,
            RCM_PORTf, &port);
        if ((rv = REG_WRITE_RCM_PORTr(unit, &acc_ctrl)) < 0) {
            goto storm_contol_get_exit;
        }
        
        /* Read Rate Control Memory register */
        if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
            goto storm_contol_get_exit;
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
            goto storm_contol_get_exit;
        }
        
        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_RCM_CTLr(unit, &acc_ctrl)) < 0) {
                goto storm_contol_get_exit;
            }
            soc_RCM_CTLr_field_get(unit, &acc_ctrl,
                RCM_RW_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto storm_contol_get_exit;
        }
        
        /* Read Rate Control Memory DATA x register */
        if (*type & DRV_STORM_CONTROL_BUCKET_2) {
            if ((rv = REG_READ_RCM_DATA2r(unit, &reg_value)) < 0) {
                goto storm_contol_get_exit;
            }
        } else {
            if ((rv = REG_READ_RCM_DATA1r(unit, &reg_value)) < 0) {
                goto storm_contol_get_exit;
            }
        }

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
        soc_RCM_DATA1r_field_get(unit, &reg_value,
            BUCKET_SIZEf, &temp);
        
        *burst_size = temp * 8 * 8;
        
 storm_contol_get_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    
    }
    soc_cm_debug(DK_PORT, 
    "drv_storm_control_get: unit = %d, port= %d, type = 0x%x, limit = %dK\n",
        unit, port, *type, *limit);
    
    return rv;
}
