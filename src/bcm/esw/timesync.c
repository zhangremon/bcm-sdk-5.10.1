/*
 * $Id: timesync.c 1.2.2.3 Broadcom SDK $
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
 * Timesync Configurations
 * Purpose: API to set and get timesync config profiles for port. 
 */

#include <soc/defs.h>
#include <sal/core/libc.h>
#if defined(BCM_KATANA_SUPPORT)
#include <soc/drv.h>
#include <soc/hash.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>

#include <bcm/error.h>
#include <bcm/port.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l3.h>
#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/katana.h>
#endif

#include <bcm_int/esw/xgs3.h>
#include <bcm_int/esw_dispatch.h>

/* Timesync module lock */
STATIC sal_mutex_t _bcm_esw_timesync_mutex[BCM_MAX_NUM_UNITS] = {NULL};

/* Timesync control profile    */
static soc_profile_mem_t * _bcm_esw_timesync_control_profile[BCM_MAX_NUM_UNITS]; 

/* Timesync defines */
#define TIMESYNC_CONTROL_PROFILE(_unit_) \
                        _bcm_esw_timesync_control_profile[unit]

#define TIMESYNC_MPLS_LABEL_PROFILE(_unit_) \
                        _bcm_esw_timesync_mpls_label_profile[unit]

/* Timesync Profile index defines */
#define TS_PORT_INDEX               0
#define TIMESYNC_MAX_PROFILE        1


/*
 * TIMESYNC module lock
 */
#define TIMESYNC_MODULE_LOCK(unit) \
        sal_mutex_take(_bcm_esw_timesync_mutex[unit], sal_mutex_FOREVER);

#define TIMESYNC_MODULE_UNLOCK(unit) \
        sal_mutex_give(_bcm_esw_timesync_mutex[unit]);

/*
 * Function:
 *      _bcm_esw_port_timesync_profile_init
 * Purpose: 
 *      This function initializes timesync port and mpls profiles
 *
 * Parameters: 
 *      unit       - (IN) bcm device
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
_bcm_esw_port_timesync_profile_init(int unit)
{
    soc_mem_t   mem = 0;
    int         entry_words, alloc_size=0;
    void        *entry;
    uint32      base_index;
    int         rv = BCM_E_NONE;
  
    /*
     * Initialize Timesync control profile 
     */
    if (TIMESYNC_CONTROL_PROFILE(unit) == NULL) {
        TIMESYNC_CONTROL_PROFILE(unit) = sal_alloc (sizeof(soc_profile_mem_t),
                                              "Timesync control profile mem");
        if (TIMESYNC_CONTROL_PROFILE(unit) == NULL) {
            return BCM_E_MEMORY;
        }
        soc_profile_mem_t_init(TIMESYNC_CONTROL_PROFILE(unit));
    } else {
        soc_profile_mem_destroy(unit, TIMESYNC_CONTROL_PROFILE(unit));
    }


#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        mem =  ING_1588_TS_DISPOSITION_PROFILE_TABLEm;
        entry_words = sizeof(ing_1588_ts_disposition_profile_table_entry_t) / sizeof(uint32);
        alloc_size = sizeof(ing_1588_ts_disposition_profile_table_entry_t);
    }
#endif /*defined(BCM_KATANA_SUPPORT) */

    SOC_IF_ERROR_RETURN
        (soc_profile_mem_create(unit, &mem, &entry_words, 1,
                                TIMESYNC_CONTROL_PROFILE(unit)));

    /* Add timesync control default entry */
    entry = sal_alloc(alloc_size, "Timesync control profile mem");
    if (entry == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(entry, 0, alloc_size);
    rv = soc_profile_mem_add(unit, TIMESYNC_CONTROL_PROFILE(unit), &entry, 1,
                             &base_index);
    sal_free(entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_esw_port_timesync_profile_delete
 *      _bcm_esw_port_timesync_profile_delete
 * Purpose: 
 *      This function deletes timesync config profiles
 *
 * Parameters: 
 *      unit       - (IN) bcm device
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
_bcm_esw_port_timesync_profile_delete(int unit)
{
    /* Destroy Timesync Control profile */
    if (TIMESYNC_CONTROL_PROFILE(unit)) {
        BCM_IF_ERROR_RETURN(
            soc_profile_mem_destroy(unit, TIMESYNC_CONTROL_PROFILE(unit)));
        sal_free(TIMESYNC_CONTROL_PROFILE(unit));
        TIMESYNC_CONTROL_PROFILE(unit) = NULL;
    }


    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_port_timesync_contorl_profile_entry_add
 * Purpose: 
 *      Internal Funciton adds timesync control profiles
 *
 * Parameters: 
 *      unit      -  (IN) Device number.
 *      ts_config -  (IN) Timesync config 
 *      *index    -  (OUT) profile index
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
STATIC int
_bcm_esw_port_timesync_control_profile_entry_add(int unit, 
                                bcm_port_timesync_config_t *ts_config, uint32 *index)
{
    int         alloc_size=0;
    void        *entry;
    int         cnt, rv = BCM_E_NONE;

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        alloc_size = sizeof(ing_1588_ts_disposition_profile_table_entry_t);
    }
#endif 

    entry = sal_alloc(alloc_size, "Timesync control profile mem");
    if (entry == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(entry, 0, alloc_size);


    /* Set timesync message control tocpu/drop fields */

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        for ( cnt = 0; cnt < sizeof(uint32); cnt++ )
        {
            switch (ts_config->pkt_drop & (1 << cnt))
            {
                case BCM_PORT_TIMESYNC_PKT_SYNC:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_0_DROPf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_DELAY_REQ:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_1_DROPf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_REQ:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_2_DROPf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_RESP:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_3_DROPf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_FOLLOWUP:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_8_DROPf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_DELAY_RESP:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_9_DROPf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_RESP_FOLLOWUP:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_10_DROPf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_ANNOUNCE:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_11_DROPf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_SIGNALLING:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_12_DROPf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_MANAGMENT:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_13_DROPf, 1);
                    break;
                default:
                    break;
            }

            switch (ts_config->pkt_tocpu & (1 << cnt))
            {
                case BCM_PORT_TIMESYNC_PKT_SYNC:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_0_COPY_TO_CPUf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_DELAY_REQ:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_1_COPY_TO_CPUf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_REQ:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_2_COPY_TO_CPUf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_RESP:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_3_COPY_TO_CPUf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_FOLLOWUP:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_8_COPY_TO_CPUf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_DELAY_RESP:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_9_COPY_TO_CPUf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_RESP_FOLLOWUP:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_10_COPY_TO_CPUf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_ANNOUNCE:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_11_COPY_TO_CPUf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_SIGNALLING:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_12_COPY_TO_CPUf, 1);
                    break;
                case BCM_PORT_TIMESYNC_PKT_MANAGMENT:
                    soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_set(unit, entry, 
                                                MESSAGE_TYPE_13_COPY_TO_CPUf, 1);
                    break;
                default:
                    break;
            }
        }
    }
#endif /* BCM_KATANA_SUPPORT */
        
        /* lock and unlocktimesync module for profile operations */
        TIMESYNC_MODULE_LOCK(unit);
        rv = soc_profile_mem_add(unit, TIMESYNC_CONTROL_PROFILE(unit), &entry, 1,
                                  index);
        TIMESYNC_MODULE_UNLOCK(unit);
        sal_free(entry);
        return rv;
}

/*
 * Function:
 *      _bcm_esw_port_timesync_contorl_profile_entry_get
 * Purpose: 
 *      Interal function gets timesync control profiles for the index
 *
 * Parameters: 
 *      unit      -  (IN) Device number.
 *      ts_config -  (OUT) Timesync config 
 *      index    -   (IN) profile index
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_bcm_esw_port_timesync_control_profile_entry_get(int unit, 
                                bcm_port_timesync_config_t *ts_config, uint32 index)
{
    int         alloc_size=0;
    void        *entry;
    int         cnt, rv = BCM_E_NONE;

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        alloc_size = sizeof(ing_1588_ts_disposition_profile_table_entry_t);
    }
#endif /* BCM_KATANA_SUPPORT */

    entry = sal_alloc(alloc_size, "Timesync control profile mem");
    if (entry == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(entry, 0, alloc_size);

    TIMESYNC_MODULE_LOCK(unit);
    rv = soc_profile_mem_get(unit, TIMESYNC_CONTROL_PROFILE(unit), index, 1, &entry );
    if (BCM_FAILURE(rv)) {
        sal_free(entry);
        TIMESYNC_MODULE_UNLOCK(unit);
        return rv;
    }

    /* Set timesync message control tocpu/drop fields */

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        for ( cnt = 0; cnt < sizeof(uint32); cnt++ )
        {
            switch (1 << cnt)
            {
                case BCM_PORT_TIMESYNC_PKT_SYNC:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                           unit, entry,MESSAGE_TYPE_0_DROPf)) {
                        ts_config->pkt_drop |=  BCM_PORT_TIMESYNC_PKT_SYNC; 
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_DELAY_REQ:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                           unit, entry, MESSAGE_TYPE_1_DROPf)) {
                        ts_config->pkt_drop |= BCM_PORT_TIMESYNC_PKT_DELAY_REQ;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_REQ:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                           unit, entry, MESSAGE_TYPE_2_DROPf)) {
                        ts_config->pkt_drop |= BCM_PORT_TIMESYNC_PKT_PDELAY_REQ;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_RESP:
                    if(soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                       unit, entry, MESSAGE_TYPE_3_DROPf)) {
                        ts_config->pkt_drop |= BCM_PORT_TIMESYNC_PKT_PDELAY_RESP;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_FOLLOWUP:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                       unit, entry, MESSAGE_TYPE_8_DROPf)) {
                        ts_config->pkt_drop |= BCM_PORT_TIMESYNC_PKT_FOLLOWUP;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_DELAY_RESP:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                       unit, entry, MESSAGE_TYPE_9_DROPf)) {
                        ts_config->pkt_drop |= BCM_PORT_TIMESYNC_PKT_DELAY_RESP;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_RESP_FOLLOWUP:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                      unit, entry, MESSAGE_TYPE_10_DROPf)) {
                        ts_config->pkt_drop |=
                                BCM_PORT_TIMESYNC_PKT_PDELAY_RESP_FOLLOWUP;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_ANNOUNCE:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                      unit, entry, MESSAGE_TYPE_11_DROPf)) {
                        ts_config->pkt_drop |= BCM_PORT_TIMESYNC_PKT_ANNOUNCE;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_SIGNALLING:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                      unit, entry, MESSAGE_TYPE_12_DROPf)) {
                        ts_config->pkt_drop |= BCM_PORT_TIMESYNC_PKT_SIGNALLING;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_MANAGMENT:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                       unit, entry, MESSAGE_TYPE_13_DROPf)) {
                        ts_config->pkt_drop |= BCM_PORT_TIMESYNC_PKT_MANAGMENT;
                    }
                    break;
                default:
                    break;
            }

            switch (1 << cnt)
            {
                case BCM_PORT_TIMESYNC_PKT_SYNC:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                          unit, entry, MESSAGE_TYPE_0_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |=  BCM_PORT_TIMESYNC_PKT_SYNC; 
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_DELAY_REQ:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                          unit, entry, MESSAGE_TYPE_1_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_DELAY_REQ;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_REQ:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                    unit, entry, MESSAGE_TYPE_2_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_PDELAY_REQ;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_RESP:
                    if(soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                     unit, entry, MESSAGE_TYPE_3_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_PDELAY_RESP;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_FOLLOWUP:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                     unit, entry, MESSAGE_TYPE_8_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_FOLLOWUP;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_DELAY_RESP:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                     unit, entry, MESSAGE_TYPE_9_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_DELAY_RESP;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_PDELAY_RESP_FOLLOWUP:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                     unit, entry, MESSAGE_TYPE_10_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |=
                                BCM_PORT_TIMESYNC_PKT_PDELAY_RESP_FOLLOWUP;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_ANNOUNCE:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                     unit, entry, MESSAGE_TYPE_11_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_ANNOUNCE;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_SIGNALLING:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                     unit, entry, MESSAGE_TYPE_12_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_SIGNALLING;
                    }
                    break;
                case BCM_PORT_TIMESYNC_PKT_MANAGMENT:
                    if (soc_ING_1588_TS_DISPOSITION_PROFILE_TABLEm_field32_get(
                                     unit, entry, MESSAGE_TYPE_13_COPY_TO_CPUf)) {
                        ts_config->pkt_tocpu |= BCM_PORT_TIMESYNC_PKT_MANAGMENT;
                    }
                    break;
                default:
                    break;
            }
        }
    }
#endif /* BCM_KATANA_SUPPORT */

        sal_free(entry);
        TIMESYNC_MODULE_UNLOCK(unit);
        return rv;
}

/*
 * Function:
 *      _bcm_esw_port_timesync_port_profile_entry_delete
 * Purpose: 
 *      Internal function to delete timesync profiles
 *
 * Parameters: 
 *      unit       - (IN) bcm device
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
STATIC int
_bcm_esw_port_timesync_port_profile_entry_delete(int unit, int index)
{
    int rv;
    TIMESYNC_MODULE_LOCK(unit);
    rv = soc_profile_mem_delete(unit, TIMESYNC_CONTROL_PROFILE(unit), index);
    TIMESYNC_MODULE_UNLOCK(unit);
    return rv;
}


/*
 * Function: 
 *      _bcm_esw__port_timesync_config_set
 * Purpose: 
 *      Set Timesync Configuration profiles for the port
 *
 * Parameters: 
 *      unit            - (IN) bcm device
 *      port            - (IN) port
 *      config_count    - (IN)Count of timesync profile
 *      *config_array   - (IN/OUT) Pointer to array of timesync profiles
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 *     Timesync config set function cleans up previous profiles before setting new
 *     profiles. katana support one timesync port profile.
 */
int
_bcm_esw_port_timesync_config_set(int unit, bcm_port_t port, int config_count,
                                  bcm_port_timesync_config_t *config_array)
{
    int                 cnt, rv = BCM_E_NONE;
    int                 prev_index = 0;
    uint32              rval, index;
    soc_mem_t           mem;
    egr_1588_sa_entry_t sa_entry;
    port_tab_entry_t    ptab;
    bcm_port_timesync_config_t *ts_config;

    /* Validate Parameters */
    if ((config_count == 0) || (config_count > TIMESYNC_MAX_PROFILE)
         || (NULL == config_array)) {
        return BCM_E_PARAM;
    }

    /* Set timesync profiles */
    for (cnt = 0; cnt < config_count; cnt++)
    {
        ts_config = (bcm_port_timesync_config_t*) (config_array + cnt);
        
        if (ts_config->flags & BCM_PORT_TIMESYNC_DEFAULT) {
            /* Set One-stemp timestamp configurations */
            if ( ts_config->flags & BCM_PORT_TIMESYNC_ONE_STEP_TIMESTAMP ) {
    
                /* Enable Correction updates for ingress and egress */
                SOC_IF_ERROR_RETURN(
                      soc_reg_field32_modify(unit, EGR_1588_EGRESS_CTRLr, port,
                                             CF_UPDATE_ENABLEf, 1));
                SOC_IF_ERROR_RETURN(
                      soc_reg_field32_modify(unit, EGR_1588_INGRESS_CTRLr, port,
                                             CF_UPDATE_ENABLEf, 1));

                /* Enable Source Address update for corrections */
                if (BCM_MAC_IS_ZERO(ts_config->src_mac_addr)) {
                    SOC_IF_ERROR_RETURN(
                        soc_reg_field32_modify(unit, EGR_1588_INGRESS_CTRLr,
                                               port, SA_UPDATE_ENABLEf, 1));
                    sal_memset(&sa_entry, 0, sizeof(egr_1588_sa_entry_t));
                    soc_mem_field_set(unit, EGR_1588_SAm ,(uint32 *)&sa_entry,
                                      SAf, (uint32*)ts_config->src_mac_addr);
                    BCM_IF_ERROR_RETURN( 
                        soc_mem_write(unit, EGR_1588_SAm, MEM_BLOCK_ALL, 
                                      port, &sa_entry));
                }
            }

                /* Set two-step correction update enable */
            if ( ts_config->flags & BCM_PORT_TIMESYNC_TWO_STEP_TIMESTAMP) {
                /* Set two-stemp timstamping in mac for all event pkts */
                SOC_IF_ERROR_RETURN(
                    READ_EGR_1588_EGRESS_CTRLr(unit, port,&rval));
                soc_reg_field_set(unit, EGR_1588_EGRESS_CTRLr, &rval, 
                                  TX_TS_SYNCf, 1);
                soc_reg_field_set(unit, EGR_1588_EGRESS_CTRLr, &rval, 
                                  TX_TS_DELAY_REQf, 1);
                soc_reg_field_set(unit, EGR_1588_EGRESS_CTRLr, &rval, 
                                  TX_TS_PDELAY_REQf, 1);
                soc_reg_field_set(unit, EGR_1588_EGRESS_CTRLr, &rval, 
                                  TX_TS_PDELAY_RESPf, 1);

                /* Sign extension from timestamp value */
                soc_reg_field_set(unit, EGR_1588_EGRESS_CTRLr, &rval, 
                                  FORCE_ITS_SIGN_FROM_TSf, 1);
                SOC_IF_ERROR_RETURN(
                    WRITE_EGR_1588_EGRESS_CTRLr(unit, port, rval));
            }

            /* Create timesync control index */
            rv = _bcm_esw_port_timesync_control_profile_entry_add(unit, ts_config,
                                                              &index);
            if (BCM_FAILURE(rv)) {
                return rv;
            }

            soc_mem_lock(unit, PORT_TABm);;
            mem = SOC_PORT_MEM_TAB(unit, port);
            rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, port, &ptab);
            if (BCM_FAILURE(rv)) {
                soc_mem_unlock(unit, PORT_TABm);;
                return BCM_E_FAIL;

            }

#if defined(BCM_KATANA_SUPPORT)
            if (SOC_IS_KATANA(unit)) {
                prev_index = soc_PORT_TABm_field32_get(unit,  &ptab, 
                                          ING_1588_TS_PROFILE_PTRf);
                soc_PORT_TABm_field32_set(unit, &ptab, 
                                          ING_1588_TS_PROFILE_PTRf, index);
            }
#endif
            rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, port, &ptab);
            soc_mem_unlock(unit, PORT_TABm);;
           
            /* delete old config profile */ 
            if (prev_index > 0) {
                _bcm_esw_port_timesync_port_profile_entry_delete(unit, prev_index);
            } 

        }

    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_port_timesync_config_get
 * Purpose: 
 *      Get Timesync configuration profiles for the port.
 *
 * Parameters: 
 *      unit            - (IN) bcm device
 *      port            - (IN) port
 *      array_size      - (IN) Count of timesync profile
 *      *config_array   - (IN/OUT) Pointer to array of timesync profiles
 *      *array_count    - (OUT) Pointer to array of timesync profiles
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 *     Timesync config get function is overloaded to return supported profile 
 *     counts when config_array is passed as null. 
 */
int
_bcm_esw_port_timesync_config_get(int unit, bcm_port_t port, int array_size,
                                  bcm_port_timesync_config_t *config_array, 
                                  int *array_count)
{
    int                 cnt, config_cnt =0, rv = BCM_E_NONE;
    uint32              rval, rval_1, index, value;
    uint32              ts_index[TIMESYNC_MAX_PROFILE];
    soc_mem_t           mem;
    egr_1588_sa_entry_t sa_entry;
    port_tab_entry_t    ptab;
    bcm_port_timesync_config_t *ts_config;

    /* Check for array_count */
    if ((NULL == array_count) ||
       (array_size > 0 && NULL == config_array)) {
        return BCM_E_PARAM;
    }

    /* Read Port Table, do limited operations, holding port tab memory lock*/
    mem = SOC_PORT_MEM_TAB(unit, port);
    sal_memset(ts_index, 0, sizeof(ts_index));
    soc_mem_lock(unit, PORT_TABm);;
    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, port, &ptab);
    if (BCM_FAILURE(rv)) {
        soc_mem_unlock(unit, PORT_TABm);;
        return rv;
    }

    index = soc_PORT_TABm_field32_get(unit,  &ptab, 
                                      ING_1588_TS_PROFILE_PTRf);
    if (index) {
        ts_index[TS_PORT_INDEX] = index;
        config_cnt++;
    }

    /* Release the port memory lock */
    soc_mem_unlock(unit, PORT_TABm);;

    /* update config cnt */
    *array_count = config_cnt;

    /* if config_array is null, return array count */
    if ((NULL == config_array)) {
        return BCM_E_NONE;
    }

    /* lock timesync module */ 
    TIMESYNC_MODULE_LOCK(unit);

    for (cnt = 0; (cnt < array_size) && (array_size <= config_cnt); cnt++)
    {
        ts_config = (bcm_port_timesync_config_t*) (config_array + cnt );

        /* Get timesync port control profile */
        if (cnt == TS_PORT_INDEX && ts_index[TS_PORT_INDEX]) {
            ts_config->flags |= BCM_PORT_TIMESYNC_DEFAULT;
            index = ts_index[TS_PORT_INDEX]; 
            value = 0;
            SOC_IF_ERROR_RETURN(
                    READ_EGR_1588_EGRESS_CTRLr(unit, port,&rval));
            rval_1 = rval;

            value |= soc_reg_field_get(unit, EGR_1588_EGRESS_CTRLr, rval, 
                                      TX_TS_SYNCf);
            value |= soc_reg_field_get(unit, EGR_1588_EGRESS_CTRLr, rval, 
                                       TX_TS_DELAY_REQf);
            value |= soc_reg_field_get(unit, EGR_1588_EGRESS_CTRLr, rval, 
                                       TX_TS_PDELAY_REQf);
            value |= soc_reg_field_get(unit, EGR_1588_EGRESS_CTRLr, rval, 
                                       TX_TS_PDELAY_RESPf);
            value |= soc_reg_field_get(unit, EGR_1588_EGRESS_CTRLr, rval, 
                                       FORCE_ITS_SIGN_FROM_TSf);
            if (value) {
               ts_config->flags|=  BCM_PORT_TIMESYNC_TWO_STEP_TIMESTAMP;
            }
            
            value = soc_reg_field_get(unit, EGR_1588_EGRESS_CTRLr, rval,
                                      CF_UPDATE_ENABLEf) && 
                    soc_reg_field_get(unit, EGR_1588_INGRESS_CTRLr, rval_1, 
                                      CF_UPDATE_ENABLEf);
            if (value) {
               ts_config->flags |=  BCM_PORT_TIMESYNC_ONE_STEP_TIMESTAMP;

               if (soc_reg_field_get(unit, EGR_1588_INGRESS_CTRLr, rval_1, 
                                      SA_UPDATE_ENABLEf)) {
                    sal_memset(&sa_entry, 0, sizeof(egr_1588_sa_entry_t));
                    soc_mem_field_get(unit, EGR_1588_SAm ,(uint32 *)&sa_entry,
                                      SAf, (uint32*)ts_config->src_mac_addr);
                }
            }

            rv = _bcm_esw_port_timesync_control_profile_entry_get(unit, ts_config,
                                                           index);
            if (BCM_FAILURE(rv)) {
                TIMESYNC_MODULE_UNLOCK(unit);
                return rv;
            }
        }

    }
    return rv;
} 

#endif /*defined(BCM_KATANA_SUPPORT) */
