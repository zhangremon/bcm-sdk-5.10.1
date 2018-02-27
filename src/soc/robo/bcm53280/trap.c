/*
 * $Id: trap.c 1.8 Broadcom SDK $
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
 *  Function : drv_trap_set
 *
 *  Purpose :
 *      Set the trap frame type to CPU.
 *
 *  Parameters :
 *      unit        :   unit id
 *      trap_mask   :   the mask of trap type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53280_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;

    soc_cm_debug(DK_VERBOSE, "drv_trap_set: unit = %d, trap mask= %x\n",
            unit, trap_mask);

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));

    /* retreive the current set first for IGMP/MLD snoop field within the 
     * value between 0 | 0x1 | 0x3 (not only on/off).
     *  - This is to avoid IGMP/MLD setting been override unexpected result  
     *     after called both trap_set and snoop_set
     */
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, IGMP_MLD_CHKf, &temp));
    if (trap_mask & (DRV_SWITCH_TRAP_IGMP | DRV_SWITCH_TRAP_MLD) && 
            (trap_mask & (DRV_SWITCH_TRAP_IGMP_DISABLE | 
            DRV_SWITCH_TRAP_MLD_DISABLE))) {
        /* means user set enable and disable together on IGMP/MLD */
        return SOC_E_CONFIG;
    } else if (trap_mask & (DRV_SWITCH_TRAP_IGMP | DRV_SWITCH_TRAP_MLD)) {
        temp = 0x1;
    } else if (trap_mask & (DRV_SWITCH_TRAP_IGMP_DISABLE | 
            DRV_SWITCH_TRAP_MLD_DISABLE)) {
        temp = 0x0;
    } else {
        /* processing flow here means user doesn't request IGMP/MLD enable
         *  or disable. Leave original configuration on IGMP/MLD.
         */
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
            (unit, &reg_value, IGMP_MLD_CHKf, &temp));

    /* @@@ CHECK_ME : Check if the VLAN bypass IGMP/MLD requirred @@@ 
     *  1. IGMP/MLD is native a VLAN aware protocol.
     *  2. Original deisgn may causes existed VLAN bypass IGMP/MLD been 
     *      overrided once this SOC driver is called.
     */
    /*
     * When IGMP/MLD snooping is enabled, 
     * need to bypass VLAN Ingress/Egress Filter for IGMP/MLD frame.
     */
    SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
        (unit, DRV_VLAN_PROP_BYPASS_IGMP_MLD, (temp) ? 1 : 0));

    /* To indicate BPDU CPUCopy is enabled or not 
      * 1 = Enables all ports to receive BPDUs to CPU
      * 0 = Drop BPDU packet
      */
    if (trap_mask & DRV_SWITCH_TRAP_BPDU1) {    
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
            (unit, &reg_value, RX_BPDU_ENf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
            (unit, &reg_value));

    /* To indicate EAP PDU CPUCopy is enabled or not
      * 1 = Enables all ports to receive EAP packet and forward to CPU
      * 0 = Drop EAP packet
      */    
    if (trap_mask & DRV_SWITCH_TRAP_8021X) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
            (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_set
            (unit, &reg_value, RX_EAP_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_SECURITY_GLOBAL_CTLr
            (unit, &reg_value));

    /* Broadcast packet */
    SOC_IF_ERROR_RETURN(REG_READ_IMP_PCTLr
            (unit, CMIC_PORT(unit), &reg_value));
    if (trap_mask & DRV_SWITCH_TRAP_BCST) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_IMP_PCTLr_field_set
            (unit, &reg_value, RX_BC_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_IMP_PCTLr
            (unit, CMIC_PORT(unit), &reg_value));
        
    if (trap_mask & DRV_SWITCH_TRAP_IPMC ||
            trap_mask & DRV_SWITCH_TRAP_GARP ||
            trap_mask & DRV_SWITCH_TRAP_ARP ||
            trap_mask & DRV_SWITCH_TRAP_8023AD ||
            trap_mask & DRV_SWITCH_TRAP_ICMP ||
            trap_mask & DRV_SWITCH_TRAP_BPDU2 ||
            trap_mask & DRV_SWITCH_TRAP_RARP ||
            trap_mask & DRV_SWITCH_TRAP_8023AD_DIS ||
            trap_mask & DRV_SWITCH_TRAP_BGMP ||
            trap_mask & DRV_SWITCH_TRAP_LLDP) {
        
        rv = SOC_E_UNAVAIL;
        return rv;
    }
    
    return rv;
}

/*
 *  Function : drv_trap_get
 *
 *  Purpose :
 *      Get the trap frame type to CPU.
 *
 *  Parameters :
 *      unit        :   unit id
 *      trap_mask   :   the mask of trap type.
 *      mac     :   mac address.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53280_trap_get(int unit, soc_port_t port, uint32 *trap_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, IGMP_MLD_CHKf, &temp));

    /* report the Trap status only */
    *trap_mask = 0;
    if (temp == 0x1) {
        *trap_mask |= DRV_SWITCH_TRAP_IGMP | DRV_SWITCH_TRAP_MLD;
    } else if (temp == 0){
        /* DO NOT reutrn DRV_SWITCH_TRAP_IGMP_DISABLE or 
         *  DRV_SWITCH_TRAP_MLD_DISABLE. These two are for set only.
         *  - the reason is to avoid unexpect disable once user called 
         *  snoop_set and trap_set.
         */
    }

    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, RX_BPDU_ENf, &temp));

    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BPDU1;
    }

    SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
            (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_get
            (unit, &reg_value, RX_EAP_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_8021X;
    }

    /* Broadcast packet */
    SOC_IF_ERROR_RETURN(REG_READ_IMP_PCTLr
            (unit, CMIC_PORT(unit), &reg_value));
    SOC_IF_ERROR_RETURN(soc_IMP_PCTLr_field_get
            (unit, &reg_value, RX_BC_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BCST;
    }

    soc_cm_debug(DK_VERBOSE, "drv_trap_get: unit = %d, trap mask= %x\n",
            unit, *trap_mask);
    return rv;
}

/*
 *  Function : drv_snoop_set
 *
 *  Purpose :
 *      Set the Snoop type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      snoop_mask   :   the mask of snoop type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53280_snoop_set(int unit, uint32 snoop_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;

    soc_cm_debug(DK_VERBOSE, "drv_snoop_set: unit = %d, snoop mask= %x\n",
            unit, snoop_mask);

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));
    
    /* retreive the current set first for IGMP/MLD snoop field within the 
     * value between 0 | 0x1 | 0x3 (not only on/off).
     *  - This is to avoid IGMP/MLD setting been override unexpected result
     *     after called both trap_set and snoop_set
     */
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, IGMP_MLD_CHKf, &temp));
    if (snoop_mask & (DRV_SNOOP_IGMP | DRV_SNOOP_MLD) && 
            (snoop_mask & (DRV_SNOOP_IGMP_DISABLE | 
            DRV_SNOOP_MLD_DISABLE))) {
        /* means user set enable and disable together on IGMP/MLD */
        return SOC_E_CONFIG;
    } else if (snoop_mask & (DRV_SNOOP_IGMP | DRV_SNOOP_MLD)) {
        temp = 0x3;
    } else if (snoop_mask & (DRV_SNOOP_IGMP_DISABLE | 
            DRV_SNOOP_MLD_DISABLE)) {
        temp = 0x0;
    } else {
        /* processing flow here means user doesn't request IGMP/MLD enable
         *  or disable. Leave original configuration on IGMP/MLD.
         */
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
            (unit, &reg_value, IGMP_MLD_CHKf, &temp));
    
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
            (unit, &reg_value));

    /* @@@ CHECK_ME : Check if the VLAN bypass IGMP/MLD requirred @@@ 
     *  1. IGMP/MLD is native a VLAN aware protocol.
     *  2. Original deisgn may causes existed VLAN bypass IGMP/MLD been 
     *      overrided once this SOC driver is called.
     */
    /*
      * When IGMP/MLD snooping is enabled, 
      * need to bypass VLAN Ingress/Egress Filter for IGMP/MLD frame.
      */
    SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
            (unit, DRV_VLAN_PROP_BYPASS_IGMP_MLD, (temp) ? 1 : 0));

    if (snoop_mask & DRV_SNOOP_ARP ||
            snoop_mask & DRV_SNOOP_RARP ||
            snoop_mask & DRV_SNOOP_ICMP ||
            snoop_mask & DRV_SNOOP_ICMPV6) {
        rv = SOC_E_UNAVAIL;
        return rv;
    }
     
    return rv; 
     
}

/*
 *  Function : drv_snoop_get
 *
 *  Purpose :
 *      Get the Snoop type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      snoop_mask   :   the mask of snoop type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm53280_snoop_get(int unit, uint32 *snoop_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;
    
    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &reg_value));

    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
            (unit, &reg_value, IGMP_MLD_CHKf, &temp));

    /* report the Trap status only */
    *snoop_mask = 0;
    if (temp == 0x3) {
        *snoop_mask |= DRV_SNOOP_IGMP | DRV_SNOOP_MLD;
    } else if (temp == 0){
        /* DO NOT reutrn DRV_SNOOP_IGMP_DISABLE or 
         *  DRV_SNOOP_MLD_DISABLE. These two are for set only.
         *  - the reason is to avoid unexpect disable once user called 
         *  snoop_set and trap_set.
         */
    }

    soc_cm_debug(DK_VERBOSE, "drv_snoop_get: unit = %d, snoop mask= %x\n",
            unit, *snoop_mask);
    return rv; 
}

/* the filed value in IGMP/MLD related register 
 *  - 0x2 is invalid.
 */
#define BCM53280_REG_SNOOP_DISABLE      0x0
#define BCM53280_REG_SNOOP_MODE         0x1
#define BCM53280_REG_TRAP_MODE          0x3

/*
 *  Function : drv_igmp_mld_snoop_mode_get
 *
 *  Purpose :
 *      Get the Snoop mode for IGMP/MLD.
 *
 *  Parameters :
 *      unit        :   unit id
 *      type        :   (IN) indicate a snoop type.
 *      mode        :   (OUT) indicate a snoop mode.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm53280_igmp_mld_snoop_mode_get(int unit, int type, int *mode)
{
    uint32  reg_val = 0, temp = 0;
    
    if (!(type == DRV_IGMP_MLD_TYPE_IGMP_MLD || 
        type == DRV_IGMP_MLD_TYPE_IGMP ||
        type == DRV_IGMP_MLD_TYPE_MLD)){
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg_val));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get(unit, 
                &reg_val, IGMP_MLD_CHKf, &temp));

    *mode = (temp == BCM53280_REG_SNOOP_MODE) ? DRV_IGMP_MLD_MODE_SNOOP : 
            (temp == BCM53280_REG_TRAP_MODE) ? DRV_IGMP_MLD_MODE_TRAP : 
                DRV_IGMP_MLD_MODE_DISABLE;

    return SOC_E_NONE; 
}

/*
 *  Function : drv_igmp_mld_snoop_mode_set
 *
 *  Purpose :
 *      Set the Snoop mode for IGMP/MLD.
 *
 *  Parameters :
 *      unit        :   unit id
 *      type        :   (IN) indicate a snoop type.
 *      mode        :   (IN) indicate a snoop mode.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. TB's special design that IGMP/MLD check will subject to any filter.
 *      That is CPU will not receive the Trap or Snoop packet once the IMP 
 *      is not the vlan member.
 *      >> VLAN_BYPASS_IGMP_MLD may causes the security issue due to the 
 *          VLAN ingress and egress filter been ignored.
 *          - the IGMP join/leave/query will flood to all linked port.
 *            no matter the ingress VID is registered or not and no matter 
 *            the egress port is VLAN member or not.
 *  2. BCM53280 in HW spec. is to turn on/off IGMP and MLD together.
 *      Thus the proper type to contorl IGMP/MLD snooping mode is  
 *      'DRV_IGMP_MLD_TYPE_IGMP_MLD' and two special cases still be allowed 
 *      are 'DRV_IGMP_MLD_TYPE_IGMP' and 'DRV_IGMP_MLD_TYPE_MLD'.
 *    - 'DRV_IGMP_MLD_TYPE_IGMP' and 'DRV_IGMP_MLD_TYPE_MLD' will effects on 
 *      each other.
 */
int 
drv_bcm53280_igmp_mld_snoop_mode_set(int unit, int type, int mode)
{
    uint32  reg_val = 0, temp = 0;

    if (!(type == DRV_IGMP_MLD_TYPE_IGMP_MLD || 
            type == DRV_IGMP_MLD_TYPE_IGMP || 
            type == DRV_IGMP_MLD_TYPE_MLD)){
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg_val));

    if (mode == DRV_IGMP_MLD_MODE_DISABLE){
        temp = BCM53280_REG_SNOOP_DISABLE;
    } else {
        if ((mode == DRV_IGMP_MLD_MODE_TRAP) || 
                (mode == DRV_IGMP_MLD_MODE_ENABLE)){
            temp = BCM53280_REG_TRAP_MODE;
        } else if (mode == DRV_IGMP_MLD_MODE_SNOOP){
            temp = BCM53280_REG_SNOOP_MODE;
        } else {
            soc_cm_debug(DK_WARN, "%s,%d, Unexpect mode assigned!\n", 
                    FUNCTION_NAME(), __LINE__);
            return SOC_E_PARAM;
        }
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set(unit, 
                &reg_val, IGMP_MLD_CHKf, &temp));
    /* Special process for TB only :
     *  - When IGMP/MLD snooping is enabled, the bypass VLAN Ingress/Egress 
     *      Filter for IGMP/MLD frame will be asserted for the expecting of 
     *      proper behavior.
     */
    SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_SET(unit, 
            DRV_VLAN_PROP_BYPASS_IGMP_MLD, 
            (mode != DRV_IGMP_MLD_MODE_DISABLE) ? 1 : 0));
        
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr(unit, &reg_val));

    return SOC_E_NONE; 
}
