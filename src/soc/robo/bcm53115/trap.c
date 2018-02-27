/*
 * $Id: trap.c 1.18.2.2 Broadcom SDK $
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

#define BCM53115_REG_VALUE_SNOOP    0
#define BCM53115_REG_VALUE_TRAP     1


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
drv_bcm53115_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;
    uint32  snoop_mask = 0;

    soc_cm_debug(DK_VERBOSE, 
    "drv_trap_set: unit = %d, trap mask= %x\n",
        unit, trap_mask);

    /* get the snooping mask firet to prevent the unexpect value set causes 
     *  other snoop setting been effected.
     *  - prevent the IGMP/MLD/ICMP/DHCP/ARP/RARP been modified unexpected.
     */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->snoop_get)
                    (unit, &snoop_mask));
     
    if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
        return rv;
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
    
    if ((rv = REG_READ_HL_PRTC_CTRLr(unit, &reg_value)) < 0) {
        return rv;
    }
    
    /* Set IGMP trap ====== */
    /* enable/disable bit for all IGMP pkt type CPU trap(CPU redirect) */
    if (trap_mask & DRV_SWITCH_TRAP_IGMP) {
        trap_mask |= DRV_SWITCH_TRAP_IGMP_UNKNOW | 
                    DRV_SWITCH_TRAP_IGMP_QUERY | 
                    DRV_SWITCH_TRAP_IGMP_REPLEV;
    }
    if (trap_mask & DRV_SWITCH_TRAP_IGMP_DISABLE){
        temp = 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_UKN_ENf, &temp);
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_QRY_ENf, &temp);
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_RPTLVE_ENf, &temp);
    } else {
        
        /* Set IGMP related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_QUERY) || 
                (snoop_mask & DRV_SNOOP_IGMP_QUERY)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_QRY_ENf, &temp);
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_IGMP_QUERY){
                temp = BCM53115_REG_VALUE_TRAP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    IGMP_QRY_FWD_MODEf, &temp);
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_REPLEV) || 
                (snoop_mask & DRV_SNOOP_IGMP_REPLEV)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_RPTLVE_ENf, &temp);
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_IGMP_REPLEV){
                temp = BCM53115_REG_VALUE_TRAP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    IGMP_RPTLVE_FWD_MODEf, &temp);
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_UNKNOW) || 
                (snoop_mask & DRV_SNOOP_IGMP_UNKNOW)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_UKN_ENf, &temp);
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_IGMP_UNKNOW){
                temp = BCM53115_REG_VALUE_TRAP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    IGMP_UKN_FWD_MODEf, &temp);
            }
        }
    }
    
    /* set MLD Trap ====== */
    /* enable/disable bit setting on all MLD pkt type snooping */
    if (trap_mask & DRV_SWITCH_TRAP_MLD) {
        trap_mask |= DRV_SWITCH_TRAP_MLD_QUERY | DRV_SWITCH_TRAP_MLD_REPDONE;
    }
    if (trap_mask & DRV_SWITCH_TRAP_MLD_DISABLE){
        temp = 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    MLD_QRY_ENf, &temp);
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    MLD_RPTDONE_ENf, &temp);
    } else {
        /* Set MLD related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        temp = ((trap_mask & DRV_SWITCH_TRAP_MLD_QUERY) || 
                    (snoop_mask & DRV_SNOOP_MLD_QUERY)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    MLD_QRY_ENf, &temp);
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_MLD_QUERY){
                temp = BCM53115_REG_VALUE_TRAP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    MLD_QRY_FWD_MODEf, &temp);
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_MLD_REPDONE) || 
                    (snoop_mask & DRV_SNOOP_MLD_REPDONE)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    MLD_RPTDONE_ENf, &temp);
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_MLD_REPDONE){
                temp = BCM53115_REG_VALUE_TRAP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    MLD_RPTDONE_FWD_MODEf, &temp);
            }
        }
    }

    /* set ICMPv4 trap (the same configuration with ICMPv4 snoop setting)*/
    /* Marked the ICMPv4 trap section for ICMPv4 in bcm5395/53115/53118 can be 
     *  snooping mode only.
     */
    /* 
    temp = (trap_mask & DRV_SWITCH_TRAP_ICMP) ? 1 : 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, HL_PRTC_CTRLr, &reg_value, ICMPV4_ENf, &temp));
    */

    /* set ICMPv6 trap :  */
    temp = (trap_mask & DRV_SWITCH_TRAP_ICMPV6_DISABLE) ? 0 : 
                (((trap_mask & DRV_SWITCH_TRAP_ICMPV6) || 
                    (snoop_mask & DRV_SNOOP_ICMPV6)) ? 1 : 0);
    soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    ICMPV6_ENf, &temp);
    if (temp){
        /* set working mode */
        temp = (trap_mask & DRV_SWITCH_TRAP_ICMPV6) ? 
                BCM53115_REG_VALUE_TRAP : BCM53115_REG_VALUE_SNOOP;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    ICMPV6_FWD_MODEf, &temp);
    }

    /* set RARP trap (the same configuration with RARP snoop setting)*/
    temp = (trap_mask & DRV_SWITCH_TRAP_RARP) ? 1 : 0;
    soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    RARP_ENf, &temp);

    /* set ARP trap (the same configuration with ARP snoop setting)*/
    temp = (trap_mask & DRV_SWITCH_TRAP_ARP) ? 1 : 0;
    soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    ARP_ENf, &temp);

    if ((rv = REG_WRITE_HL_PRTC_CTRLr(unit, &reg_value)) < 0) {
        return rv;
    }
    
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
    if (trap_mask & DRV_SWITCH_TRAP_8023AD) {
        rv = SOC_E_UNAVAIL;
        return rv;
    } 
    if (trap_mask & DRV_SWITCH_TRAP_BPDU2) {
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
    
    /* Broadcast packet */
    if ((rv = REG_READ_IMP_CTLr(unit, CMIC_PORT(unit), &reg_value)) < 0) {
        return rv;
    }
    if (trap_mask & DRV_SWITCH_TRAP_BCST) {
        temp = 1;
    } else {
        temp = 0;
    }
    soc_IMP_CTLr_field_set(unit, &reg_value, RX_BCST_ENf, &temp);
    if ((rv = REG_WRITE_IMP_CTLr(unit, CMIC_PORT(unit), &reg_value)) < 0) {
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
drv_bcm53115_trap_get(int unit, soc_port_t port, uint32 *trap_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp = 0;
    int     enable_flag;    /* for bcm53115's igmp/mld only */

    /* clear the mask */
    *trap_mask = 0;

    if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
        return rv;
    }
    soc_GMNGCFGr_field_get(unit, &reg_value, 
        RXBPDU_ENf, &temp);
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BPDU1;
    } 
    /* Broadcast packet */
    if ((rv = REG_READ_IMP_CTLr(unit, CMIC_PORT(unit), &reg_value)) < 0) {
        return rv;
    }
    soc_IMP_CTLr_field_get(unit, &reg_value, RX_BCST_ENf, &temp);

    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BCST;
    }
    
    if ((rv = REG_READ_HL_PRTC_CTRLr(unit, &reg_value)) < 0) {
        return rv;
    }
    
    /* ---- get IGMP trap setting ---- */ 
    enable_flag = FALSE;
    /* trap status on IGMP query */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_QRY_ENf, &temp);
    if (temp){
        enable_flag = TRUE;
        
        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_QRY_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_TRAP){
            *trap_mask |= DRV_SWITCH_TRAP_IGMP_QUERY;
        }
    }
    
    /* trap status on IGMP report/leave */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_RPTLVE_ENf, &temp);
    if (temp){
        enable_flag = TRUE;
        
        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_RPTLVE_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_TRAP){
            *trap_mask |= DRV_SWITCH_TRAP_IGMP_REPLEV;
        }
    }

    /* trap status on IGMP unknow */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_UKN_ENf, &temp);
    if (temp){
        enable_flag = TRUE;
        
        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_UKN_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_TRAP){
            *trap_mask |= DRV_SWITCH_TRAP_IGMP_UNKNOW;
        }
    }
    
    /* trap status on IGMP all types information */
    if (!enable_flag){
        *trap_mask |= DRV_SWITCH_TRAP_IGMP_DISABLE;
    }
    
    if ((*trap_mask & DRV_SWITCH_TRAP_IGMP_QUERY) && 
            (*trap_mask & DRV_SWITCH_TRAP_IGMP_REPLEV) &&
            (*trap_mask & DRV_SWITCH_TRAP_IGMP_UNKNOW)){
                
        *trap_mask |= DRV_SWITCH_TRAP_IGMP;
    }

    /* ---- get MLD trap setting ---- */ 
    enable_flag = FALSE;
    /* trap status on MLD query */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            MLD_QRY_ENf, &temp);
    if (temp){
        enable_flag = TRUE;
        
        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            MLD_QRY_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_TRAP){
            *trap_mask |= DRV_SWITCH_TRAP_MLD_QUERY;
        }
    }
    
    /* trap status on MLD report */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            MLD_RPTDONE_ENf, &temp);
    if (temp){
        enable_flag = TRUE;
        
        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            MLD_RPTDONE_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_TRAP){
            *trap_mask |= DRV_SWITCH_TRAP_MLD_REPDONE;
        }
    }
    
    /* trap status on MLD all types information */
    if (!enable_flag){
        *trap_mask |= DRV_SWITCH_TRAP_MLD_DISABLE;
    }
    
    if ((*trap_mask & DRV_SWITCH_TRAP_MLD_QUERY) && 
            (*trap_mask & DRV_SWITCH_TRAP_MLD_REPDONE)){
                
        *trap_mask |= DRV_SWITCH_TRAP_MLD;
    }
        
    /* get ICMPv6 trap setting */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            ICMPV6_ENf, &temp);
    if (temp) {
        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            ICMPV6_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_TRAP){
            *trap_mask |= DRV_SWITCH_TRAP_ICMPV6;
        }
    } else {
        *trap_mask |= DRV_SWITCH_TRAP_ICMPV6_DISABLE;
    }
        
    /* get ICMPv4 trap setting */
    /* Marked the ICMPv4 trap section for ICMPv4 in bcm5395/53115/53118 can be 
     *  snooping mode only.
     */
    /* 
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, HL_PRTC_CTRLr, &reg_value, ICMPV4_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_ICMP;
    }
    */

    /* get RARP trap setting */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            RARP_ENf, &temp);
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_RARP;
    }

    /* get ARP trap setting */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            ARP_ENf, &temp);
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_ARP;
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
drv_bcm53115_snoop_set(int unit, uint32 snoop_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp;
    uint32  trap_mask = 0;

    soc_cm_debug(DK_VERBOSE, 
            "drv_snoop_set: unit = %d, snoop mask= %x\n",
            unit, snoop_mask);

    /* get the snooping mask firet to prevent the unexpect value set causes 
     *  other snoop setting been effected.
     *  - prevent the IGMP/MLD/ICMP/DHCP/ARP/RARP been modified unexpected.
     */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->trap_get)
                    (unit, 0, &trap_mask));

    if ((rv = REG_READ_HL_PRTC_CTRLr(unit, &reg_value)) < 0) {
        return rv;
    }
    
    /* Set IGMP ====== */
    /* enable/disable bit setting on all IGMP pkt type snooping */
    if (snoop_mask & DRV_SNOOP_IGMP) {
        snoop_mask |= DRV_SNOOP_IGMP_UNKNOW | 
                    DRV_SNOOP_IGMP_QUERY | 
                    DRV_SNOOP_IGMP_REPLEV;
    }
    if (snoop_mask & DRV_SNOOP_IGMP_DISABLE){
        temp = 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_UKN_ENf, &temp);
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_QRY_ENf, &temp);
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_RPTLVE_ENf, &temp);
    } else {
                
        /* Set IGMP related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_QUERY) || 
                (snoop_mask & DRV_SNOOP_IGMP_QUERY)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_QRY_ENf, &temp);
        if (temp) {
            if (snoop_mask & DRV_SNOOP_IGMP_QUERY){
                temp = BCM53115_REG_VALUE_SNOOP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    IGMP_QRY_FWD_MODEf, &temp);
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_REPLEV) || 
                (snoop_mask & DRV_SNOOP_IGMP_REPLEV)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_RPTLVE_ENf, &temp);
        if (temp) {
            if (snoop_mask & DRV_SNOOP_IGMP_REPLEV){
                temp = BCM53115_REG_VALUE_SNOOP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    IGMP_RPTLVE_FWD_MODEf, &temp);
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_UNKNOW) || 
                (snoop_mask & DRV_SNOOP_IGMP_UNKNOW)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            IGMP_UKN_ENf, &temp);
        if (temp) {
            if (snoop_mask & DRV_SNOOP_IGMP_UNKNOW){
                temp = BCM53115_REG_VALUE_SNOOP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    IGMP_UKN_FWD_MODEf, &temp);
            }
        }
    }

    /* set MLD ====== */
    /* enable/disable bit setting on all MLD pkt type snooping */
    if (snoop_mask & DRV_SNOOP_MLD) {
        snoop_mask |= DRV_SNOOP_MLD_QUERY | DRV_SNOOP_MLD_REPDONE;
    }
    if (snoop_mask & DRV_SNOOP_MLD_DISABLE){
        temp = 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            MLD_QRY_ENf, &temp);
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            MLD_RPTDONE_ENf, &temp);
    } else {
        /* Set MLD related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        temp = ((trap_mask & DRV_SWITCH_TRAP_MLD_QUERY) || 
                    (snoop_mask & DRV_SNOOP_MLD_QUERY)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            MLD_QRY_ENf, &temp);
        if (temp) {
            if (snoop_mask & DRV_SNOOP_MLD_QUERY){
                temp = BCM53115_REG_VALUE_SNOOP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    MLD_QRY_FWD_MODEf, &temp);
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_MLD_REPDONE) || 
                    (snoop_mask & DRV_SNOOP_MLD_REPDONE)) ? 1 : 0;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            MLD_RPTDONE_ENf, &temp);
        if (temp) {
            if (snoop_mask & DRV_SNOOP_MLD_REPDONE){
                temp = BCM53115_REG_VALUE_SNOOP;
                soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
                    MLD_RPTDONE_FWD_MODEf, &temp);
            }
        }
    }
        
    /* set ICMP ====== */
    /* set enable bits */
    temp = (snoop_mask & DRV_SNOOP_ICMP) ? 1 : 0;
    soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            ICMPV4_ENf, &temp);
        
    /* set ICMPv6 trap :  */
    temp = (snoop_mask & DRV_SNOOP_ICMPV6_DISABLE) ? 0 : 
                (((trap_mask & DRV_SWITCH_TRAP_ICMPV6) || 
                    (snoop_mask & DRV_SNOOP_ICMPV6)) ? 1 : 0);
    soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            ICMPV6_ENf, &temp);
    if (temp){
        /* set working mode */
        temp = (snoop_mask & DRV_SNOOP_ICMPV6) ? 
                BCM53115_REG_VALUE_SNOOP : BCM53115_REG_VALUE_TRAP;
        soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            ICMPV6_FWD_MODEf, &temp);
    }
        
    /* set DHCP ====== */
    /* set enable bits */
    temp = (snoop_mask & DRV_SNOOP_DHCP) ? 1 : 0;
    soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            DHCP_ENf, &temp);
    
    /* set RARP ====== */
    /* set enable bits */
    temp = (snoop_mask & DRV_SNOOP_RARP) ? 1 : 0;
    soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            RARP_ENf, &temp);
    
    /* set ARP ====== */
    /* set enable bits */
    temp = (snoop_mask & DRV_SNOOP_ARP) ? 1 : 0;
    soc_HL_PRTC_CTRLr_field_set(unit, &reg_value, 
            ARP_ENf, &temp);
    
    if ((rv = REG_WRITE_HL_PRTC_CTRLr(unit, &reg_value)) < 0) {
        return rv;
    }
    
    /* enable unknow Multicast fowardeing for IGMP/MLD 
     *      1. Enable Multicast unknow forwarding mode. (for application can 
     *          indicate the MLF frame to defined routers.
     *      2. IP_MCf is defined as default set always in Register Spec. and 
     *          suggest not to set to zero. Here we set this field again to 
     *          avoid the improer field value.
     */
    if ((snoop_mask & DRV_SNOOP_IGMP) && 
            (!(SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
            SOC_IS_ROBO53128(unit)))) {
        if ((rv = REG_READ_NEW_CTRLr(unit, &reg_value)) < 0) {
            return rv;
        }
        temp = 1;
        soc_NEW_CTRLr_field_set(unit, &reg_value,
            IP_MCf, &temp);
        if ((rv = REG_WRITE_NEW_CTRLr(unit, &reg_value)) < 0) {
            return rv;
        }
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
drv_bcm53115_snoop_get(int unit, uint32 *snoop_mask)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp = 0;
    int     enable_flag;    /* for igmp/mld only */
    
    if ((rv = REG_READ_HL_PRTC_CTRLr(unit, &reg_value)) < 0) {
        return rv;
    }
    
    /* ---- get IGMP snoop setting ---- */ 
    *snoop_mask = 0;
    enable_flag = FALSE;
    /* snoop status on IGMP query */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_QRY_ENf, &temp);
    if (temp){
        enable_flag = TRUE;

        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_QRY_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_SNOOP){
            *snoop_mask |= DRV_SNOOP_IGMP_QUERY;
        }
    }
    
    /* snoop status on IGMP report/leave */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_RPTLVE_ENf, &temp);
    if (temp){
        enable_flag = TRUE;

        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_RPTLVE_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_SNOOP){
            *snoop_mask |= DRV_SNOOP_IGMP_REPLEV;
        }
    }

    /* snoop status on IGMP unknow */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_UKN_ENf, &temp);
    if (temp){
        enable_flag = TRUE;

        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            IGMP_UKN_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_SNOOP){
            *snoop_mask |= DRV_SNOOP_IGMP_UNKNOW;
        }
    }
    
    /* snoop status on IGMP all types information */
    if (!enable_flag){
        *snoop_mask |= DRV_SNOOP_IGMP_DISABLE;
    }
    
    if ((*snoop_mask & DRV_SNOOP_IGMP_QUERY) && 
            (*snoop_mask & DRV_SNOOP_IGMP_REPLEV) &&
            (*snoop_mask & DRV_SNOOP_IGMP_UNKNOW)){
                
        *snoop_mask |= DRV_SNOOP_IGMP;
    }

    /* ---- get MLD trap setting ---- */ 
    enable_flag = FALSE;
    /* snoop status on MLD query */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            MLD_QRY_ENf, &temp);
    if (temp){
        enable_flag = TRUE;

        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            MLD_QRY_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_SNOOP){
            *snoop_mask |= DRV_SNOOP_MLD_QUERY;
        }
    }
    
    /* snoop status on MLD report */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            MLD_RPTDONE_ENf, &temp);
    if (temp){
        enable_flag = TRUE;

        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            MLD_RPTDONE_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_SNOOP){
            *snoop_mask |= DRV_SNOOP_MLD_REPDONE;
        }
    }
    
    /* snoop status on MLD all types information */
    if (!enable_flag){
        *snoop_mask |= DRV_SNOOP_MLD_DISABLE;
    }
    
    if ((*snoop_mask & DRV_SNOOP_MLD_QUERY) && 
            (*snoop_mask & DRV_SNOOP_MLD_REPDONE)){
                
        *snoop_mask |= DRV_SNOOP_MLD;
    }
        
    /* get ICMPv6 snoop setting */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            ICMPV6_ENf, &temp);
    if (temp) {
        soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            ICMPV6_FWD_MODEf, &temp);
        if (temp == BCM53115_REG_VALUE_SNOOP){
            *snoop_mask |= DRV_SNOOP_ICMPV6;
        }
    } else {
        *snoop_mask |= DRV_SNOOP_ICMPV6_DISABLE;
    }

    
    /* get ICMP */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            ICMPV4_ENf, &temp);
    if (temp) {
        *snoop_mask |= DRV_SNOOP_ICMP;
    }

    /* get DHCP */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            DHCP_ENf, &temp);
    if (temp) {
        *snoop_mask |= DRV_SNOOP_DHCP;
    }
    
    /* get RARP */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            RARP_ENf, &temp);
    if (temp) {
        *snoop_mask |= DRV_SNOOP_RARP;
    }
    
    /* get ARP */
    soc_HL_PRTC_CTRLr_field_get(unit, &reg_value, 
            ARP_ENf, &temp);
    if (temp) {
        *snoop_mask |= DRV_SNOOP_ARP;
    }
    
    soc_cm_debug(DK_VERBOSE, "drv_snoop_get: unit = %d, snoop mask= %x\n",
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
 *      1. This routine can serve bcm53118/53101/53125/53128 due to the same 
 *          chip spec.
 */
int 
drv_bcm53115_igmp_mld_snoop_mode_get(int unit, int type, int *mode)
{
    uint32  reg_val = 0, temp_en = 0, temp_fwd = 0;
    
    if (!(SOC_IS_ROBO_ARCH_VULCAN(unit))){

        /* improper for other chips */
        return SOC_E_INTERNAL;
    }
    
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr(unit, &reg_val));
    temp_fwd = BCM53115_REG_VALUE_SNOOP;    /* preassign to value = 0 */
    
    if (type == DRV_IGMP_MLD_TYPE_IGMP_MLD){
        /* No support on this type due to IGMP and MLD on this chip can be 
         * configured independently.
         */
        return SOC_E_UNAVAIL;
    } else if ((type == DRV_IGMP_MLD_TYPE_IGMP) || 
            (type == DRV_IGMP_MLD_TYPE_IGMP_REPLEV)){
        /* to report the IGMP status, if the snooping function is enabled,
         *  we check the IGMP report/leave item to reflect the snooping mode
         *  on this type. The designing reason is the major IGMP snooping 
         *  process is for handling report/leave request. Also, we observed 
         *  that ESW chps have the same design.
         */
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                    &reg_val, IGMP_RPTLVE_ENf, &temp_en));
        if (temp_en){
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                        &reg_val, IGMP_RPTLVE_FWD_MODEf, &temp_fwd));
        }
    } else if ((type == DRV_IGMP_MLD_TYPE_MLD) || 
            (type == DRV_IGMP_MLD_TYPE_MLD_REPDONE)){
        /* to report the MLD status, if the snooping function is enabled,
         *  we check the MLD report/leave item to reflect the snooping mode
         *  on this type. The designing reason is the major MLD snooping 
         *  process is for handling report/leave request. Also, we observed 
         *  that ESW chips have the same design.
         */
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                    &reg_val, MLD_RPTDONE_ENf, &temp_en));
        if (temp_en){
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                        &reg_val, MLD_RPTDONE_FWD_MODEf, &temp_fwd));
        }
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_QUERY){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                    &reg_val, IGMP_QRY_ENf, &temp_en));
        if (temp_en){
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                        &reg_val, IGMP_QRY_FWD_MODEf, &temp_fwd));
        }
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_UNKNOWN){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                    &reg_val, IGMP_UKN_ENf, &temp_en));
        if (temp_en){
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                        &reg_val, IGMP_UKN_FWD_MODEf, &temp_fwd));
        }
    } else if (type == DRV_IGMP_MLD_TYPE_MLD_QUERY){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                    &reg_val, MLD_QRY_ENf, &temp_en));
        if (temp_en){
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get(unit, 
                        &reg_val, MLD_QRY_FWD_MODEf, &temp_fwd));
        }
    } else {
        return SOC_E_UNAVAIL;
    }

    *mode = (!temp_en) ? DRV_IGMP_MLD_MODE_DISABLE : 
            ((temp_fwd == BCM53115_REG_VALUE_SNOOP) ? 
                DRV_IGMP_MLD_MODE_SNOOP : DRV_IGMP_MLD_MODE_TRAP);

    return SOC_E_NONE; 
}

#define _DRV_SNOOP_MODE_NO_CHANGE   0

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
 *      1. This routine can serve bcm53118/53101/53125/53128 due to the same 
 *          chip spec.
 */
int 
drv_bcm53115_igmp_mld_snoop_mode_set(int unit, int type, int mode)
{
    uint32  reg_val = 0, temp_en = 0, temp_fwd = 0;

    if (!(SOC_IS_ROBO_ARCH_VULCAN(unit))){

        /* improper for other chips */
        return SOC_E_INTERNAL;
    }

    /* there are some speical deisgn for BCM53115 : 
     *  1. to set to DISABLE, the Drop will be set to 0.
     *    - This is for the consistent SOC design on all robo chips. 
     *      Such configuration on Drop is actually a dummy configuration 
     *      due to bcm53115 have no PURE Drop action on IGMP/MLD relate 
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
        temp_fwd = BCM53115_REG_VALUE_SNOOP;    /* value = 0 */
    } else if (mode == DRV_IGMP_MLD_MODE_SNOOP){
        temp_en = TRUE;
        temp_fwd = BCM53115_REG_VALUE_SNOOP;
    } else if (mode == DRV_IGMP_MLD_MODE_TRAP || 
            mode == DRV_IGMP_MLD_MODE_ENABLE){
        temp_en = TRUE;
        temp_fwd = BCM53115_REG_VALUE_TRAP;
    } else {
        soc_cm_debug(DK_WARN, "%s,snooping mode out of valid value(%d)\n",
                FUNCTION_NAME(),mode);
        return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr(unit, &reg_val));
    if (type == DRV_IGMP_MLD_TYPE_IGMP_MLD){
        /* No support on this type due to IGMP and MLD on this chip can be 
         * configured independently.
         */
        return SOC_E_UNAVAIL;
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_RPTLVE_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_RPTLVE_FWD_MODEf, &temp_fwd));
        
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_QRY_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_QRY_FWD_MODEf, &temp_fwd));
        
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_UKN_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_UKN_FWD_MODEf, &temp_fwd));
        
    } else if (type == DRV_IGMP_MLD_TYPE_MLD){
         SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                     &reg_val, MLD_RPTDONE_ENf, &temp_en));
         SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                     &reg_val, MLD_RPTDONE_FWD_MODEf, &temp_fwd));
         
         SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                     &reg_val, MLD_QRY_ENf, &temp_en));
         SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                     &reg_val, MLD_QRY_FWD_MODEf, &temp_fwd));
         
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_REPLEV){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_RPTLVE_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_RPTLVE_FWD_MODEf, &temp_fwd));
        
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_QUERY){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_QRY_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_QRY_FWD_MODEf, &temp_fwd));
        
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_UNKNOWN){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_UKN_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, IGMP_UKN_FWD_MODEf, &temp_fwd));
        
    } else if (type == DRV_IGMP_MLD_TYPE_MLD_REPDONE){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, MLD_RPTDONE_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, MLD_RPTDONE_FWD_MODEf, &temp_fwd));
    } else if (type == DRV_IGMP_MLD_TYPE_MLD_QUERY){
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, MLD_QRY_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set(unit, 
                    &reg_val, MLD_QRY_FWD_MODEf, &temp_fwd));
    } else {
        return SOC_E_UNAVAIL;
    }
    
    SOC_IF_ERROR_RETURN(REG_WRITE_HL_PRTC_CTRLr(unit, &reg_val));

    return SOC_E_NONE; 
}
