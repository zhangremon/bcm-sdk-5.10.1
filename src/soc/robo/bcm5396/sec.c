/*
 * $Id: sec.c 1.5 Broadcom SDK $
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


static  uint32 port_sec_state[SOC_MAX_NUM_PORTS];

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
drv_bcm5396_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask)
{
    int             rv = SOC_E_NONE;
    uint32          reg_addr, temp, flag = 0;
    uint32          reg_value;
    int             reg_len;
    uint32          port=0;
    l2_arl_sw_entry_t   arl_entry;
    soc_pbmp_t      temp_pbmp;
    uint32 sec_mask;

    soc_cm_debug(DK_PORT, 
        "drv_security_set: unit = %d, bmp= 0x%x, state = %d, mask = 0x%x\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), state, mask);
    switch (state) {
        case DRV_SECURITY_PORT_UNCONTROLLED:
            /*  1. disable 802.1x
                2. enable Tx/Rx
            */
            /* disable 802.1x */
            
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, AUTH_8021X_CTL2r);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, AUTH_8021X_CTL2r, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, AUTH_8021X_CTL2r, &reg_value, PORT_BLOCKf, &temp));
            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, AUTH_8021X_CTL2r, &reg_value, PORT_BLOCKf, &temp));
                
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }

            PBMP_ITER(bmp, port) {
                /* enable Tx/Rx */
                SOC_PBMP_CLEAR(temp_pbmp);
                SOC_PBMP_PORT_ADD(temp_pbmp, port);
                if ((rv = (DRV_SERVICES(unit)->port_set)
                    (unit, temp_pbmp, DRV_PORT_PROP_ENABLE_TXRX, 
                        TRUE)) < 0) {
                    return rv;
                }
                port_sec_state[port] = DRV_SECURITY_PORT_UNCONTROLLED;
            }
            break;
        case DRV_SECURITY_PORT_UNAUTHENTICATE:
            /*  1. enable 802.1x (BLK_mode == 1)
                2. disbale L2 learnning (set security mode)
                3. remove mac address associted with this port
                4. if (BLOCK_INOUT) -> disable Tx
            */

            /* enable 802.1x */
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, AUTH_8021X_CTL2r);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, AUTH_8021X_CTL2r, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, AUTH_8021X_CTL2r, &reg_value, PORT_BLOCKf, &temp));
            temp |= SOC_PBMP_WORD_GET(bmp, 0);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, AUTH_8021X_CTL2r, &reg_value, PORT_BLOCKf, &temp));
                
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }

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
                
                /* disable TX if block in-out or enable Tx anyway */
                SOC_PBMP_CLEAR(temp_pbmp);
                SOC_PBMP_PORT_ADD(temp_pbmp, port);
                if ((rv = (DRV_SERVICES(unit)->port_set)
                        (unit, temp_pbmp, DRV_PORT_PROP_ENABLE_TX, 
                        ((mask & DRV_SECURITY_BLOCK_INOUT) ? 
                                FALSE : TRUE))) < 0) {
                    return rv;
                }
                port_sec_state[port] = DRV_SECURITY_PORT_UNAUTHENTICATE;
            }

            break;
            
        case DRV_SECURITY_PORT_AUTHENTICATED:
            /* 1. enable 802.1x (but reset BLK_mode)
               2. set security mode = NONE (default), 
                    it's depends on customer's definition.
               3. enable Tx when port been AUTHENTICATED and enable 
                  learning/not-learning mode !
                  - not learning mode need to clear all existed L2 entries by 
                    this port. 
               4. Limitation on bcm5396 :
                  a. the security mode allowed below two security mode only :
                    - DRV_SECURITY_VIOLATION_NONE (learning mode)
                    - DRV_SECURITY_VIOLATION_STATIC_ACCEPT(not learning mode,
                       CPU have to help to add the valid L2 entry on this port)
                  b. other the SEC_MODE will got the return value as "UNAVAIL"
               5. For bcm5396, the 802.1x mode will be always been set as zero.
                  (Mode = 802.1x mode[page0x00,addr0xx77,bit0])
                  (PBlock = PortBlock[page0x00,addr0x78,bit0:16])
                         Uncontrol    UnAuth         Auth-learn  Auth-notlearn
                         ======================================================
                  Mode      [0/1]  [0/1]+L2_Clear        [0/1]     0+L2_lear
                  PBlock      0         1                  0          1
                         ======================================================
            */
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->dev_prop_get)
                    (unit, DRV_DEV_PROP_AUTH_SEC_MODE, &sec_mask));         

            if (!(mask & sec_mask)) {
                    return SOC_E_UNAVAIL;                
            }            

            /* check valid configuration on learning mode */
            if (mask & DRV_SECURITY_LEARN){
                if (!(mask & DRV_SECURITY_VIOLATION_NONE )) {
                    return SOC_E_UNAVAIL;
                }
            } else {
                if (!(mask & DRV_SECURITY_VIOLATION_STATIC_ACCEPT )) {
                    return SOC_E_UNAVAIL;
                }
            }
            
            if ((mask & DRV_SECURITY_IGNORE_LINK) ||
                        (mask & DRV_SECURITY_IGNORE_VIOLATION)) {
                /* not implemented yet */
                return SOC_E_UNAVAIL;
            } 
            
            /* enable 802.1x */
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, AUTH_8021X_CTL2r);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, AUTH_8021X_CTL2r, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, AUTH_8021X_CTL2r, &reg_value, PORT_BLOCKf, &temp));

            /* set security learning mode :
             *  - free port block
             */
            if (mask & DRV_SECURITY_LEARN) {
                temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, AUTH_8021X_CTL2r, &reg_value, PORT_BLOCKf, &temp));
                
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }

            /* set security not learning mode 
             *  1. set 802.1x mode(0: drop if SA miss and not 802.1x special 
             *     frame) 
             *  2. set port block
             *  3. remove learned l2 entries by port.
             */
            } else {         
                temp |= SOC_PBMP_WORD_GET(bmp, 0);
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, AUTH_8021X_CTL2r, &reg_value, PORT_BLOCKf, &temp));
                
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }

                reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, AUTH_8021X_CTL1r);
                reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, AUTH_8021X_CTL1r, 0, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }
                temp = 0;                                                         
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, AUTH_8021X_CTL1r, &reg_value, MODE_8021Xf, &temp));

                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }

                /* remove the l2 mac address associated with port */
                PBMP_ITER(bmp, port) {                
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

            }
            
            PBMP_ITER(bmp, port) {                            
                        
                /* enable Tx in learning mode */
                SOC_PBMP_CLEAR(temp_pbmp);
                SOC_PBMP_PORT_ADD(temp_pbmp, port);
                if ((rv = (DRV_SERVICES(unit)->port_set)
                        (unit, temp_pbmp, 
                        DRV_PORT_PROP_ENABLE_TX, TRUE)) < 0) {
                    return rv;
                }
                port_sec_state[port] = DRV_SECURITY_PORT_AUTHENTICATED;
            }
                      
            break;
        default:
            return SOC_E_PARAM;
    }
    
    return rv;
}

/*
 *  Function : drv_bcm5396_security_get
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
 *
 */
int 
drv_bcm5396_security_get(int unit, uint32 port, uint32 *state, uint32 *mask)
{
    int             rv = SOC_E_NONE;
    uint32          reg_addr, temp;
    uint32          reg_value;
    int             reg_len;
    
    *state = port_sec_state[port];
   
    if (*state != DRV_SECURITY_PORT_AUTHENTICATED){
        *mask = 0x0;
    } else {
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, AUTH_8021X_CTL2r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, AUTH_8021X_CTL2r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, AUTH_8021X_CTL2r, &reg_value, PORT_BLOCKf, &temp));
        
        if (temp & (0x1 << port)){
            *mask = DRV_SECURITY_VIOLATION_STATIC_ACCEPT;
        } else {
            *mask = DRV_SECURITY_LEARN | DRV_SECURITY_VIOLATION_NONE;
        }
    }
    
    return rv;
}
