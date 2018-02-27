/* -*- Mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
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
 * $Id: sbFe2000Common.h 1.13.60.1 Broadcom SDK $
 *
 * sbFe2000Common.h : FE2000 Common defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SB_FE_2000_COMMON_H_
#define _SB_FE_2000_COMMON_H_

#include <soc/sbx/sbStatus.h>
#include <soc/sbx/sbTypes.h>

#if defined(DENTER)
#undef DENTER
#endif

#if defined(DEXIT)
#undef DEXIT
#endif

#if defined (DEBUG_PRINT)
#define DENTER() SB_LOG("%s: enter\n", __PRETTY_FUNCTION__);
#define DEXIT()  SB_LOG("%s: line: %d:  exit\n", __PRETTY_FUNCTION__, __LINE__);

#else
#define DENTER()
#define DEXIT()
#endif

#define BCM_SBX_MAX(x,y)                (((x)>=(y))?(x):(y))

#define SB_FE2000_MIN_RATE_BPS 1000000
#define SB_FE2000_100M_RATE_BPS ( SB_FE2000_MIN_RATE_BPS * 100 )
#define SB_FE2000_1G_RATE_BPS ( SB_FE2000_MIN_RATE_BPS * 1000 )
#define SB_FE2000_SIMPLEX_INGRESS_RATE_BPS ( SB_FE2000_1G_RATE_BPS * 24 )
#define SB_FE2000_MAX_SPI_RATE_BPS ( SB_FE2000_1G_RATE_BPS * 16 )

#define SB_FE2000_MAX_SPI_RATE_GBPS 16
#define SB_FE2000_MAX_AG_RATE_GBPS 12
#define SB_FE2000_MAX_XG_RATE_GBPS 10

#define SB_FE2000_SIMPLEX_INGRESS_RATE_GBPS 24

#define SB_FE2000_NUM_SPI_INTERFACES 2
#define SB_FE2000_NUM_XG_INTERFACES 2
#define SB_FE2000_NUM_AG_INTERFACES 2
#define SB_FE2000_MAX_AG_PORTS 12
#define SB_FE2000_MAX_XG_PORTS 1
#define SB_FE2000_MAX_COMBINED_SPI_PORTS 65
#define SB_FE2000_MAX_PORTS_PER_SPI 64
#define SB_FE2000_MAX_CALENDAR_LENGTH 64

#define SB_FE2000_MAX_HG_SUBPORTS 50

#define SB_FE2000_NUM_MM_INSTANCES 2
#define SB_FE2000_NUM_RC_INSTANCES 2
#define SB_FE2000_MAX_QUEUES 256

#define SB_FE2000_QM_INIT_TIMEOUT 100
#define SB_FE2000_QM_FLOW_CONTROL_TARGET_PORT 0
#define SB_FE2000_QM_FLOW_CONTROL_TARGET_QUEUE 1

#define SB_FE2000_PP_SB_ROUTE_HDR_TYPE             0
#define SB_FE2000_PP_PPP_HDR_TYPE                  1
#define SB_FE2000_PP_ETHERNET_HDR_TYPE             2
#define SB_FE2000_PP_LLC_HDR_TYPE                  3
#define SB_FE2000_PP_SNAP_HDR_TYPE                 4
#define SB_FE2000_PP_VLAN_HDR_TYPE                 5
#define SB_FE2000_PP_MPLS_HDR_TYPE                 6
#define SB_FE2000_PP_IPV4_HDR_TYPE                 7
#define SB_FE2000_PP_IPV6_HDR_TYPE                 8
#define SB_FE2000_PP_GRE_HDR_TYPE                  9
#define SB_FE2000_PP_TCP_HDR_TYPE                  10
#define SB_FE2000_PP_UDP_HDR_TYPE                  11
#define SB_FE2000_PP_HIGIG_HDR_TYPE                12

#define SB_FE2000_PD_NUM_HEADER_CONFIGS    15
#define SB_FE2000_PD_MAX_CAPTURE_HEADERS   16

#define SB_FE2000_MM_NUM_MM_INSTANCES         2
#define SB_FE2000_MM_MAX_NUM_MEMORY_SEGMENTS 15

#define ZCCA_ALL_MEM_TEST_ADDRESSES 0
#define ZCCA_RANDOM_MEM_TEST_ADDRESSES 1
#define ZCCA_SMART_MEM_TEST_ADDRESSES 2
#define ZCCA_REGRESS_MEM_TEST_ADDRESSES 3

#define SB_FE2000_UNIMAC_SP_10   0x0
#define SB_FE2000_UNIMAC_SP_100  0x1
#define SB_FE2000_UNIMAC_SP_1000 0x2

#define SB_FE2000_IIC_CHIP_ADDR_REG 0x0
#define SB_FE2000_IIC_DATA_IN0_REG  0x1
#define SB_FE2000_IIC_DATA_IN1_REG  0x2
#define SB_FE2000_IIC_DATA_IN2_REG  0x3
#define SB_FE2000_IIC_DATA_IN3_REG  0x4
#define SB_FE2000_IIC_DATA_IN4_REG  0x5
#define SB_FE2000_IIC_DATA_IN5_REG  0x6
#define SB_FE2000_IIC_DATA_IN6_REG  0x7
#define SB_FE2000_IIC_DATA_IN7_REG  0x8
#define SB_FE2000_IIC_CNT_REG       0x9
#define SB_FE2000_IIC_CTRL_REG      0xa
#define SB_FE2000_IIC_ENABLE_REG    0xb
#define SB_FE2000_IIC_DATA_OUT0_REG 0xc
#define SB_FE2000_IIC_DATA_OUT1_REG 0xd
#define SB_FE2000_IIC_DATA_OUT2_REG 0xe
#define SB_FE2000_IIC_DATA_OUT3_REG 0xf
#define SB_FE2000_IIC_DATA_OUT4_REG 0x10
#define SB_FE2000_IIC_DATA_OUT5_REG 0x11
#define SB_FE2000_IIC_DATA_OUT6_REG 0x12
#define SB_FE2000_IIC_DATA_OUT7_REG 0x13
#define SB_FE2000_IIC_CTRL_HI_REG   0x14
#define SB_FE2000_IIC_PARAM_REG     0x15

#define SB_FE2000_IIC_WAIT_TIMEOUT 500
#define SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT 500

#define SB_FE2000_MAX_CM_SEGMENTS        32
#define SB_FE2000_NUM_TYPES_COUNTERS     4
#define SB_FE2000_NUM_CMU_MMU_INTERFACES 4
#define SB_FE2000_CM_PCI_EJECT           0
#define SB_FE2000_CM_SRAM_EJECT          1

/* gma - Nov 03 2006 - MMU DLL Lock defines */
#define SB_FE2000_MM_WIDE_PORT_DLL_LOCK_SINGLE_RAM 0x3
#define SB_FE2000_MM_WIDE_PORT_DLL_LOCK_DOUBLE_RAM 0xF
#define SB_FE2000_MM_NARROW_PORT_DLL_LOCK_SINGLE_RAM 0x3
#define SB_FE2000_MM_NARROW_PORT_DLL_LOCK_DOUBLE_RAM 0xF

/* gma - Nov 03 2006 - PMU defines */
#define SB_FE2000_PM_TOTAL_NUM_PROFILES (2 * 1024)
#define SB_FE2000_PM_POLICER_TYPE2_BIT 0
#define SB_FE2000_PM_CHECKER_TIMER_TYPE_BITS 100
#define SB_FE2000_PM_GENERATOR_TYPE_BITS 101
#define SB_FE2000_PM_SEMAPHORE_TYPE_BITS 110
/* rgf - Nov 29 2006 - more PMU defines */
#define SB_FE2000_PM_TOTAL_NUM_GROUPS 9
#define SB_FE2000_PM_NUM_POLICE_ID_BITS 23
#define SB_FE2000_PM_MAX_POLICER_ID 8388607

#define SB_FE2000_PP_INIT_TIMEOUT 100
#define SB_FE2000_PP_NUM_BATCHGROUPS         8

#define SB_FE2000_200_MSEC_K (200000000)
#define SB_FE2000_10_USEC_K  (10*1E3)

#define SB_FE2000_CM_MM_INT_MEM_ADDR_LOCATIONS    (16 * 1024)   /* gma - Oct 30 2006 - 8kx72 -> 16kx36 */
#define SB_FE2000_CM_MM_NARROW_MEM_ADDR_LOCATIONS (8 * 1000000) /* gma - Oct 30 2006 - up to 32MB -> 8Mx36 */

#define SB_FE2000_AG_MAC_CRC_APPEND      0x0
#define SB_FE2000_AG_MAC_CRC_KEEP        0x1
#define SB_FE2000_AG_MAC_CRC_REPLACE     0x2
#define SB_FE2000_AG_MAC_CRC_PER_PACKET  0x3

#define SB_FE2000_XG_MAC_CRC_APPEND      0x0
#define SB_FE2000_XG_MAC_CRC_KEEP        0x1
#define SB_FE2000_XG_MAC_CRC_REPLACE     0x2
#define SB_FE2000_XG_MAC_CRC_PER_PACKET  0x3

typedef struct _sbFe2000InitParamsPci {
    uint32_t bBringUp;
    uint32_t bDoSoftReset;
} sbFe2000InitParamsPci_t;

typedef struct _sbFe2000InitParamsSpiRx {
  uint8_t bBringUp;
  int32_t nAlignDelay;

  /* jts - Apr 06 2006 - NOTE: port enables will be DERIVED from the calendar */
  /* jts - Apr 06 2006 - that is, if a port is listed in the calendar we will */
  /* jts - Apr 06 2006 - enable it. */
  int32_t nCalendarLength; /**< Length of the calendar, range: [1,64] */
  int32_t nCalendarM; /**< Number of times to repeat before DIP2, range: [1,255]*/
  int32_t nCalendarEntry[SB_FE2000_MAX_CALENDAR_LENGTH];

  /* jts - Apr 06 2006 - the minimum and maximum frame size (in bytes) that */
  /* jts - Apr 06 2006 - will be allowed, configurable PER-port */
  int32_t nMinFrameSize[SB_FE2000_MAX_PORTS_PER_SPI];
  int32_t nMaxFrameSize[SB_FE2000_MAX_PORTS_PER_SPI];

  int32_t nMaxBurst1;
  int32_t nMaxBurst2;
  int32_t nRSclkEdge;

  uint8_t bLoopback;
  int32_t nFifoLines[SB_FE2000_MAX_PORTS_PER_SPI];
} sbFe2000InitParamsSpiRx_t;

typedef struct _sbFe2000InitParamsSpiTx {
  uint8_t bBringUp;
  int32_t nAlignDelay;

  /* jts - Apr 06 2006 - NOTE: port enables will be DERIVED from the calendar */
  /* jts - Apr 06 2006 - that is, if a port is listed in the calendar we will */
  /* jts - Apr 06 2006 - enable it. */
  int32_t nCalendarLength;
  int32_t nCalendarM; /**< Number of times FIFO status sent before recv DIP2, range: [1,255]*/
  int32_t nCalendarEntry[SB_FE2000_MAX_CALENDAR_LENGTH];
  int32_t nMaxBurst1;
  int32_t nMaxBurst2;
  int32_t nTSclkEdge;
  
  uint8_t bLoopback;
  int32_t nFifoLines[SB_FE2000_MAX_PORTS_PER_SPI];
} sbFe2000InitParamsSpiTx_t;

typedef enum _sbFe2000InitParamsPrPipelinePriority
{
  SB_FE2000_PR_PIPELINE_LOWEST_PRIORITY   = 0,
  SB_FE2000_PR_PIPELINE_EQUAL_TO_ONE_PRE  = 1,
  SB_FE2000_PR_PIPELINE_EQUAL_TO_ALL_PRES = 2,
  SB_FE2000_PR_PIPELINE_HIGHEST_PRIORITY  = 3
} sbFe2000InitParamsPrPipelinePriority_t;

typedef enum _sbFe2000InitParamsPtPipelinePriority
{
  SB_FE2000_PT_PIPELINE_LOWEST_PRIORITY   = 0,
  SB_FE2000_PT_PIPELINE_EQUAL_TO_ONE_PTE  = 1,
  SB_FE2000_PT_PIPELINE_EQUAL_TO_ALL_PTES = 2,
  SB_FE2000_PT_PIPELINE_HIGHEST_PRIORITY  = 3
} sbFe2000InitParamsPtPipelinePriority_t;

typedef struct _sbFe2000InitParamsPpGlobalStationMatch
{
    uint64_t uuAddress;
    uint64_t uuMask;
} sbFe2000InitParamsPpGlobalStationMatch_t;

typedef struct _sbFe2000InitParamsPpPortStationMatch
{
    uint64_t uuDmac;
    uint8_t bDmacMask;
    uint8_t bDmacConcat;
    uint8_t bPortDmacEnable;
    uint8_t uGlobalDmacEnable;
} sbFe2000InitParamsPpPortStationMatch_t;

typedef struct _sbFe2000InitParamsPpIpv4Filter
{
    uint32_t uAddress;
    uint32_t uMask;
} sbFe2000InitParamsPpIpv4Filter_t;

typedef struct _sbFe2000InitParamsPpIpv6Filter
{
    uint64_t uuAddress;
    uint64_t uuMask;
} sbFe2000InitParamsPpIpv6Filter_t;

#if 0 /* obsolete type */
typedef struct _sbFe2000InitParamsPpNativeMapFilter
{
    uint32_t uSaveOperation;
    uint32_t uSaveFilterId;
} sbFe2000InitParamsPpNativeMapFilter_t;
#endif
typedef struct _sbFe2000InitParamsPpDebugMode
{
    uint8_t bByPassLrpMode[SB_FE2000_PP_NUM_BATCHGROUPS];
    uint32_t uMirrorIndex;
    uint32_t uCopyCount;
    uint8_t bHeaderCopy;
    uint8_t bDrop;
} sbFe2000InitParamsPpDebugMode_t;

typedef struct _sbFe2000InitParamsPpQueueConfiguration
{
    uint32_t uPriority;
    uint32_t uBatchGroup;
} sbFe2000InitParamsPpQueueConfiguration_t;

typedef struct _sbFe2000InitParamsPdHeaderConfig
{
    uint32_t uBaseLength;
    uint32_t uLengthPosition;
    uint32_t uLengthSize;
    uint32_t uLengthUnits;
} sbFe2000InitParamsPdHeaderConfig_t;

typedef struct _sbFe2000InitParamsPd {

    uint8_t bBringUp;
    uint8_t bDebug;
    uint8_t bRouteHeaderPresent;
    uint8_t bDoIpv4CheckSumUpdate;
    uint8_t bDoConditionalIpv4CheckSumUpdate;
    uint8_t bDoContinueByteAdjust;
    uint32_t uTruncationRemoveValue;
    uint32_t uTruncationLengthValue;
    sbFe2000InitParamsPdHeaderConfig_t HeaderConfig[SB_FE2000_PD_NUM_HEADER_CONFIGS];

} sbFe2000InitParamsPd_t;

typedef enum _sbFe2000InitParamsCmCounterTypes
{
  SB_FE2000_CM_TURBO_COUNTER          = 2,
  SB_FE2000_CM_RANGE_TRACKER          = 3,
  SB_FE2000_CM_LEGACY_COUNTER         = 0,
  SB_FE2000_CM_CHAINED_LEGACY_COUNTER = 1
} sbFe2000InitParamsCmCounterTypes_t;

typedef struct _sbFe2000InitParamsCmSegment {

  uint8_t bEnable;
  uint8_t bEnableAutoFlush;

  sbFe2000InitParamsCmCounterTypes_t counterType;

  uint32_t uBank;
  uint32_t uEject;
  uint32_t uBaseAddr;
  uint32_t uLimit;
  uint32_t uAutoFlushRate;

} sbFe2000InitParamsCmSegment_t;

typedef struct _sbeFe2000InitParamsCm {

  uint8_t bBringUp;
  uint32_t uNumRingElements;  /* Number of elements in the counter ring. Must be a power of 2 */
  uint32_t uCounterRingThresh; /* When reached the counter_fif_data_avail int is generated */

  uint32_t uCmDmaThresh; /* DMA interface backpressure threshold. */

  sbFe2000InitParamsCmSegment_t segment[SB_FE2000_MAX_CM_SEGMENTS];

} sbFe2000InitParamsCm_t;

typedef struct _sbFe2000InitParamsRc {

    uint8_t bBringUp;

} sbFe2000InitParamsRc_t;

/* rgf - Nov 29 2006 - Group 9 (index 8) only has the id range and no refreshes */
/* rgf - Nov 29 2006 - or timestamp/timer configuration */
typedef struct _sbFe2000InitParamsPmGroupDef {
  uint8_t   bEnable; /* rgf - Nov 29 2006 - If not enabled, no refreshes for this group and */
  /* rgf - Nov 29 2006 - policer ids will not match to the group */
  uint32_t   uPolicerIdMinimum; /* rgf - Nov 29 2006 - Range of policer ids which belong to the group */
  uint32_t   uPolicerIdMaximum;

  /* rgf - Nov 29 2006 - Define background refresh behavior for the group.  Period+1 clock */
  /* rgf - Nov 29 2006 - cycles it will issue count number of refresh requests. */
  /* rgf - Nov 29 2006 - If the backlog for this group alone exceeds the threshold, an */
  /* rgf - Nov 29 2006 - error/interrupt will be issued. */
  uint32_t   uRefreshPeriod;
  uint32_t   uRefreshCount;
  uint32_t   uRefreshThreshold;

  /* rgf - Nov 29 2006 - Define the timestamp placement and timer granularity for the  */
  /* rgf - Nov 29 2006 - group.  Policers/meters within the group will have CIR/EIR */
  /* rgf - Nov 29 2006 - tokens accumulated every timer period + 1 clock cycles. */
  /* rgf - Nov 29 2006 - Policer state (64b) is composed of an 11b profile id and other */
  /* rgf - Nov 29 2006 - data.  In the case of a meter, the remaining 53b are configurable */
  /* rgf - Nov 29 2006 - between timestamp, BktE, and BktC.  Timestamp is in the msb's */
  /* rgf - Nov 29 2006 - of the 53b starting at the timestamp offset from the group. */
  /* rgf - Nov 29 2006 - Maximum value is therefore 52, 1b allocated to the timestamp */
  /* rgf - Nov 29 2006 - (probably not desirable or useful).  The number of bits for */
  /* rgf - Nov 29 2006 - BktC is defined by the policer profile which along with the */
  /* rgf - Nov 29 2006 - timestamp offset implicitly defines the bits for BktE. */
  uint8_t bTimerEnable;
  uint32_t uTimestampOffset;
  uint32_t uTimerPeriod;
} sbFe2000InitParamsPmGroupDef_t;

typedef enum _sbFe2000InitParamsPmType
{
  SB_FE2000_PM_POLICER       = 0,
  SB_FE2000_PM_CHECKER_TIMER = 1,
  SB_FE2000_PM_GENERATOR     = 2,
  SB_FE2000_PM_SEMAPHORE     = 3
} sbFe2000InitParamsPmType_t;

typedef struct _sbFe2000InitParamsPmProfile {
  sbFe2000InitParamsPmType_t profileType;

  uint8_t bEnable;

  /* gma - Nov 03 2006 - This is config information relative to a Policier */
  /* gma - Nov 03 2006 - */
  uint8_t bBlind;
  uint8_t bDropOnRed;
  uint8_t bCpFlag;    /* gma - Nov 03 2006 - Coupling Flag */
  uint8_t bRFC2698;
  uint8_t bBktCNoDec;
  uint8_t bBktENoDec;

  uint32_t uBktCSize;  /* gma - Nov 03 2006 - Commited Bucket Size */
  uint32_t uCBS;       /* gma - Nov 03 2006 - Commited Burst Size */
  uint32_t uCIRBytes;  /* gma - Nov 03 2006 - Commited Information Rate - Bytes per group timer period */
  uint32_t uEBS;       /* gma - Nov 03 2006 - Excess Burst Size */
  uint32_t uEIRBytes;  /* gma - Nov 03 2006 - Excess Information Rate - Bytes per group timer period */
  uint32_t uLenShift;
  /* gma - Nov 03 2006 - End Policier Config Info */

  /* gma - Nov 03 2006 - This is config information relative to a Checker/Timer */
  /* gma - Nov 03 2006 - */
  uint8_t bInterrupt;
  uint8_t bReset;
  uint8_t bStrict;
  uint8_t bMode32;

  uint32_t uDeadline;
  /* gma - Nov 03 2006 - End Checker/Timer Config Info */

} sbFe2000InitParamsPmProfile_t;

typedef struct _sbFe2000InitParamsPm {

  uint8_t bBringUp;
  /* rgf - Nov 29 2006 - Values greater than 22 cause the LSB of this value to select  */
  /* rgf - Nov 29 2006 - which MMU holds the policer state data for all policer ids.   */
  /* rgf - Nov 29 2006 - Values 22 or less specify that bit in the policer id will  */
  /* rgf - Nov 29 2006 - select the MMU and only the LSBs below that bit will be used */
  /* rgf - Nov 29 2006 - to form the actual policer id used to index into MMU memory. */
  /* rgf - Nov 29 2006 - Max valid value 31. */
  uint32_t uMmuSelectBit;

  /* rgf - Nov 29 2006 - Enable background refreshes and establish the overall  */
  /* rgf - Nov 29 2006 - backlog threshold which will trigger an error. */
  uint8_t bBackgroundRefreshEnable;
  uint32_t uTotalRefreshThreshold;

  sbFe2000InitParamsPmGroupDef_t group[SB_FE2000_PM_TOTAL_NUM_GROUPS];
  sbFe2000InitParamsPmProfile_t profile[SB_FE2000_PM_TOTAL_NUM_PROFILES];

} sbFe2000InitParamsPm_t;


typedef void (*sbFe2000ResetSramDllFunc_t)(sbhandle sbh);
typedef void (*sbFe2000TrainDdrFunc_t)(sbhandle sbh);

typedef enum sbFe2000InterfacePortType_e_s {
  SB_FE2000_IF_PTYPE_SPI0 = 0,
  SB_FE2000_IF_PTYPE_SPI1,
  SB_FE2000_IF_PTYPE_AGM0, /* also XGM2 */
  SB_FE2000_IF_PTYPE_AGM1, /* also XGM3 */
  SB_FE2000_IF_PTYPE_XGM0,
  SB_FE2000_IF_PTYPE_XGM1,
  SB_FE2000_IF_PTYPE_PCI,
  /* leave as last */
  SB_FE2000_IF_PTYPE_MAX
} sbFe2000InterfacePortType_e_t;

typedef struct sbFe2000Port_s {
/** @brief <p> Defines the interface port type </p> */
  unsigned int ulInterfacePortType;
/** @brief <p> Defines portId associated with the InterfacePortType </p> */
  unsigned int ulPortId;
} sbFe2000Port_t;

/** @brief QueuesConnectionDescription structure
 *
 */
typedef struct sbFe2000Connection_s {
  /** @brief <p> microcode port # </p> */
  unsigned ulUcodePort;
  /** @brief <p> microcode processing function (ingress or egress) */
  unsigned bEgress;
  /** @brief <p> source port */
  sbFe2000Port_t from;
  /** @brief <p> destination port */
  sbFe2000Port_t to;
} sbFe2000Connection_t;

#define SB_FE2000_SWS_INIT_MAX_CONNECTIONS 128
#define SB_FE2000_SWS_UNINITIALIZED_CON    -1
#define SB_FE2000_SWS_UNINITIALIZED_QID    -1

typedef struct sbFe2000Queues_s {
    int n;
    sbFe2000Connection_t connections[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int bw[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int conbybw[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int fromqid[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int toqid[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int qid2con[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int many2onecon[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int one2manycon[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int pbsel[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int minfrompages[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int maxfrompages[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int mintopages[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int maxtopages[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int fromflowthresh[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int toflowthresh[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int port2iqid[SB_FE2000_SWS_INIT_MAX_CONNECTIONS/2];
    int port2eqid[SB_FE2000_SWS_INIT_MAX_CONNECTIONS/2];
    int batchAssigned[SB_FE2000_SWS_INIT_MAX_CONNECTIONS];
    int if2con[SB_FE2000_IF_PTYPE_MAX][SB_FE2000_MAX_PORTS_PER_SPI][2];
    int ifports[SB_FE2000_IF_PTYPE_MAX];
    int ifi2con[SB_FE2000_IF_PTYPE_MAX][SB_FE2000_MAX_PORTS_PER_SPI]; /*OBSOLETE*/
    int ifexpmode[SB_FE2000_IF_PTYPE_MAX];
    int pb0pre[SB_FE2000_IF_PTYPE_MAX];
    int pb1pre[SB_FE2000_IF_PTYPE_MAX];
    int mir0qid;
    int mir1qid;
    int topciqid;
    int frompciqid;
} sbFe2000Queues_t;


typedef struct sbFe2000QmParams_s {
  uint32_t threshold;
  uint32_t min_pages;
  uint32_t max_pages;
} sbFe2000QmParams_t;

int sbFe2000SwsConfigCompute(uint32_t hpp_freq, uint32_t epoch, 
                             uint32_t contexts, uint32_t mtu,
                             uint32_t kbits_sec, uint32_t higig,
                             sbFe2000QmParams_t *mac_to_hpp, 
                             sbFe2000QmParams_t *hpp_to_spi,
                             sbFe2000QmParams_t *spi_to_hpp,
                             sbFe2000QmParams_t *hpp_to_mac);

int sbFe2000SwsConfigGet(uint32_t hpp_freq, uint32_t epoch, uint32_t contexts,
                         uint32_t hpp_to_mac_thresh, uint32_t spi_to_hpp_min_pages,
                         uint32_t *mtu, uint32_t *kbits_sec); 

void sbFe2000Reset(sbhandle userDeviceHandle);

uint32_t 
sbFe2000PmProfileMemoryWrite(sbhandle userDeviceHandle, uint32_t uProfileId, uint32_t *puData);

uint32_t
sbFe2000PmPolicerSuppressControl(sbhandle userDeviceHandle,
                                 uint32_t uPolicerId,
                                 uint8_t bEnable);
int
sbFe2000TranslateBlock(int blk_type, int blk_number);

#endif 
