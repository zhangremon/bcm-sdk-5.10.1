/*
 * $Id: flex_ctr_common.c 1.1.2.8 Broadcom SDK $
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
 * File:        flex_ctr_common.c
 * Purpose:     Manage commaon functionality for flex counter implementation
 */

#include <bcm_int/esw/flex_ctr.h>
#include <shared/idxres_afl.h>
#include <soc/scache.h>
#include <bcm/debug.h>

#define MY_FDEBUG(stuff)   BCM_DEBUG(BCM_DBG_COUNTER|BCM_DBG_WARN,stuff)


uint8     bcm_stat_flex_packet_attr_type_names_t[][64] = {
          "bcmStatFlexPacketAttrTypeUncompressed",
          "bcmStatFlexPacketAttrTypeCompressed",
          "bcmStatFlexPacketAttrTypeUdf"};
static soc_reg_t _pool_ctr_register[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
                                        [BCM_STAT_FLEX_COUNTER_MAX_POOL]={
                 {ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_0r,
                  ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_1r,
                  ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_2r,
                  ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_3r,
                  ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_4r,
                  ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_5r,
                  ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_6r,
                  ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_7r,
                  0,0,0,0,0,0,0,0 /* Kept it for future updates */},
                 {EGR_FLEX_CTR_COUNTER_UPDATE_CONTROL_0r,
                  EGR_FLEX_CTR_COUNTER_UPDATE_CONTROL_1r,
                  EGR_FLEX_CTR_COUNTER_UPDATE_CONTROL_2r,
                  EGR_FLEX_CTR_COUNTER_UPDATE_CONTROL_3r,
                  EGR_FLEX_CTR_COUNTER_UPDATE_CONTROL_4r,
                  EGR_FLEX_CTR_COUNTER_UPDATE_CONTROL_5r,
                  EGR_FLEX_CTR_COUNTER_UPDATE_CONTROL_6r,
                  EGR_FLEX_CTR_COUNTER_UPDATE_CONTROL_7r,
                  0,0,0,0,0,0,0,0 /* Kept it for future updates */}
                 };
static soc_mem_t _ctr_offset_table[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
                                  [BCM_STAT_FLEX_COUNTER_MAX_POOL]={
                 {ING_FLEX_CTR_OFFSET_TABLE_0m,
                  ING_FLEX_CTR_OFFSET_TABLE_1m,
                  ING_FLEX_CTR_OFFSET_TABLE_2m,
                  ING_FLEX_CTR_OFFSET_TABLE_3m,
                  ING_FLEX_CTR_OFFSET_TABLE_4m,
                  ING_FLEX_CTR_OFFSET_TABLE_5m,
                  ING_FLEX_CTR_OFFSET_TABLE_6m,
                  ING_FLEX_CTR_OFFSET_TABLE_7m,
                  0,0,0,0,0,0,0,0 /* Kept it for future updates */},
                 {EGR_FLEX_CTR_OFFSET_TABLE_0m,
                  EGR_FLEX_CTR_OFFSET_TABLE_1m,
                  EGR_FLEX_CTR_OFFSET_TABLE_2m,
                  EGR_FLEX_CTR_OFFSET_TABLE_3m,
                  EGR_FLEX_CTR_OFFSET_TABLE_4m,
                  EGR_FLEX_CTR_OFFSET_TABLE_5m,
                  EGR_FLEX_CTR_OFFSET_TABLE_6m,
                  EGR_FLEX_CTR_OFFSET_TABLE_7m,
                  0,0,0,0,0,0,0,0 /* Kept it for future updates */}
                 };
static soc_mem_t _ctr_counter_table[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
                                  [BCM_STAT_FLEX_COUNTER_MAX_POOL]={
                 {ING_FLEX_CTR_COUNTER_TABLE_0m,
                  ING_FLEX_CTR_COUNTER_TABLE_1m,
                  ING_FLEX_CTR_COUNTER_TABLE_2m,
                  ING_FLEX_CTR_COUNTER_TABLE_3m,
                  ING_FLEX_CTR_COUNTER_TABLE_4m,
                  ING_FLEX_CTR_COUNTER_TABLE_5m,
                  ING_FLEX_CTR_COUNTER_TABLE_6m,
                  ING_FLEX_CTR_COUNTER_TABLE_7m,
                  0,0,0,0,0,0,0,0 /* Kept it for future updates */},
                 {EGR_FLEX_CTR_COUNTER_TABLE_0m,
                  EGR_FLEX_CTR_COUNTER_TABLE_1m,
                  EGR_FLEX_CTR_COUNTER_TABLE_2m,
                  EGR_FLEX_CTR_COUNTER_TABLE_3m,
                  EGR_FLEX_CTR_COUNTER_TABLE_4m,
                  EGR_FLEX_CTR_COUNTER_TABLE_5m,
                  EGR_FLEX_CTR_COUNTER_TABLE_6m,
                  EGR_FLEX_CTR_COUNTER_TABLE_7m,
                  0,0,0,0,0,0,0,0 /* Kept it for future updates */}
                 };
static soc_reg_t _pkt_selector_key_reg[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
                                      [8]= {
                 {ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_0r,
                  ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_1r,
                  ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_2r,
                  ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_3r,
                  ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_4r,
                  ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_5r,
                  ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_6r,
                  ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_7r},
                 {EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_0r,
                  EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_1r,
                  EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_2r,
                  EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_3r,
                  EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_4r,
                  EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_5r,
                  EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_6r,
                  EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_7r}
                 };
static soc_field_t _pkt_selector_x_en_field_name[8]= {
                   SELECTOR_0_ENf,
                   SELECTOR_1_ENf,
                   SELECTOR_2_ENf,
                   SELECTOR_3_ENf,
                   SELECTOR_4_ENf,
                   SELECTOR_5_ENf,
                   SELECTOR_6_ENf,
                   SELECTOR_7_ENf,
                   };
static soc_field_t _pkt_selector_for_bit_x_field_name[8] = {
                   SELECTOR_FOR_BIT_0f,
                   SELECTOR_FOR_BIT_1f,
                   SELECTOR_FOR_BIT_2f,
                   SELECTOR_FOR_BIT_3f,
                   SELECTOR_FOR_BIT_4f,
                   SELECTOR_FOR_BIT_5f,
                   SELECTOR_FOR_BIT_6f,
                   SELECTOR_FOR_BIT_7f,
                   };

static bcm_stat_flex_ingress_mode_t flex_ingress_modes
                                    [BCM_UNITS_MAX]
                                    [BCM_STAT_FLEX_COUNTER_MAX_MODE];
static bcm_stat_flex_egress_mode_t  flex_egress_modes
                                    [BCM_UNITS_MAX]
                                    [BCM_STAT_FLEX_COUNTER_MAX_MODE];

static shr_aidxres_list_handle_t flex_aidxres_list_handle
                                 [BCM_UNITS_MAX]
                                 [BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
                                 [BCM_STAT_FLEX_COUNTER_MAX_POOL];
static bcm_stat_flex_pool_stat_t flex_pool_stat
                                 [BCM_UNITS_MAX]
                                 [BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
                                 [BCM_STAT_FLEX_COUNTER_MAX_POOL];
static uint16 *flex_base_index_reference_count
                                 [BCM_UNITS_MAX]
                                 [BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
                                 [BCM_STAT_FLEX_COUNTER_MAX_POOL];

static sal_mutex_t flex_stat_mutex[BCM_UNITS_MAX] = {NULL};

/* Both ing_flex_ctr_counter_table_0_entry_t and egr_flex_ctr_table_0_entry_t
   have same contents so using one entry only */
static ing_flex_ctr_counter_table_0_entry_t
       *flex_temp_counter[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]={NULL};

static uint64 *flex_byte_counter
               [BCM_UNITS_MAX]
               [BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
               [BCM_STAT_FLEX_COUNTER_MAX_POOL]={{{NULL}}};
static uint32 *flex_packet_counter
               [BCM_UNITS_MAX]
               [BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
               [BCM_STAT_FLEX_COUNTER_MAX_POOL]={{{NULL}}};

#define BCM_STAT_FLEX_COUNTER_LOCK(unit) \
        sal_mutex_take(flex_stat_mutex[unit], sal_mutex_FOREVER);
#define BCM_STAT_FLEX_COUNTER_UNLOCK(unit) \
        sal_mutex_give(flex_stat_mutex[unit]);

static uint8 flex_directions[][8]={
             "Ingress",
             "Egress" };
#ifdef BCM_WARM_BOOT_SUPPORT
static uint8 _flex_group_mode_total_counters_info
              [BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]
              [bcmStatGroupModeDvpType+1]={
               /* #####################################      */
               /* INGRESS GROUP MODE ==> Total Counters      */
               /* #####################################      */
               {/* bcmStatGroupModeSingle */                 1,
                /* bcmStatGroupModeTrafficType */            3,
                /* bcmStatGroupModeDlfAll */                 2,
                /* bcmStatGroupModeDlfIntPri */              17,
                /* bcmStatGroupModeTyped */                  4,
                /* bcmStatGroupModeTypedAll */               5,
                /* bcmStatGroupModeTypedIntPri */            20,
                /* bcmStatGroupModeSingleWithControl */      2,
                /* bcmStatGroupModeTrafficTypeWithControl */ 4,
                /* bcmStatGroupModeDlfAllWithControl */      3,
                /* bcmStatGroupModeDlfIntPriWithControl */   18,
                /* bcmStatGroupModeTypedWithControl */       5,
                /* bcmStatGroupModeTypedAllWithControl */    6,
                /* bcmStatGroupModeTypedIntPriWithControl */ 21,
                /* bcmStatGroupModeDot1P */                  8,
                /* bcmStatGroupModeIntPri */                 16,
                /* bcmStatGroupModeIntPriCng */              64,
                /* bcmStatGroupModeSvpType */                2,
                /* bcmStatGroupModeDscp */                   64,
                /* bcmStatGroupModeDvpType */                0},
               /* #####################################      */
               /* EGRESS GROUP MODE ==> Total Counters       */
               /* #####################################      */
               {/* bcmStatGroupModeSingle */                 1,
                /* bcmStatGroupModeTrafficType */            2,
                /* bcmStatGroupModeDlfAll */                 0,
                /* bcmStatGroupModeDlfIntPri */              0,
                /* bcmStatGroupModeTyped */                  0,
                /* bcmStatGroupModeTypedAll */               0,
                /* bcmStatGroupModeTypedIntPri */            18,
                /* bcmStatGroupModeSingleWithControl */      0,
                /* bcmStatGroupModeTrafficTypeWithControl */ 0,
                /* bcmStatGroupModeDlfAllWithControl */      0,
                /* bcmStatGroupModeDlfIntPriWithControl */   0,
                /* bcmStatGroupModeTypedWithControl */       0,
                /* bcmStatGroupModeTypedAllWithControl */    0,
                /* bcmStatGroupModeTypedIntPriWithControl */ 0,
                /* bcmStatGroupModeDot1P */                  8,
                /* bcmStatGroupModeIntPri */                 16,
                /* bcmStatGroupModeIntPriCng */              64,
                /* bcmStatGroupModeSvpType */                2,
                /* bcmStatGroupModeDscp */                   64,
                /* bcmStatGroupModeDvpType */                2}
              };
#endif

typedef struct uncmprsd_attr_bits_selector_s {
    uint32 attr_bits;
    uint8  attr_name[20];
}uncmprsd_attr_bits_selector_t;

static uncmprsd_attr_bits_selector_t ing_uncmprsd_attr_bits_selector[]={
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_CNG_ATTR_BITS,           "CNG"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_IFP_CNG_ATTR_BITS,       "IFP CNG"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS,       "INT PRI"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS,   "VLAN"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS,   "OUTER.1P"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS,   "INNER.1P"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INGRESS_PORT_ATTR_BITS,  "PORT"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_TOS_ATTR_BITS,           "TOS"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,"PktRes"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS,      "SVP"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_DROP_ATTR_BITS,          "DROP"},
       {BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS,        "IP"}
};
static uncmprsd_attr_bits_selector_t egr_uncmprsd_attr_bits_selector[]={
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_CNG_ATTR_BITS,           "CNG"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS,       "INT PRI"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS,   "VLAN"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS,   "OUTER.1P"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS,   "INNER.1P"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_EGRESS_PORT_ATTR_BITS,   "PORT"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_TOS_ATTR_BITS,           "TOS"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,"PktRes"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS,      "SVP"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DVP_TYPE_ATTR_BITS,      "DVP"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DROP_ATTR_BITS,          "DROP"},
       {BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS,        "IP"}
};

#define _DEFINE_GET_INGRESS_VALUE(field_name,field_value) \
static  uint8 _bcm_esw_get_ing_##field_name##_value( \
                bcm_stat_flex_ing_pkt_attr_bits_t *ing_pkt_attr_bits) { \
                   return field_value; \
} \

#define DEFINE_GET_INGRESS_VALUE(field_name) \
        _DEFINE_GET_INGRESS_VALUE(field_name,ing_pkt_attr_bits->field_name)

DEFINE_GET_INGRESS_VALUE(cng)
DEFINE_GET_INGRESS_VALUE(ifp_cng)
DEFINE_GET_INGRESS_VALUE(int_pri)
DEFINE_GET_INGRESS_VALUE(vlan_format)
DEFINE_GET_INGRESS_VALUE(outer_dot1p)
DEFINE_GET_INGRESS_VALUE(inner_dot1p)
DEFINE_GET_INGRESS_VALUE(ing_port)
DEFINE_GET_INGRESS_VALUE(tos)
DEFINE_GET_INGRESS_VALUE(pkt_resolution)
DEFINE_GET_INGRESS_VALUE(svp_type)
DEFINE_GET_INGRESS_VALUE(drop)
DEFINE_GET_INGRESS_VALUE(ip_pkt)

typedef struct _bcm_esw_get_ing_func_f {
    uint8 (*func)(bcm_stat_flex_ing_pkt_attr_bits_t *ing_pkt_attr_bits);
    uint8 func_desc[20];
}_bcm_esw_get_ing_func_t;
static _bcm_esw_get_ing_func_t _bcm_esw_get_ing_func[]={
    {_bcm_esw_get_ing_cng_value,"cng"},
    {_bcm_esw_get_ing_ifp_cng_value,"ifp_cng"},
    {_bcm_esw_get_ing_int_pri_value,"int_pri"},
    {_bcm_esw_get_ing_vlan_format_value,"vlan_format"},
    {_bcm_esw_get_ing_outer_dot1p_value,"outer_dot1p"},
    {_bcm_esw_get_ing_inner_dot1p_value,"inner_dot1p"},
    {_bcm_esw_get_ing_ing_port_value,"ing_port"},
    {_bcm_esw_get_ing_tos_value,"tos"},
    {_bcm_esw_get_ing_pkt_resolution_value,"pkt_resolutino"},
    {_bcm_esw_get_ing_svp_type_value,"svp"},
    {_bcm_esw_get_ing_drop_value,"drop"},
    {_bcm_esw_get_ing_ip_pkt_value,"ip_pkt"}
};

#define _DEFINE_GET_EGRESS_VALUE(field_name,field_value) \
static  uint8 _bcm_esw_get_egr_##field_name##_value( \
                bcm_stat_flex_egr_pkt_attr_bits_t *egr_pkt_attr_bits) { \
                   return field_value; \
} \

#define DEFINE_GET_EGRESS_VALUE(field_name) \
        _DEFINE_GET_EGRESS_VALUE(field_name,egr_pkt_attr_bits->field_name)

DEFINE_GET_EGRESS_VALUE(cng)
DEFINE_GET_EGRESS_VALUE(int_pri)
DEFINE_GET_EGRESS_VALUE(vlan_format)
DEFINE_GET_EGRESS_VALUE(outer_dot1p)
DEFINE_GET_EGRESS_VALUE(inner_dot1p)
DEFINE_GET_EGRESS_VALUE(egr_port)
DEFINE_GET_EGRESS_VALUE(tos)
DEFINE_GET_EGRESS_VALUE(pkt_resolution)
DEFINE_GET_EGRESS_VALUE(svp_type)
DEFINE_GET_EGRESS_VALUE(dvp_type)
DEFINE_GET_EGRESS_VALUE(drop)
DEFINE_GET_EGRESS_VALUE(ip_pkt)

typedef struct _bcm_esw_get_egr_func_f {
    uint8 (*func)(bcm_stat_flex_egr_pkt_attr_bits_t *egr_pkt_attr_bits);
    uint8 func_desc[20];
}_bcm_esw_get_egr_func_t;
static _bcm_esw_get_egr_func_t _bcm_esw_get_egr_func[]={
    {_bcm_esw_get_egr_cng_value,"cng"},
    {_bcm_esw_get_egr_int_pri_value,"int_pri"},
    {_bcm_esw_get_egr_vlan_format_value,"vlan_format"},
    {_bcm_esw_get_egr_outer_dot1p_value,"outer_dot1p"},
    {_bcm_esw_get_egr_inner_dot1p_value,"inner_dot1p"},
    {_bcm_esw_get_egr_egr_port_value,"egr_port"},
    {_bcm_esw_get_egr_tos_value,"tos"},
    {_bcm_esw_get_egr_pkt_resolution_value,"pkt_resolutino"},
    {_bcm_esw_get_egr_svp_type_value,"svp"},
    {_bcm_esw_get_egr_dvp_type_value,"dvp"},
    {_bcm_esw_get_egr_drop_value,"drop"},
    {_bcm_esw_get_egr_ip_pkt_value,"ip_pkt"}
};

static uint8 flex_objects[][32]={
             "bcmStatObjectIngPort",
             "bcmStatObjectIngVlan",
             "bcmStatObjectIngVlanXlate",
             "bcmStatObjectIngVfi",
             "bcmStatObjectIngL3Intf",
             "bcmStatObjectIngVrf",
             "bcmStatObjectIngPolicy",
             "bcmStatObjectIngMplsVcLabel",
             "bcmStatObjectIngMplsSwitchLabel",
             "bcmStatObjectEgrPort",
             "bcmStatObjectEgrVlan",
             "bcmStatObjectEgrVlanXlate",
             "bcmStatObjectEgrVfi",
             "bcmStatObjectEgrL3Intf" };

static uint8 flex_group_modes[][48]={
             "bcmStatGroupModeSingle = 0",
             "bcmStatGroupModeTrafficType = 1",
             "bcmStatGroupModeDlfAll = 2",
             "bcmStatGroupModeDlfIntPri = 3",
             "bcmStatGroupModeTyped = 4",
             "bcmStatGroupModeTypedAll = 5",
             "bcmStatGroupModeTypedIntPri = 6",
             "bcmStatGroupModeSingleWithControl = 7",
             "bcmStatGroupModeTrafficTypeWithControl = 8",
             "bcmStatGroupModeDlfAllWithControl = 9",
             "bcmStatGroupModeDlfIntPriWithControl = 10",
             "bcmStatGroupModeTypedWithControl = 11",
             "bcmStatGroupModeTypedAllWithControl = 12",
             "bcmStatGroupModeTypedIntPriWithControl = 13",
             "bcmStatGroupModeDot1P = 14",
             "bcmStatGroupModeIntPri = 15",
             "bcmStatGroupModeIntPriCng = 16",
             "bcmStatGroupModeSvpType = 17",
             "bcmStatGroupModeDscp = 18",
             "bcmStatGroupModeDvpType = 19" };

static uint32 flex_used_by_table[]={
              FLEX_COUNTER_POOL_USED_BY_PORT_TABLE,
              FLEX_COUNTER_POOL_USED_BY_VLAN_TABLE,
              FLEX_COUNTER_POOL_USED_BY_VLAN_XLATE_TABLE,
              FLEX_COUNTER_POOL_USED_BY_VFI_TABLE,
              FLEX_COUNTER_POOL_USED_BY_L3_IIF_TABLE,
              FLEX_COUNTER_POOL_USED_BY_VRF_TABLE,
              FLEX_COUNTER_POOL_USED_BY_VFP_POLICY_TABLE,
              FLEX_COUNTER_POOL_USED_BY_SOURCE_VP_TABLE,
              FLEX_COUNTER_POOL_USED_BY_MPLS_ENTRY_TABLE,
              FLEX_COUNTER_POOL_USED_BY_EGR_PORT_TABLE,
              FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_TABLE,
              FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_XLATE_TABLE,
              FLEX_COUNTER_POOL_USED_BY_EGR_VFI_TABLE,
              FLEX_COUNTER_POOL_USED_BY_EGR_L3_NEXT_HOP_TABLE };

static uint32 *local_scache_ptr[BCM_UNITS_MAX]={NULL};
static uint32 local_scache_size=0;

#ifdef BCM_WARM_BOOT_SUPPORT
static soc_scache_handle_t handle=0;
static uint32              flex_scache_allocated_size=0;
static uint32              *flex_scache_ptr[BCM_UNITS_MAX]={NULL};
#endif

typedef struct flex_counter_fields {
               uint32    offset_mode;
               uint32    pool_number;
               uint32    base_idx;
}flex_counter_fields_t;

static flex_counter_fields_t mpls_entry_flex_counter_fields[3]={
                             /* MPLS View=0 */
                             {FLEX_CTR_OFFSET_MODEf,
                              FLEX_CTR_POOL_NUMBERf,
                              FLEX_CTR_BASE_COUNTER_IDXf},
                             /* MIM_NVP View=1 */
                             {MIM_NVP__FLEX_CTR_OFFSET_MODEf,
                              MIM_NVP__FLEX_CTR_POOL_NUMBERf,
                              MIM_NVP__FLEX_CTR_BASE_COUNTER_IDXf},
                             /* MIM_ISID   View=2 */
                             {MIM_ISID__FLEX_CTR_OFFSET_MODEf,
                              MIM_ISID__FLEX_CTR_POOL_NUMBERf,
                              MIM_ISID__FLEX_CTR_BASE_COUNTER_IDXf} };

static flex_counter_fields_t egr_l3_next_hop_flex_counter_fields[6]={
                             /* L3 View=0 */
                             {L3__FLEX_CTR_OFFSET_MODEf,
                              L3__FLEX_CTR_POOL_NUMBERf,
                              L3__FLEX_CTR_BASE_COUNTER_IDXf},
                             /* MPLS View=1 */
                             {MPLS__FLEX_CTR_OFFSET_MODEf,
                              MPLS__FLEX_CTR_POOL_NUMBERf,
                              MPLS__FLEX_CTR_BASE_COUNTER_IDXf},
                             /* SD_TAG View=2 */
                             {SD_TAG__FLEX_CTR_OFFSET_MODEf,
                              SD_TAG__FLEX_CTR_POOL_NUMBERf,
                              SD_TAG__FLEX_CTR_BASE_COUNTER_IDXf},
                             /* MIM View=3 */
                             {MIM__FLEX_CTR_OFFSET_MODEf,
                              MIM__FLEX_CTR_POOL_NUMBERf,
                              MIM__FLEX_CTR_BASE_COUNTER_IDXf},
                             /* Reserved View=4 */
                             {0,
                              0,
                              0},
                             /* PROXY View=5 */
                             {PROXY__FLEX_CTR_OFFSET_MODEf,
                              PROXY__FLEX_CTR_POOL_NUMBERf,
                              PROXY__FLEX_CTR_BASE_COUNTER_IDXf} };

static flex_counter_fields_t egr_vlan_xlate_flex_counter_fields[5]={
                             /* Default VLAN_XLATE View=0 */
                             {FLEX_CTR_OFFSET_MODEf,
                              FLEX_CTR_POOL_NUMBERf,
                              FLEX_CTR_BASE_COUNTER_IDXf},
                             /* Default VLAN_XLATE_DVP View=1 */
                             {FLEX_CTR_OFFSET_MODEf,
                              FLEX_CTR_POOL_NUMBERf,
                              FLEX_CTR_BASE_COUNTER_IDXf},
                             /* Reserved View=2 */
                             {0,
                              0,
                              0},
                             /* ISID_XLATE View=3 */
                             {MIM_ISID__FLEX_CTR_OFFSET_MODEf,
                              MIM_ISID__FLEX_CTR_POOL_NUMBERf,
                              MIM_ISID__FLEX_CTR_BASE_COUNTER_IDXf},
                             /* ISID_DVP_XLATE View=4 */
                             {MIM_ISID__FLEX_CTR_OFFSET_MODEf,
                              MIM_ISID__FLEX_CTR_POOL_NUMBERf,
                              MIM_ISID__FLEX_CTR_BASE_COUNTER_IDXf} };
/*
 * Function:
 *      _bcm_esw_get_flex_counter_fields
 * Description:
 *      Gets flex counter fields(FLEX_CTR_OFFSET_MODEf,FLEX_CTR_POOL_NUMBERf
 *      and FLEX_CTR_BASE_COUNTER_IDXf) name for given table.
 *      This is important when a table has several views for flex counter
 *      field and EntryType or KeyType fieled is checked.
 * Parameters:
 *      unit                - (IN) unit number
 *      table               - (IN) Accounting Table 
 *      data                - (IN) Accounting Table Data FLEX_CTR_OFFSET_MODE
 *      offset_mode_field   - (OUT) Field for FLEX_CTR_OFFSET_MODE
 *      pool_number_field   - (OUT) Field for FLEX_CTR_POOL_NUMBER
 *      base_idx_field      - (OUT) Field for FLEX_CTR_BASE_COUNTER_IDX
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */

static bcm_error_t 
_bcm_esw_get_flex_counter_fields(
    int         unit,
    soc_mem_t   table,
    void        *data,
    soc_field_t *offset_mode_field,
    soc_field_t *pool_number_field,
    soc_field_t *base_idx_field)
{
    uint32 key_type=0;
    uint32 entry_type=0;

    *offset_mode_field=FLEX_CTR_OFFSET_MODEf;
    *pool_number_field=FLEX_CTR_POOL_NUMBERf;
    *base_idx_field=FLEX_CTR_BASE_COUNTER_IDXf;
    switch(table) {
    case MPLS_ENTRYm:
         FLEXCTR_VVERB(("MPLS_ENTRYm   "));
         if (!soc_mem_field_valid(unit,table,KEY_TYPEf)) {
             return BCM_E_CONFIG;
         }
         key_type=soc_mem_field32_get(unit,table,data,KEY_TYPEf);
         FLEXCTR_VVERB(("key_type %d ",key_type));
         if (key_type >= sizeof(mpls_entry_flex_counter_fields)/
                         sizeof(flex_counter_fields_t)) {
             FLEXCTR_ERR(("size not OK \n"));
             return BCM_E_CONFIG;
         }
         *offset_mode_field= mpls_entry_flex_counter_fields[key_type].
                             offset_mode;
         *pool_number_field= mpls_entry_flex_counter_fields[key_type].
                             pool_number;
         *base_idx_field=    mpls_entry_flex_counter_fields[key_type].
                             base_idx;
         break;
    case EGR_L3_NEXT_HOPm:
         FLEXCTR_VVERB(("EGR_L3_NEXT_HOP   =>"));
         if (!soc_mem_field_valid(unit,table,ENTRY_TYPEf)) {
             return BCM_E_CONFIG;
         }
         entry_type=soc_mem_field32_get(unit,table,data,ENTRY_TYPEf);
         FLEXCTR_VVERB(("entry_type %d ",entry_type));
         if (entry_type >= sizeof(egr_l3_next_hop_flex_counter_fields)/
                           sizeof(flex_counter_fields_t)) {
             FLEXCTR_ERR(("size not OK \n"));
             return BCM_E_CONFIG;
         }
         if (entry_type == 4) { /* Reserved */
             FLEXCTR_ERR(("reserved value NOK \n"));
             return BCM_E_CONFIG;
         }
         *offset_mode_field= egr_l3_next_hop_flex_counter_fields[entry_type].
                             offset_mode;
         *pool_number_field= egr_l3_next_hop_flex_counter_fields[entry_type].
                             pool_number;
         *base_idx_field   = egr_l3_next_hop_flex_counter_fields[entry_type].
                             base_idx;
         break;
    case EGR_VLAN_XLATEm:
         FLEXCTR_VVERB(("EGR_VLAN_XLATEm =="));
         if (!soc_mem_field_valid(unit,table,ENTRY_TYPEf)) {
             return BCM_E_CONFIG;
         }
         entry_type=soc_mem_field32_get(unit,table,data,ENTRY_TYPEf);
         FLEXCTR_VVERB(("entry_type %d ",entry_type));
         if (entry_type >= sizeof(egr_vlan_xlate_flex_counter_fields)/
                           sizeof(flex_counter_fields_t)) {
             FLEXCTR_ERR(("size not OK \n"));
             return BCM_E_CONFIG;
         }
         if (entry_type == 2) { /* Reserved */
             FLEXCTR_ERR(("reserved 2 NOK \n"));
             return BCM_E_CONFIG;
         }
         *offset_mode_field= egr_vlan_xlate_flex_counter_fields[entry_type].
                             offset_mode;
         *pool_number_field= egr_vlan_xlate_flex_counter_fields[entry_type].
                             pool_number;
         *base_idx_field   = egr_vlan_xlate_flex_counter_fields[entry_type].
                             base_idx;
         break;
    default: 
         break;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_get_flex_counter_fields_values
 * Description:
 *      Gets flex counter fields(FLEX_CTR_OFFSET_MODEf,FLEX_CTR_POOL_NUMBERf
 *      and FLEX_CTR_BASE_COUNTER_IDXf) values for given table.
 *      This is important when a table has several views for flex counter
 *      field and EntryType or KeyType fieled is checked.
 * Parameters:
 *      unit                - (IN) unit number
 *      table               - (IN) Accounting Table 
 *      data                - (IN) Accounting Table Data FLEX_CTR_OFFSET_MODE
 *      offset_mode         - (OUT) FLEX_CTR_OFFSET_MODE field value
 *      pool_number         - (OUT) FLEX_CTR_POOL_NUMBER field value
 *      base_idx            - (OUT) FLEX_CTR_BASE_COUNTER_IDX field value
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static bcm_error_t 
_bcm_esw_get_flex_counter_fields_values(
    int       unit,
    soc_mem_t table,
    void      *data,
    uint32    *offset_mode,
    uint32    *pool_number,
    uint32    *base_idx)
{
    soc_field_t offset_mode_field=FLEX_CTR_OFFSET_MODEf;
    soc_field_t pool_number_field=FLEX_CTR_POOL_NUMBERf;
    soc_field_t base_idx_field=FLEX_CTR_BASE_COUNTER_IDXf;
    BCM_IF_ERROR_RETURN(_bcm_esw_get_flex_counter_fields(
                        unit,
                        table,
                        data, 
                        &offset_mode_field,
                        &pool_number_field,
                        &base_idx_field));
    *offset_mode =  soc_mem_field32_get(unit,table,data,offset_mode_field);
    *pool_number =  soc_mem_field32_get(unit,table,data,pool_number_field);
    *base_idx =  soc_mem_field32_get(unit,table,data,base_idx_field);
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_set_flex_counter_fields_values
 * Description:
 *      Sets flex counter fields(FLEX_CTR_OFFSET_MODEf,FLEX_CTR_POOL_NUMBERf
 *      and FLEX_CTR_BASE_COUNTER_IDXf) values for given table.
 *      This is important when a table has several views for flex counter
 *      field and EntryType or KeyType fieled is checked.
 * Parameters:
 *      unit                - (IN) unit number
 *      table               - (IN) Accounting Table 
 *      data                - (IN) Accounting Table Data FLEX_CTR_OFFSET_MODE
 *      offset_mode         - (IN) FLEX_CTR_OFFSET_MODE field value
 *      pool_number         - (IN) FLEX_CTR_POOL_NUMBER field value
 *      base_idx            - (IN) FLEX_CTR_BASE_COUNTER_IDX field value
 *
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static bcm_error_t 
_bcm_esw_set_flex_counter_fields_values(
    int       unit,
    soc_mem_t table,
    void      *data,
    uint32    offset_mode,
    uint32    pool_number,
    uint32    base_idx)
{
    soc_field_t offset_mode_field=FLEX_CTR_OFFSET_MODEf;
    soc_field_t pool_number_field=FLEX_CTR_POOL_NUMBERf;
    soc_field_t base_idx_field=FLEX_CTR_BASE_COUNTER_IDXf;
    BCM_IF_ERROR_RETURN(_bcm_esw_get_flex_counter_fields(
                        unit,
                        table,
                        data, 
                        &offset_mode_field,
                        &pool_number_field,
                        &base_idx_field));
    soc_mem_field32_set(unit,table,data,offset_mode_field,offset_mode);
    soc_mem_field32_set(unit,table,data,pool_number_field,pool_number);
    soc_mem_field32_set(unit,table,data,base_idx_field,base_idx);
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_insert_stat_id
 * Description:
 *      Inserts stat id in local scache table. Useful for WARM-BOOT purpose
 * Parameters:
 *      scache_ptr            - (IN) Local scache table pointer
 *      stat_counter_id       - (IN) Flex Stat Counter Id
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static bcm_error_t 
_bcm_esw_stat_flex_insert_stat_id(uint32 *scache_ptr,uint32 stat_counter_id)
{
    uint32 index=0;
    FLEXCTR_VVERB(("Inserting %d ",stat_counter_id));
    for (index=0;index<BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE;index++) {
         if (scache_ptr[index] == 0) {
             FLEXCTR_VVERB(("Inserted \n"));
             scache_ptr[index] = stat_counter_id;
             break;
         }
    }
    if (index == BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE) {
        return BCM_E_FAIL;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_delete_stat_id
 * Description:
 *      Deletes stat id from local scache table. Useful for WARM-BOOT purpose
 * Parameters:
 *      scache_ptr            - (IN) Local scache table pointer
 *      stat_counter_id       - (IN) Flex Stat Counter Id
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static bcm_error_t 
_bcm_esw_stat_flex_delete_stat_id(uint32 *scache_ptr,uint32 stat_counter_id)
{
    uint32 index=0;
    FLEXCTR_VVERB(("Deleting ID:%d ",stat_counter_id));
    for (index=0;index<BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE;index++) {
         if (scache_ptr[index] == stat_counter_id) {
             FLEXCTR_VVERB(("Deleted \n"));
             scache_ptr[index] = 0;
             break;
         }
    }
    if (index == BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE) {
        return BCM_E_FAIL;
    }
    return BCM_E_NONE;
}
#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_esw_stat_flex_install_stat_id
 * Description:
 *      Install(ie. configures h/w) as per retrieved stat-id. 
 *      Useful for WARM-BOOT purpose
 * Parameters:
 *      unit                  - (IN) unit number
 *      scache_ptr            - (IN) Local scache table pointer
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static 
bcm_error_t _bcm_esw_stat_flex_install_stat_id(int unit,uint32  *scache_ptr)
{
    uint32                    index=0;
    uint32                    stat_counter_id=0;
    uint32                    pool_number=0;
    uint32                    base_index=0;
    bcm_stat_flex_mode_t      offset_mode=0;
    bcm_stat_object_t         object=bcmStatObjectIngPort;
    bcm_stat_group_mode_t     group_mode= bcmStatGroupModeSingle;
    bcm_stat_flex_direction_t direction=bcmStatFlexDirectionIngress;
    uint32                    total_counters=0;

    for (index=0;index<BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE;index++) {
         if (scache_ptr[index] != 0) {
             stat_counter_id = scache_ptr[index];
             _bcm_esw_stat_get_counter_id_info(
                      stat_counter_id,&group_mode,&object,
                      &offset_mode,&pool_number,&base_index);
             if ((object >= bcmStatObjectIngPort) &&
                 (object <= bcmStatObjectIngMplsSwitchLabel)) {
                  direction = bcmStatFlexDirectionIngress;
                  /* Quite possible..no attachment till now */
                  if (flex_ingress_modes[unit][offset_mode].total_counters
                      == 0) {
                       flex_ingress_modes[unit][offset_mode].total_counters =
                            _flex_group_mode_total_counters_info
                            [bcmStatFlexDirectionIngress][group_mode];
                       flex_ingress_modes[unit][offset_mode].group_mode = 
                            group_mode;
                  }
                  total_counters = flex_ingress_modes[unit][offset_mode].
                                   total_counters;
                  flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                                [pool_number].used_entries   += total_counters;
                  flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                                [pool_number].used_by_tables |= 
                                flex_used_by_table[object];
             } else {
                  direction = bcmStatFlexDirectionEgress;
                  /* Quite possible..no attachment till now */
                  if (flex_egress_modes[unit][offset_mode].total_counters
                      == 0) {
                       flex_egress_modes[unit][offset_mode].total_counters =
                            _flex_group_mode_total_counters_info
                            [bcmStatFlexDirectionEgress][group_mode];
                       flex_egress_modes[unit][offset_mode].group_mode = 
                            group_mode;
                  }
                  total_counters = flex_egress_modes[unit][offset_mode].
                                   total_counters;
                  flex_pool_stat[unit][bcmStatFlexDirectionEgress]
                                [pool_number].used_entries   += total_counters;
                  flex_pool_stat[unit][bcmStatFlexDirectionEgress]
                                [pool_number].used_by_tables |=
                                flex_used_by_table[object];
             }
             FLEXCTR_VVERB(("Installing: mode:%d group_mode:%d pool:%d"
                            "object:%d base:%d\n",offset_mode,group_mode,
                            pool_number, object,base_index));
             if (total_counters == 0) {
                 FLEXCTR_VVERB(("Counter=0.Mode not configured in h/w."
                                "skipping it\n"));
                 continue;
             }
             shr_aidxres_list_reserve_block(
                 flex_aidxres_list_handle[unit][direction][pool_number],
                 base_index,
                 total_counters);
             if (direction == bcmStatFlexDirectionIngress) {
                 flex_ingress_modes[unit][offset_mode].reference_count++;
             } else {
                 flex_egress_modes[unit][offset_mode].reference_count++;
             }
         }
    }
    return BCM_E_NONE;
}
#endif

/*
 * Function:
 *      _bcm_esw_stat_flex_retrieve_group_mode
 * Description:
 *      Retrieves Flex group mode based on configured h/w.
 *      Useful in warm boot case
 * Parameters:
 *      unit                  - (IN) unit number
 *      direction             - (IN) Flex Data flow direction
 *      offset_mode           - (IN) Flex offset mode
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static bcm_error_t 
_bcm_esw_stat_flex_retrieve_group_mode(
    int                       unit,
    bcm_stat_flex_direction_t direction,
    uint32                    offset_mode)
{
    bcm_stat_flex_ing_attr_t                    *ing_attr=NULL;
    bcm_stat_flex_ing_uncmprsd_attr_selectors_t *ing_uncmprsd_attr_selectors =
                                                NULL;
    bcm_stat_flex_ing_cmprsd_attr_selectors_t   *ing_cmprsd_attr_selectors=NULL;
    bcm_stat_flex_ing_pkt_attr_bits_t           *ing_cmprsd_pkt_attr_bits=NULL;

    bcm_stat_flex_egr_attr_t                    *egr_attr=NULL;
    bcm_stat_flex_egr_uncmprsd_attr_selectors_t *egr_uncmprsd_attr_selectors =
                                                NULL;
    bcm_stat_flex_egr_cmprsd_attr_selectors_t   *egr_cmprsd_attr_selectors=NULL;
    bcm_stat_flex_egr_pkt_attr_bits_t           *egr_cmprsd_pkt_attr_bits=NULL;

    if (direction == bcmStatFlexDirectionIngress) {
        if (flex_ingress_modes[unit][offset_mode].available == 0) {
            return BCM_E_NONE;
        }
        ing_attr                    = &(flex_ingress_modes[unit][offset_mode].
                                        ing_attr);
        ing_uncmprsd_attr_selectors = &(ing_attr->uncmprsd_attr_selectors);
        ing_cmprsd_attr_selectors   = &(ing_attr->cmprsd_attr_selectors);
        ing_cmprsd_pkt_attr_bits    = &(ing_cmprsd_attr_selectors->
                                        pkt_attr_bits);

        switch(flex_ingress_modes[unit][offset_mode].total_counters) {
        case 1: 
             /* bcmStatGroupModeSingle */
             return _bcm_esw_stat_flex_set_group_mode(
                                 unit,bcmStatFlexDirectionIngress,
                                 offset_mode,bcmStatGroupModeSingle);
        case 2: 
             /* bcmStatGroupModeDlfAll,bcmStatGroupModeSingleWithControl,
                bcmStatGroupModeSvpType */
             if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS) {
                 return _bcm_esw_stat_flex_set_group_mode(
                                 unit,bcmStatFlexDirectionIngress,
                                 offset_mode,bcmStatGroupModeSvpType);
             }
             if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS) {
                 if ((ing_uncmprsd_attr_selectors->
                      offset_table_map[UNKNOWN_PKT].offset == 1) &&
                     (ing_uncmprsd_attr_selectors->
                      offset_table_map[CONTROL_PKT].offset == 1)) {
                      return _bcm_esw_stat_flex_set_group_mode(
                                 unit,
                                 bcmStatFlexDirectionIngress,
                                 offset_mode,
                                 bcmStatGroupModeDlfAllWithControl);
                 }
                 if ((ing_uncmprsd_attr_selectors->
                      offset_table_map[UNKNOWN_PKT].offset == 0) &&
                     (ing_uncmprsd_attr_selectors->
                      offset_table_map[CONTROL_PKT].offset == 1)) {
                      return _bcm_esw_stat_flex_set_group_mode(
                                 unit,
                                 bcmStatFlexDirectionIngress,
                                 offset_mode,
                                 bcmStatGroupModeSingleWithControl);
                 }
             }
             return BCM_E_PARAM;
        case 3: 
             /* bcmStatGroupModeTrafficType,bcmStatGroupModeDlfAllWithControl */
             if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS) {
                 if (ing_uncmprsd_attr_selectors->
                     offset_table_map[KNOWN_L2MC_PKT].offset == 2) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionIngress,
                                offset_mode,
                                bcmStatGroupModeDlfAllWithControl);
                 }
                 if (ing_uncmprsd_attr_selectors->
                     offset_table_map[KNOWN_L2MC_PKT].offset == 1) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionIngress,
                                offset_mode,
                                bcmStatGroupModeTrafficType);
                 }
             }
             return BCM_E_PARAM;
        case 4: 
             /* bcmStatGroupModeTyped, bcmStatGroupModeTrafficTypeWithControl */
             if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS) {
                 if (ing_uncmprsd_attr_selectors->
                     offset_table_map[KNOWN_L2MC_PKT].offset == 2) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionIngress,
                                offset_mode,
                                bcmStatGroupModeTyped);
                 }
                 if (ing_uncmprsd_attr_selectors->
                     offset_table_map[KNOWN_L2MC_PKT].offset == 1) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionIngress,
                                offset_mode,
                                bcmStatGroupModeTrafficTypeWithControl);
                 }
             }
             return BCM_E_PARAM;
        case 5: 
             /* bcmStatGroupModeTypedAll , bcmStatGroupModeTypedWithControl */
             if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS) {
                 if (ing_uncmprsd_attr_selectors->
                     offset_table_map[L2UC_PKT].offset == 1) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionIngress,
                                offset_mode,
                                bcmStatGroupModeTyped);
                 }
                 if (ing_uncmprsd_attr_selectors->
                     offset_table_map[L2UC_PKT].offset == 2) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionIngress,
                                offset_mode,
                                bcmStatGroupModeTrafficTypeWithControl);
                 }
             }
             return BCM_E_PARAM;
        case 6: 
             /* bcmStatGroupModeTypedAllWithControl */
            if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS) {
                return _bcm_esw_stat_flex_set_group_mode(
                           unit,
                           bcmStatFlexDirectionIngress,
                           offset_mode,
                           bcmStatGroupModeTypedAllWithControl);
                }
             return BCM_E_PARAM;
        case 8: 
             /* bcmStatGroupModeDot1P */
             if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS) {
                 return _bcm_esw_stat_flex_set_group_mode(
                            unit,
                            bcmStatFlexDirectionIngress,
                            offset_mode,
                            bcmStatGroupModeDot1P);
             }
             return BCM_E_PARAM;
        case 16:
             /* bcmStatGroupModeIntPri */
             if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS) {
                 return _bcm_esw_stat_flex_set_group_mode(
                            unit,
                            bcmStatFlexDirectionIngress,
                            offset_mode,
                            bcmStatGroupModeIntPri);
             }
             return BCM_E_PARAM;
        case 17: 
             /* bcmStatGroupModeDlfIntPri */
             if (ing_attr->packet_attr_type == 
                 bcmStatFlexPacketAttrTypeCompressed) {
                 if ((ing_cmprsd_pkt_attr_bits->int_pri == 4) &&
                     (ing_cmprsd_pkt_attr_bits->pkt_resolution == 1)) {
                      return _bcm_esw_stat_flex_set_group_mode(
                                 unit,
                                 bcmStatFlexDirectionIngress,
                                 offset_mode,
                                 bcmStatGroupModeDlfIntPri);
                 }
             }
             return BCM_E_PARAM;
        case 18: 
             /* bcmStatGroupModeDlfIntPriWithControl */
             if (ing_attr->packet_attr_type == 
                 bcmStatFlexPacketAttrTypeCompressed) {
                 if ((ing_cmprsd_pkt_attr_bits->int_pri == 4) &&
                     (ing_cmprsd_pkt_attr_bits->pkt_resolution == 2)) {
                      return _bcm_esw_stat_flex_set_group_mode(
                                 unit,
                                 bcmStatFlexDirectionIngress,
                                 offset_mode,
                                 bcmStatGroupModeDlfIntPriWithControl);
                 }
             }
             return BCM_E_PARAM;
        case 20: 
             /* bcmStatGroupModeTypedIntPri */
             if (ing_attr->packet_attr_type == 
                 bcmStatFlexPacketAttrTypeCompressed) {
                 if ((ing_cmprsd_pkt_attr_bits->int_pri == 4) &&
                     (ing_cmprsd_pkt_attr_bits->pkt_resolution == 3)) {
                      return _bcm_esw_stat_flex_set_group_mode(
                                 unit,
                                 bcmStatFlexDirectionIngress,
                                 offset_mode,
                                 bcmStatGroupModeTypedIntPri);
                 }
             }
             return BCM_E_PARAM;
        case 21: 
             /* bcmStatGroupModeTypedIntPriWithControl */
             if (ing_attr->packet_attr_type == 
                 bcmStatFlexPacketAttrTypeCompressed) {
                 if ((ing_cmprsd_pkt_attr_bits->int_pri == 4) &&
                     (ing_cmprsd_pkt_attr_bits->pkt_resolution == 3)) {
                      return _bcm_esw_stat_flex_set_group_mode(
                                 unit,
                                 bcmStatFlexDirectionIngress,
                                 offset_mode,
                                 bcmStatGroupModeTypedIntPriWithControl);
                 }
             }
             return BCM_E_PARAM;
        case 64: 
             /* bcmStatGroupModeIntPriCng , bcmStatGroupModeDscp */
             if (ing_attr->packet_attr_type ==  
                 bcmStatFlexPacketAttrTypeUncompressed) {
                 if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                     BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_TOS_ATTR_BITS) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionIngress,
                                offset_mode,
                                bcmStatGroupModeDscp);
                 }
                 if (ing_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                     (BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_CNG_ATTR_BITS|
                      BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS)) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionIngress,
                                offset_mode,
                                bcmStatGroupModeIntPriCng);
                 }
             }
             return BCM_E_PARAM;
        default:
             return BCM_E_PARAM;
        }
    } else {
        if (flex_egress_modes[unit][offset_mode].available == 0) {
            return BCM_E_NONE;
        }
        egr_attr                    = &(flex_egress_modes[unit][offset_mode].
                                        egr_attr);
        egr_uncmprsd_attr_selectors = &(egr_attr->uncmprsd_attr_selectors);
        egr_cmprsd_attr_selectors   = &(egr_attr->cmprsd_attr_selectors);
        egr_cmprsd_pkt_attr_bits    = &(egr_cmprsd_attr_selectors->
                                        pkt_attr_bits);
        switch(flex_egress_modes[unit][offset_mode].total_counters) {
        case 1: 
             /* bcmStatGroupModeSingle, bcmStatGroupModeDlfAll ,
                bcmStatGroupModeSingleWithControl */
             if ((egr_attr->packet_attr_type ==  
                  bcmStatFlexPacketAttrTypeUncompressed) &&
                 (egr_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                  BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS)){
                  return _bcm_esw_stat_flex_set_group_mode(
                             unit,
                             bcmStatFlexDirectionEgress,
                             offset_mode,
                             bcmStatGroupModeSingle);
             }
             return BCM_E_PARAM;
        case 2: 
             /* bcmStatGroupModeTrafficType, bcmStatGroupModeTyped , 
                bcmStatGroupModeTypedAll,
                bcmStatGroupModeTrafficTypeWithControl,
                bcmStatGroupModeDlfAllWithControl,
                bcmStatGroupModeTypedWithControl, 
                bcmStatGroupModeTypedAllWithControl, bcmStatGroupModeSvpType , 
                bcmStatGroupModeDvpType */
             if (egr_attr->packet_attr_type ==  
                 bcmStatFlexPacketAttrTypeUncompressed) {
                 if (egr_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                     BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DVP_TYPE_ATTR_BITS) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                         unit,bcmStatFlexDirectionEgress,
                                         offset_mode,bcmStatGroupModeDvpType);
                    }
                 if (egr_uncmprsd_attr_selectors->
                     uncmprsd_attr_bits_selector ==
                     BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionEgress,
                                offset_mode,
                                bcmStatGroupModeSvpType);
                 }
                 if (egr_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                  BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionEgress,
                                offset_mode,
                                bcmStatGroupModeTrafficType);
                 }
             }
             return BCM_E_PARAM;
        case 8: 
             /* bcmStatGroupModeDot1P */
             if (egr_attr->packet_attr_type ==  
                 bcmStatFlexPacketAttrTypeUncompressed) {
                 if (egr_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                     BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionEgress,
                                offset_mode,
                                bcmStatGroupModeDot1P);
                 }
             }
             return BCM_E_PARAM;
        case 16:
             /* bcmStatGroupModeIntPri */
             if (egr_attr->packet_attr_type ==  
                 bcmStatFlexPacketAttrTypeUncompressed) {
                 if (egr_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                     BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionEgress,
                                offset_mode,
                                bcmStatGroupModeIntPri);
                 }
             }
             return BCM_E_PARAM;
        case 18: 
             /* bcmStatGroupModeTypedIntPri */
             if (egr_attr->packet_attr_type ==  
                 bcmStatFlexPacketAttrTypeCompressed) {
                 if ((egr_cmprsd_pkt_attr_bits->int_pri == 4) &&
                     (egr_cmprsd_pkt_attr_bits->pkt_resolution == 1)) {
                      return _bcm_esw_stat_flex_set_group_mode(
                                 unit,
                                 bcmStatFlexDirectionEgress,
                                 offset_mode,
                                 bcmStatGroupModeTypedIntPri);
                  }
             }
             return BCM_E_PARAM; 
        case 64: 
             /* bcmStatGroupModeIntPriCng , bcmStatGroupModeDscp */
             if (egr_attr->packet_attr_type ==  
                 bcmStatFlexPacketAttrTypeUncompressed) {
                 if (egr_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                     BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_TOS_ATTR_BITS) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionEgress,
                                offset_mode,
                                bcmStatGroupModeDscp);
                 }
                 if (egr_uncmprsd_attr_selectors->uncmprsd_attr_bits_selector ==
                     (BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_CNG_ATTR_BITS|
                      BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS)) {
                     return _bcm_esw_stat_flex_set_group_mode(
                                unit,
                                bcmStatFlexDirectionEgress,
                                offset_mode,
                                bcmStatGroupModeIntPriCng);
                 }
             }
             return BCM_E_PARAM;
        default:
             return BCM_E_PARAM;
        }
    }
    return BCM_E_PARAM;
}


/*
 * Function:
 *      _bcm_esw_stat_flex_enable_pool
 * Description:
 *      Enable/Disable ingress/egress flex counter pool.
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static bcm_error_t _bcm_esw_stat_flex_enable_pool(
            int                       unit,
            bcm_stat_flex_direction_t direction,
            soc_reg_t                 flex_pool_ctr_update_control_reg,
            uint32                    enable)
{
    uint32  flex_pool_ctr_update_control_reg_value=0;
    uint32  enable_value=1;
    uint32  num_pools[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32  index=0;

    num_pools[bcmStatFlexDirectionIngress]=
             SOC_INFO(unit).num_flex_ingress_pools;
    num_pools[bcmStatFlexDirectionEgress] =
             SOC_INFO(unit).num_flex_egress_pools;
    if (direction >= BCM_STAT_FLEX_COUNTER_MAX_DIRECTION) {
        return BCM_E_PARAM;
    }
    for(index=0;
        index < num_pools[direction];
        index++) {
        if (_pool_ctr_register[direction][index] == 
            flex_pool_ctr_update_control_reg ) {
            break;
        }
    }
    if (index == num_pools[direction]) {
         return BCM_E_PARAM;
    }
    if ( enable ) {
        FLEXCTR_VVERB(("...Enabling pool:%s \n",
               SOC_REG_NAME(unit, flex_pool_ctr_update_control_reg)));
    } else {
        FLEXCTR_VVERB(("...Disabling pool:%s \n",
               SOC_REG_NAME(unit, flex_pool_ctr_update_control_reg)));
    }
    /* First Get complete value of
       EGR/ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_?r value */
    SOC_IF_ERROR_RETURN(soc_reg32_get(
                        unit,
                        flex_pool_ctr_update_control_reg,
                        REG_PORT_ANY, 0,
                        &flex_pool_ctr_update_control_reg_value));
    /* Next set field value for
       EGR/ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_?r:COUNTER_POOL_ENABLE field*/
    if (enable) {
        enable_value=1;
    }else {
        enable_value=0;
    }
    soc_reg_field_set(unit,
                      flex_pool_ctr_update_control_reg,
                      &flex_pool_ctr_update_control_reg_value,
                      COUNTER_POOL_ENABLEf,
                      enable_value);
    /* Finally set value for
       EGR/ING_FLEX_CTR_COUNTER_UPDATE_CONTROL_?r:COUNTER_POOL_ENABLE field*/
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit,
                                      flex_pool_ctr_update_control_reg,
                                      REG_PORT_ANY, 
                                      0,
                                      flex_pool_ctr_update_control_reg_value));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_stat_flex_retrieve_total_counters
 * Description:
 *      Retries total counter based on flex h/w configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static uint32 
_bcm_esw_stat_flex_retrieve_total_counters(
    int                       unit,
    bcm_stat_flex_direction_t direction,
    uint32                    pool_number,
    uint32                    offset_mode)
{
    uint32                             index=0;
    uint32                             *flex_ctr_offset_table_entry=NULL;
    uint32                             count_enable=0;
    bcm_stat_flex_offset_table_entry_t *sw_offset_table=NULL;
    uint32                             offset=0;
    uint32                             total_counters_flag=0;
    uint32                             total_counters=0;

    flex_ctr_offset_table_entry = soc_cm_salloc(unit,sizeof(uint32)*(256),
                                                "flex_ctr_offset_table_entry");
    if (flex_ctr_offset_table_entry == NULL) {
        FLEXCTR_ERR(("Memory Allocation failed:flex_ctr_offset_table_entry\n"));
        return  BCM_E_INTERNAL;
    }
    /* 0-255,256-511,512-...*/
    if (soc_mem_read_range(unit,
                     _ctr_offset_table[direction][pool_number],
                     MEM_BLOCK_ANY,
                     (offset_mode <<8),
                     (offset_mode <<8)+ (256) - 1,
                     flex_ctr_offset_table_entry) != SOC_E_NONE) {
        FLEXCTR_ERR(("Memory Reading failed:flex_ctr_offset_table_entry \n"));
        soc_cm_sfree(unit,flex_ctr_offset_table_entry);
        return 0;
    }
    if (direction == bcmStatFlexDirectionIngress) {
        switch(flex_ingress_modes[unit][offset_mode].ing_attr.
               packet_attr_type) {
        case bcmStatFlexPacketAttrTypeUncompressed:
             sw_offset_table = &flex_ingress_modes[unit][offset_mode].ing_attr.
                                uncmprsd_attr_selectors.offset_table_map[0];
             break;
        case bcmStatFlexPacketAttrTypeCompressed:
             sw_offset_table = &flex_ingress_modes[unit][offset_mode].ing_attr.
                                cmprsd_attr_selectors.offset_table_map[0];
             break;
        case bcmStatFlexPacketAttrTypeUdf:
        default:
             /* With current implemControl must not reach over here */
             return BCM_E_PARAM;
    }
    } else {
        switch(flex_egress_modes[unit][offset_mode].egr_attr.packet_attr_type) {
        case bcmStatFlexPacketAttrTypeUncompressed:
             sw_offset_table = &flex_egress_modes[unit][offset_mode].egr_attr.
                                uncmprsd_attr_selectors.offset_table_map[0];
             break;
        case bcmStatFlexPacketAttrTypeCompressed:
             sw_offset_table = &flex_egress_modes[unit][offset_mode].egr_attr.
                                cmprsd_attr_selectors.offset_table_map[0];
             break;
        case bcmStatFlexPacketAttrTypeUdf:
        default:
             /* With current implemControl must not reach over here */
             return BCM_E_PARAM;
        }
    }
    for (index=0;index<(256);index++) {
         count_enable=0;
         /*First Get complete value of EGR/ING_FLEX_CTR_OFFSET_TABLE_?m value */
         soc_mem_field_get(unit,
                           _ctr_offset_table[direction][pool_number],
                           &flex_ctr_offset_table_entry[index],
                           COUNT_ENABLEf,&count_enable);
         soc_mem_field_get(unit,
                           _ctr_offset_table[direction][pool_number],
                           &flex_ctr_offset_table_entry[index],
                           OFFSETf,
                           &offset);
         if (count_enable) {
             total_counters_flag=1;
             if (total_counters < offset) {
                 total_counters = offset;
             }
         }
         sw_offset_table[index].offset = offset;
         sw_offset_table[index].count_enable = count_enable;
    }
    soc_cm_sfree(unit,flex_ctr_offset_table_entry);
    return (total_counters+total_counters_flag);
}

/*
 * Function:
 *      _bcm_esw_stat_flex_check_egress_table
 * Description:
 *      Checkes flex egress table. If table were configured for flex counter
 *      updates s/w copy accordingly.    
 *
 * Parameters:
 *      unit                  - (IN) unit number
 *      egress_table          - (IN) Flex Egress  Table
 *      index                 - (IN) Flex Egress  Table Index
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only.
 *
 */
static void
_bcm_esw_stat_flex_check_egress_table(
    int       unit,
    soc_mem_t egress_table,
    int       index)
{
    uint32            offset_mode=0;
    uint32            pool_number=0;
    uint32            base_idx=0;
    uint32            egress_entry_data_size=0;
    void              *egress_entry_data=NULL;
    bcm_stat_object_t object=bcmStatObjectEgrPort;

    if (!((egress_table == EGR_VLANm) ||
          (egress_table == EGR_VFIm)  ||
          (egress_table == EGR_L3_NEXT_HOPm)  ||
          (egress_table == EGR_VLAN_XLATEm)  ||
          (egress_table == EGR_PORTm))) {
           FLEXCTR_ERR(("Invalid Flex Counter Ingress Memory %s\n",
                          SOC_MEM_UFNAME(unit, egress_table)));
           return;
    }
    egress_entry_data_size= WORDS2BYTES(BYTES2WORDS(
                                        SOC_MEM_INFO(unit,egress_table).bytes));
    egress_entry_data = sal_alloc(egress_entry_data_size,"egress_table");
    if (egress_entry_data == NULL) {
        FLEXCTR_ERR(("Failed to allocate memory for Table:%s ",
                     SOC_MEM_UFNAME(unit, egress_table)));
        return ;
    }
    if (_bcm_esw_stat_flex_get_egress_object(egress_table,&object)
        != BCM_E_NONE) {
        return ;
    }
    sal_memset(egress_entry_data,0,SOC_MEM_INFO(unit, egress_table).bytes);

    if (soc_mem_read(unit, egress_table, MEM_BLOCK_ANY,index,egress_entry_data)
        == SOC_E_NONE) {
        if (soc_mem_field_valid(unit,egress_table,VALIDf)) {
            if (soc_mem_field32_get(unit,egress_table,egress_entry_data,
                VALIDf)==0) {
                sal_free(egress_entry_data);
                return;
            }
        }
        _bcm_esw_get_flex_counter_fields_values(
                          unit,egress_table,egress_entry_data,
                          &offset_mode,&pool_number,&base_idx);
        if ((offset_mode == 0) && (base_idx == 0)) {
             sal_free(egress_entry_data);
             return;
        }
        flex_base_index_reference_count[unit][bcmStatFlexDirectionEgress]
                                       [pool_number][base_idx]++;
        if (flex_base_index_reference_count[unit][bcmStatFlexDirectionEgress]
                                           [pool_number][base_idx] == 1) {
            flex_egress_modes[unit][offset_mode].reference_count++;
        }
        if (flex_egress_modes[unit][offset_mode].total_counters == 0) {
            flex_egress_modes[unit][offset_mode].total_counters =
               _bcm_esw_stat_flex_retrieve_total_counters(
                  unit, bcmStatFlexDirectionEgress, pool_number, offset_mode); 
            FLEXCTR_VVERB(("Max_offset_table_value %d\n",
                           flex_egress_modes[unit][offset_mode].
                           total_counters));
        }
        shr_aidxres_list_reserve_block(
            flex_aidxres_list_handle
            [unit][bcmStatFlexDirectionEgress][pool_number],
            base_idx,
            flex_egress_modes[unit][offset_mode].total_counters);
        flex_pool_stat[unit][bcmStatFlexDirectionEgress][pool_number].
                      used_entries += flex_egress_modes[unit][offset_mode].
                                                       total_counters;
        flex_pool_stat[unit][bcmStatFlexDirectionEgress][pool_number].
                      used_by_tables |= flex_used_by_table[object];
        FLEXCTR_VVERB(("Table:%s index=%d mode:%d pool_number:%d base_idx:%d\n",
                       SOC_MEM_UFNAME(unit, egress_table),
                       index,offset_mode,pool_number,base_idx));
    }
    sal_free(egress_entry_data);
    return;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_table
 * Description:
 *      Checkes flex ingress table. If table were configured for flex counter
 *      updates s/w copy accordingly.    
 *
 * Parameters:
 *      unit                  - (IN) unit number
 *      ingress_table         - (IN) Flex Ingress  Table
 *      index                 - (IN) Flex Ingress  Table Index
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only.
 *
 */
static void 
_bcm_esw_stat_flex_check_ingress_table(
    int       unit,
    soc_mem_t ingress_table,
    int       index)
{
    uint32            offset_mode=0;
    uint32            pool_number=0;
    uint32            base_idx=0;
    uint32            ingress_entry_data_size=0;
    void              *ingress_entry_data=NULL;
    bcm_stat_object_t object=bcmStatObjectIngPort;

    if (!((ingress_table == PORT_TABm) ||
          (ingress_table == VLAN_XLATEm)  ||
          (ingress_table == VFP_POLICY_TABLEm)  ||
          (ingress_table == MPLS_ENTRYm)  ||
          (ingress_table == SOURCE_VPm)  ||
          (ingress_table == L3_IIFm)  ||
          (ingress_table == VRFm)  ||
          (ingress_table == VFIm)  ||
          (ingress_table == VLAN_TABm))) {
           FLEXCTR_ERR(("Invalid Flex Counter Ingress Memory %s\n",
                        SOC_MEM_UFNAME(unit, ingress_table)));
           return;
    }
    ingress_entry_data_size = WORDS2BYTES(BYTES2WORDS(
                              SOC_MEM_INFO(unit, ingress_table).bytes));
    ingress_entry_data = sal_alloc(ingress_entry_data_size,"ingress_table");
    if (ingress_entry_data == NULL) {
        FLEXCTR_ERR(("Failed to allocate memory for Table:%s ",
                     SOC_MEM_UFNAME(unit, ingress_table)));
        return ;
    }
    if (_bcm_esw_stat_flex_get_ingress_object(ingress_table,&object) 
        != BCM_E_NONE) {
        return ;
    }
    sal_memset(ingress_entry_data,0,SOC_MEM_INFO(unit, ingress_table).bytes);

    if (soc_mem_read(unit, ingress_table, MEM_BLOCK_ANY,index,
        ingress_entry_data) == SOC_E_NONE) {
        if (soc_mem_field_valid(unit,ingress_table,VALIDf)) {
            if (soc_mem_field32_get(unit,ingress_table,ingress_entry_data,
                VALIDf)==0) {
                sal_free(ingress_entry_data);
                return ;
            }
        }
        _bcm_esw_get_flex_counter_fields_values(
               unit,ingress_table,ingress_entry_data,
               &offset_mode,&pool_number,&base_idx);
        if ((offset_mode == 0) && (base_idx == 0)) {
            sal_free(ingress_entry_data);
            return ;
        }
        flex_base_index_reference_count[unit][bcmStatFlexDirectionIngress]
                                       [pool_number][base_idx]++;
        if (flex_base_index_reference_count[unit][bcmStatFlexDirectionIngress]
                                           [pool_number][base_idx] == 1) {
            flex_ingress_modes[unit][offset_mode].reference_count++;
        }
        if (flex_ingress_modes[unit][offset_mode].total_counters == 0) {
            flex_ingress_modes[unit][offset_mode].total_counters =
                   _bcm_esw_stat_flex_retrieve_total_counters(
                    unit, bcmStatFlexDirectionIngress,
                    pool_number, offset_mode)  ;
            FLEXCTR_VVERB(("Max_offset_table_value %d\n",
                           flex_ingress_modes[unit][offset_mode].
                           total_counters));
        }
        shr_aidxres_list_reserve_block(
            flex_aidxres_list_handle[unit][bcmStatFlexDirectionIngress]
                                    [pool_number],
            base_idx,
            flex_ingress_modes[unit][offset_mode].
            total_counters);
        flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                      [pool_number].used_entries +=
                      flex_ingress_modes[unit][offset_mode].total_counters;
        flex_pool_stat[unit][bcmStatFlexDirectionIngress][pool_number].
                      used_by_tables |= flex_used_by_table[object];
        FLEXCTR_VVERB(("Table:%s:index=%d mode:%d pool_number:%d base_idx:%d\n",
                       SOC_MEM_UFNAME(unit, ingress_table),
                       index,offset_mode,pool_number, base_idx));
    }
    sal_free(ingress_entry_data);
    return;
}

/*
 * Function:
 *      _bcm_esw_stat_flex_init_uncmprsd_mode
 * Description:
 *      Configures flex hw for uncompressed mode
 * Parameters:
 *      unit                           - (IN) unit number
 *      mode                           - (IN) Flex Offset Mode
 *      direction                      - (IN) Flex Data flow direction(ing/egr)
 *      selector_count                 - (IN) Flex Key selector count
 *      selector_x_en_field_value      - (IN) Flex Key selector fields
 *      selector_for_bit_x_field_value - (IN) Flex Key selector field bits
 *
 * Return Value:
 *      None
 * Notes:
 *
 */
static void 
_bcm_esw_stat_flex_init_uncmprsd_mode(
    int                       unit,
    int                       mode,
    bcm_stat_flex_direction_t direction,
    uint32                    selector_count,
    uint32                    selector_x_en_field_value[8],
    uint32                    selector_for_bit_x_field_value[8])
{
    uint32  uncmprsd_attr_bits_selector=0;
    uint32  index=0;

    if (direction == bcmStatFlexDirectionIngress) {
        flex_ingress_modes[unit][mode].available=1;
        /*Will be changed WhileReadingTables */
        flex_ingress_modes[unit][mode].reference_count=0;
        /* selector_count can not be used for deciding total counters !!! */
        /* flex_ingress_modes[unit][mode].total_counters=(1<< selector_count);*/
        flex_ingress_modes[unit][mode].ing_attr.packet_attr_type =
            bcmStatFlexPacketAttrTypeUncompressed;
        for (uncmprsd_attr_bits_selector=0,index=0;index<8;index++) {
             if(selector_x_en_field_value[index] != 0) {
                if (selector_for_bit_x_field_value[index] == 0) {
                    uncmprsd_attr_bits_selector |=
                      BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS;
                    continue;
                }
                if (selector_for_bit_x_field_value[index] == 1) {
                    uncmprsd_attr_bits_selector |=
                      BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_DROP_ATTR_BITS;
                    continue;
                }
                if (selector_for_bit_x_field_value[index] == 2) {
                    uncmprsd_attr_bits_selector |=
                      BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 3) &&
                    (selector_for_bit_x_field_value[index] <= 8)) {
                     uncmprsd_attr_bits_selector|=
                   BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 9) &&
                    (selector_for_bit_x_field_value[index] <= 16)) {
                     uncmprsd_attr_bits_selector|=
                       BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_TOS_ATTR_BITS;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 17) &&
                    (selector_for_bit_x_field_value[index] <= 22)) {
                     uncmprsd_attr_bits_selector|=
                      BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INGRESS_PORT_ATTR_BITS;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 23) &&
                    (selector_for_bit_x_field_value[index] <= 25)) {
                     uncmprsd_attr_bits_selector|=
                       BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 26) &&
                    (selector_for_bit_x_field_value[index] <= 28)) {
                     uncmprsd_attr_bits_selector|=
                       BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 29) &&
                    (selector_for_bit_x_field_value[index] <= 30)) {
                     uncmprsd_attr_bits_selector|=
                       BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 31) &&
                    (selector_for_bit_x_field_value[index] <= 34)) {
                     uncmprsd_attr_bits_selector|=
                       BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 35) &&
                    (selector_for_bit_x_field_value[index] <= 36)) {
                     uncmprsd_attr_bits_selector|=
                       BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_IFP_CNG_ATTR_BITS;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 37) &&
                    (selector_for_bit_x_field_value[index] <= 38)) {
                     uncmprsd_attr_bits_selector|=
                       BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_CNG_ATTR_BITS;
                     continue;
                }
             }
        }
        flex_ingress_modes[unit][mode].ing_attr.uncmprsd_attr_selectors.
                     uncmprsd_attr_bits_selector = uncmprsd_attr_bits_selector;
        FLEXCTR_VVERB(("uncmprsd_attr_bits_selector:%x \n",
                       uncmprsd_attr_bits_selector));
        return;
    }
    if (direction == bcmStatFlexDirectionEgress) {
        flex_egress_modes[unit][mode].available=1;
        /* Will be changed WhileReadingTables */
        flex_egress_modes[unit][mode].reference_count=0;
        flex_egress_modes[unit][mode].egr_attr.packet_attr_type =
                                      bcmStatFlexPacketAttrTypeUncompressed;
        for (uncmprsd_attr_bits_selector=0,index=0;index<8;index++) {
             if(selector_x_en_field_value[index] != 0) {
                if (selector_for_bit_x_field_value[index] == 0) {
                    uncmprsd_attr_bits_selector |=
                      BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS;
                    continue;
                }
                if (selector_for_bit_x_field_value[index] == 1) {
                    uncmprsd_attr_bits_selector |=
                      BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DROP_ATTR_BITS;
                    continue;
                }
                if (selector_for_bit_x_field_value[index] == 2) {
                    uncmprsd_attr_bits_selector |=
                      BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DVP_TYPE_ATTR_BITS;
                    continue;
                }
                if (selector_for_bit_x_field_value[index] == 3) {
                    uncmprsd_attr_bits_selector |=
                      BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS;
                    continue;
                }
                if (selector_for_bit_x_field_value[index] == 4) {
                    uncmprsd_attr_bits_selector |=
                    BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 5) &&
                    (selector_for_bit_x_field_value[index] <= 12)) {
                     uncmprsd_attr_bits_selector |=
                       BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_TOS_ATTR_BITS;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 13) &&
                    (selector_for_bit_x_field_value[index] <= 18)) {
                     uncmprsd_attr_bits_selector |=
                       BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_EGRESS_PORT_ATTR_BITS;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 19) &&
                    (selector_for_bit_x_field_value[index] <= 21)) {
                     uncmprsd_attr_bits_selector |=
                       BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 22) &&
                    (selector_for_bit_x_field_value[index] <= 24)) {
                     uncmprsd_attr_bits_selector |=
                       BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 25) &&
                    (selector_for_bit_x_field_value[index] <= 26)) {
                     uncmprsd_attr_bits_selector |=
                       BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 27) &&
                    (selector_for_bit_x_field_value[index] <= 30)) {
                     uncmprsd_attr_bits_selector |=
                       BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 31) &&
                    (selector_for_bit_x_field_value[index] <= 32)) {
                     uncmprsd_attr_bits_selector |=
                       BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_CNG_ATTR_BITS;
                     continue;
                }
            }
        }
        flex_egress_modes[unit][mode].egr_attr.uncmprsd_attr_selectors.
                    uncmprsd_attr_bits_selector = uncmprsd_attr_bits_selector;
        FLEXCTR_VVERB(("uncmprsd_attr_bits_selector:%x \n",
                       uncmprsd_attr_bits_selector));
        return;
    }
    FLEXCTR_ERR(("Ooops. Control Must not reach over here \n"));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_init_cmprsd_mode
 * Description:
 *      Configures flex hw for compressed mode
 * Parameters:
 *      unit                           - (IN) unit number
 *      mode                           - (IN) Flex Offset Mode
 *      direction                      - (IN) Flex Data flow direction(ing/egr)
 *      selector_count                 - (IN) Flex Key selector count
 *      selector_x_en_field_value      - (IN) Flex Key selector fields
 *      selector_for_bit_x_field_value - (IN) Flex Key selector field bits
 *
 * Return Value:
 *      None
 * Notes:
 *
 */
static void 
_bcm_esw_stat_flex_init_cmprsd_mode(
    int                       unit,
    int                       mode,
    bcm_stat_flex_direction_t direction,
    uint32                    selector_count,
    uint32                    selector_x_en_field_value[8],
    uint32                    selector_for_bit_x_field_value[8])
{
    uint32  index=0;
    char    *dma_buff=NULL;
    if (direction == bcmStatFlexDirectionIngress) {
       /* memset(&flex_ingress_modes[unit][mode],0,
         sizeof(flex_ingress_modes[unit][mode])); */
       flex_ingress_modes[unit][mode].available=1;
       /*Will be changed WhileReadingTables */
       flex_ingress_modes[unit][mode].reference_count=0;
       flex_ingress_modes[unit][mode].total_counters= 0;
       flex_ingress_modes[unit][mode].ing_attr.packet_attr_type =
            bcmStatFlexPacketAttrTypeCompressed;
       for (index=0;index<8;index++) {
            if(selector_x_en_field_value[index] != 0) {
               if (selector_for_bit_x_field_value[index] == 0) {
                   flex_ingress_modes[unit][mode].ing_attr.
                    cmprsd_attr_selectors.pkt_attr_bits.ip_pkt = 1;
                   continue;
               }
               if (selector_for_bit_x_field_value[index] == 1) {
                   flex_ingress_modes[unit][mode].ing_attr.
                    cmprsd_attr_selectors.pkt_attr_bits.drop = 1;
                   continue;
               }
               if (selector_for_bit_x_field_value[index] == 2) {
                   flex_ingress_modes[unit][mode].ing_attr.
                    cmprsd_attr_selectors.pkt_attr_bits.svp_type = 1;
                   continue;
               }
               if ((selector_for_bit_x_field_value[index] >= 3) &&
                   (selector_for_bit_x_field_value[index] <= 8)) {
                    flex_ingress_modes[unit][mode].ing_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.pkt_resolution++;
                    continue;
               }
               if ((selector_for_bit_x_field_value[index] >= 9) &&
                   (selector_for_bit_x_field_value[index] <= 16)) {
                    flex_ingress_modes[unit][mode].ing_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.tos++;
                    continue;
               }
               if ((selector_for_bit_x_field_value[index] >= 17) &&
                   (selector_for_bit_x_field_value[index] <= 22)) {
                    flex_ingress_modes[unit][mode].ing_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.ing_port++;
                    continue;
               }
               if ((selector_for_bit_x_field_value[index] >= 23) &&
                   (selector_for_bit_x_field_value[index] <= 25)) {
                    flex_ingress_modes[unit][mode].ing_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.inner_dot1p++;
                    continue;
               }
               if ((selector_for_bit_x_field_value[index] >= 26) &&
                   (selector_for_bit_x_field_value[index] <= 28)) {
                   flex_ingress_modes[unit][mode].ing_attr.
                    cmprsd_attr_selectors.pkt_attr_bits.outer_dot1p++;
                   continue;
               }
               if ((selector_for_bit_x_field_value[index] >= 29) &&
                   (selector_for_bit_x_field_value[index] <= 30)) {
                    flex_ingress_modes[unit][mode].ing_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.vlan_format++;
                    continue;
               }
               if ((selector_for_bit_x_field_value[index] >= 31) &&
                   (selector_for_bit_x_field_value[index] <= 34)) {
                    flex_ingress_modes[unit][mode].ing_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.int_pri++;
                    continue;
               }
               if ((selector_for_bit_x_field_value[index] >= 35) &&
                   (selector_for_bit_x_field_value[index] <= 36)) {
                   flex_ingress_modes[unit][mode].ing_attr.
                    cmprsd_attr_selectors.pkt_attr_bits.ifp_cng++;
                   continue;
               }
               if ((selector_for_bit_x_field_value[index] >= 37) &&
                   (selector_for_bit_x_field_value[index] <= 38)) {
                   flex_ingress_modes[unit][mode].ing_attr.
                    cmprsd_attr_selectors.pkt_attr_bits.cng++;
                   continue;
               }
           }
       }
       if ((flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
            pkt_attr_bits.cng ) ||
           (flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
            pkt_attr_bits.ifp_cng ) ||
           (flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
            pkt_attr_bits.int_pri ))         {
            FLEXCTR_VVERB(("cng:%d \n",
                           flex_ingress_modes[unit][mode].ing_attr.
                           cmprsd_attr_selectors.pkt_attr_bits.cng));
            FLEXCTR_VVERB(("ifp_cng:%d \n",
                           flex_ingress_modes[unit][mode].ing_attr.
                           cmprsd_attr_selectors.pkt_attr_bits.ifp_cng));
            FLEXCTR_VVERB(("int_pri:%d \n",
                           flex_ingress_modes[unit][mode].ing_attr.
                           cmprsd_attr_selectors.pkt_attr_bits.int_pri));
            dma_buff = soc_cm_salloc(
                           unit,
                           sizeof(bcm_stat_flex_ing_cmprsd_pri_cnf_attr_map_t),
                           "ING_FLEX_CTR_PRI_CNG_MAPm");
            if (dma_buff == NULL) {
                FLEXCTR_ERR(("ING_FLEX_CTR_PRI_CNG_MAPm:DMAAllocationFail\n"));
                return;
            }
            if (soc_mem_read_range(
                    unit,
                    ING_FLEX_CTR_PRI_CNG_MAPm,
                    MEM_BLOCK_ANY,
                    0,
                    sizeof(bcm_stat_flex_ing_cmprsd_pri_cnf_attr_map_t) - 1,
                    dma_buff) != SOC_E_NONE) {
                FLEXCTR_ERR(("ING_FLEX_CTR_PRI_CNG_MAPm:Read failuer \n"));
                soc_cm_sfree(unit,dma_buff);
                return;
            }
            sal_memcpy(&flex_ingress_modes[unit][mode].ing_attr.
                       cmprsd_attr_selectors.pri_cnf_attr_map[0],
                       dma_buff,
                       sizeof(bcm_stat_flex_ing_cmprsd_pri_cnf_attr_map_t));
            soc_cm_sfree(unit,dma_buff);
            for (index=0;
                 index< sizeof(bcm_stat_flex_ing_cmprsd_pri_cnf_attr_map_t);
                 index++) {
                 if (flex_ingress_modes[unit][mode].ing_attr.
                     cmprsd_attr_selectors.
                     pri_cnf_attr_map[index]) {
                     FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                  index,flex_ingress_modes[unit][mode].
                                  ing_attr.cmprsd_attr_selectors.
                                  pri_cnf_attr_map[index]));
                 }
            }
       }
       if ((flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
            pkt_attr_bits.vlan_format) ||
           (flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
            pkt_attr_bits.outer_dot1p) ||
           (flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
            pkt_attr_bits.inner_dot1p)) {
            FLEXCTR_VVERB(("vlan_format:%d \n",
                           flex_ingress_modes[unit][mode].ing_attr.
                           cmprsd_attr_selectors.pkt_attr_bits.vlan_format));
            FLEXCTR_VVERB(("outer_dot1p:%d \n",
                           flex_ingress_modes[unit][mode].ing_attr.
                           cmprsd_attr_selectors.pkt_attr_bits.outer_dot1p));
            FLEXCTR_VVERB(("inner_dot1p:%d \n",
                           flex_ingress_modes[unit][mode].ing_attr.
                           cmprsd_attr_selectors.pkt_attr_bits.inner_dot1p));
            dma_buff = soc_cm_salloc(
                           unit,
                           sizeof(bcm_stat_flex_ing_cmprsd_pkt_pri_attr_map_t),
                           "ING_FLEX_CTR_PKT_PRI_MAPm");
            if (dma_buff == NULL) {
                FLEXCTR_ERR(("ING_FLEX_CTR_PKT_PRI_MAPm:DMAAllocationFail\n"));
                return;
            }
            if (soc_mem_read_range(
                    unit,
                    ING_FLEX_CTR_PKT_PRI_MAPm,
                    MEM_BLOCK_ANY,
                    0,
                    sizeof(bcm_stat_flex_ing_cmprsd_pkt_pri_attr_map_t)-1,
                    dma_buff) != SOC_E_NONE) {
                FLEXCTR_VVERB(("ING_FLEX_CTR_PKT_PRI_MAPm:Read failuer \n"));
                soc_cm_sfree(unit,dma_buff);
                return;
            }
            sal_memcpy(&flex_ingress_modes[unit][mode].ing_attr.
                       cmprsd_attr_selectors.pkt_pri_attr_map[0],
                       dma_buff,
                       sizeof(bcm_stat_flex_ing_cmprsd_pkt_pri_attr_map_t));
            soc_cm_sfree(unit,dma_buff);
            for (index=0;
                 index< sizeof(bcm_stat_flex_ing_cmprsd_pkt_pri_attr_map_t);
                 index++) {
                 if (flex_ingress_modes[unit][mode].ing_attr.
                     cmprsd_attr_selectors.
                     pkt_pri_attr_map[index]){
                     FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                    index,flex_ingress_modes[unit][mode].
                                    ing_attr.cmprsd_attr_selectors.
                                    pkt_pri_attr_map[index]));
                 }
            }
       }
       if (flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
           pkt_attr_bits.ing_port) {
           FLEXCTR_VVERB(("ing_port:%d \n",flex_ingress_modes[unit][mode].
                           ing_attr.cmprsd_attr_selectors.
                           pkt_attr_bits.ing_port));
           dma_buff = soc_cm_salloc(
                          unit,
                          sizeof(bcm_stat_flex_ing_cmprsd_port_attr_map_t),
                          "ING_FLEX_CTR_PORT_MAPm");
           if (dma_buff == NULL) {
               FLEXCTR_ERR(("ING_FLEX_CTR_PORT_MAPm:DMA Allocation failuer\n"));
               return;
           }
           if (soc_mem_read_range(
                   unit,
                   ING_FLEX_CTR_PORT_MAPm,
                   MEM_BLOCK_ANY,
                   0,
                   sizeof(bcm_stat_flex_ing_cmprsd_port_attr_map_t)-1,
                   dma_buff) != SOC_E_NONE) {
               FLEXCTR_ERR(("ING_FLEX_CTR_PORT_MAPm:Read failuer \n"));
               soc_cm_sfree(unit,dma_buff);
               return;
           }
           sal_memcpy(&flex_ingress_modes[unit][mode].ing_attr.
                      cmprsd_attr_selectors.port_attr_map[0],
                      dma_buff,
                      sizeof(bcm_stat_flex_ing_cmprsd_port_attr_map_t));
           soc_cm_sfree(unit,dma_buff);
           for (index=0;
                index< sizeof(bcm_stat_flex_ing_cmprsd_port_attr_map_t);
                index++) {
                if (flex_ingress_modes[unit][mode].ing_attr.
                    cmprsd_attr_selectors.port_attr_map[index]) {
                    FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                 index,flex_ingress_modes[unit][mode].ing_attr.
                                 cmprsd_attr_selectors.port_attr_map[index]));
                }
           }
       }
       if (flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
           pkt_attr_bits.tos ) {
           FLEXCTR_ERR(("tos:%d \n",
                        flex_ingress_modes[unit][mode].ing_attr.
                        cmprsd_attr_selectors.pkt_attr_bits.tos));
           dma_buff = soc_cm_salloc(
                          unit,
                          sizeof(bcm_stat_flex_ing_cmprsd_tos_attr_map_t),
                          "ING_FLEX_CTR_TOS_MAP");
           if (dma_buff == NULL) {
               FLEXCTR_ERR(("ING_FLEX_CTR_TOS_MAP:DMA Allocation failuer \n"));
               return;
           }
           if(soc_mem_read_range(
                  unit,
                  ING_FLEX_CTR_TOS_MAPm,
                  MEM_BLOCK_ANY,
                  0,
                  sizeof(bcm_stat_flex_ing_cmprsd_tos_attr_map_t)-1,
                  dma_buff) != SOC_E_NONE) {
              FLEXCTR_ERR(("ING_FLEX_CTR_TOS_MAPm:Read failuer \n"));
              soc_cm_sfree(unit,dma_buff);
              return;
           }
           sal_memcpy(&flex_ingress_modes[unit][mode].ing_attr.
                      cmprsd_attr_selectors.tos_attr_map[0],
                      dma_buff,
                      sizeof(bcm_stat_flex_ing_cmprsd_tos_attr_map_t));
           soc_cm_sfree(unit,dma_buff);
           for (index=0;
                index< sizeof(bcm_stat_flex_ing_cmprsd_tos_attr_map_t);
                index++) {
                if(flex_ingress_modes[unit][mode].ing_attr.
                   cmprsd_attr_selectors.tos_attr_map[index]) {
                   FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                index,
                                flex_ingress_modes[unit][mode].
                                ing_attr.cmprsd_attr_selectors.
                                tos_attr_map[index]));
                }
            }
       }
       if ((flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
             pkt_attr_bits.pkt_resolution) ||
           (flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
             pkt_attr_bits.svp_type) ||
           (flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
             pkt_attr_bits.drop)) {
            FLEXCTR_VVERB(("pkt_resolution:%d \n",
                           flex_ingress_modes[unit][mode].ing_attr.
                           cmprsd_attr_selectors.pkt_attr_bits.pkt_resolution));
            FLEXCTR_VVERB(("svp_type:%d \n",
                           flex_ingress_modes[unit][mode].ing_attr.
                           cmprsd_attr_selectors.pkt_attr_bits.svp_type));
            FLEXCTR_VVERB(("drop:%d \n",
                           flex_ingress_modes[unit][mode].ing_attr.
                           cmprsd_attr_selectors.pkt_attr_bits.drop));
            dma_buff= soc_cm_salloc(
                          unit,
                          sizeof(bcm_stat_flex_ing_cmprsd_pkt_res_attr_map_t),
                          "ING_FLEX_CTR_PKT_RES_MAPm");
            if (dma_buff == NULL) {
                FLEXCTR_ERR(("ING_FLEX_CTR_PKT_RES_MAPm:DMA AllocationFail\n"));
                return;
            }
            if(soc_mem_read_range(
                   unit,
                   ING_FLEX_CTR_PKT_RES_MAPm,
                   MEM_BLOCK_ANY,
                   0,
                   sizeof(bcm_stat_flex_ing_cmprsd_pkt_res_attr_map_t)-1,
                   dma_buff) != SOC_E_NONE) {
               FLEXCTR_ERR(("ING_FLEX_CTR_PKT_RES_MAPm:Read failuer \n"));
               soc_cm_sfree(unit,dma_buff);
               return;
            }
            sal_memcpy(&flex_ingress_modes[unit][mode].ing_attr.
                       cmprsd_attr_selectors.pkt_res_attr_map[0],
                       dma_buff,
                       sizeof(bcm_stat_flex_ing_cmprsd_pkt_res_attr_map_t));
            soc_cm_sfree(unit,dma_buff);
            for(index=0;
                index< sizeof(bcm_stat_flex_ing_cmprsd_pkt_res_attr_map_t);
                index++) {
                if (flex_ingress_modes[unit][mode].ing_attr.
                    cmprsd_attr_selectors.
                    pkt_res_attr_map[index]) {
                    FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                 index,
                                 flex_ingress_modes[unit][mode].
                                 ing_attr.cmprsd_attr_selectors.
                                 pkt_res_attr_map[index]));
                }
            }
       }
       if (flex_ingress_modes[unit][mode].ing_attr.cmprsd_attr_selectors.
           pkt_attr_bits.ip_pkt) {
           FLEXCTR_ERR(("ip_pkt:%d \n",
                        flex_ingress_modes[unit][mode].ing_attr.
                        cmprsd_attr_selectors.pkt_attr_bits.ip_pkt));
       }
       return;
    }
    if (direction == bcmStatFlexDirectionEgress) {
        flex_egress_modes[unit][mode].available=1;
        /*Will be changed WhileReadingTables */
        flex_egress_modes[unit][mode].reference_count=0;
        flex_egress_modes[unit][mode].egr_attr.packet_attr_type =
                                      bcmStatFlexPacketAttrTypeCompressed;
        for (index=0;index<8;index++) {
             if(selector_x_en_field_value[index] != 0) {
                if (selector_for_bit_x_field_value[index] == 0) {
                    flex_egress_modes[unit][mode].egr_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.ip_pkt = 1;
                    continue;
                }
                if (selector_for_bit_x_field_value[index] == 1) {
                    flex_egress_modes[unit][mode].egr_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.drop = 1;
                    continue;
                }
                if (selector_for_bit_x_field_value[index] == 2) {
                    flex_egress_modes[unit][mode].egr_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.dvp_type = 1;
                    continue;
                }
                if (selector_for_bit_x_field_value[index] == 3) {
                    flex_egress_modes[unit][mode].egr_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.svp_type = 1;
                    continue;
                }
                if ( selector_for_bit_x_field_value[index] == 4) {
                    flex_egress_modes[unit][mode].egr_attr.
                     cmprsd_attr_selectors.
                    pkt_attr_bits.pkt_resolution = 1;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 5) &&
                    (selector_for_bit_x_field_value[index] <= 12)) {
                    flex_egress_modes[unit][mode].egr_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.tos++;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 13) &&
                    (selector_for_bit_x_field_value[index] <= 18)) {
                    flex_egress_modes[unit][mode].egr_attr.
                     cmprsd_attr_selectors.pkt_attr_bits.egr_port++;
                    continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 19) &&
                    (selector_for_bit_x_field_value[index] <= 21)) {
                     flex_egress_modes[unit][mode].egr_attr.
                      cmprsd_attr_selectors.pkt_attr_bits.inner_dot1p++;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 22) &&
                    (selector_for_bit_x_field_value[index] <= 24)) {
                     flex_egress_modes[unit][mode].egr_attr.
                      cmprsd_attr_selectors.pkt_attr_bits.outer_dot1p++;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 25) &&
                    (selector_for_bit_x_field_value[index] <= 26)) {
                     flex_egress_modes[unit][mode].egr_attr.
                      cmprsd_attr_selectors.pkt_attr_bits.vlan_format++;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 27) &&
                    (selector_for_bit_x_field_value[index] <= 30)) {
                     flex_egress_modes[unit][mode].egr_attr.
                      cmprsd_attr_selectors.pkt_attr_bits.int_pri++;
                     continue;
                }
                if ((selector_for_bit_x_field_value[index] >= 31) &&
                    (selector_for_bit_x_field_value[index] <= 32)) {
                     flex_egress_modes[unit][mode].egr_attr.
                      cmprsd_attr_selectors.pkt_attr_bits.cng++;
                     continue;
                }
            }
        }
        if ((flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
              pkt_attr_bits.cng) ||
            (flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
              pkt_attr_bits.int_pri) ) {
             FLEXCTR_VVERB(("cng:%d \n",flex_egress_modes[unit][mode].egr_attr.
                            cmprsd_attr_selectors.pkt_attr_bits.cng));
             FLEXCTR_VVERB(("int_pri:%d \n",flex_egress_modes[unit][mode]. 
                            egr_attr.cmprsd_attr_selectors.pkt_attr_bits.
                            int_pri));
             dma_buff = soc_cm_salloc(
                            unit,
                            sizeof(bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t),
                            "EGR_FLEX_CTR_PRI_CNG_MAPm");
             if (dma_buff == NULL) {
                 FLEXCTR_ERR(("EGR_FLEX_CTR_PRI_CNG_MAPm:DMAAllocationFail\n"));
                 return;
             }
             if (soc_mem_read_range(
                     unit,
                     EGR_FLEX_CTR_PRI_CNG_MAPm,
                     MEM_BLOCK_ANY,
                     0,
                     sizeof(bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t)-1,
                     dma_buff) != SOC_E_NONE) {
                 FLEXCTR_ERR(("EGR_FLEX_CTR_PRI_CNG_MAPm:Read failuer \n"));
                 soc_cm_sfree(unit,dma_buff);
                 return;
             }
             sal_memcpy(&flex_egress_modes[unit][mode].egr_attr.
                        cmprsd_attr_selectors.pri_cnf_attr_map[0],
                        dma_buff,
                        sizeof(bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t));
             soc_cm_sfree(unit,dma_buff);
             for (index=0;
                  index< sizeof(bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t);
                  index++) {
                  if (flex_egress_modes[unit][mode].egr_attr.
                       cmprsd_attr_selectors.pri_cnf_attr_map[index]) {
                      FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                     index,
                                     flex_egress_modes[unit][mode].egr_attr.
                                     cmprsd_attr_selectors.
                                     pri_cnf_attr_map[index]));
                  }
             }
        }
        if ((flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
             pkt_attr_bits.vlan_format) ||
            (flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
             pkt_attr_bits.outer_dot1p) ||
            (flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
             pkt_attr_bits.inner_dot1p)) {
             FLEXCTR_VVERB(("vlan_format:%d \n",
                            flex_egress_modes[unit][mode].egr_attr.
                            cmprsd_attr_selectors.pkt_attr_bits.vlan_format));
             FLEXCTR_VVERB(("outer_dot1p:%d \n",
                            flex_egress_modes[unit][mode].egr_attr.
                            cmprsd_attr_selectors.pkt_attr_bits.outer_dot1p));
             FLEXCTR_VVERB(("inner_dot1p:%d \n",
                            flex_egress_modes[unit][mode].egr_attr.
                            cmprsd_attr_selectors.pkt_attr_bits.inner_dot1p));
             dma_buff = soc_cm_salloc(
                            unit,
                            sizeof(bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t),
                            "EGR_FLEX_CTR_PKT_PRI_MAPm");
             if (dma_buff == NULL) {
                 FLEXCTR_ERR(("EGR_FLEX_CTR_PKT_PRI_MAPm:DMAAllocationFail\n"));
                 return;
             }
             if (soc_mem_read_range(
                     unit,
                     EGR_FLEX_CTR_PKT_PRI_MAPm,
                     MEM_BLOCK_ANY,
                     0,
                     sizeof(bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t)-1,
                     dma_buff) != SOC_E_NONE) {
                 FLEXCTR_ERR(("EGR_FLEX_CTR_PKT_PRI_MAPm:Read failuer \n"));
                 soc_cm_sfree(unit,dma_buff);
                 return;
             }
             sal_memcpy(&flex_egress_modes[unit][mode].egr_attr.
                        cmprsd_attr_selectors.pkt_pri_attr_map[0],
                        dma_buff,
                        sizeof(bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t));
             soc_cm_sfree(unit,dma_buff);
             for (index=0;
                  index< sizeof(bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t);
                  index++) {
                  if (flex_egress_modes[unit][mode].egr_attr.
                       cmprsd_attr_selectors.pkt_pri_attr_map[index]) {
                      FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                     index,
                                     flex_egress_modes[unit][mode].egr_attr.
                                     cmprsd_attr_selectors.
                                     pkt_pri_attr_map[index]));
                  }
             }
        }
        if (flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
            pkt_attr_bits.egr_port) {
            FLEXCTR_VVERB(("egr_port:%d \n",flex_egress_modes[unit][mode].
                           egr_attr.cmprsd_attr_selectors.
                           pkt_attr_bits.egr_port));
            dma_buff = soc_cm_salloc(
                           unit,
                           sizeof(bcm_stat_flex_egr_cmprsd_port_attr_map_t),
                           "EGR_FLEX_CTR_PORT_MAPm");
            if (dma_buff == NULL) {
                FLEXCTR_ERR(("EGR_FLEX_CTR_PORT_MAPm:DMA AllocationFail\n"));
                return;
            }
            if (soc_mem_read_range(
                    unit,
                    EGR_FLEX_CTR_PORT_MAPm,
                    MEM_BLOCK_ANY,
                    0,
                    sizeof(bcm_stat_flex_egr_cmprsd_port_attr_map_t)-1,
                    dma_buff) != SOC_E_NONE) {
                FLEXCTR_ERR(("EGR_FLEX_CTR_PKT_PRI_MAPm:Read failuer \n"));
                soc_cm_sfree(unit,dma_buff);
                return;
            }
            sal_memcpy(&flex_egress_modes[unit][mode].egr_attr.
                       cmprsd_attr_selectors.port_attr_map[0],
                       dma_buff,
                       sizeof(bcm_stat_flex_egr_cmprsd_port_attr_map_t));
            soc_cm_sfree(unit,dma_buff);
            for (index=0;
                 index< sizeof(bcm_stat_flex_egr_cmprsd_port_attr_map_t);
                 index++) {
                 if (flex_egress_modes[unit][mode].egr_attr.
                     cmprsd_attr_selectors.port_attr_map[index]) {
                     FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                  index,
                                  flex_egress_modes[unit][mode].egr_attr.
                                  cmprsd_attr_selectors.port_attr_map[index]));
                 }
            }
        }
        if (flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
            pkt_attr_bits.tos) {
            FLEXCTR_ERR(("tos:%d \n",
                         flex_egress_modes[unit][mode].egr_attr.
                         cmprsd_attr_selectors.pkt_attr_bits.tos));
            dma_buff = soc_cm_salloc(
                           unit,
                           sizeof(bcm_stat_flex_egr_cmprsd_tos_attr_map_t),
                           "EGR_FLEX_CTR_TOS_MAPm");
            if (dma_buff == NULL) {
                FLEXCTR_ERR(("EGR_FLEX_CTR_TOS_MAPm:DMA Allocation failuer\n"));
                return;
            }
            if(soc_mem_read_range(
                   unit,
                   EGR_FLEX_CTR_TOS_MAPm,
                   MEM_BLOCK_ANY,
                   0,
                   sizeof(bcm_stat_flex_egr_cmprsd_tos_attr_map_t) -1,
                   dma_buff) != SOC_E_NONE) {
               FLEXCTR_ERR(("EGR_FLEX_CTR_TOS_MAPm:Read failuer \n"));
               soc_cm_sfree(unit,dma_buff);
               return;
            }
            sal_memcpy(&flex_egress_modes[unit][mode].egr_attr.
                       cmprsd_attr_selectors.tos_attr_map[0],
                       dma_buff,
                       sizeof(bcm_stat_flex_egr_cmprsd_tos_attr_map_t));
            soc_cm_sfree(unit,dma_buff);
            for (index=0;
                 index< sizeof(bcm_stat_flex_egr_cmprsd_tos_attr_map_t);
                 index++) {
                 if (flex_egress_modes[unit][mode].egr_attr.
                     cmprsd_attr_selectors.tos_attr_map[index]) {
                     FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                    index,flex_egress_modes[unit][mode].
                                    egr_attr.cmprsd_attr_selectors.
                                    tos_attr_map[index]));
                 }
            }
        }
        if ((flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
             pkt_attr_bits.pkt_resolution) ||
            (flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
             pkt_attr_bits.svp_type) ||
            (flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
             pkt_attr_bits.dvp_type) ||
            (flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
             pkt_attr_bits.drop) ) {
             FLEXCTR_VVERB(("pkt_resolution:%d \n",
                            flex_egress_modes[unit][mode].egr_attr.
                            cmprsd_attr_selectors.pkt_attr_bits.
                            pkt_resolution));
             FLEXCTR_VVERB(("svp_type:%d \n",
                            flex_egress_modes[unit][mode].egr_attr.
                            cmprsd_attr_selectors.pkt_attr_bits.svp_type));
             FLEXCTR_VVERB(("dvp_type:%d \n",
                            flex_egress_modes[unit][mode].egr_attr.
                            cmprsd_attr_selectors.pkt_attr_bits.dvp_type));
             FLEXCTR_VVERB(("drop:%d \n",
                            flex_egress_modes[unit][mode].egr_attr.
                            cmprsd_attr_selectors.pkt_attr_bits.drop));
             dma_buff = soc_cm_salloc(
                            unit,
                            sizeof(bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t),
                            "EGR_FLEX_CTR_PKT_RES_MAPm");
             if (dma_buff == NULL) {
                 FLEXCTR_ERR(("EGR_FLEX_CTR_PKT_RES_MAPm:DMAAllocationFail\n"));
                 return;
             }
             if (soc_mem_read_range(
                     unit,
                     EGR_FLEX_CTR_PKT_RES_MAPm,
                     MEM_BLOCK_ANY,
                     0,
                     sizeof(bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t)-1,
                     dma_buff) != BCM_E_NONE) {
                 FLEXCTR_ERR(("EGR_FLEX_CTR_TOS_MAPm:Read failuer \n"));
                 soc_cm_sfree(unit,dma_buff);
                 return;
             }
             sal_memcpy(&flex_egress_modes[unit][mode].egr_attr.
                        cmprsd_attr_selectors.pkt_res_attr_map[0],
                        dma_buff,
                        sizeof(bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t));
             soc_cm_sfree(unit,dma_buff);
             for (index=0;
                  index< sizeof(bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t);
                  index++) {
                  if (flex_egress_modes[unit][mode].egr_attr.
                       cmprsd_attr_selectors.pkt_res_attr_map[index]) {
                      FLEXCTR_VVERB(("Index:%dValue:%d\n",
                                     index,flex_egress_modes[unit][mode].
                                     egr_attr.cmprsd_attr_selectors.
                                     pkt_res_attr_map[index]));
                  }
             }
        }
        if (flex_egress_modes[unit][mode].egr_attr.cmprsd_attr_selectors.
            pkt_attr_bits.ip_pkt ) {
            FLEXCTR_VVERB(("ip_pkt:%d \n",
                           flex_egress_modes[unit][mode].
                           egr_attr.cmprsd_attr_selectors.
                           pkt_attr_bits.ip_pkt));
        }
        return;
    }
    FLEXCTR_ERR(("Ooops. Control Must not reach over here \n"));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_init_udf_mode
 * Description:
 *      Configures flex hw for udf mode
 * Parameters:
 *      unit                           - (IN) unit number
 *      mode                           - (IN) Flex Offset Mode
 *      direction                      - (IN) Flex Data flow direction(ing/egr)
 *      selector_count                 - (IN) Flex Key selector count
 *      selector_x_en_field_value      - (IN) Flex Key selector fields
 *      selector_for_bit_x_field_value - (IN) Flex Key selector field bits
 *
 * Return Value:
 *      None
 * Notes:
 *      Currently not being used!
 *
 */
static void 
_bcm_esw_stat_flex_init_udf_mode(
    int                       unit,
    int                       mode,
    bcm_stat_flex_direction_t direction,
    uint32                    selector_count,
    uint32                    selector_x_en_field_value[8],
    uint32                    selector_for_bit_x_field_value[8])
{
    uint16  udf0=0;
    uint16  udf1=0;
    uint32  index=0;
    for (udf0=0,udf1=0,index=0;index<8;index++) {
         if (selector_for_bit_x_field_value[index] != 0) {
             if (selector_for_bit_x_field_value[index] <=16) {
                 udf0 |= (1 << (selector_for_bit_x_field_value[index]-1));
             } else {
                 udf1 |= (1 << (selector_for_bit_x_field_value[index]- 16 - 1));
             }
         }
    }
    FLEXCTR_VVERB(("UDF0 : 0x%04x UDF1 0x%04x \n",udf0,udf1));
    if (direction == bcmStatFlexDirectionIngress) {
        flex_ingress_modes[unit][mode].available=1;
        /*Will be changed WhileReadingTables */
        flex_ingress_modes[unit][mode].reference_count=0;
        flex_ingress_modes[unit][mode].total_counters= ( 1 << selector_count);
        flex_ingress_modes[unit][mode].ing_attr.packet_attr_type =
             bcmStatFlexPacketAttrTypeUdf;
        flex_ingress_modes[unit][mode].ing_attr.udf_pkt_attr_selectors.
             udf_pkt_attr_bits.udf0 = udf0;
        flex_ingress_modes[unit][mode].ing_attr.udf_pkt_attr_selectors.
             udf_pkt_attr_bits.udf1 = udf1;
        return;
    }
    if (direction == bcmStatFlexDirectionEgress) {
        flex_egress_modes[unit][mode].available=1;
        /*Will be changed WhileReadingTables */
        flex_egress_modes[unit][mode].reference_count=0;
        flex_egress_modes[unit][mode].total_counters= ( 1 << selector_count);
        flex_egress_modes[unit][mode].egr_attr.packet_attr_type =
             bcmStatFlexPacketAttrTypeUdf;
        flex_egress_modes[unit][mode].egr_attr.udf_pkt_attr_selectors.
             udf_pkt_attr_bits.udf0 = udf0;
        flex_egress_modes[unit][mode].egr_attr.udf_pkt_attr_selectors.
             udf_pkt_attr_bits.udf1 = udf1;
        return;
    }
    FLEXCTR_ERR(("Ooops. Control Must not reach over here \n"));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_egress_l3_next_hop_table
 * Description:
 *      Checks egress_l3_next_hop table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void 
_bcm_esw_stat_flex_check_egress_l3_next_hop_table(int unit)
{
    uint32    index=0;
    /*egr_l3_next_hop_entry_t  egr_l3_next_hop_entry_v; */
    for(index=0;index<soc_mem_index_count(unit,EGR_L3_NEXT_HOPm);index++) {
        _bcm_esw_stat_flex_check_egress_table(unit,EGR_L3_NEXT_HOPm,index);
    }
    FLEXCTR_VVERB(("Checked EGRESS:EGR_L3_NEXT_HOP %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_egress_vlan_table
 * Description:
 *      Checks egress_vlan table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 *
 */
static void 
_bcm_esw_stat_flex_check_egress_vlan_table(int unit)
{
    uint32    index=0;
    /*egr_vlan_entry_t  egr_vlan_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,EGR_VLANm);index++) {
        _bcm_esw_stat_flex_check_egress_table(unit,EGR_VLANm,index);
    }
    FLEXCTR_VVERB(("Checked EGRESS:EGR_VLAN %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_vlan_table
 * Description:
 *      Checks ingress vlan table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 *
 */
static void 
_bcm_esw_stat_flex_check_ingress_vlan_table(int unit)
{
    uint32    index=0;
    /*vlan_tab_entry_t  vlan_tab_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,VLAN_TABm);index++) {
        _bcm_esw_stat_flex_check_ingress_table(unit,VLAN_TABm,index);
    }
    FLEXCTR_VVERB(("Checked INGRESS:VLAN_TAB %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_vrf_table
 * Description:
 *      Checks ingress vrf table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void 
_bcm_esw_stat_flex_check_ingress_vrf_table(int unit)
{
    uint32    index=0;
    /*vrf_entry_t  vrf_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,VRFm);index++) {
        _bcm_esw_stat_flex_check_ingress_table(unit,VRFm,index);
    }
    FLEXCTR_VVERB(("Checked INGRESS:VRF %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_egress_vfi_table
 * Description:
 *      Checks egress vfi table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void
_bcm_esw_stat_flex_check_egress_vfi_table(int unit)
{
    uint32    index=0;
    /*egr_vfi_entry_t  egr_vfi_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,EGR_VFIm);index++) {
        _bcm_esw_stat_flex_check_egress_table(unit,EGR_VFIm,index);
    }
    FLEXCTR_VVERB(("Checked EGRESS:EGR_VFI %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_vfi_table
 * Description:
 *      Checks ingress vfi table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void 
 _bcm_esw_stat_flex_check_ingress_vfi_table(int unit)
{
    uint32    index=0;
    /*vfi_entry_t  vfi_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,VFIm);index++) {
        _bcm_esw_stat_flex_check_ingress_table(unit,VFIm,index);
    }
    FLEXCTR_VVERB(("Checked INGRESS:VFI %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_l3_iif_table
 * Description:
 *      Checks ingress l3_iif table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void 
_bcm_esw_stat_flex_check_ingress_l3_iif_table(int unit)
{
    uint32    index=0;
    /*iif_entry_t  iif_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,L3_IIFm);index++) {
        _bcm_esw_stat_flex_check_ingress_table(unit,L3_IIFm,index);
    }
    FLEXCTR_VVERB(("Checked INGRESS:L3_IIF %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_source_vp_table
 * Description:
 *      Checks ingress source_vp table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void 
_bcm_esw_stat_flex_check_ingress_source_vp_table(int unit)
{
    uint32    index=0;
    /*source_vp_entry_t  source_vp_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,SOURCE_VPm);index++) {
        _bcm_esw_stat_flex_check_ingress_table(unit,SOURCE_VPm,index);
    }
    FLEXCTR_VVERB(("Checked INGRESS:SOURCE_VP %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_mpls_entry_table
 * Description:
 *      Checks ingress mpls_entry table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void
_bcm_esw_stat_flex_check_ingress_mpls_entry_table(int unit)
{
    uint32    index=0;
    /*mpls_entry_entry_t  mpls_entry_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,MPLS_ENTRYm);index++) {
        _bcm_esw_stat_flex_check_ingress_table(unit,MPLS_ENTRYm,index);
    }
    FLEXCTR_VVERB(("Checked INGRESS:MPLS_ENTRY %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_vfp_policy_table
 * Description:
 *      Checks ingress vfp_policy table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void
_bcm_esw_stat_flex_check_ingress_vfp_policy_table(int unit)
{
    uint32    index=0;
    /*vfp_policy_table_entry_t  vfp_policy_table_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,VFP_POLICY_TABLEm);index++) {
        _bcm_esw_stat_flex_check_ingress_table(unit,VFP_POLICY_TABLEm,index);
    }
    FLEXCTR_VVERB(("Checked INGRESS:VFP_POLICY_TABLE %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_egress_vlan_xlate_table
 * Description:
 *      Checks egress vlan_xlate table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void 
_bcm_esw_stat_flex_check_egress_vlan_xlate_table(int unit)
{
    uint32    index=0;
    /*egr_vlan_xlate_entry_t  egr_vlan_xlate_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,EGR_VLAN_XLATEm);index++) {
        _bcm_esw_stat_flex_check_egress_table(unit,EGR_VLAN_XLATEm,index);
    }
    FLEXCTR_VVERB(("Checked EGRESS:EGR_VLAN_XLATE %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_vlan_xlate_table
 * Description:
 *      Checks ingress vlan_xlate table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void
_bcm_esw_stat_flex_check_ingress_vlan_xlate_table(int unit)
{
    uint32    index=0;
    /*vlan_xlate_entry_t  vlan_xlate_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,VLAN_XLATEm);index++) {
        _bcm_esw_stat_flex_check_ingress_table(unit,VLAN_XLATEm,index);
    }
    FLEXCTR_VVERB(("Checked INGRESS:VLAN_XLATE_TABLE %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_egress_port_table
 * Description:
 *      Checks egress port table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void
_bcm_esw_stat_flex_check_egress_port_table(int unit)
{
    uint32    index=0;
    /*egr_port_entry_t  egr_port_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,EGR_PORTm);index++) {
        _bcm_esw_stat_flex_check_egress_table(unit,EGR_PORTm,index);
    }
    FLEXCTR_VVERB(("Checked EGRESS:EGR_PORT %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_check_ingress_port_table
 * Description:
 *      Checks ingress port table for flex configuration
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      None
 * Notes:
 *      Applicable in warm boot scenario only!
 */
static void
_bcm_esw_stat_flex_check_ingress_port_table(int unit)
{
    uint32    index=0;
    /*port_tab_entry_t  port_tab_entry_v;*/
    for(index=0;index<soc_mem_index_count(unit,PORT_TABm);index++) {
        _bcm_esw_stat_flex_check_ingress_table(unit,PORT_TABm,index);
    }
    FLEXCTR_VVERB(("Checked INGRESS:PORT_TABLE %d entries..\n",index-1));
}
/*
 * Function:
 *      _bcm_esw_stat_flex_set
 * Description:
 *      Set flex counter values
 * Parameters:
 *      unit                  - (IN) unit number
 *      index                 - (IN) Flex Accounting Table Index
 *      table                 - (IN) Flex Accounting Table 
 *      byte_flag             - (IN) Byte/Packet Flag
 *      flex_ctr_offset_info  - (IN) Information about flex counter values
 *      flex_entries          - (IN) Information about flex entries
 *      flex_values      - (IN) Information about flex entries values
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      
 */
static bcm_error_t 
_bcm_esw_stat_flex_set(
    int                             unit,
    uint32                          index,
    soc_mem_t                       table,
    uint32                          byte_flag,
    bcm_stat_flex_ctr_offset_info_t flex_ctr_offset_info,
    uint32                          *flex_entries,
    bcm_stat_flex_counter_value_t   *flex_values)
{
    soc_mem_t                 mem;
    uint32                    loop=0;
    uint32                    offset_mode=0;
    uint32                    pool_number=0;
    uint32                    base_idx=0;
    bcm_stat_flex_direction_t direction;
    uint32                    total_entries=0;
    uint32                    offset_index=0;
    uint32                    zero=0;
    uint32                    one=1;
    uint32                    entry_data_size=0;
    void                      *entry_data=NULL;
    uint32                    max_packet_mask=0;
    uint64                    max_byte_mask;
    uint32                    hw_val[2];     

    COMPILER_64_ZERO(max_byte_mask);


    switch(table) {
    case PORT_TABm:
    case VLAN_XLATEm:
    case VFP_POLICY_TABLEm:
    case MPLS_ENTRYm:
    case SOURCE_VPm:
    case L3_IIFm:
    case VRFm:
    case VFIm:
    case VLAN_TABm:
         direction= bcmStatFlexDirectionIngress;
         break;
    case EGR_VLANm:
    case EGR_VFIm:
    case EGR_L3_NEXT_HOPm:
    case EGR_VLAN_XLATEm:
    case EGR_PORTm:
         direction= bcmStatFlexDirectionEgress;
         break;
    default:
         FLEXCTR_ERR(("Invalid Flex Counter Memory %s\n",
                      SOC_MEM_UFNAME(unit, table)));
         return BCM_E_PARAM;
    }
    mem = _ctr_counter_table[direction][0];
    COMPILER_64_SET(max_byte_mask, zero, one);
    COMPILER_64_SHL(max_byte_mask,soc_mem_field_length(unit,mem,BYTE_COUNTERf));
    COMPILER_64_SUB_32(max_byte_mask,one);

    max_packet_mask = (1 << soc_mem_field_length(unit,mem,PACKET_COUNTERf));
    max_packet_mask -= 1;

    entry_data_size = WORDS2BYTES(BYTES2WORDS(SOC_MEM_INFO(unit,table).bytes));
    entry_data = sal_alloc(entry_data_size,"flex-counter-table");
    if (entry_data == NULL) {
        FLEXCTR_VVERB(("Failed to allocate memory for Table:%s ",
                       SOC_MEM_UFNAME(unit, table)));
        return BCM_E_INTERNAL;
    }
    sal_memset(entry_data,0,entry_data_size);

    if (soc_mem_read(unit,table,MEM_BLOCK_ANY,index,entry_data) == SOC_E_NONE) {
        if (soc_mem_field_valid(unit,table,VALIDf)) {
            if (soc_mem_field32_get(unit,table,entry_data,VALIDf)==0) {
                FLEXCTR_VVERB(("Table %s  with index %d is Not valid \n",
                               SOC_MEM_UFNAME(unit, table),index));
                sal_free(entry_data);
                return BCM_E_PARAM;
            }
        }
        _bcm_esw_get_flex_counter_fields_values(unit,table,entry_data,
                                                &offset_mode,&pool_number,
                                                &base_idx);
        if ((offset_mode == 0) && (base_idx == 0)) {
             FLEXCTR_ERR(("Table:%s:Index:%d:IsNotConfigured"
                            " for flex counter \n",
                            SOC_MEM_UFNAME(unit, table),index));
             sal_free(entry_data);
             /* Either Not configured or deallocated before */
             return BCM_E_NOT_FOUND;
        }
        if (direction == bcmStatFlexDirectionIngress) {
            total_entries= flex_ingress_modes[unit][offset_mode].total_counters;
        } else {
            total_entries = flex_egress_modes[unit][offset_mode].total_counters;
        }
        if (flex_ctr_offset_info.all_counters_flag) {
            offset_index = 0;
        } else {
            offset_index = flex_ctr_offset_info.offset_index;
            if (offset_index >= total_entries) {
                FLEXCTR_ERR(("Wrong OFFSET_INDEX.Must be <Total Counters %d \n",
                             total_entries));
                return BCM_E_PARAM;
            }
            total_entries = 1;
        }
        *flex_entries = total_entries;
        BCM_STAT_FLEX_COUNTER_LOCK(unit);
        mem = _ctr_counter_table[direction][pool_number];
        if (soc_mem_read_range(unit,
                               mem,
                               MEM_BLOCK_ANY,
                               base_idx+offset_index,
                               base_idx+offset_index+total_entries-1,
                               flex_temp_counter[direction]) != BCM_E_NONE) {
            BCM_STAT_FLEX_COUNTER_UNLOCK(unit);
            sal_free(entry_data);
            return BCM_E_INTERNAL;
        }
        for (loop = 0 ; loop < total_entries ; loop++) {
             if (byte_flag == 1) {
                 /* Mask to possible max values */
                 COMPILER_64_AND(flex_values[loop].byte_counter_value,
                                 max_byte_mask);
                 /* Update Soft Copy */
                 COMPILER_64_SET(flex_byte_counter[unit][direction]
                                 [pool_number][base_idx+offset_index+loop],
                                 COMPILER_64_HI(flex_values[loop].
                                                byte_counter_value),
                                 COMPILER_64_LO(flex_values[loop].
                                                byte_counter_value));
                 /* Change Read Hw Copy */
                 hw_val[0] = COMPILER_64_HI(flex_values[loop].
                                            byte_counter_value);
                 hw_val[1] = COMPILER_64_LO(flex_values[loop].
                                            byte_counter_value);
                 soc_mem_field_set(
                         unit,mem,(uint32 *)&flex_temp_counter[direction][loop],
                         BYTE_COUNTERf, hw_val);
                 FLEXCTR_VVERB(("Byte Count Value\t:TABLE:%sINDEX:%d COUTER-%d"
                                "(@Pool:%dDirection:%dActualOffset%d)"
                                " : %x:%x \n",SOC_MEM_UFNAME(unit, table),
                                index,loop,pool_number,direction,
                                base_idx+offset_index+loop,
                                COMPILER_64_HI(flex_values[loop].
                                               byte_counter_value),
                                COMPILER_64_LO(flex_values[loop].
                                               byte_counter_value)));
                } else {
                    flex_values[loop].pkt_counter_value &= max_packet_mask;
                    /* Update Soft Copy */
                    flex_packet_counter[unit][direction][pool_number]
                                       [base_idx+offset_index+loop] =
                                       flex_values[loop].pkt_counter_value;
                    /* Change Read Hw Copy */
                    soc_mem_field_set(
                            unit,
                            mem,
                            (uint32 *)&flex_temp_counter[direction][loop],
                            PACKET_COUNTERf,
                            &flex_values[loop].pkt_counter_value);
                    FLEXCTR_VVERB(("Packet Count Value\t:TABLE:%sINDEX:%d"
                                   "COUTER-%d (@Pool:%dDirection:%d"
                                   "ActualOffset%d) : %x \n",
                                   SOC_MEM_UFNAME(unit, table),index,loop, 
                                   pool_number,direction,
                                   base_idx+offset_index+loop,
                                   flex_values[loop].pkt_counter_value));
                }
        }
        /* Update Hw Copy */
        if (soc_mem_write_range(unit,mem,MEM_BLOCK_ALL,
                    base_idx+offset_index,
                    base_idx+offset_index+total_entries-1,
                    flex_temp_counter[direction]) != BCM_E_NONE) {
            BCM_STAT_FLEX_COUNTER_UNLOCK(unit);
            return BCM_E_INTERNAL;
        }
        BCM_STAT_FLEX_COUNTER_UNLOCK(unit);
        sal_free(entry_data);
        return BCM_E_NONE;
    }
    sal_free(entry_data);
    return BCM_E_FAIL;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_esw_stat_flex_sync
 * Description:
 *      Sync flex s/w copy to h/w copy
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_sync(int unit)
{
    FLEXCTR_VVERB(("_bcm_esw_stat_flex_sync \n"));
    if ((handle == 0) || 
        (flex_scache_allocated_size == 0) ||
        (flex_scache_ptr[unit] == NULL)) {
         FLEXCTR_ERR(("Scache memory was not allocate in init!! \n"));
         return BCM_E_CONFIG;
    }
    sal_memcpy(&flex_scache_ptr[unit][0],
               &local_scache_ptr[unit][0],
               BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE);
    FLEXCTR_VVERB(("OK \n"));
    return BCM_E_NONE;
}
#endif
/*
 * Function:
 *      _bcm_esw_stat_flex_cleanup
 * Description:
 *      Clean and free all allocated  flex resources
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_cleanup(int unit)
{
    bcm_stat_flex_direction_t    direction=bcmStatFlexDirectionIngress;
    uint32      pool_id=0;

    uint32      num_pools[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    FLEXCTR_VVERB(("_bcm_esw_stat_flex_cleanup \n"));
    num_pools[bcmStatFlexDirectionIngress]=SOC_INFO(unit).
                                           num_flex_ingress_pools;
    num_pools[bcmStatFlexDirectionEgress] =SOC_INFO(unit).
                                           num_flex_egress_pools;
    if (flex_stat_mutex[unit] != NULL) {
        BCM_STAT_FLEX_COUNTER_LOCK(unit);
        soc_counter_extra_unregister(unit, _bcm_esw_stat_flex_callback);
        sal_mutex_destroy(flex_stat_mutex[unit]);
        flex_stat_mutex[unit] = NULL;
    }
    for (direction=bcmStatFlexDirectionIngress;direction<2;direction++) {
         if (flex_temp_counter[direction] != NULL){
             soc_cm_sfree(unit,flex_temp_counter[direction]);
             flex_temp_counter[direction]=NULL;
         }
         for (pool_id=0;pool_id<num_pools[direction];pool_id++) {
              _bcm_esw_stat_flex_enable_pool(
                       unit, 
                       direction,
                       _pool_ctr_register[direction][pool_id],
                       0);
              if (flex_aidxres_list_handle[unit][direction][pool_id] != 0) {
                  shr_aidxres_list_destroy(flex_aidxres_list_handle[unit]
                                           [direction][pool_id]);
                  flex_aidxres_list_handle[unit][direction][pool_id] = 0;
              }
              flex_pool_stat[unit][direction][pool_id].used_by_tables = 0;
              flex_pool_stat[unit][direction][pool_id].used_entries = 0;
              if (flex_byte_counter[unit][direction][pool_id] != NULL) {
                  soc_cm_sfree(unit,
                               flex_byte_counter[unit][direction][pool_id]); 
                  flex_byte_counter[unit][direction][pool_id] = NULL;
              }
              if (flex_packet_counter[unit][direction][pool_id] != NULL) {
                  soc_cm_sfree(unit,
                               flex_packet_counter[unit][direction][pool_id]); 
                  flex_packet_counter[unit][direction][pool_id] = NULL;
              }
              if(flex_base_index_reference_count[unit][direction][pool_id] 
                 != NULL) {
                 sal_free(flex_base_index_reference_count[unit][direction]
                          [pool_id]);
                 flex_base_index_reference_count[unit][direction][pool_id]=NULL;
              }
         }
    }
    if (local_scache_ptr[unit] != NULL) {
        sal_free(local_scache_ptr[unit]);
        local_scache_ptr[unit] = NULL;
    }
    FLEXCTR_VVERB(("OK \n"));
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_init
 * Description:
 *      Initialize and allocate all required flex resources
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_init(int unit)
{
    bcm_stat_flex_direction_t    direction=bcmStatFlexDirectionIngress;
    uint32    mode=0;

    uint32    selector_count=0;
    uint32    index=0;
    uint32    selector_x_en_field_value[8]={0};
    uint32    selector_for_bit_x_field_value[8]={0};
    uint32    IsAnyModeConfigured=0;
    uint32    pool_id=0;

    uint32    num_pools[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32    size_pool[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32    alloc_size=0;
    uint64    selector_key_value;
#ifdef BCM_WARM_BOOT_SUPPORT
    int       stable_size=0;
    int       stable_used_size=0;
    bcm_error_t rv=BCM_E_NONE;
#endif

    FLEXCTR_VVERB(("_bcm_esw_stat_flex_init \n"));
    _bcm_esw_stat_flex_cleanup(unit);

    COMPILER_64_ZERO(selector_key_value);

    num_pools[bcmStatFlexDirectionIngress]= SOC_INFO(unit).
                                            num_flex_ingress_pools;
    num_pools[bcmStatFlexDirectionEgress] = SOC_INFO(unit).
                                            num_flex_egress_pools;
    size_pool[bcmStatFlexDirectionIngress]= SOC_INFO(unit).
                                            size_flex_ingress_pool;
    size_pool[bcmStatFlexDirectionEgress] = SOC_INFO(unit).
                                            size_flex_egress_pool;

    FLEXCTR_VVERB(("INFO: ingress_pools:%d num_flex_egress_pools:%di "
                   "size_flex_ingress_pool:%d size_flex_egress_pool:%d"
                   "MAX_DIRECTION %d MAX_MODE %d MAX_POOL %d\n",
                   num_pools[bcmStatFlexDirectionIngress],
                   num_pools[bcmStatFlexDirectionEgress],
                   size_pool[bcmStatFlexDirectionIngress],
                   size_pool[bcmStatFlexDirectionEgress],
                   BCM_STAT_FLEX_COUNTER_MAX_DIRECTION,
                   BCM_STAT_FLEX_COUNTER_MAX_MODE,
                   BCM_STAT_FLEX_COUNTER_MAX_POOL));
    if ((num_pools[bcmStatFlexDirectionIngress] == 0) ||
        (num_pools[bcmStatFlexDirectionEgress] == 0)) {
         FLEXCTR_ERR(("INFO:Number of CounterPools missing.PleaseDefine it\n"));
         return BCM_E_INTERNAL;
    }
    if ((num_pools[bcmStatFlexDirectionIngress] >
         BCM_STAT_FLEX_COUNTER_MAX_POOL) ||
        (num_pools[bcmStatFlexDirectionEgress] >
         BCM_STAT_FLEX_COUNTER_MAX_POOL)) {
         FLEXCTR_ERR(("INFO: Number of pools exceeding its max value %d \n",
                      BCM_STAT_FLEX_COUNTER_MAX_POOL));
         return BCM_E_INTERNAL;
    }
    if (flex_stat_mutex[unit] == NULL) {
        flex_stat_mutex[unit]= sal_mutex_create("Advanced flex counter mutex");
        if (flex_stat_mutex[unit] == NULL) {
            return BCM_E_MEMORY;
        }
    }

    /* Initialize Ingress Mode structures*/
    for (mode=0;mode<BCM_STAT_FLEX_COUNTER_MAX_MODE;mode++) {
         sal_memset(&flex_ingress_modes[unit][mode],0,
                    sizeof(bcm_stat_flex_ingress_mode_t));
    }
    /* Initialize Egress Mode structures*/
    for (mode=0;mode<BCM_STAT_FLEX_COUNTER_MAX_MODE;mode++) {
         sal_memset(&flex_egress_modes[unit][mode],0,
                    sizeof(bcm_stat_flex_egress_mode_t));
    }
    for (direction=bcmStatFlexDirectionIngress;direction<2;direction++) {
         alloc_size= sizeof(ing_flex_ctr_counter_table_0_entry_t)*size_pool
                           [direction];
         if (flex_temp_counter[direction] != NULL){
             FLEXCTR_WARN(("WARN:Freeing flex_temp_counter AllocatedMemory\n"));
             soc_cm_sfree(unit,flex_temp_counter[direction]);
         }
         flex_temp_counter[direction] = soc_cm_salloc(
                                            unit,
                                            alloc_size,
                                            "Advanced FlexTemp packet counter");
         if (flex_temp_counter[direction] == NULL) {
             FLEXCTR_WARN(("Advanced Flex current PacketCounterAllocationFail"
                           "for unit:%d, dir:%d pool:%d\n",
                           unit,direction,pool_id));
             return BCM_E_MEMORY;
         }
         sal_memset(flex_temp_counter[direction], 0, alloc_size);
         FLEXCTR_VVERB(("Temp counter size:%d \n",alloc_size));
         FLEXCTR_VVERB(("Byte counter size:%d \n",
                        sizeof(uint64)*size_pool[direction]));
         FLEXCTR_VVERB(("Packet counter size:%d\n",
                        sizeof(uint32)*size_pool[direction]));
         for (pool_id=0;pool_id<num_pools[direction];pool_id++) {
              FLEXCTR_VVERB(("."));
              /* Disable all counter pools */
              _bcm_esw_stat_flex_enable_pool(
                      unit, 
                      direction,
                      _pool_ctr_register[direction][pool_id],
                      0);
              /* Destroy if any exist */
              shr_aidxres_list_destroy(flex_aidxres_list_handle[unit]
                                       [direction][pool_id]);
              /* Create it */
              if (shr_aidxres_list_create(
                     &flex_aidxres_list_handle[unit][direction][pool_id],
                     0,size_pool[direction]-1,
                     0,size_pool[direction]-1,
                     8, /* Max 256 counters */
                     "flex-counter") != BCM_E_NONE) {
                  FLEXCTR_ERR(("Unrecoverable error. "
                               "Couldn'tCreate AllignedList:FlexCounter\n"));
                  return BCM_E_INTERNAL;
              }
              /* Reserver first two counters entries i.e. not to be used. */
              /* Flexible counter updates only if counter_base_index is != 0 */
              /* Refer: Arch-spec section 4.1 */
              shr_aidxres_list_reserve_block(
                          flex_aidxres_list_handle[unit][direction][pool_id],
                          0,
                          (1 << 1));
              flex_pool_stat[unit][direction][pool_id].used_by_tables = 0;
              flex_pool_stat[unit][direction][pool_id].used_entries = 0;

              if (flex_byte_counter[unit][direction][pool_id] != NULL) {
                  FLEXCTR_WARN(("WARN:Freeing AllocatedByteCountersMemory\n")); 
                  soc_cm_sfree(unit,
                               flex_byte_counter[unit][direction][pool_id]); 
              }
              flex_byte_counter[unit][direction][pool_id] = 
                   soc_cm_salloc(unit,sizeof(uint64)*size_pool[direction],
                                 "Advanced Flex byte counter");
              if (flex_byte_counter[unit][direction][pool_id] == NULL) {
                  FLEXCTR_ERR(("Advanced Flex ByteCounterAllocationFailed for"
                               "unit:%d,dir:%d pool:%d:\n", 
                               unit, direction, pool_id));
                  return BCM_E_MEMORY;
              }
              sal_memset(flex_byte_counter[unit][direction][pool_id], 0,
                         sizeof(uint64)*size_pool[direction]);

              if (flex_packet_counter[unit][direction][pool_id] != NULL) {
                  FLEXCTR_WARN(("WARN:FreeingAllocatedPacketCountersMemory\n")); 
                  soc_cm_sfree(unit,
                               flex_packet_counter[unit][direction][pool_id]); 
              }
              flex_packet_counter[unit][direction][pool_id] =
                   soc_cm_salloc(unit, sizeof(uint32)*size_pool[direction],
                                 "Advanced Flex packet counter");
              if (flex_packet_counter[unit][direction][pool_id] == NULL) {
                  FLEXCTR_ERR(("Advanced Flex ByteCounterAllocation failed for"
                               "unit:%d, dir:%d pool:%d\n",
                               unit,direction, pool_id));
                  return BCM_E_MEMORY;
              }
              sal_memset(flex_packet_counter[unit][direction][pool_id], 0,
                         sizeof(uint32)*size_pool[direction]);
              if (flex_base_index_reference_count[unit][direction]
                                                 [pool_id] != NULL) {
                  FLEXCTR_WARN(("WARN:Freeing Allocated"
                                "flex_base_index_reference_count memory\n")); 
                  sal_free(flex_base_index_reference_count[unit]
                                                          [direction][pool_id]);
              }
              flex_base_index_reference_count[unit][direction][pool_id] = 
                   sal_alloc(sizeof(uint16)*size_pool[direction],
                             "BaseIndexAllocation");
              if (flex_packet_counter[unit][direction][pool_id] == NULL){
                  FLEXCTR_ERR(("BaseIndex allocation failed for"
                               "unit:%d, dir:%d pool:%d\n",
                               unit,direction, pool_id));
                  return BCM_E_MEMORY;
              }
              sal_memset(flex_base_index_reference_count[unit][direction]
                         [pool_id], 0,sizeof(uint16)*size_pool[direction]);
         }
         FLEXCTR_VVERB(("\n"));
    }

    /* First retrieve mode related information */
    if (SOC_WARM_BOOT(unit)) {
        FLEXCTR_VVERB(("WARM booting..."));
        for (direction=bcmStatFlexDirectionIngress;direction<2;direction++) {
             for (mode=0;mode<BCM_STAT_FLEX_COUNTER_MAX_MODE;mode++) {
                  soc_reg64_get(
                      unit,
                      _pkt_selector_key_reg[direction][mode],
                      REG_PORT_ANY, 
                      0,
                      &selector_key_value);
                 for (selector_count=0,index=0;index<8;index++) {
                      selector_x_en_field_value[index]=
                                 soc_reg64_field32_get(
                                     unit,
                                     _pkt_selector_key_reg[direction]
                                                               [mode],
                                     selector_key_value,
                                     _pkt_selector_x_en_field_name[index]);
                      selector_count +=selector_x_en_field_value[index];
                      selector_for_bit_x_field_value[index]=0;
                      if (selector_x_en_field_value[index] != 0) {
                          selector_for_bit_x_field_value[index]=
                          soc_reg64_field32_get(
                              unit,
                              _pkt_selector_key_reg[direction][mode],
                              selector_key_value,
                              _pkt_selector_for_bit_x_field_name[index]);
                      }
                 }
                 if (selector_count > 0) {
                     FLEXCTR_VVERB(("Selector count :%d .. ",selector_count));
                     IsAnyModeConfigured=1;
                     if (soc_reg64_field32_get(
                             unit, 
                             _pkt_selector_key_reg[direction][mode],
                             selector_key_value,USE_UDF_KEYf) == 1) {
                         FLEXCTR_VVERB(("Direction:%d-Mode:%d UDF MODE is "
                                        "configured \n",direction,mode));
                         _bcm_esw_stat_flex_init_udf_mode(
                                  unit,
                                  mode,
                                  direction,
                                  selector_count,
                                  selector_x_en_field_value,
                                  selector_for_bit_x_field_value);
                     } else {
                         if (soc_reg64_field32_get(
                             unit, 
                             _pkt_selector_key_reg[direction][mode],
                             selector_key_value,USE_COMPRESSED_PKT_KEYf) == 1) {
                             FLEXCTR_VVERB(("Direction:%d-Mode:%d"
                                            "COMPRESSED MODE is "
                                            "configured \n",direction,mode));
                             _bcm_esw_stat_flex_init_cmprsd_mode(
                                      unit,
                                      mode,
                                      direction,
                                      selector_count,
                                      selector_x_en_field_value,
                                      selector_for_bit_x_field_value);
                         } else {
                             FLEXCTR_VVERB(("Direction:%d-Mode:%d"
                                            "UNCOMPRESSED MODE is "
                                            "configured \n",direction,mode));
                             _bcm_esw_stat_flex_init_uncmprsd_mode(
                                      unit,
                                      mode,
                                      direction,
                                      selector_count,
                                      selector_x_en_field_value,
                                      selector_for_bit_x_field_value);
                         }
                     }
                 } else {
                     FLEXCTR_VVERB(("Direction:%d-Mode:%d is unconfigured \n",
                                     direction,mode));
                 }
             }
        }
    } 
    if (IsAnyModeConfigured) {
        /* Get Ingress table info */

        /* 1) Ingress: Port Table */
        _bcm_esw_stat_flex_check_ingress_port_table(unit);
        /* 2) Ingress: VLAN_XLATE Table */
        _bcm_esw_stat_flex_check_ingress_vlan_xlate_table(unit);
        /* 3) Ingress: VFP_POLICY_TABLE Table */
        _bcm_esw_stat_flex_check_ingress_vfp_policy_table(unit);
        /* 4) Ingress: MPLS_ENTRY Table */
        _bcm_esw_stat_flex_check_ingress_mpls_entry_table(unit);
        /* 5) Ingress: SOURCE_VP Table */
        _bcm_esw_stat_flex_check_ingress_source_vp_table(unit);
        /* 6) Ingress: L3_IIF */
        _bcm_esw_stat_flex_check_ingress_l3_iif_table(unit);
        /* 7) Ingress: VFI */
        _bcm_esw_stat_flex_check_ingress_vfi_table(unit);
        /* 8) Ingress: VRF */
        _bcm_esw_stat_flex_check_ingress_vrf_table(unit);
        /* 9) Ingress: VLAN */
        _bcm_esw_stat_flex_check_ingress_vlan_table(unit);

        /* Get Egress table info */

        /* 1) EGR_L3_NEXT_HOP */
        _bcm_esw_stat_flex_check_egress_l3_next_hop_table(unit);
        /* 2) EGR_VFI */
        _bcm_esw_stat_flex_check_egress_vfi_table(unit);
        /* 3) EGR_PORT */
        _bcm_esw_stat_flex_check_egress_port_table(unit);
        /* 4) EGR_VLAN */
        _bcm_esw_stat_flex_check_egress_vlan_table(unit);
        /* 5) EGR_VLAN_XLATE */
        _bcm_esw_stat_flex_check_egress_vlan_xlate_table(unit);
        for (direction=bcmStatFlexDirectionIngress;direction<2;direction++) {
             for (mode=0;mode<BCM_STAT_FLEX_COUNTER_MAX_MODE;mode++) {
                  _bcm_esw_stat_flex_retrieve_group_mode(unit, direction,mode);
             }
        }
    }
    soc_counter_extra_register(unit, _bcm_esw_stat_flex_callback);
    if(local_scache_ptr[unit] != NULL) {
       FLEXCTR_WARN(("WARN: Freeing flex_scache_ptr existing memory"));
       sal_free(local_scache_ptr[unit]);
    }
    local_scache_size= sizeof(uint32)*BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE;
    local_scache_ptr[unit] = sal_alloc(local_scache_size,"Flex scache memory");
    if (local_scache_ptr[unit]  == NULL) {
        FLEXCTR_ERR(("ERR: Couldnot allocate flex_scache_ptr existing memory"));
        return BCM_E_MEMORY;
    }
    sal_memset(local_scache_ptr[unit],0,local_scache_size);
#ifdef BCM_WARM_BOOT_SUPPORT
    FLEXCTR_VVERB(("WARM Booting... \n"));
    /* rv = soc_scache_recover(unit); */
    flex_scache_allocated_size=0;
    SOC_SCACHE_HANDLE_SET(handle, unit, BCM_MODULE_STAT, 0);
    if (!SOC_WARM_BOOT(unit)) { 
        SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
        if (stable_size == 0) {
            FLEXCTR_VVERB(("STABLE size is zero.Probably NotConfigured yet\n"));
            return BCM_E_NONE;
        }
        SOC_IF_ERROR_RETURN(soc_stable_used_get(unit, &stable_used_size));
        if ((stable_size - stable_used_size) < local_scache_size) {
             FLEXCTR_VVERB(("Not enough scache memory left...\n"));
             return BCM_E_CONFIG;
        }
        rv = soc_scache_alloc(unit, handle,local_scache_size);
        if (!((rv== BCM_E_NONE) || (rv == BCM_E_EXISTS))) {
             FLEXCTR_VVERB(("Seems to be some internal problem:.\n"));
             return rv;
        }
        SOC_IF_ERROR_RETURN(soc_scache_ptr_get(
                            unit,
                            handle,
                            (uint8 **)&flex_scache_ptr[unit],
                            &flex_scache_allocated_size));
    } else {
        rv = soc_scache_ptr_get(
                            unit, 
                            handle,
                            (uint8 **)&flex_scache_ptr[unit],
                            &flex_scache_allocated_size);
        /* You may get  BCM_E_NOT_FOUND for level 1 warm boot */
        if (rv == BCM_E_NOT_FOUND) {
             FLEXCTR_VVERB(("Seems to be Level-1 Warm boot...continuing..\n"));
             soc_cm_print("Seems to be Level-1 Warm boot...continuing..\n");
             return BCM_E_NONE;
        }
        if (rv != BCM_E_NONE) {
             FLEXCTR_VVERB(("Seems to be some internal problem:.\n"));
             return rv;
        }
        sal_memcpy(&local_scache_ptr[unit][0],
                   &flex_scache_ptr[unit][0],
                   BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE);
    }
    if (flex_scache_ptr[unit] == NULL) {
           return BCM_E_MEMORY;
    }
    if (flex_scache_allocated_size != local_scache_size) {
        return BCM_E_INTERNAL;
    }
    if (SOC_WARM_BOOT(unit)) { 
        BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_install_stat_id(
                                     unit,flex_scache_ptr[unit]));
    }
    /* soc_scache_commit(unit); */
#else
    FLEXCTR_VVERB(("COLD Booting... \n"));
#endif
    FLEXCTR_VVERB(("OK \n"));

    /* Just an Info */
    if (SOC_CONTROL(unit)->tableDmaMutex == NULL) { /* enabled */
        FLEXCTR_VVERB(("WARNING:DMA will not be used for BulkMemoryReading\n"));
    } 
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_stat_flex_update_udf_selector_keys
 * Description:
 *      Update UDF selector keys
 * Parameters:
 *      unit                      - (IN) unit number
 *      direction                 - (IN) Flex Data Flow Direction
 *      pkt_attr_selector_key_reg - (IN) Flex Packet Attribute Seletor Key Reg
 *      udf_pkt_attr_selectors    - (IN) Flex Udf Packet Seletors
 *      total_udf_bits            - (OUT) Flex Udf Reserved bits
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      Currently not being used
 */
bcm_error_t _bcm_esw_stat_flex_update_udf_selector_keys(
            int                                    unit,
            bcm_stat_flex_direction_t              direction,
            soc_reg_t                              pkt_attr_selector_key_reg,
            bcm_stat_flex_udf_pkt_attr_selectors_t *udf_pkt_attr_selectors,
            uint32                                 *total_udf_bits)
{
    uint32 udf_valid_bits=0;
    uint8  udf_value_bit_position=0;
    uint8  key_bit_selector_position=0;
    uint16 udf_value=0;
    uint64 pkt_attr_selector_key_reg_value;
    uint32 index=0;

    COMPILER_64_ZERO(pkt_attr_selector_key_reg_value);

    (*total_udf_bits)=0;
    if (direction >= BCM_STAT_FLEX_COUNTER_MAX_DIRECTION) {
        return BCM_E_PARAM;
    }
    for(index=0; index < 8; index++) {
        if (_pkt_selector_key_reg[direction][index] == 
            pkt_attr_selector_key_reg ) {
            break;
        }
    }
    if (index == 8) {
         return BCM_E_PARAM;
    }

    /* Sanity Check: Get total number of udf bits */
    udf_value = udf_pkt_attr_selectors->udf_pkt_attr_bits.udf0;
    for (udf_value_bit_position=0;
         udf_value_bit_position<16;
         udf_value_bit_position++) {
         if (udf_value & 0x1) {
             (*total_udf_bits)++;
         }
         udf_value = udf_value >> 1;
    }
    if (*total_udf_bits > 8) {
        return BCM_E_PARAM;
    }
    udf_value = udf_pkt_attr_selectors->udf_pkt_attr_bits.udf1;
    for (udf_value_bit_position=0;
         udf_value_bit_position<16;
         udf_value_bit_position++) {
         if(udf_value & 0x1) {
            (*total_udf_bits)++;
         }
         udf_value = udf_value >> 1;
    }
    if ((*total_udf_bits) > 8) {
         return BCM_E_PARAM;
    }

    /*First Get complete value of ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_?r value */
    SOC_IF_ERROR_RETURN(soc_reg_get(    
                        unit,
                        pkt_attr_selector_key_reg,
                        REG_PORT_ANY,
                        0,
                        &pkt_attr_selector_key_reg_value));

    /* Next set field value for
       ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_?r:SELECTOR_KEY field*/
    soc_reg64_field32_set(unit, pkt_attr_selector_key_reg,
                          &pkt_attr_selector_key_reg_value, USE_UDF_KEYf, 1);
    if (udf_pkt_attr_selectors->udf_pkt_attr_bits.udf0 != 0) {
        udf_valid_bits |= 0x1;
    }
    if (udf_pkt_attr_selectors->udf_pkt_attr_bits.udf1 != 0) {
        udf_valid_bits |= 0x2;
    }
    soc_reg64_field32_set(unit, pkt_attr_selector_key_reg,
                          &pkt_attr_selector_key_reg_value, 
                          USER_SPECIFIED_UDF_VALIDf, udf_valid_bits);

    /* Now update selector keys */

    udf_value = udf_pkt_attr_selectors->udf_pkt_attr_bits.udf0;
    for (udf_value_bit_position=0;
         udf_value_bit_position<16;
         udf_value_bit_position++) {
         if (udf_value & 0x1) {
           BCM_IF_ERROR_RETURN(
               _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                        unit,
                        pkt_attr_selector_key_reg,
                        &pkt_attr_selector_key_reg_value,
                        udf_value_bit_position+1,
                        1,     /* Total Bits */
                        0x1,     /* MASK */
                        &key_bit_selector_position));
         }
         udf_value = udf_value >> 1;
    }
    udf_value = udf_pkt_attr_selectors->udf_pkt_attr_bits.udf1;
    for (;udf_value_bit_position<32; udf_value_bit_position++) {
         if(udf_value & 0x1) {
            BCM_IF_ERROR_RETURN(
               _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                        unit,
                        pkt_attr_selector_key_reg,
                        &pkt_attr_selector_key_reg_value,
                        udf_value_bit_position+1,
                        1,    /* Total Bits */
                        0x1,    /* MASK */
                        &key_bit_selector_position));
         }
         udf_value = udf_value >> 1;
    }
    /* Finally set value for ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_?r */
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, pkt_attr_selector_key_reg,
                                    REG_PORT_ANY, 0, 
                                    pkt_attr_selector_key_reg_value));
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_update_offset_table
 * Description:
 *      Update flex Offset table
 * Parameters:
 *      unit                      - (IN) unit number
 *      direction                 - (IN) Flex Data Flow Direction
 *      flex_ctr_offset_table_mem - (IN) Flex Offset Table
 *      mode                      - (IN) Flex Mode
 *      total_counters            - (IN) Flex Total Counters
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_update_offset_table(
            int                                unit,
            bcm_stat_flex_direction_t          direction,
            soc_mem_t                          flex_ctr_offset_table_mem,
            bcm_stat_flex_mode_t               mode,
            uint32                             total_counters,
            bcm_stat_flex_offset_table_entry_t offset_table_map[256])
{
    uint32                              index=0;
    uint32                              zero=0;
    uint32                              count_enable=1;
    uint32                              offset_value=0;
    ing_flex_ctr_offset_table_0_entry_t *offset_table_entry_v=NULL;
    uint32                              num_pools[
                                        BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    num_pools[bcmStatFlexDirectionIngress] = SOC_INFO(unit).
                                             num_flex_ingress_pools;
    num_pools[bcmStatFlexDirectionEgress]  = SOC_INFO(unit).
                                             num_flex_egress_pools;

    if (direction >= BCM_STAT_FLEX_COUNTER_MAX_DIRECTION) {
        return BCM_E_PARAM;
    }
    for(index=0; index < num_pools[direction]; index++) {
        if (_ctr_offset_table[direction][index] == flex_ctr_offset_table_mem ) {
            break;
        }
    }
    if (index == num_pools[direction]) {
         return BCM_E_PARAM;
    }
    FLEXCTR_VVERB(("...Updating offset_table:%s:%d \n",
                   SOC_MEM_UFNAME(unit,flex_ctr_offset_table_mem),
                   sizeof(ing_flex_ctr_offset_table_0_entry_t)));
    offset_table_entry_v = soc_cm_salloc(unit,
                               sizeof(ing_flex_ctr_offset_table_0_entry_t)*256,
                               "offset_table_entry");
    if (offset_table_entry_v == NULL) {
        return BCM_E_MEMORY;
    }
    if (soc_mem_read_range(    unit,
                flex_ctr_offset_table_mem,
                MEM_BLOCK_ANY,
                (mode <<8),
                ((mode <<8)+256)-1,
                offset_table_entry_v ) != BCM_E_NONE){
        soc_cm_sfree(unit,offset_table_entry_v);
        return BCM_E_INTERNAL;
    }
    /*for(index=0;index<255;index++) */
    for (index=0;index<total_counters;index++) {
         /* Set OFFSETf */
         if (offset_table_map == NULL) {
             offset_value = index;
             count_enable = 1;
         } else {
             offset_value = offset_table_map[index].offset;
             count_enable = offset_table_map[index].count_enable;
         }
         soc_mem_field_set(
                 unit,
                 flex_ctr_offset_table_mem,
                 (uint32 *)&offset_table_entry_v[index],
                 OFFSETf,
                 &offset_value);
         /* Set COUNT_ENABLEf */
         soc_mem_field_set(
                 unit,
                 flex_ctr_offset_table_mem,
                 (uint32 *)&offset_table_entry_v[index],
                 COUNT_ENABLEf,
                 &count_enable);
    }
    /* Clear remaining entries */
    for (;index<256;index++) {
         /* Set OFFSETf=zero */
         soc_mem_field_set(
                 unit,
                 flex_ctr_offset_table_mem,
                 (uint32 *)&offset_table_entry_v[index],
                 OFFSETf,
                 &zero);
         /* Set COUNT_ENABLEf=zero */
         soc_mem_field_set(
                 unit,
                 flex_ctr_offset_table_mem,
                 (uint32 *)&offset_table_entry_v[index],
                 COUNT_ENABLEf,
                 &zero);
    }
    if (soc_mem_write_range(    unit,
                flex_ctr_offset_table_mem,
                MEM_BLOCK_ALL,
                (mode <<8),
                ((mode <<8)+256)-1,
                offset_table_entry_v ) != BCM_E_NONE){
        soc_cm_sfree(unit,offset_table_entry_v);
        return BCM_E_INTERNAL;
    }
    soc_cm_sfree(unit,offset_table_entry_v);
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_update_selector_keys_enable_fields
 * Description:
 *      Enable flex selector keys fields
 * Parameters:
 * unit                              - (IN) unit number
 * pkt_attr_selector_key_reg         - (IN) Flex Packet Attribute Selector
 * pkt_attr_selector_key_reg_value   - (IN) Flex Packet Attribute SelectorVal
 * ctr_pkt_attr_bit_position         - (IN) Flex Packet CounterAttr Bit Position
 * ctr_pkt_attr_total_bits           - (IN) Flex Packet Counter Attr Total Bits
 * pkt_attr_field_mask_v             - (IN) Flex Packet Attr Field Mask
 * ctr_current_bit_selector_position - (IN/OUT) Current BitSelector Position
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_update_selector_keys_enable_fields(
           int       unit,
           soc_reg_t pkt_attr_selector_key_reg,
           uint64    *pkt_attr_selector_key_reg_value,
           uint32    ctr_pkt_attr_bit_position,
           uint32    ctr_pkt_attr_total_bits,
           uint8     pkt_attr_field_mask_v,
           uint8     *ctr_current_bit_selector_position)
{
    uint32 index=0;
    uint8  field_mask=0;
    uint8  total_field_bits=0;
    uint8  field_index[8]={0};

    if ((*ctr_current_bit_selector_position) + ctr_pkt_attr_total_bits > 8) {
         FLEXCTR_ERR(("Total bits exceeding 8 \n"));
         return BCM_E_INTERNAL;
    }
    field_mask = pkt_attr_field_mask_v;
    total_field_bits = 0;

    for (index=0;index < 8 ; index++) {
        if (field_mask & 0x1) {
            field_index[index] = index;
            total_field_bits++;
        }
        field_mask = field_mask >> 1;
    }
    if (total_field_bits != ctr_pkt_attr_total_bits ) {
        FLEXCTR_ERR(("Total bits exceeding not matching with mask bits \n"));
        return BCM_E_INTERNAL;
    }

    for (index=0;index<ctr_pkt_attr_total_bits;index++) {
         soc_reg64_field32_set(
                   unit,
                   pkt_attr_selector_key_reg,
                   pkt_attr_selector_key_reg_value,
                   _pkt_selector_x_en_field_name[
                    *ctr_current_bit_selector_position],
                   1);
         soc_reg64_field32_set(
                   unit,
                   pkt_attr_selector_key_reg,
                   pkt_attr_selector_key_reg_value,
                   _pkt_selector_for_bit_x_field_name[
                    *ctr_current_bit_selector_position],
                   ctr_pkt_attr_bit_position+field_index[index]);
         /*ctr_pkt_attr_bit_position+index); */
         (*ctr_current_bit_selector_position) +=1;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_egress_reserve_mode
 * Description:
 *      Reserve flex egress mode
 * Parameters:
 *      unit                  - (IN) unit number
 *      mode                  - (IN) Flex Offset Mode
 *      total_counters        - (IN) Flex Total Counters for given mode
 *      egr_attr              - (IN) Flex Egress Attributes
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_egress_reserve_mode(
            int                      unit,
            bcm_stat_flex_mode_t     mode,
            uint32                   total_counters,
            bcm_stat_flex_egr_attr_t *egr_attr)
{
    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        FLEXCTR_ERR(("Invalid flex counter mode value %d \n",mode));
        return BCM_E_PARAM;
    }
    flex_egress_modes[unit][mode].available=1;
    flex_egress_modes[unit][mode].total_counters=total_counters;
    flex_egress_modes[unit][mode].egr_attr=*egr_attr;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_ingress_reserve_mode
 * Description:
 *      Reserve flex ingress mode
 * Parameters:
 *      unit                  - (IN) unit number
 *      mode                  - (IN) Flex Offset Mode
 *      total_counters        - (IN) Flex Total Counters for given mode
 *      ing_attr              - (IN) Flex Ingress Attributes
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_ingress_reserve_mode(
            int                      unit,
            bcm_stat_flex_mode_t     mode,
            uint32                   total_counters,
            bcm_stat_flex_ing_attr_t *ing_attr)
{
    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        FLEXCTR_ERR(("Invalid flex counter mode value %d \n",mode));
        return BCM_E_PARAM;
    }
    flex_ingress_modes[unit][mode].available=1;
    flex_ingress_modes[unit][mode].total_counters=total_counters;
    flex_ingress_modes[unit][mode].ing_attr=*ing_attr;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_unreserve_mode
 * Description:
 *      UnReserve flex (ingress/egress) mode
 * Parameters:
 *      unit                  - (IN) unit number
 *      direction             - (IN) Flex Data Flow Direction
 *      mode                  - (IN) Flex Offset Mode
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_unreserve_mode(
            int                       unit,
            bcm_stat_flex_direction_t direction,
            bcm_stat_flex_mode_t      mode)
{
    uint32                                    index=0;
    bcm_stat_flex_packet_attr_type_t          packet_attr_type;
    bcm_stat_flex_ing_cmprsd_attr_selectors_t ing_cmprsd_attr_selectors= {{0}};
    bcm_stat_flex_ing_pkt_attr_bits_t         ing_pkt_attr_bits={0};
    bcm_stat_flex_egr_cmprsd_attr_selectors_t egr_cmprsd_attr_selectors= {{0}};
    bcm_stat_flex_egr_pkt_attr_bits_t         egr_pkt_attr_bits={0};
    uint8                                     zero=0;
    uint64                                    pkt_attr_selector_key_reg_value;

    uint32 num_pools[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    COMPILER_64_ZERO(pkt_attr_selector_key_reg_value);

    num_pools[bcmStatFlexDirectionIngress] = SOC_INFO(unit).
                                             num_flex_ingress_pools;
    num_pools[bcmStatFlexDirectionEgress] = SOC_INFO(unit).
                                             num_flex_egress_pools;

    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        FLEXCTR_ERR(("Invalid flex counter mode value %d \n",mode));
        return BCM_E_PARAM;
    }
    switch(direction) {
    case bcmStatFlexDirectionIngress:
         if (flex_ingress_modes[unit][mode].available == 0) {
             FLEXCTR_ERR(("flex counter mode %d not configured yet\n", mode));
             return BCM_E_NOT_FOUND;
         }
         if (flex_ingress_modes[unit][mode].reference_count != 0) {
             FLEXCTR_ERR(("FlexCounterMode:%d:IsBeingUsed.ReferenceCount:%d:\n",
                          mode,flex_ingress_modes[unit][mode].reference_count));
             return BCM_E_INTERNAL;
         }
         /* Step2:  Reset selector keys */
         SOC_IF_ERROR_RETURN(soc_reg_set(
                             unit,
                             _pkt_selector_key_reg[bcmStatFlexDirectionIngress]
                             [mode],
                             REG_PORT_ANY,
                             0,
                             pkt_attr_selector_key_reg_value));
         SOC_IF_ERROR_RETURN(soc_reg_set(
                             unit,
                             _pkt_selector_key_reg[bcmStatFlexDirectionIngress]
                             [mode+BCM_STAT_FLEX_COUNTER_MAX_MODE],
                             REG_PORT_ANY,
                             0,
                             pkt_attr_selector_key_reg_value));
         /* Step3: Reset Offset table filling */
         for (index=0;index<num_pools[bcmStatFlexDirectionIngress]; index++) {
              BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_offset_table(
                                  unit,
                                  bcmStatFlexDirectionIngress,
                                  _ctr_offset_table
                                    [bcmStatFlexDirectionIngress][index],
                                  mode,
                                  0,
                                  NULL));
         }
         /* Step4: Cleanup based on mode */
         packet_attr_type= flex_ingress_modes[unit][mode].
                            ing_attr.packet_attr_type;
         switch(packet_attr_type) {
         case bcmStatFlexPacketAttrTypeUncompressed:
              FLEXCTR_VVERB(("\n Unreserving Ingress uncmprsd mode \n"));
              break;
         case bcmStatFlexPacketAttrTypeUdf:
              FLEXCTR_VVERB(("\n Unreserving Ingress udf mode \n"));
              break;
         case bcmStatFlexPacketAttrTypeCompressed:
              ing_cmprsd_attr_selectors= flex_ingress_modes[unit][mode].
                                         ing_attr.cmprsd_attr_selectors;
              ing_pkt_attr_bits=ing_cmprsd_attr_selectors.pkt_attr_bits;
              zero=0;

              FLEXCTR_VVERB(("\n Unreserving Ingress cmprsd mode \n"));

              /* Step3: Cleanup map array */
              if ((ing_pkt_attr_bits.cng != 0) ||
                  (ing_pkt_attr_bits.ifp_cng)  ||
                  (ing_pkt_attr_bits.int_pri)) {
                  for (index=0;
                       index< sizeof(
                              bcm_stat_flex_ing_cmprsd_pri_cnf_attr_map_t);
                       index++) {
                       soc_mem_write(unit,ING_FLEX_CTR_PRI_CNG_MAPm,
                                     MEM_BLOCK_ALL, index, &zero);
                  }
              }
              if ((ing_pkt_attr_bits.vlan_format != 0) ||
                  (ing_pkt_attr_bits.outer_dot1p) ||
                  (ing_pkt_attr_bits.inner_dot1p)) {
                   for (index=0;
                        index< sizeof(
                               bcm_stat_flex_ing_cmprsd_pkt_pri_attr_map_t);
                        index++) {
                        soc_mem_write(unit, ING_FLEX_CTR_PKT_PRI_MAPm,
                                      MEM_BLOCK_ALL, index, &zero);
                   }
              }
              if (ing_pkt_attr_bits.ing_port != 0) {
                  for (index=0;
                       index< sizeof(bcm_stat_flex_ing_cmprsd_port_attr_map_t);
                       index++) {
                   soc_mem_write(unit, ING_FLEX_CTR_PORT_MAPm,
                                MEM_BLOCK_ALL, index, &zero);
                  }
              }
              if (ing_pkt_attr_bits.tos != 0) {
                  for (index=0;
                       index< sizeof(bcm_stat_flex_ing_cmprsd_tos_attr_map_t);
                       index++) {
                       soc_mem_write(unit, ING_FLEX_CTR_TOS_MAPm,
                                     MEM_BLOCK_ALL, index, &zero);
                  }
              }
              if ((ing_pkt_attr_bits.pkt_resolution != 0) ||
                  (ing_pkt_attr_bits.svp_type) ||
                  (ing_pkt_attr_bits.drop)) {
                   for (index=0;
                        index< sizeof(
                               bcm_stat_flex_ing_cmprsd_pkt_res_attr_map_t);
                        index++) {
                        soc_mem_write(unit, ING_FLEX_CTR_PKT_RES_MAPm,
                                      MEM_BLOCK_ALL, index, &zero);
                   }
              }
              break;
         }
         flex_ingress_modes[unit][mode].available=0;
         flex_ingress_modes[unit][mode].total_counters=0;
         FLEXCTR_VVERB(("\n Done \n"));
         break;
    case bcmStatFlexDirectionEgress:
         if (flex_egress_modes[unit][mode].available==0) {
             return BCM_E_NOT_FOUND;
         }
         if (flex_egress_modes[unit][mode].reference_count != 0) {
             return BCM_E_INTERNAL;
         }
         /* Step2:  Reset selector keys */
         SOC_IF_ERROR_RETURN(soc_reg_set(unit,
                             _pkt_selector_key_reg[bcmStatFlexDirectionEgress]
                                                  [mode],
                             REG_PORT_ANY,
                             0,
                             pkt_attr_selector_key_reg_value));
         SOC_IF_ERROR_RETURN(soc_reg_set(unit,
                             _pkt_selector_key_reg[bcmStatFlexDirectionEgress]
                             [mode+BCM_STAT_FLEX_COUNTER_MAX_MODE],
                             REG_PORT_ANY,
                             0,
                             pkt_attr_selector_key_reg_value));
         /* Step3: Reset Offset table filling */
         for (index=0; index < num_pools[bcmStatFlexDirectionEgress]; index++) {
              BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_offset_table(
                                   unit,bcmStatFlexDirectionEgress,
                                   _ctr_offset_table[bcmStatFlexDirectionEgress]
                                                    [index],
                                   mode,
                                   0,
                                   NULL));
         }
         /* Step4: Cleanup based on mode */
         packet_attr_type = flex_egress_modes[unit][mode].egr_attr.
                            packet_attr_type;
         switch(packet_attr_type) {
         case bcmStatFlexPacketAttrTypeUncompressed:
              FLEXCTR_VVERB(("\nUnreserving Egress uncmprsd mode \n"));
              break;
         case bcmStatFlexPacketAttrTypeUdf:
              FLEXCTR_VVERB(("\n Unreserving Egress UDF mode \n"));
              break;
         case bcmStatFlexPacketAttrTypeCompressed:
              egr_cmprsd_attr_selectors= flex_egress_modes[unit][mode].
                                         egr_attr.cmprsd_attr_selectors;
              egr_pkt_attr_bits= egr_cmprsd_attr_selectors.pkt_attr_bits;
              FLEXCTR_VVERB(("\n Unreserving Egress cmprsd mode \n"));

              /* Step3: Cleanup map array */
              if ((egr_pkt_attr_bits.cng != 0) ||
                  (egr_pkt_attr_bits.int_pri)) {
                   for(index=0;
                       index< sizeof(
                              bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t);
                       index++) {
                       soc_mem_write(unit, EGR_FLEX_CTR_PRI_CNG_MAPm,
                                     MEM_BLOCK_ALL, index, &zero);
                   }
              }
              if ((egr_pkt_attr_bits.vlan_format != 0) ||
                  (egr_pkt_attr_bits.outer_dot1p) ||
                  (egr_pkt_attr_bits.inner_dot1p)) {
                   for(index=0;
                       index< sizeof(
                              bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t);
                       index++) {
                       soc_mem_write(unit, EGR_FLEX_CTR_PKT_PRI_MAPm,
                                     MEM_BLOCK_ALL, index, &zero);
                   }
              }
              if (egr_pkt_attr_bits.egr_port != 0) {
                  for(index=0;
                      index< sizeof(bcm_stat_flex_egr_cmprsd_port_attr_map_t);
                      index++) {
                      soc_mem_write(unit, EGR_FLEX_CTR_PORT_MAPm,
                                    MEM_BLOCK_ALL, index, &zero);
                  }
              }
              if (egr_pkt_attr_bits.tos != 0){
                  for(index=0;
                      index< sizeof(bcm_stat_flex_egr_cmprsd_tos_attr_map_t);
                      index++) {
                            soc_mem_write(
                                unit,
                                EGR_FLEX_CTR_TOS_MAPm,
                                MEM_BLOCK_ALL,
                                index,
                                &zero);
                           }
                        }
              if ((egr_pkt_attr_bits.pkt_resolution != 0) ||
                  (egr_pkt_attr_bits.svp_type) ||
                  (egr_pkt_attr_bits.dvp_type) ||
                  (egr_pkt_attr_bits.drop)) {
                   for(index=0;
                       index< sizeof(
                              bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t);
                       index++) {
                        soc_mem_write(unit, EGR_FLEX_CTR_PKT_RES_MAPm,
                                      MEM_BLOCK_ALL, index, &zero);
                   }
              }
              break;
         }
         flex_egress_modes[unit][mode].available=0;
         flex_egress_modes[unit][mode].total_counters=0;
         FLEXCTR_VVERB(("\n Done \n"));
         break;
    default:
         return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_get_ingress_mode_info
 * Description:
 *      Get Ingress Mode related information
 * Parameters:
 *      unit                  - (IN) unit number
 *      mode                  - (IN) Flex Offset Mode
 *      flex_ingress_mode     - (OUT) Flex Ingress Mode Info
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_get_ingress_mode_info(
            int                          unit,
            bcm_stat_flex_mode_t         mode,
            bcm_stat_flex_ingress_mode_t *flex_ingress_mode)
{
    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        FLEXCTR_ERR(("Invalid flex counter mode value %d \n",mode));
        return BCM_E_PARAM;
    }
    if (flex_ingress_modes[unit][mode].available==0) {
        /* FLEXCTR_ERR(("flex counter mode %d not configured yet\n",mode));*/
        return BCM_E_NOT_FOUND;
    }
    *flex_ingress_mode= flex_ingress_modes[unit][mode];
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_get_egress_mode_info
 * Description:
 *      Get Egress Mode related information
 * Parameters:
 *      unit                  - (IN) unit number
 *      mode                  - (IN) Flex Offset Mode
 *      flex_egress_mode      - (OUT) Flex Egress Mode Info
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_get_egress_mode_info(
            int                         unit,
            bcm_stat_flex_mode_t        mode,
            bcm_stat_flex_egress_mode_t *flex_egress_mode)
{
    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        FLEXCTR_ERR(("Invalid flex counter mode value %d \n",mode));
        return BCM_E_PARAM;
    }
    if (flex_egress_modes[unit][mode].available==0) {
        /* FLEXCTR_ERR(("flex counter mode %d not configured yet\n",mode));*/
        return BCM_E_NOT_FOUND;
    }
    *flex_egress_mode = flex_egress_modes[unit][mode];
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_get_available_mode
 * Description:
 *      Get Free available Offset Mode
 * Parameters:
 *      unit                  - (IN) unit number
 *      direction             - (IN) Flex Data Flow Direction
 *      mode                  - (OUT) Flex Offset Mode
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_get_available_mode(
            int                       unit,
            bcm_stat_flex_direction_t direction,
            bcm_stat_flex_mode_t      *mode)
{
    uint32 mode_index=1;
    switch(direction) {
    case bcmStatFlexDirectionIngress:
         for (mode_index=0;
              mode_index < BCM_STAT_FLEX_COUNTER_MAX_MODE ;
              mode_index++) {
              if (flex_ingress_modes[unit][mode_index].available==0) {
                  *mode=mode_index;
                  return BCM_E_NONE;
              }
         }
         break;
    case bcmStatFlexDirectionEgress:
         for (mode_index=0;
              mode_index < BCM_STAT_FLEX_COUNTER_MAX_MODE ;
              mode_index++) {
              if (flex_egress_modes[unit][mode_index].available==0) {
                  *mode=mode_index;
                  return BCM_E_NONE;
              }
         }
         break;
    default: 
         return BCM_E_PARAM;
    }
    return BCM_E_FULL;
}
/*
 * Function:
 *      _bcm_esw_stat_counter_set
 * Description:
 *      Set Flex Counter Values
 * Parameters:
 *      unit                  - (IN) unit number
 *      index                 - (IN) Flex Accounting Table Index
 *      table                 - (IN) Flex Accounting Table 
 *      byte_flag             - (IN) Byte/Packet Flag
 *      counter               - (IN) Counter Index
 *      values                - (IN) Counter Value
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_counter_set(
            int              unit,
            uint32           index,
            soc_mem_t        table,
            uint32           byte_flag,
            uint32           counter_index, 
            bcm_stat_value_t *value)
{
    soc_mem_t                 mem;
    uint32                    loop=0;
    uint32                    offset_mode=0;
    uint32                    pool_number=0;
    uint32                    base_idx=0;
    bcm_stat_flex_direction_t direction;
    uint32                    total_entries=0;
    uint32                    offset_index=0;
    uint32                    zero=0;
    uint32                    one=1;
    uint32                    entry_data_size=0;
    void                      *entry_data=NULL;
    uint32                    max_packet_mask=0;
    uint64                    max_byte_mask;
    uint32                    hw_val[2];     

    COMPILER_64_ZERO(max_byte_mask);


    switch(table) {
    case PORT_TABm:
    case VLAN_XLATEm:
    case VFP_POLICY_TABLEm:
    case MPLS_ENTRYm:
    case SOURCE_VPm:
    case L3_IIFm:
    case VRFm:
    case VFIm:
    case VLAN_TABm:
         direction= bcmStatFlexDirectionIngress;
         break;
    case EGR_VLANm:
    case EGR_VFIm:
    case EGR_L3_NEXT_HOPm:
    case EGR_VLAN_XLATEm:
    case EGR_PORTm:
         direction= bcmStatFlexDirectionEgress;
         break;
    default:
         FLEXCTR_ERR(("Invalid Flex Counter Memory %s\n",
                      SOC_MEM_UFNAME(unit, table)));
         return BCM_E_PARAM;
    }
    mem = _ctr_counter_table[direction][0];
    COMPILER_64_SET(max_byte_mask, zero, one);
    COMPILER_64_SHL(max_byte_mask,soc_mem_field_length(unit,mem,BYTE_COUNTERf));
    COMPILER_64_SUB_32(max_byte_mask,one);

    max_packet_mask = (1 << soc_mem_field_length(unit,mem,PACKET_COUNTERf));
    max_packet_mask -= 1;

    entry_data_size = WORDS2BYTES(BYTES2WORDS(SOC_MEM_INFO(unit,table).bytes));
    entry_data = sal_alloc(entry_data_size,"flex-counter-table");
    if (entry_data == NULL) {
        FLEXCTR_ERR(("Failed to allocate memory for Table:%s ",
                     SOC_MEM_UFNAME(unit, table)));
        return BCM_E_INTERNAL;
    }
    sal_memset(entry_data,0,entry_data_size);

    if (soc_mem_read(unit,table,MEM_BLOCK_ANY,index,entry_data)== SOC_E_NONE) {
        if (soc_mem_field_valid(unit,table,VALIDf)) {
            if (soc_mem_field32_get(unit,table,entry_data,VALIDf)==0) {
                FLEXCTR_ERR(("Table %s  with index %d is Not valid \n",
                             SOC_MEM_UFNAME(unit, table),index));
                sal_free(entry_data);
                return BCM_E_PARAM;
            }
        }
        _bcm_esw_get_flex_counter_fields_values(
                 unit,table,entry_data, &offset_mode,&pool_number,&base_idx);
        if ((offset_mode == 0) && (base_idx == 0)) {
             FLEXCTR_ERR(("Table:%s:Index:%d:IsNotConfiguredForFlexCounter \n",
                          SOC_MEM_UFNAME(unit, table),index));
             sal_free(entry_data);
             /* Either Not configured or deallocated before */
             return BCM_E_NOT_FOUND;
        }
        if (direction == bcmStatFlexDirectionIngress) {
            total_entries= flex_ingress_modes[unit][offset_mode].total_counters;
        } else {
            total_entries = flex_egress_modes[unit][offset_mode].total_counters;
        }
        offset_index = counter_index;
        if (offset_index >= total_entries) {
            FLEXCTR_ERR(("Wrong OFFSET_INDEX. Must be < Total Counters %d \n",
                         total_entries));
            return BCM_E_PARAM;
        }
        BCM_STAT_FLEX_COUNTER_LOCK(unit);
        mem = _ctr_counter_table[direction][pool_number];
        if (soc_mem_read_range(
                    unit,
                    mem,
                    MEM_BLOCK_ANY,
                    base_idx+offset_index,
                    base_idx+offset_index+total_entries-1,
                    flex_temp_counter[direction]) != BCM_E_NONE){
            BCM_STAT_FLEX_COUNTER_UNLOCK(unit);
            sal_free(entry_data);
            return BCM_E_INTERNAL;
        }
        if (byte_flag == 1) {
            /* Mask to possible max values */
            COMPILER_64_AND(value->bytes, max_byte_mask);
            /* Update Soft Copy */
            COMPILER_64_SET(flex_byte_counter[unit][direction][pool_number]
                                             [base_idx+offset_index+loop],
                            COMPILER_64_HI(value->bytes),
                            COMPILER_64_LO(value->bytes));
            /* Change Read Hw Copy */
            hw_val[0] = COMPILER_64_LO(value->bytes);
            hw_val[1] = COMPILER_64_HI(value->bytes);
            soc_mem_field_set(
                    unit,
                    mem,
                    (uint32 *)&flex_temp_counter[direction][loop],
                    BYTE_COUNTERf,
                    hw_val);
            FLEXCTR_VVERB(("Byte Count Value\t:TABLE:%sINDEX:%d COUTER-%d"
                           "(@Pool:%dDirection:%dActualOffset%d) : %x:%x \n",
                           SOC_MEM_UFNAME(unit, table),
                           index,
                           loop,
                           pool_number,
                           direction,
                           base_idx+offset_index+loop,
                           COMPILER_64_HI(value->bytes),
                           COMPILER_64_LO(value->bytes)));
        } else {
            value->packets &= max_packet_mask;
            /* Update Soft Copy */
            flex_packet_counter[unit][direction][pool_number]
                               [base_idx+offset_index+loop] = value->packets;
            /* Change Read Hw Copy */
            soc_mem_field_set(
                    unit,
                    mem,
                    (uint32 *)&flex_temp_counter[direction][loop],
                    PACKET_COUNTERf,
                    &(value->packets));
            FLEXCTR_VVERB(("Packet Count Value\t:TABLE:%sINDEX:%d COUTER-%d"
                           "(@Pool:%dDirection:%dActualOffset%d) : %x \n",
                           SOC_MEM_UFNAME(unit, table),
                           index,
                           loop, 
                           pool_number,
                           direction,
                           base_idx+offset_index+loop,
                           value->packets));
        }
        /* Update Hw Copy */
        if (soc_mem_write_range(unit,mem,MEM_BLOCK_ALL,
                    base_idx+offset_index,
                    base_idx+offset_index+total_entries-1,
                    flex_temp_counter[direction]) != BCM_E_NONE){
            BCM_STAT_FLEX_COUNTER_UNLOCK(unit);
            return BCM_E_INTERNAL;
        }
        BCM_STAT_FLEX_COUNTER_UNLOCK(unit);
        sal_free(entry_data);
        return BCM_E_NONE;
    }
    sal_free(entry_data);
    return BCM_E_FAIL;
}
/*
 * Function:
 *      _bcm_esw_stat_counter_get
 * Description:
 *      Get Flex Counter Values
 * Parameters:
 *      unit                  - (IN) unit number
 *      index                 - (IN) Flex Accounting Table Index
 *      table                 - (IN) Flex Accounting Table 
 *      byte_flag             - (IN) Byte/Packet Flag
 *      counter               - (IN) Counter Index
 *      values                - (OUT) Counter Value
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_counter_get(
            int              unit,
            uint32           index,
            soc_mem_t        table,
            uint32           byte_flag,
            uint32           counter_index, 
            bcm_stat_value_t *value)
{
    uint32                    loop=0;
    uint32                    offset_mode=0;
    uint32                    pool_number=0;
    uint32                    base_idx=0;
    bcm_stat_flex_direction_t direction;
    uint32                    total_entries=0;
    uint32                    offset_index=0;
    uint32                    entry_data_size=0;
    void                      *entry_data=NULL;

    switch(table) {
    case PORT_TABm:
    case VLAN_XLATEm:
    case VFP_POLICY_TABLEm:
    case MPLS_ENTRYm:
    case SOURCE_VPm:
    case L3_IIFm:
    case VRFm:
    case VFIm:
    case VLAN_TABm:
         direction= bcmStatFlexDirectionIngress;
         break;
    case EGR_VLANm:
    case EGR_VFIm:
    case EGR_L3_NEXT_HOPm:
    case EGR_VLAN_XLATEm:
    case EGR_PORTm:
         direction= bcmStatFlexDirectionEgress;
         break;
    default:
         FLEXCTR_ERR(("Invalid Flex Counter Memory %s\n",
                      SOC_MEM_UFNAME(unit, table)));
         return BCM_E_PARAM;
    }
    entry_data_size = WORDS2BYTES(BYTES2WORDS(SOC_MEM_INFO(unit,table).bytes));
    entry_data = sal_alloc(entry_data_size,"flex-counter-table");
    if (entry_data == NULL) {
        FLEXCTR_ERR(("Failed to allocate memory for Table:%s ",
                     SOC_MEM_UFNAME(unit, table)));
        return BCM_E_INTERNAL;
    }
    sal_memset(entry_data,0,entry_data_size);
    if (soc_mem_read(unit,table,MEM_BLOCK_ANY,index,entry_data) == SOC_E_NONE) {
        if (soc_mem_field_valid(unit,table,VALIDf)) {
            if (soc_mem_field32_get(unit,table,entry_data,VALIDf)==0) {
                FLEXCTR_ERR(("Table %s  with index %d is Not valid \n",
                             SOC_MEM_UFNAME(unit, table),index));
                sal_free(entry_data);
                return BCM_E_PARAM;
            }
        }
        _bcm_esw_get_flex_counter_fields_values(
                 unit,
                 table,
                 entry_data,
                 &offset_mode,
                 &pool_number,
                 &base_idx);
        if ((offset_mode == 0) && (base_idx == 0)) {
             FLEXCTR_ERR(("Table:%s:Index:%d:is NotConfiguredForFlexCounter \n",
                          SOC_MEM_UFNAME(unit, table),index));
             sal_free(entry_data);
             /* Either Not configured or deallocated before */
             return BCM_E_NOT_FOUND;
         }
         if (direction == bcmStatFlexDirectionIngress) {
             total_entries = flex_ingress_modes[unit][offset_mode].
                             total_counters;
         } else {
             total_entries = flex_egress_modes[unit][offset_mode].
                             total_counters;
         }
         offset_index = counter_index;
         if (offset_index >= total_entries) {
             FLEXCTR_ERR(("Wrong OFFSET_INDEX.Must be < Total Counters %d \n",
                          total_entries));
             return BCM_E_PARAM;
         }
         BCM_STAT_FLEX_COUNTER_LOCK(unit);
         if (byte_flag == 1) {
             COMPILER_64_SET(value->bytes,
                      COMPILER_64_HI(flex_byte_counter[unit]
                                     [direction][pool_number]
                                     [base_idx+offset_index+loop]),
                      COMPILER_64_LO(flex_byte_counter[unit]
                                     [direction][pool_number]
                                     [base_idx+offset_index+loop]));
             FLEXCTR_VVERB(("Byte Count Value\t:TABLE:%sINDEX:%d COUTER-%d"
                            "(@Pool:%dDirection:%dActualOffset%d) : %x:%x \n",
                            SOC_MEM_UFNAME(unit, table),
                            index,
                            loop,
                            pool_number,
                            direction,
                            base_idx+offset_index+loop,
                            COMPILER_64_HI(value->bytes),
                            COMPILER_64_LO(value->bytes)));
         } else {
             value->packets= flex_packet_counter[unit][direction][pool_number]
                                                [base_idx+offset_index+loop];
             FLEXCTR_VVERB(("Packet Count Value\t:TABLE:%sINDEX:%d COUTER-%d"
                            "(@Pool:%dDirection:%dActualOffset%d) : %x \n",
                            SOC_MEM_UFNAME(unit, table),
                            index,
                            loop, 
                            pool_number,
                            direction,
                            base_idx+offset_index+loop,
                            value->packets));
         }
         BCM_STAT_FLEX_COUNTER_UNLOCK(unit);
         sal_free(entry_data);
         return BCM_E_NONE;
    }
    sal_free(entry_data);
    return BCM_E_FAIL;
}

/*
 * Function:
 *      _bcm_esw_stat_flex_callback
 * Description:
 *      Flex Counter threads callback function. It is called periodically
 *      to sync s/w copy with h/w copy.
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
void _bcm_esw_stat_flex_callback(int unit)
{
    soc_mem_t mem;
    uint32    num_pools[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32    size_pool[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32    pool_id=0;
    uint32    direction=bcmStatFlexDirectionIngress;
    uint32    packet_count=0;
    uint32    index;
    uint32    zero=0;
    uint32    one=1;
    uint32    byte_count_read[2];
    uint32    flex_ctr_update_control_reg_value=0;
    uint64    byte_count;
    uint64    prev_masked_byte_count;
    uint64    max_byte_size;
    uint64    max_byte_mask;

    COMPILER_64_ZERO(byte_count);
    COMPILER_64_ZERO(prev_masked_byte_count);
    COMPILER_64_ZERO(max_byte_size);
    COMPILER_64_ZERO(max_byte_mask);

    num_pools[bcmStatFlexDirectionIngress]= SOC_INFO(unit).
                                            num_flex_ingress_pools;
    num_pools[bcmStatFlexDirectionEgress] = SOC_INFO(unit).
                                            num_flex_egress_pools;
    size_pool[bcmStatFlexDirectionIngress]= SOC_INFO(unit).
                                            size_flex_ingress_pool;
    size_pool[bcmStatFlexDirectionEgress] = SOC_INFO(unit).
                                            size_flex_egress_pool;

    mem = _ctr_counter_table[direction][pool_id];

    COMPILER_64_SET(max_byte_size, zero, one);
    COMPILER_64_SHL(max_byte_size,soc_mem_field_length(unit,mem,BYTE_COUNTERf));

    COMPILER_64_SET(max_byte_mask,
                    COMPILER_64_HI(max_byte_size),
                    COMPILER_64_LO(max_byte_size));
    COMPILER_64_SUB_32(max_byte_mask,one);

    BCM_STAT_FLEX_COUNTER_LOCK(unit);
    for (direction=bcmStatFlexDirectionIngress;direction<2;direction++) {
         for (pool_id=0;pool_id<num_pools[direction];pool_id++) {
              if (soc_reg32_get(
                            unit,
                            _pool_ctr_register[direction][pool_id],
                            REG_PORT_ANY,
                            0,
                            &flex_ctr_update_control_reg_value) != SOC_E_NONE) {
                 continue;
              }
              if (soc_reg_field_get(
                                unit, 
                                _pool_ctr_register[direction][pool_id],
                                flex_ctr_update_control_reg_value,
                                COUNTER_POOL_ENABLEf) == 0) {
                 continue;
              }
              /*FLEXCTR_VVERB(("%d-%d..",direction,pool_id); */
              mem = _ctr_counter_table[direction][pool_id];
              if (soc_mem_read_range(
                          unit,
                          mem,
                          MEM_BLOCK_ANY,
                          soc_mem_index_min(unit,mem),
                          soc_mem_index_max(unit,mem),
                          flex_temp_counter[direction]) != BCM_E_NONE) {
                  BCM_STAT_FLEX_COUNTER_UNLOCK(unit);
                  return;
              }
              for (index=0;index<soc_mem_index_max(unit,mem);index++) {
                   soc_mem_field_get(
                           unit,
                           mem,
                           (uint32 *)&flex_temp_counter[direction][index],
                           PACKET_COUNTERf,
                           &packet_count);
                   if (flex_packet_counter[unit][direction][pool_id][index]
                       != packet_count) {
                       FLEXCTR_VERB(("Direction:%dPool:%d==>"
                                     "Old Packet Count Value\t:"
                                     "Index:%d %x\tNew Packet Count Value %x\n",
                                     direction,
                                     pool_id,
                                     index,
                                     flex_packet_counter[unit]
                                      [direction][pool_id][index],
                                     packet_count));
                       flex_packet_counter[unit][direction][pool_id][index] =
                                   packet_count;
                   }
                   soc_mem_field_get(
                           unit,
                           mem,
                           (uint32 *)&flex_temp_counter[direction][index],
                           BYTE_COUNTERf, 
                           byte_count_read);
                   COMPILER_64_SET(byte_count, byte_count_read[1],
                                   byte_count_read[0]);
                   COMPILER_64_SET(prev_masked_byte_count,
                                   COMPILER_64_HI(flex_byte_counter[unit]
                                                  [direction][pool_id][index]),
                                   COMPILER_64_LO(flex_byte_counter[unit]
                                                  [direction][pool_id][index]));
                   COMPILER_64_AND(prev_masked_byte_count,max_byte_mask);
                   if (COMPILER_64_GT(prev_masked_byte_count, byte_count)) {
                       FLEXCTR_VERB(("Roll over  happend \n"));
                       FLEXCTR_VERB(("...Read Byte Count    : %x:%x\n",
                                     COMPILER_64_HI(byte_count),
                                     COMPILER_64_LO(byte_count)));
                       COMPILER_64_ADD_64(byte_count,max_byte_size);
                       COMPILER_64_SUB_64(byte_count,prev_masked_byte_count);
                       FLEXCTR_VERB(("...Diffed Byte Count    : %x:%x\n",
                                     COMPILER_64_HI(byte_count),
                                     COMPILER_64_LO(byte_count)));
                   } else {
                       COMPILER_64_SUB_64(byte_count,prev_masked_byte_count);
                   }
                   /* Add difference (if it is) */
                   if (!COMPILER_64_IS_ZERO(byte_count)) {
                       FLEXCTR_VERB(("Direction:%dPool:%d==>"
                                     "Old Byte Count Value\t"
                                     ":Index:%d %x:%x\t",
                                     direction,pool_id,index,
                                     COMPILER_64_HI(flex_byte_counter[unit]
                                                 [direction][pool_id][index]),
                                     COMPILER_64_LO(flex_byte_counter[unit]
                                                 [direction][pool_id][index])));
                       COMPILER_64_ADD_64(flex_byte_counter[unit]
                                          [direction][pool_id][index],
                                          byte_count);
                       FLEXCTR_VERB(("New Byte Count Value : %x:%x\n",
                               COMPILER_64_HI(flex_byte_counter[unit]
                                             [direction][pool_id][index]),
                               COMPILER_64_LO(flex_byte_counter[unit][direction]
                                             [pool_id][index])));
                   }
              }
         }
    }
    BCM_STAT_FLEX_COUNTER_UNLOCK(unit);
    return;
}

/*
 * Function:
 *      _bcm_esw_stat_flex_get_table_info
 * Description:
 *      Get Table related Information based on accounting object value
 * Parameters:
 *      object                - (IN) Flex Accounting Object
 *      table                 - (OUT) Flex Accounting Table
 *      direction             - (OUT) Flex Data flow direction
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_stat_flex_get_table_info(
            bcm_stat_object_t         object,
            soc_mem_t                 *table,
            bcm_stat_flex_direction_t *direction)             
{
    switch(object) {
    case bcmStatObjectIngPort:
         *table=PORT_TABm;
         *direction= bcmStatFlexDirectionIngress;
         return BCM_E_NONE;
    case bcmStatObjectIngVlan:
         *table=VLAN_TABm;
         *direction= bcmStatFlexDirectionIngress;
         return BCM_E_NONE;
    case bcmStatObjectIngVlanXlate:
         *table=VLAN_XLATEm;
         *direction= bcmStatFlexDirectionIngress;
         return BCM_E_NONE;
    case bcmStatObjectIngVfi:
         *table=VFIm;
         *direction= bcmStatFlexDirectionIngress;
         return BCM_E_NONE;
    case bcmStatObjectIngL3Intf:
         *table=L3_IIFm;
         *direction= bcmStatFlexDirectionIngress;
         return BCM_E_NONE;
    case bcmStatObjectIngVrf:
         *table=VRFm;
         *direction= bcmStatFlexDirectionIngress;
         return BCM_E_NONE;
    case bcmStatObjectIngPolicy:
         *table=VFP_POLICY_TABLEm;
         *direction= bcmStatFlexDirectionIngress;
         return BCM_E_NONE;
    case bcmStatObjectIngMplsVcLabel:
         *table=SOURCE_VPm;
         *direction= bcmStatFlexDirectionIngress;
         return BCM_E_NONE;
    case bcmStatObjectIngMplsSwitchLabel:
         *table=MPLS_ENTRYm;
         *direction= bcmStatFlexDirectionIngress;
         return BCM_E_NONE;
    case bcmStatObjectEgrPort:
         *table=EGR_PORTm;
         *direction= bcmStatFlexDirectionEgress;
         return BCM_E_NONE;
    case bcmStatObjectEgrVlan:
         *table=EGR_VLANm;
         *direction= bcmStatFlexDirectionEgress;
         return BCM_E_NONE;
    case bcmStatObjectEgrVlanXlate:
         *table=EGR_VLAN_XLATEm;
         *direction= bcmStatFlexDirectionEgress;
         return BCM_E_NONE;
    case bcmStatObjectEgrVfi:
         *table=EGR_VFIm;
         *direction= bcmStatFlexDirectionEgress;
         return BCM_E_NONE;
    case bcmStatObjectEgrL3Intf:
         *table=EGR_L3_NEXT_HOPm;
         *direction= bcmStatFlexDirectionEgress;
         return BCM_E_NONE;
    default:
         FLEXCTR_ERR(("Invalid Object is passed %d\n",object));
         return BCM_E_PARAM;
    }
}
/*
 * Function:
 *      _bcm_esw_stat_flex_detach_egress_table_counters
 * Description:
 *      Detach i.e. Disable Egresss accounting table's statistics
 * Parameters:
 *      unit                  - (IN) unit number
 *      egress_table          - (IN) Flex Accounting Table
 *      index                 - (IN) Flex Accounting Table's Index
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_detach_egress_table_counters(
            int       unit,
            soc_mem_t egress_table,
            uint32    index)
{
    uint32                          offset_mode=0;
    uint32                          pool_number=0;
    uint32                          base_idx=0;
    uint32                          egress_entry_data_size=0;
    uint32                          flex_entries=0;
    bcm_stat_flex_ctr_offset_info_t flex_ctr_offset_info;
    uint32                          alloc_size=0;
    void                            *egress_entry_data=NULL;
    bcm_stat_flex_counter_value_t   *flex_counter_value=NULL;
    bcm_stat_object_t               object=bcmStatObjectEgrPort;
    uint32                          stat_counter_id=0;

    if (!((egress_table == EGR_VLANm) ||
          (egress_table == EGR_VFIm)  ||
          (egress_table == EGR_L3_NEXT_HOPm) ||
          (egress_table == EGR_VLAN_XLATEm)  ||
          (egress_table == EGR_PORTm))) {
           FLEXCTR_ERR(("Invalid Flex Counter Egress Memory %s\n",
                        SOC_MEM_UFNAME(unit, egress_table)));
           return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_egress_object(
                        egress_table,&object));
    FLEXCTR_VVERB(("Deallocating EGRESS counter for Table %s with index %d \n",
                   SOC_MEM_UFNAME(unit, egress_table),index));
    egress_entry_data_size = WORDS2BYTES(BYTES2WORDS(
                             SOC_MEM_INFO(unit, egress_table).bytes));
    FLEXCTR_VVERB(("Deallocating EgressCounter Table:%s:with" 
                   "index:%d:ENTRY_BYTES:%d\n",
                   SOC_MEM_UFNAME(unit, egress_table),
                   index,egress_entry_data_size));
    egress_entry_data = sal_alloc(egress_entry_data_size,"egress_table");
    if (egress_entry_data == NULL) {
        FLEXCTR_ERR(("Failed to allocate memory for Table:%s ",
                     SOC_MEM_UFNAME(unit, egress_table)));
         return BCM_E_INTERNAL;
    }
    sal_memset(egress_entry_data,0,egress_entry_data_size);

    if (soc_mem_read(unit, egress_table, MEM_BLOCK_ANY,
                     index,egress_entry_data) == SOC_E_NONE) {
        if (soc_mem_field_valid(unit, egress_table, VALIDf)) {
            if (soc_mem_field32_get(unit, egress_table, egress_entry_data,
                                    VALIDf)==0) {
                FLEXCTR_ERR(("Table %s  with index %d is Not valid \n",
                             SOC_MEM_UFNAME(unit, egress_table), index));
                sal_free(egress_entry_data);
                return BCM_E_PARAM;
            }
        }
        _bcm_esw_get_flex_counter_fields_values(
                 unit,egress_table,egress_entry_data,
                 &offset_mode,&pool_number,&base_idx);
        if ((offset_mode == 0) && (base_idx == 0)) {
             FLEXCTR_ERR(("Table:%s:Index:%d:is NotConfiguredForFlexCtrYet\n",
                          SOC_MEM_UFNAME(unit, egress_table), index));
             sal_free(egress_entry_data);
             return BCM_E_NOT_FOUND;/*Either NotConfigured/DeallocatedBefore */
        }

        /* Clear Counter Values */
        FLEXCTR_VVERB(("Clearing Counter Tables %s contents:Offset:%d Len:%d\n",
                       SOC_MEM_UFNAME(unit, egress_table),
                       base_idx,
                       flex_egress_modes[unit][offset_mode].total_counters));
        flex_ctr_offset_info.all_counters_flag =  1;
        alloc_size = sizeof(bcm_stat_flex_counter_value_t) *
                     flex_egress_modes[unit][offset_mode].total_counters;
        flex_counter_value = sal_alloc(alloc_size,"counter-table-values");
        if (flex_counter_value == NULL) {
            FLEXCTR_ERR(("Failed: AllocateCounterMemoryTable:%s:Index:%d:\n",
                         SOC_MEM_UFNAME(unit, egress_table),index));
            sal_free(egress_entry_data);
            return BCM_E_INTERNAL;
        }
        sal_memset(flex_counter_value,0,alloc_size);
        _bcm_esw_stat_flex_set(unit, index, egress_table, 1,
                               flex_ctr_offset_info,
                               &flex_entries,flex_counter_value);
        _bcm_esw_stat_flex_set(unit,index,egress_table,0,
                               flex_ctr_offset_info, &flex_entries,
                               flex_counter_value);
        sal_free(flex_counter_value);
        /* Reset flex fields */
        _bcm_esw_set_flex_counter_fields_values(unit,
                                                egress_table,
                                                egress_entry_data,0,0,0);
        if (soc_mem_write(unit, egress_table, MEM_BLOCK_ALL,
                          index, egress_entry_data) != SOC_E_NONE) {
            FLEXCTR_ERR(("Table:%s:Index:%d: encounter some problem \n",
                         SOC_MEM_UFNAME(unit, egress_table), index));
            sal_free(egress_entry_data);
            return    BCM_E_INTERNAL;
        }
        /* Decrement reference counts */
        flex_base_index_reference_count[unit][bcmStatFlexDirectionEgress]
                                       [pool_number][base_idx]--;
        FLEXCTR_VVERB(("Deallocated Table:%s:Index:%d:mode:%d"
                       "reference_count %d \n",
                       SOC_MEM_UFNAME(unit, egress_table),index,offset_mode,
                       flex_base_index_reference_count[unit]
                       [bcmStatFlexDirectionEgress][pool_number][base_idx]));
        sal_free(egress_entry_data);
        if (flex_base_index_reference_count[unit][bcmStatFlexDirectionEgress]
                                           [pool_number][base_idx] == 0) {
            _bcm_esw_stat_flex_enable_pool(
                    unit,bcmStatFlexDirectionEgress,
                    _pool_ctr_register[bcmStatFlexDirectionEgress][pool_number],
                    0);
        }
        _bcm_esw_stat_get_counter_id(
                      flex_egress_modes[unit][offset_mode].group_mode,
                      object,offset_mode,pool_number,
                      base_idx,&stat_counter_id);
        if (flex_base_index_reference_count[unit][bcmStatFlexDirectionEgress]
                                           [pool_number][base_idx] == 0) {
            if (_bcm_esw_stat_flex_insert_stat_id(
                        local_scache_ptr[unit],stat_counter_id) != BCM_E_NONE) {
                FLEXCTR_WARN(("WARMBOOT:Couldnot add entry in scache memory."
                       "Attach it\n"));
            }
        }
        return BCM_E_NONE;
    }
    sal_free(egress_entry_data);
    return    BCM_E_NOT_FOUND;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_destroy_egress_table_counters
 * Description:
 *      Destroy Egresss accounting table's statistics completely
 * Parameters:
 *      unit                  - (IN) unit number
 *      egress_table          - (IN) Flex Accounting Table
 *      offset_mode           - (IN) Flex offset mode for Accounting Object
 *      base_idx              - (IN) Flex Base Index for Accounting Object
 *      pool_number           - (IN) Flex Pool Number for Accounting Object
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_destroy_egress_table_counters(
            int                  unit,
            soc_mem_t            egress_table,
            bcm_stat_flex_mode_t offset_mode,
            uint32               base_idx,
            uint32               pool_number)
{
    uint32                          free_count=0;
    uint32                          alloc_count=0;
    uint32                          largest_free;
    uint32                          used_by_table=0;
    bcm_stat_object_t               object=bcmStatObjectEgrPort;
    uint32                          stat_counter_id=0;

    if (flex_base_index_reference_count[unit]
        [bcmStatFlexDirectionEgress][pool_number][base_idx] != 0) {
        FLEXCTR_ERR(("Reference count is  %d.. Please detach entries first..\n",
                     flex_base_index_reference_count[unit]
                     [bcmStatFlexDirectionEgress][pool_number]));
        return BCM_E_FAIL;
    }
    switch(egress_table) {
    case EGR_VLANm:
         used_by_table=FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_TABLE;
         break;
    case EGR_VFIm:
         used_by_table=FLEX_COUNTER_POOL_USED_BY_EGR_VFI_TABLE;
         break;
    case EGR_L3_NEXT_HOPm:
         used_by_table=FLEX_COUNTER_POOL_USED_BY_EGR_L3_NEXT_HOP_TABLE;
         break;
    case EGR_VLAN_XLATEm:
         used_by_table=FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_XLATE_TABLE;
         break;
    case EGR_PORTm:
         used_by_table=FLEX_COUNTER_POOL_USED_BY_EGR_PORT_TABLE;
         break;
    default:
         FLEXCTR_ERR(("Invalid Flex Counter Egress Memory %s\n",
                      SOC_MEM_UFNAME(unit, egress_table)));
         return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_egress_object(
                        egress_table,&object));

    /* Free pool list */
    if (shr_aidxres_list_free(flex_aidxres_list_handle
                              [unit][bcmStatFlexDirectionEgress][pool_number],
                              base_idx) != BCM_E_NONE) {
        FLEXCTR_ERR(("Freeing memory Table:%s:encounter problem \n",
                     SOC_MEM_UFNAME(unit, egress_table)));
        return BCM_E_INTERNAL;
    }
    _bcm_esw_stat_get_counter_id(
                  flex_egress_modes[unit][offset_mode].group_mode,
                  object,offset_mode,pool_number,base_idx,&stat_counter_id);
    if (_bcm_esw_stat_flex_delete_stat_id(
                 local_scache_ptr[unit],stat_counter_id) != BCM_E_NONE) {
        FLEXCTR_WARN(("WARMBOOT: Couldnot Delete entry in scache memory.\n"));
    }
    shr_aidxres_list_state(flex_aidxres_list_handle
                           [unit][bcmStatFlexDirectionEgress][pool_number],
                           NULL,NULL,NULL,NULL,
                           &free_count,&alloc_count,&largest_free,NULL);
    FLEXCTR_VVERB(("Pool status free_count:%d alloc_count:%d largest_free:%d"
                   "used_by_tables:%d used_entries:%d\n",
                   free_count,alloc_count,largest_free,
                   flex_pool_stat[unit][bcmStatFlexDirectionEgress]
                         [pool_number].used_by_tables,
                   flex_pool_stat[unit][bcmStatFlexDirectionEgress]
                         [pool_number].used_entries));
    flex_pool_stat[unit][bcmStatFlexDirectionEgress]
                  [pool_number].used_by_tables &= ~used_by_table;
    flex_pool_stat[unit][bcmStatFlexDirectionEgress][pool_number].
                  used_entries -= flex_egress_modes[unit]
                                  [offset_mode].total_counters;
    flex_egress_modes[unit][offset_mode].reference_count--;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_detach_ingress_table_counters
 * Description:
 *      Detach i.e. Disable Igresss accounting table's statistics
 * Parameters:
 *      unit                  - (IN) unit number
 *      ingress_table         - (IN) Flex Accounting Table
 *      index                 - (IN) Flex Accounting Table's Index
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_detach_ingress_table_counters(
            int       unit,
            soc_mem_t ingress_table,
            uint32    index)
{
    uint32                          offset_mode=0;
    uint32                          pool_number=0;
    uint32                          base_idx=0;
    uint32                          ingress_entry_data_size=0;
    uint32                          flex_entries=0;
    uint32                          alloc_size=0;
    bcm_stat_flex_ctr_offset_info_t flex_ctr_offset_info;
    bcm_stat_flex_counter_value_t   *flex_counter_value=NULL;
    void                            *ingress_entry_data=NULL;
    bcm_stat_object_t               object=bcmStatObjectIngPort;
    uint32                          stat_counter_id=0;

    if (!((ingress_table == PORT_TABm) ||
         (ingress_table == VLAN_XLATEm)  ||
         (ingress_table == VFP_POLICY_TABLEm)  ||
         (ingress_table == MPLS_ENTRYm)  ||
         (ingress_table == SOURCE_VPm)  ||
         (ingress_table == L3_IIFm)  ||
         (ingress_table == VRFm)  ||
         (ingress_table == VFIm)  ||
         (ingress_table == VLAN_TABm))) {
          FLEXCTR_ERR(("Invalid Flex Counter Ingress Memory %s\n",
                       SOC_MEM_UFNAME(unit, ingress_table)));
          return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_ingress_object(
                        ingress_table,&object));
    ingress_entry_data_size = WORDS2BYTES(BYTES2WORDS(
                              SOC_MEM_INFO(unit, ingress_table).bytes));
    FLEXCTR_VVERB(("Deallocating IngressCounter Table:%s:Index:%d:"
                   " ENTRY_BYTES:%d \n",
                   SOC_MEM_UFNAME(unit, ingress_table),
                   index,ingress_entry_data_size));
    ingress_entry_data = sal_alloc(ingress_entry_data_size,"ingress_table");
    if (ingress_entry_data == NULL) {
        FLEXCTR_ERR(("Failed to allocate memory for Table:%s ",
                     SOC_MEM_UFNAME(unit, ingress_table)));
        return BCM_E_INTERNAL;
    }
    sal_memset(ingress_entry_data,0,SOC_MEM_INFO(unit, ingress_table).bytes);
    if (soc_mem_read(unit, ingress_table, MEM_BLOCK_ANY,
                     index,ingress_entry_data) == SOC_E_NONE) {
        if (soc_mem_field_valid(unit,ingress_table,VALIDf)) {
            if (soc_mem_field32_get(unit,ingress_table,ingress_entry_data,
                                    VALIDf) == 0) {
                FLEXCTR_VVERB(("Table %s  with index %d is Not valid \n",
                               SOC_MEM_UFNAME(unit, ingress_table),index));
                sal_free(ingress_entry_data);
                return BCM_E_PARAM;
            }
        }
        _bcm_esw_get_flex_counter_fields_values(
                 unit,ingress_table,ingress_entry_data,
                 &offset_mode,&pool_number,&base_idx);
        if ((offset_mode == 0) && (base_idx == 0)) {
             FLEXCTR_ERR(("Table:%s:Index %d IsNotConfiguredForFlexCounter\n",
                          SOC_MEM_UFNAME(unit, ingress_table),index));
             sal_free(ingress_entry_data);
             return BCM_E_NOT_FOUND;/*Either NotConfigured/deallocated before*/
        }

        /* Clear Counter Values */
        FLEXCTR_VVERB(("Clearing Counter Tables %s contents:Offset:%d Len:%d\n",
                       SOC_MEM_UFNAME(unit, ingress_table),
                       base_idx,
                       flex_ingress_modes[unit][offset_mode].total_counters));
        flex_ctr_offset_info.all_counters_flag =  1;
        alloc_size = sizeof(bcm_stat_flex_counter_value_t) *
                  flex_ingress_modes[unit][offset_mode].total_counters;
        flex_counter_value = sal_alloc(alloc_size,"counter-table-values");
        if (flex_counter_value == NULL) {
            FLEXCTR_ERR(("Failed:AllocateCounterMemoryForTable:%s WithIndex %d",
                         SOC_MEM_UFNAME(unit, ingress_table),index));
            return BCM_E_INTERNAL;
        }
        sal_memset(flex_counter_value,0,alloc_size);
        _bcm_esw_stat_flex_set(unit,index,ingress_table,1,
                               flex_ctr_offset_info,&flex_entries,
                               flex_counter_value);
        _bcm_esw_stat_flex_set( unit,index,ingress_table,0,
                         flex_ctr_offset_info,&flex_entries,flex_counter_value);
        sal_free(flex_counter_value);

        /* Reset Table Values */
        _bcm_esw_set_flex_counter_fields_values(
                 unit,ingress_table,ingress_entry_data,0,0,0);
        if (soc_mem_write(unit,ingress_table, MEM_BLOCK_ALL,index,
                          ingress_entry_data) != SOC_E_NONE) {
            FLEXCTR_ERR(("Table:%s:Index %d encounter some problem \n",
                         SOC_MEM_UFNAME(unit, ingress_table),index));
            sal_free(ingress_entry_data);
            return    BCM_E_INTERNAL;
        }

        /* Decrement reference count */
        flex_base_index_reference_count[unit][bcmStatFlexDirectionIngress]
                                       [pool_number][base_idx]--;
        FLEXCTR_VVERB(("Deallocated for Table:%sIndex:%d:"
                       "mode %d reference_count %d\n",
                       SOC_MEM_UFNAME(unit, ingress_table),index,offset_mode,
                       flex_base_index_reference_count[unit]
                       [bcmStatFlexDirectionIngress][pool_number][base_idx]));
        sal_free(ingress_entry_data);
        if (flex_base_index_reference_count
            [unit][bcmStatFlexDirectionIngress][pool_number][base_idx] == 0) {
            _bcm_esw_stat_flex_enable_pool(
                     unit,
                     bcmStatFlexDirectionIngress,
                     _pool_ctr_register[bcmStatFlexDirectionIngress]
                                       [pool_number],
                     0);
        }
        _bcm_esw_stat_get_counter_id(
                      flex_ingress_modes[unit][offset_mode].group_mode,
                      object,offset_mode,pool_number,base_idx,&stat_counter_id);
        if (flex_base_index_reference_count[unit][bcmStatFlexDirectionIngress]
                                           [pool_number][base_idx] == 0) {
            if (_bcm_esw_stat_flex_insert_stat_id(
                        local_scache_ptr[unit],stat_counter_id) != BCM_E_NONE) {
                FLEXCTR_WARN(("WARMBOOT: Couldnot add entry in scache memory."
                              "Attach it\n"));
            }
        }
        return BCM_E_NONE;
    }
    sal_free(ingress_entry_data);
    return    BCM_E_NOT_FOUND;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_destroy_ingress_table_counters
 * Description:
 *      Destroy Igresss accounting table's statistics completely
 * Parameters:
 *      unit                  - (IN) unit number
 *      ingress_table         - (IN) Flex Accounting Table
 *      offset_mode           - (IN) Flex offset mode for Accounting Object
 *      base_idx              - (IN) Flex Base Index for Accounting Object
 *      pool_number           - (IN) Flex Pool Number for Accounting Object
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_destroy_ingress_table_counters(
            int                  unit,
            soc_mem_t            ingress_table,
            bcm_stat_flex_mode_t offset_mode,
            uint32               base_idx,
            uint32               pool_number)
{
    uint32                          free_count=0;
    uint32                          alloc_count=0;
    uint32                          largest_free=0;
    uint32                          used_by_table=0;
    bcm_stat_object_t               object=bcmStatObjectIngPort;
    uint32                          stat_counter_id=0;

    if (flex_base_index_reference_count[unit][bcmStatFlexDirectionIngress]
                                       [pool_number][base_idx] != 0) {
        FLEXCTR_ERR(("Reference count is  %d.. Please detach entries first..\n",
                     flex_ingress_modes[unit][offset_mode].reference_count));
        return BCM_E_FAIL;
    }
    switch(ingress_table) {
    case PORT_TABm:
         used_by_table = FLEX_COUNTER_POOL_USED_BY_PORT_TABLE;
         break;
    case VFP_POLICY_TABLEm:
         used_by_table = FLEX_COUNTER_POOL_USED_BY_VFP_POLICY_TABLE;
         break;
         /* VLAN and VFI shares same pool */
    case VLAN_TABm:
         used_by_table = FLEX_COUNTER_POOL_USED_BY_VLAN_TABLE;
         break;
    case VFIm:
         used_by_table = FLEX_COUNTER_POOL_USED_BY_VFI_TABLE;
         break;
         /* VRF and MPLS_VC_LABEL shares same pool */
    case VRFm:
         used_by_table = FLEX_COUNTER_POOL_USED_BY_VRF_TABLE;
         break;
    case MPLS_ENTRYm: 
         used_by_table = FLEX_COUNTER_POOL_USED_BY_MPLS_ENTRY_TABLE;
         break;
    case VLAN_XLATEm:
         used_by_table = FLEX_COUNTER_POOL_USED_BY_VLAN_XLATE_TABLE;
         break;
         /* L3_IIF and SOURCE_VP shares same pool*/
    case L3_IIFm:
         used_by_table = FLEX_COUNTER_POOL_USED_BY_L3_IIF_TABLE;
         break;
    case SOURCE_VPm:
         used_by_table = FLEX_COUNTER_POOL_USED_BY_SOURCE_VP_TABLE;
         break;
    default:
         FLEXCTR_ERR(("Invalid Table is passed %d \n",ingress_table));
         return BCM_E_INTERNAL;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_ingress_object(
                        ingress_table,&object));
    if (shr_aidxres_list_free(flex_aidxres_list_handle
                              [unit][bcmStatFlexDirectionIngress][pool_number],
                              base_idx) != BCM_E_NONE) {
        FLEXCTR_ERR(("Freeing memory Table:%s:encounter some problem \n",
                     SOC_MEM_UFNAME(unit, ingress_table)));
        return    BCM_E_INTERNAL;
    }
    _bcm_esw_stat_get_counter_id(
                  flex_ingress_modes[unit][offset_mode].group_mode,
                  object,offset_mode,pool_number,base_idx,&stat_counter_id);
    if (_bcm_esw_stat_flex_delete_stat_id(
                 local_scache_ptr[unit],stat_counter_id) != BCM_E_NONE) {
        FLEXCTR_WARN(("WARMBOOT: Couldnot Delete entry in scache memory.\n"));
    }
    shr_aidxres_list_state(flex_aidxres_list_handle
                           [unit][bcmStatFlexDirectionIngress][pool_number],
                           NULL,NULL,NULL,NULL,
                           &free_count,&alloc_count,&largest_free,NULL);
    FLEXCTR_VVERB(("Current Pool status free_count:%d alloc_count:%d"
                   "largest_free:%d used_by_tables:%d used_entries:%d\n",
                   free_count,alloc_count,largest_free,
                   flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                         [pool_number].used_by_tables,
                   flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                         [pool_number].used_entries));
    flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                  [pool_number].used_by_tables &= ~used_by_table;
    flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                  [pool_number].used_entries -= flex_ingress_modes[unit]
                                                [offset_mode].total_counters;
    flex_ingress_modes[unit][offset_mode].reference_count--;
    return    BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_attach_egress_table_counters
 * Description:
 *      Atach i.e. Enable Egresss accounting table's statistics
 * Parameters:
 *      unit                  - (IN) unit number
 *      egress_table          - (IN) Flex Accounting Table
 *      index                 - (IN) Flex Accounting Table's Index
 *      mode                  - (IN) Flex offset mode for Accounting Object
 *      base_idx              - (IN) Flex Base Index for Accounting Object
 *      pool_number           - (IN) Flex Pool Number for Accounting Object
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_attach_egress_table_counters(
            int                  unit,
            soc_mem_t            egress_table,
            uint32               index,
            bcm_stat_flex_mode_t mode,
            uint32               base_idx,
            uint32               pool_number)
{
    uint32               egress_entry_data_size=0;
    void                 *egress_entry_data=NULL;
    bcm_stat_flex_mode_t offset_mode_l={0};
    bcm_stat_object_t    object=bcmStatObjectEgrPort;
    uint32               stat_counter_id=0;
    uint32               base_idx_l=0;
    uint32               pool_number_l=0;

    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        FLEXCTR_ERR(("Invalid flex counter mode value %d \n",mode));
        return BCM_E_PARAM;
    }
    if (flex_egress_modes[unit][mode].available==0) {
        FLEXCTR_ERR(("flex CounterMode:%d:Not configured yet\n",mode));
        return BCM_E_NOT_FOUND;
    }
    if (shr_aidxres_list_elem_state(flex_aidxres_list_handle
                                    [unit][bcmStatFlexDirectionEgress]
                                    [pool_number],base_idx) != BCM_E_EXISTS) {
        FLEXCTR_ERR(("Wrong base index %u \n",base_idx));
        return BCM_E_NOT_FOUND;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_egress_object(
                        egress_table,&object));
    egress_entry_data_size = WORDS2BYTES(BYTES2WORDS(
                             SOC_MEM_INFO(unit, egress_table).bytes));
    FLEXCTR_VVERB((".Allocating EgressCounter Table:%s:Index:%d:Mode:%d"
                   " ENTRY_BYTES %d\n", 
                   SOC_MEM_UFNAME(unit,egress_table),index,mode,
                   egress_entry_data_size));
    egress_entry_data = sal_alloc(egress_entry_data_size,"egress_table");
    if (egress_entry_data == NULL) {
        FLEXCTR_ERR(("Failed to allocate memory for Table:%s ",
                     SOC_MEM_UFNAME(unit, egress_table)));
        return BCM_E_INTERNAL;
    }
    sal_memset(egress_entry_data,0,SOC_MEM_INFO(unit, egress_table).bytes);

    if (soc_mem_read(unit, egress_table, MEM_BLOCK_ANY,index,
                     egress_entry_data) != SOC_E_NONE) {
        FLEXCTR_ERR(("Read failure for Table %s with index %d \n",
                     SOC_MEM_UFNAME(unit, egress_table),index));
        sal_free(egress_entry_data);
        return BCM_E_INTERNAL;
    }
    if (soc_mem_field_valid(unit,egress_table,VALIDf)) {
        if (soc_mem_field32_get(unit,egress_table,egress_entry_data,
                                VALIDf)==0) {
            FLEXCTR_ERR(("Table %s  with index %d is Not valid \n",
                         SOC_MEM_UFNAME(unit, egress_table),index));
            sal_free(egress_entry_data);
            return BCM_E_PARAM;
        }
    }
    _bcm_esw_get_flex_counter_fields_values(
                 unit,egress_table,egress_entry_data,
                 &offset_mode_l,&pool_number_l,&base_idx_l);
    if (base_idx_l != 0) {
        FLEXCTR_ERR(("Table:%s HasAlreadyAllocatedWithIndex:%d base %d mode %d."
                     "First dealloc it \n",SOC_MEM_UFNAME(unit, egress_table),
                     index,base_idx_l,offset_mode_l));
        sal_free(egress_entry_data);
        return BCM_E_EXISTS;/*Either Not configured or deallocated before*/
    }
    _bcm_esw_set_flex_counter_fields_values(
             unit,egress_table,egress_entry_data,mode,pool_number,base_idx);
    if (soc_mem_write(unit,egress_table, MEM_BLOCK_ALL,index,
                      egress_entry_data) != SOC_E_NONE) {
        sal_free(egress_entry_data);
        return BCM_E_INTERNAL;
    }
    sal_free(egress_entry_data);
    _bcm_esw_stat_get_counter_id(
                  flex_egress_modes[unit][mode].group_mode,
                  object,mode,pool_number,base_idx,&stat_counter_id);
    if (flex_base_index_reference_count[unit][bcmStatFlexDirectionEgress]
                                       [pool_number][base_idx] == 0) {
        if (_bcm_esw_stat_flex_delete_stat_id(local_scache_ptr[unit],
                                              stat_counter_id) != BCM_E_NONE) {
            FLEXCTR_WARN(("WARMBOOT:Couldnot Delete entry in scache memory\n"));
        }
    }
    flex_base_index_reference_count[unit][bcmStatFlexDirectionEgress]
                                   [pool_number][base_idx]++;
    if (flex_base_index_reference_count[unit][bcmStatFlexDirectionEgress]
                                       [pool_number][base_idx] == 1) { 
        _bcm_esw_stat_flex_enable_pool(
                 unit,bcmStatFlexDirectionEgress,
                 _pool_ctr_register[bcmStatFlexDirectionEgress][pool_number],1);
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_create_egress_table_counters
 * Description:
 *      Create and Reserve Flex Counter Space for Egresss accounting table 
 * Parameters:
 *      unit                  - (IN) unit number
 *      egress_table          - (IN) Flex Accounting Table
 *      mode                  - (IN) Flex offset mode for Accounting Object
 *      base_idx              - (IN) Flex Base Index for Accounting Object
 *      pool_number           - (IN) Flex Pool Number for Accounting Object
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_create_egress_table_counters(
            int                  unit,
            soc_mem_t            egress_table,
            bcm_stat_flex_mode_t mode,
            uint32               *base_idx,
            uint32               *pool_number)
{
    uint32            base_idx_l=0;
    uint32            pool_number_l=0;
    uint32            default_pool_number=0;
    uint32            used_by_table=0;
    uint32            used_by_table_selection_criteria=0;
    uint32            free_count=0;
    uint32            alloc_count=0;
    uint32            largest_free=0;
    bcm_stat_object_t object=bcmStatObjectEgrPort;
    uint32            stat_counter_id=0;
    uint32            num_flex_egress_pools = SOC_INFO(unit).
                                              num_flex_egress_pools;
    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        FLEXCTR_ERR(("Invalid flex counter mode value %d \n",mode));
        return BCM_E_PARAM;
    }
    if (flex_egress_modes[unit][mode].available==0) {
        FLEXCTR_ERR(("flex CounterMode:%d:Not configured yet\n",mode));
        return BCM_E_NOT_FOUND;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_egress_object(
                        egress_table,&object));
    /* Below case statement can be avoided by passing arguements for pool_number
       and selection criteria But keeping it for better understanding 
       at same place */
    switch(egress_table) {
    case EGR_VLANm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_EGR_VLAN_TABLE_POOL_NUMBER;
         used_by_table_selection_criteria=used_by_table =
                 FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_TABLE;
         break;
    case EGR_VFIm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_EGR_VFI_TABLE_POOL_NUMBER;
         used_by_table_selection_criteria=used_by_table =
                 FLEX_COUNTER_POOL_USED_BY_EGR_VFI_TABLE;
         break;
    case EGR_L3_NEXT_HOPm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_EGR_L3_NEXT_HOP_TABLE_POOL_NUMBER;
         used_by_table_selection_criteria=used_by_table =
                 FLEX_COUNTER_POOL_USED_BY_EGR_L3_NEXT_HOP_TABLE;
         break;
    case EGR_VLAN_XLATEm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_EGR_VLAN_XLATE_TABLE_POOL_NUMBER;
         used_by_table_selection_criteria=used_by_table =
                 FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_XLATE_TABLE;
         break;
   case EGR_PORTm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_EGR_PORT_TABLE_POOL_NUMBER;
         used_by_table_selection_criteria=used_by_table =
                 FLEX_COUNTER_POOL_USED_BY_EGR_PORT_TABLE;
         break;
   default:
         FLEXCTR_ERR(("Invalid Table is passed %d \n",egress_table));
         return BCM_E_INTERNAL;
   }
   do {
      /* Either free or being used by port table only */
      if ((flex_pool_stat[unit][bcmStatFlexDirectionEgress]
           [pool_number_l].used_by_tables == 0) ||
          (flex_pool_stat[unit][bcmStatFlexDirectionEgress]
           [pool_number_l].used_by_tables & used_by_table_selection_criteria)) {
           if (shr_aidxres_list_alloc_block(flex_aidxres_list_handle
                   [unit][bcmStatFlexDirectionEgress][pool_number_l],
                   flex_egress_modes[unit][mode].total_counters,
                   &base_idx_l) == BCM_E_NONE) {
               flex_pool_stat[unit][bcmStatFlexDirectionEgress]
                    [pool_number_l].used_by_tables |= used_by_table;
               flex_pool_stat[unit][bcmStatFlexDirectionEgress]
                    [pool_number_l].used_entries += flex_egress_modes
                                                    [unit][mode].total_counters;
               FLEXCTR_VVERB(("Allocated  counter Table:%s with pool_number:%d"
                              "mode:%d base_idx:%d ref_count %d\n",
                              SOC_MEM_UFNAME(unit, egress_table),
                              pool_number_l,mode,base_idx_l,
                              flex_egress_modes[unit][mode].reference_count));
               shr_aidxres_list_state(
                   flex_aidxres_list_handle[unit][bcmStatFlexDirectionEgress]
                                           [pool_number_l],
                   NULL,NULL,NULL,NULL,&free_count,&alloc_count,&largest_free,
                   NULL);
               FLEXCTR_VVERB(("Current Pool status free_count:%d alloc_count:%d"
                              "largest_free:%d used_by_tables:%d"
                              "used_entries:%d\n",
                              free_count,alloc_count,largest_free,
                              flex_pool_stat[unit][bcmStatFlexDirectionEgress]
                                    [pool_number_l].used_by_tables,
                              flex_pool_stat[unit][bcmStatFlexDirectionEgress]
                                    [pool_number_l].used_entries));
               *base_idx = base_idx_l;
               *pool_number = pool_number_l;
               _bcm_esw_stat_get_counter_id(
                             flex_egress_modes[unit][mode].group_mode,
                             object,mode,pool_number_l,base_idx_l,
                             &stat_counter_id);
               flex_egress_modes[unit][mode].reference_count++;
               if (_bcm_esw_stat_flex_insert_stat_id(
                   local_scache_ptr[unit], stat_counter_id) != BCM_E_NONE) {
                   FLEXCTR_WARN(("WARMBOOT:Couldnot add entry in scache memory"
                                 "Attach it\n"));
               }
               return BCM_E_NONE;
           }
      }
      pool_number_l = (pool_number_l+1) % num_flex_egress_pools;
    } while(pool_number_l != default_pool_number);
    FLEXCTR_ERR(("Pools exhausted for Table:%s\n",
                 SOC_MEM_UFNAME(unit, egress_table)));
    return BCM_E_FAIL; /*or BCM_E_RESOURCE */
}
/*
 * Function:
 *      _bcm_esw_stat_flex_attach_ingress_table_counters
 * Description:
 *      Atach i.e. Enable Ingresss accounting table's statistics
 * Parameters:
 *      unit                  - (IN) unit number
 *      ingress_table         - (IN) Flex Accounting Table
 *      index                 - (IN) Flex Accounting Table's Index
 *      mode                  - (IN) Flex offset mode for Accounting Object
 *      base_idx              - (IN) Flex Base Index for Accounting Object
 *      pool_number           - (IN) Flex Pool Number for Accounting Object
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_attach_ingress_table_counters(
            int                  unit,
            soc_mem_t            ingress_table,
            uint32               index,
            bcm_stat_flex_mode_t mode,
            uint32               base_idx,
            uint32               pool_number)
{
    uint32               ingress_entry_data_size=0;
    void                 *ingress_entry_data;
    bcm_stat_flex_mode_t offset_mode_l={0};
    bcm_stat_object_t    object=bcmStatObjectIngPort;
    uint32               stat_counter_id=0;
    uint32               base_idx_l=0;
    uint32               pool_number_l=0;

    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        FLEXCTR_ERR(("Invalid flex counter mode value %d \n",mode));
        return BCM_E_PARAM;
    }
    if (flex_ingress_modes[unit][mode].available==0) {
        FLEXCTR_ERR(("flex counter mode %d not configured yet\n",mode));
        return BCM_E_NOT_FOUND;
    }
    if (shr_aidxres_list_elem_state(
            flex_aidxres_list_handle[unit][bcmStatFlexDirectionIngress]
            [pool_number],
            base_idx) != BCM_E_EXISTS) {
        FLEXCTR_ERR(("Wrong base index %u \n",base_idx));
        return BCM_E_NOT_FOUND;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_ingress_object(
                        ingress_table,&object));

    ingress_entry_data_size = WORDS2BYTES(BYTES2WORDS(
                              SOC_MEM_INFO(unit, ingress_table).bytes));
    FLEXCTR_VVERB((".Attaching INGRESS counter for Table:%s with index:%d"
                   "mode:%d ENTRY_BYTES %d \n",
                   SOC_MEM_UFNAME(unit,ingress_table),index,mode,
                   ingress_entry_data_size));
    ingress_entry_data = sal_alloc(ingress_entry_data_size,"ingress_table");
    if (ingress_entry_data == NULL) {
        FLEXCTR_ERR(("Failed to allocate memory for Table:%s ",
                     SOC_MEM_UFNAME(unit, ingress_table)));
        return BCM_E_INTERNAL;
    }
    sal_memset(ingress_entry_data,0,SOC_MEM_INFO(unit, ingress_table).bytes);
    if (soc_mem_read(unit, ingress_table, MEM_BLOCK_ANY,index,
                     ingress_entry_data) != SOC_E_NONE) {
        FLEXCTR_ERR(("Read failure for Table %s with index %d \n",
                     SOC_MEM_UFNAME(unit, ingress_table),index));
        sal_free(ingress_entry_data);
        return    BCM_E_INTERNAL;
    }
    if (soc_mem_field_valid(unit,ingress_table,VALIDf)) {
        if (soc_mem_field32_get(unit,ingress_table,ingress_entry_data,
            VALIDf)==0) {
            FLEXCTR_ERR(("Table %s  with index %d is Not valid \n",
                         SOC_MEM_UFNAME(unit, ingress_table),index));
            sal_free(ingress_entry_data);
            return BCM_E_PARAM;
        }
    }
    _bcm_esw_get_flex_counter_fields_values(
               unit,ingress_table,ingress_entry_data,
               &offset_mode_l,&pool_number_l,&base_idx_l);
    if (base_idx_l != 0) {
        FLEXCTR_ERR(("Table:%s Has already allocated with index:%d"
                     "base %d mode %d."
                     "First dealloc it \n", SOC_MEM_UFNAME(unit, ingress_table),
                     index,base_idx_l,offset_mode_l));
        sal_free(ingress_entry_data);
        return BCM_E_EXISTS;/*Either NotConfigured or deallocated before */
    }
    _bcm_esw_set_flex_counter_fields_values(
             unit,ingress_table,ingress_entry_data,mode,pool_number,base_idx);
    if (soc_mem_write(unit,ingress_table, MEM_BLOCK_ALL,index,
                      ingress_entry_data) != SOC_E_NONE) {
        sal_free(ingress_entry_data);
        return    BCM_E_INTERNAL;
    }
    sal_free(ingress_entry_data);
    _bcm_esw_stat_get_counter_id(
                  flex_ingress_modes[unit][mode].group_mode,
                  object,mode,pool_number,base_idx,&stat_counter_id);
    if (flex_base_index_reference_count[unit][bcmStatFlexDirectionIngress]
                                       [pool_number][base_idx] == 0) {
        if (_bcm_esw_stat_flex_delete_stat_id(
            local_scache_ptr[unit],stat_counter_id) != BCM_E_NONE) {
            FLEXCTR_WARN(("WARMBOOT:Couldnot Delete entry in scache memory\n"));
        }
    }
    flex_base_index_reference_count[unit][bcmStatFlexDirectionIngress]
                                   [pool_number][base_idx]++;
    if (flex_base_index_reference_count[unit][bcmStatFlexDirectionIngress]
                                   [pool_number][base_idx] == 1) {
        _bcm_esw_stat_flex_enable_pool(
                unit,bcmStatFlexDirectionIngress,
                _pool_ctr_register[bcmStatFlexDirectionIngress][pool_number],1);
    }
    return BCM_E_NONE; 
}
/*
 * Function:
 *      _bcm_esw_stat_flex_create_ingress_table_counters
 * Description:
 *      Create and Reserve Flex Counter Space for Ingresss accounting table 
 * Parameters:
 *      unit                  - (IN) unit number
 *      ingress_table         - (IN) Flex Accounting Table
 *      mode                  - (IN) Flex offset mode for Accounting Object
 *      base_idx              - (IN) Flex Base Index for Accounting Object
 *      pool_number           - (IN) Flex Pool Number for Accounting Object
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_create_ingress_table_counters(
            int                  unit,
            soc_mem_t            ingress_table,
            bcm_stat_flex_mode_t mode,
            uint32               *base_idx,
            uint32               *pool_number)
{
    uint32            base_idx_l=0;
    uint32            pool_number_l=0;
    uint32            default_pool_number=0;
    uint32            used_by_table=0;
    uint32            used_by_table_selection_criteria=0;
    uint32            free_count=0;
    uint32            alloc_count=0;
    uint32            largest_free=0;
    bcm_stat_object_t object=bcmStatObjectIngPort;
    uint32            stat_counter_id=0;
    uint32            num_flex_ingress_pools=SOC_INFO(unit).
                                             num_flex_ingress_pools;

    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        FLEXCTR_ERR(("Invalid flex counter mode value %d \n",mode));
        return BCM_E_PARAM;
    }
    if (flex_ingress_modes[unit][mode].available==0) {
        FLEXCTR_ERR(("flex counter mode %d not configured yet\n",mode));
        return BCM_E_NOT_FOUND;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_ingress_object(
                        ingress_table,&object));

    /* Below case statement can be avoided by passing arguements for 
       pool_number and selection criteria But keeping it for better 
       understanding at same place */
    switch(ingress_table) {
    case PORT_TABm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_PORT_TABLE_POOL_NUMBER;
         used_by_table_selection_criteria=used_by_table =
                 FLEX_COUNTER_POOL_USED_BY_PORT_TABLE;
         break;
    case VFP_POLICY_TABLEm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_VFP_POLICY_TABLE_POOL_NUMBER;
         used_by_table_selection_criteria=used_by_table =
                 FLEX_COUNTER_POOL_USED_BY_VFP_POLICY_TABLE;
         break;
        /* VLAN and VFI shares same pool */
    case VLAN_TABm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_VLAN_TABLE_POOL_NUMBER;
         used_by_table = FLEX_COUNTER_POOL_USED_BY_VLAN_TABLE;
         used_by_table_selection_criteria=
                 (FLEX_COUNTER_POOL_USED_BY_VLAN_TABLE |
                  FLEX_COUNTER_POOL_USED_BY_VFI_TABLE);
         break;
    case VFIm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_VFI_TABLE_POOL_NUMBER;
         used_by_table = FLEX_COUNTER_POOL_USED_BY_VFI_TABLE;
         used_by_table_selection_criteria=
                 (FLEX_COUNTER_POOL_USED_BY_VLAN_TABLE |
                  FLEX_COUNTER_POOL_USED_BY_VFI_TABLE );
         break;
        /* VRF and MPLS_VC_LABEL shares same pool */
    case VRFm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_VRF_TABLE_POOL_NUMBER;
         used_by_table = FLEX_COUNTER_POOL_USED_BY_VRF_TABLE;
         used_by_table_selection_criteria=FLEX_COUNTER_POOL_USED_BY_VRF_TABLE;
         break;

        /* VLAN_XLATE and MPLS_TABLE shares same pool */
    case MPLS_ENTRYm: 
         default_pool_number=pool_number_l=
               FLEX_COUNTER_DEFAULT_MPLS_ENTRY_TABLE_POOL_NUMBER;
         used_by_table =
               FLEX_COUNTER_POOL_USED_BY_MPLS_ENTRY_TABLE;
         used_by_table_selection_criteria=
               (FLEX_COUNTER_POOL_USED_BY_VLAN_XLATE_TABLE|
                FLEX_COUNTER_POOL_USED_BY_MPLS_ENTRY_TABLE);
         break;
    case VLAN_XLATEm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_VLAN_XLATE_TABLE_POOL_NUMBER;
         used_by_table = FLEX_COUNTER_POOL_USED_BY_VLAN_XLATE_TABLE;
         used_by_table_selection_criteria=
                 (FLEX_COUNTER_POOL_USED_BY_VLAN_XLATE_TABLE|
                  FLEX_COUNTER_POOL_USED_BY_MPLS_ENTRY_TABLE);
         break;
        /* L3_IIF and SOURCE_VP shares same pool*/
    case L3_IIFm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_L3_IIF_TABLE_POOL_NUMBER;
         /* Time being keeping separate default pool for l3_iif and source_vp */
         used_by_table = FLEX_COUNTER_POOL_USED_BY_L3_IIF_TABLE;
         /* But IfRequired L3_IIF & SOURCE_VP counters can be shared in 
            same pool*/
         used_by_table_selection_criteria=
                 FLEX_COUNTER_POOL_USED_BY_L3_IIF_TABLE |
                 FLEX_COUNTER_POOL_USED_BY_SOURCE_VP_TABLE ;
         break;
    case SOURCE_VPm:
         default_pool_number=pool_number_l=
                 FLEX_COUNTER_DEFAULT_SOURCE_VP_TABLE_POOL_NUMBER;
         /* Time being keeping separate default pool for source vp and l3_iif */
         used_by_table = FLEX_COUNTER_POOL_USED_BY_SOURCE_VP_TABLE;
         /* But IfRequired SOURCE_VP & L3_IIF counters CanBeShared in same 
            pool!*/
         used_by_table_selection_criteria=
                 FLEX_COUNTER_POOL_USED_BY_L3_IIF_TABLE |
                 FLEX_COUNTER_POOL_USED_BY_SOURCE_VP_TABLE ;
         break;
    default:
         FLEXCTR_ERR(("Invalid Table is passed %d \n",ingress_table));
         return BCM_E_INTERNAL;
    }

    do {
       /* Either free or being used by port table only */
       if ((flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                          [pool_number_l].used_by_tables == 0) ||
           (flex_pool_stat[unit][bcmStatFlexDirectionIngress][pool_number_l].
            used_by_tables & used_by_table_selection_criteria)) {
           if (shr_aidxres_list_alloc_block(flex_aidxres_list_handle[unit]
                   [bcmStatFlexDirectionIngress][pool_number_l],
                   flex_ingress_modes
                   [unit][mode].total_counters,
                   &base_idx_l) == BCM_E_NONE) {
               flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                    [pool_number_l].used_by_tables |= used_by_table;
               flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                    [pool_number_l].used_entries += flex_ingress_modes
                                                    [unit][mode].total_counters;
               FLEXCTR_VVERB(("Allocated  counter for Table:%s "
                              "pool_number:%d mode:%d base_idx:%d \n",
                              SOC_MEM_UFNAME(unit, ingress_table),
                              pool_number_l,mode,base_idx_l,
                              flex_ingress_modes[unit][mode].reference_count));
               shr_aidxres_list_state(flex_aidxres_list_handle[unit]
                   [bcmStatFlexDirectionIngress][pool_number_l],
                   NULL,NULL,NULL,NULL,&free_count,&alloc_count,&largest_free,
                   NULL);
               FLEXCTR_VVERB(("Current Pool status free_count:%d alloc_count:%d"
                              "largest_free:%d used_by_tables:%d"
                              "used_entries:%d\n",
                              free_count,alloc_count,largest_free,
                              flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                                    [pool_number_l].used_by_tables,
                              flex_pool_stat[unit][bcmStatFlexDirectionIngress]
                                    [pool_number_l].used_entries));
               *pool_number    = pool_number_l;
               *base_idx       = base_idx_l;
               _bcm_esw_stat_get_counter_id(
                             flex_ingress_modes[unit][mode].group_mode,
                             object,mode,pool_number_l,base_idx_l,
                             &stat_counter_id);
               flex_ingress_modes[unit][mode].reference_count++;
               if (_bcm_esw_stat_flex_insert_stat_id(
                   local_scache_ptr[unit],stat_counter_id) != BCM_E_NONE) {
                   FLEXCTR_WARN(("WARMBOOT:Couldnot add entry in scache memory"
                                 ".Attach it\n"));
               }
               return BCM_E_NONE;
           }
       }
       pool_number_l = (pool_number_l+1) % num_flex_ingress_pools;
    }while(pool_number_l != default_pool_number);
    FLEXCTR_ERR(("Pools exhausted for Table:%s\n",
                 SOC_MEM_UFNAME(unit,ingress_table)));
    return BCM_E_FAIL; /*or BCM_E_RESOURCE*/
}
/*
 * Function:
 *      _bcm_esw_stat_flex_set_group_mode
 * Description:
 *      Set Flex group mode  in s/w copy  for reference
 * Parameters:
 *      unit                  - (IN) unit number
 *      direction             - (IN) Flex Data Flow Direction
 *      offset_mode           - (IN) Flex Offset Mode
 *      group_mode            - (IN) Flex Group Mode
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_set_group_mode(
            int                       unit,
            bcm_stat_flex_direction_t direction,
            uint32                    offset_mode,
            bcm_stat_group_mode_t     group_mode)
{
    /* Better to check */
    if(!((group_mode >= bcmStatGroupModeSingle) &&
         (group_mode <= bcmStatGroupModeDvpType))) {
         FLEXCTR_ERR(("Invalid bcm_stat_group_mode_t passed %d \n",group_mode));
         return BCM_E_PARAM;
    }
    if (direction == bcmStatFlexDirectionIngress) {
        flex_ingress_modes[unit][offset_mode].group_mode = group_mode;
    } else {
        flex_egress_modes[unit][offset_mode].group_mode = group_mode;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_reset_group_mode
 * Description:
 *      ReSet Flex group mode  in s/w copy  for reference
 * Parameters:
 *      unit                  - (IN) unit number
 *      direction             - (IN) Flex Data Flow Direction
 *      offset_mode           - (IN) Flex Offset Mode
 *      group_mode            - (IN) Flex Group Mode
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_reset_group_mode(
            int                       unit,
            bcm_stat_flex_direction_t direction,
            uint32                    offset_mode,
            bcm_stat_group_mode_t     group_mode)
{
    /* Better to check */
    if(!((group_mode >= bcmStatGroupModeSingle) &&
         (group_mode <= bcmStatGroupModeDvpType))) {
         FLEXCTR_ERR(("Invalid bcm_stat_group_mode_t passed %d \n",group_mode));
         return BCM_E_PARAM;
    }
    if (direction == bcmStatFlexDirectionIngress) {
        flex_ingress_modes[unit][offset_mode].group_mode = 0;
    } else {
        flex_egress_modes[unit][offset_mode].group_mode = 0;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_show_mode_info
 * Description:
 *      Show Mode information
 * Parameters:
 *      unit                  - (IN) unit number
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      For Debugging purpose only
 */
void _bcm_esw_stat_flex_show_mode_info(int unit)
{
    uint32                    mode=0;
    soc_cm_print("#####################  INGRESS  #########################\n");
    soc_cm_print("Mode:\t\tReference_Count\t\tTotal_Counters\t\tGroup_Mode \n");
    for (mode=0;mode<BCM_STAT_FLEX_COUNTER_MAX_MODE;mode++) {
         if (flex_ingress_modes[unit][mode].available) {
             soc_cm_print("%u\t\t%u\t\t%u\t\t%s\n", mode,
                    flex_ingress_modes[unit][mode].reference_count,
                    flex_ingress_modes[unit][mode].total_counters,
                    flex_group_modes[flex_ingress_modes[unit][mode].
                                     group_mode]);
         } else {
             soc_cm_print("%u===UNCONFIGURED====\n", mode);
         }
    }
    soc_cm_print("#####################  EGRESS  ##########################\n");
    soc_cm_print("Mode:\t\tReference_Count\t\tTotal_Counters\t\tGroup_Mode \n");
    for (mode=0;mode<BCM_STAT_FLEX_COUNTER_MAX_MODE;mode++) {
         if (flex_egress_modes[unit][mode].available) {
             soc_cm_print("%u\t\t%u\t\t%u\t\t%s\n", mode,
                    flex_egress_modes[unit][mode].reference_count,
                    flex_egress_modes[unit][mode].total_counters,
                    flex_group_modes[flex_egress_modes[unit][mode].group_mode]);
         } else {
             soc_cm_print("%u===UNCONFIGURED====\n", mode);
         }
    }
}
/*
 * Function:
 *      _bcm_esw_stat_flex_get_ingress_object
 * Description:
 *      Get Ingress Object  corresponding to given Ingress Flex Table
 * Parameters:
 *      ingress_table                  - (IN) Flex Ingress Table
 *      object                         - (OUT) Flex Ingress Object
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_get_ingress_object(
            soc_mem_t         ingress_table,
            bcm_stat_object_t *object)
{
    switch(ingress_table) {
    case PORT_TABm: 
         *object=bcmStatObjectIngPort;
         break;
    case VLAN_TABm: 
         *object=bcmStatObjectIngVlan;
         break;
    case VLAN_XLATEm: 
         *object=bcmStatObjectIngVlanXlate;
         break;
    case VFIm: 
         *object=bcmStatObjectIngVfi;
         break;
    case L3_IIFm: 
         *object=bcmStatObjectIngL3Intf;
         break;
    case VRFm: 
         *object=bcmStatObjectIngVrf;
         break;
    case SOURCE_VPm: 
         *object=bcmStatObjectIngMplsVcLabel;
         break;
    case MPLS_ENTRYm: 
         *object=bcmStatObjectIngMplsSwitchLabel;
         break;
    case VFP_POLICY_TABLEm:
         *object=bcmStatObjectIngPolicy;
         break;
    default:
         return BCM_E_INTERNAL;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_get_egress_object
 * Description:
 *      Get Egress Object  corresponding to given Egress Flex Table
 * Parameters:
 *      egress_table                   - (IN) Flex Egress Table
 *      object                         - (OUT) Flex Egress Object
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_get_egress_object(
            soc_mem_t         egress_table,
            bcm_stat_object_t *object)
{
    switch(egress_table) {
    case EGR_PORTm:
         *object=bcmStatObjectEgrPort;
         break;
    case EGR_VLANm:
         *object=bcmStatObjectEgrVlan;
         break;
    case EGR_VLAN_XLATEm:
         *object=bcmStatObjectEgrVlanXlate;
         break;
    case EGR_VFIm:
         *object=bcmStatObjectEgrVfi;
         break;
    case EGR_L3_NEXT_HOPm:
         *object=bcmStatObjectEgrL3Intf;
         break;
    default:
         return BCM_E_INTERNAL;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_get_counter_id
 * Description:
 *      Get Stat Counter Id for given accounting table and index
 * Parameters
 *      unit                  - (IN) unit number
 *      num_of_tables         - (IN) Number of Accounting Tables
 *      table_info            - (IN) Tables Info(Name,direction and index)
 *      num_stat_counter_ids  - (OUT) Number of Stat Counter Ids
 *      stat_counter_id       - (OUT) Stat Counter Ids
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t _bcm_esw_stat_flex_get_counter_id(
            int                        unit,
            uint32                     num_of_tables,
            bcm_stat_flex_table_info_t *table_info,
            uint32                     *num_stat_counter_ids,
            uint32                     *stat_counter_id)
{
    int                        index=0;
    uint32                     offset_mode=0;
    uint32                     pool_number=0;
    uint32                     base_idx=0;
    void                       *entry_data=NULL;
    uint32                     entry_data_size=0;
    bcm_stat_object_t          object=bcmStatObjectIngPort;
    bcm_stat_group_mode_t      group= bcmStatGroupModeSingle;

    for (index=0;index < num_of_tables ;index++) {
         entry_data_size = WORDS2BYTES(BYTES2WORDS(
                           SOC_MEM_INFO(unit,table_info[index].table).bytes));
         entry_data = sal_alloc(entry_data_size,"vrf_table");
         sal_memset(entry_data,0,
                    SOC_MEM_INFO(unit, table_info[index].table).bytes);
         if (soc_mem_read(unit, table_info[index].table, MEM_BLOCK_ANY,
                          table_info[index].index,entry_data) == SOC_E_NONE) {
             if (soc_mem_field_valid(unit,table_info[index].table,VALIDf)) {
                 if (soc_mem_field32_get(unit,table_info[index].table,
                                         entry_data,VALIDf)==0) {
                     sal_free(entry_data);
                     continue ;
                 }
             }
             _bcm_esw_get_flex_counter_fields_values(
                      unit, table_info[index].table , 
                      entry_data,&offset_mode, &pool_number, &base_idx);
             if ((offset_mode == 0) && (base_idx == 0)) {
                  sal_free(entry_data);
                  continue;
             }
             sal_free(entry_data);

             if (table_info[index].direction == bcmStatFlexDirectionIngress) {
                 BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_ingress_object(
                                     table_info[index].table,&object));
                 group=flex_ingress_modes[unit][offset_mode].group_mode;
             } else {
                 BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_egress_object(
                                     table_info[index].table,&object));
                 group=flex_egress_modes[unit][offset_mode].group_mode;
             }
             _bcm_esw_stat_get_counter_id(
                             group,object,
                             offset_mode,pool_number,base_idx,
                             &stat_counter_id[index]);
             (*num_stat_counter_ids)++;
         }
    }
    if ((*num_stat_counter_ids) == 0) {
         return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_get_ingress_object
 * Description:
 *      Get Ingress Object available in give table
 * Parameters:
 *      unit                  - (IN) unit number
 *      pool_number           - (IN) Flex Pool number
 *      num_objects           - (OUT) Number of Flex Object found in given pool
 *      object                - (OUT) Flex Objects found in given pool
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      For debuging purpose only
 *
 */
void _bcm_esw_stat_get_ingress_object(
     int               unit,
     uint32            pool_number,
     uint32            *num_objects,
     bcm_stat_object_t *object)
{
     int index=0;
     uint32            ingress_table_masks
                       [BCM_STAT_FLEX_COUNTER_MAX_INGRESS_TABLE]={
                       FLEX_COUNTER_POOL_USED_BY_PORT_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_VFP_POLICY_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_VLAN_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_VFI_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_VRF_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_MPLS_ENTRY_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_VLAN_XLATE_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_L3_IIF_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_SOURCE_VP_TABLE
     }; 
     bcm_stat_object_t ingress_table_objects
                       [BCM_STAT_FLEX_COUNTER_MAX_INGRESS_TABLE]={
                       bcmStatObjectIngPort,
                       bcmStatObjectIngPolicy,
                       bcmStatObjectIngVlan,
                       bcmStatObjectIngVfi,
                       bcmStatObjectIngVrf,
                       bcmStatObjectIngMplsSwitchLabel,
                       bcmStatObjectIngVlanXlate,
                       bcmStatObjectIngL3Intf,
                       bcmStatObjectIngMplsVcLabel
     };
     *num_objects=0;
     for(index=0;index<BCM_STAT_FLEX_COUNTER_MAX_INGRESS_TABLE;index++) {
         if (flex_pool_stat[unit][bcmStatFlexDirectionIngress][pool_number].
                           used_by_tables & ingress_table_masks[index]) {
             object[*num_objects]=ingress_table_objects[index];
             (*num_objects)++;
         }
     }
}
/*
 * Function:
 *      _bcm_esw_stat_get_egress_object
 * Description:
 *      Get Egress Object available in give table
 * Parameters:
 *      unit                  - (IN) unit number
 *      pool_number           - (IN) Flex Pool number
 *      num_objects           - (OUT) Number of Flex Object found in given pool
 *      object                - (OUT) Flex Objects found in given pool
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      For debuging purpose only
 *
 */
void _bcm_esw_stat_get_egress_object(
     int               unit,
     uint32            pool_number,
     uint32            *num_objects,
     bcm_stat_object_t *object)
{
     int index=0;
     uint32            egress_table_masks
                       [BCM_STAT_FLEX_COUNTER_MAX_EGRESS_TABLE]={
                       FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_EGR_VFI_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_EGR_L3_NEXT_HOP_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_EGR_VLAN_XLATE_TABLE,
                       FLEX_COUNTER_POOL_USED_BY_EGR_PORT_TABLE
     };
     bcm_stat_object_t egress_table_objects
                       [BCM_STAT_FLEX_COUNTER_MAX_EGRESS_TABLE]={
                       bcmStatObjectEgrVlan,
                       bcmStatObjectEgrVfi,
                       bcmStatObjectEgrL3Intf,
                       bcmStatObjectEgrVlanXlate,
                       bcmStatObjectEgrPort
     };
     *num_objects=0;
     for(index=0;index<BCM_STAT_FLEX_COUNTER_MAX_EGRESS_TABLE;index++) {
         if (flex_pool_stat[unit][bcmStatFlexDirectionEgress][pool_number].
                           used_by_tables & egress_table_masks[index]) {
             object[*num_objects]=egress_table_objects[index];
             (*num_objects)++;
         }
     }
}
/*
 * Function:
 *      _bcm_esw_stat_group_dump_info
 * Description:
 *      Dump Useful Info about configured group
 * Parameters:
 *      unit                  - (IN) unit number
 *      all_flag              - (IN) If 1, object and group_mode are ignored
 *      object                - (IN) Flex Accounting object
 *      group                 - (IN) Flex Group Mode
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
void _bcm_esw_stat_group_dump_info(
     int                   unit,
     int                   all_flag,
     bcm_stat_object_t     object,
     bcm_stat_group_mode_t group)
{
    uint32                    num_objects=0;
    bcm_stat_object_t         ing_object_list[
                                 BCM_STAT_FLEX_COUNTER_MAX_INGRESS_TABLE];
    bcm_stat_object_t         egr_object_list[
                                 BCM_STAT_FLEX_COUNTER_MAX_EGRESS_TABLE];
    uint32                    index=0;
    uint32                    stat_counter_id=0;
    uint32                    pool_number=0;
    uint32                    base_index=0;
    bcm_stat_flex_mode_t      offset_mode=0;
    bcm_stat_object_t         object_l=bcmStatObjectIngPort;
    bcm_stat_group_mode_t     group_l= bcmStatGroupModeSingle;
    bcm_stat_flex_direction_t direction=bcmStatFlexDirectionIngress;
    uint32                    total_counters=0;
    uint32                    num_pools[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32                    size_pool[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    bcm_stat_flex_ing_attr_t  *ing_attr=NULL;
    bcm_stat_flex_egr_attr_t  *egr_attr=NULL;
    uint32                    attr_index=0;
    uint32                    total_attrs=0;
    uint8                     attr_value=0;

    if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
        soc_cm_print("Not Available ...\n");
        return;
    }
    soc_cm_print("Not attached(MAX=%d) Stat counter Id info \n",
           BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE);
    for (index=0;index<BCM_STAT_FLEX_COUNTER_MAX_SCACHE_SIZE;index++) {
         if (local_scache_ptr[unit][index] != 0) {
             stat_counter_id = local_scache_ptr[unit][index];
             _bcm_esw_stat_get_counter_id_info(
                           stat_counter_id,&group_l,&object_l,
                           &offset_mode,&pool_number,&base_index);
             if ((object_l >= bcmStatObjectIngPort) &&
                 (object_l <= bcmStatObjectIngMplsSwitchLabel)) {
                  direction = bcmStatFlexDirectionIngress;
                  total_counters = flex_ingress_modes[unit][offset_mode].
                                   total_counters;
             } else {
                  direction = bcmStatFlexDirectionEgress;
                  total_counters = flex_egress_modes[unit][offset_mode].
                                  total_counters;
             }
             if ((all_flag == TRUE) || 
                 ((object_l == object) &&
                  (group_l == group))) {
                   soc_cm_print("\tstat_counter_id = %d=0x%x \n",
                          stat_counter_id,stat_counter_id);
                   soc_cm_print("\t\tDirection:%s mode:%d group_mode:%s"
                          "\n\t\tpool:%d object:%s base:%d"
                          " total_counters=%d\n",
                          flex_directions[direction],offset_mode,
                          flex_group_modes[group_l],
                          pool_number,flex_objects[object_l],
                          base_index,total_counters);
             }
         }
    }
    soc_cm_print("Atached Stat counter Id info \n");
    num_pools[bcmStatFlexDirectionIngress] = SOC_INFO(unit).
                                             num_flex_ingress_pools;
    num_pools[bcmStatFlexDirectionEgress]  = SOC_INFO(unit).
                                             num_flex_egress_pools;
    size_pool[bcmStatFlexDirectionIngress] = SOC_INFO(unit).
                                             size_flex_ingress_pool;
    size_pool[bcmStatFlexDirectionEgress]  = SOC_INFO(unit).
                                             size_flex_egress_pool;

    for (direction=0;
         direction<BCM_STAT_FLEX_COUNTER_MAX_DIRECTION;
         direction++) {
         for (offset_mode=0;
              offset_mode<BCM_STAT_FLEX_COUNTER_MAX_MODE;
              offset_mode++) {
              soc_cm_print("=============================================\n");
              if (direction == bcmStatFlexDirectionIngress) {
                  if (flex_ingress_modes[unit][offset_mode].available == 0) {
                      continue;
                  }
                  total_counters=flex_ingress_modes[unit][offset_mode].
                                                   total_counters;
                  group_l=flex_ingress_modes[unit][offset_mode].group_mode;
                  ing_attr = &flex_ingress_modes[unit][offset_mode].ing_attr;
                  switch(ing_attr->packet_attr_type) {
                  case bcmStatFlexPacketAttrTypeUncompressed:
                       soc_cm_print("IngressPacketAttributMode:Uncompressed\n");
                       soc_cm_print("Attr_bits_selector:%x \n",
                                     ing_attr->uncmprsd_attr_selectors.
                                     uncmprsd_attr_bits_selector);
                       total_attrs = sizeof(ing_uncmprsd_attr_bits_selector)/
                                     sizeof(ing_uncmprsd_attr_bits_selector[0]);
                       for(attr_index=0; attr_index<total_attrs; attr_index++) {
                           if(ing_attr->uncmprsd_attr_selectors.
                              uncmprsd_attr_bits_selector & 
                              ing_uncmprsd_attr_bits_selector[attr_index].
                              attr_bits)
                              soc_cm_print("-->%s bit used\n",
                                  ing_uncmprsd_attr_bits_selector[attr_index].
                                  attr_name);
                       }
                       break;
                  case bcmStatFlexPacketAttrTypeCompressed:
                       soc_cm_print("IngressPacket Attribut Mode:Compressed\n");
                       total_attrs=sizeof(_bcm_esw_get_ing_func)/
                                   sizeof(_bcm_esw_get_ing_func[0]);
                       for(attr_index=0;attr_index<total_attrs;attr_index++) {
                          if((attr_value=_bcm_esw_get_ing_func[attr_index].func(
                             &(ing_attr->cmprsd_attr_selectors.
                               pkt_attr_bits)))){
                             soc_cm_print("-->%s:%d\n",
                                 _bcm_esw_get_ing_func[attr_index].func_desc,
                                 attr_value);
                          }
                       }
                       break;
                  default:
                       soc_cm_print("Not Implemented yet");
                  }
              } else {
                  if (flex_egress_modes[unit][offset_mode].available == 0 ) {
                      continue;
                  }
                  egr_attr = &flex_egress_modes[unit][offset_mode].egr_attr;
                  switch(egr_attr->packet_attr_type) {
                  case bcmStatFlexPacketAttrTypeUncompressed:
                       soc_cm_print("EgressPacketAttributMode:Uncompressed\n");
                       soc_cm_print("Attr_bits_selector:%x \n",
                                     egr_attr->uncmprsd_attr_selectors.
                                     uncmprsd_attr_bits_selector);
                       total_attrs = sizeof(egr_uncmprsd_attr_bits_selector)/
                                     sizeof(egr_uncmprsd_attr_bits_selector[0]);
                       for(attr_index=0; attr_index<total_attrs; attr_index++) {
                           if(egr_attr->uncmprsd_attr_selectors.
                              uncmprsd_attr_bits_selector & 
                              egr_uncmprsd_attr_bits_selector[attr_index].
                              attr_bits)
                              soc_cm_print("-->%s bit used\n",
                                  egr_uncmprsd_attr_bits_selector[attr_index].
                                  attr_name);
                       }
                       break;
                  case bcmStatFlexPacketAttrTypeCompressed:
                       soc_cm_print("EgrressPacket Attribut Mode:Compressed\n");
                       total_attrs=sizeof(_bcm_esw_get_egr_func)/
                                   sizeof(_bcm_esw_get_egr_func[0]);
                       for(attr_index=0;attr_index<total_attrs;attr_index++) {
                          if((attr_value=_bcm_esw_get_egr_func[attr_index].func(
                             &(egr_attr->cmprsd_attr_selectors.
                               pkt_attr_bits)))){
                             soc_cm_print("-->%s:%d\n",
                                 _bcm_esw_get_egr_func[attr_index].func_desc,
                                 attr_value);
                          }
                       }
                       break;
                  default:
                       soc_cm_print("Not Implemented yet");
                  }
                  total_counters=flex_egress_modes[unit][offset_mode].
                                                  total_counters;
                  group_l=flex_egress_modes[unit][offset_mode].group_mode;
              }
              soc_cm_print("-->Direction:%s offset mode=%d\n" 
                     "-->group_mode:%s total_counters=%d \n",
                      flex_directions[direction], offset_mode, 
                      flex_group_modes[group_l], total_counters);
              for (pool_number=0;
                   pool_number<num_pools[direction];
                   pool_number++) {
                   for (base_index=0;
                        base_index<size_pool[direction];
                        base_index++) {
                        if (flex_base_index_reference_count[unit][direction]
                                            [pool_number][base_index] != 0) {
                            if(direction==bcmStatFlexDirectionIngress) {
                               _bcm_esw_stat_get_ingress_object(
                                        unit, 
                                        pool_number, 
                                        &num_objects,
                                        &ing_object_list[0]);
                              for(index=0;index<num_objects;index++) {
                                  object_l=ing_object_list[index];
                                  _bcm_esw_stat_get_counter_id(
                                                group_l,object_l,offset_mode,
                                                pool_number,base_index,
                                                &stat_counter_id);
                                  if (num_objects != 1) {
                                      soc_cm_print("-->Probable..");
                                  }
                                  if ((all_flag == TRUE) || 
                                      ((object_l == object) &&
                                       (group_l == group))) {
                                        soc_cm_print(
                                            "\tstat counter id %d (0x%x) "
                                            "object=%s base index=%d (0x%x)\n",
                                            stat_counter_id,stat_counter_id,
                                            flex_objects[object_l],
                                            base_index,base_index); 
                                  }
                              }
                        } else {
                            _bcm_esw_stat_get_egress_object(
                                     unit, pool_number, 
                                     &num_objects,&egr_object_list[0]);
                            for (index=0; index<num_objects; index++) {
                                 object_l = egr_object_list[index];
                                 _bcm_esw_stat_get_counter_id(
                                                group_l,object_l,offset_mode,
                                                pool_number,base_index,
                                                &stat_counter_id);
                                 if (num_objects != 1) {
                                     soc_cm_print("Probable..");
                                 }
                                 if ((all_flag == TRUE) || 
                                     ((object_l == object) &&
                                      (group_l == group))) {
                                       soc_cm_print("\tstat counter id %d (0x%x) "
                                           "object=%s base index=%d (0x%x)\n",
                                           stat_counter_id,stat_counter_id,
                                           flex_objects[object_l],
                                           base_index,base_index); 
                                 }
                            }
                        }
                   }
                }
              }
              soc_cm_print("=============================================\n");
         }
    }
}
