/*
 * $Id: cosq.c 1.23.6.2 Broadcom SDK $
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
drv_bcm5395_queue_port_prio_to_queue_set
    (int unit, uint8 port, uint8 prio, uint8 queue_n)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, temp;  

    /* chank port number */
    if (port > (SOC_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    if ((rv = REG_READ_TC2COS_MAPr(unit, &reg_value)) < 0) {
        return rv;
    }
    temp = queue_n;
    switch ( prio ) {        
        case 0:
            soc_TC2COS_MAPr_field_set(unit, &reg_value, 
                PRT000_TO_QIDf, &temp);
            break;
        case 1:
            soc_TC2COS_MAPr_field_set(unit, &reg_value, 
                PRT001_TO_QIDf, &temp);
            break;
        case 2:         
            soc_TC2COS_MAPr_field_set(unit, &reg_value, 
                PRT010_TO_QIDf, &temp);
            break;
        case 3:
            soc_TC2COS_MAPr_field_set(unit, &reg_value, 
                PRT011_TO_QIDf, &temp);
            break;
        case 4:
            soc_TC2COS_MAPr_field_set(unit, &reg_value, 
                PRT100_TO_QIDf, &temp);
            break;
        case 5:
            soc_TC2COS_MAPr_field_set(unit, &reg_value, 
                PRT101_TO_QIDf, &temp);
            break;
        case 6:         
            soc_TC2COS_MAPr_field_set(unit, &reg_value, 
                PRT110_TO_QIDf, &temp);
            break;
        case 7:
            soc_TC2COS_MAPr_field_set(unit, &reg_value, 
                PRT111_TO_QIDf, &temp);
            break;
        default:
            return SOC_E_PARAM;
    }        
    if ((rv = REG_WRITE_TC2COS_MAPr(unit, &reg_value)) < 0) {
        return rv;
    }
     return SOC_E_NONE;
}

int
drv_bcm5395_queue_port_prio_to_queue_get
    (int unit, uint8 port, uint8 prio, uint8 *queue_n)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, temp = 0;

    /* chank port number */
    if (port > (SOC_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    if ((rv = REG_READ_TC2COS_MAPr(unit, &reg_value)) < 0) {
        return rv;
    }

    switch ( prio ) {        
       case 0:
            soc_TC2COS_MAPr_field_get(unit, &reg_value, 
                PRT000_TO_QIDf, &temp);
            break;
        case 1:
            soc_TC2COS_MAPr_field_get(unit, &reg_value, 
                PRT001_TO_QIDf, &temp);
            break;
        case 2:         
            soc_TC2COS_MAPr_field_get(unit, &reg_value, 
                PRT010_TO_QIDf, &temp);
            break;
        case 3:
            soc_TC2COS_MAPr_field_get(unit, &reg_value, 
                PRT011_TO_QIDf, &temp);
            break;
        case 4:
            soc_TC2COS_MAPr_field_get(unit, &reg_value, 
                PRT100_TO_QIDf, &temp);
            break;
        case 5:
            soc_TC2COS_MAPr_field_get(unit, &reg_value, 
                PRT101_TO_QIDf, &temp);
            break;
        case 6:         
            soc_TC2COS_MAPr_field_get(unit, &reg_value, 
                PRT110_TO_QIDf, &temp);
            break;
        case 7:
            soc_TC2COS_MAPr_field_get(unit, &reg_value, 
                PRT111_TO_QIDf, &temp);
            break;
        default:
            return SOC_E_PARAM;
    }
    *queue_n = temp;
     return SOC_E_NONE;
}

int
drv_bcm5395_queue_port_dfsv_set
    (int unit, uint8 port, uint8 dscp, uint8 prio, uint8 queue_n)
{

    int rv = SOC_E_NONE;
    uint32  reg_value;
    uint32 bmp = 0;

    /* chank port number */
    if (port > (SOC_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    if ((rv = REG_READ_QOS_EN_DIFFSERVr(unit, &reg_value)) < 0) {
        return rv;
    }

    soc_QOS_EN_DIFFSERVr_field_get(unit, &reg_value, 
        QOS_EN_DIFFSERVf, &bmp);
    bmp |= 1U << port;
    soc_QOS_EN_DIFFSERVr_field_set(unit, &reg_value, 
        QOS_EN_DIFFSERVf, &bmp);
    if ((rv = REG_WRITE_QOS_EN_DIFFSERVr(unit, &reg_value)) < 0) {
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
drv_bcm5395_queue_port_dfsv_get
    (int unit, uint8 port, uint8 dscp, uint8 *prio, uint8 *queue_n)
{

    int rv = SOC_E_NONE;
    uint32  reg_value; 
    uint32 bmp = 0;
    soc_pbmp_t pbmp;
    
    /* chank port number */
    if (port > (SOC_MAX_NUM_PORTS - 1)) {
         return SOC_E_PARAM;
    }

    if ((rv = REG_READ_QOS_EN_DIFFSERVr(unit, &reg_value)) < 0) {
        return rv;
    }

    soc_QOS_EN_DIFFSERVr_field_get(unit, &reg_value, 
        QOS_EN_DIFFSERVf, &bmp);
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
drv_bcm5395_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 mode)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, temp;
    
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mode_set: unit %d, bmp = %x, queue mode = %d\n",
        unit, SOC_PBMP_WORD_GET(bmp,0), mode);
    
    if ((rv = REG_READ_QOS_TX_CTRLr(unit, &reg_value)) < 0) {
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

    soc_QOS_TX_CTRLr_field_set(unit, &reg_value, 
        QOS_PRIORITY_CTRLf, &temp);

    if ((rv = REG_WRITE_QOS_TX_CTRLr(unit, &reg_value)) < 0) {
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
drv_bcm5395_queue_mode_get(int unit, uint32 port, uint32 *mode)
{
    int rv = SOC_E_NONE;
    uint32  reg_value, temp;

    if ((rv = REG_READ_QOS_TX_CTRLr(unit, &reg_value)) < 0) {
            return rv;
    }
    soc_QOS_TX_CTRLr_field_get(unit, &reg_value, 
        QOS_PRIORITY_CTRLf, &temp);

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
drv_bcm5395_queue_count_set(int unit, uint32 port_type, uint8 count)
{
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_count_set: unit %d, port type = %d, queue count = %d\n",
        unit, port_type, count);
    
    if (count != NUM_COS(unit)) {
        return SOC_E_PARAM;
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
drv_bcm5395_queue_count_get(int unit, uint32 port_type, uint8 *count)
{

    *count = 4;

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
drv_bcm5395_queue_WRR_weight_set(int unit, 
    uint32 port_type, uint8 queue, uint32 weight)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;
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
    
    SOC_IF_ERROR_RETURN(
        REG_READ_QOS_WEIGHTr(unit, queue, &reg_value));
    temp = weight;
    soc_QOS_WEIGHTr_field_set(unit, &reg_value, 
        WEIGHTSf, &temp);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_QOS_WEIGHTr(unit, queue, &reg_value));
    
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
drv_bcm5395_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint8 queue, uint32 *weight)
{
    uint32  reg_value;
    
    SOC_IF_ERROR_RETURN(
        REG_READ_QOS_WEIGHTr(unit, queue, &reg_value));
    
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
drv_bcm5395_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n)
{
    int i = 0, temp = 0;
    uint8 queue_t = 0;
    uint32 new_prio = 0;
    
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_prio_set: unit %d, port = %d, priority = %d, queue = %d\n",
        unit, port, prio, queue_n);

    if (port == -1) {
    /* Set 1P/1Q Prioryty map */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->queue_prio_remap_set)
            (unit, -1, prio, prio));

    /* BCM5395 only has a global priority-to-queue id mapping register */
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->queue_port_prio_to_queue_set)
                (unit, 0, prio, queue_n));
    } else {
        /* 
          * Search and get the first matched queue_n (TC2COS mapping by per-system based) 
          * with related priority value for new_prio.
          */
        for (i = 0; i < 8; i++) {
            if (!temp) {
                SOC_IF_ERROR_RETURN(
                    (DRV_SERVICES(unit)->queue_prio_get)
                        (unit, -1, i, &queue_t)); 
                if (queue_t == queue_n) {
                    new_prio = i;
                    temp = 1;
                }
            }
        }

        /* Then set the PCP2TC mapping with new_prio value by per-port based */
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->queue_prio_remap_set)
            (unit, port, prio, new_prio));
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
drv_bcm5395_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n)
{
    uint32 p;
    uint8 new_prio = 0;
    if (port == -1) {
        /* Get GE0's */
        p = 0;
        /* BCM5395 only has a global priority-to-queue id mapping register */
        SOC_IF_ERROR_RETURN(
           (DRV_SERVICES(unit)->queue_port_prio_to_queue_get)
            (unit, p, prio, queue_n));
    } else {
        p = port;
        /* Get the new_prio value from PCP2TC mapping by per-port based */
        SOC_IF_ERROR_RETURN(
           (DRV_SERVICES(unit)->queue_prio_remap_get)
            (unit, p, prio, &new_prio));

        /* Transfer the new_prio by TC2COS mapping (per-system based) */
    SOC_IF_ERROR_RETURN(
       (DRV_SERVICES(unit)->queue_port_prio_to_queue_get)
            (unit, 0, new_prio, queue_n));
    }
        
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_prio_get: unit %d, port = %d, priority = %d, queue = %d\n",
        unit, port, prio, *queue_n);
    return SOC_E_NONE;
}


int
drv_bcm5395_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio, uint8 prio)
{
    uint32  reg_addr, reg_value, temp, p;
    int     reg_len;
    soc_pbmp_t pbmp;

    SOC_PBMP_CLEAR(pbmp);
    if (port == -1) {
        SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
        SOC_PBMP_PORT_SET(pbmp, port);
    }

    PBMP_ITER(pbmp, p) {
        if (IS_GE_PORT(unit, p) || 
            (IS_FE_PORT(unit, p) && SOC_IS_ROBO53101(unit))) {
            /* For GE ports */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, INDEX(PN_PCP2TCr), p, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, INDEX(PN_PCP2TCr));
        } else { /* IMP port */
        	if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                SOC_IS_ROBO53128(unit)) {
            	reg_addr = (DRV_SERVICES(unit)->reg_addr)
                	(unit, INDEX(IMP_PCP2TCr), 0, 0);
            	reg_len = (DRV_SERVICES(unit)->reg_length_get)
                	(unit, INDEX(IMP_PCP2TCr));
            } else {
            	reg_addr = (DRV_SERVICES(unit)->reg_addr)
                	(unit, INDEX(P8_PCP2TCr), 0, 0);
            	reg_len = (DRV_SERVICES(unit)->reg_length_get)
                	(unit, INDEX(P8_PCP2TCr));
            }
        }
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_read)(unit, 
            reg_addr, &reg_value, reg_len));
        temp = (uint32 )prio;
        switch ( pre_prio ) {        
            case 0:
                soc_PN_PCP2TCr_field_set(unit, &reg_value,
                    TAG000_PRI_MAPf, &temp);
                break;
            case 1:
                soc_PN_PCP2TCr_field_set(unit, &reg_value,
                    TAG001_PRI_MAPf, &temp);
                break;
            case 2:         
                soc_PN_PCP2TCr_field_set(unit, &reg_value,
                    TAG010_PRI_MAPf, &temp);
                break;
            case 3:
                soc_PN_PCP2TCr_field_set(unit, &reg_value,
                    TAG011_PRI_MAPf, &temp);
                break;
            case 4:
                soc_PN_PCP2TCr_field_set(unit, &reg_value,
                    TAG100_PRI_MAPf, &temp);
                break;
            case 5:
                soc_PN_PCP2TCr_field_set(unit, &reg_value,
                    TAG101_PRI_MAPf, &temp);
                break;
            case 6:         
                soc_PN_PCP2TCr_field_set(unit, &reg_value,
                    TAG110_PRI_MAPf, &temp);
                break;
            case 7:
                soc_PN_PCP2TCr_field_set(unit, &reg_value,
                    TAG111_PRI_MAPf, &temp);
                break;
            default:
                return SOC_E_PARAM;
        }        
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_write)(unit, 
            reg_addr, &reg_value, reg_len));
    }
     return SOC_E_NONE;
}

int
drv_bcm5395_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, uint8 *prio)
{
    uint32  reg_value, temp = 0, p;
    soc_pbmp_t pbmp;

    SOC_PBMP_CLEAR(pbmp);
    if (port == -1) {
    /* Get GE0's PN_PCP2TC register value */
        p = 0;
    } else {
        p = port;
    }

    SOC_IF_ERROR_RETURN(
        REG_READ_PN_PCP2TCr(unit, p, &reg_value));

    switch ( pre_prio ) {        
        case 0:
            soc_PN_PCP2TCr_field_get(unit, &reg_value,
                    TAG000_PRI_MAPf, &temp);
            break;
        case 1:         
            soc_PN_PCP2TCr_field_get(unit, &reg_value,
                    TAG001_PRI_MAPf, &temp);
            break;
        case 2:         
            soc_PN_PCP2TCr_field_get(unit, &reg_value,
                    TAG010_PRI_MAPf, &temp);
            break;
        case 3:
            soc_PN_PCP2TCr_field_get(unit, &reg_value,
                    TAG011_PRI_MAPf, &temp);
            break;
        case 4:
            soc_PN_PCP2TCr_field_get(unit, &reg_value,
                    TAG100_PRI_MAPf, &temp);
            break;
        case 5:
            soc_PN_PCP2TCr_field_get(unit, &reg_value,
                    TAG101_PRI_MAPf, &temp);
            break;
        case 6:         
            soc_PN_PCP2TCr_field_get(unit, &reg_value,
                    TAG110_PRI_MAPf, &temp);
            break;
        case 7:
            soc_PN_PCP2TCr_field_get(unit, &reg_value,
                    TAG111_PRI_MAPf, &temp);
            break;
        default:
            return SOC_E_PARAM;
    }        

    *prio = temp;
     return SOC_E_NONE;

}

int
drv_bcm5395_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio)
{
    uint32  reg_addr, temp;        
    uint64  reg_value;
    int     reg_len, reg_index = 0;
    
    if (dscp < 16 ){
        reg_index = INDEX(QOS_DIFF_DSCP0r);        
    } else if ((16 <= dscp) && (dscp <  32)) {
        reg_index = INDEX(QOS_DIFF_DSCP1r);
        dscp -= 16;
    } else if((32 <= dscp) && (dscp <  48)){
        reg_index = INDEX(QOS_DIFF_DSCP2r);
        dscp -= 32;
    } else if((48 <= dscp) && (dscp <  64)){
        reg_index = INDEX(QOS_DIFF_DSCP3r);
        dscp -= 48;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, 
        reg_addr, (uint32 *)&reg_value, reg_len));

    temp = (uint32 )prio;
    switch ( dscp ) {        
        case 0:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000000f, &temp);
            break;
        case 1:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000001f, &temp);
            break;
        case 2:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000010f, &temp);
            break;
        case 3:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000011f, &temp);
            break;
        case 4:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000100f, &temp);
            break;
        case 5:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000101f, &temp);
            break;
        case 6:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000110f, &temp);
            break;
        case 7:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000111f, &temp);
            break;
        case 8:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001000f, &temp);
            break;
        case 9:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001001f, &temp);
            break;
        case 10:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001010f, &temp);
            break;
        case 11:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001011f, &temp);
            break;
        case 12:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001100f, &temp);
            break;
        case 13:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001101f, &temp);
            break;
        case 14:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001110f, &temp);
            break;
        case 15:
            soc_QOS_DIFF_DSCP0r_field_set(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001111f, &temp);
            break;            
    }

    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)(unit, 
        reg_addr, (uint32 *)&reg_value, reg_len));
    return SOC_E_NONE;
}

int
drv_bcm5395_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio)
{
    uint32  reg_addr, temp = 0;        
    uint64  reg_value;        
    int     reg_len, reg_index = 0;
    
    if (dscp < 16 ){
        reg_index = INDEX(QOS_DIFF_DSCP0r);        
    } else if ((16 <= dscp) && (dscp <  32)) {
        reg_index = INDEX(QOS_DIFF_DSCP1r);
        dscp -= 16;
    } else if((32 <= dscp) && (dscp <  48)){
        reg_index = INDEX(QOS_DIFF_DSCP2r);
        dscp -= 32;
    } else if((48 <= dscp) && (dscp <  64)){
        reg_index = INDEX(QOS_DIFF_DSCP3r);
        dscp -= 48;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_read)(unit, 
        reg_addr, (uint32 *)&reg_value, reg_len));
    
    switch ( dscp ) {        
        case 0:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000000f, &temp);
            break;
        case 1:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000001f, &temp);
            break;
        case 2:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000010f, &temp);
            break;
        case 3:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000011f, &temp);
            break;
        case 4:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000100f, &temp);
            break;
        case 5:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000101f, &temp);
            break;
        case 6:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000110f, &temp);
            break;
        case 7:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_000111f, &temp);
            break;
        case 8:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001000f, &temp);
            break;
        case 9:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001001f, &temp);
            break;
        case 10:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001010f, &temp);
            break;
        case 11:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001011f, &temp);
            break;
        case 12:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001100f, &temp);
            break;
        case 13:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001101f, &temp);
            break;
        case 14:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001110f, &temp);
            break;
        case 15:
            soc_QOS_DIFF_DSCP0r_field_get(unit, (uint32 *)&reg_value, 
                PRI_DSCP_001111f, &temp);
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
drv_bcm5395_queue_tos_set(int unit, uint8 precedence, uint8 queue_n)
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
drv_bcm5395_queue_tos_get(int unit, uint8 precedence, uint8 *queue_n)
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
drv_bcm5395_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n)
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
drv_bcm5395_queue_dfsv_get(int unit, uint8 code_point, uint8 *queue_n)
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
drv_bcm5395_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state)
{
    uint32	reg_value;
    uint32	temp;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mapping_type_set: unit %d, bmp = %x, type = %d,  %sable\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), mapping_type, state ? "en" : "dis");
    switch (mapping_type) {
    case DRV_QUEUE_MAP_NONE:
        /* set port_qos_en=1 && qos_layer_sel = 0  */
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));
        temp = PORT_BASE_QOS;
        soc_QOS_GLOBAL_CTRLr_field_set(unit, &reg_value, 
            PORT_QOS_ENf, &temp);
        temp = PRIO_QOS;
        soc_QOS_GLOBAL_CTRLr_field_set(unit, &reg_value, 
            QOS_LAYER_SELf, &temp);        
        SOC_IF_ERROR_RETURN(
            REG_WRITE_QOS_GLOBAL_CTRLr(unit, &reg_value));

        /* disable QOS_IP_EN register */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr(unit, &reg_value));
        soc_QOS_1P_ENr_field_get(unit, &reg_value,
            QOS_1P_ENf, &temp);
        temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        soc_QOS_1P_ENr_field_set(unit, &reg_value,
            QOS_1P_ENf, &temp);
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr(unit, &reg_value));

        /* disable QOS_TOS_DIF_EN register */
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_EN_DIFFSERVr(unit, &reg_value));
        soc_QOS_EN_DIFFSERVr_field_get(unit, &reg_value, 
            QOS_EN_DIFFSERVf, &temp);
        temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        soc_QOS_EN_DIFFSERVr_field_set(unit, &reg_value, 
            QOS_EN_DIFFSERVf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_QOS_EN_DIFFSERVr(unit, &reg_value));
        break;

    case DRV_QUEUE_MAP_PRIO:
        /* set port_qos_en=0 && qos_layer_sel = 0b (layer2 only) */
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));
        temp = LAYER_SEL_QOS;
        soc_QOS_GLOBAL_CTRLr_field_set(unit, &reg_value, 
            PORT_QOS_ENf, &temp);
        temp = PRIO_QOS;
        soc_QOS_GLOBAL_CTRLr_field_set(unit, &reg_value, 
            QOS_LAYER_SELf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_QOS_GLOBAL_CTRLr(unit, &reg_value));

        /* enable/disable QOS_1P_ENr */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr(unit, &reg_value));
        soc_QOS_1P_ENr_field_get(unit, &reg_value,
            QOS_1P_ENf, &temp);
        if (state) {
            temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        } else {
            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        }
        soc_QOS_1P_ENr_field_set(unit, &reg_value,
            QOS_1P_ENf, &temp);
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr(unit, &reg_value));
        /* don't need to disable QOS_TOS_DIF_ENr, 
          * the priority of QOS_1P_ENr > QOS_TOS_DIF_ENr
          * by TC Decision Tree : port_qos_en=0 && qos_layer_sel = 0b 
          */
        break;
    case DRV_QUEUE_MAP_DFSV:
        /* set port_qos_en=0 && qos_layer_sel = 10b  */
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));
        temp = LAYER_SEL_QOS;
        soc_QOS_GLOBAL_CTRLr_field_set(unit, &reg_value, 
            PORT_QOS_ENf, &temp);
        temp = IP_QOS;
        soc_QOS_GLOBAL_CTRLr_field_set(unit, &reg_value, 
            QOS_LAYER_SELf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_QOS_GLOBAL_CTRLr(unit, &reg_value));

        /* enable / diaable QOS_TOS_DIF_ENr */
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_EN_DIFFSERVr(unit, &reg_value));
        soc_QOS_EN_DIFFSERVr_field_get(unit, &reg_value, 
            QOS_EN_DIFFSERVf, &temp);
        if (state) {
            temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        } else {
            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
        }
        soc_QOS_EN_DIFFSERVr_field_set(unit, &reg_value, 
            QOS_EN_DIFFSERVf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_QOS_EN_DIFFSERVr(unit, &reg_value));
        /* don't need to disable QOS_1P_ENr, 
          * the priority of QOS_TOS_DIF_ENr > QOS_1P_ENr
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
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));
        temp = PORT_BASE_QOS;
        soc_QOS_GLOBAL_CTRLr_field_set(unit, &reg_value, 
            PORT_QOS_ENf, &temp);
        temp = ALL_QOS;
        soc_QOS_GLOBAL_CTRLr_field_set(unit, &reg_value, 
            QOS_LAYER_SELf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_QOS_GLOBAL_CTRLr(unit, &reg_value));
        
         /* enable QOS_IP_EN register */
        SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr(unit, &reg_value));
        soc_QOS_1P_ENr_field_get(unit, &reg_value,
            QOS_1P_ENf, &temp);
        temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        soc_QOS_1P_ENr_field_set(unit, &reg_value,
            QOS_1P_ENf, &temp);
        SOC_IF_ERROR_RETURN(REG_WRITE_QOS_1P_ENr(unit, &reg_value));

        /* enable QOS_TOS_DIF_EN register */
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_EN_DIFFSERVr(unit, &reg_value));
        soc_QOS_EN_DIFFSERVr_field_get(unit, &reg_value, 
            QOS_EN_DIFFSERVf, &temp);
        temp |= (SOC_PBMP_WORD_GET(bmp, 0));
        soc_QOS_EN_DIFFSERVr_field_set(unit, &reg_value, 
            QOS_EN_DIFFSERVf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_QOS_EN_DIFFSERVr(unit, &reg_value));
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
drv_bcm5395_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state)
{
    uint32	reg_value;
    uint32	temp = 0;

    switch (mapping_type)
    {
    case DRV_QUEUE_MAP_NONE:
        return SOC_E_PARAM;
    case DRV_QUEUE_MAP_PRIO:

        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));
        soc_QOS_GLOBAL_CTRLr_field_get(unit, &reg_value, 
            PORT_QOS_ENf, &temp);
        if (temp) {
            *state = FALSE;
            break;
        }
        soc_QOS_GLOBAL_CTRLr_field_get(unit, &reg_value, 
            QOS_LAYER_SELf, &temp);
        if ( temp == DFSV_QOS) {
            *state = FALSE;
            break;
        }
            
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_EN_DIFFSERVr(unit, &reg_value));
        soc_QOS_EN_DIFFSERVr_field_get(unit, &reg_value, 
            QOS_EN_DIFFSERVf, &temp);
        if (temp & (0x1 << port)) {
            *state = FALSE;
            break;
        }
        SOC_IF_ERROR_RETURN(REG_READ_QOS_1P_ENr(unit, &reg_value));
        soc_QOS_1P_ENr_field_get(unit, &reg_value,
            QOS_1P_ENf, &temp);
        if (temp & (0x1 << port)) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
     
    case DRV_QUEUE_MAP_DFSV:

        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));
        soc_QOS_GLOBAL_CTRLr_field_get(unit, &reg_value, 
            PORT_QOS_ENf, &temp);
        if (temp) {
            *state = FALSE;
            break;
        }
        soc_QOS_GLOBAL_CTRLr_field_get(unit, &reg_value, 
            QOS_LAYER_SELf, &temp);
        if ( temp == PRIO_QOS) {
            *state = FALSE;
            break;
        }
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_EN_DIFFSERVr(unit, &reg_value));
        soc_QOS_EN_DIFFSERVr_field_get(unit, &reg_value, 
            QOS_EN_DIFFSERVf, &temp);
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
        SOC_IF_ERROR_RETURN(
            REG_READ_QOS_GLOBAL_CTRLr(unit, &reg_value));
        soc_QOS_GLOBAL_CTRLr_field_get(unit, &reg_value, 
            PORT_QOS_ENf, &temp);
        if (!temp) {
            *state = FALSE;
            break;
        }
        soc_QOS_GLOBAL_CTRLr_field_get(unit, &reg_value, 
            QOS_LAYER_SELf, &temp);
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
drv_bcm5395_queue_rx_reason_set(int unit, uint8 reason, uint32 queue)
{
    uint32  reg_value, temp;

    SOC_IF_ERROR_RETURN(
        REG_READ_CPU2COS_MAPr(unit, &reg_value));

    temp = queue;
    switch (reason) {
        case DRV_RX_REASON_MIRRORING:
            if (SOC_IS_ROBO5395(unit)) {
                soc_CPU2COS_MAPr_field_set(unit, &reg_value, 
                    MIRf, &temp);
            } else if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit)) {
                soc_CPU2COS_MAPr_field_set(unit, &reg_value, 
                    MIRROR_Rf, &temp);
            } else if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                SOC_IS_ROBO53128(unit)) {
                soc_CPU2COS_MAPr_field_set(unit, &reg_value, 
                    MIRRORf, &temp);
            }
            break;
        case DRV_RX_REASON_SA_LEARNING:
            soc_CPU2COS_MAPr_field_set(unit, &reg_value, 
                    SA_LRNf, &temp);
            break;
        case DRV_RX_REASON_SWITCHING:
            soc_CPU2COS_MAPr_field_set(unit, &reg_value, 
                    SW_FLDf, &temp);
            break;
        case DRV_RX_REASON_PROTO_TERM:
            soc_CPU2COS_MAPr_field_set(unit, &reg_value, 
                    PRTC_TRMNTf, &temp);
            break;
        case DRV_RX_REASON_PROTO_SNOOP:
            soc_CPU2COS_MAPr_field_set(unit, &reg_value, 
                    PRTC_SNOOPf, &temp);
            break;
        case DRV_RX_REASON_EXCEPTION:
            soc_CPU2COS_MAPr_field_set(unit, &reg_value, 
                    EXCPT_PRCSf, &temp);
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    SOC_IF_ERROR_RETURN(
        REG_WRITE_CPU2COS_MAPr(unit, &reg_value));

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
drv_bcm5395_queue_rx_reason_get(int unit, uint8 reason, uint32 *queue)
{
    uint32  reg_value, temp = 0;

    SOC_IF_ERROR_RETURN(
        REG_READ_CPU2COS_MAPr(unit, &reg_value));
    
    switch (reason) {
        case DRV_RX_REASON_MIRRORING:
            if (SOC_IS_ROBO5395(unit)) {
                soc_CPU2COS_MAPr_field_get(unit, &reg_value, 
                    MIRf, &temp);
            } else if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit)) {
                soc_CPU2COS_MAPr_field_get(unit, &reg_value, 
                    MIRROR_Rf, &temp);
            } else if (SOC_IS_ROBO53101(unit)|| SOC_IS_ROBO53125(unit) ||
                SOC_IS_ROBO53128(unit)) {
                soc_CPU2COS_MAPr_field_get(unit, &reg_value, 
                    MIRRORf, &temp);
            }
            *queue = temp;
            break;
        case DRV_RX_REASON_SA_LEARNING:
            soc_CPU2COS_MAPr_field_get(unit, &reg_value, 
                    SA_LRNf, &temp);
            *queue = temp;
            break;
        case DRV_RX_REASON_SWITCHING:
            soc_CPU2COS_MAPr_field_get(unit, &reg_value, 
                    SW_FLDf, &temp);
            *queue = temp;
            break;
        case DRV_RX_REASON_PROTO_TERM:
            soc_CPU2COS_MAPr_field_get(unit, &reg_value, 
                    PRTC_TRMNTf, &temp);
            *queue = temp;
            break;
        case DRV_RX_REASON_PROTO_SNOOP:
            soc_CPU2COS_MAPr_field_get(unit, &reg_value, 
                    PRTC_SNOOPf, &temp);
            *queue = temp;
            break;
        case DRV_RX_REASON_EXCEPTION:
            soc_CPU2COS_MAPr_field_get(unit, &reg_value, 
                    EXCPT_PRCSf, &temp);
            *queue = temp;
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}
