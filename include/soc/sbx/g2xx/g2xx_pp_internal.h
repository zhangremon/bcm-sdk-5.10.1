/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
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
 * $Id: g2xx_pp_internal.h 1.5.108.1 Broadcom SDK $
 *
 *-----------------------------------------------------------------------------*/

#ifndef _SB_G2XX_FE_PP_INTERNAL_H_
#define _SB_G2XX_FE_PP_INTERNAL_H_

#include <soc/sbx/g2xx/g2xx.h>
#include <soc/sbx/g2xx/g2xx_pp_rule_encode.h>

#define SB_G2XX_FE_PP_EVENT0_MASK(sbh, val)     (SAND_HAL_FE2000_FIELD_MASK(sbh, PP_EVENT0_MASK, val##_DISINT))
#define SB_G2XX_FE_PP_EVENT1_MASK(sbh, val)     (SAND_HAL_FE2000_FIELD_MASK(sbh, PP_EVENT1_MASK, val##_DISINT))

#define SB_ZF_G2XX_FE_PP_CAMRAMCFG_SIZE_IN_BYTES 20
#define SB_ZF_G2XX_FE_PP_CAMRAMCFG_SIZE_IN_WORDS ((SB_ZF_G2XX_FE_PP_CAMRAMCFG_SIZE_IN_BYTES+3)/4)

#define SB_ZF_G2XX_FE_PP_CAMCFG_SIZE_IN_BYTES 48
#define SB_ZF_G2XX_FE_PP_CAMCFG_SIZE_IN_WORDS   ((SB_ZF_G2XX_FE_PP_CAMCFG_SIZE_IN_BYTES+3)/4)

#define SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SIZE_IN_WORDS \
      ((SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SIZE_IN_BYTES+3)/4)
#define SB_ZF_CAPPCAMCONFIGURATIONENTRY_SIZE_IN_WORDS \
      ((SB_ZF_CAPPCAMCONFIGURATIONENTRY_SIZE_IN_BYTES+3)/4)

#define SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS \
      ((SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_BYTES+3)/4)

#define SB_ZF_C2PPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS \
      ((SB_ZF_C2PPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_BYTES+3)/4)
/**
 * Packet Parsing Engine 
 */


#define SB_G2XX_FE_PP_IPV4_VER                     4
#define SB_G2XX_FE_PP_IPV6_VER                     6
#define SB_G2XX_FE_PP_IPV4_NOOPT_IHL               5

/* IANA Ethernet Protocol Types */
#define SB_G2XX_FE_PP_ETH_TYPE_IPV4                (0x0800)
#define SB_G2XX_FE_PP_ETH_TYPE_IPV6                (0x86DD)
#define SB_G2XX_FE_PP_ETH_TYPE_PPP                 (0x880B)
#define SB_G2XX_FE_PP_ETH_TYPE_MIM                 (0x8888)
#define SB_G2XX_FE_PP_ETH_TYPE_MPLS_UNICAST        (0x8847)
#define SB_G2XX_FE_PP_ETH_TYPE_MPLS_MCAST          (0x8848)
#define SB_G2XX_FE_PP_ETH_TYPE_PAYLOAD             (0x6000)
#define SB_G2XX_FE_PP_ETH_TYPE_LINK_OAM            (0x8809)
#define SB_G2XX_FE_PP_ETH_TYPE_SER_OAM             (0x8902)


/* IANA IPv4 Protocol Numbers */
#define SB_G2XX_FE_PP_IPV4_PID_IGMP                2
#define SB_G2XX_FE_PP_IPV4_PID_IP                  4
#define SB_G2XX_FE_PP_IPV4_PID_TCP                 6
#define SB_G2XX_FE_PP_IPV4_PID_UDP                 17
#define SB_G2XX_FE_PP_IPV4_PID_IPV6                41
#define SB_G2XX_FE_PP_IPV4_PID_GRE                 47
#define SB_G2XX_FE_PP_IPV4_PID_ETH                 97 /* RFC 1700 */
#define SB_G2XX_FE_PP_IPV4_PID_PIM                 103

/* IANA IPv6 Protocol Numbers */
#define SB_G2XX_FE_PP_IPV6_PID_HOPOPT              0
#define SB_G2XX_FE_PP_IPV6_PID_IGMP                2
#define SB_G2XX_FE_PP_IPV6_PID_IP                  4
#define SB_G2XX_FE_PP_IPV6_PID_IPV4                4
#define SB_G2XX_FE_PP_IPV6_PID_IPV6                41
#define SB_G2XX_FE_PP_IPV6_PID_TCP                 6
#define SB_G2XX_FE_PP_IPV6_PID_UDP                 17
#define SB_G2XX_FE_PP_IPV6_PID_IPV6                41
#define SB_G2XX_FE_PP_IPV6_PID_FRAG                44
#define SB_G2XX_FE_PP_IPV6_PID_GRE                 47
#define SB_G2XX_FE_PP_IPV6_PID_ETH                 97 /* RFC 1700 */
#define SB_G2XX_FE_PP_IPV6_PID_PIM                 103

/* MPLS Stack Encoding RFC 3032 */
#define SB_G2XX_FE_PP_MPLS_V4_NULL_LABEL            0
#define SB_G2XX_FE_PP_MPLS_ROUTE_ALERT_LABEL        1
#define SB_G2XX_FE_PP_MPLS_V6_NULL_LABEL            2
#define SB_G2XX_FE_PP_MPLS_OAM_LABEL                14

/* PP CAM HASH TEMPLATE IDs */
#define SB_G2XX_FE_PP_NUM_BYTE_HASH_TEMPLATES         28
#define SB_G2XX_FE_PP_NUM_BIT_HASH_TEMPLATES          4
#define SB_G2XX_FE_PP_BYTE_HASH_TEMPLATE_OFFSET       4

#define SB_G2XX_FE_PP_VID_HASH_TEMPLATE               0

#define SB_G2XX_FE_PP_BYTE_HASH_TINDEX(X)             (X-4) /* hw reg map */
#define SB_G2XX_FE_PP_ETH_HASH_TEMPLATE               4
#define SB_G2XX_FE_PP_IPV4_HASH_TEMPLATE              5
#define SB_G2XX_FE_PP_TCP_HASH_TEMPLATE               7
#define SB_G2XX_FE_PP_UDP_HASH_TEMPLATE               (SB_G2XX_FE_PP_TCP_HASH_TEMPLATE)
#define SB_G2XX_FE_PP_RTHDR_ETH_HASH_TEMPLATE         8

#define SB_G2XX_FE_PP_ETH_HASH_TEMPLATE_INDEX         (SB_G2XX_FE_PP_ETH_HASH_TEMPLATE-SB_G2XX_FE_PP_BYTE_HASH_TEMPLATE_OFFSET)
#define SB_G2XX_FE_PP_IPV4_HASH_TEMPLATE_INDEX        (SB_G2XX_FE_PP_IPV4_HASH_TEMPLATE-SB_G2XX_FE_PP_BYTE_HASH_TEMPLATE_OFFSET)
#define SB_G2XX_FE_PP_TCP_HASH_TEMPLATE_INDEX         (SB_G2XX_FE_PP_TCP_HASH_TEMPLATE-SB_G2XX_FE_PP_BYTE_HASH_TEMPLATE_OFFSET)
#define SB_G2XX_FE_PP_UDP_HASH_TEMPLATE_INDEX         (SB_G2XX_FE_PP_UDP_HASH_TEMPLATE-SB_G2XX_FE_PP_BYTE_HASH_TEMPLATE_OFFSET)
#define SB_G2XX_FE_PP_RTHDR_ETH_HASH_TEMPLATE_INDEX   (SB_G2XX_FE_PP_RTHDR_ETH_HASH_TEMPLATE-SB_G2XX_FE_PP_BYTE_HASH_TEMPLATE_OFFSET)

#define SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(x)            (((uint64_t) 1) << (39-(x)) ) /* @TODO fix me */
#define SB_G2XX_FE_PP_BIT_HASH_MASK_GEN(x)             (((uint64_t) 1) << (39-(x)) ) /* @TODO fix me 320 bit */

/* CAM configuration macros */
#define SB_G2XX_FE_PP_NUM_OF_CAMS              4
#define SB_G2XX_FE_PP_NUM_OF_CAM_STAGES        8
#define SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM     128 
#define SB_G2XX_FE_PP_NUM_OF_QUEUES            256
#define SB_G2XX_FE_PP_INVALID_CAM_ENTRY        255
#define SB_G2XX_FE_PP_NUM_OF_BATCH_GROUPS       8

/* PP Initialization Parameters */
#define SB_G2XX_FE_PP_DEFAULT_LAGR_SEED            (0xFFFFFFFF)
#define SB_G2XX_FE_PP_DEFAULT_ETYPE_EXCEPTION_MIN  (1501)
#define SB_G2XX_FE_PP_DEFAULT_ETYPE_EXCEPTION_MAX  (1535)
#define SB_G2XX_FE_PP_DEFAULT_PROVIDER_TAG         (0x88a8)
#define SB_G2XX_FE_PP_DEFAULT_CUSTOMER_TAG         (0x8100)
#define SB_G2XX_FE_PP_DEFAULT_INGR_START_QID         0
#define SB_G2XX_FE_PP_DEFAULT_INGR_END_QID           63
#define SB_G2XX_FE_PP_DEFAULT_EGR_START_QID          64
#define SB_G2XX_FE_PP_DEFAULT_EGR_END_QID            128

/* CAM Entry Parameters */
#define SB_G2XX_FE_PP_GLSM_MATCH_START               0
#define SB_G2XX_FE_PP_GLSM_MATCH_END                 15
#define SB_G2XX_FE_PP_PLSM_MATCH_START               16
#define SB_G2XX_FE_PP_PLSM_MATCH_END                 63
#define SB_G2XX_FE_PP_NUM_OF_LSM_MATCH               64

#define SB_G2XX_FE_PP_MAKE_ENTRY_VALID               0x3
#define SB_G2XX_FE_PP_MAKE_ENTRY_INVALID             0x0
#define SB_G2XX_FE_PP_CAM_PAUSE_STATE                (0x00FFFFFF)
#define SB_G2XX_FE_PP_CAM_STATE_MASK                 (0x00FFFF00)
#define SB_G2XX_FE_PP_CAM_SETX_STATE_MASK            (0x00FFFE00)


#define SB_G2XX_FE_PP_CAM0_ID                         0
#define SB_G2XX_FE_PP_CAM1_ID                         1
#define SB_G2XX_FE_PP_CAM2_ID                         2
#define SB_G2XX_FE_PP_CAM3_ID                         3

#define SB_G2XX_FE_PP_LSM_MATCH_CAM_ID                0

/*
 * PP Parsed Header Lengths 
 */
#define SB_G2XX_FE_PP_HDR_LEN_NONE(fe)            0
#define SB_G2XX_FE_PP_HDR_LEN_ETYPE(fe)           2
#define SB_G2XX_FE_PP_HDR_LEN_ETH(fe)             12
#define SB_G2XX_FE_PP_HDR_LEN_IETH(fe)            12
#define SB_G2XX_FE_PP_HDR_LEN_ERH(fe)             (SOC_SBX_G2XX_ERH_LEN(fe))
#define SB_G2XX_FE_PP_HDR_LEN_ERH_ETH(fe)         (SOC_SBX_G2XX_ERH_LEN(fe) + SB_G2XX_FE_PP_HDR_LEN_ETH(fe))
#define SB_G2XX_FE_PP_HDR_LEN_ERH_IETH(fe)        (SOC_SBX_G2XX_ERH_LEN(fe) + SB_G2XX_FE_PP_HDR_LEN_ETH(fe))
#define SB_G2XX_FE_PP_HDR_LEN_VLAN_ETYPE(fe)      6
#define SB_G2XX_FE_PP_HDR_LEN_VLAN_VLAN(fe)       8
#define SB_G2XX_FE_PP_HDR_LEN_ROUTE(fe)           12
#define SB_G2XX_FE_PP_HDR_LEN_VLAN(fe)            4
#define SB_G2XX_FE_PP_HDR_LEN_IPV4(fe)            20 
#define SB_G2XX_FE_PP_HDR_LEN_V4_OPTION(fe)       0 
#define SB_G2XX_FE_PP_HDR_LEN_TCP(fe)             20
#define SB_G2XX_FE_PP_HDR_LEN_UDP(fe)             8
#define SB_G2XX_FE_PP_HDR_LEN_ETH_TYPE(fe)        2
#define SB_G2XX_FE_PP_HDR_LEN_IPV4_DA(fe)         4
#define SB_G2XX_FE_PP_HDR_LEN_IPV4_SA(fe)         4
                                            
/**                                         
 * PP Parsed Header Types                   
 */                                         
#define SB_G2XX_FE_PP_HDR_TYPE_ERH            0x0
#define SB_G2XX_FE_PP_HDR_TYPE_ETYPE          0x1
#define SB_G2XX_FE_PP_HDR_TYPE_IETH           0x2
#define SB_G2XX_FE_PP_HDR_TYPE_VLAN           0x5
#define SB_G2XX_FE_PP_HDR_TYPE_IPV4           0x7
#define SB_G2XX_FE_PP_HDR_TYPE_ELEN           0xE
#define SB_G2XX_FE_PP_HDR_TYPE_UNKNOWN        0xF
                                            
/**                                         
 * PP CAM CHECKERS ID                       
 */                                         
#define SB_G2XX_FE_PP_CHECKER_PPP             0
#define SB_G2XX_FE_PP_CHECKER_GRE             1
#define SB_G2XX_FE_PP_CHECKER_IPV4            2
#define SB_G2XX_FE_PP_CHECKER_IPV6            3
#define SB_G2XX_FE_PP_CHECKER_ETH             4
#define SB_G2XX_FE_PP_CHECKER_VLAN            5
#define SB_G2XX_FE_PP_CHECKER_VLAN_VLAN       6
#define SB_G2XX_FE_PP_CHECKER_VLAN_VLAN_VLAN  7
#define SB_G2XX_FE_PP_CHECKER_MPLS            8
#define SB_G2XX_FE_PP_CHECKER_MPLS_MPLS       9
#define SB_G2XX_FE_PP_CHECKER_MPLS_MPLS_MPLS  10
#define SB_G2XX_FE_PP_CHECKER_TCP             11
#define SB_G2XX_FE_PP_CHECKER_UDP             12
#define SB_G2XX_FE_PP_CHECKER_NONE            15

/**
 * PP CAM HASH TEMPLATE ID
 */
#define SB_G2XX_FE_PP_HASH_TEMPLATE_NONE      0
#define SB_G2XX_FE_PP_HASH_TEMPLATE_VID       0
#define SB_G2XX_FE_PP_HASH_TEMPLATE_ETH       4
#define SB_G2XX_FE_PP_HASH_TEMPLATE_IPV4      5
#define SB_G2XX_FE_PP_HASH_TEMPLATE_IPV6      6
#define SB_G2XX_FE_PP_HASH_TEMPLATE_TCP       7
#define SB_G2XX_FE_PP_HASH_TEMPLATE_UDP       7
#define SB_G2XX_FE_PP_HASH_TEMPLATE_RTHDR_ETH 8
#define SB_G2XX_FE_PP_HASH_TEMPLATE_ERH_ETH   8
                   
/**
 * PP CAM STREAM ID
 */
#define SB_G2XX_FE_PP_STREAM_ID_BRIDGE        1
#define SB_G2XX_FE_PP_STREAM_ID_V4MC          2
#define SB_G2XX_FE_PP_STREAM_ID_V6MC          2
#define SB_G2XX_FE_PP_STREAM_ID_IPV4          5
#define SB_G2XX_FE_PP_STREAM_ID_EGRESS        4
#define SB_G2XX_FE_PP_STREAM_ID_EGR_EXCEPTION 2
#define SB_G2XX_FE_PP_STREAM_ID_EXCEPTION     0
#define SB_G2XX_FE_PP_STREAM_ID_NO_CHANGE    (0x10)



/**
 * @struct for the Hash Byte Template 
 */
typedef struct {
  uint32_t udb[2]; /* only 0-40 bits are valid */
  uint32_t bValid;
} soc_sbx_g2xx_pp_byte_hash_template_t;

/**
 * @struct for the Hash Bit Template 
 */
typedef struct {
  uint32_t udb[10]; /* all 320 bits are valid */
  uint32_t bValid;
  uint32_t fieldMask;
} soc_sbx_g2xx_pp_bit_hash_template_t;

/**
 * @struct for the cam database
 */
typedef struct {
 sbG2XXFePpCamEntry_t cfgdb[SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM];
 sbG2XXFePpRamEntry_t ramdb[SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM];
} soc_sbx_g2xx_pp_cam_db_t;

typedef enum {
  SB_G2XX_FE_PP_TPID_STACK0  = (1 << 0),
  SB_G2XX_FE_PP_TPID_STACK1  = (1 << 1),
  SB_G2XX_FE_PP_OAM_LOOPBACK = (1 << 2)
} soc_sbx_g2xx_pp_initialQueueConfig_t;
/**
 * PP Init Structure
 */

typedef sbG2XXFePpCamEntry_t * sbG2XXFePpCamEntry_p_t;
typedef sbG2XXFePpRamEntry_t * sbG2XXFePpRamEntry_p_t;
typedef sbZfC2PpCamConfigurationInstance0Entry_t         * sbZfC2PpCamConfigurationInstance0Entry_p_t;
typedef sbZfC2PpCamRamConfigurationInstance0Entry_t      * sbZfC2PpCamRamConfigurationInstance0Entry_p_t;
/*
typedef sbZfG2XXFePpInitialQueueState_t * sbZfG2XXFePpInitialQueueState_p_t;
*/
typedef uint32_t                        soc_sbx_g2xx_pp_layer4_filter_db_t;
typedef uint64_t                        soc_sbx_g2xx_pp_ip_tunnel_db_t;
typedef uint64_t                        soc_sbx_g2xx_pp_local_station_match_db_t;

typedef struct {

  soc_sbx_g2xx_pp_cam_db_t          cam[SB_G2XX_FE_PP_NUM_OF_CAMS];

  uint32_t                 uEthTypeExceptionMinValue;
  uint32_t                 uEthTypeExceptionMaxValue;
  uint32_t                 uLinkAggregationSeed;
  uint32_t                 uBatchGroupRecordSize[SB_G2XX_FE_PP_NUM_OF_BATCH_GROUPS];

  uint32_t                 uProviderTag;
  uint32_t                 uCustomerTag;
  uint32_t                 uIngrStartQid;
  uint32_t                 uIngrEndQid;
  uint32_t                 uEgrStartQid;
  uint32_t                 uEgrEndQid;
  uint32_t                 uQueueTpidState[SB_G2XX_FE_PP_NUM_OF_QUEUES];
  uint8_t                  bDebug;

  /* remove the following three */
  soc_sbx_g2xx_pp_ip_tunnel_db_t          IpTunneldb;
  soc_sbx_g2xx_pp_layer4_filter_db_t      Layer4Filterdb;
  soc_sbx_g2xx_pp_local_station_match_db_t LocalStationMatchdb;

  soc_sbx_g2xx_pp_byte_hash_template_t    ByteHashdb[SB_G2XX_FE_PP_NUM_BYTE_HASH_TEMPLATES];
  soc_sbx_g2xx_pp_bit_hash_template_t     BitHashdb[SB_G2XX_FE_PP_NUM_BIT_HASH_TEMPLATES];
} soc_sbx_g2xx_pp_rule_mgr_t;

/* PP Length Extraction Unit configuration */

typedef enum 
{
  SB_G2XX_FE_PP_CRAM_LUNITS_0_BYTES = 0,
  SB_G2XX_FE_PP_CRAM_LUNITS_1_BYTES = 1,
  SB_G2XX_FE_PP_CRAM_LUNITS_4_BYTES = 2,
  SB_G2XX_FE_PP_CRAM_LUNITS_8_BYTES = 3
} soc_sbx_g2xx_pp_length_units_t;

typedef enum 
{
  SB_G2XX_FE_PP_CRAM_LMASK_LOWER_LSB    = 0,
  SB_G2XX_FE_PP_CRAM_LMASK_LOWER_NIBBLE = 3,
  SB_G2XX_FE_PP_CRAM_LMASK_BYTE         = 7,
} soc_sbx_g2xx_pp_length_mask_t;


/**
 * Initialize the Packet Parsing Engine 
 *
 * @param pFe    Pointer to soc_sbx_g2xx_state_t struct
 * @return       Return Staus of type sbStatus_t
 */

sbStatus_t 
soc_sbx_g2xx_pp_init(int unit, sbCommonConfigParams_p_t cParams, sbIngressConfigParams_p_t iParams);

sbStatus_t 
soc_sbx_g2xx_pp_un_init(soc_sbx_g2xx_state_t *pFe, sbCommonConfigParams_p_t cParams, sbIngressConfigParams_p_t iParams);

sbStatus_t 
soc_sbx_g2xx_pp_hw_init(int unit, sbFeInitAsyncCallback_f_t cb, void *id);

sbStatus_t
soc_sbx_g2xx_pp_ipv6_str_sel_set(int unit, uint32_t valid);

sbStatus_t
soc_sbx_g2xx_pp_ipv6mc_str_sel_set(int unit, uint32_t valid);

/**
 * Set TPID Selector per Port
 *
 * @param pFe         Pointer to the soc_sbx_g2xx_state_t struct
 * @param uPortId     Port number to configure
 * @param bTpidSelect Select for Tag Protocol ID: 0 or 1
 */
sbStatus_t
soc_sbx_g2xx_pp_tpid_select_set(soc_sbx_g2xx_state_t *pFe,
                                uint32_t uPortId,
                                uint8_t bTpidSelect);

sbStatus_t
soc_sbx_g2xx_pp_cam_rule_read(int unit,
                     uint32_t uCamId,
                     uint32_t uCamEntryIndex,
                     sbG2XXFePpCamEntry_t *pCamEntry,
                     sbG2XXFePpRamEntry_t *pRamEntry);

sbStatus_t
soc_sbx_g2xx_pp_cam_rule_write(int unit,
                     uint32_t uCamId,
                     uint32_t uCamEntryIndex,
                     sbG2XXFePpCamEntry_t *pCamEntry,
                     sbG2XXFePpRamEntry_t *pRamEntry);


#endif
