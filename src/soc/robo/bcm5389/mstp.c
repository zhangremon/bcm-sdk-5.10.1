/*
 * $Id: mstp.c 1.3 Broadcom SDK $
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
 *  Function : drv_mstp_config_set
 *
 *  Purpose :
 *      Set the STP id to a specific VLAN.
 *
 *  Parameters :
 *      unit        :   unit id
 *      vlan_id   :   vlan id.
 *      mstp_gid : multiple spanning tree id.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5389_mstp_config_set(int unit, uint32 vlan_id, uint32 mstp_gid)
{

    soc_cm_debug(DK_STP, 
        "drv_mstp_config_set : unit %d, vlan = %d, STP id = %d \n",
         unit, vlan_id, mstp_gid);
    /* error checking */
    if (vlan_id > 4095) {
        return SOC_E_PARAM;
    }
    if (mstp_gid != STG_ID_DEFAULT) {
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_mstp_config_Get
 *
 *  Purpose :
 *      Get the STP id to a specific VLAN.
 *
 *  Parameters :
 *      unit        :   unit id
 *      vlan_id   :   vlan id.
 *      mstp_gid : multiple spanning tree id.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5389_mstp_config_get(int unit, uint32 vlan_id, uint32 *mstp_gid)
{

    /* error checking */
    if (vlan_id > 4095) {
        return SOC_E_PARAM;
    }

    *mstp_gid = 1;

    return SOC_E_NONE;
}

/*
 *  Function : drv_mstp_port_set
 *
 *  Purpose :
 *      Set the port state of a selected stp id.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mstp_gid : multiple spanning tree id.
 *      port    :   port number.
 *      port_state  :   state of the port.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5389_mstp_port_set(int unit, uint32 mstp_gid, uint32 port, uint32 port_state)
{
    int rv = SOC_E_NONE;
    uint32  temp;
    uint32  reg_len, reg_addr;
    uint32 reg_value;
    soc_cm_debug(DK_STP, 
    "drv_mstp_port_set : unit %d, STP id = %d, port = %d, port_state = %d \n",
         unit, mstp_gid, port, port_state);
    /* error checking */
    if (mstp_gid != STG_ID_DEFAULT) {
        return SOC_E_PARAM;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, G_PCTLr, port, 0);

    reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, G_PCTLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }    

    switch (port_state)
    {
        case DRV_PORTST_DISABLE:
            temp = 1;
            break;
        case DRV_PORTST_BLOCK:
            temp = 2;
            break;
        case DRV_PORTST_LISTEN:
        case DRV_PORTST_LEARN:
            temp = 4;
            break;
        case DRV_PORTST_FORWARD:
            temp = 5;
            break;
        default:
            return SOC_E_PARAM;
    }
    

    (DRV_SERVICES(unit)->reg_field_set)
        (unit, G_PCTLr, &reg_value, G_MISTP_STATEf, &temp);  

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }


    return rv;
}

/*
 *  Function : drv_mstp_port_get
 *
 *  Purpose :
 *      Get the port state of a selected stp id.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mstp_gid : multiple spanning tree id.
 *      port    :   port number.
 *      port_state  :   state of the port.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5389_mstp_port_get(int unit, uint32 mstp_gid, uint32 port, uint32 *port_state)
{
   int rv = SOC_E_NONE;
    uint32  portstate;
    uint32  reg_len, reg_addr, reg_value;


    /* error checking */
    if (mstp_gid != STG_ID_DEFAULT) {
        return SOC_E_PARAM;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, G_PCTLr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, G_PCTLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    (DRV_SERVICES(unit)->reg_field_get)
        (unit, G_PCTLr, &reg_value, G_MISTP_STATEf, &portstate); 
    
    switch (portstate)
    {
        case 1:
            *port_state = DRV_PORTST_DISABLE;
            break;
        case 2:
            *port_state = DRV_PORTST_BLOCK;
            break;
        case 4:
            *port_state = DRV_PORTST_LEARN;
            break;
        case 5:
            *port_state = DRV_PORTST_FORWARD;
            break;
        default:
        return SOC_E_INTERNAL;
    }
    soc_cm_debug(DK_STP, 
    "drv_mstp_port_get : unit %d, STP id = %d, port = %d, port_state = %d \n",
         unit, mstp_gid, port, *port_state);

    return SOC_E_NONE;
}
