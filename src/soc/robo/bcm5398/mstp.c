/*
 * $Id: mstp.c 1.4 Broadcom SDK $
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
#include <soc/cm.h>

int bcm5398_mst_field[] = { 
                    SPT_STA0f, SPT_STA1f, SPT_STA2f, SPT_STA3f,
                    SPT_STA4f, SPT_STA5f, SPT_STA6f, SPT_STA7f,
                    SPT_STA8f };
/*
 *  Function : drv_bcm5398_mstp_port_set
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
drv_bcm5398_mstp_port_set(int unit, uint32 mstp_gid, uint32 port, uint32 port_state)
{
    int rv = SOC_E_NONE;
    uint32	temp;
    uint32  reg_len, reg_addr;
    uint32  reg32;
    uint64 reg_value;
    uint32 max_gid;
    
    soc_cm_debug(DK_STP, 
        "drv_mstp_port_set : unit %d, STP id = %d, port = %d, port_state = %d \n",
         unit, mstp_gid, port, port_state);
    (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_MSTP_NUM, &max_gid);
    if (!soc_feature(unit, soc_feature_mstp)){
           /* error checking */
        if (mstp_gid != STG_ID_DEFAULT) {
            return SOC_E_PARAM;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, G_PCTLr, port, 0);

        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, G_PCTLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg32, reg_len)) < 0) {
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
                temp = 3;
                break;
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
            (unit, G_PCTLr, &reg32, G_MISTP_STATEf, &temp);  

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg32, reg_len)) < 0) {
            return rv;
        }

        return rv;

    }else{

        /* error checking */
        if ((mstp_gid > max_gid) || (mstp_gid < STG_ID_DEFAULT)) {
            return SOC_E_PARAM;
        }
        mstp_gid = mstp_gid % max_gid;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MST_TBLr, 0, mstp_gid);

        reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MST_TBLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
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
                temp = 3;
                break;
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
            (unit, MST_TBLr, (uint32 *)&reg_value, 
            bcm5398_mst_field[port], &temp);  

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
            return rv;
        }

        return rv;
    }

}

/*
 *  Function : drv_bcm5398_mstp_port_get
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
drv_bcm5398_mstp_port_get(int unit, uint32 mstp_gid, uint32 port, uint32 *port_state)
{
    int rv = SOC_E_NONE;
    uint32	portstate;
    uint32  reg_len, reg_addr, reg32;
    uint64  reg_value;
    uint32 max_gid;
    
    (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_MSTP_NUM, &max_gid);
    if (!soc_feature(unit, soc_feature_mstp)){
        /* error checking */
        if (mstp_gid != STG_ID_DEFAULT) {
            return SOC_E_PARAM;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, G_PCTLr, port, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, G_PCTLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg32, reg_len)) < 0) {
            return rv;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, G_PCTLr, &reg32, G_MISTP_STATEf, &portstate); 
    
        switch (portstate)
        {
            case 1:
                *port_state = DRV_PORTST_DISABLE;
                break;
            case 2:
                *port_state = DRV_PORTST_BLOCK;
                break;
            case 3:
                *port_state = DRV_PORTST_LISTEN;
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
    } else {
        /* error checking */
        if ((mstp_gid > max_gid) || (mstp_gid < STG_ID_DEFAULT)) {
            return SOC_E_PARAM;
        }
        mstp_gid = mstp_gid % max_gid;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, MST_TBLr, 0, mstp_gid);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, MST_TBLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, MST_TBLr, (uint32 *)&reg_value, 
            bcm5398_mst_field[port], &portstate); 
   
        switch (portstate)
        {
            case 1:
                *port_state = DRV_PORTST_DISABLE;
                break;
            case 2:
                *port_state = DRV_PORTST_BLOCK;
                break;
            case 3:
                *port_state = DRV_PORTST_LISTEN;
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
}
