/*
 * $Id: mbcm.c 1.37 Broadcom SDK $
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
 * File:        mbcm.c
 */

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/draco.h>
#include <bcm_int/esw/strata.h>

mbcm_functions_t mbcm_draco_driver = {
    /* L2 functions */
    bcm_draco_l2_init,
    bcm_draco_l2_term,
    bcm_draco_l2_addr_get,
    bcm_draco_l2_addr_add,
    bcm_draco_l2_addr_delete,
    bcm_draco_l2_conflict_get,

    /* Port table related functions */
    bcm_draco_port_cfg_init, 
    bcm_draco_port_cfg_get,
    bcm_draco_port_cfg_set,

    /* VLAN functions */
    bcm_draco_vlan_init,
    bcm_draco_vlan_reload,
    bcm_draco_vlan_create,
    bcm_draco_vlan_destroy,
    bcm_draco_vlan_port_add,
    bcm_draco_vlan_port_remove,
    bcm_draco_vlan_port_get,
    bcm_draco_vlan_stg_get,
    bcm_draco_vlan_stg_set,

    /* Draco trunking functions */
    bcm_draco_trunk_set,
    bcm_draco_trunk_get,
    bcm_draco_trunk_destroy,
    bcm_draco_trunk_mcast_join,

    /* Spanning Tree Group functions */
    bcm_draco_stg_stp_init,
    bcm_draco_stg_stp_get,
    bcm_draco_stg_stp_set,

    /* Multicasting functions */
    bcm_draco_mcast_addr_add,
    bcm_draco_mcast_addr_remove,
    bcm_draco_mcast_port_get,
    bcm_draco_mcast_init,
    _bcm_draco_mcast_detach,
    bcm_draco_mcast_addr_add_w_l2mcindex,
    bcm_draco_mcast_addr_remove_w_l2mcindex,
    bcm_draco_mcast_port_add,
    bcm_draco_mcast_port_remove,

    /* COSQ functions */
    bcm_draco_cosq_init,
    bcm_draco_cosq_detach,
    bcm_draco_cosq_config_set,
    bcm_draco_cosq_config_get,
    bcm_draco_cosq_mapping_set,
    bcm_draco_cosq_mapping_get,
    bcm_draco_cosq_port_sched_set,
    bcm_draco_cosq_port_sched_get,
    bcm_draco_cosq_sched_weight_max_get,
    bcm_draco_cosq_port_bandwidth_set,
    bcm_draco_cosq_port_bandwidth_get,
    bcm_draco_cosq_discard_set,
    bcm_draco_cosq_discard_get,
    bcm_draco_cosq_discard_port_set,
    bcm_draco_cosq_discard_port_get,
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_draco_cosq_sync,
#endif /* BCM_WARM_BOOT_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
    bcm_draco_cosq_sw_dump,
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

    /* Yes, we want the Strata version here */
#ifdef BCM_METER_SUPPORT
    /* Meter functions */
    bcm_strata_meter_init,
    bcm_strata_meter_create,
    bcm_strata_meter_delete,
    bcm_strata_meter_delete_all,
    bcm_strata_meter_get,
    bcm_strata_meter_set,
    bcm_strata_ffppacketcounter_set,
    bcm_strata_ffppacketcounter_get,
    bcm_strata_ffpcounter_init,
    bcm_strata_ffpcounter_create,
    bcm_strata_ffpcounter_delete,
    bcm_strata_ffpcounter_delete_all,
    bcm_strata_ffpcounter_set,
    bcm_strata_ffpcounter_get,
    bcm_strata_port_rate_egress_set,
    bcm_strata_port_rate_egress_get,
#endif /* BCM_METER_SUPPORT */

#ifdef INCLUDE_L3
    /* L3 functions */
    bcm_xgs_l3_tables_init,
    bcm_xgs_l3_tables_cleanup,
    bcm_xgs_l3_enable,
    bcm_xgs_l3_intf_get,
    bcm_xgs_l3_intf_get_by_vid,
    bcm_xgs_l3_intf_create,
    bcm_xgs_l3_intf_id_create,
    bcm_xgs_l3_intf_lookup,
    bcm_xgs_l3_intf_del,
    bcm_xgs_l3_intf_del_all,

    bcm_xgs_l3_get,
    bcm_xgs_l3_add,
    bcm_xgs_l3_del,
    bcm_xgs_l3_del_prefix,
    bcm_xgs_l3_del_intf,
    bcm_xgs_l3_del_all,
    bcm_xgs_l3_replace,
    bcm_xgs_l3_age,
    bcm_xgs_l3_traverse,

    bcm_xgs_l3_ip6_get,
    bcm_xgs_l3_ip6_add,
    bcm_xgs_l3_ip6_del,
    bcm_xgs_l3_ip6_del_prefix,
    bcm_xgs_l3_ip6_replace,
    bcm_xgs_l3_ip6_traverse,

    bcm_xgs_defip_cfg_get,
    bcm_xgs_defip_ecmp_get_all,
    bcm_xgs_defip_add,
    bcm_xgs_defip_del,
    bcm_xgs_defip_del_intf,
    bcm_xgs_defip_del_all,
    bcm_xgs_lpm_age,
    bcm_xgs_defip_traverse,

    bcm_xgs_ip6_defip_cfg_get,
    bcm_xgs_ip6_defip_ecmp_get_all,
    bcm_xgs_ip6_defip_add,
    bcm_xgs_ip6_defip_del,
    bcm_xgs_ip6_defip_traverse,

    bcm_xgs_l3_conflict_get,
    bcm_xgs_l3_info,
    bcm_xgs_lpm_sw_tbl_empty,
    bcm_xgs_lpm_hw_tbl_empty,
    bcm_xgs_lpm_compare_table,
    bcm_xgs_lpm_delete,
    bcm_xgs_lpm_insert,
    bcm_xgs_lpm_lookup,

    /* IPMC functions */
    bcm_xgs_ipmc_init,
    bcm_xgs_ipmc_detach,
    bcm_xgs_ipmc_enable,
    bcm_xgs_ipmc_src_port_check,
    bcm_xgs_ipmc_src_ip_search,
    bcm_xgs_ipmc_add,
    bcm_xgs_ipmc_delete,
    bcm_xgs_ipmc_delete_all,
    bcm_xgs_ipmc_lookup,
    bcm_xgs_ipmc_get,
    bcm_xgs_ipmc_put,
    bcm_xgs_ipmc_egress_port_get,
    bcm_xgs_ipmc_egress_port_set,    
    bcm_draco15_ipmc_repl_init,
    bcm_draco15_ipmc_repl_detach,
    bcm_draco15_ipmc_repl_get,
    bcm_draco15_ipmc_repl_add,
    bcm_draco15_ipmc_repl_delete,
    bcm_draco15_ipmc_repl_delete_all,
    bcm_draco15_ipmc_egress_intf_add,
    bcm_draco15_ipmc_egress_intf_delete,
    bcm_xgs_ipmc_age,
    bcm_xgs_ipmc_traverse,
#endif /* INCLUDE_L3 */
};
