/*
 * $Id: vlan.c 1.6.122.2 Broadcom SDK $
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
 *  Function : drv_vlan_prop_set
 *
 *  Purpose :
 *      Set the VLAN property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 */
int 
drv_bcm53101_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32  reg_value, temp;
    int rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, 
            "%s: unit = %d, property type = %d, value = %x\n",
            FUNCTION_NAME(), unit, prop_type, prop_val);
    switch (prop_type) {
    case DRV_VLAN_PROP_VTABLE_MISS_DROP:
        if ((rv = REG_READ_VLAN_CTRL5r(unit, &reg_value)) < 0) {
             return rv;
        }
        if (prop_val) {
           temp = 1;
        } else {
           temp = 0;
        }
        soc_VLAN_CTRL5r_field_set(unit, &reg_value,
            DROP_VTABLE_MISSf, &temp);
        if ((rv = REG_WRITE_VLAN_CTRL5r(unit, &reg_value)) < 0) {
             return rv;
        }
        break;
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        if ((rv = REG_READ_VLAN_CTRL0r(unit, &reg_value)) < 0) {
             return rv;
        }
        if (prop_val) {
           temp = 0;
        } else {
           temp = 3;
        }
        soc_VLAN_CTRL0r_field_set(unit, &reg_value,
            VLAN_LEARN_MODEf, &temp);

        if ((rv = REG_WRITE_VLAN_CTRL0r(unit, &reg_value)) < 0) {
             return rv;
        }
        break;            
    case DRV_VLAN_PROP_SP_TAG_TPID:
        if ((rv = REG_READ_DTAG_TPIDr(unit, &reg_value)) < 0) {
            return rv;
        }

        temp = prop_val & 0xFFFF;
        soc_DTAG_TPIDr_field_set(unit, &reg_value,
            ISP_TPIDf, &temp);

        if ((rv = REG_WRITE_DTAG_TPIDr(unit, &reg_value)) < 0) {
            return rv;
        }
            
        break;
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
        if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }

        if (prop_val){
            temp = 1;
        } else {
            temp = 0;
        }

        soc_VLAN_CTRL4r_field_set(unit, &reg_value,
            EN_DOUBLE_TAGf, &temp);
            
        if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
                        
        break;
    case DRV_VLAN_PROP_TRANSLATE_MODE:
    case DRV_VLAN_PROP_BYPASS_IGMP_MLD:
    case DRV_VLAN_PROP_BYPASS_ARP_DHCP:
    case DRV_VLAN_PROP_BYPASS_MIIM:
    case DRV_VLAN_PROP_BYPASS_MCAST:
    case DRV_VLAN_PROP_BYPASS_RSV_MCAST:
    case DRV_VLAN_PROP_BYPASS_L2_USER_ADDR:
        rv = SOC_E_UNAVAIL;
        break;
    default:
        rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}


/*
 *  Function : drv_vlan_prop_get
 *
 *  Purpose :
 *      Get the VLAN property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *
 */
int 
drv_bcm53101_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32  reg_value, temp = 0;
    int rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, 
            "%s: unit = %d, property type = %d, value = %x\n",
            FUNCTION_NAME(), unit, prop_type, *prop_val);
    switch (prop_type) {
    case DRV_VLAN_PROP_VTABLE_MISS_DROP:
        if ((rv = REG_READ_VLAN_CTRL5r(unit, &reg_value)) < 0) {
             return rv;
        }
        soc_VLAN_CTRL5r_field_get(unit, &reg_value,
            DROP_VTABLE_MISSf, &temp);
        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }
        break;
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        if ((rv = REG_READ_VLAN_CTRL0r(unit, &reg_value)) < 0) {
             return rv;
        }
        soc_VLAN_CTRL0r_field_get(unit, &reg_value,
            VLAN_LEARN_MODEf, &temp);
        
        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }
        break;            
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
        if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
        soc_VLAN_CTRL4r_field_get(unit, &reg_value,
            EN_DOUBLE_TAGf, &temp);

        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }

        break;
    case DRV_VLAN_PROP_TRANSLATE_MODE:
    case DRV_VLAN_PROP_BYPASS_IGMP_MLD:
    case DRV_VLAN_PROP_BYPASS_ARP_DHCP:
    case DRV_VLAN_PROP_BYPASS_MIIM:
    case DRV_VLAN_PROP_BYPASS_MCAST:
    case DRV_VLAN_PROP_BYPASS_RSV_MCAST:
    case DRV_VLAN_PROP_BYPASS_L2_USER_ADDR:
        rv = SOC_E_UNAVAIL;
        break;
    default:
        rv = SOC_E_UNAVAIL;
        break;
    }
    
    return rv;
}

/*
 *  Function : drv_vlan_prop_port_enable_set
 *
 *  Purpose :
 *      Set the port enable status by different VLAN property.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_val    :   vlan property value.
 *      bmp         : port bitmap
 *      val         : value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *
 */
int 
drv_bcm53101_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val)
{
    uint32  reg_value;
    uint32 temp = 0;
    int rv = SOC_E_NONE;
    soc_pbmp_t set_bmp, temp_bmp;
    int port;

    soc_cm_debug(DK_VLAN, "%s: unit=%d, prop=%d, value=0x%x\n", 
            FUNCTION_NAME(), unit, prop_type, val);
    
    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :

        SOC_IF_ERROR_RETURN(
            REG_READ_ISP_SEL_PORTMAPr(unit, &reg_value));
        soc_ISP_SEL_PORTMAPr_field_get(unit, &reg_value, 
            ISP_PORTMAPf, &temp);
    
        SOC_PBMP_CLEAR(temp_bmp);
        SOC_PBMP_WORD_SET(temp_bmp, 0, temp);
        
        /* check the action process */
        SOC_PBMP_CLEAR(set_bmp);
        SOC_PBMP_OR(set_bmp, temp_bmp);
        
        if (val == TRUE){       /* set for enable */
            SOC_PBMP_OR(set_bmp, bmp);
        }else {
            SOC_PBMP_REMOVE(set_bmp, bmp);
        }

        /* check if the set value is equal to current setting */
        if (SOC_PBMP_EQ(temp_bmp, set_bmp)){
            /* do nothing */
            return SOC_E_NONE;
        }
    
        /* write to register */
        temp = SOC_PBMP_WORD_GET(set_bmp, 0);    

        soc_ISP_SEL_PORTMAPr_field_set(unit, &reg_value, 
            ISP_PORTMAPf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_ISP_SEL_PORTMAPr(unit, &reg_value));

        break;
    case DRV_VLAN_PROP_POLICING:
        if (val == TRUE) {
            temp = 1;
        } else {
            temp = 0;
        }
        PBMP_ITER(bmp, port) {
            if (IS_CPU_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_BC_SUP_RATECTRL_IMPr(unit, 0, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(
                    REG_READ_BC_SUP_RATECTRL_Pr(unit, port, &reg_value));
            }
            soc_BC_SUP_RATECTRL_Pr_field_set(unit, &reg_value, 
                EN_VLAN_POLICINGf, &temp);
            if (IS_CPU_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_BC_SUP_RATECTRL_IMPr(unit, 0, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_BC_SUP_RATECTRL_Pr(unit, port, &reg_value));
            }
        }
        /* Unmask the packet type */
        if (temp) {
            SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET(
                unit, PBMP_ALL(unit), DRV_RATE_CONFIG_PKT_MASK, 0x0));
        }
        break;
    default :
        return SOC_E_UNAVAIL;
        break;

    }
     
    return rv;
}

/*
 *  Function : drv_vlan_prop_port_enable_get
 *
 *  Purpose :
 *      Get the port enable status by different VLAN property.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_val    :   vlan property value.
 *      port_n      : port number. 
 *      val         : (OUT) value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. if port_n = 0xffffffff , means get device basis value.

 *
 */
int 
drv_bcm53101_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 *val)
{
    uint32  reg_value = 0;
    uint32 temp = 0;
    soc_pbmp_t pbmp;
    int rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, "%s: unit=%d, prop=%d, port=%d\n", 
            FUNCTION_NAME(), unit, prop_type, port_n);

    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :
        SOC_IF_ERROR_RETURN(
            REG_READ_ISP_SEL_PORTMAPr(unit, &reg_value));
        soc_ISP_SEL_PORTMAPr_field_get(unit, &reg_value, 
            ISP_PORTMAPf, &temp);
    
        /* check if the value get is port basis or device basis. */
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_WORD_SET(pbmp, 0, temp);
        
        if (port_n == 0xffffffff) {     /* device basis */
            int     i;
    
            for (i = 0; i < SOC_PBMP_WORD_MAX; i++){
                *(val + i) = SOC_PBMP_WORD_GET(pbmp, i);
            }
        } else {
            *val = (SOC_PBMP_MEMBER(pbmp, port_n)) ? TRUE : FALSE;
        }
    
        break;
    case DRV_VLAN_PROP_POLICING:
        if (IS_CPU_PORT(unit, port_n)) {
            SOC_IF_ERROR_RETURN(
                REG_READ_BC_SUP_RATECTRL_IMPr(unit, 0, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(
                REG_READ_BC_SUP_RATECTRL_Pr(unit, port_n, &reg_value));
        }
        soc_BC_SUP_RATECTRL_Pr_field_get(unit, &reg_value, 
                EN_VLAN_POLICINGf, &temp);
        if (temp) {
            *val = TRUE;
        } else {
            *val = FALSE;
        }
        break;
    default :
        return SOC_E_UNAVAIL;

    }
    return rv;

}
