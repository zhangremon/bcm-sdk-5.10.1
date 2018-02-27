/*
 * $Id: rx.c 1.174.6.3 Broadcom SDK $
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
 * File:        rx.c
 * Purpose:     Receive packet mechanism
 * Requires:
 *
 * See sdk/doc/txrx.txt and pkt.txt for
 * information on the RX API and implementation.
 *
 * Quick overview:
 *
 *     Packet buffer allocation/deallocation is user configurable.
 *     This expects to be given monolithic (single block) buffers.
 *     When "HANDLED_OWNED" is returned by a handler, that means
 *     that the data buffer is stolen, not the packet structure.
 *
 *     Callback functions may be registered in interrupt or non-
 *     interrupt mode.  Non-interrupt is preferred.
 *
 *     Interrupt load is limited by setting overall rate limit
 *     (bcm_rx_rate_burst_set/get).
 *
 *     If a packet is not serviced in interrupt mode, it is queued
 *     based on its COS.
 *
 *     Each queue has a rate limit (bcm_rx_cos_rate_set/get) which
 *     controls the number of callbacks that will be made for the queue.
 *     The non-interrupt thread services these queues from highest to
 *     lowest and will discard packets in the queue when they exceed
 *     the queue's rate limit.
 *
 *     Packets handled at interrupt level are still accounted for in
 *     the COS rate limiting.
 *
 *     A channel is:
 *          Physically:  A separate hardware DMA process
 *          Logically:  A collection of COS bundled together.
 *     Rate limiting per channel is no longer supported (replaced
 *     by COS queue rate limiting).
 *
 *     Channels may be enabled and disabled separately from starting RX
 *     running.  However, stopping RX disables all channels.
 *
 *     Packets are started in groups called "chains", each of which
 *     is controlled by a "DV" (DMA-descriptor vector).
 *
 *     Updates to the handler linked list need to be synchronized
 *     both with thread packet processing (mutex) and interrupt
 *     packet processing (spl).
 *
 *     If no real callouts are registered (other than internal discard)
 *     don't bother starting DVs, nor queuing input pkts into cos queues.
 */

/* We need to call top-level APIs on other units */
#ifdef BCM_HIDE_DISPATCHABLE
#undef BCM_HIDE_DISPATCHABLE
#endif

#include <shared/alloc.h>

#include <soc/drv.h>
#include <soc/higig.h>

#include <bcm/rx.h>
#include <bcm_int/common/rx.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/rx.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/control.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/triumph2.h>
#include <bcm_int/esw/trident.h>

#include <bcm_int/api_xlate_port.h>

#if defined(BROADCOM_DEBUG)
#include <soc/debug.h>
#include <soc/cm.h>
#define RX_DEBUG(stuff)        soc_cm_debug stuff
#define RX_PRINT(stuff)        soc_cm_print stuff

#if 0
#define RX_VERY_VERBOSE(stuff) soc_cm_debug stuff
#else
#define RX_VERY_VERBOSE(stuff)
#endif

#else
#define RX_DEBUG(stuff)
#define RX_VERY_VERBOSE(stuff)
#endif  /* defined(BROADCOM_DEBUG) */

#include <bcm_int/rpc/rlink.h>

/*
 * Function:
 *      bcm_esw_rx_sched_register
 * Purpose:
 *      Rx scheduler registration function. 
 * Parameters:
 *      unit       - (IN) Unused. 
 *      sched_cb   - (IN) Rx scheduler routine.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_sched_register(int unit, bcm_rx_sched_cb sched_cb)
{
    return (_bcm_common_rx_sched_register(unit, sched_cb));
}

/*
 * Function:
 *      bcm_esw_rx_sched_unregister
 * Purpose:
 *      Rx scheduler de-registration function. 
 * Parameters:
 *      unit  - (IN) Unused. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_sched_unregister(int unit)
{
    return (_bcm_common_rx_sched_unregister(unit));
}

/*
 * Function:
 *      bcm_esw_rx_unit_next_get
 * Purpose:
 *      Rx started units iteration routine.
 * Parameters:
 *      unit       - (IN)  BCM device number. 
 *      unit_next  - (OUT) Next attached unit with started rx.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_unit_next_get(int unit, int *unit_next)
{
    return (_bcm_common_rx_unit_next_get(unit, unit_next));
}

/*
 * Function:
 *      bcm_esw_rx_queue_max_get
 * Purpose:
 *      Get maximum cos queue number for the device.
 * Parameters:
 *      unit    - (IN) BCM device number. 
 *      cosq    - (OUT) Maximum queue priority.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_queue_max_get(int unit, bcm_cos_queue_t  *cosq)
{  
    return (_bcm_common_rx_queue_max_get(unit, cosq));
}

/*
 * Function:
 *      bcm_esw_rx_queue_packet_count_get
 * Purpose:
 *      Get number of packets awaiting processing in the specific device/queue.
 * Parameters:
 *      unit         - (IN) BCM device number. 
 *      cosq         - (IN) Queue priority.
 *      packet_count - (OUT) Number of packets awaiting processing. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_queue_packet_count_get(int unit, bcm_cos_queue_t cosq, int *packet_count)
{
    return (_bcm_common_rx_queue_packet_count_get(unit, cosq, packet_count));
}

/*
 * Function:
 *      bcm_esw_rx_queue_rate_limit_status_get
 * Purpose:
 *      Get number of packet that can be rx scheduled 
 *      until system hits queue rx rate limit. 
 * Parameters:
 *      unit           - (IN) BCM device number. 
 *      cosq           - (IN) Queue priority.
 *      packet_tokens  - (OUT)Maximum number of packets that can be  scheduled.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_queue_rate_limit_status_get(int unit, bcm_cos_queue_t cosq, 
                                       int *packet_tokens)
{
    return (_bcm_common_rx_queue_rate_limit_status_get(unit, cosq, 
						       packet_tokens));
}


/*
 * Function:
 *      bcm_rx_init
 * Purpose:
 *      Software initialization for RX API
 * Parameters:
 *      unit - Unit to init
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Allocates rx control structure
 *      Copies default config into active config
 *      Adds discard handler
 */

int
bcm_esw_rx_init(int unit)
{
    return (_bcm_common_rx_init(unit));
}


/*
 * Function:
 *      bcm_rx_cfg_init
 * Purpose:
 *      Re-initialize the user level configuration
 * Parameters:
 *      unit - StrataXGS unit number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Can't use if currently running.  Should be called before
 *      doing a simple modification of the RX configuration in case
 *      the previous user has left it in a strange state.
 */

int
bcm_esw_rx_cfg_init(int unit)
{
    return (_bcm_common_rx_cfg_init(unit));
}

/*
 * Function:
 *      bcm_rx_start
 * Purpose:
 *      Initialize and configure the RX subsystem for a given unit
 * Parameters:
 *      unit - Unit to configure
 *      cfg - Configuration to use.  See include/bcm/rx.h
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Starts the packet receive thread if not already running.
 *      cfg may be null:  Use default config.
 *      alloc/free in cfg may be null:  Use default alloc/free functions
 */

int
bcm_esw_rx_start(int unit, bcm_rx_cfg_t *cfg)
{
    return (_bcm_common_rx_start(unit, cfg));
}

/*
 * Function:
 *      bcm_rx_stop
 * Purpose:
 *      Stop RX for the given unit; saves current configuration
 * Parameters:
 *      unit - The unit to stop
 *      cfg - OUT Configuration copied to this parameter
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This signals the thread to exit.
 */

int
bcm_esw_rx_stop(int unit, bcm_rx_cfg_t *cfg)
{
    return (_bcm_common_rx_stop(unit, cfg));
}


/*
 * Function:
 *      bcm_esw_rx_clear
 * Purpose:
 *      Clear all RX info
 * Returns:
 *      BCM_E_NONE
 */

int
bcm_esw_rx_clear(int unit)
{
    return (_bcm_common_rx_clear(unit));
}


int bcm_esw_rx_deinit(int unit)
{
    return _bcm_rx_shutdown(unit);
}

/*
 * Function:
 *      bcm_rx_cfg_get
 * Purpose:
 *      Check if init done; get the current RX configuration
 * Parameters:
 *      unit - Strata device ID
 *      cfg - OUT Configuration copied to this parameter.  May be NULL
 * Returns:
 *      BCM_E_INIT if not running on unit
 *      BCM_E_NONE if running on unit
 *      < 0 BCM_E_XXX error code
 * Notes:
 */

int
bcm_esw_rx_cfg_get(int unit, bcm_rx_cfg_t *cfg)
{
    return (_bcm_common_rx_cfg_get(unit, cfg));
}

/*
 * Function:
 *      bcm_rx_queue_register
 * Purpose:
 *      Register an application callback for the specified CPU queue
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      name - constant character string for debug purposes.
 *      cosq - CPU cos queue
 *      callback - callback function pointer.
 *      priority - priority of handler in list (0 is lowest priority).
 *      cookie - cookie passed to driver when packet arrives.
 *      flags - Register for interrupt or non-interrupt callback
 * Returns:
 *      BCM_E_NONE - callout registered.
 *      BCM_E_MEMORY - memory allocation failed.
 */

int
bcm_esw_rx_queue_register(int unit, const char *name, bcm_cos_queue_t cosq, 
                          bcm_rx_cb_f callback, uint8 priority, void *cookie, 
                          uint32 flags)
{
    return (_bcm_common_rx_queue_register(unit, name, cosq, callback,
                                       priority, cookie, flags));
}

/*
 * Function:
 *      bcm_rx_register
 * Purpose:
 *      Register an upper layer driver
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      chan - DMA channel number
 *      name - constant character string for debug purposes.
 *      priority - priority of handler in list (0 is lowest priority).
 *      f - function to call for that driver.
 *      cookie - cookie passed to driver when packet arrives.
 *      flags - Register for interrupt or non-interrupt callback
 * Returns:
 *      BCM_E_NONE - callout registered.
 *      BCM_E_MEMORY - memory allocation failed.
 * Notes:
 *      Refer bcm_rx_queue_register() if cosq is bigger than 16.
 */

int
bcm_esw_rx_register(int unit, const char *name, bcm_rx_cb_f callback,
                uint8 priority, void *cookie, uint32 flags)
{
    return (_bcm_common_rx_register(unit, name, callback,
                                    priority, cookie, flags));
}


/*
 * Function:
 *      bcm_rx_unregister
 * Purpose:
 *      De-register a callback function
 * Parameters:
 *      unit - Unit reference
 *      priority - Priority of registered callback
 *      callback - The function being registered
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Run through linked list looking for match of function and priority
 */

int
bcm_esw_rx_unregister(int unit, bcm_rx_cb_f callback, uint8 priority)
{
    return (_bcm_common_rx_unregister(unit, callback, priority));
}

/*
 * Function:
 *      bcm_rx_queue_unregister
 * Purpose:
 *      Unregister a callback function
 * Parameters:
 *      unit - Unit reference
 *      cosq - CPU cos queue
 *      priority - Priority of registered callback
 *      callback - The function being registered
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_rx_queue_unregister(int unit, bcm_cos_queue_t cosq,
                            bcm_rx_cb_f callback, uint8 priority)
{
    return (_bcm_common_rx_queue_unregister(unit, cosq, callback, priority));
}

/*
 * Function:
 *      bcm_rx_cosq_mapping_size_get
 * Purpose:
 *      Get number of COSQ mapping entries
 * Parameters:
 *      unit - Unit reference
 *      size - (OUT) number of entries
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_cosq_mapping_size_get(int unit, int *size)
{
    if (size == NULL) {
        return BCM_E_PARAM;
    }

    if (SOC_UNIT_VALID(unit) && SOC_MEM_IS_VALID(unit, CPU_COS_MAPm)) {
        *size = soc_mem_index_count(unit, CPU_COS_MAPm);
        return BCM_E_NONE;
    }

    *size = 0;
    return BCM_E_UNAVAIL;
}

#ifdef BCM_TRX_SUPPORT

/* CPU_COS_MAP_KEY format for TRX */
#define _bcm_trx_rx_cpu_cosmap_key_start                7
#define _bcm_trx_rx_cpu_cosmap_mask_start               7
#define _bcm_scorpion_rx_cpu_cosmap_key_max             39
#define _bcm_triumph_rx_cpu_cosmap_key_max              44
#define _bcm_triumph2_rx_cpu_cosmap_key_max             59
#define _bcm_trident_rx_cpu_cosmap_key_max              60
#define _bcm_katana_rx_cpu_cosmap_key_max               62
#define _bcm_enduro_rx_cpu_cosmap_key_max               50
#define _bcm_triumph_rx_cpu_max_cosq                    48
#define _bcm_scorpion_rx_cpu_max_cosq                   32
#define _bcm_hurricane_rx_cpu_max_cosq                   8



static bcm_rx_reason_t 
_bcm_trx_cpu_cos_map_key [] =
{
    bcmRxReasonBpdu,                    /* PROTOCOL_BPDU */
    bcmRxReasonIpmcReserved,            /* PROTOCOL_IPMC_RSVD */
    bcmRxReasonDhcp,                    /* PROTOCOL_DHCP */
    bcmRxReasonIgmp,                    /* PROTOCOL_IGMP */
    bcmRxReasonArp,                     /* PROTOCOL_ARP */
    bcmRxReasonUnknownVlan,             /* CPU_UVLAN. */
    bcmRxReasonSharedVlanMismatch,      /* PVLAN_MISMATCH */
    bcmRxReasonDosAttack,               /* CPU_DOS_ATTACK */
    bcmRxReasonParityError,             /* PARITY_ERROR */
    bcmRxReasonHigigControl,            /* MH_CONTROL */
    bcmRxReasonTtl1,                    /* TTL_1 */
    bcmRxReasonL3Slowpath,              /* IP_OPTIONS_PKT. */
    bcmRxReasonL2SourceMiss,            /* CPU_SLF */
    bcmRxReasonL2DestMiss,              /* CPU_DLF */
    bcmRxReasonL2Move,                  /* CPU_L2MOVE */
    bcmRxReasonL2Cpu,                   /* CPU_L2CPU */
    bcmRxReasonL2NonUnicastMiss,        /* PBT_NONUC_PKT */
    bcmRxReasonL3SourceMiss,            /* CPU_L3SRC_MISS. */
    bcmRxReasonL3DestMiss,              /* CPU_L3DST_MISS */
    bcmRxReasonL3SourceMove,            /* CPU_L3SRC_MOVE */
    bcmRxReasonMcastMiss,               /* CPU_MC_MISS */
    bcmRxReasonIpMcastMiss,             /* CPU_IPMC_MISS */
    bcmRxReasonL3HeaderError,           /* CPU_L3HDR_ERR */
    bcmRxReasonMartianAddr,             /* CPU_MARTIAN_ADDR */
    bcmRxReasonTunnelError,             /* CPU_TUNNEL_ERR */
    bcmRxReasonHigigHdrError,           /* HGHDR_ERROR */
    bcmRxReasonMcastIdxError,           /* MCIDX_ERROR */
    bcmRxReasonVlanFilterMatch,         /* VFP */
    bcmRxReasonClassBasedMove,          /* CBSM_PREVENTED */
    bcmRxReasonL2LearnLimit,            /* MAC_LIMIT */
    bcmRxReasonE2eHolIbp,               /* E2E_HOL_IBP */
    bcmRxReasonClassTagPackets,         /* HG_HDR_TYPE1 */
    bcmRxReasonNhop,                    /* NHOP */
    bcmRxReasonUrpfFail,                /* URPF_FAILED */
    bcmRxReasonFilterMatch,             /* CPU_FFP */
    bcmRxReasonIcmpRedirect,            /* ICMP_REDIRECT */
    bcmRxReasonSampleSource,            /* CPU_SFLOW_SRC */
    bcmRxReasonSampleDest,              /* CPU_SFLOW_DST */
    bcmRxReasonL3MtuFail,               /* L3_MTU_CHECK_FAIL */

    /* Below are not available for Scorpion */
    bcmRxReasonMplsLabelMiss,           /* MPLS_LABEL_MISS */
    bcmRxReasonMplsInvalidAction,       /* MPLS_INVALID_ACTION */
    bcmRxReasonMplsInvalidPayload,      /* MPLS_INVALID_PAYLOAD */
    bcmRxReasonMplsTtl,                 /* MPLS_TTL_CHECK_FAIL */
    bcmRxReasonMplsSequenceNumber,      /* MPLS_SEQ_NUM_FAIL */

    /* Below are not available Triumph */
    bcmRxReasonMplsCtrlWordError,       /* MPLS_CW_TYPE_NOT_ZERO */
    bcmRxReasonMmrp,                    /* PROTOCOL_MMRP */
    bcmRxReasonSrp,                     /* PROTOCOL_SRP */
    bcmRxReasonWlanSlowpathKeepalive,   /* CAPWAP_KEEPALIVE */
    bcmRxReasonWlanClientError,         /* WLAN_CLIENT_DATABASE_ERROR */
    bcmRxReasonWlanDot1xDrop,           /* WLAN_DOT1X_DROP */
    bcmRxReasonWlanSlowpath,            /* WLAN_CAPWAP_SLOWPATH */
    bcmRxReasonEncapHigigError,         /* EHG_NONHG */
    bcmRxReasonTunnelControl,           /* AMT_CONTROL_PKT */
    bcmRxReasonTimeSync,                /* TIME_SYNC_PKT */
    bcmRxReasonOAMSlowpath,             /* OAM_SLOWPATH */
    bcmRxReasonOAMError,                /* OAM_ERROR */
    bcmRxReasonL2Marked,                /* PROTOCOL_L2_PKT */
    bcmRxReasonL3AddrBindFail,          /* MAC_BIND_FAIL */
    bcmRxReasonIpfixRateViolation       /* IPFIX_FLOW */
};

#ifdef BCM_ENDURO_SUPPORT
static bcm_rx_reason_t 
_bcm_enduro_cpu_cos_map_key [] =
{
    bcmRxReasonBpdu,                    /* PROTOCOL_BPDU */
    bcmRxReasonIpmcReserved,            /* PROTOCOL_IPMC_RSVD */
    bcmRxReasonDhcp,                    /* PROTOCOL_DHCP */
    bcmRxReasonIgmp,                    /* PROTOCOL_IGMP */
    bcmRxReasonArp,                     /* PROTOCOL_ARP */
    bcmRxReasonUnknownVlan,             /* CPU_UVLAN. */
    bcmRxReasonSharedVlanMismatch,      /* PVLAN_MISMATCH */
    bcmRxReasonDosAttack,               /* CPU_DOS_ATTACK */
    bcmRxReasonParityError,             /* PARITY_ERROR */
    bcmRxReasonHigigControl,            /* MH_CONTROL */
    bcmRxReasonTtl1,                    /* TTL_1 */
    bcmRxReasonL3Slowpath,              /* IP_OPTIONS_PKT. */
    bcmRxReasonL2SourceMiss,            /* CPU_SLF */
    bcmRxReasonL2DestMiss,              /* CPU_DLF */
    bcmRxReasonL2Move,                  /* CPU_L2MOVE */
    bcmRxReasonL2Cpu,                   /* CPU_L2CPU */
    bcmRxReasonL2NonUnicastMiss,        /* PBT_NONUC_PKT */
    bcmRxReasonL3SourceMiss,            /* CPU_L3SRC_MISS. */
    bcmRxReasonL3DestMiss,              /* CPU_L3DST_MISS */
    bcmRxReasonL3SourceMove,            /* CPU_L3SRC_MOVE */
    bcmRxReasonMcastMiss,               /* CPU_MC_MISS */
    bcmRxReasonIpMcastMiss,             /* CPU_IPMC_MISS */
    bcmRxReasonL3HeaderError,           /* CPU_L3HDR_ERR */
    bcmRxReasonMartianAddr,             /* CPU_MARTIAN_ADDR */
    bcmRxReasonTunnelError,             /* CPU_TUNNEL_ERR */
    bcmRxReasonHigigHdrError,           /* HGHDR_ERROR */
    bcmRxReasonMcastIdxError,           /* MCIDX_ERROR */
    bcmRxReasonVlanFilterMatch,         /* VFP */
    bcmRxReasonClassBasedMove,          /* CBSM_PREVENTED */
    bcmRxReasonL2LearnLimit,            /* MAC_LIMIT */
    bcmRxReasonE2eHolIbp,               /* E2E_HOL_IBP */
    bcmRxReasonClassTagPackets,         /* HG_HDR_TYPE1 */
    bcmRxReasonNhop,                    /* NHOP */
    bcmRxReasonUrpfFail,                /* URPF_FAILED */
    bcmRxReasonFilterMatch,             /* CPU_FFP */
    bcmRxReasonIcmpRedirect,            /* ICMP_REDIRECT */
    bcmRxReasonSampleSource,            /* CPU_SFLOW_SRC */
    bcmRxReasonSampleDest,              /* CPU_SFLOW_DST */
    bcmRxReasonL3MtuFail,               /* L3_MTU_CHECK_FAIL */
    bcmRxReasonMplsLabelMiss,           /* MPLS_LABEL_MISS */
    bcmRxReasonMplsInvalidAction,       /* MPLS_INVALID_ACTION */
    bcmRxReasonMplsInvalidPayload,      /* MPLS_INVALID_PAYLOAD */
    bcmRxReasonMplsTtl,                 /* MPLS_TTL_CHECK_FAIL */
    bcmRxReasonMplsSequenceNumber,      /* MPLS_SEQ_NUM_FAIL */
    bcmRxReasonMplsCtrlWordError,       /* MPLS_CW_TYPE_NOT_ZERO */
    bcmRxReasonTimeSync,                /* TIME_SYNC_PKT */
    bcmRxReasonOAMSlowpath,             /* OAM_SLOWPATH */
    bcmRxReasonOAMError,                /* OAM_ERROR */
    bcmRxReasonOAMLMDM,                 /* OAM_LMDM */
    bcmRxReasonL2Marked                 /* PROTOCOL_L2_PKT */
};
#endif /* BCM_ENDURO_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
static bcm_rx_reason_t 
_bcm_trident_cpu_cos_map_key [] =
{
    bcmRxReasonBpdu,                    /* PROTOCOL_BPDU */
    bcmRxReasonIpmcReserved,            /* PROTOCOL_IPMC_RSVD */
    bcmRxReasonDhcp,                    /* PROTOCOL_DHCP */
    bcmRxReasonIgmp,                    /* PROTOCOL_IGMP */
    bcmRxReasonArp,                     /* PROTOCOL_ARP */
    bcmRxReasonUnknownVlan,             /* CPU_UVLAN. */
    bcmRxReasonSharedVlanMismatch,      /* PVLAN_MISMATCH */
    bcmRxReasonDosAttack,               /* CPU_DOS_ATTACK */
    bcmRxReasonParityError,             /* PARITY_ERROR */
    bcmRxReasonHigigControl,            /* MH_CONTROL */
    bcmRxReasonTtl1,                    /* TTL_1 */
    bcmRxReasonL3Slowpath,              /* IP_OPTIONS_PKT. */
    bcmRxReasonL2SourceMiss,            /* CPU_SLF */
    bcmRxReasonL2DestMiss,              /* CPU_DLF */
    bcmRxReasonL2Move,                  /* CPU_L2MOVE */
    bcmRxReasonL2Cpu,                   /* CPU_L2CPU */
    bcmRxReasonL2NonUnicastMiss,        /* PBT_NONUC_PKT */
    bcmRxReasonL3SourceMiss,            /* CPU_L3SRC_MISS. */
    bcmRxReasonL3DestMiss,              /* CPU_L3DST_MISS */
    bcmRxReasonL3SourceMove,            /* CPU_L3SRC_MOVE */
    bcmRxReasonMcastMiss,               /* CPU_MC_MISS */
    bcmRxReasonIpMcastMiss,             /* CPU_IPMC_MISS */
    bcmRxReasonL3HeaderError,           /* CPU_L3HDR_ERR */
    bcmRxReasonMartianAddr,             /* CPU_MARTIAN_ADDR */
    bcmRxReasonTunnelError,             /* CPU_TUNNEL_ERR */
    bcmRxReasonHigigHdrError,           /* HGHDR_ERROR */
    bcmRxReasonMcastIdxError,           /* MCIDX_ERROR */
    bcmRxReasonVlanFilterMatch,         /* VFP */
    bcmRxReasonClassBasedMove,          /* CBSM_PREVENTED */
    bcmRxReasonVlanTranslate,           /* VXLT_MISS */
    bcmRxReasonE2eHolIbp,               /* E2E_HOL_IBP */
    bcmRxReasonClassTagPackets,         /* HG_HDR_TYPE1 */
    bcmRxReasonNhop,                    /* NHOP */
    bcmRxReasonUrpfFail,                /* URPF_FAILED */
    bcmRxReasonFilterMatch,             /* CPU_FFP */
    bcmRxReasonIcmpRedirect,            /* ICMP_REDIRECT */
    bcmRxReasonSampleSource,            /* CPU_SFLOW_SRC */
    bcmRxReasonSampleDest,              /* CPU_SFLOW_DST */
    bcmRxReasonL3MtuFail,               /* L3_MTU_CHECK_FAIL */
    bcmRxReasonMplsLabelMiss,           /* MPLS_LABEL_MISS */
    bcmRxReasonMplsInvalidAction,       /* MPLS_INVALID_ACTION */
    bcmRxReasonMplsInvalidPayload,      /* MPLS_INVALID_PAYLOAD */
    bcmRxReasonMplsTtl,                 /* MPLS_TTL_CHECK_FAIL */
    bcmRxReasonMplsSequenceNumber,      /* MPLS_SEQ_NUM_FAIL */
    bcmRxReasonMplsCtrlWordError,       /* MPLS_CW_TYPE_NOT_ZERO */
    bcmRxReasonMmrp,                    /* PROTOCOL_MMRP */
    bcmRxReasonSrp,                     /* PROTOCOL_SRP */
    bcmRxReasonStation,                 /* MY_STATION */
    bcmRxReasonNiv,                     /* NIV_DROP_REASON_ENCODING */
    bcmRxReasonNiv,                     /*   -> */
    bcmRxReasonNiv,                     /* 3 bits */
    bcmRxReasonL3AddrBindFail,          /* MAC_BIND_FAIL */
    bcmRxReasonTunnelControl,           /* AMT_CONTROL_PKT */
    bcmRxReasonTimeSync,                /* TIME_SYNC_PKT */
    bcmRxReasonOAMSlowpath,             /* OAM_SLOWPATH */
    bcmRxReasonOAMError,                /* OAM_ERROR */
    bcmRxReasonL2Marked,                /* PROTOCOL_L2_PKT */
    bcmRxReasonTrill,                   /* TRILL_DROP_REASON_ENCODING */
    bcmRxReasonTrill,                   /*   -> */
    bcmRxReasonTrill                    /* 3 bits */
};

#define _BCM_NT_CPU_COS_MAP_SELECTIONS          7

static bcm_rx_reason_t 
_bcm_niv_cpu_cos_map_key [] =
{
    bcmRxReasonNiv,        /* Base field, must match the entries above */
    bcmRxReasonNivPrioDrop,
    bcmRxReasonNivInterfaceMiss,
    bcmRxReasonNivRpfFail,
    bcmRxReasonNivTagInvalid,
    bcmRxReasonNivTagDrop,
    bcmRxReasonNivUntagDrop
};

static bcm_rx_reason_t 
_bcm_trill_cpu_cos_map_key [] =
{
    bcmRxReasonTrill,      /* Base field, must match the entries above */
    bcmRxReasonTrillInvalid,
    bcmRxReasonTrillMiss,
    bcmRxReasonTrillRpfFail,
    bcmRxReasonTrillSlowpath,
    bcmRxReasonTrillCoreIsIs,
    bcmRxReasonTrillTtl
};
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_KATANA_SUPPORT
static bcm_rx_reason_t 
_bcm_katana_cpu_cos_map_key [] =
{
    bcmRxReasonBpdu,                    /* PROTOCOL_BPDU */
    bcmRxReasonIpmcReserved,            /* PROTOCOL_IPMC_RSVD */
    bcmRxReasonDhcp,                    /* PROTOCOL_DHCP */
    bcmRxReasonIgmp,                    /* PROTOCOL_IGMP */
    bcmRxReasonArp,                     /* PROTOCOL_ARP */
    bcmRxReasonUnknownVlan,             /* CPU_UVLAN. */
    bcmRxReasonSharedVlanMismatch,      /* PVLAN_MISMATCH */
    bcmRxReasonDosAttack,               /* CPU_DOS_ATTACK */
    bcmRxReasonParityError,             /* PARITY_ERROR */
    bcmRxReasonHigigControl,            /* MH_CONTROL */
    bcmRxReasonTtl1,                    /* TTL_1 */
    bcmRxReasonL3Slowpath,              /* IP_OPTIONS_PKT. */
    bcmRxReasonL2SourceMiss,            /* CPU_SLF */
    bcmRxReasonL2DestMiss,              /* CPU_DLF */
    bcmRxReasonL2Move,                  /* CPU_L2MOVE */
    bcmRxReasonL2Cpu,                   /* CPU_L2CPU */
    bcmRxReasonL2NonUnicastMiss,        /* PBT_NONUC_PKT */
    bcmRxReasonL3SourceMiss,            /* CPU_L3SRC_MISS. */
    bcmRxReasonL3DestMiss,              /* CPU_L3DST_MISS */
    bcmRxReasonL3SourceMove,            /* CPU_L3SRC_MOVE */
    bcmRxReasonMcastMiss,               /* CPU_MC_MISS */
    bcmRxReasonIpMcastMiss,             /* CPU_IPMC_MISS */
    bcmRxReasonL3HeaderError,           /* CPU_L3HDR_ERR */
    bcmRxReasonMartianAddr,             /* CPU_MARTIAN_ADDR */
    bcmRxReasonTunnelError,             /* CPU_TUNNEL_ERR */
    bcmRxReasonHigigHdrError,           /* HGHDR_ERROR */
    bcmRxReasonMcastIdxError,           /* MCIDX_ERROR */
    bcmRxReasonVlanFilterMatch,         /* VFP */
    bcmRxReasonClassBasedMove,          /* CBSM_PREVENTED */
    bcmRxReasonVlanTranslate,           /* VXLT_MISS */
    bcmRxReasonE2eHolIbp,               /* E2E_HOL_IBP */
    bcmRxReasonClassTagPackets,         /* HG_HDR_TYPE1 */
    bcmRxReasonNhop,                    /* NHOP */
    bcmRxReasonUrpfFail,                /* URPF_FAILED */
    bcmRxReasonFilterMatch,             /* CPU_FFP */
    bcmRxReasonIcmpRedirect,            /* ICMP_REDIRECT */
    bcmRxReasonSampleSource,            /* CPU_SFLOW_SRC */
    bcmRxReasonSampleDest,              /* CPU_SFLOW_DST */
    bcmRxReasonL3MtuFail,               /* L3_MTU_CHECK_FAIL */
    bcmRxReasonMplsLabelMiss,           /* MPLS_LABEL_MISS */
    bcmRxReasonMplsInvalidAction,       /* MPLS_INVALID_ACTION */
    bcmRxReasonMplsInvalidPayload,      /* MPLS_INVALID_PAYLOAD */
    bcmRxReasonMplsTtl,                 /* MPLS_TTL_CHECK_FAIL */
    bcmRxReasonMplsSequenceNumber,      /* MPLS_SEQ_NUM_FAIL */
    bcmRxReasonBfdSlowpath,             /* BFD_SLOWPATH */
    bcmRxReasonMmrp,                    /* PROTOCOL_MMRP */
    bcmRxReasonSrp,                     /* PROTOCOL_SRP */
    bcmRxReasonStation,                 /* MY_STATION */
    bcmRxReasonNiv,                     /* NIV_DROP_REASON_ENCODING */
    bcmRxReasonNiv,                     /*   -> */
    bcmRxReasonNiv,                     /* 3 bits */
    bcmRxReasonL3AddrBindFail,          /* MAC_BIND_FAIL */
    bcmRxReasonTunnelControl,           /* AMT_CONTROL_PKT */
    bcmRxReasonTimeSync,                /* TIME_SYNC_PKT */
    bcmRxReasonOAMSlowpath,             /* OAM_SLOWPATH */
    bcmRxReasonOAMError,                /* OAM_ERROR */
    bcmRxReasonL2Marked,                /* PROTOCOL_L2_PKT */
    bcmRxReasonOAMLMDM,                 /* OAM_LMDM */
    bcmRxReasonInvalid,                 /* RESERVED_0 */ 
    bcmRxReasonL2LearnLimit,            /* MAC_LIMIT */
    bcmRxReasonBfd,                     /* BFD_ERROR_ENCODING */
    bcmRxReasonBfd,                     /* 2 bits */
};
#endif /* BCM_KATANA_SUPPORT */

#endif /* BCM_TRX_SUPPORT */

/*
 * Function:
 *      bcm_rx_cosq_mapping_reasons_get
 * Purpose:
 *      Get all supported reasons for CPU cosq mapping  
 * Parameters:
 *      unit - Unit reference
 *      reasons - cpu cosq "reasons" mapping bitmap
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_rx_cosq_mapping_reasons_get(int unit, bcm_rx_reasons_t * reasons)
{
#ifdef BCM_TRX_SUPPORT
    uint32          sz;
    uint32          ix;
    bcm_rx_reason_t *cpu_cos_map_key = _bcm_trx_cpu_cos_map_key;

    if (reasons == NULL) {
        return BCM_E_PARAM;
    }

    BCM_RX_REASON_CLEAR_ALL(*reasons);

    if (SOC_IS_TRX(unit)) {
        if (SOC_IS_TD_TT(unit)) {
#ifdef BCM_TRIDENT_SUPPORT
            sz = _bcm_trident_rx_cpu_cosmap_key_max;
            cpu_cos_map_key = _bcm_trident_cpu_cos_map_key;
            /* Handle exceptions to bitmap */
            for (ix = 0; ix < _BCM_NT_CPU_COS_MAP_SELECTIONS; ix++) {
                BCM_RX_REASON_SET(*reasons, _bcm_niv_cpu_cos_map_key[ix]);
                BCM_RX_REASON_SET(*reasons, _bcm_trill_cpu_cos_map_key[ix]);
            }
#endif /* BCM_TRIDENT_SUPPORT */
        } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                   SOC_IS_VALKYRIE2(unit)) {
            sz = _bcm_triumph2_rx_cpu_cosmap_key_max;
#ifdef BCM_ENDURO_SUPPORT
        } else if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
            sz = _bcm_enduro_rx_cpu_cosmap_key_max;
            cpu_cos_map_key = _bcm_enduro_cpu_cos_map_key;
#endif /* BCM_ENDURO_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
        } else if (SOC_IS_KATANA(unit)) {
            sz = _bcm_katana_rx_cpu_cosmap_key_max;
            cpu_cos_map_key = _bcm_katana_cpu_cos_map_key;
#endif /* BCM_KATANA_SUPPORT */
        } else if (SOC_IS_TR_VL(unit)) {
            sz = _bcm_triumph_rx_cpu_cosmap_key_max;
        } else if (SOC_IS_SC_CQ(unit)) {
            sz = _bcm_scorpion_rx_cpu_cosmap_key_max;
        } else {
            return BCM_E_UNAVAIL;
        }

        for (ix = 0; ix < sz; ix++) {
             if (cpu_cos_map_key[ix] != bcmRxReasonInvalid) {
                 BCM_RX_REASON_SET(*reasons, cpu_cos_map_key[ix]);
             }
        }

     return BCM_E_NONE;
     }   
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rx_reasons_get
 * Purpose:
 *      Get all supported reasons for rx packets
 * Parameters:
 *      unit - Unit reference
 *      reasons - rx packet "reasons" bitmap
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_rx_reasons_get (int unit, bcm_rx_reasons_t *reasons)
{
    return (_bcm_common_rx_reasons_get(unit, reasons));
}

static soc_reg_t _rx_redirect_reg[3][2] = {
    { CMIC_PKT_REASONr, CMIC_PKT_REASON_HIr },
    { CMIC_PKT_REASON_DIRECTr, CMIC_PKT_REASON_DIRECT_HIr },
    { CMIC_PKT_REASON_MINIr, CMIC_PKT_REASON_MINI_HIr }
};

#ifdef BCM_CMICM_SUPPORT
static soc_reg_t _cmicm_rx_redirect_reg[3][2] = {
    {CMIC_PKT_REASON_0_TYPEr, CMIC_PKT_REASON_1_TYPEr},
    {CMIC_PKT_REASON_DIRECT_0_TYPEr, CMIC_PKT_REASON_DIRECT_1_TYPEr},
    {CMIC_PKT_REASON_MINI_0_TYPEr, CMIC_PKT_REASON_MINI_0_TYPEr}
};
#endif

int
bcm_esw_rx_redirect_reasons_set(int unit, bcm_rx_redirect_t mode, bcm_rx_reasons_t reasons)
{
    uint32 ix, max_index = 32;
    uint32 addr, rval, rval_hi = 0;
    soc_rx_reason_t *map;
    uint8 set = 0;

    if (!SOC_UNIT_VALID(unit)) { 
        return BCM_E_UNIT;
    }
    if ((mode < 0) || (mode > (int)_SHR_RX_REDIRECT_MAX)) {
        return SOC_E_PARAM;
    }
    map = SOC_DCB(unit).rx_reason_map;

#ifdef BCM_CMICM_SUPPORT
    if (soc_feature(unit, soc_feature_cmicm) && SOC_IS_KATANA(unit)) {
        addr = soc_reg_addr(unit, _cmicm_rx_redirect_reg[mode][0], REG_PORT_ANY, 0);
    } else
#endif
    {
        addr = soc_reg_addr(unit, _rx_redirect_reg[mode][0], REG_PORT_ANY, 0);
    }
    SOC_IF_ERROR_RETURN(soc_pci_getreg(unit, addr, &rval));
    if (soc_feature(unit, soc_feature_dcb_reason_hi)) {
        max_index = 64;
#ifdef BCM_CMICM_SUPPORT
        if (soc_feature(unit, soc_feature_cmicm) && SOC_IS_KATANA(unit)) {
            addr = soc_reg_addr(unit, _cmicm_rx_redirect_reg[mode][1], REG_PORT_ANY, 0);
        } else
#endif
        {
            addr = soc_reg_addr(unit, _rx_redirect_reg[mode][1], REG_PORT_ANY, 0);
        }
        SOC_IF_ERROR_RETURN(soc_pci_getreg(unit, addr, &rval_hi));
    }
    
    for (ix = 0; ix < max_index ; ix++) {
        if (map[ix] != socRxReasonInvalid && 
               map[ix] != socRxReasonCount) {
            if (BCM_RX_REASON_GET(reasons, (bcm_rx_reason_t)map[ix])) {
                set++;
                if (ix < 32) {
                    rval |= (1 << ix);
                } else {
                    rval_hi |= (1 << (ix-32));
                }
            } else {
                set++;
                if (ix < 32) {
                    rval &= ~(1 << ix);
                } else {
                    rval_hi &= ~(1 << (ix-32));
                }
            }
        }
    }
    
    if (set) {
#ifdef BCM_CMICM_SUPPORT
        if (soc_feature(unit, soc_feature_cmicm) && SOC_IS_KATANA(unit)) {
            addr = soc_reg_addr(unit, _cmicm_rx_redirect_reg[mode][0], REG_PORT_ANY, 0);
        } else
#endif
        {
            addr = soc_reg_addr(unit, _rx_redirect_reg[mode][0], REG_PORT_ANY, 0);
        }
        SOC_IF_ERROR_RETURN(soc_pci_write(unit, addr, rval));
        if (soc_feature(unit, soc_feature_dcb_reason_hi)) {
#ifdef BCM_CMICM_SUPPORT
            if (soc_feature(unit, soc_feature_cmicm) && SOC_IS_KATANA(unit)) {
                addr = soc_reg_addr(unit, _cmicm_rx_redirect_reg[mode][1], REG_PORT_ANY, 0);
            } else
#endif
            {
                addr = soc_reg_addr(unit, _rx_redirect_reg[mode][1], REG_PORT_ANY, 0);
            }
            SOC_IF_ERROR_RETURN(soc_pci_write(unit, addr, rval_hi));
        }
    }
    return SOC_E_NONE;
}

int
bcm_esw_rx_redirect_reasons_get(int unit, bcm_rx_redirect_t mode, bcm_rx_reasons_t *reasons)
{
    uint32 ix, max_index = 32;
    uint32 addr, rval, rval_hi;
    soc_rx_reason_t *map;

    if (!SOC_UNIT_VALID(unit)) { 
        return BCM_E_UNIT;
    }
    if ((mode < 0) || (mode > (int)_SHR_RX_REDIRECT_MAX)) {
        return SOC_E_PARAM;
    }
    map = SOC_DCB(unit).rx_reason_map;

#ifdef BCM_CMICM_SUPPORT
    if (soc_feature(unit, soc_feature_cmicm) && SOC_IS_KATANA(unit)) {
        addr = soc_reg_addr(unit, _cmicm_rx_redirect_reg[mode][0], REG_PORT_ANY, 0);
    } else
#endif
    {
        addr = soc_reg_addr(unit, _rx_redirect_reg[mode][0], REG_PORT_ANY, 0);
    }
    SOC_IF_ERROR_RETURN(soc_pci_getreg(unit, addr, &rval));
    if (soc_feature(unit, soc_feature_dcb_reason_hi)) {
        max_index = 64;
#ifdef BCM_CMICM_SUPPORT
        if (soc_feature(unit, soc_feature_cmicm) && SOC_IS_KATANA(unit)) {
            addr = soc_reg_addr(unit, _cmicm_rx_redirect_reg[mode][1], REG_PORT_ANY, 0);
        } else
#endif
        {
            addr = soc_reg_addr(unit, _rx_redirect_reg[mode][1], REG_PORT_ANY, 0);
        }
        SOC_IF_ERROR_RETURN(soc_pci_getreg(unit, addr, &rval_hi));
    }
    BCM_RX_REASON_CLEAR_ALL(*reasons);
    for (ix = 0; ix < max_index ; ix++) {
        if (map[ix] != socRxReasonInvalid && 
               map[ix] != socRxReasonCount) {
            if (ix < 32) {
                if (rval & (1 << ix)) {
                    BCM_RX_REASON_SET(*reasons, (bcm_rx_reason_t)ix);
                }
            } else {
                if (rval & (1 << (ix - 32))) {
                    BCM_RX_REASON_SET(*reasons, (bcm_rx_reason_t)ix);
                }
            }
        }
    }
    return SOC_E_NONE;
}

#ifdef BCM_TRX_SUPPORT
/* Delete an entry of CPU_COS_MAPm */
int
_bcm_trx_rx_cosq_mapping_delete(int unit, int index)
{
    cpu_cos_map_entry_t entry; 

    if (index < soc_mem_index_min(unit, CPU_COS_MAPm) ||
        index > soc_mem_index_max(unit, CPU_COS_MAPm)) {
        return BCM_E_PARAM;
    }

    /* Now zero entry */
    memset (&entry, 0, sizeof(cpu_cos_map_entry_t));

    soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, VALIDf, 0);

    /* write to memory */
    SOC_IF_ERROR_RETURN(WRITE_CPU_COS_MAPm(unit, MEM_BLOCK_ANY, 
                            index, &entry));
    return BCM_E_NONE;
}

/* Get the rx cosq mapping */
int
_bcm_trx_rx_cosq_mapping_get(int unit, int index,
                            bcm_rx_reasons_t *reasons, 
                            bcm_rx_reasons_t *reasons_mask,
                            uint8 *prio, uint8 *prio_mask,
                            uint32 *packet_type, uint32 * packet_type_mask,
                            bcm_cos_queue_t *cosq)
{
    cpu_cos_map_entry_t entry; 
    uint32 mask_high, mask_low;
    uint32 key_low, key_high;
    uint32 ix, keylowlen, masklowlen, maskhighlen;
    uint32 sw_pkt_type_mask, sw_pkt_type_key;
    bcm_rx_reason_t *cpu_cos_map_key = _bcm_trx_cpu_cos_map_key;
#ifdef BCM_TRIDENT_SUPPORT
    int ntix, niv = FALSE, trill = FALSE, bit = 0, nt_key = 0, nt_mask = 0;
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        cpu_cos_map_key = _bcm_trident_cpu_cos_map_key;
    }
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        cpu_cos_map_key = _bcm_katana_cpu_cos_map_key;
    }
#endif /* BCM_KATANA_SUPPORT */

#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
        cpu_cos_map_key = _bcm_enduro_cpu_cos_map_key;
    }
#endif /* BCM_ENDURO_SUPPORT */

    /* Verify the index */
    if (index < soc_mem_index_min(unit, CPU_COS_MAPm) ||
        index > soc_mem_index_max(unit, CPU_COS_MAPm)) {
        return BCM_E_PARAM;
    }

    /* Read the entry */
    SOC_IF_ERROR_RETURN(READ_CPU_COS_MAPm(unit, MEM_BLOCK_ANY, index, &entry));

    /* Return BCM_E_NOT_FOUND if invalid */
    if (soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, VALIDf) == 0) {
        return (BCM_E_NOT_FOUND);
    }        

    /* NULL pointer check */
    if (reasons == NULL  || reasons_mask == NULL  || 
        prio == NULL || prio_mask == NULL || 
        packet_type == NULL || packet_type_mask == NULL || 
        cosq == NULL) {
        return BCM_E_PARAM;
    }
 
    memset(reasons, 0, sizeof(bcm_rx_reasons_t));
    memset(reasons_mask, 0, sizeof(bcm_rx_reasons_t));

    *cosq = (bcm_cos_queue_t)
              soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, COSf);
    
    sw_pkt_type_mask = soc_mem_field32_get(unit, CPU_COS_MAPm, 
                                           &entry, SW_PKT_TYPE_MASKf);
    sw_pkt_type_key  = soc_mem_field32_get(unit, CPU_COS_MAPm, 
                                           &entry, SW_PKT_TYPE_KEYf);

    if (sw_pkt_type_mask == 0 && sw_pkt_type_key == 0) {
        /* all packets matched */
        *packet_type_mask = 0;
        *packet_type = 0;
    } else if (sw_pkt_type_mask == 2 && sw_pkt_type_key == 0) {
        /* Only non-switched packets */
        *packet_type_mask = BCM_RX_COSQ_PACKET_TYPE_SWITCHED;
        *packet_type = 0;
    } else if (sw_pkt_type_mask == 2 && sw_pkt_type_key == 2) {
        *packet_type_mask = BCM_RX_COSQ_PACKET_TYPE_SWITCHED;
        *packet_type = BCM_RX_COSQ_PACKET_TYPE_SWITCHED;
    } else if (sw_pkt_type_mask == 3 && sw_pkt_type_key == 2) {
        *packet_type_mask = BCM_RX_COSQ_PACKET_TYPE_SWITCHED | 
                             BCM_RX_COSQ_PACKET_TYPE_NON_UNICAST;
        *packet_type = BCM_RX_COSQ_PACKET_TYPE_SWITCHED;
    } else if (sw_pkt_type_mask == 3 && sw_pkt_type_key == 3) {
        *packet_type_mask = BCM_RX_COSQ_PACKET_TYPE_SWITCHED |
                             BCM_RX_COSQ_PACKET_TYPE_NON_UNICAST;
        *packet_type = BCM_RX_COSQ_PACKET_TYPE_SWITCHED |
                        BCM_RX_COSQ_PACKET_TYPE_NON_UNICAST;
    }
 
    if (soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, MIRR_PKT_MASKf)) {
        *packet_type_mask |= BCM_RX_COSQ_PACKET_TYPE_MIRROR;
    }

    if (soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, MIRR_PKT_KEYf)) {
        *packet_type |= BCM_RX_COSQ_PACKET_TYPE_MIRROR;
    }

    *prio_mask = 
        soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, INT_PRI_MASKf);
    *prio = 
        soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, INT_PRI_KEYf);
    mask_high = 
        soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, REASONS_MASK_HIGHf);
    mask_low = 
        soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, REASONS_MASK_LOWf);
    key_high = 
        soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, REASONS_KEY_HIGHf);
    key_low  = 
        soc_mem_field32_get(unit, CPU_COS_MAPm, &entry, REASONS_KEY_LOWf);

    /* Convert mask and key to bcm_rx_reasons_t */
    masklowlen  = soc_mem_field_length(unit, CPU_COS_MAPm, REASONS_MASK_LOWf);
    maskhighlen = soc_mem_field_length(unit, CPU_COS_MAPm, REASONS_MASK_HIGHf);
    keylowlen   = soc_mem_field_length(unit, CPU_COS_MAPm, REASONS_KEY_LOWf);

    for (ix = 0; ix < (masklowlen + maskhighlen); ix++) {
         uint32 maskbitset;
         uint32 keybitset;

         keybitset = (ix >= keylowlen)? (key_high & (1 << (ix - keylowlen))) :
                                      (key_low & (1 << ix));
         maskbitset = (ix >= masklowlen)? (mask_high & (1 << (ix - masklowlen))) :
                                      (mask_low & (1 << ix));

#ifdef BCM_TRIDENT_SUPPORT
         if (cpu_cos_map_key[ix] == _bcm_niv_cpu_cos_map_key[0]) {
             /* Gather the whole NIV field */
             if (!niv) {
                 niv = TRUE;
                 bit = 0;
                 nt_key = 0;
                 nt_mask = 0;
             } else {
                 bit++;
             }

             if (maskbitset) {
                 nt_mask |= (1 << bit);
             }
             if (keybitset) {
                 nt_key |= (1 << bit);
             }
             continue;
         } else if (niv) {
             /* Finished NIV field, record the results */
             if (nt_mask) {
                 BCM_RX_REASON_SET(*reasons_mask,
                                   _bcm_niv_cpu_cos_map_key[0]);
                 for (ntix = 1;
                      ntix < _BCM_NT_CPU_COS_MAP_SELECTIONS; ntix++) {
                     if (nt_key == ntix) {
                         BCM_RX_REASON_SET(*reasons,
                                           _bcm_niv_cpu_cos_map_key[ntix]);
                     }
                 }
             }
            niv = FALSE;
         }

         if (cpu_cos_map_key[ix] == _bcm_trill_cpu_cos_map_key[0]) {
             /* Gather the whole TRILL field */
             if (!trill) {
                 trill = TRUE;
                 bit = 0;
                 nt_key = 0;
                 nt_mask = 0;
             } else {
                 bit++;
             }

             if (maskbitset) {
                 nt_mask |= (1 << bit);
             }
             if (keybitset) {
                 nt_key |= (1 << bit);
             }
             continue;
         }
#endif /* BCM_TRIDENT_SUPPORT */

         if (maskbitset) {
             BCM_RX_REASON_SET(*reasons_mask, cpu_cos_map_key[ix]);
         }
         if (keybitset) {
             BCM_RX_REASON_SET(*reasons, cpu_cos_map_key[ix]);
         }
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (trill) {
        /* Finished TRILL field, record the results */
        if (nt_mask) {
            BCM_RX_REASON_SET(*reasons_mask,
                              _bcm_trill_cpu_cos_map_key[0]);
            for (ntix = 1;
                 ntix < _BCM_NT_CPU_COS_MAP_SELECTIONS; ntix++) {
                if (nt_key == ntix) {
                    BCM_RX_REASON_SET(*reasons,
                                      _bcm_trill_cpu_cos_map_key[ntix]);
                }
            }
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */
             
    return BCM_E_NONE;
}

/* Set the rx cosq mapping */
int
_bcm_trx_rx_cosq_mapping_set(int unit, int index,
                            bcm_rx_reasons_t reasons, bcm_rx_reasons_t
                            reasons_mask,
                            uint8 int_prio, uint8 int_prio_mask,
                            uint32 packet_type, uint32 packet_type_mask,
                            bcm_cos_queue_t cosq)
{
    bcm_rx_reason_t  ridx;
    bcm_rx_reasons_t reasons_remain;
    uint32 key_low  = 0, key_high  = 0;
    uint32 mask_low = 0, mask_high = 0;
    cpu_cos_map_entry_t entry; 
    uint8 sw_pkt_type_key = 0;
    uint8 sw_pkt_type_mask = 0;
    uint32 sz, bit;
    uint32 masklowlen, keylowlen;
    bcm_rx_reason_t *cpu_cos_map_key = _bcm_trx_cpu_cos_map_key;
#ifdef BCM_TRIDENT_SUPPORT
    bcm_rx_reason_t *nt_cpu_cos_map_key;
    int ntix, nt_key;
#endif /* BCM_TRIDENT_SUPPORT */
    
    /* Verify COSQ */
    if (SOC_IS_TD_TT(unit)) {
#ifdef BCM_TRIDENT_SUPPORT
        if (cosq >= _bcm_triumph_rx_cpu_max_cosq) {
            return BCM_E_PARAM;
        }
        sz = _bcm_trident_rx_cpu_cosmap_key_max;
        cpu_cos_map_key = _bcm_trident_cpu_cos_map_key;
#endif /* BCM_TRIDENT_SUPPORT */
    } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
               SOC_IS_VALKYRIE2(unit)) {
        if (cosq >= _bcm_triumph_rx_cpu_max_cosq) {
            return BCM_E_PARAM;
        }
        sz = _bcm_triumph2_rx_cpu_cosmap_key_max;
#ifdef BCM_KATANA_SUPPORT
    } else if (SOC_IS_KATANA(unit)) {
        if (cosq >= _bcm_triumph_rx_cpu_max_cosq) {
            return BCM_E_PARAM;
        }
        sz = _bcm_katana_rx_cpu_cosmap_key_max;
        cpu_cos_map_key = _bcm_katana_cpu_cos_map_key;
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_ENDURO_SUPPORT
    } else if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
#ifdef BCM_HURRICANE_SUPPORT
        if (SOC_IS_HURRICANE(unit)) {
            if (cosq >= _bcm_hurricane_rx_cpu_max_cosq) {
                return BCM_E_PARAM;
            }            
        } else
#endif /* BCM_HURRICANE_SUPPORT */
        if (cosq >= _bcm_triumph_rx_cpu_max_cosq) {
            return BCM_E_PARAM;
        }
        sz = _bcm_enduro_rx_cpu_cosmap_key_max;
        cpu_cos_map_key = _bcm_enduro_cpu_cos_map_key;
#endif /* BCM_ENDURO_SUPPORT */
    } else if (SOC_IS_TR_VL(unit)) {
        if (cosq >= _bcm_triumph_rx_cpu_max_cosq) {
            return BCM_E_PARAM;
        }
        sz = _bcm_triumph_rx_cpu_cosmap_key_max;
    } else if (SOC_IS_SC_CQ(unit)) {
        if (cosq >= _bcm_scorpion_rx_cpu_max_cosq) {
            return BCM_E_PARAM;
        }
        sz = _bcm_scorpion_rx_cpu_cosmap_key_max; 
    } else {
        return BCM_E_UNAVAIL;
    }

    /* Verify the index */
    if (index < soc_mem_index_min(unit, CPU_COS_MAPm) ||
        index > soc_mem_index_max(unit, CPU_COS_MAPm)) {
        return BCM_E_PARAM;
    }

    /* Verify the packet type */

    if (packet_type & BCM_RX_COSQ_PACKET_TYPE_NON_UNICAST) {
        sw_pkt_type_key |= 1;
    }
    if (packet_type & BCM_RX_COSQ_PACKET_TYPE_SWITCHED) {
        sw_pkt_type_key |= 2;
    }
    if (packet_type_mask & BCM_RX_COSQ_PACKET_TYPE_NON_UNICAST) {
        sw_pkt_type_mask |= 1;
    }
    if (packet_type_mask & BCM_RX_COSQ_PACKET_TYPE_SWITCHED) {
        sw_pkt_type_mask |= 2;
    }
    sw_pkt_type_key &= sw_pkt_type_mask;

    if ((sw_pkt_type_mask == 0x1) || 
        ((sw_pkt_type_mask != 0) && (sw_pkt_type_key == 0x1))) { 
        /* Hw doesn't support these cases */
        return BCM_E_PARAM;
    }

    reasons_remain = reasons_mask;

    masklowlen  = soc_mem_field_length(unit, CPU_COS_MAPm, 
                                             REASONS_MASK_LOWf);
    keylowlen   = soc_mem_field_length(unit, CPU_COS_MAPm, 
                                             REASONS_KEY_LOWf);

    for (bit = 0; bit < sz; bit++) {
         /* Find reason being set */
         ridx  = cpu_cos_map_key[bit];
         if (!BCM_RX_REASON_GET(reasons_mask, ridx)) {
             continue;
         }

#ifdef BCM_TRIDENT_SUPPORT
         /* Handle the NIV and TRILL special cases */
         if ((ridx == _bcm_niv_cpu_cos_map_key[0]) ||
             (ridx == _bcm_trill_cpu_cos_map_key[0])) {
             if (bit < masklowlen) {
                 
                 return BCM_E_INTERNAL;
             }
             mask_high |=  0x7 << (bit - masklowlen);

             nt_cpu_cos_map_key = (ridx == _bcm_niv_cpu_cos_map_key[0]) ?
                 _bcm_niv_cpu_cos_map_key : _bcm_trill_cpu_cos_map_key;
             nt_key = 0;
             for (ntix = 1;
                  ntix < _BCM_NT_CPU_COS_MAP_SELECTIONS; ntix++) {
                 if (BCM_RX_REASON_GET(reasons,
                                       nt_cpu_cos_map_key[ntix])) {
                     if (nt_key) {
                         return BCM_E_PARAM;
                     } else {
                         nt_key = ntix;
                     }
                 } 
                 BCM_RX_REASON_CLEAR(reasons_remain, nt_cpu_cos_map_key[ntix]);
             }

             if (bit < keylowlen) {
                 
                 return BCM_E_INTERNAL;
             }
             key_high |=  nt_key << (bit - keylowlen);
             bit += 2;
             BCM_RX_REASON_CLEAR(reasons_remain, nt_cpu_cos_map_key[0]);
             continue;
         }
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_KATANA_SUPPORT
         /* Handle BFD special cases */
         if (ridx == bcmRxReasonBfd) {
             /* dont set the mask bit to specify any bfd error */ 
             if ((bit < masklowlen) || (bit < keylowlen)) {
                 
                 return BCM_E_INTERNAL;
             }
             key_high |=  0x3 << (bit - keylowlen);
             bit += 1;
             BCM_RX_REASON_CLEAR(reasons_remain, bcmRxReasonBfd);
             continue;
         } 
#endif /* BCM_KATANA_SUPPORT */

         if (bit >= masklowlen) {
             mask_high |=  1 << (bit - masklowlen);
         } else {
             mask_low |= 1 << bit;
         } 

         if (BCM_RX_REASON_GET(reasons, ridx)) {
             if (bit >= keylowlen) {
                 key_high |= 1 << (bit - keylowlen);
             } else {
                 key_low |= 1 << bit; 
             }
         }

         /* clean the bit of reasons_remain */
         BCM_RX_REASON_CLEAR(reasons_remain, ridx);
    } /* for */

    /* check whether there are reasons unsupported */
    for (ridx = bcmRxReasonInvalid; ridx < bcmRxReasonCount; ridx++) {
         if (BCM_RX_REASON_GET(reasons_remain, ridx)) {
             return BCM_E_PARAM;
         }
    }

    /* Now zero entry */
    memset (&entry, 0, sizeof(cpu_cos_map_entry_t));

    /* Program the key and mask */
    if (mask_low || mask_high) {
        soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                            REASONS_KEY_LOWf, key_low);   
        soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                            REASONS_KEY_HIGHf, key_high);   
        /* program the mask field */
        soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                            REASONS_MASK_LOWf, mask_low);   
        soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                            REASONS_MASK_HIGHf, mask_high);   
    }

    /* Program the packet type */ 

    if (packet_type_mask & (BCM_RX_COSQ_PACKET_TYPE_NON_UNICAST |
                         BCM_RX_COSQ_PACKET_TYPE_SWITCHED)) {
        soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                            SW_PKT_TYPE_MASKf, sw_pkt_type_mask);   
        soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                            SW_PKT_TYPE_KEYf, sw_pkt_type_key);   
    }

    if (packet_type_mask & BCM_RX_COSQ_PACKET_TYPE_MIRROR) {
        soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                            MIRR_PKT_MASKf, 1);   
        if (packet_type & BCM_RX_COSQ_PACKET_TYPE_MIRROR) {
            soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                                MIRR_PKT_KEYf, 1);   
        }
    }

    /* Handle priority when int_prio_mask != 0 */
    if (int_prio_mask) {
        soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                            INT_PRI_KEYf, (int_prio & 0xf));   
        soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, 
                            INT_PRI_MASKf, (int_prio_mask & 0xf));   
    }

    soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, COSf, cosq);
    soc_mem_field32_set(unit, CPU_COS_MAPm, &entry, VALIDf, 1);

    /* write to memory */
    SOC_IF_ERROR_RETURN(WRITE_CPU_COS_MAPm(unit, MEM_BLOCK_ANY, 
                                           index, &entry));

    return BCM_E_NONE;
}
#endif /* BCM_TRX_SUPPORT */

/*
 * Function:
 *      bcm_rx_cosq_mapping_set
 * Purpose:
 *      Set the COSQ mapping to map qualified packets to the a CPU cos queue.
 * Parameters: 
 *      unit - Unit reference
 *      index - Index into COSQ mapping table (0 is lowest match priority)
 *      reasons - packet "reasons" bitmap
 *      reasons_mask - mask for packet "reasons" bitmap
 *      int_prio - internal priority value
 *      int_prio_mask - mask for internal priority value
 *      packet_type - packet type bitmap (BCM_RX_COSQ_PACKET_TYPE_*)
 *      packet_type_mask - mask for packet type bitmap
 *      cosq - CPU cos queue
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_cosq_mapping_set(int unit, int index, 
                            bcm_rx_reasons_t reasons, bcm_rx_reasons_t reasons_mask, 
                            uint8 int_prio, uint8 int_prio_mask,
                            uint32 packet_type, uint32 packet_type_mask,
                            bcm_cos_queue_t cosq)
{
#ifdef BCM_TRX_SUPPORT
   if (SOC_IS_TRX(unit)) {
       return _bcm_trx_rx_cosq_mapping_set(unit, index, reasons, reasons_mask,
                                           int_prio, int_prio_mask, 
                                           packet_type, packet_type_mask,
                                           cosq);
   }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rx_cosq_mapping_get
 * Purpose:
 *      Get the COSQ mapping at the specified index
 * Parameters:
 *      unit - Unit reference
 *      index - Index into COSQ mapping table (0 is lowest match priority)
 *      reasons - packet "reasons" bitmap
 *      reasons_mask - mask for packet "reasons" bitmap
 *      int_prio - internal priority value
 *      int_prio_mask - mask for internal priority value
 *      packet_type - packet type bitmap (BCM_RX_COSQ_PACKET_TYPE_*)
 *      packet_type_mask - mask for packet type bitmap 
 *      cosq - CPU cos queue 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_cosq_mapping_get(int unit, int index, 
                            bcm_rx_reasons_t *reasons, bcm_rx_reasons_t *reasons_mask, 
                            uint8 *int_prio, uint8 *int_prio_mask,
                            uint32 *packet_type, uint32 *packet_type_mask,
                            bcm_cos_queue_t *cosq)
{
#ifdef BCM_TRX_SUPPORT
   if (SOC_IS_TRX(unit)) {
       return _bcm_trx_rx_cosq_mapping_get(unit, index, reasons, reasons_mask,
                                           int_prio, int_prio_mask, 
                                           packet_type, packet_type_mask,
                                           cosq);
   }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rx_cosq_mapping_delete
 * Purpose:
 *      Delete the COSQ mapping at the specified index
 * Parameters: 
 *      unit - Unit reference
 *      index - Index into COSQ mapping table 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_cosq_mapping_delete(int unit, int index)
{
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
    return _bcm_trx_rx_cosq_mapping_delete(unit, index);
    }
#endif
    return BCM_E_UNAVAIL;
}

/*  
 * Function:
 *      bcm_rx_queue_channel_set
 * Purpose:
 *      Assign a RX channel to a cosq 
 * Parameters:
 *      unit - Unit reference
 *      queue_id - CPU cos queue index (0 - (max cosq - 1)) 
 *                                      (Negative for all)
 *      chan_id - channel index (0 - (BCM_RX_CHANNELS-1))
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_queue_channel_set (int unit, bcm_cos_queue_t queue_id, 
                              bcm_rx_chan_t chan_id)
{
#ifdef BCM_CMICM_SUPPORT
    if (chan_id >= BCM_RX_CHANNELS) {
        /* API access is constrained to only the PCI host channels. */
        return BCM_E_PARAM;
    }
#endif

    return (_bcm_common_rx_queue_channel_set(unit, queue_id, 
	  				     chan_id));
}

/*  
 * Function:
 *      bcm_rx_queue_channel_get
 * Purpose:
 *      Get the associated rx channel with a given cosq
 * Parameters:
 *      unit - Unit reference
 *      queue_id - CPU cos queue index (0 - (max cosq - 1)) 
 *      chan_id - channel index (0 - (BCM_RX_CHANNELS-1))
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rx_queue_channel_get(int unit, bcm_cos_queue_t queue_id, 
                             bcm_rx_chan_t *chan_id)
{
    return (_bcm_common_rx_queue_channel_get(unit, queue_id, 
	  				     chan_id));
}

/*
 * Function:
 *      bcm_rx_active
 * Purpose:
 *      Return boolean as to whether unit is running
 * Parameters:
 *      unit - StrataXGS to check
 * Returns:
 *      Boolean:   TRUE if unit is running.
 * Notes:
 *
 */

int
bcm_esw_rx_active(int unit)
{
    return (_bcm_common_rx_active(unit));
}


/*
 * Function:
 *      bcm_rx_running_channels_get
 * Purpose:
 *      Returns a bitmap indicating which channels are active
 * Parameters:
 *      unit       - Which unit to operate on
 * Returns:
 *      Bitmap of active channels
 * Notes:
 */

int
bcm_esw_rx_channels_running(int unit, uint32 *channels)
{
    return (_bcm_common_rx_channels_running(unit, channels));
}

/*
 * Function:
 *      bcm_rx_alloc
 * Purpose:
 *      Gateway to configured RX allocation function
 * Parameters:
 *      unit - Unit reference
 *      pkt_size - Packet size, see notes.
 *      flags - Used to set up packet flags
 * Returns:
 *      Pointer to new packet buffer or NULL if cannot alloc memory
 * Notes:
 *      Although the packet size is normally configured per unit,
 *      the option of using a different size is given here.  If
 *      pkt_size <= 0, then the default packet size for the unit
 *      is used.
 */

int
bcm_esw_rx_alloc(int unit, int pkt_size, uint32 flags, void **buf)
{
    return (_bcm_common_rx_alloc(unit, pkt_size, flags, buf));
}


/*
 * Function:
 *      bcm_rx_free
 * Purpose:
 *      Gateway to configured RX free function.  Generally, packet
 *      buffer was allocated with bcm_rx_alloc.
 * Parameters:
 *      unit - Unit reference
 *      pkt - Packet to free
 * Returns:
 * Notes:
 *      In particular, packets stolen from RX with BCM_RX_HANDLED_OWNED
 *      should use this to free packets.
 */

int
bcm_esw_rx_free(int unit, void *pkt_data)
{
    return (_bcm_common_rx_free(unit, pkt_data));
}


/*
 * Function:
 *      bcm_rx_free_enqueue
 * Purpose:
 *      Queue a packet to be freed by the RX thread.
 * Parameters:
 *      unit - Unit reference
 *      pkt - Packet to free
 * Returns:
 * Notes:
 *      This may be called in interrupt context to queue
 *      a packet to be freed.
 *
 *      Assumes pkt_data is 32-bit aligned.
 *      Uses the first word of the freed data as a "next" pointer
 *      for the free list.
 */
int
bcm_esw_rx_free_enqueue(int unit, void *pkt_data)
{
    return (_bcm_common_rx_free_enqueue(unit, pkt_data));
}


/****************************************************************
 *
 * Global (all COS) and per COS rate limiting configuration
 *
 ****************************************************************/


/*
 * Functions:
 *      bcm_rx_burst_set, get; bcm_rx_rate_set, get
 *      bcm_rx_cos_burst_set, get; bcm_rx_cos_rate_set, get;
 *      bcm_rx_cos_max_len_set, get
 * Purpose:
 *      Get/Set the global and per COS limits:
 *           rate:      Packets/second
 *           burst:     Packets (max tokens in bucket)
 *           max_len:   Packets (max permitted in queue).
 * Parameters:
 *      unit - Unit reference
 *      cos - For per COS functions, which COS queue affected
 *      pps - Rate in packets per second (OUT for get functions)
 *      burst - Burst rate for the system in packets (OUT for get functions)
 *      max_q_len - Burst rate for the system in packets (OUT for get functions)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      PPS must be >= 0 and
 *      Max queue length must be >= 0;
 *          otherwise param error.
 *
 *      PPS == 0 -> rate limiting disabled.
 *      max_q_len == 0 -> no limit on queue length (not recommended)
 */

int
bcm_esw_rx_rate_set(int unit, int pps)
{
    RX_INIT_CHECK(unit);

    if (pps < 0) {
        return BCM_E_PARAM;
    }
    RX_PPS(unit) = pps;

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
    if (RX_IS_LOCAL(unit) && SOC_UNIT_VALID(unit)) {
        if (soc_feature(unit, soc_feature_packet_rate_limit)) {
#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                SOC_IS_VALKYRIE2(unit) || SOC_IS_KATANA(unit)) {
                return bcm_rx_cos_rate_set(unit, BCM_RX_COS_ALL, pps);
            } else if (SOC_IS_TD_TT(unit)) { 
                int burst;
                BCM_IF_ERROR_RETURN(bcm_esw_rx_burst_get(unit, &burst));
                return bcm_tr_port_pps_rate_egress_set(unit, CMIC_PORT(unit),
                                                       (uint32)pps, (uint32)burst);
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_TRX_SUPPORT
            if (SOC_IS_TRX(unit) && !SOC_IS_HURRICANE(unit)) {
               return bcm_tr_cosq_port_pps_set(unit, CMIC_PORT(unit),
                                               -1, pps);
            }
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT2_SUPPORT
            if (SOC_IS_FIREBOLT2(unit)) {
                return bcm_fb2_cosq_port_pps_set(unit, CMIC_PORT(unit),
                                                 -1, pps);
            }
#endif /* BCM_FIREBOLT2_SUPPORT */
        }
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */

    return BCM_E_NONE;
}

int
bcm_esw_rx_rate_get(int unit, int *pps)
{
    return (_bcm_common_rx_rate_get(unit, pps));
}

int 
bcm_esw_rx_cpu_rate_set(int unit, int pps)
{
    return (_bcm_common_rx_cpu_rate_set(unit, pps));
}

int 
bcm_esw_rx_cpu_rate_get(int unit, int *pps)
{
    return (_bcm_common_rx_cpu_rate_get(unit, pps));
}


int
bcm_esw_rx_burst_set(int unit, int burst)
{
    RX_INIT_CHECK(unit);

    RX_BURST(unit) = burst;
    RX_TOKENS(unit) = burst;

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
    if (RX_IS_LOCAL(unit) && SOC_UNIT_VALID(unit)) {
        if (soc_feature(unit, soc_feature_packet_rate_limit)) {
#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
                SOC_IS_KATANA(unit)) {
                return bcm_rx_cos_burst_set(unit, BCM_RX_COS_ALL, burst);
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_TRX_SUPPORT
            if (SOC_IS_TRX(unit) && !SOC_IS_HURRICANE(unit)) {
                return bcm_tr_cosq_port_burst_set(unit, CMIC_PORT(unit), -1,
                                                  burst);
            }
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT2_SUPPORT
            if (SOC_IS_FIREBOLT2(unit)) {
                return bcm_fb2_cosq_port_burst_set(unit, CMIC_PORT(unit), -1,
                                                   burst);
            }
#endif /* BCM_FIREBOLT2_SUPPORT */
        }
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */

    return BCM_E_NONE;
}

int
bcm_esw_rx_burst_get(int unit, int *burst)
{
    return (_bcm_common_rx_burst_get(unit, burst));
}


int
bcm_esw_rx_cos_rate_set(int unit, int cos, int pps)
{
    int i;

    if (!LEGAL_COS(cos) || pps < 0) {
        return BCM_E_PARAM;
    }

    RX_INIT_CHECK(unit);
    if (cos == BCM_RX_COS_ALL) {
        for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
            RX_COS_PPS(unit, i) = pps;
        }
    } else {
        RX_COS_PPS(unit, cos) = pps;
    }

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
    if (RX_IS_LOCAL(unit) && SOC_UNIT_VALID(unit)) {
        if (soc_feature(unit, soc_feature_packet_rate_limit)) {
#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_IS_TD_TT(unit)) {
                if (cos == BCM_RX_COS_ALL) {
                    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                        BCM_IF_ERROR_RETURN
                            (bcm_td_cosq_port_pps_set(unit, CMIC_PORT(unit), i,
                                                      pps));
                    }
                } else {
                    BCM_IF_ERROR_RETURN
                        (bcm_td_cosq_port_pps_set(unit, CMIC_PORT(unit), cos,
                                                  pps));
                }
                return BCM_E_NONE;
            }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                SOC_IS_VALKYRIE2(unit)) {
                if (cos == BCM_RX_COS_ALL) {
                    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                        BCM_IF_ERROR_RETURN
                            (bcm_tr2_cosq_port_pps_set(unit, CMIC_PORT(unit), i,
                                                      pps));
                    }
                } else {
                    BCM_IF_ERROR_RETURN
                        (bcm_tr2_cosq_port_pps_set(unit, CMIC_PORT(unit), cos,
                                                  pps));
                }
                return BCM_E_NONE;
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_HURRICANE_SUPPORT
            if (SOC_IS_HURRICANE(unit)) {
                return BCM_E_NONE;
            }
#endif
#ifdef BCM_KATANA_SUPPORT
            if (SOC_IS_KATANA(unit)) {
                return BCM_E_NONE;
            }
#endif
#ifdef BCM_TRX_SUPPORT
            if (SOC_IS_TRX(unit)) {
                if (cos == BCM_RX_COS_ALL) {
                    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                        BCM_IF_ERROR_RETURN
                            (bcm_tr_cosq_port_pps_set(unit, CMIC_PORT(unit), i,
                                                      pps));
                    }
                } else {
                    BCM_IF_ERROR_RETURN
                        (bcm_tr_cosq_port_pps_set(unit, CMIC_PORT(unit), cos,
                                                  pps));
                }
                return BCM_E_NONE;
            }
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT2_SUPPORT
            if (SOC_IS_FIREBOLT2(unit)) {
                if (cos == BCM_RX_COS_ALL) {
                    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                        BCM_IF_ERROR_RETURN
                            (bcm_fb2_cosq_port_pps_set(unit, CMIC_PORT(unit),
                                                       i, pps));
                    }
                } else {
                    BCM_IF_ERROR_RETURN
                        (bcm_fb2_cosq_port_pps_set(unit, CMIC_PORT(unit),
                                                   cos, pps));
                }
                return BCM_E_NONE;
            }
 #endif /* BCM_FIREBOLT2_SUPPORT */
       }
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */

    return BCM_E_NONE;
}

int
bcm_esw_rx_cos_rate_get(int unit, int cos, int *pps)
{
    return (_bcm_common_rx_cos_rate_get(unit, cos, pps));
}

int
bcm_esw_rx_cos_burst_set(int unit, int cos, int burst)
{
    rx_queue_t *queue;
    int i;

    if (!LEGAL_COS(cos)) {
        return BCM_E_PARAM;
    }

    RX_INIT_CHECK(unit);
    if (cos == BCM_RX_COS_ALL) {
        for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
            queue = RX_QUEUE(unit, i);
            queue->burst = burst;
            queue->tokens = burst;
        }
    } else {
        queue = RX_QUEUE(unit, cos);
        queue->burst = burst;
        queue->tokens = burst;
    }

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
    if (RX_IS_LOCAL(unit) && SOC_UNIT_VALID(unit)) {
        if (soc_feature(unit, soc_feature_packet_rate_limit)) {
#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_IS_TD_TT(unit)) {
                if (cos == BCM_RX_COS_ALL) {
                    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                        BCM_IF_ERROR_RETURN
                            (bcm_td_cosq_port_burst_set(unit, CMIC_PORT(unit),
                                                        i, burst));
                    }
                } else {
                    BCM_IF_ERROR_RETURN
                        (bcm_td_cosq_port_burst_set(unit, CMIC_PORT(unit), cos,
                                                    burst));
                }
                return BCM_E_NONE;
            }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                SOC_IS_VALKYRIE2(unit)) {
                if (cos == BCM_RX_COS_ALL) {
                    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                        BCM_IF_ERROR_RETURN
                            (bcm_tr2_cosq_port_burst_set(unit, CMIC_PORT(unit),
                                                        i, burst));
                    }
                } else {
                    BCM_IF_ERROR_RETURN
                        (bcm_tr2_cosq_port_burst_set(unit, CMIC_PORT(unit), cos,
                                                    burst));
                }
                return BCM_E_NONE;
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
            if (SOC_IS_KATANA(unit)) {
                return BCM_E_NONE;
            }
#endif
#ifdef BCM_TRX_SUPPORT
            if (SOC_IS_TRX(unit) && !SOC_IS_HURRICANE(unit)) {
                if (cos == BCM_RX_COS_ALL) {
                    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                        BCM_IF_ERROR_RETURN
                            (bcm_tr_cosq_port_burst_set(unit, CMIC_PORT(unit),
                                                        i, burst));
                    }
                } else {
                    BCM_IF_ERROR_RETURN
                        (bcm_tr_cosq_port_burst_set(unit, CMIC_PORT(unit), cos,
                                                    burst));
                }
                return BCM_E_NONE;
            }
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT2_SUPPORT
            if (SOC_IS_FIREBOLT2(unit)) {
                if (cos == BCM_RX_COS_ALL) {
                    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                        BCM_IF_ERROR_RETURN
                            (bcm_fb2_cosq_port_burst_set(unit,
                                                         CMIC_PORT(unit),
                                                         i, burst));
                    }
                } else {
                    BCM_IF_ERROR_RETURN
                        (bcm_fb2_cosq_port_burst_set(unit, CMIC_PORT(unit),
                                                     cos, burst));
                }
                return BCM_E_NONE;
            }
#endif /* BCM_FIREBOLT2_SUPPORT */
        }
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */

    return BCM_E_NONE;
}

int
bcm_esw_rx_cos_burst_get(int unit, int cos, int *burst)
{
    return (_bcm_common_rx_cos_burst_get(unit, cos, burst));
}

int
bcm_esw_rx_cos_max_len_set(int unit, int cos, int max_q_len)
{
    return (_bcm_common_rx_cos_max_len_set(unit, cos, max_q_len));
}

int
bcm_esw_rx_cos_max_len_get(int unit, int cos, int *max_q_len)
{
    return (_bcm_common_rx_cos_max_len_get(unit, cos, max_q_len));
}

/****************************************************************
 *
 * RX Control
 *
 ****************************************************************/

STATIC int
_bcm_esw_rx_chan_flag_set(int unit, uint32 flag, int value)
{
    int chan;

    FOREACH_SETUP_CHANNEL(unit, chan) {
        if (value) {
            RX_CHAN_FLAGS(unit, chan) |= flag;
        } else {
            RX_CHAN_FLAGS(unit, chan) &= ~flag;
        }
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_esw_rx_chan_flag_get(int unit, uint32 flag, int *value)
{
    int chan;

    FOREACH_SETUP_CHANNEL(unit, chan) {
        *value = RX_CHAN_FLAGS(unit, chan) & flag;
        break;
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_esw_rx_user_flag_set(int unit, uint32 flag, int value)
{
    if (value) {
        RX_USER_FLAGS(unit) |= flag;
    } else {
        RX_USER_FLAGS(unit) &= ~flag;
    }

    return BCM_E_NONE;
}


STATIC int
_bcm_esw_rx_user_flag_get(int unit, uint32 flag, int *value)
{
    *value = RX_USER_FLAGS(unit) & flag;

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_rx_control_get(int unit, bcm_rx_control_t type, int *value)
 * Description:
 *      Get the status of specified RX feature.
 * Parameters:
 *      unit - Device number
 *      type - RX control parameter
 *      value - (OUT) Current value of control parameter
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 */

int
bcm_esw_rx_control_get(int unit, bcm_rx_control_t type, int *value)
{
    int rv = BCM_E_UNAVAIL;

    switch (type) {
    case bcmRxControlCRCStrip:
        rv = _bcm_esw_rx_chan_flag_get(unit, BCM_RX_F_CRC_STRIP, value);
        break;
    case bcmRxControlVTagStrip:
        rv = _bcm_esw_rx_chan_flag_get(unit, BCM_RX_F_VTAG_STRIP, value);
        break;
    case bcmRxControlRateStall:
        rv = _bcm_esw_rx_chan_flag_get(unit, BCM_RX_F_RATE_STALL, value);
        break;
    case bcmRxControlMultiDCB:
        rv = _bcm_esw_rx_chan_flag_get(unit, BCM_RX_F_MULTI_DCB, value);
        break;
    case bcmRxControlOversizedOK:
        rv = _bcm_esw_rx_chan_flag_get(unit, BCM_RX_F_OVERSIZED_OK, value);
        break;
    case bcmRxControlIgnoreHGHeader:
        rv = _bcm_esw_rx_user_flag_get(unit, BCM_RX_F_IGNORE_HGHDR, value);
        break;
    case bcmRxControlIgnoreSLHeader:
        rv = _bcm_esw_rx_user_flag_get(unit, BCM_RX_F_IGNORE_SLTAG, value);
        break;
    default:
        /* unsupported flag */
        break;
    }
    return rv;
}

/*
 * Function:
 *      bcm_rx_control_set(int unit, bcm_rx_control_t type, int value)
 * Description:
 *      Enable/Disable specified RX feature.
 * Parameters:
 *      unit - Device number
 *      type - RX control parameter
 *      value - new value of control parameter
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 *
 */

int
bcm_esw_rx_control_set(int unit, bcm_rx_control_t type, int value)
{
    int rv = BCM_E_UNAVAIL;

    switch (type) {
    case bcmRxControlCRCStrip:
        rv = _bcm_esw_rx_chan_flag_set(unit, BCM_RX_F_CRC_STRIP, value);
        break;
    case bcmRxControlVTagStrip:
        rv = _bcm_esw_rx_chan_flag_set(unit, BCM_RX_F_VTAG_STRIP, value);
        break;
    case bcmRxControlRateStall:
        rv = _bcm_esw_rx_chan_flag_set(unit, BCM_RX_F_RATE_STALL, value);
        break;
    case bcmRxControlMultiDCB:
        rv = _bcm_esw_rx_chan_flag_set(unit, BCM_RX_F_MULTI_DCB, value);
        break;
    case bcmRxControlOversizedOK:
        rv = _bcm_esw_rx_chan_flag_set(unit, BCM_RX_F_OVERSIZED_OK, value);
        break;
    case bcmRxControlIgnoreHGHeader:
        rv = _bcm_esw_rx_user_flag_set(unit, BCM_RX_F_IGNORE_HGHDR, value);
        break;
    case bcmRxControlIgnoreSLHeader:
        rv = _bcm_esw_rx_user_flag_set(unit, BCM_RX_F_IGNORE_SLTAG, value);
        break;
    default:
        /* unsupported flag */
        break;
    }
    return rv;
}

/*
 * Function:
 *      bcm_rx_remote_pkt_enqueue
 * Purpose:
 *      Enqueue a remote packet for normal RX processing
 * Parameters:
 *      unit          - The BCM unit in which queue the pkt is placed
 *      pkt           - The packet to enqueue
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_rx_remote_pkt_enqueue(int unit, bcm_pkt_t *pkt)
{
    return (_bcm_common_rx_remote_pkt_enqueue(unit, pkt));
}



#if defined(BROADCOM_DEBUG)

/*
 * Function:
 *      bcm_rx_show
 * Purpose:
 *      Show RX information for the specified device.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      Nothing.
 */

int
bcm_esw_rx_show(int unit)
{
    return _bcm_common_rx_show(unit);
}

#endif  /* BROADCOM_DEBUG */
