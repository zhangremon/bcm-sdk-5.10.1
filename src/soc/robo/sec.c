/*
 * $Id: sec.c 1.10 Broadcom SDK $
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

#ifdef STP_BY_EAP_BLK
extern int stp_8021x_block_state[2][SOC_MAX_NUM_PORTS];
extern void bcm_robo_stp_8021x_lock(int unit);
extern void bcm_robo_stp_8021x_unlock(int unit);
#endif /* STP_BY_EAP_BLK */

int
_drv_security_init(int unit)
{
    uint32 reg_addr, reg_value, temp;
    uint64 reg_value64;
    int reg_len, rv = SOC_E_NONE;


    /* enable 802.1x pass while mac secure */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MAC_SEC_CON0r);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MAC_SEC_CON0r, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, MAC_SEC_CON0r, (uint32 *)&reg_value64, 
            MAC_SEC_1X_PASSf, &temp));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }

    /* special frame pass while mac secure */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MAC_SEC_CON3r);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MAC_SEC_CON3r, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    /* only support mac secure on FE ports (BCM5324)*/
    temp = SOC_PBMP_WORD_GET(PBMP_FE_ALL(unit), 0); 
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, MAC_SEC_CON3r, &reg_value, MAC_SEC_BLK_MODEf, &temp));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    /* pass special fream while block mode is set  */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, EAP_GLO_CONr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, EAP_GLO_CONr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    
    /* bit23(EN_DHCP) in register profile is defined but not sync with 
     *    SW source definition. 
     */
    /* only allowed BPDU and lacp frame pass when EAP_BLK_MODE is set */
    temp = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_DHCPf, &temp));

    /*  set bit 22 should program EAP destination IP 0/1 registers also 
     *  especially at DIP_MASK. So here we clear this bit always.
     */
    temp = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_2_DIPf, &temp));
        
    temp = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_ARPf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_2Ff, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_2Ef, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_2Df, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_2Cf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_2Bf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_2Af, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_29f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_28f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_27f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_26f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_25f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_24f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_23f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_22f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_21f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_20f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_10f, &temp));
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_02_04_0Ff, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_BPDUf, &temp));

    /* bit1(DIS_OLD_MAN03) and bit0(EN_EAP_PT_CHK) should be reset 
       to force bcm5324 not to check EAPOL frame type. 
       - Current setting will force EAPOL frame in chip is tagged.
         (for 1Q + 1S enable and force reserve BPDU been tagged) 
    */

    temp = 0; /* force bit0 & bit1 been reset */

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, DIS_OLD_MAN03f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_EAP_PT_CHKf, &temp));
        
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    return rv;
}

int
_drv_bcm5348_security_init(int unit)
{
    uint32 reg_addr, reg_value, temp;
    uint64 reg_value64, temp64;
    int reg_len, rv = SOC_E_NONE;


    /* enable 802.1x pass while mac secure */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MAC_SEC_CON1r);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MAC_SEC_CON1r, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, MAC_SEC_CON1r, (uint32 *)&reg_value64, 
            MAC_SEC_1X_PASSf, &temp));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }

    /* special frame pass while mac secure */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MAC_SEC_CON2r);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MAC_SEC_CON2r, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }

    if (SOC_IS_ROBO5348(unit)) {
    soc_robo_64_pbmp_to_val(unit, &PBMP_ALL(unit), &temp64);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, MAC_SEC_CON2r, (uint32 *)&reg_value64, 
            MAC_SEC_BLK_MODEf, (uint32 *)&temp64));
    } else { /* 5347 */
        temp = SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, MAC_SEC_CON2r, (uint32 *)&reg_value64, 
            MAC_SEC_BLK_MODEf, &temp));
    }
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    /* pass special fream while block mode is set  */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, EAP_GLO_CONr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, EAP_GLO_CONr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    
    /* bit23(EN_DHCP) in register profile is defined but not sync with 
     *    SW source definition. 
     */
    temp = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_DHCPf, &temp));

    /*  set bit 22 should program EAP destination IP 0/1 registers also 
     *  especially at DIP_MASK. So here we clear this bit always.
     */
    temp = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_2_DIPf, &temp));
        
    temp = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_ARPf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_22_2Ff, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_21f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_20f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_11_1Ff, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_10f, &temp));
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_02_04_0Ff, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_MAC_BPDUf, &temp));

    /* bit1(DIS_OLD_MAN03) and bit0(EN_EAP_PT_CHK) should be reset 
       to force bcm5324 not to check EAPOL frame type. 
       - Current setting will force EAPOL frame in chip is tagged.
         (for 1Q + 1S enable and force reserve BPDU been tagged) 
    */

    temp = 0; /* force bit0 & bit1 been reset */

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, DIS_OLD_MAN03f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, EAP_GLO_CONr, &reg_value, EN_EAP_PT_CHKf, &temp));
        
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    return rv;
}


/*
 *  Function : drv_security_set
 *
 *  Purpose :
 *      Set the security mask to teh selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      state   :   port state.
 *      mask     :   security mask.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask)
{
    int             rv = SOC_E_NONE;
    uint32          temp, flag = 0;
    uint32          port, sa_num = 0;
    l2_arl_sw_entry_t   arl_entry;
    soc_pbmp_t sec_mac_bmp;
    uint32 sec_mask;
#ifdef STP_BY_EAP_BLK    
    uint32  reg_addr;
    int reg_len;
    uint64          reg_value64;
#endif /* STP_BY_EAP_BLK */    
    

    soc_cm_debug(DK_PORT, 
        "drv_security_set: unit = %d, bmp= 0x%x, state = %d, mask = 0x%x\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), state, mask);
    switch (state) {
        case DRV_SECURITY_PORT_UNCONTROLLED:
            /*  1. disable 802.1x
                2. enable Tx/Rx
            */                
            /* disable 802.1x */
#ifdef STP_BY_EAP_BLK            
            PBMP_ITER(bmp, port) {
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                        (unit, PORT_EAP_CONr);

                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, PORT_EAP_CONr, port, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                        return rv;
                }
                bcm_robo_stp_8021x_lock(unit);
                if (stp_8021x_block_state[0][port]) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                stp_8021x_block_state[1][port] =0; /* 8021x non-block */
                bcm_robo_stp_8021x_unlock(unit);
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_EAP_CONr, 
                    (uint32 *)&reg_value64, EAP_ENf, 
                    &temp));
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_EAP_CONr, 
                    (uint32 *)&reg_value64, EAP_BLK_MODEf, 
                    &temp));
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
            }
#else /* !STP_BY_EAP_BLK */            
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                (unit, bmp, DRV_PORT_PROP_802_1X_MODE, FALSE));         
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                (unit, bmp, DRV_PORT_PROP_802_1X_BLK_RX, FALSE));         
#endif /* STP_BY_EAP_BLK */            

            /* enable l2 learning */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->arl_learn_enable_set)
                    (unit, bmp, DRV_PORT_HW_LEARN));    
            }

            SOC_PBMP_ASSIGN(sec_mac_bmp, bmp);
            if (SOC_IS_ROBO5324(unit)) {
                SOC_PBMP_REMOVE(sec_mac_bmp, PBMP_GE_ALL(unit));
            }
            
            /* set no security function */
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                (unit, sec_mac_bmp, DRV_PORT_PROP_SEC_MAC_MODE_NONE, 0));

            /* enable Tx/Rx */
            if ((rv = (DRV_SERVICES(unit)->port_set)
                (unit, bmp, DRV_PORT_PROP_ENABLE_TXRX, 
                    TRUE)) < 0) {
                return rv;
            }
            break;

        case DRV_SECURITY_PORT_UNAUTHENTICATE:
            /*  1. enable 802.1x (BLK_mode == 1)
                2. disbale L2 learnning (set security mode)
                3. remove mac address associted with this port
                4. if (BLOCK_INOUT) -> disable Tx
            */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                SOC_IF_ERROR_RETURN(_drv_bcm5348_security_init(unit));
            } else {
                SOC_IF_ERROR_RETURN(_drv_security_init(unit));
            }
            /* enable 802.1x */
#ifdef STP_BY_EAP_BLK      
            PBMP_ITER(bmp, port) {
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, PORT_EAP_CONr);
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, PORT_EAP_CONr, port, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
                /* allow only EAPOL packets */
                bcm_robo_stp_8021x_lock(unit);
                stp_8021x_block_state[1][port] =1; /* 8021x block */
                bcm_robo_stp_8021x_unlock(unit);
                temp = 1;
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_EAP_CONr, 
                    (uint32 *)&reg_value64, EAP_ENf, &temp));
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_EAP_CONr, 
                    (uint32 *)&reg_value64, EAP_BLK_MODEf, 
                    &temp));
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
            }
#else /* !STP_BY_EAP_BLK */           
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                    (unit, bmp, DRV_PORT_PROP_802_1X_MODE, TRUE));         
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                    (unit, bmp, DRV_PORT_PROP_802_1X_BLK_RX, TRUE));
#endif /* STP_BY_EAP_BLK */

            /* disable l2 learning */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->arl_learn_enable_set)
                    (unit, bmp, DRV_PORT_DISABLE_LEARN));   
            }                               
            /* set security mode */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                if (mask & DRV_SECURITY_VIOLATION_NONE) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_NONE;
                } else if (mask & DRV_SECURITY_VIOLATION_SA_NUM) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM;
                } else if (mask & DRV_SECURITY_VIOLATION_SA_MATCH) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH;
                } else {
                    return SOC_E_PARAM;
                }
            } else {
                if (mask & DRV_SECURITY_VIOLATION_NONE) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_NONE;
                } else if (mask & DRV_SECURITY_VIOLATION_STATIC_ACCEPT) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT;
                } else if (mask & DRV_SECURITY_VIOLATION_STATIC_REJECT) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT;
                } else if (mask & DRV_SECURITY_VIOLATION_SA_NUM) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM;
                } else if (mask & DRV_SECURITY_VIOLATION_SA_MATCH) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH;
                } else {
                    return SOC_E_PARAM;
                }
            }

            SOC_PBMP_ASSIGN(sec_mac_bmp, bmp);
            if (SOC_IS_ROBO5324(unit)) {
                SOC_PBMP_REMOVE(sec_mac_bmp, PBMP_GE_ALL(unit));
            }

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                (unit, sec_mac_bmp, temp, 0));

            PBMP_ITER(bmp, port) {
                /* remove the l2 mac address associated with this port */
                flag |= DRV_MEM_OP_DELETE_BY_PORT | 
                    DRV_MEM_OP_DELETE_BY_STATIC;
                sal_memset(&arl_entry, 0, sizeof(l2_arl_sw_entry_t));
                temp = port;
                if ((rv = (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&arl_entry, &temp)) < 0) {
                    return rv;
                }
                if ((rv = (DRV_SERVICES(unit)->mem_delete)
                    (unit, DRV_MEM_ARL, (uint32 *)&arl_entry, flag)) < 0) {
                    return rv;
                }
            }
            /* disable TX if block in-out or enable Tx anyway */
            if ((rv = (DRV_SERVICES(unit)->port_set)
                    (unit, bmp, DRV_PORT_PROP_ENABLE_TX, 
                    ((mask & DRV_SECURITY_BLOCK_INOUT) ? 
                            FALSE : TRUE))) < 0) {
                return rv;
            }
            break;

        case DRV_SECURITY_PORT_AUTHENTICATED:
            /* 1. enable 802.1x (but reset BLK_mode)
               2. set security mode = NONE (default), 
                    it's depends on customer's definition.
               3. enable Tx when port been AUTHENTICATED and enable 
                  learning mode !
            */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                SOC_IF_ERROR_RETURN(_drv_bcm5348_security_init(unit));
            } else {
                SOC_IF_ERROR_RETURN(_drv_security_init(unit));
            }

                /* enable 802.1x */
#ifdef STP_BY_EAP_BLK                
            PBMP_ITER(bmp, port) {
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, PORT_EAP_CONr);
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, PORT_EAP_CONr, port, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
                temp = 1;
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_EAP_CONr, 
                    (uint32 *)&reg_value64, EAP_ENf, &temp));
                
                bcm_robo_stp_8021x_lock(unit);
                if (stp_8021x_block_state[0][port]) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                stp_8021x_block_state[1][port] =0; /* 8021x non-block */
                bcm_robo_stp_8021x_unlock(unit);
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_EAP_CONr, 
                    (uint32 *)&reg_value64, EAP_BLK_MODEf, 
                    &temp));
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
            } 
#else /*  !STP_BY_EAP_BLK */                
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                    (unit, bmp, DRV_PORT_PROP_802_1X_MODE, TRUE));         
            /* enable Rx */
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                    (unit, bmp, DRV_PORT_PROP_802_1X_BLK_RX, FALSE));
#endif /* STP_BY_EAP_BLK */            

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->dev_prop_get)
                    (unit, DRV_DEV_PROP_AUTH_SEC_MODE, &sec_mask));         

            if (!(mask & sec_mask)) {
                    return SOC_E_UNAVAIL;                
            }
            if (SOC_IS_ROBO5324(unit)) {
                /* bcm5324 does not support disable learning*/
                if (!(mask & DRV_SECURITY_LEARN)) {
                    return SOC_E_UNAVAIL;
                }
            } else {            
                if (mask & MAC_SEC_FLAGS) {
                    if (mask & DRV_SECURITY_LEARN){
                        /* if we have mac sec criterion,  should not be learnd*/ 
                        return SOC_E_PARAM;
                    }
                }
            }
                                            
            /* default at no secutiry mode */
            temp = DRV_PORT_PROP_SEC_MAC_MODE_NONE; 

            /* set security mode */
            if (mask & DRV_SECURITY_VIOLATION_SA_MATCH) {
                /* unknow sa will be drop*/
                temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH;
            }

            if (SOC_IS_ROBO5324(unit)) {
                /* mac sec*/
                if (mask & DRV_SECURITY_VIOLATION_STATIC_ACCEPT) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT;
                } else if (mask & DRV_SECURITY_VIOLATION_STATIC_REJECT) {
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT;
                }
            }
            
            if (mask & DRV_SECURITY_VIOLATION_SA_NUM) {
                if (!(mask & DRV_SECURITY_LEARN)) {                    
                    /* for dynamic sa num, should always enable learning*/
                    return SOC_E_PARAM;
                }
                temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM;
                sa_num = 0x1fff;
            } 


            SOC_PBMP_ASSIGN(sec_mac_bmp, bmp);
            if (SOC_IS_ROBO5324(unit)) {
                SOC_PBMP_REMOVE(sec_mac_bmp, PBMP_GE_ALL(unit));
            }

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                    (unit, sec_mac_bmp, temp, sa_num));

            /* enable Tx */
            if ((rv = (DRV_SERVICES(unit)->port_set)
                    (unit, bmp, 
                    DRV_PORT_PROP_ENABLE_TX, TRUE)) < 0) {
                return rv;
            }

                
            if (mask & DRV_SECURITY_IGNORE_LINK) {
                /* not implemented yet */
            } 
                
            if (mask & DRV_SECURITY_IGNORE_VIOLATION) {
                /* not implemented yet */
            }

            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)){
                /* enable l2 learning */
                if (mask & DRV_SECURITY_LEARN) {
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->arl_learn_enable_set)
                        (unit, bmp, DRV_PORT_HW_LEARN));    
                } else {
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->arl_learn_enable_set)
                        (unit, bmp, DRV_PORT_DISABLE_LEARN));   
                }
            }
            break;
        default:
            return SOC_E_PARAM;
    }
    
    return rv;
}

/*
 *  Function : drv_security_get
 *
 *  Purpose :
 *      Get the security mask to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      state   :   port state.
 *      mask     :   security mask.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Now, didn't output the mask value.
 *
 */
int 
drv_security_get(int unit, uint32 port, uint32 *state, uint32 *mask)
{
    int             rv = SOC_E_NONE;
    uint32        temp;
            
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_get)
            (unit, port, DRV_PORT_PROP_802_1X_MODE, &temp));         

    if (temp) {
         SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_get)
            (unit, port, DRV_PORT_PROP_802_1X_BLK_RX, &temp));  
        if (temp) {
            /* Unauthenticate */
            *state = DRV_SECURITY_PORT_UNAUTHENTICATE;
        } else {
            /* Authenticated */
            *state = DRV_SECURITY_PORT_AUTHENTICATED;
        }
    } else {
        *state = DRV_SECURITY_PORT_UNCONTROLLED;
    }
    soc_cm_debug(DK_PORT, 
    "drv_security_get: unit = %d, port= %d, state = %d, mask = 0x%x\n",
        unit, port, *state, *mask);

    return rv;
}

int 
drv_NULL_security_set(int unit, uint32 bmp, uint32 state, uint32 mask)
{
    return SOC_E_UNAVAIL;
}
int 
drv_NULL_security_get(int unit, uint32 port, uint32 *state, uint32 *mask)
{
    return SOC_E_UNAVAIL;
}

int
drv_security_egress_set(int unit, soc_pbmp_t bmp, int enable)
{
    SOC_IF_ERROR_RETURN(DRV_SERVICES(unit)->port_set
                    (unit, bmp, 
                    DRV_PORT_PROP_ENABLE_TX, enable));
    return SOC_E_NONE;
}

int
drv_security_egress_get(int unit, int port, int *enable)
{
    SOC_IF_ERROR_RETURN(DRV_SERVICES(unit)->port_get
                    (unit, port, 
                    DRV_PORT_PROP_ENABLE_TX, (uint32 *) enable));
    return SOC_E_NONE;
}
