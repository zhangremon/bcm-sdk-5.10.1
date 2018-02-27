/*
 * $Id: stat.h 1.8 Broadcom SDK $
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
 * File:        stat.h
 * Purpose:     SBX internal stats definitions.
 */

#ifndef   _BCM_INT_SBX_STAT_H_
#define   _BCM_INT_SBX_STAT_H_

#include <bcm/types.h>
#include <shared/idxres_afl.h>
#include <bcm_int/control.h>

/*
 * Control Structures.
 */
typedef struct bcm_stat_sbx_info_s {
    int                         segment;
    uint32                      ejectRate;
    shr_aidxres_list_handle_t   pList;
    shr_aidxres_element_t       first;
    shr_aidxres_element_t       last;
} bcm_stat_sbx_info_t;

typedef struct bcm_stat_info_s {
    int                     init;       /* TRUE if STAT module has been initialized */
    bcm_stat_sbx_info_t    *segInfo;
} bcm_stat_info_t;

#define     SBX_DASA_COUNTERS_PER_ENTRY     2

extern bcm_stat_info_t stat_info[BCM_MAX_NUM_UNITS];

#define STAT_CNTL(unit) stat_info[(unit)]

#define STAT_CHECK_INIT(unit)                                          \
    do {                                                               \
        if (!BCM_UNIT_VALID(unit)) return BCM_E_UNIT;                  \
        if (unit > (BCM_MAX_NUM_UNITS - 1)) return BCM_E_UNIT;         \
        if (STAT_CNTL(unit).init == FALSE) return BCM_E_INIT;          \
        if (STAT_CNTL(unit).init != TRUE) return STAT_CNTL(unit).init; \
    } while (0);

#define STAT_CHECK_PORT(unit, port)                                        \
    if (!SOC_PORT_VALID((unit), (port)) || !IS_E_PORT((unit), (port))) \
        return BCM_E_PORT;

#define STAT_CHECK_STAT(type)                                       \
    if ( ((type) < snmpIfInOctets) || ((type) >= snmpValCount) )    \
        return BCM_E_BADID;

#define STAT_CHECK_VLAN(unit, vlan)                                 \
    if ( ((BCM_VLAN_MAX <= vlan) || (0 == vlan)) )             \
        return BCM_E_PARAM;

/* Only allowed to set stats to zero */
#define STAT_CHECK_STAT_VALUE(val)          \
    if (0 < (val)) return BCM_E_PARAM;

#if 0
#define STAT_CHECK_VLAN_STAT(type) \
    if ( ((type) < bcmVlanStatUnicastPackets) || ((type) > bcmVlanStatCount) ) { \
        return BCM_E_BADID; \
    }
#endif

/* Debug Macros */
#define BCM_STAT_ERR(stuff)     BCM_DEBUG((BCM_DBG_COUNTER | BCM_DBG_ERR), stuff)
#define BCM_STAT_WARN(stuff)    BCM_DEBUG((BCM_DBG_COUNTER | BCM_DBG_WARN), stuff)
#define BCM_STAT_VERB(stuff)    BCM_DEBUG((BCM_DBG_COUNTER | BCM_DBG_VERBOSE), stuff)
#define BCM_STAT_VVERB(stuff)   BCM_DEBUG((BCM_DBG_COUNTER | BCM_DBG_VVERBOSE), stuff)
#define BCM_STAT_MSG(stuff)     BCM_DEBUG(BCM_DBG_NORMAL, stuff)
#define BCM_STAT_VVERB_L2_ADDR(pL2addr)                           \
    if (BCM_DEBUG_CHECK(BCM_DBG_COUNTER | BCM_DBG_VVERBOSE)) {    \
        bcm_l2_addr_dump(pL2addr);                                \
    }

/*
 * Function:
 *      _bcm_fe2000_stat_block_init
 * Description:
 *      Initialize a statistics block.
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      pCtlBlock - pointer to a control structure defining params for the
 *                  block. If NULL, use default.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXX      - Failure
 */
int _bcm_fe2000_g2p2_stat_block_init(int unit, int type,
                                     bcm_stat_sbx_info_t *pCtlBlock);

/*
 * Function:
 *      _bcm_fe2000_stat_block_alloc
 * Description:
 *      Allocate counters from a statistics block
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      count     - number of counters required
 *      start     - (OUT) where to put first of allocated counter block
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_XXXX      - Failure
 */
int _bcm_fe2000_stat_block_alloc(int unit,
                                 int type,
                                 shr_aidxres_element_t *start,
                                 shr_aidxres_element_t count);

/*
 * Function:
 *      _bcm_fe2000_stat_block_free
 * Description:
 *      Free counters from a statistics block
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      start     - (OUT) where to put first of allocated counter block
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_XXXX      - Failure
 */
int _bcm_fe2000_stat_block_free(int unit,
                                int type,
                                shr_aidxres_element_t start);

#endif /* _BCM_INT_SBX_STAT_H_ */
