/*
 * $Id: mem.c 1.4.122.1 Broadcom SDK $
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

#include <soc/cmic.h>
#include <soc/error.h>
#include <soc/register.h>
#include <soc/mcm/robo/driver.h>
#include <soc/spi.h>
#include <soc/arl.h>
#include "robo_53101.h"



#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

#define BCM53101_VLAN_MEMORY 2 
#define BCM53101_ARL_MEMORY  3

#define BCM53101_MEM_OP_WRITE    0
#define BCM53101_MEM_OP_READ     1
#define BCM53101_MEM_OP_CLEAR    2

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
_drv_bcm53101_vlan_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int     rv = SOC_E_NONE;
    int     i, index;
    uint32  retry;
    uint32  temp, reg_value;
    vlan_1q_entry_t *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0;i < count; i++ ) {        
        index = entry_id + i;

        /* fill index */
        reg_value = 0;
        temp = index;
        soc_ARLA_VTBL_ADDRr_field_set(unit, &reg_value, 
            VTBL_ADDR_INDEXf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_ADDRr(unit, &reg_value)) < 0) {
            goto mem_read_exit;
        }

        /* read control setting */
        if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
            goto mem_read_exit;
        }

        temp = BCM53101_MEM_OP_READ;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value,
            ARLA_VTBL_RW_CLRf, &temp);

        temp = 1;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value,
            ARLA_VTBL_STDNf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
                goto mem_read_exit;
            }
            soc_ARLA_VTBL_RWCTRLr_field_get(unit, &reg_value,
                ARLA_VTBL_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }

        /* get result */
        if ((rv = REG_READ_ARLA_VTBL_ENTRYr(unit, &reg_value)) < 0) {
            goto mem_read_exit;
        }

        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            MSPT_INDEXf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_SPT_GROUP_ID, 
                 (uint32 *)vlan_mem, &temp);

        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            FWD_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP, 
            (uint32 *)vlan_mem, &temp);
        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            UNTAG_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG, 
            (uint32 *)vlan_mem, &temp);
        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            FWD_MODEf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE, 
            (uint32 *)vlan_mem, &temp);
        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            POLICER_ENf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_EN, 
            (uint32 *)vlan_mem, &temp);
        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            POLICER_IDf, &temp);
        (DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_ID, 
            (uint32 *)vlan_mem, &temp);     
        
        vlan_mem++;
    }

mem_read_exit:     
    return rv;
}

/*
 *  Function : _drv_bcm53115_vlan_mem_write
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
_drv_bcm53101_vlan_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_value;
    uint32  retry, temp;
    vlan_1q_entry_t *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0;i < count; i++ ) {
        index = entry_id + i;

        reg_value = 0;
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_SPT_GROUP_ID, 
             (uint32 *)vlan_mem, &temp);
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            MSPT_INDEXf, &temp);
       
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP, 
             (uint32 *)vlan_mem, &temp);           
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            FWD_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG, 
             (uint32 *)vlan_mem, &temp);   
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            UNTAG_MAPf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE, 
             (uint32 *)vlan_mem, &temp);    
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            FWD_MODEf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_EN, 
             (uint32 *)vlan_mem, &temp);    
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            POLICER_ENf, &temp);
        (DRV_SERVICES(unit)->mem_field_get)
             (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_ID, 
             (uint32 *)vlan_mem, &temp);   
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            POLICER_IDf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_ENTRYr(unit, &reg_value)) < 0) {
            goto mem_write_exit;
        }

        /* fill index */
        reg_value = 0;
        temp = index;
        soc_ARLA_VTBL_ADDRr_field_set(unit, &reg_value, 
            VTBL_ADDR_INDEXf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_ADDRr(unit, &reg_value)) < 0) {
            goto mem_write_exit;
        }

        /* read control setting */
        if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
            goto mem_write_exit;
        }

        temp = BCM53101_MEM_OP_WRITE;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value,
            ARLA_VTBL_RW_CLRf, &temp);

        temp = 1;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value,
            ARLA_VTBL_STDNf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
            goto mem_write_exit;
        }
        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
                goto mem_write_exit;
            }
            soc_ARLA_VTBL_RWCTRLr_field_get(unit, &reg_value,
                ARLA_VTBL_STDNf, &temp);
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
 *  Function : _drv_mstp_mem_read
 *
 *  Purpose :
 *      Get the width of mstp memory. 
 *      The value returned in data is width in bits.
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
_drv_bcm53101_mstp_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_len, reg_addr;

    for (i = 0;i < count; i++ ) {        
        index = entry_id + i;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, INDEX(MST_TABr), 0, index);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, INDEX(MST_TABr));
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, entry, reg_len)) < 0) {
            goto mem_read_exit;
        }
        entry++;
    }

mem_read_exit:     
    return rv;
}

/*
 *  Function : _drv_bcm53115_mstp_mem_write
 *
 *  Purpose :
 *      Get the width of mstp memory. 
 *      The value returned in data is width in bits.
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
_drv_bcm53101_mstp_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_len, reg_addr;

    for (i = 0;i < count; i++ ) {
        index = entry_id + i;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, INDEX(MST_TABr), 0, index);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, INDEX(MST_TABr));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, entry, reg_len)) < 0) {
            goto mem_write_exit;
        }
        entry++;
    }

mem_write_exit:     
    return rv;
}




 /*
 *  Function : drv_bcm53115_mem_read
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
drv_bcm53101_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max, index;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32    temp;
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
    case DRV_MEM_MARL:
        mem_id = INDEX(L2_ARLm);
        break;
    case DRV_MEM_VLAN:
        mem_id = INDEX(VLAN_1Qm);
        break;
    case DRV_MEM_MSTP:
        mem_id = INDEX(MSPT_TABm);
        break;
    case DRV_MEM_EGRVID_REMARK:
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
    case DRV_MEM_CFP_STAT_IB:
    case DRV_MEM_CFP_STAT_OB:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_MCAST:
    case DRV_MEM_SECMAC:
    case DRV_MEM_GEN:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    
    if (((entry_id) < index_min) || 
        ((entry_id + count - 1) > index_max)) {
            return SOC_E_PARAM;
    }

    /* process read action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    /* add code here to check addr */
    if (mem_id == INDEX(L2_ARLm) ) {        
        if ((rv = REG_READ_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }

        temp = BCM53101_ARL_MEMORY;
        soc_MEM_CTRLr_field_set(unit, &acc_ctrl, 
            MEM_TYPEf, &temp);
        
        if ((rv = REG_WRITE_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }
    } else if (mem_id == INDEX(VLAN_1Qm)) {
        rv = _drv_bcm53101_vlan_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
    } else {
        rv = _drv_bcm53101_mstp_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
    }

    /* check count */
    if (count < 1) {
        MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
        return SOC_E_PARAM;
    }

    for (i = 0;i < count; i++ ) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
             MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* Return data from cache if active */

        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL && CACHE_VMAP_TST(vmap, (entry_id + i))) {
            sal_memcpy(gmem_entry, 
                cache + (entry_id + i) * entry_size, entry_size);
            continue;
        }

        /* Read memory control register */
        if ((rv = REG_READ_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }
        temp = BCM53101_MEM_OP_READ;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_RWf, &temp);

        index = entry_id + i;
        temp = index / 2;
        /* Set memory entry address */
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_ADRf, &temp);

        temp = 1;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_STDNf, &temp);
        if ((rv = REG_WRITE_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
                goto mem_read_exit;
            }
            soc_MEM_ADDRr_field_get(unit, &acc_ctrl, 
                MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }

        /* Read the current generic memory entry */
        if ((index % 2) == 0) {
            /* Read bin 0 entry */
            if ((rv = REG_READ_MEM_DEBUG_DATA_0_0r(unit, gmem_entry)) < 0) {
                goto mem_read_exit;
            }
            gmem_entry++;
            /* Read bin 1 entry */
            if (++i < count) {
                if ((rv = REG_READ_MEM_DEBUG_DATA_0_1r(
                    unit, gmem_entry)) < 0) {
                    goto mem_read_exit;
                }
                gmem_entry++;    
            }
        } else {
            if ((rv = REG_READ_MEM_DEBUG_DATA_0_1r(
                unit, gmem_entry)) < 0) {
                goto mem_read_exit;
            }
            gmem_entry++;  
        }
        

    }

 mem_read_exit:
     MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;

}

 /*
 *  Function : drv_bcm53115_mem_write
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
drv_bcm53101_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32    temp, index;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;

    soc_cm_debug(DK_MEM, "drv_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n",
         mem, entry_id, count);
         
    switch (mem)
    {
    case DRV_MEM_ARL:
    case DRV_MEM_ARL_HW:
    case DRV_MEM_MARL:
        mem_id = INDEX(L2_ARLm);
        break;
    case DRV_MEM_VLAN:
        mem_id = INDEX(VLAN_1Qm);
        break;
    case DRV_MEM_MSTP:
        mem_id = INDEX(MSPT_TABm);
        break;
    case DRV_MEM_EGRVID_REMARK:
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
    case DRV_MEM_CFP_STAT_IB:
    case DRV_MEM_CFP_STAT_OB:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_GEN:
    case DRV_MEM_MCAST:
    case DRV_MEM_SECMAC:
        return SOC_E_UNAVAIL;
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
    if (((entry_id) < index_min) || 
        ((entry_id + count - 1) > index_max)) {
            return SOC_E_PARAM;
    }

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    if (mem_id == INDEX(L2_ARLm) ){        
        if ((rv = REG_READ_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
        temp = BCM53101_ARL_MEMORY;
        soc_MEM_CTRLr_field_set(unit, &acc_ctrl, 
            MEM_TYPEf, &temp);
        
        if ((rv = REG_WRITE_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
    } else if (mem_id == INDEX(VLAN_1Qm)){
        rv = _drv_bcm53101_vlan_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;
    } else {
        rv = _drv_bcm53101_mstp_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;
    }
    
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* write data */
        index = entry_id + i;
        if ((index % 2) == 0) {
            if ((rv = REG_WRITE_MEM_DEBUG_DATA_0_0r(unit, gmem_entry)) < 0) {
                goto mem_write_exit;
            }
            if (++i < count) {
                if ((rv = REG_WRITE_MEM_DEBUG_DATA_0_1r(
                    unit, ++gmem_entry)) < 0) {
                    goto mem_write_exit;
                }
            }
        } else {
            if ((rv = REG_WRITE_MEM_DEBUG_DATA_0_1r(unit, gmem_entry)) < 0) {
                goto mem_write_exit;
            }
        }
        
        /* set memory address */
        temp = index / 2;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_ADRf, &temp);

        temp = BCM53101_MEM_OP_WRITE;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_RWf, &temp);

        temp = 1;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_STDNf, &temp);
        if ((rv = REG_WRITE_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
                goto mem_write_exit;
            }
            soc_MEM_ADDRr_field_get(unit, &acc_ctrl, 
                MEM_STDNf, &temp);
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
            if (((index % 2) == 0) && (i < count)) {
                sal_memcpy(cache + (index) * entry_size, 
                    --gmem_entry, entry_size * 2);
                CACHE_VMAP_SET(vmap, (index));
                CACHE_VMAP_SET(vmap, (index+1));
                gmem_entry++;
            } else {
                sal_memcpy(cache + (index) * entry_size, 
                    gmem_entry, entry_size);
                CACHE_VMAP_SET(vmap, (index));
            }
        }
        gmem_entry++;
    }

 mem_write_exit:
     MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
    
}

/*
 *  Function : drv_bcm53101_mem_field_get
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
drv_bcm53101_mem_field_get(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{

    soc_mem_info_t    *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int            mem_id, field_id;
    int            i, wp, bp, len;
#ifdef BE_HOST
    uint32              val32;
#endif

    soc_cm_debug(DK_MEM, "drv_mem_field_get(mem=0x%x,field_index=0x%x)\n",
         mem, field_index);
         
    switch (mem)
    {
    case DRV_MEM_ARL_HW:
        mem_id = INDEX(L2_ARLm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            field_id = INDEX(MULTCAST_PORTMAPf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            field_id = INDEX(TCf);
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            field_id = INDEX(VIDf);
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            field_id = INDEX(VALIDf);
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_ARL:
        mem_id = INDEX(L2_ARL_SWm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            field_id = INDEX(PORTIDf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            field_id = INDEX(TCf);
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            field_id = INDEX(VIDf);
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            field_id = INDEX(VALIDf);
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MARL:
        mem_id = INDEX(L2_MARL_SWm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);            
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            field_id =INDEX(PORTBMPf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            field_id = INDEX(TCf);
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            field_id = INDEX(VIDf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            field_id = INDEX(VALIDf);
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_VLAN:
        mem_id = INDEX(VLAN_1Qm);
        if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
            field_id = INDEX(MSPT_IDf);
        }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
            field_id = INDEX(UNTAG_MAPf);
        }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
            field_id = INDEX(FORWARD_MAPf);
        }else if (field_index == DRV_MEM_FIELD_FWD_MODE) {
            field_id = INDEX(FWD_MODEf);
        }else if (field_index == DRV_MEM_FIELD_POLICER_EN) {
            field_id = INDEX(POLICER_ENf);
        }else if (field_index == DRV_MEM_FIELD_POLICER_ID) {
            field_id = INDEX(POLICER_IDf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MSTP:
        mem_id = INDEX(MSPT_TABm);
        if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
            sal_memcpy(fld_data, entry, sizeof(mspt_tab_entry_t));
            return SOC_E_NONE;
        } else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_EGRVID_REMARK:
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_PROTOCOLVLAN:
        return SOC_E_UNAVAIL;
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
    if (mem_id == INDEX(L2_MARL_SWm) || mem_id == INDEX(L2_ARL_SWm)) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == INDEX(L2_ARLm)) {
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
    if (mem_id == INDEX(L2_MARL_SWm) || mem_id == INDEX(L2_ARL_SWm)) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == INDEX(L2_ARLm)) {
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
 *  Function : drv_bcm53101_mem_field_set
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
drv_bcm53101_mem_field_set(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t    *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int            mem_id, field_id;
    int            i, wp, bp, len;
#ifdef BE_HOST
    uint32               val32;
#endif

    soc_cm_debug(DK_MEM, "drv_mem_field_set(mem=0x%x,field_index=0x%x)0x%x\n",
         mem, field_index,*fld_data);
         
    switch (mem)
    {
    case DRV_MEM_ARL_HW:
        mem_id = INDEX(L2_ARLm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            field_id = INDEX(MULTCAST_PORTMAPf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            field_id = INDEX(TCf);
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            field_id = INDEX(VIDf);
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            field_id = INDEX(VALIDf);
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_ARL:
        mem_id = INDEX(L2_ARL_SWm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            field_id = INDEX(PORTIDf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            field_id = INDEX(TCf);
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            field_id = INDEX(VIDf);
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            field_id = INDEX(VALIDf);
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MARL:
        mem_id = INDEX(L2_MARL_SWm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            field_id = INDEX(MACADDRf);            
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            field_id =INDEX(PORTBMPf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            field_id = INDEX(TCf);
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            field_id = INDEX(VIDf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            field_id = INDEX(VALIDf);
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_VLAN:
        mem_id = INDEX(VLAN_1Qm);
        if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
            field_id = INDEX(MSPT_IDf);
        }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
            field_id = INDEX(UNTAG_MAPf);
        }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
            field_id = INDEX(FORWARD_MAPf);
        }else if (field_index == DRV_MEM_FIELD_FWD_MODE) {
            field_id = INDEX(FWD_MODEf);
        }else if (field_index == DRV_MEM_FIELD_POLICER_EN) {
            field_id = INDEX(POLICER_ENf);
        }else if (field_index == DRV_MEM_FIELD_POLICER_ID) {
            field_id = INDEX(POLICER_IDf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MSTP:
        mem_id = INDEX(MSPT_TABm);
        if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
            sal_memcpy(fld_data, entry, sizeof(mspt_tab_entry_t));
            return SOC_E_NONE;
        } else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_EGRVID_REMARK:
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_PROTOCOLVLAN:
        return SOC_E_UNAVAIL;
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
    if (mem_id == INDEX(L2_MARL_SWm) || mem_id == INDEX(L2_ARL_SWm)) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == INDEX(L2_ARLm)) {
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
    if (mem_id == INDEX(L2_MARL_SWm) || mem_id == INDEX(L2_ARL_SWm)) {
        val32 = entry[0];
        entry[0] = entry [2];
        entry [2] = val32;
    } else if (mem_id == INDEX(L2_ARLm)) {
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
