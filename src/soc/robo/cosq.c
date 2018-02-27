/*
 * $Id: cosq.c 1.13 Broadcom SDK $
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

#include <soc/robo.h>


/*
 *  Function : drv_queue_mode_set
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
drv_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 mode)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, reg_addr, temp;
    int         reg_len;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mode_set: unit %d, bmp = %x, queue mode = %d\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), mode);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, FCON_FLOWMIXr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FCON_FLOWMIXr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

     switch (mode) {
            case DRV_QUEUE_MODE_STRICT:
                temp = 1;
                break;
            case DRV_QUEUE_MODE_WRR:
                temp = 0;
                break;
            case DRV_QUEUE_MODE_HYBRID:
            default:
                return SOC_E_UNAVAIL;
    }
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, MODE_HQ_PREEMPTf, &temp);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
    }

    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, FCON_STRICT_HIQ_PRE_CTRL0r);

    if (temp ==1)  {
        /* DRV_QUEUE_MODE_STRICT */
        /* fill out all 1 in FCON_STRICT_HIQ_PRE_CTRLXr */
        reg_value = 0xffff;         
    } else {
        reg_value = 0;
    }

    if (SOC_IS_ROBO5324(unit)) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, FCON_STRICT_HIQ_PRE_CTRL0r, 0, 0);
        (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len);

        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, FCON_STRICT_HIQ_PRE_CTRL1r, 0, 0);
        (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len);
    }

    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                            (unit, FCON_STRICT_HIQ_PRE_CTRL0r, 0, 0);
        (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len);

        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                            (unit, FCON_STRICT_HIQ_PRE_CTRL1r, 0, 0);
        (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len);

        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                            (unit, FCON_STRICT_HIQ_PRE_CTRL2r, 0, 0);
        (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len);

        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                            (unit, FCON_STRICT_HIQ_PRE_CTRL3r, 0, 0);
        (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len);
    }

    return rv;
}

/*
 *  Function : drv_queue_mode_get
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
drv_queue_mode_get(int unit, uint32 port, uint32 *mode)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, reg_addr, temp;
    int         reg_len;
    
    /* chank port number */
    if (port > (SOC_ROBO_MAX_NUM_PORTS - 1)) {
        return SOC_E_PARAM;
    }
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, FCON_FLOWMIXr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FCON_FLOWMIXr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    (DRV_SERVICES(unit)->reg_field_get)
        (unit, FCON_FLOWMIXr, &reg_value, MODE_HQ_PREEMPTf, &temp);

    if (temp) {
        *mode = DRV_QUEUE_MODE_STRICT;
    } else {
        *mode = DRV_QUEUE_MODE_WRR;
    }
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mode_get: unit %d, port = %d, queue mode = %d\n",
        unit, port, *mode);    
    return rv;
}

/*
 *  Function : drv_queue_count_set
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
drv_queue_count_set(int unit, uint32 port_type, uint8 count)
{
    int     rv = SOC_E_NONE;
    uint32	reg_addr, reg_value;
    int		reg_len;
    uint32	num_q, temp;
    uint32      max_numq;
    int		cos, prio, ratio, remain;
    uint64  reg_value64;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_count_set: unit %d, port type = %d, queue count = %d\n",
        unit, port_type, count);

    max_numq = NUM_COS(unit);	 
    if ((count > max_numq) || (count < 1)) {
        return SOC_E_PARAM;
    }
    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len));
        /* enable CPU_Control_Enable bit */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_CTLr, (uint32 *)&reg_value64, CPU_CTRL_ENf, &temp);
        /* set queue number */
        num_q = count - 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_CTLr, (uint32 *)&reg_value64, QOS_ENf, &num_q);
        /* write register */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len));
   	} else {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_CTLr);
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value, reg_len));
        /* enable CPU_Control_Enable bit */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_CTLr, &reg_value, CPU_CTRL_ENf, &temp);
        /* set queue number */
        num_q = count - 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_CTLr, &reg_value, QOS_ENf, &num_q);
        /* write register */
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &reg_value, reg_len));
   	}

    if (SOC_IS_ROBO5324(unit)) {
        /* disable Unicast default drop feature due to flow control 
         * scheme(HW default depended on cos on/off) 
         *      - user setting at CoS on/off will cause "EN_UCAST_DROPf" been 
         *        changed. (on->1, off->0)
         *      - below setting may force Broadcast/Multicast/Unicast flow control 
         *        at the same behavior no matter CoS on or CoS off.
         */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_FLOWMIXr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_FLOWMIXr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
    
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, FCON_FLOWMIXr, &reg_value, EN_UCAST_DROPf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len));
    }

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

    /* Reload MMU settings */
    if (SOC_IS_ROBO5324(unit)) {
        soc_robo_5324_mmu_default_set(unit);
    } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        soc_robo_5348_mmu_default_set(unit);
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_queue_count_get
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
drv_queue_count_get(int unit, uint32 port_type, uint8 *count)
{
    uint32	reg_addr, reg_value;
    int		reg_len;
    uint32	num_q;
    uint64  reg_value64;

    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        	(unit, reg_addr, (uint32 *)&reg_value64, reg_len));
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_CTLr, (uint32 *)&reg_value64, QOS_ENf, &num_q);
        *count = num_q + 1;
    } else {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_CTLr);
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value, reg_len));
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_CTLr, &reg_value, QOS_ENf, &num_q);
        *count = num_q + 1;
    }

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_count_get: unit %d, port type = %d, queue count = %d\n",
        unit, port_type, *count);
    
    return SOC_E_NONE;
}

/* set WRR weight */
/*
 *  Function : drv_queue_WRR_weight_set
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
drv_queue_WRR_weight_set(int unit, 
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
            reg_index = FCON_Q0_TXDSC_CTRL_3r;
            fld_index = Q0_QUOTA_SIZEf;
            break;
        case 1:
            reg_index = FCON_Q1_TXDSC_CTRL_3r;
            fld_index = Q1_QUOTA_SIZEf;
            break;
        case 2:
            reg_index = FCON_Q2_TXDSC_CTRL_3r;
            fld_index = Q2_QUOTA_SIZEf;
            break;
        case 3:
            reg_index = FCON_Q3_TXDSC_CTRL_3r;
            fld_index = Q3_QUOTA_SIZEf;
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
 *  Function : drv_queue_WRR_weight_get
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
drv_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint8 queue, uint32 *weight)
{
    uint32  reg_addr, reg_value, temp;
    int     reg_len;
    uint32  reg_index, fld_index;
    
    switch (queue) {
        case 0:
            reg_index = FCON_Q0_TXDSC_CTRL_3r;
            fld_index = Q0_QUOTA_SIZEf;
            break;
        case 1:
            reg_index = FCON_Q1_TXDSC_CTRL_3r;
            fld_index = Q1_QUOTA_SIZEf;
            break;
        case 2:
            reg_index = FCON_Q2_TXDSC_CTRL_3r;
            fld_index = Q2_QUOTA_SIZEf;
            break;
        case 3:
            reg_index = FCON_Q3_TXDSC_CTRL_3r;
            fld_index = Q3_QUOTA_SIZEf;
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
 *  Function : drv_queue_prio_set
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
drv_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_prio_set: unit %d, port = %d, priority = %d, queue = %d\n",
        unit, port, prio, queue_n);

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TCI_THr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TCI_THr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
    temp = queue_n;
    switch ( prio ) {        
    case 0:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_000f, &temp));
        break;
    case 1:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_001f, &temp));
        break;
    case 2:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_010f, &temp));
        break;
    case 3:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_011f, &temp));
        break;
    case 4:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_100f, &temp));
        break;
    case 5:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_101f, &temp));
        break;
    case 6:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_110f, &temp));
        break;
    case 7:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_111f, &temp));
        break;
    default:
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &reg_value, reg_len));

    return SOC_E_NONE;

}

/*
 *  Function : drv_queue_prio_get
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
drv_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n)
{
    uint32	reg_addr, reg_value;
    int		reg_len;
    uint32	temp;

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TCI_THr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TCI_THr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
    switch ( prio ) {        
    case 0:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_000f, &temp));
        break;
    case 1:         
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_001f, &temp));
        break;
    case 2:         
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_010f, &temp));
        break;
    case 3:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_011f, &temp));
        break;
    case 4:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_100f, &temp));
        break;
    case 5:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_101f, &temp));
        break;
    case 6:         
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_110f, &temp));
        break;
    case 7:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_TCI_THr, 
                            &reg_value, PRITAG_111f, &temp));
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
 *  Function : drv_queue_tos_set
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
drv_queue_tos_set(int unit, uint8 precedence, uint8 queue_n)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len;

    /* need set 4 types of TOS priority registers 
        D-TYPE, T-TYPE, R-TYPE and M-TYPE */
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_tos_set: unit %d, tos value = %d, queue = %d\n",
        unit, precedence, queue_n);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_D_TOS_THr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_D_TOS_THr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
    temp = queue_n;
    switch ( precedence ) {        
    case 0:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_000f, &temp));
        break;
    case 1:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_001f, &temp));
        break;
    case 2:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_010f, &temp));
        break;
    case 3:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_011f, &temp));
        break;
    case 4:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_100f, &temp));
        break;
    case 5:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_101f, &temp));
        break;
    case 6:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_110f, &temp));
        break;
    case 7:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_111f, &temp));
        break;
    default:
        return SOC_E_PARAM;
    }

    /* write D-type register */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len));
    /* write T-type register */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr + 2, &reg_value, reg_len));
    /* write R-type register */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr + 4, &reg_value, reg_len));
    /* write M-type register */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr + 6, &reg_value, reg_len));

    return SOC_E_NONE;
}

/*
 *  Function : drv_queue_tos_get
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
drv_queue_tos_get(int unit, uint8 precedence, uint8 *queue_n)
{
    uint32	reg_addr, reg_value;
    int		reg_len;
    uint32	temp;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_D_TOS_THr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_D_TOS_THr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
    switch ( precedence ) {        
    case 0:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_000f, &temp));
        break;
    case 1:         
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_001f, &temp));
        break;
    case 2:         
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_010f, &temp));
        break;
    case 3:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_011f, &temp));
        break;
    case 4:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_100f, &temp));
        break;
    case 5:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_101f, &temp));
        break;
    case 6:         
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_110f, &temp));
        break;
    case 7:
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_D_TOS_THr, 
                            &reg_value, D_TOS_111f, &temp));
        break;
    default:
        return SOC_E_PARAM;
    }
    *queue_n = temp;
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_tos_get: unit %d, tos value = %d, queue = %d\n",
        unit, precedence, *queue_n);
    return SOC_E_NONE;
}

/*
 *  Function : drv_queue_dfsv_set
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
drv_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n)
{
    uint32	reg_addr, temp;
    int		reg_len, reg_index;
    uint64  reg_value;


    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_dfsv_set: unit %d, dscp value = %d, queue = %d\n",
        unit, code_point, queue_n);
    if (code_point  > 31) {
        reg_index = QOS_DIFF_DSCP2r;
        code_point -= 32;
    } else {
        reg_index = QOS_DIFF_DSCP1r;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value,reg_len));
    temp = queue_n;
    switch ( code_point ) {        
    case 0:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000000f, &temp));
        break;
    case 1:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000001f, &temp));
        break;
    case 2:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000010f, &temp));
        break;
    case 3:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000011f, &temp));
        break;
    case 4:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000100f, &temp));
        break;
    case 5:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000101f, &temp));
        break;
    case 6:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000110f, &temp));
        break;
    case 7:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000111f, &temp));
        break;
    case 8:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001000f, &temp));
        break;
    case 9:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001001f, &temp));
        break;
    case 10:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001010f, &temp));
        break;
    case 11:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001011f, &temp));
        break;
    case 12:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001100f, &temp));
        break;
    case 13:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001101f, &temp));
        break;
    case 14:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001110f, &temp));
        break;
    case 15:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001111f, &temp));
        break;
    case 16:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010000f, &temp));
        break;
    case 17:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010001f, &temp));
        break;
    case 18:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010010f, &temp));
        break;
    case 19:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010011f, &temp));
        break;
    case 20:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010100f, &temp));
        break;
    case 21:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010101f, &temp));
        break;
    case 22:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010110f, &temp));
        break;
    case 23:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010111f, &temp));
        break;
    case 24:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011000f, &temp));
        break;
    case 25:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011001f, &temp));
        break;
    case 26:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011010f, &temp));
        break;
    case 27:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011011f, &temp));
        break;
    case 28:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011100f, &temp));
        break;
    case 29:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011101f, &temp));
        break;
    case 30:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011110f, &temp));
        break;
    case 31:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011111f, &temp));
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
 *  Function : drv_queue_dfsv_get
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
drv_queue_dfsv_get(int unit, uint8 code_point, uint8 *queue_n)
{
    uint32	reg_addr, temp;
    int		reg_len, reg_index;
    uint64      reg_value;


    sal_memset(&reg_value, 0, 8);
    if (code_point  > 31) {
        reg_index = QOS_DIFF_DSCP2r;
        code_point -= 32;
    } else {

        reg_index = QOS_DIFF_DSCP1r;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value,reg_len));
    switch ( code_point ) {        
    case 0:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000000f, &temp));
        break;
    case 1:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000001f, &temp));
        break;
    case 2:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000010f, &temp));
        break;
    case 3:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000011f, &temp));
        break;
    case 4:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000100f, &temp));
        break;
    case 5:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000101f, &temp));
        break;
    case 6:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000110f, &temp));
        break;
    case 7:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_000111f, &temp));
        break;
    case 8:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001000f, &temp));
        break;
    case 9:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001001f, &temp));
        break;
    case 10:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001010f, &temp));
        break;
    case 11:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001011f, &temp));
        break;
    case 12:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001100f, &temp));
        break;
    case 13:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001101f, &temp));
        break;
    case 14:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001110f, &temp));
        break;
    case 15:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_001111f, &temp));
        break;
    case 16:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010000f, &temp));
        break;
    case 17:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010001f, &temp));
        break;
    case 18:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010010f, &temp));
        break;
    case 19:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010011f, &temp));
        break;
    case 20:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010100f, &temp));
        break;
    case 21:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010101f, &temp));
        break;
    case 22:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010110f, &temp));
        break;
    case 23:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_010111f, &temp));
        break;
    case 24:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011000f, &temp));
        break;
    case 25:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011001f, &temp));
        break;
    case 26:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011010f, &temp));
        break;
    case 27:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011011f, &temp));
        break;
    case 28:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011100f, &temp));
        break;
    case 29:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011101f, &temp));
        break;
    case 30:         
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011110f, &temp));
        break;
    case 31:
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP1r, 
                            (uint32 *)&reg_value, PRI_DSCP_011111f, &temp));
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
 *  Function : drv_queue_mapping_state_set
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
drv_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state)
{
    uint32	reg_addr, reg_value;
    int		reg_len;
    uint32	temp;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mapping_type_set: unit %d, bmp = %x, type = %d,  %sable\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), mapping_type, state ? "en" : "dis");
    switch (mapping_type) {
    case DRV_QUEUE_MAP_NONE:
        /* disable QOS_IP_EN register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_1P_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_1P_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_1P_ENr, &reg_value, QOS_1P_ENf, &temp));
        temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_1P_ENr, &reg_value, QOS_1P_ENf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));
        /* disable QOS_TOS_DIF_EN register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_TOS_DIF_ENr, &reg_value, QOS_TOS_DIFF_ENf, &temp));
        temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_TOS_DIF_ENr, &reg_value, QOS_TOS_DIFF_ENf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
        break;
    case DRV_QUEUE_MAP_PRIO:
        /* enable/disable QOS_1P_ENr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_1P_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_1P_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_1P_ENr, &reg_value, QOS_1P_ENf, &temp));
        if (state) {
            temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        } else {
            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_1P_ENr, &reg_value, QOS_1P_ENf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

        /* don't need to disable QOS_TOS_DIF_ENr : 
          * the setting is separated and chosen by QoS priority decision flowchart
          */
        break;
    case DRV_QUEUE_MAP_TOS:
        /* set QOS_TOS_DIF_CTLr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_CTLr);
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_TOS_DIF_CTLr, &reg_value, TOS_DIFF_SELf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));
        /* enable / diaable QOS_TOS_DIF_ENr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_TOS_DIF_ENr, &reg_value, QOS_TOS_DIFF_ENf, &temp));
        if (state) {
            temp |=(SOC_PBMP_WORD_GET(bmp, 0));
        } else {
            temp &= (SOC_PBMP_WORD_GET(bmp, 0));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_TOS_DIF_ENr, &reg_value, QOS_TOS_DIFF_ENf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

        /* don't need to disable QOS_1P : 
          * the setting is separated and chosen by QoS priority decision flowchart
          */
        break;
    case DRV_QUEUE_MAP_DFSV:
        /* set QOS_TOS_DIF_CTLr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, QOS_TOS_DIF_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, QOS_TOS_DIF_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_TOS_DIF_CTLr, &reg_value, TOS_DIFF_SELf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));
        /* enable / diaable QOS_TOS_DIF_ENr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_TOS_DIF_ENr, &reg_value, QOS_TOS_DIFF_ENf, &temp));
        if (state) {
            temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        } else {
            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_TOS_DIF_ENr, &reg_value, QOS_TOS_DIFF_ENf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

        /* don't need to disable QOS_1P : 
          * the setting is separated and chosen by QoS priority decision flowchart
          */
        break;
    case DRV_QUEUE_MAP_PORT:
         /* enable/disable Port-based QoS */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_CTLr, &reg_value, PRI_PMASKf, &temp));
        if (state) {
            temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        } else {
            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_CTLr, &reg_value, PRI_PMASKf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));
        break;
    case DRV_QUEUE_MAP_MAC:
    case DRV_QUEUE_MAP_HYBRID:
        return SOC_E_UNAVAIL;
    default :
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_queue_mapping_state_get
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
drv_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state)
{
    uint32	reg_addr, reg_value;
    int		reg_len;
    uint32	temp;

    switch (mapping_type)
    {
    case DRV_QUEUE_MAP_NONE:
        return SOC_E_PARAM;
    case DRV_QUEUE_MAP_PRIO:
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_1P_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_1P_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_1P_ENr, &reg_value, QOS_1P_ENf, &temp);
        if (temp & (0x1 << port)) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
    case DRV_QUEUE_MAP_TOS:
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_TOS_DIF_ENr, &reg_value, QOS_TOS_DIFF_ENf, &temp));
        if (!(temp & (0x1 << port))) {
            *state = FALSE;
            break;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_TOS_DIF_CTLr, &reg_value, TOS_DIFF_SELf, &temp));
        if (temp) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
    case DRV_QUEUE_MAP_DFSV:
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_TOS_DIF_ENr, &reg_value, QOS_TOS_DIFF_ENf, &temp));
        if (!(temp & (0x1 << port))) {
            *state = FALSE;
            break;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_TOS_DIF_CTLr, &reg_value, TOS_DIFF_SELf, &temp));
        if (temp) {
            *state = FALSE;
        } else {
            *state = TRUE;
        }
        break;
    case DRV_QUEUE_MAP_PORT:
        /* get Port-based QoS status */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_CTLr, &reg_value, PRI_PMASKf, &temp);
        if (temp & (0x1 << port)) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
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
drv_queue_port_prio_to_queue_set(int unit, uint8 port, 
                                                uint8 prio, uint8 queue_n)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_port_prio_to_queue_get(int unit, uint8 port, 
                                                uint8 prio, uint8 *queue_n)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_port_dfsv_set(int unit, uint8 port, uint8 dscp, 
                                                uint8 prio, uint8 queue_n)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_port_dfsv_get(int unit, uint8 port, uint8 dscp, 
                                                uint8 *prio, uint8 *queue_n)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio, uint8 prio)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, uint8 *prio)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_dfsv_unmap_set(int unit, uint8 prio, uint8 dscp)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_dfsv_unmap_get(int unit, uint8 prio, uint8 *dscp)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_rx_reason_set(int unit, uint8 reason, uint32 queue)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_rx_reason_get(int unit, uint8 reason, uint32 *queue)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_port_txq_pause_set(int unit, uint32 port, uint8 queue_n, uint8 enable)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_port_txq_pause_get(int unit, uint32 port, uint8 queue_n, uint8 * enable)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_qos_control_set(int unit, uint32 type, uint32 state)
{
    return SOC_E_UNAVAIL;
}

int 
drv_queue_qos_control_get(int unit, uint32 type, uint32 * state)
{
    return SOC_E_UNAVAIL;
}
