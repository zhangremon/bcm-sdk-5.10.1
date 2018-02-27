/*
 * $Id: stat_generic.c 1.7.44.1 Broadcom SDK $
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
 */

#include <soc/drv.h>
#include <soc/counter.h>
#include <soc/debug.h>

#include <bcm/stat.h>
#include <bcm/error.h>
#include <bcm_int/esw/stat.h>
#include <bcm_int/esw_dispatch.h>

/*
 * Function:
 *      _bcm_stat_generic_get
 * Description:
 *      Get the specified statistic for a port with common counter shared
 *      by both XE and GE MAC.
 * Parameters:
 *      unit - PCI device unit number (driver internal)
 *      port - zero-based port number
 *      type - SNMP statistics type (see stat.h)
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_PARAM - Illegal parameter.
 *      BCM_E_INTERNAL - Chip access failure.
 */
int
_bcm_stat_generic_get(int unit, bcm_port_t port, bcm_stat_val_t type,
                      uint64 *val)
{
    uint64 count, count1;
    REG_MATH_DECL;       /* Required for use of the REG_* macros */

    COMPILER_REFERENCE(&count);  /* Work around PPC compiler bug */
    COMPILER_64_ZERO(count);

    switch (type) {
        /* *** RFC 1213 *** */
    case snmpIfInOctets:
        REG_ADD(unit, port, RBYTr, count);
        break;
    case snmpIfInUcastPkts:             /* Unicast frames */
        REG_ADD(unit, port, RUCr, count);
        break;
    case snmpIfInNUcastPkts:             /* Non-unicast frames */
        REG_ADD(unit, port, RMCAr, count); /* + multicast */
        REG_ADD(unit, port, RBCAr, count); /* + broadcast */
        break;
    case snmpIfInBroadcastPkts:          /* Broadcast frames */
        REG_ADD(unit, port, RBCAr, count);
        break;
    case snmpIfInMulticastPkts:          /* Multicast frames */
        REG_ADD(unit, port, RMCAr, count);
        break;
    case snmpIfInDiscards:           /* Dropped packets including aborted */
        REG_ADD(unit, port, RDBGC0r, count); /* Ingress drop conditions */
        BCM_IF_ERROR_RETURN
            (_bcm_stat_counter_non_dma_extra_get(unit,
                                   SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING,
                                                 port, &count1));
        break;
    case snmpIfInErrors: /* RX Errors or Receive packets - non-error frames */
        REG_ADD(unit, port, RFCSr, count);
        REG_ADD(unit, port, RJBRr, count);
        if (COUNT_OVR_ERRORS(unit)) {
            REG_ADD(unit, port, ROVRr, count);
        }
        REG_ADD(unit, port, RRPKTr, count);
        break;
    case snmpIfInUnknownProtos:
        break;
    case snmpIfOutOctets:
        REG_ADD(unit, port, TBYTr, count);
        break;
    case snmpIfOutUcastPkts:             /* Unicast frames */
        REG_ADD(unit, port, TUCAr, count);
        break;
    case snmpIfOutNUcastPkts:            /* Non-unicast frames */
        REG_ADD(unit, port, TMCAr, count); /* + multicast */
        REG_ADD(unit, port, TBCAr, count); /* + broadcast */
        break;
    case snmpIfOutBroadcastPkts:         /* Broadcast frames */
        REG_ADD(unit, port, TBCAr, count);
        break;
    case snmpIfOutMulticastPkts:         /* Multicast frames */
        REG_ADD(unit, port, TMCAr, count);
        break;
    case snmpIfOutDiscards:              /* Aged packet counter */
        REG_ADD(unit, port, HOLDr, count);  /* L2 MTU drops */
        REG_ADD(unit, port, TDBGC3r, count);
        BCM_IF_ERROR_RETURN
            (_bcm_stat_counter_non_dma_extra_get(unit,
                                   SOC_COUNTER_NON_DMA_COSQ_DROP_PKT,
                                                 port, &count1));
        COMPILER_64_ADD_64(count, count1);
        break;
    case snmpIfOutErrors:   /* Error packets could not be xmitted */
        
        break;
    case snmpIfOutQLen:
        {
            uint32 qcount;
            if (bcm_esw_port_queued_count_get(unit, port, &qcount) >= 0) {
                COMPILER_64_ADD_32(count, qcount);
            }
        }
        break;
    case snmpIpInReceives:
        REG_ADD(unit, port, RIPC4r, count);
        break;
    case snmpIpInHdrErrors:
        REG_ADD(unit, port, RIPD4r, count);
        break;
    case snmpIpForwDatagrams:
        REG_ADD(unit, port, TDBGC4r, count);
        break;
    case snmpIpInDiscards:
        REG_ADD(unit, port, RIPHE4r, count);
        REG_ADD(unit, port, RIPD4r, count);
        break;

        /* *** RFC 1493 *** */

    case snmpDot1dBasePortDelayExceededDiscards:
        break;
    case snmpDot1dBasePortMtuExceededDiscards:
        REG_ADD(unit, port, RMTUEr, count);
        break;
    case snmpDot1dTpPortInFrames:
        REG_ADD(unit, port, RPKTr, count);
        break;
    case snmpDot1dTpPortOutFrames:
        REG_ADD(unit, port, TPKTr, count);
        break;
    case snmpDot1dPortInDiscards:
        REG_ADD(unit, port, RDISCr, count);
        REG_ADD(unit, port, RIPD4r, count);
        REG_ADD(unit, port, RIPD6r, count);
        REG_ADD(unit, port, RPORTDr, count);
        break;

        /* *** RFC 1757 *** */

    case snmpEtherStatsDropEvents:
        REG_ADD(unit, port, RDISCr, count);
        break;
    case snmpEtherStatsOctets:
        REG_ADD(unit, port, RBYTr, count);
        REG_ADD(unit, port, TBYTr, count);
        break;
    case snmpEtherStatsPkts:
        REG_ADD(unit, port, RPKTr, count);
        REG_ADD(unit, port, TPKTr, count);
        REG_ADD(unit, port, RRPKTr, count); /* Runts */
        break;
    case snmpEtherStatsBroadcastPkts:
        REG_ADD(unit, port, RBCAr, count);
        REG_ADD(unit, port, TBCAr, count);
        break;
    case snmpEtherStatsMulticastPkts:
        REG_ADD(unit, port, RMCAr, count);
        REG_ADD(unit, port, TMCAr, count);
        break;
    case snmpEtherStatsCRCAlignErrors:
        REG_ADD(unit, port, RFCSr, count);
        break;
    case snmpEtherStatsUndersizePkts:
        REG_ADD(unit, port, RUNDr, count);
        break;
    case snmpEtherStatsOversizePkts:
        if (COUNT_OVR_ERRORS(unit)) {
            REG_ADD(unit, port, ROVRr, count);
            REG_ADD(unit, port, TOVRr, count);
        }
        break;
    case snmpEtherRxOversizePkts:
        if (COUNT_OVR_ERRORS(unit)) {
            REG_ADD(unit, port, ROVRr, count);
        }
        break;
    case snmpEtherTxOversizePkts:
        if (COUNT_OVR_ERRORS(unit)) {
            REG_ADD(unit, port, TOVRr, count);
        }
        break;
    case snmpEtherStatsFragments:
        REG_ADD(unit, port, RFRGr, count);
        break;
    case snmpEtherStatsJabbers:
        REG_ADD(unit, port, RJBRr, count);
        break;
    case snmpEtherStatsCollisions:
        break;

        /* *** rfc1757 definition counts receive packet only *** */

    case snmpEtherStatsPkts64Octets:
        REG_ADD(unit, port, R64r, count);
        REG_ADD(unit, port, T64r, count);
        break;
    case snmpEtherStatsPkts65to127Octets:
        REG_ADD(unit, port, R127r, count);
        REG_ADD(unit, port, T127r, count);
        break;
    case snmpEtherStatsPkts128to255Octets:
        REG_ADD(unit, port, R255r, count);
        REG_ADD(unit, port, T255r, count);
        break;
    case snmpEtherStatsPkts256to511Octets:
        REG_ADD(unit, port, R511r, count);
        REG_ADD(unit, port, T511r, count);
        break;
    case snmpEtherStatsPkts512to1023Octets:
        REG_ADD(unit, port, R1023r, count);
        REG_ADD(unit, port, T1023r, count);
        break;
    case snmpEtherStatsPkts1024to1518Octets:
        REG_ADD(unit, port, R1518r, count);
        REG_ADD(unit, port, T1518r, count);
        break;
    case snmpBcmEtherStatsPkts1519to1522Octets: /* not in rfc1757 */
        REG_ADD(unit, port, R2047r, count);
        REG_ADD(unit, port, T2047r, count);
        break;
    case snmpBcmEtherStatsPkts1522to2047Octets: /* not in rfc1757 */
        REG_ADD(unit, port, R2047r, count);
        REG_ADD(unit, port, T2047r, count);
        break;
    case snmpBcmEtherStatsPkts2048to4095Octets: /* not in rfc1757 */
        REG_ADD(unit, port, R4095r, count);
        REG_ADD(unit, port, T4095r, count);
        break;
    case snmpBcmEtherStatsPkts4095to9216Octets: /* not in rfc1757 */
        REG_ADD(unit, port, R9216r, count);
        REG_ADD(unit, port, T9216r, count);
        break;

    case snmpBcmReceivedPkts64Octets:
        REG_ADD(unit, port, R64r, count);
        break;
    case snmpBcmReceivedPkts65to127Octets:
        REG_ADD(unit, port, R127r, count);
        break;
    case snmpBcmReceivedPkts128to255Octets:
        REG_ADD(unit, port, R255r, count);
        break;
    case snmpBcmReceivedPkts256to511Octets:
        REG_ADD(unit, port, R511r, count);
        break;
    case snmpBcmReceivedPkts512to1023Octets:
        REG_ADD(unit, port, R1023r, count);
        break;
    case snmpBcmReceivedPkts1024to1518Octets:
        REG_ADD(unit, port, R1518r, count);
        break;
    case snmpBcmReceivedPkts1519to2047Octets:
        REG_ADD(unit, port, R2047r, count);
        break;
    case snmpBcmReceivedPkts2048to4095Octets:
        REG_ADD(unit, port, R4095r, count);
        break;
    case snmpBcmReceivedPkts4095to9216Octets:
        REG_ADD(unit, port, R9216r, count);
        break;

    case snmpBcmTransmittedPkts64Octets:
        REG_ADD(unit, port, T64r, count);
        break;
    case snmpBcmTransmittedPkts65to127Octets:
        REG_ADD(unit, port, T127r, count);
        break;
    case snmpBcmTransmittedPkts128to255Octets:
        REG_ADD(unit, port, T255r, count);
        break;
    case snmpBcmTransmittedPkts256to511Octets:
        REG_ADD(unit, port, T511r, count);
        break;
    case snmpBcmTransmittedPkts512to1023Octets:
        REG_ADD(unit, port, T1023r, count);
        break;
    case snmpBcmTransmittedPkts1024to1518Octets:
        REG_ADD(unit, port, T1518r, count);
        break;
    case snmpBcmTransmittedPkts1519to2047Octets:
        REG_ADD(unit, port, T2047r, count);
        break;
    case snmpBcmTransmittedPkts2048to4095Octets:
        REG_ADD(unit, port, T4095r, count);
        break;
    case snmpBcmTransmittedPkts4095to9216Octets:
        REG_ADD(unit, port, T9216r, count);
        break;

    case snmpEtherStatsTXNoErrors:
        REG_ADD(unit, port, TPOKr, count);
        break;
    case snmpEtherStatsRXNoErrors:
        REG_ADD(unit, port, RPOKr, count);
        break;

        /* *** RFC 2665 *** */

    case snmpDot3StatsAlignmentErrors:
        break;
    case snmpDot3StatsFCSErrors:
        REG_ADD(unit, port, RFCSr, count);
        break;
    case snmpDot3StatsSingleCollisionFrames:
        break;
    case snmpDot3StatsMultipleCollisionFrames:
        break;
    case snmpDot3StatsSQETTestErrors:
        break;
    case snmpDot3StatsDeferredTransmissions:
        break;
    case snmpDot3StatsLateCollisions:
        break;
    case snmpDot3StatsExcessiveCollisions:
        break;
    case snmpDot3StatsInternalMacTransmitErrors:
        REG_ADD(unit, port, TUFLr, count);
        REG_ADD(unit, port, TERRr, count);
        break;
    case snmpDot3StatsCarrierSenseErrors:
        break;
    case snmpDot3StatsFrameTooLongs:
        REG_ADD(unit, port, RMTUEr, count);
        break;
    case snmpDot3StatsInternalMacReceiveErrors:
        break;
    case snmpDot3StatsSymbolErrors:
        REG_ADD(unit, port, RERPKTr, count);
        break;
    case snmpDot3ControlInUnknownOpcodes:
        REG_ADD(unit, port, RXUOr, count);
        break;
    case snmpDot3InPauseFrames:
        REG_ADD(unit, port, RXPFr, count);
        break;
    case snmpDot3OutPauseFrames:
        REG_ADD(unit, port, TXPFr, count);
        break;

        /* *** RFC 2233 high capacity versions of RFC1213 objects *** */

    case snmpIfHCInOctets:
        REG_ADD(unit, port, RBYTr, count);
        break;
    case snmpIfHCInUcastPkts:
        REG_ADD(unit, port, RUCr, count);       /* unicast pkts rcvd */
        break;
    case snmpIfHCInMulticastPkts:
        REG_ADD(unit, port, RMCAr, count);
        break;
    case snmpIfHCInBroadcastPkts:
        REG_ADD(unit, port, RBCAr, count);
        break;
    case snmpIfHCOutOctets:
        REG_ADD(unit, port, TBYTr, count);
        break;
    case snmpIfHCOutUcastPkts:
        REG_ADD(unit, port, TUCAr, count);
        break;
    case snmpIfHCOutMulticastPkts:
        REG_ADD(unit, port, TMCAr, count);
        break;
    case snmpIfHCOutBroadcastPckts:
        REG_ADD(unit, port, TBCAr, count);
        break;

        /* *** RFC 2465 *** */

    case snmpIpv6IfStatsInReceives:
        REG_ADD(unit, port, RIPC6r, count);
        REG_ADD(unit, port, IMRP6r, count);
        break;
    case snmpIpv6IfStatsInHdrErrors:
        REG_ADD(unit, port, RIPD6r, count);
        break;
    case snmpIpv6IfStatsInAddrErrors:
        REG_ADD(unit, port, RIPHE6r, count);
        break;
    case snmpIpv6IfStatsInDiscards:
        REG_ADD(unit, port, RIPHE6r, count);
        REG_ADD(unit, port, RIPD6r, count);
        break;
    case snmpIpv6IfStatsOutForwDatagrams:
        REG_ADD(unit, port, TDBGC0r, count);
        break;
    case snmpIpv6IfStatsOutDiscards:
        REG_ADD(unit, port, TDBGC1r, count);
        break;
    case snmpIpv6IfStatsInMcastPkts:
        REG_ADD(unit, port, IMRP6r, count);
        break;
    case snmpIpv6IfStatsOutMcastPkts:
        REG_ADD(unit, port, TDBGC2r, count);
        break;

        /* *** IEEE 802.1bb *** */
    case snmpIeee8021PfcRequests:
        REG_ADD(unit, port, TXPPr, count);
        break;
    case snmpIeee8021PfcIndications:
        REG_ADD(unit, port, RXPPr, count);
        break;

        /* *** RFC 1284 - unsupported in XGS *** */
    case snmpDot3StatsInRangeLengthError:
        break;

        /* *** RFC 4837 - unsupported in XGS *** */
    case snmpDot3OmpEmulationCRC8Errors:
    case snmpDot3MpcpRxGate:
    case snmpDot3MpcpRxRegister:
    case snmpDot3MpcpTxRegRequest:
    case snmpDot3MpcpTxRegAck:
    case snmpDot3MpcpTxReport:
    case snmpDot3EponFecCorrectedBlocks:
    case snmpDot3EponFecUncorrectableBlocks:
        break;

        /* IPMC counters (broadcom specific) */

    case snmpBcmIPMCBridgedPckts:
        REG_ADD(unit, port, RDBGC1r, count);
        break;
    case snmpBcmIPMCRoutedPckts:
        REG_ADD(unit, port, IMRP4r, count);
        REG_ADD(unit, port, IMRP6r, count);
        break;
    case snmpBcmIPMCInDroppedPckts:
        REG_ADD(unit, port, RDBGC2r, count);
        break;
    case snmpBcmIPMCOutDroppedPckts:
        REG_ADD(unit, port, TDBGC5r, count);
        break;

        /* EA (broadcom specific) - unsupported in XGS */
    case snmpBcmPonInDroppedOctets:
    case snmpBcmPonOutDroppedOctets:
    case snmpBcmPonInDelayedOctets:
    case snmpBcmPonOutDelayedOctets:
    case snmpBcmPonInDelayedHundredUs:
    case snmpBcmPonOutDelayedHundredUs:
    case snmpBcmPonInFrameErrors:
    case snmpBcmPonInOamFrames:
    case snmpBcmPonOutOamFrames:
    case snmpBcmPonOutUnusedOctets:
	break;

    default:
        soc_cm_debug(DK_WARN+DK_VERBOSE,
                     "_bcm_stat_generic_get: Statistic not supported: %d\n", type);
        return BCM_E_PARAM;
    }

    *val = count;

    return BCM_E_NONE;
}
