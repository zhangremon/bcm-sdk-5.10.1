#ifndef _SOC_SBX_G2P3_COUNTERS_H_
#define _SOC_SBX_G2P3_COUNTERS_H_
/*-----------------------------------------------------------------------------
 * -*- Mode:c; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*-
 * vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0:
 *
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
 * $Id: g2p3_cnt.h 1.5.158.1 Broadcom SDK $
 *
 * soc_sbx_g2p3_counters : Guadalupe2k Counters
 *
 *-----------------------------------------------------------------------------*/
#include <sal/types.h>

typedef enum {
  SOC_SBX_G2P3_TRAFFIC_TYPE_UNICAST  = 0, /* Unicast */
  SOC_SBX_G2P3_TRAFFIC_TYPE_L2MC     = 1, /* L2 Multicast */
  SOC_SBX_G2P3_TRAFFIC_TYPE_FLOOD    = 2, /* Flooded */
  SOC_SBX_G2P3_TRAFFIC_TYPE_ROUTED   = 3, /* Routed */
  SOC_SBX_G2P3_TRAFFIC_TYPE_COUNTER_MAX
} soc_sbx_g2p3_traffic_type_t;


typedef struct soc_sbx_g2p3_count_s
{
    uint64_t    bytes;
    uint64_t    packets;
} soc_sbx_g2p3_count_t;

/* VLAN counter enumeration for single mode */
typedef enum {
    SOC_SBX_G2P3_VLAN_COUNT_SINGLE_EVERYTHING,      /* everything */
    SOC_SBX_G2P3_VLAN_COUNT_SINGLE_COUNT            /* must be last */
} soc_sbx_g2p3_vlan_count_single_t;

/* VLAN counter enumeration for typed mode */
typedef enum {
    SOC_SBX_G2P3_VLAN_COUNT_TYPED_L2_UNICAST_PASS,    /* L2 unicast passed */
    SOC_SBX_G2P3_VLAN_COUNT_TYPED_L2_UNICAST_DROP,    /* L2 unicast dropped */
    SOC_SBX_G2P3_VLAN_COUNT_TYPED_L2_MULTICAST_PASS,  /* L2 multicast passed */
    SOC_SBX_G2P3_VLAN_COUNT_TYPED_L2_MULTICAST_DROP,  /* L2 multicast dropped */
    SOC_SBX_G2P3_VLAN_COUNT_TYPED_L2_FLOOD_PASS,      /* L2 flood passed */
    SOC_SBX_G2P3_VLAN_COUNT_TYPED_L2_FLOOD_DROP,      /* L2 flood dropped */
    SOC_SBX_G2P3_VLAN_COUNT_TYPED_L3_PASS,           /* L3 passed */
    SOC_SBX_G2P3_VLAN_COUNT_TYPED_L3_DROP,           /* L3 dropped */
    SOC_SBX_G2P3_VLAN_COUNT_TYPED_COUNT_PASS /* must be last */
} soc_sbx_g2p3_vlan_count_typed_t;

/* VLAN counter enumeration for MEF single mode */
typedef enum {
    SOC_SBX_G2P3_VLAN_COUNT_MEF_SINGLE_DLF_DROP,      /* Dropped */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_SINGLE1,            /* unused */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_SINGLE2,            /* unused */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_SINGLE_DP0,          /* DP 0 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_SINGLE_DP1,          /* DP 1 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_SINGLE_DP2,          /* DP 2 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_SINGLE_COUNT         /* must be last */
} soc_sbx_g2p3_vlan_count_mef_single_t;

/* VLAN counter enumeration for MEF typed mode */
typedef enum {
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_DLF_DROP,       /* Dropped */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED1,             /* unused */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED2,             /* unused */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS0_DP0,       /* COS 0, DP 0 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS0_DP1,       /* COS 0, DP 1 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS0_DP2,       /* COS 0, DP 2 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS1_DP0,       /* COS 1, DP 0 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS1_DP1,       /* COS 1, DP 1 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS1_DP2,       /* COS 1, DP 2 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS2_DP0,       /* COS 2, DP 0 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS2_DP1,       /* COS 2, DP 1 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS2_DP2,       /* COS 2, DP 2 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS3_DP0,       /* COS 3, DP 0 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS3_DP1,       /* COS 3, DP 1 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS3_DP2,       /* COS 3, DP 2 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS4_DP0,       /* COS 4, DP 0 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS4_DP1,       /* COS 4, DP 1 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS4_DP2,       /* COS 4, DP 2 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS5_DP0,       /* COS 5, DP 0 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS5_DP1,       /* COS 5, DP 1 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS5_DP2,       /* COS 5, DP 2 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS6_DP0,       /* COS 6, DP 0 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS6_DP1,       /* COS 6, DP 1 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS6_DP2,       /* COS 6, DP 2 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS7_DP0,       /* COS 7, DP 0 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS7_DP1,       /* COS 7, DP 1 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COS7_DP2,       /* COS 7, DP 2 */
    SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COUNT,          /* must be last */
} soc_sbx_g2p3_vlan_count_mef_typed_t;

typedef union
{
    soc_sbx_g2p3_count_t single[SOC_SBX_G2P3_VLAN_COUNT_SINGLE_COUNT];
    soc_sbx_g2p3_count_t typed[SOC_SBX_G2P3_VLAN_COUNT_TYPED_COUNT_PASS];
    soc_sbx_g2p3_count_t mefSingle[SOC_SBX_G2P3_VLAN_COUNT_MEF_SINGLE_COUNT];
    soc_sbx_g2p3_count_t mefTyped[SOC_SBX_G2P3_VLAN_COUNT_MEF_TYPED_COUNT];
} soc_sbx_g2p3_vlan_count_t;

#define SOC_SBX_G2P3_VLAN_COUNT_WORDS (sizeof(soc_sbx_g2p3_vlan_count_t)/sizeof(uint64_t))

#define SOC_SBX_G2P3_EGRESS_COUNT_WORDS (sizeof(soc_sbx_g2p3_egress_count_t)/sizeof(uint64_t))

/**
 * soc_sbx_g2p3_counters_ingress_read()
 *
 * @brief
 *
 * Read Ingress counters from memory
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Counter Index
 * @param pCount    - Pointer to src/dst counter structures
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_ingress_read( soc_sbx_g2p3_state_t *pFe,
              uint32_t ulIndex,
              soc_sbx_g2p3_count_t *pCount,
              uint32_t ulEntries,
              uint8_t bSync );

/**
 * soc_sbx_g2p3_counters_ingress_clear()
 *
 * @brief
 *
 * Clear Ingress  counters
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Counter Index
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_ingress_clear( soc_sbx_g2p3_state_t *pFe,
               uint32_t ulIndex,
               uint32_t ulEntries,
               uint8_t bSync );


/**
 * soc_sbx_g2p3_counters_egress_read()
 *
 * @brief
 *
 * Read egress counters from memory
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Egress Counter Index
 * @param pCount    - Pointer to egress counter structures
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_egress_read( soc_sbx_g2p3_state_t *pFe,
              uint32_t ulIndex,
              soc_sbx_g2p3_count_t *pCount,
              uint32_t ulEntries,
              uint8_t bSync );

/**
 * soc_sbx_g2p3_counters_egress_clear()
 *
 * @brief
 *
 * Clear egress counters
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Egress Counter Index
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_egress_clear( soc_sbx_g2p3_state_t *pFe,
               uint32_t ulIndex,
               uint32_t ulEntries,
               uint8_t bSync );


/**
 * soc_sbx_g2p3_counters_exception_read()
 *
 * @brief
 *
 * Read exception counters from memory
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Exception Counter Index
 * @param pCount    - Pointer to exception counter structures
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_exception_read( soc_sbx_g2p3_state_t *pFe,
              uint32_t ulIndex,
              soc_sbx_g2p3_count_t *pCount,
              uint32_t ulEntries,
              uint8_t bSync );

/**
 * soc_sbx_g2p3_counters_exception_clear()
 *
 * @brief
 *
 * Clear exception counters
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Egress Counter Index
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_exception_clear( soc_sbx_g2p3_state_t *pFe,
               uint32_t ulIndex,
               uint32_t ulEntries,
               uint8_t bSync );

/**
 * soc_sbx_g2p3_counters_drop_read()
 *
 * @brief
 *
 * Read per-port drop counters from memory
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Drop Counter Index
 * @param pCount    - Pointer to drop counter structures
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_drop_read( soc_sbx_g2p3_state_t *pFe,
              uint32_t ulIndex,
              soc_sbx_g2p3_count_t *pCount,
              uint32_t ulEntries,
              uint8_t bSync );

/**
 * soc_sbx_g2p3_counters_drop_clear()
 *
 * @brief
 *
 * Clear per-port drop counters
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Egress Counter Index
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_drop_clear( soc_sbx_g2p3_state_t *pFe,
               uint32_t ulIndex,
               uint32_t ulEntries,
               uint8_t bSync );


/**
 * soc_sbx_g2p3_counters_read()
 *
 * @brief
 *
 * Read a counter from memory
 *
 * @param pFe       - Driver control structure
 * @param eSegment  - Memory segment to read from
 * @param ulOffset  - Starting offset of entry(s) to read
 * @param ulEntries - Number of entries to read (in byte/packet pairs)
 * @param ullData   - Pointer to array for return results. Byte counts are
 *                    returned on even addresses, Packet counts on odd addresses.
 *                      ullData[0] = Offset 0 Byte Counter
 *                      ullData[1] = Offset 0 Packet Counter
 *                      ullData[2] = Offset 1 Byte Counter
 *                      ullData[3] = Offset 1 Packet Counter
 * @param bSync     - Force flush synchronization
 * @param bClear    - If set, clear counter after read
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_read(soc_sbx_g2p3_state_t *pFe,
                           int eSegment,
                           uint32_t ulOffset,
                           uint32_t ulEntries,
                           uint64_t *ullData,
                           uint8_t bSync,
                           uint8_t bClear);

/**
 * soc_sbx_g2p3_counter_segment_enable()
 *
 * @brief
 *
 * Enable or disable a segment
 *
 * @param pFe       - Driver control structure
 * @param ulSegment - Segment Id (0 -> 31)
 * @param bEnable   - Enable = 1, Disable = 0
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counter_segment_enable(soc_sbx_g2p3_state_t *pFe,
                                    uint32_t ulSegment,
                                    uint8_t bEnable);

/**
 * soc_sbx_g2p3_counter_segment_flush()
 *
 * @brief
 *
 * Enable or disable flushing (Forced Counter Ejection) on a specified
 *        segment. When this is enabled, the counter manager will cycle through
 *        a segment and serially eject counters for handling. When it reaches
 *        the end of a segment, it will wrap and continue.
 *
 * @param pFe        - Driver control structure
 * @param ulSegment  - Segment to perform flushing on
 * @param bEnable    - Enable or disable ejection on this segment
 * @param ulRateData - Configure rate of counter ejection.
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counter_segment_flush(soc_sbx_g2p3_state_t *pFe,
                                   uint32_t ulSegment,
                                   uint8_t bEnable,
                                   uint32_t ulRateData);

/**
 * soc_sbx_g2p3_counter_segment_flush_status()
 *
 * @brief
 *  Return current index in flush progress for a segment. If no flush is
 *        is in progress, return BUSY error code.
 *
 * @param pFe       - Driver control structure
 * @param ulSegment - Segment performing flushing
 * @param ulCntrId  - Current location in segment space
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counter_segment_flush_status(soc_sbx_g2p3_state_t *pFe,
                                          uint32_t ulSegment,
                                          uint32_t *ulCntrId);

/**
 * soc_sbx_g2p3_counter_force_flush()
 *
 * @brief
 *  Request a manual flush of a specified counter or counters to the host
 *        for handling. Currently, the hardware only supports ranges (n -> 0)
 *        but we should be adding support for individual counters.
 *
 * @param pFe       - Driver control structure
 * @param ulSegment - Segment to perform the forced flush on
 * @param ulOffset  - Offset within that segment
 * @param ulSize    - Number of entries to flush
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counter_force_flush(soc_sbx_g2p3_state_t *pFe,
                                 uint32_t ulSegment,
                                 uint32_t ulOffset,
                                 uint32_t ulSize);

sbStatus_t
soc_sbx_g2p3_counters_qos_rule_read(soc_sbx_g2p3_state_t *pFe,
                                    uint32_t ulIndex,
                                    soc_sbx_g2p3_count_t *pCount,
                                    uint32_t ulEntries,
                                    uint8_t bSync);

sbStatus_t
soc_sbx_g2p3_counters_qos_rule_clear(soc_sbx_g2p3_state_t *pFe,
                                     uint32_t ulIndex,
                                     uint32_t ulEntries,
                                     uint8_t bSync);

/**
 * soc_sbx_g2p3_counters_rtegr_read()
 *
 * @brief
 *
 * Read Egress counters from memory
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Counter Index
 * @param pCount    - Pointer to src/dst counter structures
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_egr_rule_read(soc_sbx_g2p3_state_t *pFe,
                          uint32_t ulIndex,
                          soc_sbx_g2p3_count_t *pCount,
                          uint32_t ulEntries,
                          uint8_t bSync);

/**
 * soc_sbx_g2p3_counters_rtsec_clear()
 *
 * @brief
 *
 * Clear Egress counters
 *
 * @param pFe       - Driver control structure
 * @param ulIndex   - Counter Index
 * @param ulEntries - Number of entries to read
 * @param bSync     - Force flush synchronization
 *
 * @return - error code, SB_OK on success
 **/
sbStatus_t
soc_sbx_g2p3_counters_egr_rule_clear(soc_sbx_g2p3_state_t *pFe,
                           uint32_t ulIndex,
                           uint32_t ulEntries,
                           uint8_t bSync);


#endif /* _SOC_SBX_G2P3_COUNTERS_H_ */
