/*
 * $Id: vlan.c 1.1 Broadcom SDK $
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
drv_bcm53128_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32  reg_addr, reg_value, temp;
    int reg_len, rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, 
            "%s: unit = %d, property type = %d, value = %x\n",
            FUNCTION_NAME(), unit, prop_type, prop_val);
    switch (prop_type) {
    case DRV_VLAN_PROP_VTABLE_MISS_DROP:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL5r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL5r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
             return rv;
        }
        if (prop_val) {
           temp = 1;
        } else {
           temp = 0;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, VLAN_CTRL5r, &reg_value, DROP_VTABLE_MISSf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
             return rv;
        }
        break;
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        if (prop_val) {
           temp = 0;
        } else {
           temp = 3;
        }        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        break;            
    case DRV_VLAN_PROP_SP_TAG_TPID:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, DTAG_TPIDr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, DTAG_TPIDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }

        temp = prop_val & 0xFFFF;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, DTAG_TPIDr, &reg_value, ISP_TPIDf, &temp));

        if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
            
        break;
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, VLAN_CTRL4r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, VLAN_CTRL4r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL4r, &reg_value, EN_DOUBLE_TAGf, &temp));

        if (prop_val){
            temp = 1;
        } else {
            temp = 0;
        }
        
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, VLAN_CTRL4r, &reg_value, EN_DOUBLE_TAGf, &temp));
            
        if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
                        
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
drv_bcm53128_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32  reg_addr, reg_value, temp;
    int reg_len, rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, 
            "%s: unit = %d, property type = %d, value = %x\n",
            FUNCTION_NAME(), unit, prop_type, *prop_val);
    switch (prop_type) {
    case DRV_VLAN_PROP_VTABLE_MISS_DROP:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL5r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL5r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL5r, &reg_value, DROP_VTABLE_MISSf, &temp));
        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }
        break;
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));
        
        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }
        break;            
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, VLAN_CTRL4r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, VLAN_CTRL4r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL4r, &reg_value, EN_DOUBLE_TAGf, &temp));

        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }

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
drv_bcm53128_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val)
{
    uint32 reg_addr, reg_len;
    uint32 reg_index = 0, fld_index = 0;    
    uint64  reg_value64;
    uint32 temp;
    int rv = SOC_E_NONE;
    soc_pbmp_t set_bmp, temp_bmp;

    soc_cm_debug(DK_VLAN, "%s: unit=%d, prop=%d, value=0x%x\n", 
            FUNCTION_NAME(), unit, prop_type, val);
    
    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :
        reg_index = ISP_SEL_PORTMAPr;
        fld_index  = ISP_PORTMAPf;

        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index,(uint32 *)&reg_value64, fld_index, &temp));
    
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
    
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len));

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
drv_bcm53128_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 *val)
{
    uint32 reg_addr, reg_len;
    uint32 reg_index = 0, fld_index = 0;    
    uint64  reg_value64;
    uint32 temp;
    soc_pbmp_t pbmp;
    int rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, "%s: unit=%d, prop=%d, port=%d\n", 
            FUNCTION_NAME(), unit, prop_type, port_n);

    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :
        reg_index = ISP_SEL_PORTMAPr;
        fld_index  = ISP_PORTMAPf;

        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index,(uint32 *)&reg_value64, fld_index, &temp));
    
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
    default :
        return SOC_E_UNAVAIL;

    }
    return rv;

}
