/*
 * $Id: stat_ge.c 1.59.44.1 Broadcom SDK $
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
 *	_bcm_stat_ge_get
 * Description:
 *	Get the specified statistic for a GE port in 1000 Mb mode on the
 *	StrataSwitch family of devices.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - zero-based port number
 *	type - SNMP statistics type (see bcm/stat.h)
 *	val  - (OUT) 64 bit statistic counter value
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_PARAM - Illegal parameter.
 *	BCM_E_INTERNAL - Chip access failure.
 */
int
_bcm_stat_ge_get(int unit, bcm_port_t port, bcm_stat_val_t type, uint64 *val,
                 int incl_non_ge_stat)
{
    uint64 count, count1;
    REG_MATH_DECL;       /* Required for use of the REG_* macros */

    COMPILER_REFERENCE(&count);  /* Work around PPC compiler bug */
    COMPILER_64_ZERO(count);

    switch (type) {
	/* *** RFC 1213 *** */

    case snmpIfInOctets:
	REG_ADD(unit, port, GRBYTr, count);	/* bytes rcvd */
        REG_ADD(unit, port, RRBYTr, count); /* Runt bytes */
	break;
    case snmpIfInUcastPkts:	/* Unicast packets received */
        if (soc_feature(unit, soc_feature_hw_stats_calc)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RUCr, count);  /* unicast pkts rcvd */
            }
    	} else if (SOC_IS_DRACO(unit)) {
    	    REG_ADD(unit, port, GRUCr, count);	/* unicast pkts rcvd */
    	} else {
    	    REG_ADD(unit, port, GRPKTr, count);	/* all pkts rcvd */
    	    REG_SUB(unit, port, GRMCAr, count);	/* - multicast */
    	    REG_SUB(unit, port, GRBCAr, count);	/* - broadcast */
            REG_SUB(unit, port, GRALNr, count); /* - bad FCS, dribble bit  */
            REG_SUB(unit, port, GRFCSr, count); /* - bad FCS, no dribble bit */
            REG_SUB(unit, port, GRFLRr, count); /* - good FCS, bad length */
            REG_SUB(unit, port, GRJBRr, count); /* - oversize, bad FCS */
            if (SOC_REG_IS_VALID(unit, GRMTUEr)) {
                REG_SUB(unit, port, GRMTUEr, count); /* - mtu exceeded, good FCS */
                if (SOC_REG_IS_VALID(unit, GROVRr)) {
                   REG_SUB(unit, port, GROVRr, count);
                }

            } else if (COUNT_OVR_ERRORS(unit)) {
                REG_SUB(unit, port, GROVRr, count); /* - oversize, good FCS */
            }
    	}
	break;
    case snmpIfInNUcastPkts:	/* Non Unicast packets received */
	REG_ADD(unit, port, GRMCAr, count);	/* multicast pkts rcvd */
	REG_ADD(unit, port, GRBCAr, count);	/* broadcast pkts rcvd */
	break;
    case snmpIfInBroadcastPkts:	/* Broadcast packets received */
	REG_ADD(unit, port, GRBCAr, count);	/* broadcast pkts rcvd */
	break;
    case snmpIfInMulticastPkts:	/* Multicast packets received */
	REG_ADD(unit, port, GRMCAr, count);	/* multicast pkts rcvd */
	break;
    case snmpIfInDiscards:	/* Dropped packets including aborted */
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RDBGC0r, count); /* Ingress drop conditions */
                BCM_IF_ERROR_RETURN
                    (_bcm_stat_counter_non_dma_extra_get(unit,
                                   SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING,
                                                         port, &count1));
                COMPILER_64_ADD_64(count, count1);
            }
        } else {
            REG_ADD(unit, port, GRDISCr, count);
            if (SOC_IS_DRACO(unit)) {
                REG_ADD(unit, port, GRDROPr, count);
            }
            REG_ADD(unit, port, GRIPDr, count);
        }
	break;
    case snmpIfInErrors:	/* Receive packets containing errors */
    	REG_ADD(unit, port, GRUNDr, count);	/* undersize pkts, good FCS */
    	REG_ADD(unit, port, GRFRGr, count);	/* undersize pkts, bad FCS */
    	REG_ADD(unit, port, GRFCSr, count);	/* FCS errors */
        if (SOC_REG_IS_VALID(unit, GRMTUEr)) {
            REG_ADD(unit, port, GRMTUEr, count); /* mtu exceeded pkts, good FCS */
        } else if (COUNT_OVR_ERRORS(unit)) {
            REG_ADD(unit, port, GROVRr, count);	/* oversize pkts, good FCS */
        }
    	REG_ADD(unit, port, GRJBRr, count);	/* oversize pkts, bad FCS */
	break;
    case snmpIfInUnknownProtos:
	break;
    case snmpIfOutOctets:
        REG_ADD(unit, port, GTBYTr, count);	/* transmit bytes */
	break;
    case snmpIfOutUcastPkts:
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        if (soc_feature(unit, soc_feature_hw_stats_calc)) {
            REG_ADD(unit,port, GTUCr, count);  /* unicast pkts sent */
        } else 
#endif /* BCM_RAPTOR_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        {
            REG_ADD(unit, port, GTPKTr, count);	/* all pkts xmited */
            REG_SUB(unit, port, GTMCAr, count);	/* - multicast */
            REG_SUB(unit, port, GTBCAr, count);	/* - broadcast */
            REG_SUB(unit, port, GTFCSr, count); /* - bad FCS */
            REG_SUB(unit, port, GTJBRr, count); /* - oversize, bad FCS */
            if (COUNT_OVR_ERRORS(unit)) {
                REG_SUB(unit, port, GTOVRr, count); /* - oversize, good FCS */
            }
        }
	break;
    case snmpIfOutNUcastPkts:
    	REG_ADD(unit, port, GTMCAr, count);	/* multicast pkts */
    	REG_ADD(unit, port, GTBCAr, count);	/* broadcast pkts */
	break;
    case snmpIfOutBroadcastPkts:
    	REG_ADD(unit, port, GTBCAr, count);	/* broadcast pkts */
	break;
    case snmpIfOutMulticastPkts:
    	REG_ADD(unit, port, GTMCAr, count);	/* multicast pkts */
	break;
    case snmpIfOutDiscards:
    	REG_ADD(unit, port, GTEDFr, count);	/* multiple deferral */
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, HOLDr, count);	/* L2 MTU drops */
                REG_ADD(unit, port, TDBGC3r, count);
                BCM_IF_ERROR_RETURN
                    (_bcm_stat_counter_extra_get(unit, EGRDROPPKTCOUNTr,
                                                 port, &count1));
                COMPILER_64_ADD_64(count, count1);
                BCM_IF_ERROR_RETURN
                    (_bcm_stat_counter_non_dma_extra_get(unit,
                                   SOC_COUNTER_NON_DMA_COSQ_DROP_PKT,
                                                         port, &count1));
                COMPILER_64_ADD_64(count, count1);
            }
        } else {
            REG_ADD(unit, port, GTAGEr, count);	/* aged out in MMU */
        }
    	/* other causes of discards? */
	break;
    case snmpIfOutErrors:
    	REG_ADD(unit, port, GTXCLr, count);	/* excessive collisions */
    	REG_ADD(unit, port, GTABRTr, count);	/* aborted */
	break;
    case snmpIfOutQLen: {
    	uint32	qcount;
        if (incl_non_ge_stat) {
            if (bcm_esw_port_queued_count_get(unit, port, &qcount) >= 0) { 
                COMPILER_64_ADD_32(count, qcount);
            }
        }
    }
    break;
    case snmpIpInReceives:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RIPC4r, count);
            }
        } else {
	    REG_ADD(unit, port, GRIPCr, count);
        }
	break;
    case snmpIpInHdrErrors:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RIPD4r, count);
            }
        } else {
            if (SOC_IS_DRACO(unit)) {
                REG_ADD(unit, port, GRIPHEr, count);
            }
        }
	break;
/* ipInAddrErrors */
    case snmpIpForwDatagrams:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, TDBGC4r, count);
            }
        } else {
            REG_ADD(unit, port, GTIPr, count);
        }
	break;
/* ipInUnknownProtos */
    case snmpIpInDiscards:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                 REG_ADD(unit, port, RIPHE4r, count);
                 REG_ADD(unit, port, RIPD4r, count);
            }
        } else {
            if (SOC_IS_DRACO(unit)) {
                REG_ADD(unit, port, GRIPHEr, count);
            }

	        REG_ADD(unit, port, GRIPDr, count);
        }
	break;
/* ipInDelivers */
/* ipOutRequests */
/* ipOutDiscards */
/* ipOutNoRoutes */

	/* *** RFC 1493 *** */

    case snmpDot1dBasePortDelayExceededDiscards:
        REG_ADD(unit, port, GTEDFr, count);	/* multiple deferral */
	break;
    case snmpDot1dBasePortMtuExceededDiscards:
        if (SOC_REG_IS_VALID(unit, GRMTUEr)) {
            REG_ADD(unit, port, GRMTUEr, count); /* mtu exceeded pkts */
        } else if (COUNT_OVR_ERRORS(unit)) {
            REG_ADD(unit, port, GROVRr, count);	/* oversize pkts */
        }
        if (COUNT_OVR_ERRORS(unit)) {
            REG_ADD(unit, port, GTOVRr, count);	/* oversize pkts */
        }
	break;
    case snmpDot1dTpPortInFrames:	/* should be only bridge mgmt */
        REG_ADD(unit, port, GRPKTr, count);
	break;
    case snmpDot1dTpPortOutFrames:	/* should be only bridge mgmt */
        REG_ADD(unit, port, GTPKTr, count);
	break;
    case snmpDot1dPortInDiscards:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RDISCr, count); 
                REG_ADD(unit, port, RIPD4r, count); 
                REG_ADD(unit, port, RIPD6r, count); 
                REG_ADD(unit, port, RPORTDr, count);
            }
        } else {
            REG_ADD(unit, port, GRDISCr, count);
            if (SOC_IS_DRACO(unit)) {
                REG_ADD(unit, port, GRDROPr, count);
            }
            REG_ADD(unit, port, GRIPDr, count);
        }
	break;

	/* *** RFC 1757 *** */

    case snmpEtherStatsDropEvents:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RDISCr, count); 
            }
        } else {
            REG_ADD(unit, port, GRDISCr, count);
        }
	break;
    case snmpEtherStatsOctets:
	REG_ADD(unit, port, GRBYTr, count);
	REG_ADD(unit, port, GTBYTr, count);
        REG_ADD(unit, port, RRBYTr, count); /* Runt bytes */
	break;
    case snmpEtherStatsPkts:
	REG_ADD(unit, port, GRPKTr, count);
	REG_ADD(unit, port, GTPKTr, count);
        REG_ADD(unit, port, RRPKTr, count); /* Runt packets */
	break;
    case snmpEtherStatsBroadcastPkts:
	REG_ADD(unit, port, GRBCAr, count);
	REG_ADD(unit, port, GTBCAr, count);
	break;
    case snmpEtherStatsMulticastPkts:
	REG_ADD(unit, port, GRMCAr, count);
	REG_ADD(unit, port, GTMCAr, count);
	break;
    case snmpEtherStatsCRCAlignErrors:	/* CRC errors + alignment errors */
	REG_ADD(unit, port, GRFCSr, count);
	break;
    case snmpEtherStatsUndersizePkts:	/* Undersize frames */
	REG_ADD(unit, port, GRUNDr, count);
	break;
    case snmpEtherStatsOversizePkts:
	if (COUNT_OVR_ERRORS(unit)) {
	    if (soc_feature(unit, soc_feature_stat_jumbo_adj)) {
		REG_ADD(unit, port, GROVRr, count);
		REG_ADD(unit, port, GTOVRr, count);
	    } else {
		REG_ADD(unit, port, GRJBRr, count);
                REG_ADD(unit, port, GTJBRr, count);
	    }
	}
	break;
    case snmpEtherRxOversizePkts:
	if (COUNT_OVR_ERRORS(unit)) {
	    if (soc_feature(unit, soc_feature_stat_jumbo_adj)) {
		REG_ADD(unit, port, GROVRr, count);
	    }
	}
	break;
    case snmpEtherTxOversizePkts:
	if (COUNT_OVR_ERRORS(unit)) {
	    if (soc_feature(unit, soc_feature_stat_jumbo_adj)) {
		REG_ADD(unit, port, GTOVRr, count);
	    }
	}
	break;
    case snmpEtherStatsFragments:
	REG_ADD(unit, port, GRFRGr, count);
	REG_ADD(unit, port, GTFRGr, count);
	break;
    case snmpEtherStatsJabbers:
	REG_ADD(unit, port, GRJBRr, count);
	REG_ADD(unit, port, GTJBRr, count);
	break;
    case snmpEtherStatsCollisions:
	REG_ADD(unit, port, GTNCLr, count);
	break;
    case snmpEtherStatsPkts64Octets:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {  
            REG_ADD(unit, port, GR64r, count);
            REG_ADD(unit, port, GT64r, count);
        } else {
	    REG_ADD(unit, port, GTR64r, count);
        }
	break;
    case snmpEtherStatsPkts65to127Octets:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, GR127r, count);
            REG_ADD(unit, port, GT127r, count);
        } else {
            REG_ADD(unit, port, GTR127r, count);
        }
	break;
    case snmpEtherStatsPkts128to255Octets:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, GR255r, count);
            REG_ADD(unit, port, GT255r, count);
        } else {
            REG_ADD(unit, port, GTR255r, count);
        }
	break;
    case snmpEtherStatsPkts256to511Octets:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, GR511r, count);
            REG_ADD(unit, port, GT511r, count);
        } else {
            REG_ADD(unit, port, GTR511r, count);
        }
	break;
    case snmpEtherStatsPkts512to1023Octets:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, GR1023r, count);
            REG_ADD(unit, port, GT1023r, count);
        } else {
            REG_ADD(unit, port, GTR1023r, count);
        }
	break;
    case snmpEtherStatsPkts1024to1518Octets:
        if (SOC_IS_DRACO(unit)) {
            REG_ADD(unit, port, GTR1518r, count);
        } else if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, GR1518r, count);
            REG_ADD(unit, port, GT1518r, count);
        }
	break;

	/* *** not actually in rfc1757 *** */

    case snmpBcmEtherStatsPkts1519to1522Octets:
	if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, GRMGVr, count);
            REG_ADD(unit, port, GTMGVr, count);
        } else { 
	    REG_ADD(unit, port, GTRMGVr, count);
        }
	break;
    case snmpBcmEtherStatsPkts1522to2047Octets:
        if (SOC_IS_DRACO(unit)) {
	    REG_ADD(unit, port, GTR2047r, count);
	} else if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, GR2047r, count);
            REG_ADD(unit, port, GT2047r, count);
	}
	break;
    case snmpBcmEtherStatsPkts2048to4095Octets:
        if (SOC_IS_DRACO(unit)) {
	    REG_ADD(unit, port, GTR4095r, count);
	} 
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, GR4095r, count);
            REG_ADD(unit, port, GT4095r, count);
	}
	break;
    case snmpBcmEtherStatsPkts4095to9216Octets:
        if (SOC_IS_DRACO(unit)) {
	    REG_ADD(unit, port, GTR9216r, count);
	} 
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            REG_ADD(unit, port, GR9216r, count);
            REG_ADD(unit, port, GT9216r, count);
	}
	break;

    case snmpBcmReceivedPkts64Octets:
        REG_ADD(unit, port, GR64r, count);
        break;
    case snmpBcmReceivedPkts65to127Octets:
        REG_ADD(unit, port, GR127r, count);
        break;
    case snmpBcmReceivedPkts128to255Octets:
        REG_ADD(unit, port, GR255r, count);
        break;
    case snmpBcmReceivedPkts256to511Octets:
        REG_ADD(unit, port, GR511r, count);
        break;
    case snmpBcmReceivedPkts512to1023Octets:
        REG_ADD(unit, port, GR1023r, count);
        break;
    case snmpBcmReceivedPkts1024to1518Octets:
        REG_ADD(unit, port, GR1518r, count);
        break;
    case snmpBcmReceivedPkts1519to2047Octets:
        REG_ADD(unit, port, GR2047r, count);
        break;
    case snmpBcmReceivedPkts2048to4095Octets:
        REG_ADD(unit, port, GR4095r, count);
        break;
    case snmpBcmReceivedPkts4095to9216Octets:
        REG_ADD(unit, port, GR9216r, count);
        break;

    case snmpBcmTransmittedPkts64Octets:
        REG_ADD(unit, port, GT64r, count);
        break;
    case snmpBcmTransmittedPkts65to127Octets:
        REG_ADD(unit, port, GT127r, count);
        break;
    case snmpBcmTransmittedPkts128to255Octets:
        REG_ADD(unit, port, GT255r, count);
        break;
    case snmpBcmTransmittedPkts256to511Octets:
        REG_ADD(unit, port, GT511r, count);
        break;
    case snmpBcmTransmittedPkts512to1023Octets:
        REG_ADD(unit, port, GT1023r, count);
        break;
    case snmpBcmTransmittedPkts1024to1518Octets:
        REG_ADD(unit, port, GT1518r, count);
        break;
    case snmpBcmTransmittedPkts1519to2047Octets:
        REG_ADD(unit, port, GT2047r, count);
        break;
    case snmpBcmTransmittedPkts2048to4095Octets:
        REG_ADD(unit, port, GT4095r, count);
        break;
    case snmpBcmTransmittedPkts4095to9216Octets:
        REG_ADD(unit, port, GT9216r, count);
        break;

    case snmpEtherStatsTXNoErrors:
	/* FE = TPKT - (TNCL + TOVR + TFRG + TUND) */
	/* GE = GTPKT - (GTOVR + GTIPD + GTABRT) */
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        if (soc_feature(unit, soc_feature_hw_stats_calc)) {
            REG_ADD(unit, port, GTPOKr, count); /* All good packets */
        } else 
#endif /* BCM_RAPTOR_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        {
            REG_ADD(unit, port, GTPKTr, count); /* All Packets */
            if (COUNT_OVR_ERRORS(unit)) {
                REG_SUB(unit, port, GTOVRr, count); /* Oversize */
            }
            if (!soc_feature(unit, soc_feature_stat_xgs3)) {
                REG_SUB(unit, port, GTIPDr, count); /* IP dropped */
                REG_SUB(unit, port, GTABRTr, count);/* Aborted */
            }
        }
	break;
    case snmpEtherStatsRXNoErrors:
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        /* Some chips have a dedicated register for this stat */
        if (soc_feature(unit, soc_feature_hw_stats_calc)) {
            REG_ADD(unit, port, GRPOKr, count);
        } else 
#endif /* BCM_RAPTOR_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        {
            /*
    	     * GE = GRPKT - (GRFCS + GRXUO + GRFLR + GRCDE + GRFCR +
    	     *                GRUND + GROVR + GRJBR + GRIPCHK/GRIPHE)
    	     */
    
            REG_ADD(unit, port, GRPKTr, count);
            REG_SUB(unit, port, GRFCSr, count);
            REG_SUB(unit, port, GRXUOr, count);
            REG_SUB(unit, port, GRFLRr, count);
    
            REG_SUB(unit, port, GRCDEr, count);
            REG_SUB(unit, port, GRFCRr, count);
            REG_SUB(unit, port, GRUNDr, count);
            if (SOC_REG_IS_VALID(unit, GRMTUEr)) {
                REG_SUB(unit, port, GRMTUEr, count); /* mtu exceeded pkts */
            } else if (COUNT_OVR_ERRORS(unit)) {
                REG_SUB(unit, port, GROVRr, count);	/* oversize pkts */
            }
            REG_SUB(unit, port, GRJBRr, count);
            if (SOC_IS_DRACO(unit)) {
                REG_SUB(unit, port, GRIPHEr, count);
            }
        }
        break;
	/* *** RFC 2665 *** */

    case snmpDot3StatsAlignmentErrors:
	REG_ADD(unit, port, GRALNr, count);
	break;
    case snmpDot3StatsFCSErrors:
	REG_ADD(unit, port, GRFCSr, count);
	break;
    case snmpDot3StatsSingleCollisionFrames:
	REG_ADD(unit, port, GTSCLr, count);
	break;
    case snmpDot3StatsMultipleCollisionFrames:
	REG_ADD(unit, port, GTMCLr, count);
	break;
    case snmpDot3StatsSQETTestErrors:
	/* always 0 */
	break;
    case snmpDot3StatsDeferredTransmissions:
	REG_ADD(unit, port, GTDFRr, count);
	break;
    case snmpDot3StatsLateCollisions:
	REG_ADD(unit, port, GTLCLr, count);
	break;
    case snmpDot3StatsExcessiveCollisions:
	REG_ADD(unit, port, GTXCLr, count);
	break;
    case snmpDot3StatsInternalMacTransmitErrors:
	/* always 0 */
	break;
    case snmpDot3StatsCarrierSenseErrors:
	REG_ADD(unit, port, GRFCRr, count);
	break;
    case snmpDot3StatsFrameTooLongs:
        if (soc_feature(unit, soc_feature_stat_jumbo_adj)) {
            if (SOC_REG_IS_VALID(unit, GRMTUEr)) {
                REG_ADD(unit, port, GRMTUEr, count); /* mtu exceeded pkts */
            } else {
                REG_ADD(unit, port, GROVRr, count); /* oversized pkts */
            }
        } else {
            REG_ADD(unit, port, GRJBRr, count);
        }
	break;
    case snmpDot3StatsInternalMacReceiveErrors:
	/* always 0 */
	break;
    case snmpDot3StatsSymbolErrors:
	REG_ADD(unit, port, GRCDEr, count);
	break;
    case snmpDot3ControlInUnknownOpcodes:
	REG_ADD(unit, port, GRXUOr, count);
	break;
    case snmpDot3InPauseFrames:
	REG_ADD(unit, port, GRXPFr, count);
	break;
    case snmpDot3OutPauseFrames:
        if (incl_non_ge_stat) {
            REG_ADD(unit, port, GTXPFr, count);
        } /* Else avoid double-counting of pause frames on GXMACs */
	break;

	/* *** RFC 2233 high capacity versions of RFC1213 objects *** */

    case snmpIfHCInOctets:
	REG_ADD(unit, port, GRBYTr, count);	/* bytes rcvd */
        REG_ADD(unit, port, RRBYTr, count); /* Runt bytes */
	break;
    case snmpIfHCInUcastPkts:
    	if (soc_feature(unit, soc_feature_hw_stats_calc)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RUCr, count);  /* unicast pkts rcvd */
            }
	} else if (SOC_IS_DRACO(unit)) {
	    REG_ADD(unit, port, GRUCr, count);	/* unicast pkts rcvd */
    	} else {
	    REG_ADD(unit, port, GRPKTr, count);	/* all pkts rcvd */
	    REG_SUB(unit, port, GRMCAr, count);	/* - multicast */
	    REG_SUB(unit, port, GRBCAr, count);	/* - broadcast */
            REG_SUB(unit, port, GRALNr, count); /* - bad FCS, dribble bit  */
            REG_SUB(unit, port, GRFCSr, count); /* - bad FCS, no dribble bit */
            REG_SUB(unit, port, GRFLRr, count); /* - good FCS, bad length */
            REG_SUB(unit, port, GRJBRr, count); /* - oversize, bad FCS */
            if (SOC_REG_IS_VALID(unit, GRMTUEr)) {
                REG_SUB(unit, port, GRMTUEr, count); /* mtu exceeded pkts */
            } else if (COUNT_OVR_ERRORS(unit)) {
                REG_SUB(unit, port, GROVRr, count);	/* oversize pkts */
            }
	}
	break;
    case snmpIfHCInMulticastPkts:
	REG_ADD(unit, port, GRMCAr, count);	/* multicast */
	break;
    case snmpIfHCInBroadcastPkts:
	REG_ADD(unit, port, GRBCAr, count);	/* broadcast */
	break;
    case snmpIfHCOutOctets:
	REG_ADD(unit, port, GTBYTr, count);	/* transmit bytes */
	break;
    case snmpIfHCOutUcastPkts:
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        if (soc_feature(unit, soc_feature_hw_stats_calc)) {
            REG_ADD(unit, port, GTUCr, count); /* All good packets */
        } else 
#endif /* BCM_RAPTOR_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        { 
            REG_ADD(unit, port, GTPKTr, count);	/* all pkts xmited */
            REG_SUB(unit, port, GTMCAr, count);	/* - multicast */
            REG_SUB(unit, port, GTBCAr, count);	/* - broadcast */
            REG_SUB(unit, port, GTFCSr, count); /* - bad FCS */
            REG_SUB(unit, port, GTJBRr, count); /* - oversize, bad FCS */
            if (COUNT_OVR_ERRORS(unit)) {
                REG_SUB(unit, port, GTOVRr, count); /* - oversize, good FCS */
            }
        }
	break;
    case snmpIfHCOutMulticastPkts:
	REG_ADD(unit, port, GTMCAr, count);	/* multicast xmited */
	break;
    case snmpIfHCOutBroadcastPckts:
	REG_ADD(unit, port, GTBCAr, count);	/* broadcast xmited */
	break;

	/* *** RFC 2465 *** */

    case snmpIpv6IfStatsInReceives:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RIPC6r, count);
	        REG_ADD(unit, port, IMRP6r, count);
            }
        }
	break;
    case snmpIpv6IfStatsInHdrErrors:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RIPD6r, count);
            }
        }
	break;
    case snmpIpv6IfStatsInAddrErrors:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RIPHE6r, count);
            }
        }
	break;
    case snmpIpv6IfStatsInDiscards:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, RIPHE6r, count);
                REG_ADD(unit, port, RIPD6r, count);
            }
        }
	break;
    case snmpIpv6IfStatsOutForwDatagrams:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, TDBGC0r, count);
            }
        }
	break;
    case snmpIpv6IfStatsOutDiscards:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, TDBGC1r, count);
            }
        }
	break;
    case snmpIpv6IfStatsInMcastPkts:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
                REG_ADD(unit, port, IMRP6r, count);
            }
        }
	break;
    case snmpIpv6IfStatsOutMcastPkts:
        if (soc_feature(unit, soc_feature_stat_xgs3)) {
            if (incl_non_ge_stat) {
	        REG_ADD(unit, port, TDBGC2r, count);
            }
        }
	break;

        /* *** IEEE 802.1bb *** */
    case snmpIeee8021PfcRequests:
        REG_ADD(unit, port, GTXPPr, count);
        break;
    case snmpIeee8021PfcIndications:
        REG_ADD(unit, port, GRXPPr, count);
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
	if (soc_feature(unit, soc_feature_ip_mcast)) {
            if (soc_feature(unit, soc_feature_stat_xgs3)) {
                if (incl_non_ge_stat) {
	            REG_ADD(unit, port, RDBGC1r, count);
                }
            } else {
	        REG_ADD(unit, port, GIMBPr, count);
            }
	}
	break;
    case snmpBcmIPMCRoutedPckts:
	if (soc_feature(unit, soc_feature_ip_mcast)) {
            if (soc_feature(unit, soc_feature_stat_xgs3)) {
                if (incl_non_ge_stat) {
	            REG_ADD(unit, port, IMRP4r, count);
                    REG_ADD(unit, port, IMRP6r, count);
                }
            } else {
	        REG_ADD(unit, port, GIMRPr, count);
            }
	}
	break;
    case snmpBcmIPMCInDroppedPckts:
	if (soc_feature(unit, soc_feature_ip_mcast)) {
            if (soc_feature(unit, soc_feature_stat_xgs3)) {
                if (incl_non_ge_stat) {
                    REG_ADD(unit, port, RDBGC2r, count);
                }
            } else {
	        REG_ADD(unit, port, GRIMDRr, count);
            }
	}
	break;
    case snmpBcmIPMCOutDroppedPckts:
	if (soc_feature(unit, soc_feature_ip_mcast)) {
            if (soc_feature(unit, soc_feature_stat_xgs3)) {
                if (incl_non_ge_stat) {
	            REG_ADD(unit, port, TDBGC5r, count);
                }
            } else {
	        REG_ADD(unit, port, GTIMDRr, count);
            }
	}
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
		     "bcm_stat_get: Statistic not supported: %d\n", type);
	return BCM_E_PARAM;
    }

    *val = count;

    return BCM_E_NONE;
}
