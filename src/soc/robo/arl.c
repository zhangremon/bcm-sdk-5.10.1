/*
 * $Id: arl.c 1.46.2.5 Broadcom SDK $
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
 * File:    arlmsg.c
 * Purpose: Keep a synchronized ARL shadow table.
 *      Provide a reliable stream of ARL insert/delete messages.
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/time.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/robo.h>

#include <soc/mcm/robo/driver.h>
#include <soc/arl.h>
#ifdef BCM_TB_SUPPORT
#include "bcm53280/robo_53280.h"
#endif

#define ARL_ENTRY_EQL(unit, e1, e2) \
    ((e1)->entry_data[0] == (e2)->entry_data[0] && \
     (e1)->entry_data[1] == (e2)->entry_data[1] && \
     (e1)->entry_data[2] == (e2)->entry_data[2])

/*
 * While the ARL is frozen, the ARLm lock is held.
 *
 * All tasks must obtain the ARLm lock before modifying the CML bits or
 * age timer registers.
 */

typedef struct freeze_s {
    int                 frozen;
    int                 save_cml[SOC_MAX_NUM_PORTS];
    int                 save_age_sec;
    int                 save_age_ena;
} freeze_t;

STATIC freeze_t arl_freeze_state[SOC_MAX_NUM_DEVICES];

int _drv_bcm5396_arl_hw_to_sw_entry (int unit, uint32 *key, l2_arl_entry_t *hw_arl, l2_arl_sw_entry_t  *sw_arl)
{
    int rv = SOC_E_NONE;
    uint32        temp;

    (DRV_SERVICES(unit)->mem_read)
        (unit, DRV_MEM_ARL, *key, 1, (uint32 *)hw_arl);
    /* Read VALID bit */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VALID, 
            (uint32 *)hw_arl, &temp);

    if (!temp) {
        return rv;
    }
    sal_memcpy(sw_arl, hw_arl, sizeof(l2_arl_entry_t));
    return rv;
}
int _drv_bcm5324_arl_hw_to_sw_entry (int unit, uint32 *key, l2_arl_entry_t *hw_arl, l2_arl_sw_entry_t  *sw_arl)
{
    uint32        temp;
    uint32      vid_rw, mcast_index;
    uint32        reg_addr, reg_value;
    uint64      rw_mac_field, temp_mac_field;
    int              reg_len;
    uint8          hash_value[6];
    uint8       mac_addr_rw[6], temp_mac_addr[6];
    uint16        hash_result;
    int         rv = SOC_E_NONE;

    (DRV_SERVICES(unit)->mem_read)
        (unit, DRV_MEM_ARL, *key, 1, (uint32 *)hw_arl);
    /* Read VALID bit */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VALID, 
            (uint32 *)hw_arl, &temp);

    if (!temp) {
        return rv;
    }

    (DRV_SERVICES(unit)->mem_field_set)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, (uint32 *)sw_arl, &temp);


    /* Read VLAN ID value */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VLANID, 
            (uint32 *)hw_arl, &vid_rw);
    
    (DRV_SERVICES(unit)->mem_field_set)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, (uint32 *)sw_arl, &vid_rw);
        
    /* Read MAC address bit 12~47 */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_MAC, 
            (uint32 *)hw_arl, (uint32 *)&rw_mac_field);
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
            (uint32 *)sw_arl, (uint32 *)&temp_mac_field);
    if (temp_mac_addr[0] & 0x01) { /* multicast entry */
        /* Multicast index */
       mcast_index = 0;
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)hw_arl, &temp);
        mcast_index = temp;
         (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_PRIORITY, 
                (uint32 *)hw_arl, &temp);
        mcast_index += (temp << 5);
         (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)hw_arl, &temp);
        mcast_index += (temp << 7);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
            (uint32 *)sw_arl, &mcast_index);
    } else { /* unicast entry */
        /* Source port number */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)hw_arl, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, (uint32 *)sw_arl, &temp);

        /* Priority queue value */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_PRIORITY, 
                (uint32 *)hw_arl, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, (uint32 *)sw_arl, &temp);

        /* Age bit */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)hw_arl, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, (uint32 *)sw_arl, &temp);
    }

    /* Static bit */
    (DRV_SERVICES(unit)->mem_field_get)(
        unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_STATIC, 
           (uint32 *)hw_arl, &temp);
    (DRV_SERVICES(unit)->mem_field_set)(
        unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, (uint32 *)sw_arl, &temp);

    return rv;

}

int _drv_bcm5398_arl_hw_to_sw_entry (int unit, uint32 *key, l2_arl_entry_t *hw_arl, l2_arl_sw_entry_t  *sw_arl)
{
    uint32        temp;
    uint32      vid_rw, mcast_pbmp;
    uint32        reg_addr, reg_value;
    uint64      rw_mac_field, temp_mac_field;
    int              reg_len;
    uint8          hash_value[6];
    uint8       mac_addr_rw[6], temp_mac_addr[6];
    uint16        hash_result;
    int         rv = SOC_E_NONE;

    (DRV_SERVICES(unit)->mem_read)
        (unit, DRV_MEM_ARL, *key, 1, (uint32 *)hw_arl);
    /* Read VALID bit */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VALID, 
            (uint32 *)hw_arl, &temp);

    if (!temp) {
        return rv;
    }

    (DRV_SERVICES(unit)->mem_field_set)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, (uint32 *)sw_arl, &temp);


    /* Read VLAN ID value */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VLANID, 
            (uint32 *)hw_arl, &vid_rw);
    
    (DRV_SERVICES(unit)->mem_field_set)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, (uint32 *)sw_arl, &vid_rw);
        
    /* Read MAC address bit 12~47 */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_MAC, 
            (uint32 *)hw_arl, (uint32 *)&rw_mac_field);
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
        } else if (temp == 0){
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
        } else {
            return SOC_E_CONFIG;
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
    _drv_arl_hash(hash_value, 6, &hash_result);
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
            (uint32 *)sw_arl, (uint32 *)&temp_mac_field);

    if (temp_mac_addr[0] & 0x01) { /* multicast entry */
        /* Multicast index */
        mcast_pbmp = 0;
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)hw_arl, &temp);
        mcast_pbmp = temp;
         (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_DEST_BITMAP1, 
                (uint32 *)hw_arl, &temp);
        mcast_pbmp += (temp << 4);
         (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)hw_arl, &temp);
        mcast_pbmp += (temp << 8);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
            (uint32 *)sw_arl, &mcast_pbmp);
    } else { /* unicast entry */
        /* Source port number */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)hw_arl, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, (uint32 *)sw_arl, &temp);

        /* Priority queue value */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_PRIORITY, 
                (uint32 *)hw_arl, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, (uint32 *)sw_arl, &temp);

        /* Age bit */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)hw_arl, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, (uint32 *)sw_arl, &temp);
    }

    /* Static bit */
    (DRV_SERVICES(unit)->mem_field_get)(
        unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_STATIC, 
           (uint32 *)hw_arl, &temp);
    (DRV_SERVICES(unit)->mem_field_set)(
        unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, (uint32 *)sw_arl, &temp);

    return rv;
}

int 
_drv_bcm53280_arl_hw_to_sw_entry (int unit, uint32 *key, 
        l2_arl_entry_t *hw_arl, l2_arl_sw_entry_t  *sw_arl)
{
    int         rv = SOC_E_NONE;
    
    rv = DRV_MEM_READ(unit, DRV_MEM_ARL_HW, *key, 1, (uint32 *)hw_arl);
    if (rv < 0){
        soc_cm_debug(DK_ERR, " %s, SOC driver problem on reading hw_arl!\n", 
                FUNCTION_NAME());
        return rv;
    }
    
    rv = DRV_MEM_READ(unit, DRV_MEM_ARL, *key, 1, (uint32 *)sw_arl);
    if (rv < 0){
        soc_cm_debug(DK_ERR, " %s, SOC driver problem on reading sw_arl!\n", 
                FUNCTION_NAME());
        return rv;
    }
    return rv;
}

int _drv_bcm53242_arl_hw_to_sw_entry (int unit, uint32 *key, l2_arl_entry_t *hw_arl, l2_arl_sw_entry_t  *sw_arl)
{
    uint32        temp;
    uint32      vid_rw, mcast_index;
    uint32        reg_value;
    uint64      rw_mac_field, temp_mac_field;
    uint8          hash_value[6];
    uint8       mac_addr_rw[6], temp_mac_addr[6];
    uint16        hash_result;
    int         rv = SOC_E_NONE;

    (DRV_SERVICES(unit)->mem_read)
        (unit, DRV_MEM_ARL, *key, 1, (uint32 *)hw_arl);
    /* Read VALID bit */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VALID, 
            (uint32 *)hw_arl, &temp);
    if (!temp) {
        return rv;
    }

    (DRV_SERVICES(unit)->mem_field_set)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, (uint32 *)sw_arl, &temp);


    /* Read VLAN ID value */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VLANID, 
            (uint32 *)hw_arl, &vid_rw);

    (DRV_SERVICES(unit)->mem_field_set)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, (uint32 *)sw_arl, &vid_rw);
        
    /* Read MAC address bit 12~47 */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_MAC, 
            (uint32 *)hw_arl, (uint32 *)&rw_mac_field);
    SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);

    /* check HASH enabled ? */
    /* check 802.1q enable */
    if ((rv = REG_READ_VLAN_CTRL0r(unit, &reg_value)) < 0) {
        return rv;
    }

    soc_VLAN_CTRL0r_field_get(unit, &reg_value,
        VLAN_ENf, &temp);

    sal_memset(hash_value, 0, sizeof(hash_value));
    if (temp) {
        soc_VLAN_CTRL0r_field_get(unit, &reg_value,
            VLAN_LEARN_MODEf, &temp);
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
    _drv_arl_hash(hash_value, 47, &hash_result);

    /* Recover the MAC bit 0~11 */
    temp = *key;
    temp = temp >> 2; 
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
            (uint32 *)sw_arl, (uint32 *)&temp_mac_field);
    if (temp_mac_addr[0] & 0x01) { /* multicast entry */
        /* Multicast index */
       mcast_index = 0;
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)hw_arl, &temp);
        mcast_index = temp;
         (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_PRIORITY, 
                (uint32 *)hw_arl, &temp);
        mcast_index += (temp << 6);
         (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)hw_arl, &temp);
        mcast_index += (temp << 11);

        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
            (uint32 *)sw_arl, &mcast_index);
    } else { /* unicast entry */
        /* Source port number */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)hw_arl, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, (uint32 *)sw_arl, &temp);

        /* Age bit */
        (DRV_SERVICES(unit)->mem_field_get)(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)hw_arl, &temp);
        (DRV_SERVICES(unit)->mem_field_set)(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, (uint32 *)sw_arl, &temp);
    }

    /* Static bit */
    (DRV_SERVICES(unit)->mem_field_get)(
        unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_STATIC, 
           (uint32 *)hw_arl, &temp);
    (DRV_SERVICES(unit)->mem_field_set)(
        unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, (uint32 *)sw_arl, &temp);

    return rv;
}

int
drv_arl_table_process(int unit, uint32 *key, void *hw_arl, void  *sw_arl)
{
    int rv = SOC_E_NONE;

    if (SOC_IS_ROBO5324(unit)) {
        rv = _drv_bcm5324_arl_hw_to_sw_entry
            (unit, key, (l2_arl_entry_t * )hw_arl, (l2_arl_sw_entry_t *) sw_arl);
    }
    
    if (SOC_IS_ROBO5396(unit)) {
        rv = _drv_bcm5396_arl_hw_to_sw_entry
            (unit, key, (l2_arl_entry_t * )hw_arl, (l2_arl_sw_entry_t *) sw_arl);
    }

    if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {
        rv = _drv_bcm5398_arl_hw_to_sw_entry
            (unit, key, (l2_arl_entry_t * )hw_arl, (l2_arl_sw_entry_t *) sw_arl);
    }

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        rv = _drv_bcm53242_arl_hw_to_sw_entry
            (unit, key, (l2_arl_entry_t * )hw_arl, (l2_arl_sw_entry_t *) sw_arl);
    }

    if (SOC_IS_ROBO5348(unit) ||SOC_IS_ROBO5347(unit)) {
        /* Temporary unavailable since it's unable to get ARL by memory address now */
        rv = SOC_E_UNAVAIL;
    }

    if (SOC_IS_TBX(unit)) {
        rv = _drv_bcm53280_arl_hw_to_sw_entry(unit, key, 
                (l2_arl_entry_t * )hw_arl, (l2_arl_sw_entry_t *) sw_arl);
    }
    return rv;
}

/* arlsync used compare information :
 *    - log the valid and age field inforamtion on this chip.
 */
typedef struct soc_arlsync_cmp_info_s{
    int     init;           /* TRUE/FALSE, default is FALSE */
    int     word_reversed;  /* l2 entry work order is revrsed or not */
    uint32  valid32_mask;   /* uint32 mask */
    uint16  valid_bp;       /* bit position */
    uint16  valid_cross_boundary;   /* cross the uint32 boundary */
    uint32  age32_mask;     /* uint32 mask */
    uint16  age_bp;         /* bit position */
    uint16  age_cross_boundary;   /* cross the uint32 boundary */
} soc_arlsync_cmp_info_t;

static soc_arlsync_cmp_info_t 
            soc_arlsync_cmp_info[SOC_MAX_NUM_SWITCH_DEVICES];   

#define DRV_ARLCMP_INFO_UINT32_BOUNDARY_TEST(_bp, _len)   \
        ((((_bp) % 32 + (_len - 1)) < 32) ? FALSE : TRUE)
        
#define DRV_ARLSYNC_MASK32_BUILD(_cross_boundary, _start, _len, _mask)    \
    if (!(_cross_boundary)) {               \
        (_mask) = (uint32)-1;               \
        (_mask) &= -1 << ((_start) % 32);   \
        (_mask) &= (1 << (((_start) + (_len) - 1) % 32) << 1) - 1;  \
    } else {            \
        (_mask) = 0;    \
    }


/* arl compare result flags 
 * 
 * Note :
 *  1. DRV_ARLCMP_NODIFF : 
 *        - for both l2 entry are invalid or 
 *        - for both l2 entry are valid with no any data field changed.
 *  2. DRV_ARLCMP_NONE_VALID_AGE_CHANGED :
 *        - means change is observed on the field except 'valid' and 'age'
 */
typedef enum drv_arlcmp_result_flag_e {
    DRV_ARLCMP_NODIFF           = 0x0,
    DRV_ARLCMP_INSERT           = 0x1,
    DRV_ARLCMP_DELETE           = 0x2,
    DRV_ARLCMP_VALID_PENDING    = 0x4,
    DRV_ARLCMP_AGE_CHANGED      = 0x8,
    DRV_ARLCMP_NONE_VALID_AGE_CHANGED    = 0x10,
    DRV_ARLCMP_UNEXPECT         = 0x20
} drv_arlcmp_result_flag_t;

/* check if soc_arlsync_cmp_info is prepared :
 *  - check init statu to prevent multiple init been porceeded.
 *      
 */
#define    DRV_ARLSYNC_CMP_CHKINIT(unit)    \
        do {   \
            if (soc_arlsync_cmp_info[(unit)].init == FALSE){    \
                SOC_IF_ERROR_RETURN(drv_arlsync_cmp_info_build(unit));  \
            }   \
        } while(0);

/* macro definition for arlsync compare */
#define DRV_ARLCMP_VALID_CHANGE_MASK   \
        (DRV_ARLCMP_INSERT | DRV_ARLCMP_DELETE | DRV_ARLCMP_VALID_PENDING)



/* SOC driver to build the valid and age field information in l2 entry */
int 
drv_arlsync_cmp_info_build(int unit)
{
    soc_field_info_t    *fieldp;
    soc_mem_info_t      *memp;
    uint32              tmp, valid_fld_id = 0, age_fld_id = 0;
    int                 cross_boundary;
    
    /* early return if init is proceeded already */
    if (soc_arlsync_cmp_info[unit].init == TRUE){
        return SOC_E_NONE;
    }

    /* all ROBO chips use the same table name of SW ARL table */
    memp = &SOC_MEM_INFO(unit, INDEX(L2_ARL_SWm));

    /* assigning the valid and age filed name for most of ROBO chips first */
    valid_fld_id = INDEX(VALID_Rf);
    age_fld_id = INDEX(AGEf);
    
    if (SOC_IS_TBX(unit)){
        valid_fld_id = INDEX(VAf);
    } else if (SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53101(unit) ||
        SOC_IS_ROBO53128(unit)){
        valid_fld_id = INDEX(VALIDf);
    }

    /* word ordering effort required per arl entry format :
     *  - TB's ARL entry word-ordering is different with other 
     *      ROBO chips.
     */
    if (SOC_IS_TBX(unit)){
        soc_arlsync_cmp_info[unit].word_reversed = FALSE;
    } else {
        soc_arlsync_cmp_info[unit].word_reversed = TRUE;
    }
    
    /* assign the valid field info */
    SOC_FIND_FIELD(valid_fld_id, memp->fields, memp->nFields, fieldp);
    assert(fieldp);
    soc_arlsync_cmp_info[unit].valid_bp = fieldp->bp; 
    cross_boundary = 
            DRV_ARLCMP_INFO_UINT32_BOUNDARY_TEST(fieldp->bp, fieldp->len);
    soc_arlsync_cmp_info[unit].valid_cross_boundary = cross_boundary;
    DRV_ARLSYNC_MASK32_BUILD(cross_boundary, fieldp->bp, fieldp->len, tmp);
    soc_arlsync_cmp_info[unit].valid32_mask = tmp;
            

    /* assign the age field info */
    SOC_FIND_FIELD(age_fld_id, memp->fields, memp->nFields, fieldp);
    assert(fieldp);
    soc_arlsync_cmp_info[unit].age_bp = fieldp->bp; 
    cross_boundary = 
            DRV_ARLCMP_INFO_UINT32_BOUNDARY_TEST(fieldp->bp, fieldp->len);
    soc_arlsync_cmp_info[unit].age_cross_boundary = cross_boundary;
    DRV_ARLSYNC_MASK32_BUILD(cross_boundary, fieldp->bp, fieldp->len, tmp);
    soc_arlsync_cmp_info[unit].age32_mask = tmp;

    soc_arlsync_cmp_info[unit].init = TRUE;

    /* varified! no error! */
    soc_cm_debug(DK_ARL + DK_VERBOSE,
            "%s, SW_ARLSYNC_COMPARE_INFO....init=%d\n"
            "  reversed=%d,va_mask=0x%08x,va_bp=%d,va_bound=%d,"
            "  age_mask=0x%08x,age_bp=%d,age_bound=%d\n",
            FUNCTION_NAME(),soc_arlsync_cmp_info[unit].init, 
            soc_arlsync_cmp_info[unit].word_reversed,
            soc_arlsync_cmp_info[unit].valid32_mask,
            soc_arlsync_cmp_info[unit].valid_bp, 
            soc_arlsync_cmp_info[unit].valid_cross_boundary,
            soc_arlsync_cmp_info[unit].age32_mask, 
            soc_arlsync_cmp_info[unit].age_bp, 
            soc_arlsync_cmp_info[unit].age_cross_boundary);
    
    return SOC_E_NONE;
}

/* macro definitions for helping the readable and maintainance of code */
#define _DRV_ARLCMP_ALL_INVALID(_old_va, _new_va)   \
            (!(_old_va) && !(_new_va))

/* old is invalid and new is valid or pending */
#define _DRV_ARLCMP_IS_INSERT(_old_va, _new_va)     \
            (!(_old_va) && (_new_va))

/* old is valid or pending and new is invalid */
#define _DRV_ARLCMP_IS_DELETE(_old_va, _new_va)     \
            ((_old_va) && !(_new_va))

/* old_valid and new_valid both TRUE, check pending status */
#define _DRV_ARLCMP_ALL_VALID(_old_pen, _new_pen)   \
            (!(_old_pen) && !(_new_pen))
#define _DRV_ARLCMP_ALL_PENDING(_old_pen, _new_pen) \
            ((_old_pen) && (_new_pen))
        
/* performing the high performance processing to compare two arl entries */
static int 
drv_arlsync_compare(int unit, 
        uint32 *old_sw_arl, uint32 *new_sw_arl, 
        drv_arlcmp_result_flag_t *result_flags)
{
    uint32  old_valid = 0, old_age = 0, old_pending = 0;
    uint32  new_valid = 0, new_age = 0, new_pending = 0;
    uint32  temp_bp, temp_mask, temp32, temp_valid;
    int     i, l2_word_size, l2_size;
    l2_arl_sw_entry_t   old_l2, new_l2;

    assert(old_sw_arl && new_sw_arl);   /* NULL check */

    if (soc_arlsync_cmp_info[unit].init != TRUE){
        soc_cm_debug(DK_WARN, 
                "%s, Unexpected condition during ARL thread started!\n",
                FUNCTION_NAME());
        *result_flags = DRV_ARLCMP_NODIFF;
        return SOC_E_INTERNAL;
    }

    l2_size = sizeof(l2_arl_sw_entry_t);
    /* coverity[dead_error_condition : FALSE] */
    l2_word_size = (l2_size / 4) + ((l2_size % 4) ? 1 : 0);

    sal_memcpy(&old_l2, old_sw_arl, l2_size);
    sal_memcpy(&new_l2, new_sw_arl, l2_size);

    /* 1. retrieve the valid status first */
    if (!soc_arlsync_cmp_info[unit].valid_cross_boundary){
        temp_bp = soc_arlsync_cmp_info[unit].valid_bp;
        temp_mask = soc_arlsync_cmp_info[unit].valid32_mask;
        /* get the uint32 data to retrieve old valid */
        if (soc_arlsync_cmp_info[unit].word_reversed){
            temp32 = *(((uint32 *)&old_l2) + 
                    ((l2_word_size - 1) - (temp_bp / 32)));
        } else {
            temp32 = *(((uint32 *)&old_l2) + (temp_bp / 32));
        }
        
        temp_valid = (temp32 & temp_mask) >> (temp_bp % 32);
        /* valid/pending both treate as valid */
        old_valid = (temp_valid) ? TRUE : FALSE;
        old_pending = FALSE;
        if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
            if (old_valid) {
                if (temp_valid == _TB_ARL_STATUS_PENDING){
                    old_pending = TRUE;
                } else if (temp_valid != _TB_ARL_STATUS_VALID){
                    /* unexpected value */
                    soc_cm_debug(DK_WARN, 
                            "%s, Unexpected value on valid field!\n",
                            FUNCTION_NAME());
                }
            }
#endif  /* BCM_TB_SUPPORT */
        }
        
        /* get the uint32 data to retrieve new valid */
        if (soc_arlsync_cmp_info[unit].word_reversed){
            temp32 = *(((uint32 *)&new_l2) + 
                    ((l2_word_size - 1) - (temp_bp / 32)));
        } else {
            temp32 = *(((uint32 *)&new_l2) + (temp_bp / 32));
        }
        temp_valid = (temp32 & temp_mask) >> (temp_bp % 32);

        /* valid/pending both treate as valid */
        new_valid = (temp_valid) ? TRUE : FALSE;
        new_pending = FALSE;
        if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
            if (new_valid) {
                if (temp_valid == _TB_ARL_STATUS_PENDING){
                    new_pending = TRUE;
                } else if (temp_valid != _TB_ARL_STATUS_VALID){
                    /* unexpected value */
                    soc_cm_debug(DK_WARN, 
                            "%s, Unexpected value on valid field!\n",
                            FUNCTION_NAME());
                }
            }
#endif  /* BCM_TB_SUPPORT */
        }

        /* early return for valid changed 
         *  - insert / delete cases
         *  - changed between valid and pending cases
         */
        if (_DRV_ARLCMP_ALL_INVALID(old_valid, new_valid)){
            /* both invalid, no more compare required */
            *result_flags = DRV_ARLCMP_NODIFF;
            goto arlcmp_finished;
        } else if (_DRV_ARLCMP_IS_INSERT(old_valid, new_valid)){
            /* insert action, no more compare required */
            *result_flags |= DRV_ARLCMP_INSERT;
            goto arlcmp_finished;
        } else if (_DRV_ARLCMP_IS_DELETE(old_valid, new_valid)){
            /* delete action, no more compare required */
            *result_flags |= DRV_ARLCMP_DELETE;
            goto arlcmp_finished;
        } else {
            /* both at valid or pending status,
             *  - assume old_valid == new_valid == TRUE
             *  - check pending status is enough in this section.
             */
            if (_DRV_ARLCMP_ALL_VALID(old_pending, new_pending) || 
                    _DRV_ARLCMP_ALL_PENDING(old_pending, new_pending)){
                /* need more compare later*/
                *result_flags = DRV_ARLCMP_NODIFF;
            } else {
                /* one is valid and the other is pending. */
                /* valid/pending changed, no more compare required */
                *result_flags |= DRV_ARLCMP_VALID_PENDING;
                goto arlcmp_finished;
            }
        }
    } else {
        /* valid_cross_boundary : ROBO chips has no such condition so far */
        *result_flags = DRV_ARLCMP_NODIFF;
        soc_cm_debug(DK_WARN, 
                "%s, Unexpected process for uint32 boundary issue!\n",
                FUNCTION_NAME());
        return SOC_E_INTERNAL;
    }
    
    /* 2. retrieve the age status */
    if (!soc_arlsync_cmp_info[unit].age_cross_boundary){
        temp_bp = soc_arlsync_cmp_info[unit].age_bp;
        temp_mask = soc_arlsync_cmp_info[unit].age32_mask;
        /* get the uint32 data to retrieve old age */
        if (soc_arlsync_cmp_info[unit].word_reversed){
            temp32 = *(((uint32 *)&old_l2) + 
                    ((l2_word_size - 1) - (temp_bp / 32)));
            old_age = (temp32 & temp_mask) >> (temp_bp % 32);
            /* mask-out age field */
            temp32 &= (uint32)(~temp_mask);
            *(((uint32 *)&old_l2) + ((l2_word_size - 1) - (temp_bp / 32))) = temp32;
        } else {
            temp32 = *(((uint32 *)&old_l2) + (temp_bp / 32));
            old_age = (temp32 & temp_mask) >> (temp_bp % 32);
            /* mask-out age field */
            temp32 &= (uint32)(~temp_mask);
            *(((uint32 *)&old_l2) + (temp_bp / 32)) = temp32;
        }
        
        
        /* get the uint32 data to retrieve new age */
        if (soc_arlsync_cmp_info[unit].word_reversed){
            temp32 = *(((uint32 *)&new_l2) + 
                    ((l2_word_size - 1) - (temp_bp / 32)));
            new_age = (temp32 & temp_mask) >> (temp_bp % 32);
            /* mask-out age field */
            temp32 &= (uint32)(~temp_mask);
            *(((uint32 *)&new_l2) + ((l2_word_size - 1) - (temp_bp / 32))) = temp32;
        } else {
            temp32 = *(((uint32 *)&new_l2) + (temp_bp / 32));
            new_age = (temp32 & temp_mask) >> (temp_bp % 32);
            /* mask-out age field */
            temp32 &= (uint32)(~temp_mask);
            *(((uint32 *)&new_l2) + (temp_bp / 32)) = temp32;
        }

        if (old_age != new_age){
            *result_flags |= DRV_ARLCMP_AGE_CHANGED;
        }
    } else {
        /* age_cross_boundary : ROBO chips has no such condition so far */
        *result_flags = DRV_ARLCMP_UNEXPECT;
        soc_cm_debug(DK_WARN, 
                "%s, Unexpected process for uint32 boundary issue!\n",
                FUNCTION_NAME());
        return SOC_E_INTERNAL;
    }
    
    /* 3. ARL entry compare 
     *  - assumed the old and new l2 entry both at valid or pending status  
     *      and the age field were both mask-out already.
     */
    for (i = 0; i < (l2_size / 4); i++){
        temp32 = *((uint32 *)&old_l2 + i) ^ *((uint32 *)&new_l2 + i);
        if (temp32){
            /* means difference observed */
            *result_flags |= DRV_ARLCMP_NONE_VALID_AGE_CHANGED;
            break;
        }
    }

arlcmp_finished:
    
    return SOC_E_NONE;
}

/*  check the valid status on a given SW l2 entry.
 *  
 */
void
_drv_arl_swl2_valid_get(int unit, l2_arl_sw_entry_t *sw_l2, int *valid)
{
    uint32  temp_bp, temp_mask, temp32, temp_valid;
    int     ent_word_size, ent_size;

    *valid = 0;
    if (sw_l2 == NULL){
        return;
    }

    ent_size = sizeof(l2_arl_sw_entry_t);
    /* coverity[dead_error_condition : FALSE] */
    ent_word_size = (ent_size / 4) + ((ent_size % 4) ? 1 : 0);
    if (!soc_arlsync_cmp_info[unit].valid_cross_boundary){
        temp_bp = soc_arlsync_cmp_info[unit].valid_bp;
        temp_mask = soc_arlsync_cmp_info[unit].valid32_mask;
        /* get the uint32 data to retrieve old valid */
        if (soc_arlsync_cmp_info[unit].word_reversed){
            temp32 = *(((uint32 *)sw_l2) + 
                    ((ent_word_size - 1) - (temp_bp / 32)));
        } else {
            temp32 = *(((uint32 *)sw_l2) + (temp_bp / 32));
        }
        temp_valid = (temp32 & temp_mask) >> (temp_bp % 32);

        if (SOC_IS_TBX(unit)){
            /* pending(b01) is treat as valid(b11) as well */
            *valid = (temp_valid) ? 1: 0;
        } else {
            /* other chips is valid bit */
            *valid = (temp_valid) ? 1: 0;
        }
    }
}

#define _DRV_HASH_ID_2_L2_ID(_h_id, _bin_cnt, _bin) \
        (((_h_id) * (_bin_cnt)) | ((_bin) % (_bin_cnt)))
#define _DRV_L2_ID_2_HASH_ID(_l2_id, _bin_cnt)  ((_l2_id) / (_bin_cnt))
#define _DRV_L2_ID_2_BIN_ID(_l2_id, _bin_cnt)  ((_l2_id) % (_bin_cnt))

#ifdef  BCM_TB_SUPPORT
#define _DRV_TB_IS_HASH32_BIN(_bin)  ((_bin) > 1)
#endif  /* BCM_TB_SUPPORT */

/* Sanity process when add a new l2 entry into SW shadow table :
 *  
 *  1. Remove the duplicated L2 entry which is existed in SW ARL already but 
 *      placed in another l2 index due to hash conflict.
 *  2. Need call back for such removment.
 *  3. return SOC_E_EXIST if found the redundant l2 entry.
 *  
 */
static int 
_drv_arl_add_sanity_process(int unit, uint32 *key, void *new_arl)
{
    int     hash_index = -1, this_bin_id = -1, bin_cnt = 0;
    int     l2_idx = -1, valid = 0, i;
    uint32  vid = 0, chk_vid = 0;
    uint64  mac_field, chk_mac_field;
    l2_arl_sw_entry_t   *sw_l2_entry;
    soc_control_t       *soc = SOC_CONTROL(unit);
#ifdef  BCM_TB_SUPPORT
    int     hash_is_crc16 = 1, hash2_index = -1;
#endif  /* BCM_TB_SUPPORT */

    COMPILER_64_ZERO(mac_field);
    COMPILER_64_ZERO(chk_mac_field);

    if (SOC_IS_TBX(unit)){   /* 16K dual hash 4bin (each hash 2 bin) */
        /* dual hash */
        bin_cnt = 4;
    } else if (SOC_IS_ROBO5324(unit)){ 
        /* 4K 2bin */
        bin_cnt = 2;
    } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
            SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit)) {
        /* 1bin */
        /* Not suitable to apply */
        return SOC_E_NONE;
    } else if (SOC_IS_ROBODINO(unit)) {
        /* 2bin */
        bin_cnt = 2;
    } else if (SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53115(unit) ||
            SOC_IS_ROBO53118(unit) ||SOC_IS_ROBO53125(unit) ||
            SOC_IS_ROBO53128(unit)) {
        /* 1K 4bin */
        bin_cnt = 4;
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        /* 4K 2bin */
        bin_cnt = 2;
    } else if (SOC_IS_ROBO53101(unit)) {
        /* 512 4bin */
        bin_cnt = 4;
    } else {
        soc_cm_debug(DK_WARN, 
                "_drv_arl_del_reduncant_l2ent(), chip is not handled!\n");
        return SOC_E_INTERNAL;
    }

    /* remove the redundant MAC+VID from other hashed bin */
    hash_index = _DRV_L2_ID_2_HASH_ID(*key, bin_cnt);
    this_bin_id = _DRV_L2_ID_2_BIN_ID(*key, bin_cnt);
#ifdef  BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        hash_is_crc16 = (_DRV_TB_IS_HASH32_BIN(this_bin_id)) ? 0 : 1;
    }
#endif  /* BCM_TB_SUPPORT */

    /* retrieve MAC+VID */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC,
            (uint32 *)new_arl, (uint32 *)&mac_field);
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID,
            (uint32 *)new_arl, (uint32 *)&vid);

#ifdef  BCM_TB_SUPPORT
    /* for dual hash device, retrieve the other hash index */
    if (SOC_IS_TBX(unit)){
        int rv = SOC_E_NONE;

        rv = _drv_tb_arl_hash_index_get(unit, 
                !hash_is_crc16, (uint32 *)new_arl, &hash2_index);
        SOC_IF_ERROR_RETURN(rv);
    }
#endif  /* BCM_TB_SUPPORT */

    /* check the bins in the same hash index */
    for (i = 0; i < bin_cnt; i++){
        if (i == this_bin_id){
            continue;
        }
        
        /* assign the l2 index from hash_id */
        l2_idx = _DRV_HASH_ID_2_L2_ID(hash_index, bin_cnt, i);
#ifdef  BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)){
            /* special for TB's dual hash once the checking bin is at the 
             * other hash bank.
             */
            if ((hash_is_crc16 && _DRV_TB_IS_HASH32_BIN(i)) || 
                    (!hash_is_crc16 && !_DRV_TB_IS_HASH32_BIN(i))) {
                l2_idx = _DRV_HASH_ID_2_L2_ID(hash2_index, bin_cnt, i);
            }
        }
#endif  /* BCM_TB_SUPPORT */

        sw_l2_entry = (l2_arl_sw_entry_t *)(soc->arl_table) + l2_idx;
        _drv_arl_swl2_valid_get(unit, sw_l2_entry, &valid);

        if (valid){
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC,
                    (uint32 *)sw_l2_entry, (uint32 *)&chk_mac_field);
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID,
                    (uint32 *)sw_l2_entry, (uint32 *)&chk_vid);
            
            if (COMPILER_64_EQ(mac_field, chk_mac_field) && vid == chk_vid) {

                /* issue the delete callback first */
                soc_robo_arl_callback(unit, sw_l2_entry, NULL);

                /* remove this redundant l2 entry from sw arl */
                sal_memset(sw_l2_entry, 0, sizeof(l2_arl_sw_entry_t));
                soc_cm_debug(DK_ARLMON, 
                        "L2 entry is removed(id=%d) before L2 ADD(id=%d)\n", 
                        *key, l2_idx);

                *key = l2_idx;
                return SOC_E_EXISTS;
            }
        }
    }

    return SOC_E_NONE;
}

/* SW ARL Callback type definitions :
 *
 *  1. _DRV_SWARL_NO_CB : indicated no callback request.
 *  2. _DRV_SWARL_INSERT_CB : callback for insert action
 *  3. _DRV_SWARL_DELETE_CB : callback for delete action
 *  4. _DRV_SWARL_MODIFY_CB : callback for modify action
 */
#define _DRV_SWARL_NO_CB        0
#define _DRV_SWARL_MODIFY_CB    1
#define _DRV_SWARL_DELETE_CB    2
#define _DRV_SWARL_INSERT_CB    3

/* SW_ARL_UPDATE_HIT force all SW ARL containing no AGE or HIT information.
 *  - This information can speed the arl_sync. process to aviod the case when 
 *      the old and new arl entry is different at age field only.
 *  - If this symbol is defined as FALSE, all sync l2 entry will be kept as 
 *      the first sync age field value.
 */
#define SW_ARL_UPDATE_HIT   FALSE

/*
 * Function:
 *  drv_arl_sync (internal)
 * Purpose:
 *  Compare old ARL contents to new and synchronize shadow table.
 * Parameters:
 *  unit    - RoboSwitch unit #
 *  old_arl - Results of previous ARL entry
 *  new_arl - Results of current ARL entry
 */
int
drv_arl_sync(int unit, uint32 *key, void *old_arl, void *new_arl)
{
    int     rv = SOC_E_NONE;
    uint32  cb_level = _DRV_SWARL_NO_CB;
    sal_usecs_t     stime, etime;
    soc_control_t   *soc = SOC_CONTROL(unit);
    drv_arlcmp_result_flag_t result_flags = 0;
    l2_arl_sw_entry_t   *old, old_arl_temp;

    int     del_hash_duplicate = 0;
    uint32  temp_key = 0;

    soc_cm_debug(DK_ARL+DK_VERBOSE, "soc_arl_sync: start\n");

    ARL_SW_TABLE_LOCK(soc);
    old = (l2_arl_sw_entry_t *)old_arl;
    sal_memcpy(&old_arl_temp, old, sizeof(l2_arl_sw_entry_t));

    stime = sal_time_usecs();
    
    rv = drv_arlsync_compare(unit, 
            (uint32 *)old_arl, (uint32 *)new_arl,  &result_flags);
    if (rv != SOC_E_NONE){
        ARL_SW_TABLE_UNLOCK(soc);
        soc_cm_debug(DK_WARN, 
                "soc_arl_sync: unexpect error! (rv=%d)\n", rv);
        return SOC_E_INTERNAL;
    }

    if (result_flags != DRV_ARLCMP_NODIFF){
        if (result_flags & DRV_ARLCMP_VALID_CHANGE_MASK){
            if (result_flags == DRV_ARLCMP_INSERT){
                cb_level = _DRV_SWARL_INSERT_CB;

                del_hash_duplicate = 1;
            } else if (result_flags == DRV_ARLCMP_DELETE){
                cb_level = _DRV_SWARL_DELETE_CB;
            } else if (result_flags == DRV_ARLCMP_VALID_PENDING){
                cb_level = _DRV_SWARL_MODIFY_CB;
            } else {
                soc_cm_debug(DK_WARN, 
                        "%s, unexpected compare result(flags=0x%x)\n",
                        FUNCTION_NAME(), result_flags);
                cb_level = _DRV_SWARL_MODIFY_CB;
            }
        } else if (result_flags & DRV_ARLCMP_NONE_VALID_AGE_CHANGED){
            cb_level = _DRV_SWARL_MODIFY_CB;

            /* MAC+VID change will be marked as modify here */
             del_hash_duplicate = 1;
        } else {
            if (result_flags & DRV_ARLCMP_AGE_CHANGED){
                /* the age difference will be treated as no change. */
                cb_level = _DRV_SWARL_NO_CB;
            } else if (result_flags & DRV_ARLCMP_UNEXPECT){
                /* possible condition is DRV_ARLCMP_UNEXPECT */ 
                soc_cm_debug(DK_WARN, 
                        "%s, unexpected compare result(flags=0x%x)\n",
                        FUNCTION_NAME(), result_flags);
                cb_level = _DRV_SWARL_MODIFY_CB;
            }
        }

        /* for SDK-29690 : Incorrect L2 callback occured when many l2 entries
         *      burst in on the fly during HW L2 fast aging.
         *
         * - Remove the redundant l2 entry with the same MAC+VID in other 
         *    SW ARL address and a callback will be issued for this action.
         */

        if (del_hash_duplicate) {
            temp_key = *key;
            rv = _drv_arl_add_sanity_process(unit, &temp_key, new_arl);
            if (rv == SOC_E_EXISTS){
                soc_cm_debug(DK_ARLMON, 
                    "SW arl at id=%d is removed to prevent duplicat ADD\n",
                    temp_key);
                rv = SOC_E_NONE;
            }
        }

        /* SW arl sync :
         *  - Do sync process even only DRV_ARLCMP_AGE_CHANGED flags set.
         */
        sal_memcpy(old_arl, new_arl, sizeof(l2_arl_sw_entry_t));

    } else {
        cb_level = _DRV_SWARL_NO_CB;
    }
    ARL_SW_TABLE_UNLOCK(soc);

    etime = sal_time_usecs();

    if (cb_level == _DRV_SWARL_MODIFY_CB) {
        soc_robo_arl_callback(unit, &old_arl_temp,
                (l2_arl_sw_entry_t *)new_arl);
    } else if (cb_level == _DRV_SWARL_DELETE_CB) {
        soc_robo_arl_callback(unit, &old_arl_temp, NULL);
    } else if (cb_level == _DRV_SWARL_INSERT_CB) {
        soc_robo_arl_callback(unit, NULL, (l2_arl_sw_entry_t *)new_arl);
    } else {
        /* no callback */
    }
    
    /* use DK_ARLMON to avoid any debug messge for DK_ARL */
    if (cb_level != _DRV_SWARL_NO_CB){
        soc_cm_debug(DK_ARLMON,
                "soc_arl_sync: done in %d usec (cb=%d)\n",
                SAL_USECS_SUB(etime, stime), cb_level);
    }
    return rv;

}

#define _TB_LEARN_FREEZE    1
#define _TB_LEARN_THAW      2

#define _TB_PORTMASK_TABLE_ID           0x52
#define _TB_MEM_OP_READ                 1
#define _TB_MEM_OP_WRITE                2

#define _TB_MEM_DATA_2_SHIFT                128     /* 64(bits)*2 */
#define _TB_PORTMASK_MEM_SA_LRN_SHIFT       151
#define _TB_PORTMASK_MEM_LRN_CTRL_SHIFT     152

static int
_soc_tb_arl_fast_learn_control(int unit, int op)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    freeze_t        *f = &arl_freeze_state[unit];
    uint32          temp_cml = 0, sw_lrn = 0, sa_lrn_dis = 0;
    uint32          reg32_val = 0, temp = 0, test_bit = 0;
    uint64          reg64_val, temp64_config;
    soc_port_t      port;
    int             rv = SOC_E_NONE, retry;

    COMPILER_64_ZERO(reg64_val);
    COMPILER_64_ZERO(temp64_config);

    if (!((op == _TB_LEARN_FREEZE) || (op == _TB_LEARN_THAW))){
        return SOC_E_PARAM;
    }
    
    MEM_RWCTRL_REG_LOCK(soc);
    
    /* processes on PORTMASKm on each port */
    reg32_val = _TB_PORTMASK_TABLE_ID;
    rv = REG_WRITE_MEM_INDEXr(unit, &reg32_val);
    if (rv != SOC_E_NONE){
        goto failed_unlock;
    }
    
    PBMP_E_ITER(unit, port) {
        reg32_val = (uint32)port;
        rv = REG_WRITE_MEM_ADDR_0r(unit, &reg32_val);
        if (rv != SOC_E_NONE){
            goto failed_unlock;
        }

        /* performing READ process */
        reg32_val = 0;
        temp = _TB_MEM_OP_READ;
        soc_MEM_CTRLr_field_set(unit, &reg32_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg32_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg32_val);
        if (rv != SOC_E_NONE){
            goto failed_unlock;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg32_val);
            if (rv < 0) {
                goto failed_unlock;
            }
            soc_MEM_CTRLr_field_get(unit, &reg32_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto failed_unlock;
        }

        /* read the learning configuration related data buffer only for 
         * performance issue.
         */
        rv = REG_READ_MEM_DATA_2r(unit, (uint32 *)&reg64_val);
        if (rv < 0) {
            goto failed_unlock;
        }

        if (op == _TB_LEARN_FREEZE){
            /* get learning related configurations */
            sal_memcpy(&temp64_config, &reg64_val, sizeof(uint64));
            test_bit = _TB_PORTMASK_MEM_SA_LRN_SHIFT - _TB_MEM_DATA_2_SHIFT;
            sa_lrn_dis = COMPILER_64_BITTEST(temp64_config, test_bit);
            test_bit = _TB_PORTMASK_MEM_LRN_CTRL_SHIFT - _TB_MEM_DATA_2_SHIFT;
            sw_lrn = COMPILER_64_BITTEST(temp64_config, test_bit);

            /* save learning configurations */
            temp_cml = (sa_lrn_dis) ? DRV_PORT_DISABLE_LEARN : 
                        ((sw_lrn) ? DRV_PORT_SW_LEARN : DRV_PORT_HW_LEARN);
            f->save_cml[port] = temp_cml;
        
            /* disable TB's learning configuration :
             *  - To freeze ARL learning on TB, set SA_LRN_DISABLE(bit 151)
             *      is enough.
             *  - considering the performance request. If this port is already
             *      disable SA learning, skip mem_write process on this port.
             *
             *  1. write to data_2 buffer
             *  2. entry write.
             */
            if (temp_cml == DRV_PORT_DISABLE_LEARN) {
                soc_cm_debug(DK_ARL, 
                        "port%d no freeze for SA_LEARN disabled already./n",
                        port);
                continue;
            }
            sa_lrn_dis = 1;
            COMPILER_64_SET(temp64_config, 0, sa_lrn_dis);
            temp = _TB_PORTMASK_MEM_SA_LRN_SHIFT - _TB_MEM_DATA_2_SHIFT;
            COMPILER_64_SHL(temp64_config, temp);
            
            COMPILER_64_OR(reg64_val, temp64_config);
        } else {
            
            /* restore original learning configuration for THAW 
             *  - considering the performance request. If this port is already
             *      disable SA learning, skip mem_write process on this port.
             */
            temp_cml = f->save_cml[port];
            if (temp_cml == DRV_PORT_DISABLE_LEARN){
                soc_cm_debug(DK_ARL, 
                        "port%d no thaw for SA_LEARN disabled originally.\n",
                        port);
                continue;
            }

            sa_lrn_dis = 1;     /* for NOT operation */
            COMPILER_64_SET(temp64_config, 0, sa_lrn_dis);
            temp = _TB_PORTMASK_MEM_SA_LRN_SHIFT - _TB_MEM_DATA_2_SHIFT;
            COMPILER_64_SHL(temp64_config, temp);
            COMPILER_64_NOT(temp64_config);
            
            COMPILER_64_AND(reg64_val, temp64_config);
        }
        
        /* performing WRITE process */
        rv = REG_WRITE_MEM_DATA_2r(unit, (uint32 *)&reg64_val);
        if (rv < 0) {
            goto failed_unlock;
        }

        reg32_val = 0;
        temp = _TB_MEM_OP_WRITE;
        soc_MEM_CTRLr_field_set(unit, &reg32_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg32_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg32_val);
        if (rv != SOC_E_NONE){
            goto failed_unlock;
        }
        
        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg32_val);
            if (rv < 0) {
                goto failed_unlock;
            }
            soc_MEM_CTRLr_field_get(unit, &reg32_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto failed_unlock;
        }                
    }

    MEM_RWCTRL_REG_UNLOCK(soc);
    return SOC_E_NONE;

failed_unlock:
    soc_cm_debug(DK_WARN, "TB's ARL can't be %s! (rv=%d)\n",
            (op == _TB_LEARN_FREEZE) ? "frozen" : "thawed", rv);
    MEM_RWCTRL_REG_UNLOCK(soc);
    return rv;

}

/*
 * Function:
 *      soc_robo_arl_is_frozen
 * Purpose:
 *   	Provides indication if ARL table is in frozen state
 * Parameters:
 *      unit    - (IN) BCM device number. 
 *      frozen  - (OUT) TRUE if ARL table is frozen, FLASE otherwise 
 * Returns:
 *      SOC_E_NONE
 */

int
soc_robo_arl_is_frozen(int unit, int *frozen)
{
   freeze_t     *f = &arl_freeze_state[unit];

   *frozen = (f->frozen > 0) ? TRUE : FALSE ;

   return (SOC_E_NONE);
}

/*
 * Function:
 *  soc_robo_arl_freeze
 * Purpose:
 *  Temporarily quiesce ARL from all activity (learning, aging)
 * Parameters:
 *  unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *  SOC_E_NONE      Success
 *  SOC_E_XXX       Error (tables not locked)
 * Notes:
 *  Leaves ARLm locked until corresponding thaw.
 *  PTABLE is locked in order to lockout bcm_port calls
 *  bcm_port calls will callout to soc_arl_frozen_cml_set/get
 */

int
soc_robo_arl_freeze(int unit)
{
    freeze_t        *f = &arl_freeze_state[unit];
    soc_port_t      port;
    soc_pbmp_t       pbmp;
    int         rv;
    uint32 val, dis_learn;

    /*
     * First time through: lock arl, disable learning and aging
     */
    soc_mem_lock(unit, L2_ARLm);
    if (f->frozen++) {
        soc_mem_unlock(unit, L2_ARLm);
        return SOC_E_NONE;      /* Recursive freeze OK */
    }

    if (SOC_IS_TBX(unit)){
        if(SOC_IS_TB_AX(unit)){
            rv = _soc_tb_arl_fast_learn_control(unit, _TB_LEARN_FREEZE);
            if (rv < 0) {
                goto fail;
            }
        } else {
            rv = REG_READ_GARLCFGr(unit, &val);
            if (SOC_FAILURE(rv)) {
                goto fail;
            }
            dis_learn = 1;
            soc_GARLCFGr_field_set(unit, &val, GLB_SA_LRN_CTRLf, &dis_learn);
            rv = REG_WRITE_GARLCFGr(unit, &val);
            if (SOC_FAILURE(rv)) {
                goto fail;
            }
        }        
    } else {
        /* disable learning */
        PBMP_E_ITER(unit, port) {
            rv = (DRV_SERVICES(unit)->arl_learn_enable_get)(unit, 
                    port, (uint32 *) &f->save_cml[port]);
            if (rv < 0) {
                goto fail;
            }
        }
        pbmp = PBMP_E_ALL(unit);
        
        rv = (DRV_SERVICES(unit)->arl_learn_enable_set)(unit,
                pbmp, DRV_PORT_DISABLE_LEARN);
        if (rv < 0) {
            goto fail;
        }
    }
        
    /* disable aging */
    rv = (DRV_SERVICES(unit)->age_timer_get)(unit,
                        (uint32 *) &f->save_age_ena,
                        (uint32 *) &f->save_age_sec);
    if (rv < 0) {
        goto fail;
    }
    if (f->save_age_ena) {
        rv = (DRV_SERVICES(unit)->age_timer_set)(unit,
                        0,
                        f->save_age_sec);
    if (rv < 0) {
            goto fail;
        }
    }

    return SOC_E_NONE;

 fail:
    f->frozen--;
    soc_mem_unlock(unit, L2_ARLm);
    return rv;
}

/*
 * Function:
 *  soc_arl_thaw
 * Purpose:
 *  Restore normal ARL activity.
 * Parameters:
 *  unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  Unlocks ARLm.
 */

int
soc_robo_arl_thaw(int unit)
{
    freeze_t        *f = &arl_freeze_state[unit];
    soc_port_t      port;
    int         rv,cml;
    soc_pbmp_t       pbm;
    uint32 dis_learn, val;

    assert(f->frozen > 0);

    if (--f->frozen) {
        return SOC_E_NONE;      /* Thaw a recursive freeze */
    }

    /*
     * Last thaw enables learning and aging, and unlocks arl
     */
    rv = SOC_E_NONE;

    if (SOC_IS_TBX(unit)){        
        if(SOC_IS_TB_AX(unit)){
            rv = _soc_tb_arl_fast_learn_control(unit, _TB_LEARN_THAW);
            if (rv < 0) {
                goto fail;
            }
        } else {
            rv = REG_READ_GARLCFGr(unit, &val);
            if (SOC_FAILURE(rv)) {
                goto fail;
            }
            dis_learn = 0;
            soc_GARLCFGr_field_set(unit, &val, GLB_SA_LRN_CTRLf, &dis_learn);
            rv = REG_WRITE_GARLCFGr(unit, &val);
            if (SOC_FAILURE(rv)) {
                goto fail;
            }
        }
    } else {
        PBMP_E_ITER(unit, port) {
            cml = f->save_cml[port];
            if (cml) {
                SOC_PBMP_CLEAR(pbm);
                SOC_PBMP_PORT_SET(pbm,port);
                rv = DRV_ARL_LEARN_ENABLE_SET(unit,
                    pbm, cml);
        
                if (rv < 0) {
                    goto fail;
                }
            }
        }
        
    }
    
    if (f->save_age_ena) {
        rv = DRV_AGE_TIMER_SET(unit,
                        f->save_age_ena,
                        f->save_age_sec);
        if (rv < 0) {
            goto fail;
        }
    }

 fail:
    soc_mem_unlock(unit, L2_ARLm);
    return rv;
}

/*
 * Function:
 *  drv_arl_learn_enable_set
 * Purpose:
 *  Setting per port SA learning process.
 * Parameters:
 *  unit    - RoboSwitch unit #
 *  pbmp    - port bitmap
 *  mode   - DRV_PORT_HW_LEARN
 *               DRV_PORT_DISABLE_LEARN
 *               DRV_PORT_SW_LEARN
 */
int
drv_arl_learn_enable_set(int unit, soc_pbmp_t pbmp, uint32 mode)
{
    int     rv = SOC_E_NONE;
    uint32  reg_v32, fld_v32 = 0;
    uint64  reg_v64, fld_v64;
    soc_pbmp_t current_pbmp, temp_pbmp;
    soc_port_t port;

    switch (mode ) {
    case DRV_PORT_HW_LEARN:
    case DRV_PORT_DISABLE_LEARN:
    /* bcm5324 can support HW_LEARN only 
     *  - return SOC_E_NONE to avoid the port info restoring failure.
     */
    if ((mode == DRV_PORT_HW_LEARN) && SOC_IS_ROBO5324(unit)){
        return SOC_E_NONE;
    }

    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        if ((rv = REG_READ_DIS_LEARNr(unit, (uint32 *)&reg_v64)) < 0) {
            return rv;
        }
    
        COMPILER_64_ZERO(fld_v64);
        if (SOC_IS_ROBO5348(unit)) {
            soc_DIS_LEARNr_field_get(unit, (uint32 *)&reg_v64,
                DIS_LEARNf, (uint32 *)&fld_v64);
            soc_robo_64_val_to_pbmp(unit, &current_pbmp, fld_v64);
        } else { /* 5347 */
            soc_DIS_LEARNr_field_get(unit, (uint32 *)&reg_v64,
                DIS_LEARNf, &fld_v32);
            SOC_PBMP_WORD_SET(current_pbmp, 0, fld_v32);
        }
    
        if (mode == DRV_PORT_HW_LEARN) { /* enable */
            SOC_PBMP_CLEAR(temp_pbmp);
            SOC_PBMP_NEGATE(temp_pbmp, pbmp);
            SOC_PBMP_AND(current_pbmp, temp_pbmp);
        } else { /* disable */
            SOC_PBMP_OR(current_pbmp, pbmp);
        }
    
        if (SOC_IS_ROBO5348(unit)) {
            soc_robo_64_pbmp_to_val(unit, &current_pbmp, &fld_v64);
            soc_DIS_LEARNr_field_set(unit, (uint32 *)&reg_v64,
                DIS_LEARNf, (uint32 *)&fld_v64);
        } else { /* 5347 */
            fld_v32 = SOC_PBMP_WORD_GET(current_pbmp, 0);
            soc_DIS_LEARNr_field_set(unit, (uint32 *)&reg_v64,
                DIS_LEARNf, &fld_v32);
        }
    
        if ((rv = REG_WRITE_DIS_LEARNr(unit, (uint32 *)&reg_v64)) < 0) {
            return rv;
        }
    } else if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {
        if ((rv = REG_READ_DIS_LEARNr(unit, &reg_v32)) < 0) {
            return rv;
        }

        soc_DIS_LEARNr_field_get(unit, &reg_v32,
                DIS_LEARNf, &fld_v32);
        SOC_PBMP_WORD_SET(current_pbmp, 0, fld_v32);
    
        if (mode == DRV_PORT_HW_LEARN) { /* enable */
            SOC_PBMP_CLEAR(temp_pbmp);
            SOC_PBMP_NEGATE(temp_pbmp, pbmp);
            SOC_PBMP_AND(current_pbmp, temp_pbmp);
        } else { /* disable */
            SOC_PBMP_OR(current_pbmp, pbmp);
        }

        fld_v32 = SOC_PBMP_WORD_GET(current_pbmp, 0);
        soc_DIS_LEARNr_field_set(unit, &reg_v32,
                DIS_LEARNf, &fld_v32);
    
        if ((rv = REG_WRITE_DIS_LEARNr(unit, &reg_v32)) < 0) {
            return rv;
        }
     } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        /* per port */
        PBMP_ITER(pbmp, port) {
            if ((rv = REG_READ_PORT_SEC_CONr(unit, port, &reg_v32)) < 0) {
                return rv;
            }

            if (mode == DRV_PORT_HW_LEARN) { /* enable */
                fld_v32 = 0;
            } else { /* disable */
                fld_v32 = 1;
            }
            soc_PORT_SEC_CONr_field_set(unit, &reg_v32,
                DIS_LEARNf, &fld_v32);
        
            if ((rv = REG_WRITE_PORT_SEC_CONr(unit, port, &reg_v32)) < 0) {
                return rv;
            }
        }
    } else {
        rv = SOC_E_UNAVAIL;
    }
    break;

   /* no support section */    
   case DRV_PORT_SW_LEARN:
   case DRV_PORT_HW_SW_LEARN:   
   case DRV_PORT_DROP:   
   case DRV_PORT_SWLRN_DROP:    
   case DRV_PORT_HWLRN_DROP:    
   case DRV_PORT_SWHWLRN_DROP:       
       rv = SOC_E_UNAVAIL;
       break;
   default:
        rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 * Function:
 *  drv_arl_learn_enable_get
 * Purpose:
 *  Setting per port SA learning process.
 * Parameters:
 *  unit    - RoboSwitch unit #
 *  port    - port
 *  mode   - Port learn mode
 */
int
drv_arl_learn_enable_get(int unit, soc_port_t port, uint32 *mode)
{
    int     rv = SOC_E_NONE;
    uint32  reg_v32, fld_v32 = 0;
    uint64  reg_v64, fld_v64;
    soc_pbmp_t current_pbmp;

    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        if ((rv = REG_READ_DIS_LEARNr(unit, (uint32 *)&reg_v64)) < 0) {
            return rv;
        }
    
        COMPILER_64_ZERO(fld_v64);
        if (SOC_IS_ROBO5348(unit)) {
            soc_DIS_LEARNr_field_get(unit, (uint32 *)&reg_v64,
                DIS_LEARNf, (uint32 *)&fld_v64);
            soc_robo_64_val_to_pbmp(unit, &current_pbmp, fld_v64);
        } else { /* 5347 */
            soc_DIS_LEARNr_field_get(unit, (uint32 *)&reg_v64,
                DIS_LEARNf, &fld_v32);
            SOC_PBMP_WORD_SET(current_pbmp, 0, fld_v32);
        }
    } else if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {
        if ((rv = REG_READ_DIS_LEARNr(unit, &reg_v32)) < 0) {
            return rv;
        }

        soc_DIS_LEARNr_field_get(unit, &reg_v32,
                DIS_LEARNf, &fld_v32);
        SOC_PBMP_WORD_SET(current_pbmp, 0, fld_v32);
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        if ((rv = REG_READ_PORT_SEC_CONr(unit, port, &reg_v32)) < 0) {
            return rv;
        }

        soc_PORT_SEC_CONr_field_get(unit, &reg_v32,
            DIS_LEARNf, &fld_v32);
    } else {
        return SOC_E_UNAVAIL;
    }

    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        if (fld_v32) {
            *mode = DRV_PORT_DISABLE_LEARN; /* This port is in DISABLE SA learn state */
        } else {
            *mode = DRV_PORT_HW_LEARN;
        }
    } else {
    if (SOC_PBMP_MEMBER(current_pbmp, port)) {
        *mode = DRV_PORT_DISABLE_LEARN; /* This port is in DISABLE SA learn state */
    } else {
        *mode = DRV_PORT_HW_LEARN;
    }
    }
    return rv;
}

/*
 * Function:
 *  drv_arl_learn_count_set
 * Purpose:
 *  Get the ARL port basis learning count related information.
 * Parameters:
 *  unit    - RoboSwitch unit #
 *  port    - (-1) is allowed to indicate the system based parameter
 *  type    -  list of types : DRV_ARL_LRN_CNT_LIMIT, DRV_ARL_LRN_CNT_INCRASE
 *              DRV_ARL_LRN_CNT_DECREASE, DRV_ARL_LRN_CNT_RESET, 
 *              
 *  value   - the set value for indicated type.
 *               
 */
int
drv_arl_learn_count_set(int unit, uint32 port, 
        uint32 type, int value)
{
    uint32  reg_value = 0, temp = 0, retry = 0;
    uint64  reg_value64, temp64;
    soc_pbmp_t pbmp;
    int     rv = SOC_E_NONE;
    
    /* 
     * this function for bcm5324/53242/53262/5348/5347,
     * and bcm5396/5389/5398 is not supported.
     */

    COMPILER_64_ZERO(reg_value64);
    COMPILER_64_ZERO(temp64);
    SOC_PBMP_CLEAR(pbmp);

    if (SOC_IS_ROBO5324(unit)) {
        if (IS_GE_PORT(unit, port)) {
            return SOC_E_UNAVAIL;
        }
    }

    if ((type == DRV_PORT_SA_LRN_CNT_LIMIT) || 
            (type == DRV_PORT_SA_LRN_CNT_INCREASE) || 
            (type == DRV_PORT_SA_LRN_CNT_DECREASE) || 
            (type == DRV_PORT_SA_LRN_CNT_RESET)){
        if (type == DRV_PORT_SA_LRN_CNT_LIMIT) {
            if (SOC_IS_ROBO5324(unit) || 
                SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
                SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                /* Config Port x Dynamic Learning Threshold register */
                SOC_IF_ERROR_RETURN(REG_READ_PORT_MAX_LEARNr
                    (unit, port, &reg_value));

                temp = (uint32)value;
                SOC_IF_ERROR_RETURN(soc_PORT_MAX_LEARNr_field_set
                    (unit, &reg_value, DYN_MAX_MAC_NOf, &temp));
                SOC_IF_ERROR_RETURN(REG_WRITE_PORT_MAX_LEARNr
                    (unit, port, &reg_value));
            } else {
                rv = SOC_E_UNAVAIL;
            }
        } else if (type == DRV_PORT_SA_LRN_CNT_RESET) {
            /* --- make sure the current learned SA number is zero --- */
            /*      1. force the per port's learned SA counter reset.
             *      2. set back to enable normal learned SA counter. 
             */
            if (SOC_IS_ROBO5324(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_MAC_SEC_CON4r
                    (unit, &reg_value));

                /* reset port's learned SA # */
                temp = (uint32)0x1 << port;
                SOC_IF_ERROR_RETURN(soc_MAC_SEC_CON4r_field_set
                    (unit, &reg_value, SW_SA_CNT_RSTf, &temp));

                SOC_IF_ERROR_RETURN(REG_WRITE_MAC_SEC_CON4r
                    (unit, &reg_value));
                
                /* enable normal port's learned SA # count process */
                temp = 0;
                SOC_IF_ERROR_RETURN(soc_MAC_SEC_CON4r_field_set
                    (unit, &reg_value, SW_SA_CNT_RSTf, &temp));

                SOC_IF_ERROR_RETURN(REG_WRITE_MAC_SEC_CON4r
                    (unit, &reg_value));
            } else if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_MAC_SEC_CON3r
                    (unit, (uint32 *)&reg_value64));

                /* reset port's learned SA # */
                if (SOC_IS_ROBO5348(unit)) {
                    SOC_IF_ERROR_RETURN(soc_MAC_SEC_CON3r_field_get
                        (unit, (uint32 *)&reg_value64, SW_SA_CNT_RSTf, (uint32 *)&temp64));
                    soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
                    SOC_PBMP_PORT_ADD(pbmp, port);
                    soc_robo_64_pbmp_to_val(unit, &pbmp, &temp64);
                    SOC_IF_ERROR_RETURN(soc_MAC_SEC_CON3r_field_set
                        (unit, (uint32 *)&reg_value64, SW_SA_CNT_RSTf, (uint32 *)&temp64));
                } else { /* 5347 */
                    SOC_IF_ERROR_RETURN(soc_MAC_SEC_CON3r_field_get
                        (unit, (uint32 *)&reg_value64, SW_SA_CNT_RSTf, &temp));
                    SOC_PBMP_WORD_SET(pbmp, 0, temp);
                    SOC_PBMP_PORT_ADD(pbmp, port);
                    temp = SOC_PBMP_WORD_GET(pbmp, 0);
                    SOC_IF_ERROR_RETURN(soc_MAC_SEC_CON3r_field_set
                        (unit, (uint32 *)&reg_value64, SW_SA_CNT_RSTf, &temp));
                }
                SOC_IF_ERROR_RETURN(REG_WRITE_MAC_SEC_CON3r
                    (unit, (uint32 *)&reg_value64));
                
                /* enable normal port's learned SA # count process */
                if (SOC_IS_ROBO5348(unit)) {
                   COMPILER_64_ZERO(temp64);
                    SOC_IF_ERROR_RETURN(soc_MAC_SEC_CON3r_field_set
                        (unit, (uint32 *)&reg_value64, SW_SA_CNT_RSTf, (uint32 *)&temp64));
                } else { /* 5347 */
                    temp = 0;
                    SOC_IF_ERROR_RETURN(soc_MAC_SEC_CON3r_field_set
                        (unit, (uint32 *)&reg_value64, SW_SA_CNT_RSTf, &temp));
                }
                
                SOC_IF_ERROR_RETURN(REG_WRITE_MAC_SEC_CON3r
                    (unit, (uint32 *)&reg_value64));
            } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_LRN_CNT_CTLr
                    (unit, &reg_value));

                /* reset port's learned SA # */
                temp = port;
                SOC_IF_ERROR_RETURN(soc_LRN_CNT_CTLr_field_set
                    (unit, &reg_value, PORT_NUM_Rf, &temp));
        
                /* set access control field to 3(RESET) */
                temp = 3;
                SOC_IF_ERROR_RETURN(soc_LRN_CNT_CTLr_field_set
                    (unit, &reg_value, ACC_CTLf, &temp));
                
                /* start learning counter control Process */
                temp = 1;
                SOC_IF_ERROR_RETURN(soc_LRN_CNT_CTLr_field_set
                    (unit, &reg_value, START_DONEf, &temp));

                SOC_IF_ERROR_RETURN(REG_WRITE_LRN_CNT_CTLr
                    (unit, &reg_value));                
        
                /* wait for complete */
                for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                    SOC_IF_ERROR_RETURN(REG_READ_LRN_CNT_CTLr
                        (unit, &reg_value));

                    SOC_IF_ERROR_RETURN(soc_LRN_CNT_CTLr_field_get
                        (unit, &reg_value, START_DONEf, &temp));
                    if (!temp) {
                        break;
                    }
                }
                if (retry >= SOC_TIMEOUT_VAL) {
                    return SOC_E_TIMEOUT;
                }        
            } else {
                rv = SOC_E_UNAVAIL;
            }
        } else {
            /* DRV_PORT_SA_LRN_CNT_INCREASE || DRV_PORT_SA_LRN_CNT_DECREASE */
            if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
                SOC_IF_ERROR_RETURN(REG_READ_LRN_CNT_CTLr(unit, &reg_value));
                
                /* set access control field to 1(INCREASE) or 2(DECREASE) */
                temp = port;
                SOC_IF_ERROR_RETURN(soc_LRN_CNT_CTLr_field_set
                        (unit, &reg_value, PORT_NUM_Rf, &temp));

                /* set access control field to 1(INCREASE) or 2(DECREASE) */
                temp = (type == DRV_PORT_SA_LRN_CNT_INCREASE) ? 1 : 2;
                SOC_IF_ERROR_RETURN(soc_LRN_CNT_CTLr_field_set
                        (unit, &reg_value, ACC_CTLf, &temp));
                
                /* start learning counter control Process */
                temp = 1;
                SOC_IF_ERROR_RETURN(soc_LRN_CNT_CTLr_field_set
                        (unit, &reg_value, START_DONEf, &temp));

                SOC_IF_ERROR_RETURN(REG_WRITE_LRN_CNT_CTLr
                        (unit, &reg_value));
                
                /* wait for complete */
                for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                    SOC_IF_ERROR_RETURN(REG_READ_LRN_CNT_CTLr
                            (unit, &reg_value));

                    SOC_IF_ERROR_RETURN(soc_LRN_CNT_CTLr_field_get
                            (unit, &reg_value, START_DONEf, &temp));
                    if (!temp) {
                        break;
                    }
                }
                if (retry >= SOC_TIMEOUT_VAL) {
                    return SOC_E_TIMEOUT;
                }        
            } else {
                /* bcm5324, bcm5348 and bcm5347 have no interface for SW to 
                 *  perform SA learn count increment and decrement.
                 * Here we return SOC_E_UNAVAIL for those chips which have no 
                 *  feature or no interface on handling SA learn count.
                 */
                rv = SOC_E_UNAVAIL;
            }
        }
    } else {
        /* DRV_PORT_SA_LRN_CNT_NUMBER : for read only */
        rv = SOC_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function:
 *  drv_arl_learn_count_get
 * Purpose:
 *  Get the ARL port basis learning count related information.
 * Parameters:
 *  unit    - RoboSwitch unit #
 *  port    - (-1) is allowed to indicate the system based parameter
 *  type    -  list of types : DRV_ARL_LRN_CNT_NUMBER, DRV_ARL_LRN_CNT_LIMIT
 *  value   - (OUT)the get value for indicated type.
 */
int
drv_arl_learn_count_get(int unit, uint32 port, 
        uint32 type, int *value)
{
    uint32  reg_value = 0, temp = 0;

    if (SOC_IS_ROBO5324(unit)) {
        if (IS_GE_PORT(unit, port)) {
            return SOC_E_UNAVAIL;
        }
    }

    if (type == DRV_PORT_SA_LRN_CNT_LIMIT){
        if (SOC_IS_ROBO5324(unit) || 
            SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
            SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
            /* Get Port x Dynamic Learning Threshold register */
            SOC_IF_ERROR_RETURN(REG_READ_PORT_MAX_LEARNr
                (unit, port, &reg_value));

            SOC_IF_ERROR_RETURN(soc_PORT_MAX_LEARNr_field_get
                (unit, &reg_value, DYN_MAX_MAC_NOf, &temp));

            *value = (int)temp;
        } else {
            return SOC_E_UNAVAIL;
        }
    } else if (type == DRV_PORT_SA_LRN_CNT_NUMBER) {
        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
            /* Get Port x Dynamic Learning counter register */
            SOC_IF_ERROR_RETURN(REG_READ_PORT_SA_CNTr
                    (unit, port, &reg_value));
            
            SOC_IF_ERROR_RETURN(soc_PORT_SA_CNTr_field_get
                    (unit, &reg_value, CUR_SA_CNTf, &temp));

            *value = (int)temp;
        } else {
            /* bcm5324, bcm5348 and bcm5347 can report port basis dynamic 
             *  learning count but may not be consistent with real HW L2 
             *  learning count since there is no interface for SW to handle 
             *  the SA learn count but HW won't help to maintain the correct 
             *  learn counter value.(the SA learn count is handled by HW  
             *  learning and aging only)
             *
             * Thus this section is implemented to report SOC_E_UNAVIAL on 
             *  bcm5324, bcm5348 and bcm5347 like other ROBO chips which have 
             *  no SA learn limit feature.
             */
            return SOC_E_UNAVAIL;
        }
        
    } else {
        return SOC_E_UNAVAIL;
    }
    
    return SOC_E_NONE;
}
