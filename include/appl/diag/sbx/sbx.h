/*
 * $Id: sbx.h 1.54.20.3 Broadcom SDK $
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
 * File:        sbx.h
 * Purpose:
 */

#ifndef _SOC_SBX_APPL_DIAG_H
#define _SOC_SBX_APPL_DIAG_H

#include <appl/diag/diag.h>
#include <appl/diag/sbx/register.h>

#define DCL_CMD(_f,_u)  \
        extern cmd_result_t     _f(int, args_t *); \
        extern char             _u[];

extern int bcm_sbx_cmd_cnt;
extern cmd_t bcm_sbx_cmd_list[];

extern soc_sbx_chip_info_t soc_sbx_chip_list[SOC_MAX_NUM_DEVICES];
extern int soc_sbx_chip_count;

DCL_CMD(cmd_sbx_afl, cmd_sbx_afl_usage)
DCL_CMD(cmd_sbx_age, cmd_sbx_age_usage)
DCL_CMD(cmd_sbx_board, cmd_sbx_board_usage)
DCL_CMD(cmd_sbx_break, cmd_sbx_break_usage)
#ifdef BROADCOM_SVK
DCL_CMD(cmd_sbx_fe2kxt_ddrconfig, cmd_sbx_fe2kxt_ddrconfig_usage)
#endif
DCL_CMD(cmd_sbx_ddr_train, cmd_sbx_ddr_train_usage)
DCL_CMD(cmd_sbx_ddr_tune, cmd_sbx_ddr_tune_usage)
DCL_CMD(cmd_sbx_sirius_ddr_mem_read, cmd_sbx_sirius_ddr_mem_read_usage)
DCL_CMD(cmd_sbx_sirius_ddr_mem_write, cmd_sbx_sirius_ddr_mem_write_usage)
DCL_CMD(cmd_sbx_sirius_ddr_phy_read, cmd_sbx_sirius_ddr_phy_read_usage)
DCL_CMD(cmd_sbx_sirius_ddr_phy_write, cmd_sbx_sirius_ddr_phy_write_usage)
DCL_CMD(cmd_sbx_sirius_ddr_phy_tune, cmd_sbx_sirius_ddr_phy_tune_usage)
DCL_CMD(cmd_sbx_sirius_ddr_phy_tune_auto, cmd_sbx_sirius_ddr_phy_tune_auto_usage)
DCL_CMD(cmd_sbx_print_info, cmd_sbx_print_info_usage)
DCL_CMD(cmd_sbx_fe2000_check_mem, cmd_sbx_fe2000_check_mem_usage)
DCL_CMD(cmd_sbx_fe2000_nic_config, cmd_sbx_fe2000_nic_config_usage)
DCL_CMD(cmd_sbx_fe2000_hc, cmd_sbx_fe2000_hc_usage)
DCL_CMD(cmd_sbx_fe2000_hd, cmd_sbx_fe2000_hd_usage)
#ifdef INCLUDE_TEST
DCL_CMD(cmd_sbx_fe2000_hg, cmd_sbx_fe2000_hg_usage)
DCL_CMD(cmd_sbx_fe2000_pkt_dump, cmd_sbx_fe2000_pkt_dump_usage)
#endif
DCL_CMD(cmd_sbx_fe2000_tcalscan, cmd_sbx_fe2000_tcalscan_usage)
#ifdef BROADCOM_SVK
DCL_CMD(cmd_sbx_fe2000_sync_ethernet, cmd_sbx_fe2000_sync_ethernet_usage)
#endif
DCL_CMD(cmd_sbx_fe2000_sep, cmd_sbx_fe2000_sep_usage)
DCL_CMD(cmd_sbx_field, cmd_sbx_field_usage)
DCL_CMD(cmd_sbx_fl, cmd_sbx_fl_usage)
DCL_CMD(cmd_sbx_failover_count, cmd_sbx_failover_count_usage)
DCL_CMD(cmd_sbx_fabric, cmd_sbx_fabric_usage)
DCL_CMD(cmd_sbx_forcemodmap, cmd_sbx_forcemodmap_usage)
DCL_CMD(cmd_sbx_gu2_cls_add, cmd_sbx_gu2_cls_add_usage)
DCL_CMD(cmd_sbx_gu2_cls_rem, cmd_sbx_gu2_cls_rem_usage)
DCL_CMD(cmd_sbx_gu2_demo, cmd_sbx_gu2_demo_usage)
DCL_CMD(cmd_sbx_gu2_get, cmd_sbx_gu2_get_usage)
DCL_CMD(cmd_sbx_gu2_readcount, cmd_sbx_gu2_readcount_usage)
DCL_CMD(cmd_sbx_gu2_set, cmd_sbx_gu2_set_usage)
DCL_CMD(cmd_sbx_gu2_util, cmd_sbx_gu2_util_usage)
DCL_CMD(cmd_sbx_g2p3_get, soc_sbx_g2p3_get_usage)
DCL_CMD(cmd_sbx_g2p3_set, soc_sbx_g2p3_set_usage)
DCL_CMD(cmd_sbx_g2p3_delete, soc_sbx_g2p3_delete_usage)
DCL_CMD(cmd_sbx_g2p3_util, cmd_sbx_g2p3_util_usage)
DCL_CMD(cmd_sbx_g2k_util, cmd_sbx_g2k_util_usage)
DCL_CMD(cmd_sbx_learn, cmd_sbx_learn_usage)
DCL_CMD(cmd_sbx_mac, cmd_sbx_mac_usage)    
DCL_CMD(cmd_sbx_lpm, cmd_sbx_lpm_usage)
DCL_CMD(cmd_sbx_mim_test, cmd_sbx_mim_test_usage)
DCL_CMD(cmd_sbx_lib_init, cmd_sbx_lib_init_usage)
DCL_CMD(cmd_sbx_mcast, cmd_sbx_mcast_usage)
DCL_CMD(cmd_sbx_mcfabinit_config, cmd_sbx_mcfabinit_config_usage)
DCL_CMD(cmd_sbx_mcfpga_rw, cmd_sbx_mcfpga_rw_usage)
DCL_CMD(cmd_sbx_mcinit_config, cmd_sbx_mcinit_config_usage)
DCL_CMD(cmd_sbx_mctune, cmd_sbx_mctune_usage)
DCL_CMD(cmd_sbx_mcenablesiports, cmd_sbx_mcenablesiports_usage)
DCL_CMD(cmd_sbx_mclcinit_config, cmd_sbx_mclcinit_config_usage)
DCL_CMD(cmd_sbx_mcremoveall, cmd_sbx_mcremoveall_usage)
DCL_CMD(cmd_sbx_mcstate_config, cmd_sbx_mcstate_config_usage)
DCL_CMD(cmd_sbx_mclcstandalone_config, cmd_sbx_mclcstandalone_config_usage)
DCL_CMD(cmd_sbx_mcpbinit_config, cmd_sbx_mcpbinit_config_usage)
DCL_CMD(cmd_sbx_multicast, cmd_sbx_multicast_usage)
DCL_CMD(cmd_sbx_mdio_serdes_read, cmd_sbx_mdio_serdes_read_usage)
DCL_CMD(cmd_sbx_mdio_serdes_write, cmd_sbx_mdio_serdes_write_usage)
DCL_CMD(cmd_sbx_mem_list, cmd_sbx_mem_list_usage)
DCL_CMD(cmd_sbx_mem_set, cmd_sbx_mem_set_usage)
DCL_CMD(cmd_sbx_mem_show, cmd_sbx_mem_show_usage)
DCL_CMD(cmd_sbx_mem_instance, cmd_sbx_mem_instance_usage)
DCL_CMD(cmd_sbx_mim, cmd_sbx_mim_usage)
DCL_CMD(cmd_sbx_pbmp, cmd_sbx_pbmp_usage)
DCL_CMD(cmd_sbx_policer, cmd_sbx_policer_usage)
DCL_CMD(cmd_sbx_port, cmd_sbx_port_usage)
DCL_CMD(cmd_sbx_port_stat, cmd_sbx_port_stat_usage)
DCL_CMD(cmd_sbx_port_rate, cmd_sbx_port_rate_usage)
DCL_CMD(cmd_sbx_print_errors, cmd_sbx_print_errors_usage)
DCL_CMD(cmd_sbx_print_counts, cmd_sbx_print_counts_usage)
DCL_CMD(cmd_sbx_qe2000_mvtget, cmd_sbx_qe2000_mvtget_usage)
DCL_CMD(cmd_sbx_qe2000_mvtset, cmd_sbx_qe2000_mvtset_usage)
DCL_CMD(cmd_sbx_qe2000_qsindirect, cmd_sbx_qe2000_qsindirect_usage)
DCL_CMD(cmd_sbx_qinfo_get, cmd_sbx_qinfo_get_usage)
DCL_CMD(cmd_sbx_reg_get,cmd_sbx_reg_get_usage)
DCL_CMD(cmd_sbx_reg_set,cmd_sbx_reg_set_usage)
DCL_CMD(cmd_sbx_reg_verify,cmd_sbx_reg_verify_usage)
DCL_CMD(cmd_sbx_soc, cmd_sbx_soc_usage)
DCL_CMD(cmd_sbx_stable, cmd_sbx_stable_usage)
DCL_CMD(cmd_sbx_stg, cmd_sbx_stg_usage)
DCL_CMD(cmd_sbx_train2k, cmd_sbx_train2k_usage)
DCL_CMD(cmd_sbx_trunk, cmd_sbx_trunk_usage)
DCL_CMD(cmd_sbx_vlan, cmd_sbx_vlan_usage)
DCL_CMD(cmd_sbx_xb_test_cnt, cmd_sbx_xb_test_cnt_usage)
DCL_CMD(cmd_sbx_tx_ring_get,cmd_sbx_tx_ring_get_usage)
DCL_CMD(cmd_sbx_completion_ring_get,cmd_sbx_completion_ring_get_usage)
DCL_CMD(cmd_sbx_reg_list,cmd_sbx_reg_list_usage)
DCL_CMD(cmd_sbx_clear, cmd_sbx_clear_usage)
DCL_CMD(cmd_sbx_counter,    cmd_sbx_counter_usage)
DCL_CMD(cmd_sbx_show, cmd_sbx_show_usage)
DCL_CMD(cmd_sbx_reg_modify, cmd_sbx_reg_modify_usage)
DCL_CMD(cmd_sbx_train, cmd_sbx_train_usage)
DCL_CMD(cmd_sbx_rx_init,cmd_sbx_rx_init_usage)
DCL_CMD(cmd_sbx_rx_stop,cmd_sbx_rx_stop_usage)
DCL_CMD(cmd_sbx_tx,cmd_sbx_tx_usage)
DCL_CMD(cmd_sbx_rx,cmd_sbx_rx_usage)
DCL_CMD(cmd_soc_sbx_rx, cmd_soc_sbx_rx_usage)
DCL_CMD(cmd_soc_sbx_tx, cmd_soc_sbx_tx_usage)
DCL_CMD(cmd_sbx_dump, cmd_sbx_dump_usage)
DCL_CMD(cmd_sbx_pvlan,  cmd_sbx_pvlan_usage)
DCL_CMD(cmd_sbx_xxsocreload, cmd_sbx_xxsocreload_usage)
DCL_CMD(cmd_sbx_g2xx_get, soc_sbx_g2xx_get_usage)
DCL_CMD(cmd_sbx_g2xx_set, soc_sbx_g2xx_set_usage)
DCL_CMD(cmd_sbx_g2xx_delete, soc_sbx_g2xx_delete_usage)
DCL_CMD(cmd_sbx_g2xx_util, cmd_sbx_g2xx_util_usage)
DCL_CMD(cmd_sbx_mem_write, cmd_sbx_mem_write_usage)
DCL_CMD(cmd_sbx_pcic_read, cmd_sbx_pcic_read_usage)
DCL_CMD(cmd_sbx_pcic_write, cmd_sbx_pcic_write_usage)
DCL_CMD(cmd_sbx_rx_mon, cmd_sbx_rx_mon_usage)
DCL_CMD(cmd_sbx_mem_modify, cmd_sbx_mem_modify_usage)
DCL_CMD(cmd_sbx_sot_policing, cmd_sbx_sot_policing_usage)
extern int sbx_diag_init(int brdtype);
extern void sbx_str_tolower(char *str);
/* callbacks to bcm_gport_traverse */
extern int _bcm_gport_delete(int unit,bcm_gport_t port,
                             int numq, uint32 flags,
                             bcm_gport_t gport, void *user_data);

extern int _bcm_gport_show(int unit,bcm_gport_t port,
                           int numq, uint32 flags,
                           bcm_gport_t gport, void *user_data);

extern int _bcm_gport_stat_enable_set(int unit,bcm_gport_t port,
                                      int numq, uint32 flags,
                                      bcm_gport_t gport, void *user_data);
extern int
sbx_cosq_gport_show(int unit,bcm_gport_t gport,
                    int verbose);

extern void
sbx_gport_map_show(int unit);

extern void
sbx_gport_map_delete(int unit);

extern void
sbx_gport_scheduler_show(int unit);

extern int
sbx_sirius_ingress_tree_show(int unit, int levels);

extern void
sbx_cosq_gport_get_node_port(int unit, bcm_gport_t gport, 
			     int *node, int *port);

extern void 
sbx_print_scheduler_tree(int unit, int level, int node);

extern void
sbx_gport_display(int unit, bcm_gport_t gport);

#endif /* _SOC_SBX_APPL_DIAG_H */
