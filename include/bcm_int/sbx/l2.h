/* 
 * $Id: l2.h 1.13 Broadcom SDK $
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
 * File:        l2.h
 * Purpose:     L2 internal definitions to the BCM library.
 */

#ifndef   _BCM_INT_SBX_L2_H_
#define   _BCM_INT_SBX_L2_H_

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* BCM_FE2000_P3_SUPPORT */

#include <bcm/types.h>
#include <bcm/debug.h>
#include <bcm/l2.h>

/*
 * Debug Output Macros
 */
#define L2_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_L2, stuff)
#define L2_WARN(stuff)          L2_DEBUG(BCM_DBG_WARN, stuff)
#define L2_ERR(stuff)           L2_DEBUG(BCM_DBG_ERR, stuff)
#define L2_VERB(stuff)          L2_DEBUG(BCM_DBG_VERBOSE, stuff)
#define L2_VVERB(stuff)         L2_DEBUG(BCM_DBG_VVERBOSE, stuff)

#define CHAR_SET   'X'
#define CHAR_CLEAR '-'

#define L2_6B_MAC_FMT       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"
#define L2_6B_MAC_PFMT(mac) (mac)[0], (mac)[1], (mac)[2], \
                            (mac)[3], (mac)[4], (mac)[5]


/* G2 Defines and Macros */
#define SB_COMMIT_COMPLETE   0xffffffff  /* Force completion on G2k call */

#define SB_SUCCESS(rv)          ((rv) == SB_OK)
#define SB_FAILURE(rv)          ((rv) != SB_OK)
#define IS_SB_IN_PROGRESS(rv)   ((rv) == SB_IN_PROGRESS)
#define TO_BCM_ERROR(op)        translate_sbx_result(op)


/* G2 FE device handler */

#ifdef BCM_FE2000_P3_SUPPORT
#define G2P3_FE_FROM_UNIT(unit) \
    ((soc_sbx_g2p3_state_t *)SOC_SBX_CONTROL(unit)->drv)
#define G2P3_FE_HANDLER_GET(unit, fe)  \
    if ((fe = G2P3_FE_FROM_UNIT(unit)) == NULL) {  \
        return BCM_E_INIT;  \
    }
#endif /* BCM_FE2000_P3_SUPPORT */



extern sal_mutex_t      _l2_mlock[BCM_LOCAL_UNITS_MAX];
#define L2_LOCK(unit)    sal_mutex_take(_l2_mlock[unit], sal_mutex_FOREVER)
#define L2_UNLOCK(unit)  sal_mutex_give(_l2_mlock[unit])

#define L2_DUMP     soc_cm_print

int
_bcm_fe2000_l2_addr_dump(int unit, bcm_mac_t mac, bcm_vlan_t vid,
                         int max_count);

int _bcm_fe2000_l2_age_ager_get(int unit);

int _bcm_fe2000_l2_flush_cache(int unit);
int _bcm_fe2000_l2_mcast_get(int unit, bcm_l2_addr_t *l2addr);

/*
 *   Function
 *      bcm_fe2000_l2_egress_range_reserve
 *   Purpose
 *     Reserve a range of L2 egress IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (IN) type    : resource to set
 *      (IN) highOrLow  : TRUE - set Upper bounds
 *                      : FALSE - set lower bounds
 *      (IN) val    : inclusive bound to set
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int bcm_fe2000_l2_egress_range_reserve(int unit, int highOrLow, uint32_t val);

/*
 *   Function
 *      bcm_fe2000_l2_egress_range_get
 *   Purpose
 *     Retrieve the range of valid L2 egress IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (OUT) first  : first valid ID
 *      (OUT) last   : last valid ID
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int bcm_fe2000_l2_egress_range_get(int unit, uint32_t *low, uint32_t *high);


/*
 * L2 Compare
 * Used internally for 'delete_by_xxx' routines.
 */
#define L2_CMP_VLAN              0x1
#define L2_CMP_MAC               0x2
#define L2_CMP_PORT              0x4
#define L2_CMP_TRUNK             0x8
#define CMP_VLAN(vid1, vid2)    ((vid1) - (vid2))

#define PARAM_NULL_CHECK(arg) \
    if ((arg) == NULL) { return BCM_E_PARAM; }

/*
 * WB sequence numbers
 * Used internally to differentiate L2, L2_CACHE scache
 */
#define L2_WB_L2            0
#define L2_WB_L2CACHE       1
#endif /* _BCM_INT_SBX_L2_H_ */
