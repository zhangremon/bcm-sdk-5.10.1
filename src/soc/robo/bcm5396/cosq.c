/*
 * $Id: cosq.c 1.14 Broadcom SDK $
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


/* Port_QoS_En */
#define LAYER_SEL_QOS 0x0
#define PORT_BASE_QOS 0x1

/* QoS_Layer_Sel */
#define PRIO_QOS 0x0
#define DFSV_QOS 0x1
#define IP_QOS  0x2
#define ALL_QOS 0x3

int
drv_bcm5396_queue_port_prio_to_queue_set
    (int unit, uint8 port, uint8 prio, uint8 queue_n)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, reg_addr, temp;
    int         reg_len;   

    /* chank port number */
    if (port > (SOC_ROBO_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, QOS_RX_CTRL_Pr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, QOS_RX_CTRL_Pr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
         (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    temp = queue_n;
    switch ( prio ) {        
        case 0:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT000_TO_QIDf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT001_TO_QIDf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT010_TO_QIDf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT011_TO_QIDf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT100_TO_QIDf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT101_TO_QIDf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT110_TO_QIDf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT111_TO_QIDf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }        
    if ((rv = (DRV_SERVICES(unit)->reg_write)
         (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
     return SOC_E_NONE;
}

int
drv_bcm5396_queue_port_prio_to_queue_get
    (int unit, uint8 port, uint8 prio, uint8 *queue_n)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, reg_addr, temp;
    int         reg_len;   

    /* chank port number */
    if (port > (SOC_ROBO_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, QOS_RX_CTRL_Pr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, QOS_RX_CTRL_Pr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
         (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    switch ( prio ) {        
       case 0:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT000_TO_QIDf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT001_TO_QIDf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT010_TO_QIDf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT011_TO_QIDf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT100_TO_QIDf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT101_TO_QIDf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT110_TO_QIDf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_RX_CTRL_Pr, 
                            &reg_value, PRT111_TO_QIDf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }
    *queue_n = temp;
     return SOC_E_NONE;
}

int
drv_bcm5396_queue_port_dfsv_set
    (int unit, uint8 port, uint8 dscp, uint8 prio, uint8 queue_n)
{

    int rv = SOC_E_NONE;
    uint32  reg_value, reg_addr;
    int         reg_len;   
    uint32 bmp;

    /* chank port number */
    if (port > (SOC_ROBO_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, QOS_EN_DIFFSERVr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, QOS_EN_DIFFSERVr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
         (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_EN_DIFFSERVr, 
        &reg_value, QOS_EN_DIFFSERVf, &bmp));
    bmp |= 1U << port;
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_EN_DIFFSERVr, 
        &reg_value, QOS_EN_DIFFSERVf, &bmp));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
         (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->queue_dfsv_remap_set)
        (unit, dscp, prio));
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->queue_port_prio_to_queue_set)
        (unit, port, prio, queue_n));            
    return rv;
}

int
drv_bcm5396_queue_port_dfsv_get
    (int unit, uint8 port, uint8 dscp, uint8 *prio, uint8 *queue_n)
{

    int rv = SOC_E_NONE;
    uint32  reg_value, reg_addr;
    int         reg_len;   
    uint32 bmp;
    soc_pbmp_t pbmp;
    
    /* chank port number */
    if (port > (SOC_ROBO_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, QOS_EN_DIFFSERVr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, QOS_EN_DIFFSERVr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
         (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_EN_DIFFSERVr, 
        &reg_value, QOS_EN_DIFFSERVf, &bmp));
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    if(!SOC_PBMP_MEMBER(pbmp,port))
    {
        return SOC_E_BADID;
    }

    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->queue_dfsv_remap_get)
        (unit, dscp, prio));

    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->queue_port_prio_to_queue_get)
        (unit, port, *prio, queue_n)); 

    return SOC_E_NONE;
}
/*
 *  Function : drv_queue_mode_set
 *
 *  Purpose :
 *      Set the queue mode of selected port type.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      bmp   :   port bitmap, not used for bcm5396.
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
drv_bcm5396_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 mode)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, reg_addr, temp;
    int         reg_len;
    soc_pbmp_t      pbmp;
    uint8 queue_n;
    
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mode_set: unit %d, bmp = %x, queue mode = %d\n",
        unit, SOC_PBMP_WORD_GET(bmp,0), mode);
    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, QOS_TX_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, QOS_TX_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
    }
    

    switch (mode) {
        case DRV_QUEUE_MODE_STRICT:
            temp = 1;
            queue_n = 3;
            SOC_PBMP_ASSIGN(pbmp, bmp);
            break;
        case DRV_QUEUE_MODE_WRR:
            temp = 0;
            break;
        case DRV_QUEUE_MODE_HYBRID:
        default:
            return SOC_E_UNAVAIL;
    }

    (DRV_SERVICES(unit)->reg_field_set)
        (unit, QOS_TX_CTRLr, &reg_value, HQ_PREEMPTf, &temp);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
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
drv_bcm5396_queue_mode_get(int unit, uint32 port, uint32 *mode)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, reg_addr, temp;
    int         reg_len;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, QOS_TX_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, QOS_TX_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, QOS_TX_CTRLr, &reg_value, HQ_PREEMPTf, &temp));
    
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
drv_bcm5396_queue_count_set(int unit, uint32 port_type, uint8 count)
{
    uint32	reg_addr, reg_value;
    int		reg_len;
    uint32	num_q;
    uint32      max_numq;
    int		cos, prio, ratio, remain;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_count_set: unit %d, port type = %d, queue count = %d\n",
        unit, port_type, count);
    max_numq = NUM_COS(unit);   

    if ((count > max_numq) || (count < 1)) {
        return SOC_E_PARAM;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TX_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TX_CTRLr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value, reg_len));
    /* set queue number */
    num_q = count - 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, QOS_TX_CTRLr, &reg_value, QOS_MODEf, &num_q);
    /* write register */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &reg_value, reg_len));

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
drv_bcm5396_queue_count_get(int unit, uint32 port_type, uint8 *count)
{
    uint32	reg_addr, reg_value;
    int		reg_len;
    uint32	num_q;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TX_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TX_CTRLr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value, reg_len));
    (DRV_SERVICES(unit)->reg_field_get)
        (unit, QOS_TX_CTRLr, &reg_value, QOS_MODEf, &num_q);
    *count = num_q + 1;

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
drv_bcm5396_queue_WRR_weight_set(int unit, 
    uint32 port_type, uint8 queue, uint32 weight)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr, reg_value, temp;
    int     reg_len;
    uint32  max_weight = 0;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_WRR_weight_set: unit %d, port type = %d, \
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
    
   
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_WEIGHTr, 0, queue);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_WEIGHTr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
    temp = weight;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, QOS_WEIGHTr, &reg_value, WEIGHTSf, &temp));
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
drv_bcm5396_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint8 queue, uint32 *weight)
{
    uint32  reg_addr, reg_value;
    int     reg_len;
    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_WEIGHTr, 0, queue);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_WEIGHTr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
    
    *weight = reg_value;
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_WRR_weight_set: unit %d, port type = %d, \
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
drv_bcm5396_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n)
{
    soc_pbmp_t      pbmp;
    uint32 p;
    
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_prio_set: unit %d, port = %d, priority = %d, queue = %d\n",
        unit, port, prio, queue_n);
    pbmp = PBMP_ALL(unit);

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    /* Set 1P/1Q Prioryty map */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->queue_prio_remap_set)
        (unit, port, prio, prio));

    PBMP_ITER(pbmp, p) {
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->queue_port_prio_to_queue_set)
                (unit, p, prio, queue_n));
    }

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
drv_bcm5396_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n)
{
    uint32	p;

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }
    
    p = 0;
    SOC_IF_ERROR_RETURN(
       (DRV_SERVICES(unit)->queue_port_prio_to_queue_get)
        (unit, p, prio, queue_n));
        
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_prio_get: unit %d, port = %d, priority = %d, queue = %d\n",
        unit, port, prio, *queue_n);
    return SOC_E_NONE;
}


int
drv_bcm5396_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio, uint8 prio)
{
    uint32  reg_addr, reg_value, temp;
    int     reg_len;
    
    if (port != -1) {
        return SOC_E_UNAVAIL;
    }
    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_1P1Q_PRI_MAPr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_1P1Q_PRI_MAPr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value, reg_len));
    temp = (uint32 )prio;
    switch ( pre_prio ) {        
        case 0:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG000_PRI_MAPf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG001_PRI_MAPf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG010_PRI_MAPf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG011_PRI_MAPf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG100_PRI_MAPf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG101_PRI_MAPf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG110_PRI_MAPf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG111_PRI_MAPf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }        
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &reg_value, reg_len));
     return SOC_E_NONE;
}

int
drv_bcm5396_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, uint8 *prio)
{
    uint32  reg_addr, reg_value, temp;
    int     reg_len;

    if (port != -1) {
        return SOC_E_UNAVAIL;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_1P1Q_PRI_MAPr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_1P1Q_PRI_MAPr);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value, reg_len));

    switch ( pre_prio ) {        
        case 0:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG000_PRI_MAPf, &temp));
            break;
        case 1:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG001_PRI_MAPf, &temp));
            break;
        case 2:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG010_PRI_MAPf, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG011_PRI_MAPf, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG100_PRI_MAPf, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG101_PRI_MAPf, &temp));
            break;
        case 6:         
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG110_PRI_MAPf, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_1P1Q_PRI_MAPr, 
                            &reg_value, TAG111_PRI_MAPf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }        

    *prio = temp;
     return SOC_E_NONE;

}

int
drv_bcm5396_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio)
{
    uint32  reg_addr, temp;        
    uint64  reg_value;
    int     reg_len, reg_index = 0;
    
    if (dscp < 16 ){
        reg_index = QOS_DIFF_DSCP0r;        
    } else if ((16 <= dscp) && (dscp <  32)) {
        reg_index = QOS_DIFF_DSCP1r;
        dscp -= 16;
    } else if((32 <= dscp) && (dscp <  48)){
        reg_index = QOS_DIFF_DSCP2r;
        dscp -= 32;
    } else if((48 <= dscp) && (dscp <  64)){
        reg_index = QOS_DIFF_DSCP3r;
        dscp -= 48;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, (uint32 *)&reg_value, reg_len));

    temp = (uint32 )prio;
    switch ( dscp ) {        
        case 0:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000000f, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000001f, &temp));
            break;
        case 2:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000010f, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000011f, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000100f, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000101f, &temp));
            break;
        case 6:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000110f, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000111f, &temp));
            break;
        case 8:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001000f, &temp));
            break;
        case 9:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001001f, &temp));
            break;
        case 10:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001010f, &temp));
            break;
        case 11:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001011f, &temp));
            break;
        case 12:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001100f, &temp));
            break;
        case 13:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001101f, &temp));
            break;
        case 14:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001110f, &temp));
            break;
        case 15:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001111f, &temp));
            break;            
    }

    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, (uint32 *)&reg_value, reg_len));
    return SOC_E_NONE;
}

int
drv_bcm5396_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio)
{
    uint32  reg_addr, temp = 0;        
    uint64  reg_value;        
    int     reg_len, reg_index = 0;
    
    if (dscp < 16 ){
        reg_index = QOS_DIFF_DSCP0r;        
    } else if ((16 <= dscp) && (dscp <  32)) {
        reg_index = QOS_DIFF_DSCP1r;
        dscp -= 16;
    } else if((32 <= dscp) && (dscp <  48)){
        reg_index = QOS_DIFF_DSCP2r;
        dscp -= 32;
    } else if((48 <= dscp) && (dscp <  64)){
        reg_index = QOS_DIFF_DSCP3r;
        dscp -= 48;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, (uint32 *)&reg_value, reg_len));
    
    switch ( dscp ) {        
        case 0:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000000f, &temp));
            break;
        case 1:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000001f, &temp));
            break;
        case 2:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000010f, &temp));
            break;
        case 3:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000011f, &temp));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000100f, &temp));
            break;
        case 5:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000101f, &temp));
            break;
        case 6:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000110f, &temp));
            break;
        case 7:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_000111f, &temp));
            break;
        case 8:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001000f, &temp));
            break;
        case 9:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001001f, &temp));
            break;
        case 10:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001010f, &temp));
            break;
        case 11:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001011f, &temp));
            break;
        case 12:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001100f, &temp));
            break;
        case 13:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001101f, &temp));
            break;
        case 14:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001110f, &temp));
            break;
        case 15:
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)(unit, QOS_DIFF_DSCP0r, 
                            (uint32 *)&reg_value, PRI_DSCP_001111f, &temp));
            break;            
    }
    *prio = temp;

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
drv_bcm5396_queue_tos_set(int unit, uint8 precedence, uint8 queue_n)
{
    return SOC_E_UNAVAIL;
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
drv_bcm5396_queue_tos_get(int unit, uint8 precedence, uint8 *queue_n)
{
    return SOC_E_UNAVAIL;
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
drv_bcm5396_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n)
{
    int rv = SOC_E_NONE;
    uint8 prio, port, state;
    soc_pbmp_t pbmp;

    pbmp = PBMP_PORT_ALL(unit);

    switch (queue_n) {
        case 0:
            prio = 1;
            break;
        case 1:
            prio = 3;            
            break;
        case 2:
            prio = 5;
            break;
        case 3:
            prio = 7;
            break;
        default :
            prio = 0;
            break;
    }
    PBMP_ITER(pbmp, port) {        
        (DRV_SERVICES(unit)->queue_mapping_type_get)
            (unit, port, DRV_QUEUE_MAP_DFSV, &state);
        if (!state)
            break;            
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->queue_port_dfsv_set)
            (unit, port, code_point, prio, queue_n));
    }

    return rv;
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
drv_bcm5396_queue_dfsv_get(int unit, uint8 code_point, uint8 *queue_n)
{
    int rv = SOC_E_NONE;
    uint8 prio, port, state;
    soc_pbmp_t pbmp;

    pbmp = PBMP_PORT_ALL(unit);

    PBMP_ITER(pbmp, port) {        
        (DRV_SERVICES(unit)->queue_mapping_type_get)
            (unit, port, DRV_QUEUE_MAP_DFSV, &state);
        if (!state) {
            break;
        }
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->queue_port_dfsv_get)
             (unit, port, code_point, &prio, queue_n));
        goto exit;
    }

exit:
    return rv;
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
drv_bcm5396_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state)
{
    uint32	reg_addr, reg_value;
    int reg_len;
    uint32	temp;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mapping_type_set: unit %d, bmp = %x, type = %d,  %sable\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), mapping_type, state ? "en" : "dis");
    switch (mapping_type) {
    case DRV_QUEUE_MAP_NONE:
        /* set port_qos_en=1 && qos_layer_sel = 0  */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_GLOBAL_CTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_GLOBAL_CTRLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        temp = PORT_BASE_QOS;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, PORT_QOS_ENf, &temp));
        temp = PRIO_QOS;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, QOS_LAYER_SELf, &temp));        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

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
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_EN_DIFFSERVr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_EN_DIFFSERVr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_EN_DIFFSERVr, &reg_value, QOS_EN_DIFFSERVf, &temp));
        temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_EN_DIFFSERVr, &reg_value, QOS_EN_DIFFSERVf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
        break;

    case DRV_QUEUE_MAP_PRIO:
        /* set port_qos_en=0 && qos_layer_sel = 0b (layer2 only) */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_GLOBAL_CTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_GLOBAL_CTRLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        temp = LAYER_SEL_QOS;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, PORT_QOS_ENf, &temp));
        temp = PRIO_QOS;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, QOS_LAYER_SELf, &temp));        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

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
        /* don't need to disable QOS_EN_DIFFSERVr, 
          * the priority of QOS_1P_ENr > QOS_EN_DIFFSERVr
          * by TC Decision Tree : port_qos_en=0 && qos_layer_sel = 0b 
          */
        break;
    case DRV_QUEUE_MAP_DFSV:
        /* set port_qos_en=0 && qos_layer_sel = 10b  */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_GLOBAL_CTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_GLOBAL_CTRLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        temp = LAYER_SEL_QOS;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, PORT_QOS_ENf, &temp));
        temp = IP_QOS;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, QOS_LAYER_SELf, &temp));        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

        /* enable / diaable QOS_TOS_DIF_ENr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_EN_DIFFSERVr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_EN_DIFFSERVr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_EN_DIFFSERVr, &reg_value, QOS_EN_DIFFSERVf, &temp));
        if (state) {
            temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        } else {
            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_EN_DIFFSERVr, &reg_value, QOS_EN_DIFFSERVf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));
        /* don't need to disable QOS_1P_ENr, 
          * the priority of QOS_EN_DIFFSERVf > QOS_1P_ENr
          * by TC Decision Tree : port_qos_en=0 && qos_layer_sel = 10b 
          */
        break;
    case DRV_QUEUE_MAP_PORT:
        /* enable/disable port based priority :
         *  - port_qos_en=0
         */
        if (SOC_PBMP_NEQ(bmp, PBMP_ALL(unit))){
            return SOC_E_UNAVAIL;   /* system based configuration only */
        }
        SOC_IF_ERROR_RETURN(REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));
        temp = (state) ? PORT_BASE_QOS : LAYER_SEL_QOS;
        SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_set(unit, 
                &reg_value, PORT_QOS_ENf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_GLOBAL_CTRLr(unit, &reg_value));
        
        break;
    case DRV_QUEUE_MAP_HYBRID:
        /* set port_qos_en=1 && qos_layer_sel = 3  */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_GLOBAL_CTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_GLOBAL_CTRLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        temp = PORT_BASE_QOS;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, PORT_QOS_ENf, &temp));
        temp = ALL_QOS;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, QOS_LAYER_SELf, &temp));        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));
        
         /* enable QOS_IP_EN register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_1P_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_1P_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_1P_ENr, &reg_value, QOS_1P_ENf, &temp));
        temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_1P_ENr, &reg_value, QOS_1P_ENf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

        /* enable QOS_TOS_DIF_EN register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_EN_DIFFSERVr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_EN_DIFFSERVr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_EN_DIFFSERVr, &reg_value, QOS_EN_DIFFSERVf, &temp));
        temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_EN_DIFFSERVr, &reg_value, QOS_EN_DIFFSERVf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
        break;
    case DRV_QUEUE_MAP_TOS:
    case DRV_QUEUE_MAP_MAC:
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
drv_bcm5396_queue_mapping_type_get(int unit, uint32 port, 
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

        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_GLOBAL_CTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_GLOBAL_CTRLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, PORT_QOS_ENf, &temp));
        if (temp) {
            *state = FALSE;
            break;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, QOS_LAYER_SELf, &temp));
        if ( temp == DFSV_QOS) {
            *state = FALSE;
            break;
        }
            
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_EN_DIFFSERVr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_EN_DIFFSERVr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_EN_DIFFSERVr, &reg_value, QOS_EN_DIFFSERVf, &temp));
        if (temp & (0x1 << port)) {
            *state = FALSE;
            break;
        }
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
     
    case DRV_QUEUE_MAP_DFSV:

        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_GLOBAL_CTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_GLOBAL_CTRLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, PORT_QOS_ENf, &temp));
        if (temp) {
            *state = FALSE;
            break;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, QOS_LAYER_SELf, &temp));
        if ( temp == PRIO_QOS) {
            *state = FALSE;
            break;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_EN_DIFFSERVr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_EN_DIFFSERVr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_EN_DIFFSERVr, &reg_value, QOS_EN_DIFFSERVf, &temp));
       if (temp & (0x1 << port)) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }

        break;
    case DRV_QUEUE_MAP_PORT:
        /* enable/disable port based priority :
         *  - port_qos_en=0
         */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_QOS_GLOBAL_CTRLr_field_get(unit, 
                &reg_value, PORT_QOS_ENf, &temp));
        *state = (temp == PORT_BASE_QOS) ? TRUE : FALSE;
        break;
    case DRV_QUEUE_MAP_HYBRID:
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_GLOBAL_CTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_GLOBAL_CTRLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, PORT_QOS_ENf, &temp));
        if (!temp) {
            *state = FALSE;
            break;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_GLOBAL_CTRLr, &reg_value, QOS_LAYER_SELf, &temp));
        if ( temp == ALL_QOS) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
    case DRV_QUEUE_MAP_TOS:
    case DRV_QUEUE_MAP_MAC:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mapping_type_get: unit %d, port = %d, type = %d,  %sable\n",
        unit, port, mapping_type, *state ? "en" : "dis");
    return SOC_E_NONE;
}
