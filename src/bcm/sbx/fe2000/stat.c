/*
 * $Id: stat.c 1.30 Broadcom SDK $
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
 * Broadcom SBX Statistics/Counters API.
 */

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/fe2k/sbFe2000Util.h>
#include <soc/sbx/fe2k_common/sbFe2000CommonUtil.h>
#include <soc/sbx/fe2k_common/sbFe2000CmuMgr.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/fe2000_counter.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#include <soc/sbx/g2p3/g2p3_int.h>
#endif

#include <bcm/types.h>
#include <bcm/stat.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/port.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/fe2000/stat.h>

#define BIGMAC_CRC_APPEND          0x00

/* Error checking for FE2000 block read/writes routines */
#define FE2000_STS_CHECK(status)    \
    if ((status) != SB_FE2000_STS_INIT_OK_K) { return SOC_E_INTERNAL; }

#if 0
typedef enum bcm_mpls_stat_s {
    bcmMplsStatInPackets,
    bcmMplsStatInBytes,
    bcmMplsStatOutPackets,
    bcmMplsStatOutBytes
} bcm_mpls_stat_t;

typedef enum bcm_ipmc_stat_s {
    bcmIpmcStatOutPackets,
    bcmIpmcStatOutBytes,
    bcmIpmcStatDropPackets,
    bcmIpmcStatDropBytes
} bcm_ipmc_stat_t;
#endif

#define G2_FE_HANDLER_GET(unit, fe)  \
    if ((fe = (sbG2Fe_t *)SOC_SBX_CONTROL(unit)->drv) == NULL) {  \
        return BCM_E_INIT;  \
    }

/* ??? I have no idea what the rate should be or the units ??? */
#define STAT_THREAD_PRIORITY_DEFAULT    50
#define STAT_THREAD_INTERVAL_DEFAULT    1000000
#define BCMSIM_STAT_INTERVAL            25000000
#define DEFAULT_EJECTION_RATE           100
#define DEFAULT_EJECTION_NOT_USED       0

/* Fixed counter block sizes. Would rather get for Gu2K Lib */
#define SBX_VLAN_COUNTER_SIZE           (64*1024)
#define SBX_EGRESS_COUNTER_SIZE         (128*1024)
#define SBX_DASA_COUNTER_SIZE           (128*1024)
#define SBX_EXCEPTION_COUNTER_SIZE      (128*1024)
#define SBX_QOS_RULE_COUNTER_SIZE       (4*1024)
#define SBX_SEC_RULE_COUNTER_SIZE       (4*1024)
#define SBX_EGR_RULE_COUNTER_SIZE       (4*1024)


/*
 * Macro to 'add' software accumulated counter to given value
 */
#ifdef STAT_ADD
#undef STAT_ADD
#endif
#define STAT_ADD(unit, port, block, counter, val)  \
    do {  \
        uint64  counter_val;  \
        SOC_IF_ERROR_RETURN(soc_sbx_fe2000_counter_port_get(unit, port,      \
                                                            block, counter,  \
                                                            &counter_val));  \
        COMPILER_64_ADD_64(val, counter_val);                                \
    } while (0)

#ifdef STAT_ADD_BLOCK_AM
#undef STAT_ADD_BLOCK_AM
#endif
#define STAT_ADD_BLOCK_AM(unit, port, counter, val)  \
    STAT_ADD(unit, port, fe2000CounterBlockAm, counter, val)

#ifdef STAT_ADD_BLOCK_XM
#undef STAT_ADD_BLOCK_XM
#endif
#define STAT_ADD_BLOCK_XM(unit, port, counter, val)  \
    STAT_ADD(unit, port, fe2000CounterBlockXm, counter, val)

#ifdef STAT_ADD_BLOCK_ILIB
#undef STAT_ADD_BLOCK_ILIB
#endif
#define STAT_ADD_BLOCK_ILIB(unit, port, counter, val)  \
    STAT_ADD(unit, port, fe2000CounterBlockIlib, counter, val)


/*
 * Max packet size default threshold for 'oversized' statistics
 */
#define STAT_MAX_PKT_SIZE_DEFAULT    1518

int
_bcm_fe2000_g2p3_stat_block_detach(int unit, soc_sbx_g2p3_counter_id_t type,
                                   bcm_stat_sbx_info_t *pCtlBlock);
/*
 * Function:
 *     _bcm_fe2000_stat_xm_tx_byte_count_adjust
 * Description:
 *     Adjust the TX byte count on the XE port.  The TxByte counters do not
 *     the CRC.  The tx counters count the bytes received for transmit, not
 *     the bytes actually transmitted.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     value - (IN/OUT) 64 bit statistic counter value
 * Returns:
 *     n/a
 *
 * Notes:
 *     Assumes valid unit and valid GE port.
 */
STATIC int
_bcm_fe2000_stat_xm_tx_byte_count_adjust(int unit, bcm_port_t port, 
                                         uint64 *value)
{
    uint32_t txCfgHi, txCfgLo;
    int crcMode;

    SOC_IF_ERROR_RETURN
        (soc_sbx_fe2000_xm_read(unit, SOC_PORT_BLOCK_NUMBER(unit, port),
                                SAND_HAL_REG_OFFSET(CA, XM_MAC_TX_CONFIG_LO),
                                &txCfgHi, &txCfgLo));

    crcMode = SAND_HAL_FE2000_GET_FIELD(SOC_SBX_SBHANDLE(unit), 
                                        XM_MAC_TX_CONFIG_LO,
                                        CRC_MODE, txCfgLo);

    if (crcMode == BIGMAC_CRC_APPEND) {
        uint64     pktCount, tmp;

        COMPILER_64_ZERO(pktCount);
        SOC_IF_ERROR_RETURN
            (soc_sbx_fe2000_counter_port_get(unit, port, fe2000CounterBlockXm,
                                             fe2000CounterXmTxAllPktCnt, 
                                             &tmp));
        COMPILER_64_ADD_64(pktCount, tmp);

        /* Foreach packet, add 4 bytes of CRC */
        COMPILER_64_SHL(pktCount, 2);
        COMPILER_64_ADD_64(*value, pktCount);
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_stat_ge_get
 * Description:
 *     Get the specified statistic for a GE port.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     type  - SNMP statistics type (see bcm/stat.h)
 *     value - (OUT) 64 bit statistic counter value
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 *
 * Notes:
 *     Assumes valid unit and valid GE port.
 */
STATIC int
_bcm_fe2000_stat_ge_get(int unit, bcm_port_t port, bcm_stat_val_t type,
                        uint64 *value)
{
    uint64  count;

    COMPILER_REFERENCE(&count);  /* Work around PPC compiler bug */
    COMPILER_64_ZERO(count);

    switch (type) {
        /* *** RFC 1213 *** */
    case snmpIfInOctets:        /* Bytes received */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxByteCnt, count);
        break;

    case snmpIfInUcastPkts:     /* Unicast packets received */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxUcPktCnt, count);
        break;

    case snmpIfInNUcastPkts:    /* Non Unicast packets received */
        /* Multicast packets received */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxMcPktCnt, count);
        /* Broadcast packets received */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxBroadcastPktCnt, count);
        break;

    case snmpIfInErrors:        /* Received packets containing errors */
        /* Undersize pkts, good FCS */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxUndersizePktCnt, count);
        /* Undersize pkts, bad FCS */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxFragmentPktCnt, count);
        /* FCS errors */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxFcsErrorCnt, count);
        /* Mtu exceeded pkts, good FCS */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxMtuExceededPktCnt, count);
        /* Oversize pkts, bad FCS */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxJabberPktCnt, count);
        break;

    case snmpIfOutOctets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxByteCnt, count);
        break;

    case snmpIfOutUcastPkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxUcPktCnt, count);
        break;

    case snmpIfOutNUcastPkts:
        /* Multicast packets transmitted */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxMcPktCnt, count);
        /* Broadcast packets transmitted */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxBroadcastPktCnt, count);
        break;

    case snmpIfOutDiscards:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxMultipleDeferralPktCnt, count);
        /* other causes of discards? */
        break;

    case snmpIfOutErrors:       /* Excessive collisions */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxExcessiveCollisionPktCnt, count);
        break;

        /* *** RFC 1493 *** */

    case snmpDot1dBasePortDelayExceededDiscards:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxMultipleDeferralPktCnt, count);
        break;

    case snmpDot1dBasePortMtuExceededDiscards:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxMtuExceededPktCnt, count);
        break;

    case snmpDot1dTpPortInFrames:    /* Should be only bridge mgmt */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxPktCnt, count);
        break;

    case snmpDot1dTpPortOutFrames:   /* Should be only bridge mgmt */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxPktCnt, count);
        break;

    case snmpDot1dPortInDiscards:
        STAT_ADD_BLOCK_ILIB(unit, port, fe2000CounterIlibDiscardPktCnt, count);
        break;

        /* *** RFC 1757 *** */
    case snmpEtherStatsOctets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxByteCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxByteCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRuntByteCnt, count);
        break;

    case snmpEtherStatsPkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxPktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxPktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRuntPktCnt, count);
        break;

    case snmpEtherStatsBroadcastPkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxBroadcastPktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxBroadcastPktCnt, count);
        break;

    case snmpEtherStatsMulticastPkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxMcPktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxMcPktCnt, count);
        break;

    case snmpEtherStatsCRCAlignErrors:    /* CRC errors + alignment errors */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxFcsErrorCnt, count);
        break;

    case snmpEtherStatsUndersizePkts:    /* Undersize frames */
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxUndersizePktCnt, count);
        break;

    case snmpEtherStatsOversizePkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxOversizePktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxOversizePktCnt, count);
        break;

    case snmpEtherStatsFragments:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxFragmentPktCnt, count);
        break;

    case snmpEtherStatsJabbers:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxJabberPktCnt, count);
        break;

    case snmpEtherStatsCollisions:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxTotalCollisionCnt, count);
        break;

    case snmpEtherStatsPkts64Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange64PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange64PktCnt, count);
        break;

    case snmpEtherStatsPkts65to127Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange127PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange127PktCnt, count);
        break;

    case snmpEtherStatsPkts128to255Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange255PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange255PktCnt, count);
        break;

    case snmpEtherStatsPkts256to511Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange511PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange511PktCnt, count);
        break;

    case snmpEtherStatsPkts512to1023Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange1023PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange1023PktCnt, count);
        break;

    case snmpEtherStatsPkts1024to1518Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange1518PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange1518PktCnt, count);
        break;

        /* *** Not actually in rfc1757 *** */

    case snmpBcmEtherStatsPkts1519to1522Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange1522PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange1522PktCnt, count);
        break;

    case snmpBcmEtherStatsPkts1522to2047Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange2047PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange2047PktCnt, count);
        break;

    case snmpBcmEtherStatsPkts2048to4095Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange4095PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange4095PktCnt, count);
        break;

    case snmpBcmEtherStatsPkts4095to9216Octets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxRange9216PktCnt, count);
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxRange9216PktCnt, count);
        break;

    case snmpEtherStatsTXNoErrors:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxValidPktCnt, count);
        break;

    case snmpEtherStatsRXNoErrors:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxValidPktCnt, count);
        break;

        /* *** RFC 2665 *** */
    case snmpDot3StatsAlignmentErrors:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxAlignmentErrorPktCnt, count);
        break;

    case snmpDot3StatsFCSErrors:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxFcsErrorCnt, count);
        break;

    case snmpDot3StatsSingleCollisionFrames:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxSingleCollisionPktCnt, count);
        break;

    case snmpDot3StatsMultipleCollisionFrames:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxMultipleCollisionPktCnt, count);
        break;

    case snmpDot3StatsSQETTestErrors:
        /* Always 0 */
        break;

    case snmpDot3StatsDeferredTransmissions:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxSingleDeferralPktCnt, count);
        break;

    case snmpDot3StatsLateCollisions:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxLateCollisionPktCnt, count);
        break;

    case snmpDot3StatsExcessiveCollisions:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxExcessiveCollisionPktCnt, count);
        break;

    case snmpDot3StatsInternalMacTransmitErrors:
        /* Always 0 */
        break;

    case snmpDot3StatsCarrierSenseErrors:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxFalseCarrierCnt, count);
        break;

    case snmpDot3StatsFrameTooLongs:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxMtuExceededPktCnt, count);
        break;

    case snmpDot3StatsInternalMacReceiveErrors:
        /* Always 0 */
        break;

    case snmpDot3StatsSymbolErrors:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxInvalidDataCnt, count);
        break;

    case snmpDot3ControlInUnknownOpcodes:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxUnknownCtrlPktCnt, count);
        break;

    case snmpDot3InPauseFrames:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxPausePktCnt, count);
        break;

    case snmpDot3OutPauseFrames:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxPausePktCnt, count);
        break;

        /* *** RFC 2233 high capacity versions of RFC1213 objects *** */
    case snmpIfHCInOctets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxByteCnt, count);
        break;

    case snmpIfHCInUcastPkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxUcPktCnt, count);
        break;

    case snmpIfHCInMulticastPkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxMcPktCnt, count);
        break;

    case snmpIfHCInBroadcastPkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmRxBroadcastPktCnt, count);
        break;

    case snmpIfHCOutOctets:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxByteCnt, count);
        break;

    case snmpIfHCOutUcastPkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxUcPktCnt, count);
        break;

    case snmpIfHCOutMulticastPkts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxMcPktCnt, count);
        break;

    case snmpIfHCOutBroadcastPckts:
        STAT_ADD_BLOCK_AM(unit, port, fe2000CounterAmTxBroadcastPktCnt, count);
        break;

    default:
        return BCM_E_UNAVAIL;
        break;
    }

    *value = count;

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_stat_xe_get
 * Description:
 *     Get the specified statistic for an XE port.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     type  - SNMP statistics type (see bcm/stat.h)
 *     value - (OUT) 64 bit statistic counter value
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 *
 * Notes:
 *     Assumes valid unit and valid XE port.
 */
STATIC int
_bcm_fe2000_stat_xe_get(int unit, bcm_port_t port, bcm_stat_val_t type,
                        uint64 *value)
{
    uint64  count;

    COMPILER_REFERENCE(&count);  /* Work around PPC compiler bug */
    COMPILER_64_ZERO(count);

    switch (type) {
        /* *** RFC 1213 *** */
    case snmpIfInOctets:        /* Bytes received */
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxByteCnt, count);
        break;

    case snmpIfInUcastPkts:     /* Unicast packets received */
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxUnicastPktCnt, count);
        break;

    case snmpIfInNUcastPkts:   /* Multicast frames plus broadcast frames */
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxMcPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxBroadcastPktCnt, count);
        break;

    case snmpIfInErrors:  /* RX Errors or Receive packets - non-error frames */
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxFcsPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxJabberPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxOversizePktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxUndersizePktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxFragmentPktCnt, count);
        break;

    case snmpIfOutOctets:      /* Bytes transmitted */
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxByteCnt, count);
        _bcm_fe2000_stat_xm_tx_byte_count_adjust(unit, port, &count);
        break;

    case snmpIfOutUcastPkts:   /* Unicast packets transmitted */
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxUnicastPktCnt, count);
        break;

    case snmpIfOutNUcastPkts:  /* Broadcast frames plus multicast frames */
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxMcPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxBroadcastPktCnt, count);
        break;

        /* *** RFC 1493 *** */
    case snmpDot1dBasePortDelayExceededDiscards:
        /* Neither processing engine or QE discard anything due to delay */
        count = 0;
        break;

    case snmpDot1dBasePortMtuExceededDiscards:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxOversizePktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxOversizePktCnt, count);
        break;

    case snmpDot1dTpPortInFrames:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxPktCnt, count);
        break;

    case snmpDot1dTpPortOutFrames:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxPktCnt, count);
        break;

    case snmpDot1dPortInDiscards:
        STAT_ADD_BLOCK_ILIB(unit, port, fe2000CounterIlibDiscardPktCnt, count);
        break;

        /* *** RFC 1757 *** */
    case snmpEtherStatsOctets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxByteCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxByteCnt, count);
        _bcm_fe2000_stat_xm_tx_byte_count_adjust(unit, port, &count);
        break;

    case snmpEtherStatsPkts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxFragmentPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxUndersizePktCnt, count);
        break;

    case snmpEtherStatsBroadcastPkts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxBroadcastPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxBroadcastPktCnt, count);
        break;

    case snmpEtherStatsMulticastPkts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxMcPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxMcPktCnt, count);
        break;

    case snmpEtherStatsCRCAlignErrors:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxFcsPktCnt, count);
        break;

    case snmpEtherStatsUndersizePkts:    /* Undersize frames */
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxUndersizePktCnt, count);
        break;

    case snmpEtherStatsOversizePkts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxOversizePktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxOversizePktCnt, count);
        break;

    case snmpEtherStatsFragments:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxFragmentPktCnt, count);
        break;

    case snmpEtherStatsJabbers:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxJabberPktCnt, count);
        break;

    case snmpEtherStatsPkts64Octets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxRange64PktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxRange64PktCnt, count);
        break;

    case snmpEtherStatsPkts65to127Octets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxRange127PktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxRange127PktCnt, count);
        break;

    case snmpEtherStatsPkts128to255Octets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxRange255PktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxRange255PktCnt, count);
        break;

    case snmpEtherStatsPkts256to511Octets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxRange511PktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxRange511PktCnt, count);
        break;

    case snmpEtherStatsPkts512to1023Octets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxRange1023PktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxRange1023PktCnt, count);
        break;

    case snmpEtherStatsPkts1024to1518Octets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxRange1518PktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxRange1518PktCnt, count);
        break;

        /* *** not actually in rfc1757 *** */
    case snmpBcmEtherStatsPkts1522to2047Octets:  /* 1519 to 2047*/
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxRange2047PktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxRange2047PktCnt, count);
        break;

    case snmpBcmEtherStatsPkts2048to4095Octets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxRange4095PktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxRange4095PktCnt, count);
        break;

    case snmpBcmEtherStatsPkts4095to9216Octets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxRange9216PktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxRange9216PktCnt, count);
        break;

    case snmpEtherStatsTXNoErrors:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxAllPktCnt, count);
        break;

    case snmpEtherStatsRXNoErrors:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxAllPktCnt, count);
        break;

        /* *** RFC 2665 *** */
    case snmpDot3StatsFCSErrors:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxFcsPktCnt, count);
        break;

    case snmpDot3StatsInternalMacTransmitErrors:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxUnderflowPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxErrorPktCnt, count);
        break;

    case snmpDot3StatsFrameTooLongs:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxMtuExceededPktCnt, count);
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxBadOversizePktCnt, count);
        break;

    case snmpDot3StatsSymbolErrors:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxErrorPktCnt, count);
        break;

    case snmpDot3ControlInUnknownOpcodes:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxUnknownCtrlPktCnt, count);
        break;

    case snmpDot3InPauseFrames:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxPausePktCnt, count);
        break;

    case snmpDot3OutPauseFrames:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxPauseCnt, count);
        break;

        /* *** RFC 2233 high capacity versions of RFC1213 objects *** */

    case snmpIfHCInOctets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxByteCnt, count);
        break;

    case snmpIfHCInUcastPkts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxUnicastPktCnt, count);
        break;

    case snmpIfHCInMulticastPkts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxMcPktCnt, count);
        break;

    case snmpIfHCInBroadcastPkts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmRxBroadcastPktCnt, count);
        break;

    case snmpIfHCOutOctets:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxByteCnt, count);
        _bcm_fe2000_stat_xm_tx_byte_count_adjust(unit, port, &count);
        break;

    case snmpIfHCOutUcastPkts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxUnicastPktCnt, count);
        break;

    case snmpIfHCOutMulticastPkts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxMcPktCnt, count);
        break;

    case snmpIfHCOutBroadcastPckts:
        STAT_ADD_BLOCK_XM(unit, port, fe2000CounterXmTxBroadcastPktCnt, count);
        break;

    default:
        return BCM_E_UNAVAIL;
        break;
    }

    *value = count;

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_sw_counter_init
 * Description:
 *     Initialize and start the counter collection, software
 *     accumulation process, on given unit.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
STATIC int
_bcm_fe2000_sw_counter_init(int unit)
{
    pbmp_t       pbmp;
    sal_usecs_t  interval;
    uint32       flags;

    /* Set bitmap of ports to collect counters ont */
    if (soc_property_get_str(unit, spn_BCM_STAT_PBMP) == NULL) {
        SOC_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    } else {
        pbmp = soc_property_get_pbmp(unit, spn_BCM_STAT_PBMP, 0);
    }

    interval = (SAL_BOOT_BCMSIM) ? BCMSIM_STAT_INTERVAL : 1000000;
    interval = soc_property_get(unit, spn_BCM_STAT_INTERVAL, interval);

    flags = soc_property_get(unit, spn_BCM_STAT_FLAGS, 0);

    SOC_IF_ERROR_RETURN(soc_sbx_fe2000_counter_init(unit, flags,
                                                    interval, pbmp));

    return BCM_E_NONE;
}

int
bcm_fe2000_stat_detach(int unit)
{
    bcm_stat_info_t            *si;
    int                         result = BCM_E_NONE;

    BCM_STAT_VVERB(("%s(%d) - Enter\n", FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || unit >= BCM_MAX_NUM_UNITS) {
        /* stat_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX
         */
        return BCM_E_UNIT;
    }

    si = &STAT_CNTL(unit);

    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        result = _bcm_fe2000_g2p3_stat_block_detach(unit,
                                                    SOC_SBX_G2P3_INGCTR_ID,
                                                    NULL);
#endif /* BCM_FE2000_P3_SUPPORT */
    }

    if (NULL != si->segInfo) {
        sal_free(si->segInfo);
    }

#if 0 
    result = soc_sbx_counter_detach(unit);
#endif

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stat_init
 * Description:
 *      Initializes the BCM stat module.
 * Parameters:
 *      unit - device unit number.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INTERNAL  - Chip access failure.
 */
int
bcm_fe2000_stat_init(int unit)
{
    bcm_stat_info_t            *si;
    int                         size;
    int                         result = BCM_E_NONE;
    int                         jumbo_size;
    int                         max_ctr_seg = 0;
    int                         blki;
    int                         init_blk[] = {
        SOC_SBX_G2P3_INGCTR_ID, SOC_SBX_G2P3_EGRCTR_ID,
    };

    BCM_STAT_VVERB(("%s(%d) - Enter\n", FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || unit >= BCM_MAX_NUM_UNITS) {
        /* stat_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX 
         */
        return BCM_E_UNIT;
    }


    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        max_ctr_seg = SOC_SBX_G2P3_COUNTER_MAX_ID;
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
     
    si = &STAT_CNTL(unit);
    if (si->segInfo != NULL) {
        result = bcm_fe2000_stat_detach(unit);
        BCM_IF_ERROR_RETURN(result);
        si->segInfo = NULL;
    }

    size = max_ctr_seg * sizeof(bcm_stat_sbx_info_t);
    if (NULL == si->segInfo) {
        si->segInfo = sal_alloc(size, "Counter Segments");
        if (NULL == si->segInfo) {
            return BCM_E_MEMORY;
        }
    }
    sal_memset(si->segInfo, 0, size);

    si->init = TRUE;

    /* Set max packet size */
    jumbo_size = soc_property_get(unit, spn_BCM_STAT_JUMBO,
                                  STAT_MAX_PKT_SIZE_DEFAULT);
    if ( (jumbo_size < STAT_MAX_PKT_SIZE_DEFAULT) || (jumbo_size > 0x3fff) ) {
        jumbo_size = STAT_MAX_PKT_SIZE_DEFAULT;
    }
    SOC_IF_ERROR_RETURN(soc_sbx_fe2000_counter_max_pkt_size(unit,
                                                            jumbo_size));

    result = _bcm_fe2000_sw_counter_init(unit);
    if (BCM_FAILURE(result)) {
        BCM_STAT_ERR(("bcm_stat_init: sw_counter_init failed: %s\n",
                      bcm_errmsg(result)));
        return result;
    }

    for (blki=0; blki < sizeof(init_blk) / sizeof(init_blk[0]); blki++) {
        result = _bcm_fe2000_g2p3_stat_block_init(unit, init_blk[blki], NULL);
        if (BCM_FAILURE(result)) {
            BCM_STAT_ERR(("%s(%d) -Could not attach counter block %d: %s\n",
                          FUNCTION_NAME(), unit, init_blk[blki], 
                          bcm_errmsg(result)));
            return result;
        }
    }
                              
    BCM_STAT_VERB(("bcm_stat_init: unit=%d rv=%d(%s)\n",
               unit, result, bcm_errmsg(result)));

    return result;
}

int _bcm_fe2000_stat_segment_valid(int unit, uint32 segment)
{
    int     rv = BCM_E_NONE;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (segment >= SOC_SBX_G2P3_COUNTER_MAX_ID) {
            rv = BCM_E_PARAM;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv =  BCM_E_INTERNAL;
    }

    return rv;
}


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
                                 shr_aidxres_element_t count)
{
    /* be sure we're initialised */
    STAT_CHECK_INIT(unit);

    BCM_IF_ERROR_RETURN(_bcm_fe2000_stat_segment_valid(unit, type));

    /* ask the list for an appropriate free contiguous block */
    return shr_aidxres_list_alloc_block(STAT_CNTL(unit).segInfo[type].pList,
                                        count,
                                        start);
}

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
                                shr_aidxres_element_t start)
{
    /* be sure we're initialised */
    STAT_CHECK_INIT(unit);

    BCM_IF_ERROR_RETURN(_bcm_fe2000_stat_segment_valid(unit, type));

    /* return the block to the list */
    return shr_aidxres_list_free(STAT_CNTL(unit).segInfo[type].pList, start);
}


/*
 * Function:
 *      _bcm_fe2000_g2p3_stat_block_init
 * Description:
 *      Initialize a statistics block for G2P3 ucode
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      pCtlBlock - pointer to a control structure defining params for the
 *                  block. If NULL, use default.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXX      - Failure
 */
int 
_bcm_fe2000_g2p3_stat_block_init(int unit, soc_sbx_g2p3_counter_id_t type,
                                     bcm_stat_sbx_info_t *pCtlBlock)
{
    int                             rv = BCM_E_NONE;
    bcm_stat_info_t                 *si;
    bcm_stat_sbx_info_t             temp_block;
    /*sbStatus_t                      sbRv;*/
    soc_sbx_g2p3_counter_params_t   params;
    uint32                          eject_rate;
    shr_aidxres_list_handle_t       plist;
    char                            *alloc_name = NULL;
    
    BCM_STAT_VVERB(("%s(%d, %d, %s) - Enter\n", FUNCTION_NAME(), unit, type,
                    pCtlBlock==NULL?"NULL":"ptr"));

    STAT_CHECK_INIT(unit);          /* check that module is intiailized */

    BCM_IF_ERROR_RETURN(_bcm_fe2000_stat_segment_valid(unit, type));
    
    si = &STAT_CNTL(unit);

    if (NULL == pCtlBlock) {
        BCM_STAT_VVERB(("%s: control block NULL; trying defaults\n", FUNCTION_NAME()));
        sal_memset(&temp_block, 0, (sizeof(bcm_stat_sbx_info_t)));

        switch (type) {
        case SOC_SBX_G2P3_PDCTR_ID:
            rv = soc_sbx_g2p3_pdctr_counter_params_get(unit, &params);
            alloc_name = "pdctr";
            break;
        case SOC_SBX_G2P3_IRTCTR_ID:
            rv = soc_sbx_g2p3_irtctr_counter_params_get(unit, &params);
            alloc_name = "irtctr";
            break;
        case SOC_SBX_G2P3_IFPCTR_ID:
            rv = soc_sbx_g2p3_ifpctr_counter_params_get(unit, &params);
            alloc_name = "ifpctr";
            break;
        case SOC_SBX_G2P3_ERTCTR_ID:
            rv = soc_sbx_g2p3_ertctr_counter_params_get(unit, &params);
            alloc_name = "ertctr";
            break;
        case SOC_SBX_G2P3_EFPCTR_ID:
            rv = soc_sbx_g2p3_efpctr_counter_params_get(unit, &params);
            alloc_name = "efpctr";
            break;
        case SOC_SBX_G2P3_EXCCTR_ID:
            rv = soc_sbx_g2p3_excctr_counter_params_get(unit, &params);
            alloc_name = "excctr";
            break;
        case SOC_SBX_G2P3_INGCTR_ID:
            rv = soc_sbx_g2p3_ingctr_counter_params_get(unit, &params);
            alloc_name = "ingctr";
            break;
        case SOC_SBX_G2P3_EGRCTR_ID:
            rv = soc_sbx_g2p3_egrctr_counter_params_get(unit, &params);
            alloc_name = "egrctr";
            break;
        default:
            rv = BCM_E_PARAM;
        }

        if (rv == BCM_E_NONE) {
            temp_block.first = 1;
            temp_block.last = params.size -1;
            temp_block.segment = type;
            
            
        }
    } else {
        sal_memcpy(&temp_block, pCtlBlock, sizeof(temp_block));
    }

    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g2p3_counter_segment_enable((SOC_SBX_CONTROL(unit))->drv,
                                                 (uint32)type, 1);
#ifndef PLISIM
        if ((rv == BCM_E_NONE) && (!SOC_WARM_BOOT(unit))){
            sbFe2000CmuSegmentClear(
              ((soc_sbx_g2p3_state_t *)(SOC_SBX_CONTROL(unit))->drv)->pCmuMgr,
              (uint32)type);
        }
#endif
    }
    if (rv == BCM_E_NONE) {
        eject_rate = temp_block.ejectRate;
        
    }

    if ((rv == BCM_E_NONE) && (alloc_name)) {
        rv = shr_aidxres_list_create(&plist, 
                                     temp_block.first, temp_block.last,
                                     temp_block.first, temp_block.last,
                                     7, alloc_name);
        if (rv == BCM_E_NONE) {
            temp_block.pList = plist;
        }
    }

    if (rv == BCM_E_NONE) {
        /* successful, so copy info to the unit description */
        sal_memcpy(&(si->segInfo[type]), &temp_block, 
                   sizeof(bcm_stat_sbx_info_t));
    }

    return rv;
}

/*
 * Function:
 *      _bcm_fe2000_g2p3_stat_block_detach
 * Description:
 *      Detach statistics block for G2P3 ucode
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      pCtlBlock - pointer to a control structure defining params for the
 *                  block. If NULL, use default.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXX      - Failure
 */
int 
_bcm_fe2000_g2p3_stat_block_detach(int unit, soc_sbx_g2p3_counter_id_t type,
                                   bcm_stat_sbx_info_t *pCtlBlock)
{
    int                             rv = BCM_E_NONE;
    bcm_stat_info_t                 *si;
    bcm_stat_sbx_info_t             temp_block;
    soc_sbx_g2p3_counter_params_t   params;

    BCM_STAT_VVERB(("%s(%d, %d, %s) - Enter\n", FUNCTION_NAME(), unit, type,
                    pCtlBlock==NULL?"NULL":"ptr"));

    STAT_CHECK_INIT(unit);          /* check that module is intiailized */

    BCM_IF_ERROR_RETURN(_bcm_fe2000_stat_segment_valid(unit, type));

    si = &STAT_CNTL(unit);

    if (NULL == pCtlBlock) {
        BCM_STAT_VVERB(("%s: control block NULL; trying defaults\n", FUNCTION_NAME()));
        sal_memset(&temp_block, 0, (sizeof(bcm_stat_sbx_info_t)));

        switch (type) {
        case SOC_SBX_G2P3_PDCTR_ID:
            rv = soc_sbx_g2p3_pdctr_counter_params_get(unit, &params);
            break;
        case SOC_SBX_G2P3_IRTCTR_ID:
            rv = soc_sbx_g2p3_irtctr_counter_params_get(unit, &params);
            break;
        case SOC_SBX_G2P3_IFPCTR_ID:
            rv = soc_sbx_g2p3_ifpctr_counter_params_get(unit, &params);
            break;
        case SOC_SBX_G2P3_ERTCTR_ID:
            rv = soc_sbx_g2p3_ertctr_counter_params_get(unit, &params);
            break;
        case SOC_SBX_G2P3_EFPCTR_ID:
            rv = soc_sbx_g2p3_efpctr_counter_params_get(unit, &params);
            break;
        case SOC_SBX_G2P3_EXCCTR_ID:
            rv = soc_sbx_g2p3_excctr_counter_params_get(unit, &params);
            break;
        case SOC_SBX_G2P3_INGCTR_ID:
            rv = soc_sbx_g2p3_ingctr_counter_params_get(unit, &params);
            break;
        case SOC_SBX_G2P3_EGRCTR_ID:
            rv = soc_sbx_g2p3_egrctr_counter_params_get(unit, &params);
            break;
        default:
            rv = BCM_E_PARAM;
        }

        if (rv == BCM_E_NONE) {
            temp_block.first = 1;
            temp_block.last = params.size -1;
            temp_block.segment = type;
            
            
        }
    } else {
        sal_memcpy(&temp_block, pCtlBlock, sizeof(temp_block));
    }   
 
    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g2p3_counter_segment_enable((SOC_SBX_CONTROL(unit))->drv,
                                                 (uint32)type, 0); 
#ifndef PLISIM
        if ((rv == BCM_E_NONE) && (!SOC_WARM_BOOT(unit))){
            sbFe2000CmuSegmentClear(
              ((soc_sbx_g2p3_state_t *)(SOC_SBX_CONTROL(unit))->drv)->pCmuMgr,
              (uint32)type);
        }
#endif /* !PLISIM */
    }

    if (rv == BCM_E_NONE) {
        rv = shr_aidxres_list_destroy(si->segInfo[type].pList);
    }

    return rv;
}

/*
 * Function:
 *     bcm_stat_sync
 * Description:
 *     Synchronize software counters with hardware.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int bcm_fe2000_stat_sync(int unit)
{
    return soc_sbx_counter_sync(unit);
}


/*
 * Function:
 *     bcm_stat_clear
 * Description:
 *     Clear the port based statistics from the port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int bcm_fe2000_stat_clear(int unit, bcm_port_t port)
{

    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(soc_sbx_fe2000_counter_port_set(unit, port,
                                                        -1, -1, 0));

    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_stat_get
 * Description:
 *     Get the specified statistic for the given port.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     type  - SNMP statistics type (see stat.h)
 *     value - (OUT) 64-bit counter value
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_stat_get(int unit, bcm_port_t port, bcm_stat_val_t type,
                    uint64 *value)
{
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    if (IS_GE_PORT(unit,port)) {

        return _bcm_fe2000_stat_ge_get(unit, port, type, value);

    } else if (IS_XE_PORT(unit,port)) {

        return _bcm_fe2000_stat_xe_get(unit, port, type, value);
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     bcm_stat_get32
 * Purpose:
 *     Get the specified statistic for the given port.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     type  - SNMP statistics type (see stat.h)
 *     value - (OUT) 32-bit counter value
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_stat_get32(int unit, bcm_port_t port, bcm_stat_val_t type,
                      uint32 *value)
{
    int     rv;
    uint64  value64;

    rv = bcm_stat_get(unit, port, type, &value64);
    if (BCM_SUCCESS(rv)) {
        COMPILER_64_TO_32_LO(*value, value64);
    }

    return rv;
}


#if 0
/*
 * Function:
 *      bcm_fe2000_mpls_label_stat_get
 * Description:
 *      Get the specified MPLS statistic from the chip
 * Parameters:
 *      unit  - device unit number.
 *      label - MPLS Label
 *      stat  - MPLS Entry statistics type
 *      val   - (OUT) 64-bit counter value.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 */
int bcm_fe2000_mpls_label_stat_get(int unit, bcm_mpls_label_t label,
                                   bcm_mpls_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_mpls_label_stat_get32
 * Description:
 *      Get the specified MPLS statistic from the chip
 * Parameters:
 *      unit  - device unit number.
 *      label - MPLS Label
 *      stat  - MPLS Entry statistics type
 *      val   - (OUT) 32-bit counter value.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 * Notes:
 *      Same as bcm_fe2000_mpls_label_stat_get, except converts result to
 *      32-bit.
 */
int bcm_fe2000_mpls_label_stat_get32(int unit, bcm_mpls_label_t label,
                                     bcm_mpls_stat_t stat, uint32 *val)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_mpls_label_stat_set
 * Description:
 *      Set the specified MPLS statistic to the indicated value
 * Parameters:
 *      unit  - device unit number.
 *      label - MPLS Label
 *      stat  - MPLS Entry statistics type
 *      val   - 64-bit value.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 * Notes:
 *      'val' must be zero (0).
 */
int bcm_fe2000_mpls_label_stat_set(int unit, bcm_mpls_label_t label,
                                   bcm_mpls_stat_t stat, uint64 val)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_mpls_label_stat_set32
 * Description:
 *      Set the specified MPLS statistic to the indicated value
 * Parameters:
 *      unit  - device unit number.
 *      label - MPLS Label
 *      stat  - MPLS Entry statistics type
 *      val   - 32-bit value.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 * Notes:
 *      Same as bcm_fe2000_mpls_label_stat_set, except accepts 32-bit value.
 *      'val' must be zero (0).
 */
int bcm_fe2000_mpls_label_stat_set32(int unit, bcm_mpls_label_t label,
                                     bcm_mpls_stat_t stat, uint32 val)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_mpls_label_stat_enable_set
 * Description:
 *      Enable/Disable statistics on the indicated MPLS entry.
 * Parameters:
 *      unit   - device unit number.
 *      label  - MPLS Label
 *      enable - TRUE/FALSE indicator indicating action to enable/disable.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 */
int bcm_fe2000_mpls_label_stat_enable_set(int unit, bcm_mpls_label_t label,
                                          int enable)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_stat_get
 * Description:
 *      Get the specified ipmc statistic from the chip
 * Parameters:
 *      unit  - device unit number.
 *      ipmc  - Pointer to IPMC entry
 *      stat  - IPMC entry statistics type
 *      val   - (OUT) 64-bit counter value.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 */
int bcm_fe2000_ipmc_stat_get(int unit, bcm_ipmc_addr_t *ipmc,
                             bcm_ipmc_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_stat_get32
 * Description:
 *      Get the specified ipmc statistic from the chip
 * Parameters:
 *      unit  - device unit number.
 *      ipmc  - Pointer to IPMC entry
 *      stat  - IPMC entry statistics type
 *      val   - (OUT) 32-bit counter value.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 * Notes:
 *      Same as bcm_fe2000_ipmc_stat_get, except converts result to 32-bit.
 */
int bcm_fe2000_ipmc_stat_get32(int unit, bcm_ipmc_addr_t *ipmc,
                               bcm_ipmc_stat_t stat, uint32 *val)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_stat_set
 * Description:
 *      Set the specified ipmc statistic to the indicated value
 * Parameters:
 *      unit  - device unit number.
 *      ipmc  - Pointer to IPMC entry
 *      stat  - IPMC entry statistics type
 *      val   - 64-bit value.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 * Notes:
 *      'val' must be zero (0).
 */
int bcm_fe2000_ipmc_stat_set(int unit, bcm_ipmc_addr_t *ipmc,
                             bcm_ipmc_stat_t stat, uint64 val)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_stat_set32
 * Description:
 *      Set the specified ipmc statistic to the indicated value
 * Parameters:
 *      unit  - device unit number.
 *      ipmc  - Pointer to IPMC entry
 *      stat  - IPMC entry statistics type
 *      val   - 32-bit value.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 * Notes:
 *      Same as bcm_fe2000_ipmc_stat_set, except accepts 32-bit value.
 *      'val' must be zero (0).
 */
int bcm_fe2000_ipmc_stat_set32(int unit, bcm_ipmc_addr_t *ipmc,
                               bcm_ipmc_stat_t stat, uint32 val)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_stat_enable_set
 * Description:
 *      Enable/Disable statistics on the indicated IPMC entry.
 * Parameters:
 *      unit   - device unit number.
 *      ipmc   - Pointer to IPMC entry
 *      enable - TRUE/FALSE indicator indicating action to enable/disable.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 *      BCM_E_INTERNAL  - Chip access failure.
 */
int bcm_fe2000_ipmc_stat_enable_set(int unit, bcm_ipmc_addr_t *ipmc,
                                    int enable)
{
    return BCM_E_UNAVAIL;
}
#endif

/*
 * Function:
 *          bcm_fe2000_stat_custom_get
 * Description:
 *      Get debug counter select value.
 * Parameters:
 *      unit  - device unit number.
 *      port  - Port number
 *      type  - SNMP statistics type.
 *      flags - (OUT) The counter select value
 *        (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_UNAVAIL   - This API is unavailable on this hardware
 */
int
bcm_fe2000_stat_custom_get(int unit, bcm_port_t port, bcm_stat_val_t type,
                           uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_stat_custom_set
 * Description:
 *      Set debug counter to count certain packet types.
 * Parameters:
 *      unit  - device unit number.
 *      port  - Port number, -1 to set all ports.
 *      type  - SNMP statistics type.
 *      flags - The counter select value (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_UNAVAIL   - This API is unavailable on this hardware
 */
int
bcm_fe2000_stat_custom_set(int unit, bcm_port_t port, bcm_stat_val_t type,
                           uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_stat_custom_add
 * Description:
 *      Add a certain packet type to debug counter to count
 * Parameters:
 *      unit    - device unit number.
 *      port    - Port number
 *      type    - SNMP statistics type.
 *      trigger - The counter select value (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_UNAVAIL   - This API is unavailable on this hardware
 */
int
bcm_fe2000_stat_custom_add(int unit, bcm_port_t port, bcm_stat_val_t type,
                        bcm_custom_stat_trigger_t trigger)
{
   return BCM_E_UNAVAIL;
}
/*
 * Function:
 *      bcm_stat_custom_delete
 * Description:
 *      Deletes a certain packet type from debug counter
 * Parameters:
 *      unit    - device unit number.
 *      port    - Port number
 *      type    - SNMP statistics type.
 *      trigger - The counter select value (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_UNAVAIL   - This API is unavailable on this hardware
 */
int
bcm_fe2000_stat_custom_delete(int unit, bcm_port_t port,bcm_stat_val_t type,
                              bcm_custom_stat_trigger_t trigger)
{
   return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_stat_custom_delete_all
 * Description:
 *      Deletes all packet types from debug counter
 * Parameters:
 *      unit    - device unit number.
 *      port    - Port number
 *      type    - SNMP statistics type.
 * Returns:
 *      BCM_E_UNAVAIL   - This API is unavailable on this hardware
 */
int
bcm_fe2000_stat_custom_delete_all(int unit, bcm_port_t port, bcm_stat_val_t type)
{
   return BCM_E_UNAVAIL;

}

/*
 * Function:
 *      bcm_fe2000_stat_custom_check
 * Description:
 *      Check if certain packet types is part of debug counter
 * Parameters:
 *      unit    - device unit number.
 *      port    - Port number
 *      type    - SNMP statistics type.
 *      trigger - The counter select value (see stat.h for bit definitions).
 *      result  - [OUT] result of a query. 0 if positive , -1 if negative
 * Returns:
 *      BCM_E_UNAVAIL   - This API is unavailable on this hardware
 */
int
bcm_fe2000_stat_custom_check(int unit, bcm_port_t port, bcm_stat_val_t type,
                             bcm_custom_stat_trigger_t trigger, int *result)
{
   return BCM_E_UNAVAIL;
}
