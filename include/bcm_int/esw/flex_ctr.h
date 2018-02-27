/*
 * $Id: flex_ctr.h 1.13.2.8 Broadcom SDK $
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
 * File:        flex_ctr.h
 * Purpose:     Contains all internal function and structure required for
 *              flex counter.
 */

#ifndef __BCM_FLEX_CTR_H__
#define __BCM_FLEX_CTR_H__

#include <sal/core/libc.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/l2x.h>
#include <bcm/l2.h>
#include <bcm/error.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l2.h>
#include <bcm_int/esw/vlan.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/mpls.h>
#include <bcm_int/esw/mim.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/trident.h>

/*
 ADDITIONAL INFO
 Packet attributes are of three types
 1) Packet Attributes                i.e. uncompressed mode
 2) UDF attributes                   i.e. udf mode
 3) Compressed Packet Attributes     i.e. compressed mode
*/

typedef enum bcm_stat_flex_packet_attr_type_e {
    bcmStatFlexPacketAttrTypeUncompressed=0,
    bcmStatFlexPacketAttrTypeCompressed=1,
    bcmStatFlexPacketAttrTypeUdf=2
}bcm_stat_flex_packet_attr_type_t;

/*
 ========================================================================
 Total 12 packets attributes are supported in ingress packet flow direction
 This includes

 1) cng             (2b: Pre FP packet color)
 2) IFP_CNG         (2b: Post FP packet color)
 3) INT_PRI         (4b: Packet internal priority)

 ######################################################################
 ****** In compressed mode, attributes(cng,IFP_CNG,INT_PRI) are
 ****** grouped into PRI_CNG_MAP map table.
 ######################################################################

 4) VLAN_FORMAT     (2b:Incoming PacketInner&Outer Vlan tag present indicators)
 5) OUTER_DOT1P     (3b: Incoming packet outer vlan tag priority.)
 6) INNER_DOT1P     (3b: Incoming packet inner vlan tag priority.)
 ######################################################################
 ****** In compressed mode, attributes(VLAN_FORMAT,OUTER_DOT1P,INNER_DOT1P) are
 ****** grouped into PKT_PRI_MAP map table
 ######################################################################

 7) INGRESS_PORT    (6b: Packet local ingress port.)
 ######################################################################
 ****** In compressed mode, attributes(INGRESS_PORT) are
 ****** grouped into PORT_MAP map table
 ######################################################################

 8) TOS             (8b: Incoming IP packet TOS byte.)
 ######################################################################
 ****** In compressed mode, attributes(TOS) are
 ****** grouped into TOS_MAP map table
 ######################################################################

 9) PACKET_RESOLUTION(6b: IFP packet resolution vector.)
 10)SVP_TYPE         (1b:SVP(SourceVirtualPortNumber)is network port indication)
 11)DROP             (1b: Drop indication.)
 ######################################################################
 ****** In compressed mode, attributes(PACKET_RESOLUTION,SVP_TYPE,DROP) are
 ****** grouped into PKT_RES_MAP map table
 ######################################################################

 12)IP Packet         (1b: Packet is IP.)
 Total 12 Packet attributes with 39 bits
 ========================================================================
*/

typedef struct bcm_stat_flex_ing_pkt_attr_bits_s {
    /*
    Below are default bits for uncompressed format.
    Need to be set for compressed format
    Total Bits :(2+2+4)+(2+3+3)+(6)+(8)+(6+1+1) + (1)
               : 8     + 8     + 6 + 8 + 8      + 1
               : 39 bits
    */
    /* PRI_CNF Group for compressed format */
    uint8   cng;            /* 2 for uncompressed frmt   */
    uint8   cng_mask;

    uint8   ifp_cng;              /* 2 for uncompressed format */
    uint8   ifp_cng_mask;

    uint8   int_pri;              /* 4 for uncompressed format */
    uint8   int_pri_mask;

    /* PKT_PRI vlan Group for compressed format */
    uint8   vlan_format;          /* 2 for uncompressed format */
    uint8   vlan_format_mask;
    uint8   outer_dot1p;          /* 3 for uncompressed format */
    uint8   outer_dot1p_mask;
    uint8   inner_dot1p;          /* 3 for uncompressed format */
    uint8   inner_dot1p_mask;

    /* Port Group for compressed format */
    uint8   ing_port;             /* 6 for uncompressed format */
    uint8   ing_port_mask;

    /* TOS Group for compressed format  */
    uint8   tos;                  /* 8 for uncompressed format */
    uint8   tos_mask;
    /* PKT_RESOLTION Group for compressed format */
    uint8   pkt_resolution;       /* 6 for uncompressed format */
    uint8   pkt_resolution_mask;

    uint8   svp_type;             /* 1 for uncompressed format */
    uint8   svp_type_mask;

    uint8   drop;                 /* 1 for uncompressed format */
    uint8   drop_mask;

    uint8   ip_pkt;               /* 1 for uncompressed format */
    uint8    ip_pkt_mask;
}bcm_stat_flex_ing_pkt_attr_bits_t;


/*
 ========================================================================
 Total 12 packets attributes are supported in egress packet flow direction
 This includes

 1) cng            (2b: Post FP packet color)
 2) INT_PRI        (4b: Packet internal priority)
 ######################################################################
 ****** In compressed mode, attributes(cng,INT_PRI) are
 ****** grouped into PRI_CNG_MAP map table.
 ######################################################################

 3) VLAN_FORMAT    (2b: OutgoingPacketInner&Outer Vlan tag present indicators.)
 4) OUTER_DOT1P    (3b: Incoming packet outer vlan tag priority.)
 5) INNER_DOT1P    (3b: Incoming packet inner vlan tag priority.)
 ######################################################################
 ****** In compressed mode, attributes(VLAN_FORMAT,OUTER_DOT1P,INNER_DOT1P) are
 ****** grouped into PKT_PRI_MAP map table
 ######################################################################

 6) EGRESS_PORT     (6b: Packet local egress port.)
 ######################################################################
 ****** In compressed mode, attributes(EGRESS_PORT) are
 ****** grouped into PORT_MAP map table
 ######################################################################

 7) TOS             (8b: Outgoing IP packet TOS byte.)
 ######################################################################
 ****** In compressed mode, attributes(TOS) are
 ****** grouped into TOS_MAP map table
 ######################################################################

 8) PACKET_RESOLUTION(1b: IFP packet resolution vector.)
 9)SVP_TYPE          (1b: SVP(SourceVirtualPortNumber) is NetworkPortindication)
 10)DVP_TYPE         (1b: DVP(DestinationVirtualPortNumber) is
                          NetworkPortIndication.)
 11)DROP             (1b: Drop indication.)
 ######################################################################
 ****** In compressed mode, attributes(PACKET_RESOLUTION,SVP_TYPE,DVP,DROP) are
 ****** grouped into PKT_RES_MAP map table
 12)IP Packet         (1b: Packet is IP.)
 Total 12 Packet attributes with 33 bits
 ========================================================================
*/
typedef struct bcm_stat_flex_egr_pkt_attr_bits_s {
    /*
    Below are default bits for uncompressed
    format. Need to be set for compressed format
    Total Bits :(2+4)+(2+3+3)+(6)+(8)+(1+1+1+1) + (1)
               : 6     + 8   + 6 + 8 + 4      + 1
               : 33 bits
    */

    /* PRI_CNF Group for compressed format */
    uint8   cng;                  /* 2 for uncompressed frmt   */
    uint8   cng_mask;

    uint8   int_pri;              /* 4 for uncompressed format */
    uint8   int_pri_mask;

    /* PKT_PRI vlan Group for compressed format */
    uint8   vlan_format;         /* 2 for uncompressed format  */
    uint8   vlan_format_mask;

    uint8   outer_dot1p;         /* 3 for uncompressed format  */
    uint8   outer_dot1p_mask;

    uint8   inner_dot1p;         /* 3 for uncompressed format   */
    uint8   inner_dot1p_mask;

    /* Port Group for compressed format */
    uint8   egr_port;            /* 6 for uncompressed format  */
    uint8   egr_port_mask;

    /* TOS Group for compressed format */
    uint8   tos;                 /* 8 for uncompressed format  */
    uint8   tos_mask;

    /* PKT_RESOLTION Group for compressed format               */
    uint8   pkt_resolution;     /* 1 for uncompressed format   */
    uint8   pkt_resolution_mask;

    uint8   svp_type;           /* 1 for uncompressed format   */
    uint8   svp_type_mask;

    uint8   dvp_type;           /* 1 for uncompressed format   */
    uint8   dvp_type_mask;

    uint8   drop;               /* 1 for uncompressed format   */
    uint8   drop_mask;

    uint8   ip_pkt;             /* 1 for uncompressed format   */
    uint8   ip_pkt_mask;
}bcm_stat_flex_egr_pkt_attr_bits_t;

/*
 There are two UDF (UserDefinedFields) chunks each contains one 16-bit UDF bit.
 Each UDF chunk has a valid bit associated with it. UDF attributes are 35-bits
 and contains following fields
 1) UDF0 (16-bits)
 2) UDF1 (16-bits)
 3) UDF_VALID0 (1-bit)
 4) UDF_VALID1 (1-bit)
 5) Drop (1-bit)
 For a packet, one or both of the udf valid bits might be invalid..

 UDF configuration happens separately and for configuring flex-counter we need
 below two udf(16b) masks only
*/


typedef struct bcm_stat_flex_udf_pkt_attr_bits_s {
    uint16  udf0;
    uint16  udf1;
}bcm_stat_flex_udf_pkt_attr_bits_t;

/* For UncompressedIngressMode,PacketAttributes bits are fixed i.e.cng 2 bits */
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_CNG_ATTR_BITS            0x1
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_IFP_CNG_ATTR_BITS        0x2
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS        0x4
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS    0x8
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS    0x10
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS    0x20
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INGRESS_PORT_ATTR_BITS   0x40
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_TOS_ATTR_BITS            0x80
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS 0x100
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS       0x200
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_DROP_ATTR_BITS           0x400
#define BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS         0x800

/* For UncompressedEgressMode,PacketAttributes bits are fixed i.e.cng 2 bits */

#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_CNG_ATTR_BITS            0x1
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS        0x2
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS    0x4
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS    0x8
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS    0x10
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_EGRESS_PORT_ATTR_BITS    0x20
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_TOS_ATTR_BITS            0x40
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS 0x80
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS       0x100
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DVP_TYPE_ATTR_BITS       0x200
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DROP_ATTR_BITS           0x400
#define BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS         0x800

typedef struct bcm_stat_flex_offset_table_entry_s {
    uint8    offset;
    uint8    count_enable;
}bcm_stat_flex_offset_table_entry_t;

typedef struct bcm_stat_flex_ing_uncmprsd_attr_selectors_s {
    uint32                             uncmprsd_attr_bits_selector;
    uint8                              total_counters;
    bcm_stat_flex_offset_table_entry_t offset_table_map[256];
}bcm_stat_flex_ing_uncmprsd_attr_selectors_t;

typedef struct bcm_stat_flex_egr_uncmprsd_attr_selectors_s {
    uint32                             uncmprsd_attr_bits_selector;
    uint8                              total_counters;
    bcm_stat_flex_offset_table_entry_t offset_table_map[256];
}bcm_stat_flex_egr_uncmprsd_attr_selectors_t;


/*
   ###################################################
   Ingress side compressed mode maps
   ###################################################

   PRI_CNG_MAP[2^(cng:2b + ifp_cng:2b +int_pri:4b=8)=256]  *  8
   PKT_PRI_MAP[2^(VLAN_FORMAT:2b + OUTER_DOT1P:3b + INNER_DOT1P:3b=8)=256] * 8
   PORT_MAP[2^(INGRESS_PORT:6b=6)=64] * 8
   TOS_MAP[2^(TOS:8b=8)=256] * 8
   PKT_RES_MAP[2^(PACKET_RESOLUTION:6b + SVP_TYPE:1b + DROP:1b=8)=256]  * 8

*/
typedef uint8 bcm_stat_flex_ing_cmprsd_pri_cnf_attr_map_t[256]; /*256 * 8 */
typedef uint8 bcm_stat_flex_ing_cmprsd_pkt_pri_attr_map_t[256]; /*256 * 8 */
typedef uint8 bcm_stat_flex_ing_cmprsd_port_attr_map_t[64];     /*64  * 8 */
typedef uint8 bcm_stat_flex_ing_cmprsd_tos_attr_map_t[256];     /*256 * 8 */
typedef uint8 bcm_stat_flex_ing_cmprsd_pkt_res_attr_map_t[256]; /*256 * 8 */

typedef struct bcm_stat_flex_ing_cmprsd_attr_selectors_s {
    bcm_stat_flex_ing_pkt_attr_bits_t               pkt_attr_bits;
    bcm_stat_flex_ing_cmprsd_pri_cnf_attr_map_t     pri_cnf_attr_map;
    bcm_stat_flex_ing_cmprsd_pkt_pri_attr_map_t     pkt_pri_attr_map;
    bcm_stat_flex_ing_cmprsd_port_attr_map_t        port_attr_map;
    bcm_stat_flex_ing_cmprsd_tos_attr_map_t         tos_attr_map;
    bcm_stat_flex_ing_cmprsd_pkt_res_attr_map_t     pkt_res_attr_map;
    uint8                                           total_counters;
    bcm_stat_flex_offset_table_entry_t              offset_table_map[256];
}bcm_stat_flex_ing_cmprsd_attr_selectors_t;

/*
   ###################################################
   Egress side compressed mode maps
   ###################################################

   PRI_CNG_MAP[2^(cng:2b + int_pri:4b=6)=64]  *  8
   PKT_PRI_MAP[2^(VLAN_FORMAT:2b + OUTER_DOT1P:3b + INNER_DOT1P:3b=8)=256] * 8
   PORT_MAP[2^(EGRESS_PORT:6b=6)=64] * 8
   TOS_MAP[2^(TOS:8b=8)=256] * 8
   PKT_RES_MAP[2^(PACKET_RESOLUTION:1b +
                  SVP_TYPE:1b +
                  DVP_TYPE:1b +
                  DROP:1b=4)=16]  * 8
*/
typedef uint8 bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t[64];
typedef uint8 bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t[256];
typedef uint8 bcm_stat_flex_egr_cmprsd_port_attr_map_t[64];
typedef uint8 bcm_stat_flex_egr_cmprsd_tos_attr_map_t[256];
typedef uint8 bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t[16];

typedef struct bcm_stat_flex_egr_cmprsd_attr_selectors_s {
    bcm_stat_flex_egr_pkt_attr_bits_t           pkt_attr_bits;
    bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t pri_cnf_attr_map;
    bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t pkt_pri_attr_map;
    bcm_stat_flex_egr_cmprsd_port_attr_map_t    port_attr_map;
    bcm_stat_flex_egr_cmprsd_tos_attr_map_t     tos_attr_map;
    bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t pkt_res_attr_map;
    uint8                                       total_counters;
    bcm_stat_flex_offset_table_entry_t          offset_table_map[256];
}bcm_stat_flex_egr_cmprsd_attr_selectors_t;

typedef struct bcm_stat_flex_udf_pkt_attr_selectors_s {
    bcm_stat_flex_udf_pkt_attr_bits_t    udf_pkt_attr_bits;
}bcm_stat_flex_udf_pkt_attr_selectors_t;


typedef    struct bcm_stat_flex_ing_attr_s {
    bcm_stat_flex_packet_attr_type_t                packet_attr_type;
    /* union of all possible ingress packet attributes selectors
    i.e. uncompressed,compressed and udf */
    bcm_stat_flex_ing_uncmprsd_attr_selectors_t     uncmprsd_attr_selectors;
    bcm_stat_flex_ing_cmprsd_attr_selectors_t       cmprsd_attr_selectors;
    bcm_stat_flex_udf_pkt_attr_selectors_t          udf_pkt_attr_selectors;
}bcm_stat_flex_ing_attr_t;

typedef    struct bcm_stat_flex_egr_attr_s {
    bcm_stat_flex_packet_attr_type_t                packet_attr_type;
    /* union of all possible egress packet attributes selectors i.e.
    uncompressed,compressed and udf */
    bcm_stat_flex_egr_uncmprsd_attr_selectors_t     uncmprsd_attr_selectors;
    bcm_stat_flex_egr_cmprsd_attr_selectors_t       cmprsd_attr_selectors;
    bcm_stat_flex_udf_pkt_attr_selectors_t          udf_pkt_attr_selectors;
}bcm_stat_flex_egr_attr_t;

/* Flex counter packet flow direction  */

typedef enum bcm_stat_flex_direction_s {
    bcmStatFlexDirectionIngress=0,
    bcmStatFlexDirectionEgress=1
}bcm_stat_flex_direction_t;


typedef    struct bcm_stat_flex_attr_s {
    bcm_stat_flex_direction_t   direction;
    bcm_stat_flex_ing_attr_t    ing_attr;
    bcm_stat_flex_egr_attr_t    egr_attr;
}bcm_stat_flex_attr_t;

/* flex counter mode. */
/* MSB bit will indicate ingress or egress mode. */
typedef uint32  bcm_stat_flex_mode_t;


typedef struct bcm_stat_flex_counter_value_s {
    uint32    pkt_counter_value;
    uint64    byte_counter_value;
}bcm_stat_flex_counter_value_t;

typedef struct bcm_stat_flex_ctr_offset_info_S {
    uint8    all_counters_flag;
    uint32   offset_index; /* Valid only when all_counters_flag is false=0*/
}bcm_stat_flex_ctr_offset_info_t;


/*  FLEX COUNTER COMMON DECLARATION */

#define BCM_STAT_FLEX_COUNTER_MAX_DIRECTION     2
#define BCM_STAT_FLEX_COUNTER_MAX_MODE          4
#define BCM_STAT_FLEX_COUNTER_MAX_POOL          16
#define BCM_STAT_FLEX_COUNTER_MAX_INGRESS_TABLE 9
#define BCM_STAT_FLEX_COUNTER_MAX_EGRESS_TABLE  5
#define BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE   32

#define FLEXCTR_MSG ("%s()[LINE:%d]\n", __FUNCTION__,  __LINE__)
#define FLEXCTR_WARN(stuff)      do { \
                                 BCM_DEBUG(BCM_DBG_WARN, FLEXCTR_MSG); \
                                 BCM_DEBUG(BCM_DBG_WARN, stuff); \
                                 } while(0);
#define FLEXCTR_ERR(stuff)       do { \
                                 BCM_DEBUG(BCM_DBG_ERR, FLEXCTR_MSG); \
                                 BCM_DEBUG(BCM_DBG_ERR, stuff); \
                                 } while(0);
#define FLEXCTR_DBG(flags,stuff) do { \
                                 BCM_DEBUG(flags|BCM_DBG_COUNTER,FLEXCTR_MSG); \
                                 BCM_DEBUG(flags|BCM_DBG_COUNTER, stuff); \
                                 } while(0);
#define FLEXCTR_VERB(stuff)      FLEXCTR_DBG(BCM_DBG_VERBOSE, stuff)
#define FLEXCTR_VVERB(stuff)     FLEXCTR_DBG(BCM_DBG_VVERBOSE, stuff)


typedef struct bcm_stat_flex_table_mode_info_t {
    soc_mem_t            soc_mem_v;
    bcm_stat_flex_mode_t mode;
}bcm_stat_flex_table_mode_info_t;


/* POOL NUMBERS for INGRESS Tables */

#define FLEX_COUNTER_DEFAULT_PORT_TABLE_POOL_NUMBER                        0
#define FLEX_COUNTER_DEFAULT_VFP_POLICY_TABLE_POOL_NUMBER                  1
/* VFI and VLAN sharing same pool id */
#define FLEX_COUNTER_DEFAULT_VFI_TABLE_POOL_NUMBER                         2
#define FLEX_COUNTER_DEFAULT_VLAN_TABLE_POOL_NUMBER                        2

/* VLAN_XLATE and MPLS_TUNNEL_LABEL sharing same pool id */
#define FLEX_COUNTER_DEFAULT_VLAN_XLATE_TABLE_POOL_NUMBER                  3
#define FLEX_COUNTER_DEFAULT_MPLS_ENTRY_TABLE_POOL_NUMBER                  3

/* MPLS_ENTRY_VC_LABEL and VRF can share same pool */
#define FLEX_COUNTER_DEFAULT_VRF_TABLE_POOL_NUMBER                         4

/* Source VP and L3_iif sharing same pool id */
#define FLEX_COUNTER_DEFAULT_L3_IIF_TABLE_POOL_NUMBER                      5
#define FLEX_COUNTER_DEFAULT_SOURCE_VP_TABLE_POOL_NUMBER                   5

/* POOL MASKS for INGRESS Tables */
#define FLEX_COUNTER_POOL_USED_BY_PORT_TABLE                               0x1
#define FLEX_COUNTER_POOL_USED_BY_VFP_POLICY_TABLE                         0x2
#define FLEX_COUNTER_POOL_USED_BY_VLAN_TABLE                               0x4
#define FLEX_COUNTER_POOL_USED_BY_VFI_TABLE                                0x8
#define FLEX_COUNTER_POOL_USED_BY_VLAN_XLATE_TABLE                         0x10
#define FLEX_COUNTER_POOL_USED_BY_MPLS_ENTRY_TABLE                         0x20
#define FLEX_COUNTER_POOL_USED_BY_VRF_TABLE                                0x40
#define FLEX_COUNTER_POOL_USED_BY_L3_IIF_TABLE                             0x80
#define FLEX_COUNTER_POOL_USED_BY_SOURCE_VP_TABLE                          0x100

/* POOL NUMBERS for EGRESS Tables */
#define FLEX_COUNTER_DEFAULT_EGR_L3_NEXT_HOP_TABLE_POOL_NUMBER             0
#define FLEX_COUNTER_DEFAULT_EGR_VFI_TABLE_POOL_NUMBER                     1
#define FLEX_COUNTER_DEFAULT_EGR_PORT_TABLE_POOL_NUMBER                    2
#define FLEX_COUNTER_DEFAULT_EGR_VLAN_TABLE_POOL_NUMBER                    3
#define FLEX_COUNTER_DEFAULT_EGR_VLAN_XLATE_TABLE_POOL_NUMBER              4

/* POOL MASKS for EGRESS Tables */
#define FLEX_COUNTER_POOL_USED_BY_EGR_L3_NEXT_HOP_TABLE                    0x1
#define FLEX_COUNTER_POOL_USED_BY_EGR_VFI_TABLE                            0x2
#define FLEX_COUNTER_POOL_USED_BY_EGR_PORT_TABLE                           0x4
#define FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_TABLE                           0x8
#define FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_XLATE_TABLE                     0x10


#define UNKNOWN_PKT       0x00       /* unknown packet */
#define CONTROL_PKT       0x01       /* pkt(12, 13) == 0x8808 */
#define BPDU_PKT          0x02       /* L2 USER ENTRY table BPDU bit */
#define L2BC_PKT          0x03       /* L2 Broadcast pkt */
#define L2UC_PKT          0x04       /* L2 Unicast pkt */
#define L2DLF_PKT         0x05       /* L2 destination lookup failure */
#define UNKNOWN_IPMC_PKT  0x06       /* Unknown IP Multicast pkt */
#define KNOWN_IPMC_PKT    0x07       /* Known IP Multicast pkt */
#define KNOWN_L2MC_PKT    0x08       /* Known L2 multicast pkt */
#define UNKNOWN_L2MC_PKT  0x09       /* Unknown L2 multicast pkt */
#define KNOWN_L3UC_PKT    0x0a       /* Known L3 Unicast pkt */
#define UNKNOWN_L3UC_PKT  0x0b       /* Unknown L3 Unicast pkt */
#define KNOWN_MPLS_PKT    0x0c       /* Known MPLS pkt */
#define KNOWN_MPLS_L3_PKT 0x0d       /* Known L3 MPLS pkt */
#define KNOWN_MPLS_L2_PKT 0x0e       /* Known L2 MPLS pkt */
#define UNKNOWN_MPLS_PKT  0x0f       /* Unknown MPLS pkt */
#define KNOWN_MIM_PKT     0x10       /* Known MIM pkt */
#define UNKNOWN_MIM_PKT   0x11       /* Unknown MIM pkt */
#define KNOWN_MPLS_MULTICAST_PKT    0x12       /* Known MPLS multicast pkt */


typedef struct  bcm_stat_flex_pool_stat_s {
    uint32  used_by_tables;
    uint32  used_entries;
}bcm_stat_flex_pool_stat_t;

/* ******************************************************************* */
/* counter-id-specific reference count will be added later i.e. after
   basic functionality starts working */
/* ******************************************************************* */
typedef struct  bcm_stat_flex_ingress_mode_s {
    uint32        available;
    uint32        reference_count;
    uint32        total_counters;
    bcm_stat_group_mode_t    group_mode;
    bcm_stat_flex_ing_attr_t ing_attr; 
}bcm_stat_flex_ingress_mode_t;
typedef struct  bcm_stat_flex_egress_mode_s {
    uint32        available;
    uint32        reference_count;
    uint32        total_counters;
    uint32        group_mode;
    bcm_stat_flex_egr_attr_t egr_attr;
}bcm_stat_flex_egress_mode_t;

typedef struct bcm_stat_flex_table_info_s {
    soc_mem_t                 table;
    uint32                    index;
    bcm_stat_flex_direction_t direction;
}bcm_stat_flex_table_info_t;

extern bcm_error_t _bcm_esw_stat_group_create (
                   int                   unit,
                   bcm_stat_object_t     object,
                   bcm_stat_group_mode_t group_mode,
                   uint32                *stat_counter_id,
                   uint32                *num_entries);
extern bcm_error_t _bcm_esw_stat_group_destroy(
                   int    unit,
                   uint32 stat_counter_id);
extern bcm_error_t _bcm_esw_stat_flex_get_ingress_mode_info(
                   int                          unit,
                   bcm_stat_flex_mode_t         mode,
                   bcm_stat_flex_ingress_mode_t *ingress_mode);
extern bcm_error_t _bcm_esw_stat_flex_get_egress_mode_info(
                   int                         unit,
                   bcm_stat_flex_mode_t        mode,
                   bcm_stat_flex_egress_mode_t *egress_mode);
extern bcm_error_t _bcm_esw_stat_flex_get_available_mode(
                   int                       unit,
                   bcm_stat_flex_direction_t direction,
                   bcm_stat_flex_mode_t      *mode);
extern bcm_error_t _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                   int       unit,
                   soc_reg_t pkt_attr_selector_key_reg,
                   uint64    *pkt_attr_selector_key_reg_value,
                   uint32    ctr_pkt_attr_bit_position,
                   uint32    ctr_pkt_attr_total_bits,
                   uint8     pkt_attr_field_mask_v,
                   uint8     *ctr_current_bit_selector_position);
extern bcm_error_t _bcm_esw_stat_flex_update_offset_table(
                   int                                unit,
                   bcm_stat_flex_direction_t          direction,
                   soc_mem_t                          flex_ctr_offset_table_mem,
                   bcm_stat_flex_mode_t               mode,
                   uint32                             total_counters,
                   bcm_stat_flex_offset_table_entry_t offset_table_map[256]);
extern bcm_error_t _bcm_esw_stat_flex_egress_reserve_mode(
                   int                      unit,
                   bcm_stat_flex_mode_t     mode,
                   uint32                   total_counters,
                   bcm_stat_flex_egr_attr_t *egr_attr);
extern bcm_error_t _bcm_esw_stat_flex_ingress_reserve_mode(
                   int                      unit,
                   bcm_stat_flex_mode_t     mode,
                   uint32                   total_counters,
                   bcm_stat_flex_ing_attr_t *ing_attr);
extern bcm_error_t _bcm_esw_stat_flex_unreserve_mode(
                   int                       unit,
                   bcm_stat_flex_direction_t direction,
                   bcm_stat_flex_mode_t      mode);
extern 
bcm_error_t _bcm_esw_stat_flex_update_udf_selector_keys(
            int                                    unit,
            bcm_stat_flex_direction_t              direction,
            soc_reg_t                              pkt_attr_selector_key_reg,
            bcm_stat_flex_udf_pkt_attr_selectors_t *udf_pkt_attr_selectors,
            uint32                                 *total_udf_bits);
extern bcm_error_t _bcm_esw_stat_flex_init(int unit);
extern bcm_error_t _bcm_esw_stat_flex_cleanup(int unit);
extern bcm_error_t _bcm_esw_stat_flex_sync(int unit);
extern void _bcm_esw_stat_flex_callback(int unit);
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
/* Need to remove it later */
extern int _bcm_tr2_subport_gport_used(int unit, bcm_gport_t port);
#endif

extern bcm_error_t _bcm_esw_stat_flex_create_egress_mode (
                   int                      unit,
                   bcm_stat_flex_egr_attr_t *egr_attr,
                   bcm_stat_flex_mode_t     *mode);
extern bcm_error_t _bcm_esw_stat_flex_create_ingress_mode (
                   int                      unit,
                   bcm_stat_flex_ing_attr_t *ing_attr,
                   bcm_stat_flex_mode_t     *mode);
extern bcm_error_t _bcm_esw_stat_flex_delete_egress_mode(
                   int                  unit,
                   bcm_stat_flex_mode_t mode);
extern bcm_error_t _bcm_esw_stat_flex_delete_ingress_mode(
                   int                  unit,
                   bcm_stat_flex_mode_t mode);


extern bcm_error_t _bcm_esw_stat_flex_destroy_egress_table_counters(
                   int                  unit,
                   soc_mem_t            egress_table,
                   bcm_stat_flex_mode_t offset_mode,
                   uint32               base_idx,
                   uint32               pool_number);
extern bcm_error_t _bcm_esw_stat_flex_detach_egress_table_counters(
                   int       unit,
                   soc_mem_t egress_table,
                   uint32    index);
extern bcm_error_t _bcm_esw_stat_flex_destroy_ingress_table_counters(
                   int                  unit,
                   soc_mem_t            ingress_table,
                   bcm_stat_flex_mode_t offset_mode,
                   uint32               base_idx,
                   uint32               pool_number);
extern bcm_error_t _bcm_esw_stat_flex_detach_ingress_table_counters(
                   int       unit,
                   soc_mem_t ingress_table,
                   uint32    index);
extern bcm_error_t _bcm_esw_stat_flex_attach_egress_table_counters(
                   int                  unit,
                   soc_mem_t            egress_table,
                   uint32               index,
                   bcm_stat_flex_mode_t mode,
                   uint32               base_idx,
                   uint32               pool_number);
extern bcm_error_t _bcm_esw_stat_flex_create_egress_table_counters(
                   int                  unit,
                   soc_mem_t            egress_table,
                   bcm_stat_flex_mode_t mode,
                   uint32               *base_idx,
                   uint32               *pool_number);
extern bcm_error_t _bcm_esw_stat_flex_attach_ingress_table_counters(
                   int                  unit,
                   soc_mem_t            ingress_table,
                   uint32               index,
                   bcm_stat_flex_mode_t mode,
                   uint32               base_idx,
                   uint32               pool_number);
extern bcm_error_t _bcm_esw_stat_flex_create_ingress_table_counters(
                   int                  unit,
                   soc_mem_t            ingress_table,
                   bcm_stat_flex_mode_t mode,
                   uint32               *base_idx,
                   uint32               *pool_number);
extern bcm_error_t _bcm_esw_stat_counter_get(
                   int              unit,
                   uint32           index,
                   soc_mem_t        table,
                   uint32           byte_flag,
                   uint32           counter_index,
                   bcm_stat_value_t *value);
extern bcm_error_t _bcm_esw_stat_counter_set(
                   int              unit,
                   uint32           index,
                   soc_mem_t        table,
                   uint32           byte_flag,
                   uint32           counter_index,
                   bcm_stat_value_t *value);
extern bcm_error_t bcm_esw_stat_flex_get_table_info(
                   bcm_stat_object_t         object,
                   soc_mem_t                 *table,
                   bcm_stat_flex_direction_t *direction);
extern bcm_error_t _bcm_esw_stat_flex_reset_group_mode(
                   int                       unit,
                   bcm_stat_flex_direction_t direction,
                   uint32                    offset_mode,
                   bcm_stat_group_mode_t     group_mode);
extern bcm_error_t _bcm_esw_stat_flex_set_group_mode(
                   int                       unit,
                   bcm_stat_flex_direction_t direction,
                   uint32                    offset_mode,
                   bcm_stat_group_mode_t     group_mode);
extern void  _bcm_esw_stat_flex_show_mode_info(int unit); 
extern bcm_error_t _bcm_esw_stat_flex_get_ingress_object(
                   soc_mem_t         ingress_table,
                   bcm_stat_object_t *object);
extern bcm_error_t _bcm_esw_stat_flex_get_egress_object(
                   soc_mem_t         egress_table,
                   bcm_stat_object_t *object);
extern bcm_error_t _bcm_esw_stat_flex_get_counter_id(
                   int                        unit,
                   uint32                     num_of_tables,
                   bcm_stat_flex_table_info_t *table_info,
                   uint32                     *num_stat_counter_ids,
                   uint32                     *stat_counter_id);
extern void _bcm_esw_stat_group_dump_info(
            int unit,
            int all_flag,
            bcm_stat_object_t object,
            bcm_stat_group_mode_t group);
extern void _bcm_esw_stat_get_counter_id(
            bcm_stat_group_mode_t group,
            bcm_stat_object_t     object,
            uint32                mode,
            uint32                pool_number,
            uint32                base_idx,
            uint32                *stat_counter_id);
extern void _bcm_esw_stat_get_counter_id_info(
            uint32                stat_counter_id,
            bcm_stat_group_mode_t *group,
            bcm_stat_object_t     *object,
            uint32                *mode,
            uint32                *pool_number,
            uint32                *base_idx);

#endif /* __BCM_FLEX_CTR_H__ */
