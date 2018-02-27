/*
 * $Id: trap.c 1.11 Broadcom SDK $
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
 *      bmp   :   port bitmap.
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
drv_bcm5395_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;

    soc_cm_debug(DK_VERBOSE, 
    "drv_trap_set: unit = %d, trap mask= %x\n",
        unit, trap_mask);

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    if (trap_mask & DRV_SWITCH_TRAP_BPDU1) {    
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_value, RXBPDU_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
        (unit, &reg_value));
    
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_value));

    /* IGMP Packets */
    if (trap_mask & DRV_SWITCH_TRAP_IGMP_DISABLE) {
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_IP_ENf, &temp));
    } else {
        if (trap_mask & DRV_SWITCH_TRAP_IGMP) {    
            /* Set IGMP related forwarding mode : 
             *      - forward_mode = 1 : Redirect to CPU only.
             *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
             */
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                (unit, &reg_value, IGMP_IP_ENf, &temp));
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                (unit, &reg_value, IGMP_FWD_MODEf, &temp));
        }
    }

    /* ICMP v6 packets */
    if (trap_mask & DRV_SWITCH_TRAP_MLD_DISABLE) {
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, ICMPV6_ENf, &temp));
    } else {
        /* Set MLD related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        if ((trap_mask & DRV_SWITCH_TRAP_ICMPV6) || 
            (trap_mask & DRV_SWITCH_TRAP_MLD)){    
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                (unit, &reg_value, ICMPV6_ENf, &temp));
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                (unit, &reg_value, ICMPV6_FWD_MODEf, &temp));
        }
    }

    if (trap_mask & DRV_SWITCH_TRAP_BPDU1) {    
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_HL_PRTC_CTRLr
        (unit, &reg_value));
    
    if (trap_mask & DRV_SWITCH_TRAP_8021X) {
        rv = SOC_E_UNAVAIL;
        return rv;
    }
    
    if (trap_mask & DRV_SWITCH_TRAP_IPMC) {
        rv = SOC_E_UNAVAIL;
        return rv;
    }
    if (trap_mask & DRV_SWITCH_TRAP_GARP) {
        rv = SOC_E_UNAVAIL;
        return rv;
    }
    if (trap_mask & DRV_SWITCH_TRAP_ARP) {
        rv = SOC_E_UNAVAIL;
        return rv;
    } 
    if (trap_mask & DRV_SWITCH_TRAP_8023AD) {
        rv = SOC_E_UNAVAIL;
        return rv;
    } 
    if (trap_mask & DRV_SWITCH_TRAP_ICMP) {
        rv = SOC_E_UNAVAIL;
        return rv;
    }
    if (trap_mask & DRV_SWITCH_TRAP_BPDU2) {
        rv = SOC_E_UNAVAIL;
        return rv;
    } 
    if (trap_mask & DRV_SWITCH_TRAP_RARP) {
        rv = SOC_E_UNAVAIL;
        return rv;
    } 
    if (trap_mask & DRV_SWITCH_TRAP_8023AD_DIS) {
        rv = SOC_E_UNAVAIL;
        return rv;
    } 
    if (trap_mask & DRV_SWITCH_TRAP_BGMP) {
        rv = SOC_E_UNAVAIL;
        return rv;
    } 
    if (trap_mask & DRV_SWITCH_TRAP_LLDP) {
        rv = SOC_E_UNAVAIL;
        return rv;
    }

    /* Enable IP_Multicast */
    if (trap_mask & DRV_SWITCH_TRAP_IGMP) {
        SOC_IF_ERROR_RETURN(REG_READ_NEW_CONTROLr
            (unit, &reg_value));
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_NEW_CONTROLr_field_set
            (unit, &reg_value, IP_MULTICASTf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NEW_CONTROLr
            (unit, &reg_value));
    }
    /* Broadcast packet */
    if ((rv = REG_READ_MII_PCTLr(unit, CMIC_PORT(unit), &reg_value)) < 0) {
        return rv;
    }
    if (trap_mask & DRV_SWITCH_TRAP_BCST) {
        temp = 1;
    } else {
        temp = 0;
    }
    soc_MII_PCTLr_field_set(unit, &reg_value, MIRX_BC_ENf, &temp);    
    if ((rv = REG_WRITE_MII_PCTLr(unit, CMIC_PORT(unit), &reg_value)) < 0) {
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
 *      port   :   port id.
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
drv_bcm5395_trap_get(int unit, soc_port_t port, uint32 *trap_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, RXBPDU_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BPDU1;
    } 
    /* Broadcast packet */
    if ((rv = REG_READ_MII_PCTLr(unit, CMIC_PORT(unit), &reg_value)) < 0) {
        return rv;
    }
    soc_MII_PCTLr_field_get(unit, &reg_value, MIRX_BC_ENf, &temp);

    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BCST;
    }
    /* IGMP packets */
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, IGMP_IP_ENf, &temp));
    if (temp) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, IGMP_FWD_MODEf, &temp));
        if (temp) {
            *trap_mask |= DRV_SWITCH_TRAP_IGMP;
        }
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, ICMPV6_ENf, &temp));
    if (temp) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, ICMPV6_FWD_MODEf, &temp));
        if (temp) {
            *trap_mask |= DRV_SWITCH_TRAP_ICMPV6;
            *trap_mask |= DRV_SWITCH_TRAP_MLD;
        }
    }

    soc_cm_debug(DK_VERBOSE, 
    "drv_trap_get: unit = %d, trap mask= %x\n",
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
drv_bcm5395_snoop_set(int unit, uint32 snoop_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;

    soc_cm_debug(DK_VERBOSE, 
    "drv_snoop_set: unit = %d, snoop mask= %x\n",
        unit, snoop_mask);
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_value));

    /* IGMP Packets */
    if (snoop_mask & DRV_SNOOP_IGMP_DISABLE) {
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_IP_ENf, &temp));
    } else {
        if (snoop_mask & DRV_SNOOP_IGMP) {    
            /* Set IGMP related forwarding mode : 
             *      - forward_mode = 1 : Redirect to CPU only.
             *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
             */
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                (unit, &reg_value, IGMP_IP_ENf, &temp));
            temp = 0;
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                (unit, &reg_value, IGMP_FWD_MODEf, &temp));
        }
    }

    /* ICMP v6 packets */
    if (snoop_mask & DRV_SNOOP_MLD_DISABLE) {
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, ICMPV6_ENf, &temp));
    } else {
        /* Set MLD related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        if ((snoop_mask & DRV_SNOOP_ICMPV6) || 
            (snoop_mask & DRV_SNOOP_MLD)){    
            temp = 1;
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                (unit, &reg_value, ICMPV6_ENf, &temp));
            temp = 0;
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                (unit, &reg_value, ICMPV6_FWD_MODEf, &temp));
        }
    }

    if (snoop_mask & DRV_SNOOP_ICMP) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, ICMPV4_ENf, &temp));
    if (snoop_mask & DRV_SNOOP_DHCP) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, DHCP_ENf, &temp));
    if (snoop_mask & DRV_SNOOP_RARP) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, RARP_ENf, &temp));
    if (snoop_mask & DRV_SNOOP_ARP) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, ARP_ENf, &temp));
    
    SOC_IF_ERROR_RETURN(REG_WRITE_HL_PRTC_CTRLr
        (unit, &reg_value));
    /* enable IP_Multicast */
    if (snoop_mask & DRV_SNOOP_IGMP) {
        SOC_IF_ERROR_RETURN(REG_READ_NEW_CONTROLr
            (unit, &reg_value));
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_NEW_CONTROLr_field_set
            (unit, &reg_value, IP_MULTICASTf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NEW_CONTROLr
            (unit, &reg_value));
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
drv_bcm5395_snoop_get(int unit, uint32 *snoop_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;
    
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, IGMP_IP_ENf, &temp));
    if (temp) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, IGMP_FWD_MODEf, &temp));
        if (temp == 0) {
            *snoop_mask |= DRV_SNOOP_IGMP;
        }
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, ICMPV6_ENf, &temp));
    if (temp) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, ICMPV6_FWD_MODEf, &temp));
        if (temp == 0) {
            *snoop_mask |= DRV_SNOOP_ICMPV6;
            *snoop_mask |= DRV_SNOOP_MLD;
        }
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, ICMPV4_ENf, &temp));
    if (temp) {
        *snoop_mask |= DRV_SNOOP_ICMP;
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, DHCP_ENf, &temp));
    if (temp) {
        *snoop_mask |= DRV_SNOOP_DHCP;
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, RARP_ENf, &temp));
    if (temp) {
        *snoop_mask |= DRV_SNOOP_RARP;
    }
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, ARP_ENf, &temp));
    if (temp) {
        *snoop_mask |= DRV_SNOOP_ARP;
    }
     soc_cm_debug(DK_VERBOSE, 
    "drv_snoop_get: unit = %d, snoop mask= %x\n",
        unit, *snoop_mask);
    return rv; 
}

/* the filed value in IGMP/MLD related register */
#define BCM5395_REG_VALUE_SNOOP    0
#define BCM5395_REG_VALUE_TRAP     1

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
 */
int 
drv_bcm5395_igmp_mld_snoop_mode_get(int unit, int type, int *mode)
{
    uint32  reg_val = 0, temp_en = 0, temp_fwd = 0;
    
    if (!SOC_IS_ROBO5395(unit)){

        /* improper for other chips */
        return SOC_E_INTERNAL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr(unit, &reg_val));
    if (type == DRV_IGMP_MLD_TYPE_IGMP_MLD){
        /* No support on this type due to IGMP and MLD on this chip can be 
         * configured independently.
         */
        return SOC_E_UNAVAIL;
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                    &reg_val, IGMP_IP_ENf, &temp_en));
        if (temp_en){
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                        &reg_val, IGMP_FWD_MODEf, &temp_fwd));
            *mode = (temp_fwd == BCM5395_REG_VALUE_SNOOP) ? 
                    DRV_IGMP_MLD_MODE_SNOOP : DRV_IGMP_MLD_MODE_TRAP;
        } else {
            *mode = DRV_IGMP_MLD_MODE_DISABLE;
        }
    } else if (type == DRV_IGMP_MLD_TYPE_MLD){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                    &reg_val, ICMPV6_ENf, &temp_en));
        if (temp_en){
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                        &reg_val, ICMPV6_FWD_MODEf, &temp_fwd));
            *mode = (temp_fwd == BCM5395_REG_VALUE_SNOOP) ? 
                    DRV_IGMP_MLD_MODE_SNOOP : DRV_IGMP_MLD_MODE_TRAP;
        } else {
            *mode = DRV_IGMP_MLD_MODE_DISABLE;
        }
    } else {
        return SOC_E_UNAVAIL;
    }

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
 */
int 
drv_bcm5395_igmp_mld_snoop_mode_set(int unit, int type, int mode)
{
    uint32  reg_val = 0, temp_en = 0, temp_fwd = 0;

    if (!SOC_IS_ROBO5395(unit)){

        /* improper for other chips */
        return SOC_E_INTERNAL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr(unit, &reg_val));
    if (type == DRV_IGMP_MLD_TYPE_IGMP_MLD){
        /* No support on this type due to IGMP and MLD on this chip can be 
         * configured independently.
         */
        soc_cm_debug(DK_WARN, "%s, IGMP+MLD configuration is no supported\n",
                FUNCTION_NAME());
        return SOC_E_UNAVAIL;
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP){

        /* there are some speical deisgn for BCM5395 : 
         *  1. to set to DISABLE, the Drop will be set to 0.
         *    - This is for the consistent SOC design on all robo chips. 
         *      Such configuration on Drop is actually a dummy configuration 
         *      due to bcm5395 have no PURE Drop action on IGMP/MLD relate 
         *      snooping control.
         *    - Such design is consistent with other FE ROBO chips like 
         *      bcm53242 or bcm53280 for those chips in Register Spec. is a 
         *      two bits register filed the DISABLE value is '0' 
         *      (means internal Drop bit is '0')
         *  2. to set to ENABLE, the Drop action will be set to 1.
         *    - Trap is the most reasonable configuration for IGMP snooping 
         *      (no customer port talk)
         */
        if (mode == DRV_IGMP_MLD_MODE_DISABLE){
            temp_en = FALSE;
            temp_fwd = BCM5395_REG_VALUE_SNOOP;     /* value = 0 */
        } else if (mode == DRV_IGMP_MLD_MODE_ENABLE || 
                mode == DRV_IGMP_MLD_MODE_SNOOP || 
                mode == DRV_IGMP_MLD_MODE_TRAP){
            temp_en = TRUE;

            if (mode == DRV_IGMP_MLD_MODE_SNOOP){
                temp_fwd = BCM5395_REG_VALUE_SNOOP;
            } else {
                /* ENABLE and TRAP */
                temp_fwd = BCM5395_REG_VALUE_TRAP;
            }
        } else {
            soc_cm_debug(DK_WARN, "%s,snooping mode out of valid value(%d)\n",
                    FUNCTION_NAME(),mode);
            return SOC_E_PARAM;
        }
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_IP_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_FWD_MODEf, &temp_fwd));
        
    } else if (type == DRV_IGMP_MLD_TYPE_MLD){
        if (mode == DRV_IGMP_MLD_MODE_DISABLE){
            temp_en = FALSE;
            temp_fwd = BCM5395_REG_VALUE_SNOOP;     /* value = 0 */
        } else if (mode == DRV_IGMP_MLD_MODE_ENABLE || 
                mode == DRV_IGMP_MLD_MODE_SNOOP || 
                mode == DRV_IGMP_MLD_MODE_TRAP){
            temp_en = TRUE;

            /* if (mode == DRV_IGMP_MLD_MODE_ENABLE), no fwd mode change */
            if (mode == DRV_IGMP_MLD_MODE_SNOOP){
                temp_fwd = BCM5395_REG_VALUE_SNOOP;
            } else {
                /* ENABLE and TRAP */
                temp_fwd = BCM5395_REG_VALUE_TRAP;
            }
        } else {
            soc_cm_debug(DK_WARN, "%s,snooping mode out of valid value(%d)\n",
                    FUNCTION_NAME(),mode);
            return SOC_E_PARAM;
        }
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, ICMPV6_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, ICMPV6_FWD_MODEf, &temp_fwd));
    } else {
        return SOC_E_UNAVAIL;
    }
    
    SOC_IF_ERROR_RETURN(REG_WRITE_HL_PRTC_CTRLr(unit, &reg_val));

    return SOC_E_NONE; 
}
