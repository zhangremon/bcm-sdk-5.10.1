/*
 * $Id: flex_ctr.c 1.1.2.3 Broadcom SDK $
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
 * File:        flex_ctr.c
 * Purpose:     Manage flex counter group creation and deletion
 */

#include <bcm_int/esw/flex_ctr.h>
#include <bcm_int/control.h>
#include <bcm/debug.h>

/* ******************************************************************  */
/*              COMPOSITION OF STAT COUNTER ID                         */
/* ******************************************************************  */
/*              mode_id =  Max 3 bits (Total Eight modes)              */
/*              group_mode_id = Max 5 bits (Total 32)                  */
/*              pool_id = 4 (Max Pool:16)                              */
/*              a/c object_id=4 (Max Object:16)                        */
/*              2 bytes for base index                                 */
/*              000    0-0000    0000   -0000      0000-0000 0000-0000 */
/*              Mode3b Group5b   Pool4b -A/cObj4b  base-index          */
/* ******************************************************************  */
#define BCM_STAT_FLEX_MODE_START_BIT     29
#define BCM_STAT_FLEX_MODE_END_BIT       31
#define BCM_STAT_FLEX_GROUP_START_BIT    24
#define BCM_STAT_FLEX_GROUP_END_BIT      28
#define BCM_STAT_FLEX_POOL_START_BIT     20
#define BCM_STAT_FLEX_POOL_END_BIT       23
#define BCM_STAT_FLEX_OBJECT_START_BIT   16
#define BCM_STAT_FLEX_OBJECT_END_BIT     19
#define BCM_STAT_FLEX_BASE_IDX_START_BIT  0
#define BCM_STAT_FLEX_BASE_IDX_END_BIT   15

#define BCM_STAT_FLEX_MODE_MASK \
    ((1<<(BCM_STAT_FLEX_MODE_END_BIT - BCM_STAT_FLEX_MODE_START_BIT+1))-1)
#define BCM_STAT_FLEX_GROUP_MASK \
    ((1<<(BCM_STAT_FLEX_GROUP_END_BIT - BCM_STAT_FLEX_GROUP_START_BIT+1))-1)
#define BCM_STAT_FLEX_POOL_MASK \
    ((1<<(BCM_STAT_FLEX_POOL_END_BIT - BCM_STAT_FLEX_POOL_START_BIT+1))-1)
#define BCM_STAT_FLEX_OBJECT_MASK \
    ((1<<(BCM_STAT_FLEX_OBJECT_END_BIT - BCM_STAT_FLEX_OBJECT_START_BIT+1))-1)
#define BCM_STAT_FLEX_BASE_IDX_MASK \
    ((1<<(BCM_STAT_FLEX_BASE_IDX_END_BIT-BCM_STAT_FLEX_BASE_IDX_START_BIT+1))-1)

typedef struct _flex_pkt_res_data_s {
    uint32 pkt_res_field;
    uint32 counter_index; 
} _flex_pkt_res_data_t;
static _flex_pkt_res_data_t ing_Single_res[19]={
                 {UNKNOWN_PKT,0},
                 {CONTROL_PKT,0},
                 {BPDU_PKT,0},
                 {L2BC_PKT,0},
                 {L2UC_PKT,0},
                 {L2DLF_PKT,0},
                 {UNKNOWN_IPMC_PKT,0},
                 {KNOWN_IPMC_PKT,0},
                 {KNOWN_L2MC_PKT,0},
                 {UNKNOWN_L2MC_PKT,0},
                 {KNOWN_L3UC_PKT,0},
                 {UNKNOWN_L3UC_PKT,0},
                 {KNOWN_MPLS_PKT,0},
                 {KNOWN_MPLS_L3_PKT,0},
                 {KNOWN_MPLS_L2_PKT,0},
                 {UNKNOWN_MPLS_PKT,0},
                 {KNOWN_MIM_PKT,0},
                 {UNKNOWN_MIM_PKT,0},
                 {KNOWN_MPLS_MULTICAST_PKT,0}
                 };
static _flex_pkt_res_data_t ing_TrafficType_res[6]={
                 {L2BC_PKT,2},
                 {L2UC_PKT,0},
                 {KNOWN_L2MC_PKT,1},
                 {UNKNOWN_L2MC_PKT,1},
                 {KNOWN_L3UC_PKT,0},
                 {UNKNOWN_L3UC_PKT,0}
                 };
static _flex_pkt_res_data_t ing_DlfAll_res[19]={
                 {UNKNOWN_PKT,1},
                 {CONTROL_PKT,1},
                 {BPDU_PKT,1},
                 {L2BC_PKT,1},
                 {L2UC_PKT,1},
                 {L2DLF_PKT,0},
                 {UNKNOWN_IPMC_PKT,1},
                 {KNOWN_IPMC_PKT,1},
                 {KNOWN_L2MC_PKT,1},
                 {UNKNOWN_L2MC_PKT,1},
                 {KNOWN_L3UC_PKT,1},
                 {UNKNOWN_L3UC_PKT,1},
                 {KNOWN_MPLS_PKT,1},
                 {KNOWN_MPLS_L3_PKT,1},
                 {KNOWN_MPLS_L2_PKT,1},
                 {UNKNOWN_MPLS_PKT,1},
                 {KNOWN_MIM_PKT,1},
                 {UNKNOWN_MIM_PKT,1},
                 {KNOWN_MPLS_MULTICAST_PKT,1}
                 };
static _flex_pkt_res_data_t ing_Typed_res[6]={
                 {L2BC_PKT,3},
                 {L2UC_PKT,1},
                 {KNOWN_L2MC_PKT,2},
                 {UNKNOWN_L2MC_PKT,2},
                 {KNOWN_L3UC_PKT,1},
                 {UNKNOWN_L3UC_PKT,0}
                 };
static _flex_pkt_res_data_t ing_TypedAll_res[19]={
                 {UNKNOWN_PKT,4},
                 {CONTROL_PKT,4},
                 {BPDU_PKT,4},
                 {L2BC_PKT,3},
                 {L2UC_PKT,1},
                 {L2DLF_PKT,4},
                 {UNKNOWN_IPMC_PKT,4},
                 {KNOWN_IPMC_PKT,4},
                 {KNOWN_L2MC_PKT,2},
                 {UNKNOWN_L2MC_PKT,2},
                 {KNOWN_L3UC_PKT,1},
                 {UNKNOWN_L3UC_PKT,0},
                 {KNOWN_MPLS_PKT,4},
                 {KNOWN_MPLS_L3_PKT,4},
                 {KNOWN_MPLS_L2_PKT,4},
                 {UNKNOWN_MPLS_PKT,4},
                 {KNOWN_MIM_PKT,4},
                 {UNKNOWN_MIM_PKT,4},
                 {KNOWN_MPLS_MULTICAST_PKT,4}
                 };
static _flex_pkt_res_data_t ing_SingleWithControl_res[19]={
                 {UNKNOWN_PKT,0},
                 {CONTROL_PKT,1},
                 {BPDU_PKT,1},
                 {L2BC_PKT,0},
                 {L2UC_PKT,0},
                 {L2DLF_PKT,0},
                 {UNKNOWN_IPMC_PKT,0},
                 {KNOWN_IPMC_PKT,0},
                 {KNOWN_L2MC_PKT,0},
                 {UNKNOWN_L2MC_PKT,0},
                 {KNOWN_L3UC_PKT,0},
                 {UNKNOWN_L3UC_PKT,0},
                 {KNOWN_MPLS_PKT,0},
                 {KNOWN_MPLS_L3_PKT,0},
                 {KNOWN_MPLS_L2_PKT,0},
                 {UNKNOWN_MPLS_PKT,0},
                 {KNOWN_MIM_PKT,0},
                 {UNKNOWN_MIM_PKT,0},
                 {KNOWN_MPLS_MULTICAST_PKT,0}
                 };
static _flex_pkt_res_data_t ing_TrafficTypeWithControl_res[8]={
                 {CONTROL_PKT,3},
                 {BPDU_PKT,3},
                 {L2BC_PKT,2},
                 {L2UC_PKT,0},
                 {KNOWN_L2MC_PKT,1},
                 {UNKNOWN_L2MC_PKT,1},
                 {KNOWN_L3UC_PKT,0},
                 {UNKNOWN_L3UC_PKT,0}
                 };
static _flex_pkt_res_data_t ing_DlfAllWithControl_res[19]={
                 {UNKNOWN_PKT,2},
                 {CONTROL_PKT,0},
                 {BPDU_PKT,0},
                 {L2BC_PKT,2},
                 {L2UC_PKT,2},
                 {L2DLF_PKT,1},
                 {UNKNOWN_IPMC_PKT,2},
                 {KNOWN_IPMC_PKT,2},
                 {KNOWN_L2MC_PKT,2},
                 {UNKNOWN_L2MC_PKT,2},
                 {KNOWN_L3UC_PKT,2},
                 {UNKNOWN_L3UC_PKT,2},
                 {KNOWN_MPLS_PKT,2},
                 {KNOWN_MPLS_L3_PKT,2},
                 {KNOWN_MPLS_L2_PKT,2},
                 {UNKNOWN_MPLS_PKT,2},
                 {KNOWN_MIM_PKT,2},
                 {UNKNOWN_MIM_PKT,2},
                 {KNOWN_MPLS_MULTICAST_PKT,2}
                 };
static _flex_pkt_res_data_t ing_TypedWithControl_res[8]={
                 {CONTROL_PKT,0},
                 {BPDU_PKT,0},
                 {L2BC_PKT,4},
                 {L2UC_PKT,2},
                 {KNOWN_L2MC_PKT,3},
                 {UNKNOWN_L2MC_PKT,3},
                 {KNOWN_L3UC_PKT,2},
                 {UNKNOWN_L3UC_PKT,1}
                 };
static _flex_pkt_res_data_t ing_TypedAllWithControl_res[19]={
                 {UNKNOWN_PKT,5},
                 {CONTROL_PKT,0},
                 {BPDU_PKT,0},
                 {L2BC_PKT,4},
                 {L2UC_PKT,2},
                 {L2DLF_PKT,5},
                 {UNKNOWN_IPMC_PKT,5},
                 {KNOWN_IPMC_PKT,5},
                 {KNOWN_L2MC_PKT,3},
                 {UNKNOWN_L2MC_PKT,3},
                 {KNOWN_L3UC_PKT,2},
                 {UNKNOWN_L3UC_PKT,1},
                 {KNOWN_MPLS_PKT,5},
                 {KNOWN_MPLS_L3_PKT,5},
                 {KNOWN_MPLS_L2_PKT,5},
                 {UNKNOWN_MPLS_PKT,5},
                 {KNOWN_MIM_PKT,5},
                 {UNKNOWN_MIM_PKT,5},
                 {KNOWN_MPLS_MULTICAST_PKT,5}
                 };
static _flex_pkt_res_data_t egr_Single_res[2]={
                 {0,0}, /*Unicast */
                 {1,0}  /*Multicast */
                 };
static _flex_pkt_res_data_t egr_TrafficType_res[2]={
                 {0,0}, /*Unicast */
                 {1,1}  /*Multicast */
                 };

/*
 * Function:
 *      _bcm_esw_stat_flex_create_mode
 * Description:
 *      Checks attributes direction and calls ingress/egress mode creation 
 *      function
 * Parameters:
 *      unit  - (IN) unit number
 *      attr  - (IN) Flex attributes
 *      mode  - (OUT) Flex mode
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      
 */
static bcm_error_t _bcm_esw_stat_flex_create_mode (
                   int                  unit,
                   bcm_stat_flex_attr_t *attr,
                   bcm_stat_flex_mode_t *mode)
{
    if (attr == NULL) {
        return BCM_E_PARAM;
    }
    if (attr->direction == bcmStatFlexDirectionIngress) {
        return _bcm_esw_stat_flex_create_ingress_mode(
                unit,
                &(attr->ing_attr),
                mode);
    }
    if (attr->direction == bcmStatFlexDirectionEgress) {
        return _bcm_esw_stat_flex_create_egress_mode(
                unit,
                &(attr->egr_attr),
                mode);
    }
    return BCM_E_PARAM;
}
/*
 * Function:
 *      _bcm_esw_fillup_ing_uncmp_attr
 * Description:
 *      Fill up ingress uncompressed flex attributes with required parameters
 *      Inialize offset table map also.
 * Parameters:
 *      ing_attr                     - (IN) Flex Ingress attributes
 *      uncmprsd_attr_bits_selector  - (IN) Uncompressed Bits Selector 
 *      total_counters               - (IN) Total Counters
 * Return Value:
 *      None
 * Notes:
 *      
 */
static void _bcm_esw_fillup_ing_uncmp_attr(
            bcm_stat_flex_ing_attr_t            *ing_attr,
            uint32                              uncmprsd_attr_bits_selector,
            uint8                               total_counters)
{
    uint32 index=0;

    ing_attr->packet_attr_type=bcmStatFlexPacketAttrTypeUncompressed;

    ing_attr->uncmprsd_attr_selectors.uncmprsd_attr_bits_selector = 
                uncmprsd_attr_bits_selector;
    ing_attr->uncmprsd_attr_selectors.total_counters = total_counters;
    /* Reset all Offset table fields */
    for (index=0;index<256;index++) {
         ing_attr->uncmprsd_attr_selectors.offset_table_map[index].offset=0;
         ing_attr->uncmprsd_attr_selectors.offset_table_map[index].
                  count_enable=0;
    }
}
/*
 * Function:
 *      _bcm_esw_fillup_ing_pkt_res_offset_table
 * Description:
        Fills up Ingress Offset table for Packet Resolution fields
 * Parameters:
 *      ing_attr     - (IN) Flex ingress attributes
 *      num_pairs    - (IN) Number of Packet Resolution Data pairs
 *      pkt_res_data - (IN) Packet Resolution Data Pointer
 * Return Value:
 *      None
 * Notes:
 *      
 */
static void _bcm_esw_fillup_ing_pkt_res_offset_table(
            bcm_stat_flex_ing_attr_t *ing_attr,
            uint32                    num_pairs,
            _flex_pkt_res_data_t     *pkt_res_data)
{
    uint32  count=0;
    int     pkt_res_field=0;
    int     counter_index=0;

    /* DROP:1bits(0th) SVP:1bits(1st) PKT_RES:6bits(2nd) bit position */

    for (count=0;count<num_pairs;count++) {
         pkt_res_field=pkt_res_data[count].pkt_res_field;
         counter_index=pkt_res_data[count].counter_index;
         ing_attr->uncmprsd_attr_selectors.offset_table_map[pkt_res_field].
                                           offset= counter_index;
         ing_attr->uncmprsd_attr_selectors.offset_table_map[pkt_res_field].
                                           count_enable=1;
    }
}
/*
 * Function:
 *      _bcm_esw_fillup_egr_uncmp_attr
 * Description:
 *      Fill up egress uncompressed flex attributes with required parameters
 *      Inialize offset table map also.
 * Parameters:
 *      egr_attr                     - (IN) Flex Egress attributes
 *      uncmprsd_attr_bits_selector  - (IN) Uncompressed Bits Selector 
 *      total_counters               - (IN) Total Counters
 * Return Value:
 *      None
 * Notes:
 *      
 */
static void _bcm_esw_fillup_egr_uncmp_attr(
            bcm_stat_flex_egr_attr_t            *egr_attr,
            uint32                              uncmprsd_attr_bits_selector,
            uint8                               total_counters)
{
    uint32 index=0;

    egr_attr->packet_attr_type= bcmStatFlexPacketAttrTypeUncompressed;

    egr_attr->uncmprsd_attr_selectors.uncmprsd_attr_bits_selector = 
                uncmprsd_attr_bits_selector;
    egr_attr->uncmprsd_attr_selectors.total_counters = total_counters;
    /* Reset all Offset table fields */
    for (index=0;index<256;index++) {
         egr_attr->uncmprsd_attr_selectors.offset_table_map[index].offset=0;
         egr_attr->uncmprsd_attr_selectors.offset_table_map[index].
                   count_enable=0;
    }
}
/*
 * Function:
 *      _bcm_esw_fillup_egr_pkt_res_offset_table
 * Description:
        Fills up Egress Offset table for Packet Resolution fields
 * Parameters:
 *      egr_attr     - (IN) Flex egress attributes
 *      num_pairs    - (IN) Number of Packet Resolution Data pairs
 *      pkt_res_data - (IN) Packet Resolution Data Pointer
 * Return Value:
 *      None
 * Notes:
 *      
 */
static void _bcm_esw_fillup_egr_pkt_res_offset_table(
            bcm_stat_flex_egr_attr_t *egr_attr,
            uint32                    num_pairs,
            _flex_pkt_res_data_t     *pkt_res_data)
{
    uint32  count=0;
    int     pkt_res_field=0;
    int     counter_index=0;

    /* DROP:1bits(0th) SVP:1bits(1st) DVP:1bits(2nd) PKT_RES:1bit(3rd) bit */

    for (count=0;count<num_pairs;count++) {
         pkt_res_field=pkt_res_data[count].pkt_res_field;
         counter_index=pkt_res_data[count].counter_index;
         egr_attr->uncmprsd_attr_selectors.offset_table_map[pkt_res_field].
                                           offset= counter_index;
         egr_attr->uncmprsd_attr_selectors.offset_table_map[pkt_res_field].
                                           count_enable=1;
    }
}
/*
 * Function:
 *      _bcm_esw_stat_get_counter_id
 * Description:
 *      Get Stat Counter Id based on offset mode,group mode,pool number,object
 *      and base index. 
 *      
 * Parameters:
 *      group             (IN)  Flex Group Mode
 *      object            (IN)  Flex Accounting Object
 *      mode              (IN)  Flex Offset Mode
 *      pool_number       (IN)  Allocated Pool Number for Flex Accounting Object
 *      base_idx          (IN)  Allocated Base Index for Flex Accounting Object
 *      stat_counter_id   (OUT) Stat Counter Id
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      
 */
void _bcm_esw_stat_get_counter_id(
     bcm_stat_group_mode_t group,
     bcm_stat_object_t     object,
     uint32                mode,
     uint32                pool_number,
     uint32                base_idx,
     uint32                *stat_counter_id)
{
     *stat_counter_id = ((mode & BCM_STAT_FLEX_MODE_MASK)            << 
                                 BCM_STAT_FLEX_MODE_START_BIT)       |
                        ((group & BCM_STAT_FLEX_GROUP_MASK)          << 
                                 BCM_STAT_FLEX_GROUP_START_BIT)      |
                        ((pool_number & BCM_STAT_FLEX_POOL_MASK)     <<
                                 BCM_STAT_FLEX_POOL_START_BIT)       |
                        ((object & BCM_STAT_FLEX_OBJECT_MASK)        << 
                                 BCM_STAT_FLEX_OBJECT_START_BIT)     |
                         (base_idx & BCM_STAT_FLEX_BASE_IDX_MASK);
}
/*
 * Function:
 *      _bcm_esw_stat_get_counter_id_info
 * Description:
 *      Get Stat Counter Id based on offset mode,group mode,pool number,object
 *      and base index. 
 *      
 * Parameters:
 *      stat_counter_id  (IN) Stat Counter Id
 *      group            (OUT)  Flex Group Mode
 *      mode             (OUT)  Flex Accounting Object
 *      offset           (OUT)  Flex Offset Mode
 *      pool_number      (OUT)  Allocated Pool Number for Flex Accounting Object
 *      base_idx         (OUT)  Allocated Base Index for Flex Accounting Object
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      
 */
void _bcm_esw_stat_get_counter_id_info(
     uint32                stat_counter_id,
     bcm_stat_group_mode_t *group,
     bcm_stat_object_t     *object,
     uint32                *mode,
     uint32                *pool_number,
     uint32                *base_idx)
{
     *mode        = (bcm_stat_flex_mode_t ) ((stat_counter_id >> 
                                              BCM_STAT_FLEX_MODE_START_BIT) &
                                             (BCM_STAT_FLEX_MODE_MASK));
     *group       = (bcm_stat_group_mode_t) ((stat_counter_id >> 
                                              BCM_STAT_FLEX_GROUP_START_BIT) &
                                             (BCM_STAT_FLEX_GROUP_MASK));
     *pool_number = ((stat_counter_id >> BCM_STAT_FLEX_POOL_START_BIT) &
                                       (BCM_STAT_FLEX_POOL_MASK));
     *object      = (bcm_stat_object_t) ((stat_counter_id >> 
                                          BCM_STAT_FLEX_OBJECT_START_BIT) &
                                         (BCM_STAT_FLEX_OBJECT_MASK));
     *base_idx    = (stat_counter_id & BCM_STAT_FLEX_BASE_IDX_MASK);
}
/*
 * Function:
 *      _bcm_esw_stat_group_create
 * Description:
 *      Reserve HW counter resources as per given group mode and acounting 
 *      object and make system ready for further stat collection action 
 *      
 * Parameters:
 *    Unit            (IN)  Unit number
 *    object          (IN)  Accounting Object
 *    Group_mode      (IN)  Group Mode
 *    Stat_counter_id (OUT) Stat Counter Id
 *    num_entries     (OUT) Number of Counter entries created 
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      
 */
bcm_error_t _bcm_esw_stat_group_create (
            int	                  unit,
            bcm_stat_object_t     object,
            bcm_stat_group_mode_t group_mode,
            uint32                *stat_counter_id,
            uint32                *num_entries)
{
    bcm_stat_flex_attr_t     *attr=NULL;
    bcm_stat_flex_mode_t     mode=0;
    bcm_error_t              rv=BCM_E_NONE;
    bcm_stat_flex_ing_attr_t *ing_attr=NULL;
    bcm_stat_flex_egr_attr_t *egr_attr=NULL;
    uint32                   map_index=0;
    uint32                   ignore_index=0;
    uint32                   counter_index=0;
    uint32                   outer_index=0;
    uint32                   inner_index=0;
    uint32                   base_index=0;
    uint32                   pool_number=0;
    uint32                   total_counters=0;

    bcm_stat_flex_ing_cmprsd_attr_selectors_t *ing_cmprsd_attr_selectors=NULL;
    bcm_stat_flex_ing_pkt_attr_bits_t *ing_cmprsd_pkt_attr_bits=NULL;

    bcm_stat_flex_egr_cmprsd_attr_selectors_t *egr_cmprsd_attr_selectors=NULL;
    bcm_stat_flex_egr_pkt_attr_bits_t *egr_cmprsd_pkt_attr_bits=NULL;


    if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
         return BCM_E_UNAVAIL;
    }

    /* Validate object id first */
    if (!((object >= bcmStatObjectIngPort) && 
          (object <= bcmStatObjectEgrL3Intf))) {
           FLEXCTR_ERR(("Invalid bcm_stat_object_t passed %d \n",object));
           return BCM_E_PARAM;
    }
    /* Validate group_mode */
    if (!((group_mode >= bcmStatGroupModeSingle) &&
          (group_mode <= bcmStatGroupModeDvpType))) {
           FLEXCTR_ERR(("Invalid bcm_stat_group_mode_t passed %d \n",
                        group_mode));
           return BCM_E_PARAM;
    }

    /* Parameters look OK. ... */

    /* 1. Allocating attribute Memory .... */

    attr = sal_alloc(sizeof(bcm_stat_flex_attr_t),"attr");
    if (attr == NULL) {
        FLEXCTR_ERR(("Failed to allocate memory for bcm_stat_flex_attr_t "));
        return BCM_E_MEMORY;
    }
    sal_memset(attr,0,sizeof(bcm_stat_flex_attr_t));

    /* 2. Deciding direction */
    if ((object >= bcmStatObjectIngPort) && 
        (object <= bcmStatObjectIngMplsSwitchLabel)) {
        /* INGRESS SIDE */
        attr->direction=bcmStatFlexDirectionIngress;
        ing_attr = &(attr->ing_attr);
        ing_cmprsd_attr_selectors=&(ing_attr->cmprsd_attr_selectors);
        ing_cmprsd_pkt_attr_bits= &(ing_attr->cmprsd_attr_selectors.
                                    pkt_attr_bits);
    } else {
        /* EGRESS SIDE */
        attr->direction=bcmStatFlexDirectionEgress;
        egr_attr = &(attr->egr_attr);
        egr_cmprsd_attr_selectors=&(egr_attr->cmprsd_attr_selectors);
        egr_cmprsd_pkt_attr_bits= &(egr_attr->cmprsd_attr_selectors.
                                    pkt_attr_bits);
    } 

    if (attr->direction == bcmStatFlexDirectionEgress) {
        switch(group_mode) {
        case bcmStatGroupModeDlfAll:
        case bcmStatGroupModeSingleWithControl:
             FLEXCTR_VVERB(("Overiding group_mode->bcmStatGroupModeSingle\n"));
             group_mode = bcmStatGroupModeSingle;
             break;
        case bcmStatGroupModeTyped:
        case bcmStatGroupModeTypedAll:
        case bcmStatGroupModeTrafficTypeWithControl:
        case bcmStatGroupModeDlfAllWithControl:
        case bcmStatGroupModeTypedWithControl:
        case bcmStatGroupModeTypedAllWithControl: 
             FLEXCTR_VVERB(("Overiding group_mode to "
                            "bcmStatGroupModeTrafficType \n"));
             group_mode = bcmStatGroupModeTrafficType;
             break;
        case bcmStatGroupModeDlfIntPri: 
        case bcmStatGroupModeDlfIntPriWithControl: 
        case bcmStatGroupModeTypedIntPriWithControl:
             FLEXCTR_VVERB(("Overiding group_mode to "
                            "bcmStatGroupModeTypedIntPri \n"));
             group_mode = bcmStatGroupModeTypedIntPri;
             break;
        default:
            break;
        }
    }

    /* 3. Filling up attributes */
    switch(group_mode) {
    case bcmStatGroupModeSingle:
         /* *********************************************/
         /* A single counter used for all traffic types */
         /* 1) UNKNOWN_PKT|CONTROL_PKT|BPDU_PKT|L2BC_PKT|L2UC_PKT|L2DLF_PKT| */
         /*    UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT|KNOWN_L2MC_PKT|               */
         /*    UNKNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|KNOWN_L3UC_PKT|             */
         /*    UNKNOWN_L3UC_PKT|KNOWN_MPLS_PKT|KNOWN_MPLS_L3_PKT|            */
         /*    KNOWN_MPLS_L2_PKT|UNKNOWN_MPLS_PKT|KNOWN_MIM_PKT|             */
         /*    UNKNOWN_MIM_PKT|KNOWN_MPLS_MULTICAST_PKT                      */

         /* ******************************************* */
         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=1;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 1);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,19,&ing_Single_res[0]);
         } else {
             total_counters=1;
             _bcm_esw_fillup_egr_uncmp_attr(
                 egr_attr,
                 BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 1);
             _bcm_esw_fillup_egr_pkt_res_offset_table(
                 egr_attr,2,
                 &egr_Single_res[0]);
         }
         break;
    case bcmStatGroupModeTrafficType:
         /* **************************************************************** */
         /* A dedicated counter per traffic type Unicast,multicast,broadcast */
         /* 1) L2UC_PKT | KNOWN_L3UC_PKT | UNKNOWN_L3UC_PKT                  */
         /* 2) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|                              */
         /* 3) L2BC_PKT|                                                     */
         /* **************************************************************** */

         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=3;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 3);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,6,&ing_TrafficType_res[0]);
         } else {
             total_counters=2;
             _bcm_esw_fillup_egr_uncmp_attr(
                 egr_attr,
                 BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 2);
             _bcm_esw_fillup_egr_pkt_res_offset_table(
                 egr_attr,2,&egr_TrafficType_res[0]);
         }
         break;
    case bcmStatGroupModeDlfAll:
         /* ************************************************************* */
         /* A pair of counters where the base counter is used for dlf and */ 
         /* the other counter is used for all traffic types               */
         /* 1) L2DLF_PKT                                                  */
         /* 2) UNKNOWN_PKT | CONTROL_PKT|BPDU_PKT|L2BC_PKT|L2UC_PKT|      */
         /*    L2DLF_PKT|UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT|KNOWN_L2MC_PKT|  */
         /*    UNKNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|KNOWN_L3UC_PKT|          */
         /*    UNKNOWN_L3UC_PKT|KNOWN_MPLS_PKT|KNOWN_MPLS_L3_PKT|         */
         /*    KNOWN_MPLS_L2_PKT|UNKNOWN_MPLS_PKT|KNOWN_MIM_PKT|          */
         /*    UNKNOWN_MIM_PKT|KNOWN_MPLS_MULTICAST_PKT                   */
         /* ************************************************************* */

         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=2;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 2);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,19,&ing_DlfAll_res[0]);
         } else {
             /* Group mode is overrided so  control shouldn't hit this part */
             FLEXCTR_ERR(("bcmStatGroupModeDlfAll is not supported"
                          "in egress side\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         break;
    case bcmStatGroupModeDlfIntPri:
         /* ************************************************************** */
         /* N+1 counters where the base counter is used for dlf and next N */
         /* are used per Cos                                               */
         /* 1) L2_DLF                                                      */
         /* 2..17) INT_PRI bits: 4bits                                     */
         /* ************************************************************** */

         if (attr->direction==bcmStatFlexDirectionEgress) {
             /* Must not hit */
             FLEXCTR_ERR(("bcmStatGroupModeDlfIntPri IsNotAvailable"
                            "in EgressSide\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         total_counters=17;
         ing_attr->packet_attr_type=bcmStatFlexPacketAttrTypeCompressed;

         ing_cmprsd_pkt_attr_bits->pkt_resolution = 1;
         ing_cmprsd_pkt_attr_bits->pkt_resolution_mask = 1;
         ing_cmprsd_pkt_attr_bits->int_pri = 4;
         ing_cmprsd_pkt_attr_bits->int_pri_mask = (1<<4)-1;

         ing_cmprsd_attr_selectors->total_counters = 17;

         /* Reset pkt_resolution map */
         for (map_index=0; map_index < 256 ;map_index++) {
              ing_cmprsd_attr_selectors->pkt_res_attr_map[map_index]=0;
         }
         /* set pkt_resolution map for  1 counters */
         for (ignore_index=0; ignore_index < (1<<2) ; ignore_index++) {
              ing_cmprsd_attr_selectors->
                        pkt_res_attr_map[(L2DLF_PKT<<2) | ignore_index]=(1<<2);
         }

         /* Reset pri_cng map */
         for (map_index=0; map_index < 256 ;map_index++) {
              ing_cmprsd_attr_selectors->pri_cnf_attr_map[map_index]=0;
         }
         /* set pri_cng map for  16 counters */
         for (ignore_index=0; ignore_index < (1<<4) ; ignore_index++) {
              for (map_index=0; map_index < 16 ;map_index++) {
                   ing_cmprsd_attr_selectors->
                   pri_cnf_attr_map[(ignore_index<<4) | map_index]=map_index;
              }
         }

         /* Reset all Offset table fields */
         for (counter_index=0;counter_index<256;counter_index++) {
              ing_cmprsd_attr_selectors->
                        offset_table_map[counter_index].offset=0;
              ing_cmprsd_attr_selectors->
                        offset_table_map[counter_index].count_enable=0;
         }
         /* Set DLF counter indexes considering INT_PRI bits don't care */
         for (counter_index=0;counter_index<16;counter_index++) {
              ing_cmprsd_attr_selectors->
                        offset_table_map[(counter_index<<1)|1].offset=0;
              ing_cmprsd_attr_selectors->
                        offset_table_map[(counter_index<<1)|1].count_enable=1;
         }
         /* Set Int pri counter indexes considering DLF=0 */
         for (counter_index=0;counter_index<16;counter_index++) {
              ing_cmprsd_attr_selectors->
                        offset_table_map[(counter_index<<1)].
                        offset=(counter_index+1);
              ing_cmprsd_attr_selectors->
                        offset_table_map[(counter_index<<1)].count_enable=1;
         }
         break;
    case bcmStatGroupModeTyped:
         /* ******************************************************* */
         /* A dedicated counter for unknown unicast, known unicast, */
         /* multicast, broadcast                                    */
         /* 1) UNKNOWN_L3UC_PKT                                     */
         /* 2) L2UC_PKT | KNOWN_L3UC_PKT                            */
         /* 3) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT                      */
         /* 4) L2BC_PKT                                             */
         /* ******************************************************* */
         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=4;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 4);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,6,&ing_Typed_res[0]);
         } else {
             /* Group mode is overrided so  control shouldn't hit this part */
             FLEXCTR_ERR(("bcmStatGroupModeTyped: is not supported"
                          " in egress side\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         break;
    case bcmStatGroupModeTypedAll:
         /* ******************************************************* */
         /* A dedicated counter for unknown unicast, known unicast, */
         /* multicast, broadcast and one for all traffic(not already*/
         /* counted)                                                */
         /* 1) UNKNOWN_L3UC_PKT                                     */
         /* 2) L2UC_PKT | KNOWN_L3UC_PKT                            */
         /* 3) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT                      */
         /* 4) L2BC_PKT                                             */
         /* 5) UNKNOWN_PKT|CONTROL_PKT|BPDU_PKT|L2DLF_PKT|          */
         /*    UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT|KNOWN_MPLS_PKT |     */
         /*    KNOWN_MPLS_L3_PKT|KNOWN_MPLS_L2_PKT|UNKNOWN_MPLS_PKT */
         /*    KNOWN_MIM_PKT|UNKNOWN_MIM_PKT|                       */
         /*    KNOWN_MPLS_MULTICAST_PKT                             */
         /* ******************************************************* */
         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=5;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 5);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,19,&ing_TypedAll_res[0]);
         } else {
             /* Group mode is overrided so  control shouldn't hit this part */
             FLEXCTR_ERR(("bcmStatGroupModeTypedAll is NotSupported"
                           " in EgressSide\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         break;
    case bcmStatGroupModeTypedIntPri:
         /* *************************************************************** */
         /* A dedicated counter for unknown unicast, known unicast,         */
         /* multicast,broadcast and N internal priority counters for traffic*/
         /* (not already counted)                                           */
         /* 1) UNKNOWN_L3UC_PKT                                             */
         /* 2) L2UC_PKT | KNOWN_L3UC_PKT                                    */ 
         /* 3) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT                              */
         /* 4) L2BC_PKT                                                     */
         /* 5..20) INT_PRI bits: 4bits                                      */
         /* *************************************************************** */
         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=20;

             ing_attr->packet_attr_type=bcmStatFlexPacketAttrTypeCompressed;
             /* Cannot consider 0 value so taking 3 i.s.o. 2 */
             ing_cmprsd_pkt_attr_bits->pkt_resolution = 3;
             ing_cmprsd_pkt_attr_bits->pkt_resolution_mask = (1<<3)-1;
             ing_cmprsd_pkt_attr_bits->int_pri = 4;
             ing_cmprsd_pkt_attr_bits->int_pri_mask=(1<<4)-1;

             ing_cmprsd_attr_selectors->total_counters = 20;
             /* Reset pkt_resolution map */
             for (map_index=0; map_index < 256 ;map_index++) {
                  ing_cmprsd_attr_selectors->pkt_res_attr_map[map_index]=0;
             }       
             /* set pkt_resolution map for  1 counters.Ignore SVP,DROP bits */
             for (ignore_index=0; ignore_index < (1<<2) ; ignore_index++) {
                  ing_cmprsd_attr_selectors->
                    pkt_res_attr_map[(UNKNOWN_L3UC_PKT<<2)|ignore_index]=(1<<2);
                  ing_cmprsd_attr_selectors->
                    pkt_res_attr_map[(L2UC_PKT<<2)|ignore_index]=(2<<2);
                  ing_cmprsd_attr_selectors->
                    pkt_res_attr_map[(KNOWN_L3UC_PKT<<2)|ignore_index]=(2<<2);
                  ing_cmprsd_attr_selectors->
                    pkt_res_attr_map[(KNOWN_L2MC_PKT<<2)|ignore_index]=(3<<2);
                  ing_cmprsd_attr_selectors->
                    pkt_res_attr_map[(UNKNOWN_L2MC_PKT<<2)|ignore_index]=(3<<2);
                  ing_cmprsd_attr_selectors->
                    pkt_res_attr_map[(L2BC_PKT<<2)|ignore_index]=(4<<2);
             }
             /* Reset pri_cng map */
             for (map_index=0; map_index < 256 ;map_index++) {
                  ing_cmprsd_attr_selectors->pri_cnf_attr_map[map_index]=0;
             }       
             /* set pri_cng map for  16 counters */
             for (ignore_index=0; ignore_index < (1<<4) ; ignore_index++) {
                  for (map_index=0; map_index < 16 ;map_index++) {
                       ing_cmprsd_attr_selectors->
                        pri_cnf_attr_map[(ignore_index<<4)|map_index]=map_index;
                  }       
             }
             /* Reset all Offset table fields */
             for (counter_index=0;counter_index<256;counter_index++) {
                  ing_cmprsd_attr_selectors->
                           offset_table_map[counter_index].offset=0;
                  ing_cmprsd_attr_selectors->
                           offset_table_map[counter_index].count_enable=0;
             }
             /* ************************************************************* */
             /* Set unicast, known unicast, multicast, broadcast counter      */
             /* indexes considering INT_PRI as 0                              */
             /* ************************************************************* */
             ing_cmprsd_attr_selectors->offset_table_map[1].offset=0;
             ing_cmprsd_attr_selectors->offset_table_map[1].count_enable=1;
             ing_cmprsd_attr_selectors->offset_table_map[2].offset=1;
             ing_cmprsd_attr_selectors->offset_table_map[2].count_enable=1;
             ing_cmprsd_attr_selectors->offset_table_map[3].offset=2;
             ing_cmprsd_attr_selectors->offset_table_map[3].count_enable=1;
             ing_cmprsd_attr_selectors->offset_table_map[4].offset=3;
             ing_cmprsd_attr_selectors->offset_table_map[4].count_enable=1;


             /* Set Int pri counter indexes NotConsidering pkt resolution bits*/
             /* Priority 0 Counter not satisfying any condition */
             ing_cmprsd_attr_selectors->offset_table_map[0].offset=4;
             ing_cmprsd_attr_selectors->offset_table_map[0].count_enable=1;
             ing_cmprsd_attr_selectors->offset_table_map[5].offset=4;
             ing_cmprsd_attr_selectors->offset_table_map[5].count_enable=1;
             ing_cmprsd_attr_selectors->offset_table_map[6].offset=4;
             ing_cmprsd_attr_selectors->offset_table_map[6].count_enable=1;
             ing_cmprsd_attr_selectors->offset_table_map[7].offset=4;
             ing_cmprsd_attr_selectors->offset_table_map[7].count_enable=1;
             /*INT_PRI*/
             for (counter_index=1;counter_index<(1<<4);counter_index++) {
                  /*PktRes*/
                  for(ignore_index=0;ignore_index<(1<<3);ignore_index++) {
                      ing_cmprsd_attr_selectors->
                       offset_table_map[(counter_index<<3)|ignore_index].
                       offset=(counter_index+4);
                      ing_cmprsd_attr_selectors->
                       offset_table_map[(counter_index<<3)|ignore_index].
                       count_enable=1;
                  }
             }
         } else {
             /* ************************************************************* */
             /* A dedicated counter for unknown unicast, known unicast,       */
             /* multicast,broadcast and N internal priority counters for      */ 
             /* traffic (not already counted)                                 */
             /* 1) Unicast                                                    */
             /* 2) Multicast                                                  */
             /* 3..18) INT_PRI bits: 4bits                                    */
             /* ************************************************************* */
            total_counters=18;
            egr_attr->packet_attr_type=bcmStatFlexPacketAttrTypeCompressed;

            egr_cmprsd_pkt_attr_bits->pkt_resolution = 1;
            egr_cmprsd_pkt_attr_bits->pkt_resolution_mask = (1<<1)-1;

            egr_cmprsd_pkt_attr_bits->int_pri = 4;
            egr_cmprsd_pkt_attr_bits->int_pri_mask=(1<<4)-1;
            egr_cmprsd_attr_selectors->total_counters = 18;
            /* Set pkt_resolution map */
            /* Unicast */
            for (map_index=0; map_index < 8 ;map_index++) {
                 egr_cmprsd_attr_selectors->pkt_res_attr_map[map_index]=0;
            }       
            /* Multicast */
            for (; map_index < 16 ;map_index++) {
                 egr_cmprsd_attr_selectors->
                           pkt_res_attr_map[map_index]=(1<<3);
            }       

            /* Reset pri_cng map */
            for (map_index=0; map_index < 64 ;map_index++) {
                 egr_cmprsd_attr_selectors->pri_cnf_attr_map[map_index]=0;
            }        
            /* set pri_cng map for  16 counters */
            for (ignore_index=0; ignore_index < (1<<2) ; ignore_index++) {
                 for (map_index=0; map_index < 16 ;map_index++) {
                      egr_cmprsd_attr_selectors->
                        pri_cnf_attr_map[(ignore_index<<4)|map_index]=map_index;
                 }       
            }
            /* Reset all Offset table fields */
            for(counter_index=0;counter_index<256;counter_index++) {
                egr_cmprsd_attr_selectors->
                          offset_table_map[counter_index].offset=0;
                egr_cmprsd_attr_selectors->
                          offset_table_map[counter_index].count_enable=0;
            }
            /* ************************************************************** */
            /* Set unicast, multicast, counter indexes                        */
            /* considering INT_PRI bits zero                                  */
            /* ************************************************************** */
            egr_cmprsd_attr_selectors->
                      offset_table_map[0].offset=0;
            egr_cmprsd_attr_selectors->
                      offset_table_map[0].count_enable=1;
            egr_cmprsd_attr_selectors->
                      offset_table_map[1].offset=1;
            egr_cmprsd_attr_selectors->
                      offset_table_map[1].count_enable=1;
            /* PRI-0 counters will be addition of unicast & multicast packets!*/
            /* Set Int pri counter indexes ignoring pkt_res bits           */
            /*INT_PRI*/
            for (counter_index=1;counter_index<(1<<4);counter_index++) {
                 /*PktRes*/
                 for (ignore_index=0;ignore_index<(1<<1);ignore_index++) {
                      egr_cmprsd_attr_selectors->
                       offset_table_map[(counter_index<<1)|ignore_index].
                       offset=(counter_index+2);
                      egr_cmprsd_attr_selectors->
                       offset_table_map[(counter_index<<1)|ignore_index].
                       count_enable=1;
                 }
            }
         }
         break;
    case bcmStatGroupModeSingleWithControl:
         /* **************************************************************   */
         /* A single counter used for all traffic types with an additional   */ 
         /* counter for control traffic                                      */
         /* 1) UNKNOWN_PKT|                    |L2BC_PKT|L2UC_PKT|L2DLF_PKT| */
         /*    UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT|KNOWN_L2MC_PKT|               */
         /*    UNKNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|KNOWN_L3UC_PKT|             */
         /*    UNKNOWN_L3UC_PKT|KNOWN_MPLS_PKT|KNOWN_MPLS_L3_PKT|            */
         /*    KNOWN_MPLS_L2_PKT|UNKNOWN_MPLS_PKT|KNOWN_MIM_PKT|             */
         /*    UNKNOWN_MIM_PKT|KNOWN_MPLS_MULTICAST_PKT                      */
         /* 2) CONTROL_PKT|BPDU_PKT                                          */
         /* **************************************************************   */
         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=2;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 2);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,19,&ing_SingleWithControl_res[0]);
         } else {
             /* Group mode is overrided so  control shouldn't hit this part */
             FLEXCTR_ERR(("bcmStatGroupModeSingleWithControl"
                          " is not supported in egress side\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         break;
    case bcmStatGroupModeTrafficTypeWithControl:
         /* ********************************************************  */
         /* A dedicated counter per traffic type unicast, multicast,  */
         /* broadcast with an additional counter for control traffic  */
         /* 1) L2UC_PKT | KNOWN_L3UC_PKT | UNKNOWN_L3UC_PKT           */
         /* 2) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|                       */
         /* 3) L2BC_PKT|                                              */
         /* 4) CONTROL_PKT|BPDU_PKT                                   */
         /* ********************************************************  */
         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=4;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 4);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,8,&ing_TrafficTypeWithControl_res[0]);
         } else {
             /* Group mode is overrided so  control shouldn't hit this part */
             FLEXCTR_ERR(("bcmStatGroupModeTrafficTypeWithControl"
                          "is not supported in egress side\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         break;
    case bcmStatGroupModeDlfAllWithControl:
         /* ************************************************************** */
         /* A pair of counters where the base counter is used for control, */
         /* the next one for dlf and the other counter is used for all     */
         /* traffic types                                                  */
         /* 1) CONTROL_PKT|BPDU_PKT                                        */
         /* 2) L2DLF_PKT                                                   */
         /* 3)UNKNOWN_PKT | CONTROL_PKT|BPDU_PKT|L2BC_PKT|L2UC_PKT|        */
         /*   L2DLF_PKT|UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT|KNOWN_L2MC_PKT|    */
         /*   UNKNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|KNOWN_L3UC_PKT|            */
         /*   UNKNOWN_L3UC_PKT|KNOWN_MPLS_PKT|KNOWN_MPLS_L3_PKT|           */
         /*   KNOWN_MPLS_L2_PKT|UNKNOWN_MPLS_PKT|KNOWN_MIM_PKT|            */
         /*   UNKNOWN_MIM_PKT|KNOWN_MPLS_MULTICAST_PKT                     */
         /* ************************************************************** */
         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=3;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 3);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,19,&ing_DlfAllWithControl_res[0]);
         } else {
             /* Group mode is overrided so  control shouldn't hit this part */
             FLEXCTR_ERR(("bcmStatGroupModeDlfAllWithControl is not supported "
                          "in egress side\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         break;
    case bcmStatGroupModeDlfIntPriWithControl:
         /* ************************************************************* */
         /* N+2 counters where the base counter is used for control, the  */
         /* next one for dlf and next N are used per Cos                  */
         /* 1) CONTROL_PKT|BPDU_PKT                                       */
         /* 2) L2_DLF                                                     */
         /* 3..18) INT_PRI bits: 4bits                                    */
         /* ************************************************************* */
         if (attr->direction==bcmStatFlexDirectionEgress) {
             FLEXCTR_ERR(("GroupModeDlfIntPriWithControl is not available in "
                          "egress side\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         total_counters=18;
         ing_attr->packet_attr_type=bcmStatFlexPacketAttrTypeCompressed;

         ing_cmprsd_pkt_attr_bits->pkt_resolution = 2;
         ing_cmprsd_pkt_attr_bits->pkt_resolution_mask = (1<<2)-1;
         ing_cmprsd_pkt_attr_bits->int_pri = 4;
         ing_cmprsd_pkt_attr_bits->int_pri_mask = (1<<4)-1;
         ing_cmprsd_attr_selectors->total_counters = 17;

         /* Reset pkt_resolution map */
         for (map_index=0; map_index < 256 ;map_index++) {
              ing_cmprsd_attr_selectors->pkt_res_attr_map[map_index]=0;
         }
         /* set pkt_resolution map for  1 counters */
         for (ignore_index=0; ignore_index < (1<<2) ; ignore_index++) {
              ing_cmprsd_attr_selectors->
                   pkt_res_attr_map[(CONTROL_PKT<<2) | ignore_index]=(1<<2);
              ing_cmprsd_attr_selectors->
                   pkt_res_attr_map[(BPDU_PKT<<2) | ignore_index]=(1<<2);
              ing_cmprsd_attr_selectors->
                   pkt_res_attr_map[(L2DLF_PKT<<2) | ignore_index]=(2<<2);
         }

         /* Reset pri_cng map */
         for (map_index=0; map_index < 256 ;map_index++) {
              ing_cmprsd_attr_selectors->pri_cnf_attr_map[map_index]=0;
         }
         /* set pri_cng map for  16 counters */
         for (ignore_index=0; ignore_index < (1<<4) ; ignore_index++) {
              for (map_index=0; map_index < 16 ;map_index++) {
                   ing_cmprsd_attr_selectors->
                     pri_cnf_attr_map[(ignore_index<<4) | map_index]=map_index;
              }
         }

         /* Reset all Offset table fields */
         for(counter_index=0;counter_index<256;counter_index++) {
                 ing_cmprsd_attr_selectors->
                           offset_table_map[counter_index].offset=0;
                 ing_cmprsd_attr_selectors->
                           offset_table_map[counter_index].count_enable=0;
         }
         /* ************************************************************ */
         /* Set CONTROL_PKT|BPDU_PKT , L2DLF counter indexes considering */
         /* INT_PRI bits 0                                               */
         /* ************************************************************ */

         ing_cmprsd_attr_selectors->offset_table_map[1].offset=0;
         ing_cmprsd_attr_selectors->offset_table_map[1].count_enable=1;
         ing_cmprsd_attr_selectors->offset_table_map[2].offset=1;
         ing_cmprsd_attr_selectors->offset_table_map[2].count_enable=1;

         /* Set IntPri counter indexes not-considering pkt resolution bits*/
         /* Priority 0 Counter not satisfying any condition  */
         ing_cmprsd_attr_selectors->offset_table_map[0].offset=2;
         ing_cmprsd_attr_selectors->offset_table_map[0].count_enable=1;
         ing_cmprsd_attr_selectors->offset_table_map[3].offset=2;
         ing_cmprsd_attr_selectors->offset_table_map[3].count_enable=1;
         /*INT_PRI*/
         for (counter_index=1;counter_index<(1<<4);counter_index++) {
              /*PktRes*/
              for (ignore_index=0;ignore_index<(1<<2);ignore_index++) {
                   ing_cmprsd_attr_selectors->
                       offset_table_map[(counter_index<<2)|ignore_index].
                       offset=(counter_index+2);
                   ing_cmprsd_attr_selectors->
                       offset_table_map[(counter_index<<2)|ignore_index].
                       count_enable=1;
              }
         }
         break;
    case bcmStatGroupModeTypedWithControl:
         /* **************************************************************** */
         /* A dedicated counter for control, unknown unicast, known unicast, */
         /* multicast, broadcast                                             */
         /* 1) CONTROL_PKT|BPDU_PKT                                          */
         /* 2) UNKNOWN_L3UC_PKT                                              */
         /* 3) L2UC_PKT | KNOWN_L3UC_PKT                                     */
         /* 4) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT                               */
         /* 5) L2BC_PKT                                                      */
         /* **************************************************************** */
         if (attr->direction == bcmStatFlexDirectionIngress) {
             total_counters=5;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 5);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,8,&ing_TypedWithControl_res[0]);
         } else {
             /* Group mode is overrided so  control shouldn't hit this part */
             FLEXCTR_ERR(("bcmStatGroupModeTypedWithControl" 
                          "is not supported in egress side\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         break;
    case bcmStatGroupModeTypedAllWithControl:
         /* ***************************************************************** */
         /* A dedicated counter for control, unknown unicast, known unicast,  */
         /* multicast, broadcast and one for all traffic (not already counted)*/
         /* 1) CONTROL_PKT|BPDU_PKT                                           */
         /* 2) UNKNOWN_L3UC_PKT                                               */
         /* 3) L2UC_PKT | KNOWN_L3UC_PKT                                      */
         /* 4) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT                                */
         /* 5) L2BC_PKT                                                       */
         /* 6) UNKNOWN_PKT|L2DLF_PKT|UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT          */
         /*    KNOWN_MPLS_PKT | KNOWN_MPLS_L3_PKT|KNOWN_MPLS_L2_PKT|          */
         /*    UNKNOWN_MPLS_PKT|KNOWN_MIM_PKT|UNKNOWN_MIM_PKT|                */
         /*    KNOWN_MPLS_MULTICAST_PKT                                       */
         /* ***************************************************************** */
         if (attr->direction == bcmStatFlexDirectionIngress) {
             total_counters=6;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS,
                 6);
             _bcm_esw_fillup_ing_pkt_res_offset_table(
                 ing_attr,19,&ing_TypedAllWithControl_res[0]);
         } else {
             /* Group mode is overrided so  control shouldn't hit this part */
             FLEXCTR_ERR(("bcmStatGroupModeTypedAllWithControl"
                          " is not supported in egress side\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         break;
    case bcmStatGroupModeTypedIntPriWithControl:
         /* *************************************************************** */
         /* A dedicated counter for control, unknown unicast, known unicast */
         /* , multicast, broadcast and N internal priority counters for     */
         /* traffic (not already counted)                                   */
         /* 1) CONTROL_PKT|BPDU_PKT                                         */
         /* 2) UNKNOWN_L3UC_PKT                                             */
         /* 3) L2UC_PKT | KNOWN_L3UC_PKT                                    */
         /* 4) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT                              */
         /* 5) L2BC_PKT                                                     */
         /* 6..21) INT_PRI bits: 4bits                                      */
         /* *************************************************************** */
         if (attr->direction == bcmStatFlexDirectionEgress) {
             FLEXCTR_ERR(("GroupModeTypedIntPriWithControl is not available in "
                          "egress side\n"));
             sal_free(attr);
             return BCM_E_INTERNAL;
         }
         total_counters=21;
         ing_attr->packet_attr_type=bcmStatFlexPacketAttrTypeCompressed;

         ing_cmprsd_pkt_attr_bits->pkt_resolution = 3;
         ing_cmprsd_pkt_attr_bits->pkt_resolution_mask = (1<<3)-1;
         ing_cmprsd_pkt_attr_bits->int_pri = 4;
         ing_cmprsd_pkt_attr_bits->int_pri_mask = (1<<4)-1;
         ing_cmprsd_attr_selectors->total_counters = 21;
            
         /* Reset pkt_resolution map */
         for (map_index=0; map_index < 256 ;map_index++) {
              ing_cmprsd_attr_selectors->pkt_res_attr_map[map_index]=0;
         }       
         /* set pkt_resolution map for  1 counters */
         for (ignore_index=0; ignore_index < (1<<2) ; ignore_index++) {
              ing_cmprsd_attr_selectors->
                 pkt_res_attr_map[(CONTROL_PKT<<2) | ignore_index]=(1<<2);
              ing_cmprsd_attr_selectors->
                 pkt_res_attr_map[(BPDU_PKT<<2) | ignore_index]=(1<<2);
              ing_cmprsd_attr_selectors->
                 pkt_res_attr_map[(UNKNOWN_L3UC_PKT<<2) | ignore_index]=(2<<2);
              ing_cmprsd_attr_selectors->
                 pkt_res_attr_map[(L2UC_PKT<<2) | ignore_index]=(3<<2);
              ing_cmprsd_attr_selectors->
                 pkt_res_attr_map[(KNOWN_L3UC_PKT<<2) | ignore_index]=(3<<2);
              ing_cmprsd_attr_selectors->
                 pkt_res_attr_map[(KNOWN_L2MC_PKT<<2) | ignore_index]=(4<<2);
              ing_cmprsd_attr_selectors->
                 pkt_res_attr_map[(UNKNOWN_L2MC_PKT<<2) | ignore_index]=(4<<2);
              ing_cmprsd_attr_selectors->
                 pkt_res_attr_map[(L2BC_PKT<<2) | ignore_index]=(5<<2);
         }
         /* Reset pri_cng map */
         for (map_index=0; map_index < 256 ;map_index++) {
              ing_cmprsd_attr_selectors->pri_cnf_attr_map[map_index]=0;
         }       
         /* set pri_cng map for  16 counters */
         for (ignore_index=0; ignore_index < (1<<4) ; ignore_index++) {
              for (map_index=0; map_index < 16 ;map_index++) {
                   ing_cmprsd_attr_selectors->
                      pri_cnf_attr_map[(ignore_index<<4) | map_index]=map_index;
              }       
         }
                 
         /* Reset all Offset table fields */
         for (counter_index=0;counter_index<256;counter_index++) {
              ing_cmprsd_attr_selectors->
                        offset_table_map[counter_index].offset=0;
              ing_cmprsd_attr_selectors->
                        offset_table_map[counter_index].count_enable=0;
         }
         /* **************************************************************** */
         /* Set unicast, known unicast, multicast, broadcast counter indexes */
         /* considering INT_PRI bits 0                                       */
         /* **************************************************************** */
         ing_cmprsd_attr_selectors->offset_table_map[1].offset=0;
         ing_cmprsd_attr_selectors->offset_table_map[1].count_enable=1;
         ing_cmprsd_attr_selectors->offset_table_map[2].offset=1;
         ing_cmprsd_attr_selectors->offset_table_map[2].count_enable=1;
         ing_cmprsd_attr_selectors->offset_table_map[3].offset=2;
         ing_cmprsd_attr_selectors->offset_table_map[3].count_enable=1;
         ing_cmprsd_attr_selectors->offset_table_map[4].offset=3;
         ing_cmprsd_attr_selectors->offset_table_map[4].count_enable=1;
         ing_cmprsd_attr_selectors->offset_table_map[5].offset=4;
         ing_cmprsd_attr_selectors->offset_table_map[5].count_enable=1;

         /* Set IntPri counter indexes not-considering pkt resolution bits*/
         /* Priority 0 Counter not satisfying any condition  */
         ing_cmprsd_attr_selectors->offset_table_map[0].offset=5;
         ing_cmprsd_attr_selectors->offset_table_map[0].count_enable=1;
         ing_cmprsd_attr_selectors->offset_table_map[6].offset=5;
         ing_cmprsd_attr_selectors->offset_table_map[6].count_enable=1;
         ing_cmprsd_attr_selectors->offset_table_map[7].offset=5;
         ing_cmprsd_attr_selectors->offset_table_map[7].count_enable=1;

         /*INT_PRI*/
         for (counter_index=1;counter_index<(1<<4);counter_index++) {
              /*PktRes*/
              for (ignore_index=0;ignore_index<(1<<3);ignore_index++) {
                   ing_cmprsd_attr_selectors->
                       offset_table_map[(counter_index<<3)|ignore_index].
                       offset=(counter_index+5);
                   ing_cmprsd_attr_selectors->
                       offset_table_map[(counter_index<<3)|ignore_index].
                       count_enable=1;
              }
         }
         break;
    case bcmStatGroupModeDot1P:
         /* ******************************************************** */
         /* A set of 8(2^3) counters selected based on Vlan priority */
         /* outer_dot1p; 3 bits 1..8                                 */
         /* ******************************************************** */

         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=8;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS,
                 8);
             /* Set Offset table fields */
             for (counter_index=0;counter_index<8;counter_index++) {
                  ing_attr->uncmprsd_attr_selectors.
                   offset_table_map[counter_index].offset=counter_index;
                  ing_attr->uncmprsd_attr_selectors.
                   offset_table_map[counter_index].count_enable=1;
             }
             /* Set Offset table fields */
             for(counter_index=0;counter_index<256;counter_index++) {
                 ing_attr->uncmprsd_attr_selectors.
                   offset_table_map[counter_index].offset=counter_index;
                 ing_attr->uncmprsd_attr_selectors.
                   offset_table_map[counter_index].count_enable=1;
             }
         } else {
             total_counters=8;
             _bcm_esw_fillup_egr_uncmp_attr(
                 egr_attr,
                 BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS,
                 8);
             /* Set Offset table fields */
             for (counter_index=0;counter_index<8;counter_index++) {
                  egr_attr->uncmprsd_attr_selectors.
                   offset_table_map[counter_index].offset=counter_index;
                  egr_attr->uncmprsd_attr_selectors.
                   offset_table_map[counter_index].count_enable=1;
             }
         }
         break;
    case bcmStatGroupModeIntPri:
         /* **************************************************** */
         /* A set of 16(2^4) counters based on internal priority */
         /* 1..16 INT_PRI bits: 4bits                            */
         /* **************************************************** */

         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=16;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS,
                 16);
             /* Set Offset table fields */
             for (counter_index=0;counter_index<16;counter_index++) {
                  ing_attr->uncmprsd_attr_selectors.
                          offset_table_map[counter_index].offset=counter_index;
                  ing_attr->uncmprsd_attr_selectors.
                          offset_table_map[counter_index].count_enable=1;
             }
         } else {
             total_counters=16;
             _bcm_esw_fillup_egr_uncmp_attr(
                 egr_attr,
                 BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS,
                 16);
             /* Set Offset table fields */
             for (counter_index=0;counter_index<16;counter_index++) {
                  egr_attr->uncmprsd_attr_selectors.
                          offset_table_map[counter_index].offset=counter_index;
                  egr_attr->uncmprsd_attr_selectors.
                          offset_table_map[counter_index].count_enable=1;
             }
         }
         break;
    case bcmStatGroupModeIntPriCng:
         /* ********************************************************** */
         /* set of 64 counters(2^(4+2)) based on Internal priority+CNG */
         /* 1..64 (INT_PRI bits: 4bits + CNG 2 bits                    */
         /* 1..64 (INT_PRI bits: 4bits + CNG 2 bits                    */
         /* ********************************************************** */
         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=64;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_CNG_ATTR_BITS|
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS,
                 64);
             /* Set Offset table fields */
             counter_index=0;
             for (outer_index=0;outer_index<4;outer_index++) {/*CNG*/
                  for (inner_index=0;inner_index<16;inner_index++) {/*IntPri*/
                       ing_attr->uncmprsd_attr_selectors.
                       offset_table_map[(outer_index<<6)|inner_index].
                       offset=counter_index++;
                       ing_attr->uncmprsd_attr_selectors.
                          offset_table_map[counter_index].count_enable=1;
                  }
             }
         } else {
             total_counters=64;
             _bcm_esw_fillup_egr_uncmp_attr(
                 egr_attr,
                 BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_CNG_ATTR_BITS|
                 BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS,
                 64);
             /* Set Offset table fields */
             counter_index=0;
             for (outer_index=0;outer_index<4;outer_index++) {/*CNG*/
                  for(inner_index=0;inner_index<16;inner_index++) {/*IntPri*/
                      egr_attr->uncmprsd_attr_selectors.
                         offset_table_map[(outer_index<<4)|inner_index].
                         offset=counter_index++;
                      egr_attr->uncmprsd_attr_selectors.
                         offset_table_map[counter_index].count_enable=1;
                  }
             }
         }
         break;
    case bcmStatGroupModeSvpType:
         /* ****************************************** */
         /* A set of 2 counters(2^1) based on SVP type */
         /* 1..2 (SVP 1 bit)                           */
         /* ****************************************** */

         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=2;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS,
                 2);
             /* Set Offset table fields */
             for (counter_index=0;counter_index<2;counter_index++) {
                  ing_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<1].offset=counter_index;
                  ing_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<1].count_enable=1;
             }
         } else {
             total_counters=2;
             _bcm_esw_fillup_egr_uncmp_attr(
                 egr_attr,
                 BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS,
                 2);
             /* Set Offset table fields */
             for (counter_index=0;counter_index<2;counter_index++) {
                  egr_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<2].offset=counter_index;
                  egr_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<2].count_enable=0;
             }
         }
         break;
    case bcmStatGroupModeDscp:
         /* ******************************************** */
         /* A set of 64 counters(2^6) based on DSCP bits */
         /* 1..64 (6 bits from TOS 8 bits)               */
         /* ******************************************** */

         if (attr->direction==bcmStatFlexDirectionIngress) {
             total_counters=64;
             _bcm_esw_fillup_ing_uncmp_attr(
                 ing_attr,
                 BCM_STAT_FLEX_ING_UNCOMPRESSED_USE_TOS_ATTR_BITS,
                 64);
             /* Set Offset table fields */
             for (counter_index=0;counter_index<64;counter_index++) {
                  ing_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<2].offset=counter_index;
                  ing_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<2].count_enable=1;
             }
         } else {
             total_counters=64;
             _bcm_esw_fillup_egr_uncmp_attr(
                 egr_attr,
                 BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_TOS_ATTR_BITS,
                 64);
             /* Set Offset table fields */
             for (counter_index=0;counter_index<2;counter_index++) {
                  egr_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<2].offset=counter_index;
                  egr_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<2].count_enable=0;
             }
         }
         break;
    case bcmStatGroupModeDvpType:
         /* ******************************************** */
         /* EGRESS SIDE ONLY:                            */
         /* A set of 2 counters(2^1) based on DVP type   */
         /* 1..2 (DVP 1 bits)                            */
         /* ******************************************** */

         if (attr->direction==bcmStatFlexDirectionIngress) {
             FLEXCTR_ERR(("bcm_stat_group_mode_t %d is NotSupported"
                           " in IngressSide\n", group_mode));
             sal_free(attr);
             return BCM_E_PARAM;
         } else {
             total_counters=2;
             _bcm_esw_fillup_egr_uncmp_attr(
                 egr_attr,
                 BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DVP_TYPE_ATTR_BITS,
                 2);
             /* Set Offset table fields */
             for (counter_index=0;counter_index<2;counter_index++) {
                  egr_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<1].offset=counter_index;
                  egr_attr->uncmprsd_attr_selectors.
                        offset_table_map[counter_index<<1].count_enable=0;
             }
         }
         break;
    };
    rv =  _bcm_esw_stat_flex_create_mode(unit,attr,&mode);
    if ((rv == BCM_E_NONE) || (rv==BCM_E_EXISTS)) {
         BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_set_group_mode(
                             unit,attr->direction, mode,group_mode));
         rv = BCM_E_NONE;
         switch(object) {
         /* Ingress Side */
         case bcmStatObjectIngPort:
              rv = _bcm_esw_stat_flex_create_ingress_table_counters(
                        unit, PORT_TABm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectIngVlan:
              rv = _bcm_esw_stat_flex_create_ingress_table_counters(
                        unit, VLAN_TABm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectIngVlanXlate:
              rv = _bcm_esw_stat_flex_create_ingress_table_counters(
                        unit, VLAN_XLATEm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectIngVfi:
              rv = _bcm_esw_stat_flex_create_ingress_table_counters(
                        unit, VFIm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectIngL3Intf:
              rv = _bcm_esw_stat_flex_create_ingress_table_counters(
                        unit, L3_IIFm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectIngVrf:
              rv = _bcm_esw_stat_flex_create_ingress_table_counters(
                        unit, VRFm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectIngPolicy:
              rv = _bcm_esw_stat_flex_create_ingress_table_counters(
                        unit, VFP_POLICY_TABLEm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectIngMplsVcLabel:
              rv = _bcm_esw_stat_flex_create_ingress_table_counters(
                        unit, SOURCE_VPm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectIngMplsSwitchLabel:
              rv = _bcm_esw_stat_flex_create_ingress_table_counters(
                        unit, MPLS_ENTRYm,mode,
                        &base_index,&pool_number);
              break;
         /* Egress Side */
         case bcmStatObjectEgrPort:
              rv = _bcm_esw_stat_flex_create_egress_table_counters(
                        unit, EGR_PORTm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectEgrVlan:
              rv = _bcm_esw_stat_flex_create_egress_table_counters(
                        unit, EGR_VLANm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectEgrVlanXlate:
              rv = _bcm_esw_stat_flex_create_egress_table_counters(
                        unit, EGR_VLAN_XLATEm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectEgrVfi:
              rv = _bcm_esw_stat_flex_create_egress_table_counters(
                        unit, EGR_VFIm,mode,
                        &base_index,&pool_number);
              break;
         case bcmStatObjectEgrL3Intf:
              rv = _bcm_esw_stat_flex_create_egress_table_counters(
                        unit, EGR_L3_NEXT_HOPm,mode,
                        &base_index,&pool_number);
              break;
         }
    }
    /* Cleanup activity ... */
    sal_free(attr);
    if (BCM_FAILURE(rv)) {
        FLEXCTR_ERR(("creation of counters passed  failed..\n"));
    } else {
        /* mode_id =  Max 3 bits (Total Eight modes) */
        /* group_mode_id = Max 5 bits (Total 32)     */
        /* pool_id = 4 (Max Pool:16)                 */
        /* a/c object_id=4 (Max Object:16)           */
        /* 2 bytes for base index                    */
        /* 000    0-0000    0000   -0000      0000-0000 0000-0000   */
        /* Mode3b Group5b   Pool4b -A/cObj4b  base-index            */
    
        _bcm_esw_stat_get_counter_id(group_mode,object, mode,
                                     pool_number,base_index,stat_counter_id);
        FLEXCTR_VVERB(("Create: mode:%d group_mode:%d pool:%d object:%d"
                       " base:%d\n stat_counter_id:%d\n",
                       mode,group_mode,pool_number,object,base_index,
                       *stat_counter_id));
        *num_entries=total_counters;
    }
    return rv;
}
/*
 * Function:
 *      _bcm_esw_stat_group_destroy
 * Description:
 *      Release HW counter resources as per given counter id and makes system 
 *      unavailable for any further stat collection action 
 * Parameters:
 *      unit            - (IN) unit number
 *      Stat_counter_id - (IN) Stat Counter Id
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      
 */
bcm_error_t _bcm_esw_stat_group_destroy(
            int	   unit,
            uint32 stat_counter_id)
{
    bcm_error_t               rv=BCM_E_NONE;
    uint32                    pool_number=0;
    uint32                    base_index=0;
    bcm_stat_flex_mode_t      offset_mode=0;
    bcm_stat_object_t         object=bcmStatObjectIngPort;
    bcm_stat_group_mode_t     group_mode= bcmStatGroupModeSingle;
    bcm_stat_flex_direction_t direction=bcmStatFlexDirectionIngress;

    if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
         return BCM_E_UNAVAIL;
    }

    _bcm_esw_stat_get_counter_id_info(stat_counter_id,&group_mode,&object, 
                                      &offset_mode,&pool_number,&base_index);
    FLEXCTR_VVERB(("Deleting : mode:%d group_mode:%d pool:%d object:%d"
                   "base:%d\n stat_counter_id:%d\n",offset_mode,group_mode,
                   pool_number,object,base_index,stat_counter_id));

    /* Validate object id first */
    if (!((object >= bcmStatObjectIngPort) && 
          (object <= bcmStatObjectEgrL3Intf))) {
           FLEXCTR_ERR(("Invalid bcm_stat_object_t passed %d \n",object));
           return BCM_E_PARAM;
    }
    if ((object >= bcmStatObjectIngPort) && 
        (object <= bcmStatObjectIngMplsSwitchLabel)) {
        direction = bcmStatFlexDirectionIngress;
    } else {
        direction = bcmStatFlexDirectionEgress;
    }
    /* Validate group_mode */
    if (!((group_mode >= bcmStatGroupModeSingle) &&
          (group_mode <= bcmStatGroupModeDvpType))) {
           FLEXCTR_ERR(("Invalid bcm_stat_group_mode_t passed %d \n",
                        group_mode));
           return BCM_E_PARAM;
    }
    switch(object) {
         /* Ingress Side */
         case bcmStatObjectIngPort:
              rv = _bcm_esw_stat_flex_destroy_ingress_table_counters(
                        unit, PORT_TABm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectIngVlan:
              rv = _bcm_esw_stat_flex_destroy_ingress_table_counters(
                        unit, VLAN_TABm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectIngVlanXlate:
              rv = _bcm_esw_stat_flex_destroy_ingress_table_counters(
                        unit, VLAN_XLATEm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectIngVfi:
              rv = _bcm_esw_stat_flex_destroy_ingress_table_counters(
                        unit, VFIm, offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectIngL3Intf:
              rv = _bcm_esw_stat_flex_destroy_ingress_table_counters(
                        unit, L3_IIFm, offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectIngVrf:
              rv = _bcm_esw_stat_flex_destroy_ingress_table_counters(
                        unit, VRFm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectIngPolicy:
              rv = _bcm_esw_stat_flex_destroy_ingress_table_counters(
                        unit, VFP_POLICY_TABLEm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectIngMplsVcLabel:
              rv = _bcm_esw_stat_flex_destroy_ingress_table_counters(
                        unit, SOURCE_VPm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectIngMplsSwitchLabel:
              rv = _bcm_esw_stat_flex_destroy_ingress_table_counters(
                        unit, MPLS_ENTRYm,offset_mode,
                        base_index,pool_number);
              break;
         /* Egress Side */
         case bcmStatObjectEgrPort:
              rv = _bcm_esw_stat_flex_destroy_egress_table_counters(
                        unit, EGR_PORTm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectEgrVlan:
              rv = _bcm_esw_stat_flex_destroy_egress_table_counters(
                        unit, EGR_VLANm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectEgrVlanXlate:
              rv = _bcm_esw_stat_flex_destroy_egress_table_counters(
                        unit, EGR_VLAN_XLATEm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectEgrVfi:
              rv = _bcm_esw_stat_flex_destroy_egress_table_counters(
                        unit, EGR_VFIm,offset_mode,
                        base_index,pool_number);
              break;
         case bcmStatObjectEgrL3Intf:
              rv = _bcm_esw_stat_flex_destroy_egress_table_counters(
                        unit, EGR_L3_NEXT_HOPm,offset_mode,
                        base_index,pool_number);
              break;
              break;
    }
    if (BCM_SUCCESS(rv)) {
        FLEXCTR_VVERB(("Destroyed egress table counters.."
                       "Trying to delete group mode itself \n"));
        /* No decision on return values as actual call is successful */
        if (direction == bcmStatFlexDirectionIngress) {
            if (_bcm_esw_stat_flex_delete_ingress_mode(
                unit,offset_mode) == BCM_E_NONE) {
                FLEXCTR_VVERB(("Destroyed Ingress Mode also \n"));
                _bcm_esw_stat_flex_reset_group_mode(
                                    unit,bcmStatFlexDirectionIngress,
                                    offset_mode,group_mode);
            }
       } else {
           if (_bcm_esw_stat_flex_delete_egress_mode(
               unit,offset_mode) == BCM_E_NONE) {
                FLEXCTR_VVERB(("Destroyed Egress Mode also \n"));
                _bcm_esw_stat_flex_reset_group_mode(
                                    unit,bcmStatFlexDirectionEgress,
                                    offset_mode,group_mode);
           }
       }
    }
    return rv;
}
