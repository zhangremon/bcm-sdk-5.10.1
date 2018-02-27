/*
 * $Id: trill.c 1.46.6.5 Broadcom SDK $
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
 * ESW Trill API
 */

#include <soc/defs.h>
#include <sal/core/libc.h>

#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/hash.h>
#include <soc/debug.h>
#include <bcm/types.h>
#include <bcm/error.h>

#include <bcm/types.h>
#include <bcm/l3.h>
#include <bcm/multicast.h>
#include <bcm/trill.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/virtual.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/trident.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/triumph2.h>
#include <bcm_int/esw/trill.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/multicast.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/switch.h>
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0
#endif /* BCM_WARM_BOOT_SUPPORT */

_bcm_td_trill_bookkeeping_t   *_bcm_td_trill_bk_info[BCM_MAX_NUM_UNITS] = { 0 };

#define TRILL_INFO(_unit_)   (_bcm_td_trill_bk_info[_unit_])
#define L3_INFO(_unit_)   (&_bcm_l3_bk_info[_unit_])

#define BCM_TRILL_CHECK_ERROR_RETURN(op, _u_, vp)  \
    if (op < 0) { \
         (void) _bcm_vp_free(_u_, _bcmVpTypeTrill, 1, vp); \
         return (op); \
    }

#define BCM_TRILL_L2MC_CHECK( _u_ , index) \
    if ((index < 0) ||\
         (index >= soc_mem_index_count(_u_, L2MCm))) { \
          return BCM_E_PARAM; \
    } \

#define BCM_TRILL_L3MC_CHECK( _u_ , index) \
    if ((index < 0) ||\
         (index >= soc_mem_index_count(_u_, L3_IPMCm))) { \
         return BCM_E_PARAM; \
    } \

/*
 * Function:
 *      _bcm_trill_check_init
 * Purpose:
 *      Check if Trill is initialized
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */


STATIC int 
_bcm_trill_check_init (int unit)
{
    _bcm_td_trill_bookkeeping_t *trill_info;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);

    if ((trill_info == NULL) || (trill_info->initialized == FALSE)) { 
         return BCM_E_INIT;
    } else {
         return BCM_E_NONE;
    }
}



/*
 * Function:
 *      bcm_trill_lock
 * Purpose:
 *      Take the Trill Lock Sempahore
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */


 int 
 bcm_td_trill_lock (int unit)
{
   int rv;

   rv = _bcm_trill_check_init (unit);
   
   if ( rv == BCM_E_NONE ) {
           sal_mutex_take(TRILL_INFO((unit))->trill_mutex, sal_mutex_FOREVER);
   }
   return rv; 
}



/*
 * Function:
 *      bcm_trill_unlock
 * Purpose:
 *      Release  the Trill Lock Semaphore
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */


void
bcm_td_trill_unlock(int unit)
{
    int rv;

    rv = _bcm_trill_check_init (unit);
    if ( rv == BCM_E_NONE ) {
         sal_mutex_give(TRILL_INFO((unit))->trill_mutex);
    }
}



/*
 * Function:
 *      _bcm_td_trill_free_resource
 * Purpose:
 *      Free all allocated software resources 
 * Parameters:
 *      unit - SOC unit number
 * Returns:
 *      Nothing
 */


STATIC void
_bcm_td_trill_free_resource(int unit)
{
    /* If software tables were not allocated we are done. */ 
    if (NULL == TRILL_INFO(unit)) {
        return;
    }

    /* Free module data. */
    sal_free(TRILL_INFO(unit));
    TRILL_INFO(unit) = NULL;
}

/*
 * Function:
 *      bcm_td_trill_ethertype_set
 * Purpose:
 *      Enable TRILL functionality
 * Parameters:
 *      unit - SOC unit number.
 *      etherType -  Non-zero value: enable TRILL support
 *                          Zero vlaue: Disable TRILL support
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_td_trill_ethertype_set(int unit, uint16 etherType)
{
    int rv = BCM_E_NONE;
    ing_trill_parse_control_entry_t  ing_ent;
    egr_trill_parse_control_entry_t egr_ent;
    ing_trill_payload_parse_control_entry_t tpid_ent;

    sal_memset(&ing_ent, 0, sizeof(ing_trill_parse_control_entry_t));	
    sal_memset(&egr_ent, 0, sizeof(egr_trill_parse_control_entry_t));
    sal_memset(&tpid_ent, 0, sizeof(ing_trill_payload_parse_control_entry_t));

    if (etherType > 0) {
         soc_mem_field32_set(unit, ING_TRILL_PARSE_CONTROLm, &ing_ent,
                                         TRILL_ETHERTYPE_ENABLEf, 0x1);
         soc_mem_field32_set(unit, ING_TRILL_PARSE_CONTROLm, &ing_ent,
                                         TRILL_ETHERTYPEf, etherType);

         soc_mem_field32_set(unit, EGR_TRILL_PARSE_CONTROLm, &egr_ent,
                                         TRILL_ETHERTYPE_ENABLEf, 0x1);
         soc_mem_field32_set(unit, EGR_TRILL_PARSE_CONTROLm, &egr_ent,
                                         TRILL_ETHERTYPEf, etherType);

         soc_mem_field32_set(unit, ING_TRILL_PAYLOAD_PARSE_CONTROLm, &tpid_ent,
                                         TPID_ENABLEf, 0x1);
    }

    BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_TRILL_PARSE_CONTROLm,
                                            MEM_BLOCK_ALL, 0, &ing_ent));
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, EGR_TRILL_PARSE_CONTROLm,
                                            MEM_BLOCK_ALL, 0, &egr_ent));
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_TRILL_PAYLOAD_PARSE_CONTROLm,
                                            MEM_BLOCK_ALL, 0, &tpid_ent));
    return rv;
}

/*
 * Function:
 *      bcm_td_trill_ethertype_get
 * Purpose:
 *      Get TRILL EtherType
 * Parameters:
 *      unit - SOC unit number.
 *      etherType - return value
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_td_trill_ethertype_get(int unit, int *etherType)
{
    int rv = BCM_E_NONE;
    ing_trill_parse_control_entry_t  ing_ent;

    sal_memset(&ing_ent, 0, sizeof(ing_trill_parse_control_entry_t));

    BCM_IF_ERROR_RETURN(
       soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &ing_ent));

    *etherType = soc_mem_field32_get(unit, ING_TRILL_PARSE_CONTROLm, &ing_ent,
                                         TRILL_ETHERTYPEf);
    return rv;
}

/*
 * Function:
 *      bcm_td_trill_ISIS_ethertype_set
 * Purpose:
 *      Set TRILL ISIS EtherType
 * Parameters:
 *      unit - SOC unit number.
 *      etherType -  Non-zero value: enable Ethertype
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_td_trill_ISIS_ethertype_set(int unit, uint16 etherType)
{
    int rv = BCM_E_NONE;
    ing_trill_parse_control_entry_t  ing_ent;
    egr_trill_parse_control_entry_t egr_ent;    
    egr_trill_parse_control_2_entry_t egr2_ent;

    if (etherType > 0) {
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &ing_ent));
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, EGR_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &egr_ent));
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, EGR_TRILL_PARSE_CONTROL_2m,
                       MEM_BLOCK_ALL, 0, &egr2_ent));

         if (soc_mem_field_valid(unit, ING_TRILL_PARSE_CONTROLm, 
                             TRILL_L2_IS_IS_ETHERTYPE_ENABLEf)) {
              soc_mem_field32_set(unit, ING_TRILL_PARSE_CONTROLm, &ing_ent,
                                         TRILL_L2_IS_IS_ETHERTYPE_ENABLEf, 0x1);
         }
         if (soc_mem_field_valid(unit, ING_TRILL_PARSE_CONTROLm, 
                             TRILL_L2_IS_IS_ETHERTYPEf)) {
             soc_mem_field32_set(unit, ING_TRILL_PARSE_CONTROLm, &ing_ent,
                                         TRILL_L2_IS_IS_ETHERTYPEf, etherType);
         }
         if (soc_mem_field_valid(unit, EGR_TRILL_PARSE_CONTROLm, 
                             TRILL_L2_IS_IS_ETHERTYPE_ENABLEf)) {
              soc_mem_field32_set(unit, EGR_TRILL_PARSE_CONTROLm, &egr_ent,
                                         TRILL_L2_IS_IS_ETHERTYPE_ENABLEf, 0x1);
         }
         if (soc_mem_field_valid(unit, EGR_TRILL_PARSE_CONTROLm, 
                             TRILL_L2_IS_IS_ETHERTYPEf)) {
              soc_mem_field32_set(unit, EGR_TRILL_PARSE_CONTROLm, &egr_ent,
                                         TRILL_L2_IS_IS_ETHERTYPEf, etherType);
         }
         if (soc_mem_field_valid(unit, EGR_TRILL_PARSE_CONTROL_2m, 
                             TRILL_L2_IS_IS_ETHERTYPE_ENABLEf)) {
              soc_mem_field32_set(unit, EGR_TRILL_PARSE_CONTROL_2m, &egr2_ent,
                                         TRILL_L2_IS_IS_ETHERTYPE_ENABLEf, 0x1);
         }
         if (soc_mem_field_valid(unit, EGR_TRILL_PARSE_CONTROL_2m, 
                             TRILL_L2_IS_IS_ETHERTYPEf)) {
              soc_mem_field32_set(unit, EGR_TRILL_PARSE_CONTROL_2m, &egr2_ent,
                                         TRILL_L2_IS_IS_ETHERTYPEf, etherType);
         }

         BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_TRILL_PARSE_CONTROLm,
                                            MEM_BLOCK_ALL, 0, &ing_ent));
         BCM_IF_ERROR_RETURN(soc_mem_write(unit, EGR_TRILL_PARSE_CONTROLm,
                                            MEM_BLOCK_ALL, 0, &egr_ent));
         BCM_IF_ERROR_RETURN(soc_mem_write(unit, EGR_TRILL_PARSE_CONTROL_2m,
                                            MEM_BLOCK_ALL, 0, &egr2_ent));
    }
    return rv;
}

/*
 * Function:
 *      bcm_td_trill_ISIS_ethertype_get
 * Purpose:
 *      Get TRILL ISIS EtherType
 * Parameters:
 *      unit - SOC unit number.
 *      etherType - return value
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_td_trill_ISIS_ethertype_get(int unit, int *etherType)
{
    int rv = BCM_E_NONE;
    ing_trill_parse_control_entry_t  ing_ent;

    sal_memset(&ing_ent, 0, sizeof(ing_trill_parse_control_entry_t));

    BCM_IF_ERROR_RETURN(
       soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &ing_ent));

    if (soc_mem_field_valid(unit, ING_TRILL_PARSE_CONTROLm, 
                             TRILL_L2_IS_IS_ETHERTYPEf)) {
         *etherType = soc_mem_field32_get(unit, ING_TRILL_PARSE_CONTROLm, 
                                  &ing_ent, TRILL_L2_IS_IS_ETHERTYPEf);
    } else {
         *etherType = 0;
    }

    return rv;
}


/*
 * Function:
 *      _bcm_td_trill_mac_set
 * Description:
 *      flag = 0:  Set the lower 24 bits of MAC address field for TRILL specific MAC
 *      flag = 1:  Set the higher 24 bits of MAC address field for TRILL specific MAC
 * Parameters:
 *      unit       - unit number (driver internal).
 *      type      - The required switch control type to set upper MAC for
 *      mac      - MAC address
 * Returns:
 *      BCM_E_xxxx
 */

STATIC void
_bcm_td_trill_mac_set(int unit, uint32 mac_field, soc_mem_t mem, 
                        void *entry, soc_field_t field, sal_mac_addr_t mac, uint8 flag) 
{
    soc_mem_mac_addr_get(unit, mem, entry, 
                        field, mac);
    if (!flag) {
         /* Set the low-order bytes */
         mac[3] = (uint8) (mac_field >> 16 & 0xff);
         mac[4] = (uint8) (mac_field >> 8 & 0xff);
         mac[5] = (uint8) (mac_field & 0xff);
    } else {
         /* Set the High-order bytes */
         mac[0] = (uint8) (mac_field >> 16 & 0xff);
         mac[1] = (uint8) (mac_field >> 8 & 0xff);
         mac[2] = (uint8) (mac_field & 0xff);
    }
    soc_mem_mac_addr_set(unit, mem, entry,
                        field, mac);
}

/*
 * Function:
 *      bcm_td_trill_mac_set
 * Description:
 *      flag = 0:  Set the lower 24 bits of MAC address field for TRILL specific MAC
 *      flag = 1:  Set the higher 24 bits of MAC address field for TRILL specific MAC
 * Parameters:
 *      unit      - unit number (driver internal).
 *      type     - The required switch control type to set upper MAC for
 *      mac     - MAC address
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_td_trill_mac_set(int unit, 
                            bcm_switch_control_t type,
                            uint32 mac_field,
                            uint8   flag)
{
    ing_trill_parse_control_entry_t  ing_ent;
    egr_trill_parse_control_entry_t egr_ent;
    egr_trill_parse_control_2_entry_t egr2_ent;
    ing_trill_payload_parse_control_entry_t ing_payload_ent;
    bcm_mac_t   mac;   

    sal_memset(mac, 0, sizeof(bcm_mac_t));    

    BCM_IF_ERROR_RETURN(
       soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &ing_ent));
    BCM_IF_ERROR_RETURN(
       soc_mem_read(unit, EGR_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &egr_ent));
    BCM_IF_ERROR_RETURN(
       soc_mem_read(unit, EGR_TRILL_PARSE_CONTROL_2m,
                       MEM_BLOCK_ALL, 0, &egr2_ent));
    BCM_IF_ERROR_RETURN(
       soc_mem_read(unit, ING_TRILL_PAYLOAD_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &ing_payload_ent));
   
    switch (type) {
         case bcmSwitchTrillISISDestMacOui:         
         case bcmSwitchTrillISISDestMacNonOui:
              BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &ing_ent));
              _bcm_td_trill_mac_set(unit, mac_field, ING_TRILL_PARSE_CONTROLm, 
                                       &ing_ent, TRILL_ALL_IS_IS_RBRIDGES_MAC_ADDRESSf, mac, flag);
              BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &ing_ent));

              BCM_IF_ERROR_RETURN(soc_mem_read(unit, EGR_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &egr_ent));
              sal_memset(mac, 0, sizeof(bcm_mac_t));
              _bcm_td_trill_mac_set(unit, mac_field, EGR_TRILL_PARSE_CONTROLm, 
                                       &egr_ent, TRILL_ALL_IS_IS_RBRIDGES_MAC_ADDRESSf, mac, flag);
              BCM_IF_ERROR_RETURN(soc_mem_write(unit, EGR_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &egr_ent));

              BCM_IF_ERROR_RETURN(soc_mem_read(unit, EGR_TRILL_PARSE_CONTROL_2m,
                                       MEM_BLOCK_ALL, 0, &egr2_ent));
              sal_memset(mac, 0, sizeof(bcm_mac_t));
              _bcm_td_trill_mac_set(unit, mac_field, EGR_TRILL_PARSE_CONTROL_2m, 
                                       &egr2_ent, TRILL_ALL_IS_IS_RBRIDGES_MAC_ADDRESSf, mac, flag);
              BCM_IF_ERROR_RETURN(soc_mem_write(unit, EGR_TRILL_PARSE_CONTROL_2m,
                                       MEM_BLOCK_ALL, 0, &egr2_ent));
              break;
        
        case bcmSwitchTrillBroadcastDestMacOui:
        case bcmSwitchTrillBroadcastDestMacNonOui: 
              BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &ing_ent));
              _bcm_td_trill_mac_set(unit, mac_field, ING_TRILL_PARSE_CONTROLm, 
                                       &ing_ent, TRILL_ALL_RBRIDGES_MAC_ADDRESSf, mac, flag);
              BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &ing_ent));

              BCM_IF_ERROR_RETURN(soc_mem_read(unit, EGR_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &egr_ent));
              sal_memset(mac, 0, sizeof(bcm_mac_t));
              _bcm_td_trill_mac_set(unit, mac_field, EGR_TRILL_PARSE_CONTROLm, 
                                       &egr_ent, TRILL_ALL_RBRIDGES_MAC_ADDRESSf, mac, flag);
              BCM_IF_ERROR_RETURN(soc_mem_write(unit, EGR_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &egr_ent));
              break;

        case bcmSwitchTrillEndStationDestMacOui:        
        case bcmSwitchTrillEndStationDestMacNonOui:
              BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_TRILL_PAYLOAD_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &ing_payload_ent));
              _bcm_td_trill_mac_set(unit, mac_field, ING_TRILL_PAYLOAD_PARSE_CONTROLm, 
                                       &ing_payload_ent, TRILL_ALL_ESADI_RBRIDGES_MAC_ADDRESSf, mac, flag);
              BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_TRILL_PAYLOAD_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &ing_payload_ent));
              break;

        default:
              return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_trill_mac_get
 * Description:
 *      flag = 0:  Get the lower 24 bits of MAC address field for TRILL specific MAC
 *      flag = 1:  Get the higher 24 bits of MAC address field for TRILL specific MAC
 * Parameters:
 *      unit       - unit number (driver internal).
 *      type      - The required switch control type to set upper MAC for
 *      mac      - MAC address
 * Returns:
 *      BCM_E_xxxx
 */

STATIC void
_bcm_td_trill_mac_get(int unit, soc_mem_t mem, 
                        void *entry, soc_field_t field, uint32 *mac_field, uint8 flag) 
{
    bcm_mac_t mac;

    soc_mem_mac_addr_get(unit, mem, entry, 
                        field, mac);

    if (flag) {
         /* read the high-order bytes from Memory */
         *mac_field = ((mac[0] << 16) | (mac[1] << 8)  | (mac[2] << 0));
    } else {
         /* read the low-order bytes from Memory */
         *mac_field = ((mac[3] << 16) | (mac[4] << 8)  | (mac[5] << 0));
    }
}


/*
 * Function:
 *      bcm_td_trill_mac_get
 * Description:
 *      flag = 0:  Get the lower 24 bits of MAC address field for TRILL specific MAC
 *      flag = 1:  Get the higher 24 bits of MAC address field for TRILL specific MAC
 * Parameters:
 *      unit      - unit number (driver internal).
 *      type     - The required switch control type to set upper MAC for
 *      mac     - MAC address
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_td_trill_mac_get(int unit, 
                            bcm_switch_control_t type,
                            uint32 *mac_field,
                            uint8   flag)
{
    ing_trill_parse_control_entry_t  ing_ent;
    egr_trill_parse_control_entry_t egr_ent;
    egr_trill_parse_control_2_entry_t egr2_ent;
    ing_trill_payload_parse_control_entry_t ing_payload_ent;

    sal_memset(&ing_ent, 0, sizeof(ing_trill_parse_control_entry_t));	
    sal_memset(&egr_ent, 0, sizeof(egr_trill_parse_control_entry_t));    
    sal_memset(&egr2_ent, 0, sizeof(egr_trill_parse_control_2_entry_t));
    sal_memset(&ing_payload_ent, 0, sizeof(ing_trill_payload_parse_control_entry_t));

    switch (type) {
         case bcmSwitchTrillISISDestMacOui:         
         case bcmSwitchTrillISISDestMacNonOui:
              BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &ing_ent));
              _bcm_td_trill_mac_get(unit, ING_TRILL_PARSE_CONTROLm, 
                                       &ing_ent, TRILL_ALL_IS_IS_RBRIDGES_MAC_ADDRESSf, mac_field, flag);

              break;
        
        case bcmSwitchTrillBroadcastDestMacOui:         
        case bcmSwitchTrillBroadcastDestMacNonOui: 
              BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &ing_ent));
              _bcm_td_trill_mac_get(unit, ING_TRILL_PARSE_CONTROLm, 
                                       &ing_ent, TRILL_ALL_RBRIDGES_MAC_ADDRESSf, mac_field, flag);

              break;

        case bcmSwitchTrillEndStationDestMacOui:        
        case bcmSwitchTrillEndStationDestMacNonOui:
              BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_TRILL_PAYLOAD_PARSE_CONTROLm,
                                       MEM_BLOCK_ALL, 0, &ing_payload_ent));
              _bcm_td_trill_mac_get(unit, ING_TRILL_PAYLOAD_PARSE_CONTROLm, 
                                       &ing_payload_ent, TRILL_ALL_ESADI_RBRIDGES_MAC_ADDRESSf, mac_field, flag);
              break;

        default:
              return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td_trill_MinTtl_set
 * Description:
 *       Set minimum TTL value for Transit and Egress Rbridge
 * Parameters:
 *      unit      - unit number (driver internal).
 *      min_ttl     - Minimum TTL value
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_td_trill_MinTtl_set(int unit, int min_ttl)
{
    int rv = BCM_E_NONE;
    ing_trill_parse_control_entry_t  ing_ent;

    sal_memset(&ing_ent, 0, sizeof(ing_trill_parse_control_entry_t));

    if (SOC_MEM_IS_VALID(unit, ING_TRILL_PARSE_CONTROLm)) {              
      if ((min_ttl > 0) && (min_ttl < 64)) {
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &ing_ent));
    
         if (soc_mem_field_valid(unit, ING_TRILL_PARSE_CONTROLm, 
                             MIN_INCOMING_HOPCOUNTf)) {
              soc_mem_field32_set(unit, ING_TRILL_PARSE_CONTROLm, &ing_ent,
                                         MIN_INCOMING_HOPCOUNTf, min_ttl);
         }

         BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_TRILL_PARSE_CONTROLm,
                                            MEM_BLOCK_ALL, 0, &ing_ent));
      }
    }
    return rv;
}

/*
 * Function:
 *      bcm_td_trill_MinTtl_get
 * Description:
 *       Get minimum TTL value for Transit and Egress Rbridge
 * Parameters:
 *      unit      - unit number (driver internal).
 *      min_ttl     - Minimum TTL value
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_td_trill_MinTtl_get(int unit, int *min_ttl)
{
    int rv = BCM_E_NONE;
    ing_trill_parse_control_entry_t  ing_ent;

    sal_memset(&ing_ent, 0, sizeof(ing_trill_parse_control_entry_t));

    if (SOC_MEM_IS_VALID(unit, ING_TRILL_PARSE_CONTROLm)) {
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &ing_ent));
    
         if (soc_mem_field_valid(unit, ING_TRILL_PARSE_CONTROLm, 
                             MIN_INCOMING_HOPCOUNTf)) {
              *min_ttl = soc_mem_field32_get(unit, ING_TRILL_PARSE_CONTROLm, 
                                            &ing_ent,
                                            MIN_INCOMING_HOPCOUNTf);
         }
    }
    return rv;
}

/*
 * Function:
 *      bcm_td_trill_TtlCheckEnable_set
 * Description:
 *       Set enable for Egress RBridge TTL Check 
 * Parameters:
 *      unit      - unit number (driver internal).
 *      enable     - TTL Check Enable
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_td_trill_TtlCheckEnable_set(int unit, int enable)
{
    int rv = BCM_E_NONE;
    ing_trill_parse_control_entry_t  ing_ent;

    sal_memset(&ing_ent, 0, sizeof(ing_trill_parse_control_entry_t));

    if (SOC_MEM_IS_VALID(unit, ING_TRILL_PARSE_CONTROLm)) {              
      if ((enable == TRUE) || (enable == FALSE)) {  
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &ing_ent));
    
         if (soc_mem_field_valid(unit, ING_TRILL_PARSE_CONTROLm, 
                             EGRESS_RBRIDGE_CHECK_HOPCOUNTf)) {
              soc_mem_field32_set(unit, ING_TRILL_PARSE_CONTROLm, &ing_ent,
                                         EGRESS_RBRIDGE_CHECK_HOPCOUNTf, enable);
         }

         BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_TRILL_PARSE_CONTROLm,
                                            MEM_BLOCK_ALL, 0, &ing_ent));
      }
    }
    return rv;

}

/*
 * Function:
 *      bcm_td_trill_TtlCheckEnable_get
 * Description:
 *       Get enable for Egress RBridge TTL Check 
 * Parameters:
 *      unit      - unit number (driver internal).
 *      enable     - TTL Check Enable
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_td_trill_TtlCheckEnable_get(int unit, int *enable)
{
    int rv = BCM_E_NONE;
    ing_trill_parse_control_entry_t  ing_ent;

    sal_memset(&ing_ent, 0, sizeof(ing_trill_parse_control_entry_t));

    if (SOC_MEM_IS_VALID(unit, ING_TRILL_PARSE_CONTROLm)) {
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, ING_TRILL_PARSE_CONTROLm,
                       MEM_BLOCK_ALL, 0, &ing_ent));
    
         if (soc_mem_field_valid(unit, ING_TRILL_PARSE_CONTROLm, 
                             EGRESS_RBRIDGE_CHECK_HOPCOUNTf)) {
              *enable = soc_mem_field32_get(unit, ING_TRILL_PARSE_CONTROLm, 
                                            &ing_ent,
                                            EGRESS_RBRIDGE_CHECK_HOPCOUNTf);
         }
    }
    return rv;
}

/*
 * Function:
 *      bcm_td_trill_allocate_bk
 * Purpose:
 *      Initialize the Trill software module
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
bcm_td_trill_allocate_bk(int unit)
{
    /* Allocate/Init unit software tables. */
    if (NULL == TRILL_INFO(unit)) {
        BCM_TD_TRILL_ALLOC(TRILL_INFO(unit), sizeof(_bcm_td_trill_bookkeeping_t),
                          "trill_bk_module_data");
        if (NULL == TRILL_INFO(unit)) {
            return (BCM_E_MEMORY);
        } else {
            TRILL_INFO(unit)->initialized = FALSE;
        }
    }
    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/* 
 * Function:
 *      bcm_td_trill_sync
 * Purpose:
 *      Store the multicast count Trill book keeping structure 
 *      onto persistent memory.
 * Parameters:
 *      unit    : (IN) Device Unit Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_td_trill_sync(int unit)
{
    int rv, alloc_sz = 0;
    uint8 *trill_state;
    soc_scache_handle_t scache_handle;
    _bcm_td_trill_bookkeeping_t *trill_info;

    trill_info = TRILL_INFO(unit);
    alloc_sz = sizeof(_bcm_td_trill_multicast_count_t) * BCM_MAX_TRILL_IPMC_GROUPS;   

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_TRILL, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 alloc_sz, &trill_state, 
                                 BCM_WB_DEFAULT_VERSION, NULL);
    if (BCM_FAILURE(rv) && (rv != BCM_E_INTERNAL)) {
        return rv;
    }
    
    sal_memcpy(trill_state, trill_info->multicast_count, alloc_sz);

    return rv;
}

/*
 * Function:
 *      _bcm_td_trill_reinit
 * Purpose:
 *      Perform recovery of Trill s/w state during warmboot.
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_trill_reinit(int unit)
{
    uint8 *trill_state;
    int alloc_sz, rv, stable_size = 0;     
    int idx, idx_min, idx_max;
    soc_scache_handle_t scache_handle;
    _bcm_td_trill_bookkeeping_t *trill_info;
    egr_dvp_attribute_entry_t egr_dvp_attribute;
    egr_trill_tree_profile_entry_t  egr_trill_profile;

    SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));

    if ((stable_size != 0) && (!SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit))) { 

        trill_info = TRILL_INFO(unit);
    
        /* Recover rBridge book keeping info */    
        idx_min = soc_mem_index_min(unit, EGR_DVP_ATTRIBUTEm);
        idx_max = soc_mem_index_max(unit, EGR_DVP_ATTRIBUTEm);

        for (idx = idx_min; idx <= idx_max; idx++) {
            SOC_IF_ERROR_RETURN(soc_mem_read(unit, EGR_DVP_ATTRIBUTEm,
                                    MEM_BLOCK_ANY, idx, &egr_dvp_attribute));
            if (soc_EGR_DVP_ATTRIBUTEm_field32_get(unit,
                         &egr_dvp_attribute, VP_TYPEf) != 0x1) {
                continue;
            }
            trill_info->rBridge[idx] = soc_EGR_DVP_ATTRIBUTEm_field32_get(unit,
                                  &egr_dvp_attribute, EGRESS_RBRIDGE_NICKNAMEf);
        }

        /* Recover rootBridge book keeping info */    
        idx_min = soc_mem_index_min(unit, EGR_TRILL_TREE_PROFILEm);
        idx_max = soc_mem_index_max(unit, EGR_TRILL_TREE_PROFILEm);

        for (idx = idx_min; idx <= idx_max; idx++) {
            SOC_IF_ERROR_RETURN(soc_mem_read(unit, EGR_TRILL_TREE_PROFILEm,
                                    MEM_BLOCK_ANY, idx, &egr_trill_profile));
            trill_info->rootBridge[idx] =
                           soc_EGR_TRILL_TREE_PROFILEm_field32_get(unit,
                                    &egr_trill_profile, ROOT_RBRIDGE_NICKNAMEf);
        }

        /* Recover multicast ref count book keeping info */    
        SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_TRILL, 0);
        alloc_sz = sizeof(_bcm_td_trill_multicast_count_t) * BCM_MAX_TRILL_IPMC_GROUPS;   
        rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                     alloc_sz, &trill_state, 
                                     BCM_WB_DEFAULT_VERSION, NULL);

        sal_memcpy(trill_info->multicast_count, trill_state, alloc_sz);
    }

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *      bcm_td_trill_init
 * Purpose:
 *      Initialize the Trill software module
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */


int
bcm_td_trill_init(int unit)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    int idx;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint8 *trill_state = 0;
    soc_scache_handle_t scache_handle;
    int  rv = BCM_E_NONE, alloc_sz = 0;
#endif /* BCM_WARM_BOOT_SUPPORT */

    if (!L3_INFO(unit)->l3_initialized) {
        soc_cm_debug(DK_ERR, "L3 module must be initialized prior to TRILL_init\n");
        return BCM_E_CONFIG;
    }

    /* Allocate BK Info */
    BCM_IF_ERROR_RETURN(bcm_td_trill_allocate_bk(unit));
    trill_info = TRILL_INFO(unit);

    /*
     * allocate resources
     */
    if (trill_info->initialized) {
         BCM_IF_ERROR_RETURN(bcm_td_trill_cleanup(unit));
         BCM_IF_ERROR_RETURN(bcm_td_trill_allocate_bk(unit));
         trill_info = TRILL_INFO(unit);
    }

    /* Create Trill protection mutex. */
    trill_info->trill_mutex = sal_mutex_create("trill_mutex");
    if (!trill_info->trill_mutex) {
         _bcm_td_trill_free_resource(unit);
         return BCM_E_MEMORY;
    }

    for (idx=0; idx < BCM_MAX_NUM_TRILL_TREES; idx++) {
         trill_info->rootBridge[idx] = 0;
    }

    for (idx=0; idx < BCM_MAX_NUM_VP; idx++) {
         trill_info->rBridge[idx] = 0;
    }

    for (idx=0; idx < BCM_MAX_TRILL_IPMC_GROUPS; idx++) {
       trill_info->multicast_count[idx].l3mc_group = 0;
       trill_info->multicast_count[idx].network_port_count = 0;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        if (BCM_FAILURE(_bcm_td_trill_reinit(unit))) {
            _bcm_td_trill_free_resource(unit);
        }
    } else { /* Coldboot */
        alloc_sz = sizeof(_bcm_td_trill_multicast_count_t) * BCM_MAX_TRILL_IPMC_GROUPS;   
        SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_TRILL, 0);
        rv = _bcm_esw_scache_ptr_get(unit, scache_handle, TRUE,
                                     alloc_sz, &trill_state, 
                                     BCM_WB_DEFAULT_VERSION, NULL);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    /* Mark the state as initialized */
    trill_info->initialized = TRUE;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_trill_hw_clear
 * Purpose:
 *     Perform hw tables clean up for Trill module. 
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */
 
STATIC int
_bcm_td_trill_hw_clear(int unit)
{
    int rv_error = BCM_E_NONE;

    return rv_error;
}

/*
 * Function:
 *      bcm_td_trill_cleanup
 * Purpose:
 *      DeInit  the Trill software module
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_td_trill_cleanup(int unit)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    int rv = BCM_E_UNAVAIL;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);

    if (FALSE == trill_info->initialized) {
        return (BCM_E_NONE);
    } 

    rv = bcm_td_trill_lock (unit);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    if (0 == SOC_HW_ACCESS_DISABLE(unit)) { 
        rv = _bcm_td_trill_hw_clear(unit);
    }

    /* Mark the state as uninitialized */
    trill_info->initialized = FALSE;

    sal_mutex_give(trill_info->trill_mutex);

    /* Destroy protection mutex. */
    sal_mutex_destroy(trill_info->trill_mutex );

    /* Free software resources */
    (void) _bcm_td_trill_free_resource(unit);

    return rv;
}

/*
 * Function:
 *		_bcm_td_trill_tree_profile_set
 * Purpose:
 *		Set TRILL Tree Profile
 * Parameters:
 *   IN :  Unit
 *   IN :  trill_name
 *   IN :  hopcount
 *   OUT: trill_tree_profile_idx
 * Returns:
 *		BCM_E_XXX
 */

int
_bcm_td_trill_tree_profile_set (int unit, uint16 trill_name, int hopcount, uint8 *trill_tree_profile_idx)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    egr_trill_tree_profile_entry_t  egr_trill_profile;
    int rv=BCM_E_UNAVAIL;
    uint8 idx;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);

    for (idx = 1; idx < BCM_MAX_NUM_TRILL_TREES; idx++) {
        if (trill_info->rootBridge[idx] == trill_name) {
              *trill_tree_profile_idx = idx;
              break;
         } else if (trill_info->rootBridge[idx] == 0) {
              *trill_tree_profile_idx = idx;
              break;
         }
    }

    if (idx == BCM_MAX_NUM_TRILL_TREES) {
         return  BCM_E_RESOURCE;
    }
    trill_info->rootBridge[*trill_tree_profile_idx] = trill_name;

    sal_memset(&egr_trill_profile, 0, sizeof(egr_trill_tree_profile_entry_t));	
    soc_mem_field32_set(unit, EGR_TRILL_TREE_PROFILEm, &egr_trill_profile,
                                            ROOT_RBRIDGE_NICKNAMEf, trill_name);
    soc_mem_field32_set(unit, EGR_TRILL_TREE_PROFILEm, &egr_trill_profile,
                                            HOPCOUNTf, hopcount);
    rv = soc_mem_write(unit, EGR_TRILL_TREE_PROFILEm,
                                            MEM_BLOCK_ALL, *trill_tree_profile_idx, &egr_trill_profile);

    return rv;
}

/*
 * Function:
 *		_bcm_td_trill_tree_profile_reset
 * Purpose:
 *		Reset TRILL Tree Profile
 * Parameters:
 *   IN :  Unit
 *   IN :  trill_Rbridge_nickname
 * Returns:
 *		BCM_E_XXX
 */

int
_bcm_td_trill_tree_profile_reset (int unit, uint16 trill_name)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    egr_trill_tree_profile_entry_t  egr_trill_profile;
    uint8 idx;
    int rv=BCM_E_UNAVAIL;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);

    for (idx = 1; idx < BCM_MAX_NUM_TRILL_TREES; idx++) {
         if (trill_info->rootBridge[idx] == trill_name) {
              trill_info->rootBridge[idx] = 0;
              sal_memset(&egr_trill_profile, 0, sizeof(egr_trill_tree_profile_entry_t));  
              rv = soc_mem_write(unit, EGR_TRILL_TREE_PROFILEm,
                                                 MEM_BLOCK_ALL, 
                                                 idx, &egr_trill_profile);
              return rv;
         }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *		bcm_td_trill_tree_profile_get
 * Purpose:
 *		Get TRILL Tree Profile index from trill_name
 * Parameters:
 *   IN :  Unit
 *   IN :  trill_tree_profile_idx
 * Returns:
 *		BCM_E_XXX
 */

STATIC void
bcm_td_trill_tree_profile_get (int unit, uint16 trill_name, uint8 *trill_tree_profile_idx)
{
    _bcm_td_trill_bookkeeping_t *trill_info = TRILL_INFO(unit);
    uint8 idx;

    for (idx = 1; idx < BCM_MAX_NUM_TRILL_TREES; idx++) {
         if (trill_info->rootBridge[idx] == trill_name) {
              *trill_tree_profile_idx = idx;
              break;
         }
    }
    if (idx == BCM_MAX_NUM_TRILL_TREES) {
         *trill_tree_profile_idx = 0;
    }
}

/*
 * Function:
 *      _bcm_td_trill_port_resolve
 * Purpose:
 *      Get the modid, port, trunk values for TRILL port
 * Parameters:
 *   IN  :  Unit
 *   IN  :  trill_port_id
 *   IN  :  encap_id
 *  OUT:  module_id
 *  OUT:  port
 *  OUT:  trunk_id
 *  OUT:  vp
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_td_trill_port_resolve(int unit, bcm_gport_t trill_port_id, 
                          bcm_if_t encap_id, bcm_module_t *modid, 
                          bcm_port_t *port, bcm_trunk_t *trunk_id, int *id)

{
    int rv = BCM_E_NONE;
    int ecmp, nh_index, nh_ecmp_index, vp;
    ing_l3_next_hop_entry_t ing_nh;
    egr_l3_next_hop_entry_t egr_nh;
    ing_dvp_table_entry_t dvp;
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry. */
    int  idx, max_ent_count, base_idx;

    _bcm_trill_check_init (unit);

    if (!BCM_GPORT_IS_TRILL_PORT(trill_port_id)) {
        return (BCM_E_BADID);
    }

    vp = BCM_GPORT_TRILL_PORT_ID_GET(trill_port_id);
    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {
        return BCM_E_NOT_FOUND;
    }
    BCM_IF_ERROR_RETURN(
         READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));
    ecmp = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, ECMPf);
    if (!ecmp) {
         nh_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp,
                                       NEXT_HOP_INDEXf);
         if (nh_index > 0) {
            BCM_IF_ERROR_RETURN (soc_mem_read(unit, ING_L3_NEXT_HOPm, MEM_BLOCK_ANY,
                                      nh_index, &ing_nh));

            if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, ENTRY_TYPEf) != 0x2) {
                 /* Entry type is not  TRILL DVP  */
                 return BCM_E_NOT_FOUND;
            }
            if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, Tf)) {
                 *trunk_id = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, TGIDf);
            } else {
                 /* Only add this to replication set if destination is local */
                *modid = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, MODULE_IDf);
                *port = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, PORT_NUMf);
            }
         }
    } else {
         /* Select the desired nhop from ECMP group - pointed by encap_id */
         nh_ecmp_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, ECMP_PTRf);
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, L3_ECMP_COUNTm, MEM_BLOCK_ANY, 
                        nh_ecmp_index, hw_buf));

         if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
              max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, COUNTf);
              base_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, BASE_PTRf);
         }  else {
              max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, COUNT_0f);
              base_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, BASE_PTR_0f);
         }
        max_ent_count++; /* Count is zero based. */ 

        if(encap_id == -1) {
              BCM_IF_ERROR_RETURN(
                   soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY, 
                        base_idx, hw_buf));
              nh_index = soc_mem_field32_get(unit, L3_ECMPm, 
                   hw_buf, NEXT_HOP_INDEXf);
              BCM_IF_ERROR_RETURN (soc_mem_read(unit, EGR_L3_NEXT_HOPm, 
                   MEM_BLOCK_ANY, nh_index, &egr_nh));
                 BCM_IF_ERROR_RETURN (soc_mem_read(unit, ING_L3_NEXT_HOPm, 
                        MEM_BLOCK_ANY, nh_index, &ing_nh));

                 if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, 
                             ENTRY_TYPEf) != 0x2) {
                        /* Entry type is not  TRILL DVP  */
                        return BCM_E_NOT_FOUND;
                 }
                 if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, Tf)) {
                     *trunk_id = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, TGIDf);
                 } else {
                     /* Only add this to replication set if destination is local */
                     *modid = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, MODULE_IDf);
                     *port = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, PORT_NUMf);
                }
        } else {
            for (idx = 0; idx < max_ent_count; idx++) {
              BCM_IF_ERROR_RETURN(
                   soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY, 
                        (base_idx+idx), hw_buf));
              nh_index = soc_mem_field32_get(unit, L3_ECMPm, 
                   hw_buf, NEXT_HOP_INDEXf);
              BCM_IF_ERROR_RETURN (soc_mem_read(unit, EGR_L3_NEXT_HOPm, 
                   MEM_BLOCK_ANY, nh_index, &egr_nh));
              if (encap_id == soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm, 
                                               &egr_nh, INTF_NUMf)) {
                 BCM_IF_ERROR_RETURN (soc_mem_read(unit, ING_L3_NEXT_HOPm, 
                        MEM_BLOCK_ANY, nh_index, &ing_nh));

                 if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, 
                             ENTRY_TYPEf) != 0x2) {
                        /* Entry type is not  TRILL DVP  */
                        return BCM_E_NOT_FOUND;
                 }
                 if (soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, Tf)) {
                     *trunk_id = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, TGIDf);
                 } else {
                     /* Only add this to replication set if destination is local */
                     *modid = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, MODULE_IDf);
                     *port = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, PORT_NUMf);
                }
                break;
              }
            }
        }
    }

    *id = vp;
    return rv;
}

/*
 * Function:
 *		_bcm_td_trill_transit_entry_key_set
 * Purpose:
 *		Set TRILL Transit forwarding entry key
 * Parameters:
 *		IN :  Unit
 *  IN :  trill_port
 *  IN : next_hop / ecmp group index
 *  IN : MPLS entry pointer
 * Returns:
 *		BCM_E_XXX
 */


STATIC void
_bcm_td_trill_transit_entry_key_set(int unit, bcm_trill_port_t *trill_port,
                                         int nh_ecmp_index, mpls_entry_entry_t   *tr_ent, int clean_flag)
{

    if (clean_flag) {
         sal_memset(tr_ent, 0, sizeof(mpls_entry_entry_t));
    }

    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         KEY_TYPEf, 0x5);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__RBRIDGE_NICKNAMEf, trill_port->name);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__CLASS_IDf, trill_port->if_class);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__DECAP_TRILL_TUNNELf, 0x0);
    if (trill_port->flags & BCM_TRILL_MULTIPATH) {
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__ECMPf, 0x1);
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__ECMP_PTRf, nh_ecmp_index);
    } else {
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__NEXT_HOP_INDEXf, nh_ecmp_index);
    }
    if (soc_mem_field_valid(unit, MPLS_ENTRYm, TRILL__UCAST_DST_DISCARDf) && 
        (trill_port->flags & BCM_TRILL_PORT_DROP)) {
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__UCAST_DST_DISCARDf, 0x1);
    }
    if (soc_mem_field_valid(unit, MPLS_ENTRYm, TRILL__DST_COPY_TO_CPUf) && 
        (trill_port->flags & BCM_TRILL_PORT_COPYTOCPU)) {
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__DST_COPY_TO_CPUf, 0x1);
    }
    if (trill_port->flags & BCM_TRILL_PORT_MULTICAST) {
        if (soc_mem_field_valid(unit, MPLS_ENTRYm, TRILL__MCAST_DST_DISCARDf) && 
            (trill_port->flags & BCM_TRILL_PORT_DROP)) {
                soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__MCAST_DST_DISCARDf, 0x1);
        }
    }
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         VALIDf, 0x1);
}


/*
 * Function:
 *      _bcm_td_trill_transit_entry_set
 * Purpose:
 *     Set TRILL Transit forwarding entry
 * Parameters:
 *      IN :  Unit
 *      IN :  trill_port
 *      IN : next_hop / ecmp group index
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_td_trill_transit_entry_set(int unit, bcm_trill_port_t *trill_port,
                                         int nh_ecmp_index)
{
    int rv = BCM_E_NONE;
    int index;
    mpls_entry_entry_t  key_ent;
    mpls_entry_entry_t  return_ent;

    _bcm_td_trill_transit_entry_key_set(unit, trill_port, 
                        nh_ecmp_index, &key_ent, 1);

    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                        &key_ent, &return_ent, 0);
    if (rv == SOC_E_NONE) {
         (void) _bcm_td_trill_transit_entry_key_set(unit, trill_port, 
                                  nh_ecmp_index, &return_ent, 0);
         rv = soc_mem_write(unit, MPLS_ENTRYm,
                                  MEM_BLOCK_ALL, index,
                                  &return_ent);
    } else if (rv != SOC_E_NOT_FOUND) {
        return rv;
    } else {
         rv = soc_mem_insert(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, &key_ent);
    }
    return rv;
}

/*
 * Function:
 *		_bcm_td_trill_transit_entry_reset
 * Purpose:
 *		Reset TRILL Transit forwarding entry
 * Parameters:
 *		IN :  Unit
 *  IN :  vp
 *  IN : next_hop / ecmp group index
 *  IN : ecmp flag
 * Returns:
 *		BCM_E_XXX
 */

STATIC int
_bcm_td_trill_transit_entry_reset(int unit, int vp, int nh_ecmp_idx, int ecmp)
{
    int rv = BCM_E_UNAVAIL;
    int index;
    bcm_trill_name_t rb_name;      /* Destination RBridge Nickname. */
    int if_class;               /* Interface class ID. */
    mpls_entry_entry_t  tr_ent;
    egr_dvp_attribute_entry_t  egr_dvp_attribute;
    source_vp_entry_t svp;

    sal_memset(&tr_ent, 0, sizeof(mpls_entry_entry_t));
 
    BCM_IF_ERROR_RETURN(
         soc_mem_read(unit, EGR_DVP_ATTRIBUTEm,
                                         MEM_BLOCK_ALL, vp, &egr_dvp_attribute));

    rb_name = soc_mem_field32_get(unit, EGR_DVP_ATTRIBUTEm, &egr_dvp_attribute,
                                         EGRESS_RBRIDGE_NICKNAMEf);

    BCM_IF_ERROR_RETURN(
         soc_mem_read(unit, SOURCE_VPm,
                                         MEM_BLOCK_ALL, vp, &svp));

    if_class = soc_mem_field32_get(unit, SOURCE_VPm, &svp, CLASS_IDf);


    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                                      KEY_TYPEf, 0x5);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                                      TRILL__RBRIDGE_NICKNAMEf, rb_name);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                                      TRILL__CLASS_IDf, if_class);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                                      TRILL__DECAP_TRILL_TUNNELf, 0x0);
    if (ecmp) {
      soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                                      TRILL__ECMPf, 0x1);
      soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                                      TRILL__ECMP_PTRf, nh_ecmp_idx);
    } else {
      soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                                      TRILL__NEXT_HOP_INDEXf, nh_ecmp_idx);
    }
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                                      VALIDf, 0x1);
    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                     &tr_ent, &tr_ent, 0);
    if (rv < 0) {
         return rv;
    }
    /* Delete the entry from HW */
    rv = soc_mem_delete(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, &tr_ent);
    return rv;
}

/*
 * Function:
 *		_bcm_td_trill_learn_entry_key_set
 * Purpose:
 *		Set TRILL learn entry key
 * Parameters:
 *		IN :  Unit
 *  IN :  trill_port
 *  IN :  VP
 *  IN : MPLS entry pointer
 * Returns:
 *		BCM_E_XXX
 */

STATIC void
 _bcm_td_trill_learn_entry_key_set(int unit, bcm_trill_port_t *trill_port,
                                          int vp, mpls_entry_entry_t   *tr_ent, int clean_flag)
 {
    uint8 trill_tree_id=0;

    if (clean_flag) {
         sal_memset(tr_ent, 0, sizeof(mpls_entry_entry_t));
    }

    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         KEY_TYPEf, 0x5);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__RBRIDGE_NICKNAMEf, trill_port->name);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__CLASS_IDf, trill_port->if_class);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__VIRTUAL_PORTf, vp);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__PHB_FROM_OUTER_L2_HEADERf, 0x1);
    if (trill_port->flags & BCM_TRILL_PORT_MULTICAST) {
         (void) bcm_td_trill_tree_profile_get (unit, trill_port->name, &trill_tree_id);
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__TREE_IDf, trill_tree_id);
    }
    if (soc_mem_field_valid(unit, MPLS_ENTRYm, TRILL__SRC_DISCARDf) && 
        (trill_port->flags & BCM_TRILL_PORT_DROP)) {
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__SRC_DISCARDf, 0x1);
    }
    if (soc_mem_field_valid(unit, MPLS_ENTRYm, TRILL__SRC_COPY_TO_CPUf) && 
        (trill_port->flags & BCM_TRILL_PORT_COPYTOCPU)) {
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__SRC_COPY_TO_CPUf, 0x1);
    }
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         VALIDf, 0x1);
 }

/*
 * Function:
 *		_bcm_td_trill_learn_entry_set
 * Purpose:
 *		Set TRILL  Learn Entry 
 * Parameters:
 *		IN :  Unit
 *  IN :  trill_port
 *  IN : vp
 * Returns:
 *		BCM_E_XXX
 */

STATIC int
_bcm_td_trill_learn_entry_set(int unit, bcm_trill_port_t *trill_port, int vp)
{
    mpls_entry_entry_t  key_ent;
    mpls_entry_entry_t  return_ent;
    int rv = BCM_E_UNAVAIL;
    int index;

    _bcm_td_trill_learn_entry_key_set(unit, trill_port, vp, &key_ent, 1);

    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                        &key_ent, &return_ent, 0);

    if (rv == SOC_E_NONE) {
         (void) _bcm_td_trill_learn_entry_key_set(unit, trill_port, vp, &return_ent, 0);
         rv = soc_mem_write(unit, MPLS_ENTRYm,
                                           MEM_BLOCK_ALL, index,
                                           &return_ent);
    } else if (rv != SOC_E_NOT_FOUND) {
        return rv;
    } else {
         rv = soc_mem_insert(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, &key_ent);
    }

    return rv;
}

/*
 * Function:
 *		_bcm_td_trill_learn_entry_reset
 * Purpose:
 *		Reset TRILL RPF entry
 * Parameters:
 *		IN :  Unit
 *  IN :  vp
 * Returns:
 *		BCM_E_XXX
 */

STATIC int
_bcm_td_trill_learn_entry_reset(int unit, int vp)
{
    int rv = BCM_E_UNAVAIL;
    int index;
    bcm_trill_name_t rb_name;      /* Destination RBridge Nickname. */
    int if_class;               /* Interface class ID. */
    mpls_entry_entry_t  tr_ent;
    egr_dvp_attribute_entry_t  egr_dvp_attribute;
    source_vp_entry_t svp;

    sal_memset(&tr_ent, 0, sizeof(mpls_entry_entry_t));

    BCM_IF_ERROR_RETURN(
         soc_mem_read(unit, EGR_DVP_ATTRIBUTEm,
                             MEM_BLOCK_ALL, vp, &egr_dvp_attribute));

    rb_name = soc_mem_field32_get(unit, EGR_DVP_ATTRIBUTEm, &egr_dvp_attribute,
                             EGRESS_RBRIDGE_NICKNAMEf);

    BCM_IF_ERROR_RETURN(
         soc_mem_read(unit, SOURCE_VPm,
                             MEM_BLOCK_ALL, vp, &svp));

    if_class = soc_mem_field32_get(unit, SOURCE_VPm, &svp, CLASS_IDf);

    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                             KEY_TYPEf, 0x5);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                             TRILL__RBRIDGE_NICKNAMEf, rb_name);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                             TRILL__CLASS_IDf, if_class);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                             TRILL__VIRTUAL_PORTf, vp);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                             TRILL__PHB_FROM_OUTER_L2_HEADERf, 0x1);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                             VALIDf, 0x1);

    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                             &tr_ent, &tr_ent, 0);
    if ( (rv != BCM_E_NOT_FOUND) && (rv != BCM_E_NONE) ) {
         return rv;
    }
    /* Delete the entry from HW */
    rv = soc_mem_delete(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, &tr_ent);
    
    if ( (rv != BCM_E_NOT_FOUND) && (rv != BCM_E_NONE) ) {
       return rv;
    } else {
       return BCM_E_NONE;
    }
}


/*
 * Function:
 *		_bcm_td_trill_decap_entry_key_set
 * Purpose:
 *		Set TRILL Decap entry key
 * Parameters:
 *		IN :  Unit
 *  IN :  trill_port
 *  IN :  VP
 *  IN : MPLS entry pointer
 * Returns:
 *		BCM_E_XXX
 */

STATIC void
 _bcm_td_trill_decap_entry_key_set(int unit, bcm_trill_port_t *trill_port,
                                          mpls_entry_entry_t   *tr_ent, int clean_flag)
 {
    if (clean_flag) {
       sal_memset(tr_ent, 0, sizeof(mpls_entry_entry_t));
    }

    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         KEY_TYPEf, 0x5);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__RBRIDGE_NICKNAMEf, trill_port->name);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__DECAP_TRILL_TUNNELf, 0x1);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__CLASS_IDf, trill_port->if_class);
    if (soc_mem_field_valid(unit, MPLS_ENTRYm, TRILL__UCAST_DST_DISCARDf) && 
        (trill_port->flags & BCM_TRILL_PORT_DROP)) {
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__UCAST_DST_DISCARDf, 0x1);
    }
    if (soc_mem_field_valid(unit, MPLS_ENTRYm, TRILL__DST_COPY_TO_CPUf) && 
        (trill_port->flags & BCM_TRILL_PORT_COPYTOCPU)) {
         soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__DST_COPY_TO_CPUf, 0x1);
    }
    if (trill_port->flags & BCM_TRILL_PORT_MULTICAST) {
        if (soc_mem_field_valid(unit, MPLS_ENTRYm, TRILL__MCAST_DST_DISCARDf) && 
            (trill_port->flags & BCM_TRILL_PORT_DROP)) {
                soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         TRILL__MCAST_DST_DISCARDf, 0x1);
        }
    }
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                         VALIDf, 0x1);
 }

/*
 * Function:
 *           _bcm_td_trill_decap_entry_set
 * Purpose:
 *           Set TRILL  Decap  Entry 
 * Parameters:
 *           IN :  Unit
 *  IN :  trill_port
 * Returns:
 *           BCM_E_XXX
 */

STATIC int
_bcm_td_trill_decap_entry_set(int unit, bcm_trill_port_t *trill_port)
{
    mpls_entry_entry_t  tr_ent;
    int rv = BCM_E_UNAVAIL;
    int index;
    mpls_entry_entry_t  return_ent;

    _bcm_td_trill_decap_entry_key_set(unit, trill_port, &tr_ent, 1);

    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                        &tr_ent, &return_ent, 0);

    if (rv == SOC_E_NONE) {
         (void) _bcm_td_trill_decap_entry_key_set(unit, trill_port, &return_ent, 0);
         rv = soc_mem_write(unit, MPLS_ENTRYm,
                                  MEM_BLOCK_ALL, index,
                                  &return_ent);
    } else if (rv != SOC_E_NOT_FOUND) {
        return rv;
    } else {
        rv = soc_mem_insert(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, &tr_ent);
    }
    return rv;
}

/*
 * Function:
 *           _bcm_td_trill_decap_entry_reset
 * Purpose:
 *           Reset TRILL Decap entry
 * Parameters:
 *            IN :  Unit
 *  IN :  vp
 * Returns:
 *            BCM_E_XXX
 */

STATIC int
_bcm_td_trill_decap_entry_reset(int unit,  int vp)
{
    int rv = BCM_E_UNAVAIL;
    int index;
    bcm_trill_name_t rb_name;      /* Destination RBridge Nickname. */
    int if_class;               /* Interface class ID. */
    mpls_entry_entry_t  tr_ent;
    egr_dvp_attribute_entry_t  egr_dvp_attribute;
    source_vp_entry_t svp;
 
    sal_memset(&tr_ent, 0, sizeof(mpls_entry_entry_t));

    BCM_IF_ERROR_RETURN(
         soc_mem_read(unit, EGR_DVP_ATTRIBUTEm,
                         MEM_BLOCK_ALL, vp, &egr_dvp_attribute));

    rb_name = soc_mem_field32_get(unit, EGR_DVP_ATTRIBUTEm, &egr_dvp_attribute,
                         EGRESS_RBRIDGE_NICKNAMEf);
 
    BCM_IF_ERROR_RETURN(
         soc_mem_read(unit, SOURCE_VPm,
                          MEM_BLOCK_ALL, vp, &svp));
 
    if_class = soc_mem_field32_get(unit, SOURCE_VPm, &svp, CLASS_IDf);
 
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                          KEY_TYPEf, 0x5);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                          TRILL__RBRIDGE_NICKNAMEf, rb_name);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                          TRILL__CLASS_IDf, if_class);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                                         TRILL__DECAP_TRILL_TUNNELf, 0x1);
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                          VALIDf, 0x1);
 
    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                          &tr_ent, &tr_ent, 0);
    if (rv < 0) {
         return rv;
    }

    /* Delete the entry from HW */
    rv = soc_mem_delete(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, &tr_ent);
    return rv;

}

/*
 * Function:
 *           bcm_td_trill_egress_set
 * Purpose:
 *           Set TRILL NextHop 
 * Parameters:
 *           IN :  Unit
 *           IN :  nh_index
 * Returns:
 *           BCM_E_XXX
 */

int
bcm_td_trill_egress_set(int unit, int nh_index)
{
    ing_l3_next_hop_entry_t ing_nh;

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_L3_NEXT_HOPm, 
                                  MEM_BLOCK_ANY, nh_index, &ing_nh));

    soc_mem_field32_set(unit, ING_L3_NEXT_HOPm, &ing_nh,
                                  ENTRY_TYPEf, 0x2); /* L2 DVP */
    if (SOC_MEM_FIELD_VALID(unit, ING_L3_NEXT_HOPm, MTU_SIZEf)) {
        soc_mem_field32_set(unit, ING_L3_NEXT_HOPm,
                &ing_nh, MTU_SIZEf, 0x3fff);
    }
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_L3_NEXT_HOPm,
                                  MEM_BLOCK_ALL, nh_index, &ing_nh));
    return BCM_E_NONE;
}

/*
 * Function:
 *           bcm_td_trill_egress_get
 * Purpose:
 *           Get TRILL Egress NextHop 
 * Parameters:
 *           IN :  Unit
 *           IN :  nh_index
 * Returns:
 *           BCM_E_XXX
 */

int
bcm_td_trill_egress_get(int unit, bcm_l3_egress_t *egr, int nh_index)
{
    ing_l3_next_hop_entry_t ing_nh;

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_L3_NEXT_HOPm, 
                                  MEM_BLOCK_ANY, nh_index, &ing_nh));

    if (0x2 == soc_mem_field32_get(unit, ING_L3_NEXT_HOPm, &ing_nh,
                                  ENTRY_TYPEf)) {
         egr->flags |= BCM_L3_TRILL_ONLY;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *            _bcm_td_trill_ingress_next_hop_set
 * Purpose:
 *           Set ING_L3_NEXT_HOP Entry 
 * Parameters:
 *           IN :  Unit
 *           IN :  nh_index
 *           IN : drop
 * Returns:
 *           BCM_E_XXX
 */

STATIC int
_bcm_td_trill_next_hop_set(int unit, int nh_index, bcm_trill_port_t *trill_port)
{
    int rv=BCM_E_UNAVAIL;
    ing_l3_next_hop_entry_t ing_nh;
    bcm_trunk_t      trunk_id;
    bcm_port_t      port;
    int drop=0;

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_L3_NEXT_HOPm, 
                                  MEM_BLOCK_ANY, nh_index, &ing_nh));

    drop = (trill_port->flags & BCM_TRILL_PORT_DROP) ? 1 : 0;

    soc_mem_field32_set(unit, ING_L3_NEXT_HOPm, &ing_nh,
                                            DROPf, drop);
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_L3_NEXT_HOPm,
                                            MEM_BLOCK_ALL, nh_index, &ing_nh));

    if (soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                              &ing_nh, Tf)) {
         trunk_id = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                            &ing_nh, TGIDf);
         /* Get all ports from Trunk_id. Fill EGR_register with NHI */
    } else {
         port = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                            &ing_nh, PORT_NUMf);
         rv = soc_reg_field32_modify(unit, EGR_PORT_TO_NHI_MAPPINGr,
                                            port, NEXT_HOP_INDEXf, nh_index);
    }

    return rv;
}

/*
 * Function:
 *		_bcm_td_trill_ingress_next_hop_reset
 * Purpose:
 *		Reset ING_L3_NEXT_HOP Entry 
 * Parameters:
 *		IN :  Unit
 *           IN :  nh_index
 *           IN : drop
 * Returns:
 *		BCM_E_XXX
 */

STATIC int
_bcm_td_trill_next_hop_reset(int unit, int nh_index)
{
    int rv=BCM_E_UNAVAIL;
    ing_l3_next_hop_entry_t ing_nh;
    bcm_trunk_t      trunk_id;
    bcm_port_t      port;

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_L3_NEXT_HOPm, 
                                  MEM_BLOCK_ANY, nh_index, &ing_nh));

    soc_mem_field32_set(unit, ING_L3_NEXT_HOPm, &ing_nh,
                                            DROPf, 0x0);
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, ING_L3_NEXT_HOPm,
                                            MEM_BLOCK_ALL, nh_index, &ing_nh));

    if (soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                              &ing_nh, Tf)) {
         trunk_id = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                            &ing_nh, TGIDf);
         /* Get all ports from Trunk_id. Fill EGR_register with NHI */
    } else {
         port = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                            &ing_nh, PORT_NUMf);
         rv = soc_reg_field32_modify(unit, EGR_PORT_TO_NHI_MAPPINGr,
                                            port, NEXT_HOP_INDEXf, 0x0);
    }

    return rv;
}

/*
 * Function:
 *     _bcm_td_trill_egress_name_set
 * Purpose:
 *     Set My RBridge Nickname
 * Parameters:
 *   IN :  Unit
 *   IN :  trill_port
 * Returns:
 *   BCM_E_XXX
 */

STATIC int
_bcm_td_trill_egress_name_set(int unit, bcm_trill_name_t  trill_name)
{
    egr_trill_rbridge_nicknames_entry_t  egr_name_entry;
    int idx;
    bcm_trill_name_t  temp_name;

      for(idx=0; idx < 4; idx++) {
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, EGR_TRILL_RBRIDGE_NICKNAMESm,
                       MEM_BLOCK_ALL, idx, &egr_name_entry));

         temp_name = 
              soc_mem_field32_get(unit, EGR_TRILL_RBRIDGE_NICKNAMESm, 
                                         &egr_name_entry,
                                         RBRIDGE_NICKNAMEf);
         if (!temp_name) {
              soc_mem_field32_set(unit, EGR_TRILL_RBRIDGE_NICKNAMESm, 
                                         &egr_name_entry,
                                         RBRIDGE_NICKNAMEf, trill_name);

              BCM_IF_ERROR_RETURN(soc_mem_write(unit, EGR_TRILL_RBRIDGE_NICKNAMESm,
                                            MEM_BLOCK_ALL, idx, &egr_name_entry));
              return BCM_E_NONE;
         }
      }
      return BCM_E_FULL;
}

/*
 * Function:
 *     _bcm_td_trill_egress_dvp_set
 * Purpose:
 *     Set Egress DVP tables
 * Parameters:
 *   IN :  Unit
 *   IN :  vp
 *   IN :  trill_port
 * Returns:
 *   BCM_E_XXX
 */

STATIC int
_bcm_td_trill_egress_dvp_set(int unit, int vp, bcm_trill_port_t *trill_port)
{
    int rv=BCM_E_UNAVAIL;
    egr_dvp_attribute_entry_t  egr_dvp_attribute;
    
    sal_memset(&egr_dvp_attribute, 0, sizeof(egr_dvp_attribute_entry_t));
    soc_mem_field32_set(unit, EGR_DVP_ATTRIBUTEm, &egr_dvp_attribute,
                                            VP_TYPEf, 0x1);
    soc_mem_field32_set(unit, EGR_DVP_ATTRIBUTEm, &egr_dvp_attribute,
                                            EGRESS_RBRIDGE_NICKNAMEf, trill_port->name);
    soc_mem_field32_set(unit, EGR_DVP_ATTRIBUTEm, &egr_dvp_attribute,
                                            HOPCOUNTf, trill_port->hopcount);
    soc_mem_field32_set(unit, EGR_DVP_ATTRIBUTEm, &egr_dvp_attribute,
                                            MTU_VALUEf, trill_port->mtu);
    soc_mem_field32_set(unit, EGR_DVP_ATTRIBUTEm, &egr_dvp_attribute,
                                            MTU_ENABLEf, 0x1);
    rv = soc_mem_write(unit, EGR_DVP_ATTRIBUTEm,
                                            MEM_BLOCK_ALL, vp, &egr_dvp_attribute);
    return rv;

}

/*
 * Function:
 *		_bcm_td_trill_egress_dvp_reset
 * Purpose:
 *		Set Egress DVP tables
 * Parameters:
 *		IN :  Unit
 *           IN :  nh_index
 *           IN :  trill_port
 * Returns:
 *		BCM_E_XXX
 */

STATIC int
_bcm_td_trill_egress_dvp_reset(int unit, int vp)
{
    int rv=BCM_E_UNAVAIL;
    egr_dvp_attribute_entry_t  egr_dvp_attribute;

    
    sal_memset(&egr_dvp_attribute, 0, sizeof(egr_dvp_attribute_entry_t));	
    rv = soc_mem_write(unit, EGR_DVP_ATTRIBUTEm,
                                            MEM_BLOCK_ALL, vp, &egr_dvp_attribute);
    return rv;
}

/*
 * Purpose:
 *		Set Egress TRILL Header fields
 * Parameters:
 *		IN :  Unit
 *           IN :  trill_name
 * Returns:
 *		BCM_E_XXX
 */

STATIC int
_bcm_td_trill_header_set(int unit, bcm_trill_name_t  trill_name)
{
    uint32  reg_val=0;

    soc_reg_field_set(unit, EGR_TRILL_HEADER_ATTRIBUTESr, 
                             &reg_val, RESERVED_FIELDf, 0);
    soc_reg_field_set(unit, EGR_TRILL_HEADER_ATTRIBUTESr, 
                             &reg_val, VERSIONf, 0);
    if (soc_mem_field_valid(unit, EGR_TRILL_RBRIDGE_NICKNAMESm, RBRIDGE_NICKNAMEf)) {
        _bcm_td_trill_egress_name_set(unit, trill_name);
    } else {
        soc_reg_field_set(unit, EGR_TRILL_HEADER_ATTRIBUTESr, 
                             &reg_val, RBRIDGE_NICKNAMEf, trill_name);
    }

    SOC_IF_ERROR_RETURN(
              WRITE_EGR_TRILL_HEADER_ATTRIBUTESr(unit, reg_val));
    return BCM_E_NONE;
}


/*
 * Function:
 *		_bcm_td_trill_adjacency_register_set
 * Purpose:
 *		Set Ingress TRILL Adjacency register
 * Parameters:
 *		IN :  Unit
 *   IN :  port
 *   IN :  mac_addr
 *   IN : vid
 * Returns:
 *		BCM_E_XXX
 */

STATIC int
_bcm_td_trill_adjacency_register_set(int unit, bcm_port_t  port, bcm_mac_t dst_mac_addr, bcm_vlan_t vid)
{
    uint64 reg_val;  
    uint64 mac_field;

    COMPILER_64_ZERO(reg_val);
    COMPILER_64_ZERO(mac_field);

    SAL_MAC_ADDR_TO_UINT64(dst_mac_addr, mac_field);

    soc_reg64_field32_set(unit, ING_TRILL_ADJACENCYr, &reg_val, 
                                  VLANf, vid);
    soc_reg64_field_set(unit, ING_TRILL_ADJACENCYr, &reg_val, 
                                  MAC_ADDRESSf, mac_field);
    SOC_IF_ERROR_RETURN
              (WRITE_ING_TRILL_ADJACENCYr(unit, port, reg_val));
    return BCM_E_NONE;
}

/*
 * Function:
 *   _bcm_td_trill_adjacency_register_reset
 * Purpose:
 *   Reset Ingress TRILL Adjacency register
 * Parameters:
 *   IN :  Unit
 *   IN :  port
 * Returns:
 *   BCM_E_XXX
 */

STATIC int
_bcm_td_trill_adjacency_register_reset(int unit, bcm_port_t  port)
{
    uint64 reg_val;  
    uint64 mac_field;
    int id;
    bcm_mac_t dst_mac_addr;

    COMPILER_64_ZERO(reg_val);
    COMPILER_64_ZERO(mac_field);
    for (id=0; id<6; id++) {
        dst_mac_addr[id] = 0x0;
    }

    SAL_MAC_ADDR_TO_UINT64(dst_mac_addr, mac_field);

    soc_reg64_field32_set(unit, ING_TRILL_ADJACENCYr, &reg_val, 
                                  VLANf, 0x0);
    soc_reg64_field_set(unit, ING_TRILL_ADJACENCYr, &reg_val, 
                                  MAC_ADDRESSf, mac_field);
    SOC_IF_ERROR_RETURN
              (WRITE_ING_TRILL_ADJACENCYr(unit, port, reg_val));
    return BCM_E_NONE;
}

/*
 * Function:
 *   _bcm_td_trill_adjacency_set
 * Purpose:
 *   Set TRILL Adjacency
 * Parameters:
 *   IN :  Unit
 *   IN :  nh_index
 *   IN :  trill_port
 * Returns:
 *   BCM_E_XXX
 */

STATIC int
_bcm_td_trill_adjacency_set(int unit, int nh_index)
{
    int rv=BCM_E_UNAVAIL;
    ing_l3_next_hop_entry_t ing_nh;
    egr_l3_next_hop_entry_t egr_nh;
    egr_l3_intf_entry_t if_entry;
    bcm_if_t  intf_idx;
    bcm_trunk_t     trunk_id;
    bcm_port_t      port;
    bcm_mac_t src_mac_addr;
    bcm_mac_t dst_mac_addr;
    bcm_vlan_t vid;


   BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_L3_NEXT_HOPm, 
                                 MEM_BLOCK_ANY, nh_index, &ing_nh));

   BCM_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_NEXT_HOPm, 
                                 MEM_BLOCK_ANY, nh_index, &egr_nh));

   intf_idx = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm, 
                                  &egr_nh, INTF_NUMf);

   BCM_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_INTFm, 
                                 MEM_BLOCK_ANY, intf_idx, &if_entry));

   /* Obtain Source-MAC, Vlan from EGR_L3_INTF */
   soc_mem_mac_addr_get(unit, EGR_L3_INTFm, &if_entry, 
                                            MAC_ADDRESSf, src_mac_addr);
   vid = soc_mem_field32_get(unit, EGR_L3_INTFm, 
                                            &if_entry,  VIDf);

   /* Obtain vlan and Destination-MAC from EGR_L3_NEXT_HOP */
   soc_mem_mac_addr_get(unit, EGR_L3_NEXT_HOPm, &egr_nh, 
                                            MAC_ADDRESSf, dst_mac_addr);

   if (soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                             &ing_nh, Tf)) {
        trunk_id = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                           &ing_nh, TGIDf);
        /* Get all ports from Trunk_id. For all ports, configure MY_STATION entry and TRILL adjacency */
   } else { 
        port = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                           &ing_nh, PORT_NUMf);

         /* Configure MY_STATION entry */
         BCM_IF_ERROR_RETURN(
              bcm_td_metro_myStation_add(unit, src_mac_addr, vid, port));
        
        /* Configure ING_TRILL_ADJACENCY */
        rv =  _bcm_td_trill_adjacency_register_set(unit, port, dst_mac_addr, vid);
   }
   return rv;
}

/*
 * Function:
 *		_bcm_td_trill_adjacency_reset
 * Purpose:
 *		Reset TRILL Adjacency
 * Parameters:
 *		IN :  Unit
 *   IN :  nh_index
 * Returns:
 *		BCM_E_XXX
 */

STATIC int
_bcm_td_trill_adjacency_reset(int unit, int nh_index)
{
    int rv=BCM_E_UNAVAIL;
    ing_l3_next_hop_entry_t ing_nh;
    egr_l3_next_hop_entry_t egr_nh;
    egr_l3_intf_entry_t if_entry;
    bcm_if_t  intf_idx;
    bcm_trunk_t      trunk_id;
    bcm_port_t      port;
    bcm_mac_t src_mac_addr;
    bcm_vlan_t vid;

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_L3_NEXT_HOPm, 
                               MEM_BLOCK_ANY, nh_index, &ing_nh));

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_NEXT_HOPm, 
                               MEM_BLOCK_ANY, nh_index, &egr_nh));
 
    intf_idx = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm, 
                                &egr_nh, INTF_NUMf);
 
    BCM_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_INTFm, 
                               MEM_BLOCK_ANY, intf_idx, &if_entry));
 
    /* Obtain Source-MAC from EGR_L3_INTF */
    soc_mem_mac_addr_get(unit, EGR_L3_INTFm, &if_entry, 
                                          MAC_ADDRESSf, src_mac_addr);
 
    /* Obtain vlan and Destination-MAC from EGR_L3_NEXT_HOP */
    vid = soc_mem_field32_get(unit, EGR_L3_INTFm, 
                                          &if_entry, OVIDf);
 
    if (soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                           &ing_nh, Tf)) {
         trunk_id = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                         &ing_nh, TGIDf);
      /* Get all ports from Trunk_id. For all ports, Delete MY_STATION entry and TRILL adjacency */
    } else { 
         port = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm,
                                         &ing_nh, PORT_NUMf);

         /* Delete MY_STATION entry */
         rv = bcm_td_metro_myStation_delete(unit, src_mac_addr, vid, port);
         if ((rv != BCM_E_NOT_FOUND) || (rv != BCM_E_FULL)) {
             if (rv != BCM_E_NONE) {
               return rv;
             }
         }

         /* Reset ING_TRILL_ADJACENCY */
         rv = _bcm_td_trill_adjacency_register_reset(unit, port);
    }
    return rv;
}


/*
 * Function:
 *		_bcm_td_trill_port_validate
 * Purpose:
 *		Validate Trill Port Parameters
 * Parameters:
 *   IN :  trill_port
 * Returns:
 *		BCM_E_XXX
 */

int
_bcm_td_trill_port_validate(bcm_trill_port_t *trill_port)
{

    if (!trill_port->name) {
         return BCM_E_PARAM;
    }

    /* Check for unsupported Flag */
    if (trill_port->flags & (~(BCM_TRILL_MULTIPATH |
                        BCM_TRILL_PORT_WITH_ID |
                        BCM_TRILL_PORT_DROP |
                        BCM_TRILL_PORT_COUNTED |                        
                        BCM_TRILL_PORT_NETWORK |
                        BCM_TRILL_PORT_REPLACE |
                        BCM_TRILL_PORT_MULTICAST |
                        BCM_TRILL_PORT_LOCAL  ))) {
         return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

 /*
  * Function:
  * 	 _bcm_td_trill_loopback_enable
  * Purpose:
  * 	 Enable loopback for TRILL Multicast
  * Parameters:
  *   IN :	unit
  * Returns:
  * 	 BCM_E_XXX
  */
 
STATIC int
_bcm_td_trill_loopback_enable(int unit)
{
    int rv = BCM_E_NONE;
    multipass_loopback_bitmap_entry_t  trill_loopback;

    sal_memset(&trill_loopback, 0, sizeof(multipass_loopback_bitmap_entry_t));

    /* Enable Loopback */
    soc_mem_field32_set(unit, MULTIPASS_LOOPBACK_BITMAPm, 
                        &trill_loopback, BITMAP_W2f, 0x2);
    rv = soc_mem_write(unit, MULTIPASS_LOOPBACK_BITMAPm,
                        MEM_BLOCK_ALL, 0, &trill_loopback);
    return rv;
}

 /*
  * Function:
  * 	 _bcm_td_trill_loopback_disable
  * Purpose:
  * 	 Disable loopback for TRILL Multicast
  * Parameters:
  *   IN :	unit
  * Returns:
  * 	 BCM_E_XXX
  */
 
STATIC int
_bcm_td_trill_loopback_disable(int unit)
{
    int rv = BCM_E_NONE;
    multipass_loopback_bitmap_entry_t  trill_loopback;

    sal_memset(&trill_loopback, 0, sizeof(multipass_loopback_bitmap_entry_t));

    /* Enable Loopback */
    soc_mem_field32_set(unit, MULTIPASS_LOOPBACK_BITMAPm, 
                        &trill_loopback, BITMAP_W2f, 0x0);
    rv = soc_mem_write(unit, MULTIPASS_LOOPBACK_BITMAPm,
                        MEM_BLOCK_ALL, 0, &trill_loopback);
    return rv;
}

 /*
  * Function:
  *     _bcm_td_trill_root_bridge_set
  * Purpose:
  *     Configure Root RBridge
  * Parameters:
  *     IN : unit
  * Returns:
  *     BCM_E_XXX
  */
 
STATIC int
_bcm_td_trill_root_bridge_set(int unit, bcm_trill_port_t *trill_port, int vp)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    int rv=BCM_E_NONE;
    int idx;
    uint8 trill_tree_profile_idx=0;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);

    /* Verify whether Root Rbridge already exists? */
    for (idx = 0; idx < BCM_MAX_NUM_TRILL_TREES; idx++) {
         if ( (trill_info->rootBridge[idx] == trill_port->name) && !(trill_port->flags & BCM_TRILL_PORT_REPLACE)) {
              (void) _bcm_vp_free(unit, _bcmVpTypeTrill, 1, vp);
              /* TRILL DVP to Root RBridge already exists */
                   return (BCM_E_EXISTS); 
         }
    }

    /* Set Tree profile */
    rv = _bcm_td_trill_tree_profile_set (unit, trill_port->name, 
    trill_port->multicast_hopcount, &trill_tree_profile_idx);
    if (rv < 0) {
         trill_info->rootBridge[trill_tree_profile_idx] = 0;
         (void) _bcm_vp_free(unit, _bcmVpTypeTrill, 1, vp);
         return rv;
    }
    return rv;
}

/*
 * Purpose:
 *	 Add Trill port to TRILL Cloud
 * Parameters:
 *	 unit	 - (IN) Device Number
 *	 trill_port - (IN/OUT) trill port information (OUT : trill_port_id)
*/
int 
bcm_td_trill_port_add(int unit, bcm_trill_port_t *trill_port)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    int rv=BCM_E_NONE;
    int nh_ecmp_index = -1, nh_index=-1;
    int  vp, num_vp, base_idx;
    int  idx, max_ent_count;
    source_vp_entry_t svp;
    ing_dvp_table_entry_t dvp;
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry. */
    uint32 flags=0;
    int cml_default_enable=0, cml_default_new=0, cml_default_move=0;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);
    BCM_IF_ERROR_RETURN(
                _bcm_td_trill_port_validate(trill_port));
    
    /* 
     * Extract next-hop index from egress object  OR
     * Extract ecmp group index from multipath egress object. */
    if (trill_port->flags & BCM_TRILL_MULTIPATH) {
         flags = BCM_L3_MULTIPATH;
    }

    if (!(trill_port->flags & BCM_TRILL_PORT_LOCAL)) {
         BCM_IF_ERROR_RETURN(
                bcm_xgs3_get_nh_from_egress_object(unit,
                           trill_port->egress_if, &flags, 1, &nh_ecmp_index));
    }

    if (trill_port->flags & BCM_TRILL_PORT_REPLACE) {

         vp = BCM_GPORT_TRILL_PORT_ID_GET(trill_port->trill_port_id);
         if (vp == -1) {
            return BCM_E_PARAM;
         }
         if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {
              return BCM_E_NOT_FOUND;
         }
         BCM_IF_ERROR_RETURN(
              READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp));
         BCM_IF_ERROR_RETURN(
              READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));
    } else if (trill_port->flags & BCM_TRILL_PORT_WITH_ID ) {
    
         if (!BCM_GPORT_IS_TRILL_PORT(trill_port->trill_port_id)) {
              return (BCM_E_BADID);
         }

         num_vp = soc_mem_index_count(unit, SOURCE_VPm);
         vp = BCM_GPORT_TRILL_PORT_ID_GET(trill_port->trill_port_id);
         if (vp >= num_vp) {
              return (BCM_E_BADID);
         }
         rv = _bcm_vp_used_get(unit, vp, _bcmVpTypeTrill);
         if (rv < 0) {
              return (BCM_E_EXISTS);
         }
         BCM_IF_ERROR_RETURN(_bcm_vp_used_set(unit, vp, _bcmVpTypeTrill));
         sal_memset(&svp, 0, sizeof(source_vp_entry_t));
         sal_memset(&dvp, 0, sizeof(ing_dvp_table_entry_t));
    } else {

         /* allocate a new VP index */
         num_vp = soc_mem_index_count(unit, SOURCE_VPm);
         rv = _bcm_vp_alloc(unit, 0, (num_vp - 1), 1, SOURCE_VPm, _bcmVpTypeTrill, &vp);
         if (rv < 0) {
              return rv;
         }
         sal_memset(&svp, 0, sizeof(source_vp_entry_t));
         sal_memset(&dvp, 0, sizeof(ing_dvp_table_entry_t));
        BCM_IF_ERROR_RETURN(_bcm_vp_used_set(unit, vp, _bcmVpTypeTrill));
    }
    if (!(trill_port->flags & BCM_TRILL_PORT_NETWORK)) {
         (void) _bcm_vp_free(unit, _bcmVpTypeTrill, 1, vp);
         return (BCM_E_PARAM); /* TRILL DVP shall always be a Network Port */
    }

    /* Set Virtual Port */
    soc_SOURCE_VPm_field32_set(unit, &svp, CLASS_IDf, 
                                  trill_port->if_class);
    soc_SOURCE_VPm_field32_set(unit, &svp, 
                                  ENTRY_TYPEf, 0x3); /* TRILL VP */
    soc_SOURCE_VPm_field32_set(unit, &svp, NETWORK_PORTf, 0x1);

        /* Set the CML */
       BCM_TRILL_CHECK_ERROR_RETURN(
            _bcm_vp_default_cml_mode_get (unit, 
                           &cml_default_enable, &cml_default_new, 
                           &cml_default_move), unit, vp);

        if (cml_default_enable) {
            /* Set the CML to default values */
            soc_SOURCE_VPm_field32_set(unit, &svp, CML_FLAGS_NEWf, cml_default_new);
            soc_SOURCE_VPm_field32_set(unit, &svp, CML_FLAGS_MOVEf, cml_default_move);
        } else {
            /* Set the CML to PVP_CML_SWITCH by default (hw learn and forward) */
            soc_SOURCE_VPm_field32_set(unit, &svp, CML_FLAGS_NEWf, 0x8);
            soc_SOURCE_VPm_field32_set(unit, &svp, CML_FLAGS_MOVEf, 0x8);
        }

    soc_SOURCE_VPm_field32_set(unit, &svp, TPID_SOURCEf, 0x3);
    BCM_TRILL_CHECK_ERROR_RETURN(
                   WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp), unit, vp);

    soc_ING_DVP_TABLEm_field32_set(unit, &dvp, 
                                  VP_TYPEf, 0x1); /* TRILL VP */    
    soc_ING_DVP_TABLEm_field32_set(unit, &dvp, NETWORK_PORTf, 0x1);


    if (trill_port->flags & BCM_TRILL_PORT_LOCAL) { /* My RBridge */
         soc_ING_DVP_TABLEm_field32_set(unit, &dvp,
                              NEXT_HOP_INDEXf, 0x0);
         BCM_TRILL_CHECK_ERROR_RETURN(
              WRITE_ING_DVP_TABLEm(unit, MEM_BLOCK_ALL, vp, &dvp), unit, vp);
         BCM_TRILL_CHECK_ERROR_RETURN(
              _bcm_td_trill_egress_dvp_set(unit, vp, trill_port), unit, vp);
         BCM_TRILL_CHECK_ERROR_RETURN(
              _bcm_td_trill_header_set(unit, trill_port->name), unit, vp);

         /* ******** TRILL Decap Entry  ******************** */
         rv =   _bcm_td_trill_decap_entry_set(unit,  trill_port);
         if (rv < 0) {
              return rv;
         }
         trill_info->rBridge[vp] = trill_port->name;

         /* Root RBridge */
         if (trill_port->flags &  BCM_TRILL_PORT_MULTICAST) {
              rv = _bcm_td_trill_root_bridge_set(unit, trill_port, vp);
         }
         if (rv < 0) {
              return rv;
         }
    } else {

         /* Root RBridge */
         if (trill_port->flags &  BCM_TRILL_PORT_MULTICAST) {
              rv = _bcm_td_trill_root_bridge_set(unit, trill_port, vp);
         }
         if (rv < 0) {
              return rv;
         }
         /* ******** TRILL Access ECMP/Next-Hop  ********** */
         if (trill_port->flags & BCM_TRILL_MULTIPATH) {
              soc_ING_DVP_TABLEm_field32_set(unit, &dvp, 
                                       ECMPf, 0x1);
              soc_ING_DVP_TABLEm_field32_set(unit, &dvp, 
                                       ECMP_PTRf, nh_ecmp_index);
              BCM_TRILL_CHECK_ERROR_RETURN(
                   WRITE_ING_DVP_TABLEm(unit, MEM_BLOCK_ALL, vp, &dvp), unit, vp);
              BCM_TRILL_CHECK_ERROR_RETURN(
                   soc_mem_read(unit, L3_ECMP_COUNTm, MEM_BLOCK_ANY, 
                        nh_ecmp_index, hw_buf), unit, vp);
              if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
                   max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, COUNTf);
                   base_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, BASE_PTRf);
              } else {
                   max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, COUNT_0f);
                   base_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, BASE_PTR_0f);
              }
              max_ent_count++; /* Count is zero based. */ 
              for (idx = 0; idx < max_ent_count; idx++) {
                   BCM_TRILL_CHECK_ERROR_RETURN(
                        soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY, 
                             (base_idx + idx), hw_buf), unit, vp);
                   nh_index = soc_mem_field32_get(unit, L3_ECMPm, 
                             hw_buf, NEXT_HOP_INDEXf);
                   BCM_TRILL_CHECK_ERROR_RETURN(
                        _bcm_td_trill_next_hop_set(unit, nh_index, trill_port), unit, vp);
                   BCM_TRILL_CHECK_ERROR_RETURN(
                        _bcm_td_trill_adjacency_set(unit, nh_index), unit, vp);
              }
         } else {
              soc_ING_DVP_TABLEm_field32_set(unit, &dvp, 
                                       NEXT_HOP_INDEXf, nh_ecmp_index);
              BCM_TRILL_CHECK_ERROR_RETURN(
                   WRITE_ING_DVP_TABLEm(unit, MEM_BLOCK_ALL, vp, &dvp), unit, vp);
              BCM_TRILL_CHECK_ERROR_RETURN(
                   _bcm_td_trill_next_hop_set(unit, nh_ecmp_index, trill_port), unit, vp);
              BCM_TRILL_CHECK_ERROR_RETURN(
                   _bcm_td_trill_adjacency_set(unit, nh_ecmp_index), unit, vp);
         }

         BCM_TRILL_CHECK_ERROR_RETURN(
              _bcm_td_trill_egress_dvp_set(unit, vp, trill_port), unit, vp);

         /* ******** TRILL Transit Forwarding Entry  ********** */
         BCM_TRILL_CHECK_ERROR_RETURN(
              _bcm_td_trill_transit_entry_set(unit, trill_port, nh_ecmp_index), unit, vp);

         /* ******** TRILL Decap Learn Entry  ********** */
         BCM_TRILL_CHECK_ERROR_RETURN(
              _bcm_td_trill_learn_entry_set(unit, trill_port, vp), unit, vp);
         trill_info->rBridge[vp] = trill_port->name;
    }
    /* Set TRILL  port ID */
    BCM_GPORT_TRILL_PORT_ID_SET(trill_port->trill_port_id, vp);
    trill_port->encap_id = nh_ecmp_index;

    return BCM_E_NONE;
}

/*
 * Purpose:
 *	 Delete Trill port from TRILL Cloud
 * Parameters:
 *	 unit		- (IN) Device Number
 *		trill_port_id - (IN) Trill port ID
 */
int
bcm_td_trill_port_delete(int unit, bcm_gport_t trill_port_id)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    int vp;
    source_vp_entry_t svp;
    ing_dvp_table_entry_t dvp;
    egr_dvp_attribute_entry_t  egr_dvp_attribute;
    int rv=BCM_E_NONE;
    int nh_index = -1, nh_ecmp_index=-1;
    int  idx, max_ent_count, base_idx;
    int ecmp =-1;
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry. 	 */
    uint16 trill_name;
    uint32  flags=0;
    int  ref_count=0;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);
    vp = BCM_GPORT_TRILL_PORT_ID_GET(trill_port_id);
    if (vp == -1) {
        return BCM_E_PARAM;
    }

    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {
         return BCM_E_NOT_FOUND;
    }
    BCM_IF_ERROR_RETURN(
       soc_mem_read(unit, EGR_DVP_ATTRIBUTEm,
                       MEM_BLOCK_ALL, vp, &egr_dvp_attribute));
  
    trill_name = soc_mem_field32_get(unit, EGR_DVP_ATTRIBUTEm, 
                                            &egr_dvp_attribute, EGRESS_RBRIDGE_NICKNAMEf);

    BCM_IF_ERROR_RETURN(
         READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));
    ecmp = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, ECMPf);
    if (ecmp) {
         nh_ecmp_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, ECMP_PTRf);
         if(nh_ecmp_index != 0) {
              flags = BCM_L3_MULTIPATH;
              BCM_IF_ERROR_RETURN(
                   bcm_xgs3_get_ref_count_from_nhi(unit, flags, &ref_count, nh_ecmp_index));
         }

         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, L3_ECMP_COUNTm, MEM_BLOCK_ANY, nh_ecmp_index, hw_buf));

              if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
                   max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, COUNTf);
                   base_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, BASE_PTRf);
              } else {
                   max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, COUNT_0f);
                   base_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, 
                                                 hw_buf, BASE_PTR_0f);
              }
              max_ent_count++; /* Count is zero based. */ 
         for (idx = 0; idx < max_ent_count; idx++) {
              BCM_IF_ERROR_RETURN(
                   soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY, 
                             (base_idx + idx), hw_buf));
              nh_index = soc_mem_field32_get(unit, L3_ECMPm, 
                             hw_buf, NEXT_HOP_INDEXf);
              if (ref_count == 1) {
                  BCM_IF_ERROR_RETURN(
                       _bcm_td_trill_next_hop_reset(unit, nh_index));
              }
         }
    } else {
        nh_ecmp_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, NEXT_HOP_INDEXf);
        if(nh_ecmp_index != 0) {
              BCM_IF_ERROR_RETURN(
                   bcm_xgs3_get_ref_count_from_nhi(unit, flags, &ref_count, nh_ecmp_index));
        }
        if (ref_count == 1) {
              if (nh_ecmp_index) {
                   BCM_IF_ERROR_RETURN(
                        _bcm_td_trill_adjacency_reset(unit, nh_ecmp_index));
                   BCM_IF_ERROR_RETURN(
                        _bcm_td_trill_next_hop_reset(unit, nh_ecmp_index));
              }
         }
    }
    if (nh_ecmp_index) {
         BCM_IF_ERROR_RETURN(
              _bcm_td_trill_transit_entry_reset(unit, vp, nh_ecmp_index, ecmp));
         BCM_IF_ERROR_RETURN(
              _bcm_td_trill_learn_entry_reset(unit, vp));
         BCM_IF_ERROR_RETURN(
              _bcm_td_trill_egress_dvp_reset(unit, vp));
    } else {
         BCM_IF_ERROR_RETURN(
              _bcm_td_trill_decap_entry_reset(unit,  vp));
         BCM_IF_ERROR_RETURN(
              _bcm_td_trill_egress_dvp_reset(unit, vp));
    }
    /* Clear the SVP and DVP table entries */
    sal_memset(&svp, 0, sizeof(source_vp_entry_t));
    BCM_IF_ERROR_RETURN(
         WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp));

    sal_memset(&dvp, 0, sizeof(ing_dvp_table_entry_t));
    BCM_IF_ERROR_RETURN(
         WRITE_ING_DVP_TABLEm(unit, MEM_BLOCK_ALL, vp, &dvp));

    BCM_IF_ERROR_RETURN(
         _bcm_td_trill_tree_profile_reset (unit, trill_name));

    /* Free RBridge */
    trill_info->rBridge[vp] = 0;

    /* Free the VP */
    (void) _bcm_vp_free(unit, _bcmVpTypeTrill, 1, vp);

    return rv;
}

 /*
  * Purpose:
  *	  Delete all Trill ports from TRILL Cloud
  * Parameters:
  *	  unit - Device Number
  */

int 
bcm_td_trill_port_delete_all(int unit)
{
    int rv=BCM_E_UNAVAIL;
    uint32 vp, num_vp;
    ing_dvp_table_entry_t dvp;
    bcm_gport_t trill_port_id;

    num_vp = soc_mem_index_count(unit, ING_DVP_TABLEm);
    for (vp = 0; vp < num_vp; vp++) {
         /* Check for validity of the VP */
         if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {  
              continue;
         }
         BCM_IF_ERROR_RETURN(
              READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));
         if ((soc_ING_DVP_TABLEm_field32_get(unit, &dvp, VP_TYPEf) == 0x1)) {
              BCM_GPORT_TRILL_PORT_ID_SET(trill_port_id, vp);
              rv = bcm_td_trill_port_delete(unit, trill_port_id);
              if (rv < 0) {
                   return rv;
              }
         }
    }
    return rv;
}

 /*
  * Purpose:
  *      Get a trill port 
  * Parameters:
  *      unit    - (IN) Device Number
  *      trill_port - (IN/OUT) Trill port information (IN : trill_port_id)
  */

int 
bcm_td_trill_port_get(int unit, bcm_trill_port_t *trill_port)
{
    uint32 vp;
    egr_dvp_attribute_entry_t  egr_dvp_attribute;
    source_vp_entry_t svp;
    ing_dvp_table_entry_t dvp;
    ing_l3_next_hop_entry_t ing_nh;
    int  nh_ecmp_index;
    int ecmp;
    int drop;
    int network_port;

    vp = BCM_GPORT_TRILL_PORT_ID_GET(trill_port->trill_port_id);
    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {
         return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN(
       soc_mem_read(unit, EGR_DVP_ATTRIBUTEm,
                       MEM_BLOCK_ALL, vp, &egr_dvp_attribute));
  
    trill_port->name = soc_mem_field32_get(unit, EGR_DVP_ATTRIBUTEm, 
                                            &egr_dvp_attribute, EGRESS_RBRIDGE_NICKNAMEf);
    trill_port->mtu = soc_mem_field32_get(unit, EGR_DVP_ATTRIBUTEm, 
                                            &egr_dvp_attribute, MTU_VALUEf);
    trill_port->hopcount = soc_mem_field32_get(unit, EGR_DVP_ATTRIBUTEm, 
                                            &egr_dvp_attribute, HOPCOUNTf);
  
    BCM_IF_ERROR_RETURN(
       soc_mem_read(unit, SOURCE_VPm,
                        MEM_BLOCK_ALL, vp, &svp));
  
    trill_port->if_class = soc_mem_field32_get(unit, SOURCE_VPm, 
                                            &svp, CLASS_IDf);

    BCM_IF_ERROR_RETURN(
         READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));
    ecmp = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, ECMPf);
    if (ecmp) {
         nh_ecmp_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, ECMP_PTRf);
         trill_port->egress_if  =  nh_ecmp_index + BCM_XGS3_MPATH_EGRESS_IDX_MIN;
         trill_port->flags |=  BCM_TRILL_MULTIPATH;
    } else {
         nh_ecmp_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, NEXT_HOP_INDEXf);
         /* Extract next hop index from unipath egress object. */
         if (nh_ecmp_index) {
              trill_port->egress_if  =  nh_ecmp_index + BCM_XGS3_EGRESS_IDX_MIN;
         } else {
              trill_port->flags |=  BCM_TRILL_PORT_LOCAL;
         }
    }
    network_port = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, NETWORK_PORTf);
    if (network_port) {
         trill_port->flags |=  BCM_TRILL_PORT_NETWORK;
    }

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, ING_L3_NEXT_HOPm, 
                                  MEM_BLOCK_ANY, nh_ecmp_index, &ing_nh));

    drop = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm, 
                                  &ing_nh, DROPf);
    if (drop) {
         trill_port->flags |=  BCM_TRILL_PORT_DROP;
    }
    return BCM_E_NONE;
}

/*
 * Purpose:
 *      Get all trill ports from TRILL Cloud
 * Parameters:
 *      unit     - (IN) Device Number
 *      port_max   - (IN) Maximum number of Trill ports in array
 *      port_array - (OUT) Array of Trill ports
 *      port_count - (OUT) Number of Trill ports returned in array
 *
 */

int 
bcm_td_trill_port_get_all(int unit, int port_max, bcm_trill_port_t *port_array, int *port_count )
{
    int rv = BCM_E_NONE;
    int vp;
    uint32 num_vp;
    ing_dvp_table_entry_t dvp;
 
    *port_count = 0;

    num_vp = soc_mem_index_count(unit, SOURCE_VPm);
    for (vp = 0; vp < num_vp; vp++) {
         if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {
              continue;
         }
         if (*port_count == port_max) {
              break;
         }
         BCM_IF_ERROR_RETURN(
              READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));
 
         if (0x1 == soc_ING_DVP_TABLEm_field32_get(unit, &dvp, VP_TYPEf)){
              BCM_GPORT_TRILL_PORT_ID_SET(port_array[*port_count].trill_port_id, vp);
              rv = bcm_td_trill_port_get(unit,
                                    &port_array[*port_count]);
              if (rv<0) {
                   return rv;
              }
              (*port_count)++;
         }
    }
    return rv;
}

/*
 * Function:
 *		_bcm_td_trill_multicast_transit_entry_key_set
 * Purpose:
 *		Set TRILL Multicast entry key
 * Parameters:
 *  IN :  Unit
 *  IN :  Trill Root name
 *  IN : MPLS entry pointer
 * Returns:
 *		BCM_E_XXX
 */

STATIC void
 _bcm_td_trill_multicast_transit_entry_key_set (int unit, uint16 root_name,
                                          mpls_entry_entry_t   *tr_ent)
{
    uint8  trill_tree_id=0;

    (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                       TRILL__TREE_IDf, trill_tree_id);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                       KEY_TYPEf, 0x5);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                       TRILL__RBRIDGE_NICKNAMEf, root_name);
    soc_mem_field32_set(unit, MPLS_ENTRYm, tr_ent,
                                       VALIDf, 0x1);
}

/*
 * Purpose:
 *      Set TRILL Multicast entry for Transit operation
 * Parameters:
 *      unit     - (IN) Device Number
 *      root_name  - (IN) trill Root_RBridge
 *      mc_index   - (IN) Multicast index
 */

int
bcm_td_trill_multicast_transit_entry_set(int unit, uint16 root_name, int mc_index)
{
    mpls_entry_entry_t  tr_key, tr_entry;
    int rv = BCM_E_UNAVAIL;
    int entry_index=0;

    sal_memset(&tr_key, 0, sizeof(mpls_entry_entry_t));
    sal_memset(&tr_entry, 0, sizeof(mpls_entry_entry_t));

    (void) _bcm_td_trill_multicast_transit_entry_key_set(unit, root_name, &tr_key);

    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &entry_index,
                        &tr_key, &tr_entry, 0);
    if (rv == SOC_E_NONE) {
         soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_entry,
                        TRILL__L3MC_INDEXf, mc_index);
         rv = soc_mem_write(unit, MPLS_ENTRYm,
                        MEM_BLOCK_ALL, entry_index, &tr_entry);
    } else {
        /* Root Entry Must be present */
        return rv;
    }
    return rv;

}

/*
 * Purpose:
 *       Reset TRILL Multicast entry for Transit operation
 * Parameters:
 *      unit     - (IN) Device Number
 *      root_name  - (IN) trill Root_RBridge
 *      mc_index   - (IN) Multicast index
 */

int
bcm_td_trill_multicast_transit_entry_reset(int unit, uint16 root_name, int mc_index)
{
    mpls_entry_entry_t  tr_ent;
    int rv = BCM_E_UNAVAIL;
    int index;

    sal_memset(&tr_ent, 0, sizeof(mpls_entry_entry_t));

    (void) _bcm_td_trill_multicast_transit_entry_key_set(unit, root_name, &tr_ent);

    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                             &tr_ent, &tr_ent, 0);
    if (rv < 0) {
         return rv;
    }
   
    /* Remove l3mc_index from HW entry */
    soc_mem_field32_set(unit, MPLS_ENTRYm, &tr_ent,
                   TRILL__L3MC_INDEXf, 0);
    rv = soc_mem_write(unit, MPLS_ENTRYm,
                   MEM_BLOCK_ALL, index, &tr_ent);

    rv = soc_mem_delete(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, &tr_ent);
    return rv;
}


/*
 * Purpose:
 *       Reset all TRILL Multicast entries for Transit operation
 * Parameters:
 *      unit     - (IN) Device Number
 *      tree_id - (IN) trill Tree ID
 *      mc_index   - (IN) Multicast index
 */

STATIC int
bcm_td_trill_multicast_transit_entry_reset_all (int unit, uint8 tree_id)
{
    int rv, idx, num_entries;
    mpls_entry_entry_t ment;

    num_entries = soc_mem_index_count(unit, MPLS_ENTRYm);
    for (idx = 0; idx < num_entries; idx++) {
        rv = READ_MPLS_ENTRYm(unit, MEM_BLOCK_ANY, idx, &ment);
        if (rv < 0) {
            return rv;
        }
        if (!soc_MPLS_ENTRYm_field32_get(unit, &ment, VALIDf)) {
            continue;
        }
        if (soc_MPLS_ENTRYm_field32_get(unit, &ment,
                                        TRILL__TREE_IDf) != tree_id) {
            continue;
        }

        /* Delete the entry from HW */
        rv = soc_mem_delete(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, &ment);
        if ( (rv != BCM_E_NOT_FOUND) && (rv != BCM_E_NONE) ) {
              return rv;
        }
    }
    return BCM_E_NONE;
}

/*
 * Purpose:
 *       Add TRILL multicast entry
 *       root_name = 0: Add Access -> Network entry
 *       root_name > 0: Add Network -> Access entry
 * Parameters:
 *           unit   - Device Number
 *           flags
 *           root_name  - (IN) trill Root_RBridge
 *           vid- (IN) Customer VLAN
 *           c_dmac	- (IN) Customer D-MAC
 *           group  - (IN) Multicast Group ID
 */

int
bcm_td_trill_multicast_add( int unit, uint32 flags,
                                                     bcm_trill_name_t  root_name, 
                                                     bcm_vlan_t vid, 
                                                     bcm_mac_t c_dmac, 
                                                     bcm_multicast_t group)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    int l2mc_index, l3mc_index;
    int rv = BCM_E_UNAVAIL;
    uint8  trill_tree_id=0;
    bcm_multicast_t trill_group;
    bcm_mac_t mac_zero = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);

    /* Check Vlan range */
    if ((vid >= BCM_VLAN_INVALID) || ( vid == BCM_VLAN_DEFAULT)) {
         return BCM_E_PARAM;
    }

    /* Check DA */
    if (c_dmac == NULL) {
         return BCM_E_PARAM;
    }

    /* Check Tree-ID */
    (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);
    if (trill_tree_id == 0) {
         return BCM_E_PARAM;
    }

    if (flags & BCM_TRILL_MULTICAST_ACCESS_TO_NETWORK) {
         l2mc_index = _BCM_MULTICAST_ID_GET(group);
         BCM_TRILL_L2MC_CHECK (unit, l2mc_index);
         if ((trill_info->multicast_count[l2mc_index].network_port_count) && (vid != BCM_VLAN_NONE)) {

              /* Access-domain Entry - Update */
              BCM_IF_ERROR_RETURN(
                            bcm_td_l2_trill_multicast_entry_add(unit, TR_L2_HASH_KEY_TYPE_BRIDGE, 
                                                   vid, c_dmac, trill_tree_id, group));

              trill_group = trill_info->multicast_count[l2mc_index].l3mc_group;
              l3mc_index = _BCM_MULTICAST_ID_GET(trill_group);
              BCM_TRILL_L3MC_CHECK (unit, l3mc_index);

             /* TRILL_Network Domain entry */
             BCM_IF_ERROR_RETURN(
                            bcm_td_l2_trill_multicast_entry_add(unit, 
                                                   TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS, 
                                                   vid, c_dmac, trill_tree_id, trill_group));
             BCM_IF_ERROR_RETURN(_bcm_td_trill_loopback_enable(unit));

            /* Set EGR_IPMC.trill_tree_id */
            rv = bcm_td_multicast_trill_group_update(unit, l3mc_index, trill_tree_id);
         }
    } else {

        /* TRILL-Network Domain - Unpruned entry */
        if ((vid == BCM_VLAN_NONE) && (!sal_memcmp(c_dmac, mac_zero, sizeof(bcm_mac_t)))) {
              l3mc_index = _BCM_MULTICAST_ID_GET(group);
              BCM_TRILL_L3MC_CHECK (unit, l3mc_index);

              rv = bcm_td_trill_multicast_transit_entry_set(unit, 
                                                 root_name, l3mc_index);
        } else if ((!sal_memcmp(c_dmac, mac_zero, sizeof(bcm_mac_t))) && (vid != BCM_VLAN_NONE)) {
              l2mc_index = _BCM_MULTICAST_ID_GET(group);
              BCM_TRILL_L2MC_CHECK (unit, l2mc_index);
 
              trill_group = trill_info->multicast_count[l2mc_index].l3mc_group;
              l3mc_index = _BCM_MULTICAST_ID_GET(trill_group);
              BCM_TRILL_L3MC_CHECK (unit, l3mc_index);

              /* TRILL-Network Domain - Pruned entry - Short */
              (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);
              rv = bcm_td_l2_trill_multicast_entry_add(unit, 
                                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT, 
                                                 vid, c_dmac, trill_tree_id, trill_group);
              if(BCM_SUCCESS(rv)){
                   rv = _bcm_td_trill_loopback_enable(unit);
              }
        } else if (vid != BCM_VLAN_NONE) {
              l2mc_index = _BCM_MULTICAST_ID_GET(group);
              BCM_TRILL_L2MC_CHECK (unit, l2mc_index);
 
              trill_group = trill_info->multicast_count[l2mc_index].l3mc_group;
              l3mc_index = _BCM_MULTICAST_ID_GET(trill_group);
              BCM_TRILL_L3MC_CHECK (unit, l3mc_index);

              /* TRILL-Network Domain - Pruned entry - Long */
              (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);
              rv = bcm_td_l2_trill_multicast_entry_add(unit, 
                                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG,
                                                 vid, c_dmac, trill_tree_id, trill_group);
              if(BCM_SUCCESS(rv)){
                   rv = _bcm_td_trill_loopback_enable(unit);
              }
        }
    }
    return rv;
}

/*
 * Purpose:
 *      Delete TRILL multicast entry
 * Parameters:
 *           unit   - Device Number
 *           flags
 *           root_name  - (IN) trill Root_RBridge
 *           vid- (IN) Customer VLAN
 *           c_dmac	  (IN) Customer D-MAC
 *           access_domain_group  - (IN) Multicast Group ID
 *           trill_domain_group  - (IN) Multicast Group ID
 */

int
bcm_td_trill_multicast_delete(int unit, uint32 flags,
                                                         bcm_trill_name_t root_name,
                                                         bcm_vlan_t vid,
                                                         bcm_mac_t c_dmac,
                                                         bcm_multicast_t group)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    int rv = BCM_E_UNAVAIL;
    uint8  trill_tree_id=0;
    bcm_multicast_t trill_group;
    int l2mc_index, l3mc_index;
    bcm_mac_t mac_zero = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);

    /* Check Vlan range */
    if ((vid >= BCM_VLAN_INVALID) || ( vid == BCM_VLAN_DEFAULT)) {
         return BCM_E_PARAM;
    }

    /* Check DA */
    if (c_dmac == NULL) {
         return BCM_E_PARAM;
    }

    /* Check Tree-ID */
    (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);
    if (trill_tree_id == 0) {
         return BCM_E_PARAM;
    }

    if (flags & BCM_TRILL_MULTICAST_ACCESS_TO_NETWORK) {
         l2mc_index = _BCM_MULTICAST_ID_GET(group); 
         BCM_TRILL_L2MC_CHECK (unit, l2mc_index);
         if ((trill_info->multicast_count[l2mc_index].network_port_count) && (vid != BCM_VLAN_NONE)) {
              trill_group = trill_info->multicast_count[l2mc_index].l3mc_group;
              l3mc_index = _BCM_MULTICAST_ID_GET(trill_group);
              BCM_TRILL_L3MC_CHECK (unit, l3mc_index);

             /* TRILL_Network Domain entry */
             rv = bcm_td_l2_trill_multicast_entry_delete(unit, 
                                            TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS, 
                                            vid, c_dmac, trill_tree_id, trill_group);
         }
    } else {
         /* TRILL-Network Domain - Unpruned entry */
         if ((vid == BCM_VLAN_NONE) &&  (!sal_memcmp(c_dmac, mac_zero, sizeof(bcm_mac_t)))) {       
              l3mc_index = _BCM_MULTICAST_ID_GET(group);
              BCM_TRILL_L3MC_CHECK (unit, l3mc_index);

              rv = bcm_td_trill_multicast_transit_entry_reset(unit, 
                                       root_name, l3mc_index);
         } else if ((!sal_memcmp(c_dmac, mac_zero, sizeof(bcm_mac_t))) && (vid != BCM_VLAN_NONE)) {

              l2mc_index = _BCM_MULTICAST_ID_GET(group);
              BCM_TRILL_L2MC_CHECK (unit, l2mc_index);
 
              trill_group = trill_info->multicast_count[l2mc_index].l3mc_group;
              l3mc_index = _BCM_MULTICAST_ID_GET(trill_group);
              BCM_TRILL_L3MC_CHECK (unit, l3mc_index);

              /* TRILL-Network Domain - Pruned entry - Short */
              (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);
              rv = bcm_td_l2_trill_multicast_entry_delete(unit, 
                                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT, 
                                                 vid, c_dmac, trill_tree_id, trill_group);
              if(BCM_SUCCESS(rv)){
                   rv = _bcm_td_trill_loopback_disable(unit);
              }
         } else if (vid != BCM_VLAN_NONE) {

              l2mc_index = _BCM_MULTICAST_ID_GET(group);
              BCM_TRILL_L2MC_CHECK (unit, l2mc_index);

              trill_group = trill_info->multicast_count[l2mc_index].l3mc_group;
              l3mc_index = _BCM_MULTICAST_ID_GET(trill_group);
              BCM_TRILL_L3MC_CHECK (unit, l3mc_index);

              /* TRILL-Network Domain - Pruned entry - Long */
              (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);
              rv = bcm_td_l2_trill_multicast_entry_delete(unit, 
                                                 TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG,
                                                 vid, c_dmac, trill_tree_id, trill_group);
              if(BCM_SUCCESS(rv)){
                   rv = _bcm_td_trill_loopback_disable(unit);
              }
         }  
    }
    return rv;
}

/*
 * Purpose:
 *      Delete all TRILL multicast entries
 * Parameters:
 *		unit   - Device Number
 */

int
bcm_td_trill_multicast_delete_all(int unit, bcm_trill_name_t  root_name)
{
    int rv=BCM_E_UNAVAIL;
    uint8  trill_tree_id=0;

   (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);
   BCM_IF_ERROR_RETURN(
              bcm_td_trill_multicast_transit_entry_reset_all (unit, trill_tree_id));
   rv = bcm_td_l2_trill_multicast_entry_delete_all (unit, trill_tree_id);

   return rv;
}

/*
 * Purpose:
 *      Verify whether TRILL ports added to L2MC group
 * Parameters:
 *      unit         - (IN) Unit number.
 *      l2mc_group        - (IN) Multicast Group
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td_trill_multicast_check(int unit, bcm_multicast_t l2mc_group)
{
    _bcm_td_trill_bookkeeping_t *trill_info = TRILL_INFO(unit);
    int rv=BCM_E_UNAVAIL;
    int l2mc_index;

    l2mc_index = _BCM_MULTICAST_ID_GET(l2mc_group);
    BCM_TRILL_L2MC_CHECK (unit, l2mc_index);
   if (trill_info->multicast_count[l2mc_index].network_port_count) {
         rv = BCM_E_NONE;
    }
    return rv;
}

/*
 * Purpose:
 *      Obtain mapped L3MC group for a given L2MC group
 * Parameters:
 *      unit         - (IN) Unit number.
 *      l2mc_group        - (IN) L2 Multicast Group
 *      l3mc_group        - (OUT) L3 Multicast Group
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td_trill_multicast_group_get(int unit, bcm_multicast_t l2mc_group, bcm_multicast_t *l3mc_group)
{
    _bcm_td_trill_bookkeeping_t *trill_info = TRILL_INFO(unit);
    int rv=BCM_E_UNAVAIL;
    int l2mc_index;

    l2mc_index = _BCM_MULTICAST_ID_GET(l2mc_group);
    BCM_TRILL_L2MC_CHECK (unit, l2mc_index);
    if (trill_info->multicast_count[l2mc_index].network_port_count) {
         *l3mc_group = trill_info->multicast_count[l2mc_index].l3mc_group;
         rv = BCM_E_NONE;
    }
    return rv;
}

/*
 * Purpose:
 *      Traverse all valid TRILL Multicast entries and call the
 * 	 supplied callback routine.
 * Parameters:
 *     unit   - Device Number
 *     cb  - User callback function, called once per Trill Multicast entry.
 *     user_data - cookie
 */

int
bcm_td_trill_multicast_traverse(int unit, bcm_trill_multicast_traverse_cb  cb,
                                                        void *user_data)
{
    return BCM_E_UNAVAIL;
}

/*
 * Purpose:
 *      Add TRILL Gports to L2MC
 * Parameters:
 *      unit         - (IN) Unit number.
 *      port         - (IN) Gport.
 *      mc_index        - (IN) Multicast Index
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td_trill_egress_add(int unit, bcm_gport_t trill_port, 
                                                      bcm_multicast_t l2mc_group, bcm_if_t encap_id)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    int vp=0;
    int network_port=0;
    ing_dvp_table_entry_t dvp;
    int rv = BCM_E_NONE;
    bcm_multicast_t  l3mc_group;
    int l2mc_index;
    
    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);

    sal_memset(&l3mc_group, 0, sizeof(bcm_multicast_t));

    if (!BCM_GPORT_IS_TRILL_PORT(trill_port)) {
        return (BCM_E_BADID);
    }

    vp = BCM_GPORT_TRILL_PORT_ID_GET(trill_port);
    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {
        return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN(
         READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));
    network_port = soc_ING_DVP_TABLEm_field32_get(unit, 
                                       &dvp, NETWORK_PORTf);
    if (!network_port) {
        return _SHR_E_PORT;
    }

    l2mc_index = _BCM_MULTICAST_ID_GET(l2mc_group);

    /* When First TRILL-port is added to  L2MC-group, create L3MC-group */
    /* 1:1 mapping between L2MC-group  and L3MC-group */

    if (trill_info->multicast_count[l2mc_index].network_port_count == 0) {
        BCM_IF_ERROR_RETURN(
                   bcm_esw_multicast_create(unit, 
                                  BCM_MULTICAST_TYPE_TRILL, &l3mc_group));
        trill_info->multicast_count[l2mc_index].l3mc_group = l3mc_group;
        rv = bcm_esw_multicast_egress_add(unit, l3mc_group, trill_port, encap_id);
        if (BCM_FAILURE(rv)){
              rv = bcm_esw_multicast_destroy(unit, l3mc_group);
        } else {
              trill_info->multicast_count[l2mc_index].network_port_count++;
        }
    } else if (trill_info->multicast_count[l2mc_index].network_port_count > 0) {
        l3mc_group = trill_info->multicast_count[l2mc_index].l3mc_group;
        rv = bcm_esw_multicast_egress_add(unit, l3mc_group, trill_port, encap_id);
        if (BCM_SUCCESS(rv)) {
              trill_info->multicast_count[l2mc_index].network_port_count++;
        }
    }
    return rv;
}

/*
 * Purpose:
 *      Delete TRILL Gports from L2MC
 * Parameters:
 *      unit         - (IN) Unit number.
 *      port         - (IN) Gport.
 *      mc_index        - (IN) Multicast Index
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td_trill_egress_delete(int unit, bcm_gport_t trill_port, 
                                                           bcm_multicast_t l2mc_group, bcm_if_t encap_id)
{
    _bcm_td_trill_bookkeeping_t *trill_info;
    int vp=0;
    int network_port=0;
    ing_dvp_table_entry_t dvp;
    int rv = BCM_E_NONE;
    bcm_multicast_t  l3mc_group;
    int l2mc_index;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    trill_info = TRILL_INFO(unit);

    sal_memset(&l3mc_group, 0, sizeof(bcm_multicast_t));

    if (!BCM_GPORT_IS_TRILL_PORT(trill_port)) {
        return (BCM_E_BADID);
    }

    vp = BCM_GPORT_TRILL_PORT_ID_GET(trill_port);
    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {
        return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN(
         READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));
    network_port = soc_ING_DVP_TABLEm_field32_get(unit, 
                                       &dvp, NETWORK_PORTf);

    l2mc_index = _BCM_MULTICAST_ID_GET(l2mc_group);
    l3mc_group = trill_info->multicast_count[l2mc_index].l3mc_group;
    BCM_IF_ERROR_RETURN (
             bcm_esw_multicast_egress_delete(unit, l3mc_group, 
                                             trill_port, encap_id));
    trill_info->multicast_count[l2mc_index].network_port_count--;

    if (trill_info->multicast_count[l2mc_index].network_port_count == 0) {
         BCM_IF_ERROR_RETURN (bcm_esw_multicast_destroy(unit, l3mc_group));
         trill_info->multicast_count[l2mc_index].l3mc_group = 0;
    }
    return rv;
}

/*
 * Function:
 *      _bcm_td_trill_multicast_source_entry_key_set
 * Purpose:
 *      Set TRILL Multicast RPF entry key
 * Parameters:
 *  IN :  Unit
 *  IN :  root_name
 *  IN :  source_name
 *  IN :  gport to desired-neighbor
 *  IN :  entry pointer
 * Returns:
 *       BCM_E_XXX
 */

STATIC int
 _bcm_td_trill_multicast_source_entry_key_set(int unit, 
                                         bcm_trill_name_t root_name, bcm_trill_name_t source_name, 
                                         bcm_gport_t gport, l3_entry_ipv4_unicast_entry_t *l3_key)
 {
    int rv = BCM_E_NONE;
    uint8  trill_tree_id=0;
    bcm_module_t  local_modid=0;
    bcm_port_t    local_port=0;
    bcm_trunk_t  local_tgid=0;
    int local_id=0;

    (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);
    rv = _bcm_esw_gport_resolve(unit, gport, &local_modid,
                                       &local_port, &local_tgid, &local_id);
    if (rv < 0) {
         return SOC_E_UNAVAIL;
    }

    soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, l3_key, 
                   TRILL__INGRESS_RBRIDGE_NICKNAMEf, source_name);
    soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, l3_key, 
                   TRILL__TREE_IDf, trill_tree_id);
    soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, l3_key, KEY_TYPEf,
                   TR_L3_HASH_KEY_TYPE_TRILL);
    if (BCM_TRUNK_INVALID != local_tgid) {
         soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, l3_key, 
                   TRILL__EXPECTED_TGIDf, local_tgid);
         soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, l3_key, 
                   TRILL__EXPECTED_Tf, 0x1);
    } else {
         soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, l3_key, 
                   TRILL__EXPECTED_MODULE_IDf, local_modid);
         soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, l3_key, 
                   TRILL__EXPECTED_PORT_NUMf, local_port);
    }

    soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, l3_key, VALIDf, 1);
    return rv;
 }

/*
 * Purpose:
 *       Obtain Trill Nickname index from name
 * Parameters:
 *       unit  - Device Number
 *       source_trill_name  - (IN) Trill Root_RBridge
 */

int
bcm_td_trill_source_trill_name_idx_get(int unit, 
                                                                   bcm_trill_name_t source_trill_name,
                                                                   int *trill_name_index)
{
    egr_trill_rbridge_nicknames_entry_t  egr_name_entry;
    int idx;

      for(idx=0; idx < 4; idx++) {
         BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, EGR_TRILL_RBRIDGE_NICKNAMESm,
                       MEM_BLOCK_ALL, idx, &egr_name_entry));

         if ( source_trill_name ==  soc_mem_field32_get(unit, 
                                         EGR_TRILL_RBRIDGE_NICKNAMESm,
                                         &egr_name_entry,
                                         RBRIDGE_NICKNAMEf) ) {
              *trill_name_index =  idx;
              return BCM_E_NONE;
         }
    }
    return BCM_E_NOT_FOUND;

}

/*
 * Purpose:
 *       Obtain Trill Nickname from index
 * Parameters:
 *       unit  - Device Number
 *       source_trill_name  - (IN) Trill Root_RBridge
 */

int
bcm_td_trill_source_trill_idx_name_get(int  unit, 
                                                                   int  trill_name_index,
                                                                   bcm_trill_name_t  *source_trill_name)
{
    egr_trill_rbridge_nicknames_entry_t  egr_name_entry;

    BCM_IF_ERROR_RETURN(
              soc_mem_read(unit, EGR_TRILL_RBRIDGE_NICKNAMESm,
                       MEM_BLOCK_ALL, trill_name_index, &egr_name_entry));

    *source_trill_name =  soc_mem_field32_get(unit, 
                                         EGR_TRILL_RBRIDGE_NICKNAMESm,
                                         &egr_name_entry,
                                         RBRIDGE_NICKNAMEf);

    return BCM_E_NONE;
}

/*
 * Purpose:
 *       Add TRILL multicast RPF  entry
 * Parameters:
 *      unit  - Device Number
 *      root_name  - (IN) Trill Root_RBridge
 *      source_name - (IN) Source Rbridge
 *      port - (IN) Ingress port
 */

int
bcm_td_trill_multicast_source_add( int unit,
                                                                     bcm_trill_name_t root_name, 
                                                                     bcm_trill_name_t source_name, 
                                                                     bcm_gport_t gport)
{
    l3_entry_ipv4_unicast_entry_t l3_key, l3_entry;
    int rv = BCM_E_NONE;
    int l3_entry_index=0;

    sal_memset(&l3_key, 0, sizeof(l3_entry_ipv4_unicast_entry_t));
    sal_memset(&l3_entry, 0, sizeof(l3_entry_ipv4_unicast_entry_t));

   /* Check for Gport type - Mod Port or TRUNK Port */
    if ( (SOC_GPORT_IS_TRUNK(gport)) || 
          (SOC_GPORT_IS_LOCAL(gport)) || 
          (SOC_GPORT_IS_DEVPORT(gport)) || 
          (SOC_GPORT_IS_MODPORT(gport))) {

         BCM_IF_ERROR_RETURN(
              _bcm_td_trill_multicast_source_entry_key_set(unit, root_name, 
                        source_name, gport, &l3_key));

    } else {
            return BCM_E_PARAM;
    }


    rv = soc_mem_search(unit, L3_ENTRY_IPV4_UNICASTm, 
                        MEM_BLOCK_ANY, &l3_entry_index,
                        &l3_key, &l3_entry, 0);
    if (rv == SOC_E_NONE) {
         rv = soc_mem_write(unit, L3_ENTRY_IPV4_UNICASTm,
                        MEM_BLOCK_ALL, l3_entry_index, &l3_key);
    } else if (rv != SOC_E_NOT_FOUND) {
        return rv;
    } else {
         rv = soc_mem_insert(unit, L3_ENTRY_IPV4_UNICASTm, MEM_BLOCK_ALL, &l3_key);
    }
    return rv;
}

/*
 * Purpose:
 *       Delete TRILL multicast RPF  entry
 * Parameters:
 *             unit  - Device Number
 *             root_name  - (IN) Trill Root_RBridge
 *             source_name - (IN) Source Rbridge
 *             port - (IN) Ingress port
 */

int
bcm_td_trill_multicast_source_delete( int unit,
                                                                         bcm_trill_name_t root_name, 
                                                                         bcm_trill_name_t source_name, 
                                                                         bcm_gport_t gport)
{
    l3_entry_ipv4_unicast_entry_t l3_entry;
    int rv = BCM_E_UNAVAIL;
    int l3_entry_index=0;

    sal_memset(&l3_entry, 0, sizeof(l3_entry_ipv4_unicast_entry_t));
   /* Check for Gport type - Mod Port or TRUNK Port */
    if ( (SOC_GPORT_IS_TRUNK(gport)) || 
          (SOC_GPORT_IS_LOCAL(gport)) || 
          (SOC_GPORT_IS_DEVPORT(gport)) || 
          (SOC_GPORT_IS_MODPORT(gport))) {

         (void) _bcm_td_trill_multicast_source_entry_key_set(unit, root_name, 
                        source_name, gport, &l3_entry);
    } else {
            return BCM_E_PARAM;
    }

    rv = soc_mem_search(unit, L3_ENTRY_IPV4_UNICASTm, MEM_BLOCK_ANY, &l3_entry_index,
                             &l3_entry, &l3_entry, 0);
    if (rv < 0) {
         return rv;
    }
   
    /* Delete the entry from HW */
    rv = soc_mem_delete(unit, L3_ENTRY_IPV4_UNICASTm, MEM_BLOCK_ALL, &l3_entry);
    
    if ( (rv != BCM_E_NOT_FOUND) && (rv != BCM_E_NONE) ) {
       return rv;
    } else {
       return BCM_E_NONE;
    }
}

/*
 * Purpose:
 *       Get TRILL multicast RPF  entry
 * Parameters:
 *             unit - Device Number
 *             root_name  - (IN) Trill Root_RBridge
 *             source_name - (IN) Source Rbridge
 *             port  - (OUT) Ingress port
 */

int
bcm_td_trill_multicast_source_get( int unit,
                                                                    bcm_trill_name_t root_name, 
                                                                    bcm_trill_name_t source_name, 
                                                                    bcm_gport_t *gport)
{
    l3_entry_ipv4_unicast_entry_t l3_key, l3_entry;
    int l3_entry_index;
    uint8  trill_tree_id=0;
    bcm_module_t  module_id, mod_out=0;
    bcm_port_t    port_num, port_out=0;
    bcm_trunk_t  tgid;

    sal_memset(&l3_key, 0, sizeof(l3_entry_ipv4_unicast_entry_t));
 
    (void) bcm_td_trill_tree_profile_get (unit, root_name, &trill_tree_id);

    soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, &l3_key, TRILL__INGRESS_RBRIDGE_NICKNAMEf, source_name);

    soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, &l3_key, TRILL__TREE_IDf, trill_tree_id);

    soc_L3_ENTRY_IPV4_UNICASTm_field32_set(unit, &l3_key, KEY_TYPEf,
                             TR_L3_HASH_KEY_TYPE_TRILL);

    BCM_IF_ERROR_RETURN (
    soc_mem_search(unit, L3_ENTRY_IPV4_UNICASTm, MEM_BLOCK_ANY,
                             &l3_entry_index, &l3_key, &l3_entry, 0));

    if (soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, &l3_entry, TRILL__EXPECTED_TGIDf)) {
         tgid = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit,
                             &l3_entry, TRILL__EXPECTED_TGIDf);
         SOC_GPORT_TRUNK_SET(*gport, tgid);
    } else {
         module_id = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit,
                             &l3_entry, TRILL__EXPECTED_MODULE_IDf);
         port_num = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit,
                             &l3_entry, TRILL__EXPECTED_PORT_NUMf);
         BCM_IF_ERROR_RETURN (
                   _bcm_gport_modport_hw2api_map(unit, module_id,
                                                                          port_num, &mod_out,
                                                                          &port_out));
         SOC_GPORT_MODPORT_SET(*gport, mod_out, port_out);
    }
    return BCM_E_NONE;
}

/* Purpose:
*	   Traverse all valid TRILL Multicast RPF entries and call the
*	supplied callback routine.
* Parameters:
*	   unit   - Device Number
*	   cb		 - User callback function, called once per Trill Multicast entry.
*	   user_data - cookie
*/

int
bcm_td_trill_multicast_source_traverse(int unit, bcm_trill_multicast_source_traverse_cb  cb,
                                                        void *user_data)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_tr_trill_stat_get
 * Purpose:
 *      Get TRILL Stats
 * Parameters:
 *      unit   - (IN) SOC unit #
 *      port   - (IN) TRILL port
 *      stat   - (IN)  specify the Stat type
 *      val    - (OUT) 64-bit Stats value
 * Returns:
 *      BCM_E_XXX
 */     

int
bcm_td_trill_stat_get(int unit, bcm_port_t port,
                           bcm_trill_stat_t stat, uint64 *val)
{
    int rv=BCM_E_NONE;
    uint32 reg_val;
    uint32  val_low;
    trill_drop_stats_entry_t  drop;
    bcm_port_t    local_port=-1;
    int local_id=-1;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
         if ( (SOC_GPORT_IS_LOCAL(port)) || 
               (SOC_GPORT_IS_DEVPORT(port)) || 
               (SOC_GPORT_IS_MODPORT(port))) {
                   BCM_IF_ERROR_RETURN
                      (bcm_esw_port_local_get(unit, port, &local_port));
         } else if (SOC_GPORT_IS_TRILL_PORT(port)) {
              /* Flex stats not supported for TRILL port */
              return BCM_E_UNAVAIL;
         } else {
              return BCM_E_PARAM;
         }
   } else if (SOC_GPORT_INVALID == port) {
         local_id = 0;
   } else {
        local_port = port;
   }

    COMPILER_64_ZERO(*val);

    switch (stat) {

         case bcmTrillInPkts:
           if (local_port != -1) {
              SOC_IF_ERROR_RETURN
                     (READ_ING_TRILL_RX_PKTSr(unit, local_port, &reg_val));

              val_low = 
                    soc_reg_field_get(unit, ING_TRILL_RX_PKTSr, 
                                                   reg_val, COUNTf);
              COMPILER_64_SET(*val, 0, val_low);
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillOutPkts:
           if (local_port != -1) {
              SOC_IF_ERROR_RETURN
                     (READ_EGR_TRILL_TX_PKTSr(unit, local_port, &reg_val));

              val_low = 
                    soc_reg_field_get(unit, EGR_TRILL_TX_PKTSr, 
                                                   reg_val, COUNTf);
              COMPILER_64_SET(*val, 0, val_low);     
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillErrorPkts:
           if (local_id == 0) {
              BCM_IF_ERROR_RETURN(
                   soc_mem_read(unit, TRILL_DROP_STATSm,
                        MEM_BLOCK_ALL, local_id, &drop));
              val_low = 
                   soc_mem_field32_get(unit, TRILL_DROP_STATSm, 
                        &drop, TRILL_ERROR_DROPSf);
              COMPILER_64_SET(*val, 0, val_low);     
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillNameMissPkts:             
           if (local_id == 0) {
              BCM_IF_ERROR_RETURN(
                   soc_mem_read(unit, TRILL_DROP_STATSm,
                        MEM_BLOCK_ALL, local_id, &drop));
              val_low = 
                   soc_mem_field32_get(unit, TRILL_DROP_STATSm, 
                        &drop, TRILL_RBRIDGE_LOOKUP_MISS_DROPSf);
              COMPILER_64_SET(*val, 0, val_low);     
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillRpfFailPkts:
           if (local_id == 0) {
              BCM_IF_ERROR_RETURN(
                   soc_mem_read(unit, TRILL_DROP_STATSm,
                        MEM_BLOCK_ALL, local_id, &drop));
              val_low = 
                   soc_mem_field32_get(unit, TRILL_DROP_STATSm, 
                        &drop, TRILL_RPF_CHECK_FAIL_DROPSf);
              COMPILER_64_SET(*val, 0, val_low);     
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillTtlFailPkts:            
           if (local_id == 0) {
              BCM_IF_ERROR_RETURN(
                   soc_mem_read(unit, TRILL_DROP_STATSm,
                        MEM_BLOCK_ALL, local_id, &drop));
              val_low = 
                   soc_mem_field32_get(unit, TRILL_DROP_STATSm, 
                        &drop, TRILL_HOPCOUNT_CHECK_FAIL_DROPSf);
              COMPILER_64_SET(*val, 0, val_low);     
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         default:
                   break;

    }

    return rv;
}

int
bcm_td_trill_stat_get32(int unit, bcm_port_t port,
                             bcm_trill_stat_t stat, uint32 *val)
{
    int rv=BCM_E_NONE;
    uint64 val64;

    rv = bcm_td_trill_stat_get(unit, port, stat, &val64);
    if (rv == BCM_E_NONE) {
        *val = COMPILER_64_LO(val64);
    }
    return rv;
}

/*
 * Function:
 *      bcm_td_trill_stat_clear
 * Purpose:
 *      Clear TRILL Stats
 * Parameters:
 *      unit   - (IN) SOC unit #
 *      port   - (IN) TRILL port
 *      stat   - (IN)  specify the Stat type
 * Returns:
 *      BCM_E_XXX
 */     

int
bcm_td_trill_stat_clear(int unit, bcm_port_t port,
                             bcm_trill_stat_t stat)
{
    int rv=BCM_E_NONE;
    uint32 reg_val=0;
    trill_drop_stats_entry_t  drop;
    bcm_port_t    local_port=-1;
    int local_id=-1;

    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
         return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
         if ( (SOC_GPORT_IS_LOCAL(port)) || 
               (SOC_GPORT_IS_DEVPORT(port)) || 
               (SOC_GPORT_IS_MODPORT(port))) {
                   BCM_IF_ERROR_RETURN
                      (bcm_esw_port_local_get(unit, port, &local_port));
         } else if (SOC_GPORT_IS_TRILL_PORT(port)) {
              /* Flex stats not supported for TRILL port */
              return BCM_E_UNAVAIL;
         } else {
              return BCM_E_PARAM;
         }
   } else if (SOC_GPORT_INVALID == port) {
         local_id = 0;
   } else {
        local_port = port;
   }

    switch (stat) {

         case bcmTrillInPkts:

           if (local_port != -1) {
              soc_reg_field_set(unit, ING_TRILL_RX_PKTSr, 
                                                   &reg_val, COUNTf, 0);
              SOC_IF_ERROR_RETURN
                     (WRITE_ING_TRILL_RX_PKTSr(unit, local_port, reg_val));
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillOutPkts:

           if (local_port != -1) {
              soc_reg_field_set(unit, EGR_TRILL_TX_PKTSr, 
                                                   &reg_val, COUNTf, 0);
              SOC_IF_ERROR_RETURN
                     (WRITE_EGR_TRILL_TX_PKTSr(unit, local_port, reg_val));
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillErrorPkts:

           if (local_id == 0) {
              soc_mem_field32_set(unit, TRILL_DROP_STATSm, &drop,
                                            TRILL_ERROR_DROPSf, 0);
              BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, TRILL_DROP_STATSm,
                                            MEM_BLOCK_ALL, local_id, &drop));
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillNameMissPkts:
              
           if (local_id == 0) {
              soc_mem_field32_set(unit, TRILL_DROP_STATSm, &drop,
                                            TRILL_RBRIDGE_LOOKUP_MISS_DROPSf, 0);
              BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, TRILL_DROP_STATSm,
                                            MEM_BLOCK_ALL, local_id, &drop));
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillRpfFailPkts:
              
           if (local_id == 0) {
              soc_mem_field32_set(unit, TRILL_DROP_STATSm, &drop,
                                            TRILL_RPF_CHECK_FAIL_DROPSf, 0);
              BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, TRILL_DROP_STATSm,
                                            MEM_BLOCK_ALL, local_id, &drop));
           } else {
              return BCM_E_UNAVAIL;
           }
           break;

         case bcmTrillTtlFailPkts:
              
           if (local_id == 0) {
              soc_mem_field32_set(unit, TRILL_DROP_STATSm, &drop,
                                            TRILL_HOPCOUNT_CHECK_FAIL_DROPSf, 0);
              BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, TRILL_DROP_STATSm,
                                            MEM_BLOCK_ALL, local_id, &drop));
            } else {
              return BCM_E_UNAVAIL;
           }
           break;

         default:
                   break;

    }

    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_td_trill_sw_dump
 * Purpose:
 *     Displays Trill State information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_td_trill_sw_dump(int unit)
{
    int idx;
    _bcm_td_trill_bookkeeping_t *trill_info = TRILL_INFO(unit);
    
    soc_cm_print("\nRoot Bridges:\n");
    for (idx = 0; idx < BCM_MAX_NUM_TRILL_TREES; idx++) {
        soc_cm_print("%d ", trill_info->rootBridge[idx]);
    }
    
    soc_cm_print("\n\nrBridges:\n");
    for (idx = 0; idx < BCM_MAX_NUM_VP; idx++) {
        if (trill_info->rBridge[idx] != 0) {
            soc_cm_print("Index:%d rBridge nickname:%d\n", idx,
                                      trill_info->rBridge[idx]);
        }
    }

    soc_cm_print("\n\nMulticast use count:\n");
    for (idx = 0; idx < BCM_MAX_TRILL_IPMC_GROUPS; idx++) {
        if (trill_info->multicast_count[idx].network_port_count != 0) {
            soc_cm_print("    Multicast group %d use count:%d\n",
                trill_info->multicast_count[idx].l3mc_group,
                trill_info->multicast_count[idx].network_port_count);
        }
    }
    
    return;
}

#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */
