/*
 * $Id: trap.c 1.12.64.3 Broadcom SDK $
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

/* the filed value in bcm5348 IGMP related register 
 *  - GMNGCFGr.IGMP_IP_CHKf (0x2 is invalid)
 */
#define BCM5348_REG_IGMP_IP_DISABLE     0x0
#define BCM5348_REG_IGMP_IP_TRAP        0x1
#define BCM5348_REG_IGMP_IP_SNOOP       0x3

/* the filed value in bcm53242 IGMP/MLD related register 
 *  - GMNGCFGr.IGMP_MLD_CHKf (0x2 is invalid)
 */
#define BCM53242_REG_IGMP_MLD_DISABLE   0x0
#define BCM53242_REG_IGMP_MLD_TRAP      0x1
#define BCM53242_REG_IGMP_MLD_SNOOP     0x3

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
drv_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;
    soc_port_t port = 0;

    soc_cm_debug(DK_VERBOSE, 
    "drv_trap_set: unit = %d, trap mask= %x\n",
        unit, trap_mask);
    if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
        return rv;
    }
    if (trap_mask & DRV_SWITCH_TRAP_IGMP) {    
        temp = 1;
        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
            temp = BCM53242_REG_IGMP_MLD_TRAP;
        } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)){
            temp = BCM5348_REG_IGMP_IP_TRAP;
        }
    } else {
        temp = 0;
        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
            if (trap_mask & DRV_SWITCH_TRAP_MLD) {
                temp = BCM53242_REG_IGMP_MLD_TRAP;
            }
        }
    }

    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        /*
         * temp =  1b'01: IGMP forward to IMP.
         *         1b'11: IGMP forward to original pbmp and IMP.
         */
        soc_GMNGCFGr_field_set(unit, &reg_value, 
            IGMP_IP_CHKf, &temp);
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){
         /* temp =  1b'01: IGMP/MLD forward to IMP.
                  1b'11: IGMP/MLD forward to original pbmp and IMP. */
         soc_GMNGCFGr_field_set(unit, &reg_value, 
            IGMP_MLD_CHKf, &temp);
    } else {
        soc_GMNGCFGr_field_set(unit, &reg_value, 
            IGMPIP_SNOP_ENf, &temp);
    }

    if (trap_mask & DRV_SWITCH_TRAP_BPDU1) {    
        temp = 1;
    } else {
        temp = 0;
    }
    soc_GMNGCFGr_field_set(unit, &reg_value, 
        RXBPDU_ENf, &temp);
    
    if ((rv = REG_WRITE_GMNGCFGr(unit, &reg_value)) < 0) {
        return rv;
    }
    
    if (trap_mask & DRV_SWITCH_TRAP_8021X) {
        temp = 1;
    } else {
        temp = 0;
    }
    if(SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5348(unit) || 
        SOC_IS_ROBO5347(unit)) {
        if ((rv = REG_READ_SPECIAL_MNGTr(unit, &reg_value)) < 0) {
            return rv;
        }
        soc_SPECIAL_MNGTr_field_set(unit, &reg_value,
            EN_SEC_DROP2MIIf, &temp);
        if ((rv = REG_WRITE_SPECIAL_MNGTr(unit, &reg_value)) < 0) {
            return rv;
        }
    } else if(SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){
        PBMP_ITER(bmp, port) {
            if ((rv = REG_READ_PORT_SEC_CONr(unit, port, &reg_value)) < 0) {
                return rv;
            }

            soc_PORT_SEC_CONr_field_set(unit, &reg_value, 
                SA_VIO_OPTf, &temp);

            if ((rv = REG_WRITE_PORT_SEC_CONr(unit, port, &reg_value)) < 0) {
                return rv;
            }
        }
    } else if(SOC_IS_ROBODINO(unit)|| SOC_IS_ROBO5398(unit)||
                SOC_IS_ROBO5397(unit)){
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
        if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
            SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
            /* No IP Multicast for 5348 / 5347 / 53242*/
        } else {
            if ((rv = REG_READ_NEW_CONTROLr(unit, &reg_value)) < 0) {
                return rv;
            }
            temp = 1;
            soc_NEW_CONTROLr_field_set(unit, &reg_value, 
                IP_MULTICASTf, &temp);
            if ((rv = REG_WRITE_NEW_CONTROLr(unit, &reg_value)) < 0) {
                return rv;
            }
        }
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
drv_trap_get(int unit, soc_port_t port, uint32 *trap_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp = 0;

    if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
        return rv;
    }
    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        soc_GMNGCFGr_field_get(unit, &reg_value, 
            IGMP_IP_CHKf, &temp);
        if (temp == BCM5348_REG_IGMP_IP_TRAP) {
            *trap_mask |= DRV_SWITCH_TRAP_IGMP;
        }
    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        soc_GMNGCFGr_field_get(unit, &reg_value, 
            IGMP_MLD_CHKf, &temp);
        if (temp == BCM53242_REG_IGMP_MLD_TRAP) {
            *trap_mask |= DRV_SWITCH_TRAP_IGMP | DRV_SWITCH_TRAP_MLD;
        }
    } else {
        soc_GMNGCFGr_field_get(unit, &reg_value, 
            IGMPIP_SNOP_ENf, &temp);
        if (temp) {
            *trap_mask |= DRV_SWITCH_TRAP_IGMP;
        }
    }
    soc_GMNGCFGr_field_get(unit, &reg_value, 
        RXBPDU_ENf, &temp);
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BPDU1;
    }
    if(SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5348(unit)||
        SOC_IS_ROBO5347(unit)) {
        if ((rv = REG_READ_SPECIAL_MNGTr(unit, &reg_value)) < 0) {
            return rv;
        }
        soc_SPECIAL_MNGTr_field_get(unit, &reg_value,
            EN_SEC_DROP2MIIf, &temp);
        if (temp) {
            *trap_mask |= DRV_SWITCH_TRAP_8021X;
        }
    } else if(SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){
            if ((rv = REG_READ_PORT_SEC_CONr(unit, port, &reg_value)) < 0) {
                return rv;
            }

            soc_PORT_SEC_CONr_field_get(unit, &reg_value, 
                SA_VIO_OPTf, &temp);
            if (temp) {
                *trap_mask |= DRV_SWITCH_TRAP_8021X;
            }
    } 
    /* Broadcast packet */
    if ((rv = REG_READ_MII_PCTLr(unit, CMIC_PORT(unit), &reg_value)) < 0) {
        return rv;
    }
    soc_MII_PCTLr_field_get(unit, &reg_value, MIRX_BC_ENf, &temp);

    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BCST;
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
drv_snoop_set(int unit, uint32 snoop_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;

    soc_cm_debug(DK_VERBOSE, 
    "drv_snoop_set: unit = %d, snoop mask= %x\n",
        unit, snoop_mask);
    if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
        return rv;
    }
    if (snoop_mask & DRV_SNOOP_IGMP) {    
        temp = 1;
        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
            temp = BCM53242_REG_IGMP_MLD_SNOOP;
        } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
            temp = BCM5348_REG_IGMP_IP_SNOOP;
        }
    } else {
        temp = 0;
    }

    if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
        soc_GMNGCFGr_field_set(unit, &reg_value, 
            IGMP_IP_CHKf, &temp);
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        soc_GMNGCFGr_field_set(unit, &reg_value, 
            IGMP_MLD_CHKf, &temp);
    } else {
        soc_GMNGCFGr_field_set(unit, &reg_value, 
            IGMPIP_SNOP_ENf, &temp);
    }
    if ((rv = REG_WRITE_GMNGCFGr(unit, &reg_value)) < 0) {
        return rv;
    }
    /* enable IP_Multicast */
    if (snoop_mask & DRV_SNOOP_IGMP) {
        if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit) ||
            SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
            /* No IP Multicast for 5348 / 5347 / 53242*/
        } else {
            if ((rv = REG_READ_NEW_CONTROLr(unit, &reg_value)) < 0) {
                return rv;
            }
            temp = 1;
            soc_NEW_CONTROLr_field_set(unit, &reg_value, 
                IP_MULTICASTf, &temp);
            if ((rv = REG_WRITE_NEW_CONTROLr(unit, &reg_value)) < 0) {
                return rv;
            }
        }
    }
    if (snoop_mask & DRV_SNOOP_ARP) {
        rv = SOC_E_UNAVAIL;
        return rv;
    }
    if (snoop_mask & DRV_SNOOP_RARP) {
        rv = SOC_E_UNAVAIL;
        return rv;
    }
    if (snoop_mask & DRV_SNOOP_ICMP) {
        rv = SOC_E_UNAVAIL;
        return rv;
    }
    if (snoop_mask & DRV_SNOOP_ICMPV6) {
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
drv_snoop_get(int unit, uint32 *snoop_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp = 0;
    
    if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
        return rv;
    }
    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        soc_GMNGCFGr_field_get(unit, &reg_value, 
            IGMP_IP_CHKf, &temp);
        if (temp == BCM5348_REG_IGMP_IP_SNOOP) {
           *snoop_mask = DRV_SNOOP_IGMP;
        }
    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        soc_GMNGCFGr_field_get(unit, &reg_value, 
            IGMP_MLD_CHKf, &temp);
        if (temp == BCM53242_REG_IGMP_MLD_SNOOP) {
           *snoop_mask = DRV_SNOOP_IGMP | DRV_SNOOP_MLD;
        }
    } else {
        soc_GMNGCFGr_field_get(unit, &reg_value, 
            IGMPIP_SNOP_ENf, &temp);
        if (temp) {
           *snoop_mask = DRV_SNOOP_IGMP;
        }
    }
    soc_cm_debug(DK_VERBOSE, 
    "drv_snoop_get: unit = %d, snoop mask= %x\n",
        unit, *snoop_mask);
    return rv; 
}

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
 *  1. This driver funciton support the earlier ROBO chip
 *      - bcm5324, bcm5396, bcm5389, bcm5397 and bcm5398
 *  2. This routine support the chips without forwarding mode configurations.
 *
 */
int 
drv_igmp_mld_snoop_mode_get(int unit, int type, int *mode)
{
    uint32  reg_val = 0, temp = 0;
    
    if (!(SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || \
            SOC_IS_ROBO5389(unit) ||SOC_IS_ROBO5398(unit) || \
            SOC_IS_ROBO5397(unit))){

        /* improper for other chips */
        return SOC_E_INTERNAL;
    } else {
        if (type != DRV_IGMP_MLD_TYPE_IGMP){
            return SOC_E_UNAVAIL;
        }
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg_val));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get(unit, 
                &reg_val, IGMPIP_SNOP_ENf, &temp));

    if (!temp){
        *mode = DRV_IGMP_MLD_MODE_DISABLE;
    } else {
        if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || \
            SOC_IS_ROBO5389(unit)){
            *mode = DRV_IGMP_MLD_MODE_TRAP;
        } else {
            /* BCM5398 and BCM5397 */
            *mode = DRV_IGMP_MLD_MODE_SNOOP;
        }
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
 *  Note :
 *  1. This driver funciton support the earlier ROBO chip
 *      - bcm5324, bcm5396, bcm5389, bcm5397 and bcm5398
 *  2. This routine support the chips without forwarding mode configurations.
 *
 */
int 
drv_igmp_mld_snoop_mode_set(int unit, int type, int mode)
{
    uint32  reg_val = 0, temp = 0;

    if (!(SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || \
            SOC_IS_ROBO5389(unit) ||SOC_IS_ROBO5398(unit) || \
            SOC_IS_ROBO5397(unit))){

        /* improper for other chips */
        return SOC_E_INTERNAL;
    } else {
        if (type != DRV_IGMP_MLD_TYPE_IGMP){
            return SOC_E_UNAVAIL;
        }
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg_val));

    if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || \
        SOC_IS_ROBO5389(unit)){
        /* bcm5324/5396/5389 support trap mode only */
        if (mode == DRV_IGMP_MLD_MODE_SNOOP){
            return SOC_E_CONFIG;
        }
    } else {
        /* BCM5398 and BCM5397 */
        /* support snoop mode only */
        if (mode == DRV_IGMP_MLD_MODE_TRAP){
            return SOC_E_CONFIG;
        }
    }
    temp = (mode != DRV_IGMP_MLD_MODE_DISABLE) ? TRUE : FALSE;
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set(unit, 
                &reg_val, IGMPIP_SNOP_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr(unit, &reg_val));
        
    return SOC_E_NONE; 
}

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
drv_bcm5348_igmp_mld_snoop_mode_get(int unit, int type, int *mode)
{
    uint32  reg_val = 0, temp = 0;
    
    if (!(SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit))){
        /* improper for other chips */
        return SOC_E_INTERNAL;
    } else {
        if (type != DRV_IGMP_MLD_TYPE_IGMP){
            return SOC_E_UNAVAIL;
        }
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg_val));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get(unit, 
                &reg_val, IGMP_IP_CHKf, &temp));

    *mode = (temp == BCM5348_REG_IGMP_IP_SNOOP) ? DRV_IGMP_MLD_MODE_SNOOP : 
            (temp == BCM5348_REG_IGMP_IP_TRAP) ? DRV_IGMP_MLD_MODE_TRAP : 
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
 */
int 
drv_bcm5348_igmp_mld_snoop_mode_set(int unit, int type, int mode)
{
    uint32  reg_val = 0, temp = 0;

    if (!(SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit))){
        /* improper for other chips */
        return SOC_E_INTERNAL;
    } else {
        if (type != DRV_IGMP_MLD_TYPE_IGMP){
            return SOC_E_UNAVAIL;
        }
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg_val));

    if (mode == DRV_IGMP_MLD_MODE_DISABLE){
        temp = BCM5348_REG_IGMP_IP_DISABLE;
    } else {
        if ((mode == DRV_IGMP_MLD_MODE_TRAP) || 
                (mode == DRV_IGMP_MLD_MODE_ENABLE)){
            temp = BCM5348_REG_IGMP_IP_TRAP;
        } else if (mode == DRV_IGMP_MLD_MODE_SNOOP){
            temp = BCM5348_REG_IGMP_IP_SNOOP;
        } else {
            soc_cm_print("%s,%d, Unexpect mode assigned!\n", 
                    FUNCTION_NAME(), __LINE__);
            return SOC_E_PARAM;
        }
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set(unit, 
                &reg_val, IGMP_IP_CHKf, &temp));    
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr(unit, &reg_val));

    return SOC_E_NONE; 
}

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
drv_bcm53242_igmp_mld_snoop_mode_get(int unit, int type, int *mode)
{
    uint32  reg_val = 0, temp = 0;
    
    if (!(SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))){
        /* improper for other chips */
        return SOC_E_INTERNAL;
    } else {
        if (!(type == DRV_IGMP_MLD_TYPE_IGMP_MLD || 
                type == DRV_IGMP_MLD_TYPE_IGMP || 
                type == DRV_IGMP_MLD_TYPE_MLD)){
            return SOC_E_UNAVAIL;
        }
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg_val));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get(unit, 
                &reg_val, IGMP_MLD_CHKf, &temp));

    *mode = (temp == BCM53242_REG_IGMP_MLD_SNOOP) ? DRV_IGMP_MLD_MODE_SNOOP : 
            (temp == BCM53242_REG_IGMP_MLD_TRAP) ? DRV_IGMP_MLD_MODE_TRAP : 
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
 *  1. BCM53242 in HW spec. is to turn on/off IGMP and MLD together.
 *      Thus the proper type to contorl IGMP/MLD snooping mode is  
 *      'DRV_IGMP_MLD_TYPE_IGMP_MLD' and two special cases still be allowed 
 *      are 'DRV_IGMP_MLD_TYPE_IGMP' and 'DRV_IGMP_MLD_TYPE_MLD'.
 *    - 'DRV_IGMP_MLD_TYPE_IGMP' and 'DRV_IGMP_MLD_TYPE_MLD' will effects on 
 *      each other.
 */
int 
drv_bcm53242_igmp_mld_snoop_mode_set(int unit, int type, int mode)
{
    uint32  reg_val = 0, temp = 0;

    if (!(SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))){
        /* improper for other chips */
        return SOC_E_INTERNAL;
    } else {
        if (!(type == DRV_IGMP_MLD_TYPE_IGMP_MLD || 
                type == DRV_IGMP_MLD_TYPE_IGMP || 
                type == DRV_IGMP_MLD_TYPE_MLD)){
            return SOC_E_UNAVAIL;
        }
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg_val));

    if (mode == DRV_IGMP_MLD_MODE_DISABLE){
        temp = BCM53242_REG_IGMP_MLD_DISABLE;
    } else {
        if ((mode == DRV_IGMP_MLD_MODE_TRAP) || 
                (mode == DRV_IGMP_MLD_MODE_ENABLE)){
            temp = BCM53242_REG_IGMP_MLD_TRAP;
        } else if (mode == DRV_IGMP_MLD_MODE_SNOOP){
            temp = BCM53242_REG_IGMP_MLD_SNOOP;
        } else {
            soc_cm_debug(DK_WARN,"%s,%d, Unexpect mode assigned!\n", 
                    FUNCTION_NAME(), __LINE__);
            return SOC_E_PARAM;
        }
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set(unit, 
                &reg_val, IGMP_MLD_CHKf, &temp));    
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr(unit, &reg_val));

    return SOC_E_NONE; 
}
