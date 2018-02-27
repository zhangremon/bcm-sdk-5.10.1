/*
 * $Id: cosq.c 1.17.6.2 Broadcom SDK $
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
#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_bcm53242_queue_mode_set
 *
 *  Purpose :
 *      Set the queue mode of selected port type.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      bmp   :   port bitmap.
 *      mode  :   queue mode.
 *
 *  Return :
 *      SOC_E_UNAVAIL.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 mode)
{
    int rv = SOC_E_NONE;
    uint32  temp;
    uint64  reg_value64;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mode_set: unit %d, bmp = %x, queue mode = %d\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), mode);
    if ((rv = REG_READ_QOS_CTLr(unit, (uint32 *)&reg_value64)) < 0) {
        return rv;
    }

     switch (mode) {
            case DRV_QUEUE_MODE_WRR:
                temp = 0;
                break;
            case DRV_QUEUE_MODE_1STRICT_3WRR:
                temp = 1;
                break;
            case DRV_QUEUE_MODE_2STRICT_2WRR:
                temp = 2;
                break;
            case DRV_QUEUE_MODE_STRICT:
                temp = 3;
                break;
            default:
                return SOC_E_UNAVAIL;
    }
    soc_QOS_CTLr_field_set(unit, (uint32 *)&reg_value64,
        SCHEDULE_SELECTf, &temp);

    if ((rv = REG_WRITE_QOS_CTLr(unit, (uint32 *)&reg_value64)) < 0) {
        return rv;
    }
    return rv;
}

/*
 *  Function : drv_bcm53242_queue_mode_get
 *
 *  Purpose :
 *      Get the queue mode of selected port type.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      mode  :   queue mode.
 *
 *  Return :
 *      SOC_E_UNAVAIL.
 *
 *  Note :
 *      
 *
 */  
int 
drv_bcm53242_queue_mode_get(int unit, uint32 port, uint32 *mode)
{
    int rv = SOC_E_NONE;
    uint32  temp;
    uint64  reg_value64;
    
    /* check port number */
    if (port > (SOC_MAX_NUM_PORTS - 1)) {
        return SOC_E_PARAM;
    }
    if ((rv = REG_READ_QOS_CTLr(unit, (uint32 *)&reg_value64)) < 0) {
        return rv;
    }
    soc_QOS_CTLr_field_get(unit, (uint32 *)&reg_value64,
        SCHEDULE_SELECTf, &temp);

    switch (temp) {
        case 0:
            *mode = DRV_QUEUE_MODE_WRR;
            break;
        case 1:
            *mode = DRV_QUEUE_MODE_1STRICT_3WRR;
            break;
        case 2:
            *mode = DRV_QUEUE_MODE_2STRICT_2WRR;
            break;
        case 3:
            *mode = DRV_QUEUE_MODE_STRICT;
            break;
        default:
            return SOC_E_INTERNAL;
    }
        
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mode_get: unit %d, port = %d, queue mode = %d\n",
        unit, port, *mode);    
    return rv;
}

/*
 *  Function : drv_bcm53242_queue_count_set
 *
 *  Purpose :
 *      Set the number of the queeus.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port_type   :   port type.
 *      count  :   number of queues.
 *
 *  Return :
 *      SOC_E_NONE : success.
 *      SOC_E_PARAM : parameters error.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_count_set(int unit, uint32 port_type, uint8 count)
{
    uint32  num_q, temp;
    uint32      max_numq;
    int     cos, prio, ratio, remain;
    uint64  reg_value64;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_count_set: unit %d, port type = %d, queue count = %d\n",
        unit, port_type, count);
    
    max_numq = NUM_COS(unit);   
    if ((count > max_numq) || (count < 1)) {
        return SOC_E_PARAM;
    }
    /* Reserved for 2, 3 queues */
    if ((count == 2) || (count == 3)) {
        return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(
        REG_READ_QOS_CTLr(unit, (uint32 *)&reg_value64));
    /* enable CPU_Control_Enable bit */
    temp = 1;
    soc_QOS_CTLr_field_set(unit, (uint32 *)&reg_value64,
        CPU_CTRL_ENf, &temp);
    /* set queue number = 4 or 0(disable) */
    num_q = count - 1;
    soc_QOS_CTLr_field_set(unit, (uint32 *)&reg_value64,
        QOS_ENf, &num_q);
    /* write register */
    SOC_IF_ERROR_RETURN(
        REG_WRITE_QOS_CTLr(unit, (uint32 *)&reg_value64));

    /* Map the eight 802.1 priority levels to the active cosqs */
    ratio = 8 / count;
    remain = 8 % count;
    cos = 0;
    for (prio = 0; prio < 8; prio++) {
          SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->queue_prio_set)
             (unit, -1, prio, cos));
       if ((prio + 1) == (((cos + 1) * ratio) +
                                ((remain < (count - cos)) ? 0 :
                                 (remain - (count- cos) + 1)))) {
           cos++;
       }
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm53242_queue_count_get
 *
 *  Purpose :
 *      Get the number of the queeus.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port_type   :   port type.
 *      count  :   number of queues.
 *
 *  Return :
 *      SOC_E_NONE : success.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_count_get(int unit, uint32 port_type, uint8 *count)
{
    uint32  num_q = 0;
    uint64  reg_value64;

    SOC_IF_ERROR_RETURN(
        REG_READ_QOS_CTLr(unit, (uint32 *)&reg_value64));
    soc_QOS_CTLr_field_get(unit, (uint32 *)&reg_value64,
        QOS_ENf, &num_q);
    *count = num_q + 1;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_count_get: unit %d, port type = %d, queue count = %d\n",
        unit, port_type, *count);
    
    return SOC_E_NONE;
}

/* set WRR weight */
/*
 *  Function : drv_bcm53242_queue_WRR_weight_set
 *
 *  Purpose :
 *      Set the weight value to the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port_type   :   port type.
 *      queue  :   queue number.
 *      weight  :   weight value.
 *
 *  Return :
 *      SOC_E_NONE : success.
 *      SOC_E_PARAM : parameters error.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_WRR_weight_set(int unit, 
    uint32 port_type, uint8 queue, uint32 weight)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr, reg_value, temp;
    int     reg_len;
    uint32  reg_index, fld_index;
    uint32  max_weight = 0;

    soc_cm_debug(DK_VERBOSE, 
        "drv_5324_queue_WRR_weight_set: unit %d, port type = %d, \
        queue = %d, weight = %d\n",
        unit, port_type, queue, weight);
    
    if ((rv = (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE, 
            &max_weight)) < 0) {
        return rv;
    }
    if ((weight > max_weight) || (weight < 1)) {
        return SOC_E_PARAM;
    }
    
    switch (queue) {
        case 0:
            reg_index = INDEX(FCON_Q0_TXDSC_CTRL_3r);
            fld_index = INDEX(Q0_QUOTA_SIZEf);
            break;
        case 1:
            reg_index = INDEX(FCON_Q1_TXDSC_CTRL_3r);
            fld_index = INDEX(Q1_QUOTA_SIZEf);
            break;
        case 2:
            reg_index = INDEX(FCON_Q2_TXDSC_CTRL_3r);
            fld_index = INDEX(Q2_QUOTA_SIZEf);
            break;
        case 3:
            reg_index = INDEX(FCON_Q3_TXDSC_CTRL_3r);
            fld_index = INDEX(Q3_QUOTA_SIZEf);
            break;
        default:
            return SOC_E_PARAM;
            break;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
    temp = weight;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, &reg_value, fld_index, &temp));
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &reg_value, reg_len));
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm53242_queue_WRR_weight_get
 *
 *  Purpose :
 *      Get the weight value to the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port_type   :   port type.
 *      queue  :   queue number.
 *      weight  :   weight value.
 *
 *  Return :
 *      SOC_E_NONE : success.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint8 queue, uint32 *weight)
{
    uint32  reg_addr, reg_value, temp;
    int     reg_len;
    uint32  reg_index, fld_index;
    
    switch (queue) {
        case 0:
            reg_index = INDEX(FCON_Q0_TXDSC_CTRL_3r);
            fld_index = INDEX(Q0_QUOTA_SIZEf);
            break;
        case 1:
            reg_index = INDEX(FCON_Q1_TXDSC_CTRL_3r);
            fld_index = INDEX(Q1_QUOTA_SIZEf);
            break;
        case 2:
            reg_index = INDEX(FCON_Q2_TXDSC_CTRL_3r);
            fld_index = INDEX(Q2_QUOTA_SIZEf);
            break;
        case 3:
            reg_index = INDEX(FCON_Q3_TXDSC_CTRL_3r);
            fld_index = INDEX(Q3_QUOTA_SIZEf);
            break;
        default:
            return SOC_E_PARAM;
            break;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, reg_index, &reg_value, fld_index, &temp));
    *weight = temp;
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_WRR_weight_get: unit %d, port type = %d, \
        queue = %d, weight = %d\n",
        unit, port_type, queue, *weight);
    
    return SOC_E_NONE;
}

/* config output queue mapping */
/*
 *  Function : drv_bcm53242_queue_prio_set
 *
 *  Purpose :
 *      Set the priority value of the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prio   :   priority value.
 *      queue_n  :   queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n)
{
    uint32  reg_value, temp;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_prio_set: unit %d, port = %d, priority = %d, queue = %d\n",
        unit, port, prio, queue_n);

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(
        REG_READ_QOS_TCI_THr(unit, &reg_value));
    temp = queue_n;
    switch ( prio ) {        
    case 0:
        soc_QOS_TCI_THr_field_set(unit, &reg_value,
            PRITAG_000f, &temp);
        break;
    case 1:
        soc_QOS_TCI_THr_field_set(unit, &reg_value,
            PRITAG_001f, &temp);
        break;
    case 2:         
        soc_QOS_TCI_THr_field_set(unit, &reg_value,
            PRITAG_010f, &temp);
        break;
    case 3:
        soc_QOS_TCI_THr_field_set(unit, &reg_value,
            PRITAG_011f, &temp);
        break;
    case 4:
        soc_QOS_TCI_THr_field_set(unit, &reg_value,
            PRITAG_100f, &temp);
        break;
    case 5:
        soc_QOS_TCI_THr_field_set(unit, &reg_value,
            PRITAG_101f, &temp);
        break;
    case 6:         
        soc_QOS_TCI_THr_field_set(unit, &reg_value,
            PRITAG_110f, &temp);
        break;
    case 7:
        soc_QOS_TCI_THr_field_set(unit, &reg_value,
            PRITAG_111f, &temp);
        break;
    default:
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(
        REG_WRITE_QOS_TCI_THr(unit, &reg_value));

    return SOC_E_NONE;

}

/*
 *  Function : drv_bcm53242_queue_prio_get
 *
 *  Purpose :
 *      Get the priority value of the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prio   :   priority value.
 *      queue_n  :   queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n)
{
    uint32  reg_value;
    uint32  temp = 0;

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(
        REG_READ_QOS_TCI_THr(unit, &reg_value));
    switch ( prio ) {        
    case 0:
        soc_QOS_TCI_THr_field_get(unit, &reg_value,
            PRITAG_000f, &temp);
        break;
    case 1:
        soc_QOS_TCI_THr_field_get(unit, &reg_value,
            PRITAG_001f, &temp);
        break;
    case 2:         
        soc_QOS_TCI_THr_field_get(unit, &reg_value,
            PRITAG_010f, &temp);
        break;
    case 3:
        soc_QOS_TCI_THr_field_get(unit, &reg_value,
            PRITAG_011f, &temp);
        break;
    case 4:
        soc_QOS_TCI_THr_field_get(unit, &reg_value,
            PRITAG_100f, &temp);
        break;
    case 5:
        soc_QOS_TCI_THr_field_get(unit, &reg_value,
            PRITAG_101f, &temp);
        break;
    case 6:         
        soc_QOS_TCI_THr_field_get(unit, &reg_value,
            PRITAG_110f, &temp);
        break;
    case 7:
        soc_QOS_TCI_THr_field_get(unit, &reg_value,
            PRITAG_111f, &temp);
        break;
    default:
        return SOC_E_PARAM;
    }
    *queue_n = temp;
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_prio_get: unit %d, port = %d, priority = %d, queue = %d\n",
        unit, port, prio, *queue_n);
    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm53242_queue_tos_set
 *
 *  Purpose :
 *      Set the precedence value of the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      precedence   :   precedence value.
 *      queue_n  :   queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_tos_set(int unit, uint8 precedence, uint8 queue_n)
{
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_bcm53242_queue_tos_get
 *
 *  Purpose :
 *      Get the precedence value of the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      precedence   :   precedence value.
 *      queue_n  :   queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_tos_get(int unit, uint8 precedence, uint8 *queue_n)
{
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_bcm53242_queue_dfsv_set
 *
 *  Purpose :
 *      Set the DSCP priority value of the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      code_point   :   DSCP priority value.
 *      queue_n  :   queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n)
{
    uint32  reg_addr, temp;
    int     reg_len, reg_index;
    uint64  reg_value;


    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_dfsv_set: unit %d, dscp value = %d, queue = %d\n",
        unit, code_point, queue_n);
    if (code_point  > 31) {
        reg_index = INDEX(QOS_DIFF_DSCP2r);
        code_point -= 32;
    } else {
        reg_index = INDEX(QOS_DIFF_DSCP1r);
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value,reg_len));
    temp = queue_n;
    switch ( code_point ) {        
    case 0:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_000000f, &temp);
        break;
    case 1: 
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_000001f, &temp);
        break;
    case 2:         
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_000010f, &temp);
        break;
    case 3:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_000011f, &temp);
        break;
    case 4:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_000100f, &temp);
        break;
    case 5:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_000101f, &temp);
        break;
    case 6:         
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_000110f, &temp);
        break;
    case 7:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_000111f, &temp);
        break;
    case 8:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_001000f, &temp);
        break;
    case 9:   
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_001001f, &temp);
        break;
    case 10:         
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_001010f, &temp);
        break;
    case 11:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_001011f, &temp);
        break;
    case 12:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_001100f, &temp);
        break;
    case 13:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_001101f, &temp);
        break;
    case 14:         
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_001110f, &temp);
        break;
    case 15:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_001111f, &temp);
        break;
    case 16:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_010000f, &temp);
        break;
    case 17:         
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_010001f, &temp);
        break;
    case 18:         
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_010010f, &temp);
        break;
    case 19:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_010011f, &temp);
        break;
    case 20:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_010100f, &temp);
        break;
    case 21:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_010101f, &temp);
        break;
    case 22:        
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_010110f, &temp);
        break;
    case 23:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_010111f, &temp);
        break;
    case 24:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_011000f, &temp);
        break;
    case 25:         
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_011001f, &temp);
        break;
    case 26:         
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_011010f, &temp);
        break;
    case 27:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_011011f, &temp);
        break;
    case 28:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_011100f, &temp);
        break;
    case 29:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_011101f, &temp);
        break;
    case 30:         
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_011110f, &temp);
        break;
    case 31:
        soc_QOS_DIFF_DSCP1r_field_set(unit, (uint32 *)&reg_value,
            PRI_DSCP_011111f, &temp);
        break;
    default:
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len));

    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm53242_queue_dfsv_get
 *
 *  Purpose :
 *      Get the DSCP priority value of the specific queue.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      code_point   :   DSCP priority value.
 *      queue_n  :   queue number.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_dfsv_get(int unit, uint8 code_point, uint8 *queue_n)
{
    uint32  reg_addr, temp = 0;
    int     reg_len, reg_index;
    uint64      reg_value;


    sal_memset(&reg_value, 0, 8);
    if (code_point  > 31) {
        reg_index = INDEX(QOS_DIFF_DSCP2r);
        code_point -= 32;
    } else {
        reg_index = INDEX(QOS_DIFF_DSCP1r);
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value,reg_len));
    switch ( code_point ) {        
    case 0:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_000000f, &temp);
        break;
    case 1:       
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_000000f, &temp);
        break;
    case 2:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_000010f, &temp);
        break;
    case 3:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_000011f, &temp);
        break;
    case 4:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_000100f, &temp);
        break;
    case 5:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_000101f, &temp);
        break;
    case 6:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_000110f, &temp);
        break;
    case 7:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_000111f, &temp);
        break;
    case 8:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_001000f, &temp);
        break;
    case 9:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_001001f, &temp);
        break;
    case 10:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_001010f, &temp);
        break;
    case 11:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_001011f, &temp);
        break;
    case 12:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_001100f, &temp);
        break;
    case 13:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_001101f, &temp);
        break;
    case 14:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_001110f, &temp);
        break;
    case 15:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_001111f, &temp);
        break;
    case 16:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_010000f, &temp);
        break;
    case 17:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_010001f, &temp);
        break;
    case 18:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_010010f, &temp);
        break;
    case 19:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_010011f, &temp);
        break;
    case 20:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_010100f, &temp);
        break;
    case 21:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_010101f, &temp);
        break;
    case 22:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_010110f, &temp);
        break;
    case 23:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_010111f, &temp);
        break;
    case 24:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_011000f, &temp);
        break;
    case 25:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_011001f, &temp);
        break;
    case 26:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_011010f, &temp);
        break;
    case 27:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_011011f, &temp);
        break;
    case 28:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_011100f, &temp);
        break;
    case 29:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_011101f, &temp);
        break;
    case 30:         
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_011110f, &temp);
        break;
    case 31:
        soc_QOS_DIFF_DSCP1r_field_get(unit, (uint32 *)&reg_value,
            PRI_DSCP_011111f, &temp);
        break;
    default:
        return SOC_E_PARAM;
    }
    *queue_n = temp;
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_dfsv_get: unit %d, dscp value = %d, queue = %d\n",
        unit, code_point, *queue_n);
    return SOC_E_NONE;
}

/* enable/disable selected mapping - prio/diffserv/tos */
/*
 *  Function : drv_bcm53242_queue_mapping_state_set
 *
 *  Purpose :
 *      Set the state to the specific queue mapping type.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      mapping_type   :   queue mapping type (prio/tos/diffserv).
 *      state  :   The state of the selected mapping type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state)
{
    uint32  temp = 0, val_32;
    uint64  reg_value64, pbmp_value, temp64;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mapping_type_set: unit %d, bmp = %x %x, type = %d,  %sable\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1),
        mapping_type, state ? "en" : "dis");
    COMPILER_64_ZERO(temp64);
    switch (mapping_type) {
    case DRV_QUEUE_MAP_NONE:
        /* disable QOS_IP_EN register */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr(
            unit, (uint32 *)&reg_value64));
        if (SOC_INFO(unit).port_num > 32) {
            soc_QOS_1P_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp64);
            soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
            COMPILER_64_NOT(pbmp_value);
            COMPILER_64_AND(temp64, pbmp_value);
            soc_QOS_1P_ENr_field_set(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp64);
        } else {
            soc_QOS_1P_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp);
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            temp &= ~val_32;
            soc_QOS_1P_ENr_field_set(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp);
        }
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr(
            unit, (uint32 *)&reg_value64));
        /* disable QOS_TOS_DIF_EN register */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_TOS_DIF_ENr(
            unit, (uint32 *)&reg_value64));
        if (SOC_INFO(unit).port_num > 32) {
            soc_QOS_TOS_DIF_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64);
            soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
            COMPILER_64_NOT(pbmp_value);
            COMPILER_64_AND(temp64, pbmp_value);
            soc_QOS_TOS_DIF_ENr_field_set(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64);
        } else {
            soc_QOS_TOS_DIF_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp);
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            temp &= ~val_32;
            soc_QOS_TOS_DIF_ENr_field_set(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp);
        }
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_TOS_DIF_ENr(
            unit, (uint32 *)&reg_value64));
        break;
    case DRV_QUEUE_MAP_PRIO:
        /* enable/disable QOS_1P_ENr */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr(
            unit, (uint32 *)&reg_value64));
        if (SOC_INFO(unit).port_num > 32) {
            soc_QOS_1P_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp64);
            soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
            if (state) {
                COMPILER_64_OR(temp64, pbmp_value);
            } else {
                COMPILER_64_NOT(pbmp_value);
                COMPILER_64_AND(temp64, pbmp_value);
            }
            soc_QOS_1P_ENr_field_set(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp64);
        } else {
            soc_QOS_1P_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp);
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            soc_QOS_1P_ENr_field_set(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp);
        }
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr(
            unit, (uint32 *)&reg_value64));

        /* don't need to disable QOS_TOS_DIF_ENr : 
          * the setting is separated and chosen by QoS priority decision flowchart
          */
        break;
    case DRV_QUEUE_MAP_DFSV:
        /* No need to switch TOS/DFSV, BCM53242 removes TOS function */

        /* enable / diaable QOS_TOS_DIF_ENr */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_TOS_DIF_ENr(
            unit, (uint32 *)&reg_value64));
        if (SOC_INFO(unit).port_num > 32) {
            soc_QOS_TOS_DIF_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64);
            soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
            if (state) {
                COMPILER_64_OR(temp64, pbmp_value);
            } else {
                COMPILER_64_NOT(pbmp_value);
                COMPILER_64_AND(temp64, pbmp_value);
            }
            soc_QOS_TOS_DIF_ENr_field_set(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64);
        } else {
            soc_QOS_TOS_DIF_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp);
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            soc_QOS_TOS_DIF_ENr_field_set(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp);

        }
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_TOS_DIF_ENr(
            unit, (uint32 *)&reg_value64));
            
        /* don't need to disable QOS_1P_ENr : 
          * the setting is separated and chosen by QoS priority decision flowchart
          */
        break;
    case DRV_QUEUE_MAP_PORT:
        /* bcm53242/ 53262 have no force port based priority solution :
         *  - the default port priority can be retrieved only if all other
         *      TC decision process been disabled. (like protocol based, mac 
         *      based, vlan based,.. etc.)
         */
    case DRV_QUEUE_MAP_TOS:
    case DRV_QUEUE_MAP_MAC:
    case DRV_QUEUE_MAP_HYBRID:
        return SOC_E_UNAVAIL;
    default :
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm53242_queue_mapping_state_get
 *
 *  Purpose :
 *      Get the state to the specific queue mapping type.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port    :   port number.
 *      mapping_type   :   queue mapping type (prio/tos/diffserv).
 *      state  :   The state of the selected mapping type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state)
{
    uint32  temp = 0;
    uint64  reg_value64, temp64;
    soc_pbmp_t pbmp;

    COMPILER_64_ZERO(temp64);
    switch (mapping_type)
    {
    case DRV_QUEUE_MAP_NONE:
        return SOC_E_PARAM;
    case DRV_QUEUE_MAP_PRIO:
        SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr(
            unit, (uint32 *)&reg_value64));
        if (SOC_INFO(unit).port_num > 32) {
            soc_QOS_1P_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp64);
            SOC_PBMP_CLEAR(pbmp);
            soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
        } else {
            soc_QOS_1P_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_1P_ENf, (uint32 *)&temp);
            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
        }
        if (SOC_PBMP_MEMBER(pbmp, port)) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
    case DRV_QUEUE_MAP_DFSV:
        SOC_IF_ERROR_RETURN(REG_READ_QOS_TOS_DIF_ENr(
            unit, (uint32 *)&reg_value64));
        if (SOC_INFO(unit).port_num > 32) {
            soc_QOS_TOS_DIF_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64);
            SOC_PBMP_CLEAR(pbmp);
            soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
        } else {
            soc_QOS_TOS_DIF_ENr_field_get(unit, (uint32 *)&reg_value64,
                QOS_TOS_DIFF_ENf, (uint32 *)&temp);
            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
        }
        if (SOC_PBMP_MEMBER(pbmp, port)) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
    case DRV_QUEUE_MAP_TOS:
    case DRV_QUEUE_MAP_PORT:
        /* bcm53242/ 53262 have no force port based priority solution :
         *  - the default port priority can be retrieved only if all other
         *      TC decision process been disabled. (like protocol based, mac 
         *      based, vlan based,.. etc.)
         */
    case DRV_QUEUE_MAP_MAC:
    case DRV_QUEUE_MAP_HYBRID:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mapping_type_get: unit %d, port = %d, type = %d,  %sable\n",
        unit, port, mapping_type, *state ? "en" : "dis");
    return SOC_E_NONE;
}

int 
drv_bcm53242_queue_port_prio_to_queue_set(int unit, uint8 port, 
                                                uint8 prio, uint8 queue_n)
{
    return SOC_E_UNAVAIL;
}

int 
drv_bcm53242_queue_port_prio_to_queue_get(int unit, uint8 port, 
                                                uint8 prio, uint8 *queue_n)
{
    return SOC_E_UNAVAIL;
}

int 
drv_bcm53242_queue_port_dfsv_set(int unit, uint8 port, uint8 dscp, 
                                                uint8 prio, uint8 queue_n)
{
    return SOC_E_UNAVAIL;
}

int 
drv_bcm53242_queue_port_dfsv_get(int unit, uint8 port, uint8 dscp, 
                                                uint8 *prio, uint8 *queue_n)
{
    return SOC_E_UNAVAIL;
}

int 
drv_bcm53242_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio, uint8 prio)
{
    return SOC_E_UNAVAIL;
}

int 
drv_bcm53242_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, uint8 *prio)
{
    return SOC_E_UNAVAIL;
}

int 
drv_bcm53242_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio)
{
    return SOC_E_UNAVAIL;
}

int 
drv_bcm53242_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio)
{
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_queue_rx_reason_set
 *
 *  Purpose :
 *      Get the queue id base on the rx reason code.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      reason    :   rx reason code.
 *      queue   :  queue ID.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53242_queue_rx_reason_set(int unit, uint8 reason, uint32 queue)
{
    uint32  reg_value, temp;

    SOC_IF_ERROR_RETURN(
        REG_READ_QOS_REASON_CODEr(unit, &reg_value));

    temp = queue;
    switch (reason) {
        case DRV_RX_REASON_MIRRORING:
            soc_QOS_REASON_CODEr_field_set(unit, &reg_value, 
                PRI_MIRRORf, &temp);
            break;
        case DRV_RX_REASON_SA_LEARNING:
            soc_QOS_REASON_CODEr_field_set(unit, &reg_value, 
                PRI_SA_LEARNf, &temp);
            break;
        case DRV_RX_REASON_SWITCHING:
            soc_QOS_REASON_CODEr_field_set(unit, &reg_value, 
                PRI_SWITCHf, &temp);
            break;
        case DRV_RX_REASON_PROTO_TERM:
            soc_QOS_REASON_CODEr_field_set(unit, &reg_value, 
                PRI_PROTOCOL_TERMf, &temp);
            break;
        case DRV_RX_REASON_PROTO_SNOOP:
            soc_QOS_REASON_CODEr_field_set(unit, &reg_value, 
                PRI_PROTOCOL_SNOOPf, &temp);
            break;
        case DRV_RX_REASON_EXCEPTION:
            soc_QOS_REASON_CODEr_field_set(unit, &reg_value, 
                PRI_EXCEPTf, &temp);
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    SOC_IF_ERROR_RETURN(
        REG_WRITE_QOS_REASON_CODEr(unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_queue_rx_reason_get
 *
 *  Purpose :
 *      Get the queue id according to the rx reason code.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      reason    :   rx reason code.
 *      queue   :  queue ID.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */

int 
drv_bcm53242_queue_rx_reason_get(int unit, uint8 reason, uint32 *queue)
{
    uint32  reg_value, temp = 0;

    SOC_IF_ERROR_RETURN(
        REG_READ_QOS_REASON_CODEr(unit, &reg_value));
    
    switch (reason) {
        case DRV_RX_REASON_MIRRORING:
            soc_QOS_REASON_CODEr_field_get(unit, &reg_value, 
                PRI_MIRRORf, &temp);
            *queue = temp;
            break;
        case DRV_RX_REASON_SA_LEARNING:
            soc_QOS_REASON_CODEr_field_get(unit, &reg_value, 
                PRI_SA_LEARNf, &temp);
            *queue = temp;
            break;
        case DRV_RX_REASON_SWITCHING:
            soc_QOS_REASON_CODEr_field_get(unit, &reg_value, 
                PRI_SWITCHf, &temp);
            *queue = temp;
            break;
        case DRV_RX_REASON_PROTO_TERM:
            soc_QOS_REASON_CODEr_field_get(unit, &reg_value, 
                PRI_PROTOCOL_TERMf, &temp);
            *queue = temp;
            break;
        case DRV_RX_REASON_PROTO_SNOOP:
            soc_QOS_REASON_CODEr_field_get(unit, &reg_value, 
                PRI_PROTOCOL_SNOOPf, &temp);
            *queue = temp;
            break;
        case DRV_RX_REASON_EXCEPTION:
            soc_QOS_REASON_CODEr_field_get(unit, &reg_value, 
                PRI_EXCEPTf, &temp);
            *queue = temp;
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;

}
