/*
 * $Id: l2cache.c 1.14 Broadcom SDK $
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
 * L2 Cache - Layer 2 BPDU and overflow address cache
 */

#include <soc/debug.h>
#include <soc/drv.h>

#include <bcm/l2.h>
#include <bcm/error.h>
#include <bcm/types.h>

#include <bcm_int/robo_dispatch.h>

/* define l2 cache entries size for robo */
#define L2_CACHE_ENTRIES_NUM  2
#define L2_CACHE_ENTRIES_NUM_5395  6    /* BCM5395/BCM53115/BCM53118 */

/*
 * Function:
 *      bcm_robo_l2_cache_delete
 * Purpose:
 *      Clear an L2 cache multiport address and vector with index
 * Parameters:
 *      unit - device number
 *      index - multiport address register index
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l2_cache_delete(int unit, int index)
{
    bcm_pbmp_t  mport_vctr;
    bcm_mac_t   mport_addr;
    uint32 mac_type;

    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_cache_delete()..\n");
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_PBMP_CLEAR(mport_vctr);
    ENET_SET_MACADDR(mport_addr, _soc_mac_all_zeroes);

    switch (index) {
        case 0:
            mac_type = DRV_MAC_MULTIPORT_0;
            break;
        case 1:             
            mac_type = DRV_MAC_MULTIPORT_1;
            break;
        case 2:
            mac_type = DRV_MAC_MULTIPORT_2;
            break;
        case 3:
            mac_type = DRV_MAC_MULTIPORT_3;
            break;
        case 4:               
            mac_type = DRV_MAC_MULTIPORT_4;
            break;
        case 5:
            mac_type = DRV_MAC_MULTIPORT_5;
            break;
        default:
            return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->mac_set)
        (unit, mport_vctr, mac_type, mport_addr, 0));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_l2_cache_delete_all
 * Purpose:
 *      Clear all L2 cache multiport address and vector
 * Parameters:
 *      unit - device number
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_l2_cache_delete_all(int unit) 
{  
    int index, size;
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_cache_delete_all()..\n");

    BCM_IF_ERROR_RETURN(bcm_l2_cache_size_get(unit, &size));

    for (index = 0 ; index < size ; index++) {
        BCM_IF_ERROR_RETURN(bcm_l2_cache_delete(unit, index));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_l2_cache_get
 * Purpose:
 *	get L2 cache multiport address and vector register with index
 *    index = 0 : DRV_MAC_MULTIPORT_0,
 *    index = 1 : DRV_MAC_MULTIPORT_1,
 *    index = 2 : DRV_MAC_MULTIPORT_2,
 *    index = 3 : DRV_MAC_MULTIPORT_3,
 *    index = 4 : DRV_MAC_MULTIPORT_4,
 *    index = 5 : DRV_MAC_MULTIPORT_5,
 * Parameters:
 *	unit        - RoboSwitch unit #.
 *	index      - multiport address register index
 *	addr       - l2 cache address
 * Returns:
 *	BCM_E_XXXX
 */
int 
bcm_robo_l2_cache_get(int unit, int index, bcm_l2_cache_addr_t *addr) 
{
    uint32 tmp_mac[2], mac_type;
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_cache_get()..\n");
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    switch (index) {
        case 0:
            mac_type = DRV_MAC_MULTIPORT_0;
            break;
        case 1:             
            mac_type = DRV_MAC_MULTIPORT_1;
            break;
        case 2:
            mac_type = DRV_MAC_MULTIPORT_2;
            break;
        case 3:
            mac_type = DRV_MAC_MULTIPORT_3;
            break;
        case 4:               
            mac_type = DRV_MAC_MULTIPORT_4;
            break;
        case 5:
            mac_type = DRV_MAC_MULTIPORT_5;
            break;
        default:
            return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN((DRV_SERVICES(unit)->mac_get)
        (unit, 0, mac_type, &addr->dest_ports, addr->mac));
    
    MAC_ADDR_TO_UINT32(addr->mac, tmp_mac);

    if ((tmp_mac[0] != 0) || (tmp_mac[1] != 0)) {
        addr->flags |= BCM_L2_CACHE_DESTPORTS;
        return BCM_E_NONE;
    } else {
        /* MAC_Addr = 0 means invalid for ROBO */
        return BCM_E_NOT_FOUND;
    }
    
}

/*
 * Function:
 *      bcm_robo_l2_cache_init
 * Purpose:
 *      Initialize the L2 cache
 * Parameters:
 *      unit - device number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Clears all entries and preloads a few BCM_L2_CACHE_BPDU
 *      entries to match previous generation of devices.
 */
int 
bcm_robo_l2_cache_init(int unit) 
{  
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    BCM_IF_ERROR_RETURN(bcm_l2_cache_delete_all(unit));
    
    return BCM_E_NONE;
}

/* user address search :
 *
 *  - for found case, return BCM_E_NONE and return id at this found index.
 *  - for not found case,
 *      a. Full condition, return BCM_E_FULL and id = -1.
 *      b. Not Full condition, return BCM_E_NOT_FOUND and return the first
 *          free index to id.
 */
int 
_bcm_robo_l2_cache_search(int unit, bcm_l2_cache_addr_t *addr, int *id)
{
    int i, rv, l2u_addr_size = 0, tmp_id = -1;
    uint32 tmp_mac[2], invalid_mac[] ={0, 0};
    bcm_l2_cache_addr_t temp_l2u_addr;

    BCM_IF_ERROR_RETURN(bcm_robo_l2_cache_size_get(unit, 
            &l2u_addr_size)); 
    
    for (i = 0; i < l2u_addr_size; i++){
        sal_memset(&temp_l2u_addr, 0, sizeof (bcm_l2_cache_addr_t));
        rv = bcm_robo_l2_cache_get( unit, 
                i, &temp_l2u_addr);
        if (rv == BCM_E_NONE){
            MAC_ADDR_TO_UINT32(temp_l2u_addr.mac, tmp_mac);

            /* keep the founded free id */
           if (!sal_memcmp(tmp_mac, invalid_mac, sizeof(tmp_mac))){
                if (tmp_id == -1){
                    tmp_id = i;
                }
            }

            /* l2 user addr, ports and flag will be compared */
            if (!sal_memcmp(addr, &temp_l2u_addr, 
                    sizeof (bcm_l2_cache_addr_t))){
                *id = i;
                rv = BCM_E_NONE;
                break;
            }
        } else if (rv == BCM_E_NOT_FOUND){
            /* keep the founded free id */
            if (tmp_id == -1){
                tmp_id = i;
            }
        } else {
            /* unexpect erorr */
            soc_cm_debug(DK_ARL, 
                    "%s,failed on check #%d L2 user address\n",
                    FUNCTION_NAME(), i);
            return rv;
        }
    }

    if ((i == l2u_addr_size) && (tmp_id == -1)){
        rv = BCM_E_FULL;
    } else {
        rv = BCM_E_NOT_FOUND;
    }
    *id = tmp_id;
    
    return rv;
}

/*
 * Function:
 *	bcm_robo_l2_cache_set
 * Purpose:
 *	set L2 cache multiport address and vector register with index
 *    index = 0 : DRV_MAC_MULTIPORT_0,
 *    index = 1 : DRV_MAC_MULTIPORT_1,
 *    index = 2 : DRV_MAC_MULTIPORT_2,
 *    index = 3 : DRV_MAC_MULTIPORT_3,
 *    index = 4 : DRV_MAC_MULTIPORT_4,
 *    index = 5 : DRV_MAC_MULTIPORT_5,
 * Parameters:
 *	unit        - RoboSwitch unit #.
 *	index     - multiport address register index
 *	addr - l2 cache address
 *    index_used - (OUT) l2 cache entry number actually used
 * Returns:
 *	BCM_E_XXXX
 */
int 
bcm_robo_l2_cache_set(int unit, int index, bcm_l2_cache_addr_t *addr, int *index_used) 
{
    uint32 mac_type = 0;
    int rv = BCM_E_NONE, search_id = -1;
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_cache_set()..\n");
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    /* SDK-29951 : API guide comply for index = -1 
     *  1. avoid the existed L2 user address and select a free index.
     *  2. return FULL if no free index.
     */
    if (index == -1){
        rv = _bcm_robo_l2_cache_search(unit, addr, &search_id);
        if (rv == BCM_E_NONE){
            /* existed already, report the founded index */
            *index_used = search_id;
            return rv;
        } else if (rv == BCM_E_FULL){
            return rv;
        } else if (rv == BCM_E_NOT_FOUND) {
            /* assign the foudned free index */
            index = search_id;
        } else {
            return BCM_E_INTERNAL;
        }
    }
    
    if(addr->flags == BCM_L2_CACHE_DESTPORTS) {
        switch (index) {
            case 0:
                mac_type = DRV_MAC_MULTIPORT_0;
                break;
            case 1:             
                mac_type = DRV_MAC_MULTIPORT_1;
                break;
            case 2:
                mac_type = DRV_MAC_MULTIPORT_2;
                break;
            case 3:
                mac_type = DRV_MAC_MULTIPORT_3;
                break;
            case 4:               
                mac_type = DRV_MAC_MULTIPORT_4;
                break;
            case 5:
                mac_type = DRV_MAC_MULTIPORT_5;
                break;
            default:
                *index_used = -1;
                return BCM_E_PARAM;
        }
        rv = DRV_MAC_SET(unit, addr->dest_ports, mac_type, addr->mac, 0);
        BCM_IF_ERROR_RETURN(rv);
        
        *index_used = index;
    } else {
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function:
 *      bcm_robo_l2_cache_size_get
 * Purpose:
 *      Get number of L2 cache entries
 * Parameters:
 *      unit - device number
 *      size - (OUT) number of entries in cache
 *               return L2_CACHE_ENTRIES_NUM in it's size field for robo.
 * Returns:
 *      L2_CACHE_ENTRIES_NUM
 */
int 
bcm_robo_l2_cache_size_get(int unit, int *size) 
{
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_cache_size_get()..\n");
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (SOC_IS_ROBO5395(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        *size = L2_CACHE_ENTRIES_NUM_5395;
    } else {
        *size = L2_CACHE_ENTRIES_NUM;
    }
    
    return BCM_E_NONE;
}
