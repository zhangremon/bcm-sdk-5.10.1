/* 
 * $Id: fe2000_counter.c 1.13 Broadcom SDK $
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
 * File:        fe2000_counter.c
 * Purpose:     Software Counter Collection module for FE2000.
 *
 */

#include <soc/error.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/counter.h>
#include <soc/sbx/fe2000_counter.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#if 0
#include <soc/sbx/fe2k_common/sbFe2000Common.h>
#endif
#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* BCM_FE2000_P3_SUPPORT */


/*
 * FE2000 has 2 AM blocks (0, 1).  Each AM block
 * contains 12 sets of counters, one per physical GE port.
 *
 * Counter sets for AM are defined as follows:
 *   sets  0-11 correspond to device block AM0 (ge0-11)
 *   sets 12-23 correspond to device block AM1 (ge12-23)
 */
#define FE2000_COUNTER_BLOCK_AM_NUM_SETS    \
        (SB_FE2000_MAX_AG_PORTS * SB_FE2000_NUM_AG_INTERFACES)
#define FE2000_COUNTER_REG_AM_WITDH         32

/*
 * FE2000 has 2 XM blocks (0, 1).  Each XM block
 * contains 1 set of counter, one per physical XE port.
 *
 * Counter sets for XM are defined as follows:
 *   set 0 corresponds to device block XM0 (xe0)
 *   set 1 corresponds to device block XM1 (xe1)
 */
#define FE2000_COUNTER_BLOCK_XM_NUM_SETS    \
        (SB_FE2000_MAX_XG_PORTS * SB_FE2000_NUM_XG_INTERFACES)
#define FE2000_COUNTER_REG_XM_WITDH         36
#define FE2000_COUNTER_REG_XM_HI_MASK       0xF


#ifdef BCM_FE2000_P3_SUPPORT
/*
 * FE2000 Ilib (available currently only in G2P3)
 *
 * Counter sets for the Ilib block counter are defined as 1 set per
 * ucode, or bcm port (bcm port number is the set number).
 */
#define FE2000_COUNTER_BLOCK_ILIB_G2P3_NUM_SETS    SBX_MAX_PORTS
#define FE2000_COUNTER_BLOCK_ILIB_G2P3_WITDH       64
#endif /* BCM_FE2000_P3_SUPPORT */


STATIC int _soc_fe2000_counter_am_read(int unit, int set, int counter,
                                       uint64 *val, int *width);
STATIC int _soc_fe2000_counter_xm_read(int unit, int set, int counter,
                                       uint64 *val, int *width);
STATIC int _soc_fe2000_counter_am_write(int unit, int set, int counter,
                                        uint64 val);
STATIC int _soc_fe2000_counter_xm_write(int unit, int set, int counter,
                                        uint64 val);

#ifdef BCM_FE2000_P3_SUPPORT
STATIC int _soc_fe2000_counter_ilib_g2p3_read(int unit, int set, int counter,
                                              uint64 *val, int *width);
STATIC int _soc_fe2000_counter_ilib_g2p3_write(int unit, int set, int counter,
                                               uint64 val);
#endif /* BCM_FE2000_P3_SUPPORT */


/*
 * Counter Blocks
 *
 * Counter blocks for the Software Counter module to collect statistics on.
 *
 * NOTE:  The order of the counter blocks must be the same
 *        as the blocks defined in 'fe2000_counter_block_t'
 */
soc_sbx_counter_block_info_t    fe2000_counter_blocks[] = {
    { fe2000CounterBlockAm,
      FE2000_COUNTER_BLOCK_AM_NUM_SETS,
      fe2000CounterAmCount,
      _soc_fe2000_counter_am_read,
      _soc_fe2000_counter_am_write,
    },
    { fe2000CounterBlockXm,
      FE2000_COUNTER_BLOCK_XM_NUM_SETS,
      fe2000CounterXmCount,
      _soc_fe2000_counter_xm_read,
      _soc_fe2000_counter_xm_write,
    },
    { fe2000CounterBlockIlib,
      /* NOTE:
       * block is initialized at runtime depending on the ucode version.
       */
      0,
      0,
      NULL,
      NULL,
    },
};

#define REG_OFFSET(_unit_, _reg_)    SAND_HAL_FE2000_REG_OFFSET(_unit_, _reg_)


/*
 * Function:
 *     _soc_fe2000_counter_am_read
 * Purpose:
 *     Read specified AM counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in AM block
 *     counter - Counter in given set to read
 *     val     - (OUT) Counter value
 *     width   - (OUT) Bits in hardware register counter
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_fe2000_counter_am_read(int unit, int set, int counter,
                            uint64 *val, int *width)
{
    int     rv;
    int32   block_instance;
    int32   block_offset;
    uint32  data;

    block_instance = set / SB_FE2000_MAX_AG_PORTS;
    block_offset   = set % SB_FE2000_MAX_AG_PORTS;

    switch (counter) {
    case fe2000CounterAmRxFcsErrorCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_FCS_ERROR_CNT), &data);
        break;
    case fe2000CounterAmRxCtrlPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_CTRL_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxPausePktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_PAUSE_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxUnknownCtrlPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_UNKNOWN_CTRL_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxAlignmentErrorPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_ALIGNMENT_ERROR_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxBadLengthPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_BAD_LENGTH_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxInvalidDataCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_INVALID_DATA_CNT), &data);
        break;
    case fe2000CounterAmRxFalseCarrierCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_FALSE_CARRIER_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxOversizePktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_OVERSIZE_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxJabberPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_JABBER_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxMtuExceededPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_MTU_EXCEEDED_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRuntPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RUNT_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxUndersizePktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_UNDERSIZE_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxFragmentPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_FRAGMENT_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxRuntByteCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RUNT_BYTE_CNT), &data);
        break;
    case fe2000CounterAmRxMcPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_MC_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxBroadcastPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_BROADCAST_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRange64PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_64_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxRange127PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_127_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRange255PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_255_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRange511PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_511_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRange1023PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_1023_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRange1518PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_1518_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRange1522PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_1522_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRange2047PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_2047_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRange4095PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_4095_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxRange9216PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_RANGE_9216_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmRxPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxByteCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_BYTE_CNT), &data);
        break;
    case fe2000CounterAmRxUcPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_UC_PKT_CNT), &data);
        break;
    case fe2000CounterAmRxValidPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_RX_VALID_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxPausePktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_PAUSE_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxJabberPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_JABBER_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxFcsErrorCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_FCS_ERROR_CNT), &data);
        break;
    case fe2000CounterAmTxCtrlPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_CTRL_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxOversizePktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_OVERSIZE_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxSingleDeferralPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_SINGLE_DEFERRAL_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxMultipleDeferralPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_MULTIPLE_DEFERRAL_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxSingleCollisionPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_SINGLE_COLLISION_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxMultipleCollisionPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_MULTIPLE_COLLISION_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxLateCollisionPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_LATE_COLLISION_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxExcessiveCollisionPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_EXCESSIVE_COLLISION_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxFragmentPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_FRAGMENT_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxTotalCollisionCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_TOTAL_COLLISION_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxMcPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_MC_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxBroadcastPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_BROADCAST_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxRange64PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_64_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxRange127PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_127_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxRange255PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_255_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxRange511PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_511_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxRange1023PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_1023_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxRange1518PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_1518_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxRange1522PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_1522_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxRange2047PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_2047_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxRange4095PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_4095_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxRange9216PktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_RANGE_9216_PKT_CNT),
                                    &data);
        break;
    case fe2000CounterAmTxPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxByteCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_BYTE_CNT), &data);
        break;
    case fe2000CounterAmTxUcPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_UC_PKT_CNT), &data);
        break;
    case fe2000CounterAmTxValidPktCnt:
        rv = soc_sbx_fe2000_am_read(unit, block_instance , block_offset,
                                    REG_OFFSET(unit, AM_TX_VALID_PKT_CNT), &data);
        break;

    default:
        return SOC_E_PARAM;
        break;
    }

    if (SOC_FAILURE(rv)) {
        return rv;
    }

    /*
     * Skip 'get' field from data value, since it's not needed here
     */
    COMPILER_64_SET((*val), 0, data);
    *width = FE2000_COUNTER_REG_AM_WITDH;

    return SOC_E_NONE;
}


/*
 * Function:
 *     _soc_fe2000_counter_am_write
 * Purpose:
 *     Write specified AM counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in AM block
 *     counter - Counter in given set
 *     val     - Counter value to write
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_fe2000_counter_am_write(int unit, int set, int counter, uint64 val)
{
    int     rv;
    int32   block_instance;
    int32   block_offset;
    uint32  data;

    block_instance = set / SB_FE2000_MAX_AG_PORTS;
    block_offset   = set % SB_FE2000_MAX_AG_PORTS;
    COMPILER_64_TO_32_LO(data, val);

    switch (counter) {
    case fe2000CounterAmRxFcsErrorCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_FCS_ERROR_CNT), data);
        break;
    case fe2000CounterAmRxCtrlPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_CTRL_PKT_CNT), data);
        break;
    case fe2000CounterAmRxPausePktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_PAUSE_PKT_CNT), data);
        break;
    case fe2000CounterAmRxUnknownCtrlPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_UNKNOWN_CTRL_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxAlignmentErrorPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_ALIGNMENT_ERROR_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxBadLengthPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_BAD_LENGTH_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxInvalidDataCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_INVALID_DATA_CNT), data);
        break;
    case fe2000CounterAmRxFalseCarrierCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_FALSE_CARRIER_CNT),
                                     data);
        break;
    case fe2000CounterAmRxOversizePktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_OVERSIZE_PKT_CNT), data);
        break;
    case fe2000CounterAmRxJabberPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_JABBER_PKT_CNT), data);
        break;
    case fe2000CounterAmRxMtuExceededPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_MTU_EXCEEDED_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRuntPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RUNT_PKT_CNT), data);
        break;
    case fe2000CounterAmRxUndersizePktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_UNDERSIZE_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxFragmentPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_FRAGMENT_PKT_CNT), data);
        break;
    case fe2000CounterAmRxRuntByteCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RUNT_BYTE_CNT), data);
        break;
    case fe2000CounterAmRxMcPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_MC_PKT_CNT), data);
        break;
    case fe2000CounterAmRxBroadcastPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_BROADCAST_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRange64PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_64_PKT_CNT), data);
        break;
    case fe2000CounterAmRxRange127PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_127_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRange255PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_255_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRange511PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_511_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRange1023PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_1023_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRange1518PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_1518_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRange1522PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_1522_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRange2047PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_2047_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRange4095PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_4095_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxRange9216PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_RANGE_9216_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmRxPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_PKT_CNT), data);
        break;
    case fe2000CounterAmRxByteCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_BYTE_CNT), data);
        break;
    case fe2000CounterAmRxUcPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_UC_PKT_CNT), data);
        break;
    case fe2000CounterAmRxValidPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_RX_VALID_PKT_CNT), data);
        break;
    case fe2000CounterAmTxPausePktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_PAUSE_PKT_CNT), data);
        break;
    case fe2000CounterAmTxJabberPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_JABBER_PKT_CNT), data);
        break;
    case fe2000CounterAmTxFcsErrorCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_FCS_ERROR_CNT), data);
        break;
    case fe2000CounterAmTxCtrlPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_CTRL_PKT_CNT), data);
        break;
    case fe2000CounterAmTxOversizePktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_OVERSIZE_PKT_CNT), data);
        break;
    case fe2000CounterAmTxSingleDeferralPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_SINGLE_DEFERRAL_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxMultipleDeferralPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_MULTIPLE_DEFERRAL_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxSingleCollisionPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_SINGLE_COLLISION_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxMultipleCollisionPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_MULTIPLE_COLLISION_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxLateCollisionPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_LATE_COLLISION_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxExcessiveCollisionPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_EXCESSIVE_COLLISION_PKT_CNT),
                                    data);
        break;
    case fe2000CounterAmTxFragmentPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_FRAGMENT_PKT_CNT), data);
        break;
    case fe2000CounterAmTxTotalCollisionCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_TOTAL_COLLISION_CNT),
                                     data);
        break;
    case fe2000CounterAmTxMcPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_MC_PKT_CNT), data);
        break;
    case fe2000CounterAmTxBroadcastPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_BROADCAST_PKT_CNT), data);
        break;
    case fe2000CounterAmTxRange64PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_64_PKT_CNT), data);
        break;
    case fe2000CounterAmTxRange127PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_127_PKT_CNT), data);
        break;
    case fe2000CounterAmTxRange255PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_255_PKT_CNT), data);
        break;
    case fe2000CounterAmTxRange511PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_511_PKT_CNT), data);
        break;
    case fe2000CounterAmTxRange1023PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_1023_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxRange1518PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_1518_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxRange1522PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_1522_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxRange2047PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_2047_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxRange4095PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_4095_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxRange9216PktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_RANGE_9216_PKT_CNT),
                                     data);
        break;
    case fe2000CounterAmTxPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_PKT_CNT), data);
        break;
    case fe2000CounterAmTxByteCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_BYTE_CNT), data);
        break;
    case fe2000CounterAmTxUcPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_UC_PKT_CNT), data);
        break;
    case fe2000CounterAmTxValidPktCnt:
        rv = soc_sbx_fe2000_am_write(unit, block_instance , block_offset,
                                     REG_OFFSET(unit, AM_TX_VALID_PKT_CNT), data);
        break;

    default:
        return SOC_E_PARAM;
        break;
    }

    return rv;
}


/*
 * Function:
 *     _soc_fe2000_counter_xm_read
 * Purpose:
 *     Read specified XM counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in XM block
 *     counter - Counter in given set to read
 *     val     - (OUT) Counter value
 *     width   - (OUT) Bits in hardware register counter
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_fe2000_counter_xm_read(int unit, int set, int counter,
                            uint64 *val, int *width)
{
    int     rv;
    int32   block_instance;
    uint32  dhi, dlo;

    block_instance = set / SB_FE2000_MAX_XG_PORTS;

    switch (counter) {
    case fe2000CounterXmRxRange64PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_64_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxRange127PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_127_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxRange255PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_255_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxRange511PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_511_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxRange1023PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_1023_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxRange1518PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_1518_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxRange2047PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_2047_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxRange4095PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_4095_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxRange9216PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_9216_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxRange16383PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_RANGE_16383_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxMaxSizePktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_MAX_SIZE_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_PKT_CNT_LO), &dhi, &dlo);
        break;
    case fe2000CounterXmRxFcsPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_FCS_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxUnicastPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_UNICAST_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxMcPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_MC_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxBroadcastPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_BROADCAST_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxCtrlPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_CTRL_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxPausePktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_PAUSE_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxUnknownCtrlPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_UNKNOWN_CTRL_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxJabberPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_JABBER_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxOversizePktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_OVERSIZE_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxBadLengthPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_BAD_LENGTH_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxAllPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_ALL_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxMtuExceededPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_MTU_EXCEEDED_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxBadOversizePktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_BAD_OVERSIZE_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxByteCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_BYTE_CNT_LO), &dhi, &dlo);
        break;
    case fe2000CounterXmRxUndersizePktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_UNDERSIZE_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxFragmentPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_FRAGMENT_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxErrorPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_ERROR_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxFrameErrorCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_FRAME_ERROR_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmRxIpgJunkCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_RX_IPG_JUNK_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxAllPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_ALL_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxPauseCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_PAUSE_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxFcsErrorCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_FCS_ERROR_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxUnicastPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_UNICAST_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxMcPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_MC_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxBroadcastPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_BROADCAST_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxOversizePktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_OVERSIZE_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxFragmentPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_FRAGMENT_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_PKT_CNT_LO), &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange64PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_64_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange127PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_127_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange255PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_255_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange511PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_511_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange1023PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_1023_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange1518PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_1518_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange2047PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_2047_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange4095PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_4095_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange9216PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_9216_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxRange16383PktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_RANGE_16383_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxMaxSizePktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_MAX_SIZE_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxUnderflowPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_UNDERFLOW_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxErrorPktCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_ERROR_PKT_CNT_LO),
                                    &dhi, &dlo);
        break;
    case fe2000CounterXmTxByteCnt:
        rv = soc_sbx_fe2000_xm_read(unit, block_instance,
                                    REG_OFFSET(unit, XM_TX_BYTE_CNT_LO), &dhi, &dlo);
        break;

    default:
        return SOC_E_PARAM;
        break;
    }

    if (SOC_FAILURE(rv)) {
        return rv;
    }

    /*
     * Skip 'get' field from data values, since it's not needed here
     */
    COMPILER_64_SET((*val), (dhi & FE2000_COUNTER_REG_XM_HI_MASK), dlo);
    *width = FE2000_COUNTER_REG_XM_WITDH;

    return SOC_E_NONE;
}


/*
 * Function:
 *     _soc_fe2000_counter_xm_write
 * Purpose:
 *     Write specified XM counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in XM block
 *     counter - Counter in given set
 *     val     - Counter value to write
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_fe2000_counter_xm_write(int unit, int set, int counter, uint64 val)
{
    int     rv;
    int32   block_instance;
    uint32  dhi, dlo;

    block_instance = set / SB_FE2000_MAX_XG_PORTS;
    COMPILER_64_TO_32_HI(dhi, val);
    COMPILER_64_TO_32_LO(dlo, val);
    dhi = dhi & FE2000_COUNTER_REG_XM_HI_MASK;


    switch (counter) {
    case fe2000CounterXmRxRange64PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_64_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxRange127PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_127_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxRange255PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_255_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxRange511PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_511_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxRange1023PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_1023_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxRange1518PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_1518_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxRange2047PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_2047_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxRange4095PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_4095_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxRange9216PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_9216_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxRange16383PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_RANGE_16383_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxMaxSizePktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_MAX_SIZE_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_PKT_CNT_LO), dhi, dlo);
        break;
    case fe2000CounterXmRxFcsPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_FCS_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxUnicastPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_UNICAST_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxMcPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_MC_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxBroadcastPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_BROADCAST_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxCtrlPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_CTRL_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxPausePktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_PAUSE_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxUnknownCtrlPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_UNKNOWN_CTRL_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxJabberPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_JABBER_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxOversizePktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_OVERSIZE_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxBadLengthPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_BAD_LENGTH_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxAllPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_ALL_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxMtuExceededPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_MTU_EXCEEDED_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxBadOversizePktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_BAD_OVERSIZE_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxByteCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_BYTE_CNT_LO), dhi, dlo);
        break;
    case fe2000CounterXmRxUndersizePktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_UNDERSIZE_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxFragmentPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_FRAGMENT_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxErrorPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_ERROR_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxFrameErrorCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_FRAME_ERROR_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmRxIpgJunkCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_RX_IPG_JUNK_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxAllPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_ALL_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxPauseCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_PAUSE_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxFcsErrorCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_FCS_ERROR_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxUnicastPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_UNICAST_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxMcPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_MC_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxBroadcastPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_BROADCAST_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxOversizePktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_OVERSIZE_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxFragmentPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_FRAGMENT_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_PKT_CNT_LO), dhi, dlo);
        break;
    case fe2000CounterXmTxRange64PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_64_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxRange127PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_127_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxRange255PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_255_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxRange511PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_511_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxRange1023PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_1023_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxRange1518PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_1518_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxRange2047PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_2047_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxRange4095PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_4095_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxRange9216PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_9216_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxRange16383PktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_RANGE_16383_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxMaxSizePktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_MAX_SIZE_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxUnderflowPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_UNDERFLOW_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxErrorPktCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_ERROR_PKT_CNT_LO),
                                     dhi, dlo);
        break;
    case fe2000CounterXmTxByteCnt:
        rv = soc_sbx_fe2000_xm_write(unit, block_instance,
                                     REG_OFFSET(unit, XM_TX_BYTE_CNT_LO), dhi, dlo);
        break;

    default:
        return SOC_E_PARAM;
        break;
    }

    return rv;
}


#ifdef BCM_FE2000_P3_SUPPORT
/*
 * Function:
 *     _soc_fe2000_counter_ilib_g2p3_read
 * Purpose:
 *     Read specified Ilib counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in block
 *     counter - Counter in given set to read
 *     val     - (OUT) Counter value
 *     width   - (OUT) Bits in counter
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_fe2000_counter_ilib_g2p3_read(int unit, int set, int counter,
                                   uint64 *val, int *width)
{
    int                           rv;
    soc_sbx_g2p3_counter_value_t  data;

    switch (counter) {
    case fe2000CounterIlibDiscardPktCnt:
        /*
         * The counter 'set' value is the port number (or 'cnum')
         * expected by the g2p3 routine.
         */
        rv = soc_sbx_g2p3_pdctr_get(unit, 0, set, &data);
        break;

    default:
        return SOC_E_PARAM;
        break;
    }

    if (SOC_FAILURE(rv)) {
        return rv;
    }

    *val   = data.packets;
    *width = FE2000_COUNTER_BLOCK_ILIB_G2P3_WITDH;

    return SOC_E_NONE;
}


/*
 * Function:
 *    _soc_fe2000_counter_ilib_g2p3_write
 * Purpose:
 *     Write specified Ilib counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in block
 *     counter - Counter in given set
 *     val     - Counter value to write (only 'zero' is allowed)
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_fe2000_counter_ilib_g2p3_write(int unit, int set, int counter, uint64 val)
{
    int                           rv;
    soc_sbx_g2p3_counter_value_t  data;

    switch (counter) {
    case fe2000CounterIlibDiscardPktCnt:
        /* Only clear is allowed */
        if (val != 0) {
            return SOC_E_PARAM;
        }

        /*
         * The counter 'set' value is the port number (or 'cnum')
         * expected by the g2p3 routine.
         */
        rv = soc_sbx_g2p3_pdctr_get(unit, 1, set, &data);
        break;

    default:
        return SOC_E_PARAM;
        break;
    }

    return rv;
}
#endif /* BCM_FE2000_P3_SUPPORT */


/*
 * Function:
 *     _soc_fe2000_counter_blockset_get
 * Purpose:
 *     Return the counter set that corresponds to the given port and
 *     counter block
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     block - Counter block
 *     set   - (OUT) Counter set for given port and block
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
STATIC int
_soc_fe2000_counter_blockset_get(int unit, soc_port_t port,
                                 int block, int *block_set)
{
    int32     block_num;
    int32     block_port;

    /*    coverity[new_values]    */
    if ((block < 0) || (block >= fe2000CounterBlockCount)) {
        return SOC_E_PARAM;
    }

    if (fe2000_counter_blocks[block].num_sets == 0) {
        return SOC_E_UNAVAIL;
    }

    switch(block) {
    case fe2000CounterBlockAm:
        if (!IS_GE_PORT(unit, port)) {
            return SOC_E_PARAM;
        }

        block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
        block_port = SOC_PORT_BLOCK_INDEX(unit, port);
        if ((block_num < 0) || (block_port < 0)) {
            return SOC_E_PORT;
        }
        *block_set = (block_num * SB_FE2000_MAX_AG_PORTS) + block_port;
        break;

    case fe2000CounterBlockXm:
        if (!IS_XE_PORT(unit, port)) {
            return SOC_E_PARAM;
        }

        block_num  = SOC_PORT_BLOCK_NUMBER(unit, port);
        block_port = SOC_PORT_BLOCK_INDEX(unit, port);
        if ((block_num < 0) || (block_port < 0)) {
            return SOC_E_PORT;
        }
        *block_set = (block_num * SB_FE2000_MAX_XG_PORTS) + block_port;
        break;

    case fe2000CounterBlockIlib:
        if ((port < 0) ||
            (port >= fe2000_counter_blocks[block].num_sets)) {
            return SOC_E_PORT;
        }
        *block_set = port;
        break;

    default:
        return SOC_E_PARAM;
        break;
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_fe2000_counter_init
 * Purpose:
 *     Initialize and start the counter collection, software
 *     accumulation process.
 * Parameters:
 *     unit     - Device number
 *     flags    - SOC_COUNTER_F_xxx flags
 *     interval - Collection period in micro-seconds,
 *                using 0 is the same as calling soc_sbx_counter_stop()
 *     pbmp     - Bitmap of ports to collect counters on
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_fe2000_counter_init(int unit, uint32 flags, int interval, pbmp_t pbmp)
{
    soc_port_t  port;
    int         block;
    int         set;
    
    /* Init Ilib block counter information depending on ucode version */
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        fe2000_counter_blocks[fe2000CounterBlockIlib].num_sets =
            FE2000_COUNTER_BLOCK_ILIB_G2P3_NUM_SETS;
        fe2000_counter_blocks[fe2000CounterBlockIlib].num_counters =
            fe2000CounterIlibCount;
        fe2000_counter_blocks[fe2000CounterBlockIlib].read =
            _soc_fe2000_counter_ilib_g2p3_read;
        fe2000_counter_blocks[fe2000CounterBlockIlib].write = 
            _soc_fe2000_counter_ilib_g2p3_write;
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        fe2000_counter_blocks[fe2000CounterBlockIlib].num_sets = 0;
        fe2000_counter_blocks[fe2000CounterBlockIlib].num_counters = 0;
        fe2000_counter_blocks[fe2000CounterBlockIlib].read = NULL;
        fe2000_counter_blocks[fe2000CounterBlockIlib].write = NULL;
        break;
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        /* Init Software Counter Collection module */
        SOC_IF_ERROR_RETURN(soc_sbx_counter_init(unit, &fe2000_counter_blocks[0],
                                                 fe2000CounterBlockCount));
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        return SOC_E_CONFIG;
    }

    /*
     * For each block, add counter sets for given ports to main
     * counter collector
     */
    for (block = 0; block < fe2000CounterBlockCount; block++) {
        PBMP_ITER(pbmp, port) {

            if (!IS_E_PORT(unit,port)) {
                continue;
            }

            /* Get corresponding counter block-set for port */
            if (SOC_FAILURE
                (_soc_fe2000_counter_blockset_get(unit, port, block, &set))) {
                continue;
            }

            soc_sbx_counter_bset_add(unit, block, set);
        }
    }

    /* Start software counter collector */
    SOC_PBMP_CLEAR(pbmp);
    SOC_IF_ERROR_RETURN(soc_sbx_counter_start(unit, flags, interval, pbmp));

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_fe2000_counter_port_get
 * Purpose:
 *     Get the specified software-accumulated counter value for given port.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     block   - Block counter
 *     counter - Counter
 *     val     - (OUT) Software accumulated value
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit
 */
int
soc_sbx_fe2000_counter_port_get(int unit, soc_port_t port,
                                int block, int counter, uint64 *val)
{
    int  set;
    
    /* Get corresponding counter block-set for port */
    SOC_IF_ERROR_RETURN
        (_soc_fe2000_counter_blockset_get(unit, port, block, &set));

    return soc_sbx_counter_get(unit, block, set, counter, val);
}


/*
 * Function:
 *     soc_sbx_fe2000_counter_port_set
 * Purpose:
 *     Set the counter value for given port to the requested value.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     block   - Block counter (a negative value means all block counters)
 *     counter - Counter to set (a negative value means all counters for port)
 *     val     - Counter value
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit
 */
int
soc_sbx_fe2000_counter_port_set(int unit, soc_port_t port,
                                int block, int counter, uint64 val)
{
    int  set;
    int  block_start, block_end, block_x;
    int  counter_start, counter_end, counter_x;

    if (block >= fe2000CounterBlockCount) {
        return SOC_E_PARAM;
    }

    /* Block counter to set (negative indicates all blocks) */
    if (block >= 0) {
        block_start = block_end = block;
    } else {
        block_start = 0;
        block_end   = fe2000CounterBlockCount - 1;
    }

    /* Counter to set (negative indicates all counters for port ) */
    if (counter >= 0) {
        counter_start = counter_end = counter;
    } else {
        /* NOTE:
         * counter_end value needs to be set later in loop depending
         * on the block
         */
        counter_start = counter_end = 0;
    }

    /* Set counter value */
    for (block_x = block_start; block_x <= block_end; block_x++) {

        /* Get corresponding counter block-set for port */
        if (SOC_FAILURE
            (_soc_fe2000_counter_blockset_get(unit, port, block_x, &set))) {
            continue;
        }

        /* Get number of counter for the block */
        if (counter < 0) {
            counter_end = fe2000_counter_blocks[block_x].num_counters - 1;
        }

        for (counter_x = counter_start; counter_x <= counter_end; counter_x++) {
            SOC_IF_ERROR_RETURN
                (soc_sbx_counter_set(unit, block_x, set, counter_x, val));
        }
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_fe2000_counter_max_pkt_size
 * Purpose:
 *     Set the max packet size for GE ports at which the
 *     packet is considered oversized in statictics counters.
 * Parameters:
 *     unit - Device number
 *     size - Max packet size used in statistic counter update.
 *            If larger, packet is counted as oversized in the port statistics.
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and max_size are valid.
 */
int
soc_sbx_fe2000_counter_max_pkt_size(int unit, int max_size)
{
    int  i;
    pbmp_t ge_pbm;
    int ge_count = 0;

    /* only perform operation if GE ports are present */
    SOC_PBMP_ASSIGN(ge_pbm, PBMP_GE_ALL(unit));
    SOC_PBMP_COUNT(ge_pbm, ge_count);

    if (ge_count == 0) {
        return SOC_E_NONE;
    }

    /* FE2000 has 2 'gport' block instances */
    for (i = 0; i < SB_FE2000_NUM_AG_INTERFACES; i++) {
        if ( SOC_SBX_CFG_FE2000_IF_IS_XG(unit, SB_FE2000_IF_PTYPE_AGM0 + i) ||
             SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_AGM0 + i) ){
            continue;
        }
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_am_write(unit, i, SB_FE2000_MAX_AG_PORTS,
                                     REG_OFFSET(unit, GPORT_MAX_PKT_SIZE_STATUS_UPDATE),
                                     (uint32)max_size));
    }

    return SOC_E_NONE;
}
