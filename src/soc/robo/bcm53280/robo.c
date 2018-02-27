/*
 * $Id: robo.c 1.10 Broadcom SDK $
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
 *
 * File:        robo.c
 * Purpose:
 * Requires:
 */
#include <shared/switch.h>
#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/mcm/robo/driver.h>
#include <soc/error.h>
#include "robo_53280.h"

#include <soc/debug.h>

#ifdef BCM_TB_SUPPORT

/*
 * Function: 
 *	    drv_dev_control_set
 * Purpose:
 *      Set the system basis management or control functions. Especially for 
 *      those functions which provides a simple on/off or enable/disable 
 *      option to control device's working feature.
 *      Also, this driver interface provide some kinds of flexibility for 
 *      designer on implementing the device management mode configuration 
 *      instead of creating a individual driver interface.
 * Parameters:
 *      ctrl_cnt    - (IN/OUT) the number count to indicate how many pairs of
 *                      control types and values.
 *                      >> Output the number of proceeded control set items.
 *      type_list   - (IN) control type list
 *      value_list  - (IN) control value list
 * Returns:
 *	    
 * Notes:
 *  1. the contorl type and value of the same list index must be matched as 
 *      a pair.
 */
int
drv_bcm53280_dev_control_set(int unit,uint32 *ctrl_cnt,
                uint32 *type_list,int *value_list)
{
    uint32  op_cnt = 0;
    uint32  reg_value = 0, fld_valud = 0;
    int     i, rv = SOC_E_NONE;

    /* Null check */
    if (op_cnt){
        assert(type_list && value_list);
    }
    
    for (i = 0; i < *ctrl_cnt; i++){
        fld_valud = (value_list[i] == 0) ? 0 : 1;
        if (type_list[i] == DRV_DEV_CTRL_MCASTREP){
            /* set the vport mcast replication feature enable status */
            rv = REG_READ_GMNGCFGr(unit, &reg_value);
            rv |= soc_GMNGCFGr_field_set(unit, &reg_value, 
                    EN_VPORT_REPLICATIONf, &fld_valud);           
            rv |= REG_WRITE_GMNGCFGr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_L2_USERADDR){
            /* on/off the L2 mulport vector feature(l2 user address) */
            rv = REG_READ_GARLCFGr(unit, &reg_value);
            rv |= soc_GARLCFGr_field_set(unit, &reg_value, 
                    MPADDR_ENf, &fld_valud);
            rv |= REG_WRITE_GARLCFGr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_RATE_METER_PLUS_IPG){
            /* rate meter operation includ or exclude IPG+Preamble(12+8 bytes)
             *  - 0 means IPG+Preamble will be excluded.
             *  - 1 means IPG+Preamble will be included.
             */
            rv = REG_READ_NEW_CONTROLr(unit, &reg_value);
            rv |= soc_NEW_CONTROLr_field_set(unit, &reg_value, 
                    WIRE_RATE_MODEf, &fld_valud);
            rv |= REG_WRITE_NEW_CONTROLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_DA_ALL0_DROP){
            /* drop the packet within SourceMAC=00-00-00-00-00-00 */
            rv = REG_READ_SPECIAL_MNGTr(unit, &reg_value);
            rv |= soc_SPECIAL_MNGTr_field_set(unit, &reg_value, 
                    EN_ALL0_DA_DROPf, &fld_valud);
            rv |= REG_WRITE_SPECIAL_MNGTr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_ARPDHCP_TOCPU){
            /* forward ARP/DHCP to CPU */
            rv = REG_READ_SPECIAL_MNGTr(unit, &reg_value);
            rv |= soc_SPECIAL_MNGTr_field_set(unit, &reg_value, 
                    PASS_ARP_DHCPf, &fld_valud);
            rv |= REG_WRITE_SPECIAL_MNGTr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_IGMP_MLD_SNOOP_MODE){
            /* set IGMP/MLD snooping mode diable/CPU_only/CopyCPU */
            fld_valud = value_list[i] ;
            if (fld_valud >= DRV_SNOOP_MODE_CNT) {
                rv = SOC_E_PARAM;
            } else {
                /* translate to real register value */
                fld_valud = (fld_valud == DRV_SNOOP_MODE_CPU_SNOOP) ? 0x3 :
                        ((fld_valud == DRV_SNOOP_MODE_CPU_TRAP) ? 0x1 : 0);
                rv = REG_READ_GMNGCFGr(unit, &reg_value);
                rv |= soc_GMNGCFGr_field_set(unit, &reg_value, 
                        IGMP_MLD_CHKf, &fld_valud);
                rv |= REG_WRITE_GMNGCFGr(unit, &reg_value);
            }
        } else if (type_list[i] == DRV_DEV_CTRL_MCAST_SA_DROP){
            /* drop the packet within SourceMAC is Mcast MAC */
            rv = REG_READ_SPECIAL_MNGTr(unit, &reg_value);
            rv |= soc_SPECIAL_MNGTr_field_set(unit, &reg_value, 
                    EN_MCST_SA_DROPf, &fld_valud);
            rv |= REG_WRITE_SPECIAL_MNGTr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_RANGE_ERROR_DROP){
            /* drop the packet if range error */
            rv = REG_READ_MAC_FM_DROP_CTLr(unit, &reg_value);
            rv |= soc_MAC_FM_DROP_CTLr_field_set(unit, &reg_value, 
                    EN_RANGE_ERR_DROPf, &fld_valud);
            rv |= REG_WRITE_MAC_FM_DROP_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_JUMBO_FRAME_DROP){
            /* drop the Jumbo packet */
            rv = REG_READ_MAC_FM_DROP_CTLr(unit, &reg_value);
            rv |= soc_MAC_FM_DROP_CTLr_field_set(unit, &reg_value, 
                    EN_JUMBO_FM_DROPf, &fld_valud);
            rv |= REG_WRITE_MAC_FM_DROP_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_DROPPED_RANGE_ERR_NO_LEARN){
            /* Disable learning of the dropped ranger error packet */
            rv = REG_READ_MAC_FM_DROP_CTLr(unit, &reg_value);
            rv |= soc_MAC_FM_DROP_CTLr_field_set(unit, &reg_value, 
                    DIS_LRN_RANGE_ERRf, &fld_valud);
            rv |= REG_WRITE_MAC_FM_DROP_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_DROPPED_JUMBO_FRM_NO_LEARN){
            /* Disable learning of the dropped jumbo packet */
            rv = REG_READ_MAC_FM_DROP_CTLr(unit, &reg_value);
            rv |= soc_MAC_FM_DROP_CTLr_field_set(unit, &reg_value, 
                    DIS_LRN_JUMBO_FM_DROPf, &fld_valud);
            rv |= REG_WRITE_MAC_FM_DROP_CTLr(unit, &reg_value);

        /* for Bypass related */            
        } else if (type_list[i] == DRV_DEV_CTRL_RX_BYPASS_CRCCHK){
            rv = REG_READ_GMNGCFGr(unit, &reg_value);
            rv |= soc_GMNGCFGr_field_set(unit, &reg_value, 
                    EN_CPU_RX_BYPASS_CRCCHKf, &fld_valud);
            rv |= REG_WRITE_GMNGCFGr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_DOS_BYPASS_TOCPU){
            rv = REG_READ_DOS_ATTACK_FILTER_DROP_CTLr(unit, &reg_value);
            rv |= soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(unit, &reg_value, 
                    CPUCOPY_BYPASS_DOSf, &fld_valud);
            rv |= REG_WRITE_DOS_ATTACK_FILTER_DROP_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_STP_BYPASS_USERADDR){
            rv = REG_READ_STP_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_STP_BYPASS_CTLr_field_set(unit, 
                    &reg_value, STP_BYPAS_L2_USER_ADDRf, &fld_valud);
            rv |= REG_WRITE_STP_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_STP_BYPASS_MAC0X){
            rv = REG_READ_STP_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_STP_BYPASS_CTLr_field_set(unit, 
                    &reg_value, STP_BYPAS_MAC0Xf, &fld_valud);
            rv |= REG_WRITE_STP_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_USERADDR){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_set(unit, 
                    &reg_value, EAP_BYPASS_L2_USER_ADDRf, &fld_valud);
            rv |= REG_WRITE_SECURITY_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_DHCP){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_set(unit, 
                    &reg_value, EAP_BYPASS_DHCPf,  &fld_valud);
            rv |= REG_WRITE_SECURITY_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_ARP){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_set(unit, 
                    &reg_value, EAP_BYPASS_ARPf, &fld_valud);
            rv |= REG_WRITE_SECURITY_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_22_2F){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_set(unit, 
                    &reg_value, EAP_BYPASS_MAC_22_2Ff, &fld_valud);
            rv |= REG_WRITE_SECURITY_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_21){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_set(unit, 
                    &reg_value, EAP_BYPASS_MAC_21f, &fld_valud);
            rv |= REG_WRITE_SECURITY_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_20){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_set(unit, 
                    &reg_value, EAP_BYPASS_MAC_20f, &fld_valud);
            rv |= REG_WRITE_SECURITY_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_11_1F){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_set(unit, 
                &reg_value, EAP_BYPASS_MAC_11_1Ff, &fld_valud);
            rv |= REG_WRITE_SECURITY_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_10){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_set(unit, 
                    &reg_value, EAP_BYPASS_MAC_10f, &fld_valud);
            rv |= REG_WRITE_SECURITY_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_0X){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_set(unit, 
                    &reg_value, EAP_BYPASS_MAC_0Xf, &fld_valud);
            rv |= REG_WRITE_SECURITY_BYPASS_CTLr(unit, &reg_value);
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXULF){
            SOC_IF_ERROR_RETURN(REG_READ_IMP_PCTLr(unit, 
                    CMIC_PORT(unit), &reg_value));
            soc_IMP_PCTLr_field_set(unit, &reg_value, 
                    RX_UC_DLF_ENf, &fld_valud);
            SOC_IF_ERROR_RETURN(REG_WRITE_IMP_PCTLr(unit, 
                    CMIC_PORT(unit), &reg_value));
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXMLF){
            SOC_IF_ERROR_RETURN(REG_READ_IMP_PCTLr(unit, 
                    CMIC_PORT(unit), &reg_value));
            soc_IMP_PCTLr_field_set(unit, &reg_value, 
                    RX_MC_DLF_ENf, &fld_valud);
            SOC_IF_ERROR_RETURN(REG_WRITE_IMP_PCTLr(unit, 
                    CMIC_PORT(unit), &reg_value));
        } else {

            if (type_list[i] < DRV_DEV_CTRL_CNT){
                rv = SOC_E_UNAVAIL;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        }
        
        if (rv){
            *ctrl_cnt = op_cnt;
            break;
            
        }
        op_cnt++;
    }
    
    return rv;
}

/*
 * Function: 
 *	    drv_dev_control_get
 * Purpose:
 *      Get the system basis management or control functions. Especially for 
 *      those functions which provides a simple on/off or enable/disable 
 *      option to control device's working feature.
 *      Also, this driver interface provide some kinds of flexibility for 
 *      designer on implementing the device management mode configuration 
 *      instead of creating a individual driver interface.
 * Parameters:
 *      ctrl_cnt    - (IN/OUT) the number count to indicate how many pairs of
 *                      control types and values.
 *                      >> Output the number of proceeded control set items.
 *      type_list   - (IN) control type list
 *      value_list  - (OUT) control value list
 * Returns:
 *	    
 * Notes:
 *  1. the contorl type and value of the same list index must be matched as 
 *      a pair.
 */

int
drv_bcm53280_dev_control_get(int unit,uint32 *ctrl_cnt,
                uint32 *type_list,int *value_list)
{
    uint32  op_cnt = 0;
    uint32  reg_value = 0, field_val = 0;
    int     i, rv = SOC_E_NONE;

    /* Null check */
    if (op_cnt){
        assert(type_list && value_list);
    }
    
    for (i = 0; i < *ctrl_cnt; i++){
        if (type_list[i] == DRV_DEV_CTRL_MCASTREP){
            /* get the vport mcast replication feature enable status */
            rv = REG_READ_GMNGCFGr(unit, &reg_value);
            rv |= soc_GMNGCFGr_field_get(unit, &reg_value, 
                    EN_VPORT_REPLICATIONf, &field_val); 
        } else if (type_list[i] == DRV_DEV_CTRL_L2_USERADDR){
            /* on/off the L2 mulport vector feature(l2 user address) */
            rv = REG_READ_GARLCFGr(unit, &reg_value);
            rv |= soc_GARLCFGr_field_get(unit, &reg_value, 
                    MPADDR_ENf, &field_val);           
        } else if (type_list[i] == DRV_DEV_CTRL_RATE_METER_PLUS_IPG){
            /* rate meter operation includ or exclude IPG+Preamble(12+8 bytes)
             *  - 0 means IPG+Preamble will be excluded.
             *  - 1 means IPG+Preamble will be included.
             */
            rv = REG_READ_NEW_CONTROLr(unit, &reg_value);
            rv |= soc_NEW_CONTROLr_field_get(unit, &reg_value, 
                    WIRE_RATE_MODEf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_DA_ALL0_DROP){
            /* drop the packet within SourceMAC=00-00-00-00-00-00 */
            rv = REG_READ_SPECIAL_MNGTr(unit, &reg_value);
            rv |= soc_SPECIAL_MNGTr_field_get(unit, &reg_value, 
                    EN_ALL0_DA_DROPf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_ARPDHCP_TOCPU){
            /* forward ARP/DHCP to CPU */
            rv = REG_READ_SPECIAL_MNGTr(unit, &reg_value);
            rv |= soc_SPECIAL_MNGTr_field_get(unit, &reg_value, 
                    PASS_ARP_DHCPf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_IGMP_MLD_SNOOP_MODE){
            /* get IGMP/MLD snooping mode diable/CPU_only/CopyCPU */
            rv = REG_READ_GMNGCFGr(unit, &reg_value);
            rv |= soc_GMNGCFGr_field_get(unit, &reg_value, 
                    IGMP_MLD_CHKf, &field_val);
            /* transalate to logical value */
            field_val = (field_val == 0x3) ? DRV_SNOOP_MODE_CPU_SNOOP :
                    ((field_val == 0x1) ? DRV_SNOOP_MODE_CPU_TRAP : 
                        DRV_SNOOP_MODE_NONE);
        } else if (type_list[i] == DRV_DEV_CTRL_MCAST_SA_DROP){
            /* drop the packet within SourceMAC is Mcast MAC */
            rv = REG_READ_SPECIAL_MNGTr(unit, &reg_value);
            rv |= soc_SPECIAL_MNGTr_field_get(unit, &reg_value, 
                    EN_MCST_SA_DROPf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_RANGE_ERROR_DROP){
            /* drop the packet if range error */
            rv = REG_READ_MAC_FM_DROP_CTLr(unit, &reg_value);
            rv |= soc_MAC_FM_DROP_CTLr_field_get(unit, &reg_value, 
                    EN_RANGE_ERR_DROPf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_JUMBO_FRAME_DROP){
            /* drop the packet if range error */
            rv = REG_READ_MAC_FM_DROP_CTLr(unit, &reg_value);
            rv |= soc_MAC_FM_DROP_CTLr_field_get(unit, &reg_value, 
                    EN_JUMBO_FM_DROPf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_DROPPED_RANGE_ERR_NO_LEARN){
            /* drop the packet if range error */
            rv = REG_READ_MAC_FM_DROP_CTLr(unit, &reg_value);
            rv |= soc_MAC_FM_DROP_CTLr_field_get(unit, &reg_value, 
                    DIS_LRN_RANGE_ERRf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_DROPPED_JUMBO_FRM_NO_LEARN){
            /* drop the packet if range error */
            rv = REG_READ_MAC_FM_DROP_CTLr(unit, &reg_value);
            rv |= soc_MAC_FM_DROP_CTLr_field_get(unit, &reg_value, 
                    DIS_LRN_JUMBO_FM_DROPf, &field_val);

        /* bypass related */
        } else if (type_list[i] == DRV_DEV_CTRL_RX_BYPASS_CRCCHK){
            rv = REG_READ_GMNGCFGr(unit, &reg_value);
            rv |= soc_GMNGCFGr_field_get(unit, &reg_value, 
                    EN_CPU_RX_BYPASS_CRCCHKf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_DOS_BYPASS_TOCPU){
            rv = REG_READ_DOS_ATTACK_FILTER_DROP_CTLr(unit, &reg_value);
            rv |= soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(unit, 
                    &reg_value, CPUCOPY_BYPASS_DOSf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_STP_BYPASS_USERADDR){
            rv = REG_READ_STP_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_STP_BYPASS_CTLr_field_get(unit, 
                    &reg_value, STP_BYPAS_L2_USER_ADDRf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_STP_BYPASS_MAC0X){
            rv = REG_READ_STP_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_STP_BYPASS_CTLr_field_get(unit, 
                    &reg_value, STP_BYPAS_MAC0Xf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_USERADDR){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_get(unit, 
                    &reg_value, EAP_BYPASS_L2_USER_ADDRf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_DHCP){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_get(unit, 
                    &reg_value, EAP_BYPASS_DHCPf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_ARP){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_get(unit, 
                    &reg_value, EAP_BYPASS_ARPf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_22_2F){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_get(unit, 
                    &reg_value, EAP_BYPASS_MAC_22_2Ff, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_21){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_get(unit, 
                    &reg_value, EAP_BYPASS_MAC_21f, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_20){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_get(unit, 
                    &reg_value, EAP_BYPASS_MAC_20f, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_11_1F){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_get(unit, 
                    &reg_value, EAP_BYPASS_MAC_11_1Ff, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_10){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_get(unit, 
                    &reg_value, EAP_BYPASS_MAC_10f, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_EAP_BYPASS_MAC_0X){
            rv = REG_READ_SECURITY_BYPASS_CTLr(unit, &reg_value);
            rv |= soc_SECURITY_BYPASS_CTLr_field_get(unit, 
                    &reg_value, EAP_BYPASS_MAC_0Xf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_FL_BYPASS_SUPPORT_LIST){
            /* this type is for read only to report the supported filter 
             * bypass control items. The reported format is a bimap value.
             */

            /* in TB chips, the report value is directed reference to the 
             * HW register spec.
             */
            field_val = DRV_FILTER_BYPASS_RX_CRCCHK | 
                    DRV_FILTER_BYPASS_DOS_TOCPU | 
                    DRV_FILTER_BYPASS_STP_USERADDR | 
                    DRV_FILTER_BYPASS_STP_MAC0X | 
                    DRV_FILTER_BYPASS_VLAN_IGMP_MLD | 
                    DRV_FILTER_BYPASS_VLAN_ARP_DHCP | 
                    DRV_FILTER_BYPASS_VLAN_MIIM | 
                    DRV_FILTER_BYPASS_VLAN_MCAST | 
                    DRV_FILTER_BYPASS_VLAN_RSV_MCAST | 
                    DRV_FILTER_BYPASS_VLAN_USERADDR | 
                    DRV_FILTER_BYPASS_EAP_USERADDR | 
                    DRV_FILTER_BYPASS_EAP_DHCP | 
                    DRV_FILTER_BYPASS_EAP_ARP | 
                    DRV_FILTER_BYPASS_EAP_MAC_22_2F | 
                    DRV_FILTER_BYPASS_EAP_MAC_21 | 
                    DRV_FILTER_BYPASS_EAP_MAC_20 | 
                    DRV_FILTER_BYPASS_EAP_MAC_11_1F | 
                    DRV_FILTER_BYPASS_EAP_MAC_10 | 
                    DRV_FILTER_BYPASS_EAP_MAC_0X;
                    
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXULF){
            SOC_IF_ERROR_RETURN(REG_READ_IMP_PCTLr(unit, 
                    CMIC_PORT(unit), &reg_value));
            soc_IMP_PCTLr_field_get(unit, &reg_value, 
                    RX_UC_DLF_ENf, &field_val);
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXMLF){
            SOC_IF_ERROR_RETURN(REG_READ_IMP_PCTLr(unit, 
                    CMIC_PORT(unit), &reg_value));
            soc_IMP_PCTLr_field_get(unit, &reg_value, 
                    RX_MC_DLF_ENf, &field_val);
        } else {
            if (type_list[i] < DRV_DEV_CTRL_CNT){
                rv = SOC_E_UNAVAIL;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        }
        value_list[i] = field_val;    

        if (rv){
            *ctrl_cnt = op_cnt;
            break;
            
        }
        op_cnt++;
    }
    
    return rv;
}

#endif  /* BCM_TB_SUPPORT */
