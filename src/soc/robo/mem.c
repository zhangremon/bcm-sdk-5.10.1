/*
 * $Id: mem.c 1.51.2.2 Broadcom SDK $
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


#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

/*
 * Empty (null) table entries
 */
uint32  _soc_robo_mem_entry_null_zeroes[SOC_ROBO_MAX_MEM_WORDS];


/*
 * Function:
 *  _soc_mem_cmp_XXX
 * Purpose:
 *  Compare two memory entries
 * Parameters:
 *  unit, entry A and entry B
 * Returns:
 *  Negative for A < B, zero for A == B, positive for A > B
 * Notes:
 *  Generic compare functions for table entries.
 *  Additional chip-specific compare functions are defined in
 *  the mcm files.
 */

int
_soc_mem_cmp_gm(int unit, void *ent_a, void *ent_b)
{
    return 0;
}

/*
 *  Function : _drv_arl_read
 *
 *  Purpose :
 *      Get the ARL entry
 *
 *  Parameters :
 *      unit        :   unit id
 *      arl_entry0   :   entry0 data pointer.
 *      arl_entry1   :   entry1 data pointer 
 *      vlan_entry0     :   data pointer of vlan id for entry0.
 *      vlan_entry1     :   data pointer of vlan id for entry1.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
_drv_arl_read(int unit, uint32 *arl_entry0, uint32 *arl_entry1, 
                                uint32 *vlan_entry0, uint32 *vlan_entry1)
{
    return SOC_E_UNAVAIL;
}

/*
 *  Function : _drv_arl_hash
 *
 *  Purpose :
 *      Get the hash result value.
 *
 *  Parameters :
 *      hash_value   :   48 bits hash value.
 *      length :            number of bytes of the value.
 *      hash_result     :   hash result.
 *
 *  Return :
 *      None
 *
 *  Note :
 *
 */
void 
_drv_arl_hash(uint8 *hash_value, uint8 loop, uint16 *hash_result)
{
    uint16  crc = 0xffff;
    uint16  poly = 0x1021;
    uint8  ch;
    uint16  i, j, v, xor_flag;
    for (i=0; i< ((loop+7)/8); i++) {
        ch = *(hash_value+i);
        v= 0x80;
        for (j=0; j<8; j++) {
            if ( (i*8+j) < loop) {
                if (crc & 0x8000) {
                    xor_flag = 1;
                } else {
                    xor_flag = 0;
                }
                crc = crc << 1;
                if (ch & v) {
                    crc = crc ^ poly;
                }
                if (xor_flag) {
                    crc = crc ^ poly;
                }
                v = v >> 1;
            }
        }
    }
    *hash_result = crc;

}

/* Function : _drv_arl_calc_hash_index
 *  - SW hash algorithm to help SW to retreive the ARL entry index based on
 *      a given sw arl entry(carry the MAC + VID hash key)
 *
 *  Note :
 *  1. TB devices doesn't need this function for the TB chip can report the 
 *      real table index through it dual hash argorithm.
 */
uint32
_drv_arl_calc_hash_index(int unit, int bin_index, void *entry)
{
    uint8   hash_value[8], mac_addr_rw[6];
    uint16  hash_result, left_over = 0, final_hash;
    uint32  vid_rw;
    uint32  table_index = 0;
    uint64  rw_mac_field;
    int     index_count;
    uint8       loop = 0, bin_num = 0;
    uint32  ivl_mode = 0;
    
    if (SOC_IS_TBX(unit)){
        /* ------- TBD --------- */
        
        /* TB chips allowed SW to retrieve the dual hashed table index 
         *  through the given MAC+VID key.
         *
         * In current design for ROBO SDK, there is no request on getting 
         *  hashed index directly. This internal routine for ROBO is left 
         *  as "TBD" status.
         */
         assert(0);
    }
    
    index_count = SOC_MEM_SIZE(unit, L2_ARLm);
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
        (uint32 *)entry, &vid_rw);

    /* Read MAC address bit 12~47 */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
        (uint32 *)entry, (uint32 *)&rw_mac_field);
    SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
    
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->vlan_prop_get)
            (unit, DRV_VLAN_PROP_VLAN_LEARNING_MODE, &ivl_mode));
            
    /* when ROBO device at SVL mode, the VID for hash is "0" */
    if (ivl_mode == FALSE){
        vid_rw = 0;
    }
    
    /* HASH value */
    if (SOC_IS_ROBODINO(unit)) {
        hash_value[0] = mac_addr_rw[5];
        hash_value[1] = mac_addr_rw[4];
        hash_value[2] = mac_addr_rw[3];
        hash_value[3] = mac_addr_rw[2];
        hash_value[4] = mac_addr_rw[1];
        hash_value[5] = mac_addr_rw[0];
        hash_value[6] = vid_rw & 0xff;
        hash_value[7] = (vid_rw >> 8) & 0xf;
        hash_value[0] = _shr_bit_rev8(hash_value[0]);
        hash_value[1] = _shr_bit_rev8(hash_value[1]);
        hash_value[2] = _shr_bit_rev8(hash_value[2]);
        hash_value[3] = _shr_bit_rev8(hash_value[3]);
        hash_value[4] = _shr_bit_rev8(hash_value[4]);
        hash_value[5] = _shr_bit_rev8(hash_value[5]);
        hash_value[6] = _shr_bit_rev8(hash_value[6]);
        hash_value[7] = _shr_bit_rev8(hash_value[7]);
    } else {
        
        hash_value[0] = (vid_rw >> 4) & 0xff;
        hash_value[1] = ((vid_rw & 0xf) << 4) + ((mac_addr_rw[0] & 0xf0) >> 4);
        hash_value[2] = ((mac_addr_rw[0] & 0xf) << 4) + 
            ((mac_addr_rw[1] & 0xf0) >> 4);
        hash_value[3] = ((mac_addr_rw[1] & 0xf) << 4) + 
            ((mac_addr_rw[2] & 0xf0) >> 4);
        hash_value[4] = ((mac_addr_rw[2] & 0xf) << 4) + 
            ((mac_addr_rw[3] & 0xf0) >> 4);
        hash_value[5] = ((mac_addr_rw[3] & 0xf) << 4) + 
            ((mac_addr_rw[4] & 0xf0) >> 4);
        hash_value[6] = ((mac_addr_rw[4] & 0xf) << 4) + 
            ((mac_addr_rw[5] & 0xf0) >> 4);
        hash_value[7] = ((mac_addr_rw[5] & 0xf) << 4);
    }
    
    if (SOC_IS_ROBO5324(unit)){ /* 4K 2bin */
        loop = 48;
        left_over = (((mac_addr_rw[4] & 0xf) << 8) + mac_addr_rw[5]);
        bin_num = 2;
    } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        /* 8K 1bin */
        loop = 47;
        left_over = (((mac_addr_rw[4] & 0x1f) << 8) + mac_addr_rw[5]);
        bin_num = 1;
    } else if (SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit)) { 
        /* 4K 1bin */
        loop = 48;
        left_over = (((mac_addr_rw[4] & 0xf) << 8) + mac_addr_rw[5]);
        bin_num = 1;
    } else if (SOC_IS_ROBODINO(unit)) {
        loop = 60;
        left_over = 0x0;
        bin_num = 2;
    } else if (SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53115(unit) ||
        SOC_IS_ROBO53118(unit) ||SOC_IS_ROBO53125(unit) ||
        SOC_IS_ROBO53128(unit)) {
        /* 1K 4bin */
        loop = 50;
        left_over = (((mac_addr_rw[4] & 0x3) << 8) + mac_addr_rw[5]);
        bin_num = 4;
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        /* 4K 2bin */
        loop = 47;
        left_over = (((mac_addr_rw[4] & 0x1f) << 8) + mac_addr_rw[5]);
        bin_num = 2;
    } else if (SOC_IS_ROBO53101(unit)) {
        /* 512, 4bin */
        loop = 51;
        left_over = (((mac_addr_rw[4] & 0x1) << 8) + mac_addr_rw[5]);
        bin_num = 4;
    }
    _drv_arl_hash(hash_value, loop, &hash_result);
    final_hash = hash_result^left_over;
    table_index = ((final_hash * bin_num) + bin_index) % index_count;
    soc_cm_debug(DK_ARL,
        "_drv_arl_calc_hash_index : table_index = 0x%x\n", table_index);
    return table_index;
}

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
 *  1. The ARL hash bucket size in bcm5324 is 2. There are two bank(or bin) in 
 *      a MAC+VID hashed bucket.
 *  2. In the MAC+VID hash search section, there are two search operations 
 *      will be preformed.
 *    a. Match basis search : report the valid entry with MAC+VID matched.
 *    b. Conflict basis search : report the valid entry in the MAC+VID hashed
 *       bucket.
 *
 */
int 
_drv_mem_search(int unit, uint32 mem, uint32 *key, 
                                    uint32 *entry, uint32 *entry_1, uint32 flags, int *index)
{
    int                 rv = SOC_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);
    uint32          count, temp;
    uint32              control = 0;
    uint8           mac_addr_rw[6], temp_mac_addr[6];
    uint64          rw_mac_field, temp_mac_field;
    uint64          entry0, entry1, *input;
    uint32          vid_rw, vid1 = 0, vid2 = 0;
    int             binNum = -1, existed = 0;
    uint32              reg_addr, reg_value;
    int                     reg_len;
    uint32              process, search_valid;
    uint32              mcast_index, src_port;
    gen_memory_entry_t      gmem_entry;
    uint8               hash_value[6];
    uint16              hash_result;
    
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
            temp = 1;
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                entry, &temp);
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
                ARLA_SRCH_RSLT_VIDr, 0, 0), &vid_rw, 2)) < 0) {
                goto mem_search_valid_get;
            }
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                entry, &vid_rw);
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
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_PRIf, &temp);
                mcast_index += (temp << 5);
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_AGEf, &temp);
                mcast_index += (temp << 7);
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
                (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    entry, &temp); 
            
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
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_SRCH_RSLTr, (uint32 *)&entry0, 
                ARLA_SRCH_RSLT_VLIDf, &temp);
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID,
                entry, &temp); 
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
            unit, mem, *key, 1, (uint32 *)&gmem_entry);
        /* Read VALID bit */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VALID, 
                (uint32 *)&gmem_entry, &temp);
        if (!temp) {
            return rv;
        }
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp);

        /* Read VLAN ID value */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VLANID, 
                (uint32 *)&gmem_entry, &vid_rw);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw);
        
        /* Read MAC address bit 12~47 */
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_MAC, 
                (uint32 *)&gmem_entry, (uint32 *)&rw_mac_field);
        SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
        /* check HASH enabled ? */
        /* check 802.1q enable */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, VLAN_CTRL0r);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, VLAN_CTRL0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)
            (unit, VLAN_CTRL0r, &reg_value, VLAN_ENf, &temp));
        sal_memset(hash_value, 0, sizeof(hash_value));
        if (temp) {
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_get)
                (unit, VLAN_CTRL0r, &reg_value, VLAN_LEARN_MODEf, &temp));
            if (temp == 3) {
                /* hash value = VID + MAC */
                hash_value[0] = (vid_rw >> 4) & 0xff;
                hash_value[1] = ((vid_rw & 0xf) << 4) + (mac_addr_rw[1] & 0xf);
                hash_value[2] = mac_addr_rw[2];
                hash_value[3] = mac_addr_rw[3];
                hash_value[4] = mac_addr_rw[4];
                hash_value[5] = mac_addr_rw[5];
            } else {
                /* hash value = MAC */
                hash_value[0] = ((mac_addr_rw[1] & 0xf) << 4) 
                    + ((mac_addr_rw[2] & 0xf0) >> 4);
                hash_value[1] = ((mac_addr_rw[2] & 0xf) << 4) 
                    + ((mac_addr_rw[3] & 0xf0) >> 4);
                hash_value[2] = ((mac_addr_rw[3] & 0xf) << 4) 
                    + ((mac_addr_rw[4] & 0xf0) >> 4);
                hash_value[3] = ((mac_addr_rw[4] & 0xf) << 4) 
                    + ((mac_addr_rw[5] & 0xf0) >> 4);
                hash_value[4] = ((mac_addr_rw[5] & 0xf) << 4);
            }
        } else {
            /* hash value = MAC value */
            hash_value[0] = ((mac_addr_rw[1] & 0xf) << 4) 
                    + ((mac_addr_rw[2] & 0xf0) >> 4);
            hash_value[1] = ((mac_addr_rw[2] & 0xf) << 4) 
                + ((mac_addr_rw[3] & 0xf0) >> 4);
            hash_value[2] = ((mac_addr_rw[3] & 0xf) << 4) 
                + ((mac_addr_rw[4] & 0xf0) >> 4);
            hash_value[3] = ((mac_addr_rw[4] & 0xf) << 4) 
                + ((mac_addr_rw[5] & 0xf0) >> 4);
            hash_value[4] = ((mac_addr_rw[5] & 0xf) << 4);
        }
        /* Get the hash revalue */
        _drv_arl_hash(hash_value, 48, &hash_result);
        /* Recover the MAC bit 0~11 */
        temp = *key;
        temp = temp >> 1; 
        hash_result = (hash_result ^ temp) & 0xfff;
        
        temp_mac_addr[0] = ((mac_addr_rw[1] & 0xf) << 4) 
            + ((mac_addr_rw[2] & 0xf0) >> 4);
        temp_mac_addr[1] = ((mac_addr_rw[2] & 0xf) << 4) 
            + ((mac_addr_rw[3] & 0xf0) >> 4);
        temp_mac_addr[2] = ((mac_addr_rw[3] & 0xf) << 4) 
            + ((mac_addr_rw[4] & 0xf0) >> 4);
        temp_mac_addr[3] = ((mac_addr_rw[4] & 0xf) << 4) 
            + ((mac_addr_rw[5] & 0xf0) >> 4);
        temp_mac_addr[4] = ((mac_addr_rw[5] & 0xf) << 4) 
            + (hash_result >> 8);
        temp_mac_addr[5] = hash_result & 0xff;
        SAL_MAC_ADDR_TO_UINT64(temp_mac_addr, temp_mac_field);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                entry, (uint32 *)&temp_mac_field);
        if (temp_mac_addr[0] & 0x01) { /* multicast entry */
            /* Multicast index */
            mcast_index = 0;
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&gmem_entry, &temp);
            mcast_index = temp;
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_PRIORITY, 
                    (uint32 *)&gmem_entry, &temp);
            mcast_index += (temp << 5);
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                    (uint32 *)&gmem_entry, &temp);
            mcast_index += (temp << 7);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
                entry, &mcast_index);
            /* ARL_control */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_ARL_CONTROL, 
                    (uint32 *)&gmem_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);

        } else { /* unicast entry */
            /* Source port number */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&gmem_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
            /* Priority queue value */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_PRIORITY, 
                    (uint32 *)&gmem_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, entry, &temp);
            /* Age bit */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                    (uint32 *)&gmem_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
            /* ARL_control */
            (DRV_SERVICES(unit)->mem_field_get)(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_ARL_CONTROL, 
                    (uint32 *)&gmem_entry, &temp);
            (DRV_SERVICES(unit)->mem_field_set)(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
        }
        /* Static bit */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_STATIC, 
                (uint32 *)&gmem_entry, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
        
        return rv;
        return SOC_E_UNAVAIL;

    /* delete by MAC */    
    } else if (flags & DRV_MEM_OP_BY_HASH_BY_MAC) {
        l2_arl_sw_entry_t   *rep_entry;
        int i, valid[ROBO_5324_L2_BUCKET_SIZE];
    
        if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
            for (i = 0; i < ROBO_5324_L2_BUCKET_SIZE; i++){
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

        /* Read Operation sucessfully */
        /* Get the ARL Entry 0/1 Register */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_ENTRY0r, 0, 0), (uint32 *)&entry0, 8)) < 0) {
            goto mem_search_exit;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, ARLA_ENTRY1r, 0, 0), (uint32 *)&entry1, 8)) < 0) {
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
                (unit, ARLA_ENTRY0r, (uint32 *)&entry0, 
                ARL_VALIDf, &temp_valid1);

            /* Check the ARL Entry 0 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY0r, (uint32 *)&entry0, 
                ARL_STATICf, &temp_static1);

            /* Check the ARL Entry 1 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY1r, (uint32 *)&entry1, 
                ARL_VALIDf, &temp_valid2);

            /* Check the ARL Entry 1 Register */
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ARLA_ENTRY1r, (uint32 *)&entry1, 
                ARL_STATICf, &temp_static2);

            if (temp_valid1) {
                /* bin 0 valid, check mac or mac+vid */
                /* get mac_addr */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_ENTRY0r, (uint32 *)&entry0, ARL_MACADDRf,
                    (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                /* get vid */
                (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_VID_ENTRY0r, 0, 0), &vid1, 2);          

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
                /* bin 1 valid */          
                /* get mac_addr */
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_ENTRY1r, (uint32 *)&entry1, ARL_MACADDRf,
                    (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                /* get vid */
                (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_VID_ENTRY1r, 0, 0), &vid2, 2);
                
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
                (unit, ARLA_ENTRY0r, (uint32 *)&entry0, ARL_VALIDf, &temp);
            if (temp) {

                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    valid[0] = TRUE;
                    existed = TRUE;
                } else {
                    /* this entry if valid, check to see if this is the MAC */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY0r, (uint32 *)&entry0, ARL_MACADDRf,
                        (uint32 *)&temp_mac_field);
                    SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                    (DRV_SERVICES(unit)->reg_read)
                        (unit, (DRV_SERVICES(unit)->reg_addr)
                        (unit, ARLA_VID_ENTRY0r, 0, 0), &vid1, 2);
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
                (unit, ARLA_ENTRY0r, (uint32 *)&entry1, ARL_VALIDf, &temp);
            if (temp) {

                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    valid[1] = TRUE;
                    existed = TRUE;
                } else {
                    /* this entry if valid, check to see if this is the MAC */
                    (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY1r, (uint32 *)&entry1, ARL_MACADDRf,
                        (uint32 *)&temp_mac_field);
                    SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                    (DRV_SERVICES(unit)->reg_read)
                        (unit, (DRV_SERVICES(unit)->reg_addr)
                        (unit, ARLA_VID_ENTRY1r, 0, 0), &vid2, 2);
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

            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                /* if no entry found, fail */
                if (binNum == -1) {
                    rv = SOC_E_FULL;
                    goto mem_search_exit;
                }
            }
        }

        for (i = 0; i < ROBO_5324_L2_BUCKET_SIZE; i++){
            if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                if (valid[i] == FALSE){
                    continue;
                }

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
                input = &entry0;
                vid_rw = vid1;
            } else {
                input = &entry1;
                vid_rw = vid2;
            }
            *index = i;

            /* Only need write the selected Entry Register */
            (DRV_SERVICES(unit)->reg_field_get)
                    (unit, ARLA_ENTRY0r, (uint32 *)input, 
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            if (temp_mac_addr[0] & 0x01) { /* The input is the mcast address */
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                        (uint32 *)rep_entry, (uint32 *)&temp_mac_field);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY0r, (uint32 *)input, 
                        ARL_MCAST_IDf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)rep_entry, &vid_rw);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY0r, (uint32 *)input, 
                        ARL_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        (uint32 *)rep_entry, &temp);
                temp = 1;
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                        (uint32 *)rep_entry, &temp);
            } else { /* The input is the unicast address */
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                        (uint32 *)rep_entry, (uint32 *)&temp_mac_field);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY1r, (uint32 *)input, 
                        ARL_PIDf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY1r, (uint32 *)input, 
                        ARL_PRIf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY1r, (uint32 *)input, 
                        ARL_AGEf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)rep_entry, &vid_rw);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY1r, (uint32 *)input, 
                        ARL_STATICf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                        (uint32 *)rep_entry, &temp);
                (DRV_SERVICES(unit)->reg_field_get)
                        (unit, ARLA_ENTRY1r, (uint32 *)input, 
                        ARL_CONf, &temp);
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        (uint32 *)rep_entry, &temp);
                temp = 1;
                (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                        (uint32 *)rep_entry, &temp);
            }
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

#define DRV_MEM_CLEAR_STATIC_UNICAST 0x1<<0
#define DRV_MEM_CLEAR_STATIC_MULTICAST 0x1<<1
#define DRV_MEM_CLEAR_STATIC_BY_PORT 0x1<<2
#define DRV_MEM_CLEAR_STATIC_BY_VLANID 0x1<<3

int
_drv_mem_clear_static(int unit, uint32* entry, uint32 types)
{
    uint32 flags, temp, port, vlan;
    int rv = SOC_E_NONE;
    soc_control_t   *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t output, output1;
    uint64 temp_mac_field;
    uint8 temp_mac_addr[6];
    int index_min, index_max, index_count;
    int32 idx,valid;
    uint32 key;

    index_min = SOC_MEM_BASE(unit, L2_ARLm);
    index_max = SOC_MEM_BASE(unit, L2_ARLm) + SOC_MEM_SIZE(unit, L2_ARLm);
    index_count = SOC_MEM_SIZE(unit, L2_ARLm);
    if(soc->arl_table != NULL){
        for (idx = index_min; idx < index_count; idx++) {
            sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
            if(!ARL_ENTRY_NULL(&soc->arl_table[idx])) {
                ARL_SW_TABLE_LOCK(soc);
                sal_memcpy(&output, &soc->arl_table[idx], 
                    sizeof(l2_arl_sw_entry_t));
                ARL_SW_TABLE_UNLOCK(soc);
            } else {
                continue;
            }
            (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                    (uint32 *)&output, (uint32 *) &valid);
            if (valid){
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    (uint32 *)&output, &temp);
                if (temp == 0) {
                    continue;
                }
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                if (temp_mac_addr[0] & 0x01) { /* mcast address */
                    if (types & DRV_MEM_CLEAR_STATIC_MULTICAST) {
                        /* Check VALNID */
                        if (types & DRV_MEM_CLEAR_STATIC_BY_VLANID) {
                            (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                                (uint32 *)entry, &vlan);
                            (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                                (uint32 *)&output, &temp);
                            if (vlan != temp) {
                                continue;
                            }
                        }
                        (DRV_SERVICES(unit)->mem_delete)
                            (unit, DRV_MEM_MARL, (uint32 *)&output, 
                            DRV_MEM_OP_BY_HASH_BY_MAC | 
                            DRV_MEM_OP_BY_HASH_BY_VLANID);
                    }
                } else { /* unicast address */
                    if (types & DRV_MEM_CLEAR_STATIC_UNICAST) {
                        /* Check Port ID */
                        if (types & DRV_MEM_CLEAR_STATIC_BY_PORT) {
                            (DRV_SERVICES(unit)->mem_field_get)
                                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT,
                                    (uint32 *)entry, &port);
                            (DRV_SERVICES(unit)->mem_field_get)
                                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT,
                                    (uint32 *)&output, &temp);
                            if (port != temp) {
                                continue;
                            }
                        }
                        /* Check VLANID */
                        if (types & DRV_MEM_CLEAR_STATIC_BY_VLANID) {
                            (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                                (uint32 *)entry, &vlan);
                            (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                                (uint32 *)&output, &temp);
                            if (vlan != temp) {
                                continue;
                            }
                        }
                        (DRV_SERVICES(unit)->mem_delete)
                            (unit, DRV_MEM_MARL, (uint32 *)&output, 
                            DRV_MEM_OP_BY_HASH_BY_MAC |
                            DRV_MEM_OP_BY_HASH_BY_VLANID);
                    }
                }
            }
        }
    }else {            
    /* Get other valid static entries */
    flags = DRV_MEM_OP_SEARCH_VALID_START;
    (DRV_SERVICES(unit)->mem_search)
        (unit, DRV_MEM_ARL, NULL, NULL, NULL, flags);
    flags = DRV_MEM_OP_SEARCH_VALID_GET;
    rv = SOC_E_EXISTS;
    while (rv == SOC_E_EXISTS) {
        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
        rv  = (DRV_SERVICES(unit)->mem_search)(unit, DRV_MEM_ARL, 
            &key, (uint32 *)&output, (uint32 *)&output1, flags);
        if (rv == SOC_E_EXISTS) {
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                (uint32 *)&output, &temp);
            if (temp == 0) {
                continue;
            }
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                (uint32 *)&output, (uint32 *)&temp_mac_field);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            if (temp_mac_addr[0] & 0x01) { /* mcast address */
                if (types & DRV_MEM_CLEAR_STATIC_MULTICAST) {
                    /* Check VALNID */
                    if (types & DRV_MEM_CLEAR_STATIC_BY_VLANID) {
                        (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                                (uint32 *)entry, &vlan);
                        (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                                (uint32 *)&output, &temp);
                        if (vlan != temp) {
                            continue;
                        }
                    }
                    (DRV_SERVICES(unit)->mem_delete)
                        (unit, DRV_MEM_MARL, (uint32 *)&output, 
                        DRV_MEM_OP_BY_HASH_BY_MAC | 
                        DRV_MEM_OP_BY_HASH_BY_VLANID);
                }
            } else { /* unicast address */
                if (types & DRV_MEM_CLEAR_STATIC_UNICAST) {
                    /* Check Port ID */
                    if (types & DRV_MEM_CLEAR_STATIC_BY_PORT) {
                        (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                                (uint32 *)entry, &port);
                        (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                                (uint32 *)&output, &temp);
                        if (port != temp) {
                            continue;
                        }
                    }
                    /* Check VLANID */
                    if (types & DRV_MEM_CLEAR_STATIC_BY_VLANID) {
                        (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                                (uint32 *)entry, &vlan);
                        (DRV_SERVICES(unit)->mem_field_get)
                                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                                (uint32 *)&output, &temp);
                        if (vlan != temp) {
                            continue;
                        }
                    }
                    (DRV_SERVICES(unit)->mem_delete)
                        (unit, DRV_MEM_MARL, (uint32 *)&output, 
                        DRV_MEM_OP_BY_HASH_BY_MAC |
                        DRV_MEM_OP_BY_HASH_BY_VLANID);
                }
            }
        } 
    }
}
    return SOC_E_NONE;
}

int
_drv_arl_database_delete_by_portvlan(int unit, uint32 src_port, 
    uint32 vlanid, uint32 flags)
{
    uint32 temp, port, vlan;
    int rv = SOC_E_NONE;
    soc_control_t   *soc = SOC_CONTROL(unit);
    int index_min, index_max, index_count;
    int32 idx,valid;

    index_min = SOC_MEM_BASE(unit, L2_ARLm);
    index_max = SOC_MEM_BASE(unit, L2_ARLm) + SOC_MEM_SIZE(unit, L2_ARLm);
    index_count = SOC_MEM_SIZE(unit, L2_ARLm);
    
    if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
        if (soc->arl_table != NULL) {
            ARL_SW_TABLE_LOCK(soc);
            sal_memset(&soc->arl_table[0], 0, 
                sizeof (l2_arl_sw_entry_t) * index_count);
            ARL_SW_TABLE_UNLOCK(soc);
        }
        return rv;
    }
    if (soc->arl_table != NULL) {
        ARL_SW_TABLE_LOCK(soc);
        for (idx = index_min; idx < index_count; idx++) {
            if(!ARL_ENTRY_NULL(&soc->arl_table[idx])) {
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                    (uint32 *)&soc->arl_table[idx], (uint32 *) &valid);
                if (valid) {
                    /* Check PORT */
                    if (flags & DRV_MEM_OP_DELETE_BY_PORT){
                        (DRV_SERVICES(unit)->mem_field_get)
                            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                            (uint32 *)&soc->arl_table[idx], &port);
                        if (port != src_port) {
                            continue;
                        }
                    }
                    /* Check VLAN */
                    if (flags & DRV_MEM_OP_DELETE_BY_VLANID){
                        (DRV_SERVICES(unit)->mem_field_get)
                            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                            (uint32 *)&soc->arl_table[idx], &vlan);
                        if (vlan !=vlanid) {
                            continue;
                        }
                    }
                    /* Check static/dynamic entry */
                    if (!(flags & DRV_MEM_OP_DELETE_BY_STATIC)){
                        (DRV_SERVICES(unit)->mem_field_get)
                            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                            (uint32 *)&soc->arl_table[idx], &temp);
                        if (temp != 0) {
                            continue;
                        }
                    }
                    soc_arl_database_delete(unit, idx);
                }
            }
        }
        ARL_SW_TABLE_UNLOCK(soc);
    }
    return rv;
}

/* Function : _drv_arl_database_delete()
 *  - serve the delete request on SW ARL table and execute the registered 
 *      callback routine for such deletion.
 *
 * Note :
 *  1. The 2nd parameter, "index", for all robo chips before TB was designed 
 *      to carry the bin_id of the target entry to delete.
 *      - The early ROBO device before TB can't report the hashed index. Thus 
 *          our SW ARL design implemented a SW basis hash algorithm to 
 *          retrieve this index.
 *          (TB new feature has implemented to report this hash basis index)
 */
int
_drv_arl_database_delete(int unit, int index, void *entry)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    uint32  valid;
    uint32  table_index = 0;
    l2_arl_sw_entry_t output;
    uint32 cb = 0;
    
    if (SOC_IS_TBX(unit)){
        table_index = index;
    } else {
        table_index = _drv_arl_calc_hash_index(unit, index, entry);
    }
    soc_cm_debug(DK_ARL | DK_TESTS, 
            "%s, arl_index=%d\n", FUNCTION_NAME(), table_index);

    if (soc->arl_table != NULL) {
        ARL_SW_TABLE_LOCK(soc);
        soc_arl_database_dump(unit, table_index, &output);
        (DRV_SERVICES(unit)->mem_field_get)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
            (uint32 *)&output, &valid);
        if (valid) {
            cb = 1;
            valid = 0;
            (DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                (uint32 *)&output, &valid);
            sal_memcpy(&(soc->arl_table[table_index]), &output, 
                sizeof(l2_arl_sw_entry_t));
        }
        ARL_SW_TABLE_UNLOCK(soc);
        if (cb) {
            soc_robo_arl_callback(unit, 
                (l2_arl_sw_entry_t *)&output, NULL);
        }
    }
    return SOC_E_NONE;
}

/* Function : _drv_arl_database_insert()
 *  - serve the insert request on SW ARL table and execute the registered 
 *      callback routine for such insertion.
 *
 * Note :
 *  1. The 2nd parameter, "index", for all robo chips before TB was designed 
 *      to carry the bin_id of the target entry to delete.
 *      - The early ROBO device before TB can't report the hashed index. Thus 
 *          our SW ARL design implemented a SW basis hash algorithm to 
 *          retrieve this index.
 *          (TB new feature has implemented to report this hash basis index)
 *  2. This routine currently is called in DRV_MEM_INSERT() routine only.
 */
int
_drv_arl_database_insert(int unit, int index, void *entry)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    uint32          table_index = 0, valid = 0;
    int             rv = SOC_E_NONE, is_modify = 0;
    l2_arl_sw_entry_t   old_entry, output;

    if (SOC_IS_TBX(unit)){
        table_index = index;
    } else {
        table_index = _drv_arl_calc_hash_index(unit, index, entry);
    }
    soc_cm_debug(DK_ARL | DK_TESTS, 
            "%s, arl_index=%d\n", FUNCTION_NAME(), table_index);

    if (soc->arl_table != NULL) {
        
        sal_memcpy(&output, entry, 
                sizeof(l2_arl_sw_entry_t));
        
        /* SDK-32894 : L2 callback issue while SW station movement occurred.
         *
         * Fix process :
         *  - issue an ARL_DELETE callback before ARL_INSERT callback if the 
         *      indicated L2 entry is valid in SW ARL table.
         *      (ARL modify callback process = ARL_DELETE + ARL_INSERT)
         */
        
        ARL_SW_TABLE_LOCK(soc);
        /* to retreive current SW ARL entry first */
        soc_arl_database_dump(unit, table_index, &old_entry);

        /* sw arl insert */
        sal_memcpy(&(soc->arl_table[table_index]), &output, 
                sizeof(l2_arl_sw_entry_t));
        ARL_SW_TABLE_UNLOCK(soc);
        
        rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_VALID, (uint32 *)&old_entry, &valid);
        if (rv){
            soc_cm_debug(DK_WARN, 
                    "%s, unexpect error(%d) while retrieving valid status!",
                    FUNCTION_NAME(), rv);
            return rv;
        }

        if (SOC_IS_TBX(unit)){
            /* The valid field valid at PENDING(0x1) and VALID(0x3) will be 
             *  treated as valid status here.
             */
            is_modify = ((valid == 0x1) || (valid == 0x3)) ? 1 : 0;
        } else {
            is_modify = (valid) ? 1 : 0;
        }

        /* issue the delete callback before insert callback if original entry 
         * is valid.
         */
        if (is_modify) {
            soc_robo_arl_callback(unit, 
                (l2_arl_sw_entry_t *)&old_entry, NULL);
        }
        
        soc_robo_arl_callback(unit, NULL,
              (l2_arl_sw_entry_t *)&output);
    }
    return SOC_E_NONE;
}
/*
 *  Function : drv_mem_length_get
 *
 *  Purpose :
 *      Get the number of entries of the selected memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      data   :   total number entries of this memory type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_mem_length_get(int unit, uint32 mem, uint32 *data)
{

    soc_mem_info_t *meminfo;
    
    switch (mem)
    {
        case DRV_MEM_ARL:
            meminfo = &SOC_MEM_INFO(unit, L2_ARLm);
            break;
        case DRV_MEM_MARL:
            meminfo = &SOC_MEM_INFO(unit, L2_MARL_SWm);
            break;            
        case DRV_MEM_VLAN:
            meminfo = &SOC_MEM_INFO(unit, VLAN_1Qm);
            break;
        case DRV_MEM_MSTP:
            meminfo = &SOC_MEM_INFO(unit, MSPT_TABm);
            break;
        case DRV_MEM_MCAST:
            meminfo = &SOC_MEM_INFO(unit, MARL_PBMPm);
            break;
        case DRV_MEM_SECMAC:
            meminfo = &SOC_MEM_INFO(unit, STSEC_MAC0m);
            break;
        case DRV_MEM_GEN:
            meminfo = &SOC_MEM_INFO(unit, GEN_MEMORYm);
            break;
        default:
            return SOC_E_PARAM;
    }

  *data = meminfo->index_max - meminfo->index_min + 1;

  return SOC_E_NONE;
}


/*
 *  Function : drv_mem_width_get
 *
 *  Purpose :
 *      Get the width of selected memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      data   :   total number bits of entry.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_mem_width_get(int unit, uint32 mem, uint32 *data)
{

    switch (mem)
    {
        case DRV_MEM_ARL:
            *data = sizeof(l2_arl_entry_t);
            break;
         case DRV_MEM_MARL:
            *data = sizeof(l2_marl_sw_entry_t);
            break;
        case DRV_MEM_VLAN:
            *data = sizeof(vlan_1q_entry_t);
            break;
        case DRV_MEM_MSTP:
            *data = sizeof(mspt_tab_entry_t);
            break;
        case DRV_MEM_MCAST:
            *data = sizeof(marl_pbmp_entry_t);
            break;
        case DRV_MEM_SECMAC:
            *data = sizeof(stsec_mac0_entry_t);
            break;
        case DRV_MEM_GEN:
            *data = sizeof(gen_memory_entry_t);
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}


    #define NUMBER_OF_PORTS_PER_BLOCK       8
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
drv_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max, base;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32 mem_addr = 0;
    uint32  temp, secmac_num;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;

    soc_cm_debug(DK_MEM, 
        "drv_mem_read(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
            mem_id = L2_ARLm;
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
        case DRV_MEM_SECMAC:
            if ((rv = (DRV_SERVICES(unit)->dev_prop_get)
                (unit, DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT, 
                    &secmac_num)) < 0) {
                goto mem_read_exit;
            }
            i = (entry_id / (secmac_num* NUMBER_OF_PORTS_PER_BLOCK));
            switch (i) {
                case 0:
                    mem_id = STSEC_MAC0m;
                    break;
                case 1:
                    mem_id = STSEC_MAC1m;
                    break;
                case 2:
                    mem_id = STSEC_MAC2m;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            entry_id -= (secmac_num * NUMBER_OF_PORTS_PER_BLOCK) * i;
            break;
        case DRV_MEM_GEN:
            mem_id = GEN_MEMORYm;
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    /* add code here to check addr */
    base = soc_mem_base(unit, mem_id);
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    /* check count */
    if (count < 1) {
        return SOC_E_PARAM;
    }

    /* process read action */
    MEM_LOCK(unit, GEN_MEMORYm);
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

        /* Read memory control register */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, (DRV_SERVICES(unit)->reg_addr)
            (unit, GENMEM_CTLr, 0, 0), &acc_ctrl, 1)) < 0) {
            goto mem_read_exit;
        }
        temp = MEM_TABLE_READ;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, MEM_RWf, &temp);
        temp = base + entry_id + i;
        /* Set memory entry address */
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_ADDRr, &mem_addr, GENMEM_ADDRf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, MEM_RW_ADDR_ADDR, &mem_addr, 3)) < 0) {
            goto mem_read_exit;}

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_STDNf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, MEM_RW_CTRL_ADDR, &acc_ctrl, 1)) < 0) {
            goto mem_read_exit;}

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, MEM_RW_CTRL_ADDR, &acc_ctrl, 1)) < 0) {
                goto mem_read_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }

        /* Read the current generic memory entry */
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, MEM_RW_DATA_ADDR, gmem_entry, 8)) < 0) {
            goto mem_read_exit;
        }
        gmem_entry++;
        

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
drv_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{

    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max, base;
    uint32 i;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32 mem_addr = 0;
    uint32  temp, secmac_num;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 reset_mem_addr = 0;
    
    soc_cm_debug(DK_MEM, "drv_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
            mem_id = L2_ARLm;
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
        case DRV_MEM_SECMAC:
            if ((rv = (DRV_SERVICES(unit)->dev_prop_get)
                (unit, DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT, &secmac_num)) < 0) {
                goto mem_write_exit;
            }
            i = (entry_id / (secmac_num * NUMBER_OF_PORTS_PER_BLOCK));
            switch (i) {
                case 0:
                    mem_id = STSEC_MAC0m;
                    break;
                case 1:
                    mem_id = STSEC_MAC1m;
                    break;
                case 2:
                    mem_id = STSEC_MAC2m;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            entry_id -= (secmac_num * NUMBER_OF_PORTS_PER_BLOCK) * i;
            break;
        case DRV_MEM_GEN:
            mem_id = GEN_MEMORYm;
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    /* add code here to check addr */
    base = soc_mem_base(unit, mem_id);
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);
    
    if (count < 1) {
        return SOC_E_PARAM;
    }

    /* process write action */
    MEM_LOCK(unit, GEN_MEMORYm);
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, GEN_MEMORYm);
            return SOC_E_PARAM;
        }

        /* write data */
        if ((rv =(DRV_SERVICES(unit)->reg_write)
            (unit, MEM_RW_DATA_ADDR, gmem_entry, 8)) < 0) {
            goto mem_write_exit;
        }
        
        /* set memory address */
        temp = base + entry_id + i;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_ADDRr, &mem_addr, GENMEM_ADDRf, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, MEM_RW_ADDR_ADDR, &mem_addr, 3)) < 0) {
            goto mem_write_exit;
        }
        
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, MEM_RW_CTRL_ADDR, &acc_ctrl, 1)) < 0) {
            goto mem_write_exit;
        }
        temp = MEM_TABLE_WRITE;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, MEM_RWf, &temp);

        temp = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_STDNf, &temp);
        
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, MEM_RW_CTRL_ADDR, &acc_ctrl, 1)) < 0) {
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, MEM_RW_CTRL_ADDR, &acc_ctrl, 1)) < 0) {
                goto mem_write_exit;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, GENMEM_CTLr, &acc_ctrl, GENMEM_STDNf, &temp);
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
            sal_memcpy(cache + (entry_id + i) * entry_size, 
                gmem_entry, entry_size);
            CACHE_VMAP_SET(vmap, (entry_id + i));
        }
        gmem_entry++;

        /*
         * Robo5324 only.
         * GNATS 1506. Read/write ARL table via page 0x05 registers fail,
         *             after write static security mac memory.
         * Software workaround: Set GENMEN_ADDR 0 before r/w ARL table. 
         */
        if (mem == DRV_MEM_SECMAC) {
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, MEM_RW_ADDR_ADDR, &reset_mem_addr, 3)) < 0) {
                goto mem_write_exit;
            }
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
drv_mem_field_get(int unit, uint32 mem, 
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
                field_id = INDEX(EVEN_PBMPf);
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP1) {
                field_id = INDEX(ODD_PBMPf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(EVEN_VALIDf);
            }else if (field_index == DRV_MEM_FIELD_VALID1) {
                field_id = INDEX(ODD_VALIDf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_SECMAC:
            mem_id = STSEC_MAC0m;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VLAIDf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_GEN:
            return SOC_E_PARAM;
            break;
        case DRV_MEM_ARL_HW:
            mem_id = L2_ARLm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDR2f);
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(MARL_PBMP_IDXf);
                /* should be L2_MARLm, but no defined */
                mem_id = L2_ARLm;
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
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
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
    if (mem_id == L2_ARL_SWm) {
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
    if (mem_id == L2_ARL_SWm) {
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
drv_mem_field_set(int unit, uint32 mem, 
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
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(EVEN_PBMPf);
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP1) {
                field_id = INDEX(ODD_PBMPf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(EVEN_VALIDf);
            }else if (field_index == DRV_MEM_FIELD_VALID1) {
                field_id = INDEX(ODD_VALIDf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_SECMAC:
            mem_id = STSEC_MAC0m;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VLAIDf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_GEN:
            return SOC_E_PARAM;
            break;
        case DRV_MEM_ARL_HW:
            mem_id = L2_ARLm;
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDR2f);
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(MARL_PBMP_IDXf);
                /* should be L2_MARLm, but no defined */
                mem_id = L2_ARLm;
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
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
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
    if (mem_id == L2_ARL_SWm) {
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
    if (mem_id == L2_ARL_SWm) {
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
drv_mem_clear(int unit, uint32 mem)
{
    int rv = SOC_E_NONE;
    uint32 count;
    uint32 entry_id, del_id;
    int mem_id;
    uint32 *entry;


    soc_cm_debug(DK_MEM, "drv_mem_clear : mem=0x%x\n", mem);
    count = 0;
    entry_id =0;
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
        case DRV_MEM_MSTP:
            mem_id = MSPT_TABm;
            break;
        case DRV_MEM_MCAST:
            mem_id = MARL_PBMPm;
            break;
        case DRV_MEM_GEN:
            mem_id = GEN_MEMORYm;
            break;
        case DRV_MEM_SECMAC:
            /* 
            Since there 3 memory blocks belong to Secure MAC memory, 
            we need to clear all of them.
            */
            for (mem_id = STSEC_MAC0m; mem_id <= STSEC_MAC2m; mem_id++) {
                count = soc_robo_mem_index_count(unit, mem_id);
                switch(mem_id) {
                    case STSEC_MAC2m:
                        entry_id = count * 2;
                        break;
                    case STSEC_MAC1m:
                        entry_id = count * 1;
                        break;
                    case STSEC_MAC0m:
                    default:
                        entry_id = 0;
                        break;
                }
            }
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
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

    for (del_id = entry_id; del_id < (entry_id + count) ; del_id++) {
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
 *  Function : drv_mem_search
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
drv_mem_search(int unit, uint32 mem, 
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

    rv = _drv_mem_search(unit, mem, key, entry, entry1, flags, &index);
    
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
 *
 */
int 
drv_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int             rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t       output;
    uint32          temp, count, mem_id;
    uint64          entry_reg;
    uint8           mac_addr[6];
    uint64               mac_field, mac_field_output, mac_field_entry;
    uint32          vid, control, vid_output, vid_entry;
    int             index;

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
    if ((rv =  _drv_mem_search
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
            (unit, ARLA_ENTRY0r, (uint32 *)&entry_reg, 
            ARL_MACADDRf, (uint32 *)&mac_field);

        if (mac_addr[0] & 0x01) { /* The input is the mcast address */
            /* multicast group index */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
               (unit, ARLA_ENTRY0r, (uint32 *)&entry_reg, ARL_MCAST_IDf, &temp);

            /* arl_control */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_ENTRY0r, (uint32 *)&entry_reg, ARL_CONf, &temp);

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
                (unit, ARLA_ENTRY0r, (uint32 *)&entry_reg, ARL_STATICf, &temp);

            /* valid bit */
            temp = 1;
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_ENTRY0r, (uint32 *)&entry_reg, ARL_VALIDf, &temp);
        } else { /* unicast address */
            /* source port id */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_ENTRY1r, (uint32 *)&entry_reg, ARL_PIDf, &temp);

            /* priority */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_ENTRY1r, (uint32 *)&entry_reg, ARL_PRIf, &temp);

            /* age */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_ENTRY1r, (uint32 *)&entry_reg, ARL_AGEf, &temp);

            /* arl_control */
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_ENTRY1r, (uint32 *)&entry_reg, ARL_CONf, &temp);

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
                (unit, ARLA_ENTRY1r, (uint32 *)&entry_reg, ARL_STATICf, &temp);

            temp = 1;
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ARLA_ENTRY1r, (uint32 *)&entry_reg, ARL_VALIDf, &temp);
        }


        /* write ARL and VID entry register*/
        if (index == 0){ /* entry 0 */
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_VID_ENTRY0r, 0, 0), &vid, 2)) < 0) {
                goto mem_insert_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_ENTRY0r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
                goto mem_insert_exit;
            }
        } else { /* entry 1 */
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_VID_ENTRY1r, 0, 0), &vid, 2)) < 0) {
                goto mem_insert_exit;
            }
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (DRV_SERVICES(unit)->reg_addr)
                (unit, ARLA_ENTRY1r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
                goto mem_insert_exit;
            }
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
drv_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int                 rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t       output;
    uint64               entry_reg;
    uint32          temp, count, mem_id;
    uint32          control;
    int             index;
    uint32          reg_len, reg_addr, reg_value, bmp;
    uint32 ag_port_mode = 0, ag_vlan_mode = 0;
    uint32              ag_static_mode = 0;
    uint32  mst_con, age_vid, age_port;
    uint32 src_port = 0, vlanid = 0;

    soc_cm_debug(DK_MEM, "drv_mem_delete : mem=0x%x, flags = 0x%x\n",
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
    if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
        ag_static_mode = 1;
        ag_port_mode = 1;
    }
    
    if ((ag_port_mode) || (ag_vlan_mode) || (ag_static_mode)) {
        /* aging port mode */
        if (ag_port_mode) {
            if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
                bmp = SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0);
            } else { /* by port */
                temp = 0;
                (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
                src_port = temp;
                bmp = 0x1 << temp;
            }
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CHIP0_PORT_AGEr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CHIP0_PORT_AGEr, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
            /*
             * keep original age port bitmap
             */
            age_port = reg_value;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, CHIP0_PORT_AGEr, &reg_value, AGE_EN_PRTf, &bmp));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
        }
        /* aging vlan mode */
        if (ag_vlan_mode) {
            temp = 0;
            (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &temp);
            vlanid = temp;
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, AGEOUT_EN_VIDr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, AGEOUT_EN_VIDr, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
            /* 
             * keep original age vlan ID
             */
            age_vid = reg_value;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_field_set)
                (unit, AGEOUT_EN_VIDr, &reg_value, AGE_EN_VIDf, &temp));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto mem_delete_exit;
            }
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
        /* Static entries will be deleted after fast age out finished */
        temp = 0;
        SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_set)(unit, MST_CONr, &reg_value, 
            EN_AGE_STATICf, &temp));
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
        /* Special Case for Static Entries */
        if (ag_static_mode ||  (flags & DRV_MEM_OP_DELETE_BY_STATIC) ) {
            if ((ag_port_mode) || (ag_vlan_mode)) {
                temp = 0;
                if (mem == DRV_MEM_ARL) {
                    temp |= DRV_MEM_CLEAR_STATIC_UNICAST;
                } else {
                    temp |= DRV_MEM_CLEAR_STATIC_MULTICAST;
                }
                if ((flags & DRV_MEM_OP_DELETE_ALL_ARL) == 0) { 
                    if (ag_port_mode ) {
                        temp |= DRV_MEM_CLEAR_STATIC_BY_PORT;
                    }
                    if (ag_vlan_mode ) {
                        temp |= DRV_MEM_CLEAR_STATIC_BY_VLANID;
                    }
                } else {
                     temp |= DRV_MEM_CLEAR_STATIC_UNICAST;
                     temp |= DRV_MEM_CLEAR_STATIC_MULTICAST;
                }
                _drv_mem_clear_static(unit, entry, temp);
            }
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
        if ((rv =  _drv_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, NULL,
            flags, &index))< 0) {
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
            if (index == 0) { 
                
                /* entry 0 */
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_ENTRY0r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
                    goto mem_delete_exit;
                }
                /* check static bit if set */
                if (!ag_static_mode){
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                                    (unit, ARLA_ENTRY0r, (uint32 *)&entry_reg, 
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
                (unit, ARLA_ENTRY0r, (uint32 *)&entry_reg, ARL_VALIDf, &temp));
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_ENTRY0r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
                    goto mem_delete_exit;
                }
            } else { 
                
                /* entry 1 */
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_ENTRY1r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
                    goto mem_delete_exit;
                }
                /* check static bit if set */
                if (!ag_static_mode){
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                                    (unit, ARLA_ENTRY1r, (uint32 *)&entry_reg, 
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
                (unit, ARLA_ENTRY0r, (uint32 *)&entry_reg, ARL_VALIDf, &temp));
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, (DRV_SERVICES(unit)->reg_addr)
                    (unit, ARLA_ENTRY1r, 0, 0), (uint32 *)&entry_reg, 8)) < 0) {
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
    /* 
     * Restore MST Settings
     */
    if ((ag_port_mode) || (ag_vlan_mode)) {
        /* MST configuration */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MST_CONr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MST_CONr, 0, 0);
        (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &mst_con, reg_len);
        if (ag_port_mode) {
            /* Age Ports */
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CHIP0_PORT_AGEr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CHIP0_PORT_AGEr, 0, 0);
            (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &age_port, reg_len);
        }
        if (ag_vlan_mode) {
            /* Age Vid */
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, AGEOUT_EN_VIDr);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, AGEOUT_EN_VIDr, 0, 0);
            (DRV_SERVICES(unit)->reg_write)(unit, reg_addr, &age_vid, reg_len);
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

/*
 * Function:
 *      soc_mem_debug_set
 * Purpose:
 *      Enable or disable MMU debug mode.
 * Returns:
 *      Previous enable state, or SOC_E_XXX on error.
 */

int
soc_robo_mem_debug_set(int unit, int enable)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_mem_debug_get
 * Purpose:
 *      Return current MMU debug mode status
 */

int
soc_robo_mem_debug_get(int unit, int *enable)
{
    return SOC_E_NONE;
}


/*
 *  Function : drv_mem_cache_get
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
drv_mem_cache_get(int unit,
                  uint32 mem, uint32 *enable)
{
    int mem_id;
    assert(SOC_UNIT_VALID(unit));

    switch (mem) {
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            break;
        case DRV_MEM_SECMAC:
            if (SOC_IS_ROBO5324(unit)) {
                /*
                 * Secure MAC memory mean STSEC_MAC0m, STSEC_MAC1m
                 * and STSEC_MAC2m. Select one pf them.
                 */
                mem_id = STSEC_MAC0m;
                break;
            }
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
 *  Function : drv_mem_cache_set
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
drv_mem_cache_set(int unit,
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
            mem_id = INDEX(VLAN_1Qm);
            count = 1;
            break;
        case DRV_MEM_SECMAC:
            if (SOC_IS_ROBO5324(unit)) {
                /*
                 * For Secure MAC memory need enable STSEC_MAC0m, STSEC_MAC1m
                 * and STSEC_MAC2m. 
                 */
                mem_id = STSEC_MAC0m;
                count = 3;
                break;
            }
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
                     unit, (SOC_IS_ROBO(unit)?SOC_ROBO_MEM_UFNAME(unit, mem_id):""),
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
