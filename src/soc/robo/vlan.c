/*
 * $Id: vlan.c 1.13.176.2 Broadcom SDK $
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

soc_pbmp_t  bcm5324_pvlan_value[SOC_MAX_NUM_PORTS];
#ifdef STP_BY_EAP_BLK
extern soc_pbmp_t stp_non_forward_pbmp;
#endif /* STP_BY_EAP_BLK */

/*
 *  Function : drv_vlan_vt_add
 *
 *  Purpose :
 *      Add the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        :  RoboSwitch unit number.
 *      vt_type     : VT table type. (ingress/egress/..) 
 *      port        : port id. 
 *      cvid         :  customer vid(= inner_vid = old_vid)
 *      sp_vid      : service provide vid( = outer_vid = new_vid)
 *      pri         : priority (not used in bcm53242)
 *      mode        : vt_mode (trasparent / mapping)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. designed as VT table is PORT+VID basis. 
 *      2. some parameter might not fully supported in current ROBO chips.
 *
 */
int 
drv_vlan_vt_add(int unit, uint32 vt_type, uint32 port, uint32 cvid, 
                uint32 sp_vid, uint32 pri, uint32 mode)
{
    
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_vlan_vt_delete
 *
 *  Purpose :
 *      Delete the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      vt_type     : VT table type. (ingress/egress/..) 
 *      port        : port id.
 *      vid         :  VLAN ID 
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. designed as VT table is PORT+VID basis. 
 *
 */
int 
drv_vlan_vt_delete(int unit, uint32 vt_type, uint32 port, uint32 vid)
{
    /* VLAN Xlat feature is highly device oriented , check each chip's folder 
     *  if supported 
     */
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_vlan_vt_delete_all
 *
 *  Purpose :
 *      Delete all the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      vt_type     : VT table type. (ingress/egress/..) 
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *
 */
int 
drv_vlan_vt_delete_all(int unit, uint32 vt_type)
{
    /* VLAN Xlat feature is highly device oriented , check each chip's folder 
     *  if supported 
     */
    return SOC_E_UNAVAIL;
}

/*
 *  Function : drv_vlan_vt_set
 *
 *  Purpose :
 *      Set the VLAN translation property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      vid         :  VLAN ID 
 *      port        : port id.
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_vlan_vt_set(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 prop_val)
{
    /* VLAN Xlat feature is highly device oriented , check each chip's folder 
     *  if supported 
     */
    return SOC_E_UNAVAIL;
}


/*
 *  Function : drv_vlan_vt_get
 *
 *  Purpose :
 *      Get the VLAN translation property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      vid         :  VLAN ID 
 *      port        : port id.
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_vlan_vt_get(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 *prop_val)
{
    /* VLAN Xlat feature is highly device oriented , check each chip's folder 
     *   if supported 
     */
    return SOC_E_UNAVAIL;
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
 *      1. new driver serivce for BCM53242 currently only. 
 *
 */
int 
drv_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val)
{
    /* check each chip's folder if suitable on supporting */
    return SOC_E_UNAVAIL;
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
 *      1. if port_n = 0xffffffff (int = -1), means get device basis value.
 *      2. new driver serivce for BCM53242 currently only. 

 *
 */
int 
drv_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 *val)
{
    /* check each chip's folder if suitable on supporting */
    return SOC_E_UNAVAIL;
}


/*
 *  Function : drv_vlan_mode_set
 *
 *  Purpose :
 *      Set the VLAN mode. (port-base/tag-base)
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      mode   :   vlan mode.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_vlan_mode_set(int unit, uint32 mode)
{
    uint32	reg_value, temp;

    soc_cm_debug(DK_VLAN, 
        "drv_vlan_mode_set: unit = %d, mode = %d\n",
        unit, mode);
    switch (mode) 
    {
        case DRV_VLAN_MODE_TAG:
            /* set 802.1Q VLAN Enable */
            SOC_IF_ERROR_RETURN(
                REG_READ_VLAN_CTRL0r(unit, &reg_value));
            temp = 1;
            soc_VLAN_CTRL0r_field_set(unit, &reg_value, 
                VLAN_ENf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_VLAN_CTRL0r(unit, &reg_value));
            if (SOC_IS_ROBO5324(unit)) {
                if (soc_feature(unit, soc_feature_mstp)){
                    /* enable reserved BPDU tagging :
                     *  - Untagged BPDU won't sent to CPU without set this bit.
                     *    (for no tagging)
                     */
                    SOC_IF_ERROR_RETURN(
                        REG_READ_VLAN_CTRL1r(unit, &reg_value));

                    temp = 1;
                    soc_VLAN_CTRL1r_field_set(unit, &reg_value, 
                        EN_RSV_MCAST_V_TAGf, &temp);
                    SOC_IF_ERROR_RETURN(
                        REG_WRITE_VLAN_CTRL1r(unit, &reg_value));
                }                
            }
            if (SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5348(unit) ||
                SOC_IS_ROBO5347(unit)) {
                /* enable GMRP/GVRP tagging :
                 *  - GMRP/GVRP frame won't sent to CPU without set this bit
                 *    (for no tagging).
                 */
                SOC_IF_ERROR_RETURN(
                    REG_READ_VLAN_CTRL2r(unit, &reg_value));
                temp = 1;
                soc_VLAN_CTRL2r_field_set(unit, &reg_value, 
                    EN_GMRP_GVRP_V_TAGf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_VLAN_CTRL2r(unit, &reg_value));
            }                
            /* enable GMRP/GVRP been sent to CPU :
             *  - GMRP/GVRP frame won't sent to CPU without set this bit.
             */
            SOC_IF_ERROR_RETURN(
                REG_READ_VLAN_CTRL4r(unit, &reg_value));
            temp = 1; /* Drop frame if ingress vid violation */
            soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                INGR_VID_CHKf, &temp);

            temp = 1;
            soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                EN_MGE_REV_GVRPf, &temp);
            soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                EN_MGE_REV_GMRPf, &temp);
            
            SOC_IF_ERROR_RETURN(
                REG_WRITE_VLAN_CTRL4r(unit, &reg_value));
                
            break;
        case DRV_VLAN_MODE_PORT_BASE:
            /* set 802.1Q VLAN Disable */
            SOC_IF_ERROR_RETURN(
                REG_READ_VLAN_CTRL0r(unit, &reg_value));
            temp = 0;
            soc_VLAN_CTRL0r_field_set(unit, &reg_value, 
                VLAN_ENf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_VLAN_CTRL0r(unit, &reg_value));
            break;
        default :
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_vlan_mode_get
 *
 *  Purpose :
 *      Get the VLAN mode. (port-base/tag-base)
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      mode   :   vlan mode.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_vlan_mode_get(int unit, uint32 *mode)
{
    uint32	reg_value, temp = 0;

    SOC_IF_ERROR_RETURN(
        REG_READ_VLAN_CTRL0r(unit, &reg_value));
    soc_VLAN_CTRL0r_field_get(unit, &reg_value, 
                VLAN_ENf, &temp);
    if (temp) {
        *mode = DRV_VLAN_MODE_TAG;
    } else {
        *mode = DRV_VLAN_MODE_PORT_BASE;
    }
    soc_cm_debug(DK_VLAN, 
        "drv_vlan_mode_get: unit = %d, mode = %d\n",
        unit, *mode);
    return SOC_E_NONE;
}

/* config port base vlan */
/*
 *  Function : drv_port_vlan_pvid_set
 *
 *  Purpose :
 *      Set the default tag value of the selected port.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      vid     :   vlan value.
 *      prio    :   priority value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_port_vlan_pvid_set(int unit, uint32 port, uint32 outer_tag, uint32 inner_tag)
{
    uint32	reg_value;

    soc_cm_debug(DK_VLAN, 
    "drv_port_vlan_pvid_set: unit = %d, port = %d, outer_tag = 0x%x, inner_tag = 0x%x\n",
        unit, port, outer_tag, inner_tag);
    reg_value = outer_tag;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_DEFAULT_1Q_TAGr(unit, port, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_port_vlan_pvid_get
 *
 *  Purpose :
 *      Get the default tag value of the selected port.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      vid     :   vlan value.
 *      prio    :   priority value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_port_vlan_pvid_get(int unit, uint32 port, uint32 *outer_tag, uint32 *inner_tag)
{
    uint32	reg_value;

    SOC_IF_ERROR_RETURN(
        REG_READ_DEFAULT_1Q_TAGr(unit, port, &reg_value));

    *outer_tag = reg_value;
    
    soc_cm_debug(DK_VLAN, 
    "drv_port_vlan_pvid_get: unit = %d, port = %d, outer_tag = 0x%x, inner_tag = 0x%x\n",
        unit, port, *outer_tag, *inner_tag);
    return SOC_E_NONE;
}

/*
 *  Function : drv_port_vlan_set
 *
 *  Purpose :
 *      Set the group member ports of the selected port. (port-base)
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      bmp     :   group member port bitmap.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_port_vlan_set(int unit, uint32 port, soc_pbmp_t bmp)
{
    uint32	reg_value, temp;
    uint64  reg_value64, temp64;
#ifdef STP_BY_EAP_BLK   
    soc_pbmp_t mod_bmp;
#endif /* STP_BY_EAP_BLK */    

    soc_cm_debug(DK_VLAN, 
    "drv_port_vlan_set: unit = %d, port = %d, bmp = %x\n",
        unit, port, SOC_PBMP_WORD_GET(bmp, 0));
    if (SOC_IS_ROBO5348(unit)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_VLAN_CTLr(unit, port, (uint32 *)&reg_value64));
        soc_robo_64_pbmp_to_val(unit, &bmp, &temp64);
        soc_PORT_VLAN_CTLr_field_set(unit, (uint32 *)&reg_value64,
            PORT_EGRESS_ENf, (uint32 *)&temp64);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_PORT_VLAN_CTLr(unit, port, (uint32 *)&reg_value64));
    }else if (SOC_IS_ROBO5347(unit)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_VLAN_CTLr(unit, port, (uint32 *)&reg_value64));
        temp = SOC_PBMP_WORD_GET(bmp, 0);
        soc_PORT_VLAN_CTLr_field_set(unit, (uint32 *)&reg_value64,
            PORT_EGRESS_ENf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_PORT_VLAN_CTLr(unit, port, (uint32 *)&reg_value64));
    }else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_EGCTLr(unit, port, (uint32 *)&reg_value64));
        temp = SOC_PBMP_WORD_GET(bmp, 0);
        soc_PORT_EGCTLr_field_set(unit, (uint32 *)&reg_value64, 
            PORT_EGRESS_ENf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_PORT_EGCTLr(unit, port, (uint32 *)&reg_value64));
    } else {
        /* bcm53115 can work in following section */
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_VLAN_CTLr(unit, port, &reg_value));
#ifdef STP_BY_EAP_BLK
        SOC_PBMP_ASSIGN(mod_bmp, bmp);
        SOC_PBMP_REMOVE(mod_bmp, stp_non_forward_pbmp);
        temp = SOC_PBMP_WORD_GET(mod_bmp, 0);
        if (SOC_IS_ROBO5324(unit)) {
            SOC_PBMP_ASSIGN(bcm5324_pvlan_value[port], mod_bmp);
        }
#else /* !STP_BY_EAP_BLK */
        temp = SOC_PBMP_WORD_GET(bmp, 0);
        if (SOC_IS_ROBO5324(unit)) {
            SOC_PBMP_ASSIGN(bcm5324_pvlan_value[port], bmp);
        }
#endif /* STP_BY_EAP_BLK */
        soc_PORT_VLAN_CTLr_field_set(unit, &reg_value,
            PORT_EGRESS_ENf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_PORT_VLAN_CTLr(unit, port, &reg_value));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_port_vlan_get
 *
 *  Purpose :
 *      Get the group member ports of the selected port. (port-base)
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      bmp     :   group member port bitmap.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_port_vlan_get(int unit, uint32 port, soc_pbmp_t *bmp)
{
    uint32	reg_value, temp = 0;
    uint64  reg_value64, temp64;

    if (SOC_IS_ROBO5348(unit)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_VLAN_CTLr(unit, port, (uint32 *)&reg_value64));
        COMPILER_64_ZERO(temp64);
        soc_PORT_VLAN_CTLr_field_get(unit, (uint32 *)&reg_value64,
            PORT_EGRESS_ENf, (uint32 *)&temp64);
        soc_robo_64_val_to_pbmp(unit, bmp, temp64);
    } else if (SOC_IS_ROBO5347(unit)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_VLAN_CTLr(unit, port, (uint32 *)&reg_value64));
        soc_PORT_VLAN_CTLr_field_get(unit, (uint32 *)&reg_value64,
            PORT_EGRESS_ENf, &temp);
        SOC_PBMP_WORD_SET(*bmp, 0, temp);
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_EGCTLr(unit, port, (uint32 *)&reg_value64));
        soc_PORT_EGCTLr_field_get(unit, (uint32 *)&reg_value64, 
            PORT_EGRESS_ENf, &temp);
        SOC_PBMP_WORD_SET(*bmp, 0, temp);
    } else {
        if (SOC_IS_ROBO5324(unit)) {
            SOC_PBMP_ASSIGN(*bmp, bcm5324_pvlan_value[port]);
        } else {
            /* bcm53115 can work in following section */
            SOC_IF_ERROR_RETURN(
                REG_READ_PORT_VLAN_CTLr(unit, port, &reg_value));
            soc_PORT_VLAN_CTLr_field_get(unit, &reg_value,
                PORT_EGRESS_ENf, &temp);
            SOC_PBMP_WORD_SET(*bmp, 0, temp);
         }
    }
    soc_cm_debug(DK_VLAN, 
    "drv_port_vlan_get: unit = %d, port = %d, bmp = %x\n",
        unit, port, SOC_PBMP_WORD_GET(*bmp, 0));
    return SOC_E_NONE;
}


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
 *  Note :
 *      
 *
 */
int 
drv_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32  reg_addr, reg_value, temp;
    int reg_len, rv = SOC_E_NONE;

    soc_cm_debug(DK_VLAN, 
    "drv_vlan_prop_set: unit = %d, property type = %d, value = %x\n",
        unit, prop_type, prop_val);
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
 *
 */
int 
drv_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32  reg_addr, reg_value, temp;
    int reg_len, rv = SOC_E_NONE;

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
        default:
            rv = SOC_E_UNAVAIL;
    }
    soc_cm_debug(DK_VLAN, 
    "drv_vlan_prop_get: unit = %d, property type = %d, value = %x\n",
        unit, prop_type, *prop_val);
    return rv;
}
