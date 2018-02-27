/*
 * $Id: stat_hg.c 1.47.44.1 Broadcom SDK $
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
 *	_bcm_stat_hg_get
 * Description:
 *	Get the specified statistic for a HG port on the StrataSwitch family
 *      of devices.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - zero-based port number
 *	type - SNMP statistics type (see stat.h)
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_PARAM - Illegal parameter.
 *	BCM_E_INTERNAL - Chip access failure.
 */
int
_bcm_stat_hg_get(int unit, bcm_port_t port, bcm_stat_val_t type, uint64 *val)
{
    uint64 count, count1;
    REG_MATH_DECL;       /* Required for use of the REG_* macros */

    COMPILER_REFERENCE(&count);  /* Work around PPC compiler bug */
    COMPILER_64_ZERO(count);

    switch (type) {
    case snmpIfInOctets:
	REG_ADD(unit, port, IRBYTr, count);
	break;
    case snmpIfInUcastPkts:
	REG_ADD(unit, port, IRPKTr, count);
	REG_SUB(unit, port, IRMCAr, count); /* - multicast */
	REG_SUB(unit, port, IRBCAr, count); /* - broadcast */
        REG_SUB(unit, port, IRFCSr, count); /* - bad FCS */
        REG_SUB(unit, port, IRXCFr, count); /* - good FCS, all MAC ctrl */
        REG_SUB(unit, port, IRFLRr, count); /* - good FCS, bad length */
        REG_SUB(unit, port, IRJBRr, count); /* - oversize, bad FCS */
        if (COUNT_OVR_ERRORS(unit)) {
            REG_SUB(unit, port, IROVRr, count); /* - oversize, good FCS */
        }
	break;
    case snmpIfInNUcastPkts:	/* Multicast frames plus broadcast frames */
	REG_ADD(unit, port, IRMCAr, count); /* + multicast */
	REG_ADD(unit, port, IRBCAr, count); /* + broadcast */
	break;
    case snmpIfInBroadcastPkts:	/* broadcast frames */
	REG_ADD(unit, port, IRBCAr, count); /* + broadcast */
	break;
    case snmpIfInMulticastPkts:	/* Multicast frames */
	REG_ADD(unit, port, IRMCAr, count); /* + multicast */
	break;
    case snmpIfInDiscards:	/* Dropped packets including aborted */
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
	    REG_ADD(unit, port, RDBGC0r, count); /* Ingress drop conditions */
            BCM_IF_ERROR_RETURN
                (_bcm_stat_counter_non_dma_extra_get(unit,
                                   SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING,
                                                     port, &count1));
            COMPILER_64_ADD_64(count, count1);
        } else {
	    REG_ADD(unit, port, IRDISCr, count);
        }
	/*
	 * SOC_IS_HERCULES1: add in IHOLD0-7, packets destined for each
	 * COSQ that were dropped due to lack of XQ resources
	 */
	break;
    case snmpIfInErrors: /* RX Errors or Receive packets - non-error frames */
	REG_ADD(unit, port, IRFCSr, count);
	REG_ADD(unit, port, IRJBRr, count);
	if (COUNT_OVR_ERRORS(unit)) {
	    REG_ADD(unit, port, IROVRr, count);
	}
	REG_ADD(unit, port, IRUNDr, count);
	REG_ADD(unit, port, IRFRGr, count);
	break;
    case snmpIfInUnknownProtos:	/* Not supported */
	break;
    case snmpIfOutOctets:	/* TX bytes */
	REG_ADD(unit, port, ITBYTr, count);
	break;
    case snmpIfOutUcastPkts:	/* ALL - mcast - bcast */
	REG_ADD(unit, port, ITPKTr, count);
	REG_SUB(unit, port, ITMCAr, count); /* - multicast */
	REG_SUB(unit, port, ITBCAr, count); /* - broadcast */
	break;
    case snmpIfOutNUcastPkts:	/* broadcast frames plus multicast frames */
	REG_ADD(unit, port, ITMCAr, count); /* + multicast */
	REG_ADD(unit, port, ITBCAr, count); /* + broadcast */
	break;
    case snmpIfOutBroadcastPkts:	/* broadcast frames */
	REG_ADD(unit, port, ITBCAr, count); /* + broadcast */
	break;
    case snmpIfOutMulticastPkts:	/* multicast frames */
	REG_ADD(unit, port, ITMCAr, count); /* + multicast */
	break;
    case snmpIfOutDiscards:	/* Aged packet counter */
        REG_ADD(unit, port, HOLDr, count);	/* L2 MTU drops */
	REG_ADD(unit, port, ITAGEr, count);
        BCM_IF_ERROR_RETURN
            (_bcm_stat_counter_extra_get(unit, EGRDROPPKTCOUNTr,
                                         port, &count1));
        COMPILER_64_ADD_64(count, count1);
        BCM_IF_ERROR_RETURN
            (_bcm_stat_counter_non_dma_extra_get(unit,
                                   SOC_COUNTER_NON_DMA_COSQ_DROP_PKT,
                                                 port, &count1));
        COMPILER_64_ADD_64(count, count1);
	break;
    case snmpIfOutErrors:   /* Error packets could not be xmitted */
	REG_ADD(unit, port, ITABRTr, count);
	break;
    case snmpIfOutQLen: {
	uint32	qcount;
	if (bcm_esw_port_queued_count_get(unit, port, &qcount) >= 0) {
	    COMPILER_64_ADD_32(count, qcount);
	}
    }
    break;
    case snmpIpInReceives:
	break;
    case snmpIpInHdrErrors:
	break;
    case snmpIpForwDatagrams:
	break;
    case snmpIpInDiscards:
	break;

	/* *** RFC 1493 *** */

    case snmpDot1dBasePortDelayExceededDiscards:
	break;
    case snmpDot1dBasePortMtuExceededDiscards:
	if (COUNT_OVR_ERRORS(unit)) {
	    REG_ADD(unit, port, IROVRr, count);     /* oversize pkts */
	    REG_ADD(unit, port, ITOVRr, count);     /* oversize pkts */
	}
	break;
    case snmpDot1dTpPortInFrames:
	REG_ADD(unit, port, IRPKTr, count);
	break;
    case snmpDot1dTpPortOutFrames:
	REG_ADD(unit, port, ITPKTr, count);
	break;
    case snmpDot1dPortInDiscards:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
	    REG_ADD(unit, port, RDISCr, count);
        } else {
	    REG_ADD(unit, port, IRDISCr, count);
        }
	break;

	/* *** RFC 1757 *** */

    case snmpEtherStatsDropEvents:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
	    REG_ADD(unit, port, RDISCr, count);
        } else {
	    REG_ADD(unit, port, IRDISCr, count);
	    REG_ADD(unit, port, ITAGEr, count);
        }
	/*
	 * SOC_IS_HERCULES1: add in IHOLD0-7, packets destined for each
	 * COSQ that were dropped due to lack of XQ resources
	 */
	break;
    case snmpEtherStatsOctets:
	REG_ADD(unit, port, IRBYTr, count);
	REG_ADD(unit, port, ITBYTr, count);
	break;
    case snmpEtherStatsPkts:
	REG_ADD(unit, port, IRPKTr, count);
	REG_ADD(unit, port, ITPKTr, count);
        REG_ADD(unit, port, IRUNDr, count); /* Runts */
        REG_ADD(unit, port, IRFRGr, count); /* Fragments */
	break;
    case snmpEtherStatsBroadcastPkts:	/* Broadcast packets (RX/TX) */
	REG_ADD(unit, port, IRBCAr, count); /* + rx broadcast  */
	REG_ADD(unit, port, ITBCAr, count); /* + tx broadcast */
	break;
    case snmpEtherStatsMulticastPkts:	/* Multicast packets (TX+RX) */
	REG_ADD(unit, port, IRMCAr, count); /* + rx multicast */
	REG_ADD(unit, port, ITMCAr, count); /* + tx multicast */
	break;
    case snmpEtherStatsCRCAlignErrors:
	REG_ADD(unit, port, IRFCSr, count);
	break;
    case snmpEtherStatsUndersizePkts:	/* Undersize frames */
	REG_ADD(unit, port, IRUNDr, count);
	break;
    case snmpEtherStatsOversizePkts:
	if (COUNT_OVR_ERRORS(unit)) {
	    REG_ADD(unit, port, IROVRr, count);
	    REG_ADD(unit, port, ITOVRr, count);
	}
	break;
    case snmpEtherRxOversizePkts:
	if (COUNT_OVR_ERRORS(unit)) {
	    REG_ADD(unit, port, IROVRr, count);
	}
	break;
    case snmpEtherTxOversizePkts:
	if (COUNT_OVR_ERRORS(unit)) {
	    REG_ADD(unit, port, ITOVRr, count);
	}
	break;
    case snmpEtherStatsFragments:
	REG_ADD(unit, port, IRFRGr, count);
	break;
    case snmpEtherStatsJabbers:
	REG_ADD(unit, port, IRJBRr, count);
	break;
    case snmpEtherStatsCollisions:
	/* always 0 */
	break;
    case snmpEtherStatsPkts64Octets:
	if (SOC_IS_DRACO1(unit) || SOC_IS_HERCULES1(unit)) {
            REG_ADD(unit, port, ITR64r, count);
        } else {
            REG_ADD(unit, port, IR64r, count);
            REG_ADD(unit, port, IT64r, count);
        }
	break;
    case snmpEtherStatsPkts65to127Octets:
	if (SOC_IS_DRACO1(unit) || SOC_IS_HERCULES1(unit)) {
            REG_ADD(unit, port, ITR127r, count);
        } else {
            REG_ADD(unit, port, IR127r, count);
            REG_ADD(unit, port, IT127r, count);
        }
	break;
    case snmpEtherStatsPkts128to255Octets:
	if (SOC_IS_DRACO1(unit) || SOC_IS_HERCULES1(unit)) {
            REG_ADD(unit, port, ITR255r, count);
        } else {
            REG_ADD(unit, port, IR255r, count);
            REG_ADD(unit, port, IT255r, count);
        }
	break;
    case snmpEtherStatsPkts256to511Octets:
	if (SOC_IS_DRACO1(unit) || SOC_IS_HERCULES1(unit)) {
            REG_ADD(unit, port, ITR511r, count);
        } else {
            REG_ADD(unit, port, IR511r, count);
            REG_ADD(unit, port, IT511r, count);
        }
	break;
    case snmpEtherStatsPkts512to1023Octets:
	if (SOC_IS_DRACO1(unit) || SOC_IS_HERCULES1(unit)) {
            REG_ADD(unit, port, ITR1023r, count);
        } else {
            REG_ADD(unit, port, IR1023r, count);
            REG_ADD(unit, port, IT1023r, count);
        }
	break;
    case snmpEtherStatsPkts1024to1518Octets:
	if (SOC_IS_DRACO1(unit) || SOC_IS_HERCULES1(unit)) {
            REG_ADD(unit, port, ITR1522r, count);
        } else if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, IR1518r, count);
            REG_ADD(unit, port, IT1518r, count);
        } else {
            REG_ADD(unit, port, IR2047r, count);
            REG_ADD(unit, port, IT2047r, count);
        }
	break;

	/* *** not actually in rfc1757 *** */

    case snmpBcmEtherStatsPkts1519to1522Octets:
	    break;
    case snmpBcmEtherStatsPkts1522to2047Octets:
	if (!(SOC_IS_DRACO1(unit) || SOC_IS_HERCULES1(unit))) {
            REG_ADD(unit, port, IR2047r, count);
            REG_ADD(unit, port, IT2047r, count);
        }
	break;
    case snmpBcmEtherStatsPkts2048to4095Octets:
	if (!(SOC_IS_DRACO1(unit) || SOC_IS_HERCULES1(unit))) {
            REG_ADD(unit, port, IR4095r, count);
            REG_ADD(unit, port, IT4095r, count);
        }
	break;
    case snmpBcmEtherStatsPkts4095to9216Octets:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, IR9216r, count);
            REG_ADD(unit, port, IT9216r, count);
	} else if (!(SOC_IS_DRACO1(unit) || SOC_IS_HERCULES1(unit))) {
            REG_ADD(unit, port, IR8191r, count);
            REG_ADD(unit, port, IT8191r, count);
            REG_ADD(unit, port, IR16383r, count);
            REG_ADD(unit, port, IT16383r, count);
        }
	break;

    case snmpBcmReceivedPkts64Octets:
        REG_ADD(unit, port, IR64r, count);
        break;
    case snmpBcmReceivedPkts65to127Octets:
        REG_ADD(unit, port, IR127r, count);
        break;
    case snmpBcmReceivedPkts128to255Octets:
        REG_ADD(unit, port, IR255r, count);
        break;
    case snmpBcmReceivedPkts256to511Octets:
        REG_ADD(unit, port, IR511r, count);
        break;
    case snmpBcmReceivedPkts512to1023Octets:
        REG_ADD(unit, port, IR1023r, count);
        break;
    case snmpBcmReceivedPkts1024to1518Octets:
        REG_ADD(unit, port, IR1518r, count);
        break;
    case snmpBcmReceivedPkts1519to2047Octets:
        REG_ADD(unit, port, IR2047r, count);
        break;
    case snmpBcmReceivedPkts2048to4095Octets:
        REG_ADD(unit, port, IR4095r, count);
        break;
    case snmpBcmReceivedPkts4095to9216Octets:
        REG_ADD(unit, port, IR9216r, count);
        break;

    case snmpBcmTransmittedPkts64Octets:
        REG_ADD(unit, port, IT64r, count);
        break;
    case snmpBcmTransmittedPkts65to127Octets:
        REG_ADD(unit, port, IT127r, count);
        break;
    case snmpBcmTransmittedPkts128to255Octets:
        REG_ADD(unit, port, IT255r, count);
        break;
    case snmpBcmTransmittedPkts256to511Octets:
        REG_ADD(unit, port, IT511r, count);
        break;
    case snmpBcmTransmittedPkts512to1023Octets:
        REG_ADD(unit, port, IT1023r, count);
        break;
    case snmpBcmTransmittedPkts1024to1518Octets:
        REG_ADD(unit, port, IT1518r, count);
        break;
    case snmpBcmTransmittedPkts1519to2047Octets:
        REG_ADD(unit, port, IT2047r, count);
        break;
    case snmpBcmTransmittedPkts2048to4095Octets:
        REG_ADD(unit, port, IT4095r, count);
        break;
    case snmpBcmTransmittedPkts4095to9216Octets:
        REG_ADD(unit, port, IT9216r, count);
        break;

    case snmpEtherStatsTXNoErrors:
	REG_ADD(unit, port, ITPKTr, count);
	REG_SUB(unit, port, ITFRGr, count);
	if (COUNT_OVR_ERRORS(unit)) {
	    REG_SUB(unit, port, ITOVRr, count);
	}
	REG_SUB(unit, port, ITUFLr, count);
	REG_SUB(unit, port, ITERRr, count);
	break;
    case snmpEtherStatsRXNoErrors: /* RPKT - ( RFCS + RXUO + RFLR) */
	REG_ADD(unit, port, IRPKTr, count);
	REG_SUB(unit, port, IRFCSr, count);
	REG_SUB(unit, port, IRJBRr, count);
	if (COUNT_OVR_ERRORS(unit)) {
	    REG_SUB(unit, port, IROVRr, count);
	}
	REG_SUB(unit, port, IRUNDr, count);
	REG_SUB(unit, port, IRFRGr, count);
	REG_SUB(unit, port, IRERPKTr, count);
	break;

	/* *** RFC 2665 *** */

    case snmpDot3StatsAlignmentErrors:
	break;
    case snmpDot3StatsFCSErrors:
	REG_ADD(unit, port, IRFCSr, count);
	break;
    case snmpDot3StatsSingleCollisionFrames:
	break;
    case snmpDot3StatsMultipleCollisionFrames:
	break;
    case snmpDot3StatsSQETTestErrors:
	/* always 0 */
	break;
    case snmpDot3StatsDeferredTransmissions:
	break;
    case snmpDot3StatsLateCollisions:
	break;
    case snmpDot3StatsExcessiveCollisions:
	break;
    case snmpDot3StatsInternalMacTransmitErrors:
	REG_ADD(unit, port, ITUFLr, count);
	REG_ADD(unit, port, ITERRr, count);
	break;
    case snmpDot3StatsCarrierSenseErrors:
	break;
    case snmpDot3StatsFrameTooLongs:
	    if (SOC_REG_IS_VALID(unit, IRMEGr) && SOC_REG_IS_VALID(unit, IRMEBr)) {
             REG_ADD(unit, port, IRMEGr, count);
             REG_ADD(unit, port, IRMEBr, count);
        } else {
             REG_ADD(unit, port, IRJBRr, count);
        }
    break;
    case snmpDot3StatsInternalMacReceiveErrors:
	break;
    case snmpDot3StatsSymbolErrors:
	REG_ADD(unit, port, IRERBYTr, count);
	break;
    case snmpDot3ControlInUnknownOpcodes:
	REG_ADD(unit, port, IRXUOr, count);
	break;
    case snmpDot3InPauseFrames:
	REG_ADD(unit, port, IRXPFr, count);
	break;
    case snmpDot3OutPauseFrames:
	REG_ADD(unit, port, ITXPFr, count);
	break;

	/* *** RFC 2233 high capacity versions of RFC1213 objects *** */

    case snmpIfHCInOctets:
	REG_ADD(unit, port, IRBYTr, count);
	break;
    case snmpIfHCInUcastPkts:
	REG_ADD(unit, port, IRPKTr, count);
	REG_SUB(unit, port, IRMCAr, count);
	REG_SUB(unit, port, IRBCAr, count);
        REG_SUB(unit, port, IRFCSr, count); /* - bad FCS */
        REG_SUB(unit, port, IRXCFr, count); /* - good FCS, all MAC ctrl */
        REG_SUB(unit, port, IRFLRr, count); /* - good FCS, bad length */
        REG_SUB(unit, port, IRJBRr, count); /* - oversize, bad FCS */
        if (COUNT_OVR_ERRORS(unit)) {
            REG_SUB(unit, port, IROVRr, count); /* - oversize, good FCS */
        }
	break;
    case snmpIfHCInMulticastPkts:
	REG_ADD(unit, port, IRMCAr, count);
	break;
    case snmpIfHCInBroadcastPkts:
	REG_ADD(unit, port, IRBCAr, count);
	break;
    case snmpIfHCOutOctets:
	REG_ADD(unit, port, ITBYTr, count);
	break;
    case snmpIfHCOutUcastPkts:
	REG_ADD(unit, port, ITPKTr, count);
	REG_SUB(unit, port, ITMCAr, count);
	REG_SUB(unit, port, ITBCAr, count);
	break;
    case snmpIfHCOutMulticastPkts:
	REG_ADD(unit, port, ITMCAr, count);
	break;
    case snmpIfHCOutBroadcastPckts:
	REG_ADD(unit, port, ITBCAr, count);
	break;

        /* *** RFC 2465 *** */

    case snmpIpv6IfStatsInReceives:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, RIPC6r, count);
            REG_ADD(unit, port, IMRP6r, count);
        }
        break;
    case snmpIpv6IfStatsInHdrErrors:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, RIPD6r, count);
        }
        break;
    case snmpIpv6IfStatsInAddrErrors:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, RIPHE6r, count);
        }
        break;
    case snmpIpv6IfStatsInDiscards:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
             REG_ADD(unit, port, RIPD6r, count);
        }
        break;
    case snmpIpv6IfStatsOutForwDatagrams:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, TDBGC0r, count);
        }
        break;
    case snmpIpv6IfStatsOutDiscards:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, TDBGC1r, count);
        }
        break;
    case snmpIpv6IfStatsInMcastPkts:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
             REG_ADD(unit, port, IMRP6r, count);
        }
        break;
    case snmpIpv6IfStatsOutMcastPkts:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, TDBGC2r, count);
        }
        break;

        /* *** IEEE 802.1bb *** */
    case snmpIeee8021PfcRequests:
        REG_ADD(unit, port, ITXPPr, count);
        break;
    case snmpIeee8021PfcIndications:
        REG_ADD(unit, port, IRXPPr, count);
        break;

	/* IPMC counters (broadcom specific) */

    case snmpBcmIPMCBridgedPckts:
	break;
    case snmpBcmIPMCRoutedPckts:
	break;
    case snmpBcmIPMCInDroppedPckts:
	break;
    case snmpBcmIPMCOutDroppedPckts:
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
		     "_bcm_stat_hg_get: Statistic not supported: %d\n", type);
	return BCM_E_PARAM;
    }

    *val = count;

    return BCM_E_NONE;
}
