/*
 * $Id: mem.c 1.20.56.2 Broadcom SDK $
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
 * SOC Memory (Table) Utilities
 */

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <sal/appl/io.h>

#include <soc/l2x.h>
#include <soc/cmic.h>
#include <soc/error.h>
#include <soc/register.h>
#include <soc/mcm/robo/driver.h>
#include <soc/spi.h>



#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

#define VLAN_MEMORY 2 
#define ARL_MEMORY  3

/*
 *  Function : _drv_mem_search
 *
 *  Purpose :
 *      Search selected memory for the key value
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      key   :   the pointer of the data to be search.
 *      entry     :   entry data pointer (if found).
 *      flags     :   search flags.
 *      index   :   entry index in this memory bank (0 or 1).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. bcm5389 has 2 banks(bins) in each ARL hash bucket.
 *  2. DRV_MEM_OP_SEARCH_CONFLICT
 *      a. will be performed in the MAC+VID hash search section.
 *      b. '*entry_1' not be used but 'entry' will be used as l2_entry array
 *          and this l2_entry array size should not smaller than ARL table  
 *          hash bucket size(bin number).
 *      c. all valid entries in the MAC+VID hash bucket will be reported.
 *
 */
int 
_drv_bcm5389_mem_search(int unit, uint32 mem, uint32 *key, 
                                    uint32 *entry, uint32 *entry_1, uint32 flags, int *index)
{
    int                 rv = SOC_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);
    uint32          count, temp;
    uint32              control = 0;
    uint8           mac_addr_rw[6], temp_mac_addr[6];
    uint64          rw_mac_field, temp_mac_field;
    uint64  entry0, entry1, *mac_vid;
    uint32  result, result1, *input;
    uint32  vid_rw,vid1 = 0, vid2 = 0, valid;
    int             binNum = -1, existed = 0;
    uint32              reg_addr, reg_value;
    int                     reg_len;
    uint32              process, search_valid;
    uint32              mcast_index, src_port;
    int multicast = 0;
    int reg_enum = 0;
    l2_arl_entry_t  l2_arl_entry;

    if (flags & DRV_MEM_OP_SEARCH_DONE) {
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_SRCH_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_SRCH_CTLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_search_done;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_SRCH_CTLr, &reg_value, 
            ARLA_SRCH_STDNf, &temp)) < 0) {
            goto mem_search_done;
        }
        if (temp) {
            rv = SOC_E_BUSY;    
        }else {
            rv = SOC_E_NONE;
        }
mem_search_done:
        return rv;

    }else if (flags & DRV_MEM_OP_SEARCH_VALID_START) {
        /* Set ARL Search Control register */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_SRCH_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_SRCH_CTLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &control, reg_len)) < 0) {
            goto mem_search_valid_start;
        }
    process = 1;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_SRCH_CTLr, 
                &control, ARLA_SRCH_STDNf, &process));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &control, reg_len)) < 0) {
            goto mem_search_valid_start;
        }
mem_search_valid_start:        
        return rv;
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_GET) {
        if (flags & DRV_MEM_OP_SEARCH_PORT) {
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    key, &src_port)) < 0) {
                goto  mem_search_valid_get;
            }
        }
 
        process = 1;
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)(unit, 
                (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_SRCH_CTLr, 0, 0), &control, 1)) < 0) {
               goto  mem_search_valid_get;
            }
            (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_SRCH_CTLr, 
                &control, ARLA_SRCH_STDNf, &process);
            (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_SRCH_CTLr, 
                &control, ARLA_SRCH_VLIDf, &search_valid);
            /* ARL search operation was done */
            if (!process){
                break;
            }
            if (!search_valid){
                continue;
            }
            count = 0;
            if (!(flags & DRV_MEM_OP_SEARCH_PORT)) {
                /* index value */
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                          (unit, ARLA_SRCH_ADRr);
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                           (unit, ARLA_SRCH_ADRr, 0, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    goto mem_search_valid_get;
                }
                if ((rv = (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_ADRr, &reg_value, 
                    ARLA_SRCH_ADRf, key)) < 0) {
                    goto mem_search_valid_get;
                }
            }

            /* Read ARL Search Result VID Register */
            vid_rw = 0;
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                ARLA_SRCH_RSLT_MACVIDr, 0, 0), (uint32 *)&entry0, 
                (DRV_SERVICES(unit)->reg_length_get)
                (unit,ARLA_SRCH_RSLT_MACVIDr))) < 0) {
                goto mem_search_valid_get;
            }
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                ARLA_SRCH_RSLT_MACVIDr, (uint32 *)&entry0, 
                ARLA_SRCH_RSLT_VIDf, &vid_rw);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                ARLA_SRCH_RSLTr, 0, 0), &result, 
                (DRV_SERVICES(unit)->reg_length_get)
                (unit,ARLA_SRCH_RSLTr))) < 0) {
                goto mem_search_valid_get;
            }

            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                ARLA_SRCH_RSLTr, &result, 
                ARLA_SRCH_RSLT_VLIDf, &valid);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                entry, &valid);

            if (valid) {
                binNum = 0;
                /* MAC Address */
                (DRV_SERVICES(unit)->reg_field_get)(unit, 
                    ARLA_SRCH_RSLT_MACVIDr, (uint32 *)&entry0, 
                    ARLA_SRCH_MACADDRf, (uint32 *)&temp_mac_field);

                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                    
                if (temp_mac_addr[0] & 0x01) {
                    multicast = 1;
                } 
                if (flags & DRV_MEM_OP_SEARCH_PORT) {
                    if (multicast) {
                        continue;
                    }
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_SRCH_RSLTr, &result, 
                        PORTID_Rf, &temp);
                    if (temp != src_port) {
                        continue;
                    }
                }
            }
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                entry, &vid_rw);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                entry, (uint32 *)&temp_mac_field);

            if (multicast) { /* mcast address */
                reg_enum = MARLA_SRCH_RSLTr;        

                mcast_index = 0;
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_enum, &result, 
                    FWD_PRT_MAPf, &mcast_index);
                ((DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                    entry, (uint32 *)&mcast_index)); 

                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_enum, &result, 
                    ARLA_SRCH_RSLT_AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_AGE, 
                    entry, &temp); 
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_enum, &result, 
                    ARLA_SRCH_RSLT_PRIf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_PRIORITY, 
                    entry, &temp); 
            } else { /* unicast address */                    
                reg_enum = ARLA_SRCH_RSLTr;

                /* Source Port Number */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_enum, &result, 
                    PORTID_Rf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    entry, &temp); 
            
                /* Priority Queue */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_enum, &result, 
                    ARLA_SRCH_RSLT_PRIf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                    entry, &temp); 

                /* Static Bit */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_enum, &result, 
                    ARLA_SRCH_RSLT_STATICf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &temp); 

                /* Hit bit */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, reg_enum, &result, 
                    ARLA_SRCH_RSLT_AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                    entry, &temp); 
            }
            rv = SOC_E_EXISTS;
            goto mem_search_valid_get;
        }
        rv = SOC_E_TIMEOUT;
mem_search_valid_get:
        return rv;
    } else if (flags & DRV_MEM_OP_BY_INDEX) {
        /* The entry index is inside the input parameter "key". */ 
        /* Read the memory raw data */
        (DRV_SERVICES(unit)->mem_read)(
            unit, mem, *key, 1, (uint32 *)&l2_arl_entry);
        /* Read VALID bit */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                (uint32 *)&l2_arl_entry, &temp);
        if (!temp) {
            return rv;
        }
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp);

        /* Read VLAN ID value */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                (uint32 *)&l2_arl_entry, &vid_rw);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw);
        
        /* Read MAC address bit 12~47 */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                (uint32 *)&l2_arl_entry, (uint32 *)&rw_mac_field);
        SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, rw_mac_field);
        
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                entry, (uint32 *)&temp_mac_field);
        if (temp_mac_addr[0] & 0x01) { /* multicast entry */
            /* Multicast index */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                    (uint32 *)&l2_arl_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &temp);
             /* Age bit */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_MARL, DRV_MEM_FIELD_AGE, 
                    (uint32 *)&l2_arl_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_MARL, DRV_MEM_FIELD_AGE, entry, &temp);
        } else { /* unicast entry */
            /* Source port number */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&l2_arl_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
            /* Priority queue value */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                    (uint32 *)&l2_arl_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, entry, &temp);
            /* Age bit */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                    (uint32 *)&l2_arl_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
        }
        /* Static bit */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                (uint32 *)&l2_arl_entry, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
        
        return rv;

    /* delete by MAC */    
    } else if (flags & DRV_MEM_OP_BY_HASH_BY_MAC) {
        l2_arl_sw_entry_t   *rep_entry;
        int i, valid[ROBO_5389_L2_BUCKET_SIZE];
    
        if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
            for (i = 0; i < ROBO_5389_L2_BUCKET_SIZE; i++){
                /* check the parameter for output entry buffer */
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
                if (rep_entry == NULL){
                    soc_cm_debug(DK_WARN, 
                            "%s,entries buffer not allocated!\n", 
                            FUNCTION_NAME());
                    return SOC_E_PARAM;
                }
        
                sal_memset(rep_entry, 0, sizeof(l2_arl_sw_entry_t));
                valid[i] = FALSE;
            }
        }
    
        ARL_MEM_SEARCH_LOCK(soc);
        /* enable 802.1Q and set VID+MAC to hash */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_search_exit;
        }
        temp = 1;
        if ((rv = (DRV_SERVICES(unit)->reg_field_set)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_ENf, &temp)) < 0){
            goto mem_search_exit;
        }
        /* check IVL or SVL */

        if ((rv = (DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp)) < 0) {
            goto mem_search_exit;
        }
                
        if (temp == 3){     /* VLAN is at IVL mode */
            if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                temp = 3;
            } else {
                temp = 0;
            }
            
            if((rv = (DRV_SERVICES(unit)->reg_field_set)
                (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp)) < 0){
                goto mem_search_exit;
            }
        }
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
             goto mem_search_exit;
        }
        /* Write MAC Address Index Register */
        if ((rv = (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                key, (uint32 *)&rw_mac_field)) < 0) {
            goto mem_search_exit;
        }
        SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_MACr, 0, 0), &rw_mac_field, 6)) < 0) {
            goto mem_search_exit;
        }

        if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
            /* Write VID Table Index Register */
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, key, &vid_rw)) < 0){
                goto mem_search_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_VIDr, 0, 0), &vid_rw, 2)) < 0) {
                goto mem_search_exit;
            }
        }

        /* Write ARL Read/Write Control Register */
        /* Read Operation */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            goto mem_search_exit;
        }
        temp = MEM_TABLE_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, ARL_RWf, &temp);
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            goto mem_search_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                goto mem_search_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
            if (!temp)
                break;
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_search_exit;
        }
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_FWD_ENTRY0r);
        /* Read Operation sucessfully */
        /* Get the ARL Entry 0/1 Register */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_FWD_ENTRY0r, 0, 0), &result, reg_len)) < 0) {
            goto mem_search_exit;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_FWD_ENTRY1r, 0, 0), &result1, reg_len)) < 0) {
            goto mem_search_exit;
        }

        /* check DRV_MEM_OP_REPLACE */
        if (flags & DRV_MEM_OP_REPLACE) {
            uint32 temp_valid1 = 0;
            uint32 temp_valid2 = 0;
            uint32 temp_static1 = 0;
            uint32 temp_static2 = 0;

            soc_cm_debug(DK_ARL, 
                "DRV_MEM_OP_REPLACE\n");

            if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                soc_cm_debug(DK_ARL, 
                    "%s, Conflict search won't be performed in REPLACE!\n",
                    FUNCTION_NAME());
            }

            /* Check the ARL Entry 0 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_FWD_ENTRY0r, (uint32 *)&result, 
                ARL_VALIDf, &temp_valid1);

            /* Check the ARL Entry 0 Register Static*/
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_FWD_ENTRY0r, (uint32 *)&result, 
                ARL_STATICf, &temp_static1);

            /* Check the ARL Entry 1 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_FWD_ENTRY1r, (uint32 *)&result1, 
                ARL_VALIDf, &temp_valid2);

            /* Check the ARL Entry 1 Register Static*/
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_FWD_ENTRY1r, (uint32 *)&result1, 
                ARL_STATICf, &temp_static2);

            if (temp_valid1) {
                /* bin 0 valid, check mac or mac+vid */
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_MACVID_ENTRY0r, 0, 0), &entry0, 8)) < 0) {
                        goto mem_search_exit;
                }
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_MACVID_ENTRY0r, (uint32 *)&entry0, ARL_MACADDRf,
                    (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_MACVID_ENTRY0r, (uint32 *)&entry0, VID_Rf,
                    &vid1);
                
                /* check if we have to overwrite this valid bin0 */
                if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                    /* check mac + vid */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6) && 
                        (vid1 == vid_rw)) {
                        /* select bin 0 to overwrite it */
                            binNum = 0;
                    }
                } else {
                    /* check mac */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        /* select bin 0 to overwrite it */                  
                            binNum = 0;
                    }                
                } 
            } 

            if (temp_valid2) {
                /* bin 0 valid, check mac or mac+vid */
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_MACVID_ENTRY1r, 0, 0), &entry1, 8)) < 0) {
                        goto mem_search_exit;
                }
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_MACVID_ENTRY1r, (uint32 *)&entry1, ARL_MACADDRf,
                    (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_MACVID_ENTRY1r, (uint32 *)&entry1, VID_Rf,
                    &vid2);
                
                /* check if we have to overwrite this valid bin0 */
                if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                    /* check mac + vid */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6) && 
                        (vid2 == vid_rw)) {
                        /* select bin 1 to overwrite it */
                            binNum = 1;
                    }
                } else {
                    /* check mac */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        /* select bin 1 to overwrite it */             
                            binNum = 1;
                    }                
                } 
            } 

            /* can't find a entry to overwrite based on same mac + vid */
            if (binNum == -1) {
                if (temp_valid1 == 0) {
                    binNum = 0;
                } else if (temp_valid2 == 0) {
                    binNum = 1;
                } else {
                    /* both valid, pick non-static one */
                    if (temp_static1 == 0) {
                        binNum = 0;
                    } else if (temp_static2 == 0) {
                        binNum = 1;
                    } else {
                        /* table full */
                        rv = SOC_E_FULL;
                        goto mem_search_exit;                    
                    }
                }
            }    
        /* Not DRV_MEM_OP_REPLACE */
        } else {
            /* Check the ARL Entry 0 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_FWD_ENTRY0r, &result, ARL_VALIDf, &temp);
            if (temp) {

                if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
                    binNum = 0;
                    valid[0] = TRUE;
                } else {
                    /* this entry if valid, check to see if this is the MAC */
                    if ((rv = (DRV_SERVICES(unit)->reg_read)
                        (unit, (DRV_SERVICES(unit)->reg_addr)
                        (unit, ARLA_MACVID_ENTRY0r, 0, 0), &entry0, 8)) < 0) {
                            goto mem_search_exit;
                    }
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_MACVID_ENTRY0r, (uint32 *)&entry0, ARL_MACADDRf,
                        (uint32 *)&temp_mac_field);
                    SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_MACVID_ENTRY0r, (uint32 *)&entry0, VID_Rf,
                        &vid1);
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                            if (vid1 == vid_rw) {
                                binNum = 0;
                                existed = 1;
                            }
                        } else {
                            binNum = 0;
                            existed = 1;
                        }
                    }
                }

            } else {
                binNum = 0;
            }

            /* Check the ARL Entry 1 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_FWD_ENTRY1r, &result1, ARL_VALIDf, &temp);
            if (temp) {

                if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
                    binNum = 0;
                    valid[1] = TRUE;
                } else {
                    /* this entry if valid, check to see if this is the MAC */
                    if ((rv = (DRV_SERVICES(unit)->reg_read)
                        (unit, (DRV_SERVICES(unit)->reg_addr)
                        (unit, ARLA_MACVID_ENTRY1r, 0, 0), &entry1, 8)) < 0) {
                            goto mem_search_exit;
                    }
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_MACVID_ENTRY1r, (uint32 *)&entry1, ARL_MACADDRf,
                        (uint32 *)&temp_mac_field);
                    SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_MACVID_ENTRY1r, (uint32 *)&entry1, VID_Rf,
                        &vid2);
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                            if (vid2 == vid_rw) {
                                binNum = 1;
                                existed = 1;
                            }
                        } else {
                            binNum = 1;
                            existed = 1;
                        }
                    }
                }
            } else {
                if (binNum == -1) binNum = 1;
            }

            /* if no entry found, fail */
            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)) {
                if (binNum == -1) {
                    rv = SOC_E_FULL;
                    goto mem_search_exit;
                }
             }

        }

        for (i = 0; i < ROBO_5389_L2_BUCKET_SIZE; i++){
            if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                if (valid[i] == FALSE){
                    continue;
                }

                existed = 1;
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
            } else {
                /* match basis search */
                if (i != binNum){
                    continue;
                }

                rep_entry = (l2_arl_sw_entry_t *)entry;
            }

            /* assign the processing hw arl entry */
            if (i == 0) {
                mac_vid = &entry0;
                input = &result;
                vid_rw = vid1;
            } else {
                mac_vid = &entry1;
                input = &result1;
                vid_rw = vid2;
            }
            *index = i;

            /* Only need write the selected Entry Register */
            (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_MACVID_ENTRY0r, (uint32 *)mac_vid, 
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            
            (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                        (uint32 *)rep_entry, (uint32 *)&temp_mac_field);
                       
            if (temp_mac_addr[0] & 0x01) { /* The input is the mcast address */
                
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, MARLA_FWD_ENTRY0r, input, 
                        FWD_PRT_MAPf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                        (uint32 *)rep_entry, &temp);
                
            } else { /* The input is the unicast address */
                
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_FWD_ENTRY0r, (uint32 *)input, 
                        PORTID_Rf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        (uint32 *)rep_entry, &temp);
                
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_FWD_ENTRY0r, (uint32 *)input, 
                        ARL_AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                        (uint32 *)rep_entry, &temp);
                
            }
            (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                    (uint32 *)rep_entry, &vid_rw);
            temp = 1;
            (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                    (uint32 *)rep_entry, &temp);
            (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_FWD_ENTRY0r, (uint32 *)input, 
                    ARL_PRIf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                    (uint32 *)rep_entry, &temp);
            
            (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_FWD_ENTRY0r, (uint32 *)input, 
                    ARL_STATICf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    (uint32 *)rep_entry, &temp);
            
        }
        
        
        if (flags & DRV_MEM_OP_REPLACE) {
            rv = SOC_E_NONE;
        } else {
            if(existed){
                rv = SOC_E_EXISTS;    
            } else {
                rv = SOC_E_NOT_FOUND;
            }
        }
        
mem_search_exit:
    ARL_MEM_SEARCH_UNLOCK(soc);
    return rv;


    } else {
        return SOC_E_PARAM;
    }

}
/*
 *  Function : _drv_vlan_mem_read
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the read result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm5389_vlan_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32 retry;
    uint32  reg_len, reg_addr, temp, reg_value;
    vlan_1q_entry_t *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0;i < count; i++ ) {        
        index = entry_id + i;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_ADDRr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_ADDRr);

        /* fill index */
        reg_value = 0;
        temp = index;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_ADDRr, &reg_value, VTBL_ADDR_INDEXf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        /* read control setting */                
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_RWCTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_RWCTRLr);

        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        temp = MEM_TABLE_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_RWf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_read_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }

        /* get result */        
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_ENTRYr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_ENTRYr);

        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_VTBL_ENTRYr, 
            &reg_value, VALID_Rf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_VALID, 
                 (uint32 *)vlan_mem, &temp);
        
        (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_VTBL_ENTRYr, 
            &reg_value, FWD_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP, 
            (uint32 *)vlan_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_get)(unit, ARLA_VTBL_ENTRYr, 
            &reg_value, UNTAG_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG, 
            (uint32 *)vlan_mem, &temp);
        

        vlan_mem++;
    }

mem_read_exit:  
    return rv;
}

/*
 *  Function : _drv_bcm5396_vlan_mem_write
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the write result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm5389_vlan_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_len, reg_addr, reg_value;
    uint32  retry, temp;
    vlan_1q_entry_t *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0;i < count; i++ ) {
        index = entry_id + i;

        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_ENTRYr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_ENTRYr);

       reg_value = 0;
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_VALID, 
             (uint32 *)vlan_mem, &temp);
        (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_VTBL_ENTRYr, 
             &reg_value, VALID_Rf, &temp);
       
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP, 
             (uint32 *)vlan_mem, &temp);           
        (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_VTBL_ENTRYr, 
             &reg_value, FWD_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG, 
             (uint32 *)vlan_mem, &temp);            
        (DRV_SERVICES(unit)->reg_field_set)(unit, ARLA_VTBL_ENTRYr, 
             &reg_value, UNTAG_MAPf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_write_exit;
        }

        /* fill index */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_ADDRr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_ADDRr);
        reg_value = 0;
        temp = index;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_ADDRr, &reg_value, VTBL_ADDR_INDEXf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_write_exit;
        }

        /* read control setting */                
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VTBL_RWCTRLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ARLA_VTBL_RWCTRLr);

        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_write_exit;
        }

        temp = MEM_TABLE_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_RWf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_write_exit;
        }
        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_write_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_VTBL_RWCTRLr, &reg_value, ARLA_VTBL_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_write_exit;
        }

            vlan_mem ++;
    }

mem_write_exit:     
    return rv;
} 


/*
 *  Function : drv_mem_read
 *
 *  Purpose :
 *      Get the width of selected memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the read result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
drv_bcm5389_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max, index;
    uint32 mem_id, temp;
    uint32 acc_ctrl = 0;
    uint32 mem_addr = 0;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint64 mem_data0, mem_data1;
    int reg_low, reg_high, reg_low_len, reg_high_len;
    uint8   temp_mac_addr[6];
    uint64  temp_mac_field;
    l2_arl_entry_t *arl_mem = 0;
    vlan_1q_entry_t *vlan_mem = 0;

    soc_cm_debug(DK_MEM, 
        "drv_mem_read(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
            mem_id = L2_ARLm;
            arl_mem = (l2_arl_entry_t *)entry;
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            vlan_mem = (vlan_1q_entry_t *)entry;
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
        default:
            return SOC_E_PARAM;
    }

    
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    /* check count */
    if (count < 1) {
        return SOC_E_PARAM;
    }

    /* process read action */
    MEM_LOCK(unit, GEN_MEMORYm);
    /* add code here to check addr */
    if (mem_id == L2_ARLm ) {        
        if ((rv = REG_READ_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }

        temp = ARL_MEMORY;
        soc_MEM_CTRLr_field_set(unit,&acc_ctrl, 
            MEM_TYPEf, &temp);
        
        if ((rv = REG_WRITE_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }
    }else if (mem_id ==VLAN_1Qm) {
        rv = _drv_bcm5389_vlan_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
    }

    for (i = 0;i < count; i++ ) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, GEN_MEMORYm);
            return SOC_E_PARAM;
        }

        /* Return data from cache if active */

        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL && CACHE_VMAP_TST(vmap, (entry_id + i))) {
            sal_memcpy(entry, cache + (entry_id + i) * entry_size, entry_size);
            continue;
        }

        /* Read memory address register */
        mem_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, MEM_ADDRr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, mem_addr, &acc_ctrl, 2)) < 0) {
            goto mem_read_exit;
        }
        temp = MEM_TABLE_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_RWf, &temp);


        index = entry_id + i;
        temp = index;
        /* Set memory entry address */
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_ADRf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, mem_addr, &acc_ctrl, 2)) < 0) {
            goto mem_read_exit;}

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_STDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, mem_addr, &acc_ctrl, 2)) < 0) {
            goto mem_read_exit;}

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, mem_addr, &acc_ctrl, 2)) < 0) {
                goto mem_read_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, MEM_ADDRr, &acc_ctrl, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }


        if (mem_id == L2_ARLm) {
            reg_high = MEM_ARL_HIGHr;
            reg_low = MEM_ARL_LOWr;
            
            reg_high_len =DRV_SERVICES(unit)->reg_length_get(unit,reg_high);
            reg_low_len = DRV_SERVICES(unit)->reg_length_get(unit,reg_low);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, reg_low, 0, 0), (uint32 *)&mem_data0, 
                    reg_low_len)) < 0) {
                goto mem_read_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, reg_high, 0, 0), (uint32 *)&mem_data1, 
                    reg_high_len)) < 0) {
                goto mem_read_exit;
            }

            (DRV_SERVICES(unit)->reg_field_get)(unit, reg_low, 
                (uint32 *)&mem_data0, MACADDRf, (uint32 *)&temp_mac_field);                    
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);                    
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                 (uint32 *)arl_mem, (uint32 *)&temp_mac_field); 
            (DRV_SERVICES(unit)->reg_field_get)(unit, reg_low, 
                (uint32 *)&mem_data0, PRIORITY_Rf, &temp);                    
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                 (uint32 *)arl_mem, &temp);

            (DRV_SERVICES(unit)->reg_field_get)(unit, reg_low, 
                (uint32 *)&mem_data0, VALID_Rf, &temp);                    
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                 (uint32 *)arl_mem, &temp);                

            (DRV_SERVICES(unit)->reg_field_get)(unit, reg_high, 
                (uint32 *)&mem_data1, VID_Rf, &temp);                    
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                 (uint32 *)arl_mem, &temp); 
            (DRV_SERVICES(unit)->reg_field_get)(unit, reg_low, 
                (uint32 *)&mem_data0, STATICf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                 (uint32 *)arl_mem, &temp);


            if (temp_mac_addr[0] & 0x01) {
                 (DRV_SERVICES(unit)->reg_field_get)(unit, reg_low, 
                    (uint32 *)&mem_data0, PORTBMPf, &temp);
                 (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                     (uint32 *)arl_mem, &temp);
            } else {
                (DRV_SERVICES(unit)->reg_field_get)(unit, reg_low, 
                    (uint32 *)&mem_data0, AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                     (uint32 *)arl_mem, &temp);
                (DRV_SERVICES(unit)->reg_field_get)(unit, reg_low, 
                    (uint32 *)&mem_data0, PORTID_Rf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                     (uint32 *)arl_mem, &temp);                
            }
            arl_mem ++;
        }
    }

 mem_read_exit:
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;

}


 /*
 *  Function : drv_mem_write
 *
 *  Purpose :
 *      Writes an internal memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the read result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
drv_bcm5389_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{

    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max, index = 0;
    uint32 i;
    uint32 mem_id, temp;
    uint32 acc_ctrl = 0;
    uint32 mem_addr = 0;
    uint32 data_addr0 = 0, data_addr1 = 0;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint64 mem_data0, mem_data1;
    l2_arl_entry_t *arl_mem = 0;
    vlan_1q_entry_t *vlan_mem = 0;
    int reg_low, reg_high, reg_low_len, reg_high_len;
    uint8   temp_mac_addr[6];
    uint64  temp_mac_field;

    
    soc_cm_debug(DK_MEM, "drv_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
            mem_id = L2_ARLm;
            arl_mem = (l2_arl_entry_t *)entry;
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            vlan_mem = (vlan_1q_entry_t *)entry;
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:            
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
            return SOC_E_PARAM;
        default:
            return SOC_E_PARAM;
    }

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);
    
    if (count < 1) {
        return SOC_E_PARAM;
    }

    /* process write action */
    MEM_LOCK(unit, GEN_MEMORYm);
    if (mem_id == L2_ARLm ){        
        if ((rv = REG_READ_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
        
        temp = ARL_MEMORY;
        soc_MEM_CTRLr_field_set(unit,&acc_ctrl, 
            MEM_TYPEf, &temp);
        
        if ((rv = REG_WRITE_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
    } else if (mem_id == VLAN_1Qm){
        rv = _drv_bcm5389_vlan_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;    
    } 
    
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, GEN_MEMORYm);
            return SOC_E_PARAM;
        }

        /* write data */
        mem_addr = (DRV_SERVICES(unit)->reg_addr)
                                (unit, MEM_ADDRr, 0, 0);
        data_addr0 = (DRV_SERVICES(unit)->reg_addr)
                    (unit, MEM_DATA_LOWr, 0, 0);
        data_addr1 = (DRV_SERVICES(unit)->reg_addr)
                    (unit, MEM_DATA_HIGHr, 0, 0);
        COMPILER_64_ZERO(mem_data0);
        COMPILER_64_ZERO(mem_data1);
        index = entry_id + i;
 
        if (mem_id == L2_ARLm) {
            
            reg_high = MEM_ARL_HIGHr;
            reg_low = MEM_ARL_LOWr;

            reg_high_len =(DRV_SERVICES(unit)->reg_length_get)(unit,reg_high);
            reg_low_len = (DRV_SERVICES(unit)->reg_length_get)(unit,reg_low);
                    
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                (uint32 *)arl_mem, (uint32 *)&temp_mac_field); 
            (DRV_SERVICES(unit)->reg_field_set)(unit, reg_low, 
                (uint32 *)&mem_data0, MACADDRf, (uint32 *)&temp_mac_field);                           
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);            
        
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                (uint32 *)arl_mem, &temp); 
            (DRV_SERVICES(unit)->reg_field_set)(unit, reg_low, 
                (uint32 *)&mem_data0, PRIORITY_Rf, (uint32 *)&temp);
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                (uint32 *)arl_mem, &temp); 
            (DRV_SERVICES(unit)->reg_field_set)(unit, reg_low, 
                (uint32 *)&mem_data0, VALID_Rf, &temp);

            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                (uint32 *)arl_mem, &temp); 
            (DRV_SERVICES(unit)->reg_field_set)(unit, reg_low, 
                (uint32 *)&mem_data0, STATICf, &temp); 

            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                (uint32 *)arl_mem, &temp); 
            (DRV_SERVICES(unit)->reg_field_set)(unit, reg_high, 
                (uint32 *)&mem_data1, VID_Rf, (uint32 *)&temp); 

            if (temp_mac_addr[0] & 0x01) {
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                    (uint32 *)arl_mem, &temp); 
                (DRV_SERVICES(unit)->reg_field_set)(unit, reg_low, 
                    (uint32 *)&mem_data0, PORTBMPf, &temp);             
            } else {
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                    (uint32 *)arl_mem, &temp); 
                (DRV_SERVICES(unit)->reg_field_set)(unit, reg_low, 
                    (uint32 *)&mem_data1, AGEf, &temp);
                
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)arl_mem, &temp); 
                (DRV_SERVICES(unit)->reg_field_set)(unit, reg_low, 
                    (uint32 *)&mem_data1, PORTID_Rf, (uint32 *)&temp);                           
            }            

            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, data_addr0, (uint32 *)&mem_data0, reg_high_len)) < 0) {
                goto mem_write_exit;
            }              
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, data_addr1, (uint32 *)&mem_data1, reg_low_len)) < 0) {
                goto mem_write_exit;
            }
            arl_mem ++;
        } 
        /* set memory address */

        temp = index;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_ADRf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, mem_addr, &acc_ctrl, 2)) < 0) {
            goto mem_write_exit;
        }
        
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, mem_addr, &acc_ctrl, 2)) < 0) {
            goto mem_write_exit;
        }
        temp = MEM_TABLE_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_RWf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, MEM_ADDRr, &acc_ctrl, MEM_STDNf, &temp);
        
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, mem_addr, &acc_ctrl, 2)) < 0) {
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, mem_addr, &acc_ctrl, 2)) < 0) {
                goto mem_write_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, MEM_ADDRr, &acc_ctrl, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_write_exit;
        }
        
        /* Write back to cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL) {
            sal_memcpy(cache + (entry_id + i) * entry_size, entry, entry_size);
            CACHE_VMAP_SET(vmap, (entry_id + i));
        }

    }

 mem_write_exit:
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;
    
}

/*
 *  Function : drv_mem_field_get
 *
 *  Purpose :
 *      Extract the value of a field from a memory entry value.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *      field_index    :  field type.
 *      entry   :   entry value pointer.
 *      fld_data   :   field value pointer.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1.For DRV_MEM_MSTP, because there is no port information 
 *          on the parameter list, so it will return all the ports' state.
 *      2. For DRV_MEM_ARL, the entry type will be l2_arl_sw_entry_t 
 *          and the mem_id is L2_ARL_SWm.
 */
int
drv_bcm5389_mem_field_get(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{

    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int         mem_id, field_id;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32              val32;
#endif

    soc_cm_debug(DK_MEM, "drv_mem_field_get(mem=0x%x,field_index=0x%x)\n",
         mem, field_index);
         
    switch (mem)
    {
        case DRV_MEM_ARL_HW:        
        case DRV_MEM_ARL:
            mem_id = L2_ARLm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = PORTID_Rf;
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                soc_cm_debug(DK_MEM, "getting multicast destination bitmap for unicast mem\n");
                mem_id = L2_MARL_SWm;
                field_id =PORTBMPf;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRIORITY_Rf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = AGEf;
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = STATICf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MARL:
            mem_id = L2_MARL_SWm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;            
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id =PORTBMPf;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRIORITY_Rf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = STATICf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = UNTAG_MAPf;
            }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = FORWARD_MAPf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
        default:
            return SOC_E_PARAM;
    }    
    
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(entry);
    assert(fld_data);

    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);
    assert(fieldinfo);
    bp = fieldinfo->bp;
#ifdef BE_HOST
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else {
        val32 = entry[0];
        entry[0] = entry[1];
        entry[1] = val32;
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif


    if (fieldinfo->len < 32) {
        mask = (1 << fieldinfo->len) - 1;
    } else {
        mask = -1;
    }
    
    wp = bp / 32;
    bp = bp & (32 - 1);
    len = fieldinfo->len;

    /* field is 1-bit wide */
    if (len == 1) {
        fld_data[0] = ((entry[FIX_MEM_ORDER_E(wp, meminfo)] >> bp) & 1);
    } else {

    if (fieldinfo->flags & SOCF_LE) {
        for (i = 0; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len > 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;
            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }
            /* get field value --- 32 bits each time */
            fld_data[i] = (entry[FIX_MEM_ORDER_E(wp++, meminfo)] 
                & mask_lo) >> bp;
            if (mask_hi) {
                fld_data[i] |= (entry[FIX_MEM_ORDER_E(wp, meminfo)] 
                    & mask_hi) << (32 - bp);
            }
            i++;
        }
    } else {
        i = (len - 1) / 32;

        while (len > 0) {
            assert(i >= 0);
            fld_data[i] = 0;
            do {
                fld_data[i] = (fld_data[i] << 1) |
                ((entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] >>
                (bp & (32 - 1))) & 1);
                len--;
                bp++;
            } while (len & (32 - 1));
            i--;
        }
    }
    }
#ifdef BE_HOST
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else {
        val32 = entry[0];
        entry[0] = entry[1];
        entry[1] = val32;
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    
    return SOC_E_NONE;
}


 /*
 *  Function : drv_mem_field_set
 *
 *  Purpose :
 *      Set the value of a field in a 8-, 16-, 32, and 64-bit memory¡¦s value.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *      field_index    :  field type.
 *      entry   :   entry value pointer.
 *      fld_data   :   field value pointer.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1.For DRV_MEM_MSTP, because there is no port information 
 *          on the parameter list, so it will set the value on the 
 *          fld_data to memory entry.
 *      2. For DRV_MEM_ARL, the entry type will be l2_arl_sw_entry_t 
 *          and the mem_id is L2_ARL_SWm.
 */
int
drv_bcm5389_mem_field_set(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int         mem_id, field_id;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32               val32;
#endif

    soc_cm_debug(DK_MEM, "drv_mem_field_set(mem=0x%x,field_index=0x%x)\n",
         mem, field_index);
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
            mem_id = L2_ARLm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = PORTID_Rf;
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                soc_cm_debug(DK_MEM, "setting multicast destination bitmap for unicast mem\n");
                field_id = PORTBMPf;
                mem_id = L2_MARL_SWm;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRIORITY_Rf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = AGEf;
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = STATICf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MARL:
            mem_id = L2_MARL_SWm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = MACADDRf;            
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id =PORTBMPf;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = PRIORITY_Rf;
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = VID_Rf;            
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = STATICf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = UNTAG_MAPf;
            }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = FORWARD_MAPf;
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = VALID_Rf;            
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
        default:
            return SOC_E_PARAM;
    }    
    
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(entry);
    assert(fld_data);
    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);
    assert(fieldinfo);
#ifdef BE_HOST
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else {
        val32 = entry[0];
        entry[0] = entry[1];
        entry[1] = val32;
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    
    bp = fieldinfo->bp;
    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = fieldinfo->len; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len > 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;

            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }

            /* set field value --- 32 bits each time */
            entry[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask_lo;
            entry[FIX_MEM_ORDER_E(wp++, meminfo)] |= 
                ((fld_data[i] << bp) & mask_lo);
            if (mask_hi) {
                entry[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask_hi);
                entry[FIX_MEM_ORDER_E(wp, meminfo)] |= 
                    ((fld_data[i] >> (32 - bp)) & mask_hi);
            }

            i++;
        }
    } else {                   
        /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0) {
            len--;
            entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] |=
            (fld_data[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }
#ifdef BE_HOST
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else {
        val32 = entry[0];
        entry[0] = entry[1];
        entry[1] = val32;
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    return SOC_E_NONE;
}

/*
 *  Function : drv_mem_clear
 *
 *  Purpose :
 *      Clear whole memory entries.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 *
 */
int 
drv_bcm5389_mem_clear(int unit, uint32 mem)
{
    int rv = SOC_E_NONE;
    uint32 count;
    int mem_id;
    uint32 *entry;
    uint32 del_id;

    soc_cm_debug(DK_MEM, "drv_mem_clear : mem=0x%x\n", mem);
    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
            mem_id = L2_ARLm;
            /* Clear Dynamic ARL entries */
            ((DRV_SERVICES(unit)->mem_delete)
                (unit, DRV_MEM_ARL, NULL, DRV_MEM_OP_DELETE_ALL_ARL));
            return SOC_E_NONE;
            break;
        case DRV_MEM_MARL:
            mem_id = L2_ARLm;
            rv = ((DRV_SERVICES(unit)->mem_delete)
                (unit, DRV_MEM_MARL, NULL, DRV_MEM_OP_DELETE_ALL_ARL));
            return SOC_E_NONE;
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }
   
    count = soc_robo_mem_index_count(unit, mem_id);
    
    /* Prevent the Coverity Check(multiple sizeof(uint32))*/
    entry = (uint32 *)sal_alloc((sizeof(_soc_mem_entry_null_zeroes) / 
            sizeof(uint32)) * sizeof(uint32),
            "null_entry");
    if (entry == NULL) {
        soc_cm_print("Insufficient memory.\n");
        return SOC_E_MEMORY;
    }
    sal_memset(entry, 0, sizeof(_soc_mem_entry_null_zeroes));

    for (del_id = 0; del_id < count; del_id++) {
        rv = (DRV_SERVICES(unit)->mem_write)
            (unit, mem, del_id, 1, entry);
        if (rv != SOC_E_NONE){
            soc_cm_debug(DK_WARN, "%s : failed at mem_id=%d, entry=%d!\n", 
                    FUNCTION_NAME(), mem, del_id);
            break;
        }
    }
    sal_free(entry);
    
    return rv;

    
}

/*
 *  Function : drv_bcm5396_mem_search
 *
 *  Purpose :
 *      Search selected memory for the key value
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      key   :   the pointer of the data to be search.
 *      entry     :   entry data pointer (if found).
 *      flags     :   search flags.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *
 */
int 
drv_bcm5389_mem_search(int unit, uint32 mem, 
    uint32 *key, uint32 *entry, uint32 *entry1, uint32 flags)
{
    int                 rv = SOC_E_NONE;
    int                 mem_id;
    int                 index;

    soc_cm_debug(DK_MEM, "drv_mem_search : mem=0x%x, flags = 0x%x)\n",
         mem, flags);
    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            mem_id = L2_ARLm;
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    rv = _drv_bcm5389_mem_search(unit, mem, key, entry, 
        entry1, flags, &index);
    
    return rv;

    
}

/*
 *  Function : drv_mem_insert
 *
 *  Purpose :
 *      Insert an entry to specific memory
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      entry     :   entry data pointer.
 *      flags     :   insert flags (no use now).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *      flags = DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID;
 */
int 
drv_bcm5389_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int                 rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t       output;
    uint32          temp, count, mem_id;
    uint64          entry_reg;
    uint32  reg32;            
    uint8           mac_addr[6];
    uint64               mac_field, mac_field_output, mac_field_entry;
    uint32          vid, control, vid_output, vid_entry;
    int index, reg_macvid, reg_fwd, reg_len;

    soc_cm_debug(DK_MEM, "drv_mem_insert : mem=0x%x, flags = 0x%x)\n",
         mem, flags);
    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:            
            mem_id = L2_ARL_SWm;
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }
    MEM_LOCK(unit,L2_ARLm);
    /* search entry */
    sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
    if ((rv =  _drv_bcm5389_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            NULL, flags, &index))< 0) {
        if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS))
            goto mem_insert_exit;              
    }
    /* Return SOC_E_NONE instead of SOC_E_EXISTS to fit DV test. */
    if (rv == SOC_E_EXISTS) {
        if (!sal_memcmp(&output, entry, 
                sizeof(l2_arl_sw_entry_t)) ){
            rv = SOC_E_NONE;
            goto mem_insert_exit;
        } else {
            /* MAC Address */
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&mac_field_output)) < 0) {
                goto mem_insert_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&mac_field_entry)) < 0) {
                goto mem_insert_exit;
            }
            if (!sal_memcmp(&mac_field_entry, &mac_field_output, 
                    sizeof(mac_field_output)) ){
                 /*  VLAN ID  */
                if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)&output, &vid_output)) < 0) {
                    goto mem_insert_exit;
                }
                if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        entry, &vid_entry)) < 0) {
                    goto mem_insert_exit;
                }
                if (vid_output != vid_entry){
                    rv = SOC_E_NONE;
                    goto mem_insert_exit;
                }                
            } else {
               rv = SOC_E_NONE;
                goto mem_insert_exit;
            }
        }
    }

    /* write entry if entry does not exist or mem content changed*/
        /* form entry */
        /* VLAN ID */
        if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid)) < 0) {
            goto mem_insert_exit;
        }
        (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_MACVID_ENTRY0r, (uint32 *)&entry_reg, VID_Rf, &vid);

        /* MAC Address */
        if ((rv = (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                entry, (uint32 *)&mac_field)) < 0) {
            goto mem_insert_exit;
        }
        SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field);
        
        (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_MACVID_ENTRY0r, (uint32 *)&entry_reg, 
                ARL_MACADDRf, (uint32 *)&mac_field);

        if (mac_addr[0] & 0x01) { /* The input is the mcast address */
            /* multicast group index */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
               (unit, MARLA_FWD_ENTRY0r, &reg32, FWD_PRT_MAPf, &temp);
            /* static */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_STATICf, &temp);
            /* valid bit */
            temp = 1;
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, MARLA_FWD_ENTRY0r, &reg32, ARL_VALIDf, &temp);
        } else { /* unicast address */
            /* source port id */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, PORTID_Rf, &temp);

            /* priority */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_PRIf, &temp);

            /* age */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_AGEf, &temp);

            /* static */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_STATICf, &temp);

            temp = 1;
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY0r, &reg32, ARL_VALIDf, &temp);
        }


        /* write ARL and MAC/VID entry register*/
        if (index == 0){ /* entry 0 */
            reg_macvid = ARLA_MACVID_ENTRY0r;
            reg_fwd = ARLA_FWD_ENTRY0r;
        } else {
            reg_macvid = ARLA_MACVID_ENTRY1r;
            reg_fwd = ARLA_FWD_ENTRY1r;
        }
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_fwd);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, reg_macvid, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
            goto mem_insert_exit;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, reg_fwd, 0, 0), &reg32, reg_len)) < 0) {
            goto mem_insert_exit;
        }
       

        /* Write ARL Read/Write Control Register */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            goto mem_insert_exit;
        }
        temp = MEM_TABLE_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, ARL_RWf, &temp);
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            goto mem_insert_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                goto mem_insert_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
            if (!temp)
                break;
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_insert_exit;
        }

        sw_arl_update = 1;
    
mem_insert_exit:
    MEM_UNLOCK(unit,L2_ARLm);

    if (sw_arl_update){
        /* Add the entry to sw database */
        _drv_arl_database_insert(unit, index, entry);
    }

    return rv;

    
}

/*
 *  Function : drv_mem_delete
 *
 *  Purpose :
 *      Remove an entry to specific memory or remove entries by flags 
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      entry     :   entry data pointer.
 *      flags     :   delete flags.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *
 */
int 
drv_bcm5389_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int                 rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t       output;
    uint32               entry_reg;
    uint32          temp, count, mem_id;
    uint32          control;
    int             index;
    uint32          reg_len, reg_addr, reg_value, bmp;
    uint32 ag_port_mode = 0, ag_vlan_mode = 0;
    uint32              ag_static_mode = 0;
    uint32  src_port = 0, vlanid = 0;


    soc_cm_debug(DK_MEM, "drv_mem_delete : mem=0x%x, flags = 0x%x)\n",
         mem, flags);
    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            mem_id = L2_ARL_SWm;
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    if (flags & DRV_MEM_OP_DELETE_BY_PORT) {
        ag_port_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_VLANID) {
        ag_vlan_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_SPT) {
        return SOC_E_UNAVAIL;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_STATIC) {
        ag_static_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
        ag_static_mode = 1;
        ag_port_mode = 1;
    }
    
    if ((ag_port_mode) || (ag_vlan_mode)) {
        /* aging port mode */
        if (ag_port_mode) {
            
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, FAST_AGING_PORTr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FAST_AGING_PORTr, 0, 0);
            reg_value = 0;
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }            
            if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
                temp = 1;
                SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, FAST_AGING_PORTr, &reg_value, 
                        AGE_OUT_ALL_PORTSf, &temp));

            } else { /* by port */
                temp = 0;
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
                src_port = temp;
                bmp = temp;
                SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, FAST_AGING_PORTr, &reg_value, AGE_SRC_PORTf, &bmp));

            }
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }

            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, FAST_AGING_VIDr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FAST_AGING_VIDr, 0, 0);
            reg_value = 0;
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
            temp = 1;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, FAST_AGING_VIDr, &reg_value, 
                        AGE_OUT_ALL_VIDSf, &temp));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }

        } else if (ag_vlan_mode) {
        /* aging vlan mode */
            temp = 0;
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, FAST_AGING_VIDr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FAST_AGING_VIDr, 0, 0);
            reg_value = 0;
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }

            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &temp);
            vlanid = temp;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGING_VIDr, &reg_value, AGE_VIDf, &temp));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }

            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, FAST_AGING_PORTr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, FAST_AGING_PORTr, 0, 0);
            reg_value = 0;
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
 
            temp = 1;
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
                    (unit, FAST_AGING_PORTr, &reg_value, 
                    AGE_OUT_ALL_PORTSf, &temp));

            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }

        }

        /* start fast aging process */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FAST_AGE_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FAST_AGE_CTLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }

        if (ag_static_mode) {
            temp = 1;    
        } else {
            temp = 0;
        }
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_CTLr, &reg_value, 
            EN_FAST_AGE_STATICf, &temp));

        temp = 1;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, FAST_AGE_CTLr, &reg_value, 
            FAST_AGE_START_DONEf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                    (unit, FAST_AGE_CTLr, &reg_value, 
                    FAST_AGE_START_DONEf, &temp));
            if (!temp) {
                break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_delete_exit;
        }
        /* Remove entries from software table by port/vlan */
        _drv_arl_database_delete_by_portvlan(unit, src_port,
            vlanid, flags);
    
    } else { /* normal deletion */
        soc_cm_debug(DK_MEM, 
                    "\t Normal ARL DEL with Static=%d\n",
                    ag_static_mode);

        MEM_LOCK(unit,L2_ARLm);
        /* search entry */
        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
        if ((rv =  _drv_bcm5389_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            NULL, flags, &index))< 0){
            if((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
                if (rv == SOC_E_FULL) {
                /* For mem_delete if mem_search return SOC_E_FULL it means
                  * the entry is not found. */
                    rv = SOC_E_NOT_FOUND;
                }
                goto mem_delete_exit;              
            }
        }

        /* write entry */
        if (rv == SOC_E_EXISTS) {
            /* clear the VALID bit*/
            temp = 0;
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit,ARLA_FWD_ENTRY0r);
            if (index == 0) { 
                /* entry 0 */
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_FWD_ENTRY0r, 0, 0), &entry_reg, reg_len)) < 0){
                    goto mem_delete_exit;
                }
                /* check static bit if set */
                if (!ag_static_mode){
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                            (unit, ARLA_FWD_ENTRY0r, (uint32 *)&entry_reg, 
                                    ARL_STATICf, &temp));
                    if (temp) {
                        soc_cm_debug(DK_MEM, 
                                    "\t Entry exist with static=%d\n",
                                    temp);
                        rv = SOC_E_NOT_FOUND;
                        goto mem_delete_exit;
                    }
                }
                
                temp = 0;
                SOC_IF_ERROR_RETURN(
                    (DRV_SERVICES(unit)->reg_field_set)
                    (unit, ARLA_FWD_ENTRY0r, &entry_reg, ARL_VALIDf, &temp));

                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_FWD_ENTRY0r, 0, 0), &entry_reg, reg_len)) < 0){
                    goto mem_delete_exit;
                }
            } else { 
                
                /* entry 1 */
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_FWD_ENTRY1r, 0, 0), &entry_reg, reg_len)) < 0){
                    goto mem_delete_exit;
                }
                /* check static bit if set */
                if (!ag_static_mode){
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                            (unit, ARLA_FWD_ENTRY1r, (uint32 *)&entry_reg, 
                                    ARL_STATICf, &temp));
                    if (temp) {
                        soc_cm_debug(DK_MEM, 
                                    "\t Entry exist with static=%d\n",
                                    temp);
                        rv = SOC_E_NOT_FOUND;
                        goto mem_delete_exit;

                    }
                }

                temp = 0;
                SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_FWD_ENTRY1r, &entry_reg, ARL_VALIDf, &temp));
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_FWD_ENTRY1r, 0, 0), 
                    (uint32 *)&entry_reg, reg_len)) < 0) {
                    goto mem_delete_exit;
                }
            }

            /* Write ARL Read/Write Control Register */
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                goto mem_delete_exit;
            }
            temp = MEM_TABLE_WRITE;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_RWCTLr, &control, ARL_RWf, &temp));
            temp = 1;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                goto mem_delete_exit;
            }

            /* wait for complete */
            for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                    goto mem_delete_exit;
                }
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
                if (!temp) {
                    break;
                }
            }

            if (count >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto mem_delete_exit;
            }

            sw_arl_update = 1;
        }
    }

mem_delete_exit:
    if (((ag_port_mode) || (ag_vlan_mode) )) {
            if(ag_static_mode){
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, FAST_AGE_CTLr);
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, FAST_AGE_CTLr, 0, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }
                temp = 0;
                SOC_IF_ERROR_RETURN(
                    (DRV_SERVICES(unit)->reg_field_set)
                        (unit, FAST_AGE_CTLr, &reg_value, 
                        EN_FAST_AGE_STATICf, &temp));
               if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }

            }
    }else {
        MEM_UNLOCK(unit,L2_ARLm);
    }

    if (sw_arl_update){
        /* Remove the entry from sw database */
        _drv_arl_database_delete(unit, index, &output);
    }

    return rv;

    
}
