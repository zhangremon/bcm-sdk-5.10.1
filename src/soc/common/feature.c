/*
 * $Id: feature.c 1.536.2.30 Broadcom SDK $
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
 * Functions returning TRUE/FALSE providing chip/feature matrix.
 * TRUE means chip supports feature.
 *
 * File:        feature.c
 * Purpose:     Define features by chip, functionally.
 */

#include <sal/types.h>
#include <soc/drv.h>
#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbx_drv.h>
#endif
#include <soc/cm.h>
#include <soc/debug.h>

#define SOC_FEATURE_DEBUG
#if defined(SOC_FEATURE_DEBUG)
#define SOC_FEATURE_DEBUG_PRINT(_x) SOC_DEBUG_PRINT(_x)
#else
#define SOC_FEATURE_DEBUG_PRINT(_x)
#endif

char    *soc_feature_name[] = SOC_FEATURE_NAME_INITIALIZER;

#ifdef  BCM_5670
/*
 * BCM5670 A0
 */
int
soc_features_bcm5670_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5670_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_stat_dma:
    case soc_feature_dcb_type4:
    case soc_feature_led_proc:
    case soc_feature_ip_mcast:
    case soc_feature_tx_fast_path:
    case soc_feature_policer_mode_flow_rate_committed:
        return TRUE;
    case soc_feature_xgxs_v1:
        return (rev_id == BCM5670_A0_REV_ID);
    case soc_feature_xgxs_v2:
        return (rev_id != BCM5670_A0_REV_ID);
    case soc_feature_fabric_debug:
    case soc_feature_srcmod_filter:
        return (rev_id != BCM5670_A0_REV_ID);
    default:
        return FALSE;
    }
}
#endif  /* BCM_5670 */

#ifdef  BCM_88732
/*
 * BCM88732 A0
 */
int
soc_features_bcm88732_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 88732_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);
    switch (feature) {
    case soc_feature_no_bridging:
    case soc_feature_no_higig:
    case soc_feature_no_mirror:
    case soc_feature_no_learning:
    case soc_feature_xmac:
    case soc_feature_dcb_type22:
    case soc_feature_counter_parity:
    case soc_feature_eee:
    case soc_feature_priority_flow_control:
    case soc_feature_storm_control:
    case soc_feature_flex_port:
    case soc_feature_xy_tcam:
        return TRUE;
    case soc_feature_dcb_type16:
    case soc_feature_arl_hashed:
    case soc_feature_class_based_learning:
    case soc_feature_lpm_prefix_length_max_128:
    case soc_feature_tunnel_gre:
    case soc_feature_tunnel_any_in_6:
    case soc_feature_fifo_dma:
    case soc_feature_subport:
    case soc_feature_lpm_tcam:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_l3_ip6:
    case soc_feature_l3_lookup_cmd:
    case soc_feature_l3_sgv:
    case soc_feature_higig2:
    case soc_feature_bigmac_rxcnt_bug:
    case soc_feature_vlan_action:
    case soc_feature_vlan_translation:
    case soc_feature_vlan_ctrl:
    case soc_feature_color_prio_map:
    case soc_feature_virtual_switching:
    case soc_feature_gport_service_counters:
    case soc_feature_l2_multiple:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_l2_user_table:
    case soc_feature_filter:
    case soc_feature_field:
    case soc_feature_port_lag_failover:

        return FALSE;
    default:
        return soc_features_bcm56820_a0(unit, feature);
    }
}
#endif

#ifdef  BCM_5675
/*
 * BCM5675 A0
 */
int
soc_features_bcm5675_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5675_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_stat_dma:
    case soc_feature_dcb_type4:
    case soc_feature_led_proc:
    case soc_feature_xgxs_v2:
    case soc_feature_bigmac_fault_stat:
    case soc_feature_fabric_debug:
    case soc_feature_srcmod_filter:
    case soc_feature_modmap:
    case soc_feature_ip_mcast:
    case soc_feature_tx_fast_path:
    case soc_feature_policer_mode_flow_rate_committed:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif  /* BCM_5675 */

#ifdef  BCM_5673
/*
 * BCM5673 A0
 */
int
soc_features_bcm5673_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5673_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_stat_dma:
    case soc_feature_arl_hashed:
    case soc_feature_arl_lookup_cmd:
    case soc_feature_arl_lookup_retry:
    case soc_feature_cfap_pool:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_arl_insert_ovr:
    case soc_feature_stg_xgs:
    case soc_feature_stg:
    case soc_feature_dcb_type5:
    case soc_feature_filter:
    case soc_feature_filter_extended:
    case soc_feature_filter_metering:
    case soc_feature_filter_xgs:
    case soc_feature_filter_pktfmtext:
    case soc_feature_ingress_metering:
    case soc_feature_egress_metering:
    case soc_feature_stack_my_modid:
    case soc_feature_remap_ut_prio:
    case soc_feature_led_proc:
    case soc_feature_xgxs_v3:
    case soc_feature_bigmac_fault_stat:
    case soc_feature_phy_cl45:
    case soc_feature_dmux:
    case soc_feature_table_dma:
    case soc_feature_l3x_ins_igns_hit:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
    case soc_feature_tx_fast_path:
    case soc_feature_policer_mode_flow_rate_committed:
        return TRUE;
    case soc_feature_filter_128_rules:
    case soc_feature_fast_rate_limit:
        return (rev_id != BCM5673_A0_REV_ID);
   default:
        return FALSE;
    }
}
#endif  /* BCM_5673 */

#ifdef  BCM_5674
/*
 * BCM5674 A0
 */
int
soc_features_bcm5674_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5674_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_stat_dma:
    case soc_feature_arl_hashed:
    case soc_feature_arl_lookup_cmd:
    case soc_feature_arl_lookup_retry:
    case soc_feature_cfap_pool:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_arl_insert_ovr:
    case soc_feature_stg_xgs:
    case soc_feature_stg:
    case soc_feature_dcb_type5:
    case soc_feature_filter:
    case soc_feature_filter_extended:
    case soc_feature_filter_metering:
    case soc_feature_filter_xgs:
    case soc_feature_filter_pktfmtext:
    case soc_feature_ingress_metering:
    case soc_feature_egress_metering:
    case soc_feature_stack_my_modid:
    case soc_feature_remap_ut_prio:
    case soc_feature_led_proc:
    case soc_feature_xgxs_v4:
    case soc_feature_bigmac_fault_stat:
    case soc_feature_phy_cl45:
    case soc_feature_dmux:
    case soc_feature_table_dma:
    case soc_feature_fast_rate_limit:
    case soc_feature_filter_krules:
    case soc_feature_mstp_mask:
    case soc_feature_lynx_l3_expanded:
    case soc_feature_l3_sgv:
    case soc_feature_l3x_ins_igns_hit:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
    case soc_feature_tx_fast_path:
    case soc_feature_policer_mode_flow_rate_committed:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif  /* BCM_5674 */

#ifdef  BCM_5690
/*
 * BCM5690 A0/A1
 */
int
soc_features_bcm5690_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         l3disable, ffpdisable;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5690_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    l3disable = (dev_id == BCM5692_DEVICE_ID ||
                 dev_id == BCM5693_DEVICE_ID);
    ffpdisable = l3disable && rev_id == BCM5690_A0_REV_ID;

    switch (feature) {
    case soc_feature_stat_dma:
    case soc_feature_table_dma:
    case soc_feature_arl_hashed:
    case soc_feature_arl_lookup_cmd:
    case soc_feature_arl_lookup_retry:
    case soc_feature_arl_insert_ovr:
    case soc_feature_cfap_pool:
    case soc_feature_fe_gig_macs:
    case soc_feature_stg_xgs:
    case soc_feature_stg:
    case soc_feature_dcb_type3:
    case soc_feature_filter_extended:
    case soc_feature_filter_metering:
    case soc_feature_filter_xgs:
    case soc_feature_ingress_metering:
    case soc_feature_egress_metering:
    case soc_feature_stack_my_modid:
    case soc_feature_remap_ut_prio:
    case soc_feature_led_proc:
    case soc_feature_rsv_mask:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
    case soc_feature_trunk_egress:
    case soc_feature_tx_fast_path:
    case soc_feature_policer_mode_flow_rate_committed:
        return TRUE;
    case soc_feature_ip_mcast:
    case soc_feature_l3:
        return !l3disable;
    case soc_feature_filter:
        return !ffpdisable;
    case soc_feature_l3x_delete_bf_bug:
        return (rev_id == BCM5690_A0_REV_ID);   /* Fixed in A1 */
    case soc_feature_xgxs_v1:
        return (rev_id == BCM5690_A0_REV_ID);
    case soc_feature_xgxs_v2:
        return (rev_id >= BCM5690_A1_REV_ID);
    case soc_feature_ext_gth_hd_ipg:
        return (rev_id <= BCM5690_A1_REV_ID);
    case soc_feature_l2x_ins_sets_hit:
    case soc_feature_l3x_ins_igns_hit:
    case soc_feature_phy_5690_link_war:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif  /* BCM_5690 */

#ifdef  BCM_5695
/*
 * BCM5695 A0
 */
int
soc_features_bcm5695_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         l3disable;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5695_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    l3disable = (dev_id == BCM5697_DEVICE_ID ||
                 dev_id == BCM5698_DEVICE_ID);

    switch (feature) {
    case soc_feature_stat_dma:
    case soc_feature_table_dma:
    case soc_feature_arl_hashed:
    case soc_feature_arl_lookup_cmd:
    case soc_feature_arl_lookup_retry:
    case soc_feature_arl_insert_ovr:
    case soc_feature_cfap_pool:
    case soc_feature_fe_gig_macs:
    case soc_feature_stg_xgs:
    case soc_feature_stg:
    /* case soc_feature_dcb_type3: */
    case soc_feature_dcb_type7:
    case soc_feature_filter_extended:
    case soc_feature_filter_metering:
    case soc_feature_filter_xgs:
    case soc_feature_filter_pktfmtext:
    case soc_feature_ingress_metering:
    case soc_feature_egress_metering:
    case soc_feature_stack_my_modid:
    case soc_feature_remap_ut_prio:
    case soc_feature_led_proc:
    case soc_feature_filter:
    case soc_feature_xgxs_v2:
    case soc_feature_aging_extended:
    case soc_feature_filter_draco15:
    case soc_feature_l3_sgv:
    case soc_feature_rsv_mask:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
    case soc_feature_dscp_map_mode_all:
    case soc_feature_tx_fast_path:
    case soc_feature_trunk_egress:
    case soc_feature_policer_mode_flow_rate_committed:
        return TRUE;
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_l3_lookup_cmd:
        return !l3disable;
    case soc_feature_block_ctrl_ingress:
        return (rev_id > BCM5695_A0_REV_ID);
    case soc_feature_mstp_lookup:
    case soc_feature_ipmc_lookup:
        return (rev_id >= BCM5695_B0_REV_ID);
    case soc_feature_l3_sgv_aisb_hash:
        return (rev_id < BCM5695_B0_REV_ID);
    case soc_feature_mstp_uipmc:
        return (rev_id == BCM5695_B0_REV_ID);
    default:
        return FALSE;
    }
}
#endif  /* BCM_5695 */

#ifdef  BCM_5665
/*
 * BCM5665 A0
 */
int
soc_features_bcm5665_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5665_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_stat_dma:
    case soc_feature_table_dma:
    case soc_feature_schmsg_alias:
    case soc_feature_arl_hashed:
    case soc_feature_arl_lookup_cmd:
    case soc_feature_arl_lookup_retry:
    case soc_feature_l3x_delete_bf_bug:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_arl_insert_ovr:
    case soc_feature_fe_gig_macs:
    case soc_feature_stg_xgs:
    case soc_feature_stg:
    case soc_feature_dcb_type6:
    case soc_feature_filter:
    case soc_feature_filter_extended:
    case soc_feature_filter_metering:
    case soc_feature_filter_xgs:
    case soc_feature_filter_pktfmtext:
    case soc_feature_ingress_metering:
    case soc_feature_egress_metering:
    case soc_feature_xgxs_v2:
    case soc_feature_stack_my_modid:
    case soc_feature_remap_ut_prio:
    case soc_feature_led_proc:
    case soc_feature_filter_tucana:
    case soc_feature_dcb_st0_bug:
    case soc_feature_recheck_cntrs:
    case soc_feature_rsv_mask:
    case soc_feature_l3x_ins_igns_hit:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
    case soc_feature_mod1:
    case soc_feature_tx_fast_path:
    case soc_feature_policer_mode_flow_rate_committed:
        return TRUE;
    default:
        return FALSE;
    }
}

/*
 * BCM5665 B0
 */
int
soc_features_bcm5665_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5665_b0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_stat_dma:
    case soc_feature_table_dma:
    case soc_feature_schmsg_alias:
    case soc_feature_arl_hashed:
    case soc_feature_arl_lookup_cmd:
    case soc_feature_arl_lookup_retry:
    case soc_feature_l3x_delete_bf_bug:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_arl_insert_ovr:
    case soc_feature_fe_gig_macs:
    case soc_feature_stg_xgs:
    case soc_feature_stg:
    case soc_feature_dcb_type6:
    case soc_feature_filter:
    case soc_feature_filter_extended:
    case soc_feature_filter_metering:
    case soc_feature_filter_xgs:
    case soc_feature_filter_pktfmtext:
    case soc_feature_ingress_metering:
    case soc_feature_egress_metering:
    case soc_feature_xgxs_v2:
    case soc_feature_stack_my_modid:
    case soc_feature_remap_ut_prio:
    case soc_feature_led_proc:
    case soc_feature_filter_tucana:
    case soc_feature_mstp_mask:
    case soc_feature_l3_sgv:
    case soc_feature_rsv_mask:
    case soc_feature_l3x_ins_igns_hit:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
    case soc_feature_mod1:
    case soc_feature_tx_fast_path:
    case soc_feature_policer_mode_flow_rate_committed:
        return TRUE;
    default:
        return FALSE;
    }
}

/*
 * BCM5650 C0
 */
int
soc_features_bcm5650_c0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," bcm5650_c0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_stat_dma:
    case soc_feature_table_dma:
    case soc_feature_schmsg_alias:
    case soc_feature_arl_hashed:
    case soc_feature_arl_lookup_cmd:
    case soc_feature_arl_lookup_retry:
    case soc_feature_l3x_delete_bf_bug:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_arl_insert_ovr:
    case soc_feature_fe_gig_macs:
    case soc_feature_stg_xgs:
    case soc_feature_stg:
    case soc_feature_dcb_type6:
    case soc_feature_filter:
    case soc_feature_filter_extended:
    case soc_feature_filter_metering:
    case soc_feature_filter_xgs:
    case soc_feature_filter_pktfmtext:
    case soc_feature_ingress_metering:
    case soc_feature_egress_metering:
    case soc_feature_xgxs_v2:
    case soc_feature_stack_my_modid:
    case soc_feature_remap_ut_prio:
    case soc_feature_led_proc:
    case soc_feature_filter_tucana:
    case soc_feature_mstp_mask:
    case soc_feature_l3_sgv:
    case soc_feature_rsv_mask:
    case soc_feature_l3x_ins_igns_hit:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
    case soc_feature_mod1:
    case soc_feature_trunk_egress:
    case soc_feature_tx_fast_path:
    case soc_feature_policer_mode_flow_rate_committed:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif  /* BCM_5665 */

#if defined(BCM_56601) || defined(BCM_56602)
/*
 * BCM56601 A0
 */
int
soc_features_bcm56601_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         l3disable;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56601_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

/*
    l3disable = (dev_id == BCM56601_DEVICE_ID ||
                 dev_id == BCM56602_DEVICE_ID);
*/
    l3disable = 0;

    switch (feature) {
        case soc_feature_table_dma:
        case soc_feature_tslam_dma:
        case soc_feature_dcb_type10:
        case soc_feature_schmsg_alias:
        case soc_feature_aging_extended:
        case soc_feature_schan_hw_timeout:
        case soc_feature_phy_cl45:
        case soc_feature_mdio_enhanced:
        case soc_feature_stat_dma:
        case soc_feature_cpuport_stat_dma:
        case soc_feature_cpuport_switched:
        case soc_feature_cpuport_mirror:
        case soc_feature_fe_gig_macs:
        case soc_feature_trimac:
        case soc_feature_cos_rx_dma:
        case soc_feature_xgxs_lcpll:
        case soc_feature_dodeca_serdes:
        case soc_feature_txdma_purge:
        case soc_feature_rxdma_cleanup:
        case soc_feature_xgxs_v5:
        case soc_feature_fe_maxframe:   /* fe_maxfr = MAXFR + 1 */
        case soc_feature_vlan_mc_flood_ctrl: /* Per VLAN PFM support */

        case soc_feature_arl_hashed:
        case soc_feature_l2_multiple:
        case soc_feature_l2_hashed:
        case soc_feature_l2_lookup_cmd:
        case soc_feature_l2_lookup_retry:
        case soc_feature_l2_user_table:
        case soc_feature_arl_insert_ovr:
        case soc_feature_stg_xgs:
        case soc_feature_stg:
        case soc_feature_stack_my_modid:
        case soc_feature_remap_ut_prio:
        case soc_feature_led_proc:
        case soc_feature_field:
        case soc_feature_field_ingress_late:
        case soc_feature_mpls:
        case soc_feature_bigmac_fault_stat:
        case soc_feature_mem_cmd:
        case soc_feature_ingress_metering:
        case soc_feature_egress_metering:
        case soc_feature_stat_jumbo_adj:
        case soc_feature_stat_xgs3:
        case soc_feature_color:
        case soc_feature_dscp:
        case soc_feature_dscp_map_mode_all:
        case soc_feature_trunk_egress:
            /* Only 2 mapping modes. All or None */
        case soc_feature_xgs1_mirror:
        case soc_feature_vlan_translation:
        case soc_feature_trunk_extended:
        case soc_feature_bucket_support:
        case soc_feature_hg_trunk_override:
        case soc_feature_hg_trunking:
        case soc_feature_field_pkt_res_adj:
        case soc_feature_deskew_dll:
        case soc_feature_unknown_ipmc_tocpu:
        case soc_feature_proto_pkt_ctrl:
        case soc_feature_basic_dos_ctrl:
        case soc_feature_stat_dma_error_ack:
        case soc_feature_tunnel_gre:
        case soc_feature_reset_delay:
        case soc_feature_sample_offset8: 
        case soc_feature_extended_pci_error:
            return TRUE;
        case soc_feature_ip_mcast:
        case soc_feature_ip_mcast_repl:
        case soc_feature_l3:
        case soc_feature_l3_ip6:
        case soc_feature_l3_lookup_cmd:
        case soc_feature_l3_sgv:
            return !l3disable;
        case soc_feature_prime_ctr_writes:
        case soc_feature_seer_bcam_tune:
        case soc_feature_mcu_fifo_suppress:
        case soc_feature_l3defip_bound_adj:
        case soc_feature_egr_ts_ctrl:
            return (rev_id == BCM56601_A0_REV_ID);
        default:
            return FALSE;
    }
}

/*
 * BCM56601 B0
 */
int
soc_features_bcm56601_b0(int unit, soc_feature_t feature)
{
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56601_b0"));
    switch (feature) {
    case soc_feature_port_trunk_index:
    case soc_feature_ipmc_grp_parity:
    case soc_feature_l3defip_parity:
    case soc_feature_field_udf_hg:
    case soc_feature_field_color_indep:
    case soc_feature_field_qual_IpType:
    case soc_feature_mpls_pop_search:
    case soc_feature_cpu_proto_prio:
    case soc_feature_untagged_vt_miss:
    case soc_feature_ignore_pkt_tag:
    case soc_feature_l2_modfifo:
    case soc_feature_port_egr_block_ctl:
    case soc_feature_remote_learn_trust:
    case soc_feature_field_int_fsel_adj:
    case soc_feature_src_mac_group:
    case soc_feature_ext_tcam_sharing:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_sample_thresh16:
        return TRUE;
    case soc_feature_field_pkt_res_adj:
    case soc_feature_deskew_dll:
    case soc_feature_sample_offset8: 
        return FALSE;
    default:
        return soc_features_bcm56601_a0(unit, feature);
    }
}

/*
 * BCM56601 C0
 */
int
soc_features_bcm56601_c0(int unit, soc_feature_t feature)
{
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56601_c0"));
    switch (feature) {
    case soc_feature_mpls_bos_lookup:
    case soc_feature_mpls_per_vlan:
    case soc_feature_vlan_translation_range:
    case soc_feature_static_pfm:
    case soc_feature_higig_lookup:
        return TRUE;
    default:
        return soc_features_bcm56601_b0(unit, feature);
    }
}
#endif /* BCM_56601 || BCM_56602 */

#if defined(BCM_56602)
/*
 * BCM56602 A0
 */
int
soc_features_bcm56602_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         l3disable;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56602_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

/*
    l3disable = (dev_id == BCM56601_DEVICE_ID ||
                 dev_id == BCM56602_DEVICE_ID);
*/
    l3disable = 0;

    switch (feature) {
    case soc_feature_trimac:
    case soc_feature_fe_gig_macs:
        return FALSE;
    default:
        return soc_features_bcm56601_a0(unit, feature);
    }
}

/*
 * BCM56602 B0
 */
int
soc_features_bcm56602_b0(int unit, soc_feature_t feature)
{
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56602_b0"));
    switch (feature) {
    case soc_feature_port_trunk_index:
    case soc_feature_ipmc_grp_parity:
    case soc_feature_l3defip_parity:
    case soc_feature_field_udf_hg:
    case soc_feature_field_color_indep:
    case soc_feature_field_qual_IpType:
    case soc_feature_mpls_pop_search:
    case soc_feature_cpu_proto_prio:
    case soc_feature_untagged_vt_miss:
    case soc_feature_ignore_pkt_tag:
    case soc_feature_l2_modfifo:
    case soc_feature_port_egr_block_ctl:
    case soc_feature_remote_learn_trust:
    case soc_feature_field_int_fsel_adj:
    case soc_feature_ext_tcam_sharing:
    case soc_feature_src_mac_group:
    case soc_feature_nip_l3_err_tocpu:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_sample_thresh16:
        return TRUE;
    case soc_feature_field_pkt_res_adj:
    case soc_feature_deskew_dll:
    case soc_feature_sample_offset8:
        return FALSE;
    default:
        return soc_features_bcm56602_a0(unit, feature);
    }
}

/*
 * BCM56602 C0
 */
int
soc_features_bcm56602_c0(int unit, soc_feature_t feature)
{
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56602_c0"));
    switch (feature) {
    case soc_feature_mpls_per_vlan:
    case soc_feature_vlan_translation_range:
    case soc_feature_higig_lookup:
        return TRUE;
    default:
        return soc_features_bcm56602_b0(unit, feature);
    }
}
#endif  /* BCM_56602 */

#if defined(BCM_56504) || defined(BCM_56102) || defined(BCM_56304) || \
    defined(BCM_56514) || defined(BCM_56112) || defined(BCM_56314)
/*
 * BCM56504 A0
 */
int
soc_features_bcm56504_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         l3disable;
    int         a0;
    int         helix;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56504_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    a0    = (rev_id == BCM56504_A0_REV_ID);
    helix = (dev_id == BCM56404_DEVICE_ID);

    l3disable = (dev_id == BCM56305_DEVICE_ID || 
                 dev_id == BCM56306_DEVICE_ID ||
                 dev_id == BCM56307_DEVICE_ID || 
                 dev_id == BCM56308_DEVICE_ID ||
                 dev_id == BCM56309_DEVICE_ID ||
                 dev_id == BCM56315_DEVICE_ID || 
                 dev_id == BCM56316_DEVICE_ID ||
                 dev_id == BCM56317_DEVICE_ID || 
                 dev_id == BCM56318_DEVICE_ID ||
                 dev_id == BCM56319_DEVICE_ID ||
                 dev_id == BCM56516_DEVICE_ID ||
                 dev_id == BCM56517_DEVICE_ID || 
                 dev_id == BCM56518_DEVICE_ID ||
                 dev_id == BCM56519_DEVICE_ID || 
                 dev_id == BCM56700_DEVICE_ID ||
                 dev_id == BCM56701_DEVICE_ID);

    l3disable = (l3disable ||
                 dev_id == BCM53301_DEVICE_ID ||
		 dev_id == BCM56218_DEVICE_ID);

    l3disable = (l3disable ||
                 (SOC_SWITCH_BYPASS_MODE(unit) != SOC_SWITCH_BYPASS_MODE_NONE));

    switch (feature) {
        case soc_feature_table_dma:
        case soc_feature_tslam_dma:
        case soc_feature_dcb_type9:
        case soc_feature_schmsg_alias:
        case soc_feature_l2_lookup_cmd:
        case soc_feature_l2_user_table:
        case soc_feature_aging_extended:
        case soc_feature_arl_hashed:
        case soc_feature_l2_modfifo:
        case soc_feature_phy_cl45:
        case soc_feature_mdio_enhanced:
        case soc_feature_schan_hw_timeout:
        case soc_feature_stat_dma:
        case soc_feature_cpuport_stat_dma:
        case soc_feature_cpuport_switched:
        case soc_feature_cpuport_mirror:
        case soc_feature_fe_gig_macs:
        case soc_feature_trimac:
        case soc_feature_cos_rx_dma:
        case soc_feature_xgxs_lcpll:
        case soc_feature_dodeca_serdes:
        case soc_feature_xgxs_v5:
        case soc_feature_txdma_purge:
        case soc_feature_rxdma_cleanup:
        case soc_feature_fe_maxframe:   /* fe_maxfr = MAXFR + 1 */
        case soc_feature_vlan_mc_flood_ctrl: /* Per VLAN PFM support */

        case soc_feature_l2_hashed:
        case soc_feature_l2_lookup_retry:
        case soc_feature_arl_insert_ovr:
        case soc_feature_cfap_pool:
        case soc_feature_stg_xgs:
        case soc_feature_stg:
        case soc_feature_stack_my_modid:
        case soc_feature_remap_ut_prio:
        case soc_feature_led_proc:
        case soc_feature_field:
        case soc_feature_bigmac_fault_stat:
        case soc_feature_ingress_metering:
        case soc_feature_egress_metering:
        case soc_feature_stat_jumbo_adj:
        case soc_feature_stat_xgs3:
        case soc_feature_trunk_egress:
        case soc_feature_color:
        case soc_feature_dscp:
        case soc_feature_dscp_map_mode_all:
            /* Only 2 mapping modes. All or None */
        case soc_feature_egr_vlan_check:
        case soc_feature_xgs1_mirror:
        case soc_feature_vlan_translation:
        case soc_feature_trunk_extended:
        case soc_feature_bucket_support:
        case soc_feature_hg_trunk_override: 
        case soc_feature_hg_trunking:
        case soc_feature_field_mirror_pkts_ctl:
        case soc_feature_unknown_ipmc_tocpu:
        case soc_feature_basic_dos_ctrl:
        case soc_feature_proto_pkt_ctrl:
        case soc_feature_stat_dma_error_ack:
        case soc_feature_asf:
        case soc_feature_xport_convertible: 
        case soc_feature_sgmii_autoneg:
        case soc_feature_sample_offset8:
        case soc_feature_extended_pci_error:
        case soc_feature_l3_defip_ecmp_count:
        case soc_feature_field_action_l2_change:
            return TRUE;
        case soc_feature_l3_defip_map:  /* Map out unused L3_DEFIP blocks */
            return a0;
        case soc_feature_status_link_fail:
        case soc_feature_egr_vlan_pfm:  /* MH PFM control per VLAN FB A0 */
        case soc_feature_asf_no_10_100:
            return (rev_id < BCM56504_B0_REV_ID);
        case soc_feature_ipmc_repl_freeze:
            return (rev_id < BCM56504_B2_REV_ID);
        case soc_feature_policer_mode_flow_rate_committed:
            return (dev_id == BCM56504_DEVICE_ID); /* Only for Firebolt proper */
        case soc_feature_lpm_tcam:
        case soc_feature_ip_mcast:
        case soc_feature_ip_mcast_repl:
        case soc_feature_l3:
        case soc_feature_l3_ip6:
        case soc_feature_l3_lookup_cmd:
        case soc_feature_l3_sgv:
            return !l3disable;
        case soc_feature_field_slices8:
            return helix;
        case soc_feature_field_slices4:
        case soc_feature_fp_based_routing: 
        case soc_feature_fp_routing_mirage:
            return (dev_id == BCM53300_DEVICE_ID);
        default:
            return FALSE;
    }
}

/*
 * BCM56504 B0
 */
int
soc_features_bcm56504_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56504_b0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_l2x_parity:
    case soc_feature_l3defip_parity:
            if (SAL_BOOT_BCMSIM) {
                return FALSE;
            }
            /* Fall through */
    case soc_feature_parity_err_tocpu:
    case soc_feature_nip_l3_err_tocpu:
    case soc_feature_l3mtu_fail_tocpu:
    case soc_feature_meter_adjust:      /* Adjust for IPG */
    case soc_feature_field_mirror_ovr:
    case soc_feature_field_udf_higig:
    case soc_feature_field_udf_ethertype:
    case soc_feature_field_comb_read:
    case soc_feature_field_wide:
    case soc_feature_field_slice_enable:
    case soc_feature_field_cos:
    case soc_feature_field_color_indep:
    case soc_feature_field_qual_drop:
    case soc_feature_field_qual_IpType:
    case soc_feature_field_qual_Ip6High:
    case soc_feature_src_modid_blk:
    case soc_feature_dbgc_higig_lkup:
    case soc_feature_port_trunk_index:
    case soc_feature_color_inner_cfi:
    case soc_feature_untagged_vt_miss:
    case soc_feature_module_loopback:
    case soc_feature_tunnel_dscp_trust:
    case soc_feature_higig_lookup:
    case soc_feature_egr_l3_mtu:
    case soc_feature_egr_mirror_path:
    case soc_feature_port_egr_block_ctl:
    case soc_feature_ipmc_group_mtu:
    case soc_feature_tunnel_6to4_secure:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_big_icmpv6_ping_check:
    case soc_feature_src_modid_blk_ucast_override:
    case soc_feature_egress_blk_ucast_override:
    case soc_feature_static_pfm:
    case soc_feature_dcb_type13:
    case soc_feature_sample_thresh16:
        return TRUE;
    case soc_feature_src_mac_group:
    case soc_feature_remote_learn_trust:
        return (rev_id >= BCM56504_B2_REV_ID);
    case soc_feature_dcb_type9:
    case soc_feature_field_mirror_pkts_ctl:
    case soc_feature_l3x_parity:
    case soc_feature_sample_offset8:
        return FALSE;
    default:
        return soc_features_bcm56504_a0(unit, feature);
    }
}
#endif  /* BCM_565[01]4 BCM_561[01]2 BCM_563[01]4 */

#ifdef  BCM_56102
/*
 * BCM56102 A0
 */
int
soc_features_bcm56102_a0(int unit, soc_feature_t feature)
{
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56102_a0"));
    switch (feature) {
    case soc_feature_l2x_parity:
    case soc_feature_parity_err_tocpu:
    case soc_feature_meter_adjust:      /* Adjust for IPG */
    case soc_feature_field_slices8:
    case soc_feature_xgxs_power:
    case soc_feature_field_mirror_ovr:
    case soc_feature_src_modid_blk:
    case soc_feature_field_udf_higig:
    case soc_feature_field_comb_read:
    case soc_feature_egr_mirror_path:
    case soc_feature_egr_vlan_check:
    case soc_feature_vlan_translation:
    case soc_feature_ipmc_repl_freeze:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_big_icmpv6_ping_check:
    case soc_feature_asf_no_10_100:
    case soc_feature_static_pfm:
    case soc_feature_reset_delay:
        return TRUE;
    case soc_feature_egr_vlan_pfm:  /* MH PFM control per VLAN FB A0 only */
    case soc_feature_status_link_fail:
    case soc_feature_field_mirror_pkts_ctl:
    case soc_feature_l3x_parity:
    case soc_feature_l3_defip_ecmp_count:
        return FALSE;
    default:
        return soc_features_bcm56504_a0(unit, feature);
    }
}
#endif  /* BCM_56102 */

#ifdef  BCM_56112
/*
 * BCM56112 A0
 */
int
soc_features_bcm56112_a0(int unit, soc_feature_t feature)
{
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56112_a0"));
    switch (feature) {
    case soc_feature_l2x_parity:
    case soc_feature_parity_err_tocpu:
    case soc_feature_nip_l3_err_tocpu:
    case soc_feature_meter_adjust:      /* Adjust for IPG */
    case soc_feature_field_slices8:
    case soc_feature_field_qual_Ip6High:
    case soc_feature_xgxs_power:
    case soc_feature_lmd:
    case soc_feature_field_mirror_ovr:
    case soc_feature_src_modid_blk:
    case soc_feature_field_udf_higig:
    case soc_feature_field_comb_read:
    case soc_feature_egr_mirror_path:
    case soc_feature_egr_vlan_check:
    case soc_feature_remote_learn_trust:
    case soc_feature_ipmc_group_mtu:
    case soc_feature_src_mac_group:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_reset_delay:
        return TRUE;
    case soc_feature_ipmc_repl_freeze:
    case soc_feature_egr_vlan_pfm:  /* MH PFM control per VLAN FB A0 only */
    case soc_feature_status_link_fail:
    case soc_feature_asf_no_10_100:
    case soc_feature_l3x_parity:
    case soc_feature_l3_defip_ecmp_count:
        return FALSE;
    default:
        return soc_features_bcm56504_b0(unit, feature);
    }
}
#endif  /* BCM_56112 */

#ifdef  BCM_56304
/*
 * BCM56304 B0
 */
int
soc_features_bcm56304_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56304_b0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_l2x_parity:
    case soc_feature_parity_err_tocpu:
    case soc_feature_meter_adjust:      /* Adjust for IPG */
    case soc_feature_field_slices8:
    case soc_feature_field_mirror_ovr:
    case soc_feature_src_modid_blk:
    case soc_feature_field_udf_higig:
    case soc_feature_field_comb_read:
    case soc_feature_egr_mirror_path:
    case soc_feature_egr_vlan_check:
    case soc_feature_xgxs_power:
    case soc_feature_vlan_translation:
    case soc_feature_ipmc_repl_freeze:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_big_icmpv6_ping_check:
    case soc_feature_asf_no_10_100:
    case soc_feature_static_pfm:
    case soc_feature_reset_delay:
    case soc_feature_sample_thresh16:
        return TRUE;
    case soc_feature_egr_vlan_pfm:  /* MH PFM control per VLAN FB A0 only */
    case soc_feature_status_link_fail:
    case soc_feature_field_mirror_pkts_ctl:
    case soc_feature_l3x_parity:
    case soc_feature_sample_offset8:
    case soc_feature_l3_defip_ecmp_count:
        return FALSE;
    case soc_feature_fp_based_routing: 
    case soc_feature_fp_routing_mirage:
        return (dev_id == BCM53300_DEVICE_ID);
    default:
        return soc_features_bcm56504_a0(unit, feature);
    }
}
#endif  /* BCM_56304 */

#ifdef  BCM_56314
/*
 * BCM56314 A0
 */
int
soc_features_bcm56314_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         l3disable;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56314_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    l3disable = (dev_id == BCM56315_DEVICE_ID || 
                 dev_id == BCM56316_DEVICE_ID ||
                 dev_id == BCM56317_DEVICE_ID || 
                 dev_id == BCM56318_DEVICE_ID ||
                 dev_id == BCM56319_DEVICE_ID);

    switch (feature) {
    case soc_feature_l2x_parity:
    case soc_feature_parity_err_tocpu:
    case soc_feature_nip_l3_err_tocpu:
    case soc_feature_meter_adjust:      /* Adjust for IPG */
    case soc_feature_field_slices8:
    case soc_feature_field_qual_Ip6High:
    case soc_feature_field_mirror_ovr:
    case soc_feature_src_modid_blk:
    case soc_feature_field_udf_higig:
    case soc_feature_field_comb_read:
    case soc_feature_egr_mirror_path:
    case soc_feature_egr_vlan_check:
    case soc_feature_xgxs_power:
    case soc_feature_lmd:
    case soc_feature_remote_learn_trust:
    case soc_feature_ipmc_group_mtu:
    case soc_feature_src_mac_group:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_reset_delay:
        return TRUE;
    case soc_feature_ipmc_repl_freeze:
    case soc_feature_egr_vlan_pfm:  /* MH PFM control per VLAN FB A0 only */
    case soc_feature_status_link_fail:
    case soc_feature_asf_no_10_100:
    case soc_feature_l3x_parity:
    case soc_feature_l3_defip_ecmp_count:
        return FALSE;
    default:
        return soc_features_bcm56504_b0(unit, feature);
    }
}
#endif  /* BCM_56314 */

#ifdef  BCM_56800
/*
 * BCM56800 A0
 */
int
soc_features_bcm56800_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         fabric;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56800_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    fabric = (dev_id == BCM56700_DEVICE_ID ||
              dev_id == BCM56701_DEVICE_ID ||
              dev_id == BCM56720_DEVICE_ID ||
              dev_id == BCM56721_DEVICE_ID ||
              dev_id == BCM56725_DEVICE_ID);

    switch (feature) {
    case soc_feature_dcb_type11:
    case soc_feature_higig2:
    case soc_feature_color:
    case soc_feature_color_inner_cfi:
    case soc_feature_untagged_vt_miss:
    case soc_feature_module_loopback:
    case soc_feature_xgxs_v6:
    case soc_feature_egr_l3_mtu:
    case soc_feature_egr_vlan_check:
    case soc_feature_hg_trunk_failover:
    case soc_feature_hg_trunking:
    case soc_feature_hg_trunk_override:
    case soc_feature_vlan_translation:
    case soc_feature_modmap:
    case soc_feature_two_ingress_pipes:
    case soc_feature_force_forward:
    case soc_feature_port_egr_block_ctl:
    case soc_feature_remote_learn_trust:
    case soc_feature_ipmc_group_mtu:
    case soc_feature_bigmac_rxcnt_bug:
    case soc_feature_cpu_proto_prio:
    case soc_feature_nip_l3_err_tocpu:
    case soc_feature_tunnel_6to4_secure:
    case soc_feature_field_wide:
    case soc_feature_field_qual_Ip6High:
    case soc_feature_field_mirror_ovr:
    case soc_feature_field_udf_ethertype:
    case soc_feature_field_comb_read:
    case soc_feature_field_slice_enable:
    case soc_feature_field_cos:
    case soc_feature_field_qual_drop:
    case soc_feature_field_qual_IpType:
    case soc_feature_field_udf_higig: 
    case soc_feature_field_color_indep: 
    case soc_feature_port_flow_hash:
    case soc_feature_trunk_group_size:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_l3mtu_fail_tocpu:
    case soc_feature_src_modid_blk_opcode_override:
    case soc_feature_src_modid_blk_ucast_override:
    case soc_feature_egress_blk_ucast_override:
    case soc_feature_ipmc_repl_penultimate:
    case soc_feature_sgmii_autoneg: 
    case soc_feature_l3x_parity:
    case soc_feature_tunnel_dscp_trust:
        return TRUE;
    case soc_feature_dcb_type9:
    case soc_feature_egr_vlan_pfm:  /* MH PFM control per VLAN FB A0 only */
    case soc_feature_txdma_purge:
    case soc_feature_xgxs_lcpll:
    case soc_feature_xgxs_v5:
    case soc_feature_asf_no_10_100:
    case soc_feature_ipmc_repl_freeze:
        return FALSE;
    case soc_feature_ctr_xaui_activity:
        return (rev_id == BCM56800_A0_REV_ID);
    case soc_feature_urpf:
        return !fabric;
    case soc_feature_arl_hashed:
    case soc_feature_l2_user_table:
    case soc_feature_trunk_egress:
    case soc_feature_proto_pkt_ctrl:
    case soc_feature_xport_convertible: 
        if (fabric) {
            return FALSE;
        }
        /* Fall through */
    default:
        return soc_features_bcm56304_b0(unit, feature);
    }
}
#endif  /* BCM_56800 */

#ifdef  BCM_56624
/*
 * BCM56624 A0
 */
int
soc_features_bcm56624_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56624_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_dcb_type14:
    case soc_feature_table_hi:      /* > 32 bits in PBM */
    case soc_feature_vlan_ctrl:          /* per VLAN property control */  
    case soc_feature_igmp_mld_support:
    case soc_feature_trunk_group_overlay:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
    case soc_feature_mac_learn_limit:
    case soc_feature_system_mac_learn_limit:
    case soc_feature_class_based_learning:
    case soc_feature_storm_control:
    case soc_feature_aging_extended:
    case soc_feature_unimac:
    case soc_feature_dual_hash:
    case soc_feature_l3_entry_key_type:
    case soc_feature_generic_table_ops:
    case soc_feature_mem_push_pop:
    case soc_feature_lpm_prefix_length_max_128:
    case soc_feature_color_prio_map:
    case soc_feature_tunnel_gre:
    case soc_feature_tunnel_any_in_6:
    case soc_feature_field_virtual_slice_group:
    case soc_feature_field_intraslice_double_wide:
    case soc_feature_field_egress_flexible_v6_key:
    case soc_feature_field_multi_stage:
    case soc_feature_field_ingress_global_meter_pools:
    case soc_feature_field_egress_global_counters:
    case soc_feature_field_ing_egr_separate_packet_byte_counters:
    case soc_feature_field_ingress_ipbm:
    case soc_feature_dcb_reason_hi:
    case soc_feature_trunk_group_size:
    case soc_feature_multi_sbus_cmds:
    case soc_feature_esm_support:
    case soc_feature_fifo_dma:
    case soc_feature_ipfix:
    case soc_feature_src_mac_group:
    case soc_feature_higig_lookup:
    case soc_feature_xgxs_v7:
    case soc_feature_mpls:
    case soc_feature_mpls_software_failover:
    case soc_feature_subport:
    case soc_feature_l2_pending:
    case soc_feature_vlan_translation_range:
    case soc_feature_vlan_action:
    case soc_feature_packet_rate_limit:
    case soc_feature_hw_stats_calc:
    case soc_feature_sample_thresh16:
    case soc_feature_virtual_switching:
    case soc_feature_enhanced_dos_ctrl:
    case soc_feature_use_double_freq_for_ddr_pll:
    case soc_feature_ignore_cmic_xgxs_pll_status:
    case soc_feature_rcpu_1:
    case soc_feature_l3_ingress_interface:
    case soc_feature_ppa_bypass:
    case soc_feature_egr_dscp_map_per_port:
    case soc_feature_l3_dynamic_ecmp_group:
    case soc_feature_modport_map_profile:
    case soc_feature_qos_profile:
    case soc_feature_field_action_l2_change:
#ifdef INCLUDE_RCPU
    case soc_feature_rcpu_priority:
#endif /* INCLUDE_RCPU */

        return TRUE;
    case soc_feature_xgxs_v6:
    case soc_feature_dcb_type11:
    case soc_feature_fe_gig_macs:
    case soc_feature_ctr_xaui_activity:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_two_ingress_pipes:
    case soc_feature_field_slices8:
    case soc_feature_ipmc_group_mtu:
    case soc_feature_egr_l3_mtu:
    case soc_feature_trimac:
    case soc_feature_unknown_ipmc_tocpu:
    case soc_feature_sample_offset8:
    case soc_feature_reset_delay:
    case soc_feature_l3_defip_ecmp_count:
    case soc_feature_tunnel_dscp_trust:
        return FALSE;
    case soc_feature_always_drive_dbus:
        return (dev_id == BCM56624_DEVICE_ID || dev_id == BCM56626_DEVICE_ID ||
                dev_id == BCM56628_DEVICE_ID || dev_id == BCM56629_DEVICE_ID)
                && rev_id == BCM56624_A1_REV_ID;
    default:
        return soc_features_bcm56800_a0(unit, feature);
    }
}

/*
 * BCM56624 B0
 */
int
soc_features_bcm56624_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         flexible_xgport;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56624_b0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    flexible_xgport = FALSE;
    if ((dev_id == BCM56626_DEVICE_ID && rev_id != BCM56626_B0_REV_ID) ||
        (dev_id == BCM56628_DEVICE_ID && rev_id != BCM56628_B0_REV_ID)) {
        flexible_xgport =
            soc_property_get(unit, spn_FLEX_XGPORT, flexible_xgport);
    }

    switch (feature) {
    case soc_feature_xgport_one_xe_six_ge:
    case soc_feature_sample_thresh24:
    case soc_feature_flexible_dma_steps:
        return TRUE;
    case soc_feature_sample_thresh16:
    case soc_feature_use_double_freq_for_ddr_pll:
    case soc_feature_ignore_cmic_xgxs_pll_status:
        return FALSE;
    case soc_feature_flexible_xgport:
        return flexible_xgport;
    default:
        return soc_features_bcm56624_a0(unit, feature);
    }
}
#endif  /* BCM_56624 */

#ifdef  BCM_56680
/*
 * BCM56680 A0
 */
int
soc_features_bcm56680_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56680_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_esm_support:
        return FALSE;
    default:
        return soc_features_bcm56624_a0(unit, feature);
    }
}

/*
 * BCM56680 B0
 */
int
soc_features_bcm56680_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56680_b0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_esm_support:
    case soc_feature_flexible_xgport:
        return FALSE;
    default:
        return soc_features_bcm56624_b0(unit, feature);
    }
}
#endif  /* BCM_56680 */

#ifdef  BCM_56634
/*
 * BCM56634 A0
 */
int
soc_features_bcm56634_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         esm = TRUE;
    int         flex_port;
    soc_info_t  *si = &SOC_INFO(unit);

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56634_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    if (dev_id == 0xb639 || dev_id == 0xb526 || dev_id == 0xb538) {
        esm = FALSE;
    }

    switch (dev_id) {
    case BCM56636_DEVICE_ID:
    case BCM56638_DEVICE_ID:
    case BCM56639_DEVICE_ID:
        flex_port = TRUE;
	break;
    default:
        flex_port = FALSE;
    } 

    switch (feature) {
    case soc_feature_oam:
    case soc_feature_time_support:
    case soc_feature_timesync_support:
    case soc_feature_dcb_type19:
    case soc_feature_gport_service_counters:
    case soc_feature_counter_parity:
    case soc_feature_ip_source_bind:
    case soc_feature_auto_multicast:
    case soc_feature_mpls_failover:
    case soc_feature_embedded_higig:
    case soc_feature_field_qualify_gport:
    case soc_feature_field_action_timestamp:
    case soc_feature_field_action_l2_change:
    case soc_feature_field_action_redirect_ipmc:
    case soc_feature_field_action_redirect_nexthop:
    case soc_feature_field_slice_dest_entity_select:
    case soc_feature_field_virtual_queue:
    case soc_feature_field_vfp_flex_counter:
    case soc_feature_tunnel_protocol_match:
    case soc_feature_ipfix_rate:
    case soc_feature_ipfix_flow_mirror:
    case soc_feature_vlan_queue_map:
    case soc_feature_subport_enhanced:
    case soc_feature_mirror_flexible:
    case soc_feature_qos_profile:
    case soc_feature_fast_egr_cell_count:
    case soc_feature_led_data_offset_a0:
    case soc_feature_lport_tab_profile:
    case soc_feature_failover:
    case soc_feature_rx_timestamp:
    case soc_feature_sysport_remap:
    case soc_feature_timestamp_counter:
    case soc_feature_ser_parity:
    case soc_feature_pkt_tx_align:
    case soc_feature_mpls_enhanced:
        return TRUE;
    case soc_feature_wlan:
    case soc_feature_egr_mirror_true:
    case soc_feature_mim:
    case soc_feature_internal_loopback:
    case soc_feature_mmu_virtual_ports:
        return si->internal_loopback;
    case soc_feature_flex_port:
        return flex_port;
    case soc_feature_esm_support:
        return esm;
    case soc_feature_dcb_type14:
    case soc_feature_use_double_freq_for_ddr_pll:
    case soc_feature_xgport_one_xe_six_ge:
    case soc_feature_flexible_xgport:
        return FALSE;
    case soc_feature_priority_flow_control:
        return (rev_id != BCM56634_A0_REV_ID);
    default:
        return soc_features_bcm56624_b0(unit, feature);
    }
}

/*
 * BCM56634 B0
 */
int
soc_features_bcm56634_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         mim, defip_hole = FALSE, mpls = TRUE, oam = TRUE;
    soc_info_t  *si = &SOC_INFO(unit);

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56634_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);
    mim = si->internal_loopback;    
    if (dev_id == 0xb538) {
        mpls = mim = oam = FALSE;
        defip_hole = TRUE;
    }

    switch (feature) {
    case soc_feature_gmii_clkout:
        return TRUE;
    case soc_feature_l3_defip_hole:  /* Hole in L3_DEFIP block */
        return defip_hole;
    case soc_feature_unimac_tx_crs:
    case soc_feature_esm_rxfifo_resync:
        return TRUE;
    case soc_feature_mpls:
    case soc_feature_mpls_failover:
    case soc_feature_mpls_software_failover:
    case soc_feature_mpls_enhanced:
        return mpls;
    case soc_feature_mim:
        return mim;
    case soc_feature_oam:
        return oam;
    default:
        return soc_features_bcm56634_a0(unit, feature);
    }
}
#endif  /* BCM_56634 */

#ifdef  BCM_56524
/*
 * BCM56524 A0
 */
int
soc_features_bcm56524_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56524_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_esm_support:
        return FALSE;
    case soc_feature_flex_port:
        return (dev_id == BCM56526_DEVICE_ID);
    default:
        return soc_features_bcm56634_a0(unit, feature);
    }
}

/*
 * BCM56524 B0
 */
int
soc_features_bcm56524_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         mpls = TRUE, mim = TRUE, oam = TRUE, defip_hole = TRUE;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56634_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);
    if (dev_id == 0xb534) {
        mpls = mim = oam = FALSE;
    }
    if (dev_id == 0xb630) {
        defip_hole = FALSE;
    }
    switch (feature) {
    case soc_feature_gmii_clkout:
        return TRUE;
    case soc_feature_l3_defip_hole:  /* Hole in L3_DEFIP block */
        return defip_hole;
    case soc_feature_unimac_tx_crs:
        return TRUE;
    case soc_feature_mpls:
    case soc_feature_mpls_failover:
    case soc_feature_mpls_enhanced:
        return mpls;
    case soc_feature_mim:
        return mim;
    case soc_feature_oam:
        return oam;
    default:
        return soc_features_bcm56524_a0(unit, feature);
    }
}
#endif  /* BCM_56524 */

#ifdef  BCM_56685
/*
 * BCM56685 A0
 */
int
soc_features_bcm56685_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56685_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_esm_support:
    case soc_feature_wlan:
        return FALSE;
    default:
        return soc_features_bcm56634_a0(unit, feature);
    }
}

/*
 * BCM56685 B0
 */
int
soc_features_bcm56685_b0(int unit, soc_feature_t feature)
{
    switch (feature) {
    case soc_feature_gmii_clkout:
        return TRUE;
    case soc_feature_unimac_tx_crs:
        return TRUE;
    default:
        return soc_features_bcm56685_a0(unit, feature);
    }
}
#endif  /* BCM_56685 */

#ifdef  BCM_56334
/*
 * BCM56334 A0
 */
int
soc_features_bcm56334_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int             helix3 = FALSE;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56334_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    helix3 = (dev_id == BCM56320_DEVICE_ID || dev_id == BCM56321_DEVICE_ID);

    switch (feature) {
    case soc_feature_esm_support:
    case soc_feature_ipfix:
    case soc_feature_dcb_type14:
    case soc_feature_register_hi:   /* > 32 bits in PBM */
    case soc_feature_table_hi:      /* > 32 bits in PBM */
    case soc_feature_ppa_bypass:
        return FALSE;
    case soc_feature_oam:
    case soc_feature_mim:
    case soc_feature_mpls:
    case soc_feature_mpls_enhanced:
        return !helix3;
    case soc_feature_lpm_tcam:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_l3_ip6:
    case soc_feature_l3_lookup_cmd:
    case soc_feature_l3_sgv:
    case soc_feature_subport:
    case soc_feature_vlan_egr_it_inner_replace:
    case soc_feature_vlan_vp:
        return TRUE;
    case soc_feature_delay_core_pll_lock:
        return (dev_id == BCM56333_DEVICE_ID);
    case soc_feature_dcb_type20:
    case soc_feature_field_slices8:
    case soc_feature_field_meter_pools8:
    case soc_feature_field_action_timestamp:
    case soc_feature_internal_loopback:
    case soc_feature_vlan_queue_map:
    case soc_feature_subport_enhanced:
    case soc_feature_qos_profile:
    case soc_feature_field_action_l2_change:
    case soc_feature_field_action_redirect_nexthop:
    case soc_feature_rx_timestamp:
    case soc_feature_rx_timestamp_upper:
    case soc_feature_led_data_offset_a0:
    case soc_feature_timesync_support:
    case soc_feature_timestamp_counter:
    case soc_feature_time_support:
    case soc_feature_flex_port:
    case soc_feature_lport_tab_profile:
    case soc_feature_field_virtual_queue:
    case soc_feature_gmii_clkout:
    case soc_feature_priority_flow_control:
    case soc_feature_ptp:
    case soc_feature_field_slice_dest_entity_select:
        return TRUE;
    default:
        return soc_features_bcm56624_a0(unit, feature);
    }
}

/*
 * BCM56334 B0
 */
int
soc_features_bcm56334_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56334_b0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    default:
        return soc_features_bcm56334_a0(unit, feature);
    }
}
#endif /* BCM_56334 */

#ifdef BCM_56142

/*
 * BCM56142 A0
 */
int
soc_features_bcm56142_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56142_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
        case soc_feature_vlan_queue_map:
        case soc_feature_field_multi_stage:
        case soc_feature_mpls:
        case soc_feature_mim:
        case soc_feature_oam:
        case soc_feature_field_slices8:
        case soc_feature_subport:
        case soc_feature_subport_enhanced:
        case soc_feature_lpm_prefix_length_max_128:
        case soc_feature_class_based_learning:
        case soc_feature_urpf:
        case soc_feature_l3_ingress_interface:
        case soc_feature_ipmc_repl_penultimate:
        case soc_feature_trunk_group_size:
        case soc_feature_xgs1_mirror:
        case soc_feature_failover:
        case soc_feature_vlan_vp:
            return FALSE;
        case soc_feature_field_slices4:
        case soc_feature_linear_drr_weight:
        case soc_feature_egr_dscp_map_per_port:
        case soc_feature_port_trunk_index:
        case soc_feature_eee:
            return TRUE;
        default:
            return soc_features_bcm56334_a0(unit, feature);
    }
}
#endif /* BCM_56142 */

#if defined(BCM_56218) || defined(BCM_56224) || defined(BCM_53314)
/*
 * BCM56218 A0
 */
int
soc_features_bcm56218_a0(int unit, soc_feature_t feature)
{
    uint32  rcpu = SOC_IS_RCPU_UNIT(unit);
    int     l3_disable, field_slices4, field_slices2, fe_ports;
    uint16  dev_id;
    uint8   rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56218_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);
   
    l3_disable = (dev_id == BCM53716_DEVICE_ID);

    field_slices4 = (dev_id == BCM53718_DEVICE_ID ||
                     dev_id == BCM53714_DEVICE_ID);

    field_slices2 = (dev_id == BCM53716_DEVICE_ID);

    fe_ports = (dev_id == BCM56014_DEVICE_ID ||
                dev_id == BCM56018_DEVICE_ID ||
                dev_id == BCM56024_DEVICE_ID ||
                dev_id == BCM56025_DEVICE_ID);
    
    switch (feature) {
        case soc_feature_l2x_parity:
            if (SAL_BOOT_BCMSIM) {
                return FALSE;
            }
            /* Fall through */
        case soc_feature_parity_err_tocpu:
        case soc_feature_nip_l3_err_tocpu:
        case soc_feature_meter_adjust:      /* Adjust for IPG */
        case soc_feature_src_modid_blk:
        case soc_feature_dcb_type12:
        case soc_feature_schmsg_alias:
        case soc_feature_l2_lookup_cmd:
        case soc_feature_l2_user_table:
        case soc_feature_aging_extended:
        case soc_feature_arl_hashed:
        case soc_feature_l2_modfifo:
        case soc_feature_mdio_enhanced:
        case soc_feature_schan_hw_timeout:
        case soc_feature_cpuport_switched:
        case soc_feature_cpuport_mirror:
        case soc_feature_fe_gig_macs:
        case soc_feature_trimac:
        case soc_feature_cos_rx_dma:
        case soc_feature_xgxs_lcpll:
        case soc_feature_dodeca_serdes:
        case soc_feature_rxdma_cleanup:
        case soc_feature_fe_maxframe:   /* fe_maxfr = MAXFR + 1 */
        case soc_feature_vlan_mc_flood_ctrl: /* Per VLAN PFM support */

        case soc_feature_trunk_extended:
        case soc_feature_l2_hashed:
        case soc_feature_l2_lookup_retry:
        case soc_feature_arl_insert_ovr:
        case soc_feature_cfap_pool:
        case soc_feature_stg_xgs:
        case soc_feature_stg:
        case soc_feature_stack_my_modid:
        case soc_feature_remap_ut_prio:
        case soc_feature_led_proc:
        case soc_feature_ingress_metering:
        case soc_feature_egress_metering:
        case soc_feature_stat_jumbo_adj:
        case soc_feature_stat_xgs3:
        case soc_feature_color:
        case soc_feature_dscp:
        case soc_feature_dscp_map_mode_all:
        case soc_feature_register_hi:   /* > 32 bits in PBM */
        case soc_feature_table_hi:      /* > 32 bits in PBM */
        case soc_feature_port_egr_block_ctl:
        case soc_feature_higig2:
        case soc_feature_storm_control:
        case soc_feature_hw_stats_calc:
        case soc_feature_cpu_proto_prio:
        case soc_feature_linear_drr_weight:     /* Linear DRR Weight calc */
        case soc_feature_igmp_mld_support:      /* IGMP/MLD snooping support */
        case soc_feature_enhanced_dos_ctrl:
        case soc_feature_src_mac_group:
        case soc_feature_tunnel_6to4_secure:
        /* Raptor supports DSCP map per port */
        case soc_feature_dscp_map_per_port: 
        case soc_feature_src_trunk_port_bridge:
        case soc_feature_big_icmpv6_ping_check:
        case soc_feature_src_modid_blk_ucast_override:
        case soc_feature_src_modid_blk_opcode_override:
        case soc_feature_egress_blk_ucast_override:
        case soc_feature_rcpu_1:
        case soc_feature_phy_cl45:
        case soc_feature_static_pfm:
        case soc_feature_sgmii_autoneg:
        case soc_feature_module_loopback:
        case soc_feature_sample_thresh16:
        case soc_feature_extended_pci_error:
        case soc_feature_directed_mirror_only:
        case soc_feature_tunnel_dscp_trust:
            return TRUE;
        case soc_feature_post:
            return (rev_id <= BCM56018_A1_REV_ID);

        case soc_feature_field:         /* Field Processor */
        case soc_feature_field_mirror_ovr:
        case soc_feature_field_udf_higig2:
        case soc_feature_field_udf_ethertype:
        case soc_feature_field_comb_read:
        case soc_feature_field_wide:
        case soc_feature_field_slice_enable:
        case soc_feature_field_cos:
        case soc_feature_field_color_indep:
        case soc_feature_field_qual_drop:
        case soc_feature_field_qual_IpType:
        case soc_feature_field_qual_Ip6High:
        case soc_feature_field_ingress_ipbm:
        case soc_feature_field_virtual_slice_group:
            return TRUE;

        case soc_feature_field_slices8:
            return !field_slices2 && !field_slices4; /* If not 2, 4, it is 8 */

        case soc_feature_field_slices4:
            return field_slices4;
	    
        case soc_feature_field_slices2:
            return field_slices2;
	    
        case soc_feature_l3:
        case soc_feature_l3_ip6:
        case soc_feature_fp_based_routing: 
            return !l3_disable;

        case soc_feature_table_dma:
        case soc_feature_tslam_dma:
        case soc_feature_stat_dma:
        case soc_feature_cpuport_stat_dma:
            return (rcpu) ? FALSE : TRUE;

        case soc_feature_fe_ports:
            return fe_ports;
       
        case soc_feature_stat_dma_error_ack:
        case soc_feature_system_mac_learn_limit:
        case soc_feature_sample_offset8:
        default:
            return FALSE;

    }
}
#endif  /* BCM_56218 */

#ifdef  BCM_56224
/*
 * BCM56224 A0
 */
int
soc_features_bcm56224_a0(int unit, soc_feature_t feature)
{
    int     l3_disable, field_slices4;
    uint16  dev_id;
    uint8   rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56224_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);
   
    l3_disable = (dev_id == BCM56225_DEVICE_ID ||
                  dev_id == BCM56227_DEVICE_ID ||
                  dev_id == BCM56229_DEVICE_ID ||
                  dev_id == BCM56025_DEVICE_ID ||
                  dev_id == BCM53724_DEVICE_ID ||
                  dev_id == BCM53726_DEVICE_ID); 

    field_slices4 = (dev_id == BCM53724_DEVICE_ID ||
                     dev_id == BCM53726_DEVICE_ID);

    switch (feature) {
        case soc_feature_l2x_parity:
            if (SAL_BOOT_BCMSIM) {
                return FALSE;
            } else {
                return TRUE;
            }
        case soc_feature_l3x_parity:
        case soc_feature_l3defip_parity:
            if (SAL_BOOT_BCMSIM) {
                return FALSE;
            }
            /* Fall through */
        case soc_feature_lpm_tcam:
        case soc_feature_ip_mcast:
	case soc_feature_ip_mcast_repl:
        case soc_feature_l3:
        case soc_feature_l3_ip6:
        case soc_feature_l3_lookup_cmd:
        case soc_feature_l3_sgv:
        case soc_feature_ipmc_lookup:
            return !l3_disable;
        case soc_feature_l3mtu_fail_tocpu:
            return FALSE;
        case soc_feature_unimac:
        case soc_feature_dual_hash:
	case soc_feature_dcb_type15: 
	case soc_feature_vlan_ctrl:
	case soc_feature_color:
	case soc_feature_force_forward:
        case soc_feature_color_prio_map:
        case soc_feature_hg_trunk_override:
        case soc_feature_hg_trunking:
        case soc_feature_port_trunk_index:
        case soc_feature_vlan_translation:  
        case soc_feature_egr_vlan_check:
        case soc_feature_xgxs_lcpll:
        case soc_feature_bucket_support:
        case soc_feature_ip_ep_mem_parity:
        case soc_feature_mac_learn_limit:
#ifdef INCLUDE_RCPU
        case soc_feature_rcpu_priority:
#endif /* INCLUDE_RCPU */
            return TRUE;
        case soc_feature_field_slices4:
            return field_slices4;
        case soc_feature_register_hi:   /* > 32 bits in PBM */
        case soc_feature_table_hi:      /* > 32 bits in PBM */
        case soc_feature_trimac:
	case soc_feature_dcb_type12:
        case soc_feature_field_slices8:
        case soc_feature_fp_based_routing:
	    return FALSE;
        case soc_feature_post:
            return ((dev_id == BCM56024_DEVICE_ID) || 
                    (dev_id == BCM56025_DEVICE_ID));
        case soc_feature_mdio_setup:
            return (((dev_id == BCM56024_DEVICE_ID) || 
                    (dev_id == BCM56025_DEVICE_ID)) && 
                    (rev_id >= BCM56024_B0_REV_ID));
	default:
	    return soc_features_bcm56218_a0(unit, feature);
    }
}

int
soc_features_bcm56224_b0(int unit, soc_feature_t feature)
{
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56224_b0"));
    switch (feature) {
	case soc_feature_dcb_type18: 
        case soc_feature_fast_egr_cell_count:
            return TRUE;
        case soc_feature_dcb_type15:
        case soc_feature_post:
	    return FALSE;
	default:
	    return soc_features_bcm56224_a0(unit, feature);
    }
}
#endif /* BCM_56224 */

#ifdef  BCM_53314
/*
 * BCM53314 A0
 */
int
soc_features_bcm53314_a0(int unit, soc_feature_t feature)
{
    int     field_slices4, hawkeye_a0_war;
    uint16  dev_id;
    uint8   rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 53314_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);
   
    field_slices4 = (dev_id == BCM53312_DEVICE_ID ||
                     dev_id == BCM53313_DEVICE_ID ||
                     dev_id == BCM53314_DEVICE_ID ||
                     dev_id == BCM53322_DEVICE_ID ||
                     dev_id == BCM53323_DEVICE_ID ||
                     dev_id == BCM53324_DEVICE_ID );

    hawkeye_a0_war = (rev_id == BCM53314_A0_REV_ID);

    switch (feature) {
        case soc_feature_l2x_parity:
            if (SAL_BOOT_BCMSIM) {
                return FALSE;
            } else {
                return TRUE;
            }
        case soc_feature_l3x_parity:
        case soc_feature_l3defip_parity:
        case soc_feature_hg_trunk_override:
        case soc_feature_hg_trunking:
            return FALSE;
        case soc_feature_l3:
        case soc_feature_l3_ip6:
        case soc_feature_fp_based_routing: 
        case soc_feature_fp_routing_hk:
            return TRUE;            
        case soc_feature_lpm_tcam:
        case soc_feature_ip_mcast:
	case soc_feature_ip_mcast_repl:
        case soc_feature_l3_lookup_cmd:
        case soc_feature_l3_sgv:
        case soc_feature_l3mtu_fail_tocpu:
        case soc_feature_ipmc_lookup:
            return FALSE;
        case soc_feature_unimac:
        case soc_feature_dual_hash:
	case soc_feature_dcb_type17: 
	case soc_feature_vlan_ctrl:
	case soc_feature_color:
	case soc_feature_force_forward:
        case soc_feature_color_prio_map:
        case soc_feature_port_trunk_index:
        case soc_feature_vlan_translation:  
        case soc_feature_egr_vlan_check:
        case soc_feature_bucket_support:
        case soc_feature_ip_ep_mem_parity:
        case soc_feature_fast_egr_cell_count:
#ifdef INCLUDE_RCPU
        case soc_feature_rcpu_priority:
#endif /* INCLUDE_RCPU */
        case soc_feature_phy_lb_needed_in_mac_lb:
        case soc_feature_rx_timestamp:
        case soc_feature_mac_learn_limit:
        case soc_feature_system_mac_learn_limit:
        case soc_feature_timesync_support:
            return TRUE;
        case soc_feature_field_slices4:
            return field_slices4;
        case soc_feature_xgxs_lcpll:
        case soc_feature_register_hi:   /* > 32 bits in PBM */
        case soc_feature_table_hi:      /* > 32 bits in PBM */
        case soc_feature_trimac:
        case soc_feature_dcb_type12:
        case soc_feature_dcb_type15:
        case soc_feature_field_slices8:
        case soc_feature_static_pfm:
	    return FALSE;
        case soc_feature_hawkeye_a0_war:
	    return hawkeye_a0_war;
	default:
	    return soc_features_bcm56218_a0(unit, feature);
    }
}

/*
 * BCM53324 A0
 */
int
soc_features_bcm53324_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 53324_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
        case soc_feature_field_slices4:
        case soc_feature_eee:
            return TRUE;
        case soc_feature_hawkeye_a0_war:
            return FALSE;
        default:
            return soc_features_bcm53314_a0(unit, feature);
    }
}
#endif /* BCM_53314 */

#ifdef  BCM_56514
/*
 * BCM56514 A0
 */
int
soc_features_bcm56514_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         l3disable;
#ifndef EXCLUDE_BCM56324
    int         helix2;
#endif /* EXCLUDE_BCM56324 */

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56514_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

#ifndef EXCLUDE_BCM56324
    helix2 = (dev_id == BCM56324_DEVICE_ID ||
              dev_id == BCM56322_DEVICE_ID);
#endif /* EXCLUDE_BCM56324 */

    l3disable = (dev_id == BCM56516_DEVICE_ID ||
                 dev_id == BCM56517_DEVICE_ID || 
                 dev_id == BCM56518_DEVICE_ID ||
                 dev_id == BCM56519_DEVICE_ID);

    switch (feature) {
    case soc_feature_l3_defip_map:  /* Map out unused L3_DEFIP blocks */
    case soc_feature_status_link_fail:
    case soc_feature_egr_vlan_pfm:  /* MH PFM control per VLAN FB A0 */
    case soc_feature_ipmc_repl_freeze:
    case soc_feature_basic_dos_ctrl:
    case soc_feature_proto_pkt_ctrl:
    case soc_feature_dcb_type9:
    case soc_feature_unknown_ipmc_tocpu:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_asf_no_10_100:
    case soc_feature_txdma_purge:
    case soc_feature_l3_defip_ecmp_count:
        return FALSE;  /* Override the 56504 B0 values*/
    case soc_feature_cpu_proto_prio:
    case soc_feature_enhanced_dos_ctrl:
    case soc_feature_igmp_mld_support:   /* IGMP/MLD snooping support */
    case soc_feature_vlan_ctrl:          /* per VLAN property control */  
    case soc_feature_trunk_group_size:   /* trunk group size support  */
    case soc_feature_dcb_type13:
    case soc_feature_hw_stats_calc:
    case soc_feature_dscp_map_per_port:
    case soc_feature_src_modid_blk_opcode_override: 
    case soc_feature_dual_hash: 
    case soc_feature_field_multi_stage:
    case soc_feature_field_virtual_slice_group:
    case soc_feature_field_qual_Ip6High:
    case soc_feature_field_intraslice_double_wide:
    case soc_feature_linear_drr_weight:     /* Linear DRR Weight calc */
    case soc_feature_src_trunk_port_bridge:
    case soc_feature_lmd:
    case soc_feature_field_intraslice_basic_key: 
    case soc_feature_force_forward:
    case soc_feature_color_prio_map:
    case soc_feature_field_egress_global_counters:
    case soc_feature_src_mac_group:
    case soc_feature_packet_rate_limit:
    case soc_feature_field_action_l2_change:
        return TRUE;
#ifndef EXCLUDE_BCM56324
    case soc_feature_field_slices8:
        return helix2;
    case soc_feature_urpf:
        return !helix2 && !l3disable;
    case soc_feature_l3defip_parity:
        if (helix2) {
            return FALSE;
        }
        /* Fall through */
#else
    case soc_feature_urpf:
        return !l3disable;
#endif /* EXCLUDE_BCM56324 */
    default:
        return soc_features_bcm56504_b0(unit, feature);
    }
}
#endif  /* BCM_56514 */

#ifdef  BCM_56820
/*
 * BCM56820 A0
 */
int
soc_features_bcm56820_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    int         bypass_mode;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56820_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    bypass_mode = 
        soc_property_get(unit, spn_SWITCH_BYPASS_MODE,
                         SOC_SWITCH_BYPASS_MODE_NONE);

    switch (feature) {
    case soc_feature_dcb_type16:
    case soc_feature_unimac:
    case soc_feature_xgxs_v7:
    case soc_feature_trunk_group_overlay:
    case soc_feature_trunk_group_size:
    case soc_feature_vlan_ctrl:          /* per VLAN property control */  
    case soc_feature_force_forward:
    case soc_feature_hw_stats_calc:
    case soc_feature_dual_hash:
    case soc_feature_generic_table_ops:
    case soc_feature_igmp_mld_support:
    case soc_feature_mem_push_pop:
    case soc_feature_src_mac_group:
    case soc_feature_higig_lookup:
    case soc_feature_lpm_prefix_length_max_128:
    case soc_feature_fifo_dma:
    case soc_feature_l2_pending:
    case soc_feature_packet_rate_limit:
    case soc_feature_sample_thresh16:
    case soc_feature_enhanced_dos_ctrl:
    case soc_feature_status_link_fail:
    case soc_feature_src_trunk_port_bridge:
    case soc_feature_egr_dscp_map_per_port:
    case soc_feature_port_lag_failover:
    case soc_feature_l3_dynamic_ecmp_group:
#ifdef INCLUDE_RCPU
    case soc_feature_rcpu_1:
    case soc_feature_rcpu_priority:
    case soc_feature_rcpu_tc_mapping:
#endif /* INCLUDE_RCPU */
        return TRUE;
    case soc_feature_lpm_tcam:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_l3_ip6:
    case soc_feature_l3_lookup_cmd:
    case soc_feature_l3_sgv:
    case soc_feature_field_egress_flexible_v6_key:
    case soc_feature_field_multi_stage:
    case soc_feature_field_egress_global_counters:
    case soc_feature_vlan_translation:  
    case soc_feature_untagged_vt_miss:
    case soc_feature_egr_l3_mtu:
    case soc_feature_ipmc_group_mtu:
    case soc_feature_tunnel_gre:
    case soc_feature_tunnel_any_in_6:
    case soc_feature_tunnel_6to4_secure:
    case soc_feature_unknown_ipmc_tocpu:
    case soc_feature_class_based_learning:
    case soc_feature_storm_control:
    case soc_feature_vlan_translation_range:
    case soc_feature_vlan_action:
    case soc_feature_color:
    case soc_feature_color_prio_map:
    case soc_feature_subport:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
        /* In bypass mode, all of these features are unusable */
        return bypass_mode == SOC_SWITCH_BYPASS_MODE_NONE;
   case soc_feature_field:
    case soc_feature_field_virtual_slice_group:
    case soc_feature_field_intraslice_double_wide:
    case soc_feature_field_ing_egr_separate_packet_byte_counters:
    case soc_feature_field_ingress_global_meter_pools:
    case soc_feature_field_ingress_two_slice_types:
    case soc_feature_field_slices12:
    case soc_feature_field_meter_pools4:
    case soc_feature_field_ingress_ipbm:
        /* In L3 + FP bypass mode, IFP is unavailable */
        return bypass_mode != SOC_SWITCH_BYPASS_MODE_L3_AND_FP;
    case soc_feature_xgxs_v6:
    case soc_feature_dcb_type11:
    case soc_feature_fe_gig_macs:
    case soc_feature_stat_dma_error_ack:
    case soc_feature_ctr_xaui_activity:
    case soc_feature_field_slices8:
    case soc_feature_trimac:
    case soc_feature_sample_offset8:
        return FALSE;
    case soc_feature_ignore_cmic_xgxs_pll_status:
    case soc_feature_delay_core_pll_lock:
        return (rev_id == BCM56820_A0_REV_ID);
    case soc_feature_priority_flow_control:
        return (rev_id != BCM56820_A0_REV_ID);
    case soc_feature_pkt_tx_align:
        return (rev_id >= BCM56820_B0_REV_ID);
    default:
        return soc_features_bcm56800_a0(unit, feature);
    }
}
#endif /* BCM_56820 */

#ifdef  BCM_56840
/*
 * BCM56840 A0
 */
int
soc_features_bcm56840_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56840_a0"));

    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_dcb_type19:
    case soc_feature_multi_sbus_cmds:
    case soc_feature_esm_support:
    case soc_feature_ipfix:
    case soc_feature_ipfix_rate:
    case soc_feature_ipfix_flow_mirror:
    case soc_feature_mac_learn_limit:
    case soc_feature_system_mac_learn_limit:
    case soc_feature_oam:
    case soc_feature_internal_loopback:
    case soc_feature_embedded_higig:
    case soc_feature_trunk_egress:
    case soc_feature_bigmac_rxcnt_bug:
    case soc_feature_ppa_bypass:
    case soc_feature_xgs1_mirror:
    case soc_feature_sysport_remap:
    case soc_feature_timestamp_counter:
        return FALSE;
    case soc_feature_field_intraslice_double_wide:
    case soc_feature_dcb_type21:
    case soc_feature_xmac:
    case soc_feature_logical_port_num:
    case soc_feature_mmu_config_property:
    case soc_feature_l2_bulk_control:
    case soc_feature_l2_bulk_bypass_replace:
    case soc_feature_two_ingress_pipes:
    case soc_feature_generic_counters:
    case soc_feature_niv:
    case soc_feature_mim:
    case soc_feature_hg_trunk_override_profile:
    case soc_feature_hg_dlb:
    case soc_feature_ets:
    case soc_feature_qcn:
    case soc_feature_priority_flow_control:
    case soc_feature_vlan_vp:
    case soc_feature_vp_group_ingress_vlan_membership:
    case soc_feature_eee:
    case soc_feature_ctr_xaui_activity:
    case soc_feature_vlan_egr_it_inner_replace:
    case soc_feature_flex_port:
    case soc_feature_ser_parity:
    case soc_feature_xy_tcam:
    case soc_feature_vlan_pri_cfi_action:
    case soc_feature_vlan_copy_action:
    case soc_feature_modport_map_dest_is_port_or_trunk:
    case soc_feature_mirror_encap_profile:
    case soc_feature_directed_mirror_only:
    case soc_feature_ptp:
        return TRUE;
    case soc_feature_field_egress_flexible_v6_key:
    case soc_feature_field_multi_stage:
    case soc_feature_field_egress_global_counters:
    case soc_feature_vlan_translation:  
    case soc_feature_untagged_vt_miss:
    case soc_feature_tunnel_gre:
    case soc_feature_tunnel_any_in_6:
    case soc_feature_tunnel_6to4_secure:
    case soc_feature_class_based_learning:
    case soc_feature_storm_control:
    case soc_feature_vlan_translation_range:
    case soc_feature_vlan_action:
    case soc_feature_color:
    case soc_feature_color_prio_map:
    case soc_feature_dscp:
    case soc_feature_dscp_map_per_port:
    case soc_feature_field:
    case soc_feature_field_virtual_slice_group:
    case soc_feature_field_ing_egr_separate_packet_byte_counters:
    case soc_feature_field_ingress_global_meter_pools:
    case soc_feature_field_ingress_two_slice_types:
    case soc_feature_field_slices10:
    case soc_feature_field_meter_pools4:
    case soc_feature_field_ingress_ipbm:
    case soc_feature_src_modid_base_index:
    case soc_feature_field_action_redirect_ecmp:
    case soc_feature_field_slice_dest_entity_select:
        if (SOC_IS_XGS_FABRIC(unit)) {
            return SOC_SWITCH_BYPASS_MODE(unit) == SOC_SWITCH_BYPASS_MODE_NONE;
        } else {
            return TRUE;
        }
        break;
    case soc_feature_lpm_tcam:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_l3_ingress_interface:
    case soc_feature_trill:
    case soc_feature_l3_ip6:
    case soc_feature_l3_lookup_cmd:
    case soc_feature_l3_sgv:
    case soc_feature_wesp:
    case soc_feature_mpls:
    case soc_feature_subport:
        if (SOC_IS_XGS_FABRIC(unit)) {
            return FALSE;
        } else {
            return TRUE;
        }
    default:
        return soc_features_bcm56634_a0(unit, feature);
    }
}

/*
 * BCM56840 B0
 */
int
soc_features_bcm56840_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56840_b0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_vp_group_egress_vlan_membership:
    case soc_feature_xy_tcam_direct:
    case soc_feature_field_qual_my_station_hit:
        return TRUE;
    case soc_feature_l3_ecmp_1k_groups:
        return (dev_id == BCM56842_DEVICE_ID ||
                dev_id == BCM56844_DEVICE_ID ||
                dev_id == BCM56846_DEVICE_ID);
    case soc_feature_l2_bulk_bypass_replace:
        return FALSE;
    default:
        return soc_features_bcm56840_a0(unit, feature);
    }
}
#endif  /* BCM_56840 */

#ifdef  BCM_56725
/*
 * BCM56725 A0
 */
int
soc_features_bcm56725_a0(int unit, soc_feature_t feature)
{
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56725_a0"));
    switch (feature) {
    case soc_feature_unimac:
    case soc_feature_class_based_learning:
    case soc_feature_lpm_prefix_length_max_128:
    case soc_feature_tunnel_gre:
    case soc_feature_tunnel_any_in_6:
    case soc_feature_fifo_dma:
    case soc_feature_subport:
    case soc_feature_lpm_tcam:
    case soc_feature_ip_mcast:
    case soc_feature_ip_mcast_repl:
    case soc_feature_l3:
    case soc_feature_l3_ip6:
    case soc_feature_l3_lookup_cmd:
    case soc_feature_l3_sgv:
        return FALSE;
    default:
        return soc_features_bcm56820_a0(unit, feature);
    }
}
#endif /* BCM_56725 */


#ifdef	BCM_5324
/*
 * BCM5324 XX
 */
int
soc_features_bcm5324_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5324_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    case soc_feature_mac_learn_limit:
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_arl_mode_control:
        return TRUE;
    default:
        return FALSE;
    }
}

int
soc_features_bcm5324_a1(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5324_a1"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    case soc_feature_mac_learn_limit:
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_arl_mode_control:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_5324 */


#ifdef	BCM_5396

int
soc_features_bcm5396_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5396_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    /* soc_feature_dodeca_serdes: for fiber with SerDes + PHY_PassThrough */
    case soc_feature_dodeca_serdes: 
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_no_stat_mib:
case soc_feature_sgmii_autoneg:        
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_5396 */

#ifdef	BCM_5389

int
soc_features_bcm5389_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5389_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:   
    /* soc_feature_dodeca_serdes: for fiber with SerDes + PHY_PassThrough */
    case soc_feature_dodeca_serdes: 
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
    case soc_feature_sgmii_autoneg:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_5389 */

#ifdef	BCM_5398

int
soc_features_bcm5398_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5398_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_arl_mode_control:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_5398 */

#ifdef	BCM_5348
/*
 * BCM5324 XX
 */
int
soc_features_bcm5348_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5348_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    case soc_feature_mac_learn_limit:
    /* soc_feature_dodeca_serdes: for fiber with SerDes + PHY_PassThrough */
    case soc_feature_dodeca_serdes:         
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_field:
    case soc_feature_field_slice_enable:
    case soc_feature_arl_mode_control:
    case soc_feature_robo_ge_serdes_mac_autosync:             
    case soc_feature_sgmii_autoneg:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_5348 */

#ifdef	BCM_5397

int
soc_features_bcm5397_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5397_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_arl_mode_control:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_5397 */

#ifdef	BCM_5347
int
soc_features_bcm5347_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5347_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    case soc_feature_mac_learn_limit:
    /* soc_feature_dodeca_serdes: for fiber with SerDes + PHY_PassThrough */
    case soc_feature_dodeca_serdes: 
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_field:
    case soc_feature_field_slice_enable:
    case soc_feature_arl_mode_control:
    case soc_feature_robo_ge_serdes_mac_autosync:             
    case soc_feature_sgmii_autoneg:        
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_5347 */
#ifdef	BCM_5395
int
soc_features_bcm5395_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 5395_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_field:
    case soc_feature_field_slice_enable:
    case soc_feature_arl_mode_control:
    case soc_feature_hw_dos_prev:
    case soc_feature_tag_enforcement:
    case soc_feature_igmp_mld_support:
    case soc_feature_eav_support:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_5395 */

#ifdef	BCM_53242
int
soc_features_bcm53242_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 53242_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    case soc_feature_mac_learn_limit:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_field:
    case soc_feature_arl_mode_control:
    case soc_feature_tag_enforcement:
    case soc_feature_vlan_translation:
    case soc_feature_igmp_mld_support:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_53242 */

#ifdef	BCM_FE2000_A0
/*
 * BCM88020 A0
 */
int
soc_features_bcm88020_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_dodeca_serdes: 
    case soc_feature_xgxs_v5:
    case soc_feature_lmd:
    case soc_feature_xgxs_lcpll:
    case soc_feature_xport_convertible: 
    case soc_feature_phy_cl45:
    case soc_feature_higig2:
    case soc_feature_field:
        return TRUE;
    default:
        return FALSE;
    }
}
/*
 * BCM88025 A0
 */
int
soc_features_bcm88025_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (feature) {
    case soc_feature_dodeca_serdes: 
    case soc_feature_xgxs_v5:
    case soc_feature_lmd:
    case soc_feature_xgxs_lcpll:
    case soc_feature_xport_convertible: 
    case soc_feature_phy_cl45:
    case soc_feature_higig2:
    case soc_feature_field:
        return TRUE;
    default:
        return FALSE;
    }
}

#endif  /* BCM_FE2000_A0 */


#ifdef	BCM_88230
/*
 * BCM88230 A0
 */
int
soc_features_bcm88230_a0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    uint32      tme_mode, hybrid_mode;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    /* hardcoded soc features */
    switch (feature) {
	case soc_feature_dcb_type19:
	case soc_feature_schmsg_alias:
	case soc_feature_table_dma:
	case soc_feature_tslam_dma:
	case soc_feature_higig2:
	case soc_feature_mem_push_pop:
	case soc_feature_fifo_dma:
        case soc_feature_mc_group_ability:
        case soc_feature_packet_adj_len:
        case soc_feature_multi_sbus_cmds:
        case soc_feature_extended_cmic_error:
        case soc_feature_unimac: /* for lab bringup only */
        case soc_feature_phy_cl45:
        case soc_feature_mdio_enhanced:
        case soc_feature_sgmii_autoneg:
        case soc_feature_ingress_size_templates:
	    return TRUE;
	    break;
	default:
	    break;
    }

    /* soc features depends on configuration */
    if (SOC_CONTROL(unit) == NULL) {
        return(FALSE);
    }
    if (SOC_SBX_CONTROL(unit) == NULL) {
        return(FALSE);
    }
    if (SOC_SBX_CFG(unit) == NULL) {
        return(FALSE);
    }

    tme_mode = soc_property_get(unit, spn_QE_TME_MODE, SOC_SBX_CFG(unit)->bTmeMode);
    hybrid_mode = soc_property_get(unit, spn_HYBRID_MODE, SOC_SBX_CFG(unit)->bHybridMode);

    switch (feature) {
	case soc_feature_discard_ability:
            return( ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) ||
		     (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
		     (SOC_SBX_CFG(unit)->bTmeMode ==  SOC_SBX_QE_MODE_HYBRID)) ?
		    TRUE : FALSE );
	    break;
        case soc_feature_egress_metering:
            /* Support is available for egress shaping */
            SOC_DEBUG_PRINT((DK_VERBOSE , "rate egress_metering feature available\n"));
            return( TRUE );
            break;
        case soc_feature_cosq_gport_stat_ability:
            /* Support is available for gport statistics */
            SOC_DEBUG_PRINT((DK_VERBOSE , "gport statistics feature available\n"));
            return( TRUE );
            break;
        case soc_feature_standalone:
            if ((tme_mode == SOC_SBX_QE_MODE_TME) ||
                (tme_mode == SOC_SBX_QE_MODE_TME_BYPASS)) {
                return TRUE;
            } else {
                return FALSE;
            } 
            break;
        case soc_feature_hybrid:
            return( ( (hybrid_mode == TRUE) &&
                      (tme_mode != TRUE /* SOC_SBX_QE_MODE_TME */) ) ? TRUE : FALSE);
            break;
        case soc_feature_node:
            return(TRUE);
            break;
        case soc_feature_node_hybrid:
            return( ((tme_mode == SOC_SBX_QE_MODE_HYBRID) ? TRUE : FALSE) );
            break;
        case soc_feature_egr_independent_fc:
            switch (SOC_SBX_CFG(unit)->uFabricConfig) {
                case SOC_SBX_SYSTEM_CFG_VPORT:
                case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
                case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
		    return((SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl == FALSE) ? FALSE : TRUE);
                    break;

                case SOC_SBX_SYSTEM_CFG_DMODE:
                default:
                    return(FALSE);
                    break;
            }
            break;
        case soc_feature_egr_multicast_independent_fc:
	    return((SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) ? FALSE : TRUE);
            break;
	case soc_feature_discard_ability_color_black:
	    return (TRUE);
	    break;
        case soc_feature_distribution_ability:
	    /* ESET need to be awared for FIC traffic for node type */
	    return ( ((tme_mode == SOC_SBX_QE_MODE_TME) ||
		      (tme_mode == SOC_SBX_QE_MODE_TME_BYPASS))
		      ? FALSE : TRUE );
	    break;
	default:
	    return FALSE;
    }
}

/*
 * BCM88230 B0
 */
int
soc_features_bcm88230_b0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    uint32      tme_mode, hybrid_mode;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    /* hardcoded soc features */
    switch (feature) {
	/* case soc_feature_dcb_type27:*/
	case soc_feature_dcb_type19:
	case soc_feature_schmsg_alias:
	case soc_feature_table_dma:
	case soc_feature_tslam_dma:
	case soc_feature_higig2:
	case soc_feature_mem_push_pop:
	case soc_feature_fifo_dma:
        case soc_feature_mc_group_ability:
        case soc_feature_packet_adj_len:
        case soc_feature_multi_sbus_cmds:
        case soc_feature_extended_cmic_error:
        case soc_feature_unimac: /* for lab bringup only */
        case soc_feature_phy_cl45:
        case soc_feature_mdio_enhanced:
        case soc_feature_sgmii_autoneg:
        case soc_feature_ingress_size_templates:
        case soc_feature_priority_flow_control:
	    return TRUE;
	    break;
	default:
	    break;
    }

    /* soc features depends on configuration */
    if (SOC_CONTROL(unit) == NULL) {
        return(FALSE);
    }
    if (SOC_SBX_CONTROL(unit) == NULL) {
        return(FALSE);
    }
    if (SOC_SBX_CFG(unit) == NULL) {
        return(FALSE);
    }

    tme_mode = soc_property_get(unit, spn_QE_TME_MODE, SOC_SBX_CFG(unit)->bTmeMode);
    hybrid_mode = soc_property_get(unit, spn_HYBRID_MODE, SOC_SBX_CFG(unit)->bHybridMode);

    switch (feature) {
	case soc_feature_discard_ability:
            return( ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) ||
		     (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
		     (SOC_SBX_CFG(unit)->bTmeMode ==  SOC_SBX_QE_MODE_HYBRID)) ?
		    TRUE : FALSE );
	    break;
        case soc_feature_egress_metering:
            /* Support is available for egress shaping */
            SOC_DEBUG_PRINT((DK_VERBOSE , "rate egress_metering feature available\n"));
            return( TRUE );
            break;
        case soc_feature_cosq_gport_stat_ability:
            /* Support is available for gport statistics */
            SOC_DEBUG_PRINT((DK_VERBOSE , "gport statistics feature available\n"));
            return( TRUE );
            break;
        case soc_feature_standalone:
            if ((tme_mode == SOC_SBX_QE_MODE_TME) ||
                (tme_mode == SOC_SBX_QE_MODE_TME_BYPASS)) {
                return TRUE;
            } else {
                return FALSE;
            } 
            break;
        case soc_feature_hybrid:
            return( ( (hybrid_mode == TRUE) &&
                      (tme_mode != TRUE /* SOC_SBX_QE_MODE_TME */) ) ? TRUE : FALSE);
            break;
        case soc_feature_node:
            return(TRUE);
            break;
        case soc_feature_node_hybrid:
            return( ((tme_mode == SOC_SBX_QE_MODE_HYBRID) ? TRUE : FALSE) );
            break;
        case soc_feature_egr_independent_fc:
            switch (SOC_SBX_CFG(unit)->uFabricConfig) {
                case SOC_SBX_SYSTEM_CFG_VPORT:
                case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
                case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
                    return((SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl == FALSE) ? FALSE : TRUE);
                    break;

                case SOC_SBX_SYSTEM_CFG_DMODE:
                default:
                    return(FALSE);
                    break;
            }
            break;
        case soc_feature_egr_multicast_independent_fc:
	    return((SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) ? FALSE : TRUE);
            break;
	case soc_feature_discard_ability_color_black:
	    return (TRUE);
	    break;
        case soc_feature_distribution_ability:
	    /* ESET need to be awared for FIC traffic for node type */
	    return ( ((tme_mode == SOC_SBX_QE_MODE_TME) ||
		      (tme_mode == SOC_SBX_QE_MODE_TME_BYPASS))
		      ? FALSE : TRUE );
	    break;
	default:
	    return FALSE;
    }
}

/*
 * BCM88230 C0
 */
int
soc_features_bcm88230_c0(int unit, soc_feature_t feature)
{
    uint16      dev_id;
    uint8       rev_id;
    uint32      tme_mode, hybrid_mode;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    /* hardcoded soc features */
    switch (feature) {
	/* case soc_feature_dcb_type27:*/
	/* case soc_feature_dcb_type27:*/
	case soc_feature_dcb_type19:
	case soc_feature_schmsg_alias:
	case soc_feature_table_dma:
	case soc_feature_tslam_dma:
	case soc_feature_higig2:
	case soc_feature_mem_push_pop:
	case soc_feature_fifo_dma:
        case soc_feature_mc_group_ability:
        case soc_feature_packet_adj_len:
        case soc_feature_multi_sbus_cmds:
        case soc_feature_extended_cmic_error:
        case soc_feature_unimac: /* for lab bringup only */
        case soc_feature_phy_cl45:
        case soc_feature_mdio_enhanced:
        case soc_feature_sgmii_autoneg:
        case soc_feature_ingress_size_templates:
        case soc_feature_priority_flow_control:
        case soc_feature_source_port_priority_flow_control:
	    return TRUE;
	    break;
	default:
	    break;
    }

    /* soc features depends on configuration */
    if (SOC_CONTROL(unit) == NULL) {
        return(FALSE);
    }
    if (SOC_SBX_CONTROL(unit) == NULL) {
        return(FALSE);
    }
    if (SOC_SBX_CFG(unit) == NULL) {
        return(FALSE);
    }

    tme_mode = soc_property_get(unit, spn_QE_TME_MODE, SOC_SBX_CFG(unit)->bTmeMode);
    hybrid_mode = soc_property_get(unit, spn_HYBRID_MODE, SOC_SBX_CFG(unit)->bHybridMode);

    switch (feature) {
	case soc_feature_discard_ability:
            return( ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) ||
		     (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
		     (SOC_SBX_CFG(unit)->bTmeMode ==  SOC_SBX_QE_MODE_HYBRID)) ?
		    TRUE : FALSE );
	    break;
        case soc_feature_egress_metering:
            /* Support is available for egress shaping */
            SOC_DEBUG_PRINT((DK_VERBOSE , "rate egress_metering feature available\n"));
            return( TRUE );
            break;
        case soc_feature_cosq_gport_stat_ability:
            /* Support is available for gport statistics */
            SOC_DEBUG_PRINT((DK_VERBOSE , "gport statistics feature available\n"));
            return( TRUE );
            break;
        case soc_feature_standalone:
            if ((tme_mode == SOC_SBX_QE_MODE_TME) ||
                (tme_mode == SOC_SBX_QE_MODE_TME_BYPASS)) {
                return TRUE;
            } else {
                return FALSE;
            } 
            break;
        case soc_feature_hybrid:
            return( ( (hybrid_mode == TRUE) &&
                      (tme_mode != TRUE /* SOC_SBX_QE_MODE_TME */) ) ? TRUE : FALSE);
            break;
        case soc_feature_node:
            return(TRUE);
            break;
        case soc_feature_node_hybrid:
            return( ((tme_mode == SOC_SBX_QE_MODE_HYBRID) ? TRUE : FALSE) );
            break;
        case soc_feature_egr_independent_fc:
            switch (SOC_SBX_CFG(unit)->uFabricConfig) {
                case SOC_SBX_SYSTEM_CFG_VPORT:
                case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
                case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
                    return((SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl == TRUE) ? TRUE : FALSE);
                    break;

                case SOC_SBX_SYSTEM_CFG_DMODE:
                default:
                    return(FALSE);
                    break;
            }
            break;
        case soc_feature_egr_multicast_independent_fc:
	    return((SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) ? FALSE : TRUE);
            break;
	case soc_feature_discard_ability_color_black:
	    return (TRUE);
	    break;
        case soc_feature_distribution_ability:
	    /* ESET need to be awared for FIC traffic for node type */
	    return ( ((tme_mode == SOC_SBX_QE_MODE_TME) ||
		      (tme_mode == SOC_SBX_QE_MODE_TME_BYPASS))
		      ? FALSE : TRUE );
	    break;
	default:
	    return FALSE;
    }
}
#endif  /* BCM_88230 */

#ifdef	BCM_53262
int
soc_features_bcm53262_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 53262_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    case soc_feature_mac_learn_limit:
    /* soc_feature_dodeca_serdes: for fiber with SerDes + PHY_PassThrough */
    case soc_feature_dodeca_serdes: 
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_field:
    case soc_feature_arl_mode_control:
    case soc_feature_tag_enforcement:
    case soc_feature_vlan_translation:
    case soc_feature_igmp_mld_support:
    case soc_feature_robo_ge_serdes_mac_autosync:
    case soc_feature_sgmii_autoneg:        
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_53262 */

#ifdef	BCM_53115
int
soc_features_bcm53115_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 53115_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_field:
    case soc_feature_field_tcam_parity_check:
    case soc_feature_arl_mode_control:
    case soc_feature_hw_dos_prev:
    case soc_feature_tag_enforcement:
    case soc_feature_igmp_mld_support:
    case soc_feature_vlan_translation:
    case soc_feature_802_3as:
    case soc_feature_color_prio_map:
    case soc_feature_eav_support:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_53115 */

#ifdef	BCM_53118
int
soc_features_bcm53118_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 53118_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_arl_mode_control:
    case soc_feature_hw_dos_prev:
    case soc_feature_tag_enforcement:
    case soc_feature_igmp_mld_support:
    case soc_feature_802_3as:
    case soc_feature_eav_support:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_53118 */

#ifdef  BCM_53280
int
soc_features_bcm53280_a0(int unit, soc_feature_t feature)
{
   switch (feature) {
    case soc_feature_l2_hashed:
    case soc_feature_stg:
    case soc_feature_vlan_translation:
    case soc_feature_vlan_action:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    case soc_feature_subport:
    case soc_feature_l2_pending:
    case soc_feature_mac_learn_limit:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_igmp_mld_support:
    case soc_feature_field:        
    case soc_feature_field_multi_stage:
    case soc_feature_field_tcam_hw_move:
    case soc_feature_hw_dos_prev:
    case soc_feature_hw_dos_report:
    case soc_feature_sgmii_autoneg:
    case soc_feature_field_tcam_parity_check:        
        return TRUE;
    default:
        return FALSE;
    }
}
int
soc_features_bcm53280_b0(int unit, soc_feature_t feature)
{
    char *s;
   switch (feature) {
    case soc_feature_l2_hashed:    
    case soc_feature_stg:
    case soc_feature_vlan_translation:
    case soc_feature_vlan_action:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    case soc_feature_subport:
    case soc_feature_l2_pending:
    case soc_feature_mac_learn_limit:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_igmp_mld_support:
    case soc_feature_field:        
    case soc_feature_field_multi_stage:
    case soc_feature_hw_dos_prev:
    case soc_feature_hw_dos_report:
    case soc_feature_sgmii_autoneg:
        return TRUE;
    case soc_feature_field_tcam_hw_move:
    case soc_feature_field_tcam_parity_check:        
        s = soc_property_get_str(unit, "board_name");
        if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0))
            return FALSE;
        else
            return TRUE;
    default:
        return FALSE;
    }

}
#endif  /* BCM_53280 */

#ifdef  BCM_53101
int
soc_features_bcm53101_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 53101_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_arl_mode_control:
    case soc_feature_hw_dos_prev:
    case soc_feature_tag_enforcement:
    case soc_feature_igmp_mld_support:
    case soc_feature_802_3as:
    case soc_feature_eav_support:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif  /* BCM_53101 */

#ifdef	BCM_53125
int
soc_features_bcm53125_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 53125_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_field:
    case soc_feature_field_tcam_parity_check:      
    case soc_feature_arl_mode_control:
    case soc_feature_hw_dos_prev:
    case soc_feature_tag_enforcement:
    case soc_feature_igmp_mld_support:
    case soc_feature_vlan_translation:
    case soc_feature_802_3as:
    case soc_feature_color_prio_map:
    case soc_feature_eav_support:
    case soc_feature_eee:
    case soc_feature_int_cpu_arbiter:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_53125 */

#ifdef	BCM_53128
int
soc_features_bcm53128_a0(int unit, soc_feature_t feature)
{
    COMPILER_REFERENCE(unit);
    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 53128_a0"));

    /* Wiil be added later */
    switch (feature) {
    case soc_feature_stg:
    case soc_feature_l2_hashed:
    case soc_feature_l2_lookup_cmd:
    case soc_feature_l2_lookup_retry:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_field: /* For auto voip feature */
    case soc_feature_arl_mode_control:
    case soc_feature_hw_dos_prev:
    case soc_feature_tag_enforcement:
    case soc_feature_igmp_mld_support:
    case soc_feature_802_3as:
    case soc_feature_eav_support:
    case soc_feature_eee:
    case soc_feature_int_cpu_arbiter:
        return TRUE;
    default:
        return FALSE;
    }
}
#endif	/* BCM_53128 */

#ifdef BCM_53600
int
soc_features_bcm53600_a0(int unit, soc_feature_t feature)
{
  char *s;
   switch (feature) {
    case soc_feature_l2_hashed:
    case soc_feature_stg:
    case soc_feature_vlan_translation:
    case soc_feature_vlan_action:
    case soc_feature_igmp_ip:
    case soc_feature_dscp:
    case soc_feature_auth:
    case soc_feature_subport:
    case soc_feature_l2_pending:
    case soc_feature_mac_learn_limit:
    /* soc_feature_robo_sw_override: Robo port MAC state maintained by CPU  */
    case soc_feature_robo_sw_override: 
#ifdef INCLUDE_MSTP
    case soc_feature_mstp:
#endif
    case soc_feature_igmp_mld_support:
    case soc_feature_field:        
    case soc_feature_field_multi_stage:
    case soc_feature_hw_dos_prev:
    case soc_feature_hw_dos_report:
    case soc_feature_sgmii_autoneg:
    case soc_feature_arl_mode_control:
        return TRUE;
    case soc_feature_field_tcam_hw_move:
    case soc_feature_field_tcam_parity_check:        
        s = soc_property_get_str(unit, "board_name");
        if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0))
            return FALSE;
        else
            return TRUE;
    default:
        return FALSE;
    }
}

#endif /* BCM_53600 */

#ifdef  BCM_56440
/*
 * BCM56440 A0
 */
int
soc_features_bcm56440_a0(int unit, soc_feature_t feature)
{
    int     ddr3_disable;
    uint16  dev_id;
    uint8   rev_id;

    SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," 56440_a0"));
    soc_cm_get_id(unit, &dev_id, &rev_id);

    ddr3_disable = ((dev_id == BCM56444_DEVICE_ID) || 
                    (dev_id == BCM56445_DEVICE_ID) ||  
                    (dev_id == BCM56446_DEVICE_ID) ||
                    (dev_id == BCM56447_DEVICE_ID) ||
                    (dev_id == BCM56448_DEVICE_ID)); 

    switch (feature) {
    case soc_feature_dcb_type21:
    case soc_feature_logical_port_num:
    case soc_feature_wlan:
    case soc_feature_gport_service_counters:
    case soc_feature_priority_flow_control:
#ifndef _KATANA_DEBUG
    /* Temporarily Disabled until implementation complete */
    case soc_feature_rcpu_priority:
    case soc_feature_field_vfp_flex_counter:
#endif
    case soc_feature_asf:
    case soc_feature_qcn:
    case soc_feature_egr_mirror_true:
    case soc_feature_trill:
    case soc_feature_two_ingress_pipes:
    case soc_feature_niv:
    case soc_feature_ip_source_bind:
    case soc_feature_vp_group_ingress_vlan_membership:
    case soc_feature_vp_group_egress_vlan_membership:
    case soc_feature_ets:
    case soc_feature_xy_tcam:
    case soc_feature_field_ingress_two_slice_types:
    case soc_feature_mmu_config_property:
        return FALSE;
    case soc_feature_dcb_type24:
    case soc_feature_multi_sbus_cmds:
    case soc_feature_cmicm:
    case soc_feature_bfd:
    case soc_feature_oam:
    case soc_feature_ptp:
    case soc_feature_mpls_entropy:
    case soc_feature_field_slices12:
    case soc_feature_field_virtual_slice_group:
    case soc_feature_field_intraslice_double_wide:
    case soc_feature_field_egress_flexible_v6_key:
    case soc_feature_field_multi_stage:
    case soc_feature_field_ingress_global_meter_pools:
    case soc_feature_field_egress_global_counters:
    case soc_feature_field_ing_egr_separate_packet_byte_counters:
    case soc_feature_field_ingress_ipbm:
    case soc_feature_global_meter:
    case soc_feature_mac_learn_limit:
    case soc_feature_mim:    
    case soc_feature_advanced_flex_counter:
    case soc_feature_time_support:
        return TRUE;
    case soc_feature_ces:
        /* For now, set this feature to False. During SOC reset, this may be
         * set to TRUE as per CES Enable Bond Option..
        */
        return FALSE;
    case soc_feature_ddr3:
        return !ddr3_disable;
    default:
        return soc_features_bcm56840_a0(unit, feature);
    }
}
#endif	/* BCM_56440 */

/*
 * Function:    soc_feature_init
 * Purpose:     initialize features into the SOC_CONTROL cache
 * Parameters:  unit    - the device
 * Returns:     void
 */
void
soc_feature_init(int unit)
{
    soc_feature_t       f;

    assert(COUNTOF(soc_feature_name) == (soc_feature_count + 1));

    sal_memset(SOC_CONTROL(unit)->features, 0,
               sizeof(SOC_CONTROL(unit)->features));
    for (f = 0; f < soc_feature_count; f++) {
        SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE,"%s : ", soc_feature_name[f]));
        if (SOC_DRIVER(unit)->feature(unit, f)) {
            SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," TRUE\n"));
            SOC_FEATURE_SET(unit, f);
        } else {
            SOC_FEATURE_DEBUG_PRINT((DK_VERBOSE," FALSE\n"));
        }
    }
}
