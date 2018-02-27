/*
 * $Id: mem.c 1.35 Broadcom SDK $
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
#include <soc/arl.h>
#include <soc/cfp.h>


#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

#define VLAN_MEMORY 2 
#define ARL_MEMORY  3

/* Define Table Type for Access */
#define ARL_TABLE_ACCESS       0x0
#define VLAN_TABLE_ACCESS      0x1
#define MARL_PBMP_TABLE_ACCESS 0x2
#define MSPT_TABLE_ACCESS      0x3

/*
 *  Function : _drv_bcm5348_mem_search
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
 *  1. The ARL hash bucket size in bcm5348 is 1. That is there is no multiple 
 *      bank(or bin) in a MAC+VID hashed bucket.
 *  2. In the MAC+VID hash search section, there are two search operations 
 *      will be preformed.
 *    a. Match basis search : report the valid entry with MAC+VID matched.
 *    b. Conflict basis search : report the valid entry in the MAC+VID hashed
 *       bucket.
 *
 */
int 
_drv_bcm5348_mem_search(int unit, uint32 mem, uint32 *key, 
                                    uint32 *entry, uint32 *entry_1, uint32 flags, int *index)
{
    int             rv = SOC_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);
    uint32          count, temp, value;
    uint32          control = 0;
    uint8           mac_addr_rw[6], temp_mac_addr[6];
    uint64          rw_mac_field, temp_mac_field;
    uint64          entry0, *input;
    uint32          vid_rw, vid1=0; 
    int             binNum = -1, existed = 0;
    uint32          reg_addr, reg_value;
    int             reg_len;
    uint32          process, search_valid;
    uint32          src_port;
    uint32 mcast_index;

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
            
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_START) {
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
                goto mem_search_valid_get;
            }
        }
 
        process = 1;
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)(unit, 
                (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_SRCH_CTLr, 0, 0), &control, 1)) < 0) {
                goto mem_search_valid_get;
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
                    SRCH_ADRf, key)) < 0) {
                    goto mem_search_valid_get;
                }
            }

            /* Read ARL Search Result VID Register */
            vid_rw = 0;
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                ARLA_SRCH_RSLT_VIDr, 0, 0), &reg_value, 
                (DRV_SERVICES(unit)->reg_length_get)
                (unit,ARLA_SRCH_RSLT_VIDr))) < 0) {
                goto mem_search_valid_get;
            }
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                ARLA_SRCH_RSLT_VIDr, &reg_value, 
                ARLA_SRCH_RSLT_VIDf, &vid_rw);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw);

            /* Read ARL Search Result MAC register */
            (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)(unit, 
                ARLA_SRCH_RSLTr, 0, 0), (uint32 *)&entry0, 8);
                   
            if (flags & DRV_MEM_OP_SEARCH_PORT) {
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_PRIDf, &temp);
                if (temp != src_port) {
                    continue;
                }
            }
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                ARLA_SRCH_RSLT_VLIDf, &temp);
            /* Set VALID field */
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp);

            /* MAC Address */
            (DRV_SERVICES(unit)->reg_field_get)(unit, 
                ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                ARLA_SRCH_RSLT_ADDRf, (uint32 *)&temp_mac_field);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                entry, (uint32 *)&temp_mac_field);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            if (temp_mac_addr[0] & 0x01) { /* mcast address */
                /* The the multicast format didn't define, we need 
                   collect 3 fields to get the multicast index value.
                   multicast index : bit 55~48
                   age : bit 55
                   priority : bit 54~53
                   port id : bit 52~48
                */
                mcast_index = 0;
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_PRIDf, &temp);
                mcast_index = temp;
                if (SOC_IS_ROBO5347(unit)) {
                    mcast_index += 24;
                }
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ENTRY_RSRV0f, &temp);
                mcast_index += (temp << 6);
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_PRIf, &temp);
                mcast_index += (temp << 9);
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_AGEf, &temp);
                mcast_index += (temp << 11);
                ((DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
                    entry, (uint32 *)&mcast_index)); 

                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_STATICf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &temp); 
                /* arl_control */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARL_SRCH_RSLT_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                    entry, &temp); 

            } else { /* unicast address */
                /* Source Port Number */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_PRIDf, &temp);
                ((DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    entry, &temp)); 
           
                /* Priority Queue */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_PRIf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                    entry, &temp); 

                /* Static Bit */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_STATICf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &temp); 

                /* Hit bit */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                    entry, &temp); 

                /* arl_control */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARL_SRCH_RSLT_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                    entry, &temp); 
            }
            rv = SOC_E_EXISTS;
            goto mem_search_valid_get;
        }
        rv = SOC_E_TIMEOUT;
mem_search_valid_get:
        return rv;

    } else if (flags & DRV_MEM_OP_BY_INDEX) {
        /*
         * Since no one calls this case, left it empty temporary.
         */
        soc_cm_print("_drv_bcm5348_mem_search: flag = DRV_MEM_OP_BY_INDEX\n");
        return SOC_E_UNAVAIL;
    /* delete by MAC */    
    } else if (flags & DRV_MEM_OP_BY_HASH_BY_MAC) {

        if (entry == NULL){
            soc_cm_debug(DK_WARN, 
                    "%s,entries buffer not allocated!\n", 
                    FUNCTION_NAME());
            return SOC_E_PARAM;
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
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_ENf, &temp));

        /* check IVL or SVL */
        SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)
                (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));
        if (temp == 3){     /* VLAN is at IVL mode */
            if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                temp = 3;
            } else {
                temp = 0;
            }
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));
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
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, key, &vid_rw)) < 0) {
                goto mem_search_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_VIDr, 0, 0), &vid_rw, 2)) < 0) {
                goto mem_search_exit;
            }
        }

        MEM_RWCTRL_REG_LOCK(soc);
        /* Write ARL Read/Write Control Register */
        /* Read Operation */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_search_exit;
        }
        temp = MEM_TABLE_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, TAB_RWf, &temp);

        temp = ARL_TABLE_ACCESS;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, TAB_INDEXf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_search_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_search_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
            if (!temp)
                break;
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_search_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);

        /* Read Operation sucessfully */
        /* Get the ARL Entry Register */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_ENTRYr, 0, 0), (uint32 *)&entry0, 8)) < 0) {
            goto mem_search_exit;
        }

        /* check DRV_MEM_OP_REPLACE */
        if (flags & DRV_MEM_OP_REPLACE) {
            uint32 temp_static;

            if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
                soc_cm_debug(DK_WARN, 
                        "%s, Conflict search won't be performed\n",
                        FUNCTION_NAME());
            }
            soc_cm_debug(DK_ARL, 
                "DRV_MEM_OP_REPLACE\n");

           /* Check the ARL Entry 0 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)&entry0, ARL_VALIDf, &temp);
           if (temp) {
              /* this entry if valid, check to see if this is the MAC */
              (DRV_SERVICES(unit)->reg_field_get)
                  (unit, ARLA_ENTRYr, (uint32 *)&entry0, ARL_MACADDRf,
                  (uint32 *)&temp_mac_field);
              SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
              (DRV_SERVICES(unit)->reg_read)
                  (unit, (DRV_SERVICES(unit)->reg_addr)
                  (unit, ARLA_VID_ENTRYr, 0, 0), &vid1, 2);
              
                /* Check the ARL Entry 0 Register if static */
              (DRV_SERVICES(unit)->reg_field_get)
                  (unit, ARLA_ENTRYr, (uint32 *)&entry0, ARL_STATICf,
                 &temp_static);

               /* check if we have to overwrite this valid bin0 */
               if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                   /* check mac + vid */
                   if (!memcmp(temp_mac_addr, mac_addr_rw, 6) && 
                       (vid1 == vid_rw)) {
                       /* select bin 1 to overwrite it */              
                           binNum = 0;
                   }
               } else {
                   /* check mac */
                   if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                       /* select bin 1 to overwrite it */                  
                           binNum = 0;
                   }                
               } 
           } 
        
           /* can't find a entry to overwrite based on same mac + vid */
           if (binNum == -1) {
               if (temp == 0) {
                   binNum = 0;
               } else {
                   /* both valid, pick non-static one */
                   if (temp_static == 0) {
                       binNum = 0;
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
                (unit, ARLA_ENTRYr, (uint32 *)&entry0, ARL_VALIDf, &temp);
            if (temp) {

                if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
                    binNum = 0;
                    existed = 1;
                } else {
                    /* this entry if valid, check to see if this is the MAC */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRYr, (uint32 *)&entry0, ARL_MACADDRf,
                        (uint32 *)&temp_mac_field);
                    SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                    (DRV_SERVICES(unit)->reg_read)
                        (unit, (DRV_SERVICES(unit)->reg_addr)
                        (unit, ARLA_VID_ENTRYr, 0, 0), &vid1, 2);
                    
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
                /* not a valid entry */
                
                if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
                    rv = SOC_E_NOT_FOUND;
                    goto mem_search_exit;
                } else {
                    binNum = 0;
                }
            }

            /* if no entry found, fail */
            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)) {
                if (binNum == -1) {
                    rv = SOC_E_FULL;
                    goto mem_search_exit;
                }
            }
        }

        input = &entry0;
        vid_rw = vid1;
        *index = 0;

        /* Only need write the selected Entry Register */
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_ENTRYr, (uint32 *)input, 
                ARL_MACADDRf, (uint32 *)&temp_mac_field);
        SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
        if (temp_mac_addr[0] & 0x01) { /* The input is the mcast address */
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&temp_mac_field);
            value = 0;
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_PIDf, &temp);
            value = temp;
            if (SOC_IS_ROBO5347(unit)) {
                value += 24;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_ENTRY_RSRV0f, &temp);
            value |= (temp << 6);
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_PRIf, &temp);
            value |= (temp << 8);
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_AGEf, &temp);
            value |= (temp << 9);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &value);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw);
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_CONf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
            temp = 1;
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp);
        } else { /* The input is the unicast address */
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&temp_mac_field);
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_PIDf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_PRIf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_AGEf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw);
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_STATICf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRYr, (uint32 *)input, ARL_CONf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
            temp = 1;
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp);
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

int 
_drv_bcm5348_mem_arl_entry_delete(int unit, uint8 *mac_addr, uint32 vid)
{
    uint32 reg_addr, reg_len, reg_val32, count, temp;
    uint64 reg_val64, mac_field;
    int rv = SOC_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);

    /* write MAC Addr */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_MACr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_MACr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }

    SAL_MAC_ADDR_TO_UINT64(mac_addr, mac_field);
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_MACr, (uint32 *)&reg_val64, 
        MAC_ADDR_INDXf, (uint32 *)&mac_field));

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }

    /* write VID */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_VIDr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_VIDr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_val32, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_VIDr, &reg_val32, ARLA_VIDTAB_INDXf, &vid));

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_val32, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }

    /* Clear entire entry */
    COMPILER_64_ZERO(reg_val64);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_ENTRYr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_ENTRYr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }

    /* Clear VID entry */
    reg_val32 = 0;
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_VID_ENTRYr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_VID_ENTRYr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_val32, reg_len)) < 0) {
        goto marl_entry_delete_exit;
    }

    MEM_RWCTRL_REG_LOCK(soc);
    /* Write ARL Read/Write Control Register */
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &reg_val32, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto marl_entry_delete_exit;
    }
    temp = MEM_TABLE_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, TAB_RWf, &temp);
    temp = ARL_TABLE_ACCESS;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, TAB_INDEXf, &temp);
    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &reg_val32, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto marl_entry_delete_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &reg_val32, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto marl_entry_delete_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_RWCTLr, &reg_val32, ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto marl_entry_delete_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);

marl_entry_delete_exit:
    return rv;
}

#define DRV_MEM_DELETE_TABLE_ARL    0x1
#define DRV_MEM_DELETE_TABLE_MARL   0x2
#define DRV_MEM_DELETE_TABLE_ARL_MARL   \
    ( DRV_MEM_DELETE_TABLE_ARL|DRV_MEM_DELETE_TABLE_MARL)

#define DRV_MEM_DELETE_FLAG_SW  0x1
#define DRV_MEM_DELETE_FLAG_HW  0x2
#define DRV_MEM_DELETE_FLAG_ALL 0x3
/*
 *  Function : _drv_bcm5348_mem_MARL_clear
 *
 *  Purpose :
 *      Clear MARL MEM (BOTH HW table & SW table)
 *
 *  Parameters :
 *      unit        :   unit id
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm5348_mem_ARL_clear(int unit, uint32 flag, uint32 table)
{
    int rv = SOC_E_NONE;
    soc_control_t   *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t output;
    uint64 temp_mac_field;
    uint8 temp_mac_addr[6];
    int index_min, index_max, index_count;
    int idx, temp_vid;
    uint32 valid;

    index_min = SOC_MEM_BASE(unit, L2_ARLm);
    index_max = SOC_MEM_BASE(unit, L2_ARLm) + SOC_MEM_SIZE(unit, L2_ARLm);
    index_count = SOC_MEM_SIZE(unit, L2_ARLm);
    if(soc->arl_table != NULL){
        ARL_SW_TABLE_LOCK(soc);
        for (idx = index_min; idx < index_count; idx++) {
            sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
            if(!ARL_ENTRY_NULL(&soc->arl_table[idx])) {
                sal_memcpy(&output, &soc->arl_table[idx], 
                    sizeof(l2_arl_sw_entry_t));
            } else {
                continue;
            }
            (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                    (uint32 *)&output, &valid);
            if (valid){
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&temp_mac_field);
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                    (uint32 *)&output, (uint32 *)&temp_vid);

                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                if (temp_mac_addr[0] & 0x01) { /* mcast address */
                    if ( table & DRV_MEM_DELETE_TABLE_MARL) {
                        if (flag & DRV_MEM_DELETE_FLAG_HW) {
                            _drv_bcm5348_mem_arl_entry_delete(unit, 
                                       temp_mac_addr, temp_vid);
                        }
                        if (flag & DRV_MEM_DELETE_FLAG_SW){
                            soc_robo_arl_callback(unit,
                                (l2_arl_sw_entry_t *)&output, NULL); 
                            /* Remove the entry from sw database */
                            sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
                            sal_memcpy(&soc->arl_table[idx], &output,
                                sizeof(l2_arl_sw_entry_t));
                        }
                    } /* DRV_MEM_DELETE_TABLE_MARL*/                                         
                } else { /* unicast address*/
                    if ( table & DRV_MEM_DELETE_TABLE_ARL) {
                        if (flag & DRV_MEM_DELETE_FLAG_HW) {
                            _drv_bcm5348_mem_arl_entry_delete(unit, 
                                       temp_mac_addr, temp_vid);
                        }
                        if (flag & DRV_MEM_DELETE_FLAG_SW){
                            soc_robo_arl_callback(unit,
                                (l2_arl_sw_entry_t *)&output, NULL); 
                            /* Remove the entry from sw database */
                            sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
                            sal_memcpy(&soc->arl_table[idx], &output,
                                sizeof(l2_arl_sw_entry_t));
                        }
                    } /* DRV_MEM_DELETE_TABLE_ARL*/
                }
            }
        }
        ARL_SW_TABLE_UNLOCK(soc);
    } else {
        soc_cm_debug(DK_ERR,"soc arl table not allocated");
        rv = SOC_E_FAIL;
    }
    return rv;
}    

int 
_drv_bcm5348_mem_delete_all(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_len, reg_value;
    uint32 port, temp, count = 0;
    uint32 mst_con, flag, table;

    if (mem == DRV_MEM_MARL) {
        flag = DRV_MEM_DELETE_FLAG_ALL;
        table = DRV_MEM_DELETE_TABLE_MARL;
        rv = _drv_bcm5348_mem_ARL_clear(unit, flag, table);
    } else {
        /* Use Fast age to clear ARL HW table */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MST_CONr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MST_CONr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &mst_con, reg_len)) < 0) {
            goto mem_delete_exit;
        }
    
        /* aging port mode */
        /* 
         *   GNATS :  11132
         * There are some unicast entries for BCM5347 with port id less than 24.
         * Using port ageout mechanism from port 0 to port 52 to make sure
         * all unicast entries had been aged out.
         */
        for (port = 0; port < 53; port++) {
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, AGEOUT_CTLr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, AGEOUT_CTLr, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, AGEOUT_CTLr, &reg_value, AGE_EN_PORTf, &port));
    
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
            /* start fast aging process */
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MST_CONr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MST_CONr, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
    
            temp = 1;
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
                AGE_MODE_PRTf, &temp));
        
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
                EN_AGE_STATICf, &temp));
        
            SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
                FAST_AGE_STDNf, &temp));
    
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
                        (unit, MST_CONr, &reg_value, 
                        FAST_AGE_STDNf, &temp));
                if (!temp) {
                    break;
                }
            }
    
            if (count >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto mem_delete_exit;
            }
        }

        /* clear ARL SW table */
        flag = DRV_MEM_DELETE_FLAG_SW;
        table = DRV_MEM_DELETE_TABLE_ARL;
        rv = _drv_bcm5348_mem_ARL_clear(unit, flag, table);

mem_delete_exit:
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MST_CONr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MST_CONr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &mst_con, reg_len)) < 0) {
            goto mem_delete_exit;
        }
    }
    return rv;
}

int
_drv_bcm5348_cfp_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    drv_cfp_entry_t cfp_entry;
    int entry_len, rv = SOC_E_NONE;
    uint8 *data_ptr;
    uint32 mem_id, counter;
    uint32 i, index_min, index_max;
    soc_mem_info_t *meminfo;

    data_ptr = (uint8 *)entry;
    /* Get the length of entry */
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
            mem_id = CFP_TCAM_S0m;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_ACT:
            mem_id = CFP_ACT_POLm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_METER:
            mem_id = CFP_METERm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            mem_id = CFP_STAT_IBm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /* check index */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);

    for (i = 0; i < count; i++) {
        if (((entry_id + i) < index_min) || 
            ((entry_id + i) > index_max)) {
            return SOC_E_PARAM;
        }

        switch (mem) {
            case DRV_MEM_TCAM_DATA:
            case DRV_MEM_TCAM_MASK:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_read)(unit, (entry_id + i), 
                    DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                if (mem == DRV_MEM_TCAM_DATA) {
                    sal_memcpy(data_ptr, cfp_entry.tcam_data, entry_len);
                } else {
                    sal_memcpy(data_ptr, cfp_entry.tcam_mask, entry_len);
                }
                break;
            case DRV_MEM_CFP_ACT:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_read)(unit, (entry_id + i), 
                    DRV_CFP_RAM_ACT, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                sal_memcpy(data_ptr, cfp_entry.act_data, entry_len);
                break;
            case DRV_MEM_CFP_METER:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_read)(unit, (entry_id + i), 
                    DRV_CFP_RAM_METER, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                sal_memcpy(data_ptr, cfp_entry.meter_data, entry_len);
                break;
            case DRV_MEM_CFP_STAT_IB:
                sal_memset(&counter, 0, sizeof(uint32));
                if ((rv = (DRV_SERVICES(unit)->cfp_stat_get)
                    (unit, DRV_CFP_STAT_INBAND, 
                    (entry_id + i), &counter)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                sal_memcpy(data_ptr, &counter, entry_len);
                break;
            case DRV_MEM_CFP_STAT_OB:
                sal_memset(&counter, 0, sizeof(uint32));
                if ((rv = (DRV_SERVICES(unit)->cfp_stat_get)
                    (unit, DRV_CFP_STAT_OUTBAND, 
                    (entry_id + i), &counter)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                sal_memcpy(data_ptr, &counter, entry_len);
                break;
            default:
                return SOC_E_PARAM;
        }
        data_ptr = data_ptr + entry_len;
    }

    return rv;
}


int
_drv_bcm5348_cfp_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    drv_cfp_entry_t cfp_entry;
    int entry_len, rv = SOC_E_NONE;
    uint8 *data_ptr;
    uint32 mem_id, counter;
    uint32 i, index_min, index_max;
    soc_mem_info_t *meminfo;

    data_ptr = (uint8 *)entry;
    /* Get the length of entry */
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
            mem_id = CFP_TCAM_S0m;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_ACT:
            mem_id = CFP_ACT_POLm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_METER:
            mem_id = CFP_METERm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            mem_id = CFP_STAT_IBm;
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /* check index */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);

    for (i = 0; i < count; i++) {
        if (((entry_id + i) < index_min) || 
            ((entry_id + i) > index_max)) {
            return SOC_E_PARAM;
        }

        switch (mem) {
            case DRV_MEM_TCAM_DATA:
            case DRV_MEM_TCAM_MASK:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_read)(unit, (entry_id + i), 
                    DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                if (mem == DRV_MEM_TCAM_DATA) {
                    sal_memcpy(cfp_entry.tcam_data, data_ptr, entry_len);
                } else {
                    sal_memcpy(cfp_entry.tcam_mask, data_ptr, entry_len);
                }
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_write)(unit, (entry_id + i), 
                    DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_write(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            case DRV_MEM_CFP_ACT:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                sal_memcpy(cfp_entry.act_data, data_ptr, entry_len);
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_write)(unit, (entry_id + i), 
                    DRV_CFP_RAM_ACT, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_write(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            case DRV_MEM_CFP_METER:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                sal_memcpy(cfp_entry.meter_data, data_ptr, entry_len);
                if ((rv = (DRV_SERVICES(unit)->cfp_entry_write)(unit, (entry_id + i), 
                    DRV_CFP_RAM_METER, &cfp_entry)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_write(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            case DRV_MEM_CFP_STAT_IB:
                sal_memset(&counter, 0, sizeof(uint32));
                sal_memcpy(&counter, data_ptr, entry_len);
                if ((rv = (DRV_SERVICES(unit)->cfp_stat_set)
                    (unit, DRV_CFP_STAT_INBAND, 
                    (entry_id + i), counter)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            case DRV_MEM_CFP_STAT_OB:
                sal_memset(&counter, 0, sizeof(uint32));
                sal_memcpy(&counter, data_ptr, entry_len);
                if ((rv = (DRV_SERVICES(unit)->cfp_stat_set)
                    (unit, DRV_CFP_STAT_OUTBAND, 
                    (entry_id + i), counter)) < 0){
                    soc_cm_debug(DK_ERR, 
                    "_drv_bcm5348_cfp_read(mem=0x%x,entry_id=0x%x)\n",
                    mem, entry_id + i);
                    return rv;
                }
                break;
            default:
                return SOC_E_PARAM;
        }
        data_ptr = data_ptr + entry_len;
    }

    return rv;
}

int
_drv_bcm5348_cfp_field_get(int unit, uint32 mem, uint32 field_index, 
        uint32 *entry, uint32 *fld_data)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t cfp_entry;
    int entry_len;
    soc_mem_info_t *meminfo;
    
    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
            meminfo = &SOC_MEM_INFO(unit, CFP_TCAM_S0m);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_TCAM, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm5348_cfp_field_get(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            break;
        case DRV_MEM_TCAM_MASK:
            meminfo = &SOC_MEM_INFO(unit, CFP_TCAM_MASKm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_mask, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_TCAM_MASK, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm5348_cfp_field_get(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            break;
        case DRV_MEM_CFP_ACT:
            meminfo = &SOC_MEM_INFO(unit, CFP_ACT_POLm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.act_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_ACT, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm5348_cfp_field_get(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            break;
        case DRV_MEM_CFP_METER:
            meminfo = &SOC_MEM_INFO(unit, CFP_METERm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.meter_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_METER, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm5348_cfp_field_get(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }

    return rv;
}


int
_drv_bcm5348_cfp_field_set(int unit, uint32 mem, uint32 field_index, 
        uint32 *entry, uint32 *fld_data)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t cfp_entry;
    int entry_len;
    soc_mem_info_t *meminfo;

    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
            meminfo = &SOC_MEM_INFO(unit, CFP_TCAM_S0m);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_set)
                (unit, DRV_CFP_RAM_TCAM, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm5348_cfp_field_set(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            sal_memcpy(entry, cfp_entry.tcam_data, entry_len);
            break;
        case DRV_MEM_TCAM_MASK:
            meminfo = &SOC_MEM_INFO(unit, CFP_TCAM_MASKm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_mask, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_set)
                (unit, DRV_CFP_RAM_TCAM_MASK, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm5348_cfp_field_set(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            sal_memcpy(entry, cfp_entry.tcam_mask, entry_len);
            break;
        case DRV_MEM_CFP_ACT:
            meminfo = &SOC_MEM_INFO(unit, CFP_ACT_POLm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.act_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_set)
                (unit, DRV_CFP_RAM_ACT, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm5348_cfp_field_set(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            sal_memcpy(entry, cfp_entry.act_data, entry_len);
            break;
        case DRV_MEM_CFP_METER:
            meminfo = &SOC_MEM_INFO(unit, CFP_METERm);
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.meter_data, entry, entry_len);
            if ((rv = (DRV_SERVICES(unit)->cfp_field_set)
                (unit, DRV_CFP_RAM_METER, field_index, &cfp_entry, fld_data)) < 0){
                soc_cm_debug(DK_ERR, 
                "_drv_bcm5348_cfp_field_set(mem=0x%x,field=0x%x)\n",
                mem, field_index);
                return rv;
            }
            sal_memcpy(entry, cfp_entry.meter_data, entry_len);
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }

    return rv;
}


 /*
 *  Function : drv_bcm5348_mem_read
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
drv_bcm5348_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32 other_table_idx = 0;
    uint32  temp;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 reg_addr;
    int reg_len;
    uint8 *mem_ptr;
    uint64 zero_value;
    soc_control_t           *soc = SOC_CONTROL(unit);
#ifdef BE_HOST
    uint32 val32;
#endif

    soc_cm_debug(DK_MEM, 
        "drv_mem_read(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
            return SOC_E_UNAVAIL;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            break;
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            rv = _drv_bcm5348_cfp_read(
                unit, mem, entry_id, count, entry);
            return rv;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    /* process read action */
    MEM_LOCK(unit, GEN_MEMORYm);

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
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
            sal_memcpy(gmem_entry, cache + (entry_id + i) * entry_size, 
                entry_size);
            continue;
        }

        /* Clear memory data access registers */
        COMPILER_64_ZERO(zero_value);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA0r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA0r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_read_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA1r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA1r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_read_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA2r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA2r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_read_exit;
        }

        /* Set memory index */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_INDEXr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_INDEXr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &other_table_idx, reg_len)) < 0) {
            goto mem_read_exit;
        }
        temp = entry_id + i;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, OTHER_TABLE_INDEXr, &other_table_idx, TABLE_INDEXf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &other_table_idx, reg_len)) < 0) {
            goto mem_read_exit;
        }

        MEM_RWCTRL_REG_LOCK(soc);
        /* Read memory control register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_RWCTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_RWCTLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_read_exit;
        }
        
        temp = MEM_TABLE_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, TAB_RWf, &temp);

        /* Decide which table to be read */
        switch(mem_id) {
            case VLAN_1Qm:
                temp = VLAN_TABLE_ACCESS;
                break;
            case MSPT_TABm:
                temp = MSPT_TABLE_ACCESS;
                break;
            case MARL_PBMPm:
                temp = MARL_PBMP_TABLE_ACCESS;
                break;
            default:
                rv = SOC_E_UNAVAIL;
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_read_exit;
        }
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, TAB_INDEXf, &temp);

        /* Start Read Process */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, ARL_STRTDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_read_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_RWCTLr, &acc_ctrl, ARL_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_read_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);

        /* Read the current generic memory entry */
        mem_ptr = (uint8 *)gmem_entry;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA0r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA0r);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, mem_ptr, reg_len)) < 0) {
            goto mem_read_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA1r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA1r);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, mem_ptr+8, reg_len)) < 0) {
            goto mem_read_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA2r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA2r);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, mem_ptr+16, reg_len)) < 0) {
            goto mem_read_exit;
        }
#ifdef BE_HOST
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == VLAN_1Qm || mem_id == MARL_PBMPm){
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }

        for (i = 0; i < 3; i++) {
           val32 = entry[2*i];
           entry[2*i] = entry[2*i+1];
           entry[2*i+1] = val32;
        }
    }
#endif
        gmem_entry++;
    }

 mem_read_exit:
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;

}

 /*
 *  Function : drv_bcm5348_mem_write
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
drv_bcm5348_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32  temp;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 reg_addr;
    int reg_len;
    uint32 other_table_idx = 0;
    uint8 *mem_ptr;
    uint64 zero_value;
    soc_control_t           *soc = SOC_CONTROL(unit);
#ifdef BE_HOST
    uint32              val32;
#endif

    soc_cm_debug(DK_MEM, "drv_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
            return SOC_E_UNAVAIL;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            break;
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            rv = _drv_bcm5348_cfp_write(
                unit, mem, entry_id, count, entry);
            return rv;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }


    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);
    
    /* process write action */
    MEM_LOCK(unit, GEN_MEMORYm);

#ifdef BE_HOST
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == VLAN_1Qm || mem_id == MARL_PBMPm){
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }

        for (i = 0; i < 3; i++) {
           val32 = entry[2*i];
           entry[2*i] = entry[2*i+1];
           entry[2*i+1] = val32;
        }
    }
#endif

    if (count < 1) {
        return SOC_E_PARAM;
    }

    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, GEN_MEMORYm);
            return SOC_E_PARAM;
        }

        /* Clear memory data access registers */
        COMPILER_64_ZERO(zero_value);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA0r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA0r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_write_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA1r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA1r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_write_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA2r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA2r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&zero_value, reg_len)) < 0) {
            goto mem_write_exit;
        }

        /* write data */
        mem_ptr = (uint8 *)gmem_entry;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA0r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA0r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, mem_ptr, reg_len)) < 0) {
            goto mem_write_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA1r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA1r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, mem_ptr+8, reg_len)) < 0) {
            goto mem_write_exit;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_DATA2r, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_DATA2r);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, mem_ptr+16, reg_len)) < 0) {
            goto mem_write_exit;
        }

        /* Set memory index */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, OTHER_TABLE_INDEXr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, OTHER_TABLE_INDEXr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &other_table_idx, reg_len)) < 0) {
            goto mem_write_exit;
        }
        temp = entry_id + i;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, OTHER_TABLE_INDEXr, &other_table_idx, TABLE_INDEXf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &other_table_idx, reg_len)) < 0) {
            goto mem_write_exit;
        }

        MEM_RWCTRL_REG_LOCK(soc);
        /* Read memory control register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ARLA_RWCTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ARLA_RWCTLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }
        
        temp = MEM_TABLE_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, TAB_RWf, &temp);

        /* Decide which table to be read */
        switch(mem_id) {
            case VLAN_1Qm:
                temp = VLAN_TABLE_ACCESS;
                break;
            case MSPT_TABm:
                temp = MSPT_TABLE_ACCESS;
                break;
            case MARL_PBMPm:
                temp = MARL_PBMP_TABLE_ACCESS;
                break;
            default:
                rv = SOC_E_UNAVAIL;
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_write_exit;
        }
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, TAB_INDEXf, &temp);

        /* Start Read Process */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_RWCTLr, &acc_ctrl, ARL_STRTDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &acc_ctrl, reg_len)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_write_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_RWCTLr, &acc_ctrl, ARL_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);
        
        /* Write back to cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL) {
            sal_memcpy(cache + (entry_id + i) * entry_size, 
                gmem_entry, entry_size);
            CACHE_VMAP_SET(vmap, (entry_id + i));
        }
        gmem_entry++;
    }

 mem_write_exit:
    MEM_UNLOCK(unit, GEN_MEMORYm);
    return rv;
}

/*
 *  Function : drv_bcm5348_mem_field_get
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
drv_bcm5348_mem_field_get(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int         mem_id = 0, field_id;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32              val32;
#endif

    soc_cm_debug(DK_MEM, "drv_mem_field_get(mem=0x%x,field_index=0x%x)\n",
         mem, field_index);
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            mem_id = L2_ARL_SWm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(MARL_PBMP_IDXf);
                mem_id = L2_MARL_SWm;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRIORITY_Rf);
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = INDEX(STATICf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = INDEX(CONTROLf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
                field_id = INDEX(MSPT_IDf);
            }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = INDEX(UNTAG_MAPf);
            }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = INDEX(FORWARD_MAPf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(fld_data, entry, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = PBMP_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
            return _drv_bcm5348_cfp_field_get
                (unit, mem, field_index, entry, fld_data);
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
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
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else {
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }
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
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else {
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    if ((SOC_IS_ROBO5347(unit)) && (field_id == PORTID_Rf)) {
        *fld_data -= 24;
    }
    return SOC_E_NONE;
}

 /*
 *  Function : drv_bcm5348_mem_field_set
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
drv_bcm5348_mem_field_set(int unit, uint32 mem, 
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

    soc_cm_debug(DK_MEM, "drv_mem_field_set(mem=0x%x,field_index=0x%x)0x%x\n",
         mem, field_index,*fld_data);
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            mem_id = L2_ARL_SWm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(MARL_PBMP_IDXf);
                mem_id = L2_MARL_SWm;
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRIORITY_Rf);
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = INDEX(STATICf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = INDEX(CONTROLf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
                field_id = INDEX(MSPT_IDf);
            }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = INDEX(UNTAG_MAPf);
            }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = INDEX(FORWARD_MAPf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(entry, fld_data, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = PBMP_Rf;
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
            return _drv_bcm5348_cfp_field_set
                (unit, mem, field_index, entry, fld_data);
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
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
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else {
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    
    /* Port number should add 24 for BCM5347 */
    if ((SOC_IS_ROBO5347(unit)) && (field_id == PORTID_Rf)) {
        *fld_data += 24;
    }
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
    if (mem_id == L2_ARLm ||mem_id == L2_MARL_SWm || mem_id == L2_ARL_SWm) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else {
        for (i = 0; i < 3; i++) {
           val32 = entry[i];
           entry[i] = entry[5-i];
           entry[5-i] = val32;
        }
    }
    if (fieldinfo->len > 32) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    if ((SOC_IS_ROBO5347(unit)) && (field_id == PORTID_Rf)) {
        *fld_data -= 24;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm5348_mem_clear
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
drv_bcm5348_mem_clear(int unit, uint32 mem)
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
            /* Clear Dynamic ARL entries */
            ((DRV_SERVICES(unit)->mem_delete)
                (unit, DRV_MEM_ARL, NULL, DRV_MEM_OP_DELETE_ALL_ARL));
            return SOC_E_NONE;
            break;
        case DRV_MEM_MARL:
            rv = ((DRV_SERVICES(unit)->mem_delete)
                (unit, DRV_MEM_MARL, NULL, DRV_MEM_OP_DELETE_ALL_ARL));
            return SOC_E_NONE;
            break;
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            break;
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
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
 *  Function : drv_bcm5348_mem_search
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
drv_bcm5348_mem_search(int unit, uint32 mem, 
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

    rv = _drv_bcm5348_mem_search(unit, mem, key, entry, 
        entry1, flags, &index);

    return rv;
}

/*
 *  Function : drv_bcm5348_mem_insert
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
 *
 */
int 
drv_bcm5348_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int                 rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t       output;
    uint32          temp, count, mem_id;
    uint64          entry_reg;
    uint8           mac_addr[6];
    uint64          mac_field, mac_field_output, mac_field_entry;
    uint32          vid, control, vid_output, vid_entry;
    int             index;
    uint32 value;
    soc_control_t           *soc = SOC_CONTROL(unit);

    soc_cm_debug(DK_MEM, "drv_mem_insert : mem=0x%x, flags = 0x%x)\n",
         mem, flags);
    switch(mem) {
        case DRV_MEM_ARL:
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
    if ((rv =  _drv_bcm5348_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            NULL, flags, &index))< 0) {
        if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
            goto mem_insert_exit;              
        }
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
    /* write entry */
    
    /* form entry */
    /* VLAN ID */
    if ((rv = (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid)) < 0) {
        goto mem_insert_exit;
    }

    /* MAC Address */
    if ((rv = (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            entry, (uint32 *)&mac_field)) < 0) {
        goto mem_insert_exit;
    }
    SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field);
    
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, 
        ARL_MACADDRf, (uint32 *)&mac_field);

    if (mac_addr[0] & 0x01) { /* The input is the mcast address */
        /* multicast group index */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &temp);
        value = temp & 0x3f;
        if (SOC_IS_ROBO5347(unit)) {
            value -= 24;
        }
        (DRV_SERVICES(unit)->reg_field_set)
           (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_PIDf, &value);
        value = (temp >> 6) & 0x07;
        (DRV_SERVICES(unit)->reg_field_set)
           (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_ENTRY_RSRV0f, &value);
        value = (temp >> 9) & 0x03;
        (DRV_SERVICES(unit)->reg_field_set)
           (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_PRIf, &value);
        value = (temp >> 11) & 0x01;
        (DRV_SERVICES(unit)->reg_field_set)
           (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_AGEf, &value);
        /* arl_control */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_CONf, &temp);
        /* static :
         *  - ROBO chip arl_control_mode at none-zero value cab't work 
         *    without static setting.
         */
        if (temp == 0 ) {
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
        } else {
            temp = 1;
        }
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_STATICf, &temp);

        /* valid bit */
        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_VALIDf, &temp);
    } else { /* unicast address */
        /* source port id */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_PIDf, &temp);

        /* priority */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, entry, &temp);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_PRIf, &temp);

        /* age */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_AGEf, &temp);

        /* arl_control */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_CONf, &temp);

        /* static :
         *  - ROBO chip arl_control_mode at none-zero value cab't work 
         *    without static setting.
         */
        if (temp == 0 ) {
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
        } else {
            temp = 1;
        }
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_STATICf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, ARL_VALIDf, &temp);
    }


    /* write ARL and VID entry register*/
    if (index == 0){ /* entry 0 */
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_VID_ENTRYr, 0, 0), &vid, 2)) < 0) {
            goto mem_insert_exit;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_ENTRYr, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
            goto mem_insert_exit;
        }
    }

    MEM_RWCTRL_REG_LOCK(soc);
    /* Write ARL Read/Write Control Register */
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_insert_exit;
    }
    temp = MEM_TABLE_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &control, TAB_RWf, &temp);
    temp = ARL_TABLE_ACCESS;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &control, TAB_INDEXf, &temp);
    temp = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, (DRV_SERVICES(unit)->reg_addr)
        (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_insert_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_RWCTLr, 0, 0), &control, 1)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_insert_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, ARLA_RWCTLr, &control, ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_insert_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);

    
    /* Return SOC_E_NONE instead of SOC_E_EXISTS to fit DV test. */
    if (rv == SOC_E_EXISTS) {
        rv = SOC_E_NONE;
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
 *  Function : drv_bcm5348_mem_delete
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
drv_bcm5348_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t output;
    uint32 temp, count, mem_id;
    int index;
    uint32  reg_len, reg_addr, reg_value;
    uint32 ag_port_mode = 0, ag_vlan_mode = 0, ag_spt_mode = 0;
    uint32  ag_static_mode = 0;
    uint32  mst_con, age_out_ctl;
    uint64 temp_mac_field;
    uint8  temp_mac_addr[6];
    uint32 temp_vid;
    uint64 entry_reg;
    uint32      src_port= 0, vlanid = 0;

    soc_cm_debug(DK_MEM, "drv_mem_delete : mem=0x%x, flags = 0x%x)\n",
         mem, flags);
    switch(mem) {
        case DRV_MEM_ARL:
            mem_id = L2_ARL_SWm;
            break;
        case DRV_MEM_MARL:
            mem_id = L2_ARL_SWm;
            /* MARL entries should do normal deletion, not fast aging. */
            flags &= ~DRV_MEM_OP_DELETE_BY_PORT;
            flags &= ~DRV_MEM_OP_DELETE_BY_VLANID;
            flags &= ~DRV_MEM_OP_DELETE_BY_SPT;
            flags &= ~DRV_MEM_OP_DELETE_BY_STATIC;
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
        ag_spt_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_STATIC) {
        ag_static_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
        rv = _drv_bcm5348_mem_delete_all(unit, mem, entry, flags);
        
        return rv;
    }
    
    if ((ag_port_mode) || (ag_vlan_mode)) {
        /* 
         * aging port and vlan mode 
         */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, AGEOUT_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, AGEOUT_CTLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }

        /*
         * keep original age port id and vid
         */
        age_out_ctl = reg_value;

        /* aging port mode */
        if (ag_port_mode) {
            temp = 0;
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
            src_port = temp;

            if (SOC_IS_ROBO5347(unit)) {
                temp = temp + 24;
            }
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, AGEOUT_CTLr, &reg_value, AGE_EN_PORTf, &temp));
        }

        /* aging vlan mode */
        if (ag_vlan_mode) {
            temp = 0;
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &temp);
            vlanid = temp;

            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, AGEOUT_CTLr, &reg_value, AGE_EN_VIDf, &temp));
        } 

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }

        /* start fast aging process */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MST_CONr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MST_CONr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto mem_delete_exit;
        }
        mst_con = reg_value;
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
            AGE_MODE_PRTf, &ag_port_mode));
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
            AGE_MODE_VLANf, &ag_vlan_mode));
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
            EN_AGE_STATICf, &ag_static_mode));
        temp = 1;
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
            FAST_AGE_STDNf, &temp));
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
                    (unit, MST_CONr, &reg_value, 
                    FAST_AGE_STDNf, &temp));
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
        /* get static status for normal deletion */
        if (flags & DRV_MEM_OP_DELETE_BY_STATIC) {
            ag_static_mode = 1;
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &ag_static_mode));
        }

        soc_cm_debug(DK_MEM, 
                    "\t Normal ARL DEL with Static=%d\n",
                    ag_static_mode);

        MEM_LOCK(unit,L2_ARLm);
        /* search entry */
        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
        if ((rv =  _drv_bcm5348_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            NULL, flags, &index))< 0) {
            if((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
                if (rv == SOC_E_FULL) {
                /* For mem_delete if mem_search return SOC_E_FULL it means
                  * the entry is not found. */
                    rv = SOC_E_NOT_FOUND;
                }
                goto mem_delete_exit;              
            }
        }

        /* clear entry */
        if (rv == SOC_E_EXISTS) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_ENTRYr, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
                goto mem_delete_exit;
            }
            /* check static bit if set */
            if (!ag_static_mode){
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                                (unit, ARLA_ENTRYr, (uint32 *)&entry_reg, 
                                ARL_STATICf, &temp));
                if (temp) {
                    soc_cm_debug(DK_MEM, 
                                "\t Entry exist with static=%d\n",
                                temp);
                    rv = SOC_E_NOT_FOUND;
                    goto mem_delete_exit;
                }
            }

            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&temp_mac_field)) < 0) {
                goto mem_delete_exit;
            }
                
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

            /* Write VID Table Index Register */
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                entry, &temp_vid)) < 0) {
                goto mem_delete_exit;
            }

            rv = _drv_bcm5348_mem_arl_entry_delete(unit, 
                                       temp_mac_addr, temp_vid);

            if (rv == SOC_E_NONE) {
                sw_arl_update = 1;
            }
        }
    }

mem_delete_exit:
    /* 
     * Restore MST Settings
     */
    if ((ag_port_mode) || (ag_vlan_mode)) {
        /* MST configuration */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MST_CONr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MST_CONr, 0, 0);
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &mst_con, reg_len);

        /* Age Ports and VIDs */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, AGEOUT_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, AGEOUT_CTLr, 0, 0);
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &age_out_ctl, reg_len);
    }else {
        MEM_UNLOCK(unit,L2_ARLm);
    }

    if (sw_arl_update){
        /* Remove the entry from sw database */
        _drv_arl_database_delete(unit, index, &output);
    }

    return rv;
}

/*
 *  Function : drv_bcm5348_mem_cache_get
 *
 *  Purpose :
 *    Get the status of caching is enabled for a specified memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory type.
 *      enable     :   status of cacheing enable or not.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for VLAN and SEC_MAC memory now.
 *
 */

int
drv_bcm5348_mem_cache_get(int unit,
                  uint32 mem, uint32 *enable)
{
    int mem_id;
    assert(SOC_UNIT_VALID(unit));

    switch (mem) {
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            break;
        default:
            *enable = FALSE;
            return SOC_E_NONE;
    }
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    /* 
    For robo-SDK, we defined one block for each memory types.
    */
    *enable = (SOC_MEM_STATE(unit, mem_id).cache[0] != NULL)? 1:0; 

    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm5348_mem_cache_set
 *
 *  Purpose :
 *    Set the status of caching is enabled for a specified memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory type.
 *      enable     :   status of cacheing enable or not.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for VLAN and SEC_MAC memory now.
 *
 */

int
drv_bcm5348_mem_cache_set(int unit,
                  uint32 mem, uint32 enable)
{
    soc_memstate_t *memState;
    int index_cnt;
    int cache_size, vmap_size;
    uint8 *vmap;
    uint32 *cache;
    int mem_id;
    int     count, i;
    int entry_size;

    assert(SOC_UNIT_VALID(unit));
    switch (mem) {
        case DRV_MEM_VLAN:
            mem_id = VLAN_1Qm;
            count = 1;
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    for (i = 0; i < count; i++) {
        assert(SOC_MEM_IS_VALID(unit, mem_id));
        entry_size = soc_mem_entry_bytes(unit, mem_id);
        memState = &SOC_MEM_STATE(unit, mem_id);
        index_cnt = soc_robo_mem_index_max(unit, mem_id) + 1;
        cache_size = index_cnt * entry_size;
        vmap_size = (index_cnt + 7) / 8;

        soc_cm_debug(DK_SOCMEM,
                     "drv_mem_cache_set: unit %d memory %s %sable\n",
                     unit, SOC_ROBO_MEM_UFNAME(unit, mem_id),
                     enable ? "en" : "dis");

        /* 
        For robo-SDK, we defined one block for each memory types.
        */
        /*
         * Turn off caching if currently enabled.
         */

        cache = memState->cache[0];
        vmap = memState->vmap[0];

        /* Zero before free to prevent potential race condition */

        if (cache != NULL) {
            memState->cache[0] = NULL;
            memState->vmap[0] = NULL;

            sal_free(cache);
            sal_free(vmap);
        }

        if (!enable) {
            return SOC_E_NONE;
        }

        MEM_LOCK(unit, mem_id);

        /* Allocate new cache */

        if ((cache = sal_alloc(cache_size, "table-cache")) == NULL) {
            MEM_UNLOCK(unit, mem_id);
            return SOC_E_MEMORY;
        }

        if ((vmap = sal_alloc(vmap_size, "table-vmap")) == NULL) {
            sal_free(cache);
            MEM_UNLOCK(unit, mem_id);
            return SOC_E_MEMORY;
        }

        sal_memset(vmap, 0, vmap_size);

        /* Set memState->cache last to avoid race condition */

        soc_cm_debug(DK_SOCMEM,
                     "drv_mem_cache_set: cache=%p size=%d vmap=%p\n",
                     (void *)cache, cache_size, (void *)vmap);

        memState->vmap[0] = vmap;
        memState->cache[0] = cache;

        MEM_UNLOCK(unit, mem_id);

        mem_id++;
    }

    return SOC_E_NONE;
}
